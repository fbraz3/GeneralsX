# Fast Build Script for GeneralsX Windows Development
# Builds GeneralsXZH with optimal settings and logging
#
# Usage: .\build_zh.ps1 [-Preset "vc6"] [-Jobs 4] [-Clean]
#

param(
    [string]$Preset = "vc6",
    [int]$Jobs = 4,
    [switch]$Clean,
    [switch]$Profile,
    [switch]$Debug
)

$ErrorActionPreference = "Stop"

# Determine preset based on flags
if ($Debug) {
    $Preset = "$Preset-debug"
}
elseif ($Profile) {
    $Preset = "$Preset-profile"
}

$BuildDir = "build/$Preset"
$LogFile = "logs/build.log"

Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║                  GeneralsX Build Script                    ║" -ForegroundColor Cyan
Write-Host "║                   Preset: $Preset" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Create logs directory
New-Item -ItemType Directory -Force -Path "logs" | Out-Null

# Clean if requested
if ($Clean) {
    Write-Host "🗑️  Cleaning build directory..." -ForegroundColor Yellow
    if (Test-Path $BuildDir) {
        Remove-Item -Path $BuildDir -Recurse -Force
        Write-Host "   ✓ Build directory cleaned" -ForegroundColor Green
    }
    Write-Host ""
}

# Check if configuration exists
if (-not (Test-Path "$BuildDir/build.ninja")) {
    Write-Host "⚙️  Configuring CMake with preset: $Preset" -ForegroundColor Cyan
    try {
        & cmake --preset $Preset 2>&1 | Tee-Object -FilePath $LogFile -Append | Select-Object -Last 20
        Write-Host "   ✓ Configuration complete" -ForegroundColor Green
    } catch {
        Write-Error "CMake configuration failed. Check $LogFile for details."
        exit 1
    }
    Write-Host ""
}

# Build
Write-Host "🔨 Building GeneralsXZH..." -ForegroundColor Cyan
$StartTime = Get-Date

try {
    & cmake --build $BuildDir --target GeneralsXZH -j $Jobs 2>&1 | Tee-Object -FilePath $LogFile -Append
    $ExitCode = $LASTEXITCODE
} catch {
    Write-Error "Build failed. Check $LogFile for details."
    exit 1
}

$EndTime = Get-Date
$Duration = ($EndTime - $StartTime).TotalSeconds

if ($ExitCode -eq 0) {
    Write-Host ""
    Write-Host "✅ Build completed successfully in $($Duration) seconds" -ForegroundColor Green
    
    # Check build artifact
    $ExePath = "build/$Preset/GeneralsMD/GeneralsXZH.exe"
    if (Test-Path $ExePath) {
        $FileSize = (Get-Item $ExePath).Length / 1MB
        Write-Host "   📦 Output: $ExePath ($([Math]::Round($FileSize, 2)) MB)" -ForegroundColor Green
    }
} else {
    Write-Host ""
    Write-Error "Build failed with exit code $ExitCode"
    
    # Show last few error lines
    Write-Host ""
    Write-Host "Last errors from build log:" -ForegroundColor Yellow
    Select-String -Path $LogFile -Pattern "error|fatal|undefined" -ErrorAction SilentlyContinue | 
        Select-Object -Last 10 | 
        ForEach-Object { Write-Host "  $_" -ForegroundColor Red }
    
    exit 1
}

Write-Host ""
Write-Host "💡 Next step: .\deploy_zh.ps1" -ForegroundColor Cyan
