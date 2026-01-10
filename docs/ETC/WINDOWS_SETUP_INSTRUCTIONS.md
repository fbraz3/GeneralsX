# GeneralsX - Windows Local Development Setup

> **Status**: Complete guide for Windows x86/x64 development with VC6 portable toolchain.  
> **Last Updated**: January 2026  
> **Platform**: Windows 10/11, x86_64  

---

## Overview

This guide walks through setting up a **complete local development environment** on Windows for building GeneralsX with **Visual C++ 6 Portable** and modern C++20 toolchain. The process adapts the automated GitHub Actions pipeline for local machines.

**Key Features:**
- ✅ VC6 portable (no Visual Studio installation required)
- ✅ Automated hash verification for security
- ✅ vcpkg dependency management
- ✅ CMake + Ninja build system
- ✅ Full local build & run workflow

---

## Prerequisites

### System Requirements

| Requirement | Minimum | Recommended |
|-------------|---------|-------------|
| OS | Windows 10 (Build 14393) | Windows 11 |
| Architecture | x86_64 | x86_64 |
| Disk Space | 50 GB free | 100 GB free |
| RAM | 8 GB | 16+ GB |
| Network | 2 Mbps (downloads) | 10+ Mbps |

### Why These Specs?

- **50 GB minimum**: 15 GB build artifacts + 30 GB game assets (.big files) + 5 GB tools
- **16 GB RAM recommended**: Parallel compilation (-j 4 uses ~1.5 GB per job)
- **Windows 10+ required**: Native Bash support via WSL2 (optional), PowerShell 5.0+

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

### 1.4: Visual Studio 2022 (Optional, for win32 preset)

Only needed if you want to build with modern MSVC instead of VC6:

```powershell
# Download Visual Studio 2022 Community
# https://visualstudio.microsoft.com/downloads/

# During installation, select:
# - Desktop development with C++
# - Windows 10 SDK
# - MSVC v143 toolset
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

## Step 3: Automatic VC6 Portable Setup (Recommended)

We provide **VS Code integration tasks** to automate VC6 download and verification. Open the project in VS Code and run tasks from the Command Palette (`Ctrl+Shift+P`):

```
> Tasks: Run Task > Setup VC6 Portable (Windows)
```

This task:
1. ✅ Downloads VC6 portable from itsmattkc/MSVC600
2. ✅ Verifies SHA256 hash for security
3. ✅ Extracts to `C:\VC6\VC6SP6\`
4. ✅ Sets up environment variables
5. ✅ Caches installation for faster rebuilds

**Expected output:**
```
Downloading VC6 Portable Installation...
Verifying File Integrity...
Extracting Archive...
VC6 setup completed successfully!
```

### 3.1: Manual VC6 Setup (Alternative)

If VS Code tasks are unavailable:

```powershell
# Create VC6 directory
New-Item -ItemType Directory -Force -Path "C:\VC6" | Out-Null

# Download VC6 Portable from itsmattkc repo
# Using PowerShell 7+ (recommended)
$VC6Url = "https://github.com/itsmattkc/MSVC600/archive/001c4bafdcf2ef4b474d693acccd35a91e848f40.zip"
$OutputFile = "$env:TEMP\VS6_VisualStudio6.zip"
$ExpectedHash = "D0EE1F6DCEF7DB3AD703120D9FB4FAD49EBCA28F44372E40550348B1C00CA583"

Write-Host "Downloading VC6 Portable..." -ForegroundColor Cyan
Invoke-WebRequest -Uri $VC6Url -OutFile $OutputFile

# Verify hash
$FileHash = (Get-FileHash -Path $OutputFile -Algorithm SHA256).Hash
Write-Host "Downloaded hash: $FileHash"
Write-Host "Expected hash:   $ExpectedHash"

if ($FileHash -ne $ExpectedHash) {
    Write-Error "Hash verification failed!"
    exit 1
}

# Extract
Write-Host "Extracting VC6..." -ForegroundColor Cyan
Expand-Archive -Path $OutputFile -DestinationPath "C:\VC6" -Force
Move-Item -Path "C:\VC6\MSVC600-001c4bafdcf2ef4b474d693acccd35a91e848f40" -Destination "C:\VC6\VC6SP6" -Force
Remove-Item $OutputFile

Write-Host "VC6 setup completed!" -ForegroundColor Green
```

### 3.2: Verify VC6 Installation

```powershell
# Check directory structure
Get-ChildItem "C:\VC6\VC6SP6" | Format-Table

# Expected:
# ├── Common\
# ├── VC98\
# └── ...

# Verify compiler exists
Test-Path "C:\VC6\VC6SP6\VC98\BIN\cl.exe"
# Should return: True
```

---

## Step 4: Configure CMake Presets

### 4.1: VC6 Preset (32-bit)

```powershell
# Configure using VC6 preset
cmake --preset vc6

# This creates: build\vc6\
# ✅ Uses VC6 portable toolchain
# ✅ 32-bit output (GeneralsX.exe, GeneralsXZH.exe)
# ✅ Release mode with optimizations
```

### 4.2: Win32 Preset (64-bit with Modern MSVC)

```powershell
# If Visual Studio 2022 is installed
cmake --preset win32-vcpkg

