# Upstream Sync Plan: TheSuperHackers (11-08-2026)

## 1. Overview and Objective
- **Branch**: `thesuperhackers-sync-11-08-2026`
- **Upstream Remote**: `thesuperhackers` (`git@github.com:TheSuperHackers/GeneralsGameCode.git`)
- **Incoming Commits**: 18 commits from `thesuperhackers/main` up to `b42b7d3f8`
- **Objective**: Reconcile upstream network/replay frame tick fixes, WW3D2 renderer transform abstraction, D3DX removal, and `max2w3d` cleanups while strictly preserving GeneralsX's cross-platform architecture (SDL3, DXVK, MiniAudio, FFmpeg, NGMP multiplayer, macOS/Linux water shader parity, and CI/CD workflows).

---

## 2. Incoming Upstream Changes Analysis
Key upstream improvements being merged:
1. **Network & MessageStream Frame Tick Decoupling** (`c5ff17922`, `9bebb265a`, `3d0229016` - PR #3615):
   - Decouples `Network::GetCommandsFromCommandList` from `MSG_FRAME_TICK` via `Network::updateLogicFrame()`.
   - Destroys `MSG_FRAME_TICK` at the end of `GameClientMessageDispatcher` so ticks do not pile up in `TheCommandList` during pause or network stalls.
   - Deduplicates redundant `MSG_SET_REPLAY_CAMERA` and `MSG_DESTROY_SELECTED_GROUP` messages in `MessageStream::propagateMessages`.
   - Simplifies `RecorderClass::CRCInfo` to not skip the first local CRC in multiplayer replays (as frame 0 logic CRC is either sent properly when `RETAIL_COMPATIBLE_CRC == 0` or suppressed at generation when `RETAIL_COMPATIBLE_CRC == 1`).
2. **Replay Sync Error Flag** (`7849c7b17` - PR #3152):
   - Adds `-ignoreReplaySyncErrors` CLI flag (`TheDebugIgnoreReplaySyncErrors`) to log and skip replay CRC mismatches without halting/pausing playback.
3. **Precompiled Pixel Shaders & D3DX Removal** (`85135752c` PR #3650, `0e521229f` PR #3649):
   - Replaces runtime `D3DXAssembleShader` in `W3DProfilerFrameCapture.cpp` and `W3DWater.cpp` with precompiled token arrays.
   - Replaces `D3DXGetErrorStringA` and `D3DXGetFVFVertexSize` with self-contained implementations in `dx8wrapper.cpp` and `dx8fvf.cpp`.
4. **WW3D2 Transform Routing via Renderer** (`5fce8e7a8` - PR #3419):
   - Routes world/view/projection transform calls through `Renderer::Set_Transform` and `Renderer::Get_Transform`.
5. **Max2W3D & Pluglib Legacy Cleanup** (`5222ec98c`..`b42b7d3f8` - PR #3653):
   - Removes unused PS2 material, voxelizer, MeshDeform, and obsolete dialogs from `Core/Tools/WW3D/max2w3d` and `pluglib`.

---

## 3. Conflict Analysis & Resolution Strategy

### Subsystem A: CI/CD Workflows
1. **`.github/workflows/ci.yml` & `.github/workflows/reusable-build-toolchain.yml`**
   - **Conflict**: Upstream modified `.github/workflows/ci.yml` and `.github/workflows/reusable-build-toolchain.yml` for MSVC/VC6 ccache (`788066e28`).
   - **Resolution**: Per Critical Merge Instructions ("Never replace our CI/CD infrastructure with upstream versions"), restore `HEAD` (`git checkout HEAD -- .github/workflows/ci.yml .github/workflows/reusable-build-toolchain.yml`) so GeneralsX CI remains untouched.

### Subsystem B: Networking & Replay Subsystem
1. **`Core/GameEngine/Source/GameNetwork/Network.cpp`**
   - **Conflict**: `Bool isTransferCommand(GameMessage *msg);` in `HEAD` vs `void updateLogicFrame();` in `thesuperhackers/main`.
   - **Resolution**: Adopt `void updateLogicFrame();` and remove the unused `Network::isTransferCommand` helper (which has zero callers in the codebase, replaced by `isMessageTypeWithinNetworkRange`).
2. **`Generals/Code/GameEngine/Include/Common/Recorder.h` & `GeneralsMD/Code/GameEngine/Include/Common/Recorder.h`**
   - **Conflict**: Forward-declared `class CRCInfo;` and heap pointer `CRCInfo *m_crcInfo;` in `HEAD` vs nested value class `RecorderClass::CRCInfo` and value member `CRCInfo m_crcInfo;` in `thesuperhackers/main`.
   - **Resolution**: Adopt upstream's nested `RecorderClass::CRCInfo` and value member `CRCInfo m_crcInfo;`. This eliminates dynamic allocation and prevents any null pointer dereference on early replay abort while aligning with `c5ff17922`.
3. **`Generals/Code/GameEngine/Source/Common/Recorder.cpp` & `GeneralsMD/Code/GameEngine/Source/Common/Recorder.cpp`**
   - **Conflict 1 (Lines ~123 & ~1023)**: `RecorderClass::CRCInfo` methods defined at top of `Recorder.cpp` in upstream vs standalone `class CRCInfo` defined around line 1023 in `HEAD`.
   - **Conflict 2 (Line ~1150)**: `TheInGameUI->messageNoFormat(TheGameText->FETCH_OR_SUBSTITUTE("GUI:ReplayCRCMismatch", ...))` in `HEAD` vs `TheInGameUI->message("GUI:CRCMismatch")` in upstream, plus duplicate `const UnsignedInt mismatchFrame` declaration introduced by `7849c7b17`.
   - **Conflict 3 (Line ~1360)**: `m_crcInfo = NEW CRCInfo(header.localPlayerIndex, isMultiplayer);` in `HEAD` vs `m_crcInfo = CRCInfo(header.localPlayerIndex);` in upstream.
   - **Resolution**:
     - Use `RecorderClass::CRCInfo` value member (`m_crcInfo.`) consistently across `Recorder.cpp` and remove the duplicate standalone `class CRCInfo` definition around line 1023.
     - Keep GeneralsX's `mismatchFrame` underflow guard (`const UnsignedInt mismatchFrame = (TheGameLogic->getFrame() > m_crcInfo.GetQueueSize()) ? ... : 0;`) and `mismatchFrame > 0` check before `if (TheDebugIgnoreReplaySyncErrors)`, removing the redundant inner `mismatchFrame` declaration.
     - Preserve GeneralsX's replay-specific UI message (`GUI:ReplayCRCMismatch` via `FETCH_OR_SUBSTITUTE`) and stderr diagnostic logging alongside upstream's `TheDebugIgnoreReplaySyncErrors` early return.

### Subsystem C: W3D Graphics & Shaders
1. **`Core/GameEngineDevice/Source/W3DDevice/GameClient/Water/W3DWater.cpp`**
   - **Conflict**: Upstream converted `m_riverWaterPixelShader`, `m_waterPixelShader`, and `m_trapezoidWaterPixelShader` from `D3DXAssembleShader` strings to precompiled token arrays. GeneralsX explicitly zeroes those three legacy shaders and compiles `m_reflectionPixelShader` from precompiled tokens for cross-platform macOS/Linux/Windows water parity.
   - **Resolution**: Preserve the GeneralsX block (`m_riverWaterPixelShader = 0; m_waterPixelShader = 0; m_trapezoidWaterPixelShader = 0;` and precompiled `m_reflectionPixelShader` creation).

---

## 4. Risk Areas & Mitigation
- **WW3D2 Transform Routing (`5fce8e7a8`)**: Verify that all touched files in `Core/`, `Generals/`, and `GeneralsMD/` include `WW3D2/Renderer.h` where needed and compile cleanly on macOS and Linux.
- **D3DX Removal (`0e521229f`)**: Verify that `Get_DX8_Error_Code_Name` in `dx8wrapper.cpp` compiles against DXVK's `d3d8.h` error macros on macOS and Linux.
- **Duplicate Files Audit**: Verify that no files exist in both `Core/` and `Generals/` or `GeneralsMD/`.

---

## 5. Execution Steps
1. Revert upstream changes to `.github/workflows/`.
2. Resolve conflicts in `Network.cpp`, `W3DWater.cpp`, `Generals/.../Recorder.{h,cpp}`, and `GeneralsMD/.../Recorder.{h,cpp}`.
3. Verify zero merge markers remain (`<<<<<<<`, `=======`, `>>>>>>>`).
4. Audit for duplicate files across `Core/` and `Generals/`/`GeneralsMD/`.
5. Configure, build, and smoke-test `GeneralsXZH` and `GeneralsX` on macOS (`macos-vulkan`) and Linux (`linux64-deploy`).
6. Update `docs/WORKLOG/2026-11-DIARY.md`, clean temporary build/runtime artifacts, commit, and push `thesuperhackers-sync-11-08-2026`.
