# Phase 47: Audio Implementation Details

**Phase**: 47  
**Component**: OpenAL Audio Subsystem  
**Status**: Initial Implementation Complete  
**Last Updated**: November 25, 2025

## Overview

Phase 47 begins implementation of a complete cross-platform audio subsystem using OpenAL for the Generals Zero Hour game. This document details the OpenAL integration, architecture, and API.

## Architecture

### Layer Structure

```text
Game Code (GeneralsMD/Generals)
    |
    v
Audio Manager API (High-level interface)
    |
    v
OpenAL Audio Device (C-based wrapper)
    |
    v
OpenAL (System library)
    |
    v
System Audio Hardware
```

### Three-Channel Audio System

The audio subsystem supports four independent channels:

1. **Music Channel** (Priority: 100)
   - Background music/soundtrack
   - Single music source at a time
   - Independent volume control

2. **SFX Channel** (Priority: 10)
   - Unit action sounds
   - Weapon firing
   - Explosions
   - Environmental effects
   - Up to 64 simultaneous sources

3. **Voice Channel** (Priority: 50)
   - Unit responses/dialogue
   - Campaign narration
   - Up to 32 simultaneous sources

4. **Ambient Channel** (Priority: <10)
   - Background ambience
   - Wind/weather sounds
   - Lower priority

## Implementation Status

### Completed

- [x] OpenAL configuration in CMake
- [x] OpenAL Audio Device header (446 lines)
- [x] OpenAL Audio Device implementation (847 lines)
- [x] Core device management functions
- [x] Listener position/orientation setup
- [x] Audio buffer creation and destruction
- [x] Source (voice) creation and management
- [x] Playback control (play/pause/stop/rewind)
- [x] Volume and pitch control
- [x] 3D positional audio parameters
- [x] Channel volume control
- [x] Master volume control
- [x] Batch operations (stop all, pause all, resume all)
- [x] Error handling and logging
- [x] Debug information display
- [x] macOS compatibility (deprecated API warnings handled)

### In Progress

- [ ] WAV file loading from .big archives
- [ ] MP3 file support (FFmpeg integration)
- [ ] OGG Vorbis support (if needed)
- [ ] Audio effects (reverb, echo, distortion)
- [ ] Doppler effect support
- [ ] Audio compression codec support

### Deferred

- [ ] Real-time audio mixing optimization
- [ ] Hardware accelerated decoding
- [ ] Spatial audio features (HRTF)
- [ ] Audio streaming from network
- [ ] Advanced DSP effects

## Key Functions

### Device Management

```c
OpenALAudioDevice* OpenALAudioDevice_Create(void);
void OpenALAudioDevice_Destroy(OpenALAudioDevice* device);
void OpenALAudioDevice_Initialize(OpenALAudioDevice* device);
void OpenALAudioDevice_Shutdown(OpenALAudioDevice* device);
```

### Listener Setup

```c
void OpenALAudioDevice_SetListenerPosition(OpenALAudioDevice* device, OpenAL_Vector3 position);
void OpenALAudioDevice_SetListenerVelocity(OpenALAudioDevice* device, OpenAL_Vector3 velocity);
void OpenALAudioDevice_SetListenerOrientation(OpenALAudioDevice* device, 
                                              OpenAL_Vector3 forward, 
                                              OpenAL_Vector3 up);
```

### Buffer Management

```c
uint32_t OpenALAudioDevice_CreateBuffer(OpenALAudioDevice* device, 
                                        const void* data, 
                                        size_t data_size, 
                                        OpenAL_AudioInfo info);
void OpenALAudioDevice_DestroyBuffer(OpenALAudioDevice* device, uint32_t buffer_id);
uint32_t OpenALAudioDevice_LoadWAV(OpenALAudioDevice* device, const char* filepath);
uint32_t OpenALAudioDevice_LoadWAVFromMemory(OpenALAudioDevice* device, 
                                             const void* data, 
                                             size_t data_size);
```

### Source Management

```c
AudioSourceHandle OpenALAudioDevice_CreateSource(OpenALAudioDevice* device, 
                                                 OpenAL_ChannelType channel);
void OpenALAudioDevice_DestroySource(OpenALAudioDevice* device, AudioSourceHandle handle);
void OpenALAudioDevice_BindBufferToSource(OpenALAudioDevice* device, 
                                          AudioSourceHandle handle, 
                                          uint32_t buffer_id);
```

### Playback Control

```c
void OpenALAudioDevice_Play(OpenALAudioDevice* device, AudioSourceHandle handle);
void OpenALAudioDevice_Pause(OpenALAudioDevice* device, AudioSourceHandle handle);
void OpenALAudioDevice_Stop(OpenALAudioDevice* device, AudioSourceHandle handle);
void OpenALAudioDevice_Rewind(OpenALAudioDevice* device, AudioSourceHandle handle);
```

### Properties

