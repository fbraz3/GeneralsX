# 2026-03 Lessons Learned

## Session 2026-03-31 - Main menu fallback labels must be owned by menu lifecycle

- Problem: The GeneralsX watermark fallback label remained visible after leaving the main menu and entering gameplay.
- Root cause: The fallback `GameWindow` was created from the root window manager and not consistently tied to the main menu parent/lifecycle.
- Fix: Create the fallback label under `parentMainMenu` and destroy/reset it in both menu shutdown paths (`shutdownComplete` and `MainMenuShutdown`) for Generals and Zero Hour.
- Validation: After build and runtime verification, watermark appears in main menu only and no longer leaks into in-game scenes.
- Prevention: Any temporary/fallback UI created for menu rendering must be parent-owned by menu containers and explicitly torn down on all menu exit paths.

## Session 2026-03-29 - DXVK util_math.h must include cstddef on macOS clang arm64

- Problem: macOS Zero Hour build failed while compiling DXVK `d3d9_options.cpp` with `unknown type name 'size_t'; did you mean 'std::size_t'?`.
- Root cause: `references/fbraz3-dxvk/src/util/util_math.h` declared `CACHE_LINE_SIZE` using `size_t` but only included `<cmath>`, relying on transitive includes that are not guaranteed by clang arm64.
- Fix: Added `#include <cstddef>` in `src/util/util_math.h` on DXVK fork branch.
- Validation: Reconfigured with `-DSAGE_DXVK_USE_LOCAL_FORK=ON` and rebuilt `z_generals`; final link step for `GeneralsMD/GeneralsXZH` completed and binary exists in `build/macos-vulkan/GeneralsMD/GeneralsXZH`.
- Prevention: For DXVK utility headers consumed broadly across targets, include direct standard headers for foundational types (`size_t`, fixed-width ints) instead of relying on indirect includes.

## Session 2026-03-24 - Docker host UID/GID propagation must include the vcpkg mount strategy

- Problem: Docker-based Linux configure/build scripts wrote root-owned files into the repository on macOS/Linux hosts.
- Root cause: The containers ran as root while bind-mounting the workspace, and the modern scripts also diverged from the documented vcpkg strategy by using a named Docker volume instead of a host-owned path.
- Fix: Run Docker build/configure/smoke containers with the host UID:GID, set ephemeral writable HOME/XDG cache directories inside the container, and mount vcpkg from a host-owned path (`~/.generalsx/vcpkg` by default, overridable with `VCPKG_DIR`).
- Validation: Script audit confirmed every active `docker run` path now propagates host UID/GID except the legacy wrapper, which was already doing so.
- Prevention: When a container must write into a bind-mounted workspace, treat `--user <host_uid>:<host_gid>` and a host-owned cache mount as a single change; fixing only one side leaves either ownership regressions or permission failures.

## Session 2026-03-24 - Upstream merge can silently make SDL3GameEngine abstract via virtual signature drift

- Problem: macOS Zero Hour build failed in `GeneralsMD/Code/Main/SDL3Main.cpp` with `error: allocating an object of abstract class type 'SDL3GameEngine'`.
- Root cause: Upstream sync changed `GameEngine` pure virtual `createParticleSystemManager` signature to `createParticleSystemManager(Bool dummy)`, but SDL3 backends in both `GeneralsMD` and `Generals` still used `createParticleSystemManager(void)`.
- Fix: Updated SDL3GameEngine headers and implementations in both game variants to match `Bool dummy` signature; kept behavior unchanged with `(void)dummy` and existing `W3DParticleSystemManager` return.
- Validation: `cmake --build build/macos-vulkan --target z_generals` returned `EXIT:0`.
- Prevention: After every upstream merge, diff all pure virtual interface declarations (`GameEngine`, `SubsystemInterface`, device interfaces) against SDL3/Win32 implementations before first full build.

## Session 2026-03-24 - Do not force Linux OpenAL driver list on macOS

