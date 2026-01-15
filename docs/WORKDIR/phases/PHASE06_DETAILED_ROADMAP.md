# PHASE 06: Detailed Roadmap - SDL2 Audit + OpenAL Implementation

**Status**: Planning  
**Start Date**: 2026-01-15  
**Baseline**: Phase 05 Complete (VC6 32-bit, SDL2 + DirectX8 + Miles Audio)

---

## 🎯 Phase 06 Goals

1. **SDL2 Implementation Audit**: Ensure ALL windowing/input runs via SDL2, ZERO Win32 native calls
2. **OpenAL Audio Foundation**: Replace Miles Audio with OpenAL abstraction layer
3. **Deliverable**: VC6 game fully functional on SDL2 + OpenAL, ready for Vulkan phase

---

## 📋 Phase 06.1: SDL2 Implementation Audit (Week 1-2)

### Goal
Verify that game runs 100% on SDL2 abstraction layer. Any Win32 API calls in input/window management must be identified and rerouted through SDL2.

### 06.1.1: Map Current SDL2 Integration
**Effort**: 2 days  
**Owner**: Lead Architect

**Tasks**:
- [ ] Search codebase for SDL2 usage patterns
  ```bash
  grep -r "SDL_" GeneralsMD/Code/GameEngineDevice/ | wc -l
  grep -r "CreateWindow\|GetMessage\|DispatchMessage" GeneralsMD/Code --include="*.cpp" | grep -v "//"
  ```
- [ ] Document all SDL2 wrappers currently implemented:
  - [ ] SDL2 window creation
  - [ ] SDL2 event handling (keyboard, mouse)
  - [ ] SDL2 input callbacks
  - [ ] SDL2 timer functions
  
- [ ] Create matrix: "Win32 API vs SDL2 Alternative"
  - Current state: Which are already abstracted
  - Gaps: Which Win32 APIs bypass SDL2

**Deliverable**: `docs/SDL2_AUDIT_REPORT.md` (detailed findings)

---

### 06.1.2: Identify Win32 Leakage
**Effort**: 3 days  
**Owner**: Code Reviewer

**Search for direct Win32 API calls in these modules**:

1. **Input System** (`GameEngineDevice/Source/Win32Device/GameClient/`)
   - [ ] Win32DIKeyboard.cpp - Check for DirectInput calls that bypass SDL2
   - [ ] Win32DIMouse.cpp - Check for DirectInput calls that bypass SDL2
   - [ ] Win32Mouse.cpp - Check for Windows cursor APIs
   
2. **Window Management** (`Core/GameEngineDevice/` + `Main/`)
   - [ ] Win32GameEngine.cpp - Check for CreateWindow, etc
   - [ ] Win32OSDisplay.cpp - Check for raw Win32 display calls
   
3. **Message Loop** (`Main/WinMain.cpp`)
   - [ ] GetMessage, DispatchMessage - Should be SDL2 event loop
   - [ ] PeekMessage - Should be SDL_PollEvent
   
4. **File System** (Already abstracted, but verify)
   - [ ] Win32LocalFileSystem.cpp - Should only do file I/O, not window stuff
   
5. **Audio System** (To be replaced)
   - [ ] Current Miles Audio calls - Document for removal in 06.2

**Deliverable**: `docs/SDL2_LEAKAGE_INVENTORY.md`
- List of all Win32 calls found
- Severity (critical vs informational)
- Suggested SDL2 replacement

---

### 06.1.3: Create SDL2 Wrapper Functions (if gaps exist)
**Effort**: 2-3 days (if needed)  
**Owner**: SDL2 Specialist

**For each Win32 leakage found, create SDL2 wrapper**:

Example: If raw `GetCursorPos()` found:
```cpp
// GameEngineDevice/Include/SDL2Device/Common/SDL2Input.h
void SDL2_GetCursorPosition(int* outX, int* outY);

// GameEngineDevice/Source/SDL2Device/Common/SDL2Input.cpp
void SDL2_GetCursorPosition(int* outX, int* outY) {
    SDL_GetMouseState(outX, outY);
}
```

**Create wrapper headers for**:
- [ ] SDL2Input.h - Keyboard/Mouse input
- [ ] SDL2Window.h - Window management  
- [ ] SDL2Display.h - Display/cursor control
- [ ] SDL2Time.h - Timing (if not already done)

**Deliverable**: Complete SDL2 wrapper layer for all identified gaps

---

### 06.1.4: Build & Validate
**Effort**: 1 day  
**Owner**: Build Engineer

**Tasks**:
- [ ] Clean build: `cmake --preset vc6 && cmake --build build/vc6`
- [ ] Verify compilation clean (0 errors)
- [ ] Run game: `GeneralsXZH.exe -win -noshellmap`
- [ ] Test scenarios:
  - [ ] Navigate main menu (keyboard + mouse)
  - [ ] Start skirmish game
  - [ ] Click units (mouse input works)
  - [ ] Press ESC to quit (keyboard works)

**Deliverable**: `logs/phase06_1_validation.log` - All tests pass ✅

---

### 06.1.5: Documentation & Validation Report
**Effort**: 1 day  
**Owner**: Documentation Lead

