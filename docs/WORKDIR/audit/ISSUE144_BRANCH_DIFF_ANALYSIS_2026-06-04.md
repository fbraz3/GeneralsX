# Issue #144 Branch Diff Analysis

**Date:** 2026-06-04
**Branch:** `fix/issue-144-macos-cyrillic-text`
**Compared against:** `a7279e682` (main, 2026-06-02)
**Branch HEAD at time of analysis:** `5b8b43697`
**Status:** Branch breaks macOS replay CI (CRC mismatch at frame 1700/6900). User reports this branch has accumulated "changes we shouldn't have made" and broke the game engine.

## 1. Scope of the diff

- **Total files touched (incl. docs, scripts, workflows):** 101
- **Code files (`.cpp`/`.h`/`.cmake`/CMake):** 86
- **Code line count:** +2,338 / -1,853

The vast majority of the diff (Recorder.h/cpp, NetPacket.h/cpp, NetCommandList/Ref, GameEngine.h/cpp, Audio, AI, several Object/Update files, etc.) is the **2026-05-21 thesuperhackers upstream sync** that was merged into this branch. That sync is already on main and is **not** the source of the regression.

This report focuses on the **Issue #144 work authored by fbraz3** — the commits and code that target Cyrillic font rendering. Those are the changes that have to be reviewed and trimmed before any future re-application.

## 2. Commit Inventory (fbraz3 only)

Sorted oldest → newest. Grouped by purpose. The grouping reflects my judgment of what each commit is for, not the commit message (which sometimes mixes categories).

### 2.1. The actual Cyrillic fix (essential, keep these)
| SHA | Purpose | Files |
|---|---|---|
| `70fcb1fdd` | **CORE FIX**: `uselocale(newlocale(LC_CTYPE_MASK, "UTF-8", 0))` wrap around `vswprintf` in `UnicodeString::format_va` — fixes macOS rejection of non-ASCII wide-char format strings | `Core/GameEngine/Source/Common/System/UnicodeString.cpp` |
| `135b1b64d` | Portability fix for the above: guard `<xlocale.h>` behind `__APPLE__`, define `_GNU_SOURCE` on Linux | `Core/GameEngine/Source/Common/System/UnicodeString.cpp` |

These two commits are the **only** commits that actually solve the reported bug. Reporter confirmed `Строительство: 57%` renders correctly with just these applied (no other Cyrillic commits).

### 2.2. Cyrillic font pipeline (necessary for the fix to work, but contains redundancy)
| SHA | Purpose | Files |
|---|---|---|
| `d8fd346e3` | First attempt at Cyrillic font fallback: introduced `LoadUnicodeFallbackFont` with a hardcoded fallback list | `GeneralsMD/.../GUI/W3DGameFont.cpp`, `Generals/.../GUI/W3DGameFont.cpp` |
| `8de42b104` | Break circular fallback: skip preferred font if it equals base font | `GeneralsMD/.../GUI/W3DGameFont.cpp`, `Generals/.../GUI/W3DGameFont.cpp` |
| `15e669dcf` | macOS Fontconfig override during deploy (system Arial fallback) | `scripts/build/macos/deploy-macos-zh.sh` |
| `6b1c77e72` | **Drawable caption font resolution** (`ResolveDrawableCaptionFont`) — adds Arial Unicode MS as a final fallback | `GeneralsMD/.../GameClient/Drawable.cpp` (and style-only edits in InGameUI.cpp, GameWindowManager.cpp) |

### 2.3. Mod/CSF robustness (overlapping, partially redundant)
| SHA | Purpose | Files |
|---|---|---|
| `3a28555c5` | **CSF label fallback** (incomplete russifier CSF): adds `m_fallbackStringInfo`/`m_fallbackStringLUT`/`m_fallbackTextCount` members; refactors `getCSFInfo`/`parseCSF` to take parameters; loads a second CSF instance at `FileInstance 1` and falls back to it in `fetch()` | `Core/GameEngine/Source/GameClient/GameText.cpp` |
| `38f0e49db` | **Language.ini fallback chain**: when `registryLanguage != "English"`, load stock `Data\English\Language.ini` with `INI_LOAD_MULTIFILE` to fill missing keys | `Core/GameEngine/Source/GameClient/GlobalLanguage.cpp` |
| `ab2268e5c` | **Constraint on the previous**: only load stock English if `m_unicodeFontName.isEmpty()` AND `Data\English\Language.ini` is present in the deploy. Fixes the regression on brazilian deploys (INI_CANT_OPEN_FILE) and on localizations that already have a UnicodeFontName | `Core/GameEngine/Source/GameClient/GlobalLanguage.cpp` |

