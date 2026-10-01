"""Gera as variacoes de upgrade de uma torre, montadas por partes, sem abrir a interface.

Uso (na raiz do repositorio):
    blender --background --python tools/blender/gerar_variacoes.py -- <chave> [a-b-c ...]

Sem combinacao, gera as 64 e a folha de contato. Com uma ou mais combinacoes, gera so essas e as
folhas de revisao delas em dist/modelos/<chave>/ (fora do git).

Saidas:
    assets/modelos/<chave>/<a>-<b>-<c>.glb e .json
    docs/design/capturas/modelos/<chave>_variacoes.png      folha 8x8 na vista do jogo
    docs/design/capturas/modelos/<chave>_variacoes_48.png   a mesma folha no tamanho do mapa (48 px por unidade)
    dist/modelos/<chave>/resumo.json                        triangulos, malhas e bytes de cada variacao
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

import partes  # noqa: E402
import poli  # noqa: E402

CELULA = 184   # lado de cada quadro da folha grande, em pixels
FONTE = {"0": "111101101101111", "1": "010110010010111", "2": "111001111100111", "3": "111001111001111",
         "4": "101101111001001", "5": "111100111001111", "-": "000000111000000"}


def _ler(caminho):
    img = bpy.data.images.load(caminho)
    w, h = img.size
    a = np.empty(w * h * 4, dtype=np.float32)
    img.pixels.foreach_get(a)
    bpy.data.images.remove(img)
    return a.reshape(h, w, 4)


def _rotulo(folha, texto, x, topo, escala):
    """Escreve o rotulo com fonte de pontos 3x5 (a folha esta de baixo para cima)."""
    larg = (len(texto) * 4 + 1) * escala
    folha[topo - 7 * escala:topo, x:x + larg, :3] = 0.06
    for i, ch in enumerate(texto):
        for k, bit in enumerate(FONTE[ch]):
            if bit == "1":
                px, py = x + (1 + i * 4 + k % 3) * escala, topo - (1 + k // 3) * escala
                folha[py - escala:py, px:px + escala, :3] = 1.0


def folha_contato(celulas, destino, lado, escala):
    """Grade 8x8 das variacoes, na ordem das combinacoes, com o rotulo a-b-c em cada quadro."""
    n = 8
    folha = np.ones((lado * n, lado * n, 4), dtype=np.float32)
    for i, (rot, caminho) in enumerate(celulas):
        lin, col = divmod(i, n)
        y0, x0 = (n - 1 - lin) * lado, col * lado
        folha[y0:y0 + lado, x0:x0 + lado] = _ler(caminho)[:lado, :lado]
        folha[y0:y0 + 1, x0:x0 + lado, :3] = 0.06
        folha[y0:y0 + lado, x0:x0 + 1, :3] = 0.06
        _rotulo(folha, rot, x0 + 2, y0 + lado - 1, escala)
    img = bpy.data.images.new("folha", lado * n, lado * n, alpha=False)
    img.pixels.foreach_set(folha.ravel())
    img.filepath_raw = destino
    img.file_format = "PNG"
    img.save()
    bpy.data.images.remove(img)


def main():
    args = sys.argv[sys.argv.index("--") + 1:]
    chave, pedidas = args[0], [tuple(int(v) for v in a.split("-")) for a in args[1:]]
    torre = partes.carregar(chave)
    alvo, tam = torre.ENQUADRE
    combos = pedidas or partes.combinacoes()
    c = poli.cena("bb_" + chave)
    for o in list(bpy.data.objects):  # cubo, camera e luz padrao do Blender
        bpy.data.objects.remove(o, do_unlink=True)
    pasta_dist = os.path.join(RAIZ, "dist", "modelos", chave)
    os.makedirs(os.path.join(pasta_dist, "celulas"), exist_ok=True)
    resumo, grandes, pequenas = {}, [], []
    lado48 = int(tam * 48)
    t0 = time.time()
    for tier in combos:
        rot = "-".join(str(v) for v in tier)
        partes.limpar(c)
        pecas_, pivos = partes.montar(c, chave, *tier)
        info = partes.exportar(c, chave, rot, pecas_, pivos, RAIZ)
        info["ok"] = info["tris"] <= partes.TETO_TRIS and info["malhas"] <= partes.TETO_MALHAS and info["cor"]
        resumo[rot] = info
        g = os.path.join(pasta_dist, "celulas", rot + ".png")
        p = os.path.join(pasta_dist, "celulas", rot + "_48.png")
        poli.render(c, g, "jogo", alvo=alvo, largura=tam, altura=tam, px=CELULA / tam)
        poli.render(c, p, "jogo", alvo=alvo, largura=tam, altura=tam, px=48)
        grandes.append((rot, g))
        pequenas.append((rot, p))
        if pedidas:
            for v in ("tres_quartos", "frente", "costas"):
                poli.render(c, os.path.join(pasta_dist, f"{rot}_{v}.png"), v, alvo=alvo, largura=tam, altura=tam, px=int(680 / tam))
            poli.render(c, os.path.join(pasta_dist, f"{rot}_jogo.png"), "jogo", alvo=alvo, largura=tam, altura=tam, px=int(680 / tam))
        print(f"[variacao] {chave} {rot} tris={info['tris']} malhas={info['malhas']} bytes={info['bytes']} {'ok' if info['ok'] else 'FORA'}", flush=True)
    if not pedidas:
        capturas = os.path.join(RAIZ, "docs", "design", "capturas", "modelos")
        folha_contato(grandes, os.path.join(capturas, chave + "_variacoes.png"), CELULA, 3)
        folha_contato(pequenas, os.path.join(capturas, chave + "_variacoes_48.png"), lado48, 2)
        with open(os.path.join(pasta_dist, "resumo.json"), "w", encoding="utf-8") as f:
            json.dump(resumo, f, indent=1)
    pior = max(resumo, key=lambda r: resumo[r]["tris"])
    print("[gerar_variacoes]", json.dumps({
        "torre": chave, "variacoes": len(resumo), "max_tris": resumo[pior]["tris"], "em": pior,
        "max_malhas": max(i["malhas"] for i in resumo.values()), "bytes": sum(i["bytes"] for i in resumo.values()),
        "fora": [r for r, i in resumo.items() if not i["ok"]], "segundos": round(time.time() - t0)}))


main()
