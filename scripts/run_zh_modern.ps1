# Run Script for GeneralsXZH (win64-modern DXVK + SDL3 + OpenAL)
# Launches the game with logging and error reporting
#
# Usage:
#   .\scripts\run_zh_modern.ps1
#   .\scripts\run_zh_modern.ps1 -Fullscreen
#   .\scripts\run_zh_modern.ps1 -Debug
#   .\scripts\run_zh_modern.ps1 -Mod "path/to/mod"
#

param(
    [switch]$Fullscreen,
    [switch]$Debug,
    [string]$Mod,
    [switch]$NoShellMap
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$GameExe = "GeneralsMD\Run\GeneralsXZH.exe"
$LogFile = "logs/run_modern.log"

New-Item -ItemType Directory -Force -Path "logs" | Out-Null

Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║        GeneralsXZH Game Launcher (win64-modern)          ║" -ForegroundColor Cyan
Write-Host "║           DXVK Graphics + SDL3 Window + OpenAL Audio      ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Check if game exists
if (-not (Test-Path $GameExe)) {
    Write-Error "Game executable not found: $GameExe`nPlease run build_zh_modern.ps1 and deploy_zh_modern.ps1 first."
    exit 1
}

Write-Host "✅ Game executable verified: $GameExe" -ForegroundColor Green
Write-Host ""

# Verify required DLLs
$RequiredDLLs = @("d3d8.dll", "SDL3.dll", "OpenAL32.dll")
$GameDir = "GeneralsMD\Run"

Write-Host "🔍 Checking runtime dependencies..." -ForegroundColor Cyan
$MissingDLLs = @()
foreach ($dll in $RequiredDLLs) {
    $dllPath = Join-Path $GameDir $dll
    if (Test-Path $dllPath) {
        Write-Host "   ✓ $dll" -ForegroundColor Green
    } else {
        Write-Host "   ✗ $dll (MISSING)" -ForegroundColor Red
        $MissingDLLs += $dll
    }
}

if ($MissingDLLs.Count -gt 0) {
    Write-Host ""
    Write-Error "Missing required DLLs: $($MissingDLLs -join ', ')`nRun: .\scripts\deploy_zh_modern.ps1 -Preset win64-modern"
    exit 1
}

Write-Host ""

# Check if assets exist
$DataDir = "GeneralsMD\Run\Data"
if (-not (Test-Path $DataDir)) {
    Write-Warning "⚠️  Game assets not found at: $DataDir"
    Write-Host "   Please set up game assets using symbolic link or copy." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "   Example:" -ForegroundColor Gray
    Write-Host "   New-Item -ItemType SymbolicLink -Path $DataDir -Target 'C:\Path\To\Generals\Data'" -ForegroundColor Gray
    Write-Host ""
}

# Build command line
$Args = @()

if ($Fullscreen) {
    $Args += "-fullscreen"
    Write-Host "🖥️  Mode: Fullscreen" -ForegroundColor Cyan
} else {
    $Args += "-win"
    Write-Host "🖥️  Mode: Windowed" -ForegroundColor Cyan
}

if (-not $NoShellMap) {
    $Args += "-noshellmap"
}

if ($Mod) {
    $Args += "-mod", $Mod
    Write-Host "📦 Mod: $Mod" -ForegroundColor Cyan
}

Write-Host ""

# Set DXVK diagnostic logging if Debug is enabled
if ($Debug) {
    Write-Host "🐛 Debug mode enabled - DXVK logging active" -ForegroundColor Yellow
    $env:DXVK_LOG_LEVEL = "info"
}

# Launch game
Write-Host "🎮 Launching: $GameExe $($Args -join ' ')" -ForegroundColor Cyan
Write-Host "   Log: $LogFile" -ForegroundColor Gray
Write-Host ""

try {
    Push-Location $GameDir

    if ($Debug) {
        # Run with visible output for debugging
        Write-Host "════════════════════════════════════════════════════════════" -ForegroundColor Cyan
        & $([IO.Path]::Combine([IO.Path]::GetFullPath("."), "GeneralsXZH.exe")) $Args 2>&1 | Tee-Object -FilePath ([IO.Path]::Combine([IO.Path]::GetFullPath("..\.."), $LogFile))
        Write-Host "════════════════════════════════════════════════════════════" -ForegroundColor Cyan
    } else {
        # Run with logging redirected to file
        & $([IO.Path]::Combine([IO.Path]::GetFullPath("."), "GeneralsXZH.exe")) $Args 2>&1 | Tee-Object -FilePath ([IO.Path]::Combine([IO.Path]::GetFullPath("..\.."), $LogFile)) | Out-Null
    }

    Pop-Location
} catch {
    Write-Error "Failed to launch game: $_"
    exit 1
}

Write-Host ""
Write-Host "✓ Game session ended" -ForegroundColor Green
Write-Host "  Log: $LogFile" -ForegroundColor Gray
