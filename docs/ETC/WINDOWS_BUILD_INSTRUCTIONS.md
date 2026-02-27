# GeneralsX - Windows Build Instructions

This guide provides step-by-step instructions for building GeneralsX on Windows 10/11 using **CMake** with the new **`win64-modern` preset** (DXVK + SDL3 + OpenAL) or the legacy **32-bit presets** (VC6/Win32).

## ⚡ Quick Start (Windows Modern - Recommended)

**The `win64-modern` preset provides a unified cross-platform build** matching the Linux/macOS architecture.

```powershell
# Clone repository
git clone https://github.com/fbraz3/GeneralsX.git
cd GeneralsX

# Configure build (one-time setup)
cmake --preset win64-modern

# Build Zero Hour (GeneralsXZH)
cmake --build --preset win64-modern --target z_generals

# Deploy runtime DLLs
.\scripts\deploy_zh_modern.ps1 -Preset win64-modern

# Run the game
.\GeneralsMD\Run\GeneralsXZH.exe -win
```

**Result**: Native Windows EXE (~200 MB) with DXVK (DirectX → Vulkan), SDL3 windowing, and OpenAL audio.

---

## 🔧 Technology Stack

### Modern Stack (win64-modern) - RECOMMENDED

| Component | Technology | Details |
|-----------|-----------|---------|
| **Graphics** | DXVK 2.6 | DirectX 8 → Vulkan (GPU-accelerated) |
| **Windowing/Input** | SDL3 3.4.2 | Cross-platform, modern, low-latency |
| **Audio** | OpenAL Soft 1.24.2 | WASAPI backend (modern, low-latency) |
| **Video** | FFmpeg | Optional subtitle/video support |
| **Architecture** | x86_64 | 64-bit native Windows |
| **Compiler** | MSVC 2022 | Modern C++20 support |
| **Build System** | CMake + Ninja | Fast, parallel compilation |

### Legacy Stacks (VC6 / win32) - COMPATIBILITY MODE

| Component | VC6 (32-bit) | Win32 (32-bit) |
|-----------|-------------|----------------|
| **Graphics** | DirectX 8 native | DirectX 8 native |
| **Windowing** | Win32 API | Win32 API |
| **Audio** | Miles Sound System | Miles Sound System |
| **Video** | Bink Video | Bink Video |
| **Architecture** | x86 (32-bit) | x86 (32-bit) |
| **Compiler** | Visual Studio 6 | MSVC 2022 x86 |
| **Status** | Original retail | Modernized Windows |

---

## Prerequisites

### System Requirements

- **OS**: Windows 10 version 1909+ or Windows 11
- **RAM**: 8 GB minimum (16 GB recommended for parallel builds)
- **Disk**: 50 GB free (for dependencies + build artifacts)
- **GPU**: Any GPU with Vulkan 1.1+ support (NVIDIA, AMD, Intel since ~2016)

### Essential Software

#### For win64-modern Preset (Modern Stack)

1. **Visual Studio 2022 Community** (or Pro/Enterprise)
   - Download: https://visualstudio.microsoft.com/vs/
   - Required workload: "Desktop development with C++"
   - Optional: Individual components → "Windows 10/11 SDK"

2. **CMake** 3.25 or higher
   ```powershell
   # Install via Winget (Windows 11)
   winget install Kitware.CMake
   
   # Or download from https://cmake.org/download/
   ```

3. **Ninja** build system
   ```powershell
   # Install via Winget (Windows 11)
   winget install Ninja-build.Ninja
   
   # Or download from https://github.com/ninja-build/ninja/releases
   ```

4. **Vulkan SDK** (CRITICAL for DXVK)
   - Download: https://vulkan.lunarg.com/sdk/home
   - Windows version required
   - After install, verify environment variable:
     ```powershell
     echo $env:VULKAN_SDK
     # Should output: C:\VulkanSDK\VERSION
     ```

5. **Git** for version control
   - Download: https://git-scm.com/download/win

#### For Legacy Presets (VC6 / win32)

- Visual Studio 6 (VC6 preset) - **deprecated, use win64-modern instead**
- Or MSVC 2022 for win32 preset (x86 32-bit build)

### Verification Checklist

```powershell
# After installing all prerequisites, verify:

# 1. CMake
cmake --version
# Expected: cmake version 3.25+

# 2. Ninja
ninja --version
# Expected: 1.10+

# 3. Vulkan SDK
$env:VULKAN_SDK
# Expected: C:\VulkanSDK\1.3.xxx

# 4. Git
git --version
# Expected: git version 2.30+

# 5. MSVC 2022
cl.exe
# Expected: Microsoft (R) C/C++ Optimizing Compiler Version 19.3+
# If not found, run: "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
```

---

## Build Configuration

### 1. Clone Repository

