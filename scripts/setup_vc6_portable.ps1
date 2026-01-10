# Setup VC6 Portable for Windows Development
# This script downloads, verifies, and configures Visual C++ 6 Portable
#
# Usage: .\setup_vc6_portable.ps1
#

param(
    [string]$InstallPath = "C:\VC6",
    [string]$Commit = "001c4bafdcf2ef4b474d693acccd35a91e848f40",
    [string]$ExpectedHash = "D0EE1F6DCEF7DB3AD703120D9FB4FAD49EBCA28F44372E40550348B1C00CA583"
)

$ErrorActionPreference = "Stop"

Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║     VC6 Portable Setup for GeneralsX Development           ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Check if already installed
if (Test-Path "$InstallPath\VC6SP6\VC98\BIN\cl.exe") {
    Write-Host "✅ VC6 is already installed at $InstallPath\VC6SP6" -ForegroundColor Green
    $response = Read-Host "Do you want to reinstall? (y/n)"
    if ($response -ne "y") {
        Write-Host "Exiting without changes." -ForegroundColor Yellow
        exit 0
    }
}

# Create installation directory
Write-Host "📁 Creating installation directory..." -ForegroundColor Cyan
New-Item -ItemType Directory -Force -Path $InstallPath | Out-Null
Write-Host "   ✓ Directory: $InstallPath" -ForegroundColor Green

# Download VC6 Portable
Write-Host ""
Write-Host "⬇️  Downloading VC6 Portable (this may take 5-10 minutes)..." -ForegroundColor Cyan

$DownloadUrl = "https://github.com/itsmattkc/MSVC600/archive/$Commit.zip"
$TempFile = "$env:TEMP\VS6_VisualStudio6_$Commit.zip"
$ReleaseDir = "$InstallPath\MSVC600-$Commit"

try {
    Invoke-WebRequest -Uri $DownloadUrl -OutFile $TempFile -ErrorAction Stop
    Write-Host "   ✓ Download complete" -ForegroundColor Green
} catch {
    Write-Error "Failed to download VC6 portable from $DownloadUrl`nError: $_"
    exit 1
}

# Verify file hash
Write-Host ""
Write-Host "🔐 Verifying file integrity..." -ForegroundColor Cyan

$FileHash = (Get-FileHash -Path $TempFile -Algorithm SHA256).Hash
Write-Host "   Downloaded: $FileHash" -ForegroundColor Yellow
Write-Host "   Expected:   $ExpectedHash" -ForegroundColor Yellow

if ($FileHash -ne $ExpectedHash) {
    Write-Error "Hash verification FAILED!`nFile may be corrupted or tampered with.`nDownloaded: $FileHash`nExpected:   $ExpectedHash"
    Remove-Item $TempFile -Force
    exit 1
}

Write-Host "   ✓ Hash verification passed" -ForegroundColor Green

# Extract archive
Write-Host ""
Write-Host "📦 Extracting VC6 Portable (this may take 1-2 minutes)..." -ForegroundColor Cyan

try {
    Expand-Archive -Path $TempFile -DestinationPath $InstallPath -Force -ErrorAction Stop
    Write-Host "   ✓ Extraction complete" -ForegroundColor Green
} catch {
    Write-Error "Failed to extract VC6 portable`nError: $_"
    Remove-Item $TempFile -Force
    exit 1
}

# Rename to standard location
Write-Host ""
Write-Host "🔧 Configuring VC6 installation..." -ForegroundColor Cyan

if (Test-Path $ReleaseDir) {
    try {
        Move-Item -Path $ReleaseDir -Destination "$InstallPath\VC6SP6" -Force -ErrorAction Stop
        Write-Host "   ✓ Directory structure configured" -ForegroundColor Green
    } catch {
        Write-Error "Failed to move VC6 directory`nError: $_"
        exit 1
    }
} else {
    Write-Error "VC6 release directory not found at $ReleaseDir after extraction"
    exit 1
}

# Clean up temp file
Remove-Item $TempFile -Force -ErrorAction SilentlyContinue

# Verify installation
Write-Host ""
Write-Host "✔️  Verifying VC6 installation..." -ForegroundColor Cyan

$CompilerPath = "$InstallPath\VC6SP6\VC98\BIN\cl.exe"
if (-not (Test-Path $CompilerPath)) {
    Write-Error "VC6 compiler not found at $CompilerPath`nInstallation verification failed!"
    exit 1
}

Write-Host "   ✓ VC6 compiler found: $CompilerPath" -ForegroundColor Green

# Setup environment variables (session only)
Write-Host ""
Write-Host "🌍 Setting up environment variables..." -ForegroundColor Cyan

$VSCommonDir = "$InstallPath\VC6SP6\Common"
$MSDevDir = "$VSCommonDir\msdev98"
$MSVCDir = "$InstallPath\VC6SP6\VC98"
$VcOsDir = "WINNT"

$env:VSCommonDir = $VSCommonDir
$env:MSDevDir = $MSDevDir
$env:MSVCDir = $MSVCDir
$env:VcOsDir = $VcOsDir
$env:PATH = "$MSDevDir\BIN;$MSVCDir\BIN;$VSCommonDir\TOOLS\$VcOsDir;$VSCommonDir\TOOLS;$env:PATH"
$env:INCLUDE = "$MSVCDir\ATL\INCLUDE;$MSVCDir\INCLUDE;$MSVCDir\MFC\INCLUDE;$env:INCLUDE"
$env:LIB = "$MSVCDir\LIB;$MSVCDir\MFC\LIB;$env:LIB"

Write-Host "   ✓ Environment variables configured (session only)" -ForegroundColor Green

# Verify compiler accessibility
Write-Host ""
Write-Host "✅ Testing VC6 compiler..." -ForegroundColor Cyan

try {
    $compilerVersion = & cl.exe 2>&1 | Select-Object -First 1
    Write-Host "   ✓ Compiler version: $compilerVersion" -ForegroundColor Green
} catch {
    Write-Warning "Could not directly invoke cl.exe, but file exists. This may be normal if running from non-admin terminal."
}

# Summary
Write-Host ""
Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Green
Write-Host "║                   ✅ SETUP COMPLETE                         ║" -ForegroundColor Green
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Green

Write-Host ""
Write-Host "📍 Installation location: $InstallPath\VC6SP6" -ForegroundColor Cyan
Write-Host "🔨 Compiler path:         $CompilerPath" -ForegroundColor Cyan
Write-Host ""
Write-Host "⚠️  Note: Environment variables are set for this PowerShell session only."
Write-Host "    To make them permanent, use 'setx' commands or add them to your profile." -ForegroundColor Yellow

Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  1. Configure CMake: cmake --preset vc6" -ForegroundColor White
Write-Host "  2. Build:           cmake --build build/vc6 --target GeneralsXZH -j 4" -ForegroundColor White
Write-Host "  3. Deploy:          Copy-Item 'build/vc6/GeneralsMD/GeneralsXZH.exe' -Destination '$(env:USERPROFILE)/GeneralsX/GeneralsMD/'" -ForegroundColor White
Write-Host ""
