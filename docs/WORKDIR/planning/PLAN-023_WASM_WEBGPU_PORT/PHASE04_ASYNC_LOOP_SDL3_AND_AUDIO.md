# Phase 04: Async Browser Main Loop, SDL3 Windowing & Threadless MiniAudio

## Objective
Adapt the engine loop (`GameEngine::executeFrame()` + `SDL3GameEngine::browserFrame()`), loading progress (`emscripten_sleep`), SDL3 browser canvas/cursors, and MiniAudio (threadless + FFmpeg-optional + stream residency probe) so the game runs smoothly inside Chrome/Edge/Safari without blocking the browser main thread.

**Important**: Remember the Blacklist from [PHASE00](./PHASE00_BRANCH_AND_HYGIENE.md). Do **NOT** port `-botmatch`, `-pauseframe`, `-savestateframe`, `-loadstate`, `-captureframe*`, or `GeneralsXParity.sav` code into `GameEngine`, `GameClient`, `W3DDisplay`, or `SDL3GameEngine`.

---

## 1. Core Engine & File Helpers

1. **`Core/GameEngine/Include/Common/ArchiveFile.h`** (around line 64):
   - Add a public accessor so the JS streaming bridge can inspect `(archive, offset, size)` for audio files:
     ```cpp
     const ArchivedFileInfo * friend_getArchivedFileInfo(const AsciiString& filename) const { return getArchivedFileInfo(filename); }
     ```

2. **`Core/GameEngine/Source/Common/FramePacer.cpp`** (around line 61, inside `FramePacer::update()`):
   - Browser builds must never sleep inside the frame loop:
     ```cpp
     #if defined(__EMSCRIPTEN__)
         m_updateTime = m_frameRateLimit.wait(RenderFpsPreset::UncappedFpsValue);
     #else
         const UnsignedInt maxFps = getActualFramesPerSecondLimit();
         m_updateTime = m_frameRateLimit.wait(maxFps);
     #endif
     ```

3. **`Core/GameEngine/Source/GameClient/GameText.cpp`** (around line 1412):
   - Emscripten's `vswprintf` does not support `%hs`. Replace `missingString.format(L"MISSING: '%hs'", label);` with:
     ```cpp
     AsciiString missingLabel;
     missingLabel.format("MISSING: '%s'", label);
     UnicodeString missingString;
     missingString.translate(missingLabel);
     ```

---

## 2. Threadless MiniAudio (`Core/GameEngineDevice/Source/MiniAudioDevice/MiniAudioManager.cpp`)

Apply the Emscripten + no-FFmpeg changes from `wasm-generals/Core/GameEngineDevice/Source/MiniAudioDevice/MiniAudioManager.cpp` carefully (preserving recent `GeneralsX` bugfixes like `isMusicPlaying()` at lines 809–822):

1. Guard `#include "VideoDevice/FFmpeg/FFmpegFile.h"` and `extern "C" { #include <libavcodec/avcodec.h> }` (lines 65–70) with `#ifdef RTS_HAS_FFMPEG ... #endif`.
2. Add `GeneralsXFileResident` declaration and `gxOpenAudioFile(const char *name)` helper (lines 650–672 of the `wasm-generals` diff):
   - Under `__EMSCRIPTEN__`, if `!GeneralsXFileResident(name)`, skip decoding and return `NULL` so uncached network chunks do not cache silent buffers; otherwise open via `TheFileSystem->openFile(name)`.
3. In `MiniAudioManager::update()` (right after `AudioManager::update()`):
   ```cpp
   #if defined(__EMSCRIPTEN__)
       while (ma_resource_manager_process_next_job(&m_resourceManager) == MA_SUCCESS) {}
   #endif
   ```
4. In `MiniAudioManager::playAudioEvent()`:
   - Wrap the `FFmpegFile` decode path in `#ifdef RTS_HAS_FFMPEG` (using `gxOpenAudioFile(fileToPlay.str())`).
   - Add the `#else` branch calling `ma_sound_init_from_file(&m_engine, fileToPlay.str(), flags, groupToUse, NULL, sound)` directly via MiniAudio's VFS.
5. In `MiniAudioManager::openDevice()` (right before `ma_resource_manager_init`):
   ```cpp
   #if defined(__EMSCRIPTEN__)
       resourceManagerConfig.jobThreadCount = 0;
       resourceManagerConfig.flags |= MA_RESOURCE_MANAGER_FLAG_NO_THREADING | MA_RESOURCE_MANAGER_FLAG_NON_BLOCKING;
   #endif
   ```
6. In `vfsFileOpen()`, replace `TheFileSystem->openFile(filename)` with `gxOpenAudioFile(filename)`.

---

## 3. W3DDisplay & SDL3Mouse