**Create**:
- [ ] `docs/SDL2_IMPLEMENTATION_COMPLETE.md`
  - Summary of audit findings
  - All Win32 leakage fixed
  - Validation results
  - Architecture diagram (Win32Device → SDL2 abstraction)

**Acceptance Criteria**:
- [x] Zero direct Win32 API calls in input/window code
- [x] All events routed through SDL2
- [x] Game runs identically to before audit
- [x] No regression in functionality

**Deliverable**: Documentation + passing validation tests

---

## 🎵 Phase 06.2: OpenAL Audio Foundation (Week 3-4)

### Goal
Replace Miles Audio with OpenAL abstraction layer. Create clean `AudioDevice` interface that game code uses.

---

### 06.2.1: Design AudioDevice Abstraction
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

### 06.2.2: Research OpenAL Integration
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
  - [ ] `docs/OPENAL_INTEGRATION_SPEC.md`
  - ALContext initialization strategy
  - Thread safety model
  - Error handling approach

**Deliverable**: OPENAL_INTEGRATION_SPEC.md + compatibility verified

---

### 06.2.3: Implement OpenALDevice
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

### 06.2.4: Hook Game Audio Calls
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

### 06.2.5: Audio File Asset Audit
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

### 06.2.6: Build, Test, Validate
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
GeneralsXZH.exe -win -noshellmap 2>&1 | tee logs/audio_test.log

# Listen for:
# - OpenAL initialization success
# - Music playback
# - SFX playback
# - No errors logged
```

**Deliverable**: QA Report - All tests passed ✅

---

### 06.2.7: Documentation & Migration Guide
**Effort**: 1 day  
**Owner**: Documentation Lead

**Create**:
- [ ] `docs/OPENAL_IMPLEMENTATION_COMPLETE.md`
  - Architecture overview
  - AudioDevice interface documentation
  - Usage examples
  - Known limitations/future improvements

- [ ] `docs/MIGRATION_FROM_MILES_TO_OPENAL.md`
  - Miles API → OpenAL mapping
  - Code change examples
  - Troubleshooting guide

- [ ] `docs/AUDIO_FILE_FORMAT_SPEC.md`
  - Which formats supported
  - File location conventions
  - Performance characteristics

**Deliverable**: Complete audio documentation + migration guide

---

## 📅 Phase 06 Timeline Summary

| Phase | Duration | Deliverable |
|-------|----------|-------------|
| **06.1.1** | 2 days | SDL2 integration map |
| **06.1.2** | 3 days | Win32 leakage inventory |
| **06.1.3** | 2-3 days | SDL2 wrapper functions |
| **06.1.4** | 1 day | Validation tests pass |
| **06.1.5** | 1 day | Documentation + Report |
| **Subtotal 06.1** | **~10 days** | **SDL2 100% validated** ✅ |
| | | |
| **06.2.1** | 2 days | AudioDevice interface |
| **06.2.2** | 2 days | OpenAL spec + research |
| **06.2.3** | 4-5 days | OpenALDevice implementation |
| **06.2.4** | 2-3 days | Hook game audio calls |
| **06.2.5** | 2 days | Audio asset audit + convert |
| **06.2.6** | 2 days | Testing & QA |
| **06.2.7** | 1 day | Documentation |
| **Subtotal 06.2** | **~16 days** | **OpenAL fully integrated** ✅ |
| | | |
| **PHASE 06 TOTAL** | **~26 days** | **SDL2 + OpenAL complete** ✅ |

---

## ✅ Phase 06 Success Criteria

### SDL2 Audit (06.1)
- [x] Zero Win32 API calls in input/window management
- [x] All windowing/input events routed through SDL2
- [x] Game runs identically with no regression
- [x] Code audit documented and reviewed

### OpenAL Implementation (06.2)
- [x] AudioDevice abstraction fully designed and implemented
- [x] OpenAL backend functional (all major audio systems working)
- [x] Miles Audio completely replaced
- [x] All game audio tested and validated
- [x] Documentation complete

### Overall Phase 06
- [x] Game fully functional on SDL2 + OpenAL
- [x] Ready for Vulkan graphics phase (06.3)
- [x] Ready for Wine testing and validation
- [x] Zero technical debt related to audio/window management

---

## 🚀 Next Phase (06.3): Vulkan Graphics

Once 06.1 + 06.2 complete:
- AudioDevice pattern proven → GraphicsDevice will follow same architecture
- SDL2 abstraction validated → Confident in window/input layer
- Game runs perfectly → Can focus 100% on graphics abstraction

**Estimated start**: Early February 2026

---

## 📚 Reference Materials

- **OpenAL Documentation**: `docs/Support/OpenAL/` (if available)
- **SDL2 Docs**: [libsdl.org](https://wiki.libsdl.org/)
- **ioquake3 Audio**: Reference for 3D audio patterns
- **Previous Phases**: Phase 05 documentation (SDL2 integration baseline)

---

**Prepared by**: GeneralsX Development Team  
**Date**: 2026-01-15  
**Status**: Ready for Phase 06 Kickoff
