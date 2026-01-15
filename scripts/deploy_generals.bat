@echo off
REM Deploy GeneralsX with OpenAL32.dll
setlocal enabledelayedexpansion

set "SRC=%~dp0..\build\vc6\Generals\GeneralsX.exe"
set "DST=%USERPROFILE%\GeneralsX\Generals\GeneralsX.exe"
set "DLL_SRC=C:\vcpkg\installed\x86-windows\bin\OpenAL32.dll"
set "DLL_DST=%USERPROFILE%\GeneralsX\Generals\OpenAL32.dll"

echo Creating destination directory...
if not exist "%USERPROFILE%\GeneralsX\Generals" mkdir "%USERPROFILE%\GeneralsX\Generals"

echo Copying GeneralsX.exe...
copy /Y "!SRC!" "!DST!"
if errorlevel 1 (
    echo Error copying GeneralsX.exe
    exit /b 1
)

echo Copying OpenAL32.dll...
copy /Y "!DLL_SRC!" "!DLL_DST!"
if errorlevel 1 (
    echo Error copying OpenAL32.dll
    exit /b 1
)

echo Deploy completed successfully (GeneralsX.exe + OpenAL32.dll)
exit /b 0
