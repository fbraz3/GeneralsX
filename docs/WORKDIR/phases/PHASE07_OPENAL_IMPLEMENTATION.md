# PHASE 07: OpenAL Audio Implementation

**Status**: Planning  
**Start Date**: After Phase 06 Complete  
**Baseline**: Phase 06 Complete (VC6 32-bit, 100% SDL2 validated)

---

## 🎯 Phase 07 Goal

Replace Miles Audio with OpenAL abstraction layer. Create clean `AudioDevice` interface that game code uses, enabling 64-bit support and cross-platform audio.

**Success Criteria**:
- [x] AudioDevice abstraction fully designed and implemented
- [x] OpenAL backend functional (all major audio systems working)
- [x] Miles Audio completely replaced
- [x] All game audio tested and validated
- [x] Documentation complete

---

## 📋 Detailed Roadmap

### 07.1: Design AudioDevice Abstraction
**Effort**: 2 days  
**Owner**: Audio Architect

**Create header**: `Core/GameEngineDevice/Include/AudioDevice/AudioDevice.h`

```cpp
class AudioDevice {
public:
    virtual ~AudioDevice() {}
    
    // Initialization
    virtual void init() = 0;
    virtual void shutdown() = 0;
    virtual void update(float deltaTime) = 0;
    
    // Music playback
    virtual int playMusic(const char* filename, bool loop) = 0;
    virtual void stopMusic(int musicId) = 0;
    virtual void setMusicVolume(int musicId, float volume) = 0;
    
    // Sound effects (2D)
    virtual int playSound(const char* filename, float volume) = 0;
    virtual void stopSound(int soundId) = 0;
    
    // 3D Sound (crucial for RTS)
    virtual int play3DSound(const char* filename, 
                            float x, float y, float z,
                            float volume, float pitch) = 0;
    virtual void set3DSoundPosition(int soundId, 
                                    float x, float y, float z) = 0;
    virtual void stop3DSound(int soundId) = 0;
    
    // Listener (camera position)
    virtual void setListenerPosition(float x, float y, float z) = 0;
    virtual void setListenerOrientation(float forwardX, float forwardY, float forwardZ,
                                        float upX, float upY, float upZ) = 0;
    
    // Voice/Speech
    virtual int playVoice(const char* filename) = 0;
    virtual void stopVoice(int voiceId) = 0;
    virtual bool isVoicePlaying(int voiceId) = 0;
};
```

**Also create**:
- [ ] `AudioDeviceFactory.h` - Factory for creating AudioDevice instances
- [ ] `AudioDeviceOpenAL.h` - OpenAL implementation
- [ ] `AudioDeviceMiles.h` - Miles Audio fallback (for now)

**Deliverable**: Complete AudioDevice interface + documentation

---

### 07.2: Research OpenAL Integration
**Effort**: 2 days  
**Owner**: OpenAL Expert

**Tasks**:
- [ ] Review OpenAL-soft documentation
  - [ ] Context creation/management
  - [ ] Source/Buffer management
  - [ ] 3D audio positioning
  - [ ] File loading (WAV, OGG)

- [ ] Study reference implementations:
  - [ ] ioquake3 audio subsystem
  - [ ] Doom 3 audio wrapper
  - [ ] Other game engine examples

- [ ] Check VC6 compatibility:
  - [ ] Can VC6 link to libopenal.lib?
  - [ ] Header compatibility
  - [ ] Runtime compatibility (Windows DLLs)

- [ ] Create technical specification:
  - [ ] `docs/WORKDIR/support/OPENAL_INTEGRATION_SPEC.md`
  - ALContext initialization strategy
  - Thread safety model
  - Error handling approach

**Deliverable**: OPENAL_INTEGRATION_SPEC.md + compatibility verified

---

### 07.3: Implement OpenALDevice
**Effort**: 4-5 days  
**Owner**: Audio Implementation Lead

