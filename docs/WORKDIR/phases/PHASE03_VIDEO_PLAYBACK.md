# PHASE03: Video Playback - Bink Alternative (SPIKE)

**Status**: 📋 Not Started (Blocked by Phase 2)
**Type**: Research Spike (may result in "defer indefinitely")
**Created**: 2026-02-08
**Prerequisites**: Phase 1 + Phase 2 complete

## Goal

Investigate and prototype Bink video replacement for intro/campaign cinematics. This is a **SPIKE** - outcome may be implementation, workaround, or deferral.

## Progress Snapshot
⏳ Blocked by Phase 1 + 2  
🎯 This is a research spike, not guaranteed implementation  
📚 Will investigate multiple approaches and recommend path forward

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

## Research Questions

### A. Game Engine Integration Analysis

**Tasks**:
- [ ] Locate Bink API calls in codebase:
  ```bash
  grep -r "BinkOpen\|BinkDoFrame\|BinkCopyToBuffer" GeneralsMD/
  ```
- [ ] Document video playback entry points:
  - Where are videos triggered? (main menu, campaign, victory)
  - What are the file paths? (`Data/Movies/*.bik`?)
  - How is video integrated with audio system?
- [ ] Identify video rendering surface:
  - Does Bink render to texture or screen buffer?
  - How does it integrate with DX8/DXVK?
- [ ] Document video control flow:
  - Can videos be skipped?
  - How does game handle missing videos?

**Expected Findings**:
- Bink likely isolated in platform layer (`Win32Device/` or `GameClient/`)
- May have skip functionality already (ESC key?)
- May gracefully handle missing `.bik` files

### B. Reference Implementation Research

**fighter19 Approach**:
- [ ] Check `references/fighter19-dxvk-port/` for video handling
- [ ] Document if videos are stubbed, replaced, or working
- [ ] Note any FFmpeg integration or workarounds

**jmarshall Approach**:
- [ ] Check `references/jmarshall-win64-modern/` for video handling
- [ ] **Likely still uses Bink** (Windows build, proprietary libs available)
- [ ] Document any modernization patterns (Bink2 upgrade?)

**Other Linux Ports**:
- [ ] dsalzner Linux attempt: `references/dsalzner-linux-attempt/`
- [ ] Check if videos were addressed or skipped

### C. Alternative Video Codecs

**Option 1: FFmpeg Decoding**

**Pros**:
- Open source, cross-platform
- Supports many codecs (H.264, VP9, Theora)
- Can decode Bink with `libbink` (if available)
- Mature, well-documented

**Cons**:
- Large dependency (~50MB)
- Codec licensing (H.264 patents - use VP9/AV1 instead)
- Integration effort (create video player class)

**Implementation Approach**:
- [ ] Add FFmpeg to CMake dependencies:
  ```cmake
  find_package(FFmpeg COMPONENTS avcodec avformat swscale REQUIRED)
  ```
- [ ] Create `FFmpegVideoPlayer` class:
  ```cpp
  class FFmpegVideoPlayer {
      AVFormatContext* m_FormatCtx;
      AVCodecContext* m_CodecCtx;
      SwsContext* m_SwsCtx;  // For YUV→RGB conversion
      
      bool openVideo(const char* filename);
      bool decodeFrame(AVFrame* frame);
      void renderFrame(AVFrame* frame);  // Copy to SDL2 texture or DX8 surface
  };
  ```
- [ ] Wire into game engine video trigger points

**Option 2: Convert Bink → VP9/H.264**

**Approach**:
- Use Bink Tools (Windows) or FFmpeg to re-encode videos
- Ship game with `.webm` (VP9) or `.mp4` (H.264) videos
- Play with FFmpeg as above

**Pros**:
- One-time conversion, then pure open-source pipeline
- Smaller file sizes (modern codecs more efficient)

**Cons**:
- Users must convert their own videos (legal/licensing issues)
- Quality loss from re-encoding
- Effort to document conversion process

**Option 3: Stub Out Videos**

**Approach**:
- Replace Bink calls with no-ops
- Skip intro, show main menu immediately
- Log "Video playback not supported on Linux"

**Pros**:
- Zero implementation effort
- No dependencies
- Videos are non-essential for gameplay

**Cons**:
- Poor user experience (missing cinematics)
- Campaign briefings important for story
- Community may reject "incomplete" port

**Implementation**:
- [ ] Create `VideoPlayerStub` class:
  ```cpp
  class VideoPlayerStub {
      bool playVideo(const char* filename) {
          fprintf(stderr, "[VideoPlayerStub] Skipping video: %s (Linux)\n", filename);
          return false;  // Signal video "finished"
      }
  };
  ```
- [ ] Wire `#ifdef SAGE_USE_FFMPEG` or stub

---

## Implementation Plan (If Proceeding)

### Phase 3A: Prototype FFmpeg Player (Spike)

**Goal**: Prove FFmpeg can decode and render one intro video

**Tasks**:
- [ ] Add FFmpeg to `cmake/ffmpeg.cmake`:
  ```cmake
  if(SAGE_USE_FFMPEG)
      find_package(PkgConfig REQUIRED)
      pkg_check_modules(FFMPEG REQUIRED
          libavcodec
          libavformat
          libavutil
          libswscale
      )
      target_link_libraries(GameEngineDevice PRIVATE ${FFMPEG_LIBRARIES})
      target_include_directories(GameEngineDevice PRIVATE ${FFMPEG_INCLUDE_DIRS})
  endif()
  ```
