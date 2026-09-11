@echo off

call "%~dp0firmware-versions.cmd"
if errorlevel 1 exit /b %errorlevel%

set "DF_BUILD_CONFIGURATION=%~1"
if not defined DF_BUILD_CONFIGURATION set "DF_BUILD_CONFIGURATION=release"
if /I "%DF_BUILD_CONFIGURATION%"=="Debug" set "DF_BUILD_CONFIGURATION=debug"
if /I "%DF_BUILD_CONFIGURATION%"=="Release" set "DF_BUILD_CONFIGURATION=release"
if /I not "%DF_BUILD_CONFIGURATION%"=="debug" if /I not "%DF_BUILD_CONFIGURATION%"=="release" (
    echo [ERROR] Unsupported configuration: "%DF_BUILD_CONFIGURATION%". Use Debug or Release.
    exit /b 8
)

set "DF_BUILD_PLATFORM=%~2"
if not defined DF_BUILD_PLATFORM set "DF_BUILD_PLATFORM=x64"
if /I not "%DF_BUILD_PLATFORM%"=="x64" (
    echo [ERROR] Unsupported platform: "%DF_BUILD_PLATFORM%". Use x64.
    exit /b 9
)
set "DF_BUILD_PLATFORM=x64"

exit /b 0
