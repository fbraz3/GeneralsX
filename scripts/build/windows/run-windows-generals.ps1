# GeneralsX modern Windows x64 run script for base Generals.
# GeneralsX @build BenderAI 02/04/2026 Run deployed GeneralsX with runtime environment setup.

param(
    [string]$RuntimeRoot = "$env:USERPROFILE\GeneralsX",
    [string[]]$GameArgs = @("-win")
)

$ErrorActionPreference = "Stop"

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..\..")).Path
Set-Location $RepoRoot

$runtimeRunner = Join-Path $RepoRoot "scripts/build/windows/run-windows-runtime.ps1"
if (-not (Test-Path $runtimeRunner)) {
    Write-Error "Runtime launcher script not found: $runtimeRunner"
    exit 1
}

powershell -ExecutionPolicy Bypass -File $runtimeRunner -Game Generals -RuntimeRoot $RuntimeRoot -GameArgs $GameArgs
