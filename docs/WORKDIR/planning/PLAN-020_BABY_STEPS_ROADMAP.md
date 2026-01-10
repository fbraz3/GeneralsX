# PLAN-020: Baby-Steps Roadmap (SDL2 + Vulkan, Cross-Platform)

## Purpose

Define a pragmatic, low-risk sequence of milestones to drive GeneralsX toward a stable, cross-platform (macOS/Linux/Windows) playable state, while keeping merges from upstream feasible.

## Context (What We Know)

- Upstream (TheSuperHackers/GeneralsGameCode) supports both **VS6** and **VS2022**, and documents **CMake-based** workflows for modern toolchains.
- EA’s original source drop highlights that compiling with modern MSVC requires **extensive changes**, and even more for Win64.
- This repository is already oriented around **SDL2 + Vulkan**, with explicit macOS portability handling (MoltenVK portability enumeration/subset).

## Fresh Start vs Continuing This Codebase

### Why a Fresh Start Can Be Attractive

A clean restart from upstream can reduce uncertainty when you suspect that accumulated local changes introduced regressions.
It can also make it easier to attribute failures (“is it upstream, or us?”) and to keep upstream merges low-friction.

### Why a Fresh Start Can Be Risky

In practice, a full restart is often a hidden rewrite:

- You may lose (or need to re-discover) dozens of hard-earned fixes that are not obvious from symptoms alone (paths, timing, threading, asset loading edge cases, portability flags, etc.).
- You may reintroduce already-solved cross-platform problems and spend time proving the same conclusions again.
- If your current branch is already close to the desired architecture (SDL2 + Vulkan), the opportunity cost of restarting can be very high.

### Decision Criteria (Use This to Choose)

Prefer **continuing the current codebase** when:

- You can reproduce the bug reliably and it is likely localized.
- You have confidence that platform/Vulkan bring-up infrastructure is mostly correct.
- You need to keep forward momentum on playability checkpoints (A–D).

Prefer a **fresh start from upstream** when:

- There is no reliable repro and failures appear “random” or cross-cutting.
- The current branch diverged heavily from upstream with broad refactors.
- Upstream merges are consistently painful and block progress.
- You cannot easily isolate the suspect changes (no clear commit boundaries, large patch sets).

### Recommended Approach: Controlled Fresh Start (Hybrid)

Instead of a full restart (discard everything) or continuing blindly, do a **controlled fresh start**:

1. Create a new branch from a known-good upstream baseline.
1. Re-apply changes as a small number of *reviewable patch series*, in this order:

    - Platform layer (SDL2 window/event loop, filesystem paths, timing, threads)
    - Configuration (INI/registry replacement, user data locations)
    - Renderer backend plumbing (DX8Wrapper-facing API + Vulkan backend)
    - Platform-specific Vulkan portability (macOS: portability enumeration/subset)

1. After each series, validate with the same Checkpoints A–D.
1. If a regression appears, you have a narrow suspect range and can fix it without guessing.

This hybrid keeps the main benefit of a “fresh start” (clean provenance and easier debugging) while preserving the value of previously solved problems.

### Practical Recommendation for GeneralsX

Given that this repository already contains explicit cross-platform foundations (SDL2 config paths and Vulkan/MoltenVK portability handling), a **full fresh start is usually not the most feasible path**.
If you’re worried about hidden regressions, the controlled fresh start approach above is the best risk-reduction strategy.

## Principles

- **Prefer stable boundaries**: keep platform glue in shims; keep game logic untouched.
- **One cross-platform backend** is better than multiple “half backends”.
- **Avoid wrapper chains** (D3D8→D3D9→DXVK→Vulkan→MoltenVK→Metal) as a long-term plan.
- **Make upstream merges routine**: reduce diff size and keep changes localized.

## Recommended Order (High Level)

### 1) Branching / Upstream Hygiene

