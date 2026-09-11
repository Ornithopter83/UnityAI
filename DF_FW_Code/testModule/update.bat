@echo OFF
REM === FW UPDATE
@echo ON
%~dp0esptool_V4.5.1.exe --chip esp32s3 --port COM%2 --baud 921600  --before default_reset --after hard_reset write_flash  -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 %~dp0\%1\DF_%1.ino.bootloader.bin 0x8000 %~dp0\%1\DF_%1.ino.partitions.bin 0xe000 %~dp0\%1\boot_app0.bin 0x10000 %~dp0\%1\DF_%1.ino.bin

@echo OFF
REM === WAIT 1SEC
timeout /t 1 >nul

start "" %~dp0realterm_V3.0.1.44.exe port=%2 baud=115200 flow=0 rts=1 dtr=0 caption="RealTerm1ST" newinstance=1

REM === WAIT 1SEC
timeout /t 2 >nul
taskkill /FI "WINDOWTITLE eq RealTerm1ST" /F >nul

REM == WAIT 10 SEC
REM ping -n 11 127.0.0.1 >nul
@echo ON
timeout /t 10 /nobreak

@echo OFF
exit


