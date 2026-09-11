@echo off
setlocal

call "%~dp0clean-main.cmd" "%~1" "%~2"
if errorlevel 1 exit /b %errorlevel%
call "%~dp0clean-rod.cmd" "%~1" "%~2"
if errorlevel 1 exit /b %errorlevel%

echo [DF Firmware] Main and Rod intermediate and published artifacts cleaned.
exit /b 0
