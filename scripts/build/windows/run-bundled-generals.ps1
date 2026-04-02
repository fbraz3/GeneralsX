# GeneralsX Windows bundled runtime launcher for base Generals.
# GeneralsX @build BenderAI 02/04/2026 Run bundled GeneralsX with local runtime environment setup.

param(
    [string[]]$GameArgs = @("-win")
)

$ErrorActionPreference = "Stop"

$installDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$runtimeRoot = Split-Path -Parent $installDir
$exePath = Join-Path $installDir "GeneralsX.exe"

if (-not (Test-Path $exePath)) {
    Write-Error "Bundled runtime executable not found: $exePath"
    exit 1
}

$env:CNC_GENERALS_PATH = $installDir
$env:CNC_GENERALS_ZH_PATH = (Join-Path $runtimeRoot "GeneralsZH")
if (-not $env:CNC_GENERALS_INSTALLPATH) {
    $env:CNC_GENERALS_INSTALLPATH = $installDir
}
$env:CNC_ZH_INSTALLPATH = $env:CNC_GENERALS_ZH_PATH
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