**File Structure**:
```
Core/GameEngineDevice/Source/AudioDevice/
├── OpenAL/
│   ├── OpenALDevice.h
│   ├── OpenALDevice.cpp         (main implementation)
│   ├── OpenALBuffer.h           (sound caching)
│   ├── OpenALBuffer.cpp
│   ├── OpenALSource.h           (sound instances)
│   ├── OpenALSource.cpp
│   ├── OpenALListener.h         (3D positioning)
│   └── OpenALListener.cpp
```

**Implementation Tasks**:

1. **OpenALDevice.cpp** (main class)
   - [ ] `init()` - Create ALContext, ALDevice
   - [ ] `shutdown()` - Clean up resources
   - [ ] `update()` - Process pending operations
   - [ ] Error handling wrapper for OpenAL calls

2. **OpenALBuffer.cpp** (sound file caching)
   - [ ] `loadWAV()` - Parse WAV files
   - [ ] `loadOGG()` - Parse OGG Vorbis files (optional, nice-to-have)
   - [ ] `createALBuffer()` - Upload to OpenAL
   - [ ] Caching strategy (avoid reloading same file)

3. **OpenALSource.cpp** (sound playback)
   - [ ] `play()` - Create ALSource, play
   - [ ] `stop()` - Stop and clean up
   - [ ] `setPitch()`, `setGain()` - Audio effects
   - [ ] `setPosition()` - 3D positioning
   - [ ] `isPlaying()` - Query state

4. **OpenALListener.cpp** (3D audio)
   - [ ] `setPosition()` - Listener location (camera)
   - [ ] `setOrientation()` - Camera direction
   - [ ] `setVelocity()` - Doppler effect (optional)

5. **Miles Audio Compatibility Layer** (temporary)
   - [ ] `AudioDeviceMiles.h` - Keep Miles for fallback
   - [ ] Use factory to switch at runtime: `AudioDevice::CreateDefault()`

**Testing Each Component**:
- [ ] Unit test: Can create ALContext
- [ ] Unit test: Can load WAV file
- [ ] Unit test: Can play simple sound
- [ ] Unit test: 3D positioning works

**Deliverable**: Fully functional OpenALDevice implementation

---

### 07.4: Hook Game Audio Calls
**Effort**: 2-3 days  
**Owner**: Game Integration Lead

**Find all current audio calls**:
```bash
grep -r "SoundManager\|AudioManager\|playSound\|Miles" \
  GeneralsMD/Code/GameEngine/ --include="*.cpp" | head -50
```

**Create wrapper functions** in existing game code:
```cpp
// Example: SoundManager.cpp (existing)
void SoundManager::playSoundEffect(const char* filename, float volume) {
    // BEFORE: Direct Miles API
    // AFTER: Route through AudioDevice
    if (g_audioDevice) {
        g_audioDevice->playSound(filename, volume);
    }
}
```

**Identify categories of audio**:
1. **Background Music** - Single stream, loops
2. **Sound Effects** - Short clips, many simultaneous
3. **Unit Voices** - Speech, positional (3D)
4. **Ambient Sounds** - Wind, crowd, environmental
5. **UI Sounds** - Button clicks, confirmations

**For each category**:
- [ ] Map current Miles calls
- [ ] Create AudioDevice equivalent
- [ ] Update game code to use new calls
- [ ] Test in-game

**Deliverable**: All game audio routed through AudioDevice interface

---

### 07.5: Audio File Asset Audit
**Effort**: 2 days  
**Owner**: Asset Manager

**Tasks**:
- [ ] Inventory all audio files
  ```bash
  find GeneralsMD/ -type f \( -name "*.wav" -o -name "*.mp3" -o -name "*.ogg" \)
  ```

- [ ] Determine format support needed:
  - [ ] WAV - REQUIRED (easiest for OpenAL)
  - [ ] MP3 - OPTIONAL (need decoder)
  - [ ] OGG Vorbis - NICE (compression, already in some games)

