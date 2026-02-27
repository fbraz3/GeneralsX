# GeneralsX Build Scripts

PowerShell scripts for building and running GeneralsX on Windows.

## Quick Start (win64-modern)

The `win64-modern` preset is the **recommended** way to build GeneralsX on Windows, featuring DXVK graphics (DirectX → Vulkan), SDL3 windowing, and OpenAL audio.

### Complete Pipeline (All-in-One)

```powershell
# Configure + Build + Deploy + Run (full workflow)
.\scripts\pipeline_win64_modern.ps1

# Configure + Build + Deploy only (skip running the game)
.\scripts\pipeline_win64_modern.ps1 -SkipRun

# Clean build (remove old artifacts first)
.\scripts\pipeline_win64_modern.ps1 -Clean
```

### Individual Steps

If you prefer to run each step separately:

```powershell
# 1. Configure CMake (one-time setup)
.\scripts\configure_cmake_modern.ps1

# 2. Build GeneralsXZH
.\scripts\build_zh_modern.ps1

# 3. Deploy runtime DLLs
.\scripts\deploy_zh_modern.ps1 -Preset win64-modern

# 4. Launch the game
.\scripts\run_zh_modern.ps1
```

## Available Scripts

### Core Build Scripts

#### `configure_cmake_modern.ps1`
Configures CMake with the `win64-modern` preset.

```powershell
.\scripts\configure_cmake_modern.ps1
.\scripts\configure_cmake_modern.ps1 -Arch x64 -HostArch x64    # Explicit architecture
```

**What it does**:
- Activates MSVC 2022 compiler environment
- Validates Vulkan SDK installation
- Runs CMake configuration (downloads SDL3, DXVK, OpenAL, FFmpeg)
- Generates Ninja build files

**Typical duration**: 
- First run: 5-15 minutes (downloads + compiles dependencies)
- Subsequent runs: 30-60 seconds

#### `build_zh_modern.ps1`
Builds GeneralsXZH executable.

```powershell
.\scripts\build_zh_modern.ps1                  # Default: 4 parallel jobs
.\scripts\build_zh_modern.ps1 -Jobs 8         # Use 8 parallel jobs (faster)
.\scripts\build_zh_modern.ps1 -Clean          # Clean + rebuild
.\scripts\build_zh_modern.ps1 -Debug          # Debug build
```

**Output**: `build/win64-modern/GeneralsMD/GeneralsXZH.exe` (~200 MB)

#### `deploy_zh_modern.ps1`
Deploys executable and runtime DLLs to the game directory.

```powershell
.\scripts\deploy_zh_modern.ps1 -Preset win64-modern           # Default: Zero Hour
.\scripts\deploy_zh_modern.ps1 -Preset win64-modern -Generals # Deploy Generals instead
.\scripts\deploy_zh_modern.ps1 -Preset win64-modern -GameDir "C:\Custom\Path"  # Custom location
```

**Deployed files**:
- `GeneralsXZH.exe` or `GeneralsX.exe`
- `d3d8.dll` (DXVK graphics)
- `SDL3.dll`, `SDL3_image.dll` (SDL3 windowing)
- `OpenAL32.dll` (OpenAL audio)
- FFmpeg DLLs (if available)

#### `run_zh_modern.ps1`
Launches GeneralsXZH with optional flags.

```powershell
.\scripts\run_zh_modern.ps1                     # Windowed mode (default)
.\scripts\run_zh_modern.ps1 -Fullscreen         # Fullscreen mode
.\scripts\run_zh_modern.ps1 -Debug              # Enable DXVK diagnostic logging
.\scripts\run_zh_modern.ps1 -NoShellMap         # Skip intro (faster launch)
.\scripts\run_zh_modern.ps1 -Mod "path/to/mod"  # Load a mod
```

#### `pipeline_win64_modern.ps1`
Executes the complete workflow: Configure → Build → Deploy → Run.

```powershell
.\scripts\pipeline_win64_modern.ps1             # Full pipeline
.\scripts\pipeline_win64_modern.ps1 -SkipRun    # Skip game launch
.\scripts\pipeline_win64_modern.ps1 -Clean      # Clean + rebuild
.\scripts\pipeline_win64_modern.ps1 -Jobs 12    # Parallel jobs
```

### Environment Setup Scripts

