# GeneralsX modern Windows x64 runtime launcher.
# GeneralsX @build BenderAI 02/04/2026 Launch installed game with runtime environment variables and console output.

param(
    [ValidateSet("Generals", "GeneralsZH")]
    [string]$Game = "GeneralsZH",
    [string]$RuntimeRoot = "$env:USERPROFILE\GeneralsX",
    [string[]]$GameArgs = @("-win")
)

$ErrorActionPreference = "Stop"

$installDir = if ($Game -eq "Generals") {
    Join-Path $RuntimeRoot "Generals"
} else {
    Join-Path $RuntimeRoot "GeneralsZH"
}

$generalsDir = Join-Path $RuntimeRoot "Generals"
$zhDir = Join-Path $RuntimeRoot "GeneralsZH"

$exeName = if ($Game -eq "Generals") { "GeneralsX.exe" } else { "GeneralsXZH.exe" }
$exePath = Join-Path $installDir $exeName

if (-not (Test-Path $exePath)) {
    Write-Error "Runtime executable not found: $exePath. Run deploy-windows-generals.ps1 or deploy-windows-zh.ps1 first."
    exit 1
}

$env:CNC_GENERALS_PATH = $generalsDir
$env:CNC_GENERALS_ZH_PATH = $zhDir
if (-not $env:CNC_GENERALS_INSTALLPATH) {
    $env:CNC_GENERALS_INSTALLPATH = $generalsDir
}
$env:CNC_ZH_INSTALLPATH = $zhDir
$env:PATH = "$installDir;$env:PATH"

Write-Host "CNC_GENERALS_PATH=$($env:CNC_GENERALS_PATH)"
Write-Host "CNC_GENERALS_ZH_PATH=$($env:CNC_GENERALS_ZH_PATH)"
Write-Host "CNC_GENERALS_INSTALLPATH=$($env:CNC_GENERALS_INSTALLPATH)"
Write-Host "CNC_ZH_INSTALLPATH=$($env:CNC_ZH_INSTALLPATH)"

$argsPrintable = ($GameArgs | ForEach-Object {
    if ($_ -match "\s") { '"{0}"' -f $_ } else { $_ }
}) -join " "
Write-Host "Running: $exePath $argsPrintable"

Push-Location $installDir
try {
    $process = Start-Process -FilePath $exePath -ArgumentList $GameArgs -NoNewWindow -Wait -PassThru
    $exitCode = $process.ExitCode
}
finally {
    Pop-Location
}

exit $exitCode
