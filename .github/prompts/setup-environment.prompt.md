---
name: Setup GeneralsX Environment
description: Complete Windows MSVC 2022 development environment setup for GeneralsX game engine port. Covers BuildTools, CMake, vcpkg, compilation, and deployment.
argument-hint: "[step|windows|cmake|vcpkg|build|deploy|run]"
agent: agent
---

## Your Goal (Success Criteria)

The final goal of this prompt is to ensure the VS Code task:

`Configure (Windows MSVC BuildTools, win32 preset)`

runs to completion with **zero errors**.

If the user says "setup environment", prioritize making that Configure task succeed first. Building/running the game is secondary and should only be attempted after Configure is clean.

## Preferred Automation (VS Code Tasks)

Prefer using the repository's preconfigured VS Code tasks, because they already:
- load the MSVC BuildTools environment
- pick the correct CMake preset and build folder
- log output to files under `logs/`

Suggested task flow:
1. `Configure (Windows MSVC BuildTools, win32 preset)`
2. `Build GeneralsXZH (Windows MSVC2022)`
3. `Deploy GeneralsXZH (Windows)`
4. `Run GeneralsXZH (Windows MSVC2022)`

For crash investigation:
- `Debug GeneralsXZH (Windows MSVC2022)`

## Configure Task Output

The Configure task writes a log to:

`logs/configure_win32_msvc_buildtools.log`

When diagnosing failures, ask for that log (or guide the user to open it) and focus fixes on the first real error (not downstream cascades).

## Environment Specifications

- **Target Platform**: Windows 32-bit (x86)
- **Build Tools**: MSVC Build Tools 2022 (v14.50.35717)
- **Build System**: CMake 3.31+ with Ninja 1.11+
- **Package Manager**: vcpkg at `C:\vcpkg\`
- **Debugger**: WinDbg (Windows Debugger) for crash analysis
- **CMake Preset**: `win32` (Release, 32-bit)
- **Dependencies**: SDL2 2.32.10, FFmpeg 8.0.1, OpenAL-Soft 1.25.0, ZLib 1.3.1, fmt 12.1.0

## Setup Workflow

### Step 1: Install Visual Studio Build Tools 2022

1. Download from: [Visual Studio Build Tools 2022](https://visualstudio.microsoft.com/downloads/)
2. Run installer -> Select "Modify"
3. Click "..." button -> Select "Import configuration"
4. Navigate to the project VS configuration file: `assets/.vsconfig`
5. Complete installation (15-30 minutes)

**Verification:**
```powershell
Test-Path 'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools'
```

### Step 2: Install CMake and Ninja

```powershell
# CMake 3.31+ from:
# https://github.com/Kitware/CMake/releases
cmake --version

# Ninja installation via automation
cd c:\Users\<YourUsername>\Projects\GeneralsX
.\scripts\install_ninja.ps1
.\scripts\add_ninja_to_path.ps1
ninja --version
```

### Step 3: Bootstrap vcpkg

```powershell
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg.exe list --triplet x86-windows
```

### Step 4: Install WinDbg (Debugger)

WinDbg is required for crash investigation and debugging:

```powershell
cd c:\Users\<YourUsername>\Projects\GeneralsX
.\scripts\install_windbg.ps1
```

**Verify installation:**
```powershell
windbg.exe -version
```

If the script cannot auto-install, follow the manual installation:
1. Download from: [Debugging Tools for Windows](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-download-tools)
2. Run installer, accept default location
3. Verify: `Get-Command windbg.exe`

### Step 5: Configure CMake Project

```powershell
cd c:\Users\<YourUsername>\Projects\GeneralsX
.\scripts\cmake_final.ps1 -Preset win32
```

If the user is using VS Code, prefer running the task:
- `Configure (Windows MSVC BuildTools, win32 preset)`

Do not move on until Configure succeeds.

Optional (sanity checks):
```powershell
.\scripts\verify_environment.ps1
```

**Expected output includes:**
- SDL2 configured successfully
- OpenAL configured successfully
- ZLib found: version 1.3.1
- CMake configuration SUCCESSFUL

### Step 6: Build the Project

```powershell
cmake --build build\win32 --target z_generals --config Release --parallel 4
```

**Expected duration**: 30-60 minutes (first build)
**Output**: `build\win32\GeneralsMD\Release\GeneralsXZH.exe`
**Log**: `logs\build_zh_msvc2022.log`

### Step 7: Deploy Executables

```powershell
$DeployDir = "$env:USERPROFILE\GeneralsX\GeneralsMD"
New-Item -ItemType Directory -Path $DeployDir -Force | Out-Null
Copy-Item build\win32\GeneralsMD\Release\GeneralsXZH.exe $DeployDir -Force
Copy-Item C:\vcpkg\installed\x86-windows\bin\fmt.dll $DeployDir -Force
```

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

### "cl.exe not found"
- **Cause**: MSVC environment not initialized
- **Fix**: Run `.\scripts\setup_msvc_buildtools_env.ps1` before CMake

### "SDL2 not found"
- **Cause**: vcpkg SDL2 not installed or CMake search path incorrect
- **Fix**: Verify `cmake/sdl2.cmake` includes `C:/vcpkg/packages/sdl2_${VCPKG_DEFAULT_TRIPLET}`

### "windbg.exe not found"
- **Cause**: Debugging Tools for Windows not installed
- **Fix**: Run `.\scripts\install_windbg.ps1` or manually install from Microsoft
- **Manual**: [Debugging Tools for Windows Download](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-download-tools)
- **Note**: Required for Debug tasks; can still build/run without it

### Build fails with "fatal error C1083: Cannot open include file"
- **Fix**: Clear cache and reconfigure
```powershell
Remove-Item -Recurse -Force build\win32
.\scripts\cmake_final.ps1 -Preset win32
```

## Quick Reference Commands

```powershell
# Configure
.\scripts\cmake_final.ps1 -Preset win32

# Build ZH (primary target)
cmake --build build\win32 --target z_generals --config Release --parallel 4

# Build base Generals (secondary target)
cmake --build build\win32 --target g_generals --config Release --parallel 4

# Deploy
Copy-Item build\win32\GeneralsMD\Release\GeneralsXZH.exe $env:USERPROFILE\GeneralsX\GeneralsMD\ -Force

# Run
cd $env:USERPROFILE\GeneralsX\GeneralsMD; .\GeneralsXZH.exe -win -noshellmap

# Check build log
Get-Content logs\build_zh_msvc2022.log -Tail 50
```

## Key Notes

- **First build takes 30-60 minutes**: Parallel with 4 jobs enabled
- **GeneralsXZH is primary**: Zero Hour expansion (stable/tested)
- **vcpkg_installed/ is gitignored**: Build artifacts (5GB+) not committed
- **Windowed mode crucial**: Prevents fullscreen lockups during crashes
- **WinDbg is required for debugging**: Use Debug tasks for crash investigation (logs to `logs/debugTerminal_msvc2022.log`)
- **PowerShell only**: Use `pwsh` terminal, not cmd.exe
