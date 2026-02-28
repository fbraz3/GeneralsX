#!/usr/bin/env pwsh
# configure_win64_modern.ps1 - Configure win64-modern preset with all bypass flags
# Workarounds:
#   1. ninja-wrapper: intercepts -t recompact (cmake bug: rules.ninja never written for try_compile)
#   2. CMAKE_C_ABI_COMPILED=TRUE: skip main project ABI detection try_compile
#   3. SDL_CPU_X64=1: skip SDL3 CPU architecture detection try_compile

$vsRoot     = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
$vsCMake    = "$vsRoot\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$clFwd      = "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/VC/Tools/MSVC/14.50.35717/bin/Hostx64/x64/cl.exe"
$ninjaWrap  = "C:/tools/ninja-wrapper/ninja.exe"

$env:VCToolsVersion  = "14.50.35717"
$env:VCToolsInstallDir = "$vsRoot\VC\Tools\MSVC\14.50.35717\"
$env:PATH    = "C:\tools\ninja-wrapper;$vsRoot\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;$vsRoot\VC\Tools\MSVC\14.50.35717\bin\Hostx64\x64;C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64;$env:PATH"
$env:INCLUDE = "$vsRoot\VC\Tools\MSVC\14.50.35717\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um;C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared"
$env:LIB     = "$vsRoot\VC\Tools\MSVC\14.50.35717\lib\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64"

Write-Host "-- cmake: $vsCMake"
Write-Host "-- cl:    $clFwd"
Write-Host "-- ninja: $ninjaWrap"

& $vsCMake --preset win64-modern `
    "-DCMAKE_C_COMPILER=$clFwd" `
    "-DCMAKE_CXX_COMPILER=$clFwd" `
    "-DCMAKE_MAKE_PROGRAM=$ninjaWrap" `
    -DCMAKE_C_COMPILER_WORKS=TRUE `
    -DCMAKE_CXX_COMPILER_WORKS=TRUE `
    -DCMAKE_C_ABI_COMPILED=TRUE `
    -DCMAKE_CXX_ABI_COMPILED=TRUE `
    -DCMAKE_C_SIZEOF_DATA_PTR=8 `
    -DCMAKE_CXX_SIZEOF_DATA_PTR=8 `
    -DSDL_CPU_X64=1 `
    -DRTS_BUILD_OPTION_FFMPEG=OFF

$exitCode = $LASTEXITCODE
Write-Host "-- Configure exit code: $exitCode"
exit $exitCode
