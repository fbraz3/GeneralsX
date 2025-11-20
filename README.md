[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/fbraz3/GeneralsGameCode)

# GeneralsX - Cross-Platform Command & Conquer: Generals

A comprehensive cross-platform port of Command & Conquer: Generals and Zero Hour, bringing the classic RTS experience to **macOS**, **Linux**, and **Windows** through modern Vulkan rendering and SDL2 API.

## Project Goals

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

### 🎮 Features

**Cross-Platform Compatibility**:

- Native compilation on macOS, Linux, and Windows
- Unified configuration system replacing Windows Registry
- Platform-native file system integration

**Graphics Enhancements**:

- Vulkan rendering pipeline for modern GPU compatibility
- Improved texture loading and memory management
- Enhanced graphics debugging and profiling tools

**Modern Development**:

- Updated from Visual C++ 6.0 to modern C++20 standards
- CMake build system for consistent cross-platform builds
- Comprehensive test suite for validation across platforms - planned
- Automated CI/CD for continuous integration - planned

## 📦 Official Downloads

For **stable releases and official builds**, visit:
**[TheSuperHackers/GeneralsGameCode Releases](https://github.com/TheSuperHackers/GeneralsGameCode/releases)**

## 🔨 Building from Source

### macOS - Work In Progress

Comprehensive build instructions for macOS development:
**[📖 macOS Build Guide](docs/MACOS_BUILD_INSTRUCTIONS.md)**

Key requirements:

- Xcode Command Line Tools
- Homebrew (CMake, Ninja)
- Original game assets

```bash
# Quick build (macOS)
git clone https://github.com/fbraz3/GeneralsX.git
cd GeneralsX
cmake --preset vc6
cmake --build build/vc6 --target GeneralsXZH -j 4
```

### Linux

Linux port is under development:
**[📖 Linux Build Guide](docs/LINUX_BUILD_INSTRUCTIONS.md)**

Timeline: Q1 2026 based on macOS foundation

### Windows

Windows builds are maintained through the official TheSuperHackers repository with enhanced cross-platform compatibility.

### 📖 Documentation

Complete documentation is available in the **[docs/](docs/)** directory:

- **[docs/MACOS_BUILD_INSTRUCTIONS.md](docs/MACOS_BUILD_INSTRUCTIONS.md)** - Complete macOS build instructions and troubleshooting
- **[docs/LINUX_BUILD_INSTRUCTIONS.md](docs/LINUX_BUILD_INSTRUCTIONS.md)** - Linux port status and contribution guidelines
- **[docs/MACOS_PORT_DIARY.md](docs/MACOS_PORT_DIARY.md)** - Technical development diary with detailed phase progress and implementation notes
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Guidelines for contributing to cross-platform development

### 🐛 Known Issues

For documented limitations and known bugs, see:
**[📋 Known Issues Index](docs/KNOWN_ISSUES/README.md)**

## 🚀 Future Enhancements

### 🧵 Multithreading Modernization

This is a future initiative to leverage multi-core CPUs while preserving deterministic gameplay. High-level plan:

- Start with low-risk tasks (parallel asset/INI loading, background audio/I/O)
- Evolve to moderate threading (AI batches, object updates with partitioning)
- Consider advanced loop decoupling (producer–consumer) once stable

Read the full analysis and phased plan in docs/Misc/MULTITHREADING_ANALYSIS.md.

---

## 🤝 Contributing

Contributions are welcome! We're particularly interested in:

**Current Priority Areas**:

- **TheMetaMap Subsystem Resolution** - Help resolve CommandMap.ini loading issues
- **Final Subsystem Validation** - Test remaining 5/42 subsystems (12%)
- **Graphics Pipeline Testing** - Validate W3D/OpenGL rendering functionality
- **Linux Compatibility Research** - Prepare foundation for Linux port
- **Cross-Platform Testing** - Validate functionality across systems

**How to Contribute**:

1. Check current issues and GitHub discussions
2. Read platform-specific build guides ([macOS](docs/MACOS_BUILD_INSTRUCTIONS.md), [Linux](docs/LINUX_BUILD_INSTRUCTIONS.md))
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
