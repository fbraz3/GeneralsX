# Windows64 Task 05 - OpenAL and FFmpeg

## Scope

Execute OpenAL and FFmpeg as functional media backends for Windows64.

## Files

- `cmake/openal.cmake`
- `cmake/FindFFmpeg.cmake`
- `Core/GameEngineDevice/Source/OpenALAudioDevice/*`
- Any video playback integration points

## Deliverable

- Windows64 modern builds use OpenAL for audio and FFmpeg for video.

## Implementation Reading (LLM)

- `cmake/openal.cmake`
- `cmake/FindFFmpeg.cmake`
- `Core/GameEngineDevice/CMakeLists.txt`
- `Core/GameEngineDevice/Source/OpenALAudioDevice/OpenALAudioManager.cpp`
- `Core/GameEngineDevice/Source/VideoDevice/FFmpeg/FFmpegVideoPlayer.cpp`
- `references/jmarshall-win64-modern/Code/Audio/`

## Execution Outputs

- Windows64 modern runtime uses OpenAL device init path by default.
- FFmpeg playback path is wired for Windows64 runtime.

## Checklist

- Confirm OpenAL backend selection on Windows.
- Remove runtime assumptions about Miles in the modern path.
- Remove runtime assumptions about Bink in the modern path.

## Done Criteria

- `windows64-deploy` build reaches menu with OpenAL active.
- FFmpeg-backed video path initializes on Windows64 runtime.