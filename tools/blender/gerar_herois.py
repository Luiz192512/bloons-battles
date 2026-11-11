"""Gera os modelos 3D dos herois (um .glb por estagio de nivel), sem abrir a interface.

Uso (na raiz do repositorio):
    blender --background --python tools/blender/gerar_herois.py -- [chave ...] [estagio]

Sem chave, gera os 18 herois com todos os estagios e as folhas de contato. Com chaves, so esses;
um numero depois das chaves gera so aquele estagio (por exemplo "-- quincy 2"). Saidas:
    assets/modelos/<chave>/<e>-0-0.glb e .json              e = estagio (0 = nivel 1)
    docs/design/capturas/modelos/herois.png                 folha 6x3 do estagio 0, vista do jogo
    docs/design/capturas/modelos/herois_3q.png              a mesma folha na vista tres quartos
    docs/design/capturas/modelos/herois_estagios.png        uma linha por heroi, uma coluna por estagio
    docs/design/capturas/modelos/herois_estagios_48.png     a mesma no tamanho do mapa (48 px por unidade)
    dist/modelos/herois/                                    quadros soltos e, com chaves, as folhas (fora do git)
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


def folha(linhas_de_quadros, destino, lado=CELULA):
    """Grade de quadros: cada linha e uma lista de caminhos (None deixa o quadro vazio)."""
    colunas = max(len(l) for l in linhas_de_quadros)
    linhas = len(linhas_de_quadros)
    f = np.ones((lado * linhas, lado * colunas, 4), dtype=np.float32)
    for lin, quadros in enumerate(linhas_de_quadros):
        for col, caminho in enumerate(quadros):
            if not caminho:
                continue
            y0, x0 = (linhas - 1 - lin) * lado, col * lado
            f[y0:y0 + lado, x0:x0 + lado] = _ler(caminho)[:lado, :lado]
            f[y0:y0 + 1, x0:x0 + lado, :3] = 0.06
            f[y0:y0 + lado, x0:x0 + 1, :3] = 0.06
    img = bpy.data.images.new("folha", lado * colunas, lado * linhas, alpha=False)
    img.pixels.foreach_set(f.ravel())
    img.filepath_raw = destino
    img.file_format = "PNG"
    img.save()
    bpy.data.images.remove(img)


def _em_linhas(quadros, colunas):
    return [quadros[i:i + colunas] for i in range(0, len(quadros), colunas)]


def main():
    pedidos = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    so_estagio = int(pedidos.pop()) if pedidos and pedidos[-1].isdigit() else None
    chaves = pedidos or list(herois.HEROIS)
    c = poli.cena("bb_herois")
    for o in list(bpy.data.objects):  # cubo, camera e luz padrao do Blender
        bpy.data.objects.remove(o, do_unlink=True)
    pasta = os.path.join(RAIZ, "dist", "modelos", "herois")
    os.makedirs(pasta, exist_ok=True)
    resumo, jogo, tq, grade, grade48 = {}, [], [], [], []
    t0 = time.time()
    for chave in chaves:
        alvo, tam = herois.ENQUADRE.get(chave, herois.ENQUADRE_PADRAO)
        lado48 = int(tam * 48)
        linha, linha48 = [], []
        for e in range(herois.estagios(chave)):
            if so_estagio is not None and e != so_estagio:
                continue
            partes.limpar(c)
            m = partes.Montagem(c, chave, (e, 0, 0))
            herois.montar(m, chave, e)
            pecas_, pivos = m.fechar()
            rot = f"{e}-0-0"
            info = partes.exportar(c, chave, rot, pecas_, pivos, RAIZ)
            info["ok"] = info["tris"] <= partes.TETO_TRIS and info["malhas"] <= partes.TETO_MALHAS and info["cor"]
            resumo[f"{chave}/{rot}"] = info
            quadro = {}
            for vista in ("jogo", "tres_quartos"):
                quadro[vista] = os.path.join(pasta, f"{chave}_{e}_{vista}.png")
                poli.render(c, quadro[vista], vista, alvo=alvo, largura=tam, altura=tam, px=CELULA / tam)
            pequeno = os.path.join(pasta, f"{chave}_{e}_48.png")
            poli.render(c, pequeno, "jogo", alvo=alvo, largura=tam, altura=tam, px=48)
            linha.append(quadro["jogo"])
            linha48.append(pequeno)
            if e == 0:
                jogo.append(quadro["jogo"])
                tq.append(quadro["tres_quartos"])
            print(f"[heroi] {chave} {rot} tris={info['tris']} malhas={info['malhas']} bytes={info['bytes']} {'ok' if info['ok'] else 'FORA'}", flush=True)
        grade.append(linha)
        grade48.append((lado48, linha48))
    capturas = os.path.join(RAIZ, "docs", "design", "capturas", "modelos") if not pedidos and so_estagio is None else pasta
    if not pedidos and so_estagio is None:
        folha(_em_linhas(jogo, 6), os.path.join(capturas, "herois.png"))
        folha(_em_linhas(tq, 6), os.path.join(capturas, "herois_3q.png"))
        with open(os.path.join(pasta, "resumo.json"), "w", encoding="utf-8") as f:
            json.dump(resumo, f, indent=1)
    folha(grade, os.path.join(capturas, "herois_estagios.png"))
    # a 48 px os quadros tem o lado do enquadre de cada heroi: a folha usa o maior
    lado = max(l for l, _ in grade48)
    folha48(grade48, os.path.join(capturas, "herois_estagios_48.png"), lado)
    print("[gerar_herois]", json.dumps({
        "arquivos": len(resumo), "max_tris": max(i["tris"] for i in resumo.values()), "bytes": sum(i["bytes"] for i in resumo.values()),
        "fora": [k for k, i in resumo.items() if not i["ok"]], "segundos": round(time.time() - t0)}))


def folha48(grade48, destino, lado):
    """Folha no tamanho do mapa: cada quadro centrado numa celula do tamanho do maior enquadre."""
    colunas = max(len(q) for _, q in grade48)
    linhas = len(grade48)
    f = np.ones((lado * linhas, lado * colunas, 4), dtype=np.float32)
    f[:, :, :3] = (0.12, 0.40, 0.05)   # o mesmo verde do fundo dos quadros
    for lin, (l, quadros) in enumerate(grade48):
        for col, caminho in enumerate(quadros):
            q = _ler(caminho)[:l, :l]
            y0, x0 = (linhas - 1 - lin) * lado + (lado - l) // 2, col * lado + (lado - l) // 2
            f[y0:y0 + l, x0:x0 + l] = q
    img = bpy.data.images.new("folha", lado * colunas, lado * linhas, alpha=False)
    img.pixels.foreach_set(f.ravel())
    img.filepath_raw = destino
    img.file_format = "PNG"
    img.save()
    bpy.data.images.remove(img)


main()
