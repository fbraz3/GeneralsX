<#
.SYNOPSIS
    Stage a self-contained Windows x64 runtime directory (exe, pdb, vcpkg DLLs, SDL3 DLLs, generalsx.big, and fonts).
#>
param(
    [string]$Destination = "",
    [ValidateSet("win64", "win64-debug")]
    [string]$Preset = "win64",
    [ValidateSet("RelWithDebInfo", "Debug", "Release")]
    [string]$Config = $(if ($Preset -eq "win64-debug") { "Debug" } else { "RelWithDebInfo" }),
    [ValidateSet("zh", "generals")]
    [string]$Game = "zh"
)

$ErrorActionPreference = "Stop"
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..\..")).Path
$BuildDir = Join-Path $RepoRoot "build\$Preset"

if ($Game -eq "zh") {
    $Folder = "GeneralsMD"
    $ExeName = "GeneralsXZH.exe"
    $PdbName = "GeneralsXZH.pdb"
    if (-not $Destination) { $Destination = Join-Path $RepoRoot "dist\windows-x64-GeneralsXZH" }
} else {
    $Folder = "Generals"
    $ExeName = "GeneralsX.exe"
    $PdbName = "GeneralsX.pdb"
    if (-not $Destination) { $Destination = Join-Path $RepoRoot "dist\windows-x64-GeneralsX" }
}

$ExePath = Join-Path $BuildDir "$Folder\$Config\$ExeName"
$PdbPath = Join-Path $BuildDir "$Folder\$Config\$PdbName"
if (-not (Test-Path $ExePath)) {
    throw "Built executable not found: $ExePath. Run build-windows-$(if ($Game -eq 'zh') {'zh'} else {'generals'}).ps1 first."
}

New-Item -ItemType Directory -Force -Path $Destination | Out-Null
Copy-Item -Force $ExePath (Join-Path $Destination $ExeName)
if (Test-Path $PdbPath) {
    Copy-Item -Force $PdbPath (Join-Path $Destination $PdbName)
}

$BigPath = Join-Path $BuildDir "generalsx.big"
if (Test-Path $BigPath) {
    Copy-Item -Force $BigPath (Join-Path $Destination "generalsx.big")
}

$FontsSrc = Join-Path $RepoRoot "assets\fonts"
if (Test-Path $FontsSrc) {
    $FontsDst = Join-Path $Destination "fonts"
    New-Item -ItemType Directory -Force -Path $FontsDst | Out-Null
    Copy-Item -Force (Join-Path $FontsSrc "*") $FontsDst
}

foreach ($binDir in @(
    (Join-Path $BuildDir "vcpkg_installed\x64-windows\bin"),
    (Join-Path $RepoRoot "vcpkg_installed\x64-windows\bin")
)) {
    if (Test-Path $binDir) {
        Get-ChildItem -Path $binDir -Filter "*.dll" | ForEach-Object {
            Copy-Item -Force $_.FullName (Join-Path $Destination $_.Name)
        }
    }
}

# Stage SDL3.dll and SDL3_image.dll when SAGE_USE_SDL3=ON
$cacheFile = Join-Path $BuildDir "CMakeCache.txt"
$sdlEnabled = (Test-Path $cacheFile) -and ($null -ne (Select-String -Path $cacheFile -Pattern "^SAGE_USE_SDL3:BOOL=ON$"))
if ($sdlEnabled) {
    foreach ($sdlDll in @("SDL3.dll", "SDL3_image.dll")) {
        $candidates = @(
            (Join-Path $BuildDir "_deps\sdl3-build\$Config\$sdlDll"),
            (Join-Path $BuildDir "_deps\sdl3_image-build\$Config\$sdlDll"),
            (Join-Path $BuildDir "$Folder\$Config\$sdlDll"),
            (Join-Path $BuildDir "$Config\$sdlDll")
        )
        $srcDll = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
        if (-not $srcDll) {
            $found = Get-ChildItem -Path $BuildDir -Filter $sdlDll -Recurse -ErrorAction SilentlyContinue |
                Sort-Object { if ($_.FullName -match [regex]::Escape("\$Config\")) { 0 } else { 1 } } |
                Select-Object -First 1
            if ($found) { $srcDll = $found.FullName }
        }
        if (-not $srcDll) {
            throw "SAGE_USE_SDL3 is ON but $sdlDll was not found under $BuildDir"
        }
        Copy-Item -Force $srcDll (Join-Path $Destination $sdlDll)
    }
}

Write-Host "Deployed Windows x64 runtime to $Destination"
