# GeneralsX - Windows Local Development Setup

> **Status**: Complete guide for Windows development using Visual Studio 2022 / MSVC Build Tools (win32 preset).
> **Last Updated**: January 2026
> **Platform**: Windows 10/11, x86_64

---

## Overview

This guide walks through setting up a **complete local development environment** on Windows for building GeneralsX with **Visual Studio 2022** and modern C++20 toolchain. The process adapts the automated GitHub Actions pipeline for local machines.

**Key Features:**
- ✅ Visual Studio 2022 with CMake integration
- ✅ Automated .vsconfig component installation
- ✅ vcpkg dependency management
- ✅ CMake + Ninja build system
- ✅ Full local build & run workflow

---

## Prerequisites

### System Requirements

| Requirement | Minimum | Recommended |
|-------------|---------|-------------|
| OS | Windows 10 Build 19041+ | Windows 11 |
| Architecture | x86_64 (ARM64 partial support) | x86_64 |
| Disk Space | 60 GB free | 100+ GB free |
| RAM | 16 GB | 32+ GB |
| Network | 5 Mbps (downloads) | 25+ Mbps |

### Why These Specs?

- **60 GB minimum**: 20 GB Visual Studio 2022 + 15 GB build artifacts + 30 GB game assets (.big files) + 5 GB vcpkg cache
- **16+ GB RAM recommended**: Parallel compilation (-j 4 uses ~2 GB per job; Visual Studio uses additional memory)
- **Windows 10 Build 19041+ required**: Full MSVC2022 and CMake support

---

## Step 1: Install Development Tools

### 1.1: Git for Windows

