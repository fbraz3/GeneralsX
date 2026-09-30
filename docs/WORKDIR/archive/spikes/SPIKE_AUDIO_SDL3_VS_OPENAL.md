# Audio Backend Spike: SDL3 Audio vs OpenAL

**Date**: 2026-02-21  
**Session**: 53  
**Question**: Should Phase 2 use SDL3's built-in audio system or OpenAL?

---

## What Was Investigated

### Fighter19's Complete OpenAL Implementation
- `OpenALAudioManager.cpp` — **3,098 lines**, fully working on Linux (Zero Hour)
- `OpenALAudioCache.cpp` — **301 lines**, FFmpeg-based audio file decoder
- `OpenALAudioStream.cpp` — **91 lines**, streaming buffer system (for music)

**Fighter19 audio stack**:
```
Game AudioManager interface (AudioEventRTS, AudioRequest, etc.)
          ↓
    OpenALAudioManager      ← event scheduling, priority, volume, 3D
          ↓
  OpenALAudioFileCache      ← FFmpeg decodes MP3/WAV/OGG → PCM → AL buffer
          ↓
  OpenAL (libopenal-dev)    ← PCM playback, 3D positioning, listener
```

### SDL3 Audio Capabilities (from SDL3 wiki)
- `SDL_AudioStream` — mixing, format conversion, resampling
- `SDL_LoadWAV` — loads `.wav` files only (no MP3/OGG natively)
- `SDL_OpenAudioDevice` + `SDL_BindAudioStream` — outputs PCM to device
- No 3D spatial audio of any kind

### Our Current Stub
- `OpenALAudioManager.cpp` — 643 lines (device/context lifecycle, all event methods stubbed)
- No file decoding, no event processing, no 3D wiring, no streaming

---

## SDL3 Audio — What It Would Take

| Requirement | SDL3 Audio Status |
|-------------|------------------|
| WAV playback | ✅ Built-in (`SDL_LoadWAV`) |
| MP3 playback | ❌ Needs external decoder (dr_mp3, FFmpeg, or SDL3_mixer) |
| 3D positional audio | ❌ Not supported at any level |
| Mixing multiple sources | ⚠️ SDL3 audio mixes streams, but no priority/handle system |
| AudioManager interface integration | ❌ Would need full from-scratch integration (~3000+ lines) |
| Reference implementation | ❌ Zero reference for this codebase |

**Additional SDL3_mixer** (`libsdl3-mixer`) could add MP3/OGG support and basic mixing, but:
- Still no 3D audio
- Still a new dependency
- Still no reference implementation

**Total extra work**: Write ~3,000+ lines of new integration from scratch, then fake 3D audio on top of a 2D mixer. Result would be worse than OpenAL and cost more effort.

---

## OpenAL — What It Would Take

| Requirement | OpenAL Status |
|-------------|--------------|
| WAV/MP3 playback | ✅ Via FFmpeg (fighter19's `OpenALAudioFileCache`) |
| 3D positional audio | ✅ Native (`alSource3f AL_POSITION`, `alListenerfv`) |
| Streaming music | ✅ `OpenALAudioStream` (streaming buffer queue) |
| AudioManager interface | ✅ Fighter19 already mapped all 60+ virtual methods |
| Reference implementation | ✅ 3,490 lines of working Zero Hour code |

**Additional dependencies**: `libopenal-dev` + `libavcodec-dev` (FFmpeg)  
→ Both are standard packages on any modern Linux distro. Already referenced in our CMake.

**Total extra work**: Port fighter19's 3 files to our codebase. Adapt includes/namespaces. Test.

---

## Decision: OpenAL Wins

**SDL3 audio is disqualified for three reasons:**

1. **No 3D audio.** C&C ZH is an RTS. Explosions attenuate with distance. Units have positional voices. The `AudioManager` interface has `setDeviceListenerPosition()`, 3D source positions, rolloff factors. SDL3 audio simply cannot express these concepts — you'd have to build a fake distance attenuation layer on top of a 2D mixer, which is more work than just using OpenAL.

2. **No reference implementation.** Fighter19 has 3,490 lines of working Zero Hour OpenAL code. SDL3 audio has zero. We would be writing everything from scratch with no guide.

3. **Still needs FFmpeg anyway.** The game's audio files are MP3/WAV (some potentially Miles-compressed). SDL3 can't decode them natively. You'd add `dr_mp3` or FFmpeg or SDL3_mixer — same dep count as OpenAL + FFmpeg, with worse capabilities.

**SDL3 audio would make sense for a greenfield project with simple 2D audio needs.** This is not that.

---

## Implementation Plan (OpenAL)

Port fighter19's three files in order:

### Step 1: `OpenALAudioFileCache` (FFmpeg decoder)
- Port `OpenALAudioCache.cpp/h` from fighter19
- Adds: `FFmpegFile` wrapper, buffer cache, `decodeFFmpeg()`
- CMake: `find_package(FFmpeg)` in `cmake/audio.cmake`
- **Required before any sound works**

### Step 2: `OpenALAudioStream` (music streaming)
- Already done in fighter19 (91 lines, minimal changes needed)
- Port `OpenALAudioStream.cpp/h`
- **Required for background music**

### Step 3: Full `OpenALAudioManager` (event system)
- Port fighter19's 3,098-line implementation over our 643-line stub
- Key systems: `playAudioEvent`, `stopAudioEvent`, source pool, 3D listener, priority/fading
- **This is the bulk of Phase 2 work**

### Step 4: CMake + Docker
```cmake
find_package(FFmpeg REQUIRED COMPONENTS avcodec avformat avutil swresample)
find_package(OpenAL REQUIRED)
```
```dockerfile
RUN apt install -y libopenal-dev libavcodec-dev libavformat-dev libavutil-dev libswresample-dev
```

---

## Spike Conclusion

**Recommendation**: Proceed with OpenAL + FFmpeg as specified in PHASE02. Skip SDL3 audio.  
**Estimated Phase 2 duration**: 2–3 sessions (port + integration + testing).  
**Risk**: Low — fighter19's implementation already works in production for Zero Hour on Linux.
