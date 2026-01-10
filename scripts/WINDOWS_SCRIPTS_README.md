# GeneralsX Helper Scripts

Quick-access helper scripts for Windows and cross-platform development workflows.

## Windows PowerShell Scripts

All Windows scripts are designed for **PowerShell 5.0+** (included with Windows 10/11).

### Quick Start

```powershell
# 1. Verify environment setup
.\scripts\verify_environment.ps1

# 2. Setup VC6 portable (one-time)
.\scripts\setup_vc6_portable.ps1

# 3. Build GeneralsXZH
.\scripts\build_zh.ps1

# 4. Deploy to user directory
.\scripts\deploy_zh.ps1

# 5. Run game
.\scripts\run_zh.ps1
```

---

## Script Reference

### `verify_environment.ps1`

**Purpose**: Verify all development tools are installed and accessible.

**Usage**:
```powershell
.\scripts\verify_environment.ps1
```

**Checks**:
- ✅ Git installation
- ✅ CMake installation
- ✅ Ninja build system
- ✅ PowerShell 5.0+
- ✅ VC6 portable installation
- ✅ GeneralsX repository
- ✅ Build directories
- ✅ Deployment directories

**Output**:
```
✅ Git: git version 2.40.0
✅ CMake: cmake version 3.27.0
✅ Ninja: 1.11.0
✅ PowerShell (5.0+): 5.1.19041.4...
✅ VC6 Portable: Found
✅ Git Repository: Found
❌ GeneralsX Build Directory: Not found [CRITICAL]
   Fix: Run: cmake --preset vc6
```

---

### `setup_vc6_portable.ps1`

**Purpose**: Download, verify, and configure Visual C++ 6 Portable for VC6 builds.

**Usage**:
```powershell
.\scripts\setup_vc6_portable.ps1 [-InstallPath "C:\VC6"]
```

**Parameters**:
- `-InstallPath` (default: `C:\VC6`) - Installation directory for VC6 portable
- `-Commit` (default: specific commit hash) - GitHub commit to download
- `-ExpectedHash` (default: specific SHA256) - Expected hash for verification