# This creates: build\win32-vcpkg\
# ✅ Uses MSVC 2022 (v143)
# ✅ 64-bit output
# ✅ vcpkg manages dependencies
```

### 4.3: Available Presets

| Preset | Compiler | Architecture | Output |
|--------|----------|--------------|--------|
| `vc6` | VC6 Portable | 32-bit | Release |
| `vc6-debug` | VC6 Portable | 32-bit | Debug |
| `vc6-profile` | VC6 Portable | 32-bit | Profile |
| `win32` | MSVC 2022 | 32-bit | Release |
| `win32-debug` | MSVC 2022 | 32-bit | Debug |
| `win32-vcpkg` | MSVC 2022 + vcpkg | 32-bit | Release |

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
# Build using VC6 preset
cmake --build build/vc6 --target GeneralsXZH -j 4 2>&1 | Tee-Object -FilePath logs/build.log

# Expected output:
# [1/150] Compiling...
# [150/150] Linking GeneralsXZH.exe
# Build complete (25-45 minutes first time, 30-60 seconds with ccache)

# Check for errors
Select-String -Path "logs/build.log" -Pattern "error|fatal" | Select-Object -First 10
```

### 5.2: Build Generals (Original - Secondary)

```powershell
# Build original Generals
cmake --build build/vc6 --target GeneralsX -j 4 2>&1 | Tee-Object -FilePath logs/build_generals.log
```

### 5.3: Build with Logging Enabled

```powershell
# Configure with debug logging (GeneralsXZH)
cmake --preset vc6-releaselog
cmake --build build/vc6-releaselog --target GeneralsXZH -j 4

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
Copy-Item -Path "build/vc6/GeneralsMD/GeneralsXZH.exe" `
          -Destination "$env:USERPROFILE\GeneralsX\GeneralsMD\GeneralsXZH.exe" `
          -Force -Verbose

# Deploy GeneralsX (Generals)
Copy-Item -Path "build/vc6/Generals/GeneralsX.exe" `
          -Destination "$env:USERPROFILE\GeneralsX\Generals\GeneralsX.exe" `
          -Force -Verbose

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
# Fast incremental build (30-60 seconds)
cmake --build build/vc6 --target GeneralsXZH -j 4

# Deploy & run
Copy-Item "build/vc6/GeneralsMD/GeneralsXZH.exe" `
          "$env:USERPROFILE\GeneralsX\GeneralsMD\GeneralsXZH.exe" -Force
cd "$env:USERPROFILE\GeneralsX\GeneralsMD"
.\GeneralsXZH.exe -win
```

### 9.2: Clean Rebuild (if cache corruption)

```powershell
# Full clean rebuild
Remove-Item -Path "build\vc6" -Recurse -Force -ErrorAction SilentlyContinue
cmake --preset vc6
cmake --build build/vc6 --target GeneralsXZH -j 4

# This takes 25-45 minutes
```

### 9.3: Use ccache for Faster Rebuilds

ccache dramatically speeds up rebuilds by caching compilation results:

```powershell
# Install ccache via Chocolatey
choco install ccache

# Verify installation
ccache --version

# Clear cache if needed
ccache --clear

# Monitor cache usage
ccache --show-stats
```

With ccache enabled, subsequent builds drop from 25-45 minutes to **30-60 seconds**.

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

# If missing, copy from assets:
Copy-Item "build\vc6\GeneralsMD\*" `
          "$env:USERPROFILE\GeneralsX\GeneralsMD\" `
          -Exclude "*.exe" -Force -Verbose
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
|------|------|
| **Step 1**: Install tools | 10-15 min |
| **Step 2**: Clone repo | 5 min |
| **Step 3**: VC6 setup | 15-20 min (download + verify) |
| **Step 4**: CMake configure | 2-3 min |
| **Step 5**: First build | 25-45 min |
| **Step 6**: Deploy | 1 min |
| **Step 7**: Asset setup | 10-20 min (or instant with symlink) |
| **Step 8**: First run | 2-5 min |
| **TOTAL** | **60-120 minutes** |
| Subsequent builds | 30-60 sec (with ccache) |

---

## Next Steps

1. ✅ **Build**: `cmake --build build/vc6 --target GeneralsXZH -j 4`
2. ✅ **Deploy**: Copy .exe to `$env:USERPROFILE\GeneralsX\GeneralsMD\`
3. ✅ **Run**: `.\GeneralsXZH.exe -win -noshellmap`
4. ✅ **Develop**: Edit code, rebuild, test
5. ✅ **Commit**: Push changes to GitHub

---

## References

- [VC6 Portable - itsmattkc/MSVC600](https://github.com/itsmattkc/MSVC600)
- [CMake Documentation](https://cmake.org/cmake/help/latest/)
- [Ninja Build System](https://ninja-build.org/)
- [vcpkg Package Manager](https://vcpkg.io/)
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
