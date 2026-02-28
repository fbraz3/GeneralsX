# Configure CMake with MSVC BuildTools environment for win64-modern preset
# This script sets up the MSVC environment and then runs CMake configure
#
# Usage:
#   .\scripts\configure_cmake_modern.ps1
#   .\scripts\configure_cmake_modern.ps1 -Arch x64
#

param(
    [ValidateSet('x86', 'x64', 'arm', 'arm64')]
    [string]$Arch = 'x64',

    [ValidateSet('x86', 'x64', 'arm', 'arm64')]
    [string]$HostArch = 'x64'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║         CMake Configuration (win64-modern preset)        ║" -ForegroundColor Cyan
Write-Host "║                    DXVK + SDL3 + OpenAL                  ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Setup MSVC environment
Write-Host "⚙️  Setting up MSVC BuildTools environment..." -ForegroundColor Cyan
Write-Host "   Architecture: $Arch (host: $HostArch)" -ForegroundColor Gray
& "$PSScriptRoot\setup_msvc_buildtools_env.ps1" -Arch $Arch -HostArch $HostArch

if ($LASTEXITCODE -ne 0) {
    Write-Error "MSVC environment setup failed. Verify Visual Studio 2022 is installed."
    exit 1
}

Write-Host ""

# Verify cl.exe is available
$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $cl) {
    Write-Error "cl.exe not found after environment setup"
    exit 1
}

Write-Host "✅ MSVC compiler verified: $($cl.Source)" -ForegroundColor Green
Write-Host ""

# Force vcpkg to use MSVC 14.50 (which has complete CRT libs including MSVCRTD.lib)
# MSVC 14.44 is present but missing MSVCRTD.lib - vcpkg would pick 14.44 otherwise
$VCToolsRoot = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Tools\MSVC"
$preferredMSVC = "14.50.35717"
if (Test-Path "$VCToolsRoot\$preferredMSVC\lib\x64\msvcrtd.lib") {
    $env:VCToolsVersion = $preferredMSVC
    $env:VCToolsInstallDir = "$VCToolsRoot\$preferredMSVC\"
    Write-Host "✅ Forced MSVC version: $preferredMSVC (has complete CRT libs)" -ForegroundColor Green
} else {
    Write-Host "⚠️  MSVC $preferredMSVC not found, using default" -ForegroundColor Yellow
}
Write-Host ""

# Use VS BuildTools bundled CMake 4.1.1 + ninja-wrapper
# CMake's Ninja generator has a bug in try_compile: it generates build.ninja with
# "include CMakeFiles/rules.ninja" but never creates that file. When CMake calls
# "ninja -t recompact" on the try_compile directory, ninja fails with:
#   "build.ninja:35: loading 'CMakeFiles\rules.ninja': GetLastError() = 2"
# The ninja-wrapper pre-creates CMakeFiles/rules.ninja as a stub before forwarding.
# Source: C:\tools\ninja-wrapper\ninja_wrapper.c (compiled with cl.exe)
$vsCMakeDir = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake"
$vsCMake = "$vsCMakeDir\CMake\bin\cmake.exe"
$ninjaWrapperDir = "C:\tools\ninja-wrapper"
$ninjaWrapper = "$ninjaWrapperDir\ninja.exe"
if (Test-Path $vsCMake) {
    # Prepend ninja-wrapper FIRST so it shadows both bundled Ninja 1.12 and C:\tools\ninja
    $env:PATH = "$ninjaWrapperDir;$vsCMakeDir\CMake\bin;$env:PATH"
    Write-Host "✅ Using VS BuildTools CMake $(& $vsCMake --version | Select-String 'cmake version')" -ForegroundColor Green
} else {
    Write-Host "⚠️  VS BuildTools CMake not found, using default cmake" -ForegroundColor Yellow
}
if (Test-Path $ninjaWrapper) {
    Write-Host "✅ Using ninja-wrapper (pre-creates CMakeFiles/rules.ninja for CMake try_compile bug)" -ForegroundColor Green
} else {
    Write-Host "⚠️  ninja-wrapper not found at $ninjaWrapperDir" -ForegroundColor Yellow
    Write-Host "   Rebuild: cl /nologo /O2 $ninjaWrapperDir\ninja_wrapper.c /Fe:$ninjaWrapperDir\ninja.exe /link kernel32.lib" -ForegroundColor Gray
}
Write-Host ""

# Run CMake configure with win64-modern preset
Write-Host "🔧 Configuring CMake with preset: win64-modern" -ForegroundColor Cyan
Write-Host "   This will download and build dependencies (SDL3, DXVK, OpenAL, FFmpeg)..." -ForegroundColor Gray
Write-Host "   ⏱️  First run: 5-15 minutes | Subsequent runs: 30-60 seconds" -ForegroundColor Gray
Write-Host ""

$StartTime = Get-Date

# BuildTools discovery workaround for cmake
# Set CMAKE_C_COMPILER and CMAKE_CXX_COMPILER explicitly if cmake can't find VS
$CompilerPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Tools\MSVC\14.50.35717\bin\Hostx64\x64\cl.exe"

try {
    # Use forward slashes for compiler path (backslashes cause cmake to generate
    # invalid escape sequences in CMakeCCompiler.cmake, breaking inner cmake runs)
    $CompilerPathFwd = $CompilerPath.Replace('\', '/')
    $ninja111Exe = "C:/tools/ninja-wrapper/ninja.exe"
    Write-Host "Attempting CMake configure with preset..."
    cmake --preset win64-modern `
        -DCMAKE_C_COMPILER="$CompilerPathFwd" `
        -DCMAKE_CXX_COMPILER="$CompilerPathFwd" `
        -DCMAKE_MAKE_PROGRAM="$ninja111Exe"
    $ExitCode = $LASTEXITCODE
} catch {
    Write-Error "CMake configuration failed: $_"
    exit 1
}

$EndTime = Get-Date
$Duration = ($EndTime - $StartTime).TotalSeconds

if ($ExitCode -eq 0) {
    Write-Host ""
    Write-Host "✅ CMake configuration completed successfully in $([Math]::Round($Duration)) seconds" -ForegroundColor Green
    Write-Host "   Build directory: build/win64-modern/" -ForegroundColor Green
    Write-Host "   Generator: Ninja" -ForegroundColor Green
    Write-Host ""
    Write-Host "💡 Next steps:" -ForegroundColor Cyan
    Write-Host "   1. Build: .\scripts\build_zh_modern.ps1" -ForegroundColor White
    Write-Host "   2. Deploy: .\scripts\deploy_zh_modern.ps1 -Preset win64-modern" -ForegroundColor White
    Write-Host "   3. Run: .\scripts\run_zh_modern.ps1" -ForegroundColor White
} else {
    Write-Host ""
    Write-Error "CMake configuration failed with exit code $ExitCode"
    Write-Host ""
    Write-Host "💡 Troubleshooting:" -ForegroundColor Yellow
    Write-Host "   - Verify Vulkan SDK is installed: https://vulkan.lunarg.com/sdk/home" -ForegroundColor Gray
    Write-Host "   - Check Vulkan SDK path: echo `$env:VULKAN_SDK" -ForegroundColor Gray
    Write-Host "   - Clean and retry: rm -r build/win64-modern; .\scripts\configure_cmake_modern.ps1" -ForegroundColor Gray
    exit 1
}
