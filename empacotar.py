"""Gera os dois arquivos de entrega pedidos no formulario.

    python empacotar.py

Cria em entrega/:
  - BloonsBattles_codigo.zip          (codigo fonte, sem executavel)
  - BloonsBattles_com_executavel.zip  (codigo fonte + dist/BloonsBattles.exe)

O executavel e gerado antes com PyInstaller.
"""

from __future__ import annotations

import subprocess
import sys
import zipfile
from pathlib import Path

RAIZ = Path(__file__).resolve().parent
SAIDA = RAIZ / "entrega"
EXE = RAIZ / "dist" / "BloonsBattles.exe"


def arquivos_do_codigo() -> list[Path]:
    saida = subprocess.run(["git", "ls-files"], cwd=RAIZ, capture_output=True, text=True, check=True)
    return [RAIZ / linha for linha in saida.stdout.splitlines() if linha]


def gerar_executavel() -> None:
    subprocess.run([sys.executable, "-m", "PyInstaller", "--noconfirm", "--onefile", "--windowed",
                    "--name", "BloonsBattles", "--log-level", "WARN", "jogar.py"], cwd=RAIZ, check=True)


def zipar(destino: Path, extras: list[Path]) -> None:
    with zipfile.ZipFile(destino, "w", zipfile.ZIP_DEFLATED) as z:
        for arq in arquivos_do_codigo() + extras:
            z.write(arq, Path("BloonsBattles") / arq.relative_to(RAIZ))
    print(f"gerado {destino.relative_to(RAIZ)} ({destino.stat().st_size // 1024} KB)")


def main() -> None:
    SAIDA.mkdir(exist_ok=True)
    gerar_executavel()
    zipar(SAIDA / "BloonsBattles_codigo.zip", [])
    zipar(SAIDA / "BloonsBattles_com_executavel.zip", [EXE])


if __name__ == "__main__":
    main()
