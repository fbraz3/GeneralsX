@echo off
REM Vulkan SDK setup script for Windows VC6 32-bit GeneralsX
REM Downloads and extracts Vulkan SDK if not already installed

setlocal enabledelayedexpansion

echo.
echo [Vulkan SDK Setup] GeneralsX Phase 09 Graphics Backend
echo.

REM Check if Vulkan SDK is already installed
if defined VULKAN_SDK (
    echo VULKAN_SDK is set to: %VULKAN_SDK%
    if exist "%VULKAN_SDK%\Include\vulkan\vulkan.h" (
        echo Vulkan SDK headers found. Proceeding...
        exit /b 0
    )
)

REM Try to find Vulkan SDK in common paths
if exist "C:\VulkanSDK" (
    echo Found Vulkan SDK in C:\VulkanSDK
    for /d %%D in (C:\VulkanSDK\*) do (
        if exist "%%D\Include\vulkan\vulkan.h" (
            set "VULKAN_SDK=%%D"
            setx VULKAN_SDK "!VULKAN_SDK!"
            echo Set VULKAN_SDK=!VULKAN_SDK!
            exit /b 0
        )
    )
)

REM Download Vulkan SDK
echo.
echo Vulkan SDK not found. Downloading Vulkan SDK 1.4.335...
echo.
echo To install Vulkan SDK manually:
echo   1. Visit: https://vulkan.lunarg.com/sdk/home
echo   2. Download VulkanSDK-Windows-1.4.335.0.exe
echo   3. Run the installer
echo   4. Choose installation path (default: C:\VulkanSDK)
echo   5. Set VULKAN_SDK environment variable to installation path
echo   6. Restart your terminal
echo.
echo After installation, run CMake again:
echo   cmake --preset vc6
echo.

exit /b 1
