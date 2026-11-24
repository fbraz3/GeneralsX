---
name: sync-thesuperhackers-upstream
description: Sync the repository with TheSuperHackers upstream by creating a dated sync branch, merging thesuperhackers/main, carefully resolving conflicts (preserving SDL3/DXVK/MiniAudio/NGMP, preventing Core duplicates, and following upstream determinism as the golden pattern), validating builds, and reporting deliverables.
---

# Sync TheSuperHackers Upstream

Perform a full upstream sync from the `thesuperhackers` remote into `GeneralsX` while preserving the cross-platform architecture and adopting upstream bugfixes, stability work, and determinism improvements.

---

## 1. Briefing & Divergence Context

`TheSuperHackers` and `GeneralsX` have complementary goals with distinct platform targets that must drive every merge decision:

- **`TheSuperHackers`**: Focuses on bug fixes, stability, optimizations, unifying `Generals/` and `GeneralsMD/` into `Core/`, cross-platform deterministic math (the golden pattern), and preserving compatibility with original Windows binaries and legacy GameSpy.
- **`GeneralsX`**: Focuses on native cross-platform execution (macOS ARM64, Linux x86_64, Windows) using **SDL3 + DXVK + MiniAudio + FFmpeg**, replacing the discontinued GameSpy service with **Next-Gen Multiplayer Protocol (NGMP / GeneralsOnline)** via WebSocket, REST, and GameNetworkingSockets (ICE/STUN/TURN).

---

## 2. Required Workflow

1. **Verify Clean State**: Check `git status --porcelain` is empty before starting. If uncommitted changes exist, stop and ask the user how to proceed.
2. **Create Sync Branch**: Determine the current local date and create a branch from updated `main` named `thesuperhackers-sync-MM-DD-YYYY`.
3. **Configure Remote & Fetch**:
   - Check if `thesuperhackers` remote exists; if not, add it (`git@github.com:TheSuperHackers/GeneralsGameCode.git`).
   - Run `git fetch thesuperhackers`.
4. **Merge Upstream**: Run `git merge --no-commit --no-ff thesuperhackers/main` into the new sync branch so the merge stays uncommitted until validation and the worklog update are complete.
5. **Conflict Resolution Plan**:
   Before editing conflicted files, analyze the conflict list and structure a resolution plan by subsystem:
   - Build system & CMake presets
   - `Core/` unification & `UD`/`DU` (modify/delete) conflicts
   - Platform abstraction layers (SDL3, DXVK, MiniAudio, FFmpeg)
   - Cross-platform determinism (`WWMath` / `gamemath` / `ScopedFPUGuard`)
   - NGMP / GeneralsOnline & shared WOL multiplayer UI
   - INI parsing & file load order
   - CI/CD & repository metadata
6. **Execute Resolution**: Resolve conflicts carefully step by step, documenting major decisions.
7. **Sanity Audits**:
   - Verify no unresolved merge markers remain (`<<<<<<<`, `=======`, `>>>>>>>`).
   - **Duplicate File Audit**: Verify files moved upstream to `Core/` were not left duplicated in `Generals/` or `GeneralsMD/`. Remove obsolete duplicates via `git rm`.
   - Remove any generated runtime/build artifacts (e.g., local shader/DXVK caches) from the working tree.
8. **Validate Builds & Smoke**:
   - Validate configure and build for one platform completely before moving to the next (do not run parallel platform validations).
   - Check macOS (`macos-vulkan`) and/or Linux (`linux64-deploy`) builds for both `GeneralsXZH` and `GeneralsX`.
   - Verify runtime smoke launch if applicable.
9. **Update Worklog & Commit**:
   - Update `docs/WORKLOG/YYYY-MM-DIARY.md` per project documentation rules.
   - Commit the merge result using Conventional Commits (e.g. `chore(sync): merge thesuperhackers/main into GeneralsX (MM-DD-YYYY)`).
   - Push the branch `thesuperhackers-sync-MM-DD-YYYY` to `origin`.

---

## 3. Critical Merge Instructions

Never use blanket conflict strategies (`--ours` / `--theirs`) across large areas of the tree. Analyze each conflict individually:

### Core Unification & Duplicate Prevention
- Upstream actively moves shared files from `Generals/` and `GeneralsMD/` into `Core/`.
- For modify/delete (`UD` / `DU`) conflicts where a file moved to `Core/`:
  - Follow the unified `Core/` layout.
  - Ensure all `GeneralsX` cross-platform improvements (SDL3 windowing, DXVK rendering, 64-bit integer casts, `WWMath` determinism) are ported into the `Core/` file.
  - Delete the obsolete duplicate in `Generals/` or `GeneralsMD/` with `git rm` and verify `CMakeLists.txt` references the `Core/` file.

