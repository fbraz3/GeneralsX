# PHASE02: Linux Audio - OpenAL Integration

**Status**: 📋 Not Started (Blocked by Phase 1 smoke tests)
**Created**: 2026-02-08
**Prerequisites**: Phase 1 complete (DXVK + SDL3 working)

## Goal

Replace Miles Sound System with OpenAL for native Linux audio, using jmarshall's implementation as reference while adapting for Zero Hour's expanded audio features.

## Progress Snapshot
⏳ Waiting for Phase 1 smoke tests to complete  
📚 Research jmarshall OpenAL patterns (Generals base game)  
🎯 Plan Zero Hour audio hook adaptations

---

## Context

**Current State (Phase 1)**:
- ✅ OpenALAudioManager exists (~550 lines)
- ✅ Device/context lifecycle implemented
- ✅ Source pooling ready (32 2D + 128 3D + 4 stream)
- ⚠️ Audio events stubbed (no actual audio playback)
- ⚠️ Music tracks stubbed (no file loading/streaming)
- ⚠️ 3D audio listener not wired (update() stub)

**Phase 2 Focus**:
Wire OpenAL backend to game audio system, implement audio event tracking, music streaming, and 3D audio positioning.

---

## Scope

### In Scope ✅
- Audio event tracking (addAudioEvent with handle management)
- Sound effect playback (gunshots, explosions, unit responses)
- Music streaming (background tracks, victory/defeat themes)
- 3D audio positioning (listener + source positions)
- Audio format conversion (Miles formats → OpenAL formats)
- Volume/pan/pitch controls
- Buffer management and streaming
- Miles→OpenAL API compatibility layer

### Out of Scope ❌
- Video audio sync (Phase 3)
- EAX/reverb effects (future enhancement)
- Lip-sync/dialogue system (future)
- Network audio sync (future)

---

## Implementation Plan

### A. Research Phase (Estimated: 2-3 days)

**jmarshall Reference Analysis**:
- [ ] Study `references/jmarshall-win64-modern/Code/Audio/` structure
- [ ] Document MusicManager implementation (streaming patterns)
- [ ] Document AudioManager event system (handle tracking)
- [ ] Document buffer lifecycle (creation, queueing, cleanup)
- [ ] Identify Generals vs Zero Hour differences
- [ ] Create adaptation strategy document

**Key Files to Analyze** (jmarshall):
```
Code/Audio/
├── AudioManager.cpp      # Core audio event system
├── MusicManager.cpp      # Background music streaming
├── AudioEventRTS.cpp     # RTS-specific audio events
└── ALBuffer.cpp          # Buffer management
```

**fighter19 Audio Reference** (if applicable):
- [ ] Check if fighter19 has any audio improvements beyond stubs
- [ ] Document any DXVK-specific audio considerations

### B. Audio Event System (Core Foundation)

**Goal**: Wire OpenALAudioManager::addAudioEvent() to actual playback

**Tasks**:
- [ ] Create `AudioEvent` struct (handle, source, state, 3D params)
- [ ] Implement handle generation (unique ID per event)
- [ ] Implement `m_ActiveEvents` tracking (std::map<AudioEventHandle, AudioEvent>)
- [ ] Wire `addAudioEvent()`:
  ```cpp
  AudioEventHandle addAudioEvent(const AudioEventInfo* info) {
      // 1. Allocate source from pool
      // 2. Load buffer (implement loadAudioBuffer)
      // 3. Set source properties (gain, pitch, position)
      // 4. alSourcePlay(source)
      // 5. Return handle
  }
  ```
- [ ] Implement `removeAudioEvent(handle)`:
  ```cpp
  void removeAudioEvent(AudioEventHandle handle) {
      // 1. Find event in m_ActiveEvents
      // 2. alSourceStop(source)
      // 3. Release source to pool
      // 4. Remove from map
  }
  ```
- [ ] Implement `update()`:
  ```cpp
  void update() {
      // 1. Poll all active sources (alGetSourcei AL_SOURCE_STATE)
      // 2. Remove finished one-shot sounds
      // 3. Update 3D listener position (see Section D)
  }
  ```

**Testing**:
- Simple test: Load `.wav` file, play on button press
- Verify handle lifecycle (create → play → remove)
- Check source pool recycling

### C. Audio Buffer Management

**Goal**: Load game audio files into OpenAL buffers

**Tasks**:
- [ ] Implement `loadAudioBuffer(const char* filename)`:
  - Detect format (WAV, MP3, OGG - check game assets)
  - Decode audio data (may need libsndfile or similar)
  - Create AL buffer: `alGenBuffers()`, `alBufferData()`
  - Return ALuint buffer ID
