# GeneralsX - macOS Build Instructions

> ⚠️ **IMPORTANT NOTICE - macOS BUILD NOT FUNCTIONAL**
> 
> **This guide is for FUTURE REFERENCE ONLY**. macOS builds are currently **NOT functional** and represent a **planned future development target**.
> 
> **Current Status**:
> - ❌ macOS builds **DO NOT WORK** yet
> - 📋 This is a **planned feature** for post-Linux stabilization
> - 🐧 Active development is on **Linux native builds** (Phase 1 complete)
> 
> **Why This Guide Exists**:
> - Planning reference for future macOS port
> - Architecture documentation for when Linux port is stable
> - Will be uncommented and tested after Phase 2-3 (Audio/Video) complete
> 
> **What Works NOW**: Linux builds via Docker - See [LINUX_BUILD_INSTRUCTIONS.md](./LINUX_BUILD_INSTRUCTIONS.md)
> 
> **Proceed at your own risk** - these instructions are untested and may not compile.

---

## Prerequisites (NOT YET FUNCTIONAL)

### Essential Tools
- **Xcode Command Line Tools** (latest version)
- **Homebrew** for package management
- **CMake** 3.20 or higher
- **Ninja** build system (recommended)
- **Apple Silicon (M1/M2/M3)** macOS

### Install Prerequisites

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install CMake and Ninja
brew install cmake ninja
```

## Build Configuration

### 1. Clone Repository
```bash
git clone https://github.com/fbraz3/GeneralsX.git
cd GeneralsX
```

### 2. Configure Build (ARM64 Apple Silicon)
```bash
# Configure using macos preset for native ARM64 architecture
cmake --preset macos
```

#### Legacy VC6 preset
```bash
# For compatibility or Windows development
cmake --preset vc6
```

## Build Targets

### Primary Target: Zero Hour (GeneralsXZH)
```bash
# Build the main Zero Hour executable
cmake --build build/macos --target GeneralsXZH -j 4

# Executable location: build/macos/GeneralsMD/GeneralsXZH
```

### Secondary Target: Original Generals (GeneralsX)
```bash
# Build the original Generals executable
cmake --build build/macos --target GeneralsX -j 4

# Executable location: build/macos/Generals/GeneralsX
```

### Core Libraries (Optional Testing)
```bash
# Build core libraries independently
cmake --build build/macos --target ww3d2 wwlib wwmath -j 4
```

### Build with Dynamic Core Allocation
```bash
# Use half of available CPU cores to avoid system overload
cmake --build build/macos --target GeneralsXZH -j $(sysctl -n hw.ncpu | awk '{print int($1/2)}')
```

## Compilation Cache (ccache)

**Highly recommended** for significantly faster rebuilds.

### Installation & Setup

```bash
# Install ccache via Homebrew
brew install ccache

# Configure cache size (10GB recommended)
ccache --set-config=max_size=10G

# Verify configuration
ccache -s
```

### Using ccache with CMake

ccache is **automatically enabled by default** when installed.

```bash
# Standard build - ccache automatically used if installed
cmake --preset macos
cmake --build build/macos --target GeneralsXZH -j 4
```

### Performance Comparison
- **First build (no cache)**: ~3-4 minutes
- **Rebuild with 10% changes**: ~30-60 seconds (3-6x faster)
- **Rebuild with 1% changes**: ~10-20 seconds (6-12x faster)
- **Full rebuild from cache**: ~45-90 seconds (2-4x faster)

### ccache Statistics & Management

```bash
# View cache statistics
ccache -s

# Clear cache (if needed)
ccache -C

# Zero cache statistics (reset counters)
ccache -z
```

### Optional Environment Variables

```bash
# Add to ~/.zshrc or ~/.bashrc for persistent configuration
export CCACHE_DIR=$HOME/.ccache
export CCACHE_MAXSIZE=10G
export CCACHE_COMPRESS=1
export CCACHE_COMPRESSLEVEL=6
export CCACHE_SLOPPINESS=pch_defines,time_macros

# Apply changes
source ~/.zshrc
```

## Debug Build Configurations

### Debug Build
```bash
cmake --preset macos -DRTS_BUILD_OPTION_DEBUG=ON
cmake --build build/macos --target GeneralsXZH -j 4
```

### Release Build (Default)
```bash
cmake --preset macos -DRTS_BUILD_OPTION_DEBUG=OFF
cmake --build build/macos --target GeneralsXZH -j 4
```

## Build Cleanup

```bash
# Clean previous build if experiencing issues
rm -rf build/macos
cmake --preset macos
```

## Troubleshooting

### CMake can't find dependencies

```bash
# Update Homebrew and reinstall CMake
brew update && brew upgrade cmake
```

### Architecture mismatch errors

```bash
# Clean and rebuild
rm -rf build/macos
cmake --preset macos
cmake --build build/macos --target GeneralsXZH -j 4
```

### Linking errors

```bash
# Clean and rebuild
rm -rf build/macos
cmake --preset macos
cmake --build build/macos --target GeneralsXZH -j 4
```

### ccache not working

```bash
# Check if ccache is being used
echo $CMAKE_C_COMPILER_LAUNCHER
echo $CMAKE_CXX_COMPILER_LAUNCHER
# Should output: /usr/local/bin/ccache

# Verify installation
which ccache
ccache --version

# Verbose output for debugging
CCACHE_DEBUG=1 cmake --build build/macos --target GeneralsXZH -j 4
```

### Verify Architecture

```bash
file build/macos/GeneralsMD/GeneralsXZH
# Should show: Mach-O 64-bit executable arm64
```

## Additional Resources

- **Linux Builds (WORKING)**: See [./LINUX_BUILD_INSTRUCTIONS.md](./LINUX_BUILD_INSTRUCTIONS.md) - **USE THIS INSTEAD**
- **Development Diary**: See [../DEV_BLOG/README.md](../DEV_BLOG/README.md)
- **Phase Documentation**: See [../WORKDIR/phases/](../WORKDIR/phases/)
- **Docker Scripts**: See [../../scripts/README_DOCKER_SCRIPTS.md](../../scripts/README_DOCKER_SCRIPTS.md)

## Support

For **working Linux builds**, use the Docker scripts in [LINUX_BUILD_INSTRUCTIONS.md](./LINUX_BUILD_INSTRUCTIONS.md).

For macOS port planning/future development, check [Issues](https://github.com/fbraz3/GeneralsX/issues) with the `macos` label.

---

> ⚠️ **REMINDER**: macOS builds are **NOT FUNCTIONAL** - This is a **future development target** planned after Linux port stabilization (Phase 2-3 complete).

**Last updated**: February 17, 2026  
**Target Architecture**: ARM64 Native (Apple Silicon) - **PLANNED FUTURE**  
**Status**: ❌ **NOT FUNCTIONAL** - Documentation only for future reference  
**Current Focus**: 🐧 Linux native builds (Phase 1 complete ✅)
