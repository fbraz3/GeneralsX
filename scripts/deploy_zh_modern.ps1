# scripts/deploy_zh_modern.ps1
# GeneralsX @build BenderAI 26/02/2026 - Phase 06
# Deploys win64-modern build + DXVK/SDL3/OpenAL/FFmpeg runtime DLLs
#
# Usage:
#   .\scripts\deploy_zh_modern.ps1 -Preset win64-modern -GameDir GeneralsMD\Run
#   .\scripts\deploy_zh_modern.ps1 -Generals  # Deploy Generals instead (use win64-modern preset)

param(
    [string]$Preset = "win64-modern",
    [switch]$Generals = $false,
    [string]$GameDir = ""
)

# Determine target
$TargetName = if ($Generals) { "GeneralsX" } else { "GeneralsXZH" }
$TargetDir = if ($Generals) { "Generals" } else { "GeneralsMD" }

# Use provided GameDir or default
if ([string]::IsNullOrEmpty($GameDir)) {
    $GameDir = Join-Path $PSScriptRoot "..\$TargetDir\Run"
}

# Build output directory
$BuildDir = Join-Path $PSScriptRoot "..\build\$Preset\$TargetDir"
# Build root (for _deps which live at build/<preset>/_deps, not inside the target subdir)
$BuildDepsDir = Join-Path $PSScriptRoot "..\build\$Preset\_deps"

# Ensure target directories exist
if (!(Test-Path $GameDir)) {
    Write-Host "ERROR: Game directory does not exist: $GameDir" -ForegroundColor Red
    exit 1
}

if (!(Test-Path $BuildDir)) {
    Write-Host "ERROR: Build directory does not exist: $BuildDir" -ForegroundColor Red
    Write-Host "Make sure you have built with: cmake --build --preset $Preset --target z_generals" -ForegroundColor Yellow
    exit 1
}

Write-Host "=====================================================" -ForegroundColor Cyan
Write-Host "Deploying $TargetName (win64-modern)" -ForegroundColor Cyan
Write-Host "=====================================================" -ForegroundColor Cyan
Write-Host ""

# 1. Copy EXE
Write-Host "Copying EXE..." -ForegroundColor Green
$ExeSource = Join-Path $BuildDir "$TargetName.exe"
if (Test-Path $ExeSource) {
    Copy-Item $ExeSource $GameDir -Force
    Write-Host "OK Copied $TargetName.exe" -ForegroundColor Green
} else {
    Write-Host "EXE not found: $ExeSource" -ForegroundColor Red
    Write-Host "Expected build output at: $ExeSource" -ForegroundColor Yellow
    Write-Host "Run build first: cmake --build build/$Preset --target z_generals" -ForegroundColor Yellow
}

# 2. Copy DXVK Windows DLLs (x64)
# GeneralsX @bugfix BenderAI 03/03/2026 - DXVK dir is under build/<preset>/_deps, not build/_deps; d3d12.dll removed (not in package)
Write-Host ""
Write-Host "Copying DXVK DLLs..." -ForegroundColor Green
$DxvkDir = Join-Path $BuildDepsDir "dxvk_win_release-src\x64"
if (Test-Path $DxvkDir) {
    # d3d12.dll intentionally omitted - not included in DXVK Windows release package
    $DxvkDlls = @("d3d8.dll", "d3d9.dll", "d3d10core.dll", "d3d11.dll", "dxgi.dll")
    foreach ($dll in $DxvkDlls) {
        $src = Join-Path $DxvkDir $dll
        if (Test-Path $src) {
            Copy-Item $src $GameDir -Force
            Write-Host "✓ Copied $dll" -ForegroundColor Green
        }
    }
} else {
    Write-Host "! DXVK directory not found (not yet downloaded): $DxvkDir" -ForegroundColor Yellow
    Write-Host "  This is normal on first build; will be fetched during FetchContent" -ForegroundColor Yellow
}

# 3. Copy SDL3.dll
# GeneralsX @bugfix BenderAI 03/03/2026 - SDL3 _deps lives at build/<preset>/_deps (build root), not inside target subdir; dir name is lowercase sdl3-build
Write-Host ""
Write-Host "Copying SDL3 DLL..." -ForegroundColor Green
$Sdl3Dll = Join-Path $BuildDepsDir "sdl3-build\SDL3.dll"
if (Test-Path $Sdl3Dll) {
    Copy-Item $Sdl3Dll $GameDir -Force
    Write-Host "OK Copied SDL3.dll" -ForegroundColor Green
} else {
    Write-Host "SDL3.dll not found at: $Sdl3Dll" -ForegroundColor Red
}

