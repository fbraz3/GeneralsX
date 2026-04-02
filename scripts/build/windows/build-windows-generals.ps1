# GeneralsX modern Windows x64 build script for base Generals.
# GeneralsX @build BenderAI 01/04/2026 Configure and build GeneralsX with windows64-deploy preset.

param(
    [string]$Preset = "windows64-deploy",
    [switch]$BuildOnly
)

$ErrorActionPreference = "Stop"

function Get-VcVarsPath {
    $vswherePath = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswherePath) {
        $installationPath = & $vswherePath -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($LASTEXITCODE -eq 0 -and $installationPath) {
            $candidate = Join-Path $installationPath "VC\Auxiliary\Build\vcvars64.bat"
            if (Test-Path $candidate) {
                return $candidate
            }
        }
    }

    $fallback = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    if (Test-Path $fallback) {
        return $fallback
    }

    throw "Unable to locate vcvars64.bat"
}

function Invoke-InMsvcEnv {
    param(
        [string]$Command,
        [string]$LogFile
    )

    $vcvarsPath = Get-VcVarsPath
    $wrapped = "set INCLUDE=&& set LIB=&& set LIBPATH=&& call `"$vcvarsPath`" >nul && $Command"
    $previousErrorAction = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    cmd /d /c $wrapped 2>&1 | Tee-Object -FilePath $LogFile
    $ErrorActionPreference = $previousErrorAction
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..\..\")).Path
Set-Location $RepoRoot

New-Item -ItemType Directory -Path "logs" -Force | Out-Null

if (-not $BuildOnly) {
    Invoke-InMsvcEnv -Command "cmake --preset $Preset" -LogFile "logs/configure_windows.log"
}

Invoke-InMsvcEnv -Command "cmake --build build/$Preset --target g_generals --config RelWithDebInfo" -LogFile "logs/build_generals_windows.log"

Write-Host "Build complete: GeneralsX ($Preset)"