- [ ] Create `GeneralsMD/Code/GameEngineDevice/Include/FFmpegVideoPlayer.h`
- [ ] Create `GeneralsMD/Code/GameEngineDevice/Source/FFmpegVideoPlayer.cpp`
- [ ] Implement basic playback:
  1. `openVideo()` → `avformat_open_input()`, find video stream
  2. `decodeFrame()` → `av_read_frame()`, `avcodec_send_packet()`, `avcodec_receive_frame()`
  3. `renderFrame()` → Convert YUV→RGB with `sws_scale()`, copy to SDL texture
- [ ] Test with sample `.mp4` video (not game video yet)

**Spike Outcome**:
- ✅ Success: FFmpeg decodes + renders to SDL window → Proceed to Phase 3B
- ❌ Failure: Too complex, performance issues → Recommend stub or defer

### Phase 3B: Game Integration (If Spike Succeeds)

**Tasks**:
- [ ] Locate game video trigger points:
  - Main menu intro video
  - Campaign briefing videos
  - Victory/defeat cinematics
- [ ] Replace Bink calls with FFmpegVideoPlayer:
  ```cpp
  #ifdef _WIN32
      BinkVideoPlayer* player = new BinkVideoPlayer();
  #else
      FFmpegVideoPlayer* player = new FFmpegVideoPlayer();
  #endif
  player->playVideo("Data/Movies/intro.bik");  // Or .webm
  ```
- [ ] Handle video skipping (ESC key or mouse click):
  ```cpp
  while (player->isPlaying()) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
          if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
              player->stop();
          }
      }
      player->decodeFrame();
      player->renderFrame();
  }
  ```
- [ ] Test with actual game videos (Bink or converted)

### Phase 3C: Video Conversion Tooling (If Needed)

**If users must convert videos**:
- [ ] Create `scripts/convert_videos.sh`:
  ```bash
  #!/bin/bash
  # Convert all .bik videos to .webm (VP9)
  for file in Data/Movies/*.bik; do
      ffmpeg -i "$file" -c:v libvpx-vp9 -b:v 2M -c:a libopus "${file%.bik}.webm"
  done
  ```
- [ ] Document in `docs/LINUX_VIDEO_SETUP.md`
- [ ] Note legal disclaimer (users must own original game)

---

## Acceptance Criteria (Phase 3)

Phase 3 is **COMPLETE** when one of the following outcomes is achieved:

### Outcome A: Implementation ✅
- [ ] FFmpeg video player implemented and working
- [ ] At least one intro video plays on Linux
- [ ] Videos can be skipped (ESC key)
- [ ] Game handles missing videos gracefully (skip to next scene)
- [ ] Windows builds still use Bink (no regressions)
- [ ] Documentation for video conversion (if needed)

### Outcome B: Stub with Graceful Degradation ✅
- [ ] Game launches without crashing when videos missing
- [ ] Clear log message: "Video playback not supported on Linux"
- [ ] Immediately proceeds to next scene (main menu, campaign briefing text)
- [ ] User experience acceptable (no black screens or hangs)
- [ ] Documentation explains limitation

### Outcome C: Deferred Indefinitely ✅
- [ ] Research documented in `docs/WORKDIR/support/phase3-video-research.md`
- [ ] Rationale clearly stated (complexity vs. value)
- [ ] Recommendation: Focus on gameplay, defer videos to future or community
- [ ] Stub implementation ensures no crashes

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

## Timeline (Spike)

- **Week 1**: Research (codebase analysis, reference implementations, FFmpeg feasibility)
- **Week 2**: Prototype (FFmpeg decode + render one video)
- **Decision Point**: Proceed, stub, or defer?
- **Week 3-4** (if proceeding): Game integration, video conversion docs, testing

**Total Estimate**: 2-4 weeks (or 1 week if stubbed)

---

## Deliverables

### If Implemented:
- `FFmpegVideoPlayer.{h,cpp}` implementation
- CMake FFmpeg integration (`cmake/ffmpeg.cmake`)
- Video conversion script (`scripts/convert_videos.sh`)
- Documentation: `docs/LINUX_VIDEO_SETUP.md`

### If Stubbed:
- `VideoPlayerStub.{h,cpp}` (graceful no-op)
- Documentation: "Video playback limitations on Linux"

### If Deferred:
- Research document: `docs/WORKDIR/support/phase3-video-research.md`
- Recommendation for future work or community contribution

---

## Notes

- **Videos are NOT essential for gameplay**: Skirmish mode (primary use case) doesn't need videos
- **Campaign mode may require briefing videos**: Consider text-based alternative
- **Community feedback is critical**: Poll users on priority (gameplay vs. cinematics)
- **Legal concerns**: Users must own original game, converting videos is gray area
- **Windows builds unaffected**: Bink remains for Windows, videos work there

---

**Status Tracking**:
- [ ] A. Game Engine Integration Analysis
- [ ] B. Reference Implementation Research
- [ ] C. Alternative Video Codecs (research)
- [ ] Decision: Implement / Stub / Defer?
- [ ] (If implementing) Phase 3A: FFmpeg Prototype
- [ ] (If implementing) Phase 3B: Game Integration
- [ ] (If implementing) Phase 3C: Conversion Tooling

**Progress**: 0/? (spike, outcome uncertain)
