@echo off
set "DF_REPO_ROOT=%~dp0.."
for %%I in ("%DF_REPO_ROOT%") do set "DF_REPO_ROOT=%%~fI"

set "DF_ARDUINO_CLI=%DF_REPO_ROOT%\toolchain\arduino-cli\arduino-cli.exe"
set "DF_ARDUINO_CONFIG=%DF_REPO_ROOT%\toolchain\arduino-cli.yaml"
set "DF_ARDUINO_FQBN=esp32:esp32:esp32s3:UploadSpeed=921600,USBMode=hwcdc,CDCOnBoot=cdc,MSCOnBoot=default,DFUOnBoot=default,UploadMode=default,CPUFreq=240,FlashMode=dio,FlashSize=4M,PartitionScheme=default,DebugLevel=none,PSRAM=disabled,LoopCore=1,EventsCore=1,EraseFlash=none,JTAGAdapter=default"
set "DF_BOOT_APP=%DF_REPO_ROOT%\toolchain\arduino-data\packages\esp32\hardware\esp32\2.0.17\tools\partitions\boot_app0.bin"

if not exist "%DF_ARDUINO_CLI%" (
    echo [ERROR] Arduino CLI not found: "%DF_ARDUINO_CLI%"
    exit /b 2
)

if not exist "%DF_ARDUINO_CONFIG%" (
    echo [ERROR] Arduino CLI config not found: "%DF_ARDUINO_CONFIG%"
    exit /b 3
)

if not exist "X:\toolchain\arduino-data\packages\esp32" (
    if exist "X:\" (
        echo [ERROR] Drive X: is already in use by another location.
        exit /b 4
    )
    subst X: "%DF_REPO_ROOT%"
    if errorlevel 1 (
        echo [ERROR] Unable to map the repository to drive X:.
        exit /b 5
    )
)

if not exist "%DF_BOOT_APP%" (
    echo [ERROR] ESP32 Core boot_app0.bin not found: "%DF_BOOT_APP%"
    exit /b 6
)

exit /b 0
