# Configure CMake with MSVC BuildTools environment for win64-modern preset
# This script sets up the MSVC environment and then runs CMake configure
#
# Usage:
#   .\scripts\configure_cmake_modern.ps1
#   .\scripts\configure_cmake_modern.ps1 -Arch x64
#

param(
    [ValidateSet('x86', 'x64', 'arm', 'arm64')]
    [string]$Arch = 'x64',

    [ValidateSet('x86', 'x64', 'arm', 'arm64')]
    [string]$HostArch = 'x64'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║         CMake Configuration (win64-modern preset)        ║" -ForegroundColor Cyan
Write-Host "║                    DXVK + SDL3 + OpenAL                  ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Setup MSVC environment
Write-Host "⚙️  Setting up MSVC BuildTools environment..." -ForegroundColor Cyan
Write-Host "   Architecture: $Arch (host: $HostArch)" -ForegroundColor Gray
& "$PSScriptRoot\setup_msvc_buildtools_env.ps1" -Arch $Arch -HostArch $HostArch

if ($LASTEXITCODE -ne 0) {
    Write-Error "MSVC environment setup failed. Verify Visual Studio 2022 is installed."
    exit 1
}

Write-Host ""

# Verify cl.exe is available
$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $cl) {
    Write-Error "cl.exe not found after environment setup"
    exit 1
}

Write-Host "✅ MSVC compiler verified: $($cl.Source)" -ForegroundColor Green
Write-Host ""

# Run CMake configure with win64-modern preset
Write-Host "🔧 Configuring CMake with preset: win64-modern" -ForegroundColor Cyan
Write-Host "   This will download and build dependencies (SDL3, DXVK, OpenAL, FFmpeg)..." -ForegroundColor Gray
Write-Host "   ⏱️  First run: 5-15 minutes | Subsequent runs: 30-60 seconds" -ForegroundColor Gray
Write-Host ""

$StartTime = Get-Date

try {
    cmake --preset win64-modern
    $ExitCode = $LASTEXITCODE
} catch {
    Write-Error "CMake configuration failed: $_"
    exit 1
}

$EndTime = Get-Date
$Duration = ($EndTime - $StartTime).TotalSeconds

if ($ExitCode -eq 0) {
    Write-Host ""
    Write-Host "✅ CMake configuration completed successfully in $([Math]::Round($Duration)) seconds" -ForegroundColor Green
    Write-Host "   Build directory: build/win64-modern/" -ForegroundColor Green
    Write-Host "   Generator: Ninja" -ForegroundColor Green
    Write-Host ""
    Write-Host "💡 Next steps:" -ForegroundColor Cyan
    Write-Host "   1. Build: .\scripts\build_zh_modern.ps1" -ForegroundColor White
    Write-Host "   2. Deploy: .\scripts\deploy_zh_modern.ps1 -Preset win64-modern" -ForegroundColor White
    Write-Host "   3. Run: .\scripts\run_zh_modern.ps1" -ForegroundColor White
} else {
    Write-Host ""
    Write-Error "CMake configuration failed with exit code $ExitCode"
    Write-Host ""
    Write-Host "💡 Troubleshooting:" -ForegroundColor Yellow
    Write-Host "   - Verify Vulkan SDK is installed: https://vulkan.lunarg.com/sdk/home" -ForegroundColor Gray
    Write-Host "   - Check Vulkan SDK path: echo `$env:VULKAN_SDK" -ForegroundColor Gray
    Write-Host "   - Clean and retry: rm -r build/win64-modern; .\scripts\configure_cmake_modern.ps1" -ForegroundColor Gray
    exit 1
}
