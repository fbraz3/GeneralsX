# Getting the Game Files

GeneralsX is an open-source engine port — it provides the launcher and runtime, but **cannot distribute the original game assets** due to copyright. You must own a legitimate copy of *Command & Conquer: Generals - Zero Hour* (or the base game) to play.

We build and test against the [Steam version](https://store.steampowered.com/app/2732960/Command__Conquer_Generals_Zero_Hour/). Other retail releases may work, but are not officially supported.

Once you have the game files, see [INSTALLATION.md](INSTALLATION.md) for how to set up GeneralsX on your platform.

---

## The Problem: Steam Does Not Offer a macOS / Linux Download

The original game is a Windows-only title on Steam. If you open Steam on macOS or Linux, the **Install** button will not appear. You need to obtain the Windows game files through one of the methods below and then copy them to your system.

---

## Option 1 — Copy From a Windows Installation (Easiest)

If you have access to a Windows PC or a Windows virtual machine:

1. Install *C&C Generals: Zero Hour* (and/or *Generals*) via Steam on Windows.
2. Locate the installation folder, typically:
   - `C:\Program Files (x86)\Steam\steamapps\common\Command and Conquer Generals Zero Hour`
   - `C:\Program Files (x86)\Steam\steamapps\common\Command and Conquer Generals`
3. Copy the folder(s) to your Mac or Linux system via USB drive, network share, or cloud storage.
4. Place them in the expected locations:
   - `$HOME/GeneralsX/GeneralsZH` for Zero Hour
   - `$HOME/GeneralsX/Generals` for the base game

---

## Option 2 — CrossOver Trial (No Windows PC Needed)

[CrossOver](https://www.codeweavers.com/crossover) allows you to run Windows software — including the Windows Steam client — directly on macOS or Linux. It offers a **free 14-day trial**.

1. Download and install CrossOver from [codeweavers.com/crossover](https://www.codeweavers.com/crossover).
2. Inside CrossOver, create a new bottle and install the **Windows version of Steam**.
3. Log in to Steam and download *C&C Generals: Zero Hour* (and/or *Generals*).
4. Locate the installed game files inside the CrossOver bottle (usually under `~/Library/Application Support/CrossOver/Bottles/<bottle-name>/drive_c/Program Files (x86)/Steam/steamapps/common/` on macOS).
5. Copy the game folder(s) to the expected locations described above.

> **Note**: CrossOver is only needed to download the game files. You do not need it to run GeneralsX — the GeneralsX engine runs natively on macOS and Linux.

---

## Option 3 — SteamCMD (Advanced, Command-Line)

[SteamCMD](https://developer.valvesoftware.com/wiki/SteamCMD) is Valve's official headless Steam client. It can download Windows game files on any platform, including macOS and Linux.

1. Download and install SteamCMD by following the [official instructions](https://developer.valvesoftware.com/wiki/SteamCMD).
2. Run SteamCMD and log in with your Steam account:

   ```bash
   ./steamcmd.sh
   ```

   Inside the SteamCMD prompt:

   ```
   login <your_steam_username>
   force_install_dir ./zh_files
   app_update 2732960 validate
   quit
   ```

   - App ID `2732960` is *C&C Generals: Zero Hour*.
   - App ID `2732940` is *C&C Generals* (base game).

3. After the download completes, move the game folder to the expected location:
   ```bash
   mv ./zh_files "$HOME/GeneralsX/GeneralsZH"
   ```

---

## Next Steps

After placing the game files in the correct directories, follow [INSTALLATION.md](INSTALLATION.md) to download the GeneralsX release for your platform and launch the game.

If you run into any issues, please [open a discussion](https://github.com/fbraz3/GeneralsX/discussions) or [report a bug](https://github.com/fbraz3/GeneralsX/issues/new/choose).
