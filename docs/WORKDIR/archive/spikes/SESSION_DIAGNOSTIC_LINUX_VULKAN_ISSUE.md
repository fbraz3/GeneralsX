# Session Diagnostic: Linux Vulkan LLVM SIGSEGV

**Date**: 16 Feb 2026  
**Issue**: GeneralsXZH Linux build crashing during initialization  
**Status**: Partially Resolved

## Root Cause #1: Vulkan LLVM SIGSEGV (RESOLVED)

### Problem
- Game was crashing with `SIGSEGV` in `llvm::Regex::Regex()` during SDL3 Vulkan initialization
- `SDL_Vulkan_LoadLibrary(nullptr)` was attempting to load ALL Vulkan drivers, including Lavapipe
- Lavapipe uses LLVM JIT compilation which crashed during `vkEnumerateInstanceExtensionProperties()`

### Solution
- Removed `SDL_WINDOW_VULKAN` flag from window creation in [SDL3GameEngine.cpp](GeneralsMD/Code/GameEngineDevice/Source/SDL3GameEngine.cpp#L107)
- Commented out `SDL_Vulkan_LoadLibrary()` call entirely
- Added `RADV_USE_ACO=1` flag to avoid LLVM-based driver compilation (not effective alone)

### Diagnosis Steps
1. `sudo ldconfig -p | grep LLVM` - Found 2 LLVM versions (18.1, 20.1)
2. `vulkaninfo` - Confirmed 3 Vulkan drivers available (AMD RADV, Lavapipe, and others)
3. GDB backtrace showed crash occurring in Vulkan initialization layer loading LLVM
4. Initial attempt with `VK_ICD_FILENAMES` to restrict to AMD RADV only still failed
5. Final solution: avoid Vulkan entirely for initial Linux build - use OpenGL/software rendering

### Files Modified
- [GeneralsMD/Code/GameEngineDevice/Source/SDL3GameEngine.cpp](GeneralsMD/Code/GameEngineDevice/Source/SDL3GameEngine.cpp) - Line 85-107

## Root Cause #2: TheDisplay->init() Unknown Exception (UNRESOLVED)

### Problem
- After fixing Vulkan crash, game progresses further but crashes in `GameClient::init()` when calling `TheDisplay->init()`
- Crash is an unknown exception (not `std::exception`) being thrown from W3D rendering device initialization

### Diagnostic Output
```
DEBUG: createGameDisplay() returned: 0x72982ce5ba58
DEBUG: About to call TheDisplay->init()
ERROR: TheDisplay->init() threw unknown exception
FATAL ERROR: *
*
```

### Likely Causes
1. W3D renderer device initialization failing (DirectX8 via DXVK wrapper)
2. Texture/buffer allocation failures
3. Platform-specific initialization issues in GameDisplay implementation

### Next Steps
- Inspect W3D GameDisplay implementation to add more detailed error handling
- Check DX8 device creation via DXVK interface
- Consider adding platform detection to use fallback rendering instead

### Files to Investigate
- Find and analyze `createGameDisplay()` implementation
- Inspect `GameDisplay::init()` (likely W3DGameDisplay on Linux)
- Check [Core/GameEngineDevice/Source/W3DDevice/](Core/GameEngineDevice/Source/W3DDevice/) for rendering initialization

## Build Logs
- Configure: `logs/build_zh_linux64-deploy_docker.log`
- GDB previous: `logs/gdb.log`, `logs/gdb_aco.log`, `logs/gdb_x11.log`, `logs/gdb_no_vulkan.log`
- Current: `logs/gdb_final.log`

## System Info
- Host GPU: AMD Radeon (Vega 8 + Polaris 12)
- Linux: Ubuntu 22.04
- Vulkan: 1.3.275
- LLVM: 20.1.2 (system libLLVM)

## Recommendations
1. **For Phase 1**: Consider building with software rendering/OpenGL fallback if W3D/DXVK init too complex
2. **For Phase 2**: Map W3D device creation failures with more granular error reporting
3. **Long-term**: Test on clean Ubuntu container to eliminate environment-specific issues
