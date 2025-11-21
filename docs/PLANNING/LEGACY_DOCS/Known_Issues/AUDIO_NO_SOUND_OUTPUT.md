# Known Issue: Audio System - No Sound Output Despite Successful Playback

**Severity**: MEDIUM  
**Component**: OpenAL Audio Backend (Phase 33)  
**Platform**: macOS ARM64 (Apple Silicon)  
**Discovered**: October 21, 2025

## Summary

OpenAL audio system reports successful playback initialization and execution (AL_PLAYING state confirmed), but no audible sound is produced from the device speakers.

## Technical Details

### Symptoms

1. **OpenAL initialization successful**:
   - Device opens: "Alto-falantes (MacBook Pro)"
   - Context created and activated
   - Source pools allocated (32 2D, 128 3D, 1 music)
   - No OpenAL errors reported

2. **Audio file loading successful**:
   - MP3 file loaded from VFS: `USA_11.mp3` (4,581,567 bytes)
   - MP3 decoding successful: buffer 2561 created
   - File cached in OpenALAudioLoader

3. **Playback execution successful**:
   - `alSourcePlay()` called successfully
   - Source state verified: **AL_PLAYING (4114)**
   - Source configured: volume=1.0, looping=YES
   - Audio added to streaming list

4. **No audible output**:
   - Speakers produce no sound
   - System volume confirmed not muted
   - Other applications produce sound normally

### Diagnostic Logs

```
OpenALAudioManager::openDevice() - Available OpenAL devices:
OpenALAudioManager::openDevice() - Device opened successfully: 'Alto-falantes (MacBook Pro)'
OpenALAudioManager::openDevice() - Context created successfully
OpenALAudioManager::openDevice() - Generated 32 2D sources
OpenALAudioManager::openDevice() - Generated 128 3D sources
OpenALAudioManager::openDevice() - Generated music source

GameEngine::init() - Audio settings: audioOn=1, musicOn=1, soundsOn=1, sounds3DOn=1, speechOn=1

OpenALAudioManager::playSample() - Event: 'Shell', isMusic: 1
OpenALAudioManager::playSample() - Filename: 'Data\Audio\Tracks\USA_11.mp3'
OpenALAudioLoader: Loading 'Data\Audio\Tracks\USA_11.mp3' from VFS
OpenALAudioLoader: Loaded 4581567 bytes from 'Data\Audio\Tracks\USA_11.mp3'
OpenALAudioLoader: Decoding MP3 file
OpenALAudioLoader: Successfully loaded and cached buffer 2561
OpenALAudioManager::playSample() - Using dedicated music source
OpenALAudioManager::playSample() - Source configured: volume=1, looping=YES
OpenALAudioManager::playSample() - alSourcePlay() called, state=AL_PLAYING (4114)
OpenALAudioManager::playSample() - Added to streams list (music)
OpenALAudioManager::playSample() - Playback started successfully
```

### System Configuration

- **OS**: macOS 14.x+ (Apple Silicon)
- **Device**: MacBook Pro with built-in speakers
- **OpenAL**: System framework (native macOS)
- **Audio Output**: "Alto-falantes (MacBook Pro)" (internal speakers)

## Potential Root Causes

### 1. MP3 Decoding Issue (MOST LIKELY)

**Hypothesis**: MP3 decoder may be producing invalid PCM data or incorrect format.

**Evidence**:
- Buffer created successfully (no errors)
- State shows AL_PLAYING
- But no audio output

**Investigation needed**:
```cpp
// Verify decoded data in OpenALAudioLoader::decodeMp3()
std::cerr << "Decoded PCM: channels=" << channels 
          << ", sampleRate=" << sampleRate 
          << ", dataSize=" << decodedSize << std::endl;

// Check ALenum format matches decoded data
std::cerr << "OpenAL format: " << format 
          << " (AL_FORMAT_MONO16=" << AL_FORMAT_MONO16 
          << ", AL_FORMAT_STEREO16=" << AL_FORMAT_STEREO16 << ")" << std::endl;
```

**Test**:
- Try loading a WAV file instead of MP3 (eliminates codec as variable)
- Verify decoded PCM data is not all zeros
- Check sample rate matches device expectations