- Problem: macOS runtime launched and rendered normally but had no audio output.
- Symptom: Startup logs showed `ALSOFT_DRIVERS=pulse,alsa,oss,jack,null,wave` being set in `SDL3Main.cpp` before audio initialization.
- Root cause: `FilterPipeWireOpenAL()` was executed on non-Linux platforms and forced a Linux-specific backend list. On macOS this excludes CoreAudio and can fall through to `null`, producing silence.
- Fix: Scoped the PipeWire/OpenAL workaround to Linux only in both `GeneralsMD/Code/Main/SDL3Main.cpp` and `Generals/Code/Main/SDL3Main.cpp`; non-Linux now keeps default OpenAL backend selection.
- Validation: macOS build + deploy + runtime test confirmed audio playback restored; log now reports `OpenAL: keeping default driver selection on non-Linux platform`.
- Prevention: Keep backend workaround env vars platform-scoped. Never export Linux backend lists (`pulse/alsa/...`) on macOS where CoreAudio must remain selectable.

## Session 2026-03-24 - macOS bundle @rpath scan must check staged sibling dylibs first

- Problem: `bundle-macos-zh.sh` emitted repeated `WARNING: Unable to resolve dependency` messages for `@rpath/libgamespy.dylib`, `@rpath/libdxvk_d3d8.0.dylib`, `@rpath/libdxvk_d3d9.0.dylib`, and `@rpath/libopenal.1.dylib` even though those libraries were already copied into `Contents/Resources/lib`.
- Root cause: The recursive dependency resolver only searched `LC_RPATH` expansions and external fallback paths for `@rpath/*`, but did not try the current loader directory first. For staged bundle libs with self/sibling links, this produced false unresolved warnings.
- Fix: In both macOS bundle scripts, `resolve_dep_path()` now checks `${loader_dir}/${dep_name}` before iterating `LC_RPATH` candidates.
- Validation: Re-running `./scripts/build/macos/bundle-macos-zh.sh` completed with zero `WARNING: Unable to resolve dependency` lines.
- Prevention: During macOS bundle traversal, always resolve `@rpath` against staged sibling dylibs first, then `LC_RPATH`, then toolchain fallbacks.

## Session 2026-03-21 - Rotor streak artifacts from volumetric edge-pair UB and unbounded extrusion

- **Problem**: Animated helicopter rotor shadows produced rotating black streaks extending to the screen edge.
- **Root cause**:
  1. Silhouette edge sorting in `W3DVolumetricShadow::constructVolume` and `constructVolumeVB` used `Int` pointer reinterprets on `Short` arrays to swap edge pairs, which is strict-aliasing/alignment UB on 64-bit/ARM and can corrupt strip connectivity.
  2. Shadow extrusion distance (`vectorScaleMax`) was passed to volume construction without clamping in `updateMeshVolume`, so shallow light rays could generate runaway extrusion spikes.
- **Fix**:
  - Replaced `Int` reinterpret-cast swaps with explicit `Short` pair swaps.
  - Clamped `vectorScaleMax` to `MAX_EXTRUSION_LENGTH` before calling `constructVolume` / `constructVolumeVB`.
- **Validation**: `GeneralsXZH` macOS build and deploy completed successfully after the patch.
- **Prevention**: Avoid packed type-punning in geometry index pipelines; always clamp world-space extrusion lengths before feeding volume builders.

## Session 2026-03-21 - Avoid Matrix4x4 transpose shortcuts in ZH volumetric shadow world transforms

- **Problem**: Animated object shadows in Zero Hour rendered with incorrect orientation while static shadows remained correct.
- **Symptom**: Moving/animated casters (for example rotor or moving sub-mesh cases) produced visibly wrong projected/stencil shadow geometry.
- **Root cause**: Zero Hour `W3DVolumetricShadow.cpp` had a refactor that replaced the stable `To_D3DMATRIX(...)` path with manual `Matrix4x4(...).Transpose()` and direct `D3DMATRIX` cast in three render paths (`RenderMeshVolume`, `RenderDynamicMeshVolume`, and bounds rendering). This introduced matrix-convention drift versus the Generals base implementation.
- **Fix**: Restored Generals-stable matrix conversion in ZH using `To_D3DMATRIX(*meshXform)` (and `To_D3DMATRIX(identity)` for bounds path), removing manual transpose/cast usage.
- **Validation**: `GeneralsXZH` macOS build completed successfully after patch.
- **Prevention**: For shadow/render transform uploads, prefer engine conversion helpers (`To_D3DMATRIX`) over manual transpose/cast patterns, and always diff against Generals base before accepting ZH-only math changes.