#### `setup_msvc_buildtools_env.ps1`
Activates MSVC 2022 compiler environment (called automatically by other scripts).

```powershell
.\scripts\setup_msvc_buildtools_env.ps1
.\scripts\setup_msvc_buildtools_env.ps1 -Arch x64 -HostArch x64
```

**Note**: This script is automatically invoked by `configure_cmake_modern.ps1` and `build_zh_modern.ps1`.

### Legacy Scripts (Windows Compatibility Mode)

#### `configure_cmake.ps1`
Configure with legacy presets (VC6, win32). **Not recommended for new builds.**

```powershell
.\scripts\configure_cmake.ps1 -Preset vc6       # Legacy VC6 32-bit
.\scripts\configure_cmake.ps1 -Preset win32     # MSVC 2022 32-bit
```

#### `build_zh.ps1`
Build with legacy presets.

```powershell
.\scripts\build_zh.ps1 -Preset vc6 -Jobs 4
.\scripts\build_zh.ps1 -Preset win32 -Jobs 8
```

## Development Workflow

### Fast Iteration (Incremental Build)

After initial setup, you can speed up builds by only recompiling changed files:

```powershell
# Initial setup (complete pipeline)
.\scripts\pipeline_win64_modern.ps1 -SkipRun

# Later, after code changes:
.\scripts\build_zh_modern.ps1                   # Just rebuild (reuses configuration)
.\scripts\deploy_zh_modern.ps1 -Preset win64-modern  # Redeploy EXE + DLLs
.\scripts\run_zh_modern.ps1                     # Run
```

### Clean Rebuild

If you hit strange compilation errors, clean everything and start fresh:

```powershell
rm build/win64-modern -Recurse -Force
.\scripts\pipeline_win64_modern.ps1 -Clean
```

### Debugging

Enable detailed logging:

```powershell
# Run with DXVK debug output
.\scripts\run_zh_modern.ps1 -Debug

# Check game output log
cat logs/run_modern.log | Select-Object -Last 50

# Build with verbose output
$env:CMAKE_VERBOSE_VERBOSITY = "VERBOSE"
.\scripts\build_zh_modern.ps1
```

## Troubleshooting

### "VulkanSDK not found"

```powershell
# Install Vulkan SDK from: https://vulkan.lunarg.com/sdk/home

# Verify installation
echo $env:VULKAN_SDK
# Expected: C:\VulkanSDK\x.y.z

# Reconfigure
.\scripts\configure_cmake_modern.ps1
```

### "MSVC compiler not found"

```powershell
# Verify Visual Studio 2022 is installed
Get-Command cl.exe

# If not found, install from: https://visualstudio.microsoft.com/vs/

# Then restart PowerShell and reconfigure
.\scripts\configure_cmake_modern.ps1
```

### "d3d8.dll not found"

```powershell
# Redeploy DLLs
.\scripts\deploy_zh_modern.ps1 -Preset win64-modern

# Verify all DLLs are present
ls GeneralsMD\Run\*.dll
```

### Game crashes on startup

```powershell
# Check DXVK diagnostic output
.\scripts\run_zh_modern.ps1 -Debug

# Look for errors in the log
cat logs/run_modern.log | grep -i "error\|warning\|fatal"

# Try windowed mode with no shader map
.\scripts\run_zh_modern.ps1 -NoShellMap
```

## VS Code Integration

These scripts are integrated with VS Code tasks. Open Command Palette (`Ctrl+Shift+P`) and search for:

- `Tasks: Run Tasks → Configure (Windows win64-modern)`
- `Tasks: Run Tasks → Build GeneralsXZH (DXVK+SDL3+OpenAL)`
- `Tasks: Run Tasks → Deploy GeneralsXZH (win64-modern)`
- `Tasks: Run Tasks → Run GeneralsXZH (Windows win64-modern)`
- `Tasks: Run Tasks → Pipeline (win64-modern Full)`

## See Also

- [Windows Build Instructions](../docs/ETC/WINDOWS_BUILD_INSTRUCTIONS.md)
- [Linux Build Instructions](../docs/ETC/LINUX_BUILD_INSTRUCTIONS.md)
- [macOS Build Instructions](../docs/ETC/MACOS_BUILD_INSTRUCTIONS.md)
- [Command Line Parameters](../docs/ETC/COMMAND_LINE_PARAMETERS.md)
