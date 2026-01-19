# Configure CMake with MSVC BuildTools and install vcpkg dependencies
# This script:
# 1. Sets up the MSVC environment
# 2. Downloads/updates vcpkg if needed
# 3. Installs dependencies specified in vcpkg.json
# 4. Runs CMake configure

param(
    [string]$Preset = "win32",
    [ValidateSet('x86','x64','arm','arm64')]
    [string]$Arch = 'x86',
    [ValidateSet('x86','x64','arm','arm64')]
    [string]$HostArch = 'x64'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

Write-Host "================================" -ForegroundColor Cyan
Write-Host "CMake Configuration with vcpkg" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""

# 1. Setup MSVC environment
Write-Host "[1/4] Setting up MSVC BuildTools environment..." -ForegroundColor Yellow
& ".\scripts\setup_msvc_buildtools_env.ps1" -Arch $Arch -HostArch $HostArch

# Verify cl.exe is available
$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $cl) {
    Write-Host "❌ ERROR: cl.exe not found after environment setup" -ForegroundColor Red
    exit 1
}
Write-Host "✅ MSVC environment ready" -ForegroundColor Green
Write-Host ""

# 2. Download vcpkg if not present
Write-Host "[2/4] Checking vcpkg installation..." -ForegroundColor Yellow
$vcpkgExe = "C:\vcpkg\vcpkg.exe"
if (-not (Test-Path $vcpkgExe)) {
    Write-Host "vcpkg not found. Downloading from https://github.com/microsoft/vcpkg" -ForegroundColor Cyan
    if (-not (Test-Path "C:\vcpkg")) {
        New-Item -ItemType Directory -Path "C:\vcpkg" -Force | Out-Null
    }

    $vcpkgUrl = "https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip"
    $zipPath = "$env:TEMP\vcpkg-master.zip"

    Write-Host "Downloading vcpkg from $vcpkgUrl..." -ForegroundColor Cyan
    Invoke-WebRequest -Uri $vcpkgUrl -OutFile $zipPath

    Write-Host "Extracting vcpkg..." -ForegroundColor Cyan
    Expand-Archive -Path $zipPath -DestinationPath "C:\vcpkg" -Force
    Move-Item -Path "C:\vcpkg\vcpkg-master\*" -Destination "C:\vcpkg" -Force
    Remove-Item "C:\vcpkg\vcpkg-master" -Force -ErrorAction SilentlyContinue
    Remove-Item $zipPath -Force

    # Bootstrap vcpkg
    Write-Host "Bootstrapping vcpkg..." -ForegroundColor Cyan
    & cmd /c "C:\vcpkg\bootstrap-vcpkg.bat"
}

if (-not (Test-Path $vcpkgExe)) {
    Write-Host "❌ ERROR: vcpkg.exe not found at $vcpkgExe" -ForegroundColor Red
    exit 1
}
Write-Host "✅ vcpkg ready at $vcpkgExe" -ForegroundColor Green
Write-Host ""

# 3. Install dependencies from vcpkg.json
Write-Host "[3/4] Installing vcpkg dependencies..." -ForegroundColor Yellow
Write-Host "vcpkg manifest: vcpkg.json" -ForegroundColor Cyan
$triplet = "x86-windows"
Write-Host "Building for: $triplet" -ForegroundColor Cyan

# Add C:\vcpkg to PATH for CMake integration
$env:PATH = "C:\vcpkg;$env:PATH"

Write-Host "✅ vcpkg dependencies will be managed by CMake" -ForegroundColor Green
Write-Host ""

# 4. Run CMake configure with vcpkg integration
Write-Host "[4/4] Running CMake configure with preset: $Preset..." -ForegroundColor Yellow
cmake `
    -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake" `
    -DVCPKG_TARGET_TRIPLET="$triplet" `
    --preset $Preset

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ ERROR: CMake configuration failed with exit code $LASTEXITCODE" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host ""
Write-Host "✅ CMake configuration completed successfully!" -ForegroundColor Green
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  Build:  cmake --build build\win32 --target z_generals --config Release" -ForegroundColor White
Write-Host "  Deploy: (see docs/ETC/WINDOWS_SETUP_INSTRUCTIONS.md)" -ForegroundColor White
