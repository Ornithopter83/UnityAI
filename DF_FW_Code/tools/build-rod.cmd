@echo off
setlocal

call "%~dp0arduino-env.cmd"
if errorlevel 1 exit /b %errorlevel%
call "%~dp0build-output-env.cmd" "%~1" "%~2"
if errorlevel 1 exit /b %errorlevel%

set "DF_SKETCH=%DF_REPO_ROOT%\firmware\DF_Rod"
set "DF_BUILD_PATH=%DF_REPO_ROOT%\artifacts\build\DF_Rod"
set "DF_OUTPUT_PATH=%DF_REPO_ROOT%\artifacts\firmware\DF_Rod"
set "DF_RELEASE_PATH=%DF_REPO_ROOT%\bin\%DF_BUILD_CONFIGURATION%\%DF_BUILD_PLATFORM%\%DF_ROD_VERSION%"
if not exist "%DF_BUILD_PATH%" mkdir "%DF_BUILD_PATH%"
if errorlevel 1 exit /b %errorlevel%
if not exist "%DF_OUTPUT_PATH%" mkdir "%DF_OUTPUT_PATH%"
if errorlevel 1 exit /b %errorlevel%

echo [DF Rod] Clean build started from current source.
"%DF_ARDUINO_CLI%" --config-file "%DF_ARDUINO_CONFIG%" compile --clean --fqbn "%DF_ARDUINO_FQBN%" --libraries "%DF_REPO_ROOT%\libraries" --build-path "%DF_BUILD_PATH%" --output-dir "%DF_OUTPUT_PATH%" "%DF_SKETCH%"
set "DF_BUILD_EXIT=%ERRORLEVEL%"
if not "%DF_BUILD_EXIT%"=="0" (
    echo [ERROR] DF Rod build failed with exit code %DF_BUILD_EXIT%.
    exit /b %DF_BUILD_EXIT%
)

copy /y "%DF_BOOT_APP%" "%DF_OUTPUT_PATH%\boot_app0.bin" >nul
if errorlevel 1 (
    echo [ERROR] Unable to stage Rod boot_app0.bin.
    exit /b 6
)

for %%F in ("%DF_OUTPUT_PATH%\DF_Rod.ino.bin" "%DF_OUTPUT_PATH%\DF_Rod.ino.bootloader.bin" "%DF_OUTPUT_PATH%\DF_Rod.ino.partitions.bin" "%DF_OUTPUT_PATH%\boot_app0.bin") do (
    if not exist "%%~fF" (
        echo [ERROR] Required Rod artifact not found: "%%~fF"
        exit /b 5
    )
)

if exist "%DF_RELEASE_PATH%" rmdir /s /q "%DF_RELEASE_PATH%"
if errorlevel 1 exit /b %errorlevel%
mkdir "%DF_RELEASE_PATH%"
if errorlevel 1 exit /b %errorlevel%

for %%F in ("DF_Rod.ino.bin" "DF_Rod.ino.bootloader.bin" "DF_Rod.ino.partitions.bin" "boot_app0.bin") do (
    copy /y "%DF_OUTPUT_PATH%\%%~F" "%DF_RELEASE_PATH%\%%~F" >nul
    if errorlevel 1 (
        echo [ERROR] Unable to publish Rod artifact: "%%~F"
        exit /b 10
    )
)

for /f %%C in ('dir /b /a-d "%DF_RELEASE_PATH%" ^| find /c /v ""') do set "DF_RELEASE_FILE_COUNT=%%C"
if not "%DF_RELEASE_FILE_COUNT%"=="4" (
    echo [ERROR] Rod release folder must contain exactly 4 files. Found %DF_RELEASE_FILE_COUNT%.
    exit /b 11
)

echo [DF Rod] Build completed.
for %%F in ("%DF_RELEASE_PATH%\DF_Rod.ino.bin" "%DF_RELEASE_PATH%\DF_Rod.ino.bootloader.bin" "%DF_RELEASE_PATH%\DF_Rod.ino.partitions.bin" "%DF_RELEASE_PATH%\boot_app0.bin") do echo [ARTIFACT] %%~nxF %%~zF bytes
echo [OUTPUT] "%DF_RELEASE_PATH%"
exit /b 0