- [ ] Create buffer cache (avoid reloading same file):
  ```cpp
  std::map<std::string, ALuint> m_BufferCache;
  ```
- [ ] Implement buffer cleanup (destructor, shutdown):
  ```cpp
  void releaseAllBuffers() {
      for (auto& pair : m_BufferCache) {
          alDeleteBuffers(1, &pair.second);
      }
      m_BufferCache.clear();
  }
  ```

**Audio File Locations** (Zero Hour):
- Check `Data/Audio/` for `.wav`, `.mp3` files
- Check if audio is in `.big` archives (may need decompression)
- Document format used by Miles in original game

**Dependencies**:
- May need: libsndfile, libvorbis, or mpg123 for decoding
- Check CMake dependencies: `find_package(SndFile)`

### D. 3D Audio Positioning

**Goal**: Update listener and source positions for spatial audio

**Tasks**:
- [ ] Wire listener position in `update()`:
  ```cpp
  void update() {
      // Get camera/listener position from game engine
      Vector3 position = getListenerPosition();
      Vector3 velocity = getListenerVelocity();
      Vector3 forward = getListenerForward();
      Vector3 up = getListenerUp();
      
      alListener3f(AL_POSITION, position.x, position.y, position.z);
      alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
      ALfloat orientation[] = {forward.x, forward.y, forward.z, up.x, up.y, up.z};
      alListenerfv(AL_ORIENTATION, orientation);
  }
  ```
- [ ] Implement per-source 3D positioning:
  ```cpp
  void setAudioEventPosition(AudioEventHandle handle, const Vector3& pos) {
      AudioEvent* event = findEvent(handle);
      if (event) {
          alSource3f(event->source, AL_POSITION, pos.x, pos.y, pos.z);
      }
  }
  ```
- [ ] Add 3D audio parameters to AudioEventInfo:
  - Position, velocity, reference distance, max distance
  - Rolloff factor for distance attenuation

**Game Integration**:
- Hook into GameEngine camera updates (listener follows camera)
- Hook into unit positions (gunshots, explosions at unit location)

### E. Music Streaming System

**Goal**: Background music playback with track management

**Reference**: jmarshall's `MusicManager.cpp`

**Tasks**:
- [ ] Implement streaming source (separate from SFX pool):
  ```cpp
  ALuint m_MusicSource;
  ALuint m_MusicBuffers[4];  // 4 buffers for streaming
  ```
- [ ] Implement `playMusicTrack(const char* filename)`:
  - Open file stream (libsndfile or similar)
  - Queue initial buffers (`alSourceQueueBuffers`)
  - Start playback (`alSourcePlay`)
  - Set looping behavior
- [ ] Implement streaming update loop:
  ```cpp
  void updateMusicStream() {
      int buffersProcessed = 0;
      alGetSourcei(m_MusicSource, AL_BUFFERS_PROCESSED, &buffersProcessed);
      
      while (buffersProcessed--) {
          ALuint buffer;
          alSourceUnqueueBuffers(m_MusicSource, 1, &buffer);
          
          // Read next chunk from file
          streamMusicData(buffer);
          
          // Re-queue buffer
          alSourceQueueBuffers(m_MusicSource, 1, &buffer);
      }
  }
  ```
- [ ] Implement `nextMusicTrack()` / `prevMusicTrack()`:
  - Stop current track
  - Load next from playlist
  - Crossfade (optional, Phase 4)
- [ ] Wire to game music system (menu themes, battle music, victory/defeat)

**Music File Locations** (Zero Hour):
- Check `Data/Audio/Music/` for tracks
- Document playlist system (XML? INI?)

### F. Miles→OpenAL Compatibility Layer

**Goal**: Minimize game code changes by wrapping Miles API

**Strategy** (if feasible):
- [ ] Create `miles_compat.h` with OpenAL-backed Miles stubs
- [ ] Map Miles functions to OpenAL equivalents:
  ```cpp
  // Miles API                    // OpenAL equivalent
  AIL_open_stream()         →     OpenAL streaming source
  AIL_set_stream_volume()   →     alSourcef(AL_GAIN)
  AIL_stream_status()       →     alGetSourcei(AL_SOURCE_STATE)
  ```
- [ ] Document unmapped features (EAX, MIDI, etc.)

**Alternative Approach**:
- Direct replacement: Change game code to call OpenALAudioManager directly
- Pros: Cleaner, no legacy API baggage
- Cons: More game code changes

**Decision**: Choose based on game code inspection (how tightly coupled is Miles?)

### G. Testing & Validation

