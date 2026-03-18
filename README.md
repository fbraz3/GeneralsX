[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/fbraz3/GeneralsGameCode)
[![GeneralsX CI](https://github.com/fbraz3/GeneralsX/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/fbraz3/GeneralsX/actions/workflows/ci.yml)
[![GitHub Release](https://img.shields.io/github/v/release/fbraz3/GeneralsX?include_prereleases&sort=date&display_name=tag&style=flat&label=Release)](https://github.com/fbraz3/GeneralsX/releases)

# GeneralsX - Cross-Platform Command & Conquer: Generals

A community-driven cross-platform port of Command & Conquer: Generals and Zero Hour, enabling the classic RTS to run natively on **Linux, macOS, and Windows** under a single modern codebase: **SDL3** (windowing/input) + **DXVK** (DirectX 8 → Vulkan graphics) + **OpenAL** (audio) + **64-bit**.

**Current Status**:

- Linux build: Functional with [a few issues](https://github.com/fbraz3/GeneralsX/issues?q=is%3Aissue%20state%3Aopen%20label%3ALinux)
- macOS build: Functional with [minor issues](https://github.com/fbraz3/GeneralsX/issues?q=is%3Aissue%20state%3Aopen%20label%3AmacOS)
- Windows build: [Planned](https://github.com/fbraz3/GeneralsX/issues/29)

## Project Goals

This repository targets a **single codebase** that builds and runs on all three major desktop platforms, replacing the original Windows-only DirectX 8 / Miles Sound System stack with portable open-source equivalents.

To stay up to date on project status, visit our [Dev Blog](docs/DEV_BLOG/).

## How does this project differ from TheSuperHackers' work?

The excellent work from TheSuperHackers focuses on stability and bug fixes while maintaining compatibility with original game binaries. Our goal is to deliver cross-platform support, even when that may break retail compatibility.

## Where does the GeneralsX name come from?

There are two reasons for this name:

1. X = Cross - reflects the cross-platform efforts
2. I am a big fan of the Mega Man X franchise, so this is also a tribute to that classic series.

## How to download

For **official releases and instructions**, visit:

* [GeneralsX Releases](https://github.com/fbraz3/GeneralsX/releases)
* [TheSuperHackers Releases (Windows only)](https://github.com/TheSuperHackers/GeneralsGameCode/releases)

## 💖 Support This Project

Cross-platform game development requires significant time, resources, and technical expertise. If GeneralsX has been valuable to you or the Command & Conquer community, consider supporting continued development:

- **[Sponsor on GitHub](https://github.com/sponsors/fbraz3)**

Your support helps with:

- **Development Time** - Hundreds of hours invested in cross-platform porting
- **Testing Infrastructure** - Multiple platforms, hardware configurations, and tools
- **Documentation** - Comprehensive guides and technical resources
- **Community Support** - Maintaining issues, discussions, and contributions

*Every contribution, no matter the size, makes a difference in keeping classic games alive across all platforms!*

## 🔨 Building from Source

### Linux

Native ELF builds with minor issues.

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

### Windows (Modern SDL3/DXVK)

Modern 64-bit Windows build using the same SDL3 + DXVK + OpenAL stack. Separate branch - **Planned**

### macOS (Apple Silicon)

Native macOS builds with minor issues.

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

### Runtime Asset Paths (Packaged Layouts)

GeneralsX supports running binaries and game assets from different directories. This is useful for package/distribution formats like `.deb`, `.rpm`, and `.app` bundles.

Asset root resolution order:

1. Environment variables
2. `Options.ini`
3. Default install path resolution (registry/platform fallback)
4. Current working directory (last fallback)

Environment variables:

- `CNC_GENERALS_PATH`: Base Generals asset root
- `CNC_GENERALS_ZH_PATH`: Zero Hour asset root

Optional `Options.ini` overrides:

```ini
[Paths]
AssetPath=/path/to/assets/current-game
GeneralsAssetPath=/path/to/assets/base-generals
```

Notes:

- `AssetPath` is used for the current game executable.
- `GeneralsAssetPath` is used by Zero Hour when it needs base Generals data.
- Environment variables have higher priority than `Options.ini`.

### 🐛 Known Issues & Limitations

For documented limitations and known bugs, check the [issues page](https://github.com/fbraz3/GeneralsX/issues).

### ⚙️ Build Requirements

The repository uses a vcpkg manifest (`vcpkg.json`) paired with a lockfile (`vcpkg-lock.json`). Key dependencies:

- **SDL3** - Cross-platform windowing and input handling (all platforms)
- **DXVK** - DirectX 8 → Vulkan translation layer (all platforms)
- **Vulkan SDK** - Modern graphics rendering backend
- **OpenAL** - Cross-platform audio system (replacing Miles Sound System)
- **CMake** - Build system with per-platform presets

For dependency management details, see [vcpkg.json](vcpkg.json).

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

- **Runtime Testing** - Validate Linux and macOS binary smoke tests and gameplay
- **Cross-Platform Testing** - Validate functionality across Linux distributions and macOS versions
- **macOS Port** - ARM64 native working, Phase 2 audio pending
- **Windows Modern Port** - SDL3 + DXVK + OpenAL 64-bit build (TBD)
- **Performance Optimization** - Identify and fix bottlenecks
- **Documentation** - Improve build guides and technical resources

**How to Contribute**:

1. Check current issues and GitHub discussions
2. Read platform-specific build guides ([Windows](docs/ETC/), [macOS](docs/ETC/MACOS_BUILD_INSTRUCTIONS.md), [Linux](docs/ETC/LINUX_BUILD_INSTRUCTIONS.md))
3. Follow [CONTRIBUTING.md](CONTRIBUTING.md) guidelines
4. Submit issues or pull requests with detailed information

## 🙏 Special Thanks

- **[Xezon](https://github.com/xezon)** and contributors for maintaining the GeneralsGameCode project
- **[Fighter19](https://github.com/Fighter19)** for developing the SDL3 and OpenAL solution that inspired this project
- **Westwood Studios** for creating the legendary Command & Conquer series
- **EA Games** for Command & Conquer: Generals, which continues to inspire gaming communities
- **All contributors and sponsors** helping to make this game truly cross-platform and accessible worldwide

## 📄 License

See the [LICENSE](./LICENSE.md) file for details.

EA has not endorsed and does not support this product. All trademarks are the property of their respective owners.