## Session 2026-03-20 - DXVK on macOS requires DXVK_WSI_DRIVER + SDL3 installed for meson

- **Problem**: CI-built macOS .app crashes with "unknown exception" from `TheDisplay->init()`.
- **Root cause (multi-layered)**:
  1. `DXVK_WSI_DRIVER` env var is **required** on non-Win32 platforms by DXVK (it throws `DxvkError` if not set and the platform is not Win32). Neither run scripts nor CI workflow set it.
  2. `DxvkError` did NOT inherit `std::exception`, so the game's `catch(std::exception& e)` could not catch it → always "unknown exception".
  3. CI did NOT have `sdl3` installed via Homebrew, so DXVK's meson found only SDL2 (via ffmpeg) and compiled with `-DDXVK_WSI_SDL2` only. Setting `DXVK_WSI_DRIVER=SDL3` at runtime then fails with "Failed to initialize WSI." because the SDL3 WSI backend was not compiled in. The game uses SDL3 windows, so SDL2 WSI would also fail at Vulkan surface creation.
  4. `VK_ICD_FILENAMES` was deprecated in Vulkan Loader 1.3.236+ — must also set `VK_DRIVER_FILES`.
- **Fixes**:
  - `DxvkError` now inherits `std::exception` in DXVK fork's `util_error.h`.
  - `wsi_platform.cpp` auto-selects SDL3 > SDL2 > GLFW when `DXVK_WSI_DRIVER` is unset.
  - All macOS run scripts set `DXVK_WSI_DRIVER="SDL3"`.
  - `build-macos.yml` now runs `brew install sdl3` so DXVK gets compiled with SDL3 WSI.
  - All bundle/deploy run.sh scripts set both `VK_ICD_FILENAMES` and `VK_DRIVER_FILES`.
- **Prevention**: DXVK native builds include SDL2 **and** SDL3 WSI backends when both are installed. The correct one (SDL3 for us) must be selected at runtime via `DXVK_WSI_DRIVER`. CI environments must explicitly install SDL3 via Homebrew before running the cmake configure step, or DXVK will silently fall back to SDL2-only.

## Session 2026-03-19 - Bundle must include non-system dylibs from host toolchains

- Problem: Local bundles could launch on the build machine but miss host-linked non-system dylibs (for example Homebrew-installed libraries) on another machine.
- Symptom: Runtime dyld failures in distributed bundles even though core project dylibs were copied.
- Root cause: Bundle scripts copied only known project/runtime libraries and did not recursively collect absolute non-system dependencies reported by `otool -L`.
- Fix: Added recursive dependency scanning in macOS bundle scripts to copy non-system absolute-path dylibs into the bundle `Resources/lib` directory.
- Prevention: For macOS distribution artifacts, always verify linked dependencies with `otool -L` and bundle everything outside `/System/Library` and `/usr/lib`.

## Session 2026-03-19 - `@rpath` dependencies need explicit resolution during bundle scan

- Problem: Bundle startup failed with `dyld` error for `@rpath/libsharpyuv.0.dylib` referenced by `libwebp.7.dylib`.
- Root cause: External dependency collector skipped all `@*` entries from `otool -L`, so indirect `@rpath` libraries were never copied.
- Fix: Resolve `@rpath`, `@loader_path`, and `@executable_path` against `LC_RPATH` plus fallback Homebrew paths during recursive dylib scan.
- Prevention: Never discard `@rpath` entries in macOS bundle dependency traversal; resolve and copy or fail with actionable diagnostics.

