@echo off
setlocal

call "%~dp0arduino-env.cmd"
if errorlevel 1 exit /b %errorlevel%
call "%~dp0build-output-env.cmd" "%~1" "%~2"
if errorlevel 1 exit /b %errorlevel%

set "DF_SKETCH=%DF_REPO_ROOT%\firmware\DF_Main"
set "DF_BUILD_PATH=%DF_REPO_ROOT%\artifacts\build\DF_Main"
set "DF_OUTPUT_PATH=%DF_REPO_ROOT%\artifacts\firmware\DF_Main"
set "DF_RELEASE_PATH=%DF_REPO_ROOT%\bin\%DF_BUILD_CONFIGURATION%\%DF_BUILD_PLATFORM%\%DF_MAIN_VERSION%"

if not exist "%DF_SKETCH%\DF_Main.ino" (
    echo [ERROR] Main sketch not found: "%DF_SKETCH%\DF_Main.ino"
    exit /b 4
)

if not exist "%DF_BUILD_PATH%" mkdir "%DF_BUILD_PATH%"
if errorlevel 1 exit /b %errorlevel%
if not exist "%DF_OUTPUT_PATH%" mkdir "%DF_OUTPUT_PATH%"
if errorlevel 1 exit /b %errorlevel%

echo [DF Main] Clean build started.
"%DF_ARDUINO_CLI%" --config-file "%DF_ARDUINO_CONFIG%" compile --clean --fqbn "%DF_ARDUINO_FQBN%" --libraries "%DF_REPO_ROOT%\libraries" --build-path "%DF_BUILD_PATH%" --output-dir "%DF_OUTPUT_PATH%" "%DF_SKETCH%"
set "DF_BUILD_EXIT=%ERRORLEVEL%"
if not "%DF_BUILD_EXIT%"=="0" (
    echo [ERROR] DF Main build failed with exit code %DF_BUILD_EXIT%.
    exit /b %DF_BUILD_EXIT%
)

copy /y "%DF_BOOT_APP%" "%DF_OUTPUT_PATH%\boot_app0.bin" >nul
if errorlevel 1 (
    echo [ERROR] Unable to stage Main boot_app0.bin.
    exit /b 6
)

for %%F in ("%DF_OUTPUT_PATH%\DF_Main.ino.bin" "%DF_OUTPUT_PATH%\DF_Main.ino.bootloader.bin" "%DF_OUTPUT_PATH%\DF_Main.ino.partitions.bin" "%DF_OUTPUT_PATH%\boot_app0.bin") do (
    if not exist "%%~fF" (
        echo [ERROR] Required Main artifact not found: "%%~fF"
        exit /b 5
    )
)

if exist "%DF_RELEASE_PATH%" rmdir /s /q "%DF_RELEASE_PATH%"
if errorlevel 1 exit /b %errorlevel%
mkdir "%DF_RELEASE_PATH%"
if errorlevel 1 exit /b %errorlevel%

for %%F in ("DF_Main.ino.bin" "DF_Main.ino.bootloader.bin" "DF_Main.ino.partitions.bin" "boot_app0.bin") do (
    copy /y "%DF_OUTPUT_PATH%\%%~F" "%DF_RELEASE_PATH%\%%~F" >nul
    if errorlevel 1 (
        echo [ERROR] Unable to publish Main artifact: "%%~F"
        exit /b 10
    )
)

for /f %%C in ('dir /b /a-d "%DF_RELEASE_PATH%" ^| find /c /v ""') do set "DF_RELEASE_FILE_COUNT=%%C"
if not "%DF_RELEASE_FILE_COUNT%"=="4" (
    echo [ERROR] Main release folder must contain exactly 4 files. Found %DF_RELEASE_FILE_COUNT%.
    exit /b 11
)

echo [DF Main] Build completed.
for %%F in ("%DF_RELEASE_PATH%\DF_Main.ino.bin" "%DF_RELEASE_PATH%\DF_Main.ino.bootloader.bin" "%DF_RELEASE_PATH%\DF_Main.ino.partitions.bin" "%DF_RELEASE_PATH%\boot_app0.bin") do echo [ARTIFACT] %%~nxF %%~zF bytes
echo [OUTPUT] "%DF_RELEASE_PATH%"
exit /b 0
