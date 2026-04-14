@echo off
setlocal enabledelayedexpansion

set COMPILER=g++
set INCLUDE_FREEGLUT=C:\vcpkg\installed\x64-mingw-dynamic\include
set LIB_FREEGLUT=C:\vcpkg\installed\x64-mingw-dynamic\lib
set DLL=C:\vcpkg\installed\x64-mingw-dynamic\bin\libfreeglut.dll
set OUTPUT=build\soccer_field.exe

if not exist build mkdir build

echo Compilando com MinGW...
"%COMPILER%" -I"%INCLUDE_FREEGLUT%" main.cpp ^
  -L"%LIB_FREEGLUT%" ^
  -lfreeglut -lopengl32 -luser32 ^
  -o "%OUTPUT%"

if %ERRORLEVEL% EQU 0 (
    copy "%DLL%" build\
    echo.
    echo Sucesso! Executavel: %OUTPUT%
    echo Rode: .\%OUTPUT%
) else (
    echo Erro na compilacao!
)
pause
