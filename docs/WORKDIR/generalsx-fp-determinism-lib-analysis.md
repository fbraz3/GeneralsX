# Cross-Platform FP Determinism for GeneralsX - Library Analysis

**Context:** [GeneralsX](https://github.com/fbraz3/GeneralsX) needs game-physics math to be
bit-identical across operating systems and CPU architectures (macOS ARM64, Linux x86_64,
future Windows) so that lockstep multiplayer and replays do not desync (CRC mismatch).

**Candidates evaluated:**

- [freemint/fdlibm](https://github.com/freemint/fdlibm)
- [TheAssemblyArmada/GameMath](https://github.com/TheAssemblyArmada/GameMath) - **already integrated** (PR #176)
- [streflop (Nicolas Brodu)](https://nicolas.brodu.net/programmation/streflop/index.html)

---

## Bottom line first

Switching to **fdlibm or streflop will almost certainly NOT fix the remaining desyncs**, because
the desyncs left are not caused by the transcendental functions - which GameMath already handles.
Keep GameMath. The open PR #217 and issue #215 already point at the real culprits: **FMA
contraction, implicit `double` promotion, and FP-environment leaks**. None of those is a
"which libm" problem.

---

## The critical realization: all three are the *same* library underneath

Sun netlib fdlibm (1993)  <- the common ancestor
  |-- FreeBSD msun  <-- GameMath    (float-only, C, BSD-2)      <- already in use
  |-- GNU libm 2.4  <-- streflop    (+SoftFloat, C++, LGPL)
  |-- freemint/fdlibm               (Atari m68k/ColdFire fork, C)

PR #176 even describes GameMath's routed functions as *"fdlibm-based deterministic functions"* -
because they are. Therefore:

- **Switching to `freemint/fdlibm` is circular.** It replaces a modern, cross-platform,
  CMake-integrated fdlibm derivative (GameMath) with an older one whose maintained fork targets
  **Atari**. Identical math results, worse integration. Don't.
- **streflop's transcendentals are also fdlibm-lineage**, so its `sin/cos/tan/sqrt/atan2` will not
  return different values than GameMath for the same `float` inputs. Its value is *elsewhere*
  (`Double` type + SoftFloat + FP-env management), not in "better math."

---

## Why GameMath did not stop the desyncs

The remaining divergence between **macOS ARM64 <-> Linux x86_64** comes from things a math library
cannot fix by itself:

| # | Root cause | Evidence in repo | Does a lib swap fix it? |
|---|---|---|---|
| 1 | **FMA contraction** - `a*b+c` fused into one op with different rounding. Clang on Apple Silicon and GCC/Clang on x86 do this at `-O2` by default. | Classic ARM64-vs-x86 divergence; no global flag yet | **No** - compiler flag, not a lib |
| 2 | **Implicit `double` promotion / overload ambiguity** - calls silently hitting native `double` math, bypassing GameMath | PR #217: fixing `SqrtOrigin`/`Atan2Origin`/`PowOrigin` ambiguity + explicit `(float)` casts | **No** - type/call-site fix |
| 3 | **`double`-precision intermediates** (x87 80-bit vs SSE 64-bit vs NEON) | PR #217 commit: *"isolate double-precision math to prevent x87/NEON desync"* | **Partially** - GameMath is **float-only**; here streflop/fdlibm *could* help |
| 4 | **FP-environment leaks** - audio thread leaves rounding/FTZ-DAZ skewed; `setFPMode()` not called on all paths | Issue #215: OpenAL thread corrupts mouse-pick math | **No** - must set/restore FP env at boundaries |
| 5 | **Non-FP divergence** - pointer ordering, `unordered_map` iteration, uninitialized reads, RNG stream | CRC/replay checker exists, but these are invisible to any libm | **No** |

Only cause **#3** is one where a different library is even relevant - and only *after* #1, #2, and
#4 are fixed.

---

## Head-to-head, tailored to GeneralsX

| Dimension | freemint/fdlibm | **GameMath (current)** | streflop |
|---|---|---|---|
| Language / API | C | C (`gm_` prefix) | C++ (namespaced) + C shim |
| Precision covered | float + double + long double | **float only** | Simple + **Double** (+partial Extended) |
| FP-env / denormal management | No | No | Yes (`streflop_init()`, FTZ/DAZ, x87 squasher) |
| Deterministic RNG included | No | No | Yes (Mersenne Twister) |
| Software FPU (bit-identical on *all* archs) | No | No (relies on IEEE754 hardware) | Yes (SoftFloat mode) |
| License | Sun permissive / BSD-2 | LGPL-2.1 |
| GPLv3 compatible (GeneralsX is GPLv3) | Yes | Yes | Yes (LGPL->GPLv3 is fine) |
| Modern-platform port state | Atari-focused | Clean CMake, cross-platform | Needs Spring-style porting for MSVC x64 / ARM |
| Provenance fit | Low (Atari) | **Highest** - by Gun1Blade, core **Thyme** dev; Thyme reimplements *this exact SAGE/Generals engine* | High (Spring RTS ships it across the same OS set) |
| Fixes FMA (#1)? | No | No | No |
| Fixes double paths (#3)? | Yes (has double) | No (float only) | Yes (has Double) |
| Fixes FP-env leak (#4)? | No | No | Packages it, but you still must call it |

**Licensing note:** the objection GameMath's README raises against streflop ("LGPL limits static
linking") **does not apply here**, because GeneralsX is GPLv3 and GPLv3 absorbs LGPL. That concern
only affects *proprietary* projects. License is not a deciding factor.

---

## When streflop is actually worth adding

Consider streflop **only as a second phase**, and only if - after fixing FMA + promotion + FP-env -
divergence persists. Its genuine unique advantages here are exactly two:

1. **The `Double` type** - if the simulation has sync-critical `double` math (cause #3), GameMath
   cannot cover it and streflop can.
2. **SoftFloat mode** - the *only* option here that yields **bit-identical results across every
   architecture** (ARM64, x86_64, x87, future Windows) because it bypasses the hardware FPU
   entirely. Cost: slower, C++, heavier.

If adopting it, **use the Spring RTS streflop fork**, not Brodu's 2012 v0.3 upstream - Spring's fork
adds MSVC x64 assembly (`FPUSettings.asm`) and has years of hardening across the exact
Windows/Linux/macOS matrix GeneralsX targets. The original v0.3 will not build cleanly on modern
MSVC/ARM. Its x87-specific machinery is largely dead weight (Mac/Linux 64-bit and future Windows
64-bit use SSE2/NEON, not x87).

---

## Action plan that actually kills the desyncs (prioritized)

1. **Disable FMA contraction globally** - `-ffp-contract=off` (Clang/GCC), `/fp:precise` and *no*
   `/fp:fast` (MSVC), plus `#pragma STDC FP_CONTRACT OFF` in hot simulation TUs. Most likely fix for
   ARM64<->x86 divergence, and free.
2. **Ban fast-math everywhere** - no `-ffast-math`, `-Ofast`, `/fp:fast`, including in third-party
   dependencies.
3. **Finish PR #217** - eliminate every implicit `double` promotion in `GameLogic`; ensure each
   `sqrt/sin/cos/tan/pow/atan2` in the sim resolves to the GameMath float overload. Grep the logic
   for bare `sqrt(`, `pow(`, `sin(`, `atan2(`.
4. **Handle the double-precision paths (cause #3)** - force them to float where safe, or route them
   through a deterministic double libm. This is the only step where library choice matters
   (streflop `Double`, or fdlibm double).
5. **Lock the FP environment at boundaries (issue #215)** - call `setFPMode()` at the entry of
   `GameLogic::update()` **and** in the mouse-pick / audio / render callbacks that leak it; restore
   on exit. Set FTZ/DAZ consistently on all threads.
6. **Build a determinism harness** - CRC the full sim state each frame, replay the same input on Mac
   ARM64 + Linux x64, and binary-search the first diverging frame. This identifies the exact call
   that desyncs - far more useful than swapping libraries blindly.
7. **Rule out non-FP divergence** - `unordered_map` iteration order, pointer-dependent branching,
   uninitialized struct padding fed into the CRC, RNG seed/stream.

Do 1-6 first. If - and only if - divergence remains on `double` logic afterward, adopt the
**Spring streflop fork** for those paths (or go full SoftFloat for guaranteed bit-identical results
at a performance cost). Raw `freemint/fdlibm` is not recommended at all: it adds nothing GameMath
does not already provide, with worse integration.

---

## Verdict  

| Question | Answer |
|---|---|
| Switch to raw fdlibm? | **No** - circular (GameMath *is* fdlibm) and an integration regression. |
| Keep GameMath? | **Yes** - correct choice, same provenance as the Generals reimplementation (Thyme), already integrated. |
| Adopt streflop now? | **Not yet** - only after fixing flags/promotion/FP-env, and only for `double` paths or a full SoftFloat guarantee. Use the Spring fork if you do. |
| Are the remaining desyncs a library problem? | **No** - they are build-flag + type-promotion + FP-environment-discipline problems (PR #217 / issue #215 are already on the right track). |
