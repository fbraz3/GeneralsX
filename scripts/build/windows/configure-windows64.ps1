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

# GeneralsX @bugfix GitHub Copilot 21/05/2026 Execute vcpkg to install required packages (GLM, GLI, zlib) before CMake
Write-Host "Installing dependencies via vcpkg..."
if (Test-Path "vcpkg.exe") {
    Write-Host "vcpkg.exe found. Installing packages with lock file..."
    & vcpkg.exe install --triplet x86_64-windows --recurse 2>&1 | Tee-Object -FilePath "logs/vcpkg_install.log"
} else {
    Write-Host "vcpkg.exe not found. Attempting to download..."
    $vcpkgUrl = "https://github.com/microsoft/vcpkg/archive/refs/tags/2024.11.11.zip"
    Invoke-WebRequest -Uri $vcpkgUrl -OutFile "vcpkg.zip" -UseBasicParsing
    Expand-Archive -Path "vcpkg.zip" -DestinationPath "vcpkg_extract" -Force
    Move-Item -Path "vcpkg_extract\vcpkg\vcpkg.exe" -Destination "vcpkg.exe" -Force
    Remove-Item -Path "vcpkg.zip", "vcpkg_extract" -Recurse -Force
    Write-Host "vcpkg downloaded and extracted successfully."
    Write-Host "Installing packages with lock file..."
    & vcpkg.exe install --triplet x86_64-windows --recurse 2>&1 | Tee-Object -FilePath "logs/vcpkg_install.log"
}

Write-Host "Configuring preset windows64-deploy..."
# GeneralsX @bugfix GitHub Copilot 21/05/2026 Prevent PowerShell from treating CMake stderr warnings as terminating errors.
$previousErrorActionPreference = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
cmake --preset windows64-deploy 2>&1 | Tee-Object -FilePath $logFile
$cmakeExitCode = $LASTEXITCODE
$ErrorActionPreference = $previousErrorActionPreference

if ($cmakeExitCode -ne 0) {
    Write-Error "Configure failed. Check $logFile"
    exit $cmakeExitCode
}

Write-Host "Configure complete. Log: $logFile"
