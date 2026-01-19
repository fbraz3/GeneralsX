# Complete CMake configuration with MSVC BuildTools and vcpkg
# This ensures all environment variables are properly set

param(
    [string]$Preset = "win32"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

Write-Host "================================" -ForegroundColor Cyan
Write-Host "CMake Configuration - Final" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""

# 1. Setup MSVC environment first
Write-Host "[1/3] Setting up MSVC BuildTools environment..." -ForegroundColor Yellow
& ".\scripts\setup_msvc_buildtools_env.ps1" -Arch x86 -HostArch x64

Write-Host ""

# 2. Verify vcpkg is ready
Write-Host "[2/3] Verifying vcpkg installation..." -ForegroundColor Yellow
$vcpkgExe = "C:\vcpkg\vcpkg.exe"
if (-not (Test-Path $vcpkgExe)) {
    Write-Host "ERROR: vcpkg not found at $vcpkgExe" -ForegroundColor Red
    exit 1
}
Write-Host "OK: vcpkg ready" -ForegroundColor Green

Write-Host ""

# 3. Configure CMake
Write-Host "[3/3] Running CMake configure..." -ForegroundColor Yellow
Write-Host "Preset: $Preset" -ForegroundColor Cyan
Write-Host "Triplet: x86-windows" -ForegroundColor Cyan
Write-Host ""

# Configure vcpkg toolchain
$env:VCPKG_ROOT = "C:\vcpkg"
$env:CMAKE_TOOLCHAIN_FILE = "$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"

cmake `
    --preset $Preset `
    -DCMAKE_TOOLCHAIN_FILE="$env:CMAKE_TOOLCHAIN_FILE" `
    -DVCPKG_TARGET_TRIPLET="x86-windows"

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "OK: CMake configuration SUCCESSFUL!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next step: Build the project" -ForegroundColor Cyan
    Write-Host "  Command: cmake --build build\win32 --target z_generals --config Release --parallel 4" -ForegroundColor White
} else {
    Write-Host ""
    Write-Host "ERROR: CMake configuration failed" -ForegroundColor Red
    exit $LASTEXITCODE
}
