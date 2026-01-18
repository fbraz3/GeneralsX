# Setup MSVC Environment for CMake Configuration
# This script initializes the environment for MSVC2022 BuildTools compilation

param(
    [string]$VC18Root = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools",
    [string]$WindowsSDKVersion = "10.0.26100.0",
    [string]$WindowsSDKRoot = "C:\Program Files (x86)\Windows Kits\10"
)

Write-Host "Setting up MSVC2022 BuildTools environment..." -ForegroundColor Cyan

# Validate MSVC installation
$CompilerPath = "$VC18Root\VC\Tools\MSVC\14.50.35717\bin\Hostx86\x86\cl.exe"
if (-not (Test-Path $CompilerPath)) {
    Write-Error "MSVC compiler not found at $CompilerPath"
    exit 1
}

# Set compiler paths - use forward slashes for CMake compatibility
$env:CMAKE_C_COMPILER = ($CompilerPath -replace '\\', '/')
$env:CMAKE_CXX_COMPILER = ($CompilerPath -replace '\\', '/')

# Set Windows SDK paths
$WindowsSDKBin = "$WindowsSDKRoot\bin\$WindowsSDKVersion\x86"
$WindowsSDKLib = "$WindowsSDKRoot\Lib\$WindowsSDKVersion\um\x86"
$WindowsSDKInclude = "$WindowsSDKRoot\Include\$WindowsSDKVersion"

# Update PATH to include SDK tools (RC, MT, etc)
$env:PATH = "$WindowsSDKBin;$VC18Root\VC\Tools\MSVC\14.50.35717\bin\Hostx86\x86;$env:PATH"

# Set RC and MT paths explicitly - use forward slashes for CMake
$env:RC = ($WindowsSDKBin + '\rc.exe') -replace '\\', '/'
$env:MT = ($WindowsSDKBin + '\mt.exe') -replace '\\', '/'

# Set library and include paths
$env:LIB = "$WindowsSDKLib;$VC18Root\VC\Tools\MSVC\14.50.35717\lib\x86;$WindowsSDKRoot\Lib\$WindowsSDKVersion\ucrt\x86"
$env:INCLUDE = "$WindowsSDKInclude\um;$WindowsSDKInclude\shared;$WindowsSDKInclude\ucrt;$VC18Root\VC\Tools\MSVC\14.50.35717\include"

Write-Host "✅ Environment configured:" -ForegroundColor Green
Write-Host "  CMAKE_C_COMPILER = $env:CMAKE_C_COMPILER" -ForegroundColor White
Write-Host "  CMAKE_CXX_COMPILER = $env:CMAKE_CXX_COMPILER" -ForegroundColor White
Write-Host "  RC = $env:RC" -ForegroundColor White
Write-Host "  MT = $env:MT" -ForegroundColor White
Write-Host "  PATH includes Windows SDK tools" -ForegroundColor White
Write-Host ""

# Test compiler
Write-Host "Testing MSVC compiler..." -ForegroundColor Yellow
try {
    $output = & $CompilerPath /? 2>&1
    if ($output) {
        Write-Host "✅ Compiler test successful!" -ForegroundColor Green
    }
} catch {
    Write-Warning "Could not test compiler: $_"
}

# Test RC compiler
Write-Host "Testing Resource Compiler (RC)..." -ForegroundColor Yellow
try {
    $rcTest = & $env:RC /? 2>&1
    if ($rcTest) {
        Write-Host "✅ RC compiler test successful!" -ForegroundColor Green
    }
} catch {
    Write-Warning "Could not test RC compiler: $_"
}
