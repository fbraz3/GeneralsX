[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/fbraz3/GeneralsGameCode)
[![GeneralsX CI](https://github.com/fbraz3/GeneralsX/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/fbraz3/GeneralsX/actions/workflows/ci.yml)
[![GitHub Release](https://img.shields.io/github/v/release/fbraz3/GeneralsX?include_prereleases&sort=date&display_name=tag&style=flat&label=Release)](https://github.com/fbraz3/GeneralsX/releases)

# GeneralsX - Cross-Platform Command & Conquer: Generals

GeneralsX delivers **Linux and macOS** builds of **Command & Conquer: Generals and Zero Hour** through a single modern codebase.

## How to download

For **official releases and instructions**, visit:

* [GeneralsX Releases](https://github.com/fbraz3/GeneralsX/releases)  - Linux and Mac
* [TheSuperHackers Releases](https://github.com/TheSuperHackers/GeneralsGameCode/releases) - Windows
* [Fighter19 Releases](https://github.com/Fighter19/CnC_Generals_Zero_Hour/releases) - Original Linux-focused Zero Hour reference releases

> See our [Tutorial Docs](docs/HOWTO/README.md) for step-by-step guides.

### Community Ports based on GeneralsX

* [Generals-Mac-iOS-iPad](https://github.com/ammaarreshi/Generals-Mac-iOS-iPad) - iOS port by [@ammaarreshi](https://github.com/ammaarreshi)
* [Generals-Android](https://github.com/fadi-labib/Generals-Android) - Android port by [@fadi-labib](https://github.com/fadi-labib)
* [GeneralsXWeb](https://github.com/meerzulee/GeneralsXWeb) - Web port by [@meerzulee](https://github.com/meerzulee)

## 💖 Support This Project

The optional sponsorship link exists to help cover the maintenance costs specific to GeneralsX: Linux/macOS integration, project-specific adaptation work, testing infrastructure, packaging, tooling, release work, and documentation.

- **[Sponsor on GitHub](https://github.com/sponsors/fbraz3)**

Your support specifically helps with:

- **Integration, Adaptation and Enhancements** - Merging reference work, resolving incompatibilities, and carrying project-specific fixes needed for supported platforms
- **Testing Infrastructure** - Validation across Linux and macOS, plus exploratory work needed to keep future platform paths viable
- **Packaging & Releases** - AppImage, Flatpak, macOS bundles, CI pipeline
- **Documentation & Maintenance** - Build guides, installation instructions, developer resources, and ongoing repository upkeep

## Where does the GeneralsX name come from?

There are two reasons for this name:

1. X = Cross - reflects the cross-platform efforts
2. I am a big fan of the Mega Man X franchise, so this is also a tribute to that classic series.

## Project Goals

GeneralsX exists to turn upstream preservation and porting work into a practical and maintainable project for active Linux and macOS players.

Its main goals are:

- Preserve retail gameplay behavior while modernizing the platform layer.
- Maintain a **single codebase** with Linux and macOS as the active targets. Both Zero Hour and the Generals base game are stable and functional; bugfixes and improvements must be applied to both, while keeping a future Windows path possible.
- Carry the adaptation work needed to make the stack function in practice across supported platforms, including repository-specific fixes when upstream constraints leave gaps.
- Deliver reproducible builds, packaging, and release workflows that make the port usable beyond local development setups.
- Replace the original Windows-only DirectX 8 / Miles stack with portable open-source equivalents where appropriate.
- Keep upstream lineage clear by distinguishing foundational work from the integration, packaging, and platform support specific to GeneralsX.

## How does this project relate to other community projects?

GeneralsX builds on complementary community efforts with different roles.

**TheSuperHackers** provides the main upstream foundation for stability, bug fixes, retail compatibility, and long-term maintenance of the original game code.

**Fighter19's fork**, including major work by **feliwir**, is a key Zero Hour cross-platform reference that established much of the ecosystem groundwork used here, including SDL3 windowing, DXVK-based rendering, OpenAL audio, FFmpeg media support, filesystem modernization, and related Linux-focused portability work.

While GeneralsX builds on important community work, this project also includes substantial original effort in integration, adaptation, platform-specific fixes, enhancements, testing, packaging, and ongoing maintenance.

Because these projects serve different but complementary goals, not every change belongs in the same place. Improvements aligned with upstream stability or core maintenance priorities should be contributed back to TheSuperHackers, while GeneralsX keeps changes specific to cross-platform delivery, packaging, and platform integration.

##  Building from Source

- [ Linux Build Guide](docs/BUILD/LINUX.md)
- [ macOS Build Guide](docs/BUILD/MACOS.md)

###  Known Issues & Limitations

For documented limitations and known bugs, check the [issues page](https://github.com/fbraz3/GeneralsX/issues).

---

## 🤝 How to Contribute

1. Check [current issues](https://github.com/fbraz3/GeneralsX/issues) and [GitHub discussions](https://github.com/fbraz3/GeneralsX/discussions)
2. Read platform-specific build guides ([Windows](docs/ETC/), [macOS](docs/BUILD/MACOS.md), [Linux](docs/BUILD/LINUX.md))
3. Submit issues or pull requests with detailed information

## 🙏 Special Thanks

- **[Westwood Studios](https://cnc-comm.com/westwood-studios)** for creating the legendary Command & Conquer series
- **[EA Games](https://www.ea.com/)** for Command & Conquer: Generals, which continues to inspire gaming communities
- **[TheSuperHackers / Xezon](https://github.com/TheSuperHackers/GeneralsGameCode)** and contributors for the upstream stability, bug fixes, and code modernization that form the foundation of GeneralsX
- **[Fighter19](https://github.com/Fighter19)** for the cross-platform port that pioneered SDL3 windowing, DXVK graphics, and MinGW build support on Linux
- **[feliwir](https://github.com/feliwir)** for the foundational cross-platform systems implemented in Fighter19's fork: OpenAL audio, FFmpeg video decoding, C++17 filesystem, and Freetype/Fontconfig text rendering
- **All contributors and sponsors** for helping to make this game truly cross-platform and accessible worldwide

## 📄 License

See the [LICENSE](./LICENSE.md) file for details.

EA has not endorsed and does not support this product. All trademarks are the property of their respective owners.
