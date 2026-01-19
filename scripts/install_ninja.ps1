# Install Ninja build system
$ninjaUrl = "https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-win.zip"
$ninjaDir = "C:\tools\ninja"
$zipPath = Join-Path $env:TEMP "ninja-win.zip"

# Create directory
New-Item -ItemType Directory -Force -Path $ninjaDir | Out-Null

# Download
Write-Host "Downloading Ninja from $ninjaUrl..." -ForegroundColor Cyan
Invoke-WebRequest -Uri $ninjaUrl -OutFile $zipPath

# Extract
Write-Host "Extracting Ninja to $ninjaDir..." -ForegroundColor Cyan
Expand-Archive -Path $zipPath -DestinationPath $ninjaDir -Force

# Cleanup
Remove-Item $zipPath -Force

# Verify
if (Test-Path "$ninjaDir\ninja.exe") {
    Write-Host "Ninja installed successfully at $ninjaDir" -ForegroundColor Green
    Write-Host "Adding $ninjaDir to PATH..." -ForegroundColor Cyan

    # Add to PATH for current session
    $env:PATH = "$ninjaDir;$env:PATH"

    # Verify
    ninja --version
} else {
    Write-Host "Error: Ninja installation failed" -ForegroundColor Red
    exit 1
}
