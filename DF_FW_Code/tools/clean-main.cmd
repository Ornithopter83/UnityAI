@echo off
setlocal
call "%~dp0arduino-env.cmd"
if errorlevel 1 exit /b %errorlevel%
call "%~dp0build-output-env.cmd" "%~1" "%~2"
if errorlevel 1 exit /b %errorlevel%

if exist "%DF_REPO_ROOT%\artifacts\build\DF_Main" rmdir /s /q "%DF_REPO_ROOT%\artifacts\build\DF_Main"
if errorlevel 1 exit /b %errorlevel%
if exist "%DF_REPO_ROOT%\artifacts\firmware\DF_Main" rmdir /s /q "%DF_REPO_ROOT%\artifacts\firmware\DF_Main"
if errorlevel 1 exit /b %errorlevel%
if exist "%DF_REPO_ROOT%\bin\%DF_BUILD_CONFIGURATION%\%DF_BUILD_PLATFORM%\%DF_MAIN_VERSION%" rmdir /s /q "%DF_REPO_ROOT%\bin\%DF_BUILD_CONFIGURATION%\%DF_BUILD_PLATFORM%\%DF_MAIN_VERSION%"
if errorlevel 1 exit /b %errorlevel%

echo [DF Main] Intermediate and published artifacts cleaned.
exit /b 0
