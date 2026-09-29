#!/usr/bin/env bash
# Gera os dois arquivos de entrega pedidos no formulario (a partir do Linux ou WSL):
#   entrega/BloonsBattles_codigo.zip          codigo fonte, sem executavel
#   entrega/BloonsBattles_com_executavel.zip  codigo fonte + executaveis do Windows
# Precisa de: cmake, git, zip e mingw-w64 (sudo apt install cmake git zip mingw-w64).
set -euo pipefail
cd "$(dirname "$0")/.."
RAIZ=$(pwd)

cmake -S . -B build-win -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-win --parallel
x86_64-w64-mingw32-strip build-win/BloonsBattles.exe build-win/bloons_servidor.exe

rm -rf entrega && mkdir -p entrega/tmp/BloonsBattles
git ls-files -z | xargs -0 -I{} cp --parents {} entrega/tmp/BloonsBattles/
(cd entrega/tmp && zip -qr ../BloonsBattles_codigo.zip BloonsBattles)
cp build-win/BloonsBattles.exe build-win/bloons_servidor.exe entrega/tmp/BloonsBattles/
(cd entrega/tmp && zip -qr ../BloonsBattles_com_executavel.zip BloonsBattles)
rm -rf entrega/tmp
ls -lh entrega
