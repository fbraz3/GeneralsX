# Setup MSVC BuildTools Environment Variables for Current PowerShell Session
#
# Directly enumerates MSVC/WinSDK paths via vswhere.exe and the Windows registry instead of
# invoking VsDevCmd.bat. This avoids the "input line too long" failure that occurs when
# PATH exceeds 8191 chars (a common problem in large development environments).
#
# GeneralsX @bugfix BenderAI 02/03/2026 Replace VsDevCmd.bat invocation (fails with PATH >8191 chars)
#   with direct path enumeration via vswhere.exe + registry. Sets INCLUDE/LIB/PATH precisely.
#
# Usage:
#   .\scripts\setup_msvc_buildtools_env.ps1
#   .\scripts\setup_msvc_buildtools_env.ps1 -Arch x64 -HostArch x64

param(
    [ValidateSet('x86','x64','arm','arm64')]
    [string]$Arch = 'x86',

    [ValidateSet('x86','x64','arm','arm64')]
    [string]$HostArch = 'x86',

    # Optional: explicit VS installation root (e.g. 'C:\Program Files\Microsoft Visual Studio\2022\Enterprise')
    [string]$VsRoot = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

Write-Host 'Setting up MSVC BuildTools environment variables...' -ForegroundColor Cyan

# ---------------------------------------------------------------------------
# 1. Locate VS installation via vswhere.exe
# ---------------------------------------------------------------------------
function Find-VsInstallPath {
    param([string]$ExplicitRoot)

    if ($ExplicitRoot -and (Test-Path $ExplicitRoot)) {
        return $ExplicitRoot
    }

    $vswhere = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path $vswhere) {
        $found = & $vswhere -latest -products '*' -requires 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64' -property installationPath 2>$null
        if ($found -and (Test-Path $found)) {
            return $found.Trim()
        }
    }

    # Fallback: well-known candidate roots
    foreach ($root in @(
        'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools',
        'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools',
        'C:\Program Files\Microsoft Visual Studio\2022\Community',
        'C:\Program Files\Microsoft Visual Studio\2022\Professional',
        'C:\Program Files\Microsoft Visual Studio\2022\Enterprise'
    )) {
        if (Test-Path (Join-Path $root 'VC\Tools\MSVC')) {
            return $root
        }
    }

    throw 'Visual Studio installation not found. Install VS Build Tools with VC++.'
}

$vsRoot = Find-VsInstallPath -ExplicitRoot $VsRoot

# ---------------------------------------------------------------------------
# 2. Find latest MSVC toolset version
# ---------------------------------------------------------------------------
$msvcToolsRoot = Join-Path $vsRoot 'VC\Tools\MSVC'
if (-not (Test-Path $msvcToolsRoot)) {
    throw "MSVC toolset directory not found: $msvcToolsRoot"
}

$msvcVersion = Get-ChildItem $msvcToolsRoot -Directory |
    Where-Object { $_.Name -match '^\d+\.\d+\.\d+$' } |
    Sort-Object { [Version]$_.Name } |
    Select-Object -Last 1 -ExpandProperty Name

if (-not $msvcVersion) {
    throw "No MSVC toolset found under $msvcToolsRoot"
}

$msvcDir = Join-Path $msvcToolsRoot $msvcVersion

