@echo off
REM Deploy GeneralsXZH with OpenAL32.dll
setlocal enabledelayedexpansion

set "SRC=%~dp0..\build\vc6\GeneralsMD\GeneralsXZH.exe"
set "DST=%USERPROFILE%\GeneralsX\GeneralsMD\GeneralsXZH.exe"
set "DLL_SRC=C:\vcpkg\installed\x86-windows\bin\OpenAL32.dll"
set "DLL_DST=%USERPROFILE%\GeneralsX\GeneralsMD\OpenAL32.dll"

echo Creating destination directory...
if not exist "%USERPROFILE%\GeneralsX\GeneralsMD" mkdir "%USERPROFILE%\GeneralsX\GeneralsMD"

echo Copying GeneralsXZH.exe...
copy /Y "!SRC!" "!DST!"
if errorlevel 1 (
    echo Error copying GeneralsXZH.exe
    exit /b 1
)

echo Copying OpenAL32.dll...
copy /Y "!DLL_SRC!" "!DLL_DST!"
if errorlevel 1 (
    echo Error copying OpenAL32.dll
    exit /b 1
)

echo Deploy completed successfully (GeneralsXZH.exe + OpenAL32.dll)
exit /b 0
