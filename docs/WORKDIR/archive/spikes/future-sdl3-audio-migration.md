# Future Architecture: SDL3 Audio Migration

**Document Type**: Strategic Architecture Proposal (Phase 4+)  
**Author**: Community & Bender AI  
**Date**: 2026-02-13  
**Status**: Planning / Not Implemented  
**Priority**: **MEDIUM** (Strategic modernization, not blocking but important long-term)

---

## Executive Summary

This document proposes a **strategic modernization** of the GeneralsX audio system by migrating from OpenAL to SDL3 Audio. This initiative addresses a critical long-term goal: **eliminating all proprietary dependencies** and centralizing on a single, fully open-source, cross-platform library ecosystem.

**Key Insights**:
1. Both OpenAL and SDL3 Audio implement the same audio abstraction layer pattern (`AudioManager` interface)
2. Migration requires minimal game logic changes—just swapping the backend implementation
3. **This completes a strategic victory**: Original game (Miles + Bink proprietaries) → Phase 2 (open source) → Phase 4 (unified open source)
4. Creates a **fully auditable, community-driven codebase** with zero proprietary tech dependencies

---

## The Proprietary Code Legacy Problem

### Original Game (2003)
```
Graphics:  DirectX 8 (Microsoft proprietary)
Audio:     Miles Sound System (RAD Game Tools proprietary)
Video:     Bink Video (RAD Game Tools proprietary)
```
**Result**: Game locked to Windows, dependent on proprietary SDK maintenance

### Current Phase 2 (Linux Port)
```
Graphics:  Vulkan (open source) + DXVK (open source wrapper)
Audio:     OpenAL (open source) ← community maintained
Video:     FFmpeg (open source) ← community maintained
```
**Progress**: Mostly free of proprietary tech! But OpenAL is separate library.

### Proposed Phase 4+ (Strategic Unification)
```
Graphics:  Vulkan (open source) + SDL3 renderer (open source)
Audio:     SDL3 Audio (open source) ← single library
Video:     FFmpeg (open source) ← community maintained
```
**Victory**: **100% open source, fully auditable, zero proprietary dependencies**
- Single library handles both graphics + audio
- Community can fork/maintain indefinitely
- Legal clarity (no licensing issues)
- Attracts contributors who value code freedom

---

## Why Consider SDL3 Audio?

### Current State (Phase 2)
- **Audio Backend**: OpenAL (cross-platform, proven in games)
- **Dependencies**: LibOpenAL + system audio drivers (ALSA, PulseAudio, CoreAudio)
- **Integration**: Separate from SDL3 graphics system
- **Status**: Functional, but requires two separate audio/graphics libraries

### Dependency Evolution

| Aspect | Original (2003) | Phase 2 (Current) | Phase 4 (SDL3 Future) |
|--------|---|---|---|
| **Graphics** | DirectX (MSFT proprietary) | Vulkan + DXVK (open) | Vulkan + SDL3 (open) |
| **Audio** | Miles Sound System (RAD proprietary) | OpenAL (open source) | **SDL3 Audio (open source)** |
| **Video** | Bink Video (RAD proprietary) | FFmpeg (open source) | FFmpeg (open source) |
| **Library Count** | 3 proprietary SDKs | 4 libraries (OpenAL + SDL3 + FFmpeg + Vulkan) | **3 libraries (SDL3 + FFmpeg + Vulkan)** |
| **Build Complexity** | Single proprietary stack | Multiple open-source deps | **Single unified ecosystem** |
| **Code Auditability** | None (proprietary) | Partial (OpenAL separate) | **100% auditable, 100% open** |
| **Community Fork Ability** | Impossible | Possible (but complex) | **Trivial (single lib)** |
| **Legal Risk** | High (SDKs may disappear) | Low | **None (pure open source)** |

### SDL3 Audio Advantages (Technical)

| Aspect | OpenAL (Current) | SDL3 Audio (Proposed) |
|--------|------------------|---------------------|
| **Integration** | Separate library | Part of SDL3 ecosystem |
| **Backend Selection** | System-dependent | SDL3 handles automatically |
| **Dependencies** | libopenal + system drivers | Zero extra (SDL3 already used) |
| **API Complexity** | Low-level (callbacks, sources, listeners) | Higher-level (simpler playback API) |
| **Audio Formats** | WAV, OGG (via external libs) | WAV, FLAC, OGG native support |
| **Synchronization** | Manual coordination needed | Automatic with SDL3 rendering |
| **Maintenance** | Share upstream (OpenAL maintainers) | Maintained by SDL team |
| **Cross-Platform** | Yes (Windows, macOS, Linux) | Yes (Windows, macOS, Linux) |

