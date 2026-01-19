# WinDbg Installation Script for GeneralsX
# Installs WinDbg (Windows Debugger) from Windows SDK

param(
    [string]$WinDbgPath = 'C:\Program Files (x86)\Windows Kits\10\Debuggers\x86\windbg.exe'
)

Write-Host "Installing WinDbg (Windows Debugger)..." -ForegroundColor Cyan

# Method 1: Try to find WinDbg in Windows Kits (most common)
if (Test-Path $WinDbgPath) {
    Write-Host "✅ WinDbg already installed at: $WinDbgPath" -ForegroundColor Green
    exit 0
}

Write-Host "WinDbg not found at standard location." -ForegroundColor Yellow
Write-Host ""

# Method 2: Try winget (if available)
try {
    $wingetResult = winget --version 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Attempting installation via winget..." -ForegroundColor Cyan
        winget install Microsoft.WinDbg -e
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✅ WinDbg installed via winget" -ForegroundColor Green
            exit 0
        }
    }
} catch {
    Write-Host "winget not available, skipping..." -ForegroundColor Gray
}

# Method 3: Manual installation instructions
Write-Host ""
Write-Host "⚠️  Manual WinDbg Installation Required" -ForegroundColor Yellow
Write-Host ""
Write-Host "Option A: Download from Microsoft (Recommended)" -ForegroundColor Cyan
Write-Host "1. Go to: https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-download-tools" -ForegroundColor White
Write-Host "2. Download 'Debugging Tools for Windows (WinDbg)'" -ForegroundColor White
Write-Host "3. Run installer and accept default location" -ForegroundColor White
Write-Host "4. Verify with: Get-Command windbg.exe" -ForegroundColor White
Write-Host ""

Write-Host "Option B: Install via Windows SDK" -ForegroundColor Cyan
Write-Host "1. Download Windows SDK from: https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/" -ForegroundColor White
Write-Host "2. During installation, select 'Debugging Tools for Windows'" -ForegroundColor White
Write-Host "3. Complete installation" -ForegroundColor White
Write-Host ""

Write-Host "After installation, verify with:" -ForegroundColor Cyan
Write-Host "  windbg.exe -version" -ForegroundColor Gray
Write-Host ""

# Try to find WinDbg in PATH (user might have already installed)
try {
    $found = Get-Command windbg.exe -ErrorAction SilentlyContinue
    if ($found) {
        Write-Host "✅ WinDbg found in PATH: $($found.Source)" -ForegroundColor Green
        exit 0
    }
} catch {
    # Not found
}

Write-Host "❌ WinDbg not found. Please install using one of the methods above." -ForegroundColor Red
Write-Host "   Without WinDbg, you cannot use the Debug tasks in VS Code." -ForegroundColor Yellow
exit 1
