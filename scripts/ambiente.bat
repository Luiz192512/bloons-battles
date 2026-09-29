@echo off
rem Prepara o ambiente de compilacao: se o CMake nao estiver no PATH, procura o
rem Visual Studio (ou Build Tools) instalado e usa o compilador e o CMake dele.
rem Usado por compilar.bat e publicar_release.bat (chame com "call").

where cmake >nul 2>nul && where cl >nul 2>nul && exit /b 0
where cmake >nul 2>nul && where g++ >nul 2>nul && exit /b 0

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto sem_vs
set "VSDIR="
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%i"
if not defined VSDIR goto sem_vs

call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul || goto sem_vs
set "PATH=%VSDIR%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%VSDIR%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
where cmake >nul 2>nul || goto sem_cmake
exit /b 0

:sem_vs
echo Nao encontrei o Visual Studio com C++ nem um CMake no PATH.
echo Rode scripts\instalar_dependencias.bat e tente de novo.
exit /b 1

:sem_cmake
echo Encontrei o Visual Studio, mas nao o CMake. Rode scripts\instalar_dependencias.bat.
exit /b 1
