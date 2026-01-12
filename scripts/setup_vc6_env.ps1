# Setup VC6 Environment Variables for Current Session
# This script initializes all necessary environment variables for VC6 compilation
#
# Usage: .\scripts\setup_vc6_env.ps1
#

param(
    [string]$VC6Root = "C:\VC6\VC6SP6"
)

Write-Host "Setting up VC6 environment variables..." -ForegroundColor Cyan

# Validate VC6 installation
$CompilerPath = "$VC6Root\VC98\BIN\cl.exe"
if (-not (Test-Path $CompilerPath)) {
    Write-Error "VC6 compiler not found at $CompilerPath"
    Write-Host "Please install VC6 Portable using: .\scripts\setup_vc6_portable.ps1" -ForegroundColor Yellow
    exit 1
}

# Set environment variables
$VSCommonDir = "$VC6Root\Common"
$MSDevDir = "$VSCommonDir\MSDev98"
$MSVCDir = "$VC6Root\VC98"
$VcOsDir = "WINNT"

# Update PATH - prepend VC6 bin directories
$env:PATH = "$MSDevDir\BIN;$MSVCDir\BIN;$VSCommonDir\TOOLS\$VcOsDir;$VSCommonDir\TOOLS;$env:PATH"

# Update INCLUDE - set include paths
$env:INCLUDE = "$MSVCDir\ATL\INCLUDE;$MSVCDir\INCLUDE;$MSVCDir\MFC\INCLUDE"

# Update LIB - set library paths  
$env:LIB = "$MSVCDir\LIB;$MSVCDir\MFC\LIB"

# Set VC6 specific variables
$env:VSCommonDir = $VSCommonDir
$env:MSDevDir = $MSDevDir
$env:MSVCDir = $MSVCDir
$env:VcOsDir = $VcOsDir
$env:VCINSTALLDIR = $MSVCDir

# Set runtime library option (MultiThreaded DLL)
$env:LINK = "/SUBSYSTEM:WINDOWS"

Write-Host "✅ Environment variables configured:" -ForegroundColor Green
Write-Host "  PATH updated with VC6 bin directories" -ForegroundColor White
Write-Host "  INCLUDE = $env:INCLUDE" -ForegroundColor White
Write-Host "  LIB = $env:LIB" -ForegroundColor White
Write-Host ""
Write-Host "Compiler path: $CompilerPath" -ForegroundColor Cyan
Write-Host ""

# Test compiler
Write-Host "Testing VC6 compiler..." -ForegroundColor Yellow
try {
    # Try to run cl.exe with /? flag
    $output = & $CompilerPath /? 2>&1
    if ($output) {
        Write-Host "✅ Compiler test successful!" -ForegroundColor Green
    }
} catch {
    Write-Warning "Could not test compiler: $_"
}
