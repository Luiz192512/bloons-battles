"""Gera o retrato 2D de cada torre e heroi a partir do proprio modelo 3D, sem abrir a interface.

O retrato e o desenho da loja: o modelo base (0-0-0) visto de frente, em tres quartos, com cor reforcada,
traco de tinta em volta da silhueta (como figurinha) e fundo transparente. Macaco sai em busto, para o rosto e o chapeu
ficarem grandes no cartao; maquina e construcao saem inteiras.

Uso (na raiz do repositorio):
    blender --background --python tools/blender/gerar_retratos.py -- [chave ...]

Saidas:
    assets/retratos/<chave>.png                     256 x 256, com transparencia (o jogo carrega)
    docs/design/capturas/modelos/retratos.png       folha com todos, para revisar
"""
import os
import sys

AQUI = os.path.dirname(os.path.abspath(__file__))
RAIZ = os.path.dirname(os.path.dirname(AQUI))
sys.path.insert(0, AQUI)

import bpy  # noqa: E402
import numpy as np  # noqa: E402
from mathutils import Vector  # noqa: E402

import herois  # noqa: E402
import partes  # noqa: E402
import poli  # noqa: E402

LADO = 256
TORRES = ["dardo", "bumerangue", "bomba", "tachinha", "gelo", "cola", "sniper", "submarino", "bucaneiro", "as", "heli", "morteiro",
          "dartling", "mago", "super", "ninja", "alquimista", "druida", "fazenda", "espinhos", "vila", "engenheiro"]
# sem macaco em pe (ou com ele pequeno em cima de algo): o retrato mostra o conjunto inteiro
INTEIRAS = {"bomba", "tachinha", "submarino", "bucaneiro", "as", "heli", "morteiro", "fazenda", "espinhos", "vila", "churchill"}
BUSTO = ((0.03, 0, 0.72), 0.98)
SUPER = 2   # renderiza maior e reduz, para o traco sair liso
# quem tem chapeu alto, cajado ou corpo maior precisa de mais quadro
QUADRO = {"pat": ((0.03, 0, 0.96), 1.30), "obyn": ((0.03, 0, 0.80), 1.16), "adora": ((0.03, 0, 0.80), 1.16), "silas": ((0.03, 0, 0.84), 1.24),
          "mago": ((0.03, 0, 0.84), 1.24), "druida": ((0.03, 0, 0.78), 1.12), "super": ((0.03, 0, 0.86), 1.16), "etienne": ((-0.08, 0, 0.78), 1.16),
          "corvus": ((0.03, 0, 0.80), 1.16), "ezili": ((0.03, 0, 0.78), 1.12), "psi": ((0, 0, 0.78), 1.10), "dan": ((0.03, 0, 0.76), 1.08),
          "geraldo": ((0.03, 0, 0.76), 1.08), "brickell": ((0.03, 0, 0.76), 1.06), "sauda": ((0, 0, 0.72), 1.12), "quincy": ((0.03, 0, 0.72), 1.08)}


def _ler(caminho):
    img = bpy.data.images.load(caminho)
    w, h = img.size
    a = np.empty(w * h * 4, dtype=np.float32)
    img.pixels.foreach_get(a)
    bpy.data.images.remove(img)
    return a.reshape(h, w, 4)


def _srgb(x):
    return np.where(x <= 0.0031308, x * 12.92, 1.055 * np.power(np.clip(x, 0, 1), 1 / 2.4) - 0.055)


def _linear(x):
    return np.where(x <= 0.04045, x / 12.92, np.power((x + 0.055) / 1.055, 2.4))


def acabamento(caminho):
    """Do render cru ao retrato: reforca a cor, poe o traco de tinta em volta e reduz para LADO."""
    q = _ler(caminho)
    cor, a = _srgb(q[:, :, :3]), q[:, :, 3]
    cinza = cor.mean(axis=2, keepdims=True)
    cor = np.clip(cinza + (cor - cinza) * 1.35, 0, 1)        # mais saturacao
    cor = np.clip((cor - 0.5) * 1.18 + 0.47, 0, 1)           # mais contraste
    # traco: a silhueta engordada em tinta, por baixo do desenho
    r = 5 * SUPER
    gordo = np.zeros_like(a)
    h0, w0 = a.shape
    borda = np.pad(a, r)   # sem dar a volta na imagem
    for dy in range(-r, r + 1):
        for dx in range(-r, r + 1):
            if dx * dx + dy * dy <= r * r:
                gordo = np.maximum(gordo, borda[r + dy:r + dy + h0, r + dx:r + dx + w0])
    tinta = np.array([0.086, 0.078, 0.102], dtype=np.float32)
    rgb = cor * a[:, :, None] + tinta * (1 - a[:, :, None])
    saida = np.dstack([rgb * gordo[:, :, None], gordo])       # premultiplicado, para reduzir sem franja
    h, w = saida.shape[0] // SUPER, saida.shape[1] // SUPER
    saida = saida[:h * SUPER, :w * SUPER].reshape(h, SUPER, w, SUPER, 4).mean(axis=(1, 3))
    al = saida[:, :, 3:4]
    final = np.dstack([_linear(np.where(al > 0, saida[:, :, :3] / np.maximum(al, 1e-6), 0)), al]).astype(np.float32)
    img = bpy.data.images.new("retrato", w, h, alpha=True)
    img.alpha_mode = "STRAIGHT"
    img.pixels.foreach_set(final.ravel())
    img.filepath_raw = caminho
    img.file_format = "PNG"
    img.save()
    bpy.data.images.remove(img)