## Session 2026-03-19 - Avoid GNU-only `head -n -2` in macOS scripts

- Problem: The macOS bundle script failed at the final archive listing step even after generating the zip successfully.
- Symptom: Script exited non-zero with `head: illegal line count -- -2` on macOS (BSD userland).
- Root cause: The script used GNU-style `head -n -2`, which is not supported by BSD `head`.
- Fix: Replaced `tail/head` trimming with portable `sed '1,3d;$d'` when printing `unzip -l` output.
- Prevention: In cross-platform shell scripts, avoid GNU-specific flags for `head`, `sed`, and `stat`; prefer POSIX-compatible expressions.

## Session 2026-03-17 - macOS CI bundle must match local deploy runtime contract

- Problem: The GitHub Actions macOS bundle packed dylibs at the runtime root, but `run.sh` exported `DYLD_LIBRARY_PATH` to `${SCRIPT_DIR}/lib`, which does not exist in the artifact.
- Symptom: CI bundle was "built successfully" but runtime launch could fail to resolve dynamic libraries and Vulkan ICD setup differed from local deploy.
- Root cause: CI used a generic wrapper (`scripts/qa/smoke/run-bundled-game.sh`) that expects a `lib/` layout and does not export `VK_ICD_FILENAMES`, while local deploy scripts assume root-level dylibs and set a local ICD manifest.
- Fix: Updated `.github/workflows/build-macos.yml` to include `resources/dxvk/dxvk.conf`, generate a macOS-specific `run.sh` aligned with local deploy (`DYLD_LIBRARY_PATH=${SCRIPT_DIR}`, `VK_ICD_FILENAMES=${SCRIPT_DIR}/MoltenVK_icd.json`), and keep required-lib checks focused on runtime-critical files.
- Prevention: For macOS, CI packaging must follow the same runtime contract as `scripts/build/macos/deploy-macos-*.sh` (library layout, ICD env vars, and DXVK config handling) to avoid "CI green, runtime broken" regressions.

## Session 2026-03-16 - Asset root must be resolved before BIG scan for packaged layouts

- Problem: BIG loading historically assumed assets lived in the current working directory (or registry-only install path), which breaks packaged distribution where binaries and data are split.
- Symptom: Launching from app/deb/rpm style layouts failed to find BIG/shader assets unless users started the process from the data folder.
- Root cause: `StdBIGFileSystem` and `Win32BIGFileSystem` initialized archive search paths with CWD-first assumptions and no unified runtime override chain.
- Fix: Implemented a deterministic lookup order for primary assets in both filesystem backends: `ENV > INI > default > current`.
  - ENV: `CNC_GENERALS_PATH` (Generals) and `CNC_GENERALS_ZH_PATH` (Zero Hour), with compatibility fallback for previous variable names
  - INI: `[Paths] AssetPath` from `Options.ini`
  - Default: registry install path and executable-relative fallbacks
  - Current: CWD as final fallback only
- Zero Hour extension: Added parallel lookup for base Generals assets (`CNC_GENERALS_PATH`, `[Paths] GeneralsAssetPath` in `Options.ini`, then existing install defaults).
- Validation: Incremental macOS builds succeeded for both `GeneralsXZH` and `GeneralsX` after the path-resolution changes.

## Session 2026-03-16 - GitHub Actions: Use granular change detection for multi-variant builds

- Problem: CI workflow used single boolean filter (`should-build`) that triggered all platform builds (Linux, macOS, Windows) when ANY file in [GeneralsMD, Generals, Core, cmake] changed.
- Symptom: Pushing a fix to Generals base game caused wasteful ZH builds on all platforms (Linux, macOS, Windows). Pushing a ZH fix unnecessarily triggered base-game Generals build even though it wasn't affected.
- Root cause: `paths-filter@v3` job in orchestrator only exposed one output flag, forcing all downstream build jobs to use the same trigger condition.
- Fix: Split `detect-changes` outputs into two independent filters:
  - `zh-should-build`: Triggered by GeneralsMD/, Core/, cmake/ changes
  - `base-should-build`: Triggered by Generals/, Core/, cmake/ changes
  - Each build job (build-linux, build-macos, build-macos-generals, build-windows) now consults only its relevant output.