1. **`Core/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp`** *(Note: unified into `Core/` in `GeneralsX` HEAD)*:
   - Inside `SDL3_ApplyWindowModeForRenderConfig(Bool windowed, Int renderWidth, Int renderHeight)` (line 613), right after `if (!TheSDL3Window) return;`:
     ```cpp
     #if defined(__EMSCRIPTEN__)
         if (!SDL_SetWindowSize(TheSDL3Window, renderWidth, renderHeight)) {
             fprintf(stderr, "WARNING: SDL_SetWindowSize(%d,%d) failed: %s\n", renderWidth, renderHeight, SDL_GetError());
         }
         return;
     #endif
     ```
   - Do **NOT** add `TheCommandLinePauseFrame` / `TheCommandLineSaveStateFrame` capture hooks from `wasm-generals`.

2. **`Generals/Code/GameEngineDevice/Source/SDL3Device/GameClient/SDL3Mouse.cpp`** & **`GeneralsMD/Code/GameEngineDevice/Source/SDL3Device/GameClient/SDL3Mouse.cpp`**:
   - In `SDL3Mouse::loadCursorFromFile`, replace all 5 `#ifdef __linux__` guards with `#if defined(__linux__) || defined(__EMSCRIPTEN__)` so ANI/static cursor fallbacks and hotspot properties work in Emscripten.

---

## 4. `GameClient`, `GameEngine`, and `GameLogic` Frame Splitting

1. **`GeneralsMD/Code/GameEngine/Include/GameClient/GameClient.h`** & **`Source/GameClient/GameClient.cpp`**:
   - Add `Bool m_drawingEnabled;` (initialized to `TRUE` in constructor), `setDrawingEnabled(Bool enabled)`, and `isDrawingEnabled() const`.
   - Guard both `TheDisplay->DRAW();` calls in `GameClient::update()` with `if (m_drawingEnabled)`.

2. **`GeneralsMD/Code/GameEngine/Include/Common/GameEngine.h`** & **`Source/Common/GameEngine.cpp`**:
   - In `GameEngine.h` (`protected:`): declare `void executeFrame();`.
   - In `GameEngine.cpp`: refactor `GameEngine::execute()` to call `executeFrame()` inside `while (!m_quitting)`, moving the loop body into `GameEngine::executeFrame()` (keeping `static const DWORD startTime` under `RTS_DEBUG`). **Omit** all `botMatch*` counters and `startCommandLineBotMatch()`.

3. **`GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp`**:
   - Add `#if defined(__EMSCRIPTEN__)` -> `#include <emscripten.h>` at top.
   - In `GameLogic::updateLoadProgress(Int progress)` (lines 1140–1150):
     ```cpp
     #if defined(__EMSCRIPTEN__)
         fprintf(stderr, "GENERALSX_LOAD_PROGRESS %d %d\n", progress, m_loadScreen ? 1 : 0);
     #endif
         if( m_loadScreen )
         {
             m_loadScreen->update( progress );
     #if defined(__EMSCRIPTEN__)
             EM_ASM({ if (window.miniaudio) window.miniaudio.devices.forEach(function(d) { if (d && d.webaudio && d.webaudio.state === "running") d.webaudio.suspend(); }); });
             emscripten_sleep(0);
     #endif
         }
     ```
   - In `GameLogic::tryStartNewGame(Bool loadingSaveGame)`:
     - Right after `setLoadingMap( TRUE );`:
       ```cpp
       #if defined(__EMSCRIPTEN__)
           emscripten_sleep(250);
           EM_ASM({ if (window.miniaudio) window.miniaudio.devices.forEach(function(d) { if (d && d.webaudio) d.webaudio.suspend(); }); });
       #endif
       ```
     - Before `updateLoadProgress(LOAD_PROGRESS_END);`:
       ```cpp
       #if defined(__EMSCRIPTEN__)
           {
               Int holdTicks = 0;
               while (EM_ASM_INT({ return globalThis.__gxHoldMatch ? 1 : 0; }) && holdTicks++ < 9000)
               {
                   updateLoadProgress(101);
                   emscripten_sleep(100);
               }
           }
       #endif
       ```
     - Right after `deleteLoadScreen();`: emit `fprintf(stderr, "GENERALSX_LOAD_DONE\n");` under `#if defined(__EMSCRIPTEN__)`.
     - Right after `setLoadingMap( FALSE );`:
       ```cpp
       #if defined(__EMSCRIPTEN__)
           EM_ASM({ if (window.miniaudio) window.miniaudio.devices.forEach(function(d) { if (d && d.webaudio) d.webaudio.resume(); }); });
       #endif
       ```

---

## 5. `SDL3GameEngine`, `SDL3Main.cpp`, and `GeneralsMD/Code/Main/CMakeLists.txt`

1. **`GeneralsMD/Code/GameEngineDevice/Include/SDL3GameEngine.h`**:
   - Under `#if defined(__EMSCRIPTEN__)`, expose `static void browserFrame(void* engine);`.