def enquadrar(c, vista):
    """Centro e tamanho do quadro que pega o modelo inteiro, visto da direcao dada."""
    d = Vector(vista).normalized()
    lado = Vector((0, 0, 1)).cross(d).normalized()
    cima = d.cross(lado)
    us, ws, ps = [], [], []
    for o in c.objects:
        if o.type == "MESH":
            for v in o.data.vertices:
                p = o.matrix_world @ v.co
                us.append(p.dot(lado)), ws.append(p.dot(cima)), ps.append(p.dot(d))
    centro = lado * (min(us) + max(us)) / 2 + cima * (min(ws) + max(ws)) / 2 + d * (sum(ps) / len(ps))
    return tuple(centro), max(max(us) - min(us), max(ws) - min(ws))


def folha(arquivos, destino, colunas=8):
    linhas = (len(arquivos) + colunas - 1) // colunas
    f = np.zeros((LADO * linhas, LADO * colunas, 4), dtype=np.float32)
    f[:, :, :3] = (0.93, 0.84, 0.63)   # bege do cartao da loja
    f[:, :, 3] = 1
    for i, arq in enumerate(arquivos):
        lin, col = divmod(i, colunas)
        y0, x0 = (linhas - 1 - lin) * LADO, col * LADO
        q = _ler(arq)
        a = q[:, :, 3:4]
        f[y0:y0 + LADO, x0:x0 + LADO, :3] = f[y0:y0 + LADO, x0:x0 + LADO, :3] * (1 - a) + q[:, :, :3] * a
    img = bpy.data.images.new("folha", LADO * colunas, LADO * linhas, alpha=False)
    img.pixels.foreach_set(f.ravel())
    img.filepath_raw = destino
    img.file_format = "PNG"
    img.save()
    bpy.data.images.remove(img)


def main():
    pedidos = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    chaves = pedidos or TORRES + list(herois.HEROIS)
    c = poli.cena("bb_retratos")
    for o in list(bpy.data.objects):
        bpy.data.objects.remove(o, do_unlink=True)
    poli.VISTAS["retrato"] = (0.42, -1, 0.20)
    c.render.film_transparent = True
    c.render.image_settings.color_mode = "RGBA"
    pasta = os.path.join(RAIZ, "assets", "retratos")
    os.makedirs(pasta, exist_ok=True)
    feitos = []
    for chave in chaves:
        partes.limpar(c)
        if chave in herois.HEROIS:
            m = partes.Montagem(c, chave, (0, 0, 0))
            herois.HEROIS[chave](m)
            m.fechar()
            enquadre = herois.ENQUADRE.get(chave, herois.ENQUADRE_PADRAO)
        else:
            partes.montar(c, chave, 0, 0, 0)
            enquadre = partes.carregar(chave).ENQUADRE
        alvo, tam = enquadrar(c, poli.VISTAS["retrato"]) if chave in INTEIRAS else QUADRO.get(chave, BUSTO)
        tam *= 1.12   # folga para o traco nao encostar na borda
        arq = os.path.join(pasta, chave + ".png")
        poli.render(c, arq, "retrato", alvo=alvo, largura=tam, altura=tam, px=(LADO * SUPER + 0.5) / tam)
        acabamento(arq)
        feitos.append(arq)
        print(f"[retrato] {chave}", flush=True)
    if not pedidos:
        folha(feitos, os.path.join(RAIZ, "docs", "design", "capturas", "modelos", "retratos.png"))
    print("[gerar_retratos]", len(feitos), "retratos,", sum(os.path.getsize(a) for a in feitos), "bytes")


main()
