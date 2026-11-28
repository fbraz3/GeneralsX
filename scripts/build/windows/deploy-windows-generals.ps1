<#
.SYNOPSIS
    Stage a self-contained Windows x64 runtime directory for GeneralsX (base game).
#>
param(
    [string]$Destination = "",
    [ValidateSet("win64", "win64-debug")]
    [string]$Preset = "win64",
    [ValidateSet("RelWithDebInfo", "Debug", "Release")]
    [string]$Config = $(if ($Preset -eq "win64-debug") { "Debug" } else { "RelWithDebInfo" })
)

$ErrorActionPreference = "Stop"
& (Join-Path $PSScriptRoot "deploy-windows-zh.ps1") -Destination $Destination -Preset $Preset -Config $Config -Game "generals"
