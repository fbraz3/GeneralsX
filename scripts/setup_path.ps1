$ErrorActionPreference = 'Continue'

Write-Host "Setting up system PATH for CDB.exe..." -ForegroundColor Cyan
Write-Host ""

$debuggerPath = "C:\Program Files (x86)\Windows Kits\10\Debuggers"

# Check if directory exists
if (-not (Test-Path $debuggerPath)) {
    Write-Host "✗ Debuggers directory not found: $debuggerPath" -ForegroundColor Red
    exit 1
}

# Get current PATH
$currentPath = [Environment]::GetEnvironmentVariable("PATH", "Machine")

# Check if already added
if ($currentPath -like "*Debuggers*") {
    Write-Host "✓ Debuggers directory already in system PATH" -ForegroundColor Green
    exit 0
}

# Add to PATH
try {
    $newPath = "$debuggerPath;$currentPath"
    [Environment]::SetEnvironmentVariable("PATH", $newPath, "Machine")

    Write-Host "✓ Added to system PATH: $debuggerPath" -ForegroundColor Green
    Write-Host ""

    # Verify
    $verify = [Environment]::GetEnvironmentVariable("PATH", "Machine")
    if ($verify -like "*Debuggers*") {
        Write-Host "✓ PATH configuration verified!" -ForegroundColor Green
    }

    exit 0
}
catch {
    Write-Host "✗ Error: $_" -ForegroundColor Red
    exit 1
}