- Validation: Workflow YAML verified for correct output references and conditional syntax.
- Prevention: When orchestrating multi-variant matrix builds, separate concern by input file paths and expose granular filter outputs. Each build job should specify its dependencies explicitly in the `if:` condition.

## Session 2026-03-15 - macOS base-game port should mirror Zero Hour script hardening

- Problem: Base Generals had no dedicated macOS build/deploy scripts while Zero Hour already carried validated logic for Vulkan SDK checks, DXVK dylib fallback paths, and runtime wrapper generation.
- Symptom: macOS base-game setup required manual command sequences and ad-hoc file copies, which increased drift risk versus Zero Hour.
- Root cause: Script parity work had focused on Zero Hour first, but the equivalent base-game path was never backported.
- Fix: Added `scripts/build/macos/build-macos-generals.sh` and `scripts/build/macos/deploy-macos-generals.sh`, then wired matching VS Code tasks for build/deploy/pipeline without auto-run.
- Validation: Built target `g_generals` on preset `macos-vulkan` and deployed runtime payload into `~/GeneralsX/Generals` successfully.
- Prevention: When platform scripts are introduced for one game variant, create a parity checklist and mirror the hardened behaviors to the sibling variant in the same session.

## Session 2026-03-14 - Base Generals language detection parity with Zero Hour

- Problem: Base Generals on Linux always attempted `Data\\english\\Language.ini` even on localized installs.
- Symptom: Startup logs showed `loadFileDirectory('Data\\english\\Language')` with zero files read on Brazilian Portuguese data sets.
- Root cause: `Generals/Code/GameEngine/Source/Common/System/registry.cpp` kept `_UNIX` stubs that always returned `FALSE`, so `GetRegistryLanguage()` stayed on cached default `"english"`. Zero Hour already had Linux fallback logic (`tryAutoDetectLanguage`) but base Generals did not.
- Fix: Added Linux env-var registry mapping (`CNC_GENERALS_<KEY>`) and BIG-based language auto-detection in base Generals registry code, with candidates for both base (`English.big`, `Brazilian.big`, etc.) and ZH-style (`EnglishZH.big`, `BrazilianZH.big`, etc.) localized packs.
- Prevention: For shared platform behavior (registry emulation, language fallback), always audit both `Generals/` and `GeneralsMD/` implementations before closing Linux parity tasks.

## Session 2026-03-14 - Input factories must preserve non-null initialization contracts

- Problem: `W3DGameClient::createMouse()` on non-Windows could return `nullptr` if `SDL3GameEngine` or its window handle was unavailable.
- Symptom: `GameClient::init()` immediately calls `TheMouse->parseIni()` after `createMouse()`, so a null return turns a setup invariant violation into a later null dereference.
- Root cause: The SDL3 migration encoded a fatal precondition failure as a nullable factory result even though callers treat mouse creation as mandatory.
- Fix: On non-Windows, keep creating `SDL3Mouse` when the SDL window exists, but abort immediately with a fatal diagnostic if the engine/window invariant is broken; do not return `nullptr` from the factory.
- Prevention: Factories used during core subsystem bootstrap must either always return a valid object or terminate at the point of invariant failure. Never defer that failure to downstream member calls.

## Session 2026-03-14 - fenv alone is not enough for Linux replay determinism on x86

- Problem: `setFPMode()` on non-Windows had been reduced to `fesetenv(FE_DFL_ENV)` plus `fesetround(FE_TONEAREST)`, which does not reproduce the Windows baseline's `_PC_24` x87 precision control.
- Symptom: Linux/x86 could run gameplay math with wider x87 precision than the original Windows simulation path, creating replay/desync risk even though the rounding mode looked correct.
- Root cause: `fenv` restores default environment and rounding, but it does not by itself force the x87 control word to 24-bit precision or explicitly realign SSE `MXCSR` rounding.
- Fix: In both Generals and Zero Hour `GameLogic.cpp`, keep the portable `fenv` reset, then on x86/x86_64 explicitly program the x87 control word to single-precision/round-to-nearest and align `MXCSR` rounding with `_MM_ROUND_NEAREST`.
- Prevention: When porting deterministic legacy game code, treat Windows `_controlfp(... _PC_24 | _RC_NEAR)` as behavioral contract, not as an implementation detail that can be replaced with generic `fenv` calls.

