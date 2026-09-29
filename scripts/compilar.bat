@echo off
rem Compila o jogo no Windows. Precisa de Visual Studio 2022 (C++) ou MinGW-w64, mais CMake e Git.
rem Se faltar algo, rode antes scripts\instalar_dependencias.bat.
rem A raylib e baixada automaticamente na primeira compilacao.
setlocal
cd /d "%~dp0.."
call "%~dp0ambiente.bat" || goto erro
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release || goto erro
cmake --build build --config Release --parallel || goto erro
ctest --test-dir build -C Release --output-on-failure || goto erro
echo.
if exist build\Release\BloonsBattles.exe (
  echo Pronto. O executavel esta em build\Release\BloonsBattles.exe
) else (
  echo Pronto. O executavel esta em build\BloonsBattles.exe
)
pause
exit /b 0
:erro
echo Falhou. Veja as mensagens acima.
pause
exit /b 1
