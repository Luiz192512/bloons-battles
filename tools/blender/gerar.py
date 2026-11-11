"""Gera os modelos 3D das torres sem abrir a interface do Blender.

Uso (na raiz do repositorio):
    blender --background --python tools/blender/gerar.py -- dardo bomba bucaneiro

Para cada torre grava:
    assets/modelos/<chave>.glb          modelo, uma malha por grupo animavel
    assets/modelos/<chave>.json         grupo e pivo de cada malha, na ordem do .glb
    docs/design/capturas/modelos/<chave>.png   previa na vista 3/4
    dist/blender/<chave>.blend          cena para retocar a mao (fora do git)
"""
import importlib
import json
import os
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(os.path.dirname(AQUI))
sys.path.insert(0, AQUI)

import bpy  # noqa: E402

import comum  # noqa: E402

# ordem das malhas no arquivo: do que fica parado ao que mais se mexe
ORDEM = ["base", "corpo", "cauda", "cabeca", "torreta", "braco"]


def gerar(chave):
    comum.limpar()
    mod = importlib.import_module(chave)
    pivos = mod.construir()
    grupos = comum.juntar_grupos(ORDEM)

    pasta = os.path.join(RAIZ, "assets", "modelos")
    os.makedirs(pasta, exist_ok=True)
    glb = os.path.join(pasta, chave + ".glb")
    comum.exportar_glb(glb)

    # o .json segue a ordem real das malhas no .glb, que e a ordem em que o jogo as carrega
    info = []
    for nome in comum.malhas_do_glb(glb):
        g = nome.split("_", 1)[1] if "_" in nome else nome
        info.append({"grupo": g, "pivo": comum.para_gltf(pivos.get(g, (0, 0, 0)))})
    with open(os.path.join(pasta, chave + ".json"), "w", encoding="utf-8") as f:
        json.dump({"malhas": info}, f, indent=1)

    os.makedirs(os.path.join(RAIZ, "dist", "blender"), exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(RAIZ, "dist", "blender", chave + ".blend"))

    pasta_previa = os.path.join(RAIZ, "docs", "design", "capturas", "modelos")
    os.makedirs(pasta_previa, exist_ok=True)
    motor = comum.previa(os.path.join(pasta_previa, chave + ".png"), alvo=getattr(mod, "ALVO_PREVIA", (0, 0, 0.5)),
                         distancia=getattr(mod, "DIST_PREVIA", 3.2))
    print(f"[gerar] {chave}: grupos {grupos}, malhas {[i['grupo'] for i in info]}, previa {motor}")


if __name__ == "__main__":
    chaves = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    for ch in chaves or ["dardo", "bomba", "bucaneiro"]:
        gerar(ch)