### 2.4. Tooltip work (only useful for 2D tooltips, not captions)
| SHA | Purpose | Files |
|---|---|---|
| `70fcb1fdd` (sub-section) | `overrideTooltipGadgetFont()` — replaces "Arial" in 2D tooltip gadgets with a Unicode-capable font. Useful for in-game build-cost tooltips | `GeneralsMD/.../GUICallbacks/ControlBarPopupDescription.cpp`, `Generals/.../GUICallbacks/ControlBarPopupDescription.cpp` |

### 2.5. Diagnostic instrumentation (kept per reporter request, but mostly noise)
| SHA | Purpose | Files |
|---|---|---|
| `9df81a3e7` | First wave of `[GX-ISSUE144]` stderr logs across font resolution, language loading, drawable caption, control bar tooltip | 7 files in `GeneralsMD/Code/GameEngine/Source/GameClient/` + `W3DGameFont.cpp` |
| `70fcb1fdd` (sub-section) | Second wave: FreeType path/face logging, Cyrillic delegation logging, `Store_Freetype_Char` U+0400–U+04FF logging, `ResolveDrawableCaptionFont` logs, `drawConstructPercent` logs | `Core/Libraries/Source/WWVegas/WW3D2/render2dsentence.cpp`, `GeneralsMD/.../GUI/W3DGameFont.cpp`, `GeneralsMD/.../GameClient/Drawable.cpp` |

### 2.6. CI / replay work (orthogonal, came from a different problem)
| SHA | Purpose | Files |
|---|---|---|
| `be37079be` | Restore retail CRC pacing compatibility (`RETAIL_COMPATIBLE_CRC` block in `canUpdateRegularGameLogic`/`update`) | `Generals/.../GameEngine.cpp`, `GeneralsMD/.../GameEngine.cpp` |
| `924bdcfa9` | Temporarily disable replay jobs in CI | `.github/workflows/replay-tests.yml` |
| `85b0d1d8b` | Add replay build fingerprints | `.github/workflows/replay-tests.yml` |
| `e6b8a2f5a` | Add provenance diagnostics | `.github/workflows/replay-tests.yml` |
| `57bdf0df9` | Drop host mapcache before tests | `.github/workflows/replay-tests.yml` |
| `0bc3a40d0` | Show full debug in replay CI | `.github/workflows/replay-tests.yml` |

### 2.7. Defensive (orthogonal to Cyrillic, written in panic at the end)
| SHA | Purpose | Files |
|---|---|---|
| `e38c00a2f` | Gate `Drawable` caption path + `AlternateUnicodeFont` chain on `TheGlobalData->m_headless`. **Locally verified: does NOT fix the CRC mismatch.** | `GeneralsMD/.../GameClient/Drawable.cpp`, `GeneralsMD/.../GUI/W3DGameFont.cpp` |

### 2.8. Docs (no code)
| SHA | Files |
|---|---|
| `92116b715` | `README.md` |
| `e6ad163d1` | `AGENTS.md` |
| `c9b458822` | `.github/copilot-instructions.md` |
| `5b8b43697` | `docs/DEV_BLOG/2026-06-DIARY.md` |
| (inside 6b1c77e72, 38f0e49db, 70fcb1fdd, ab2268e5c, e38c00a2f) | Various diary entries and `docs/WORKDIR/archive/reports/ISSUE144_SESSION_SUMMARY_*.md` |
| (inside 8b2af810d) | `docs/WORKDIR/lessons/2026-05-LESSONS.md` |

## 3. What the upstream sync brought (NOT my work, already on main)

`8b2af810d chore(sync): merge thesuperhackers main 2026-05-21` and `b904a8502 merge: bring in main (docs restructure + scripts + replay tests)` plus `68b5c486f Merge pull request #146 ...` brought the following upstream changes into our branch:

