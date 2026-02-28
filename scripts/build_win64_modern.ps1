# Build script for win64-modern preset
# Usage: powershell -NoProfile -ExecutionPolicy Bypass -File scripts\build_win64_modern.ps1

$vsRoot = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
$cmake  = "$vsRoot\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$ninja  = "$vsRoot\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
$clDir  = "$vsRoot\VC\Tools\MSVC\14.50.35717\bin\Hostx64\x64"
$wdk    = "C:\Program Files (x86)\Windows Kits\10"

$env:PATH    = "C:\tools\ninja-wrapper;$vsRoot\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;$vsRoot\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;$clDir;$wdk\bin\10.0.26100.0\x64;$($env:PATH)"
$env:INCLUDE = "$vsRoot\VC\Tools\MSVC\14.50.35717\include;$wdk\Include\10.0.26100.0\ucrt;$wdk\Include\10.0.26100.0\um;$wdk\Include\10.0.26100.0\shared"
$env:LIB     = "$vsRoot\VC\Tools\MSVC\14.50.35717\lib\x64;$wdk\Lib\10.0.26100.0\um\x64;$wdk\Lib\10.0.26100.0\ucrt\x64"

$target  = if ($args[0]) { $args[0] } else { "z_generals" }
$jobs    = if ($args[1]) { $args[1] } else { "4" }

New-Item -ItemType Directory -Force -Path logs | Out-Null

Write-Host "Building target: $target with $jobs jobs"
$logFile = "logs\build_v2_$(Get-Date -Format 'HHmmss').log"
& $cmake --build build/win64-modern --target $target -j $jobs 2>&1 | Tee-Object -FilePath $logFile
Write-Host "BUILD EXIT: $LASTEXITCODE"
Write-Host "Log: $logFile"
