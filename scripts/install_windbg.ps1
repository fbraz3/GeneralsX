# Install Debugging Tools for Windows (CDB.exe)
# Provides automated installation and verification of CDB.exe (Console Debugger)
#
# Usage: .\install_windbg.ps1 [-Method 'winget|choco|manual']
#

param(
    [ValidateSet('auto', 'winget', 'choco', 'manual')]
    [string]$Method = 'auto'
)

$ErrorActionPreference = "Continue"

Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║  Debugging Tools Installation Script (CDB.exe - Console)    ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Check if CDB.exe is already installed
Write-Host "🔍 Checking for existing CDB.exe installation..." -ForegroundColor Cyan

$CdbPaths = @(
    "C:\Program Files (x86)\Windows Kits\10\Debuggers\x86\cdb.exe",
    "C:\Program Files\Windows Kits\10\Debuggers\x86\cdb.exe",
    "C:\Program Files (x86)\Windows Kits\11\Debuggers\x86\cdb.exe",
    "C:\Program Files\Windows Kits\11\Debuggers\x86\cdb.exe"
)

$FoundPath = $null
foreach ($Path in $CdbPaths) {
    if (Test-Path $Path) {
        $FoundPath = $Path
        break
    }
}

if ($FoundPath) {
    Write-Host "✅ CDB.exe already installed at: $FoundPath" -ForegroundColor Green
    try {
        $Version = & $FoundPath -version 2>&1 | Select-Object -First 1
        Write-Host "   Version: $Version" -ForegroundColor Green
    } catch {
        # Version flag might not work with all versions
        Write-Host "   (version check skipped)" -ForegroundColor DarkGray
    }
    Write-Host ""
    $Response = Read-Host "Reinstall anyway? (y/n: default n)"
    if ($Response -ne "y") {
        Write-Host "Exiting without changes." -ForegroundColor Yellow
        exit 0
    }
    Write-Host ""
}

# Determine installation method
Write-Host "📦 Determining installation method..." -ForegroundColor Cyan
Write-Host ""

$MethodUsed = $null

# Try winget (Windows 11+, usually pre-installed)
if ($Method -eq 'auto' -or $Method -eq 'winget') {
    $WinGetExists = $null -ne (Get-Command winget -ErrorAction SilentlyContinue)
    if ($WinGetExists) {
        Write-Host "Found: winget (Windows Package Manager)" -ForegroundColor Yellow
        $Response = Read-Host "Use winget to install? (y/n: default y)"
        if ($Response -ne "n") {
            Write-Host ""
            Write-Host "⬇️  Installing debugger packages via winget..." -ForegroundColor Cyan
            try {
                # NOTE: WinDbg Preview may NOT include cdb.exe.
                # If cdb.exe is not found after install, the script will fall back to manual SDK instructions.
                winget install --id Microsoft.WinDbg -e --accept-source-agreements
                $MethodUsed = 'winget'
                Write-Host "   ✓ winget install completed" -ForegroundColor Green
            } catch {
                Write-Warning "winget installation failed: $_"
                $MethodUsed = $null
            }
        }
    }
}

# Try Chocolatey if winget failed or not available
if (-not $MethodUsed -and ($Method -eq 'auto' -or $Method -eq 'choco')) {
    $ChocoExists = $null -ne (Get-Command choco -ErrorAction SilentlyContinue)
    if ($ChocoExists) {
        Write-Host "Found: Chocolatey (package manager)" -ForegroundColor Yellow
        if (-not $MethodUsed) {
            $Response = Read-Host "Use Chocolatey to install? (y/n: default y)"
            if ($Response -ne "n") {
                Write-Host ""
                Write-Host "⬇️  Installing debugger packages via Chocolatey..." -ForegroundColor Cyan
                try {
                    choco install windbg -y
                    $MethodUsed = 'choco'
                    Write-Host "   ✓ Chocolatey install completed" -ForegroundColor Green
                } catch {
                    Write-Warning "Chocolatey installation failed: $_"
                    $MethodUsed = $null
                }
            }
        }
    } else {
        Write-Host "Chocolatey not found (optional)" -ForegroundColor DarkGray
    }
}

# Manual instructions if automated failed
if (-not $MethodUsed -or $Method -eq 'manual') {
    Write-Host ""
    Write-Host "═════════════════════════════════════════════════════════════" -ForegroundColor Yellow
    Write-Host "📋 Manual Installation Instructions:" -ForegroundColor Yellow
    Write-Host "═════════════════════════════════════════════════════════════" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Option A: Download Windows SDK (includes CDB.exe)" -ForegroundColor White
    Write-Host "  1. Visit: https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/" -ForegroundColor DarkGray
    Write-Host "  2. Click 'Get Windows SDK'" -ForegroundColor DarkGray
    Write-Host "  3. During installation, select 'Debugging Tools for Windows'" -ForegroundColor DarkGray
    Write-Host "  4. Install to default location" -ForegroundColor DarkGray
    Write-Host ""

    Write-Host "Option B: Download Debugging Tools Standalone" -ForegroundColor White
    Write-Host "  1. Visit: https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-download-tools" -ForegroundColor DarkGray
    Write-Host "  2. Click 'Download Debugging Tools for Windows'" -ForegroundColor DarkGray
    Write-Host "  3. Run the installer and follow setup wizard" -ForegroundColor DarkGray
    Write-Host "  4. Default install location: C:\Program Files (x86)\Windows Kits\\" -ForegroundColor DarkGray
    Write-Host ""
    Write-Host "═════════════════════════════════════════════════════════════" -ForegroundColor Yellow

    if ($MethodUsed) {
        Write-Host ""
        Write-Host "After completing manual installation, run this script again to verify." -ForegroundColor Cyan
    }
}

# Verification
Write-Host ""
Write-Host "✔️  Verifying CDB.exe installation..." -ForegroundColor Cyan

$FoundPath = $null
foreach ($Path in $CdbPaths) {
    if (Test-Path $Path) {
        $FoundPath = $Path
        break
    }
}

# Also check PATH
if (-not $FoundPath) {
    $CdbCmd = Get-Command cdb.exe -ErrorAction SilentlyContinue
    if ($CdbCmd) {
        $FoundPath = $CdbCmd.Source
    }
}

if ($FoundPath) {
    Write-Host "   ✓ CDB.exe found at: $FoundPath" -ForegroundColor Green
} else {
    Write-Host "   ⚠️  CDB.exe not found in standard locations" -ForegroundColor Yellow
    Write-Host "   To get CDB.exe, install 'Debugging Tools for Windows' (Windows SDK or standalone tools)" -ForegroundColor Yellow
    Write-Host ""
    exit 1
}

# Summary
Write-Host ""
Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Green
Write-Host "║              ✅ CDB.exe Setup Complete                     ║" -ForegroundColor Green
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Green

Write-Host ""
Write-Host "📍 CDB.exe Location:    $FoundPath" -ForegroundColor Cyan
Write-Host ""
Write-Host "🚀 Next steps:" -ForegroundColor Cyan
Write-Host "  1. Use VS Code task: Crash Analysis GeneralsXZH (CDB)" -ForegroundColor White
Write-Host "  2. Or run manually:" -ForegroundColor White
Write-Host "     cdb.exe -c \"g; kb; q\" C:\Path\To\GeneralsXZH.exe -win" -ForegroundColor White
Write-Host ""
Write-Host "📖 Debugger Documentation: https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/" -ForegroundColor DarkGray