**What it does**:
1. Downloads VC6 portable from [itsmattkc/MSVC600](https://github.com/itsmattkc/MSVC600)
2. **Verifies SHA256 hash** for security
3. Extracts to installation path
4. Sets up environment variables
5. Tests compiler accessibility

**Expected time**: 5-10 minutes (depending on download speed)

**Output**:
```
📁 Creating installation directory...
⬇️  Downloading VC6 Portable (this may take 5-10 minutes)...
🔐 Verifying file integrity...
   Downloaded: D0EE1F6DCEF7DB3AD703120D9FB4FAD49EBCA28F...
   Expected:   D0EE1F6DCEF7DB3AD703120D9FB4FAD49EBCA28F...
   ✓ Hash verification passed
📦 Extracting VC6 Portable...
✅ Setup Complete
```

**⚠️ Important**: Environment variables are set for the **current PowerShell session only**. For permanent setup, run VS Code tasks or use `setx` commands.

---

### `build_zh.ps1`

**Purpose**: Build GeneralsXZH (or GeneralsX) with optimized settings.

**Usage**:
```powershell
# Build GeneralsXZH (default, recommended)
.\scripts\build_zh.ps1

# Build with different preset
.\scripts\build_zh.ps1 -Preset "vc6"
.\scripts\build_zh.ps1 -Preset "win32-vcpkg"

# Build with custom job count
.\scripts\build_zh.ps1 -Jobs 8

# Clean rebuild
.\scripts\build_zh.ps1 -Clean

# Debug build
.\scripts\build_zh.ps1 -Debug

# Profile build
.\scripts\build_zh.ps1 -Profile
```

**Parameters**:
- `-Preset` (default: `vc6`) - CMake preset to use
- `-Jobs` (default: `4`) - Parallel compilation jobs
- `-Clean` (flag) - Remove build directory and rebuild from scratch
- `-Debug` (flag) - Build debug version with symbols
- `-Profile` (flag) - Build with profiling enabled

**Output**:
```
⚙️  Configuring CMake with preset: vc6
🔨 Building GeneralsXZH...
[1/150] Compiling GameEngine.cpp
[2/150] Linking...
✅ Build completed successfully in 42.5 seconds
   📦 Output: build/vc6/GeneralsMD/GeneralsXZH.exe (18.5 MB)

💡 Next step: .\deploy_zh.ps1
```

**Build Times**:
- First build: 25-45 minutes
- Subsequent builds (incremental): 30-60 seconds (with ccache)
- Clean rebuild: 25-45 minutes

---

### `deploy_zh.ps1`

**Purpose**: Copy built executable to deployment location.

**Usage**:
```powershell
# Deploy GeneralsXZH (default)
.\scripts\deploy_zh.ps1

# Deploy GeneralsX (original)
.\scripts\deploy_zh.ps1 -Generals

# Deploy with different preset
.\scripts\deploy_zh.ps1 -Preset "win32-vcpkg"
```

**Parameters**:
- `-Preset` (default: `vc6`) - CMake preset to use
- `-Generals` (flag) - Deploy Generals instead of GeneralsXZH

**Deploy Locations**:
- **GeneralsXZH**: `%USERPROFILE%\GeneralsX\GeneralsMD\GeneralsXZH.exe`
- **GeneralsX**: `%USERPROFILE%\GeneralsX\Generals\GeneralsX.exe`

**Output**:
```
📋 Deployment target: GeneralsXZH (Zero Hour)
📁 Creating deployment directory...
📦 Copying executable...
   ✓ Source:      build/vc6/GeneralsMD/GeneralsXZH.exe
   ✓ Destination: C:\Users\YourName\GeneralsX\GeneralsMD\GeneralsXZH.exe
   ✓ Size: 18.5 MB

✅ Deployment completed successfully

💡 Next steps:
  1. cd C:\Users\YourName\GeneralsX\GeneralsMD
  2. .\GeneralsXZH.exe -win -noshellmap
  3. Or use: .\run_zh.ps1
```

---

### `run_zh.ps1`

**Purpose**: Launch GeneralsXZH with automatic logging and error reporting.

**Usage**:
```powershell
# Launch in windowed mode (default)
.\scripts\run_zh.ps1

# Launch in fullscreen
.\scripts\run_zh.ps1 -Fullscreen

# Launch with mod
.\scripts\run_zh.ps1 -Mod "path/to/mod.big"

# Launch with debug output visible
.\scripts\run_zh.ps1 -Debug
```

**Parameters**:
- `-Fullscreen` (flag) - Run in fullscreen mode instead of windowed
- `-Mod` (string) - Path to mod file to load
- `-Debug` (flag) - Show game output in console (useful for debugging)

**Output**:
```
📺 Mode: Windowed
🎮 Launching: C:\Users\YourName\GeneralsX\GeneralsMD\GeneralsXZH.exe -win -noshellmap

[Game window launches...]

📋 Checking for crash logs...
   ✓ No recent crashes found

💡 Game output saved to: run.log
```

**Crash Log Checking**:
The script automatically checks for crash logs in:
```
%USERPROFILE%\Documents\Command and Conquer Generals Zero Hour Data\
```

If crashes are found, it displays the most recent ones.

---

## Typical Development Workflow

### Daily Workflow (30 seconds)

```powershell
# 1. Build (30-60 seconds with ccache)
.\scripts\build_zh.ps1

# 2. Deploy (1 second)
.\scripts\deploy_zh.ps1

# 3. Run & test
.\scripts\run_zh.ps1

# 4. Fix code, repeat from step 1
```

### Full Cycle (first time = ~90 minutes, subsequent = ~2 minutes)

```powershell
# 1. Verify environment
.\scripts\verify_environment.ps1

# 2. Setup VC6 (one-time)
.\scripts\setup_vc6_portable.ps1

# 3. Configure CMake
cmake --preset vc6

# 4-7. Build, deploy, run
.\scripts\build_zh.ps1
.\scripts\deploy_zh.ps1
.\scripts\run_zh.ps1
```

### Debugging Workflow

```powershell
# 1. Build with debug symbols
.\scripts\build_zh.ps1 -Debug

# 2. Deploy
.\scripts\deploy_zh.ps1

# 3. Run with debug output
.\scripts\run_zh.ps1 -Debug

# 4. Analyze output and crash logs
Get-Content "run.log"
Get-Content "$env:USERPROFILE\Documents\Command and Conquer Generals Zero Hour Data\*.txt"
```

---

## Environment Variables

These scripts set up the following environment variables for VC6 builds:

| Variable | Value | Purpose |
|----------|-------|---------|
| `MSVCDir` | `C:\VC6\VC6SP6\VC98` | VC6 compiler directory |
| `VSCommonDir` | `C:\VC6\VC6SP6\Common` | VC6 common files |
| `MSDevDir` | `C:\VC6\VC6SP6\Common\msdev98` | MSDEV directory |
| `INCLUDE` | `...VC98\INCLUDE;...` | C++ header paths |
| `LIB` | `...VC98\LIB;...` | Linker library paths |
| `PATH` | `...cl.exe;...` | Compiler executable paths |

⚠️ **Note**: These are set per-session. For permanent setup, use VS Code tasks or add to PowerShell profile.

---

## Troubleshooting

### Script Execution Policy Error

```
Cannot be loaded because running scripts is disabled on this system
```

**Solution**:
```powershell
# For current session only
Set-ExecutionPolicy -ExecutionPolicy Bypass -Scope Process

# For all sessions (requires admin)
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Build Fails with "VC6 not found"

```powershell
# Re-run setup
.\scripts\setup_vc6_portable.ps1

# Then rebuild
.\scripts\build_zh.ps1 -Clean
```

### Deploy Fails with "File locked"

```powershell
# Close the game if running
# Then retry deploy
.\scripts\deploy_zh.ps1
```

### Game Crashes on Startup

```powershell
# Run with debug output
.\scripts\run_zh.ps1 -Debug

# Check build log
Select-String -Path "logs/build.log" -Pattern "error" | Select-Object -First 10

# Check crash logs
Get-ChildItem "$env:USERPROFILE\Documents\Command and Conquer Generals Zero Hour Data\" | 
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 3
```

---

## References

- **Full Setup Guide**: [WINDOWS_SETUP_INSTRUCTIONS.md](../docs/ETC/WINDOWS_SETUP_INSTRUCTIONS.md)
- **VC6 Portable**: [itsmattkc/MSVC600](https://github.com/itsmattkc/MSVC600)
- **CMake**: [cmake.org](https://cmake.org/)
- **Ninja**: [ninja-build.org](https://ninja-build.org/)
- **GeneralsX**: [github.com/fbraz3/GeneralsX](https://github.com/fbraz3/GeneralsX)

---

**Last Updated**: January 2026  
**Maintained by**: GeneralsX Development Team
