@echo off
setlocal

cd /d "%~dp0.."
where g++.exe >nul 2>nul
if errorlevel 1 (
    echo [ERROR] g++.exe was not found in PATH.
    exit /b 1
)

if not exist "artifacts\host-tests" mkdir "artifacts\host-tests"
g++.exe -std=c++98 -Wall -Wextra -pedantic "tests\protocol\DFProtocol_HostTest.cpp" "libraries\DFProtocol\src\DFProtocol.cpp" -o "artifacts\host-tests\DFProtocol_HostTest.exe"
if errorlevel 1 exit /b 1

"artifacts\host-tests\DFProtocol_HostTest.exe"
if errorlevel 1 exit /b 1

echo [DF Protocol] Host test completed.
exit /b 0
