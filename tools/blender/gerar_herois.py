"""Gera os modelos 3D dos herois (um .glb por heroi), sem abrir a interface.

Uso (na raiz do repositorio):
    blender --background --python tools/blender/gerar_herois.py -- [chave ...]

Sem chave, gera os 18 e a folha de contato. Saidas:
    assets/modelos/<chave>/0-0-0.glb e .json
    docs/design/capturas/modelos/herois.png        folha 6x3 na vista do jogo, na ordem de herois.HEROIS
    docs/design/capturas/modelos/herois_3q.png     a mesma folha na vista tres quartos
    dist/modelos/herois/<chave>_<vista>.png        quadros soltos (fora do git)
"""
import json
import os
import sys
import time

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(os.path.dirname(AQUI))
sys.path.insert(0, AQUI)

import bpy  # noqa: E402
import numpy as np  # noqa: E402

import herois  # noqa: E402
import partes  # noqa: E402
import poli  # noqa: E402

CELULA = 300   # lado de cada quadro da folha, em pixels


def _ler(caminho):
    img = bpy.data.images.load(caminho)
    w, h = img.size
    a = np.empty(w * h * 4, dtype=np.float32)
    img.pixels.foreach_get(a)
    bpy.data.images.remove(img)
    return a.reshape(h, w, 4)


def folha(quadros, destino, colunas=6):
    linhas = (len(quadros) + colunas - 1) // colunas
    f = np.ones((CELULA * linhas, CELULA * colunas, 4), dtype=np.float32)
    for i, caminho in enumerate(quadros):
        lin, col = divmod(i, colunas)
        y0, x0 = (linhas - 1 - lin) * CELULA, col * CELULA
        f[y0:y0 + CELULA, x0:x0 + CELULA] = _ler(caminho)[:CELULA, :CELULA]
        f[y0:y0 + 1, x0:x0 + CELULA, :3] = 0.06
        f[y0:y0 + CELULA, x0:x0 + 1, :3] = 0.06
    img = bpy.data.images.new("folha", CELULA * colunas, CELULA * linhas, alpha=False)
    img.pixels.foreach_set(f.ravel())
    img.filepath_raw = destino
    img.file_format = "PNG"
    img.save()
    bpy.data.images.remove(img)


def main():
    pedidos = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    chaves = pedidos or list(herois.HEROIS)
    c = poli.cena("bb_herois")
    for o in list(bpy.data.objects):  # cubo, camera e luz padrao do Blender
        bpy.data.objects.remove(o, do_unlink=True)
    pasta = os.path.join(RAIZ, "dist", "modelos", "herois")
    os.makedirs(pasta, exist_ok=True)
    resumo, jogo, tq = {}, [], []
    t0 = time.time()
    for chave in chaves:
        partes.limpar(c)
        m = partes.Montagem(c, chave, (0, 0, 0))
        herois.HEROIS[chave](m)
        pecas_, pivos = m.fechar()
        info = partes.exportar(c, chave, "0-0-0", pecas_, pivos, RAIZ)
        info["ok"] = info["tris"] <= partes.TETO_TRIS and info["malhas"] <= partes.TETO_MALHAS and info["cor"]
        resumo[chave] = info
        alvo, tam = herois.ENQUADRE.get(chave, herois.ENQUADRE_PADRAO)
        for vista, lista in (("jogo", jogo), ("tres_quartos", tq)):
            arq = os.path.join(pasta, f"{chave}_{vista}.png")
            poli.render(c, arq, vista, alvo=alvo, largura=tam, altura=tam, px=CELULA / tam)
            lista.append(arq)
        print(f"[heroi] {chave} tris={info['tris']} malhas={info['malhas']} bytes={info['bytes']} {'ok' if info['ok'] else 'FORA'}", flush=True)
    if not pedidos:
        capturas = os.path.join(RAIZ, "docs", "design", "capturas", "modelos")
        folha(jogo, os.path.join(capturas, "herois.png"))
        folha(tq, os.path.join(capturas, "herois_3q.png"))
        with open(os.path.join(pasta, "resumo.json"), "w", encoding="utf-8") as f:
            json.dump(resumo, f, indent=1)
    print("[gerar_herois]", json.dumps({
        "herois": len(resumo), "max_tris": max(i["tris"] for i in resumo.values()), "bytes": sum(i["bytes"] for i in resumo.values()),
        "fora": [k for k, i in resumo.items() if not i["ok"]], "segundos": round(time.time() - t0)}))


main()
