#!/usr/bin/env pwsh
# GeneralsX @build GitHub Copilot 20/05/2026 Build Zero Hour target on windows64-deploy preset.

$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Resolve-Path (Join-Path $scriptDir "..\..\..")
Set-Location $projectRoot

# GeneralsX @bugfix GitHub Copilot 20/05/2026 Ensure MinGW/MSYS2 toolchain binaries are available in PATH for task execution.
$msysDir = if ($env:MINGW_PREFIX -and (Test-Path $env:MINGW_PREFIX)) { Split-Path -Parent $env:MINGW_PREFIX }
           elseif ($env:MSYS2_PATH -and (Test-Path $env:MSYS2_PATH)) { $env:MSYS2_PATH }
           elseif ($env:RUNNER_TEMP -and (Test-Path (Join-Path $env:RUNNER_TEMP "msys2\mingw64"))) { Join-Path $env:RUNNER_TEMP "msys2" }
           elseif ($env:RUNNER_TEMP -and (Test-Path (Join-Path $env:RUNNER_TEMP "msys64\mingw64"))) { Join-Path $env:RUNNER_TEMP "msys64" }
           elseif ($env:RUNNER_TEMP -and (Test-Path (Join-Path $env:RUNNER_TEMP "setup-msys2\msys64\mingw64"))) { Join-Path $env:RUNNER_TEMP "setup-msys2\msys64" }
           elseif ($env:RUNNER_TEMP -and (Test-Path (Join-Path $env:RUNNER_TEMP "setup-msys2\mingw64"))) { Join-Path $env:RUNNER_TEMP "setup-msys2" }
           elseif (Test-Path (Join-Path $projectRoot "msys64\mingw64")) { Join-Path $projectRoot "msys64" }
           elseif (Test-Path "C:\msys64\mingw64") { "C:\msys64" }
           else { "" }
if ($msysDir) {
    $env:PATH = "$msysDir\mingw64\bin;$msysDir\usr\bin;" + $env:PATH
    if (-not $env:MINGW_PREFIX -or -not (Test-Path $env:MINGW_PREFIX)) {
        $env:MINGW_PREFIX = "$msysDir\mingw64"
    }
}
$vcpkgRoot = Join-Path $projectRoot "vcpkg"
$env:VCPKG_ROOT = $vcpkgRoot
$env:VCPKG_DEFAULT_TRIPLET = "x64-mingw-dynamic"
$env:VCPKG_DEFAULT_HOST_TRIPLET = "x64-mingw-dynamic"
$env:VCPKG_TARGET_TRIPLET = "x64-mingw-dynamic"

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
