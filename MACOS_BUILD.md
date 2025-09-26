# GeneralsX - macOS Build Guide

This guide provides detailed instructions for building GeneralsX on macOS with native Apple Silicon (ARM64) support.

## 📋 Prerequisites

### Essential Tools
- **Xcode Command Line Tools** (latest version)
- **Homebrew** for package management
- **CMake** 3.20 or higher
- **Ninja** build system (recommended)
- **Apple Silicon (M1/M2/M3)** or Intel macOS support

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

### 2. Build Configuration (ARM64 Native - Recommended)
```bash
# Configure using macos-arm64 preset for native ARM64 architecture
cmake --preset macos-arm64
```

### Alternative: Intel x64 macOS
```bash
# For Intel Macs using dedicated Intel preset
cmake --preset macos-x64
```

### Compatibility: Legacy VC6 preset
```bash
# For compatibility or Windows development
cmake --preset vc6
```

## 🏗️ Building

### 🎯 Primary Target: Zero Hour (z_generals)
```bash
# Build the main Zero Hour executable (ARM64 native recommended)
cmake --build build/macos-arm64 --target z_generals -j 4

# Executable will be created at: build/macos-arm64/GeneralsMD/generalszh
```

### 🎮 Secondary Target: Original Generals (g_generals)
```bash
# Build the original Generals executable
cmake --build build/macos-arm64 --target g_generals -j 4

# For Intel Macs:
cmake --build build/macos-x64 --target g_generals -j 4

# Executables will be created at:
# ARM64: build/macos-arm64/Generals/generals
# Intel: build/macos-x64/Generals/generals
```

### 🔧 Core Libraries (Optional Testing)
```bash
# To test core libraries independently
# ARM64:
cmake --build build/macos-arm64 --target ww3d2 wwlib wwmath -j 4

# Intel x64:
cmake --build build/macos-x64 --target ww3d2 wwlib wwmath -j 4
```

### ⚡ Performance Build Optimization
```bash
# Use dynamic core allocation (recommended)
cmake --build build/macos-arm64 --target z_generals -j $(sysctl -n hw.ncpu | awk '{print int($1/2)}')
```

## 🎮 Runtime Setup

### Game Assets (Required)
The game requires original Command & Conquer: Generals/Zero Hour assets to run properly.

```bash
# Create directory for assets
mkdir -p $HOME/Downloads/generals

# Copy the executable (Zero Hour recommended)
# ARM64:
cp ./build/macos-arm64/GeneralsMD/generalszh $HOME/Downloads/generals/

# Intel x64:
cp ./build/macos-x64/GeneralsMD/generalszh $HOME/Downloads/generals/

# Copy original game assets to $HOME/Downloads/generals/
# Required: Data/, Maps/, etc. from original installation
# Zero Hour assets recommended for best compatibility
```

### Running the Game (ARM64 Native Performance)
```bash
# Run in directory with assets (ARM64 native execution)
cd $HOME/Downloads/generals && ./generalszh

# Alternative: Original Generals
cd $HOME/Downloads/generals && ./generals
```

## 🐛 Debug and Development

### Debugging with LLDB (ARM64 Native)
```bash
# Use automatic debug script for crash investigation
cd $HOME/Downloads/generals && lldb -s $REPO_PATH/scripts/debug_script.lldb generalszh

# Direct execution with debug output
cd $HOME/Downloads/generals && ./generalszh
```

### Alternative Build Configurations

#### ARM64 Native Debug Build (Recommended)
```bash
# For development with debug symbols (ARM64)
cmake --preset macos-arm64 -DRTS_BUILD_OPTION_DEBUG=ON
cmake --build build/macos-arm64 --target z_generals -j 4
```

#### Intel x64 Debug Build
```bash
# For development with debug symbols (Intel)
cmake --preset macos-x64 -DRTS_BUILD_OPTION_DEBUG=ON
cmake --build build/macos-x64 --target z_generals -j 4
```

#### ARM64 Native Release Build (Default)
```bash
# For optimized performance (ARM64)
cmake --preset macos-arm64 -DRTS_BUILD_OPTION_DEBUG=OFF
cmake --build build/macos-arm64 --target z_generals -j 4
```

