#!/usr/bin/env pwsh
# GeneralsX @build GitHub Copilot 20/05/2026 Configure windows64-deploy preset using project script pattern.

$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Resolve-Path (Join-Path $scriptDir "..\..\..")
Set-Location $projectRoot

# GeneralsX @bugfix GitHub Copilot 20/05/2026 Ensure MinGW/MSYS2 toolchain binaries are available in PATH for task execution.
$env:PATH = "C:\msys64\mingw64\bin;C:\msys64\usr\bin;" + $env:PATH

New-Item -ItemType Directory -Path logs -Force | Out-Null
$logFile = "logs/configure_windows64.log"

Write-Host "Configuring preset windows64-deploy..."
cmake --preset windows64-deploy 2>&1 | Tee-Object -FilePath $logFile

if ($LASTEXITCODE -ne 0) {
    Write-Error "Configure failed. Check $logFile"
    exit $LASTEXITCODE
}

Write-Host "Configure complete. Log: $logFile"
