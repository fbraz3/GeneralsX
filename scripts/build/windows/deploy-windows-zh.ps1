#!/usr/bin/env pwsh
# GeneralsX @build GitHub Copilot 20/05/2026 Deploy Zero Hour Windows bundle from windows64-deploy output.

$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Resolve-Path (Join-Path $scriptDir "..\..\..")
Set-Location $projectRoot

# GeneralsX @bugfix GitHub Copilot 20/05/2026 Ensure MinGW/MSYS2 toolchain binaries are available in PATH for runtime DLL discovery consistency.
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

$buildDir = "build/windows64-deploy"
$exeSrc = Join-Path $buildDir "GeneralsMD/GeneralsXZH.exe"
$bundleDir = "build/bundles/windows-generalsxzh-windows64-deploy"

if (-not (Test-Path $exeSrc)) {
    Write-Error "Executable not found: $exeSrc"
    exit 1
}

New-Item -ItemType Directory -Path $bundleDir -Force | Out-Null
Copy-Item $exeSrc (Join-Path $bundleDir "GeneralsXZH.exe") -Force

# GeneralsX @build fbraz3 29/09/2026 Package DXVK, OpenAL, SDL3, and MinGW runtime DLLs for self-contained bundle.
$runtimeCandidates = @(
    (Join-Path $buildDir "d3d8.dll"),
    (Join-Path $buildDir "dxgi.dll"),
    (Join-Path $buildDir "d3d11.dll"),
    (Join-Path $buildDir "_deps/dxvk_windows-src/x64/d3d8.dll"),
    (Join-Path $buildDir "_deps/dxvk_windows-src/x64/dxgi.dll"),
    (Join-Path $buildDir "_deps/dxvk_windows-src/x64/d3d11.dll"),
    (Join-Path $buildDir "_deps/openal_soft-build/OpenAL32.dll"),
    (Join-Path $buildDir "libgamespy_import.dll"),
    (Join-Path $buildDir "vcpkg_installed/x64-mingw-dynamic/bin/libzlib1.dll")
)

foreach ($dll in $runtimeCandidates) {
    if (Test-Path $dll) {
        Copy-Item $dll $bundleDir -Force
    }
}

# Copy SDL3 and SDL3_image runtime DLLs
$sdlDlls = Get-ChildItem -Path (Join-Path $buildDir "_deps") -Filter "SDL3*.dll" -Recurse -File -ErrorAction SilentlyContinue
foreach ($sdlDll in $sdlDlls) {
    Copy-Item $sdlDll.FullName $bundleDir -Force
}

$mingwBin = if ($env:MINGW_PREFIX -and (Test-Path (Join-Path $env:MINGW_PREFIX "bin"))) {
    Join-Path $env:MINGW_PREFIX "bin"
} elseif ($msysDir -and (Test-Path "$msysDir\mingw64\bin")) {
    "$msysDir\mingw64\bin"
} elseif (Test-Path "C:\msys64\mingw64\bin") {
    "C:\msys64\mingw64\bin"
} elseif (Get-Command x86_64-w64-mingw32-g++.exe -ErrorAction SilentlyContinue) {
    Split-Path (Get-Command x86_64-w64-mingw32-g++.exe).Source
} else {
    $null
}

if ($mingwBin) {
    $mingwDlls = @("libgcc_s_seh-1.dll", "libstdc++-6.dll", "libwinpthread-1.dll", "libpng16-16.dll", "zlib1.dll")
    foreach ($d in $mingwDlls) {
        $p = Join-Path $mingwBin $d
        if (Test-Path $p) {
            Copy-Item $p $bundleDir -Force
        }
    }
    # GeneralsX @build fbraz3 01/10/2026 Copy FFmpeg runtime DLLs for OpenAL audio decoding
    Get-ChildItem -Path $mingwBin -Filter "av*.dll" -File -ErrorAction SilentlyContinue | ForEach-Object { Copy-Item $_.FullName $bundleDir -Force }
    Get-ChildItem -Path $mingwBin -Filter "sw*.dll" -File -ErrorAction SilentlyContinue | ForEach-Object { Copy-Item $_.FullName $bundleDir -Force }
}

Write-Host "Deploy complete: $bundleDir"
