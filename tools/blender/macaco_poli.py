"""O macaco padrao em malha poligonal continua (um corpo so para todas as torres).

Partes moveis, cada uma uma malha: corpo, cauda, cabeca e braco (o do ataque). A pelagem e uma
malha trocavel que entra junto da cabeca. Altura 1,0, em pe na origem, olhando para -Y.
"""
import math

import bmesh

import poli

OMBRO = (0.150, 0.0, 0.475)
MAO = (0.285, -0.150, 0.415)


def _corpo(c, pelo="pelo", pele="pele"):
    bm = bmesh.new()
    # tronco em pera: peito mais estreito, barriga mais larga e para a frente
    poli.bloco_esfera(bm, (0, 0.005, 0.335), (0.175, 0.150, 0.165))
    poli.bloco_esfera(bm, (0, -0.005, 0.445), (0.135, 0.120, 0.120))
    poli.bloco_esfera(bm, (0, -0.045, 0.325), (0.125, 0.110, 0.120))
    for sx in (-1, 1):
        # pernas curtas e pes grandes
        poli.bloco_tubo(bm, [(0.085 * sx, 0.0, 0.250), (0.100 * sx, -0.010, 0.085)], [0.070, 0.058])
        poli.bloco_esfera(bm, (0.108 * sx, -0.060, 0.045), (0.074, 0.120, 0.045))
    # braco esquerdo comprido, solto ao lado
    poli.bloco_tubo(bm, [(-0.150, 0.0, 0.475), (-0.235, -0.010, 0.375), (-0.262, -0.035, 0.265)], [0.056, 0.048, 0.044])
    poli.bloco_esfera(bm, (-0.268, -0.045, 0.225), (0.058, 0.066, 0.068))
    o = poli.remalhar("corpo", bm, c, faces=720, voxel=0.011, suave=8)
    poli.colorir(o, [
        (pele, lambda p: max(p.z - 0.088, p.y - 0.10)),                                   # pes
        (pele, lambda p: max(p.z - 0.268, p.x + 0.20)),                                   # mao esquerda
        (pele, lambda p: max((p.x / 0.110) ** 2 + ((p.z - 0.330) / 0.130) ** 2 - 1, (p.y + 0.02) * 8)),   # barriga
    ], base=pelo)
    return o


def _braco(c, pelo="pelo", pele="pele"):
    bm = bmesh.new()
    cotovelo = (0.245, -0.040, 0.405)
    poli.bloco_tubo(bm, [OMBRO, cotovelo, (0.275, -0.125, 0.412)], [0.058, 0.048, 0.044])
    poli.bloco_esfera(bm, MAO, (0.062, 0.066, 0.062))
    o = poli.remalhar("braco", bm, c, faces=190, voxel=0.010, suave=6, simetria=False)
    poli.colorir(o, [(pele, lambda p: p.y + 0.105)], base=pelo)
    return o


def _cabeca(c, pelo="pelo", pele="pele"):
    bm = bmesh.new()
    poli.bloco_esfera(bm, (0, 0.0, 0.735), (0.262, 0.232, 0.222))            # cranio largo
    poli.bloco_esfera(bm, (0, -0.150, 0.655), (0.150, 0.110, 0.098))         # focinho
    for sx in (-1, 1):
        poli.bloco_esfera(bm, (0.105 * sx, -0.120, 0.660), (0.090, 0.085, 0.080))   # bochechas
        poli.bloco_esfera(bm, (0.082 * sx, -0.150, 0.800), (0.075, 0.060, 0.070))   # arco das sobrancelhas
        poli.bloco_esfera(bm, (0.292 * sx, 0.020, 0.760), (0.108, 0.052, 0.116))    # orelha em disco
    o = poli.remalhar("cabeca", bm, c, faces=800, voxel=0.010, suave=6)

    def orelha(p):
        return max(((abs(p.x) - 0.305) / 0.068) ** 2 + ((p.z - 0.760) / 0.078) ** 2 - 1, (p.y - 0.020) * 8, (0.235 - abs(p.x)) * 8)

    def mascara(p):
        # dois arcos em volta dos olhos unidos ao focinho, so na frente da cabeca
        olhos = min(((p.x - sx * 0.080) / 0.126) ** 2 + ((p.z - 0.792) / 0.104) ** 2 for sx in (-1, 1)) - 1
        foc = (p.x / 0.190) ** 2 + ((p.z - 0.655) / 0.110) ** 2 - 1
        return max(min(olhos, foc), (p.y + 0.060) * 8)

    poli.colorir(o, [(pele, orelha), (pele, mascara)], base=pelo)
    return o


