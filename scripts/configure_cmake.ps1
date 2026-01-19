# Configure CMake with MSVC BuildTools environment
# This script sets up the MSVC environment and then runs CMake configure

param(
    [string]$Preset = "win32",
    [ValidateSet('x86','x64','arm','arm64')]
    [string]$Arch = 'x86',
    [ValidateSet('x86','x64','arm','arm64')]
    [string]$HostArch = 'x64'
)

Write-Host "Configuring CMake with preset: $Preset" -ForegroundColor Cyan

# Setup MSVC environment
Write-Host "Setting up MSVC BuildTools environment..." -ForegroundColor Yellow
& ".\scripts\setup_msvc_buildtools_env.ps1" -Arch $Arch -HostArch $HostArch

# Verify cl.exe is available
$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $cl) {
    Write-Host "ERROR: cl.exe not found after environment setup" -ForegroundColor Red
    exit 1
}

Write-Host "cl.exe found: $($cl.Source)" -ForegroundColor Green

# Run CMake configure
Write-Host "Running CMake configure..." -ForegroundColor Cyan
cmake --preset $Preset

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: CMake configuration failed with exit code $LASTEXITCODE" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "✅ CMake configuration completed successfully" -ForegroundColor Green
