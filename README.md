[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/fbraz3/GeneralsGameCode)
[![GeneralsX CI](https://github.com/fbraz3/GeneralsX/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/fbraz3/GeneralsX/actions/workflows/ci.yml)
[![GitHub Release](https://img.shields.io/github/v/release/fbraz3/GeneralsX?include_prereleases&sort=date&display_name=tag&style=flat&label=Release)](https://github.com/fbraz3/GeneralsX/releases)

# GeneralsX - Cross-Platform Command & Conquer: Generals

A community-driven port of Command & Conquer: Generals and Zero Hour, enabling the classic RTS to run natively on **Linux, macOS, and Windows** under a single modern codebase: **SDL3** (windowing/input) + **DXVK** (DirectX 8 → Vulkan graphics) + **OpenAL** (audio) + **64-bit**.

> Note: This project is not related to any mods with similar names and does not aim to extend or modify gameplay.

## Project Goals

This repository targets a **single codebase** that builds and runs on all three major desktop platforms, replacing the original Windows-only DirectX 8 / Miles Sound System stack with portable open-source equivalents.

To stay up to date on project status, visit our [Dev Blog](docs/DEV_BLOG/).

## How to download

For **official releases and instructions**, visit:

* [GeneralsX Releases](https://github.com/fbraz3/GeneralsX/releases)  - Linux and Mac / 64bit
* [TheSuperHackers Releases](https://github.com/TheSuperHackers/GeneralsGameCode/releases) - Windows only / 32bit

## Installing the game

For release/runtime setup instructions (Linux and macOS), see:

- [docs/BUILD/INSTALLATION.md](docs/BUILD/INSTALLATION.md)

> **Don't have the game files yet?** The Steam version does not offer a macOS or Linux download. See [docs/BUILD/GETTING_THE_GAME_FILES.md](docs/BUILD/GETTING_THE_GAME_FILES.md) for three ways to obtain the original game assets (copy from Windows, CrossOver trial, or SteamCMD).

## How does this project differ from TheSuperHackers' work?

TheSuperHackers is the upstream foundation behind GeneralsX. Their project prioritizes stability, bug fixes, and compatibility with the original retail binaries, while GeneralsX focuses on a native cross-platform port for Linux, macOS, and modern Windows using SDL3, DXVK, OpenAL, and a 64-bit toolchain.

Because of that difference, not every change made here belongs upstream. Improvements that also fit TheSuperHackers' goals should be contributed there; changes that exist specifically for cross-platform support, new dependencies, or retail-breaking portability work stay in GeneralsX.

## Where does the GeneralsX name come from?

There are two reasons for this name:

1. X = Cross - reflects the cross-platform efforts
2. I am a big fan of the Mega Man X franchise, so this is also a tribute to that classic series.

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

- [🐧 Linux Build Guide](docs/BUILD/LINUX.md)
- [🍎 macOS Build Guide](docs/BUILD/MACOS.md)

### 🐛 Known Issues & Limitations

For documented limitations and known bugs, check the [issues page](https://github.com/fbraz3/GeneralsX/issues).

---

## 🤝 How to Contribute

1. Check [current issues](https://github.com/fbraz3/GeneralsX/issues) and [GitHub discussions](https://github.com/fbraz3/GeneralsX/discussions)
2. Read platform-specific build guides ([Windows](docs/ETC/), [macOS](docs/BUILD/MACOS.md), [Linux](docs/BUILD/LINUX.md))
3. Follow [CONTRIBUTING.md](CONTRIBUTING.md) guidelines
4. Submit issues or pull requests with detailed information

## 🙏 Special Thanks

- **[Westwood Studios](https://cnc-comm.com/westwood-studios)** for creating the legendary Command & Conquer series
- **[EA Games](https://www.ea.com/)** for Command & Conquer: Generals, which continues to inspire gaming communities
- **[Xezon](https://github.com/xezon)** and contributors for maintaining the GeneralsGameCode project
- **[Fighter19](https://github.com/Fighter19)** for developing the SDL3 and OpenAL solution that inspired this project
- **All contributors and sponsors** for helping to make this game truly cross-platform and accessible worldwide

## 📄 License

See the [LICENSE](./LICENSE.md) file for details.

EA has not endorsed and does not support this product. All trademarks are the property of their respective owners.
