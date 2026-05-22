#!/usr/bin/env pwsh
# GeneralsX @build GitHub Copilot 20/05/2026 Build Zero Hour target on windows64-deploy preset.

$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Resolve-Path (Join-Path $scriptDir "..\..\..")
Set-Location $projectRoot

# GeneralsX @bugfix GitHub Copilot 20/05/2026 Ensure MinGW/MSYS2 toolchain binaries are available in PATH for task execution.
$env:PATH = "C:\msys64\mingw64\bin;C:\msys64\usr\bin;" + $env:PATH

New-Item -ItemType Directory -Path logs -Force | Out-Null
$logFile = "logs/build_windows64_zh.log"

Write-Host "Building target z_generals..."
# GeneralsX @bugfix GitHub Copilot 21/05/2026 Prevent PowerShell from treating CMake stderr warnings as terminating errors.
$previousErrorActionPreference = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
cmake --build --preset windows64-deploy --target z_generals -j4 2>&1 | Tee-Object -FilePath $logFile
$cmakeExitCode = $LASTEXITCODE
$ErrorActionPreference = $previousErrorActionPreference

if ($cmakeExitCode -ne 0) {
    Write-Error "Build failed. Check $logFile"
    exit $cmakeExitCode
}

Write-Host "Build complete. Log: $logFile"