# 4. Copy SDL3_image.dll (optional - Windows build uses native cursor loading, SDL3_image may not be present)
Write-Host "Copying SDL3_image DLL (optional)..." -ForegroundColor Green
$Sdl3ImageDll = Join-Path $BuildDepsDir "SDL3_image-build\SDL3_image.dll"
if (Test-Path $Sdl3ImageDll) {
    Copy-Item $Sdl3ImageDll $GameDir -Force
    Write-Host "OK Copied SDL3_image.dll" -ForegroundColor Green
} else {
    Write-Host "  (SDL3_image.dll not present - expected on Windows builds)" -ForegroundColor Gray
}

# 5. Copy OpenAL Soft
# GeneralsX @bugfix BenderAI 03/03/2026 - OpenAL _deps also lives at build root, not inside target subdir
Write-Host ""
Write-Host "Copying OpenAL DLL..." -ForegroundColor Green
$OpenalDll = Join-Path $BuildDepsDir "openal_soft-build\OpenAL32.dll"
if (Test-Path $OpenalDll) {
    Copy-Item $OpenalDll $GameDir -Force
    Write-Host "OK Copied OpenAL32.dll" -ForegroundColor Green
} else {
    Write-Host "OpenAL DLL not found at: $OpenalDll" -ForegroundColor Red
}

# 6. Copy FFmpeg DLLs (optional, if built)
Write-Host ""
Write-Host "Copying FFmpeg DLLs (if available)..." -ForegroundColor Green
$FfmpegDlls = Get-ChildItem $BuildDir -Filter "av*.dll" -ErrorAction SilentlyContinue
if ($FfmpegDlls) {
    foreach ($dll in $FfmpegDlls) {
        Copy-Item $dll.FullName $GameDir -Force
        Write-Host "✓ Copied $($dll.Name)" -ForegroundColor Green
    }
} else {
    Write-Host "  (No FFmpeg DLLs found)" -ForegroundColor Gray
}

# 7. Copy other library DLLs that might be needed
Write-Host ""
Write-Host "Copying other runtime DLLs..." -ForegroundColor Green
Get-ChildItem $BuildDir -Filter "sw*.dll" -ErrorAction SilentlyContinue | ForEach-Object {
    Copy-Item $_.FullName $GameDir -Force
    Write-Host "✓ Copied $($_.Name)" -ForegroundColor Green
}

# 8. Copy optional DXVK config (if exists)
Write-Host ""
Write-Host "Checking for DXVK config..." -ForegroundColor Green
$DxvkConf = Join-Path $PSScriptRoot "../dxvk.conf"
if (Test-Path $DxvkConf) {
    Copy-Item $DxvkConf $GameDir -Force
    Write-Host "OK Copied dxvk.conf" -ForegroundColor Green
} else {
    Write-Host "  (No dxvk.conf found - using DXVK defaults)" -ForegroundColor Gray
}

# 9. Generate run.bat and run.ps1 in game dir (mirrors Linux run.sh pattern)
# GeneralsX @build felipebraz 03/03/2026 Generate launcher wrappers in game dir
Write-Host ""
Write-Host "Generating run launchers in game dir..." -ForegroundColor Green

$GameDirAbs = (Resolve-Path $GameDir).Path

# --- run.bat (double-click friendly, cmd/powershell compatible) ---
$RunBatContent = @"
@echo off
rem GeneralsX @build felipebraz 03/03/2026 Windows launcher wrapper
rem Sets DXVK environment variables and launches the game.
rem Usage: run.bat [-win] [-noshellmap] [-fullscreen] [other args]
rem        Double-click to launch windowed (default flags applied).

setlocal

set "SCRIPT_DIR=%~dp0"

rem ---- DXVK environment ----
if not defined DXVK_LOG_LEVEL set DXVK_LOG_LEVEL=warn
if not defined DXVK_HUD       set DXVK_HUD=0

rem ---- Auto-detect base Generals install path ----
rem Mirrors Linux: if ZH_Generals\ sibling dir exists, point to it.
if not defined CNC_GENERALS_INSTALLPATH (
    if exist "%SCRIPT_DIR%ZH_Generals\" (
        set "CNC_GENERALS_INSTALLPATH=%SCRIPT_DIR%ZH_Generals\"
    )
)