# ---------------------------------------------------------------------------
# 3. Find Windows SDK via registry
# ---------------------------------------------------------------------------
function Find-WindowsSdk {
    $registryPaths = @(
        'HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows Kits\Installed Roots',
        'HKLM:\SOFTWARE\Microsoft\Windows Kits\Installed Roots'
    )
    foreach ($regPath in $registryPaths) {
        if (Test-Path $regPath) {
            $props = Get-ItemProperty $regPath -ErrorAction SilentlyContinue
            $sdkRoot = $props.'KitsRoot10'
            if ($sdkRoot -and (Test-Path $sdkRoot)) {
                # Find latest SDK version
                $sdkLibRoot = Join-Path $sdkRoot 'lib'
                if (Test-Path $sdkLibRoot) {
                    $sdkVersion = Get-ChildItem $sdkLibRoot -Directory |
                        Where-Object { $_.Name -match '^\d+\.\d+\.\d+\.\d+$' } |
                        Sort-Object { [Version]$_.Name } |
                        Select-Object -Last 1 -ExpandProperty Name
                    if ($sdkVersion) {
                        return [PSCustomObject]@{
                            Root    = $sdkRoot.TrimEnd('\')
                            Version = $sdkVersion
                        }
                    }
                }
            }
        }
    }
    throw 'Windows 10/11 SDK not found in registry. Install the Windows SDK.'
}

$winSdk = Find-WindowsSdk

# ---------------------------------------------------------------------------
# 4. Compute all paths for the requested arch
# ---------------------------------------------------------------------------
# Map Arch param to directory names used by MSVC/WinSDK
$archDirMap = @{
    'x86'   = 'x86'
    'x64'   = 'x64'
    'arm'   = 'arm'
    'arm64' = 'arm64'
}
$hostBinDir = "Host$($HostArch.Substring(0,1).ToUpper() + $HostArch.Substring(1))"
$archDir    = $archDirMap[$Arch]

# MSVC bin (compiler, linker, etc.)
$msvcBin    = Join-Path $msvcDir "bin\$hostBinDir\$archDir"

# MSVC include + lib
$msvcInclude = Join-Path $msvcDir 'include'
$msvcLib     = Join-Path $msvcDir "lib\$archDir"

# ATL/MFC include (optional - present when VC ATL component is installed)
$msvcAtlInclude = Join-Path $msvcDir "atlmfc\include"

# Windows SDK paths
$sdkRoot    = $winSdk.Root
$sdkVer     = $winSdk.Version
$sdkInclude = @(
    (Join-Path $sdkRoot "include\$sdkVer\ucrt"),
    (Join-Path $sdkRoot "include\$sdkVer\um"),
    (Join-Path $sdkRoot "include\$sdkVer\shared"),
    (Join-Path $sdkRoot "include\$sdkVer\winrt"),
    (Join-Path $sdkRoot "include\$sdkVer\cppwinrt")
) | Where-Object { Test-Path $_ }

$sdkLib = @(
    (Join-Path $sdkRoot "lib\$sdkVer\ucrt\$archDir"),
    (Join-Path $sdkRoot "lib\$sdkVer\um\$archDir")
) | Where-Object { Test-Path $_ }

$sdkBin = Join-Path $sdkRoot "bin\$sdkVer\$archDir"

# MSVC ATL/MFC lib (optional; VS 18+ BuildTools only ships spectre-mitigated ATL)
# Try non-spectre first, then spectre, for forward compatibility.
$msvcAtlLib = $null
foreach ($atlLibCandidate in @(
    (Join-Path $msvcDir "atlmfc\lib\$archDir"),
    (Join-Path $msvcDir "atlmfc\lib\spectre\$archDir")
)) {
    if (Test-Path $atlLibCandidate) {
        $msvcAtlLib = $atlLibCandidate
        break
    }
}

# ---------------------------------------------------------------------------
# 5. Validate critical paths
# ---------------------------------------------------------------------------
foreach ($critical in @($msvcBin, $msvcInclude, $msvcLib)) {
    if (-not (Test-Path $critical)) {
        throw "Critical MSVC path not found: $critical"
    }
}

# ---------------------------------------------------------------------------
# 6. Clear legacy VC6 environment leakage
# ---------------------------------------------------------------------------
foreach ($name in @('INCLUDE','LIB','LIBPATH','VCINSTALLDIR','VSCommonDir','MSDevDir','MSVCDir','VcOsDir')) {
    Remove-Item "Env:$name" -ErrorAction SilentlyContinue
}

# ---------------------------------------------------------------------------
# 7. Set environment variables
# ---------------------------------------------------------------------------
$env:VCINSTALLDIR = Join-Path $vsRoot 'VC\'

# INCLUDE: MSVC headers + ATL/MFC headers (if present) + Windows SDK headers
$includeList = @($msvcInclude)
if (Test-Path $msvcAtlInclude) { $includeList += $msvcAtlInclude }
$includeList += $sdkInclude
$env:INCLUDE = ($includeList | Where-Object { $_ }) -join ';'

# LIB: MSVC lib + ATL/MFC lib (if found) + Windows SDK lib
$libList = @($msvcLib)
if ($msvcAtlLib) { $libList += $msvcAtlLib }
$libList += $sdkLib
$env:LIB = ($libList | Where-Object { $_ }) -join ';'

# LIBPATH: same as LIB for MSVC
$env:LIBPATH = $env:LIB

# PATH: prepend MSVC bin + WinSDK bin to existing PATH (preserve user tools)
$pathAdditions = @($msvcBin)
if (Test-Path $sdkBin) { $pathAdditions += $sdkBin }
$vsCommonTools = Join-Path $vsRoot 'Common7\Tools'
$vsCommonIde   = Join-Path $vsRoot 'Common7\IDE'
if (Test-Path $vsCommonTools) { $pathAdditions += $vsCommonTools }
if (Test-Path $vsCommonIde)   { $pathAdditions += $vsCommonIde }
$env:PATH = ($pathAdditions -join ';') + ';' + $env:PATH

# ---------------------------------------------------------------------------
# 8. Validate toolchain availability
# ---------------------------------------------------------------------------
$cl = Get-Command cl.exe  -ErrorAction SilentlyContinue
$rc = Get-Command rc.exe  -ErrorAction SilentlyContinue
$mt = Get-Command mt.exe  -ErrorAction SilentlyContinue
$link = Get-Command link.exe -ErrorAction SilentlyContinue

if (-not $cl)   { throw 'cl.exe not found after setting up MSVC environment.' }
if (-not $rc)   { throw 'rc.exe not found after setting up MSVC environment.' }
if (-not $mt)   { throw 'mt.exe not found after setting up MSVC environment.' }
if (-not $link) { throw 'link.exe not found after setting up MSVC environment.' }

Write-Host "✅ MSVC BuildTools environment configured:" -ForegroundColor Green
Write-Host "  VS root:      $vsRoot"                  -ForegroundColor White
Write-Host "  MSVC version: $msvcVersion"             -ForegroundColor White
Write-Host "  WinSDK ver:   $sdkVer"                  -ForegroundColor White
Write-Host "  Arch:         $HostArch -> $Arch"       -ForegroundColor White
Write-Host "  cl.exe:       $($cl.Source)"            -ForegroundColor White
Write-Host "  link.exe:     $($link.Source)"          -ForegroundColor White
Write-Host "  rc.exe:       $($rc.Source)"            -ForegroundColor White
Write-Host "  LIB paths:    $($env:LIB -split ';' | Select-Object -First 2 | ForEach-Object { "`n    - $_" })" -ForegroundColor DarkGray
