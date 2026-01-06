# Phase 49.2: FFmpegVideoPlayer Factory Connection

## Overview

This document provides step-by-step implementation details for connecting the `FFmpegVideoPlayer` to the game's factory method.

## Problem Statement

The `createVideoPlayer()` method in `W3DGameClient.h` has an empty `#ifdef RTS_HAS_FFMPEG` block. Despite `FFmpegVideoPlayer` being fully implemented, it's never instantiated.

## Current Code

File: `GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DGameClient.h`

Lines ~108-113:

```cpp
#ifdef RTS_HAS_FFMPEG
#else
#endif
virtual VideoPlayerInterface *createVideoPlayer( void ) { return NULL; }
```

## Required Fix

```cpp
#ifdef RTS_HAS_FFMPEG
#include "VideoDevice/FFmpeg/FFmpegVideoPlayer.h"
virtual VideoPlayerInterface *createVideoPlayer( void ) { return NEW FFmpegVideoPlayer; }
#else
virtual VideoPlayerInterface *createVideoPlayer( void ) { return NULL; }
#endif
```

## Implementation Details

### Step 1: Locate the File

```bash
vim GeneralsMD/Code/GameEngineDevice/Include/W3DDevice/GameClient/W3DGameClient.h
```

### Step 2: Find the Code Block

Search for `createVideoPlayer` (around line 110-115).

### Step 3: Replace the Block

Replace:

```cpp
#ifdef RTS_HAS_FFMPEG
#else
#endif
virtual VideoPlayerInterface *createVideoPlayer( void ) { return NULL; }
```

With:

```cpp
#ifdef RTS_HAS_FFMPEG
#include "VideoDevice/FFmpeg/FFmpegVideoPlayer.h"
virtual VideoPlayerInterface *createVideoPlayer( void ) { return NEW FFmpegVideoPlayer; }
#else
virtual VideoPlayerInterface *createVideoPlayer( void ) { return NULL; }
#endif
```

## Verification Steps

### Step 1: Verify RTS_HAS_FFMPEG is Defined

Check `Core/GameEngineDevice/CMakeLists.txt`:

```bash
grep -n "RTS_HAS_FFMPEG" Core/GameEngineDevice/CMakeLists.txt
```

Expected output:

```
167:        target_compile_definitions(corei_gameenginedevice_public INTERFACE RTS_HAS_FFMPEG)
```

### Step 2: Verify FFmpegVideoPlayer Exists

```bash
ls -la Core/GameEngineDevice/Source/VideoDevice/FFmpeg/
```

Expected:

```
FFmpegFile.cpp
FFmpegVideoPlayer.cpp
```

### Step 3: Build

```bash
cmake --build build/macos --target GeneralsXZH -j 4 2>&1 | tee logs/phase49_2_build.log
```

### Step 4: Test Video Playback

```bash
cd $HOME/GeneralsX/GeneralsMD
timeout 60s ./GeneralsXZH 2>&1 | tee logs/phase49_2_video_test.log
grep -i "Video\|FFmpeg\|Bink" logs/phase49_2_video_test.log
```

## Expected Behavior

After this fix:

1. Game startup should show EA logo video
2. Game should proceed to main menu
3. Log should show video player initialization:

```
FFmpegVideoPlayer::init() - Initializing FFmpeg video player
FFmpegVideoPlayer::createStream() - About to open bink file
```

## Video Files Location

The game looks for videos in:

- `Data/Movies/*.bik` - Standard videos
- `Data/<Language>/Movies/*.bik` - Localized videos

Example video files:

- `EALogo.bik` - EA Games logo
- `NewGameIntro.bik` - Opening cinematic
- Various campaign mission videos

## Dependencies

- FFmpeg library must be linked (verified in vcpkg.json)
- RTS_HAS_FFMPEG must be defined (verified in CMakeLists.txt)
- Video .bik files must be present in Data/Movies/

## Notes

### Why FFmpeg Instead of Bink

The original game used RAD Game Tools' Bink video codec. For the cross-platform port:

1. Bink SDK is proprietary and Windows-only
2. FFmpeg is cross-platform and open source
3. FFmpegVideoPlayer can decode Bink format (via FFmpeg's bink decoder)

### Video Format Support

FFmpegVideoPlayer supports:

- Bink Video (.bik) - Original game format
- Most common video formats via FFmpeg

### Performance Considerations

- FFmpeg decoding is CPU-based
- Modern systems can easily handle 720p video playback
- Memory usage is minimal (~50MB during video playback)
