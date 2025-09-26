# GeneralsX - macOS Build Guide

This guide provides detailed instructions for building GeneralsX on macOS, including ARM64 native compilation for Apple Silicon (M1/M2).

## 📋 Prerequisites

### Essential Tools
- **Xcode Command Line Tools** (latest version)
- **Homebrew** for package management
- **CMake** 3.20 or higher
- **Ninja** build system (recommended)

### Recommended Target
- **Main Executable Target**: `z_generals` (Zero Hour expansion, ARM64 native)
- **Apple Silicon**: Use ARM64 native compilation for best performance

### Prerequisites Installation

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install CMake and Ninja
brew install cmake ninja
```

## 🔧 Environment Setup

### 1. Repository Clone
```bash
git clone https://github.com/fbraz3/GeneralsX.git
cd GeneralsX
```

### 2. Build Configuration
```bash
# Configure using vc6 preset (recommended for compatibility)
# For Apple Silicon (M1/M2):
cmake --preset vc6 -DCMAKE_OSX_ARCHITECTURES=arm64
# Para Intel Macs:
cmake --preset vc6
```

## 🏗️ Building

### Build Zero Hour (Recommended)
```bash
# Build the main Zero Hour executable (ARM64 native recommended)
cmake --build build/vc6 --target z_generals -j 4

# Executable will be created at: build/vc6/GeneralsMD/generalszh
# For Apple Silicon, this binary will be ARM64 native
```

### Build Original Generals
```bash
# Build the original Generals executable
cmake --build build/vc6 --target g_generals -j 4

# Executable will be created at: build/vc6/Generals/generals
```

### Build Core Libraries (Optional)
```bash
# To test core libraries independently
cmake --build build/vc6 --target ww3d2 wwlib wwmath
```

## 🎮 Runtime Setup

### Game Assets (Required)
The game requires original Command & Conquer: Generals assets to run properly.

```bash
# Create directory for assets
mkdir -p $HOME/Downloads/generals

# Copy the executable
cp ./build/vc6/GeneralsMD/generalszh $HOME/Downloads/generals/

# Copy original game assets to $HOME/Downloads/generals/
# Required: Data/, Maps/, etc. from original installation
```

### Running the Game
```bash
# Run in directory with assets
cd $HOME/Downloads/generals && ./generalszh
```

## 🐛 Debug and Development

### Debugging with LLDB
```bash
# Use automatic debug script
cd $HOME/Downloads/generals && lldb -s $REPO_PATH/scripts/debug_script.lldb generalszh
```

### Alternative Build Configurations

#### Debug Build
```bash
# For development with debug symbols (Apple Silicon)
cmake --preset vc6 -DCMAKE_OSX_ARCHITECTURES=arm64 -DRTS_BUILD_OPTION_DEBUG=ON
cmake --build build/vc6 --target z_generals -j 4
```

#### Release Build (Default)
```bash
# For optimized performance (Apple Silicon)
cmake --preset vc6 -DCMAKE_OSX_ARCHITECTURES=arm64 -DRTS_BUILD_OPTION_DEBUG=OFF
cmake --build build/vc6 --target z_generals -j 4
```

## ⚡ Performance Tips

### Parallel Compilation
```bash
# Use half of available cores to avoid system overload
# For 8-core machine: -j 4
# For 16-core machine: -j 8
cmake --build build/vc6 --target z_generals -j $(sysctl -n hw.ncpu | awk '{print int($1/2)}')
# For Apple Silicon, this ensures optimal performance
```

### Build Cleanup
```bash
# Clean previous build if needed
rm -rf build/vc6
cmake --preset vc6
```

## 📊 macOS Port Status

### ✅ Functional
- **Complete compilation**: 100% of modules compile successfully
- **Linking**: All libraries link correctly
- **Memory Management**: Memory corruption fixes implemented
- **Cross-Platform APIs**: Win32→POSIX compatibility layer working

### 🔄 In Development
- **Startup Crash**: Ongoing investigation of initialization crash
- **Graphics Pipeline**: W3D/OpenGL rendering validation
- **Asset Loading**: Resource loading optimization

### 🎯 Next Steps
- Startup crash resolution
- Testing with different asset versions
- Complete gameplay validation

## 📚 Additional Resources

- **Progress Tracking**: [MACOS_PORT.md](MACOS_PORT.md)
- **Next Steps**: [NEXT_STEPS.md](NEXT_STEPS.md)

## 🆘 Troubleshooting

### Common Issues

#### CMake can't find dependencies
```bash
# Update Homebrew and reinstall CMake
brew update && brew upgrade cmake
```

#### Linking errors
```bash
# Clean and rebuild
rm -rf build/vc6
cmake --preset vc6
cmake --build build/vc6 --target z_generals -j 4
```

#### Runtime crash
```bash
# Check if assets are in correct location
ls $HOME/Downloads/generals/Data/
ls $HOME/Downloads/generals/Maps/
```

### Support
For macOS port specific issues, check [Issues](https://github.com/fbraz3/GeneralsX/issues) or open a new one.

---
**Last updated**: September 2025  
**Status**: Phase 19 - Runtime Debugging and Stabilization