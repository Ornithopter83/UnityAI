@echo off
rem DEPRECATED 2026-08-27. Original: tools/stage-rod.cmd. Replacement: direct firmware/DF_Rod build.
setlocal

call "%~dp0arduino-env.cmd"
if errorlevel 1 exit /b %errorlevel%

set "DF_SOURCE=%DF_REPO_ROOT%\Vr1.0.1.0"
set "DF_SKETCH=%DF_REPO_ROOT%\artifacts\sketch\DF_Rod"

if not exist "%DF_SOURCE%\DF_Rod.ino" (
    echo [ERROR] Rod source sketch not found: "%DF_SOURCE%\DF_Rod.ino"
    exit /b 4
)

if exist "%DF_SKETCH%" rmdir /s /q "%DF_SKETCH%"
if errorlevel 1 exit /b %errorlevel%
mkdir "%DF_SKETCH%"
if errorlevel 1 exit /b %errorlevel%

copy /y "%DF_SOURCE%\DF_Rod.ino" "%DF_SKETCH%\DF_Rod.ino" >nul
if errorlevel 1 exit /b %errorlevel%
copy /y "%DF_SOURCE%\*.cpp" "%DF_SKETCH%\" >nul
if errorlevel 1 exit /b %errorlevel%
copy /y "%DF_SOURCE%\*.h" "%DF_SKETCH%\" >nul
if errorlevel 1 exit /b %errorlevel%

echo [DF Rod] Current source staged at "%DF_SKETCH%".
exit /b 0
