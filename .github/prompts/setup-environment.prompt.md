---
name: Setup-GeneralsX-Environment
description: Complete Windows VC6 development environment setup for GeneralsX game engine port. Focuses on VC6 Portable, CMake/Ninja, and making the VC6 Configure task succeed.
argument-hint: "[vc6|configure|build|deploy|run|debug]"
agent: agent
---

## Your Goals (Success Criteria)


### Objective 1: Setup a complete Windows VC6 development environment for GeneralsX.

The final goal for this is to ensure the VS Code task:

`Configure (Windows VC6)`

runs to completion with **zero errors**.

If the user says "setup environment", prioritize making that Configure task succeed first. Building/running the game is secondary and should only be attempted after Configure is clean.

### Objective 2: Setup CDB.exe (Console Debugger) for automated crash analysis.

We want to enable automated crash analysis and debugging via CDB.exe (Console Debugger).

cdb.exe is part of Windows Driver Kit (WDK) and usually resides under these directories:
- `C:\Program Files (x86)\Windows Kits\10\Debuggers\x86`
- `C:\Program Files (x86)\Windows Kits\10\Debuggers\x64`

the easiest way to install it is via Visual Studio Installer with the "Debugging Tools for Windows" component.

we also have an automated script to install it:
`.\scripts\install_cdb.ps1` and a config file at `assets\.vsconfig` that pre-selects the required components.

You need to ensure the executable is installed and available in the system PATH.

## Preferred Automation (VS Code Tasks)

Prefer using the repository's preconfigured VS Code tasks, because they already:
- set up the VC6 environment (via `scripts/setup_vc6_env.ps1`)
- pick the correct CMake preset (`vc6`) and build folder
- log output to files under `logs/`

Suggested task flow:
1. `Configure (Windows VC6)`
2. `Build GeneralsXZH (Windows VC6)`
3. `Deploy GeneralsXZH (Windows)`
4. `Run GeneralsXZH (Windows)`

For crash investigation:
- **CDB.exe (CLI Debugger)**: `Crash Analysis GeneralsXZH (CDB)` → logs to `logs/cdbTerminal_vc6.log`
- **(Optional) WinDbg GUI**: `Debug GeneralsXZH (WinDbg GUI)` → logs to `logs/debugTerminal_vc6.log`

### Available Debug Tasks

| Task Name | Debugger | Type | Output Log | Use Case |
|-----------|----------|------|------------|----------|
| `Crash Analysis GeneralsXZH (CDB)` | CDB.exe | CLI | `logs/cdbTerminal_vc6.log` | **Primary** - Automated crash analysis, LLM-friendly output |
| `Crash Analysis GeneralsX (CDB)` | CDB.exe | CLI | `logs/cdbTerminal_generalsv.log` | **Primary** - Automated crash analysis for base game |
| `Debug GeneralsXZH (WinDbg GUI)` | WinDbg | GUI | `logs/debugTerminal_vc6.log` | Interactive debugging (optional) |
| `Debug GeneralsX (WinDbg GUI)` | WinDbg | GUI | `logs/debugTerminal_generalsv.log` | Interactive debugging for base game (optional) |

## Configure Task Output

The Configure task writes a log to:

`logs/configure_vc6.log`

When diagnosing failures, ask for that log (or guide the user to open it) and focus fixes on the first real error (not downstream cascades).

If `Configure (Windows VC6)` fails, use this quick checklist:

1. Open `logs/configure_vc6.log`
2. Find the **first** occurrence of `CMake Error` (later ones are usually cascading)
3. Fix that root cause, then re-run `Configure (Windows VC6)`

Helpful PowerShell snippets:

```powershell
# Show the last ~200 lines (often includes the first failure summary)
Get-Content logs\configure_vc6.log -Tail 200

# Find the first CMake error line
Select-String -Path logs\configure_vc6.log -Pattern "CMake Error" | Select-Object -First 1

# Show context around the first CMake error line
$m = Select-String -Path logs\configure_vc6.log -Pattern "CMake Error" | Select-Object -First 1
if ($m) { Get-Content logs\configure_vc6.log | Select-Object -Skip ([Math]::Max(0, $m.LineNumber - 25)) -First 60 }
```

## Environment Specifications

- **Target Platform**: Windows 32-bit (x86)
- **Compiler Toolchain**: Visual C++ 6 (VC6) via VC6 Portable (REQUIRED)
- **Build System**: CMake 3.25+ with Ninja 1.11+ (REQUIRED)
- **Debugger**: CDB.exe (Console Debugger) for automated crash analysis
- **CMake Preset**: `vc6` (Release, 32-bit)
- **VC6 Install Path (default)**: `C:\VC6\VC6SP6` (expected by `scripts/setup_vc6_env.ps1`)

