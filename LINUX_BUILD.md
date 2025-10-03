# GeneralsX - Linux Build Guide

This guide provides detailed instructions for building GeneralsX on Linux 64-bit systems.

## 📋 Prerequisites

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

## 🔧 Environment Setup

### 1. Repository Clone
```bash
git clone https://github.com/fbraz3/GeneralsX.git
cd GeneralsX
```

### 2. Build Configuration
```bash
# Configure using linux preset for 64-bit Linux
cmake --preset linux
```

## 🏗️ Building

### 🎯 Primary Target: Zero Hour (GeneralsXZH)
```bash
# Build the main Zero Hour executable
cmake --build build/linux --target GeneralsXZH -j 4

# Executable will be created at: build/linux/GeneralsMD/GeneralsXZH
```

### 🎮 Secondary Target: Original Generals (GeneralsX)
```bash
# Build the original Generals executable
cmake --build build/linux --target GeneralsX -j 4

# Executable will be created at: build/linux/Generals/generals
```

### 🔧 Core Libraries (Optional Testing)
```bash
# To test core libraries independently
cmake --build build/linux --target ww3d2 wwlib wwmath -j 4
```

### ⚡ Performance Build Optimization
```bash
# Use dynamic core allocation (recommended)
cmake --build build/linux --target GeneralsXZH -j $(nproc --ignore=1)
```

## 🎮 Runtime Setup

### Game Assets (Required)
The game requires original Command & Conquer: Generals/Zero Hour assets to run properly.

```bash
# Create directory for assets
mkdir -p $HOME/Downloads/generals

# Copy the executable (Zero Hour recommended)
cp ./build/linux/GeneralsMD/GeneralsXZH $HOME/Downloads/generals/

# Copy original game assets to $HOME/Downloads/generals/
# Required: Data/, Maps/, etc. from original installation
# Zero Hour assets recommended for best compatibility
```

### Running the Game
```bash
# Run in directory with assets
cd $HOME/Downloads/generals && ./GeneralsXZH

# Alternative: Original Generals
cd $HOME/Downloads/generals && ./generals
```

## 🐛 Debug and Development

### Debugging with GDB
```bash
# Use GDB for crash investigation
cd $HOME/Downloads/generals && gdb ./GeneralsXZH

# Direct execution with debug output
cd $HOME/Downloads/generals && ./GeneralsXZH
```

### Alternative Build Configurations

#### Debug Build
```bash
# For development with debug symbols
cmake --preset linux -DRTS_BUILD_OPTION_DEBUG=ON
cmake --build build/linux --target GeneralsXZH -j 4
```

#### Release Build (Default)
```bash
# For optimized performance
cmake --preset linux -DRTS_BUILD_OPTION_DEBUG=OFF
cmake --build build/linux --target GeneralsXZH -j 4
```

## ⚡ Performance Tips

### Linux Native Compilation Benefits
- **Native Performance**: Optimized for Linux 64-bit architecture
- **Distribution Flexibility**: Works across major Linux distributions
- **Development Tools**: Full access to Linux debugging and profiling tools
- **Open Source Ecosystem**: Integration with Linux development workflow

### Parallel Compilation
```bash
# Use all cores except one to avoid system overload
cmake --build build/linux --target GeneralsXZH -j $(nproc --ignore=1)
```

### Build Cleanup
```bash
# Clean previous build if needed
rm -rf build/linux
cmake --preset linux
```

## 📊 Linux Port Status

### 🔄 In Development (Phase 22.7+)
- **Build System**: CMake preset implementation ready
- **Cross-Platform APIs**: Win32→POSIX compatibility layer in progress
- **Testing**: Validation pending across major distributions
- **Graphics Pipeline**: OpenGL rendering system planned

### 🎯 Planned Features
- **Distribution Support**: Ubuntu, Debian, Fedora, Arch, openSUSE
- **Package Management**: Integration with system package managers
- **Desktop Integration**: .desktop files and system integration
- **Performance Optimization**: Linux-specific performance enhancements

