# Set Ninja in PATH
$ninjaPath = "C:\tools\ninja"
$currentPath = [Environment]::GetEnvironmentVariable("PATH", "User")

if ($currentPath -notlike "*$ninjaPath*") {
    $newPath = "$ninjaPath;$currentPath"
    [Environment]::SetEnvironmentVariable("PATH", $newPath, "User")
    Write-Host "Added $ninjaPath to User PATH" -ForegroundColor Green
} else {
    Write-Host "$ninjaPath already in PATH" -ForegroundColor Green
}

# Also set for current session
$env:PATH = "$ninjaPath;$env:PATH"

# Test
& "$ninjaPath\ninja.exe" --version
