# Setup MSVC BuildTools Environment Variables for Current PowerShell Session
# This script imports the environment produced by VsDevCmd.bat so that cl.exe/rc.exe/mt.exe
# are available on PATH and legacy VC6 INCLUDE/LIB settings do not leak into modern builds.
#
# Usage:
#   .\scripts\setup_msvc_buildtools_env.ps1
#   .\scripts\setup_msvc_buildtools_env.ps1 -Arch x86 -HostArch x86

param(
    [ValidateSet('x86','x64','arm','arm64')]
    [string]$Arch = 'x86',

    [ValidateSet('x86','x64','arm','arm64')]
    [string]$HostArch = 'x86',

    [string]$VsRoot = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Resolve-VsDevCmdPath {
    param([string]$ExplicitRoot)

    $candidateRoots = @()

    if ($ExplicitRoot -and (Test-Path $ExplicitRoot)) {
        $candidateRoots += $ExplicitRoot
    }

    $candidateRoots += @(
        'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools',
        'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools'
    )

    foreach ($root in $candidateRoots) {
        $vsDevCmd = Join-Path $root 'Common7\Tools\VsDevCmd.bat'
        if (Test-Path $vsDevCmd) {
            return $vsDevCmd
        }
    }

    return $null
}

Write-Host 'Setting up MSVC BuildTools environment variables...' -ForegroundColor Cyan

$vsDevCmdPath = Resolve-VsDevCmdPath -ExplicitRoot $VsRoot
if (-not $vsDevCmdPath) {
    throw 'VsDevCmd.bat not found. Install Visual Studio Build Tools (MSVC) and Windows SDK.'
}

# Clear known VC6 environment leakage first.
foreach ($name in @('INCLUDE','LIB','LIBPATH','VCINSTALLDIR','VSCommonDir','MSDevDir','MSVCDir','VcOsDir')) {
    if (Test-Path "Env:$name") {
        Remove-Item "Env:$name" -ErrorAction SilentlyContinue
    }
}

# Import the environment from VsDevCmd.bat into this PowerShell session.
# We call cmd.exe, run VsDevCmd, then dump the resulting environment with `set`.
$cmdLine = "`"$vsDevCmdPath`" -arch=$Arch -host_arch=$HostArch -no_logo && set"
$envDump = cmd.exe /c $cmdLine

foreach ($line in $envDump) {
    if ($line -match '^(?<Name>[^=]+)=(?<Value>.*)$') {
        $name = $Matches['Name']
        $value = $Matches['Value']
        try {
            Set-Item -Path "Env:$name" -Value $value
        } catch {
            # Skip any weird/unsettable environment names.
        }
    }
}

# Validate toolchain availability.
$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
$rc = Get-Command rc.exe -ErrorAction SilentlyContinue
$mt = Get-Command mt.exe -ErrorAction SilentlyContinue

if (-not $cl) { throw 'MSVC cl.exe not found after importing VsDevCmd environment.' }
if (-not $rc) { throw 'Windows SDK rc.exe not found after importing VsDevCmd environment.' }
if (-not $mt) { throw 'Windows SDK mt.exe not found after importing VsDevCmd environment.' }

Write-Host '✅ MSVC BuildTools environment configured:' -ForegroundColor Green
Write-Host "  VsDevCmd: $vsDevCmdPath" -ForegroundColor White
Write-Host "  cl.exe:  $($cl.Source)" -ForegroundColor White
Write-Host "  rc.exe:  $($rc.Source)" -ForegroundColor White
Write-Host "  mt.exe:  $($mt.Source)" -ForegroundColor White
