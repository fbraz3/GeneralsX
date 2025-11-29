# Phase 63: Audio System - OpenAL Integration

**Phase**: 63
**Title**: OpenAL Audio Backend Implementation
**Duration**: 5-7 days
**Status**: NOT STARTED
**Dependencies**: Phase 60+ (Game initializing)

---

## Overview

Phase 63 implements audio support using OpenAL as a cross-platform replacement for DirectSound/Miles Sound System. Audio is essential for gameplay feedback and immersion.

### Current State

- ✅ Game renders visually
- ❌ No audio output
- ❌ Sound effects not playing
- ❌ Music not playing
- ❌ 3D positional audio not working

### Goal

Full audio support with music, sound effects, UI sounds, and 3D positional audio.

---

## Audio System Overview

### Original Miles Sound System

The game uses Miles Sound System (MSS) for audio:

```
MilesAudioDevice
├── Music playback (MP3/streaming)
├── 2D sound effects (UI, voice)
├── 3D positional audio (units, explosions)
└── Audio mixing
```

### OpenAL Replacement

```
OpenALAudioDevice
├── ALDevice (audio device)
├── ALContext (audio context)
├── ALSources (sound emitters)
├── ALBuffers (audio data)
└── ALListeners (camera/player)
```

---

## Technical Requirements

### 1. OpenAL Initialization

```cpp
bool OpenALAudioDevice::Initialize()
{
    // Open default device
    m_device = alcOpenDevice(nullptr);
    if (!m_device) return false;
    
    // Create context
    m_context = alcCreateContext(m_device, nullptr);
    if (!m_context) return false;
    
    alcMakeContextCurrent(m_context);
    
    // Set listener at origin
    alListener3f(AL_POSITION, 0, 0, 0);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    float orientation[] = {0, 0, -1, 0, 1, 0};
    alListenerfv(AL_ORIENTATION, orientation);
    
    return true;
}
```

### 2. Audio Buffer Loading

```cpp
uint32_t OpenALAudioDevice::LoadSound(const void* data, size_t size, AudioFormat format)
{
    ALuint buffer;
    alGenBuffers(1, &buffer);
    
    ALenum al_format;
    switch (format) {
        case AUDIO_MONO8: al_format = AL_FORMAT_MONO8; break;
        case AUDIO_MONO16: al_format = AL_FORMAT_MONO16; break;
        case AUDIO_STEREO8: al_format = AL_FORMAT_STEREO8; break;
        case AUDIO_STEREO16: al_format = AL_FORMAT_STEREO16; break;
    }
    
    alBufferData(buffer, al_format, data, size, sample_rate);
    
    return buffer;
}
```

### 3. Sound Playback

```cpp
uint32_t OpenALAudioDevice::PlaySound(uint32_t buffer, bool loop)
{
    ALuint source;
    alGenSources(1, &source);
    
    alSourcei(source, AL_BUFFER, buffer);
    alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    alSourcef(source, AL_GAIN, 1.0f);
    
    alSourcePlay(source);
    
    return source;
}
```

### 4. 3D Positional Audio

```cpp
void OpenALAudioDevice::SetSourcePosition(uint32_t source, float x, float y, float z)
{
    alSource3f(source, AL_POSITION, x, y, z);
}

void OpenALAudioDevice::SetListenerPosition(float x, float y, float z)
{
    alListener3f(AL_POSITION, x, y, z);
}

void OpenALAudioDevice::SetListenerOrientation(float fx, float fy, float fz, float ux, float uy, float uz)
{
    float orientation[] = {fx, fy, fz, ux, uy, uz};
    alListenerfv(AL_ORIENTATION, orientation);
}
```

---

## Implementation Plan

### Day 1-2: OpenAL Setup

1. Add OpenAL dependency to CMake
2. Create OpenALAudioDevice class
3. Implement Initialize/Shutdown
4. Test basic audio output

### Day 3-4: Sound Loading

1. Parse WAV files from .big archives
2. Load audio data into OpenAL buffers
3. Implement buffer management
4. Test sound effect playback

### Day 5: Music Streaming

1. Implement MP3/OGG streaming
2. Background music playback
3. Crossfade between tracks
4. Volume control

### Day 6: 3D Audio

1. Implement source positioning
2. Implement listener tracking
3. Attenuation model
4. Test with unit sounds

### Day 7: Integration

1. Hook into game audio calls
2. Replace Miles API calls
3. Test all audio scenarios
4. Performance optimization

---

## File Structure

```
Core/GameEngineDevice/Source/OpenALAudioDevice/
├── OpenALAudioDevice.h
├── OpenALAudioDevice.cpp
├── OpenALBuffer.h
├── OpenALBuffer.cpp
├── OpenALSource.h
├── OpenALSource.cpp
└── OpenALStreaming.cpp
```

---

## CMake Integration

```cmake
# cmake/openal.cmake
find_package(OpenAL REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenAL::OpenAL)
```

Or use vcpkg:

```json
{
  "dependencies": [
    "openal-soft"
  ]
}
```

---

## Reference Implementation

The jmarshall-win64-modern reference has a complete OpenAL implementation:

```bash
references/jmarshall-win64-modern/Code/GameEngineDevice/Source/OpenALAudioDevice/
```

Use this as the primary reference for implementation.

---

## Testing Strategy

### Test 1: Basic Sound

Play simple WAV sound effect.

### Test 2: Music

Play background music, verify streaming.

### Test 3: UI Sounds

Verify button clicks produce sound.

### Test 4: 3D Audio

Position sound in world, verify it pans with camera.

### Test 5: Volume Control

Adjust master volume, verify change.

### Test 6: Multiple Sounds

Play multiple sounds simultaneously.

---

## Success Criteria

- [ ] OpenAL initializes successfully
- [ ] WAV sounds load and play
- [ ] Music streams without stutter
- [ ] UI sounds work
- [ ] 3D positional audio works
- [ ] Volume controls function
- [ ] No audio glitches or pops
- [ ] Performance acceptable (<5% CPU)

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| Audio format issues | Medium | Support common formats |
| Streaming stutter | Medium | Buffer ahead |
| 3D audio incorrect | Low | Test with known positions |
| Performance | Medium | Limit concurrent sources |

---

**Created**: November 28, 2025
**Author**: GeneralsX Development Team
