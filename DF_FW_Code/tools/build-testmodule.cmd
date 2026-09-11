@echo off
setlocal

set "DF_TM_CONFIGURATION=%~1"
if "%DF_TM_CONFIGURATION%"=="" set "DF_TM_CONFIGURATION=Release"

if /I not "%DF_TM_CONFIGURATION%"=="Release" if /I not "%DF_TM_CONFIGURATION%"=="Debug" (
  echo [DFTestModule] Unsupported configuration: %DF_TM_CONFIGURATION%
  exit /b 2
)

set "DF_TM_PROJECT=%~dp0..\testModule\DFTestModule.csproj"
set "DF_TM_OUTPUT=%~dp0..\bin\testmodule\%DF_TM_CONFIGURATION%"

dotnet publish "%DF_TM_PROJECT%" -c "%DF_TM_CONFIGURATION%" -p:Platform=x64 -r win-x64 --self-contained true -o "%DF_TM_OUTPUT%"
if errorlevel 1 exit /b %errorlevel%

if not exist "%DF_TM_OUTPUT%\DFTestModule_V041.exe" (
  echo [DFTestModule] Publish output not found: %DF_TM_OUTPUT%\DFTestModule_V041.exe
  exit /b 3
)

echo [DFTestModule] Single EXE: %DF_TM_OUTPUT%\DFTestModule_V041.exe
exit /b 0
