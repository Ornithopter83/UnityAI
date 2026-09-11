@echo off
setlocal
if "%~3"=="" (
    echo Usage: upload-rod-ota.cmd COM_PORT FIRMWARE_BIN VERSION
    exit /b 2
)
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0upload-rod-ota.ps1" -Port "%~1" -Firmware "%~2" -Version "%~3"
exit /b %errorlevel%
