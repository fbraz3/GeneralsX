# Upstream Sync Plan: TheSuperHackers (2026-08-16)

## Overview
Syncing `TheSuperHackers/GeneralsGameCode` (`main` branch) into `GeneralsX` on branch `thesuperhackers-sync-08-16-2026`.

This sync imports upstream bugfixes, refactoring (e.g. `DynamicAudioEventRTS` reference counting, network packet serialization, heightmap/scorch unification), and core improvements while strictly preserving GeneralsX's cross-platform architecture (SDL3, DXVK, MiniAudio/OpenAL, FP determinism, platform-isolated CMake configuration, and CI workflows).

## Conflict Analysis & Resolution Strategy

### 1. CI / GitHub Workflows
- **File**: `.github/workflows/build-toolchain.yml` (Conflict: DU - deleted in HEAD, modified upstream)
- **Decision**: Reject upstream file and delete it (`git rm .github/workflows/build-toolchain.yml`).
- **Reasoning**: GeneralsX rules strictly state: *"Never replace our CI/CD infrastructure with upstream versions."* GeneralsX uses its own CI pipelines.

### 2. Build System & CMake
- **File**: `GeneralsMD/Code/Main/CMakeLists.txt`
- **Conflict**: Link libraries list. Upstream has raw Windows libraries (`binkstub`, `comctl32`, `d3d8`, etc.) directly in `target_link_libraries(z_generals PRIVATE ...)` while GeneralsX isolates Windows-specific libraries under `if(WIN32)` and links cross-platform modules (`core_profile_legacy`, `d3dx8`, `z_gameengine`, `z_gameenginedevice`, `zi_always`) conditionally with SDL3.
- **Decision**: Preserve GeneralsX's platform isolation. Keep `if(WIN32)` block for Windows dependencies and keep cross-platform libraries clean.

### 3. Audio Subsystem Refactor
- **Files**:
  - `Core/GameEngine/Include/Common/AudioRequest.h`
  - `Core/GameEngine/Source/Common/Audio/AudioRequest.cpp`
  - `Core/GameEngineDevice/Source/MilesAudioDevice/MilesAudioManager.cpp`
  - `Core/GameEngineDevice/Source/MiniAudioDevice/MiniAudioManager.cpp` (Validation/Adaptation)
  - `Core/GameEngineDevice/Source/OpenALAudioDevice/OpenALAudioManager.cpp` (Validation/Adaptation)
- **Conflict**: Upstream replaced the legacy `union { AudioEventRTS *m_pendingEvent; AudioHandle m_handleToInteractOn; }` and `Bool m_usePendingEvent` with safe reference counting using `RefCountPtr<DynamicAudioEventRTS> m_pendingEvent` and separate `AudioHandle m_handleToInteractOn` to prevent dangling pointers when re-requesting audio loops.
- **Decision**:
  - Adopt upstream's `AudioRequest` structure with `RefCountPtr<DynamicAudioEventRTS>`.
  - In `MilesAudioManager.cpp`, accept upstream's `req->m_pendingEvent` refactor and `.Peek()` accessors.
  - Update `MiniAudioManager.cpp` and `OpenALAudioManager.cpp` where `m_usePendingEvent` was checked: replace `!req->m_usePendingEvent` with `req->m_pendingEvent == nullptr` and adapt `.Peek()` accessors to maintain strict OpenAL/MiniAudio parity.

### 4. Game Networking Subsystem
- **Files**:
  - `Core/GameEngine/Include/GameNetwork/NetCommandMsg.h`
  - `Core/GameEngine/Include/GameNetwork/NetPacket.h`
  - `Core/GameEngine/Source/GameNetwork/NetPacket.cpp`
- **Conflict**:
  - `NetCommandMsg.h`: Upstream added virtual method `readMessageData(NetCommandRef& ref, NetPacketBuf buf) const final`.
  - `NetPacket.h` & `NetPacket.cpp`: Upstream updated `ConstructBigCommandList` to return `NetCommandList*` and use `NetCommandDataChunk` for chunking rather than legacy `NetPacketList`.
- **Decision**: Accept upstream networking improvements. These improve packet chunking and buffer handling without impacting platform abstraction.

### 5. Unified Core Files & Auto-merged Areas
- **Files**:
  - `Shell.h`, `ShellHooks.h`, `ShellMenuScheme.h`, `Shell.cpp`, `ShellMenuScheme.cpp` moved from `GeneralsMD` to `Core/GameEngine`.
  - Heightmap visual and scorch unified files (`W3DScorch.cpp`, `W3DScorch.h`).
- **Decision**: Accept unified structure and ensure CMake builds both `Generals` and `GeneralsMD` properly with the unified Core sources.

## Execution Steps
1. Resolve `.github/workflows/build-toolchain.yml` via deletion.
2. Resolve `GeneralsMD/Code/Main/CMakeLists.txt` keeping platform-isolated linking.
3. Resolve `AudioRequest.h` and `AudioRequest.cpp`.
4. Resolve `MilesAudioManager.cpp` and adapt `MiniAudioManager.cpp` / `OpenALAudioManager.cpp`.
5. Resolve `NetCommandMsg.h`, `NetPacket.h`, and `NetPacket.cpp`.
6. Audit for determinism rules (WWMath usage, FPU guards, NaN guards) in new/modified upstream code.
7. Verify and test CMake configuration and build on macOS (`cmake --preset macos-vulkan` or native build).
8. Ensure all merge markers are gone.
9. Update `docs/WORKLOG/2026-08-DIARY.md`.
10. Commit merge and push `thesuperhackers-sync-08-16-2026`.
