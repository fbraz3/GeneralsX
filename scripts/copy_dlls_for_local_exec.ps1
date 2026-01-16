#!/usr/bin/env pwsh
# Copy DLLs from asset directory to executable directory
# This ensures DLLs are found when running from build directory

param(
    [string]$ExeDir = "$PSScriptRoot/../../../build/vc6/GeneralsMD",
    [string]$AssetDir = "$env:USERPROFILE/.GeneralsX/GeneralsMD/Data"
)

$ExeDir = (Resolve-Path $ExeDir -ErrorAction SilentlyContinue).Path
$AssetDir = (Resolve-Path $AssetDir -ErrorAction SilentlyContinue).Path

if (!$ExeDir) {
    Write-Host "Error: Executable directory not found" -ForegroundColor Red
    exit 1
}

Write-Host "Copying DLLs for local execution..." -ForegroundColor Cyan
Write-Host "From: $AssetDir"
Write-Host "To:   $ExeDir"

if (!(Test-Path $AssetDir)) {
    Write-Host "Warning: Asset directory not found: $AssetDir" -ForegroundColor Yellow
    Write-Host "DLLs might not be copied" -ForegroundColor Yellow
    exit 1
}

$dllFiles = Get-ChildItem "$AssetDir\*.dll" -ErrorAction SilentlyContinue
if (!$dllFiles) {
    Write-Host "Warning: No DLL files found in asset directory" -ForegroundColor Yellow
    exit 1
}

$copiedCount = 0
foreach ($dll in $dllFiles) {
    $target = "$ExeDir\$($dll.Name)"
    Copy-Item -Path $dll.FullName -Destination $target -Force
    Write-Host "  ✓ $($dll.Name)"
    $copiedCount++
}

Write-Host "`nCopied $copiedCount DLL files" -ForegroundColor Green
Write-Host "Ready to run from: $ExeDir" -ForegroundColor Green
