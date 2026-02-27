# Fast Build Script for GeneralsX - win64-modern (DXVK + SDL3 + OpenAL)
# Builds GeneralsXZH with optimal settings and logging
#
# Usage:
#   .\scripts\build_zh_modern.ps1
#   .\scripts\build_zh_modern.ps1 -Jobs 8
#   .\scripts\build_zh_modern.ps1 -Clean
#   .\scripts\build_zh_modern.ps1 -Debug
#

param(
    [int]$Jobs = 4,
    [switch]$Clean,
    [switch]$Debug
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$Preset = if ($Debug) { "win64-modern-debug" } else { "win64-modern" }
$BuildDir = "build/$Preset"
$LogFile = "logs/build_modern.log"

New-Item -ItemType Directory -Force -Path "logs" | Out-Null

Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║            Build GeneralsXZH (win64-modern preset)        ║" -ForegroundColor Cyan
Write-Host "║                 DXVK + SDL3 + OpenAL                     ║" -ForegroundColor Cyan
Write-Host "║                      Preset: $Preset" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Setup MSVC environment
Write-Host "⚙️  Setting up MSVC BuildTools environment..." -ForegroundColor Cyan
& "$PSScriptRoot\setup_msvc_buildtools_env.ps1" -Arch x64 -HostArch x64 | Out-Null

if ($LASTEXITCODE -ne 0) {
    Write-Error "MSVC environment setup failed"
    exit 1
}

Write-Host "✅ MSVC environment ready" -ForegroundColor Green
Write-Host ""

# Clean if requested
if ($Clean) {
    Write-Host "🗑️  Cleaning build directory..." -ForegroundColor Yellow
    if (Test-Path $BuildDir) {
        Remove-Item -Path $BuildDir -Recurse -Force
        Write-Host "   ✓ Build directory cleaned" -ForegroundColor Green
    }
    Write-Host ""
}

# Check if configuration exists, configure if needed
if (-not (Test-Path "$BuildDir/build.ninja")) {
    Write-Host "⚙️  CMake configuration not found, configuring with preset: $Preset" -ForegroundColor Cyan
    try {
        cmake --preset $Preset 2>&1 | Tee-Object -FilePath $LogFile -Append | Select-Object -Last 15
        Write-Host "   ✓ Configuration complete" -ForegroundColor Green
    } catch {
        Write-Error "CMake configuration failed. Check $LogFile for details."
        exit 1
    }
    Write-Host ""
}

# Build
Write-Host "🔨 Building GeneralsXZH ($Preset)..." -ForegroundColor Cyan
Write-Host "   Jobs: $Jobs | Log: $LogFile" -ForegroundColor Gray
Write-Host ""

$StartTime = Get-Date

try {
    cmake --build $BuildDir --target z_generals -j $Jobs 2>&1 | Tee-Object -FilePath $LogFile -Append
    $ExitCode = $LASTEXITCODE
} catch {
    Write-Error "Build failed: $_"
    exit 1
}

$EndTime = Get-Date
$Duration = ($EndTime - $StartTime).TotalSeconds

if ($ExitCode -eq 0) {
    Write-Host ""
    Write-Host "✅ Build completed successfully in $([Math]::Round($Duration)) seconds" -ForegroundColor Green

    # Check build artifact
    $ExePath = "build/$Preset/GeneralsMD/GeneralsXZH.exe"
    if (Test-Path $ExePath) {
        $FileSize = (Get-Item $ExePath).Length / 1MB
        Write-Host "   📦 Output: $ExePath" -ForegroundColor Green
        Write-Host "      Size: $([Math]::Round($FileSize, 2)) MB" -ForegroundColor Green
    }

    Write-Host ""
    Write-Host "💡 Next steps:" -ForegroundColor Cyan
    Write-Host "   1. Deploy: .\scripts\deploy_zh_modern.ps1 -Preset $Preset" -ForegroundColor White
    Write-Host "   2. Run: .\scripts\run_zh_modern.ps1" -ForegroundColor White

} else {
    Write-Host ""
    Write-Error "Build failed with exit code $ExitCode"

    # Show last few error lines
    Write-Host ""
    Write-Host "Last errors from build log:" -ForegroundColor Yellow
    Select-String -Path $LogFile -Pattern "error|fatal|undefined|error C[0-9]{4}" -ErrorAction SilentlyContinue |
        Select-Object -Last 15 |
        ForEach-Object { Write-Host "  $_" -ForegroundColor Red }

    Write-Host ""
    Write-Host "Full log: $LogFile" -ForegroundColor Gray
    exit 1
}
