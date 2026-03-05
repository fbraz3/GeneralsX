# GeneralsX CI/CD Pipeline

## Overview

The GeneralsX CI pipeline provides automated cross-platform builds for all three supported operating systems:

1. **Linux** (x86_64) — `linux64-deploy` preset
2. **macOS** (arm64) — `macos-vulkan` preset  
3. **Windows** (x64 modern) — `win64-modern` preset

## Triggers

**Automatic**: Runs on:
- Push to `main` or any `*-sdl` branch
- Pull requests to `main` or any `*-sdl` branch

**Manual**: Via GitHub Actions → "Run workflow"

## File Change Detection

The `detect-changes` job determines whether builds should run by analyzing which files were modified.

**Triggers build if changes detected in**:
- `GeneralsMD/**` — Zero Hour game code
- `Core/**` — Shared engine/libraries
- `Dependencies/**` — External dependencies
- `cmake/**` — CMake configuration
- `CMakeLists.txt`, `CMakePresets.json`
- Workflow files (`.github/workflows/`)

**Skips build if only these change**:
- Documentation, READMEs, comments
- `Generals/**` (base game, not priority yet)
- Unrelated files

## Build Jobs

All three build jobs run **in parallel** (no dependencies between them):

### 1. Linux Build
- Runs on: `ubuntu-latest`
- Game: `GeneralsMD`
- Preset: `linux64-deploy`
- Docker-based build with SteamRT Sniper container
- Timeout: 120 minutes

### 2. macOS Build
- Runs on: `macos-latest`
- Game: `GeneralsMD`
- Preset: `macos-vulkan`
- Native Apple Silicon (arm64) build with MoltenVK
- Timeout: 120 minutes

### 3. Windows Build
- Runs on: `windows-latest`
- Game: `GeneralsMD`
- Preset: `win64-modern`
- Modern Windows build with MSVC + DXVK + OpenAL
- Timeout: 120 minutes

## CI Summary

After all builds complete, the `ci-summary` job generates a status table showing:
- ✅ Success
- ❌ Failed
- ⏭️ Skipped (no file changes detected)

**Fail-fast**: If ANY platform build fails, the entire CI workflow is marked as failed.

## Future Enhancements

- [ ] Replay compatibility tests (when game is feature-complete)
- [ ] Performance benchmarking
- [ ] Binary artifact uploads (bundles, .app, .exe, .elf)
- [ ] Smoke tests (headless gameplay on select maps)
