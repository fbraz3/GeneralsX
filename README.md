[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/fbraz3/GeneralsGameCode)

# GeneralsX - Cross-Platform Command & Conquer: Generals

A comprehensive cross-platform port of Command & Conquer: Generals and Zero Hour, bringing the classic RTS experience to **macOS**, **Linux**, and **Windows** through modern OpenGL rendering and extensive platform compatibility layers.

## 🎉 Project Evolution

This project was born as a fork to provide public releases of the [TheSuperHackers](https://github.com/TheSuperHackers/GeneralsGameCode) project.

Since the [public releases PR](https://github.com/TheSuperHackers/GeneralsGameCode/pull/929) was officially integrated into the original project, I decided to change the project's focus.

This repository now focuses on **experimental cross-platform development** and serves as the technical foundation for multi-platform support. For **official releases and stable builds** (Windows only), visit:

**👉 [TheSuperHackers/GeneralsGameCode Releases](https://github.com/TheSuperHackers/GeneralsGameCode/releases)**

## 💖 Support This Project

Cross-platform game development requires significant time, resources, and technical expertise. If GeneralsX has been valuable to you or the Command & Conquer community, consider supporting continued development:

**[🎯 Sponsor on GitHub](https://github.com/sponsors/fbraz3)**

Your support helps with:
- **Development Time** - Hundreds of hours invested in cross-platform porting
- **Testing Infrastructure** - Multiple platforms, hardware configurations, and tools
- **Documentation** - Comprehensive guides and technical resources
- **Community Support** - Maintaining issues, discussions, and contributions

*Every contribution, no matter the size, makes a difference in keeping classic games alive across all platforms!*

### 🌍 Cross-Platform Vision

This project transforms the Windows-exclusive Command & Conquer: Generals into a truly cross-platform game:

- **🍎 macOS Native Support** - Full compatibility with Apple Silicon and Intel Macs
- **🐧 Linux Distribution** - Native builds for major Linux distributions  
- **🪟 Windows Enhanced** - Improved stability and modern graphics support
- **🎮 Unified Experience** - Consistent gameplay across all platforms
- **🔧 Modern Architecture** - Updated from VC6/C++98 to modern C++20 standards

### 🚀 Current Port Status

**Phase 19 - Runtime Debugging and Stabilization**

**✅ MAJOR ACHIEVEMENTS**:
- **✅ 100% Compilation Success** - All modules compile and link successfully on macOS
- **✅ Memory Corruption Fixes** - AsciiString and memory management issues resolved
- **✅ Cross-Platform Compatibility** - Full Win32→POSIX API compatibility layer
- **✅ Build System** - CMake/Ninja infrastructure fully operational across platforms
- **✅ Official Integration** - Code successfully merged into TheSuperHackers project

**🔧 CURRENT FOCUS**:
- **Startup Crash Investigation** - Resolving initialization issues during game launch
- **Graphics Pipeline Validation** - Ensuring W3D/OpenGL rendering works correctly
- **Asset Loading Optimization** - Improving resource management and loading

**Platform Status**:
- **macOS**: Compiles ✅, Links ✅, Runtime debugging in progress 🔄
- **Linux**: Planned for Q1 2026 based on macOS foundation 📅
- **Windows**: Enhanced compatibility maintained ✅

### 🛠 Platform Support Status

| Platform | Status | Graphics API | Build System | Documentation |
|----------|--------|--------------|--------------|---------------|
| **macOS** | 🟡 Runtime Debugging | OpenGL | CMake/Ninja ✅ | [MACOS_BUILD.md](MACOS_BUILD.md) |
| **Linux** | � Planned Q1 2026 | OpenGL | CMake/Ninja ✅ | [LINUX_BUILD.md](LINUX_BUILD.md) |
| **Windows** | ✅ Enhanced | DirectX 8 + OpenGL | CMake/Ninja ✅ | Windows-native |

### 🎮 Features

**Cross-Platform Compatibility**:
- Native compilation on macOS, Linux, and Windows
- Unified configuration system replacing Windows Registry
- Cross-platform networking and multiplayer support
- Platform-native file system integration

**Graphics Enhancements**:
- OpenGL rendering pipeline for modern GPU compatibility
- Maintained DirectX 8 support for Windows legacy systems
- Improved texture loading and memory management
- Enhanced graphics debugging and profiling tools

**Why OpenGL over Vulkan?**

We chose OpenGL as our cross-platform graphics API for several strategic reasons:

- **🍎 macOS Native Support**: macOS deprecated OpenGL but still provides native support, while Vulkan requires complex wrapper layers (MoltenVK) that translate Vulkan calls to Metal, adding unnecessary complexity and potential performance overhead
- **🕰 Legacy Compatibility**: The original game engine was designed for DirectX 8 (2002), making OpenGL's more traditional immediate-mode approach a natural fit for porting existing rendering code
- **🔧 Development Speed**: OpenGL's simpler state machine aligns better with the original graphics architecture, allowing faster porting with fewer fundamental code rewrites
- **📚 Mature Ecosystem**: OpenGL has decades of documentation, debugging tools, and community knowledge that accelerates development
- **🎯 Sufficient Performance**: For a 2003 RTS game, OpenGL provides more than adequate performance while maintaining broad hardware compatibility
- **🖥 Hardware Support**: OpenGL works excellently on older hardware that C&C Generals players might still be using, including integrated graphics
- **⚡ Future Path**: The OpenGL foundation can be enhanced with modern extensions or eventually serve as a stepping stone to Vulkan if needed

This approach prioritizes **getting the game running cross-platform quickly** rather than chasing cutting-edge graphics performance that the original game design doesn't require.

**Modern Development**:
- Updated from Visual C++ 6.0 to modern C++20 standards
- CMake build system for consistent cross-platform builds
- Comprehensive test suite for validation across platforms
- Automated CI/CD for continuous integration

### 📋 Implementation Architecture

**Multi-Layer Compatibility System**:

1. **Core Layer** (`Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h`)
   - Foundation Windows API types and functions
   - Cross-platform memory, threading, filesystem operations
   - DirectX base structures and constants

2. **Game-Specific Layers** (`Generals/` and `GeneralsMD/`)
   - Extended DirectX 8 interfaces and methods
   - Game-specific functionality and assets
   - Version-specific compatibility implementations

3. **Platform Abstraction**
   - Conditional compilation for platform-specific code
   - Unified API surface with platform-native backends
   - Resource management and error handling

### 🚀 Getting Started

## 📦 Official Downloads

For **stable releases and official builds**, visit:
**[TheSuperHackers/GeneralsGameCode Releases](https://github.com/TheSuperHackers/GeneralsGameCode/releases)**

## 🔨 Building from Source

### macOS
Comprehensive build instructions for macOS development:
**[📖 macOS Build Guide](MACOS_BUILD.md)**

Key requirements:
- Xcode Command Line Tools
- Homebrew (CMake, Ninja)
- Original game assets

```bash
# Quick build (macOS)
git clone https://github.com/fbraz3/GeneralsX.git
cd GeneralsX
cmake --preset vc6
cmake --build build/vc6 --target z_generals -j 4
```

### Linux
Linux port is under development:
**[📖 Linux Build Guide](LINUX_BUILD.md)**

Timeline: Q1 2026 based on macOS foundation

### Windows
Windows builds are maintained through the official TheSuperHackers repository with enhanced cross-platform compatibility.

### 📖 Documentation

- **[MACOS_BUILD.md](MACOS_BUILD.md)** - Complete macOS build instructions and troubleshooting
- **[LINUX_BUILD.md](LINUX_BUILD.md)** - Linux port status and contribution guidelines
- **[MACOS_PORT.md](MACOS_PORT.md)** - Detailed technical progress and implementation notes
- **[NEXT_STEPS.md](NEXT_STEPS.md)** - Current development phase and upcoming work
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Guidelines for contributing to cross-platform development
- **[TESTING.md](TESTING.md)** - Cross-platform testing procedures and validation

### 🎯 Roadmap

**Phase 19 (Current): Runtime Debugging**
- 🔄 Investigate and resolve startup crash
- 🔄 Validate graphics pipeline functionality
- 🔄 Optimize asset loading and memory management

**Phase 20: Linux Port Foundation**
- � Extend compatibility layer for Linux APIs
- 📋 Adapt build system for Linux distributions
- 📋 Initial Linux compilation targets

**Phase 21: Multi-Platform Testing**
- 📋 Cross-platform gameplay validation
- 📋 Performance optimization and profiling
- 📋 Automated testing infrastructure

**Phase 22: Distribution and Integration**
- 📋 Package manager integration (Homebrew, APT)
- 📋 Continuous integration across platforms
- 📋 Community feedback and stable releases

## 🤝 Project Relationship

This repository represents the **experimental development branch** of cross-platform Command & Conquer: Generals support. The project has been **successfully integrated** into the official [TheSuperHackers/GeneralsGameCode](https://github.com/TheSuperHackers/GeneralsGameCode) repository.

**For end users**: Download stable releases from the [official repository](https://github.com/TheSuperHackers/GeneralsGameCode/releases)

**For developers**: This repository continues as the **technical foundation** for cross-platform development, focusing on:
- macOS/Linux compatibility research
- Cross-platform API development  
- Experimental features and improvements
- Technical documentation and methodology

## 🤝 Contributing

Contributions are welcome! We're particularly interested in:

**Current Priority Areas**:
- **macOS Runtime Debugging** - Help resolve startup crash issues
- **Linux Compatibility Research** - Prepare foundation for Linux port
- **Cross-Platform Testing** - Validate functionality across systems
- **Documentation Improvements** - Enhance technical guides and tutorials

**How to Contribute**:
1. Check current issues and [NEXT_STEPS.md](NEXT_STEPS.md)
2. Read platform-specific build guides ([macOS](MACOS_BUILD.md), [Linux](LINUX_BUILD.md))
3. Follow [CONTRIBUTING.md](CONTRIBUTING.md) guidelines
4. Submit issues or pull requests with detailed information

**Contributing to Official Project**:
For contributions to the main project, visit: [TheSuperHackers/GeneralsGameCode](https://github.com/TheSuperHackers/GeneralsGameCode)

## 🙏 Special Thanks

- **[TheSuperHackers Team](https://github.com/TheSuperHackers)** for their foundational work and **official integration** of this cross-platform effort
- **[Xezon](https://github.com/xezon)** and contributors for maintaining the GeneralsGameCode project
- **Westwood Studios** for creating the legendary Command & Conquer series
- **EA Games** for Command & Conquer: Generals, which continues to inspire gaming communities
- **All contributors and sponsors** helping to make this game truly cross-platform and accessible worldwide

*Special thanks to [GitHub Sponsors](https://github.com/sponsors/fbraz3) supporting this open-source effort!*

## 📄 License

See the [LICENSE](./LICENSE.md) file for details.

---

**🎯 From weekly releases to cross-platform excellence - contributing to Command & Conquer's future across all platforms!**
