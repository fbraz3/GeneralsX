[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/fbraz3/GeneralsGameCode)

# GeneralsX - Cross-Platform Command & Conquer: Generals

A comprehensive cross-platform port of Command & Conquer: Generals and Zero Hour, bringing the classic RTS experience to **macOS**, **Linux**, and **Windows** through modern Vulkan rendering and SDL2 API.

## Project Goals

This repository focuses on **cross-platform development** and serves as the technical foundation for multi-platform support of the classic RTS game.

To keep updated about this project status, visit our [Dev Blog](docs/DEV_BLOG/)

For **official releases and stable builds** (Windows only), visit:

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
- Unified configuration system via INI files (replacing Windows Registry)
- Platform-native file system integration
- Wine/Proton compatibility for Linux via Windows subsystem

**Graphics Enhancements**:

- Vulkan rendering pipeline for modern GPU compatibility
- Improved texture loading and memory management
- Enhanced graphics debugging and profiling tools
- SDL2 for cross-platform window management and input handling

**Modern Development**:

- Updated from Visual C++ 6.0 to modern C++20 standards
- CMake build system for consistent cross-platform builds
- Comprehensive development documentation and phase tracking
- Automated builds for Windows, macOS, and Linux

## 📦 Official Downloads

For **stable releases and official builds**, visit:
**[TheSuperHackers/GeneralsGameCode Releases](https://github.com/TheSuperHackers/GeneralsGameCode/releases)**

## 🔨 Building from Source

### Windows

Primary development platform with full MSVC BuildTools 2022 support:

```bash
# Quick build (Windows)
git clone https://github.com/fbraz3/GeneralsX.git
cd GeneralsX
cmake --preset win32
cmake --build build/win32 --target GeneralsXZH -j 4
```

### macOS - In Development

Comprehensive build instructions for macOS development:
**[📖 macOS Build Guide](docs/ETC/MACOS_BUILD_INSTRUCTIONS.md)**

Key requirements:

- Xcode Command Line Tools
- Homebrew (CMake, Ninja)
- Original game assets

### Linux - Planned

Linux port is under development based on macOS foundation and Wine compatibility layer.

### 📖 Documentation

Complete documentation is available in the **[docs/](docs/)** directory:

- **[docs/ETC/MACOS_BUILD_INSTRUCTIONS.md](docs/ETC/MACOS_BUILD_INSTRUCTIONS.md)** - Complete macOS build instructions and troubleshooting
- **[docs/ETC/LINUX_BUILD_INSTRUCTIONS.md](docs/ETC/LINUX_BUILD_INSTRUCTIONS.md)** - Linux port status and contribution guidelines
- **[docs/DEV_BLOG/](docs/DEV_BLOG/)** - Technical development diary organized by month
- **[docs/WORKDIR/](docs/WORKDIR/)** - Phase planning, implementation notes, and strategic decisions
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Guidelines for contributing to cross-platform development

### 🐛 Known Issues & Limitations

For documented limitations and known bugs, check the development diary in [docs/DEV_BLOG/](docs/DEV_BLOG/).

### ⚙️ Build Requirements

The repository uses a vcpkg manifest (`vcpkg.json`) paired with a lockfile (`vcpkg-lock.json`). Key dependencies:

- **SDL2** - Cross-platform windowing and input handling
- **OpenAL** - Cross-platform audio system
- **Vulkan SDK** - Modern graphics rendering (optional for Phase 1)
- **CMake** - Build system

For dependency management details, see [vcpkg.json](vcpkg.json).

## 🚀 Project Phases

The modernization is organized into phases:

- **Phase 1 (Current)**: Windows 32-bit with SDL2/OpenAL
- **Phase 2**: Vulkan graphics backend (replacing DirectX 8)
- **Phase 3**: Wine cross-platform support for macOS/Linux
- **Phase 4+**: Enhanced features and optimizations

See [docs/DEV_BLOG/](docs/DEV_BLOG/) for detailed phase progress.

## 🚀 Future Enhancements

### 🧵 Multithreading Modernization

Future initiative to leverage multi-core CPUs while preserving deterministic gameplay. High-level plan:

- Start with low-risk tasks (parallel asset/INI loading, background audio/I/O)
- Evolve to moderate threading (AI batches, object updates with partitioning)
- Consider advanced loop decoupling (producer–consumer) once stable

---

## 🤝 Contributing

Contributions are welcome! We're particularly interested in:

**Current Priority Areas**:

- **Graphics Pipeline Testing** - Validate rendering functionality
- **Cross-Platform Testing** - Validate functionality across systems
- **Wine Compatibility Research** - Prepare foundation for Linux/macOS ports
- **Performance Optimization** - Identify and fix bottlenecks
- **Documentation** - Improve build guides and technical resources

**How to Contribute**:

1. Check current issues and GitHub discussions
2. Read platform-specific build guides ([Windows](docs/ETC/), [macOS](docs/ETC/MACOS_BUILD_INSTRUCTIONS.md), [Linux](docs/ETC/LINUX_BUILD_INSTRUCTIONS.md))
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

EA has not endorsed and does not support this product. All trademarks are the property of their respective owners.
