# Run Script for GeneralsX Windows
# Launches the game with logging and error reporting
#
# Usage: .\run_zh.ps1 [-Fullscreen] [-Mod "path/to/mod"]
#

param(
    [switch]$Fullscreen,
    [string]$Mod,
    [switch]$Debug
)

$DeployDir = "$env:USERPROFILE\GeneralsX\GeneralsMD"
$GameExe = "$DeployDir\GeneralsXZH.exe"
$LogFile = "run.log"

Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║            GeneralsX Game Launcher (Windows)               ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Check if game exists
if (-not (Test-Path $GameExe)) {
    Write-Error "Game executable not found: $GameExe`nPlease run build_zh.ps1 and deploy_zh.ps1 first."
    exit 1
}

# Check if assets exist
$DataDir = "$DeployDir\Data"
if (-not (Test-Path $DataDir)) {
    Write-Warning "Game assets not found at: $DataDir"
    Write-Host "Please set up game assets using symbolic link or copy." -ForegroundColor Yellow
}

# Build command line
$Args = @("-win")

if ($Fullscreen) {
    $Args[0] = "-fullscreen"
    Write-Host "📺 Mode: Fullscreen" -ForegroundColor Cyan
} else {
    Write-Host "📺 Mode: Windowed" -ForegroundColor Cyan
}

$Args += "-noshellmap"

if ($Mod) {
    $Args += "-mod", $Mod
    Write-Host "📦 Mod: $Mod" -ForegroundColor Cyan
}

Write-Host ""
Write-Host "🎮 Launching: $GameExe $($Args -join ' ')" -ForegroundColor Cyan
Write-Host ""

# Launch game
try {
    Push-Location $DeployDir
    
    if ($Debug) {
        # Run with visible output for debugging
        & $GameExe $Args 2>&1 | Tee-Object -FilePath $LogFile
    } else {
        # Run with logging redirected to file
        & $GameExe $Args 2>&1 | Tee-Object -FilePath $LogFile | Out-Null
    }
    
    Pop-Location
} catch {
    Write-Error "Failed to launch game`nError: $_"
    exit 1
}

# Check for crash logs
Write-Host ""
Write-Host "📋 Checking for crash logs..." -ForegroundColor Cyan

$CrashLogDir = "$env:USERPROFILE\Documents\Command and Conquer Generals Zero Hour Data"
if (Test-Path $CrashLogDir) {
    $CrashLogs = Get-ChildItem "$CrashLogDir\*.txt" -ErrorAction SilentlyContinue | 
                 Sort-Object LastWriteTime -Descending | 
                 Select-Object -First 3
    
    if ($CrashLogs) {
        Write-Host "   ⚠️  Found recent crash logs:" -ForegroundColor Yellow
        $CrashLogs | ForEach-Object {
            Write-Host "      • $($_.Name)" -ForegroundColor Yellow
        }
        Write-Host ""
        Write-Host "   View latest: Get-Content '$CrashLogDir\$($CrashLogs[0].Name)'" -ForegroundColor Cyan
    }
}

Write-Host ""
Write-Host "💡 Game output saved to: $LogFile" -ForegroundColor Cyan
