# Complete Pipeline for GeneralsXZH (win64-modern)
# Configure + Build + Deploy + Run in one script
#
# Usage:
#   .\scripts\pipeline_win64_modern.ps1
#   .\scripts\pipeline_win64_modern.ps1 -SkipRun    # Configure, Build, Deploy only
#   .\scripts\pipeline_win64_modern.ps1 -Clean      # Clean before configure
#

param(
    [switch]$SkipRun,
    [switch]$Clean,
    [int]$Jobs = 4
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$StartTime = Get-Date

Write-Host ""
Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║   GeneralsXZH Complete Pipeline (win64-modern preset)    ║" -ForegroundColor Cyan
Write-Host "║        Configure → Build → Deploy → Run                  ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Step 1: Configure
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "STEP 1: Configure CMake (win64-modern)" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan

if ($Clean) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    if (Test-Path "build/win64-modern") {
        Remove-Item -Path "build/win64-modern" -Recurse -Force
    }
}

& "$PSScriptRoot\configure_cmake_modern.ps1"
if ($LASTEXITCODE -ne 0) {
    Write-Error "❌ Configure failed"
    exit 1
}

Write-Host ""

# Step 2: Build
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "STEP 2: Build GeneralsXZH" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan

& "$PSScriptRoot\build_zh_modern.ps1" -Jobs $Jobs
if ($LASTEXITCODE -ne 0) {
    Write-Error "❌ Build failed"
    exit 1
}

Write-Host ""

# Step 3: Deploy
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "STEP 3: Deploy (Copy DLLs + EXE)" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan

& "$PSScriptRoot\deploy_zh_modern.ps1" -Preset win64-modern
if ($LASTEXITCODE -ne 0) {
    Write-Error "❌ Deploy failed"
    exit 1
}

Write-Host ""

# Step 4: Run (optional)
if (-not $SkipRun) {
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
    Write-Host "STEP 4: Launch Game" -ForegroundColor Cyan
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan

    & "$PSScriptRoot\run_zh_modern.ps1"
}

$EndTime = Get-Date
$Duration = ($EndTime - $StartTime).TotalSeconds

Write-Host ""
Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Green
Write-Host "║                    ✅ PIPELINE COMPLETE                    ║" -ForegroundColor Green
Write-Host "║            Total time: $([Math]::Round($Duration)) seconds" -ForegroundColor Green
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Green
