# How to Install GeneralsX

## Prerequisites

1. You must own a legitimate copy of the game. We build and test against the [Steam version](https://store.steampowered.com/app/2732960/Command__Conquer_Generals_Zero_Hour/). Other retail releases may work, but are not officially supported.
2. Copy the game data from your existing installation (for example, from `C:\Program Files (x86)\Steam\steamapps\common\Command & Conquer Generals - Zero Hour`) to a local folder on your Linux or macOS system. A recommended layout is:
   - `$HOME/GeneralsX/Generals` for Command & Conquer: Generals
   - `$HOME/GeneralsX/GeneralsZH` for Command & Conquer: Generals - Zero Hour

## Linux

1. Download the `.AppImage` file from this release (`GeneralsXZH-linux-x86_64.AppImage` for Zero Hour, `GeneralsX-linux-x86_64.AppImage` for the base game).
2. Open a terminal, make it executable, and run it:

   ```bash
   chmod +x GeneralsXZH-linux-x86_64.AppImage
   ./GeneralsXZH-linux-x86_64.AppImage
   ```

   Troubleshooting: AppImages commonly use FUSE to mount their embedded filesystem at launch. If direct execution fails on a minimal or sandboxed system, try:

   ```bash
   APPIMAGE_EXTRACT_AND_RUN=1 ./GeneralsXZH-linux-x86_64.AppImage
   ```

   Or extract and run manually:

   ```bash
   ./GeneralsXZH-linux-x86_64.AppImage --appimage-extract
   ./squashfs-root/AppRun
   ```

3. The AppImage auto-detects game data in the following default location:
   - `$HOME/GeneralsX/GeneralsZH`

   If your assets are stored elsewhere, set the environment variable before launching:

   ```bash
   CNC_GENERALS_ZH_PATH=/path/to/your/zero-hour-data ./GeneralsXZH-linux-x86_64.AppImage -win
   ```

   For the base game, use `CNC_GENERALS_PATH` instead.

4. All runtime libraries (DXVK, SDL3, OpenAL, FFmpeg, etc.) are bundled inside the AppImage. No additional packages need to be installed.

> **GPU Driver note**: Vulkan support must be provided by your host GPU driver. For NVIDIA use the proprietary driver, for AMD/Intel use Mesa 21+. The AppImage does not bundle GPU drivers.

## macOS

1. Download the macOS `.zip` file from this release.
2. Extract the `.zip` and copy the app bundle into your `Applications` folder.
3. Make sure your game assets are placed in the following locations:
   - `$HOME/GeneralsX/Generals` for Generals
   - `$HOME/GeneralsX/GeneralsZH` for Zero Hour
4. Because the app is not code-signed, macOS Gatekeeper will initially block it. After the first launch attempt, go to **System Settings -> Privacy & Security** and allow the application to run.

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

## Multiplayes features

- LAN play - it's broken for now and we have a [issue](https://github.com/fbraz3/GeneralsX/issues/86) to work on that.
- Online features - not implemented and planned for the future.

## Known Issues & Limitations

For documented limitations and known bugs, check the [issues page](https://github.com/fbraz3/GeneralsX/issues).

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
