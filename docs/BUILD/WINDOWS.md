# GeneralsX - Windows Build Instructions (x64 Modern Stack)

This guide covers the modern Windows build flow using the unified cross-platform stack:

- DXVK (D3D8 translation at runtime via local d3d8.dll)
- SDL3
- OpenAL
- x64 only

## Prerequisites

- Windows 11 or Windows 10 x64
- Visual Studio 2022 Build Tools (Desktop C++ workload)
- CMake 3.25+
- Ninja
- Git
- PowerShell 7+ (or Windows PowerShell 5.1)
- vcpkg (the build presets use VCPKG_ROOT)

## Configure and Build

From repository root:

```powershell
cmake --preset windows64-deploy
cmake --build build/windows64-deploy --target z_generals --config RelWithDebInfo
```

Base game build:

```powershell
cmake --build build/windows64-deploy --target g_generals --config RelWithDebInfo
```

Debug/testing preset:

```powershell
cmake --preset windows64-testing
cmake --build build/windows64-testing --target z_generals --config Debug
```

## Build Scripts

Use the helper scripts under scripts/build/windows:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build\windows\build-windows-zh.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\build\windows\build-windows-generals.ps1
```

Deploy Zero Hour binary to local runtime directory:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build\windows\deploy-windows-zh.ps1
```

Run Zero Hour:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build\windows\run-windows-zh.ps1
```

## Runtime Notes

- Compile-time headers/libraries come from Windows SDK in modern presets when SAGE_USE_DX8=OFF.
- Runtime rendering backend should be provided by DXVK d3d8.dll placed next to the game executable.
- CI bundles d3d8.dll from DXVK release artifacts for validation and downstream packaging.

## VS Code Tasks

Use these tasks from tasks.json:

- [Windows] Build GeneralsXZH
- [Windows] Build GeneralsX
- [Windows] Deploy GeneralsXZH
- [Windows] Run GeneralsXZH
- [Windows] Pipeline: Build + Deploy + Run ZH

## Troubleshooting

### CMake cannot find vcpkg toolchain

Set VCPKG_ROOT to your local vcpkg installation path before running configure.

### Missing d3d8.dll at runtime

Copy DXVK x64 d3d8.dll to the same directory as GeneralsXZH.exe.

### Build succeeded but executable path differs

For multi-config generators, binaries can be inside a configuration subfolder such as RelWithDebInfo or Debug.
