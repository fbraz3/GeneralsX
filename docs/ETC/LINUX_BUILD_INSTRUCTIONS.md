# GeneralsX - Linux Build Instructions

This guide provides step-by-step instructions for building GeneralsX on Linux 64-bit systems.

## Prerequisites

### Essential Tools
- **GCC** or **Clang** compiler (C++20 support required)
- **CMake** 3.20 or higher
- **Ninja** build system (recommended)
- **Git** for repository management

### Distribution-Specific Installation

#### Ubuntu/Debian
```bash
# Install build tools
sudo apt update
sudo apt install build-essential cmake ninja-build git

# Install additional dependencies
sudo apt install libgl1-mesa-dev libglu1-mesa-dev libasound2-dev
```

#### Fedora
```bash
# Install build tools
sudo dnf install gcc-c++ cmake ninja-build git

# Install additional dependencies
sudo dnf install mesa-libGL-devel mesa-libGLU-devel alsa-lib-devel
```

#### Arch Linux
```bash
# Install build tools
sudo pacman -S base-devel cmake ninja git

# Install additional dependencies
sudo pacman -S mesa glu alsa-lib
```

## Build Configuration

### 1. Clone Repository
```bash
git clone https://github.com/fbraz3/GeneralsX.git
cd GeneralsX
```

### 2. Configure Build
```bash
# Configure using linux preset for 64-bit Linux
cmake --preset linux
```

## Build Targets

### Primary Target: Zero Hour (GeneralsXZH)
```bash
# Build the main Zero Hour executable
cmake --build build/linux --target GeneralsXZH -j 4

# Executable location: build/linux/GeneralsMD/GeneralsXZH
```

### Secondary Target: Original Generals (GeneralsX)
```bash
# Build the original Generals executable
cmake --build build/linux --target GeneralsX -j 4

# Executable location: build/linux/Generals/generals
```

### Core Libraries (Optional Testing)
```bash
# Build core libraries independently
cmake --build build/linux --target ww3d2 wwlib wwmath -j 4
```

### Build with Dynamic Core Allocation
```bash
# Use all cores except one to avoid system overload
cmake --build build/linux --target GeneralsXZH -j $(nproc --ignore=1)
```

## Debug Build Configurations

### Debug Build
```bash
cmake --preset linux -DRTS_BUILD_OPTION_DEBUG=ON
cmake --build build/linux --target GeneralsXZH -j 4
```

### Release Build (Default)
```bash
cmake --preset linux -DRTS_BUILD_OPTION_DEBUG=OFF
cmake --build build/linux --target GeneralsXZH -j 4
```

## Build Cleanup

```bash
# Clean previous build if needed
rm -rf build/linux
cmake --preset linux
```

## Troubleshooting

### CMake can't find dependencies
```bash
# Update package manager and reinstall cmake
# Ubuntu/Debian:
sudo apt update && sudo apt upgrade cmake

# Fedora:
sudo dnf update cmake

# Arch Linux:
sudo pacman -Syu cmake
```

### Compiler version issues
```bash
# Ensure C++20 support
gcc --version  # Should be 10+ or later
clang --version  # Should be 13+ or later

# Install newer compiler if needed
# Ubuntu/Debian:
sudo apt install gcc-11 g++-11

# Fedora:
sudo dnf install gcc

# Arch Linux:
sudo pacman -S gcc
```

### Linking errors
```bash
# Clean and rebuild
rm -rf build/linux
cmake --preset linux
cmake --build build/linux --target GeneralsXZH -j 4
```

### Missing graphics libraries
```bash
# Ubuntu/Debian:
sudo apt install libgl1-mesa-dev libglu1-mesa-dev

# Fedora:
sudo dnf install mesa-libGL-devel mesa-libGLU-devel

# Arch Linux:
sudo pacman -S mesa glu
```

### Missing audio libraries
```bash
# Ubuntu/Debian:
sudo apt install libasound2-dev

# Fedora:
sudo dnf install alsa-lib-devel

# Arch Linux:
sudo pacman -S alsa-lib
```

### Verify Build
```bash
# Verify 64-bit executable
file build/linux/GeneralsMD/GeneralsXZH
# Should show: ELF 64-bit LSB executable, x86-64
```

## Port Status

### Current Status
The Linux port is currently in **planning phase**. The build system is ready, but the port is pending completion of the macOS port which will serve as the technical foundation.

### Planned Timeline
- Initial Analysis: Q4 2025
- Build System: Q1 2026
- Core Libraries: Q1 2026
- Graphics/Audio: Q2 2026
- Testing: Q2 2026

## Additional Resources

- **Project Status**: See [../WORKDIR/support/PROJECT_STATUS.md](../WORKDIR/support/PROJECT_STATUS.md)
- **Development Diary**: See [../DEV_BLOG/README.md](../DEV_BLOG/README.md)
- **macOS Build**: See [./MACOS_BUILD_INSTRUCTIONS.md](./MACOS_BUILD_INSTRUCTIONS.md) for reference implementation
- **Compatibility Layer**: `Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h`

## Support

For Linux build-specific issues, check [Issues](https://github.com/fbraz3/GeneralsX/issues) or open a new one with the `linux` label.

---
**Last updated**: October 20, 2025
**Target Architecture**: Linux 64-bit (x86_64)
**Status**: Planning Phase
