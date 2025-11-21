# Audio Backend Status

## Current State (Phase 32 - October 19, 2025)

### ✅ Working Components

1. **Asset Loading**
   - Music.big and MusicZH.big load successfully
   - AudioEnglishZH.big and AudioZH.big load successfully
   - VFS layer functioning correctly

2. **INI Parsing**
   - 69 music tracks parsed from Music.ini
   - AudioSettings.ini processed (with minor field parser errors)
   - All MusicTrack definitions registered in hash system

3. **Audio Event System**
   - AudioEventRTS creation works
   - Event registration returns valid handles (e.g., handle=6 for Shell)
   - Event lifecycle management fixed (static pointer pattern prevents premature deletion)
   - AudioManager::addAudioEvent() routes events correctly

4. **Filename Resolution Pattern**
   - Discovered: ALL music tracks have empty `filename=''` after INI parsing
   - This is **architectural design**, not a bug
   - Filename resolution happens at runtime via:
     - `generateFilenamePrefix(AT_Music)` - generates prefix path
     - `m_eventInfo->m_filename` concatenation (currently empty)
     - `adjustForLocalization()` - adds language/region modifiers
     - VFS lookup resolves symbolic names to actual .mp3 files in .big archives

### ❌ Missing Implementation: OpenAL Audio Backend

**Problem**: Current OpenAL implementation is a **stub placeholder** with no actual audio functionality.

**File**: `Core/GameEngineDevice/Source/OpenALDevice/OpenALAudioManager.cpp`

**What's missing**:

```cpp
// Current stub implementation
void OpenALAudioManager::init() {
    std::cerr << "OpenALAudioManager::init() - Stub implementation" << std::endl;
    m_initialized = true;
    // ❌ No OpenAL device initialization
    // ❌ No context creation
    // ❌ No source/buffer allocation
}

void OpenALAudioManager::update() {
    // ❌ Empty - doesn't process audio events
}

void OpenALAudioManager::friend_forcePlayAudioEventRTS(const AudioEventRTS* eventToPlay) {
    // ❌ Empty - doesn't play anything
}

Bool OpenALAudioManager::isMusicPlaying(void) const {
    return false;  // ❌ Always returns false
}
```

**Impact**:
- No audio device opened (no `alcOpenDevice()`)
- No OpenAL context created (no `alcCreateContext()`)
- No audio buffers allocated (no `alGenBuffers()`)
- No sources for playback (no `alGenSources()`)
- AudioEventRTS objects created but never played
- Music files never loaded from .big archives
- No actual sound output

### 🎯 Solution: Port Complete OpenAL Implementation

**Reference**: `references/jmarshall-win64-modern/Code/GameEngineDevice/Source/OpenALAudioDevice/`

**Files to port**:

1. **OpenALAudioManager.cpp** (1,486 lines)
   - Real `init()`: Opens device, creates context, allocates sources
   - Real `update()`: Processes playing audio, checks completion, manages queue
   - Real `friend_forcePlayAudioEventRTS()`: Loads and plays audio files
   - Music playback: Dedicated `m_musicSource` for streaming
   - Source pooling: 2D/3D source pools for simultaneous sounds
   - Volume/positioning: Real 3D audio calculations

2. **OpenALAudioLoader.cpp** (if exists)
   - File loading from VFS
   - Format decoding (.mp3, .wav, etc.)
   - Buffer management

3. **OpenALPlayingAudio.h/cpp**
   - Tracks active audio instances
   - Links AudioEventRTS to OpenAL sources/buffers
   - State management (playing, paused, stopped)

**Key implementation details from reference**:

```cpp
// Real initialization
void OpenALAudioManager::init() {
    AudioManager::init();
    m_selectedSpeakerType = TheAudio->translateSpeakerTypeToUnsignedInt(m_prefSpeaker);
    openDevice();  // ✅ Opens real OpenAL device
    TheAudio->refreshCachedVariables();
}

void OpenALAudioManager::openDevice() {
    device = alcOpenDevice(NULL);  // ✅ Real OpenAL device
    context = alcCreateContext(device, NULL);  // ✅ Real context
    alcMakeContextCurrent(context);
    
    // ✅ Allocate source pools
    alGenSources(getNum2DSamples(), m_sourcePool2D);
    alGenSources(getNum3DSamples(), m_sourcePool3D);
    alGenSources(1, &m_musicSource);  // ✅ Dedicated music source
}

// Real update loop
void OpenALAudioManager::update() {
    // ✅ Process playing sounds
    // ✅ Check completion states
    // ✅ Update 3D positions
    // ✅ Handle fade in/out
}
```

**Platform considerations**:

- **macOS**: OpenAL framework available (`-framework OpenAL`)
- **Linux**: Requires OpenAL Soft (`libopenal-dev`)
- **Windows**: Already uses Miles Sound System (different backend)

**CMake changes needed**:

```cmake
# cmake/openal.cmake
if(APPLE)
    find_library(OPENAL_LIBRARY OpenAL REQUIRED)
    target_link_libraries(OpenALDevice PRIVATE ${OPENAL_LIBRARY})
elseif(UNIX)
    find_package(OpenAL REQUIRED)
    target_link_libraries(OpenALDevice PRIVATE OpenAL::OpenAL)
endif()
```

## Test Results (Manual Audio Test - October 19, 2025)

**Log file**: `logs/manual_audio_test.log`

**Execution**:
```bash
cd $HOME/GeneralsX/GeneralsMD
USE_METAL=1 ./GeneralsXZH 2>&1 | tee logs/manual_audio_test.log
```

**Observed behavior**:
1. ✅ Music.big/MusicZH.big loaded successfully
2. ✅ 69 music tracks parsed from INI
3. ✅ Shell music event created (handle=6)
4. ✅ Event kept alive (lifecycle fix working)
5. ❌ **NO AUDIO OUTPUT** - OpenAL stub doesn't play anything
6. Game runs normally otherwise (graphics, menus, etc.)

**Confirmation**:
```
GameEngine::init() - TEST: Attempting to start 'Shell' menu music...
AudioManager::addAudioEvent() - Adding MUSIC track: 'Shell' (handle=6, filename='\\')
GameEngine::init() - TEST: Shell music addAudioEvent returned handle=6 (event kept alive)
```

Event created successfully but OpenAL stub never plays it.

## Implementation Priority

**Phase 35 (proposed)**: OpenAL Backend Implementation

**Effort estimate**: 3-5 hours
- Port OpenALAudioManager from jmarshall reference (~1,500 lines)
- Port OpenALPlayingAudio helper classes
- Port OpenALAudioLoader for file decoding
- Update CMake for OpenAL linking
- Test music playback
- Test sound effects (2D/3D)
- Verify source pooling

**Risk**: LOW
- Reference implementation already proven on Windows 64-bit
- OpenAL API standardized across platforms
- Can keep stub as fallback if needed

**Benefit**: HIGH
- Enables actual audio playback
- Unlocks music system testing
- Required for complete game experience
- Foundation for sound effects implementation

## Related Documentation

- `docs/DEV_BLOG/YYYY-MM-DIARY.md (organized by month)` - Main porting progress
- `docs/PLANNING/3/PHASE32/AUDIO_PIPELINE_INVESTIGATION.md` - Current phase work
- `docs/Misc/BIG_FILES_REFERENCE.md` - Asset structure (Music.big contents)
- `references/jmarshall-win64-modern/` - Proven OpenAL implementation source

## Next Steps

1. **Decision**: Port full OpenAL implementation vs. keep stub
2. **If porting**: Create Phase 35 documentation
3. **If porting**: Copy OpenALAudioManager.cpp from reference
4. **If porting**: Adapt for macOS (framework linking, path differences)
5. **If porting**: Test with Shell music
6. **If porting**: Verify source pooling with multiple sounds
7. **Alternative**: Document as known limitation, focus on graphics/gameplay

---

**Last Updated**: October 19, 2025
**Status**: OpenAL stub confirmed as root cause of no audio output
**Resolution**: Requires Phase 35 implementation or documented limitation
