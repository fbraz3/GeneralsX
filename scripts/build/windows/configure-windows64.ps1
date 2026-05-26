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

# GeneralsX @bugfix GitHub Copilot 25/05/2026 Allow explicit CI/local triplet selection via GENERALSX_VCPKG_TRIPLET while preserving safe fallback behavior.
$vcpkgTriplet = if ($env:GENERALSX_VCPKG_TRIPLET) {
    $env:GENERALSX_VCPKG_TRIPLET
}
elseif ($env:VCPKG_DEFAULT_TRIPLET) {
    $env:VCPKG_DEFAULT_TRIPLET
}
elseif ($env:CI) {
    "x64-windows"
}
elseif (Get-Command x86_64-w64-mingw32-g++.exe -ErrorAction SilentlyContinue) {
    "x64-mingw-dynamic"
}
else {
    "x64-windows"
}

# GeneralsX @bugfix GitHub Copilot 25/05/2026 Allow explicit host triplet override for CI experiments while defaulting host triplet to selected target triplet.
$vcpkgHostTriplet = if ($env:GENERALSX_VCPKG_HOST_TRIPLET) {
    $env:GENERALSX_VCPKG_HOST_TRIPLET
}
elseif ($env:VCPKG_DEFAULT_HOST_TRIPLET) {
    $env:VCPKG_DEFAULT_HOST_TRIPLET
}
else {
    $vcpkgTriplet
}

# GeneralsX @bugfix GitHub Copilot 25/05/2026 Ensure vcpkg resolves the repository-local root before any invocation so inherited C:\vcpkg settings do not trigger stderr warnings.
$env:VCPKG_ROOT = $vcpkgRoot
$env:VCPKG_DEFAULT_TRIPLET = $vcpkgTriplet
$env:VCPKG_DEFAULT_HOST_TRIPLET = $vcpkgHostTriplet
$env:VCPKG_TARGET_TRIPLET = $vcpkgTriplet

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
Write-Host "Installing packages with lock file (triplet: $vcpkgTriplet, host-triplet: $vcpkgHostTriplet)..."
# GeneralsX @bugfix GitHub Copilot 25/05/2026 Prevent stderr warnings from native tools becoming blocking PowerShell error records while preserving real non-zero exit handling.
$previousErrorActionPreference = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
& $vcpkgExe install --vcpkg-root $vcpkgRoot --triplet $vcpkgTriplet --host-triplet $vcpkgHostTriplet --recurse 2>&1 | Tee-Object -FilePath "logs/vcpkg_install.log"
$vcpkgExitCode = $LASTEXITCODE
$ErrorActionPreference = $previousErrorActionPreference
if ($vcpkgExitCode -ne 0) {
    Write-Error "vcpkg install failed. Check logs/vcpkg_install.log"
    exit $vcpkgExitCode
}

# GeneralsX @bugfix GitHub Copilot 25/05/2026 Ensure CMake preset resolves vcpkg packages (glm/gli/zlib) on Windows CI.
Write-Host "Using VCPKG_ROOT=$($env:VCPKG_ROOT)"
Write-Host "Using VCPKG_DEFAULT_HOST_TRIPLET=$($env:VCPKG_DEFAULT_HOST_TRIPLET)"
Write-Host "Using VCPKG_TARGET_TRIPLET=$($env:VCPKG_TARGET_TRIPLET)"

Write-Host "Configuring preset windows64-deploy..."
# GeneralsX @bugfix GitHub Copilot 21/05/2026 Prevent PowerShell from treating CMake stderr warnings as terminating errors.
$previousErrorActionPreference = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
cmake --preset windows64-deploy -Wno-dev 2>&1 | Tee-Object -FilePath $logFile
$cmakeExitCode = $LASTEXITCODE
$ErrorActionPreference = $previousErrorActionPreference

if ($cmakeExitCode -ne 0) {
    Write-Error "Configure failed. Check $logFile"
    exit $cmakeExitCode
}

Write-Host "Configure complete. Log: $logFile"
