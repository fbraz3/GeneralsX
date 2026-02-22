# PHASE03: Video Playback - Bink Alternative (SPIKE)

**Status**: 🚀 In Progress (Framework Compiled, Testing Phase)
**Type**: Implementation (not a spike - fighter19 provides complete reference)
**Created**: 2026-02-08
**Updated**: 2026-02-22 (Compilation successful, game launches)
**Prerequisites**: Phase 1 + Phase 2 complete

## Goal

Integrate FFmpeg-based video playback for Linux from fighter19 reference. Replace Bink video calls with FFmpeg decoder for Bink→VP9/H.264 videos.

## Progress Snapshot
✅ **Framework Copied**: VideoDevice headers + sources from fighter19 reference
✅ **CMake Updated**: FFmpeg configuration added (libswscale for frame conversion)
✅ **Docker Updated**: libswscale-dev added to build dependencies  
🔨 **Next**: Compile, test video playback, handle game integration

---

## Context

**Original Game Video System**:
- Uses **Bink Video** codec (proprietary, RAD Game Tools)
- Plays intro cinematics on game launch
- Campaign briefing/outro videos
- Victory/defeat cinematics

**Problem**:
- Bink SDK is proprietary and Windows-only
- No native Linux support
- Codec is heavily integrated into game engine

**Phase 3 Options**:
1. **Replace with FFmpeg** (decode Bink or re-encode to VP9/H.264)
2. **Stub out videos** (skip to main menu)
3. **Wine/Proton compatibility layer** (use Windows Bink libs)
4. **Defer indefinitely** (lowest priority feature)

---

## Scope

### In Scope ✅
- Research Bink codec integration points in game engine
- Investigate FFmpeg-based video player integration
- Prototype minimal video playback (one intro video)
- Document findings and recommend approach
- Implement chosen solution OR document deferral rationale

### Out of Scope ❌
- Full campaign video re-encoding pipeline (user responsibility)
- Audio/video sync perfection (acceptable if minor desync)
- Subtitles/localization (future)
- Mod video compatibility (future)

---

## Implementation Status (Session 56 - 2026-02-22)

### ✅ Completed

**From fighter19 Reference**:
- [x] ✅ **VideoDevice Architecture**: Copied from fighter19 (`Include/VideoDevice/` and `Source/VideoDevice/`)
  - `VideoDevice/Bink/BinkVideoPlayer.{h,cpp}` — Windows video player (Bink API)
  - `VideoDevice/FFmpeg/FFmpegVideoPlayer.{h,cpp}` — Linux/cross-platform FFmpeg player
  - `VideoDevice/FFmpeg/FFmpegFile.{h,cpp}` — FFmpeg file I/O and frame decoding

- [x] ✅ **CMake Configuration**: Updated `GeneralsMD/Code/GameEngineDevice/CMakeLists.txt`
  - Added `libswscale` to FFmpeg dependencies (YUV→RGB frame conversion)
  - Configured `SAGE_USE_FFMPEG` compilation flag
  - Linked VideoDevice sources conditionally when `SAGE_USE_FFMPEG` is ON

- [x] ✅ **Docker Build Dependencies**: Updated `resources/dockerbuild/Dockerfile.linux`
  - Added `libswscale-dev` (frame format conversion library)
  - Already had `libavcodec-dev`, `libavformat-dev`, `libavutil-dev`

- [x] ✅ **Compilation Success** (Session 56):
  - Docker image rebuilt with libswscale-dev ✅
  - CMake reconfigured: finds all 4 FFmpeg libraries ✅
  - FFmpegFile.cpp include path fixed (file.h not File.h) ✅
  - VideoDevice FFmpeg sources compile without errors ✅
  - Final binary: 179M GeneralsXZH ✅

- [x] ✅ **Game Launch** (Session 56):
  - Game initializes without crashes ✅
  - SDL3 video subsystem loads ✅
  - Audio system initialized ✅
  - File system accessible ✅

### 🔨 In Progress

- [ ] **Video Playback Test**: Trigger video in-game and verify FFmpegVideoPlayer handles it
- [ ] **Audio/Video Sync**: Check if audio and video timestamps align
- [ ] **Format Support**: Verify what video formats are supported (Bink, WebM, MP4, etc.)

### ❌ Blockers

None remaining — framework is complete and compiling successfully.

---

## Technical Details (From fighter19 Analysis)

---

## Next Steps (Session 57+)

### Phase 3: Video Playback Testing

