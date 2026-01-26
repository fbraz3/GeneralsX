$ErrorActionPreference = 'Continue'

function Write-Log {
	param(
		[Parameter(Mandatory = $true)][AllowEmptyString()][string]$Message,
		[Parameter(Mandatory = $false)][string]$Color
	)

	if ($Color) {
		Write-Host $Message -ForegroundColor $Color
	} else {
		Write-Host $Message
	}

	Write-Output $Message
}

function Resolve-CdbPath {
	# First try PATH
	$cmd = Get-Command cdb.exe -ErrorAction SilentlyContinue
	if ($cmd -and $cmd.Source) {
		return $cmd.Source
	}

	# Then check known installation directories
	$candidates = @(
		"$env:ProgramFiles(x86)\Windows Kits\10\Debuggers\x64\cdb.exe",
		"$env:ProgramFiles(x86)\Windows Kits\10\Debuggers\x86\cdb.exe",
		"$env:ProgramFiles\Windows Kits\10\Debuggers\x64\cdb.exe",
		"$env:ProgramFiles\Windows Kits\10\Debuggers\x86\cdb.exe",
		"$env:ProgramFiles(x86)\Windows Kits\11\Debuggers\x64\cdb.exe",
		"$env:ProgramFiles(x86)\Windows Kits\11\Debuggers\x86\cdb.exe",
		"$env:ProgramFiles\Windows Kits\11\Debuggers\x64\cdb.exe",
		"$env:ProgramFiles\Windows Kits\11\Debuggers\x86\cdb.exe"
	)

	return ($candidates | Where-Object { Test-Path $_ } | Select-Object -First 1)
}

function Get-DebuggersDirectory {
	# Get the debuggers directory from found cdb.exe path
	$cdbPath = Resolve-CdbPath
	if (-not $cdbPath) {
		return $null
	}

	# Return the debuggers directory (one level up from the architecture folder)
	return Split-Path -Parent (Split-Path -Parent $cdbPath)
}

function Add-DebuggersToPath {
	param([string]$DebuggersDirPath)

	if (-not $DebuggersDirPath) {
		Write-Log "✗ Debuggers directory path is invalid" "Red"
		return $false
	}

	# Check if directory exists
	if (-not (Test-Path $DebuggersDirPath)) {
		Write-Log "✗ Debuggers directory not found: $DebuggersDirPath" "Red"
		return $false
	}

	# Get current system PATH
	$currentPath = [Environment]::GetEnvironmentVariable("PATH", "Machine")

	# Check if debuggers directory is already in PATH
	if ($currentPath -like "*$DebuggersDirPath*") {
		Write-Log "✓ Debuggers directory already in system PATH" "Green"
		return $true
	}

	# Add to system PATH
	try {
		$newPath = "$DebuggersDirPath;$currentPath"
		[Environment]::SetEnvironmentVariable("PATH", $newPath, "Machine")
		Write-Log "✓ Added debuggers directory to system PATH: $DebuggersDirPath" "Green"

		# Refresh PATH in current session
		$env:PATH = $newPath
		return $true
	}
	catch {
		Write-Log "✗ Failed to add debuggers to system PATH (may require admin)" "Red"
		Write-Log "  Error: $_" "Red"
		return $false
	}
}

Write-Log "" ""
Write-Log "════════════════════════════════════════════════════════════" "Cyan"
Write-Log "  Debugging Tools for Windows (CDB.exe) Installer" "Cyan"
Write-Log "════════════════════════════════════════════════════════════" "Cyan"
Write-Log "" ""

# Step 1: Check if cdb.exe already exists
Write-Log "Step 1: Searching for existing CDB.exe installation..." "Yellow"
$existingCdb = Resolve-CdbPath
if ($existingCdb) {
	Write-Log "✓ Found cdb.exe at: $existingCdb" "Green"

	# Try to add to PATH if not already there
	$debuggerDir = Get-DebuggersDirectory
	if ($debuggerDir) {
		Add-DebuggersToPath $debuggerDir | Out-Null
	}

	Write-Log "" ""
	exit 0
}

Write-Log "✗ CDB.exe not found in PATH or common Windows Kits locations" "Yellow"
Write-Log "" ""

# Step 2: Check if Visual Studio installer exists
Write-Log "Step 2: Locating Visual Studio Installer..." "Yellow"

$vsInstallerPath = "$env:ProgramFiles\Microsoft Visual Studio\Installer\vs_installer.exe"
$vsSetupExePath = "$env:ProgramFiles\Microsoft Visual Studio\Installer\VisualStudioSetup.exe"

if ([string]::IsNullOrEmpty($vsInstallerPath) -or -not (Test-Path $vsInstallerPath)) {
	$vsInstallerPath = $null
}

