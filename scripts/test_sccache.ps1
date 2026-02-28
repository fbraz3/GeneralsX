# GeneralsX @build BenderAI 27/02/2026
# Test compiler cache effectiveness (Windows version for sccache)
# Compares cache hit rates and build times before/after cache warm-up

param(
    [string]$Preset = "win64-modern",
    [string]$Target = "z_ww3d2",
    [int]$Jobs = 4,
    [switch]$Full
)

$ErrorActionPreference = "Stop"

# Color helpers
function Write-Header {
    param([string]$Message)
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host $Message -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
}

function Write-Success {
    param([string]$Message)
    Write-Host "✓ $Message" -ForegroundColor Green
}

function Write-Warning {
    param([string]$Message)
    Write-Host "⚠ $Message" -ForegroundColor Yellow
}

# Check if sccache exists
function Test-Sccache {
    try {
        $null = & sccache --version 2>&1
        return $true
    }
    catch {
        return $false
    }
}

# Get sccache stats
function Get-CacheStats {
    $output = & sccache -s 2>&1
    return $output
}

# Main test
function Main {
    Write-Header "Test: Compiler Cache Effectiveness"
    Write-Host ""
    
    # Check if sccache is available
    if (-not (Test-Sccache)) {
        Write-Host "ERROR: sccache not found in PATH" -ForegroundColor Red
        Write-Host ""
        Write-Host "Install with: cargo install sccache"
        Write-Host "Then add to PATH or restart PowerShell"
        exit 1
    }
    
    Write-Host "[1] Initial cache state:"
    $initialStats = Get-CacheStats
    Write-Host $initialStats
    Write-Host ""
    
    if (-not (Test-Path "build/$Preset")) {
        Write-Warning "Build directory not found: build/$Preset"
        Write-Host "Run first: cmake --preset $Preset"
        exit 1
    }
    
    Write-Host "[2] Clearing cache for clean test..."
    & sccache -z 2>&1 | Out-Null
    Write-Success "Cache cleared"
    Write-Host ""
    
    Write-Host "[3] FIRST BUILD (cold cache - NOT cached)..."
    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    & cmake --build "build/$Preset" --target $Target -j $Jobs 2>&1 | Tee-Object -Variable firstBuild -FilePath "logs/cache-test-first-build.log"
    $stopwatch.Stop()
    $firstTime = $stopwatch.Elapsed.TotalSeconds
    
    Write-Host ""
    Write-Host "First build time: ${firstTime}s" -ForegroundColor Yellow
    Write-Host ""
    
    Write-Host "[4] Cache state after first build:"
    $stats1 = Get-CacheStats
    Write-Host $stats1
    Write-Host ""
    
    Write-Host "[5] Clean build (remove intermediate files but keep cache)..."
    & cmake --build "build/$Preset" --target $Target --clean-first 2>&1 | Tee-Object -Variable cleanBuild -FilePath "logs/cache-test-clean.log"
    Write-Host ""
    
    Write-Host "[6] SECOND BUILD (warm cache - should be faster!)..."
    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    & cmake --build "build/$Preset" --target $Target -j $Jobs 2>&1 | Tee-Object -Variable secondBuild -FilePath "logs/cache-test-second-build.log"
    $stopwatch.Stop()
    $secondTime = $stopwatch.Elapsed.TotalSeconds
    
    Write-Host ""
    Write-Host "Second build time: ${secondTime}s" -ForegroundColor Yellow
    Write-Host ""
    
    Write-Host "[7] Final cache state:"
    $stats2 = Get-CacheStats
    Write-Host $stats2
    Write-Host ""
    
    # Calculate speedup
    if ($firstTime -gt 0) {
        $speedup = [Math]::Round($firstTime / $secondTime, 2)
        $timeSaved = [Math]::Round($firstTime - $secondTime, 1)
        
        Write-Header "RESULTS"
        Write-Host "First build:  ${firstTime}s"
        Write-Host "Second build: ${secondTime}s"
        Write-Host "Speedup:      ${speedup}x faster"
        Write-Host "Time saved:   ${timeSaved}s per build"
        Write-Host ""
        
        if ($speedup -ge 2.0) {
            Write-Host "✓ Cache is working WELL (2x+ speedup)" -ForegroundColor Green
        }
        elseif ($speedup -ge 1.5) {
            Write-Host "⚠ Cache is working but could be better (1.5x speedup)" -ForegroundColor Yellow
        }
        else {
            Write-Host "✗ Cache effectiveness is low (<1.5x speedup)" -ForegroundColor Red
            Write-Host "Check: SCCACHE_SLOPPINESS=include_file_mtime is set"
        }
    }
    
    Write-Host ""
    Write-Host "Logs saved to: logs/cache-test-*.log"
}

# Run main
Main