### 2. Buffer Data Upload Issue

**Hypothesis**: `alBufferData()` may be receiving invalid parameters.

**Investigation needed**:
```cpp
// In OpenALAudioLoader after decoding
alBufferData(buffer, format, pcmData, pcmDataSize, sampleRate);
ALenum error = alGetError();
if (error != AL_NO_ERROR) {
    std::cerr << "ERROR: alBufferData failed: " << error << std::endl;
}

// Verify buffer properties
ALint size, frequency, bits, channels;
alGetBufferi(buffer, AL_SIZE, &size);
alGetBufferi(buffer, AL_FREQUENCY, &frequency);
alGetBufferi(buffer, AL_BITS, &bits);
alGetBufferi(buffer, AL_CHANNELS, &channels);
std::cerr << "Buffer properties: size=" << size 
          << ", freq=" << frequency 
          << ", bits=" << bits 
          << ", channels=" << channels << std::endl;
```

### 3. Listener Configuration

**Hypothesis**: OpenAL listener may not be positioned correctly.

**Investigation needed**:
```cpp
// Check listener state
ALfloat listenerPos[3], listenerOri[6];
alGetListenerfv(AL_POSITION, listenerPos);
alGetListenerfv(AL_ORIENTATION, listenerOri);
std::cerr << "Listener: pos=(" << listenerPos[0] << "," << listenerPos[1] << "," << listenerPos[2] << ")" << std::endl;
```

### 4. macOS Audio Permissions

**Hypothesis**: Application may lack microphone/audio permissions (unlikely since device opens).

**Test**:
- Check System Settings > Privacy & Security > Microphone
- Verify Terminal.app has audio permissions
- Try running from different terminal (iTerm2, etc.)

### 5. OpenAL Framework Version

**Hypothesis**: macOS system OpenAL framework may have compatibility issues.

**Investigation needed**:
```cpp
// Query OpenAL version
const ALCchar* version = alcGetString(device, ALC_EXTENSIONS);
std::cerr << "OpenAL extensions: " << (version ? version : "none") << std::endl;
```

**Alternative**:
- Try OpenAL Soft (open-source implementation) via Homebrew:
  ```bash
  brew install openal-soft
  # Update CMake to link against OpenAL Soft instead of system framework
  ```

## Workarounds

None currently available - audio functionality completely blocked.

## Reproduction Steps

1. Compile GeneralsXZH with Phase 33 OpenAL implementation
2. Deploy to `$HOME/GeneralsX/GeneralsMD/`
3. Run: `cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH`
4. Observe logs showing successful playback
5. Listen - no sound output

## Expected Behavior

Shell menu music (`USA_11.mp3`) should play audibly from MacBook Pro speakers when game initializes.

## Actual Behavior

Logs confirm OpenAL reports AL_PLAYING state, but no audible sound is produced.

## Impact

- **Game Experience**: No background music, sound effects, or voice lines
- **Testing**: Cannot validate audio gameplay features
- **User Experience**: Silent game reduces immersion significantly

## Next Steps

1. **Add PCM data validation logging** to verify decoded audio is valid
2. **Test with WAV file** to eliminate MP3 codec as variable
3. **Verify buffer upload** parameters match decoded format
4. **Check listener configuration** for positioning issues
5. **Test OpenAL Soft** as alternative to system framework
6. **Compare with reference implementation** byte-by-byte for differences

## Files Involved

- `Core/GameEngineDevice/Source/OpenALDevice/OpenALAudioLoader.cpp` (MP3 decoding)
- `Core/GameEngineDevice/Source/OpenALDevice/OpenALAudioManager.cpp` (playback control)
- `cmake/openal.cmake` (OpenAL framework linking)

## Related Issues

None - first occurrence of this specific symptom.

## References

- Test log: `/tmp/audio_debug_test.txt`
- OpenAL specification: https://www.openal.org/documentation/
- macOS OpenAL framework: `/System/Library/Frameworks/OpenAL.framework/`

## Status

**OPEN** - Investigation required, blocked pending detailed PCM data validation.

---

**Created**: October 21, 2025  
**Last Updated**: October 21, 2025  
**Assigned To**: Pending investigation