**We've crossed a milestone**: ✅ Framework compiled, game launches. Now test actual video playback.

**Immediate Tasks**:
- [ ] **1. Locate Video Call Sites**: Where does the game trigger videos?
  ```bash
  grep -r "BinkOpen\|PlayVideo\|theVideoPlayer\|VideoPlayer" GeneralsMD/Code/GameClient/ | head -20
  ```
  Expected: Find intro video, campaign briefing, victory cinematics

- [ ] **2. Identify Video File Locations**: What video files exist?
  ```bash
  find GeneralsMD/Data -name "*.bik" -o -name "*.webm" -o -name "*.mp4" 2>/dev/null
  ```
  Expected: Likely `GeneralsMD/Data/Movies/*.bik` or similar

- [ ] **3. Test Video Loading**: Manually call FFmpegVideoPlayer with a video file
  - Create simple test: Load `Data/Movies/intro.bik` (if exists)
  - Verify no crashes, file opens successfully
  - Check FFmpeg log for decode errors

- [ ] **4. Game Integration**: Wire FFmpeg player into game engine
  - Find where BinkOpen/BinkDoFrame are called
  - Replace with FFmpegVideoPlayer equivalent
  - Handle both Windows (Bink) and Linux (FFmpeg) paths

- [ ] **5. Format Conversion Planning**: If game only has .bik files
  - Document how to convert Bink to WebM/H.264
  - Plan for user distribution (if needed)
  - Check if FFmpeg can decode Bink natively

### Phase 3 Complete Outcome (After Testing)

**Success Path**:
✅ If videos play:
- [ ] At least one intro/campaign video plays without crashes
- [ ] Audio (if any) syncs with video
- [ ] Videos can be skipped (ESC key or timeout)
- [ ] Windows Bink path unchanged

**Fallback Path**:
✅ If videos can't be decoded:
- [ ] Implement VideoPlayerStub (graceful skip)
- [ ] Log: "Video playback not supported on Linux"
- [ ] Game continues to menu without hanging
- [ ] Clear documentation of limitation

**Success Path**:
✅ If compilation succeeds:
- [ ] VideoDevice FFmpeg compiles without errors
- [ ] Game launches without video-related crashes
- [ ] Proceed to video playback integration

**Failure Handling**:
❌ If compilation fails:
- [ ] Identify missing includes/symbols from fighter19
- [ ] Adjust FFmpegVideoPlayer.cpp for GeneralsX compatibility
- [ ] Check FFmpeg library availability in Docker
- [ ] May need to create compatibility wrappers

---

## Fighter19 Framework Details

### Architecture Overview

**VideoDevice Abstraction** (from fighter19):
- `Include/VideoDevice/BinkVideoPlayer.h`: Windows video player (Bink API) — **STUB on Linux**
- `Include/VideoDevice/FFmpeg/FFmpegFile.h`: FFmpeg file I/O abstraction
- `Include/VideoDevice/FFmpeg/FFmpegVideoPlayer.h`: Linux/cross-platform FFmpeg player (149 lines)
- `Source/VideoDevice/FFmpeg/FFmpegFile.cpp`: FFmpeg implementation
- `Source/VideoDevice/FFmpeg/FFmpegVideoPlayer.cpp`: Video decoding + rendering (565 lines)

### Key Classes

**FFmpegVideoStream** (manages decoding):
```cpp
class FFmpegVideoStream : public VideoStream {
    AVCodecContext* m_CodecContext;       // Decoder context
    AVFrame* m_DecodedFrame;              // Raw YUV frame
    AVFrame* m_RGBFrame;                  // Converted RGB frame
    SwsContext* m_SwsContext;             // YUV→RGB converter
    
    void decodeFrame(AVPacket* packet);   // Decode packet to YUV
    void convertFrameToRGB();             // YUV→RGB via libswscale
    uint32_t* getPixels();                // Return RGB pixel data
};
```

**FFmpegVideoPlayer** (manages streams):
```cpp
class FFmpegVideoPlayer : public VideoPlayer {
    AVFormatContext* m_FormatContext;     // Container (demuxer)
    FFmpegFile* m_File;                   // File abstraction
    FFmpegVideoStream* m_VideoStream;     // Active video stream
    
    bool openVideo(const char* filename); // Open .bik, .webm, .mp4
    bool isFrameReady();                  // Check if frame decoded
    void frameDecompress();               // Decode next frame
    void frameRender();                   // Render to surface/texture
    void frameNext();                     // Advance to next frame
};
```

### Dependencies (FFmpeg Libraries)

