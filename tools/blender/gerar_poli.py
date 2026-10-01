"""Gera uma torre poligonal sem abrir a interface do Blender (uma torre por execucao).

Uso (na raiz do repositorio):
    blender --background --python tools/blender/gerar_poli.py -- dardo

Grava assets/modelos/<chave>.glb e .json, a fonte em assets/modelos/fonte/<chave>.blend e as
folhas de revisao em docs/design/capturas/modelos/<chave>_*.png.
"""
import json
import os
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(os.path.dirname(AQUI))
sys.path.insert(0, AQUI)

import poli  # noqa: E402
import torres_poli  # noqa: E402

chave = sys.argv[sys.argv.index("--") + 1]
c = poli.cena("bb_" + chave)
for _o in list(poli.bpy.data.objects):  # cubo, camera e luz padrao do Blender
    poli.bpy.data.objects.remove(_o, do_unlink=True)
partes, pivos = torres_poli.TORRES[chave](c)
info = {g: {"tris": poli.triangulos([o]), "quads": poli.quads_pct([o])} for g, o in partes.items()}
info["total_tris"] = poli.triangulos(list(partes.values()))
info["export"] = poli.exportar(c, chave, partes, pivos, RAIZ)
pasta = os.path.join(RAIZ, "docs", "design", "capturas", "modelos")
alvo, tam = torres_poli.ENQUADRE[chave]
for v in ("tres_quartos", "frente", "jogo"):
    poli.render(c, os.path.join(pasta, f"{chave}_{v}.png"), v, alvo=alvo, largura=tam, altura=tam, px=int(680 / tam))
poli.render(c, os.path.join(pasta, f"{chave}_jogo_48.png"), "jogo", alvo=alvo, largura=tam, altura=tam, px=48)
print("[gerar_poli]", chave, json.dumps(info))
