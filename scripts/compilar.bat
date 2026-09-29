@echo off
rem Compila o jogo no Windows. Precisa de CMake e Git, mais Visual Studio 2022 (C++) ou MinGW-w64.
rem A raylib e baixada automaticamente na primeira compilacao.
cd /d "%~dp0.."
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release || goto erro
cmake --build build --config Release --parallel || goto erro
ctest --test-dir build -C Release --output-on-failure || goto erro
echo.
echo Pronto. O executavel esta em build\Release\BloonsBattles.exe (Visual Studio) ou build\BloonsBattles.exe (MinGW).
pause
exit /b 0
:erro
echo Falhou. Veja as mensagens acima.
pause
exit /b 1