- **libavcodec**: Video codec implementation (decode H.264, VP9, Bink if available)
- **libavformat**: Container demuxing (parse .bik, .webm, .mp4 file headers)
- **libavutil**: Utility functions (memory, logging, math)
- **libswscale**: Frame format conversion (YUV420→RGB32 for rendering)

All 4 libraries now in:
- CMake: `GeneralsMD/Code/GameEngineDevice/CMakeLists.txt`
- Docker: `resources/dockerbuild/Dockerfile.linux`

### Video File Format Support

Fighter19 implementation can decode:
- **`.bik`** (Bink video) — If FFmpeg compiled with Bink support (check at runtime)
- **`.webm`** (VP9 video) — Standard, widely supported
- **`.mp4`** (H.264 video) — Standard, widely supported
- **`.avi`**, **`.mkv`** — Other containers with supported codecs

**Action Required**: Check if game videos are `.bik` or need conversion to `.webm`/`.mp4`

---

## Compilation Verification Checklist

Use this checklist to verify Phase 3 integration is successful:

```bash
# 1. Check Docker build includes libswscale
docker run --rm ubuntu:22.04 dpkg -l | grep libswscale
# Expected: libswscale-dev, libswscale-...

# 2. Verify CMakeLists.txt has VideoDevice sources
grep -n "VideoDevice.*FFmpeg\|FFmpegFile\|FFmpegVideoPlayer" GeneralsMD/Code/GameEngineDevice/CMakeLists.txt
# Expected: 2+ matches (files and compilation flag)

# 3. Check libswscale in CMakeLists
grep -n "libswscale" GeneralsMD/Code/GameEngineDevice/CMakeLists.txt
# Expected: Found in pkg_check_modules

# 4. Verify headers copied
ls -la GeneralsMD/Code/GameEngineDevice/Include/VideoDevice/
# Expected: Bink/ and FFmpeg/ subdirectories

# 5. Verify sources copied
ls -la GeneralsMD/Code/GameEngineDevice/Source/VideoDevice/
# Expected: FFmpeg/FFmpegFile.cpp, FFmpeg/FFmpegVideoPlayer.cpp

# 6. Build with Linux preset
./scripts/docker-build-linux-zh.sh linux64-deploy 2>&1 | tee logs/phase3_build.log
# Expected: No linking errors related to FFmpeg or VideoDevice
```

---

## Acceptance Criteria (Phase 3)

Phase 3 is **COMPLETE** when the following is achieved:

### Phase 3 Complete = Working Video Playback on Linux + Windows Compatibility Maintained

**Minimum Requirements**:
- [ ] ✅ **VideoDevice Framework Integrated** (Session 55 - DONE)
  - FFmpeg headers and sources copied from fighter19
  - CMakeLists.txt updated with VideoDevice + libswscale
  - Docker image updated with libswscale-dev

- [ ] 🔨 **Framework Compiles** (Session 56+ - PENDING)
  - Docker build succeeds with no FFmpeg/VideoDevice errors
  - CMake correctly finds libavcodec, libavformat, libavutil, libswscale
  - FFmpegFile.cpp and FFmpegVideoPlayer.cpp compile without issues

- [ ] 🔨 **Game Engine Wiring Complete** (Session 56+ - PENDING)
  - VideoPlayer instances created without crashes
  - Bink video call sites identified and wired to FFmpeg backend
  - Game reaches main menu on Linux with video system initialized

- [ ] 🔨 **Video Playback Working** (Session 56+ - PENDING)
  - At least one intro video plays without crashes
  - Video audio syncs with game audio system (or gracefully disabled)
  - Videos can be skipped (ESC key or click)

### Windows Compatibility (Must Pass)
- [ ] ✅ **VC6 Preset Still Builds** (maintained from Phase 2)
- [ ] ✅ **Win32 Preset Still Builds** (maintained from Phase 2)
- [ ] ✅ **Windows Builds Use Bink** (no regressions)
  - Videos play on Windows with original Bink player
  - No changes to Windows video path

### Fallback Options (If Any Blocker)
- [ ] **Partial Success**: Game launches, videos skip gracefully with log message
- [ ] **Stub Fallback**: If FFmpeg integration fails, implement VideoPlayerStub that skips videos
- [ ] **Defer**: If technical debt too high, document findings + recommend future work

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| FFmpeg too complex | High | Time-box spike to 1 week, recommend stub if blocked |
| Bink codec licensing | High | Use VP9/H.264 conversion, require users convert their own files |
| Audio/video desync | Medium | Accept minor desync, or disable video audio (use game music) |
| Performance issues | Low | Modern CPUs decode VP9/H.264 easily, profile on low-end hardware |
| Video rendering glitches | Medium | Test on multiple GPUs, may need YUV→RGB shader optimization |

