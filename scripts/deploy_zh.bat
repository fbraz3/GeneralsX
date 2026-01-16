@echo off
REM Deploy GeneralsXZH with SDL2.dll and OpenAL32.dll
setlocal enabledelayedexpansion

set "SRC=%~dp0..\build\vc6\GeneralsMD\GeneralsXZH.exe"
set "DST=%USERPROFILE%\GeneralsX\GeneralsMD\GeneralsXZH.exe"
set "SDL2_SRC=C:\vcpkg\installed\x86-windows\bin\SDL2.dll"
set "SDL2_DST=%USERPROFILE%\GeneralsX\GeneralsMD\SDL2.dll"
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

echo Copying SDL2.dll...
copy /Y "!SDL2_SRC!" "!SDL2_DST!"
if errorlevel 1 (
    echo Error copying SDL2.dll
    exit /b 1
)

echo Copying OpenAL32.dll...
copy /Y "!DLL_SRC!" "!DLL_DST!"
if errorlevel 1 (
    echo Error copying OpenAL32.dll
    exit /b 1
)

echo Deploy completed successfully (GeneralsXZH.exe + SDL2.dll + OpenAL32.dll)
exit /b 0