def _rosto(c):
    """Olhos, nariz e sorriso: pecas pequenas que entram na malha da cabeca."""
    pecas = []
    # olho torneado em volta do eixo do olhar: a pupila sai redonda, sem depender da malha
    perfil = [(math.sin(math.radians(g)), -math.cos(math.radians(g))) for g in (0, 17, 34, 58, 90, 135, 180)]
    perfil[0] = (0, -1)
    perfil[-1] = (0, 1)
    for sx in (-1, 1):
        bm = poli.torno(perfil, seg=16, cores=["tinta", "tinta", "branco", "branco", "branco", "branco"])
        for v in bm.verts:
            v.co.x *= 0.060
            v.co.y *= 0.072
            v.co.z *= 0.042
        # o polo da pupila aponta para a frente e um pouco para dentro
        poli.orientar(bm, (0.10 * sx, 1, 0), (0.086 * sx, -0.190, 0.792))
        pecas.append(poli.objeto(f"olho_{sx}", bm, c, nivel=0))
    bm = poli.gaiola((0.034, 0.020, 0.024), cortes=0, cor="tinta")
    poli.mover(bm, (0, -0.258, 0.690))
    pecas.append(poli.objeto("nariz", bm, c, nivel=1))
    sorriso = [(0.070 * math.sin(a), -0.252 + 0.012 * abs(math.sin(a)), 0.632 - 0.020 * math.cos(a))
               for a in (math.radians(g) for g in (-62, -31, 0, 31, 62))]
    pecas.append(poli.objeto("sorriso", poli.membro(sorriso, [0.004, 0.007, 0.008, 0.007, 0.004], cor="tinta"), c, nivel=1))
    return pecas


def _pelagem(c, estilo, cor="pelo_escuro"):
    """Pelo por cima do corpo padrao: redondo na base, pontudo na ponta."""
    pecas = []

    def mecha(nome, pts, raios):
        pecas.append(poli.objeto(nome, poli.membro(pts, raios, cor=cor), c, nivel=1))

    if estilo == "topete":
        for i, (x, y, dx, dy, h, r) in enumerate(((0, -0.075, 0, -0.110, 0.150, 0.050), (-0.085, -0.045, -0.085, -0.055, 0.110, 0.042),
                                                  (0.085, -0.045, 0.085, -0.055, 0.110, 0.042), (0, 0.020, 0, 0.030, 0.135, 0.046))):
            mecha(f"topete_{i}", [(x, y, 0.915), (x + dx * 0.45, y + dy * 0.45, 0.915 + h * 0.62), (x + dx, y + dy, 0.915 + h)],
                  [r, r * 0.62, 0.004])
    elif estilo == "crista":
        for i in range(5):
            y = -0.130 + i * 0.066
            alt = 0.150 - abs(i - 1.5) * 0.022
            mecha(f"crista_{i}", [(0, y, 0.915), (0, y + 0.015, 0.915 + alt * 0.6), (0, y + 0.045, 0.915 + alt)], [0.040, 0.028, 0.004])
    elif estilo == "tufos":
        for sx in (-1, 1):
            for k, (dz, comp) in enumerate(((0.0, 0.130), (-0.060, 0.105), (0.055, 0.100))):
                mecha(f"tufo_{sx}_{k}", [(0.205 * sx, -0.060, 0.640 + dz), ((0.205 + comp * 0.55) * sx, -0.080, 0.628 + dz * 1.3),
                                         ((0.205 + comp) * sx, -0.085, 0.600 + dz * 1.7)], [0.040, 0.028, 0.004])
    elif estilo == "barba":
        mecha("barba", [(0, -0.190, 0.600), (0, -0.215, 0.520), (0, -0.215, 0.440)], [0.085, 0.060, 0.004])
    return pecas


def _cauda(c, pelo="pelo"):
    centro, r = (0, 0.290, 0.300), 0.150
    pts = [(0, 0.115, 0.255)]
    for g in (200, 250, 300, 350, 40, 90, 130):
        a = math.radians(g)
        pts.append((0, centro[1] + r * math.cos(a), centro[2] + r * math.sin(a)))
    raios = [0.034, 0.032, 0.030, 0.028, 0.025, 0.022, 0.018, 0.004]
    return poli.objeto("cauda", poli.membro(pts, raios, cor=pelo), c, nivel=1)


PIVOS = {"corpo": (0, 0, 0.30), "cauda": (0, 0.115, 0.255), "cabeca": (0, 0, 0.56), "braco": OMBRO}


def construir(c, pelagem="topete", pelo="pelo", pele="pele", pelo_escuro="pelo_escuro"):
    """Monta o macaco na cena dada e devolve as quatro malhas moveis e os pivos.

    pelo, pele e pelo_escuro sao nomes de cor da PALETA; so o Macaco de Gelo troca o padrao."""
    corpo = _corpo(c, pelo, pele)
    braco = _braco(c, pelo, pele)
    cauda = _cauda(c, pelo)
    cabeca = poli.juntar("cabeca", [_cabeca(c, pelo, pele)] + _rosto(c) + _pelagem(c, pelagem, pelo_escuro), c)
    partes = {"corpo": corpo, "cauda": cauda, "cabeca": cabeca, "braco": braco}
    return partes, dict(PIVOS)