## Session 2026-03-09 - macOS deploy path regression

- Problem: `scripts/build/macos/deploy-macos-zh.sh` resolved `PROJECT_ROOT` with `$(dirname "$0")/..`, which points to `.../scripts/build` when called from `scripts/build/macos`.
- Symptom: Deploy looked for binary at `scripts/build/build/macos-vulkan/GeneralsMD/GeneralsXZH` and failed.
- Root cause: Script location changed to `scripts/build/macos`, but root-resolution logic still assumed a different layout.
- Fix: Use `SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"` and `PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"`.
- Scope: Applied to `scripts/build/macos/deploy-macos-zh.sh`, `scripts/build/macos/run-macos-zh.sh`, and `scripts/build/macos/bundle-macos-zh.sh`.
- Prevention: In scripts under nested folders, derive repo root from `BASH_SOURCE[0]` and keep helper command examples aligned with the actual script paths.

## Session 2026-03-09 - Linux deploy/bundle mirrored the same root-path bug

- Problem: `scripts/build/linux/deploy-linux-zh.sh` and `scripts/build/linux/bundle-linux-zh.sh` used the same `$(dirname "$0")/..` root resolution pattern.
- Symptom: They would resolve `PROJECT_ROOT` as `.../scripts/build`, causing wrong binary/library paths (`.../scripts/build/build/linux64-deploy/...`).
- Fix: Standardized both scripts to `SCRIPT_DIR + ../../..` based on `${BASH_SOURCE[0]}`.
- Scope: Applied to `scripts/build/linux/deploy-linux-zh.sh` and `scripts/build/linux/bundle-linux-zh.sh`; updated Linux helper command paths to `./scripts/build/linux/...` for consistency.
- Prevention: When scripts are moved into deeper subfolders, audit all sibling platform scripts for the same root-resolution assumption before closing the change.

## Session 2026-03-14 - Base Generals Linux scripts drifted from Zero Hour fixes

- Problem: `scripts/build/linux/deploy-linux.sh` still used the pre-reorg root calculation and also pointed to the Zero Hour runtime/binary paths.
- Symptom: Deploy searched under `scripts/build/build/linux64-deploy/GeneralsMD/GeneralsX` and copied base-game output into `~/GeneralsX/GeneralsMD` instead of `~/GeneralsX/Generals`.
- Root cause: The Zero Hour Linux scripts were fixed after the folder move, but the sibling base-game scripts were not audited in the same pass.
- Fix: Standardized the base deploy/bundle scripts to `${BASH_SOURCE[0]}` + `../../..`, corrected base binary/runtime locations, and updated stale helper references to `scripts/build/linux/...` and `scripts/env/docker/...`.
- Prevention: When a script family is duplicated per game target, always apply and verify structural fixes across both variants before closing the change.

## Session 2026-03-14 - Follow-up audit caught stale usage strings and one broken variable block

- Problem: A follow-up grep found leftover usage strings pointing to pre-reorg script paths, and one patch had accidentally overwritten `DOCKER_IMAGE` in `docker-build-linux-generals.sh`.
- Symptom: The build script had invalid shell structure near the variable declarations, and smoke/help output still suggested nonexistent commands under `scripts/` root.
- Fix: Restored the missing `DOCKER_IMAGE` assignment, updated smoke/build image usage text to the current `scripts/build/linux/...` and `scripts/env/docker/...` paths, and corrected the base bundle extraction hint to reference `Generals/` data.
- Prevention: After path migrations, always run grep for old command strings and re-open the edited file header to catch malformed variable blocks early.