### Cross-Platform Determinism (Golden Pattern)
- **TheSuperHackers' determinism architecture is our golden pattern.** When upstream updates `WWMath`, `gamemath`, or simulation math determinism, **adopt upstream's determinism updates** rather than reverting them, and evaluate them with extra care:
  - Verify all simulation math in `GameLogic` and shared math primitives (`Trig.cpp`, `Coord2D`, `Coord3D`, `BezierSegment`) routes through `WWMath` / `gamemath` (`gm_*f`) wrappers.
  - Ensure `double` promotion is not introduced in transcendental/power math evaluations (must stay `float` for x87/NEON bit-parity).
  - Verify NaN/Inf division safety (`WWMath::Div_Safe` or zero-guards before integer casts).
  - Ensure `ScopedFPUGuard` remains injected at `GameLogic::update()`, MiniAudio callbacks, and `View::pickDrawable()`.
  - Keep `-ffp-contract=off` globally enforced (no `-ffast-math` or FMA contraction).
  - Reconcile any `GeneralsX`-specific fixes (e.g. ARM64/Apple Silicon quirks) cleanly with upstream's updates.

### Platform Stack & Isolation
- Preserve **SDL3** (windowing/input), **DXVK** (graphics), **MiniAudio** (primary audio; OpenAL is deprecated), and **FFmpeg** (video).
- Enforce strict platform isolation: no Win32, Cocoa, or X11 calls in game logic (`GameEngine/`).
- Treat INI parser changes as high risk on macOS (check Apple deployment target compatibility for numeric parsing).
- **Windows transitive header removals**: When upstream removes broad Windows/ATL headers (e.g., `atlbase.h` from `PreRTS.h`), Windows-only NGMP/networking headers (such as `NetworkMesh.h` including `<ws2ipdef.h>`) can lose transitive `<winsock2.h>` includes, or collide with `<winsock.h>` pulled by `udp.h`/GameSpy. Ensure `<winsock2.h>` is included in `PreRTS.h` before `<windows.h>` (and before `<ws2ipdef.h>`/`<ws2tcpip.h>`).
- **Emscripten/POSIX macro collisions**: New upstream constants (e.g., `TextureAtlas::PAGE_SIZE`) may collide with C/POSIX macros in Emscripten/musl `<limits.h>` (`PAGE_SIZE`, etc.). Guard with `#ifdef <MACRO> / #undef <MACRO> / #endif` after system includes when needed, and verify `webgpu` and Windows `generalsxzh` CI targets.

### NGMP (GeneralsOnline) Preservation
- Upstream does not know about NGMP and actively modifies legacy GameSpy/Peer networking.
- Preserve all files under `GeneralsMD/Code/GameEngine/Source/GameNetwork/GeneralsOnline/`, `Core/GameEngine/Source/GameNetwork/GeneralsOnline/`, `cmake/ngmp.cmake`, and `#if defined(SAGE_USE_NGMP)` blocks.
- In shared multiplayer UI (`WOL*.cpp`, `MainMenu.cpp`), reconcile upstream fixes without restoring legacy `TheGameSpyInfo` or GameSpy queues over `NGMP_OnlineServicesManager`, `OnlineServices_LobbyInterface`, `TheNGMPGame`, and `NGMPEvent`.
- Preserve loading screen ticking (`GameSpyLoadScreen.cpp`, `MapTransferLoadScreen.cpp`) and `NextGenTransport.cpp` ticking to prevent GNS/ICE starvation.
- Consult `.github/instructions/ngmp.instructions.md` when resolving networking conflicts.

### CI/CD & Repository Infrastructure
- **Never overwrite `GeneralsX` CI/CD, repo config, or AI policy** with upstream files. Keep `.agents/`, `.github/workflows/`, `.github/ISSUE_TEMPLATE/`, `.github/instructions/`, `.github/prompts/`, `.github/copilot-instructions.md`, `AGENTS.md`, and `AI_POLICY.md` intact (`AGENTS.md` at the repository root is our sole source of truth, and `.github/copilot-instructions.md` only points to it). Reject any upstream additions/modifications to these paths, as well as any other upstream agent instruction files (such as `CLAUDE.md`).

---

## 4. Deliverables

After completing the sync, provide:
1. **Sync Summary**: Key upstream improvements, bugfixes, and determinism updates merged.
2. **Conflict Resolution Report**: Important merge decisions and rationale per subsystem.
3. **Risk Assessment**: Files or subsystems needing extra testing/review.
4. **Branch & Commit Info**: Branch name, commit SHA, push status, and PR URL (if requested).
5. **Required User Testing Checklist**:
   - Configure & build on all supported platforms (macOS, Linux, Windows)
   - Confirm zero duplicate files between `Core/` and `Generals/`/`GeneralsMD/`
   - Game launch & Main menu
   - Skirmish gameplay & cross-platform determinism (Mac ARM64 vs Linux x86_64)
   - Campaign flow
   - NGMP online multiplayer (login, lobby roster, staging sync, P2P ICE match start)
   - Audio playback (MiniAudio) & Video playback (FFmpeg)
   - Renderer stability (DXVK/Vulkan) & Input handling (SDL3)
   - Mod loading & any areas touched by conflicts
