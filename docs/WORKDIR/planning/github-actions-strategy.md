# GitHub Actions - macOS Build + Replay Test Strategy

**Date**: 2026-02-27  
**Status**: Phase 1 (macOS build) implemented; Phase 2+ planned

## Current Implementation

### Phase 1: macOS Cross-Platform Build (Implemented ✅)

**New Files**:
- `.github/workflows/build-macos.yml` - Reusable macOS build workflow
- Updated `.github/workflows/ci.yml` - Added `build-generalsmd-macos` and `build-generals-macos` jobs

**Features**:
- Triggered via `workflow_dispatch` (manual trigger in GitHub UI)
- Installs macOS dependencies (cmake, ninja, meson)
- Auto-downloads Vulkan SDK (required for DXVK + MoltenVK)
- Configures with `macos-vulkan` preset
- Compiles GeneralsXZH and Generals
- Verifies binary artifacts
- Uploads build logs as artifacts

**How to Trigger**:
1. Go to **Actions** tab in GitHub repo
2. Select **GenCI** workflow
3. Click **Run workflow** → Select **main** branch → **Run workflow**
4. Wait ~2 hours for macOS build to complete

---

## Future Phases

### Phase 2: Multi-Platform PR Validation (Not Yet Implemented)

**Goal**: Every PR must build on all three platforms + pass deterministic replay tests.

**Requirements**:
1. **Enable on PR** (currently disabled):
   ```yaml
   on:
     push:
       branches: [main]
     pull_request:
       branches: [main]
     workflow_dispatch:
   ```

2. **Build all three platforms**:
   - Windows (VC6 + replays) ✅ Already in CI
   - Windows (MSVC2022/win32) ✅ Already in CI
   - Linux (via Docker, `linux64-deploy`) ❌ Not yet added
   - macOS (ARM64, `macos-vulkan`) ✅ Just added

3. **Deterministic replay test after successful build**:
   - Requires VC6 optimized builds (`RTS_BUILD_OPTION_DEBUG=OFF`)
   - Run same replays across all platforms
   - Verify bytewise determinism:
     ```bash
     # Expected: VC6 optimized build replay hash = Linux/macOS replay hash
     generalszh.exe -jobs 4 -headless -replay GeneralsReplays/GeneralsZH/1.04/*.rep
     ```

### Phase 3: Linux Docker CI (Not Yet Implemented)

**Goal**: Add Linux build to multi-platform CI.

**Plan**:
- Create `.github/workflows/build-linux.yml` (similar to `build-macos.yml`)
- Uses Docker containers (avoids Linux-specific toolchain dependencies in GH runners)
- Leverages existing `scripts/docker-build-linux-zh.sh`
- Calls `docker run` within GH Actions

### Phase 4: Replay Determinism Validation (Not Yet Implemented)

**Goal**: Automatically validate cross-platform replay determinism.

**Plan**:
1. Build Windows VC6 optimized (`vc6+t+e`)
2. Generate replay hash fingerprint
3. Build Linux and macOS
4. Run same replays, compare hashes
5. Fail if fingerprints don't match (indicates rendering or logic divergence)

**Implementation**:
- Create `.github/workflows/validate-replay-determinism.yml`
- Requires: Windows + Linux + macOS builds completed
- Uses `check-replays.yml` pattern

---

## Immediate Next Steps

### 1. Test Current macOS Build (this week)

```bash
# Trigger locally:
./scripts/build-macos-zh.sh

# Or via GitHub UI:
Actions → GenCI → Run workflow → Check logs
```

### 2. Add Linux Docker Build (Phase 3, priority)

Create `.github/workflows/build-linux.yml`:
```yaml
name: Build Linux (Docker)
on:
  workflow_call:
    inputs:
      game: { type: string }
      preset: { type: string, default: "linux64-deploy" }
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Run Docker Build
        run: ./scripts/docker-build-linux-${{ inputs.game }}.sh ${{ inputs.preset }}
```

### 3. Enable PR Validation (Phase 2, after Linux ready)

Uncomment in `ci.yml`:
```yaml
on:
  push:
    branches: [main]
  pull_request:
    branches: [main]
  workflow_dispatch:
```

### 4. Add Replay Determinism Check (Phase 4, future)

Create matrix job combining all platform builds → Compare replay hashes.

---

## Architecture Notes

**Why split workflows?**
- `build-toolchain.yml` - Windows (VC6 + MSVC2022)
- `build-macos.yml` - macOS (ARM64 native)
- `build-linux.yml` - Linux (Docker container) [planned]
- `check-replays.yml` - Replay validation (already exists)

Each workflow is reusable and can be called independently or from `ci.yml`.

**Determinism Requirements**:
- VC6 builds: Must use `RTS_BUILD_OPTION_DEBUG=OFF` (retail optimized)
- Cross-platform: Rendering/audio changes **must not break game logic**
- Replay hash: Must be identical across platforms for same replay input

---

## Vulkan SDK Handling

**Current approach** (in `build-macos.yml`):
```bash
curl https://sdk.lunarg.com/download/latest/mac/vulkan-sdk.dmg
# Takes ~10 minutes first run, cached thereafter
```

**Future optimization**:
- Cache Vulkan SDK in GH Actions (use `actions/cache`)
- Create dedicated GH Actions to handle SDK install

---

## Monitoring & Troubleshooting

**Track progress in**:
- `docs/WORKDIR/reports/` - Weekly CI status
- `docs/WORKDIR/lessons/` - Known issues & solutions

**Common failures**:
- Vulkan SDK download timeout → Retry or cache locally
- macOS ARM64 Meson host detection → Use `meson-arm64-native.ini` (already in place)
- DXVK ephemeral patches → Pre-guard pattern in `windows_compat.h` (already solved)
