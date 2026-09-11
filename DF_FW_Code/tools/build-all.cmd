@echo off
setlocal

echo [DF Firmware] Main and Rod clean build started.

call "%~dp0build-main.cmd" "%~1" "%~2"
set "DF_MAIN_EXIT=%ERRORLEVEL%"
if not "%DF_MAIN_EXIT%"=="0" (
    echo [ERROR] Combined build stopped after Main failure. Exit code %DF_MAIN_EXIT%.
    exit /b %DF_MAIN_EXIT%
)

call "%~dp0build-rod.cmd" "%~1" "%~2"
set "DF_ROD_EXIT=%ERRORLEVEL%"
if not "%DF_ROD_EXIT%"=="0" (
    echo [ERROR] Combined build stopped after Rod failure. Exit code %DF_ROD_EXIT%.
    exit /b %DF_ROD_EXIT%
)

call "%~dp0arduino-env.cmd"
if errorlevel 1 exit /b %errorlevel%
call "%~dp0build-output-env.cmd" "%~1" "%~2"
if errorlevel 1 exit /b %errorlevel%

set "DF_MAIN_OUTPUT=%DF_REPO_ROOT%\bin\%DF_BUILD_CONFIGURATION%\%DF_BUILD_PLATFORM%\%DF_MAIN_VERSION%"
set "DF_ROD_OUTPUT=%DF_REPO_ROOT%\bin\%DF_BUILD_CONFIGURATION%\%DF_BUILD_PLATFORM%\%DF_ROD_VERSION%"
set "DF_MAIN_BIN=%DF_MAIN_OUTPUT%\DF_Main.ino.bin"
set "DF_ROD_BIN=%DF_ROD_OUTPUT%\DF_Rod.ino.bin"

if not exist "%DF_MAIN_BIN%" (
    echo [ERROR] Main application binary is missing after combined build.
    exit /b 6
)
if not exist "%DF_ROD_BIN%" (
    echo [ERROR] Rod application binary is missing after combined build.
    exit /b 7
)

echo [DF Firmware] Main and Rod build completed.
for %%F in ("%DF_MAIN_BIN%" "%DF_ROD_BIN%") do echo [APPLICATION] %%~nxF %%~zF bytes
echo [MAIN OUTPUT] "%DF_MAIN_OUTPUT%"
echo [ROD OUTPUT] "%DF_ROD_OUTPUT%"
exit /b 0