### Installation Priority

1. **REQUIRED (Steps 1-3)**: VC6, CMake, Ninja
2. **REQUIRED (Step 4)**: Debugging Tools for Windows (CDB.exe) - required for Objective 2 (automated crash analysis)
3. **REQUIRED (Step 5)**: CMake Configuration
4. **REQUIRED (Step 6)**: Build
5. **RECOMMENDED (Steps 7-9)**: Deploy and run the game

## Setup Workflow

### Step 1: Install VC6 Portable (one-time)

The repository provides an automated installer for VC6 Portable.

```powershell
cd c:\Users\<YourUsername>\Projects\GeneralsX
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process
.\scripts\setup_vc6_portable.ps1
```

**Verification:**
```powershell
Test-Path 'C:\VC6\VC6SP6\VC98\BIN\cl.exe'
```

### Step 2: Install CMake and Ninja

```powershell
# CMake 3.25+ (recommended 3.31+) from:
# https://github.com/Kitware/CMake/releases
cmake --version

# Verify Ninja is available
ninja --version
```

### Step 3: Verify the repo tooling

```powershell
cd c:\Users\<YourUsername>\Projects\GeneralsX
.\scripts\verify_environment.ps1
```

If the verify script reports VC6 missing, run Step 1 again.

Optional: vcpkg is only needed if you choose a vcpkg-based preset (e.g., `vc6-vcpkg`). The primary VC6 workflow (`vc6` preset + `Configure (Windows VC6)` task) does not require vcpkg.

### Step 4: Install Debugging Tools for Windows (including CDB.exe)

CDB.exe (Console Debugger) is **required** to satisfy Objective 2 (automated crash analysis). If you only care about Objective 1 (build + run), you can skip it.

#### Automated Installation (Recommended - using Visual Studio Installer)

The repository includes a pre-configured `.vsconfig` file that automates the installation of Debugging Tools for Windows via Visual Studio Installer.

Run the CDB installation script:

```powershell
cd c:\Users\<YourUsername>\Projects\GeneralsX
.\scripts\install_cdb.ps1
```

