# macOS Action Bundle Analysis (2026-03-19)

## Objective
Update `.github/workflows/build-macos.yml` to generate .app bundles instead of loose dylibs in tar.gz.

## Current State (Before)
- Action compiles the game binary
- Creates a "bundle" with loose dylibs + run.sh wrapper in tar.gz
- Does NOT use the bundle scripts (`scripts/build/macos/bundle-macos-*.sh`)

## Target State (After)
- Action compiles the game binary
- Executes `scripts/build/macos/bundle-macos-zh.sh` or `.../bundle-macos-generals.sh`
- Generates distributable .app bundle (.zip) with:
  - Self-contained app package (GeneralsXZH.app or GeneralsX.app)
  - All dylibs bundled in Contents/Resources/lib/
  - App icon in Contents/Resources/
  - run.sh launcher as CFBundleExecutable
  - MoltenVK ICD JSON for Vulkan routing
  - dxvk.conf for DXVK runtime config

## Dependencies Analysis

### Discovered Homebrew Packages (from local bundle execution)
Running `./scripts/build/macos/bundle-macos-zh.sh` reveals external dylibs sourced from Homebrew:

**Core FFmpeg Chain** (installs via `brew install ffmpeg`):
- libavformat, libavcodec, libswscale, libavutil, libswresample (FFmpeg core)
- libswresample (FFmpeg audio resampling)

**FFmpeg Optional Codecs** (dependencies of ffmpeg formula):
- libbluray (Blu-ray codec)
- libvpx (VP8/VP9 codec)
- libdav1d (AV1 codec)
- libaom (AOM VP9/AV1 encoder)
- opencore-amr (AMR audio codec)
- lame (MP3 codec)
- libmp3lame (LAME MP3 encoder)
- opus (Opus audio codec)
- speex (Speex audio codec)

**FFmpeg Encryption/Protocol Support**:
- gnutls (TLS library for HTTPS in FFmpeg)
- libssh (SSH protocol for FFmpeg)
- libssh2 (SSH2 protocol for FFmpeg)
- librist (RIST protocol for FFmpeg)
- srt (SRT protocol for FFmpeg)
- zeromq (ZeroMQ protocol for FFmpeg)

**FFmpeg Image/Video Processing**:
- jpeg-xl (JPEG-XL codec)
- jxl_threads (JPEG-XL threading)
- webp (WebP codec)
- libwebpmux (WebP muxer)
- libpng (PNG image support)
- libjxl (JPEG-XL library)

**FFmpeg Quality Metrics**:
- libvmaf (video quality metrics)
- snappy (compression for FFmpeg)

**FFmpeg System Support**:
- xz (LZMA compression)
- aribb24 (ARIB subtitle support)

### Installation Strategy

**Option A: Explicit FFmpeg + Explicit Dependencies** (Recommended)
```bash
# Core FFmpeg with most codecs
brew install ffmpeg

# Ensure explicit accessibility of all discovered libs
brew install \
  libbluray gnutls librist srt libssh zeromq \
  libvpx webp jpeg-xl dav1d opencore-amr snappy aom libvmaf lame \
  xz aribb24 libpng
```

**Best Practice**: FFmpeg formula should bring most deps as transitive, but explicitly listing ensures no optional deps are missed due to brew formula changes.

## Bundle Script Assumptions

The bundle scripts (`bundle-macos-zh.sh`, `bundle-macos-generals.sh`) assume:

1. ✅ Binary compiled: `build/macos-vulkan/GeneralsMD/GeneralsXZH` (or `Generals/GeneralsX`)
2. ✅ SDL3, SDL3_image, DXVK dylibs in build/_deps/ (built by CMake)
3. ✅ libgamespy.dylib in build root (FetchContent output)
4. ✅ Vulkan SDK installed with libvulkan + libMoltenVK
5. ✅ External dylibs discoverable via /opt/homebrew/ (bundle scans recursively)
6. ✅ Icons present: `assets/generalsx-zh_icon.png` and `assets/generalsx_icon.png`
7. ✅ dxvk.conf present: `resources/dxvk/dxvk.conf`

## Action Updates Required

### 1. Install FFmpeg Dependencies
Add explicit Homebrew package list after existing ffmpeg install.

### 2. Execute Bundle Script
Replace current loose-dylib packaging with:
```bash
./scripts/build/macos/bundle-macos-zh.sh
# or
./scripts/build/macos/bundle-macos-generals.sh
```

### 3. Upload .zip Instead of tar.gz
Change artifact from tar.gz to .zip:
- ZH: `GeneralsXZH-macos-arm64.zip`
- Generals: `GeneralsX-macos-arm64.zip`

## Validation Checklist

- [ ] Bundle script runs without errors in CI environment
- [ ] All FFmpeg dependencies installed (no missing dylib errors)
- [ ] .app bundle generated with correct structure
- [ ] Icon appears in Dock when app launches
- [ ] run.sh sets DYLD_LIBRARY_PATH correctly
- [ ] MoltenVK ICD JSON found and routed
- [ ] Game launches and reaches main menu
- [ ] Artifact upload succeeds (zip file)

## Rollback Plan
If bundle script fails in CI:
1. Keep current loose-dylib fallback available
2. Add `set +e` trap to capture error logs
3. Document failure in issue template for debugging

## Next Steps
1. Update `.github/workflows/build-macos.yml`:
   - Add explicit FFmpeg dependency packages
   - Replace "Deploy Bundle" step with bundle script execution
   - Update artifact upload to .zip
2. Test on workflow_dispatch first
3. Validate .app structure in artifacts
4. Merge to main
