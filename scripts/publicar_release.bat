@echo off
rem Compila o .exe portatil (sem DLLs), gera os zips de entrega e publica uma Release
rem no GitHub com o GitHub CLI (gh). Nao depende do GitHub Actions.
rem
rem Uso:  scripts\publicar_release.bat v1.0
rem Precisa: tudo de compilar.bat, mais o gh autenticado (gh auth login).
setlocal
cd /d "%~dp0.."
if "%~1"=="" (
  echo Uso: scripts\publicar_release.bat ^<versao^>   exemplo: scripts\publicar_release.bat v1.0
  exit /b 1
)
set "VERSAO=%~1"

where gh >nul 2>nul || (echo Instale o GitHub CLI: winget install -e --id GitHub.cli & exit /b 1)
gh auth status >nul 2>nul || (echo Faca login antes: gh auth login & exit /b 1)
git diff --quiet HEAD || (echo Ha alteracoes nao commitadas. Commit antes de publicar. & exit /b 1)

call "%~dp0ambiente.bat" || exit /b 1

echo === Compilando (pasta build-release)
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release || goto erro
cmake --build build-release --config Release --parallel || goto erro
ctest --test-dir build-release -C Release --output-on-failure || goto erro

set "BIN=build-release"
if exist build-release\Release\BloonsBattles.exe set "BIN=build-release\Release"

echo === Montando os zips de entrega
if exist entrega rmdir /s /q entrega
mkdir entrega\tmp
git archive --format=zip --prefix=BloonsBattles/ -o entrega\BloonsBattles_codigo.zip HEAD || goto erro
copy /y "%BIN%\BloonsBattles.exe" entrega\ >nul || goto erro
copy /y "%BIN%\bloons_servidor.exe" entrega\ >nul || goto erro
copy /y "%BIN%\bloons_monitor.exe" entrega\ >nul || goto erro
powershell -NoProfile -Command ^
  "Expand-Archive entrega\BloonsBattles_codigo.zip entrega\tmp; Copy-Item entrega\BloonsBattles.exe,entrega\bloons_servidor.exe,entrega\bloons_monitor.exe entrega\tmp\BloonsBattles; Compress-Archive entrega\tmp\BloonsBattles entrega\BloonsBattles_com_executavel.zip -Force" || goto erro
rmdir /s /q entrega\tmp

echo === Publicando a release %VERSAO%
gh release create "%VERSAO%" --target main --title "Bloons TD Battles %VERSAO%" ^
  --notes "Baixe BloonsBattles.exe e abra. Nao precisa instalar nada. Para jogar em dois PCs, veja o README (firewall)." ^
  entrega\BloonsBattles.exe entrega\bloons_servidor.exe entrega\BloonsBattles_codigo.zip entrega\BloonsBattles_com_executavel.zip || goto erro

echo.
echo Release publicada. Link:
gh release view "%VERSAO%" --json url -q .url
exit /b 0
:erro
echo Falhou. Veja as mensagens acima.
exit /b 1