The script will:
1. Check if CDB.exe is already installed
2. Locate the Visual Studio Installer
3. Use the pre-configured `assets\.vsconfig` to install Debugging Tools for Windows (via Windows Driver Kit)
4. Automatically add the debuggers directory (`C:\Program Files (x86)\Windows Kits\10\Debuggers\`) to your system PATH
5. Verify the installation

**What gets installed:**
- **Windows Driver Kit (WDK)** - Contains Debugging Tools for Windows
- Includes: CDB.exe (Console Debugger), WinDbg (GUI debugger), and other debugging utilities
- Installation location: `C:\Program Files (x86)\Windows Kits\10\` (default)

**Installation time:** 5-15 minutes (depending on existing Visual Studio installation)

**Verify installation:**
```powershell
Get-Command cdb.exe
```

Should return:
```
CommandType     Name                                               Version    Source
-----------     ----                                               -------    ------
Application     cdb.exe                                            <version>  C:\Program Files (x86)\Windows Kits\10\Debuggers\[arch]\cdb.exe
```

#### Manual Installation (if script fails)

**Option A: Visual Studio Installer (Recommended)**
1. Download Visual Studio: [https://visualstudio.microsoft.com/downloads/](https://visualstudio.microsoft.com/downloads/)
2. Run the Visual Studio Installer
3. Click **Modify** (if VS is already installed) or **Install**
4. Search for "Windows Driver Kit" or "Debugging Tools for Windows" in the component list
5. Select the component and click **Modify** / **Install**
6. Wait for installation to complete
7. Close installer and run the `install_cdb.ps1` script again to verify and add PATH

**Option B: Windows SDK with Feature Selection**
1. Visit: [Windows SDK Downloads](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/)
2. Download and run the installer
3. **IMPORTANT**: In the feature list, select **"Debugging Tools for Windows"** (can uncheck others to save space)
4. Complete installation to default location (`C:\Program Files (x86)\Windows Kits\10\`)
5. Run `install_cdb.ps1` to verify and add PATH

**Option C: Standalone Debugging Tools**
1. Visit: [Debugging Tools for Windows](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-download-tools)
2. Click "Download Debugging Tools for Windows"
3. Run installer and follow wizard
4. Accept default installation location
5. Run `install_cdb.ps1` to verify and add PATH

After manual installation, the script should automatically add the debuggers directory to your system PATH. If PATH was not updated, verify manually:

```powershell
# Check if debuggers directory is in PATH
$env:PATH -split ';' | Select-String "Windows Kits"

# If not found, add manually (requires admin):
# Settings → System → Advanced system settings → Environment variables
# Add: C:\Program Files (x86)\Windows Kits\10\Debuggers\x86
#      (or x64 for 64-bit systems)
```

**Why CDB.exe?**
- **100% CLI**: No GUI dependencies, perfect for automation
- **LLM-friendly**: Structured output, easy to parse programmatically
- **Scriptable**: Run commands via `-c` flag for automated crash analysis
- **Included**: Comes with Debugging Tools (WinDbg GUI may also be installed, but CDB.exe is the CLI tool we use)
- **Automatic PATH**: `install_cdb.ps1` automatically adds debuggers directory to system PATH

### Step 5: Configure CMake Project (VC6)

If the user is using VS Code, prefer running the task:
- `Configure (Windows VC6)`

Do not move on until Configure succeeds.

Optional (sanity checks):
```powershell
.\scripts\verify_environment.ps1
```

**Expected output includes:**
- SDL2 configured successfully
- OpenAL configured successfully
- CMake configuration SUCCESSFUL

### Step 6: Build the Project

```powershell
# Preferred: use the VS Code task `Build GeneralsXZH (Windows VC6)`.
# Manual build (if needed):
cd c:\Users\<YourUsername>\Projects\GeneralsX
.\scripts\build_zh.ps1 -Preset vc6
```

**Expected duration**: 30-60 minutes (first build)
**Output**: `build\vc6\GeneralsMD\GeneralsXZH.exe`
**Log**: `logs\build_generalszh.log` (when using the VS Code task)

### Step 7: Deploy Executables

Preferred: use the VS Code task `Deploy GeneralsXZH (Windows)`.

### Step 8: Configure Game Assets

```powershell
# Option A: Copy from original game
$OriginalGame = 'C:\Program Files (x86)\Steam\steamapps\common\Command and Conquer The Ultimate Collection\Command and Conquer Generals Zero Hour\Data'
Copy-Item $OriginalGame "$env:USERPROFILE\GeneralsX\GeneralsMD\Data" -Recurse -Force

# Option B: Symlink original (saves space)
New-Item -ItemType SymbolicLink -Path "$env:USERPROFILE\GeneralsX\GeneralsMD\Data" -Target $OriginalGame -Force
```

### Step 9: Run the Game

```powershell
cd "$env:USERPROFILE\GeneralsX\GeneralsMD"
.\GeneralsXZH.exe -win -noshellmap
```

**Command-line parameters:**
- `-win`: Windowed mode (recommended for debugging)
- `-noshellmap`: Skip shell map (faster startup)
- `-fullscreen`: Force fullscreen
- `-mod <path>`: Load mod from directory or .big file

## Troubleshooting

### "VC6 compiler not found"
- **Cause**: VC6 Portable is not installed at the default path, or the toolchain env is not set.
- **Fix (Automated)**: Run `.\scripts\setup_vc6_portable.ps1`
  - Downloads and installs VC6 Portable to `C:\VC6\VC6SP6\`
  - Sets up temporary environment variables
  - Takes 5-10 minutes to download, 1-2 minutes to extract
- **Verify**: `Test-Path 'C:\VC6\VC6SP6\VC98\BIN\cl.exe'`
- **Note**: Required before running Configure task

### "SDL2 not found"
- **Cause**: Dependency discovery failed (often due to an incomplete dependency setup or the wrong preset).
- **Fix**:
  - For the primary VC6 flow, first ensure `Configure (Windows VC6)` is being used and review `logs/configure_vc6.log` for the first real error.
  - If you intentionally use a vcpkg preset (e.g., `vc6-vcpkg`), ensure `VCPKG_ROOT` is set and dependencies are installed for the correct triplet.

### "cdb.exe not found"
- **Cause**: Debugging Tools for Windows not installed
- **Fix (Automated)**: Run `.\scripts\install_cdb.ps1`
  - Locates Visual Studio Installer
  - Uses pre-configured `assets\.vsconfig` to install Debugging Tools for Windows
  - Automatically adds debuggers directory to system PATH
  - Provides manual installation instructions if Visual Studio Installer is not found
- **Manual (Option A - Visual Studio Installer)**: [Visual Studio Downloads](https://visualstudio.microsoft.com/downloads/)
  - Open Visual Studio Installer, click Modify
  - Search for "Windows Driver Kit" or "Debugging Tools for Windows"
  - Select and install the component
  - Then run `install_cdb.ps1` to verify and add PATH
- **Manual (Option B - Windows SDK)**: [Windows SDK Download](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/)
  - During install, select "Debugging Tools for Windows"
  - Install to default location, then run `install_cdb.ps1`
- **Manual (Option C - Standalone)**: [Debugging Tools for Windows Download](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-download-tools)
  - Run installer and accept default location
  - Then run `install_cdb.ps1`
- **Verify**: `Get-Command cdb.exe`
- **Add to PATH (manual)**: If script cannot add to system PATH (requires admin):
  - Settings → System → Advanced system settings → Environment variables
  - Add `C:\Program Files (x86)\Windows Kits\10\Debuggers\x86` to PATH (or x64 for 64-bit)
- **How to Use CDB.exe**:
  - **Via VS Code task** (recommended): Run `Crash Analysis GeneralsXZH (CDB)` → output logged to `logs/cdbTerminal_vc6.log`
  - **Manual CLI** (for LLM integration):
    ```powershell
    # Capture stack trace on crash
    cdb.exe -c "g; kb; q" GeneralsXZH.exe -win

    # Set breakpoint and run
    cdb.exe -c "bp GameMemory::isValidMemoryPointer; g" GeneralsXZH.exe -win

    # Dump memory region
    cdb.exe -c "dd 0xaddress; q" GeneralsXZH.exe
    ```
- **Note**:
  - **Required** for Objective 2 (automated crash analysis)
  - **Optional** for Objective 1 (build + run)
  - Can build and test without it, then install later if needed

### Build fails with "fatal error C1083: Cannot open include file"
- **Fix**: Clear cache and reconfigure
```powershell
Remove-Item -Recurse -Force build\vc6
cmake --preset vc6
```

## Quick Reference Commands

```powershell
# Configure (manual)
cmake --preset vc6

# Build ZH (primary target, manual)
.\scripts\build_zh.ps1 -Preset vc6

# Deploy (preferred)
# Use the VS Code task `Deploy GeneralsXZH (Windows)`

# Run
cd $env:USERPROFILE\GeneralsX\GeneralsMD; .\GeneralsXZH.exe -win -noshellmap

# Check configure log
Get-Content logs\configure_vc6.log -Tail 80

# === CDB.exe (Console Debugger - CLI) ===
# Use VS Code task: "Crash Analysis GeneralsXZH (CDB)"
# Or run manually:

# Verify CDB.exe is available
Get-Command cdb.exe

# Run executable and capture stack trace on crash
cdb.exe -c "g; kb; q" "$env:USERPROFILE\GeneralsX\GeneralsMD\GeneralsXZH.exe" -win

# Set breakpoint at specific function and run
cdb.exe -c "bp D3D8Wrapper::Render; g" "$env:USERPROFILE\GeneralsX\GeneralsMD\GeneralsXZH.exe" -win

# Dump memory at address
cdb.exe -c "dd 0x12345678; q" GeneralsXZH.exe

# === WinDbg (GUI Debugger - Optional) ===
# Use VS Code task: "Debug GeneralsXZH (WinDbg GUI)"
# Or run manually:
windbg.exe "$env:USERPROFILE\GeneralsX\GeneralsMD\GeneralsXZH.exe" -win
```

## Key Notes

- **First build takes 30-60 minutes**: Parallel with 4 jobs enabled
- **GeneralsXZH is primary**: Zero Hour expansion (stable/tested)
- **vcpkg_installed/ is gitignored**: Build artifacts (5GB+) not committed
- **Windowed mode crucial**: Prevents fullscreen lockups during crashes
- **Debug/Analysis Tasks**:
  - **CDB.exe (Console Debugger - CLI)** - Recommended for Objective 2
    - Task: `Crash Analysis GeneralsXZH (CDB)` → `logs/cdbTerminal_vc6.log`
    - 100% CLI-based, LLM-friendly output, structured parsing
    - Install via `.\scripts\install_cdb.ps1` (uses Visual Studio Installer + .vsconfig)
  - **WinDbg (GUI Debugger)** - Optional for interactive debugging
    - Task: `Debug GeneralsXZH (WinDbg GUI)` → `logs/debugTerminal_vc6.log`
    - Included with Debugging Tools for Windows (installed by install_cdb.ps1)
    - Useful for manual step-through debugging
- **PowerShell only**: Use `pwsh` terminal, not cmd.exe
