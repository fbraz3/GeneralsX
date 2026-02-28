# GeneralsX @build BenderAI 27/02/2026
# Setup sccache for GeneralsX on Windows
# Speeds up builds by caching compilation artifacts while respecting __TIME__/__DATE__ macros
#
# Requirements:
# - sccache installed on system PATH or via cargo
# - Optional: To use cloud backends, configure AWS_ACCESS_KEY_ID, etc.

param(
    [switch]$Force,
    [switch]$Test,
    [string]$CacheSize = "20G",
    [string]$CachePath = ""
)

$ErrorActionPreference = "Stop"

# Color helpers for output
function Write-Header {
    param([string]$Message)
    Write-Host "========================================" -ForegroundColor Green
    Write-Host $Message -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
}

function Write-Success {
    param([string]$Message)
    Write-Host "✓ $Message" -ForegroundColor Green
}

function Write-Warning {
    param([string]$Message)
    Write-Host "⚠ $Message" -ForegroundColor Yellow
}

function Write-Error-Custom {
    param([string]$Message)
    Write-Host "✗ $Message" -ForegroundColor Red
}

# Check sccache installation
function Test-SccacheInstalled {
    try {
        $version = & sccache --version 2>&1
        return $true
    }
    catch {
        return $false
    }
}

# Get sccache version
function Get-SccacheVersion {
    $version = & sccache --version 2>&1
    return $version.Split("`n")[0]
}

# Show installation instructions
function Show-InstallInstructions {
    Write-Host ""
    Write-Warning "sccache is not installed"
    Write-Host ""
    Write-Host "Install sccache using one of these methods:"
    Write-Host ""
    Write-Host "1) Via Cargo (recommended, requires Rust):"
    Write-Host "   cargo install sccache"
    Write-Host ""
    Write-Host "2) Download precompiled binary:"
    Write-Host "   https://github.com/mozilla/sccache/releases"
    Write-Host "   Then add to PATH or move to C:\Program Files\sccache\"
    Write-Host ""
    Write-Host "3) Via Scoop (if installed):"
    Write-Host "   scoop install sccache"
    Write-Host ""
    Write-Host "4) Via Chocolatey (if installed):"
    Write-Host "   choco install sccache"
    Write-Host ""
}

# Main setup
function Main {
    Write-Header "Setup: sccache for GeneralsX (Windows)"
    Write-Host ""
    
    # Step 1: Check installation
    Write-Host "[1] Checking sccache installation..."
    if (Test-SccacheInstalled) {
        $version = Get-SccacheVersion
        Write-Success "sccache is installed: $version"
    }
    else {
        Write-Error-Custom "sccache is NOT installed"
        Show-InstallInstructions
        
        if (-not $Force) {
            Write-Host ""
            Write-Host "Please install sccache and run this script again." -ForegroundColor Yellow
            exit 1
        }
    }
    Write-Host ""
    
    # Step 2: Configure environment variables
    Write-Host "[2] Configuring sccache environment variables..."
    
    # Default cache directory (AppData is Windows standard)
    if ([string]::IsNullOrWhiteSpace($CachePath)) {
        $CachePath = "$env:APPDATA\.sccache"
    }
    
    # Set environment variables for this session
    $env:SCCACHE_DIR = $CachePath
    $env:SCCACHE_CACHE_SIZE = $CacheSize
    $env:SCCACHE_COMPRESS = "1"
    $env:SCCACHE_SLOPPINESS = "include_file_mtime"
    
    Write-Success "SCCACHE_DIR = $CachePath"
    Write-Success "SCCACHE_CACHE_SIZE = $CacheSize"
    Write-Success "SCCACHE_COMPRESS = 1"
    Write-Success "SCCACHE_SLOPPINESS = include_file_mtime (fixes __TIME__/__DATE__ issue)"
    Write-Host ""
    
    # Step 3: Persist to user environment (optional)
    Write-Host "[3] Making settings persistent (User Environment)..."
    try {
        [Environment]::SetEnvironmentVariable("SCCACHE_DIR", $CachePath, "User")
        [Environment]::SetEnvironmentVariable("SCCACHE_CACHE_SIZE", $CacheSize, "User")
        [Environment]::SetEnvironmentVariable("SCCACHE_COMPRESS", "1", "User")
        [Environment]::SetEnvironmentVariable("SCCACHE_SLOPPINESS", "include_file_mtime", "User")
        Write-Success "Environment variables persisted to User profile"
    }
    catch {
        Write-Warning "Could not set User environment variables: $_"
        Write-Host "Settings will only persist for this PowerShell session"
    }
    Write-Host ""
    
    # Step 4: Create cache directory
    Write-Host "[4] Creating cache directory..."
    if (-not (Test-Path $CachePath)) {
        New-Item -ItemType Directory -Path $CachePath -Force | Out-Null
        Write-Success "Created: $CachePath"
    }
    else {
        Write-Success "Cache directory exists: $CachePath"
    }
    Write-Host ""
    
    # Step 5: Clear old cache (optional)
    Write-Host "[5] Clearing cache statistics..."
    & sccache -z 2>&1 | Out-Null
    Write-Success "Cache cleared"
    Write-Host ""
    
    # Step 6: Show cache status
    Write-Host "[6] Current sccache status:"
    $status = & sccache -s 2>&1
    Write-Host $status
    Write-Host ""
    
    # Step 7: Test cache (if requested)
    if ($Test) {
        Write-Host "[7] Testing sccache (compiling small example)..."
        Write-Warning "Test mode not fully implemented yet"
        Write-Host "You can manually test with: cmake --build build/win64-modern -j4"
        Write-Host ""
    }
    
    Write-Header "Setup COMPLETE!"
    Write-Host ""
    Write-Host "Next steps:"
    Write-Host "  1. Configure your CMake preset: cmake --preset win64-modern"
    Write-Host "  2. Build normally: cmake --build build/win64-modern"
    Write-Host "  3. Monitor cache: sccache -s"
    Write-Host ""
    Write-Host "To disable sccache temporarily:"
    Write-Host "  $env:SCCACHE_DISABLE = 1"
    Write-Host ""
    Write-Host "For debugging cache misses:"
    Write-Host "  $env:SCCACHE_DEBUG = 1"
    Write-Host ""
}

# Run main
Main
