[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/fbraz3/GeneralsGameCode)

# GeneralsX - Cross-Platform Command & Conquer: Generals

A comprehensive cross-platform port of Command & Conquer: Generals and Zero Hour, bringing the classic RTS experience to **Linux** and **Windows** through native DXVK (DirectX 8 → Vulkan) rendering and SDL3 API.

**Current Status**: Linux native builds functional with Docker (Phase 1 complete). macOS planned for future development.

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

- **🐧 Linux Native Support** - ✅ **FUNCTIONAL** - Native builds via Docker with DXVK + SDL3
- **🪟 Windows Enhanced** - Maintained compatibility with original VC6 builds
- **🍎 macOS Native Support** - 📋 **PLANNED** - Future development target
- **🎮 Modern Graphics** - DXVK translates DirectX 8 → Vulkan for native Linux rendering
- **🔧 Modern Architecture** - SDL3 windowing/input, portable INI configuration

### 🎮 Features

**Cross-Platform Compatibility**:

- **Linux native builds** via Docker (SDL3 + DXVK)
- **Windows compatibility** maintained (VC6/MSVC2022 presets)
- Unified configuration system via INI files (replacing Windows Registry)
- Platform-native file system integration
- **No Wine/Proton required** - Native DirectX → Vulkan translation via DXVK

**Graphics Enhancements**:

- **DXVK** - Native DirectX 8 → Vulkan translation layer
- **SDL3** for cross-platform window management and input handling
- Improved texture loading and memory management
- Enhanced graphics debugging and profiling tools
- Modern GPU compatibility via Vulkan backend

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

### Linux - ✅ FUNCTIONAL (Docker)

Linux native builds are **fully functional** using Docker:
**[📖 Linux Build Guide](docs/ETC/LINUX_BUILD_INSTRUCTIONS.md)**

Quick start:
```bash
# Clone and build with Docker
git clone https://github.com/fbraz3/GeneralsX.git
cd GeneralsX
./scripts/docker-build-linux-zh.sh linux64-deploy
```

Key features:
- Native ELF binaries (not Wine!)
- DXVK for DirectX → Vulkan rendering
- SDL3 for windowing/input
- Docker-based builds (no system pollution)

### macOS - 📋 PLANNED (Future)

**⚠️ macOS builds are NOT currently functional - this is a future development target.**

Planned build instructions for macOS development:
**[📖 macOS Build Guide](docs/ETC/MACOS_BUILD_INSTRUCTIONS.md)** *(Reference only)*

Targeted for future implementation after Linux port stabilization.

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

- **SDL3** - Cross-platform windowing and input handling
- **DXVK** - DirectX 8 → Vulkan translation layer (Linux)
- **Vulkan SDK** - Modern graphics rendering backend
- **OpenAL** - Cross-platform audio system (Phase 2 - planned)
- **CMake** - Build system

For dependency management details, see [vcpkg.json](vcpkg.json).

## 🚀 Project Phases

The Linux native port is organized into phases:

- **Phase 0**: ✅ **COMPLETE** - Deep analysis & planning (DXVK architecture, OpenAL patterns)
- **Phase 1**: ✅ **COMPLETE** - Linux Graphics (DXVK integration, SDL3 windowing, Docker builds)
- **Phase 2**: 🔄 **NEXT** - Linux Audio (OpenAL integration, Miles → OpenAL compatibility)
- **Phase 3**: 📋 **PLANNED** - Video Playback (Bink alternative investigation)
- **Phase 4+**: 📋 **FUTURE** - Polish, optimization, macOS port

**Approach**: Native DXVK (DirectX → Vulkan), NOT Wine emulation.

See [docs/DEV_BLOG/](docs/DEV_BLOG/) and [docs/WORKDIR/phases/](docs/WORKDIR/phases/) for detailed phase progress.

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

- **Phase 2 (Audio)** - OpenAL integration for Linux native audio
- **Runtime Testing** - Validate Linux binary smoke tests and gameplay
- **Cross-Platform Testing** - Validate functionality across distributions
- **Performance Optimization** - Identify and fix bottlenecks
- **Documentation** - Improve build guides and technical resources
- **macOS Port** - Future development (after Linux stabilization)

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