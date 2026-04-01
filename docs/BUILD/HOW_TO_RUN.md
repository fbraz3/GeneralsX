# How to Run GeneralsX

This is a **beta** release. Some bugs are still expected. If you run into any problems, please [open an issue](https://github.com/fbraz3/GeneralsX/issues) so we can investigate.

## Prerequisites

1. You must own a legitimate copy of the game. We build and test against the [Steam version](https://store.steampowered.com/app/2732960/Command__Conquer_Generals_Zero_Hour/). Other retail releases may work, but are not officially supported.
2. Copy the game data from your existing installation (for example, from `C:\Program Files (x86)\Steam\steamapps\common\Command & Conquer Generals - Zero Hour`) to a local folder on your Linux or macOS system. A recommended layout is:
   - `$HOME/GeneralsX/Generals` for Command & Conquer: Generals
   - `$HOME/GeneralsX/GeneralsZH` for Command & Conquer: Generals - Zero Hour

Legacy fallback during migration is still supported:

- `$HOME/GeneralsX/GeneralsMD` for Command & Conquer: Generals - Zero Hour

## Linux

1. Download the Linux archive from this release.
2. Extract all files into your Zero Hour directory (for example, `$HOME/GeneralsX/GeneralsZH`). Overwrite existing files if prompted.
3. Some dependencies (such as DXVK) require specific environment variables. The easiest way to launch the game is to run the provided `run.sh` script from a terminal.

If your existing setup still uses `$HOME/GeneralsX/GeneralsMD`, release scripts keep compatibility with that legacy path.

## macOS

1. Download the macOS `.zip` file from this release.
2. Extract the `.zip` and copy the app bundle into your `Applications` folder.
3. Make sure your game assets are placed in the following locations:
   - `$HOME/GeneralsX/Generals` for Generals
   - `$HOME/GeneralsX/GeneralsZH` for Zero Hour
4. Because the app is not code-signed, macOS Gatekeeper will initially block it. After the first launch attempt, go to **System Settings -> Privacy & Security** and allow the application to run.

If your existing setup still uses `$HOME/GeneralsX/GeneralsMD`, release scripts keep compatibility with that legacy path.

## Requirements

GeneralsX has been developed and tested primarily on the following environments:

- Ubuntu 24.04 LTS (x86_64)
- macOS 26 "Tahoe" on Apple Silicon (M1 / ARM64)

Other Linux distributions or macOS versions may work, but you may need to install additional dependencies manually. On some systems, certain libraries might need to be built from source.

## Tested Platforms

Current development and test matrix:

| Platform           | Architecture           | Status  |
|--------------------|------------------------|---------|
| Ubuntu 24.04 LTS   | x86_64                 | Working |
| macOS 26 "Tahoe"   | ARM64 (Apple Silicon)  | Working |

Support for other platforms and configurations is possible but not yet officially tested.

## Known Issues

### Multiplayer

- LAN play has not been tested and is likely broken.
- Online features are not implemented and are only planned for the distant future.

### Linux

- Skirmish games currently result in an **instant win** as soon as the match starts, so this mode is not yet playable.
  - Campaign and Generals Challenge modes are working.
- [Zero Hour] Building and unit shadows do not render correctly. This is a visual issue only and does not affect gameplay logic.
- Stealth units and GLA stealth buildings are visible as normal units/buildings instead of being hidden.
- The sound system is mostly functional, but there are issues with long voice lines and some sound effects cutting out or not playing correctly.
- There may be additional issues that have not yet been identified.

### macOS

All items listed in the Linux section apply, **except**:

- All single-player modes, including Skirmish, Campaign, and Generals Challenge, are currently working on macOS.

## Reporting Bugs

If you encounter problems while running the game, please [open an issue](https://github.com/fbraz3/GeneralsX/issues/new/choose) and include as much detail as possible, such as:

- Operating system and version
- CPU architecture (x86_64 / ARM64)
- Steps to reproduce the issue
- Logs or terminal output (if available)

This information greatly helps us reproduce and fix issues.

## Contributing

Contributions of all sizes are welcome.

If you are interested in helping with development, bug fixes, testing on additional platforms, or improving compatibility, feel free to open a pull request or start a discussion in the repository.

Even small contributions, such as testing, documentation improvements, or well-documented bug reports, are very valuable.

## Credits

This project exists thanks to the Command & Conquer community and the many tools created around the game over the years.

Special thanks to everyone who has contributed time to testing, debugging, and development.