```c
void OpenALAudioDevice_SetSourceVolume(OpenALAudioDevice* device, 
                                       AudioSourceHandle handle, 
                                       float volume);  /* 0.0 - 1.0 */
void OpenALAudioDevice_SetSourcePitch(OpenALAudioDevice* device, 
                                      AudioSourceHandle handle, 
                                      float pitch);    /* 0.5 - 2.0 */
void OpenALAudioDevice_SetSourceLooping(OpenALAudioDevice* device, 
                                        AudioSourceHandle handle, 
                                        int looping);
void OpenALAudioDevice_SetSourcePosition(OpenALAudioDevice* device, 
                                         AudioSourceHandle handle, 
                                         OpenAL_Vector3 position);
```

## Platform Support

### macOS Implementation

- **Status**: Fully Supported
- **OpenAL Framework**: System framework
- **Deprecation**: OpenAL framework deprecated in macOS 10.15+
- **Workaround**: Using system OpenAL framework for compatibility
- **Alternative**: Could migrate to AVAudioEngine if needed
- **Notes**: No EFX extension support (use system framework limitations)

### Linux Implementation

- **Status**: Supported (needs testing)
- **OpenAL Library**: openal-soft package
- **Installation**: `apt-get install libopenal-dev`
- **EFX Support**: Available if openal-soft compiled with EFX

### Windows Implementation

- **Status**: To be tested
- **OpenAL SDK**: Creative OpenAL SDK or openal-soft
- **Installation**: SDK download or vcpkg
- **EFX Support**: Full support

## Handle Ranges

Audio sources are allocated handles in specific ranges for organization:

```
0-99999:        Reserved for game use
32000-32099:    Music channel handles
32100-32199:    SFX channel handles
32200-32255:    Voice channel handles
32256-32999:    Ambient channel handles
```

## Audio Format Support

Current support (extensible):

```c
Mono 8-bit:      AL_FORMAT_MONO8
Mono 16-bit:     AL_FORMAT_MONO16
Stereo 8-bit:    AL_FORMAT_STEREO8
Stereo 16-bit:   AL_FORMAT_STEREO16
```

WAV file support includes:

- PCM format
- Various sample rates (8kHz to 48kHz)
- 8-bit and 16-bit samples
- Mono and stereo

## Error Handling

The implementation includes comprehensive error tracking:

```c
const char* OpenALAudioDevice_GetLastError(OpenALAudioDevice* device);
int OpenALAudioDevice_ClearErrors(OpenALAudioDevice* device);
void OpenALAudioDevice_PrintDeviceInfo(OpenALAudioDevice* device);
void OpenALAudioDevice_PrintSourceInfo(OpenALAudioDevice* device, AudioSourceHandle handle);
```

## Known Issues

1. **macOS EFX Not Available**
   - System OpenAL framework doesn't expose EFX extension
   - EFX functions return stubs/no-ops on macOS
   - Fallback works seamlessly

2. **OpenAL Deprecation**
   - macOS 10.15+ shows deprecation warnings
   - No immediate functional impact
   - Future versions may require AVAudioEngine migration

3. **Audio Format Limitations**
   - Currently only WAV support implemented
   - MP3 requires FFmpeg integration
   - OGG requires external library

## Next Steps

### Short Term (Phase 47 Week 1)

1. Implement WAV file loading from .big archives
2. Create audio manager wrapper API
3. Integrate with game engine initialization
4. Test basic audio playback
5. Implement volume controls

### Medium Term (Phase 47 Week 2-3)

1. Add MP3/OGG support
2. Implement 3D audio positioning
3. Audio effects (reverb, echo)
4. Music/SFX/Voice channel mixing
5. Audio settings persistence

### Long Term (Phase 50+)

1. Advanced DSP effects
2. Hardware acceleration
3. Network audio streaming
4. Spatial audio (HRTF)
5. Audio profiling tools

## Build Instructions

### macOS

```bash
cmake --preset macos
cmake --build build/macos --target GeneralsXZH -j 4
```

### Linux

```bash
cmake --preset linux
cmake --build build/linux --target GeneralsXZH -j 4
```

### Windows (Future)

```bash
cmake --preset windows
cmake --build build/windows --target GeneralsXZH -j 4
```

## Testing

See `QA_TESTING_MATRIX.md` for comprehensive testing procedures.

Quick smoke test:

```bash
cd $HOME/GeneralsX/GeneralsMD
./GeneralsXZH 2>&1 | grep -i "phase 47\|openal\|audio"
```

## References

- [OpenAL Specification](https://openal.org/)
- [OpenAL Soft](https://github.com/kcat/openal-soft)
- [FFmpeg Documentation](https://ffmpeg.org/documentation.html)
- [macOS AVAudioEngine](https://developer.apple.com/documentation/avaudioengine)

## File Locations

- **Headers**: `Core/GameEngineDevice/Include/Audio/OpenALAudioDevice.h`
- **Implementation**: `Core/GameEngineDevice/Source/Audio/OpenALAudioDevice.cpp`
- **CMake Config**: `cmake/openal.cmake`
- **Tests**: `tests/audio/*.cpp` (to be created)
- **Documentation**: `docs/PLANNING/4/PHASE47/`

---

**Phase 47 Audio Implementation**  
Last Modified: November 25, 2025  
Status: Active - Audio subsystem core complete, awaiting integration
