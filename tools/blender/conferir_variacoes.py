"""Confere as variacoes exportadas de uma ou mais torres, lendo os .glb e .json (Python comum, sem Blender).

Uso (na raiz do repositorio):
    python tools/blender/conferir_variacoes.py dardo [bomba ...]

Para cada torre: as 64 combinacoes existem, cada .glb tem cor por vertice, ate 6 malhas e ate
10.000 triangulos, e o .json lista os grupos na ordem das malhas do .glb, com pivo. Imprime a
linha de estado do relatorio (docs/modelos-3d-variacoes.md).
"""
import json
import os
import struct
import sys

RAIZ = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TETO_TRIS, TETO_MALHAS = 10000, 6
GRUPOS = ["base", "corpo", "cauda", "cabeca", "torreta", "braco"]


def combinacoes():
    todas = []
    for a in range(6):
        for b in range(6):
            for c in range(6):
                t = sorted((a, b, c))
                if t[0] == 0 and t[1] <= 2:
                    todas.append(f"{a}-{b}-{c}")
    return todas


def ler_glb(caminho):
    with open(caminho, "rb") as f:
        f.read(12)
        tam, _tipo = struct.unpack("<II", f.read(8))
        return json.loads(f.read(tam))


def conferir(chave):
    pasta = os.path.join(RAIZ, "assets", "modelos", chave)
    erros, pior, pior_em, max_malhas, total = [], 0, "", 0, 0
    feitas = 0
    for rot in combinacoes():
        glb, js = os.path.join(pasta, rot + ".glb"), os.path.join(pasta, rot + ".json")
        if not (os.path.exists(glb) and os.path.exists(js)):
            erros.append(f"{rot}: falta o arquivo")
            continue
        feitas += 1
        total += os.path.getsize(glb)
        doc = ler_glb(glb)
        nomes, tris = [], 0
        for no in doc.get("nodes", []):
            if "mesh" not in no:
                continue
            me = doc["meshes"][no["mesh"]]
            for pr in me["primitives"]:
                nomes.append(me.get("name", ""))
                tris += doc["accessors"][pr["indices"]]["count"] // 3
                if "COLOR_0" not in pr["attributes"]:
                    erros.append(f"{rot}: malha {me.get('name')} sem cor")
                if "material" in pr and len(doc.get("materials", [])) > 1:
                    erros.append(f"{rot}: mais de um material")
        if doc.get("textures") or doc.get("skins"):
            erros.append(f"{rot}: tem textura ou armature")
        if tris > TETO_TRIS:
            erros.append(f"{rot}: {tris} triangulos")
        if len(nomes) > TETO_MALHAS:
            erros.append(f"{rot}: {len(nomes)} malhas")
        if tris > pior:
            pior, pior_em = tris, rot
        max_malhas = max(max_malhas, len(nomes))
        info = json.load(open(js, encoding="utf-8"))["malhas"]
        grupos_glb = [n.rsplit("_", 1)[-1].split(".")[0] for n in nomes]
        if [i["grupo"] for i in info] != grupos_glb:
            erros.append(f"{rot}: o .json nao bate com a ordem do .glb")
        if any(g not in GRUPOS for g in grupos_glb) or any(len(i.get("pivo", [])) != 3 for i in info):
            erros.append(f"{rot}: grupo desconhecido ou pivo ausente")
    mb = f"{total / 1e6:.1f}".replace(".", ",")
    print(f"| {chave} | {feitas} de 64 | {pior:,} ({pior_em}) | {max_malhas} | {mb} MB |".replace(",", ".", 1) if pior >= 1000 else
          f"| {chave} | {feitas} de 64 | {pior} ({pior_em}) | {max_malhas} | {mb} MB |")
    for e in erros:
        print("  ERRO", e)
    return not erros


if __name__ == "__main__":
    ok = all([conferir(ch) for ch in sys.argv[1:]])
    sys.exit(0 if ok else 1)