Download and install from [git-scm.com](https://git-scm.com/download/win):

```powershell
# Verify installation
git --version
# Expected: git version 2.40.0 (or higher)
```

**Recommended options during installation:**
- Use Git from the command line and also from 3rd-party software
- Use the native Windows Secure Channel Library
- Use Windows' default console window
- Enable file caching
- Enable Git Credential Manager

### 1.2: CMake (Build System)

Download from [cmake.org](https://cmake.org/download/):

```powershell
# Download CMake (3.27.0 or higher)
# OR install via Chocolatey:
choco install cmake

# Verify installation
cmake --version
# Expected: cmake version 3.27.0 (or higher)

# Add to PATH if not auto-added during installation
```

### 1.3: Ninja (Build Generator)

**Option A: Download Binary**

```powershell
# Download from https://github.com/ninja-build/ninja/releases
# Extract ninja.exe to a folder in PATH, e.g., C:\tools\ninja\

# Verify
ninja --version
# Expected: 1.11.0 (or higher)
```

**Option B: Install via Chocolatey**

```powershell
choco install ninja
ninja --version
```

### 1.4: Visual Studio 2022 (Required)

Download and install from [visualstudio.microsoft.com](https://visualstudio.microsoft.com/downloads/):

**Option A: Automated Installation (Recommended)**

The project includes a `.vsconfig` file that specifies all required components:

```powershell
# Run Visual Studio Installer and import the configuration
# File > Import configuration > Select assets\.vsconfig

# Or use command line:
$vsInstaller = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vs_installer.exe'
$vsConfigPath = '.\assets\.vsconfig'
Start-Process -FilePath $vsInstaller -ArgumentList "import --config `"$vsConfigPath`"" -Wait
```

**Option B: Manual Selection**

If importing fails, select these workloads during installation:
- ✅ **Desktop development with C++**
- ✅ **Windows 10 SDK**
- ✅ **C++ CMake tools for Windows**
- ✅ **Visual C++ build tools**

Then install individual components:
- ✅ MSVC v143 C++ x64/x86 build tools (latest)
- ✅ C++ CMake tools for Windows
- ✅ vcpkg package manager

**Verify Installation**

```powershell
# Check MSVC compiler
Test-Path 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.4*\bin\Hostx64\x64\cl.exe'
# Should return: True

# Check CMake integration
cmake --version
```

---

## Step 2: Clone Repository and Initialize Submodules

```powershell
# Clone the repository
git clone https://github.com/fbraz3/GeneralsX.git
cd GeneralsX

# Initialize submodules (vcpkg, references, etc.)
git submodule update --init --recursive

# Verify (should show no uncommitted changes)
git status
```

---

## Step 3: Initialize vcpkg Dependencies

The project uses **vcpkg** for managing C++ dependencies. Initialize it before building:

```powershell
# Download and bootstrap vcpkg
git submodule update --init --recursive

# If vcpkg is not initialized, run from project root
.\vcpkg\bootstrap-vcpkg.bat

# Verify vcpkg
.\vcpkg\vcpkg.exe --version
```

**Expected output:**

```
vcpkg package management program version 2025-01-XX
```

---

## Step 4: Configure CMake Presets

### 4.1: Win32 Preset (Primary - 32-bit with Modern MSVC)

```powershell
# Configure using win32 preset
cmake --preset win32

# This creates: build\win32\
# ✅ Uses MSVC 2022 (v143) from Visual Studio
# ✅ 32-bit output (GeneralsX.exe, GeneralsXZH.exe)
# ✅ Release mode with optimizations
# ✅ vcpkg manages all C++ dependencies
```

### 4.2: Alternative Presets

| Preset | Compiler | Architecture | Build Type |
| --- | --- | --- | --- |
| `win32` | MSVC 2022 | 32-bit | Release |
| `win32-debug` | MSVC 2022 | 32-bit | Debug |
| `win32-releaselog` | MSVC 2022 | 32-bit | Release + Logging |

```powershell
# List all presets
cmake --list-presets

# List build presets
cmake --build build/vc6 --list-presets
```

---

## Step 5: Build the Game

### 5.1: Build GeneralsXZH (Zero Hour - Primary)

```powershell
# Build using win32 preset
cmake --build build/win32 --target z_generals --config Release --parallel 4 2>&1 | Tee-Object -FilePath logs/build_zh_msvc2022.log

# Expected output:
# [1/250] Compiling...
# [250/250] Linking GeneralsXZH.exe
# Build complete (30-60 minutes first time, depends on disk speed and parallelism)

# Check for errors
Select-String -Path "logs/build_zh_msvc2022.log" -Pattern "error|fatal" | Select-Object -First 10
```

### 5.2: Build Generals (Original - Secondary)

```powershell
# Build original Generals
cmake --build build/win32 --target g_generals --config Release --parallel 4 2>&1 | Tee-Object -FilePath logs/build_generals_msvc2022.log
```

### 5.3: Build with Logging Enabled

```powershell
# Configure with release + logging
cmake --preset win32-releaselog
cmake --build build/win32-releaselog --target z_generals --config Release --parallel 4

# This build includes:
# ✅ Debug logging output
# ✅ Crash information recording
# ✅ Performance profiling
```

---

## Step 6: Deploy Executable

### 6.1: Create Deployment Directory

```powershell
# Create deployment locations
New-Item -ItemType Directory -Force -Path "$env:USERPROFILE\GeneralsX\GeneralsMD" | Out-Null
New-Item -ItemType Directory -Force -Path "$env:USERPROFILE\GeneralsX\Generals" | Out-Null

# Verify
Get-Item "$env:USERPROFILE\GeneralsX"
```

### 6.2: Deploy Binaries

```powershell
# Deploy GeneralsXZH (Zero Hour)
Copy-Item -Path "build/win32/GeneralsMD/Release/GeneralsXZH.exe" `
          -Destination "$env:USERPROFILE\GeneralsX\GeneralsMD\GeneralsXZH.exe" `
          -Force -Verbose

# Deploy GeneralsX (Generals)
Copy-Item -Path "build/win32/Generals/Release/GeneralsX.exe" `
          -Destination "$env:USERPROFILE\GeneralsX\Generals\GeneralsX.exe" `
          -Force -Verbose

# Deploy vcpkg dependencies (if required)
# fmt.dll and other runtime libraries
$fmtDll = 'C:\vcpkg\buildtrees\fmt\x86-windows-rel\bin\fmt.dll'
if (-not (Test-Path $fmtDll)) { $fmtDll = 'C:\vcpkg\installed\x86-windows\bin\fmt.dll' }
if (Test-Path $fmtDll) {
    Copy-Item -Path $fmtDll -Destination "$env:USERPROFILE\GeneralsX\GeneralsMD\fmt.dll" -Force
}

# Verify
Get-ChildItem "$env:USERPROFILE\GeneralsX\GeneralsMD\*.exe"
Get-ChildItem "$env:USERPROFILE\GeneralsX\Generals\*.exe"
```

---

## Step 7: Prepare Game Assets

GeneralsX requires original game assets (`.big` files) to run. These come from the original game installation.

### 7.1: Locate Original Game Installation

```powershell
# Steam installation (Windows)
$steamPath = "C:\Program Files (x86)\Steam\steamapps\common\Command and Conquer The Ultimate Collection\Command and Conquer Generals Zero Hour\"

# Check if it exists
Test-Path $steamPath
# Should return: True

# List assets
Get-ChildItem "$steamPath\Data\*.big" | Select-Object Name
```

### 7.2: Link or Copy Assets

**Option A: Symbolic Link (Recommended - saves disk space)**

```powershell
# Navigate to deployment directory
cd "$env:USERPROFILE\GeneralsX\GeneralsMD"

# Create symlink to original game assets
New-Item -ItemType SymbolicLink `
         -Path "Data" `
         -Target "C:\Program Files (x86)\Steam\steamapps\common\Command and Conquer The Ultimate Collection\Command and Conquer Generals Zero Hour\Data" `
         -Force | Out-Null

# Verify
Get-Item "Data" -ErrorAction SilentlyContinue
```

**Option B: Copy Assets (full disk copy)**

```powershell
# If symlinks fail or you want a complete copy
Copy-Item -Path "C:\Program Files (x86)\Steam\steamapps\common\Command and Conquer The Ultimate Collection\Command and Conquer Generals Zero Hour\Data" `
          -Destination "$env:USERPROFILE\GeneralsX\GeneralsMD\Data" `
          -Recurse -Force -Verbose

# This takes 5-10 minutes depending on disk speed
```

### 7.3: Verify Assets

```powershell
# Check assets folder
Get-ChildItem "$env:USERPROFILE\GeneralsX\GeneralsMD\Data\*.big" | Measure-Object

# Expected: Multiple .big files (2GB+ total)
```

---

## Step 8: Run the Game

### 8.1: Launch from Command Line

```powershell
# Navigate to game directory
cd "$env:USERPROFILE\GeneralsX\GeneralsMD"

# Run with windowed mode (recommended for development)
.\GeneralsXZH.exe -win -noshellmap 2>&1 | Tee-Object -FilePath run.log

# Alternative: fullscreen
.\GeneralsXZH.exe -fullscreen 2>&1 | Tee-Object -FilePath run.log

# Check for crash logs
Get-ChildItem "$env:USERPROFILE\Documents\Command and Conquer Generals Zero Hour Data\*.txt" |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 5
```

### 8.2: Run via VS Code

Use the provided VS Code tasks:
1. Open Command Palette: `Ctrl+Shift+P`
2. Select task: `Run GeneralsXZH (Windows)`
3. Game launches in new terminal window

### 8.3: Command-Line Parameters

```powershell
# Common parameters
.\GeneralsXZH.exe -win                    # Windowed mode
.\GeneralsXZH.exe -fullscreen             # Fullscreen mode
.\GeneralsXZH.exe -noshellmap             # Skip shell map
.\GeneralsXZH.exe -mod "path/to/mod"      # Load custom mod

# Combine parameters
.\GeneralsXZH.exe -win -noshellmap -mod custom.big
```

---

## Step 9: Daily Development Workflow

### 9.1: Quick Rebuild

```powershell
# Fast incremental build (2-10 minutes depending on changes)
cmake --build build/win32 --target z_generals --config Release --parallel 4

# Deploy & run
Copy-Item "build/win32/GeneralsMD/Release/GeneralsXZH.exe" `
          "$env:USERPROFILE\GeneralsX\GeneralsMD\GeneralsXZH.exe" -Force
cd "$env:USERPROFILE\GeneralsX\GeneralsMD"
.\GeneralsXZH.exe -win
```

### 9.2: Clean Rebuild (if dependencies changed)

```powershell
# Full clean rebuild
Remove-Item -Path "build\win32" -Recurse -Force -ErrorAction SilentlyContinue
cmake --preset win32
cmake --build build/win32 --target z_generals --config Release --parallel 4

# This takes 30-60 minutes depending on system and parallelism
```

### 9.3: Use Parallel Compilation

MSVC2022 supports parallel compilation for faster builds:

```powershell
# Use all CPU cores
cmake --build build/win32 --target z_generals --config Release --parallel

# Or specify number of parallel jobs
cmake --build build/win32 --target z_generals --config Release --parallel 8

# Monitor build with verbose output
cmake --build build/win32 --target z_generals --config Release --parallel 4 --verbose
```

---

## Step 10: Debugging

### 10.1: Run Under Debugger

Using **WinDbg** (included with Windows SDK):

```powershell
# Launch debugger with GeneralsXZH
windbg.exe "$env:USERPROFILE\GeneralsX\GeneralsMD\GeneralsXZH.exe" -win

# Common commands in WinDbg:
# g           - Go (continue execution)
# bp func     - Set breakpoint
# bl          - List breakpoints
# .ecxr       - Show exception context
# !peb        - Show process environment block
```

### 10.2: Crash Log Analysis

```powershell
# Check crash logs
$crashDir = "$env:USERPROFILE\Documents\Command and Conquer Generals Zero Hour Data\"
Get-ChildItem "$crashDir\*.txt" | Sort-Object LastWriteTime -Descending | Select-Object -First 1

# View latest crash
Get-Content (Get-ChildItem "$crashDir\*.txt" | Sort-Object LastWriteTime -Descending | Select-Object -First 1).FullName
```

---

## Troubleshooting

### Issue: "CMake not found"

```powershell
# Solution: Add CMake to PATH
$cmakePath = "C:\Program Files\CMake\bin"
$env:PATH += ";$cmakePath"

# Verify
cmake --version
```

### Issue: "Ninja not found"

```powershell
# Solution: Add Ninja to PATH
$ninjaPath = "C:\tools\ninja"
$env:PATH += ";$ninjaPath"

# Verify
ninja --version
```

### Issue: "VC6 portable download fails"

```powershell
# Check network connectivity
Test-NetConnection github.com -Port 443

# Try manual download:
# https://github.com/itsmattkc/MSVC600/archive/001c4bafdcf2ef4b474d693acccd35a91e848f40.zip

# Verify hash manually
$hash = (Get-FileHash "VS6_VisualStudio6.zip" -Algorithm SHA256).Hash
Write-Host "Your hash: $hash"
Write-Host "Expected:  D0EE1F6DCEF7DB3AD703120D9FB4FAD49EBCA28F44372E40550348B1C00CA583"
```

### Issue: "Compilation errors with VC6"

```powershell
# Check VC6 environment variables
$env:MSVCDir
$env:INCLUDE
$env:LIB
$env:PATH

# If missing, re-run setup task or manual setup
```

### Issue: "Game crashes on startup"

```powershell
# Check crash logs
Get-ChildItem "$env:USERPROFILE\Documents\Command and Conquer Generals Zero Hour Data\" |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 3

# Run with debug logging
.\GeneralsXZH.exe -win 2>&1 | Tee-Object "debug_run.log"

# Search for errors
Select-String -Path "debug_run.log" -Pattern "error|fatal|exception"
```

### Issue: "Symbols not found" (asset loading)

```powershell
# Verify asset structure
Get-ChildItem "$env:USERPROFILE\GeneralsX\GeneralsMD\Data\" | Measure-Object

# Should show multiple .big files (2GB+ total)

# Check INI configuration files
Get-ChildItem "$env:USERPROFILE\.config\GeneralsX\" -ErrorAction SilentlyContinue

# If missing, ensure assets are linked/copied correctly
Test-Path "$env:USERPROFILE\GeneralsX\GeneralsMD\Data\art.big"
```

### Issue: \"vcpkg dependencies missing\"

```powershell
# Reinstall vcpkg dependencies
.\vcpkg\vcpkg.exe install

# Or clean and reconfigure CMake
Remove-Item -Path "build\win32\CMakeCache.txt" -Force
cmake --preset win32
```

---

## Environment Setup Verification

Run this verification script to confirm everything is ready:

```powershell
# Save as verify_setup.ps1

Write-Host "=== GeneralsX Windows Setup Verification ===" -ForegroundColor Cyan

$checks = @(
    @{ Name = "Git"; Command = "git --version" },
    @{ Name = "CMake"; Command = "cmake --version" },
    @{ Name = "Ninja"; Command = "ninja --version" },
    @{ Name = "VC6"; Path = "C:\VC6\VC6SP6\VC98\BIN\cl.exe" },
    @{ Name = "GeneralsX Repo"; Path = ".git" }
)

foreach ($check in $checks) {
    if ($check.Command) {
        try {
            $result = Invoke-Expression $check.Command 2>&1 | Select-Object -First 1
            Write-Host "✅ $($check.Name): $result" -ForegroundColor Green
        } catch {
            Write-Host "❌ $($check.Name): Not found" -ForegroundColor Red
        }
    } elseif ($check.Path) {
        if (Test-Path $check.Path) {
            Write-Host "✅ $($check.Name): Found" -ForegroundColor Green
        } else {
            Write-Host "❌ $($check.Name): Not found at $($check.Path)" -ForegroundColor Red
        }
    }
}

Write-Host "`n=== Ready to build? ===" -ForegroundColor Cyan
```

Run verification:

```powershell
.\verify_setup.ps1
```

---

## Time Estimates

| Step | Time |
| --- | --- |
| **Step 1**: Install tools (CMake/Ninja) | 5-10 min |
| **Step 2**: Clone repo + submodules | 10-15 min |
| **Step 3**: Install Visual Studio 2022 | 45-90 min |
| **Step 4**: Initialize vcpkg | 3-5 min |
| **Step 5**: CMake configure | 2-5 min |
| **Step 6**: First build | 30-60 min (high I/O, -j 4) |
| **Step 7**: Deploy | 1-2 min |
| **Step 8**: Asset setup | 10-20 min (or instant with symlink) |
| **Step 9**: First run | 2-5 min |
| **TOTAL** | **2-3 hours** |
| Subsequent builds | 5-15 min (with -j 4) |

---

## Next Steps

1. ✅ **Build**: `cmake --build build/win32 --target z_generals --config Release --parallel 4`
2. ✅ **Deploy**: Copy .exe to `$env:USERPROFILE\GeneralsX\GeneralsMD\`
3. ✅ **Run**: `.\GeneralsXZH.exe -win -noshellmap`
4. ✅ **Develop**: Edit code, rebuild, test
5. ✅ **Commit**: Push changes to GitHub

---

## References

- [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/)
- [CMake Documentation](https://cmake.org/cmake/help/latest/)
- [Ninja Build System](https://ninja-build.org/)
- [vcpkg Package Manager](https://vcpkg.io/)
- [MSVC Compiler Documentation](https://docs.microsoft.com/en-us/cpp/)
- [GeneralsX GitHub](https://github.com/fbraz3/GeneralsX)

---

## Support

**For questions or issues:**
1. Check [WINDOWS_TROUBLESHOOTING.md](WINDOWS_TROUBLESHOOTING.md) for common problems
2. Review `logs/build.log` for compilation errors
3. Check crash logs in `%USERPROFILE%\Documents\Command and Conquer Generals Zero Hour Data\`
4. Ask on [Discord](https://www.community-outpost.com/discord)

---

**Created**: January 2026
**Maintained by**: GeneralsX Development Team
**License**: GPL-3.0 (same as GeneralsX)
