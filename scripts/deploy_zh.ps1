# Deploy Script for GeneralsX Windows
# Copies built executable to deployment location
#
# Usage: .\deploy_zh.ps1 [-Preset "vc6"] [-Generals] [-NoSign]
#

param(
    [string]$Preset = "vc6",
    [switch]$Generals,
    [switch]$NoSign
)

$ErrorActionPreference = "Stop"

$BuildDir = "build/$Preset"
$DeployBase = "$env:USERPROFILE\GeneralsX"

Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║                  GeneralsX Deploy Script                   ║" -ForegroundColor Cyan
Write-Host "║                   Preset: $Preset" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Determine target
if ($Generals) {
    $GameDir = "Generals"
    $BuildTarget = "$BuildDir\Generals\GeneralsX.exe"
    $DeployTarget = "$DeployBase\Generals\GeneralsX.exe"
    $DisplayName = "GeneralsX (Original)"
} else {
    $GameDir = "GeneralsMD"
    $BuildTarget = "$BuildDir\GeneralsMD\GeneralsXZH.exe"
    $DeployTarget = "$DeployBase\GeneralsMD\GeneralsXZH.exe"
    $DisplayName = "GeneralsXZH (Zero Hour)"
}

# Check if build exists
if (-not (Test-Path $BuildTarget)) {
    Write-Error "Build output not found: $BuildTarget`nPlease run build_zh.ps1 first."
    exit 1
}

Write-Host "📋 Deployment target: $DisplayName" -ForegroundColor Cyan

# Create deployment directory
Write-Host "📁 Creating deployment directory..." -ForegroundColor Cyan
New-Item -ItemType Directory -Force -Path "$DeployBase\$GameDir" | Out-Null
Write-Host "   ✓ Directory: $DeployBase\$GameDir" -ForegroundColor Green

# Copy executable
Write-Host "📦 Copying executable..." -ForegroundColor Cyan
try {
    Copy-Item -Path $BuildTarget -Destination $DeployTarget -Force
    Write-Host "   ✓ Source:      $BuildTarget" -ForegroundColor Green
    Write-Host "   ✓ Destination: $DeployTarget" -ForegroundColor Green
} catch {
    Write-Error "Failed to copy executable`nError: $_"
    exit 1
}

# Verify deployment
if (-not (Test-Path $DeployTarget)) {
    Write-Error "Deployment verification failed: File not found at $DeployTarget"
    exit 1
}

$FileSize = (Get-Item $DeployTarget).Length / 1MB
Write-Host "   ✓ Size: $([Math]::Round($FileSize, 2)) MB" -ForegroundColor Green

Write-Host ""
Write-Host "✅ Deployment completed successfully" -ForegroundColor Green

Write-Host ""
Write-Host "💡 Next steps:" -ForegroundColor Cyan
Write-Host "  1. Navigate to: cd $DeployBase\$GameDir" -ForegroundColor White
Write-Host "  2. Run game:    .\GeneralsXZH.exe -win -noshellmap" -ForegroundColor White
Write-Host "  3. Or use:      .\run_zh.ps1" -ForegroundColor White
