# Windows64 Task 05 - OpenAL and FFmpeg on Windows

**Status**: IN PROGRESS  
**Date**: 2026-05-19  
**Primary Target**: GeneralsXZH  
**Build Target**: windows64-deploy

## Scope

Implement and validate the modern media stack for the Windows64 MinGW path:
- OpenAL (audio)
- FFmpeg (video decoding)

This phase is only complete when we have Windows64 configure/build/smoke evidence.

## What Is Already Implemented

### OpenAL

- openal-soft v1.24.2 is integrated via FetchContent in [cmake/openal.cmake](cmake/openal.cmake).
- Windows path enforces WASAPI backend.
- OpenAL is enabled by default for modern Windows path.

### FFmpeg

- FFmpeg discovery is centralized via [cmake/FindFFmpeg.cmake](cmake/FindFFmpeg.cmake).
- [Core/GameEngineDevice/CMakeLists.txt](Core/GameEngineDevice/CMakeLists.txt) now consumes `find_package(FFmpeg)` results directly.
- The old hard dependency on `PkgConfig::FFMPEG` in GameEngineDevice has been removed so Windows64 is not blocked by pkg-config wiring.

## Remaining Work

- Provide a confirmed FFmpeg Windows64 artifact source for this environment (prebuilt kit or controlled local package path).
- Run configure/build for `windows64-deploy` on a machine with MinGW x86_64 compiler tools available in PATH.
- Execute smoke run and record logs as evidence.
- Confirm runtime linkage/deployment for FFmpeg and OpenAL in the produced package.

## Validation Checklist

- `cmake --preset windows64-deploy` succeeds.
- `cmake --build build/windows64-deploy --target z_generals` succeeds.
- Runtime smoke test (`-win -noshellmap`) succeeds.
- Audio initialization uses OpenAL without Miles fallback.
- Video path initializes FFmpeg where expected.

## Current Blocker

In the current workstation, `windows64-deploy` configure is blocked before feature validation because MinGW x86_64 compilers are not available in PATH:
- x86_64-w64-mingw32-gcc
- x86_64-w64-mingw32-g++

## Done Criteria

Task 05 can be marked completed only after:
- OpenAL and FFmpeg are both active in Windows64 configure/build output.
- Windows64 smoke run evidence is captured and stored in project logs/docs.
- No regressions are introduced for Linux/macOS build paths.