```powershell
# Via Git
git clone https://github.com/fbraz3/GeneralsX.git
cd GeneralsX

# Or update existing clone
git pull origin windows-sdl
```

### 2. Configure Build (win64-modern)

```powershell
# One-time setup - generates build files
cmake --preset win64-modern

# Optional: Debug variant
cmake --preset win64-modern-debug
```

**What happens**:
- Downloads and builds SDL3, OpenAL Soft, DXVK headers
- Validates Vulkan SDK installation
- Generates Ninja build files in `build/win64-modern/`

**Typical duration**: 5-15 minutes (first run), 30-60 seconds (subsequent runs)

### 3. Build Targets

#### **Zero Hour (GeneralsXZH)** - PRIMARY TARGET
```powershell
# Build executable
cmake --build --preset win64-modern --target z_generals

# Build location: build/win64-modern/GeneralsMD/GeneralsXZH.exe
```

#### **Original Generals (GeneralsX)** - SECONDARY TARGET
```powershell
# Build executable
cmake --build --preset win64-modern --target g_generals

# Build location: build/win64-modern/Generals/GeneralsX.exe
```

#### **Core Libraries** (Optional testing)
```powershell
# Build individual libraries
cmake --build --preset win64-modern --target ww3d2 wwlib wwmath
```

#### **Parallel Builds** (Faster compilation)
```powershell
# Use all CPU cores for compilation (faster)
cmake --build --preset win64-modern --target z_generals -j 16

# Or use half for system responsiveness
cmake --build --preset win64-modern --target z_generals -j 8
```

---

## Runtime Deployment

After building, you must deploy runtime DLLs alongside the executable.

### Automated Deployment (Recommended)

```powershell
# Deploy win64-modern build
.\scripts\deploy_zh_modern.ps1 -Preset win64-modern

# Optional: Deploy Generals instead of Zero Hour
.\scripts\deploy_zh_modern.ps1 -Preset win64-modern -Generals

# Optional: Deploy to custom directory
.\scripts\deploy_zh_modern.ps1 -Preset win64-modern -GameDir "C:\Path\To\Game"
```

**What gets deployed**:
- `GeneralsXZH.exe` → `GeneralsMD\Run\`
- `d3d8.dll` (DXVK) → Game directory
- `SDL3.dll`, `SDL3_image.dll` → Game directory
- `OpenAL32.dll` (OpenAL Soft) → Game directory
- FFmpeg DLLs (if available) → Game directory

### Manual Verification

```powershell
# Verify all DLLs are present
ls GeneralsMD\Run\*.dll

# Expected:
# d3d8.dll          ← DXVK graphics
# d3d9.dll          ← DXVK optional
# SDL3.dll          ← SDL3 windowing
# SDL3_image.dll    ← SDL3 image loading
# OpenAL32.dll      ← OpenAL audio
# av*.dll           ← FFmpeg (optional)
```

---

## Running the Game

### Quick Start

```powershell
# Windowed mode (recommended for testing)
.\GeneralsMD\Run\GeneralsXZH.exe -win

# Fullscreen mode
.\GeneralsMD\Run\GeneralsXZH.exe -fullscreen

# Skip intro/shell map (faster launch)
.\GeneralsMD\Run\GeneralsXZH.exe -win -noshellmap
```

### Verify Graphics Backend

```powershell
# Enable DXVK diagnostic logging
$env:DXVK_LOG_LEVEL = "info"

# Run the game
.\GeneralsMD\Run\GeneralsXZH.exe -win

# Check output for DXVK messages:
# "DXVK: Direct3DCreate8 called"
# "DXVK: Created D3D11 device"
```

### Verify Audio Backend

- Listen for: background music, UI sounds, speech during gameplay
- If silent: check Event Viewer for OpenAL errors (Windows Logs → Application)

---

## Build Presets Reference

### Modern Stack (Recommended)

| Preset | Architecture | Graphics | Audio | Status |
|--------|--------------|----------|-------|--------|
| `win64-modern` | x86_64 | DXVK | OpenAL | ✅ Production |
| `win64-modern-debug` | x86_64 | DXVK | OpenAL | ✅ Development |

### Legacy Stacks (Compatibility)

| Preset | Architecture | Graphics | Audio | Status | Notes |
|--------|--------------|----------|-------|--------|-------|
| `vc6` | x86 (32-bit) | D3D8 native | Miles | ✅ Stable | Original retail build |
| `win32` | x86 (32-bit) | D3D8 native | Miles | ✅ Stable | MSVC 2022 x86 variant |

---

## Troubleshooting

### Build Errors

#### "VulkanSDK not found"
```
Error: Vulkan SDK not found. Required for DXVK builds.
```
**Solution**:
1. Download & install Vulkan SDK: https://vulkan.lunarg.com/sdk/home
2. Set environment variable:
   ```powershell
   $env:VULKAN_SDK = "C:\VulkanSDK\1.3.xxx"
   ```
3. Reconfigure: `cmake --preset win64-modern`

#### "MSVC compiler not found"
```
Error: Could not find VC++ compiler
```
**Solution**:
1. Install Visual Studio 2022 Community
2. Run Visual Studio installer → Modify → Desktop development with C++
3. Reconfigure: `cmake --preset win64-modern`

#### "Ninja not found"
```
Error: ninja: not found
```
**Solution**:
```powershell
winget install Ninja-build.Ninja
# Then restart PowerShell and reconfigure
```

### Runtime Errors

#### "d3d8.dll not found"
```
Error: The code execution cannot proceed because d3d8.dll was not found
```
**Solution**:
```powershell
# Redeploy DLLs
.\scripts\deploy_zh_modern.ps1 -Preset win64-modern

