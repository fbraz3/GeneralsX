# HOWTO: Investigating and Reporting Crashes & Issues on Linux

When Command & Conquer: Generals or Zero Hour (GeneralsX) unexpectedly quits, exits immediately on startup without an error window, or crashes with a segmentation fault (`SIGSEGV`), Linux captures diagnostic logs and stack traces that are critical for identifying the problem.

This guide covers how to troubleshoot silent Flatpak exits, capture terminal logs, obtain backtraces using `gdb`, check system coredumps, and report actionable bug reports to the developers.

---

## 1. Flatpak: Troubleshooting Silent Exits

If you installed GeneralsX via Flatpak and clicking the icon in your desktop launcher (GNOME, KDE, etc.) causes the application to immediately close without displaying an error window, **this is usually not a crash, but an unfulfilled startup prerequisite**.

When applications are launched from desktop menus, standard output (`stdout`) and error streams (`stderr`) are suppressed by the desktop environment.

### Step 1: Run the Flatpak from the Terminal

Open your terminal and run the Flatpak command directly:

```bash
# For Zero Hour:
flatpak run com.fbraz3.GeneralsXZH

# For the base game:
flatpak run com.fbraz3.GeneralsX
```

### Step 2: Check for Missing Retail Game Assets

If the terminal output displays:
```text
ERROR: Could not find game data directory inside Flatpak sandbox.
```

GeneralsX is a native engine replacement and requires the original retail game data files (Big files, INI, maps, etc.) to run.

The Flatpak sandbox is pre-configured with filesystem permissions for the following host paths:
- **Zero Hour**: `~/GeneralsX/GeneralsZH/` or `~/.local/share/GeneralsX/GeneralsZH/`
- **Base Game**: `~/GeneralsX/Generals/` or `~/.local/share/GeneralsX/Generals/`

Make sure your retail game data is placed in one of these directories. See the [Getting the Game Files](GETTING_THE_GAME_FILES.md) guide for details.

---

## 2. Interactive Inspection Inside the Flatpak Sandbox

You can open an interactive Bash shell inside the exact container environment to inspect files, permissions, and environment variables:

```bash
flatpak run --command=bash com.fbraz3.GeneralsXZH
```

Inside the sandbox shell:
```bash
# Check detected install path
echo "$CNC_GENERALS_INSTALLPATH"

# Verify that game assets are visible
ls -la "$CNC_GENERALS_INSTALLPATH"

# Run the binary directly to see raw output
/app/bin/GeneralsXZH
```

To exit the sandbox shell, type `exit`.

---

## 3. Capturing a Backtrace with GDB Inside Flatpak

If the game crashes with a segmentation fault (`SIGSEGV` / `CrashDump`), capturing a backtrace with **GDB** pinpoints the exact line of code that failed.

### 1. Install the Freedesktop SDK (if not already installed)
GDB is included with the Flatpak SDK:
```bash
flatpak install org.freedesktop.Sdk//25.08
```

### 2. Run with GDB Attached
Run the Flatpak using the `--devel` flag:
```bash
flatpak run --devel --command=gdb com.fbraz3.GeneralsXZH
```

### 3. Start Execution
Inside the GDB prompt, start the game:
```gdb
(gdb) run
```

### 4. Capture the Call Stack
Reproduce the issue until the game crashes. GDB will intercept the signal and pause execution. Type:
```gdb
(gdb) bt full
```

To capture backtraces across all running threads (render, audio, logic):
```gdb
(gdb) thread apply all bt
```

Copy the terminal output to attach to your bug report.

---

## 4. Native Builds & System Coredump Inspection

If you compiled GeneralsX natively from source, you can capture crash logs directly from the system.

### Option A: Using `coredumpctl` (Systemd)

On modern Linux distributions running systemd (Ubuntu, Fedora, Arch, Debian), crashes automatically generate a core dump.

1. List recent crashes:
   ```bash
   coredumpctl list GeneralsXZH
   ```
2. View detailed crash information and stack trace:
   ```bash
   coredumpctl info
   ```
3. Open the crashed state directly in GDB:
   ```bash
   coredumpctl gdb
   ```
   Then run `bt full` inside GDB.

### Option B: Running Natively Under GDB

```bash
# Navigate to binary location
cd build/linux64-deploy/GeneralsMD

# Start GDB
gdb --args ./GeneralsXZH -win

# Run inside GDB
(gdb) run

# When it crashes
(gdb) bt full
```

---

## 5. Graphics & Vulkan (DXVK) Diagnostics

GeneralsX uses **DXVK** to translate DirectX 8 calls to native **Vulkan**. If the game crashes on startup before the window appears, check your Vulkan drivers:

1. **Verify Vulkan driver support**:
   ```bash
   vulkaninfo --summary
   ```
   Ensure your discrete or integrated GPU is listed and using official drivers (e.g. Mesa RADV, Intel ANV, or NVIDIA proprietary).

2. **Check DXVK logs**:
   DXVK automatically writes a log file in the game directory:
   - Zero Hour: `GeneralsXZH_d3d8.log`
   - Base Game: `GeneralsX_d3d8.log`

3. **Wayland vs X11 Display Server**:
   If running under Wayland and experiencing window creation issues, test with the X11 backend:
   ```bash
   # Flatpak:
   SDL_VIDEODRIVER=x11 flatpak run com.fbraz3.GeneralsXZH

   # Native:
   SDL_VIDEODRIVER=x11 ./GeneralsXZH -win
   ```

---

## 6. How to Submit a Linux Bug Report

When reporting a Linux crash or launch issue on GitHub:

1. **Title:** Include platform and symptom (e.g., `[Linux] Crash on startup in Vulkan swapchain` or `[Flatpak] Immediate exit after splash screen`).
2. **Environment Information:**
   - **Distribution:** e.g., Ubuntu 24.04 LTS / Arch Linux / Fedora 41
   - **GPU & Driver:** e.g., AMD Radeon RX 6700 XT (Mesa 24.1) / NVIDIA RTX 3080 (Driver 550.78)
   - **Display Server:** Wayland or X11
   - **Installation Method:** Flatpak or Native Source Build (Commit hash)
3. **Terminal Output / GDB Backtrace:** Paste the logs inside a markdown details block:
   ````markdown
   <details>
   <summary>Terminal Log / GDB Backtrace</summary>

   ```text
   PASTE_LOGS_HERE
   ```
   </details>
   ````
4. **DXVK Log:** Attach `GeneralsXZH_d3d8.log` if the issue is graphical or launch-related.