### When SDL3 Audio Shines
✅ **Eliminates proprietary tech entirely** (strategic victory)  
✅ **Single library paradigm** (SDL3 = graphics + audio + windowing)  
✅ Simplified build system (one library, cleaner CMake)  
✅ Better audio/video sync (Phase 3 FFmpeg integration)  
✅ Reduced external dependencies (fewer build-time surprises)  
✅ Future macOS port (Phase 5) benefits  
✅ Community attraction ("fully open source" > "mostly open source")  
✅ 100% Code auditability (no black-box OpenAL concerns)  

### When to Keep OpenAL
✅ Already implemented and tested (don't fix what works)  
✅ Advanced 3D audio features (if needed beyond current game audio)  
✅ Minimize change risk during Phase 2  

---

## Migration Strategy

### Phase 1: Design SDL3 AudioManager Implementation

**Goal**: Plan replacement without breaking changes

**Tasks**:
1. **Analyze current `OpenALAudioManager` interface**
   - Document all public methods
   - Map to SDL3 Audio equivalents
   - Identify gaps or feature mismatches

2. **Design `SDL3AudioManager` class**
   ```cpp
   class SDL3AudioManager : public AudioManager {
   public:
       // Lifecycle (same as OpenAL)
       virtual void init();
       virtual void reset();
       virtual void update();
       
       // Playback control
       virtual void openDevice();
       virtual void closeDevice();
       virtual void stopAudio(AudioAffect which);
       virtual void pauseAudio(AudioAffect which);
       
       // Audio events
       virtual AudioHandle addAudioEvent(const AudioEventRTS *event);
       virtual void removeAudioEvent(AudioHandle handle);
       
       // 3D audio
       virtual void setListenerPosition(const Coord3D *pos, const Coord3D *orient);
       
   private:
       SDL_AudioDeviceID m_device;        // SDL3 audio device handle
       std::map<AudioHandle, SDL_AudioStream*> m_activeStreams;  // Active audio streams
       // ... buffer pool, listener pos, etc.
   };
   ```

3. **Map OpenAL → SDL3 concepts**
   ```
   OpenAL                          SDL3 Audio
   ================================================
   alGenSources()              →   SDL_CreateAudioStream()
   alSourcePlay(source)        →   SDL_SetAudioStreamGetCallback()
   alListenerfv()              →   (Manual tracking) 
   ALuint source               →   SDL_AudioStream*
   alDeleteBuffers()           →   SDL_DestroySample()
   ```

**Deliverable**: Design document with API mapping

### Phase 2: Prototype SDL3 Backend

**Goal**: Build proof-of-concept (`SDL3AudioManager`)

**Scope**:
- [ ] Implement basic 2D audio playback
- [ ] Wire audio format conversion (raw bytes → SDL format)
- [ ] Test with simple WAV file
- [ ] Measure CPU/memory usage vs. OpenAL
- [ ] Benchmark 10+ simultaneous sounds

**Acceptance Criteria**:
- ✅ SDL3 audio plays sound effects without crashes
- ✅ Volume control works
- ✅ No significant CPU overhead vs. OpenAL
- ✅ Build system integration clean (no new build flags needed)

**Risk**: API compatibility, performance regression

### Phase 3: Integration Testing

**Goal**: Wire SDL3 backend to game audio pipeline

**Tasks**:
- [ ] Replace `OpenALAudioManager` with `SDL3AudioManager` in factory
- [ ] Run full audio test suite (same 2D/3D/music tests as OpenAL)
- [ ] Compare audio output quality (A/B test)
- [ ] Profile performance (FPS, memory, latency)
- [ ] Test all game audio scenarios:
  - SFX (explosions, footsteps, gunfire)
  - Music (main menu, gameplay, victory)
  - Voice lines (unit responses, mission dialogue)
  - 3D audio (turret targeting, unit positioning)

**Acceptance Criteria**:
- ✅ Audio quality matches OpenAL baseline
- ✅ No audio-visual desynchronization
- ✅ Performance within 5% of OpenAL
- ✅ All regression tests pass

### Phase 4: Deployment & Optimization

**Goal**: Ready SDL3 Audio for production

**Tasks**:
- [ ] Optimize buffer pooling for SDL3 (may differ from OpenAL)
- [ ] Profile under heavy load (100+ audio events)
- [ ] Test on multiple Linux distros (audio subsystems vary)
- [ ] Validate Windows/macOS backends (if Phase 5 underway)
- [ ] Document migration rationale for maintainers

**Deliverable**: Stable `SDL3AudioManager` ready for production

---

## Implementation Details

### BufferPool Management

**OpenAL (Current)**:
```cpp
// Source pool
std::vector<ALuint> m_2dSources;    // 32 pre-allocated sources
std::vector<ALuint> m_3dSources;    // 128 pre-allocated sources

// Buffer management
std::map<std::string, ALuint> m_bufferCache;  // filename → buffer
```

**SDL3 (Proposed)**:
```cpp
// Stream pooling (simpler than source management)
std::vector<SDL_AudioStream*> m_activeStreams;
std::map<std::string, SDL_AudioBuffer*> m_bufferCache;

// Callback-based playback
SDL_SetAudioStreamGetCallback(stream, myAudioCallback, userdata);
```

### 3D Audio Positioning

**OpenAL**:
```cpp
// Listener position (automatic 3D spatialization)
alListener3f(AL_POSITION, x, y, z);

// Source position
alSource3f(source, AL_POSITION, x, y, z);
alSourcef(source, AL_GAIN, volume);  // Auto-attenuate with distance
```

**SDL3** (More Manual):
```cpp
// SDL3 doesn't auto-attenuate, but we can:
// 1. Calculate distance manually
// 2. Adjust gain based on distance
// 3. Pan left/right based on angle

float distance = calculateDistance(listenerPos, sourcePos);
float gain = calculateGainFromDistance(distance);
SDL_SetAudioStreamVolume(stream, gain);
```

**Trade-off**: SDL3 requires manual 3D calculations, but gives fine-grained control.

### Audio Format Conversion

**OpenAL** (via external libs):
```cpp
// Load OGG → PCM
vorbis_file vf;
ov_open_callbacks(...);
ov_read(...);  // Stream OGG data
```

**SDL3** (Native Support):
```cpp
// Load OGG directly
SDL_AudioSpec spec;
SDL_AudioBuffer* buffer = SDL_LoadWAV("music.ogg", &spec, nullptr);
// SDL3 auto-detects format (WAV, FLAC, OGG)
```

---

## Build System Integration

### Current (OpenAL)
```cmake
# cmake/audio.cmake
find_package(OpenAL REQUIRED)
target_link_libraries(GameEngineDevice OpenAL::OpenAL)

# CMakeLists.txt
if(SAGE_USE_OPENAL)
    target_sources(GameEngineDevice OpenAL/OpenALAudioManager.cpp)
endif()
```

### Proposed (SDL3 Audio)
```cmake
# No new find_package() needed - SDL3 already found
# Just use SDL3::SDL3

if(SAGE_USE_SDL3_AUDIO)
    target_sources(GameEngineDevice SDL3Audio/SDL3AudioManager.cpp)
    target_link_libraries(GameEngineDevice SDL3::SDL3)
endif()
```

### Compile Flags
```cmake
# Optional: Use both backends side-by-side during transition
option(SAGE_USE_OPENAL "Use OpenAL audio" ON)
option(SAGE_USE_SDL3_AUDIO "Use SDL3 audio (future)" OFF)

# Only one backend active at compile time
if(SAGE_USE_OPENAL)
    target_compile_definitions(GameEngineDevice PRIVATE USE_OPENAL)
elseif(SAGE_USE_SDL3_AUDIO)
    target_compile_definitions(GameEngineDevice PRIVATE USE_SDL3_AUDIO)
else()
    message(FATAL_ERROR "No audio backend selected!")
endif()
```

---

## Potential Issues & Mitigations

| Issue | Probability | Mitigation |
|-------|------------|-----------|
| **SDL3 API changes** | Medium | Lock SDL3 version in vcpkg.json once proven |
| **3D audio quality loss** | Low | Manual attenuation good enough for RTS (not 3D shooter) |
| **Performance regression** | Low | Early profiling (Phase 2 prototype) |
| **Audio sync lag** | Low | SDL3 maintains same timing as OpenAL |
| **Platform quirks** | Medium | Test on Ubuntu, Fedora, Steam Deck early |
| **Distraction from Phase 2** | **HIGH** | **DO NOT ATTEMPT UNTIL OPENAL STABLE** |

---

## Timeline & Effort

### Realistic Estimate
- **Phase 2 Design**: 2-3 hours (API analysis, documentation)
- **Phase 2 Prototype**: 8-12 hours (implement `SDL3AudioManager`, basic tests)
- **Phase 3 Integration**: 8-12 hours (wire to game, run regression suite)
- **Phase 4 Optimization**: 4-8 hours (profiling, tuning, docs)

**Total**: ~24-35 hours (3-4 developer-days)

### When to Attempt
✅ **After Phase 2 (OpenAL) is STABLE and proven**  
✅ Not blocking any current milestone  
✅ Community feedback shows audio working well  
✅ Strategic goal: "eliminate proprietary tech" drives priority  
✅ Phase 5 (macOS) imminent and benefit from unified SDL3 system

❌ **NOT** during Phase 2 active development (too risky)

**Strategic Context**: This migration represents the **final step** in liberating a 23-year-old proprietary game engine into a fully open-source, community-maintained project. That's a meaningful milestone worth planning for.

---

## Alternative: Keep OpenAL

**Valid Option**: OpenAL is proven, stable, and works well. No compelling reason to change if:
- Audio quality is acceptable
- Performance is good
- No maintenance burden
- Community is happy

**Decision Criteria** (collect at end of Phase 2):
- User survey: "Is audio quality acceptable?" (target >90% positive)
- Performance: "Is FPS impact acceptable?" (target <5% overhead)
- Maintenance: "Are there upstream OpenAL issues we must fix?" (if yes, consider SDL3)

---

## References

### SDL3 Audio Documentation
- **Official**: https://wiki.libsdl.org/SDL3/APIByCategory (Audio section)
- **API**: `SDL_audio.h` in SDL3 source
- **Examples**: https://github.com/libsdl-org/SDL/tree/main/examples

### OpenAL Comparison
- **Our Implementation**: [GeneralsXZH OpenALAudioManager](../../../../GeneralsMD/Code/GameEngineDevice/Include/OpenALAudioManager.h)
- **Current Status**: Functional 2D/3D/music playback

### Related Phases
- **Phase 2**: Current OpenAL implementation (blocking Phase 3/4)
- **Phase 3**: Video playback (FFmpeg integration would benefit from unified SDL3 system)
- **Phase 4**: Polish & Hardening (suitable for this migration if deemed beneficial)
- **Phase 5**: macOS port (would be easier with unified SDL3 audio/graphics)

---

## Decision Checkpoint

**Before committing to SDL3 Audio migration, stakeholders should decide**:

1. **Is OpenAL working well?** (target: >95% user satisfaction)
   - If NO → investigate why, fix OpenAL
   - If YES → SDL3 is nice-to-have but not urgent

2. **Does SDL3 Audio provide clear benefits?** (evaluate after Phase 2)
   - If NO → keep OpenAL (principle: don't fix what works)
   - If YES → proceed to Phase 2 design

3. **Is Phase 5 (macOS) planned soon?** (affects priority)
   - If YES → SDL3 migration becomes higher priority (unified system)
   - If NO → defer indefinitely (low priority)

---

## Conclusion

SDL3 Audio is a **strategic modernization** that would:
- **Eliminate proprietary tech entirely** (Miles/Bink → 100% open source)
- **Centralize ecosystem** on single library (graphics + audio under SDL3)
- **Simplify build system** (fewer dependencies, cleaner CMake)
- **Benefit Phase 5** (macOS port gets unified SDL3 system)
- **Reduce maintenance burden** long-term (fewer upstream partnerships)
- **Attract community** ("fully open source" messaging)

**Strategic Value**: This migration completes a 20+ year journey from proprietary game engine → fully community-maintained open-source project. That's a meaningful narrative and technical accomplishment.

**Timing**: Phase 4+ effort (not urgent during Phase 2). Stabilize OpenAL first, then evaluate post-Phase-2-completion. If audio quality is good and strategic goal matters to community, prioritize SDL3 migration over other Phase 4 polish items.

**Status**: Document this proposal for future consideration. Revisit after Phase 2 audio validation.

---

**Document End**  
*Proposal created: 2026-02-13*  
*Suggested review: After Phase 2 completion (target: 2026-03-XX)*
