<#
.SYNOPSIS
    Configure and build GeneralsX (Generals Base Game) for Windows x64 (MSVC + Ninja Multi-Config + vcpkg).
#>
param(
    [ValidateSet("win64", "win64-debug")]
    [string]$Preset = "win64",
    [ValidateSet("RelWithDebInfo", "Debug", "Release")]
    [string]$Config = $(if ($Preset -eq "win64-debug") { "Debug" } else { "RelWithDebInfo" }),
    [switch]$Reconfigure
)

$ErrorActionPreference = "Stop"
& (Join-Path $PSScriptRoot "build-windows-zh.ps1") -Preset $Preset -Config $Config -Target "g_generals" -Reconfigure:$Reconfigure