## Session 2026-03-09 - DXVK patch drift replaced by fork-pinned source

- Problem: macOS crashed with `EXC_BREAKPOINT` in `dxvk::env::getExeName()` because DXVK source lacked a valid `__APPLE__` branch in `getExePath()`.
- Root cause: The dynamic patch pipeline reported success for Patch 6 but did not actually modify `util_env.cpp` due brittle text replacement assumptions.
- Fix: Applied the full macOS patchset directly in `fbraz3/dxvk` branch `generalsx-macos-v2.6`, manually added `_NSGetExecutablePath` branch in `src/util/util_env.cpp`, and pinned CMake to commit `ffcdbcaf1b5a321406ffed43c4e815fd7c7e1797`.
- Build integration: `cmake/dx8.cmake` now fetches `https://github.com/fbraz3/dxvk.git`, uses `DXVK_SOURCE_DIR=_deps/dxvk-src-fbraz3`, and disables `PATCH_COMMAND` for macOS ExternalProject.
- Validation: Game reached runtime loop without reproducing the immediate `Direct3DCreate8` trap.

## Session 2026-03-10 - Build success is not startup safety in CI

- Problem: Build-only CI can pass while runtime startup still crashes (segfault/abort) in SDL3/DXVK/OpenAL initialization paths.
- Insight: Headless replay is excellent for determinism and logic regressions, but it can bypass graphics/audio/UI startup paths and miss bootstrap crashes.
- Decision: Use layered CI gates: build integrity, runtime bootstrap smoke (required), replay determinism (required, scoped), and sanitizers (nightly).
- Reference: `docs/WORKDIR/planning/PLAN-021_CI_RUNTIME_CONFIDENCE.md`.

## Session 2026-03-12 - Backslash path literals break save/replay on Linux

- Problem: Save and replay directory helpers used Windows suffixes like `"Save\\"` and `"Replays\\"` unconditionally.
- Symptom: Linux created literal entries such as `Save\\` and `Save\\00000002.sav` under `~/.local/share/generals_zh/`, so save/load and replay discovery behaved incorrectly.
- Root cause: Backslash is a normal filename character on POSIX, not a path separator.
- Fix: Added platform-specific separators in `GameState::getSaveDirectory()` and `RecorderClass::getReplayDir()/getReplayArchiveDir()` for both ZH and Generals (`\\` on Windows, `/` on non-Windows).
- Prevention: Never hardcode Windows path separators in cross-platform code; centralize path construction by platform or use filesystem paths when possible.

## Session 2026-03-19 - GitHub Actions: Replace loose dylib bundling with .app bundle scripts

- Problem: macOS CI workflow manually copied dylibs into a tar.gz with a generic `run.sh` wrapper, duplicating logic from local bundle scripts and missing external Homebrew dependencies.
- Symptom: CI package could be missing transitive dylibs (e.g., libsharpyuv from libwebp), causing runtime failures even though the package "built successfully".
- Root cause: Manual dylib copying in CI did not recursively collect external dependencies or run the bundle script's `@rpath` resolver.
- Solution: 
  - Updated `.github/workflows/build-macos.yml` to install explicit FFmpeg + all discovered dependency packages (libbluray, gnutls, librist, srt, libssh, zeromq, libvpx, webp, jpeg-xl, dav1d, opencore-amr, snappy, aom, libvmaf, lame, xz, aribb24, libpng).
  - Replaced "Deploy Bundle (loose dylibs)" step with execution of `scripts/build/macos/bundle-macos-zh.sh` or `.../bundle-macos-generals.sh`.
  - Bundle scripts now generate distributable `.app` bundles (`.zip`) with full `@rpath` resolution, icons, and proper environment setup.
  - Simplified CI packaging logic from 129 lines to 36 lines by reusing and trusting existing bundle scripts.
- Validation: CI workflow now uses the same bundling code as local development, reducing "works locally but fails in CI" regressions.
- Prevention: For future CI artifact changes, always execute existing local scripts (build, bundle, test) in CI instead of reimplementing them; trust scripts over ad-hoc shell glue.