- Maintain a clean branch that tracks upstream regularly (merge/fetch cadence).
- Maintain a focused cross-platform branch that only contains:

  - platform abstractions (filesystem, windowing, timers, threads)
  - renderer boundary work (DX8Wrapper-facing, backend underneath)
  - minimal diagnostic/logging improvements needed for bring-up

Outcome: smaller, reviewable diffs and fewer painful rebases.

### 2) Toolchain Baseline (Don’t Build the Future on VS6)

- Day-to-day builds should target:

  - **Windows**: CMake + MSVC 2022
  - **macOS/Linux**: CMake + Clang

- Keep VS6 only for historical binary matching investigations, not for day-to-day porting.

Rationale:

- Upstream explicitly invests in modern toolchains and CMake.
- EA’s drop warns that “just compiling on modern MSVC” is a non-trivial modernization project by itself.

### 3) Platform Layer First (SDL2-first at the System Boundary)

Treat SDL2 as the “OS layer” for:

- window + event pump
- input
- timing
- filesystem user paths
- threading primitives (where needed)

Rationale:

- If the event loop + file paths aren’t correct, graphics work becomes impossible to debug.

### 4) Renderer Next (Vulkan-native Under a Stable Engine API)

- Keep the engine-facing API surface stable (DX8Wrapper-style calls).
- Implement/iterate the Vulkan backend beneath it.

Important macOS note:

- When using MoltenVK, Vulkan device enumeration requires `VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR` and commonly `VK_KHR_portability_subset`.

### 5) Choose Platform Focus (Linux vs macOS)

- If **macOS (Apple Silicon)** is the primary target: keep MoltenVK portability requirements and validation workflows on by default.
- If **Linux** is preferred as bring-up target: still enforce portability constraints early so macOS doesn’t become a rewrite.

Recommendation for this repo:

- **macOS-first** is acceptable because portability enumeration/subset handling is already present.

## Concrete Milestones / Checkpoints

These are the “baby steps” that prevent endless refactors.

### Checkpoint A — Boot to Menu

- Game starts, loads assets, reaches splash/main menu reliably.
- Config and working directories are correct on all platforms.

### Checkpoint B — UI + Shell Map

- Navigate menus; shell map loads.
- No fatal rendering errors; steady frame loop.

### Checkpoint C — Skirmish Bring-up

- Start skirmish, spawn units, run 5 minutes.
- Audio active, input stable, no crash on common interactions.

### Checkpoint D — Stability + Relaunch

- Clean exit; relaunch works.
- Config persistence verified (INI paths, user data).

## What NOT To Do (Common Traps)

- Don’t treat DXVK/DXGL-style wrappers as the final architecture on macOS.

  - They are valuable bring-up tools, but the long-term maintenance/debugging cost is high.

- Don’t move to Win64 early.

  - Make the game stable and deterministic cross-platform in 32-bit first.

- Don’t scatter platform-specific `#ifdef`s into gameplay code.

  - Keep platform decisions inside shims and the renderer backend boundary.

## Risk Register (Short)

- **Portability gaps** (macOS): missing portability flags/extensions leads to “no devices” or subtle feature mismatch.
- **Asset/config path drift**: different platform paths break loading in ways that look like renderer bugs.
- **Backend mismatch**: multiple partially-working renderers increase surface area and regressions.
- **Merge debt**: large refactors reduce upstream merge frequency and increase conflicts.

## Evidence / References

- Upstream build guides (VS6/VS2022/CMake):

  - <https://github.com/TheSuperHackers/GeneralsGameCode/wiki/build_guides>

- EA source drop notes on modern MSVC conversion cost:

  - <https://github.com/electronicarts/CnC_Generals_Zero_Hour>

- DXVK (D3D8/9/10/11 → Vulkan; includes “DXVK Native”):

  - <https://github.com/doitsujin/dxvk>

- MoltenVK (Vulkan portability layer on Metal):

  - <https://github.com/KhronosGroup/MoltenVK>

- D3D8-to-D3D9 translation reference (useful as a tactical bridge):

  - <https://github.com/crosire/d3d8to9>
