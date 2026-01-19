---
applyTo: '**'
---

# Environment Setup Instructions (Windows MSVC 2022)

This document provides step-by-step instructions for setting up a complete GeneralsX development environment on Windows using MSVC Build Tools 2022.

## Prerequisites

- Windows 10 or later (64-bit)
- 60+ GB free disk space
- Administrator privileges (for Visual Studio Build Tools installation)
- PowerShell 5.0 or later
- Git for Windows (for version control)

## Step 1: Install Visual Studio Build Tools 2022

Visual Studio Build Tools provides the MSVC compiler needed to build GeneralsX. The project includes a `.vsconfig` file with pre-configured components.

### Option A: Using VS Installer (Recommended)

1. Download **Visual Studio Build Tools 2022** from: [Visual Studio Build Tools 2022](https://visualstudio.microsoft.com/downloads/)
2. Run the installer and select **"Modify"**
3. Click **"Continue"** to proceed
4. In the **Workload** selection screen, click the **"..."** menu button (top-right)
5. Select **"Import configuration"**
6. Navigate to `.vsconfig` file in the project root
7. Click **"Continue"** to install the pre-configured components
8. Wait for installation to complete (15-30 minutes)

### Option B: Manual Component Selection

If Option A doesn't work, manually select:

**Workloads:**
- Desktop development with C++
- Game development with C++

**Individual Components:**
- MSVC v143 - VS 2022 C++ x64/x86 build tools
- Windows 11 SDK (latest)
- C++ CMake tools for Windows

## Step 2: Install CMake and Ninja

The build system requires CMake 3.20+ and Ninja build generator.

### CMake Installation

```powershell
# Download and install CMake 3.31+ from:
# https://github.com/Kitware/CMake/releases

# Verify installation
cmake --version
```

CMake will be added to PATH during installation. Verify it works in a new PowerShell terminal.

### Ninja Installation

Run the automated installation script:

```powershell
cd c:\Users\<YourUsername>\Projects\GeneralsX
.\scripts\install_ninja.ps1
.\scripts\add_ninja_to_path.ps1
```

**What these scripts do:**
- `install_ninja.ps1`: Downloads Ninja 1.11.1 to `C:\tools\ninja\`
- `add_ninja_to_path.ps1`: Permanently adds `C:\tools\ninja\` to system PATH via registry

Verify installation:
```powershell
ninja --version
```

## Step 3: Set Up vcpkg for Dependency Management

vcpkg automatically downloads and builds required libraries (SDL2, FFmpeg, OpenAL, ZLib, fmt).

### Download and Bootstrap vcpkg

```powershell
# Clone vcpkg to system location (NOT in project folder)
cd C:\
git clone https://github.com/Microsoft/vcpkg.git

# Bootstrap vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
```

This creates `C:\vcpkg\vcpkg.exe` which CMake will automatically use via the vcpkg.cmake toolchain.

## Step 4: Configure CMake Project

The project provides an automated CMake configuration script that sets up MSVC environment and discovers all dependencies.

```powershell
cd c:\Users\<YourUsername>\Projects\GeneralsX

# Run the final CMake configuration script
.\scripts\cmake_final.ps1 -Preset win32
```

**What this script does:**
1. Sources MSVC Build Tools environment (via `setup_msvc_buildtools_env.ps1`)
2. Verifies vcpkg availability at `C:\vcpkg\`
3. Runs CMake configuration with:
   - Preset: `win32` (release build, 32-bit Windows)
   - Toolchain: vcpkg CMake toolchain
   - Triplet: x86-windows (32-bit)
4. Discovers dependencies: SDL2, FFmpeg, OpenAL, ZLib, fmt
5. Generates Visual Studio solution files in `build\win32\`

Expected output includes:
```
✅ CMake configuration SUCCESSFUL!

Next step: Build the project
  Command: cmake --build build\win32 --target z_generals --config Release --parallel 4
```

**Troubleshooting:**
- If CMake reports "cl.exe not found": Verify Visual Studio Build Tools installation completed successfully
- If SDL2 not found: Ensure `C:\vcpkg\` exists and `vcpkg.exe` is present
- If configuration hangs: Check Event Viewer for MSVC installation errors

## Step 5: Build the Project

Build the GeneralsXZH (Zero Hour) expansion, which is the primary stable target:

```powershell
cd c:\Users\<YourUsername>\Projects\GeneralsX

# Build the project
cmake --build build\win32 --target z_generals --config Release --parallel 4
```

**Expected:**
- Duration: 30-60 minutes on first build (parallel build with 4 jobs)
- Output: Two executables created:
  - `build\win32\GeneralsMD\Release\GeneralsXZH.exe` (Zero Hour)
  - `build\win32\Generals\Release\GeneralsX.exe` (Base Generals)

**Build Logs:**
- Full build output saved to: `logs\build_zh_msvc2022.log`

**If build fails:**
1. Check build log: `Get-Content logs\build_zh_msvc2022.log | Select-Object -Last 50`
2. Verify all dependencies installed: `C:\vcpkg\vcpkg.exe list --triplet x86-windows`
3. Re-run CMake configuration to refresh build files

## Step 6: Deploy Executables

Copy built executables to the deployment directory where the game runtime expects them.

```powershell
# Deploy GeneralsXZH (Zero Hour)
$ZhExe = 'c:\Users\<YourUsername>\Projects\GeneralsX\build\win32\GeneralsMD\Release\GeneralsXZH.exe'
$DeployDir = "$env:USERPROFILE\GeneralsX\GeneralsMD"

# Create deployment directory if it doesn't exist
if (-not (Test-Path $DeployDir)) {
    New-Item -ItemType Directory -Path $DeployDir -Force | Out-Null
}

# Copy executable
Copy-Item -Path $ZhExe -Destination $DeployDir -Force

# Copy runtime dependencies (fmt.dll)
$FmtDll = 'C:\vcpkg\installed\x86-windows\bin\fmt.dll'
if (Test-Path $FmtDll) {
    Copy-Item -Path $FmtDll -Destination $DeployDir -Force
}

Write-Host "Deployment complete: $DeployDir"
```

**Or use VS Code task:**
- Task: **Deploy GeneralsXZH (Windows)**
- Command: `run_task("shell: Deploy GeneralsXZH (Windows)")`

## Step 7: Configure Game Assets

GeneralsX requires original game assets (`.big` files, textures, sounds).

### Option A: Copy from Original Installation

If you have the original game installed on Steam:

```powershell
$OriginalGame = 'C:\Program Files (x86)\Steam\steamapps\common\Command and Conquer The Ultimate Collection\Command and Conquer Generals Zero Hour\'
$DeployDir = "$env:USERPROFILE\GeneralsX\GeneralsMD"

# Copy Data folder
Copy-Item -Path "$OriginalGame\Data" -Destination $DeployDir -Recurse -Force
```

### Option B: Symlink Original Installation

For development (avoids duplicating 5+ GB of assets):

```powershell
$OriginalGame = 'C:\Program Files (x86)\Steam\steamapps\common\Command and Conquer The Ultimate Collection\Command and Conquer Generals Zero Hour\Data'
$DeployDir = "$env:USERPROFILE\GeneralsX\GeneralsMD\Data"

# Create symbolic link (requires admin)
New-Item -ItemType SymbolicLink -Path $DeployDir -Target $OriginalGame -Force
```

## Step 8: Run the Game

### Run in Windowed Mode (Recommended for Development)

```powershell
cd "$env:USERPROFILE\GeneralsX\GeneralsMD"
.\GeneralsXZH.exe -win -noshellmap
```

**Command-line parameters:**
- `-win`: Windowed mode (prevents fullscreen lockup during crashes)
- `-noshellmap`: Skip shell map initialization (faster startup)
- `-fullscreen`: Force fullscreen mode
- `-mod <path>`: Load mod from directory or .big file

### Using VS Code Task

**Task:** `Run GeneralsXZH (Windows MSVC2022)`
- Automatically runs from deployment directory
- Logs output to: `logs\runTerminal_msvc2022.log`
- Recommended for monitoring crashes and debugging

## Step 9: Debug Crashes

If the game crashes:

1. **Check crash logs** in deployment directory: `%USERPROFILE%\GeneralsX\GeneralsMD\`
2. **Run under debugger** using VS Code task: `Debug GeneralsXZH (Windows MSVC2022)`
3. **Analyze WinDbg output** in: `logs\debugTerminal_msvc2022.log`

### Common Issues

**"GeneralsXZH.exe - Application Error"**
- Verify all game assets are deployed (Data folder exists)
- Check fmt.dll is copied to deployment directory
- Run with `-win -noshellmap` flags

**"SDL2 initialization failed"**
- Verify SDL2 discovery during CMake configuration
- Check `logs\build_zh_msvc2022.log` for SDL2 linking errors
- Re-run CMake configuration: `.\scripts\cmake_final.ps1 -Preset win32`

**Build fails with "fatal error C1083: Cannot open include file"**
- Clear build cache: `rm -r build\win32`
- Re-run CMake configuration
- Verify all vcpkg dependencies installed: `C:\vcpkg\vcpkg.exe list --triplet x86-windows`

## Environment Variables Reference

The build system uses these environment variables (set by scripts):

| Variable | Value | Purpose |
|----------|-------|---------|
| `VCPKG_ROOT` | `C:\vcpkg` | vcpkg installation directory |
| `VCPKG_DEFAULT_TRIPLET` | `x86-windows` | 32-bit Windows target |
| `VCPKG_TOOLCHAIN` | `C:\vcpkg\scripts\buildsystems\vcpkg.cmake` | CMake toolchain file |

## Quick Reference Commands

```powershell
# Configure project
.\scripts\cmake_final.ps1 -Preset win32

# Build GeneralsXZH (Zero Hour)
cmake --build build\win32 --target z_generals --config Release --parallel 4

# Build GeneralsX (Base game)
cmake --build build\win32 --target g_generals --config Release --parallel 4

# Deploy
Copy-Item build\win32\GeneralsMD\Release\GeneralsXZH.exe $env:USERPROFILE\GeneralsX\GeneralsMD\ -Force

# Run
cd $env:USERPROFILE\GeneralsX\GeneralsMD; .\GeneralsXZH.exe -win -noshellmap

# Check build log
Get-Content logs\build_zh_msvc2022.log | Select-Object -Last 100

# Check runtime log
Get-Content logs\runTerminal_msvc2022.log | Select-Object -Last 100
```

## Directory Structure After Setup

```
C:\
├── vcpkg/                           # vcpkg package manager
│   ├── vcpkg.exe
│   ├── installed/
│   │   └── x86-windows/             # 32-bit Windows libraries
│   │       ├── lib/                 # Static libraries
│   │       └── bin/                 # DLLs
│   └── packages/                    # Build artifacts
│       ├── sdl2_x86-windows/
│       ├── ffmpeg_x86-windows/
│       ├── openal-soft_x86-windows/
│       └── zlib_x86-windows/

C:\tools\
└── ninja/
    └── ninja.exe                    # Ninja build generator

$env:USERPROFILE\
└── GeneralsX/
    ├── GeneralsMD/                  # Zero Hour deployment
    │   ├── GeneralsXZH.exe          # Built executable
    │   ├── fmt.dll                  # Runtime dependency
    │   └── Data/                    # Game assets
    └── Generals/                    # Base game deployment
        ├── GeneralsX.exe
        ├── fmt.dll
        └── Data/
```

## Next Steps

1. **Verify everything is working**: Run game from Step 8
2. **If first-time contributor**: Read [CONTRIBUTING.md](../../CONTRIBUTING.md)
3. **Check development status**: Review [docs/DEV_BLOG/](../../docs/DEV_BLOG/) for phase progress
4. **Join development**: Start with issues marked `good-first-issue` or `help-wanted`

## Support

For environment setup issues:
- Check troubleshooting section in Step 4-5
- Review relevant build logs in `logs/` directory
- Consult [WINDOWS_SETUP_INSTRUCTIONS.md](WINDOWS_SETUP_INSTRUCTIONS.md) for additional context
- Report environment bugs with full logs and system info