rem ---- Custom Vulkan ICD (optional, for SwiftShader / lavapipe testing) ----
rem Uncomment and set path to use a software Vulkan renderer:
rem set "VK_ICD_FILENAMES=%SCRIPT_DIR%lvp_icd.x86_64.json"

rem ---- Redirect stderr to log file ----
if not exist "%SCRIPT_DIR%logs\" mkdir "%SCRIPT_DIR%logs"

rem ---- Launch ----
cd /d "%SCRIPT_DIR%"
if "%~1"=="" (
    rem No args: default windowed + skip shell map
    "$TargetName.exe" -win -noshellmap 2>"%SCRIPT_DIR%logs\run.log"
) else (
    "$TargetName.exe" %* 2>"%SCRIPT_DIR%logs\run.log"
)

endlocal
"@

Set-Content -Path (Join-Path $GameDirAbs "run.bat") -Value $RunBatContent -Encoding ASCII
Write-Host "OK Generated run.bat" -ForegroundColor Green

# --- run.ps1 (PowerShell, waits for process and tees log to console) ---
$RunPs1Content = @"
# GeneralsX @build felipebraz 03/03/2026 Windows PowerShell launcher wrapper
# Sets DXVK environment variables and launches the game, waiting for it to exit.
# Usage: .\run.ps1 [-win] [-noshellmap] [-fullscreen] [-Debug] [other args]
param(
    [switch]`$Win        = `$true,
    [switch]`$NoShellMap = `$true,
    [switch]`$Fullscreen,
    [switch]`$Debug
)

`$ScriptDir = `$PSScriptRoot
`$Exe       = Join-Path `$ScriptDir '$TargetName.exe'
`$LogDir    = Join-Path `$ScriptDir 'logs'
`$LogFile   = Join-Path `$LogDir    'run.log'

if (-not (Test-Path `$Exe)) {
    Write-Error "Executable not found: `$Exe"
    exit 1
}

New-Item -ItemType Directory -Force -Path `$LogDir | Out-Null

# ---- DXVK environment ----
if (-not `$env:DXVK_LOG_LEVEL) { `$env:DXVK_LOG_LEVEL = if (`$Debug) { 'info' } else { 'warn' } }
if (-not `$env:DXVK_HUD)       { `$env:DXVK_HUD       = '0' }

# ---- Auto-detect base Generals install path ----
if (-not `$env:CNC_GENERALS_INSTALLPATH) {
    `$zhGenerals = Join-Path `$ScriptDir 'ZH_Generals'
    if (Test-Path `$zhGenerals) { `$env:CNC_GENERALS_INSTALLPATH = `$zhGenerals }
}

# ---- Custom Vulkan ICD (optional, uncomment for SwiftShader / lavapipe) ----
# `$env:VK_ICD_FILENAMES = Join-Path `$ScriptDir 'lvp_icd.x86_64.json'

# ---- Build args ----
`$GameArgs = @()
if (`$Fullscreen) { `$GameArgs += '-fullscreen' } else { if (`$Win)        { `$GameArgs += '-win' } }
if (`$NoShellMap) { `$GameArgs += '-noshellmap' }

Write-Host "Launching $TargetName"
Write-Host "  Args : `$(`$GameArgs -join ' ')"
Write-Host "  Log  : `$LogFile"
Write-Host ""

Push-Location `$ScriptDir
try {
    & `$Exe `$GameArgs 2>&1 | Tee-Object -FilePath `$LogFile
} finally {
    Pop-Location
    Remove-Item Env:\DXVK_LOG_LEVEL -ErrorAction SilentlyContinue
    Remove-Item Env:\DXVK_HUD       -ErrorAction SilentlyContinue
}
"@

Set-Content -Path (Join-Path $GameDirAbs "run.ps1") -Value $RunPs1Content -Encoding UTF8
Write-Host "OK Generated run.ps1" -ForegroundColor Green

# Summary
Write-Host ""
Write-Host "=====================================================" -ForegroundColor Cyan
Write-Host "Deploy complete!" -ForegroundColor Green
Write-Host "=====================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Deployed to: $GameDirAbs" -ForegroundColor Cyan
Write-Host ""
Write-Host "Run options:" -ForegroundColor Yellow
Write-Host "  Double-click : $TargetName\Run\run.bat" -ForegroundColor Yellow
Write-Host "  PowerShell   : cd '$GameDirAbs'; .\run.ps1" -ForegroundColor Yellow
Write-Host "  Direct       : cd '$GameDirAbs'; .\$TargetName.exe -win -noshellmap" -ForegroundColor Yellow
Write-Host ""
