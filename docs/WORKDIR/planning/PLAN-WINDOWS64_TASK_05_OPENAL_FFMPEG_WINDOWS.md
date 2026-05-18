# Windows64 Task 05 - OpenAL and FFmpeg

## Scope

Make OpenAL and FFmpeg the functional media backends for Windows64.

## Files

- `cmake/openal.cmake`
- `cmake/FindFFmpeg.cmake`
- `Core/GameEngineDevice/Source/OpenALAudioDevice/*`
- Any video playback integration points

## Deliverable

- Windows64 modern builds use OpenAL for audio and FFmpeg for video.

## Checklist

- Confirm OpenAL backend selection on Windows.
- Remove runtime assumptions about Miles in the modern path.
- Remove runtime assumptions about Bink in the modern path.