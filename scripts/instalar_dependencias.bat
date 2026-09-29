@echo off
rem Instala o que e preciso para COMPILAR o jogo no Windows, usando o winget
rem (ja vem no Windows 10 e 11). Para so JOGAR nao precisa de nada disso: basta o .exe.
rem
rem Instala: Git, CMake e Visual Studio 2022 Build Tools com C++ (compilador MSVC).
rem O Build Tools tem cerca de 3 GB e pode pedir permissao de administrador.

where winget >nul 2>nul || (
  echo O winget nao foi encontrado. Instale o "Instalador de Aplicativo" pela Microsoft Store.
  pause
  exit /b 1
)

echo [1/3] Git
winget install -e --id Git.Git --accept-package-agreements --accept-source-agreements

echo [2/3] CMake
winget install -e --id Kitware.CMake --accept-package-agreements --accept-source-agreements

echo [3/3] Visual Studio 2022 Build Tools (C++)
winget install -e --id Microsoft.VisualStudio.2022.BuildTools --accept-package-agreements --accept-source-agreements ^
  --override "--quiet --wait --norestart --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"

echo.
echo Pronto. Feche e abra o terminal (para o PATH atualizar) e rode scripts\compilar.bat
pause
