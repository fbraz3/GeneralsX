[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/fbraz3/GeneralsGameCode)

# GeneralsX - Cross-Platform Command & Conquer: Generals

A community-driven cross-platform port of Command & Conquer: Generals and Zero Hour, enabling the classic RTS to run natively on **Linux, macOS, and Windows** under a single modern codebase: **SDL3** (windowing/input) + **DXVK** (DirectX 8 → Vulkan graphics) + **OpenAL** (audio) + **64-bit**.

**Current Status**: Linux native builds functional (Phase 1 complete). macOS ARM64 native builds functional (Phase 1 complete, DXVK + MoltenVK working). Audio (Phase 2) in progress on both platforms.

## Project Goals

This repository targets a **single codebase** that builds and runs on all three major desktop platforms, replacing the original Windows-only DirectX 8 / Miles Sound System stack with portable open-source equivalents.

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
- **🍎 macOS Native Support** - � **IN PROGRESS** - ARM64 native builds working (DXVK + MoltenVK)
- **🎮 Modern Graphics** - DXVK translates DirectX 8 → Vulkan for native Linux rendering
- **🔧 Modern Architecture** - SDL3 windowing/input, portable INI configuration

### 🎮 Features

**Cross-Platform Compatibility**:

- **Single codebase** for Linux, macOS, and Windows
- **Linux native builds** via Docker or native GCC/Clang (SDL3 + DXVK)
- **Windows legacy builds** maintained (VC6/MSVC2022 presets)
- **macOS in progress** via DXVK + MoltenVK (Vulkan → Metal) -- ARM64 native builds working
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

### Linux - ✅ Primary Focus

Native ELF builds with DXVK + SDL3. Graphics working, audio in progress.

**[📖 Linux Build Guide](docs/ETC/LINUX_BUILD_INSTRUCTIONS.md)**

```bash
# Clone and build with Docker
git clone https://github.com/fbraz3/GeneralsX.git
cd GeneralsX
./scripts/docker-build-linux-zh.sh linux64-deploy

# Or build natively on Linux
cmake --preset linux64-deploy
cmake --build build/linux64-deploy --target z_generals
```

### Windows (Legacy VC6/MSVC 2022)

Upstream-compatible builds using native DirectX 8 + Miles Sound System:

```bash
# Quick build (Windows)
git clone https://github.com/fbraz3/GeneralsX.git
cd GeneralsX
cmake --preset win32
cmake --build build/win32 --target GeneralsXZH -j 4
```

### Windows (Modern SDL3/DXVK) - 📋 PLANNED

Modern 64-bit Windows build using the same SDL3 + DXVK + OpenAL stack. Separate branch, TBD.

### macOS - � IN PROGRESS (Apple Silicon)

Native ARM64 builds working via DXVK + MoltenVK. Audio and video are not yet
implemented (Phase 2/3), but the game engine initializes and renders.

**[📖 macOS Build Guide](docs/ETC/MACOS_BUILD_INSTRUCTIONS.md)**

Quick start:
```bash
# Install prerequisites once: brew install cmake ninja meson
# + LunarG Vulkan SDK: https://vulkan.lunarg.com/sdk/home#mac
git clone https://github.com/fbraz3/GeneralsX.git
cd GeneralsX
./scripts/build-macos-zh.sh
./scripts/deploy-macos-zh.sh
./scripts/run-macos-zh.sh -win
```

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

- **SDL3** - Cross-platform windowing and input handling (all platforms)
- **DXVK** - DirectX 8 → Vulkan translation layer (all platforms)
- **Vulkan SDK** - Modern graphics rendering backend
- **OpenAL** - Cross-platform audio system (replacing Miles Sound System)
- **CMake** - Build system with per-platform presets

For dependency management details, see [vcpkg.json](vcpkg.json).

## 🚀 Project Phases

The cross-platform port is organized into phases (each platform branch may progress independently):

- **Phase 0**: ✅ **COMPLETE** - Deep analysis & planning (DXVK architecture, OpenAL patterns)
- **Phase 1**: ✅ **COMPLETE** - Graphics (DXVK + SDL3): Linux x86_64 and macOS ARM64 native builds working
- **Phase 2**: 🔄 **IN PROGRESS** - Audio (OpenAL integration, Miles → OpenAL compatibility)
- **Phase 3**: 📋 **PLANNED** - Video Playback (Bink alternative investigation)
- **Phase 4+**: 📋 **FUTURE** - Polish, optimization, hardening

**Approach**: Single codebase, SDL3 + DXVK + OpenAL on all platforms. Native Vulkan, NOT Wine emulation.

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

- **Phase 2 (Audio)** - OpenAL integration for cross-platform audio
- **Runtime Testing** - Validate Linux and macOS binary smoke tests and gameplay
- **Cross-Platform Testing** - Validate functionality across Linux distributions and macOS versions
- **macOS Port** - 🔄 In progress (ARM64 native working, Phase 2 audio pending)
- **Windows Modern Port** - SDL3 + DXVK + OpenAL 64-bit build (TBD)
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
- **[Fighter19](https://github.com/Fighter19)** for developing the SDL3 and OpenAL solution that inspired this project
- **Westwood Studios** for creating the legendary Command & Conquer series
- **EA Games** for Command & Conquer: Generals, which continues to inspire gaming communities
- **All contributors and sponsors** helping to make this game truly cross-platform and accessible worldwide

*Special thanks to [GitHub Sponsors](https://github.com/sponsors/fbraz3) supporting this open-source effort!*

## 📄 License

See the [LICENSE](./LICENSE.md) file for details.

EA has not endorsed and does not support this product. All trademarks are the property of their respective owners.