2. **`GeneralsMD/Code/GameEngineDevice/Source/SDL3GameEngine.cpp`**:
   - Port the clean `#if defined(__EMSCRIPTEN__)` block from `wasm-generals`:
     - Exports (`EMSCRIPTEN_KEEPALIVE`): `GeneralsXOpenLanMenu`, `GeneralsXLogicFrame`, `GeneralsXLanSetIdentity`, `GeneralsXLanSetName`, `GeneralsXLanHost`, `GeneralsXLanJoinFirst`, `GeneralsXLanAccept`, `GeneralsXMouseX`, `GeneralsXMouseY`, `GeneralsXFileResident` (calling `GeneralsXEnsureResident`), `GeneralsXSetAudioMuted`, `GeneralsXSetPaused`, `GeneralsXLanSetMapMinPlayers`, `GeneralsXLanSetSlotAI`, `GeneralsXLanStart`, `GeneralsXLanSurrender`, `GeneralsXLanState`, `GeneralsXLanEndFrame`, and `GeneralsXPump`.
     - In `SDL3GameEngine::execute()`: under `#if defined(__EMSCRIPTEN__)`, call `TheFramePacer->reset(); emscripten_set_main_loop_arg(browserFrame, this, 0, true); emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, 16);`.
     - Implement `SDL3GameEngine::browserFrame(void* enginePointer)` with reentrancy guard (`s_gxInFrame`), `WebGPUDeviceCanSubmitFrame()`, and clean `engine->executeFrame()` (no `botMatch` acceleration loop needed — single step per tick).
     - In `SDL3GameEngine::createAudioManager(Bool dummy)`: return `new NullAudioManager();` when `dummy || (TheGlobalData && !TheGlobalData->m_audioOn)` or when neither `SAGE_USE_MINIAUDIO` nor `SAGE_USE_OPENAL` is enabled.

3. **`GeneralsMD/Code/Main/SDL3Main.cpp`**:
   - Under `#if defined(SAGE_USE_WEBGPU)`:
     - Call `setenv("CNC_GENERALS_ZH_PATH", "/GeneralsZH", 1);` and `setenv("CNC_GENERALS_PATH", "/Generals", 1);`.
     - Skip `FilterSoftwareVulkanICDs()`, `FilterPipeWireOpenAL()`, and `SDL_Vulkan_LoadLibrary()`.
     - Use `Uint32 windowFlags = SDL_WINDOW_HIDDEN;` (NOT `SDL_WINDOW_RESIZABLE`, as explained in `wasm-generals` SDL3Main.cpp so CSS letterboxing does not corrupt SDL mouse scaling).
     - Skip `SDL_GetPrimaryDisplay()` / `SDL_SetWindowPosition()` under `#if !defined(SAGE_USE_WEBGPU)`.

4. **`GeneralsMD/Code/Main/CMakeLists.txt`**:
   - Add the `if(SAGE_USE_WEBGPU)` block from `wasm-generals/GeneralsMD/Code/Main/CMakeLists.txt`:
     - Link `webgpu_context`.
     - Add link options: `LINKER:--error-limit=0`, `-sSUPPORT_LONGJMP=emscripten`, `-sMODULARIZE=1`, `-sEXPORT_ES6=1`, `-sEXPORT_NAME=createGeneralsX`, `-sEXPORTED_RUNTIME_METHODS=FS,IDBFS,addRunDependency,removeRunDependency,callMain,ccall`.
     - Set `SUFFIX ".js"`.
     - **Important font preload improvement**:
       - Keep optional `GENERALSX_WEB_FONT_FILE`, and when not set, automatically preload the repository's bundled free fonts (`assets/fonts/`) into `/fonts`:
         ```cmake
         if(GENERALSX_WEB_FONT_FILE)
             ...
         elseif(EXISTS "${CMAKE_SOURCE_DIR}/assets/fonts")
             target_link_options(z_generals PRIVATE
                 "SHELL:--preload-file \"${CMAKE_SOURCE_DIR}/assets/fonts@/fonts\""
             )
         endif()
         ```
     - Keep optional `GENERALSX_WEB_ASSET_DIR` / `GENERALSX_WEB_ASSET_FILES` / `GENERALSX_WEB_PRELOAD_DIRS` for headless/automated preloading if desired (user retail play streams `.big` files dynamically via `web/` without preloading them into `.data`).

---

## Checklist

- [x] Add `friend_getArchivedFileInfo` in `ArchiveFile.h`.
- [x] Update `FramePacer.cpp` and `GameText.cpp` for `__EMSCRIPTEN__`.
- [x] Update `MiniAudioManager.cpp` (FFmpeg guard, threadless job queue, residency probe).
- [x] Update `Core/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp` (`SDL3_ApplyWindowModeForRenderConfig`).
- [x] Update `SDL3Mouse.cpp` in both `Generals/` and `GeneralsMD/`. *(Verified: `GeneralsX` HEAD already includes unconditional hotspot clamping and static cursor fallback.)*
- [x] Update `GameClient.h/.cpp` (`m_drawingEnabled`) and `GameEngine.h/.cpp` (`executeFrame()`).
- [x] Update `GameLogic.cpp` (`emscripten_sleep` + WebAudio suspend/resume + `__gxHoldMatch`).
- [x] Update `SDL3GameEngine.h/.cpp`, `SDL3Main.cpp`, and `GeneralsMD/Code/Main/CMakeLists.txt` (with `assets/fonts@/fonts` default preload).