**Unit Tests** (manual, no automated tests yet):
- [ ] Load and play single `.wav` file
- [ ] Multiple simultaneous sounds (source pool stress test)
- [ ] 3D audio (walk around sound source, verify attenuation)
- [ ] Music streaming (long track, verify no gaps/stutters)
- [ ] Volume controls (master, SFX, music volumes independent)

**Integration Tests**:
- [ ] Launch game, verify menu music plays
- [ ] Start skirmish, verify unit response sounds
- [ ] Trigger explosions, verify 3D positioning
- [ ] Play for 30 minutes, check for audio glitches/leaks

**Performance**:
- [ ] Profile OpenAL overhead (should be <1% CPU)
- [ ] Check memory usage (buffer cache size)
- [ ] Verify no audio/video desync (Phase 3 dependency)

### H. CMake & Dependencies

**Tasks**:
- [ ] Add OpenAL detection to CMake:
  ```cmake
  if(SAGE_USE_OPENAL)
      find_package(OpenAL REQUIRED)
      target_link_libraries(GameEngineDevice PRIVATE OpenAL::OpenAL)
  endif()
  ```
- [ ] Add audio decoding library (libsndfile):
  ```cmake
  find_package(SndFile REQUIRED)
  target_link_libraries(GameEngineDevice PRIVATE SndFile::sndfile)
  ```
- [ ] Update Docker build to include dependencies:
  ```dockerfile
  RUN apt install -y libopenal-dev libsndfile1-dev
  ```

---

## Acceptance Criteria (Phase 2)

Phase 2 is **COMPLETE** when:
- [ ] Game plays menu music on launch (Linux build)
- [ ] Sound effects work (gunshots, explosions, unit voices)
- [ ] 3D audio positioning works (sounds attenuate with distance)
- [ ] Music tracks advance (victory/defeat themes trigger)
- [ ] No audio crashes or memory leaks (30-minute stress test)
- [ ] Windows builds still use Miles Sound System (no regressions)

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| Audio format incompatibility | High | Research game archive structure early, test sample files |
| Miles API deeply coupled | Medium | Create compatibility layer or document replacement strategy |
| OpenAL performance issues | Low | Profile early, OpenAL is mature and efficient |
| Buffer memory leaks | Medium | Implement strict buffer lifecycle, stress test |
| 3D audio artifacts | Low | Use reference distances from original game |

---

## References

### Primary
- **jmarshall OpenAL implementation**: `references/jmarshall-win64-modern/Code/Audio/`
- **OpenAL Programming Guide**: `docs/ETC/` (download if needed)
- **Miles Sound System docs**: `docs/ETC/` (reverse-engineer if no docs available)

### Game Audio Structure
- [ ] Document Zero Hour audio file locations
- [ ] Document audio format (WAV? MP3? Compressed?)
- [ ] Document playlist system
- [ ] Document 3D audio parameters (reference distances, rolloff)

---

## Timeline (Estimated)

- **Week 1**: Research (jmarshall patterns, game audio structure)
- **Week 2**: Audio event system + buffer management
- **Week 3**: 3D audio positioning + music streaming
- **Week 4**: Testing, integration, bug fixes

**Total Estimate**: 4 weeks (blockers: game audio format discovery, Miles API coupling)

---

## Deliverables

- Updated `OpenALAudioManager.{h,cpp}` with full event system
- Audio buffer management implementation
- Music streaming system
- CMake dependencies for OpenAL + audio decoding
- Phase 2 test results (manual tests documented)
- Dev blog updates

---

## Phase 2 → Phase 3 Handoff

**Prerequisites for Phase 3 (Video Playback)**:
- Audio system working (Phase 2 complete)
- SDL3 windowing stable (Phase 1 complete)
- Game launches and runs without crashes

**Phase 3 Blockers**:
- Bink video codec (proprietary, may need alternative)
- Audio/video sync requirements
- Intro video skip mechanism

---

## Notes

- **jmarshall reference is Generals base game ONLY**: Zero Hour may have additional audio features (new unit voices, expanded music system). Adapt accordingly.
- **Audio file discovery is critical**: If audio is compressed in `.big` archives, we need decompression first.
- **OpenAL is mature**: Expect fewer issues than DXVK/graphics layer.
- **Preserve Windows builds**: Miles Sound System must remain functional behind compile guards.

---

**Status Tracking**:
- [ ] A. Research Phase
- [ ] B. Audio Event System
- [ ] C. Buffer Management
- [ ] D. 3D Audio Positioning
- [ ] E. Music Streaming
- [ ] F. Miles Compatibility Layer
- [ ] G. Testing & Validation
- [ ] H. CMake & Dependencies

**Progress**: 0/8 sections complete
