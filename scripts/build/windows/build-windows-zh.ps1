<#
.SYNOPSIS
    Configure and build GeneralsXZH (Zero Hour) for Windows x64 (MSVC + Ninja Multi-Config + vcpkg).
#>
param(
    [ValidateSet("win64", "win64-debug")]
    [string]$Preset = "win64",
    [ValidateSet("RelWithDebInfo", "Debug", "Release")]
    [string]$Config = $(if ($Preset -eq "win64-debug") { "Debug" } else { "RelWithDebInfo" }),
    [string]$Target = "z_generals",
    [switch]$Reconfigure
)

$ErrorActionPreference = "Stop"
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..\..")).Path

$pyCandidate = Join-Path $env:LOCALAPPDATA "Programs\Python\Python312"
if (Test-Path (Join-Path $pyCandidate "python.exe")) {
    $env:PATH = "$pyCandidate;$env:PATH"
}
if (-not (Get-Command ninja.exe -ErrorAction SilentlyContinue)) {
    $ninjaMatch = Get-ChildItem (Join-Path $env:LOCALAPPDATA "Microsoft\WinGet\Packages\Ninja-build.Ninja*") -Filter "ninja.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($ninjaMatch) {
        $env:PATH = "$($ninjaMatch.DirectoryName);$env:PATH"
    }
}
if (-not (Get-Command sccache.exe -ErrorAction SilentlyContinue) -and -not (Get-Command ccache.exe -ErrorAction SilentlyContinue)) {
    $cacheMatch = Get-ChildItem (Join-Path $env:LOCALAPPDATA "Microsoft\WinGet\Packages\Mozilla.sccache*") -Filter "sccache.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $cacheMatch) {
        $cacheMatch = Get-ChildItem (Join-Path $env:LOCALAPPDATA "Microsoft\WinGet\Packages\Ccache.Ccache*") -Filter "ccache.exe" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
    }
    if ($cacheMatch) {
        $env:PATH = "$($cacheMatch.DirectoryName);$env:PATH"
    }
}

$preferredVcpkgRoot = if ($env:VCPKG_ROOT) { $env:VCPKG_ROOT } elseif (Test-Path "C:\vcpkg\vcpkg.exe") { "C:\vcpkg" } else { "" }

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    if (-not (Test-Path $vswhere)) {
        throw "Visual Studio 2022 / Build Tools not found (missing $vswhere)."
    }
    $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath | Select-Object -First 1
    if (-not $vsPath) {
        throw "No Visual Studio installation with VC x64 tools found."
    }
    $vcvars = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
    if (-not (Test-Path $vcvars)) {
        throw "vcvars64.bat not found at $vcvars"
    }
    Write-Host "Loading MSVC x64 environment from $vcvars"
    $vcEnvLines = cmd /c "call `"$vcvars`" >nul && set"
    if ($LASTEXITCODE -ne 0) {
        throw "vcvars64.bat failed ($LASTEXITCODE)"
    }
    $vcEnvLines | ForEach-Object {
        if ($_ -match "^([^=]+)=(.*)$") {
            [Environment]::SetEnvironmentVariable($matches[1], $matches[2], "Process")
        }
    }
    if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
        throw "Failed to initialize MSVC x64 environment (cl.exe not on PATH)."
    }
}

# vcvars64.bat sets VCPKG_ROOT to VS's bundled copy; restore caller/default C:\vcpkg.
if ($preferredVcpkgRoot) {
    $env:VCPKG_ROOT = $preferredVcpkgRoot
}
if (-not $env:VCPKG_ROOT) {
    throw "VCPKG_ROOT is not set and C:\vcpkg\vcpkg.exe was not found."
}
# Ensure MSVC outputs English "Note: including file:" for Ninja /showIncludes filtering.
if (-not $env:VSLANG) {
    $env:VSLANG = "1033"
}

$cmakeConfigureArgs = @()
if (-not $env:GITHUB_ACTIONS -and $RepoRoot.Length -gt 55) {
    # Windows MAX_PATH (260 chars) breaks deep vcpkg buildtrees under worktree paths.
    $cmakeConfigureArgs += "-DVCPKG_INSTALL_OPTIONS=--x-buildtrees-root=C:/vcpkg_bld"
}

Push-Location $RepoRoot
try {
    $buildDir = Join-Path $RepoRoot "build\$Preset"
    if ($Reconfigure -or -not (Test-Path (Join-Path $buildDir "CMakeCache.txt"))) {
        Write-Host "Configuring CMake preset '$Preset'..."
        cmake --preset $Preset @cmakeConfigureArgs
        if ($LASTEXITCODE -ne 0) { throw "CMake configure failed ($LASTEXITCODE)" }
    }

    Write-Host "Building target '$Target' ($Config)..."
    cmake --build $buildDir --config $Config --target $Target
    if ($LASTEXITCODE -ne 0) { throw "CMake build failed ($LASTEXITCODE)" }

    Write-Host "Build completed cleanly in $buildDir"
} finally {
    Pop-Location
}