### 📈 Next Steps
- Complete Win32→POSIX API compatibility layer
- Testing across target distributions
- Graphics subsystem validation
- Full gameplay testing

## 📚 Additional Resources

- **Progress Tracking**: [MACOS_PORT.md](MACOS_PORT.md)
- **Next Steps**: [NEXT_STEPS.md](NEXT_STEPS.md)
- **macOS Guide**: [MACOS_BUILD.md](MACOS_BUILD.md)

## 🆘 Troubleshooting

### Common Issues

#### CMake can't find dependencies
```bash
# Update package manager and reinstall cmake
# Ubuntu/Debian:
sudo apt update && sudo apt upgrade cmake

# Fedora:
sudo dnf update cmake

# Arch Linux:
sudo pacman -Syu cmake
```

#### Compiler errors
```bash
# Ensure C++20 support
gcc --version  # Should be 10+ or later
clang --version  # Should be 13+ or later
```

#### Linking errors
```bash
# Clean and rebuild
rm -rf build/linux
cmake --preset linux
cmake --build build/linux --target GeneralsXZH -j 4
```

#### Missing graphics libraries
```bash
# Ubuntu/Debian:
sudo apt install libgl1-mesa-dev libglu1-mesa-dev

# Fedora:
sudo dnf install mesa-libGL-devel mesa-libGLU-devel

# Arch Linux:
sudo pacman -S mesa glu
```

### Performance Issues
```bash
# Verify 64-bit executable
file build/linux/GeneralsMD/GeneralsXZH
# Should show: ELF 64-bit LSB executable, x86-64

# Check system resources
htop  # or: top -p $(pgrep GeneralsXZH)
```

### Support
For Linux port specific issues, check [Issues](https://github.com/fbraz3/GeneralsX/issues) or open a new one.

---
**Last updated**: Dezembro 30, 2024  
**Status**: Phase 22.7+ - Linux Support Implementation  
**Architecture**: Linux 64-bit (x86_64)

### 📚 Contributions

If you have Linux development experience and want to contribute to the port:

1. **Fork** this repository
2. **Analyze** the macOS port progress in `MACOS_PORT.md`
3. **Study** the compatibility layer in `Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h`
4. **Open** an Issue describing your contribution proposal

### 🔗 Resources

- **macOS Progress**: [MACOS_BUILD.md](MACOS_BUILD.md) - Use as reference
- **Compatibility Layer**: `Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h`
- **Build System**: `CMakePresets.json` and `cmake/` directory
- **Cross-Platform Patterns**: Search for `#ifdef _WIN32` in the code

### 📢 Updates

To receive notifications about Linux port progress:

- **Watch** this repository on GitHub
- **Follow** [Issues](https://github.com/fbraz3/GeneralsX/issues) with `linux` label
- **Check** [NEXT_STEPS.md](NEXT_STEPS.md) regularly

### 🤝 Community

GeneralsX is a community project. Your contribution to the Linux port will be:
- **Recognized** in project credits
- **Documented** in development history
- **Valued** by the Command & Conquer community

---

## ⏰ Estimated Timeline

| Milestone | Status | Estimate |
|-----------|--------|----------|
| Initial Analysis | 🔄 Planned | Q4 2025 |
| Build System | 📅 Pending | Q1 2026 |
| Core Libraries | 📅 Pending | Q1 2026 |
| Graphics/Audio | 📅 Pending | Q2 2026 |
| Testing | 📅 Pending | Q2 2026 |

*Timeline subject to change based on contributor availability*

---

**💡 Tip**: While waiting for the Linux port, you can follow and contribute to the macOS port development, which will serve as the technical foundation for the Linux port.

**📧 Contact**: For Linux port discussions, open an [Issue](https://github.com/fbraz3/GeneralsX/issues/new) with the appropriate template.

---
**Last updated**: September 2025  
**Status**: Planning and Initial Analysis