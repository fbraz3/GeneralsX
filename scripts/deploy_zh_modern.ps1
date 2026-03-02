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
    Write-Host "✓ Copied dxvk.conf (enables detailed logging)" -ForegroundColor Green
} else {
    Write-Host "  (No dxvk.conf found - using DXVK defaults)" -ForegroundColor Gray
}

# Summary
Write-Host ""
Write-Host "=====================================================" -ForegroundColor Cyan
Write-Host "Deploy complete!" -ForegroundColor Green
Write-Host "=====================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Deployed to: $GameDir" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "  1. Run: .\scripts\run_zh.ps1 -Preset $Preset -Win" -ForegroundColor Yellow
Write-Host "  2. Or manually run: $TargetName.exe -win" -ForegroundColor Yellow
Write-Host ""