- `bf8d5be02` `bugfix(gameengine): Fix logic in GameEngine::canUpdateRegularGameLogic()` (TheSuperHackers)
- `842c7a532` `fix(recorder): Fix memory leak of RecorderClass::m_crcInfo` (TheSuperHackers)
- `7cb3c11e7` `fix(particlesys): Add or simplify null-checks` (TheSuperHackers)
- `7dc2632ea` `bugfix: Restore retail compatibility after retail behavior flags change` (TheSuperHackers)
- `186314fd5` `ci(workflows): move linux replay to flatpak` (PR #143)
- `5852010ea` `bugfix(aigroup): Prevent game crash when a player is selected in Replay playback` (TheSuperHackers)
- `5a4187cea` `fix(network): Verify accepted type of incoming game messages` (TheSuperHackers)
- `c1bce4f5d` `perf(network): Replace extern global network variables with const ones` (TheSuperHackers)
- `f7ecdb4cc` `refactor(network): Use switch cases in utility functions` (TheSuperHackers)
- `f8476f04e` `bugfix(physics): Prevent building crash damage from dealing damage to other objects` (TheSuperHackers)
- `159447416` `bugfix(physics): Prevent dead units from repeatedly dealing crash damage` (TheSuperHackers)
- `5ef79850b` `fix(gameengine): Remove music tracks as prerequisite to initialize the game engine` (TheSuperHackers)
- `2219f63c8` `refactor(netpacket): Simplify NetPacket functions for packet buffer reads` (TheSuperHackers)
- `8951ad908` `fix(object): Prevent crash if Object::m_drawable is a nullptr on save load` (TheSuperHackers)
- `d8fd346e3` (the first Cyrillic fallback, before they had a "W3DGameFont" path)
- `be37079be` `fix(replay): restore retail crc pacing compatibility` (GeneralsX/our work, but kept)

These are all on main (`a7279e682`) and were never the cause of the replay-CI regression on this branch — the only thing main had to fix was the new replay-CI infrastructure (re-recording). They are not the regression. They are what we should keep.

## 4. The actual Cyrillic fix — verified minimum

The reporter confirmed exit-0 on a build that contained **only** the `vswprintf` `uselocale` wrap (`70fcb1fdd` + `135b1b64d`), without any of the font-fallback/CSF/Language/tooltip work. This means:

> The vswprintf locale fix is sufficient on its own to make the construction caption `Строительство: 57%` render correctly in 3D on macOS.

Everything else in section 2.2–2.4 is **supportive infrastructure** that became relevant only when other edge cases (russifier mod, tooltip gadget fonts, custom `UnicodeFontName` in Language.ini) showed up. The reporter is now confident the bug is fixable with just `70fcb1fdd` + `135b1b64d`.

## 5. The bug in `6b1c77e72` — what should never have been committed

The commit `6b1c77e72 fix(localization): harden drawable caption fallback` is named like a localization fix but the actual diff in `GeneralsMD/Code/GameEngine/Source/GameClient/Drawable.cpp:calcPhysicsXformWheels` includes physics changes that have **nothing to do with fonts or localization**:

### 5.1. "Autolevel" pitch/roll spring branch (cosmetic, mostly harmless)

```cpp
if (!airborne) {
    m_locoInfo->m_pitchRate += ((-PITCH_STIFFNESS * (m_locoInfo->m_pitch - groundPitch)) + (-PITCH_DAMPING * m_locoInfo->m_pitchRate));
    if (m_locoInfo->m_pitchRate > 0.0f)
        m_locoInfo->m_pitchRate *= 0.5f;
    m_locoInfo->m_rollRate += ((-ROLL_STIFFNESS * (m_locoInfo->m_roll - groundRoll)) + (-ROLL_DAMPING * m_locoInfo->m_rollRate));
}
else {
    //Autolevel   <-- new branch added in 6b1c77e72
    m_locoInfo->m_pitchRate += ( (-PITCH_STIFFNESS * m_locoInfo->m_pitch) + (-PITCH_DAMPING * m_locoInfo->m_pitchRate) );
    m_locoInfo->m_rollRate += ( (-ROLL_STIFFNESS * m_locoInfo->m_roll) + (-ROLL_DAMPING * m_locoInfo->m_rollRate) );
}
```

This is the "autolevel" effect: spring-damper toward 0 pitch/roll while airborne. The concept is plausible (vehicles self-level mid-air). **But** we did not validate this against retail behavior. It almost certainly changes the in-game state hash at any frame where a vehicle is mid-air, which would explain a CRC mismatch at frame 1700/6900.

Note: `cc4a683fd bugfix(water): Fix river visuals in black shroud (#2749)` on upstream also happens to add the same autolevel block — this commit was authored 4 days **after** ours. We may have inspired it, or the change was independently added. Either way, upstream kept it; we don't need to keep ours.

### 5.2. **Broken roll clamp — almost certainly the CRC-mismatch culprit**

```cpp
// compute total pitch and roll of tank
info.m_totalPitch = m_locoInfo->m_pitch + m_locoInfo->m_accelerationPitch;

// THis logic had recently been added to Drawable::applyPhysicsXform(), which was naughty, since it clamped the roll in every drawable in the game
// Now only motorcycles enjoy this constraint
Real unclampedRoll = m_locoInfo->m_roll + m_locoInfo->m_accelerationRoll;
info.m_totalRoll = (unclampedRoll > 0.5f && unclampedRoll < -0.5f ? unclampedRoll : 0.0f);
```

The condition `unclampedRoll > 0.5f && unclampedRoll < -0.5f` is **logically impossible** for any real value. It is `&&` where the author meant `||`. The result is `info.m_totalRoll` is always `0.0f`, regardless of actual roll.

Original line was simply:
```cpp
info.m_totalRoll = m_locoInfo->m_roll + m_locoInfo->m_accelerationRoll;
```

This means:
1. Every drawable's render matrix is now rotated 0 around the X axis where it should be rotated by the actual roll.
2. Every drawable's roll contribution to physics Xform is now identically zero.
3. The recorded replays on `ci-replays-repo` were made against the original (non-buggy) code, so the roll differs at every frame, so the CRC differs at frame 1700/6900.

**This is almost certainly the regression.** The faulty condition was a typo introduced in the same commit that was supposed to "harden the drawable caption fallback" — a commit that was never reviewed for physics correctness because the reviewer was looking at font code.

### 5.3. Wheel offsets consolidated (a real but unjustified change)

```cpp
// before
if (newInfo.m_frontLeftHeightOffset < m_locoInfo->m_wheelInfo.m_frontLeftHeightOffset) {
    m_locoInfo->m_wheelInfo.m_frontLeftHeightOffset += (...)/2.0f;
} else {
    m_locoInfo->m_wheelInfo.m_frontLeftHeightOffset = newInfo.m_frontLeftHeightOffset;
}
if (newInfo.m_frontRightHeightOffset < m_locoInfo->m_wheelInfo.m_frontRightHeightOffset) {
    m_locoInfo->m_wheelInfo.m_frontRightHeightOffset += (...)/2.0f;
} else {
    m_locoInfo->m_wheelInfo.m_frontRightHeightOffset = newInfo.m_frontRightHeightOffset;
}
// rear left, rear right: same pattern

// after
if (newInfo.m_frontLeftHeightOffset < m_locoInfo->m_wheelInfo.m_frontLeftHeightOffset) {
    m_locoInfo->m_wheelInfo.m_frontLeftHeightOffset += (...)/2.0f;
    m_locoInfo->m_wheelInfo.m_frontRightHeightOffset = m_locoInfo->m_wheelInfo.m_frontLeftHeightOffset;   // <-- consolidation
} else {
    m_locoInfo->m_wheelInfo.m_frontLeftHeightOffset = newInfo.m_frontLeftHeightOffset;
    m_locoInfo->m_wheelInfo.m_frontRightHeightOffset = m_locoInfo->m_wheelInfo.m_frontLeftHeightOffset;   // <-- consolidation
}
// rear: same consolidation
```

The "Right" wheel is now forced to equal the "Left" wheel. This is a substantive behavior change that:
- (a) does not match retail (we cannot confirm it matches original Generals/Zero Hour code by sight),
- (b) is unrelated to Cyrillic or localization,
- (c) changes the wheel animation/state for every drawable in every frame.

The whole roll-offset block (lines that compute `m_frontRightHeightOffset += -SPRING_FACTOR*(rollHeight/3+rollHeight/2);` etc.) was also commented out:

```cpp
/*
if (rollHeight>0) { ... } else { ... }
*/
```

This is just a regression in suspension dynamics that contributes to the CRC mismatch in a way that depends on which vehicles are turning on the map.

### 5.4. The `Drawable::Drawable` ctor changes (mostly fine)

`ResolveDrawableCaptionFont()` and the ctor wiring is the actually-asked-for Cyrillic fix. It is correct and small.

### 5.5. Style-only changes (irrelevant)

- `m_pitchRate *= 0.5f;` removed in the "autolevel" branch
- whitespace + braces in InGameUI.cpp
- whitespace + braces in GameWindowManager.cpp
- `return ;` → `return;` in `calcPhysicsXformMotorcycle` (cosmetic)
- `Real` → int conversion in `screen.y = REAL_TO_INT(...)` → `screen.y = ... + ...` (removed REAL_TO_INT — possible regression in 2D positioning, but only for `ICON_BOMB_REMOTE`)

None of these are the regression source.

## 6. Other "not the actual fix" items

### 6.1. CSF fallback refactor (`3a28555c5`)

The refactor of `getCSFInfo` and `parseCSF` to take parameters is large. It changed the function signature from "uses class members" to "uses arguments". This is correct for the fallback use case, but:

- The fallback only matters when a mod provides an incomplete CSF.
- The signature change means the function is more flexible but also more verbose.
- It adds `m_fallbackStringInfo`, `m_fallbackStringLUT`, `m_fallbackTextCount` members, so the GameTextManager is heavier.
- The fallback only resolves labels missing in the main CSF; the bsearch is done twice on miss.
- This is sound code but it is **not** needed for the Cyrillic caption fix in 3D, only for any 2D UI text via `TheGameText->fetch()`.

### 6.2. Language.ini fallback (`38f0e49db` + `ab2268e5c`)

Correct as a russifier recovery, but only matters when the user's deploy has a `Language.ini` that does not define `UnicodeFontName`. For the brazilian deploy (CI), this is **not** the active code path because brazilian Language.ini defines UnicodeFontName.

This is the source of the **brazilian deploy regression** that was then fixed by `ab2268e5c`. Without `ab2268e5c`, the brazilian deploy throws `INI_CANT_OPEN_FILE` at init. With `ab2268e5c`, the fallback is gated and inert for brazilian.

### 6.3. Tooltip font override (`70fcb1fdd` sub-section)

`overrideTooltipGadgetFont()` is good code but only matters for **2D build-cost tooltip rendering**. It is not on the caption path.

It also re-orders the early-return condition in `repopulateBuildTooltipLayout` from `(!prevWindow || !m_buildToolTipLayout)` to `(!m_buildToolTipLayout)` followed by `(!prevWindow)` and a hidden check. This is a *behavior change* in the tooltip code path: previously, a null `prevWindow` would skip repopulation; now it only does so if the layout is hidden. This could potentially affect replay-CRC for tooltips.

### 6.4. Headless guards (`e38c00a2f`)

Locally verified: do not fix the CRC mismatch. The in-game hash is still `0xBAF0EDA0` after the guards are applied. Pushed to CI for confirmation anyway.

## 7. What is the actual minimum fix?

To fix Issue #144 on a fresh branch from main:

1. Cherry-pick or reapply `70fcb1fdd` (the `vswprintf` `uselocale` wrap).
2. Cherry-pick or reapply `135b1b64d` (the Linux portability fix).
3. (Optional) Cherry-pick `d8fd346e3` + `8de42b104` for the Cyrillic font fallback in `W3DGameFont`, if you want the toolbar/tooltip/menu text to also look right.
4. (Optional) Cherry-pick `15e669dcf` for the macOS Fontconfig Arial override.
5. (Optional) Cherry-pick `6b1c77e72` **but** with the `Drawable::calcPhysicsXformWheels` block reverted to its original form, and with the ctor / `ResolveDrawableCaptionFont` portion only.

Do **not** cherry-pick:
- `3a28555c5` (CSF fallback) — only matters for russifier mods, large refactor.
- `38f0e49db` (unconstrained Language.ini fallback) — buggy, replaced by `ab2268e5c`.
- `ab2268e5c` — fine on its own but the brazilian CI doesn't exercise it.
- `9df81a3e7` (diagnostics) — noise; remove when done.
- `0bc3a40d0` (CI debug) — only useful while investigating.
- `e38c00a2f` (headless guards) — defensive but ineffective for CRC; can be re-applied later if needed.
- The `tooltip repopulate` reorder inside `70fcb1fdd` — unrelated to captions; verify against retail.

## 8. Verifying the broken roll clamp hypothesis

If we want to confirm the roll-clamp bug is the cause of the CRC mismatch:

1. Revert only the roll-clamp line in `6b1c77e72` (and the autolevel block, the wheel consolidation, the commented-out roll offset block).
2. Build, deploy, run the same headless replay test.
3. If the CRC matches the recorded replay (`0x41B96B1D` at frame 6900), the regression is confirmed and the fix is "just don't touch physics in this branch".

A bisect-based confirmation script can be:

```bash
git diff a7279e682..HEAD -- 'GeneralsMD/Code/GameEngine/Source/GameClient/Drawable.cpp' \
  | grep -E "m_totalRoll|Autolevel|m_frontRightHeightOffset = m_locoInfo|m_pitchRate \+= \( \(-PITCH_STIFFNESS \* m_locoInfo->m_pitch\)"
```

to get the lines that need to be reverted.

## 9. Recommendation for the future clean branch

Start from main (`a7279e682`). Apply only:

1. `70fcb1fdd` (vswprintf locale fix) + `135b1b64d` (Linux portability) — required.
2. `6b1c77e72` with a **clean diff** that:
   - adds `ResolveDrawableCaptionFont()`,
   - wires it in the ctor and `setCaptionText` and `drawConstructPercent`,
   - adds the diagnostic logs (they helped the reporter confirm),
   - does **not** touch `calcPhysicsXformWheels`, `applyPhysicsXform`, or any wheel/suspension logic.
3. `d8fd346e3` + `8de42b104` (Cyrillic font fallback) — required if you want consistent Cyrillic outside captions.
4. `ab2268e5c` (constrained Language.ini fallback) — required for the brazilian CI.

Re-record replays against the new build before pushing (so the CRC baseline is correct), or accept that the replay-CI needs to be re-recorded once and then runs deterministically from there.

The "extra" commits (`3a28555c5`, `15e669dcf`, tooltip work, `9df81a3e7` diagnostics, `e38c00a2f` headless guards) can come later in their own PRs.

## 10. CI / upstream sync items that should be in the clean branch (from main, not from this branch)

- `be37079be` `fix(replay): restore retail crc pacing compatibility` (GeneralsX)
- All TheSuperHackers sync work from `8b2af810d`
- `186314fd5` `ci(workflows): move linux replay to flatpak` (PR #143)
- The new replay-CI infrastructure from `b17e14ca6` (Merge branch 'fix/replay-tests')

These are already on main so they will come along automatically.

## 11. Final word

The "what did we do so far" answer to the user is:

- **The actual Cyrillic fix** is 2 commits: `70fcb1fdd` + `135b1b64d`. About 22 lines of real code.
- **The "supporting" Cyrillic work** is 5 more commits: `d8fd346e3`, `8de42b104`, `15e669dcf`, `6b1c77e72` (caption path), `ab2268e5c` (brazilian deploy fix). Adds another ~150 lines, scattered across 4-5 files.
- **The collateral damage** is also in `6b1c77e72`: about 90 lines of physics changes (`calcPhysicsXformWheels`, autolevel, broken roll clamp, wheel consolidation). These almost certainly break CRC. They were probably meant to be a separate PR for a different issue but got rolled into the localization commit because the reviewer's attention was on font code.
- **The defensive work** (`e38c00a2f`) does not fix anything.
- **The diagnostics** (`9df81a3e7` + sub-section of `70fcb1fdd`) helped the reporter confirm the fix and should be removed only after the new clean branch is verified.

The user is correct: we modified physics in a commit named "harden drawable caption fallback" and the consequence is a CRC mismatch in the replay CI. Going forward, the clean branch should not touch `Drawable::calcPhysicsXformWheels`, `Drawable::applyPhysicsXform`, or `m_wheelInfo` at all.
