# GeneralsX modern Windows x64 deploy script for Zero Hour.
# GeneralsX @build BenderAI 02/04/2026 Deploy GeneralsXZH with required Windows runtime DLLs.

param(
    [string]$Preset = "windows64-deploy",
    [string]$RuntimeDir = "$env:USERPROFILE\GeneralsX\GeneralsZH",
    [string]$DxvkDllPath = ""
)

$ErrorActionPreference = "Stop"

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..\..\")).Path
Set-Location $RepoRoot

$exeCandidates = @(
    (Join-Path $RepoRoot "build/$Preset/GeneralsMD/RelWithDebInfo/GeneralsXZH.exe"),
    (Join-Path $RepoRoot "build/$Preset/GeneralsMD/RelWithDebInfo/generalszh.exe"),
    (Join-Path $RepoRoot "build/$Preset/GeneralsMD/GeneralsXZH.exe"),
    (Join-Path $RepoRoot "build/$Preset/GeneralsMD/generalszh.exe")
)
$Exe = $exeCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1

if ($null -eq $Exe) {
    Write-Error "GeneralsXZH executable not found in build/$Preset. Run build-windows-zh.ps1 first."
    exit 1
}

New-Item -ItemType Directory -Path $RuntimeDir -Force | Out-Null
Copy-Item -Path $Exe -Destination (Join-Path $RuntimeDir "GeneralsXZH.exe") -Force

$setEnvScript = Join-Path $RepoRoot "scripts/build/windows/setenv-windows-zh.bat"
if (Test-Path $setEnvScript) {
    Copy-Item -Path $setEnvScript -Destination (Join-Path $RuntimeDir "setenv-windows-zh.bat") -Force
}

# Gather all runtime DLLs required for standalone execution.
$buildConfig = if ($Preset -eq "windows64-testing") { "Debug" } else { "RelWithDebInfo" }
$runtimeDllRegex = @(
    '^SDL3\.dll$',
    '^SDL3_image\.dll$',
    '^OpenAL32\.dll$',
    '^d3d8\.dll$',
    '^av(codec|device|filter|format|util)-[0-9]+\.dll$',
    '^sw(resample|scale)-[0-9]+\.dll$',
    '^libpng.*\.dll$',
    '^zlib1\.dll$'
)

$runtimeSearchDirs = @(
    (Join-Path $RepoRoot "build/$Preset/GeneralsMD/$buildConfig"),
    (Join-Path $RepoRoot "build/$Preset/GeneralsMD"),
    (Join-Path $RepoRoot "build/$Preset/_deps/openal_soft-build"),
    (Join-Path $RepoRoot "build/$Preset/_deps/sdl3-build"),
    (Join-Path $RepoRoot "build/$Preset/_deps/sdl3_image-build"),
    (Join-Path $RepoRoot "build/$Preset/vcpkg_installed/x64-windows/bin")
) | Where-Object { Test-Path $_ }

if ($Preset -eq "windows64-testing") {
    $debugBinPath = Join-Path $RepoRoot "build/$Preset/vcpkg_installed/x64-windows/debug/bin"
    if (Test-Path $debugBinPath) {
        $runtimeSearchDirs += $debugBinPath
    }
}

$dxvkCandidates = @()
if ($DxvkDllPath) {
    $dxvkCandidates += $DxvkDllPath
}
$dxvkCandidates += @(
    (Join-Path $RepoRoot "build/$Preset/GeneralsMD/d3d8.dll"),
    (Join-Path $RepoRoot "build/$Preset/Generals/d3d8.dll"),
    (Join-Path $RepoRoot "GeneralsMD/Run/d3d8.dll"),
    (Join-Path $RepoRoot "Generals/Run/d3d8.dll"),
    "C:/dxvk-runtime/dxvk-2.6/x64/d3d8.dll"
)

$dxvkDll = $dxvkCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if ($null -eq $dxvkDll) {
    Write-Error "Required DXVK runtime not found (d3d8.dll). Provide -DxvkDllPath or install C:/dxvk-runtime/dxvk-2.6/x64/d3d8.dll."
    exit 1
}

$copiedDlls = @{}
foreach ($searchDir in $runtimeSearchDirs) {
    Get-ChildItem -Path $searchDir -File -Filter *.dll | ForEach-Object {
        $dllName = $_.Name
        if ($runtimeDllRegex | Where-Object { $dllName -match $_ }) {
            Copy-Item -Path $_.FullName -Destination (Join-Path $RuntimeDir $dllName) -Force
            $copiedDlls[$dllName.ToLowerInvariant()] = $true
        }
    }
}

Copy-Item -Path $dxvkDll -Destination (Join-Path $RuntimeDir "d3d8.dll") -Force
$copiedDlls["d3d8.dll"] = $true

$requiredDlls = @("SDL3.dll", "OpenAL32.dll", "d3d8.dll")
$missingRequired = $requiredDlls | Where-Object { -not $copiedDlls.ContainsKey($_.ToLowerInvariant()) }
if ($missingRequired.Count -gt 0) {
    Write-Error ("Missing required runtime DLLs in deploy target: {0}" -f ($missingRequired -join ", "))
    exit 1
}

Write-Host ("Copied runtime DLLs: {0}" -f ((Get-ChildItem -Path $RuntimeDir -Filter *.dll | Select-Object -ExpandProperty Name | Sort-Object) -join ", "))
Write-Host "Deploy complete: $RuntimeDir"
