# Testing Guide

This document covers testing procedures for different platforms and build configurations.

## Test Replays

The GeneralsReplays folder contains replays and the required maps that are tested in CI to ensure that the game is retail compatible.

### Windows Testing
You can also test with these replays locally:
- Copy the replays into a subfolder in your `%USERPROFILE%/Documents/Command and Conquer Generals Zero Hour Data/Replays` folder.
- Copy the maps into `%USERPROFILE%/Documents/Command and Conquer Generals Zero Hour Data/Maps`
- Start the test with this: (copy into a .bat file next to your executable)
```bat
START /B /W generalszh.exe -jobs 4 -headless -replay subfolder/*.rep > replay_check.log
echo %errorlevel%
PAUSE
```
It will run the game in the background and check that each replay is compatible. You need to use a VC6 build with optimizations and RTS_BUILD_OPTION_DEBUG = OFF, otherwise the game won't be compatible.

## macOS Testing

### Compilation Testing
Test the macOS port compilation progress:

```bash
# Check current compilation status
cd GeneralsGameCode/build/vc6
ninja g_generals 2>&1 | grep "FAILED:" | wc -l

# View compilation progress
ninja g_generals 2>&1 | grep -E "(Building|FAILED)" | head -20

# Test specific modules
ninja g_ww3d2  # Graphics module
ninja core_wwlib  # Core libraries
```

### DirectX Compatibility Testing
Verify the DirectX compatibility layer:

```bash
# Compile individual files to test DirectX compatibility
cd build/vc6
c++ -DENABLE_OPENGL=1 -D_UNIX -I../../Core/Libraries/Source/WWVegas/WW3D2 \
    -c ../../Generals/Code/Libraries/Source/WWVegas/WW3D2/assetmgr.cpp

# Test specific DirectX functionality
c++ -DENABLE_OPENGL=1 -D_UNIX -I../../Core/Libraries/Source/WWVegas/WW3D2 \
    -c ../../Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8fvf.cpp
```

### Cross-Platform Build Testing
```bash
# Clean build test
rm -rf build && mkdir build && cd build
cmake -DENABLE_OPENGL=ON -DRTS_BUILD_GENERALS=ON ..
ninja g_generals

# Test different configurations
cmake -DENABLE_OPENGL=ON -DRTS_BUILD_ZEROHOUR=ON ..
cmake -DENABLE_OPENGL=ON -DRTS_BUILD_CORE_TOOLS=ON ..
```

## Linux Testing

Follow similar procedures as macOS for compilation testing. The DirectX compatibility layer should work on Linux as well.

```bash
# Linux specific test
make g_generals -j$(nproc)
```

## Testing Checklist

### Before Each Release
- [ ] Windows build compiles without errors
- [ ] macOS build compiles with minimal errors (track progress)  
- [ ] Linux build compiles without errors
- [ ] DirectX compatibility layer provides all required interfaces
- [ ] Core libraries link successfully
- [ ] Game startup doesn't crash immediately

### macOS Port Specific
- [ ] All DirectX interfaces compile without missing methods
- [ ] Windows API compatibility functions work correctly
- [ ] No pointer casting errors on 64-bit macOS
- [ ] CMake properly detects macOS platform and applies compatibility headers
- [ ] Core libraries (core_wwlib) build successfully
- [ ] WW3D2 graphics module compiles with <5 file failures

For detailed macOS porting progress, see [MACOS_PORT.md](./MACOS_PORT.md).