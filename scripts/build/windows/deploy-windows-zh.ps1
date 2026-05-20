#!/usr/bin/env pwsh
# GeneralsX @build GitHub Copilot 20/05/2026 Deploy Zero Hour Windows bundle from windows64-deploy output.

$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Resolve-Path (Join-Path $scriptDir "..\..\..")
Set-Location $projectRoot

# GeneralsX @bugfix GitHub Copilot 20/05/2026 Ensure MinGW/MSYS2 toolchain binaries are available in PATH for runtime DLL discovery consistency.
$env:PATH = "C:\msys64\mingw64\bin;C:\msys64\usr\bin;" + $env:PATH

$buildDir = "build/windows64-deploy"
$exeSrc = Join-Path $buildDir "GeneralsMD/GeneralsXZH.exe"
$bundleDir = "build/bundles/windows-generalsxzh-windows64-deploy"

if (-not (Test-Path $exeSrc)) {
    Write-Error "Executable not found: $exeSrc"
    exit 1
}

New-Item -ItemType Directory -Path $bundleDir -Force | Out-Null
Copy-Item $exeSrc (Join-Path $bundleDir "GeneralsXZH.exe") -Force

$runtimeCandidates = @(
    (Join-Path $buildDir "d3d8.dll"),
    (Join-Path $buildDir "dxgi.dll"),
    (Join-Path $buildDir "d3d11.dll"),
    (Join-Path $buildDir "_deps/openal_soft-build/OpenAL32.dll"),
    (Join-Path $buildDir "libgamespy_import.dll")
)

foreach ($dll in $runtimeCandidates) {
    if (Test-Path $dll) {
        Copy-Item $dll $bundleDir -Force
    }
}

Write-Host "Deploy complete: $bundleDir"
