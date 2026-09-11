@echo off

rem Read the firmware source on every build/clean. Do not duplicate version values here.
set "DF_MAIN_VERSION="
set "DF_ROD_VERSION="
for /f "usebackq delims=" %%V in (`powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "%~dp0firmware-versions.ps1"`) do set "%%V"
if not defined DF_MAIN_VERSION goto version_error
if not defined DF_ROD_VERSION goto version_error
exit /b 0

:version_error
set "DF_MAIN_VERSION="
set "DF_ROD_VERSION="
echo [ERROR] Unable to resolve firmware versions from source. Build/clean stopped.
exit /b 12