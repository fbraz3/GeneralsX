# GeneralsX modern Windows x64 bundle script for base Generals.
# GeneralsX @build BenderAI 02/04/2026 Bundle GeneralsX with required Windows runtime DLLs.

param(
    [string]$Preset = "windows64-deploy",
    [string]$OutputDir = "build\bundles",
    [string]$DxvkDllPath = ""
)

$ErrorActionPreference = "Stop"

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..\..")).Path
Set-Location $RepoRoot

$exeCandidates = @(
    (Join-Path $RepoRoot "build/$Preset/Generals/RelWithDebInfo/generalsv.exe"),
    (Join-Path $RepoRoot "build/$Preset/Generals/generalsv.exe"),
    (Join-Path $RepoRoot "build/$Preset/Generals/RelWithDebInfo/GeneralsX.exe"),
    (Join-Path $RepoRoot "build/$Preset/Generals/RelWithDebInfo/generals.exe"),
    (Join-Path $RepoRoot "build/$Preset/Generals/GeneralsX.exe"),
    (Join-Path $RepoRoot "build/$Preset/Generals/generals.exe")
)
$exePath = $exeCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1

if ($null -eq $exePath) {
    Write-Error "GeneralsX executable not found in build/$Preset. Run build-windows-generals.ps1 first."
    exit 1
}

$outputPath = Join-Path $RepoRoot $OutputDir
$stagingDir = Join-Path $outputPath ("windows-generals-{0}" -f $Preset)
$bundleZip = Join-Path $outputPath ("windows-generalsx-{0}.zip" -f $Preset)
$buildConfig = if ($Preset -eq "windows64-testing") { "Debug" } else { "RelWithDebInfo" }

if (Test-Path $stagingDir) {
    Remove-Item -Path $stagingDir -Recurse -Force
}

New-Item -ItemType Directory -Path $stagingDir -Force | Out-Null
Copy-Item -Path $exePath -Destination (Join-Path $stagingDir "GeneralsX.exe") -Force

$setEnvScript = Join-Path $RepoRoot "scripts/build/windows/setenv-windows-generals.bat"
if (Test-Path $setEnvScript) {
    Copy-Item -Path $setEnvScript -Destination (Join-Path $stagingDir "setenv-windows-generals.bat") -Force
}

$bundleRunScript = Join-Path $RepoRoot "scripts/build/windows/run-bundled-generals.ps1"
if (Test-Path $bundleRunScript) {
    Copy-Item -Path $bundleRunScript -Destination (Join-Path $stagingDir "run.ps1") -Force
}

# Gather all runtime DLLs required for standalone execution.
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
    (Join-Path $RepoRoot "build/$Preset/Generals/$buildConfig"),
    (Join-Path $RepoRoot "build/$Preset/Generals"),
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
    (Join-Path $RepoRoot "build/$Preset/Generals/d3d8.dll"),
    (Join-Path $RepoRoot "build/$Preset/GeneralsMD/d3d8.dll"),
    (Join-Path $RepoRoot "Generals/Run/d3d8.dll"),
    (Join-Path $RepoRoot "GeneralsMD/Run/d3d8.dll"),
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
            Copy-Item -Path $_.FullName -Destination (Join-Path $stagingDir $dllName) -Force
            $copiedDlls[$dllName.ToLowerInvariant()] = $true
        }
    }
}

Copy-Item -Path $dxvkDll -Destination (Join-Path $stagingDir "d3d8.dll") -Force
$copiedDlls["d3d8.dll"] = $true

$requiredDlls = @("SDL3.dll", "OpenAL32.dll", "d3d8.dll")
$missingRequired = $requiredDlls | Where-Object { -not $copiedDlls.ContainsKey($_.ToLowerInvariant()) }
if ($missingRequired.Count -gt 0) {
    Write-Error ("Missing required runtime DLLs in bundle staging: {0}" -f ($missingRequired -join ", "))
    exit 1
}

$extraBundleFiles = @(
    @{
        SourceCandidates = @(
            (Join-Path $RepoRoot "build/$Preset/gamespy.dll"),
            (Join-Path $RepoRoot "build/$Preset/Generals/gamespy.dll"),
            (Join-Path $RepoRoot "build/$Preset/GeneralsMD/gamespy.dll")
        )
        DestinationName = "gamespy.dll"
        Required = $true
    }
)

foreach ($fileEntry in $extraBundleFiles) {
    $sourcePath = $fileEntry.SourceCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if ($null -eq $sourcePath) {
        if ($fileEntry.Required) {
            Write-Error ("Missing required bundle file: {0}" -f $fileEntry.DestinationName)
            exit 1
        }
        continue
    }

    Copy-Item -Path $sourcePath -Destination (Join-Path $stagingDir $fileEntry.DestinationName) -Force
}

Write-Host ("Copied runtime DLLs: {0}" -f ((Get-ChildItem -Path $stagingDir -Filter *.dll | Select-Object -ExpandProperty Name | Sort-Object) -join ", "))

if (Test-Path $bundleZip) {
    Remove-Item $bundleZip -Force
}

New-Item -ItemType Directory -Path $outputPath -Force | Out-Null
Compress-Archive -Path (Join-Path $stagingDir "*") -DestinationPath $bundleZip -Force

Write-Host "Bundle complete: $bundleZip"
