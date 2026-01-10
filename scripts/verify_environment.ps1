# Windows Development Environment Verification
# Checks all prerequisites and provides setup guidance
#
# Usage: .\verify_environment.ps1 [-Fix]
#

param(
    [switch]$Fix
)

Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║       GeneralsX Windows Environment Verification          ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

$AllChecksPass = $true

# Define checks
$Checks = @(
    @{
        Name = "Git"
        Command = "git --version"
        Critical = $true
        FixGuide = "Download from https://git-scm.com/download/win"
    },
    @{
        Name = "CMake"
        Command = "cmake --version"
        Critical = $true
        FixGuide = "choco install cmake  (requires Chocolatey) OR download from https://cmake.org/download/"
    },
    @{
        Name = "Ninja"
        Command = "ninja --version"
        Critical = $true
        FixGuide = "choco install ninja  (requires Chocolatey) OR download from https://github.com/ninja-build/ninja/releases"
    },
    @{
        Name = "PowerShell (5.0+)"
        Command = "`$PSVersionTable.PSVersion"
        Critical = $false
        FixGuide = "Update Windows or install PowerShell 7+ from https://github.com/PowerShell/PowerShell/releases"
    },
    @{
        Name = "VC6 Portable"
        Path = "C:\VC6\VC6SP6\VC98\BIN\cl.exe"
        Critical = $true
        FixGuide = "Run .\scripts\setup_vc6_portable.ps1"
    },
    @{
        Name = "Git Repository"
        Path = ".git"
        Critical = $true
        FixGuide = "Run: git clone https://github.com/fbraz3/GeneralsX.git"
    },
    @{
        Name = "GeneralsX Build Directory"
        Path = "build/vc6"
        Critical = $false
        FixGuide = "Run: cmake --preset vc6"
    },
    @{
        Name = "GeneralsX Deployment Directory"
        Path = "$env:USERPROFILE\GeneralsX"
        Critical = $false
        FixGuide = "Will be created automatically on first deploy"
    }
)

# Run checks
$Results = @()

foreach ($Check in $Checks) {
    $Result = @{
        Name = $Check.Name
        Status = $null
        Message = ""
        Critical = $Check.Critical
        Fix = $Check.FixGuide
    }
    
    if ($Check.Command) {
        try {
            $Output = Invoke-Expression $Check.Command 2>&1 | Select-Object -First 1
            $Result.Status = "✅"
            $Result.Message = $Output
        } catch {
            $Result.Status = "❌"
            $Result.Message = "Command not found"
            $AllChecksPass = $false
        }
    } elseif ($Check.Path) {
        if (Test-Path $Check.Path) {
            $Result.Status = "✅"
            $Result.Message = "Found"
        } else {
            $Result.Status = "❌"
            $Result.Message = "Not found"
            if ($Check.Critical) {
                $AllChecksPass = $false
            }
        }
    }
    
    $Results += $Result
}

# Display results
Write-Host "📋 Component Status:" -ForegroundColor Cyan
Write-Host ""

foreach ($Result in $Results) {
    $Color = if ($Result.Status -eq "✅") { "Green" } else { "Red" }
    $Critical = if ($Result.Critical) { " [CRITICAL]" } else { "" }
    
    Write-Host "$($Result.Status) $($Result.Name)$Critical" -ForegroundColor $Color
    if ($Result.Message) {
        Write-Host "   $($Result.Message)" -ForegroundColor DarkGray
    }
    
    if ($Result.Status -eq "❌" -and -not $Fix) {
        Write-Host "   Fix: $($Result.Fix)" -ForegroundColor Yellow
    }
}

Write-Host ""

# Summary
if ($AllChecksPass) {
    Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Green
    Write-Host "║           ✅ All critical components verified!             ║" -ForegroundColor Green
    Write-Host "║                  Ready to build GeneralsX                  ║" -ForegroundColor Green
    Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Green
    
    Write-Host ""
    Write-Host "🚀 Quick start:" -ForegroundColor Cyan
    Write-Host "  1. .\scripts\build_zh.ps1" -ForegroundColor White
    Write-Host "  2. .\scripts\deploy_zh.ps1" -ForegroundColor White
    Write-Host "  3. .\scripts\run_zh.ps1" -ForegroundColor White
    
} else {
    Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Red
    Write-Host "║        ❌ Missing critical components                       ║" -ForegroundColor Red
    Write-Host "║            Please fix issues above before building        ║" -ForegroundColor Red
    Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Red
    
    Write-Host ""
    Write-Host "Missing components:" -ForegroundColor Yellow
    
    foreach ($Result in ($Results | Where-Object { $_.Status -eq "❌" -and $_.Critical })) {
        Write-Host "  • $($Result.Name)" -ForegroundColor Yellow
        Write-Host "    $($Result.Fix)" -ForegroundColor DarkGray
    }
    
    exit 1
}

Write-Host ""
Write-Host "📖 Full setup guide: docs/ETC/WINDOWS_SETUP_INSTRUCTIONS.md" -ForegroundColor Cyan
