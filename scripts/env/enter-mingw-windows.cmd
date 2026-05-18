@echo off
setlocal
set "SCRIPT_DIR=%~dp0"
powershell.exe -NoExit -ExecutionPolicy Bypass -Command "Set-Location '%CD%'; . '%SCRIPT_DIR%setup-mingw-windows.ps1' -ShowSummary"