---

## Decision Framework

**Proceed with implementation if**:
- FFmpeg spike succeeds in 1 week
- Video conversion process is documented and feasible
- Community feedback indicates videos are important

**Stub out videos if**:
- FFmpeg spike fails or too complex
- Legal/licensing concerns
- Videos are low-priority (campaign briefings can be text-only)

**Defer indefinitely if**:
- Phase 1/2 issues consume all resources
- Community consensus is "gameplay first, videos later"
- Alternative: Recommend users play campaign on Windows, skirmish on Linux

---

## References

### Technical
- **FFmpeg Documentation**: https://ffmpeg.org/doxygen/trunk/
- **FFmpeg Video Player Tutorial**: https://github.com/leandromoreira/ffmpeg-libav-tutorial
- **Bink Video Format**: RAD Game Tools (proprietary, reverse-engineer if needed)

### Game Assets
- [ ] Document video file locations (`Data/Movies/`?)
- [ ] Document video formats (Bink 1? Bink 2?)
- [ ] Document video triggers (INI files? Hardcoded?)

### Reference Implementations
- fighter19: `references/fighter19-dxvk-port/` (check video handling)
- jmarshall: `references/jmarshall-win64-modern/` (likely still Bink)
- dsalzner: `references/dsalzner-linux-attempt/` (check if addressed)

---

## Timeline (Updated)

**Session 55 (Complete)**: Framework Integration
- ✅ Fighter19 implementation analyzed
- ✅ VideoDevice headers/sources copied
- ✅ CMakeLists.txt updated (libswscale + VideoDevice)
- ✅ Dockerfile.linux updated (libswscale-dev)
- ✅ Phase 3 documentation updated

**Session 56+ (Next)**: Compilation & Integration
- [ ] Docker build test (verify libswscale)
- [ ] CMake verification (find_package FFmpeg)
- [ ] Compilation test (FFmpeg sources)
- [ ] Video file discovery (locate.bik/.webm files)
- [ ] Bink integration points (grep for PlayVideo calls)
- [ ] Game engine wiring (VideoPlayer factory)
- [ ] Integration test (game launch without crashes)

**Session 57+ (If Succeeds)**: Playback Testing
- [ ] Video playback functional test
- [ ] Audio sync verification
- [ ] Video skipping (ESC key)
- [ ] Performance benchmarking
- [ ] Platform test (Vulkan rendering + FFmpeg)

**Total Estimate**: 3-4 sessions (1 for framework ✅, 2-3 for integration+testing)

---

## Deliverables

### Session 55 (COMPLETED):
- ✅ VideoDevice framework integration (headers + sources)
- ✅ CMakeLists.txt with libswscale + VideoDevice sources
- ✅ Dockerfile.linux with libswscale-dev
- ✅ Phase 3 documentation updated

### Session 56+ (In Progress):
- [ ] Docker build passing with FFmpeg libraries
- [ ] CMake configuration accepts all 4 FFmpeg libraries
- [ ] FFmpegVideoPlayer.cpp and FFmpegFile.cpp compile without errors

### Session 57+ (If Succeeds):
- [ ] FFmpegVideoPlayer fully integrated in GameEngineDevice
- [ ] Video files playable from Linux binary
- [ ] All acceptance criteria met

### Fallback (If Blocker):
- [ ] VideoPlayerStub implementation (graceful skip)
- [ ] Research document explaining issues
- [ ] Recommendation for future work

---

## Status Tracking

**Framework Integration (Session 55)**:
- [x] Fighter19 analysis completed
- [x] VideoDevice copied to codebase
- [x] CMakeLists.txt updated
- [x] Dockerfile.linux updated
- [x] Phase 3 documentation updated

**Compilation & Integration (Session 56+)**:
- [ ] Docker build test
- [ ] CMake FFmpeg detection
- [ ] VideoDevice compilation
- [ ] Bink integration points identified
- [ ] Game engine wiring complete

**Playback Testing (Session 57+)**:
- [ ] Video playback functional
- [ ] Audio sync working (or disabled gracefully)
- [ ] Performance acceptable
- [ ] All platforms tested

**Overall Phase 3 Progress**: 25% (framework integration 1/4 steps complete)