if (-not (Test-Path $vsSetupExePath)) {
	$vsSetupExePath = $null
}

$vsInstallerFound = $vsInstallerPath -or $vsSetupExePath

if (-not $vsInstallerFound) {
	Write-Log "✗ Visual Studio Installer not found" "Yellow"
	Write-Log "" ""
	Write-Log "Manual Installation Required:" "Cyan"
	Write-Log "" ""
	Write-Log "Option A (Recommended): Use Visual Studio Installer Configuration" "White"
	Write-Log "  1. Download Visual Studio: https://visualstudio.microsoft.com/downloads/" "DarkGray"
	Write-Log "  2. Run the installer, select 'Modify' if VS is already installed" "DarkGray"
	Write-Log "  3. Look for 'Windows Driver Kit' component or 'Debugging Tools for Windows'" "DarkGray"
	Write-Log "  4. Install and complete setup" "DarkGray"
	Write-Log "  5. Re-run this script" "DarkGray"
	Write-Log "" ""
	Write-Log "Option B (Direct Download): Get Windows SDK" "White"
	Write-Log "  1. Download Windows SDK: https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/" "DarkGray"
	Write-Log "  2. Run installer, select 'Debugging Tools for Windows'" "DarkGray"
	Write-Log "  3. Complete installation and re-run this script" "DarkGray"
	Write-Log "" ""
	exit 1
}

Write-Log "✓ Visual Studio Installer found" "Green"
Write-Log "" ""

# Step 3: Use Visual Studio Installer with .vsconfig
Write-Log "Step 3: Installing via Visual Studio Installer..." "Yellow"
Write-Log "" ""

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$vsconfigPath = Join-Path $scriptDir "..\assets\.vsconfig"

if (-not (Test-Path $vsconfigPath)) {
	Write-Log "✗ .vsconfig file not found at: $vsconfigPath" "Red"
	Write-Log "" ""
	exit 1
}

Write-Log "Using configuration file: $vsconfigPath" "DarkGray"

# Use the found installer
$installerCmd = $vsInstallerPath ? $vsInstallerPath : $vsSetupExePath

try {
	Write-Log "Running Visual Studio Installer (this may take several minutes)..." "Cyan"
	Write-Log "" ""

	# Run the installer with the .vsconfig file
	# The --config flag points to the .vsconfig file for automated component selection
	$process = Start-Process -FilePath $installerCmd `
		-ArgumentList @("--config `"$vsconfigPath`"") `
		-NoNewWindow `
		-Wait `
		-PassThru

	if ($process.ExitCode -ne 0) {
		Write-Log "⚠ Installer returned exit code: $($process.ExitCode)" "Yellow"
	}

	Write-Log "" ""
	Write-Log "Installer process completed. Waiting for file system to settle..." "Cyan"
	Start-Sleep -Seconds 3

}
catch {
	Write-Log "✗ Failed to run Visual Studio Installer" "Red"
	Write-Log "  Error: $_" "Red"
	Write-Log "" ""
	exit 1
}

# Step 4: Verify installation
Write-Log "Step 4: Verifying CDB.exe installation..." "Yellow"
$cdb = Resolve-CdbPath
if (-not $cdb) {
	Write-Log "✗ CDB.exe was not found after installation attempt" "Red"
	Write-Log "" ""
	Write-Log "Troubleshooting:" "Yellow"
	Write-Log "  1. Ensure Visual Studio Installer completed successfully" "DarkGray"
	Write-Log "  2. Check that 'Windows Driver Kit' component is selected in .vsconfig" "DarkGray"
	Write-Log "  3. Try manually opening Visual Studio Installer and selecting the component" "DarkGray"
	Write-Log "" ""
	exit 1
}

Write-Log "✓ Found cdb.exe at: $cdb" "Green"
Write-Log "" ""

# Step 5: Add debuggers directory to system PATH
Write-Log "Step 5: Adding debuggers directory to system PATH..." "Yellow"
$debuggerDir = Get-DebuggersDirectory
if ($debuggerDir) {
	$pathAdded = Add-DebuggersToPath $debuggerDir
	if (-not $pathAdded) {
		Write-Log "⚠ Warning: Could not add to system PATH (may require admin privileges)" "Yellow"
	}
}

Write-Log "" ""
Write-Log "════════════════════════════════════════════════════════════" "Green"
Write-Log "✓ Installation Complete!" "Green"
Write-Log "════════════════════════════════════════════════════════════" "Green"
Write-Log "" ""
Write-Log "CDB.exe is ready to use. You can verify with:" "Cyan"
Write-Log "  cdb -version" "DarkGray"
Write-Log "" ""
exit 0