#### Intel x64 Release Build
```bash
# For optimized performance (Intel)
cmake --preset macos-x64 -DRTS_BUILD_OPTION_DEBUG=OFF
cmake --build build/macos-x64 --target z_generals -j 4
```

## ⚡ Performance Tips

### ARM64 Native Compilation Benefits
- **Superior Performance**: Native Apple Silicon execution
- **Better Memory Management**: Improved stability and efficiency
- **Enhanced Debugging**: Better crash investigation capabilities
- **Future-Proof**: Optimized for current and future Apple hardware

### Parallel Compilation
```bash
# Use half of available cores to avoid system overload
# For 8-core M1: -j 4, For 10-core M1 Pro: -j 5, For M1 Max: -j 8
cmake --build build/macos-arm64 --target z_generals -j $(sysctl -n hw.ncpu | awk '{print int($1/2)}')
```

### Build Cleanup
```bash
# Clean previous build if architecture changed
# ARM64:
rm -rf build/macos-arm64
cmake --preset macos-arm64

# Intel x64:
rm -rf build/macos-x64
cmake --preset macos-x64
```

## 📊 macOS Port Status

### ✅ Fully Functional (Phase 22.7)
- **ARM64 Native Compilation**: 100% successful on Apple Silicon
- **Complete Compilation**: All modules compile successfully  
- **Linking**: All libraries link correctly
- **Vector Corruption Protection**: 17+ trillion element detection working perfectly
- **Cross-Platform APIs**: Win32→POSIX compatibility layer stable
- **Advanced INI Processing**: Thousands of successful operations

### � Current Investigation (Phase 22.7)
- **End Token Parsing**: Investigating persistent INI parser exceptions during End token processing
- **Exception Analysis**: "Unknown exception in field parser for: 'End'" and "'  End'" resolution
- **Bypass Optimization**: Enhancing End token bypass mechanisms for complete stability

### 🎯 Recent Achievements
- **Major Breakthrough**: Vector corruption crash completely resolved
- **ARM64 Success**: Native Apple Silicon compilation working
- **Protection System**: Robust validation prevents all corruption scenarios
- **Significant Progress**: Program advances far beyond previous crash points

### 📈 Next Steps
- End token exception resolution
- Complete game engine initialization 
- Full gameplay validation testing

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

#### Architecture mismatch errors
```bash
# Clean and rebuild with explicit architecture
# ARM64:
rm -rf build/macos-arm64
cmake --preset macos-arm64
cmake --build build/macos-arm64 --target z_generals -j 4

# Intel x64:
rm -rf build/macos-x64
cmake --preset macos-x64
cmake --build build/macos-x64 --target z_generals -j 4
```

#### Linking errors
```bash
# Clean and rebuild
rm -rf build/macos-arm64
cmake --preset macos-arm64
cmake --build build/macos-arm64 --target z_generals -j 4
```

#### Runtime crash or "Unknown exception in field parser"
```bash
# Check if assets are in correct location
ls $HOME/Downloads/generals/Data/
ls $HOME/Downloads/generals/Maps/

# Run with debug output
cd $HOME/Downloads/generals && ./generalszh

# Use LLDB for crash investigation
cd $HOME/Downloads/generals && lldb -s $REPO_PATH/scripts/debug_script.lldb generalszh
```

### Performance Issues
```bash
# Verify architecture execution
# ARM64:
file build/macos-arm64/GeneralsMD/generalszh
# Should show: Mach-O 64-bit executable arm64

# Intel x64:
file build/macos-x64/GeneralsMD/generalszh
# Should show: Mach-O 64-bit executable x86_64

# Check system resources
top -pid $(pgrep generalszh)
```

### Support
For macOS port specific issues, check [Issues](https://github.com/fbraz3/GeneralsX/issues) or open a new one.

---
**Last updated**: Dezembro 30, 2024  
**Status**: Phase 22.7 - INI Parser End Token Exception Investigation  
**Architecture**: ARM64 Native (Apple Silicon) + Intel Compatibility