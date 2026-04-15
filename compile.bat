@echo off
cd /d "%~dp0"

set "PATH=C:\msys64\ucrt64\bin;%PATH%"

set "COMPILER=C:\msys64\ucrt64\bin\g++.exe"
set "INCLUDE_FREEGLUT=C:\msys64\ucrt64\include"
set "LIB_FREEGLUT=C:\msys64\ucrt64\lib"
set "DLL=C:\msys64\ucrt64\bin\libfreeglut.dll"
set "OUTPUT=build\soccer_field.exe"

if not exist "build" mkdir "build"

if not exist "%COMPILER%" (
    echo Erro: compilador nao encontrado em "%COMPILER%"
    pause
    exit /b 1
)

if not exist "%INCLUDE_FREEGLUT%\GL\freeglut.h" (
    echo Erro: cabecalho nao encontrado em "%INCLUDE_FREEGLUT%\GL\freeglut.h"
    pause
    exit /b 1
)

echo Compilando com MinGW...
"%COMPILER%" -I"%INCLUDE_FREEGLUT%" "main.cpp" -L"%LIB_FREEGLUT%" -lfreeglut -lopengl32 -luser32 -o "%OUTPUT%"

if errorlevel 1 (
    echo Erro na compilacao!
    pause
    exit /b 1
)

copy /Y "%DLL%" "build\" >nul
echo.
echo Sucesso! Executavel: %OUTPUT%
echo Rode: .\%OUTPUT%
pause
