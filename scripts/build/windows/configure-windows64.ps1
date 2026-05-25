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
# GeneralsX @bugfix GitHub Copilot 25/05/2026 Replace invalid vcpkg ZIP download flow with deterministic repo bootstrap (ZIP URL returned 404 in CI).
$vcpkgRoot = Join-Path $projectRoot "vcpkg"
$vcpkgExe = Join-Path $vcpkgRoot "vcpkg.exe"
$vcpkgCommit = "ffc071e0c08432c60c9b64f00334c0227667931b"
# GeneralsX @bugfix GitHub Copilot 25/05/2026 Use a valid MinGW vcpkg triplet by default; previous x86_64-windows value was invalid and failed both Windows CI matrix jobs.
$vcpkgTriplet = if ($env:VCPKG_DEFAULT_TRIPLET) { $env:VCPKG_DEFAULT_TRIPLET } else { "x64-mingw-dynamic" }

if (-not (Test-Path $vcpkgExe)) {
    Write-Host "vcpkg.exe not found. Bootstrapping vcpkg from source..."

    if (-not (Test-Path $vcpkgRoot)) {
        git clone https://github.com/microsoft/vcpkg.git $vcpkgRoot
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Failed to clone vcpkg repository"
            exit $LASTEXITCODE
        }
    }

    Push-Location $vcpkgRoot
    try {
        git fetch --tags --prune
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Failed to fetch vcpkg repository updates"
            exit $LASTEXITCODE
        }

        git checkout $vcpkgCommit
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Failed to checkout vcpkg commit $vcpkgCommit"
            exit $LASTEXITCODE
        }

        & .\bootstrap-vcpkg.bat -disableMetrics
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Failed to bootstrap vcpkg"
            exit $LASTEXITCODE
        }
    }
    finally {
        Pop-Location
    }
}

if (-not (Test-Path $vcpkgExe)) {
    Write-Error "vcpkg.exe is still missing after bootstrap"
    exit 1
}

Write-Host "vcpkg.exe ready at: $vcpkgExe"
Write-Host "Installing packages with lock file (triplet: $vcpkgTriplet)..."
& $vcpkgExe install --triplet $vcpkgTriplet --recurse 2>&1 | Tee-Object -FilePath "logs/vcpkg_install.log"
if ($LASTEXITCODE -ne 0) {
    Write-Error "vcpkg install failed. Check logs/vcpkg_install.log"
    exit $LASTEXITCODE
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