- [ ] For each unsupported format:
  - [ ] Convert to WAV or OGG
  - OR
  - [ ] Implement decoder (libogg, libvorbis)

- [ ] Create file format specification:
  - [ ] Where audio files located
  - [ ] Naming convention for lookup
  - [ ] Streaming vs buffered strategy

**Deliverable**: All game audio in supported formats, documented

---

### 07.6: Build, Test, Validate
**Effort**: 2 days  
**Owner**: QA Lead

**Build Tasks**:
- [ ] Add OpenAL library to vcpkg.json (or link manually)
- [ ] Update CMakeLists.txt to link libopenal.lib
- [ ] Clean build: `cmake --preset vc6 && cmake --build build/vc6`
- [ ] Verify 0 linker errors

**Test Plan**:

1. **Audio Device Tests**
   - [ ] Game starts without audio errors
   - [ ] OpenAL context initializes successfully
   - [ ] Error handling works (graceful fallback if OpenAL missing)

2. **Music Playback**
   - [ ] Main menu music plays
   - [ ] Music loops properly
   - [ ] Music volume control works
   - [ ] Music stops on game start

3. **Sound Effects**
   - [ ] Click sounds on UI work
   - [ ] Weapon fire sounds trigger correctly
   - [ ] Explosion sounds audible and in sync

4. **3D Audio**
   - [ ] Unit voice lines have correct positional audio
   - [ ] Sounds attenuate with distance
   - [ ] Pan left/right as unit moves
   - [ ] Listener updates as camera moves

5. **Voice/Speech**
   - [ ] Narrator voice at menu works
   - [ ] Unit acknowledgments clear
   - [ ] No stuttering/dropout

6. **Performance**
   - [ ] Audio doesn't block game loop
   - [ ] <5ms per audio update frame
   - [ ] Memory usage reasonable (<50MB)

**Test Execution**:
```bash
# Launch with logging
GeneralsXZH.exe -win -noshellmap 2>&1 | tee logs/phase07_audio_test.log

# Listen for:
# - OpenAL initialization success
# - Music playback
# - SFX playback
# - No errors logged
```

**Deliverable**: QA Report - All tests passed ✅

---

### 07.7: Documentation & Migration Guide
**Effort**: 1 day  
**Owner**: Documentation Lead

**Create**:
- [ ] `docs/WORKDIR/support/OPENAL_IMPLEMENTATION_COMPLETE.md`
  - Architecture overview
  - AudioDevice interface documentation
  - Usage examples
  - Known limitations/future improvements

- [ ] `docs/WORKDIR/support/MIGRATION_FROM_MILES_TO_OPENAL.md`
  - Miles API → OpenAL mapping
  - Code change examples
  - Troubleshooting guide

- [ ] `docs/WORKDIR/support/AUDIO_FILE_FORMAT_SPEC.md`
  - Which formats supported
  - File location conventions
  - Performance characteristics

**Deliverable**: Complete audio documentation + migration guide

---

## 📅 Timeline Summary

| Task | Duration | Deliverable |
|------|----------|-------------|
| **07.1** | 2 days | AudioDevice interface |
| **07.2** | 2 days | OpenAL spec + research |
| **07.3** | 4-5 days | OpenALDevice implementation |
| **07.4** | 2-3 days | Hook game audio calls |
| **07.5** | 2 days | Audio asset audit + convert |
| **07.6** | 2 days | Testing & QA |
| **07.7** | 1 day | Documentation |
| **TOTAL** | **~16 days** | **OpenAL fully integrated** ✅ |

---

## 🚀 Next Phase (Phase 08): Vulkan Graphics Implementation

Once Phase 07 complete:
- AudioDevice pattern proven and tested
- Game runs fully on SDL2 + OpenAL
- Ready for graphics abstraction layer (GraphicsDevice)
- Foundation solid for cross-platform support

---

**Prepared by**: GeneralsX Development Team  
**Date**: 2026-01-15  
**Status**: Ready for Phase 07 Kickoff (after Phase 06 complete)