# Verify deployment
ls GeneralsMD\Run\d3d8.dll
```

#### "Game crashes on startup"
**Debug steps**:
1. Run with diagnostic logging:
   ```powershell
   $env:DXVK_LOG_LEVEL = "debug"
   .\GeneralsMD\Run\GeneralsXZH.exe -win -noshellmap
   ```
2. Check `dxvk.log` in game directory for error messages
3. Try windowed mode: `-win` flag
4. Verify GPU supports Vulkan: https://vulkan.gpuinfo.org/

#### "Silent audio - no sounds"
**Debug steps**:
1. Verify OpenAL32.dll deployed:
   ```powershell
   ls GeneralsMD\Run\OpenAL32.dll
   ```
2. Check Windows Event Viewer → Application Logs for OpenAL errors
3. Verify system audio is not muted
4. Try different audio output device in Windows Settings

---

## Developer Mode

### Incremental Builds (Fast Iteration)

```powershell
# Configure once
cmake --preset win64-modern

# Then just build (no reconfigure)
cmake --build --preset win64-modern --target z_generals

# Redeploy only EXE (not DLLs, unless they changed)
Copy-Item build/win64-modern/GeneralsMD/GeneralsXZH.exe GeneralsMD/Run/ -Force
```

### Compile Database (IDE Integration)

CMake auto-generates `build/win64-modern/compile_commands.json` for IDE indexing:

```powershell
# VSCode clangd will auto-detect it
# Clion will import it automatically
# Vim/Neovim with clangd: it finds the nearest compile_commands.json
```

### Clean Rebuild

```powershell
# Full clean (removes all artifacts)
rm build/win64-modern -Recurse -Force

# Reconfigure
cmake --preset win64-modern

# Rebuild
cmake --build --preset win64-modern --target z_generals
```

---

## Performance Optimization

### Parallel Compilation

```powershell
# Determine CPU core count
$cores = (Get-WmiObject Win32_ComputerSystem).NumberOfLogicalProcessors

# Build with all cores (fastest, may use 100% CPU)
cmake --build --preset win64-modern --target z_generals -j $cores

# Or use half cores (allows other work)
cmake --build --preset win64-modern --target z_generals -j ($cores / 2)
```

### Link Time Optimization (LTO)

Currently disabled by default for faster build times. To enable:

```powershell
# Modify CMakeUserPresets.json or rebuild with -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

---

## VS Code Integration

### Available Tasks

Open Command Palette (`Ctrl+Shift+P`) and run:

- `Tasks: Run Tasks → Configure (Windows win64-modern)`
- `Tasks: Run Tasks → Build GeneralsXZH (Windows MSVC2022, win64-modern)`
- `Tasks: Run Tasks → Deploy GeneralsXZH (Windows win64-modern)`
- `Tasks: Run Tasks → Pipeline: Configure + Build + Deploy + Run (win64-modern ZH)`

### Recommended Extensions

```json
{
    "recommendations": [
        "ms-vscode.cpptools",
        "ms-vscode.cmake-tools",
        "llvm-vs-code-extensions.vscode-clangd",
        "eamodio.gitlens"
    ]
}
```

---

## Next Steps

1. **Verify installation**: Run all prerequisite checks above
2. **First build**: `cmake --preset win64-modern && cmake --build --preset win64-modern --target z_generals`
3. **Deploy**: `.\scripts\deploy_zh_modern.ps1 -Preset win64-modern`
4. **Test**: `.\GeneralsMD\Run\GeneralsXZH.exe -win`
5. **Report issues**: If you encounter errors, check the troubleshooting section or file an issue on GitHub

---

## See Also

- [Linux Build Instructions](./LINUX_BUILD_INSTRUCTIONS.md)
- [macOS Build Instructions](./MACOS_BUILD_INSTRUCTIONS.md)
- [Command Line Parameters](./COMMAND_LINE_PARAMETERS.md)
- [General Contributing Guidelines](../../CONTRIBUTING.md)
