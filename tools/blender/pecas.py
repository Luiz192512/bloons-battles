"""Pecas reaproveitadas pelas torres: aneis, folhas, cascas de cabeca, roupas e armas simples.

Cada funcao recebe a montagem (m) e devolve a lista de objetos da peca, ja na posicao do macaco
padrao (em pe na origem, olhando para -Y). Quem chama decide o encaixe (m.por, m.somar).
"""
import math

import bmesh
from mathutils import Matrix, Vector

import macaco_poli
import poli

CABECA = (0, 0.0, 0.735)             # centro do cranio do macaco padrao
MAO = macaco_poli.MAO                # mao direita (a do ataque)
MAO_ESQ = (-0.268, -0.045, 0.225)    # mao esquerda (a livre)
PES = ((0.108, -0.060, 0.045), (-0.108, -0.060, 0.045))
OLHOS = ((0.086, -0.190, 0.792), (-0.086, -0.190, 0.792))


def girar(direcao, de=(0, -1, 0)):
    """Matriz que leva o eixo 'de' para a direcao dada."""
    return Vector(de).rotation_difference(Vector(direcao).normalized()).to_matrix().to_4x4()


def em(pos, direcao=None, escala=1.0, de=(0, -1, 0)):
    """Matriz de posicao, direcao e escala de uma peca feita em coordenadas locais."""
    m = Matrix.Translation(Vector(pos))
    if direcao is not None:
        m = m @ girar(direcao, de)
    if escala != 1.0:
        m = m @ Matrix.Scale(escala, 4)
    return m


# ---------------------------------------------------------------- formas
def arco(centro, rx, ry, g0, g1, n=10, inclina=0.0, tomba=0.0):
    """Pontos de um arco de elipse no plano XY. inclina sobe/desce com y; tomba, com x."""
    pts = []
    for i in range(n + 1):
        a = math.radians(g0 + (g1 - g0) * i / n)
        x, y = rx * math.cos(a), ry * math.sin(a)
        pts.append((centro[0] + x, centro[1] + y, centro[2] + inclina * math.sin(a) + tomba * math.cos(a)))
    return pts


def faixa(m, nome, centro, rx, ry, tubo, cor, g0=0, g1=360, n=12, inclina=0.0, tomba=0.0):
    """Anel macio (tubo de quads) em volta de uma parte do corpo: lenco, faixa, cinto."""
    pts = arco(centro, rx, ry, g0, g1, n, inclina, tomba)
    return m.obj(nome, poli.membro(pts, [tubo] * len(pts), cor=cor), nivel=1)


def toro(m, nome, centro, raio, tubo, cor, normal=(0, 0, 1), seg=14, lados=6):
    """Anel rigido (aro de oculos, halo, argola)."""
    perfil = [(raio + tubo * math.cos(2 * math.pi * k / lados), tubo * math.sin(2 * math.pi * k / lados)) for k in range(lados + 1)]
    bm = poli.torno(perfil, seg=seg, cor=cor)
    return m.obj(nome, poli.orientar(bm, normal, centro), nivel=0)


def esfera(m, nome, centro, raios, cor, cortes=1, nivel=1):
    if not isinstance(raios, (tuple, list)):
        raios = (raios, raios, raios)
    bm = poli.gaiola(raios, cortes=cortes, cor=cor)
    poli.mover(bm, centro)
    return m.obj(nome, bm, nivel=nivel)


def cone(m, nome, base, direcao, raio, comp, cor, seg=6, fechado=False):
    """Ponta viva: espinho, pena, chifre."""
    perfil = ([(0, 0)] if fechado else []) + [(raio, 0), (0, comp)]
    return m.obj(nome, poli.orientar(poli.torno(perfil, seg=seg, cor=cor), direcao, base), nivel=0, vivo=True)


def cilindro(m, nome, base, direcao, perfil, cor="cinza_escuro", cores=None, seg=12):
    """Peca torneada (perfil = [(raio, altura)]) apontada para a direcao dada."""
    return m.obj(nome, poli.orientar(poli.torno(perfil, seg=seg, cor=cor, cores=cores), direcao, base), nivel=0, vivo=True)


def bloco(m, nome, tam, pos, cor, chanfro=0.012, seg=1, matriz=None):
    """Caixa chanfrada (viga, placa, coronha)."""
    bm = poli.caixa(tam, cor, chanfro=chanfro, seg=seg)
    poli.mover(bm, pos)
    return m.obj(nome, bm, nivel=0, vivo=True, matriz=matriz)


def tubo(m, nome, pontos, raios, cor, nivel=1, matriz=None):
    if not isinstance(raios, (tuple, list)):
        raios = [raios] * len(pontos)
    return m.obj(nome, poli.membro(pontos, raios, cor=cor), nivel=nivel, matriz=matriz)


def folha(m, nome, grade, cor, esp=0.02, nivel=1):
    """Pano com espessura (capa, vela, bandeira): grade = linhas de pontos, todas do mesmo tamanho."""
    bm = bmesh.new()
    vs = [[bm.verts.new(p) for p in linha] for linha in grade]
    for i in range(len(vs) - 1):
        for k in range(len(vs[0]) - 1):
            bm.faces.new((vs[i][k], vs[i][k + 1], vs[i + 1][k + 1], vs[i + 1][k]))
    bmesh.ops.solidify(bm, geom=bm.faces[:], thickness=esp)
    for f in bm.faces:
        f.material_index = poli.INDICE[cor]
    return m.obj(nome, bm, nivel=nivel)


def estrela(m, nome, centro, raio, cor, normal=(0, -1, 0), pontas=5, esp=0.022, miolo=0.45):
    """Emblema em estrela, de frente para a direcao 'normal'."""
    bm = bmesh.new()
    frente, tras = bm.verts.new((0, 0, esp)), bm.verts.new((0, 0, -esp * 0.2))
    aro = []
    for k in range(pontas * 2):
        a = math.pi / 2 + math.pi * k / pontas
        r = raio if k % 2 == 0 else raio * miolo
        aro.append(bm.verts.new((r * math.cos(a), r * math.sin(a), 0)))
    for k in range(len(aro)):
        k2 = (k + 1) % len(aro)
        bm.faces.new((frente, aro[k], aro[k2]))
        bm.faces.new((tras, aro[k2], aro[k]))
    for f in bm.faces:
        f.material_index = poli.INDICE[cor]
    # o eixo Z local e o "de frente"; o Y local (ponta de cima) fica para cima no mundo
    rot = Matrix((Vector((1, 0, 0)), Vector((0, 0, 1)), Vector((0, -1, 0)))).transposed().to_4x4()
    mat = Matrix.Translation(Vector(centro)) @ girar(normal) @ rot
    return m.obj(nome, bm, nivel=0, vivo=True, matriz=mat)


# ---------------------------------------------------------------- cabeca
def casca(m, nome, cor, piso, raios=(0.300, 0.268, 0.262), cortes=2, centro=CABECA):
    """Casca que veste o cranio (lenco, capuz, capacete). piso(x, y) = altura minima, relativa
    ao centro da cabeca: e a barra da peca."""
    bm = poli.gaiola(raios, cortes=cortes, cor=cor)
    for v in bm.verts:
        v.co.z = max(v.co.z, piso(v.co.x, v.co.y))
    poli.mover(bm, centro)
    return m.obj(nome, bm, nivel=1)


def barra_casca(m, nome, cor, piso, raios=(0.300, 0.268, 0.262), tubo_r=0.022, centro=CABECA):
    """Barra de outra cor contornando a borda de uma casca (onde a esfera encontra o piso)."""
    pts = []
    for g in range(0, 361, 30):
        ca, sa = math.cos(math.radians(g)), -math.sin(math.radians(g))
        f = 1.0
        for _ in range(3):
            z = piso(raios[0] * f * ca, raios[1] * f * sa)
            f = math.sqrt(max(0.04, 1 - (z / raios[2]) ** 2))
        pts.append((centro[0] + raios[0] * f * ca * 0.985, centro[1] + raios[1] * f * sa * 0.985, centro[2] + z + 0.004))
    return tubo(m, nome, pts, tubo_r, cor)


def piso_lenco(x, y):
    return 0.150 - 0.34 * (y + 0.20)


def piso_capuz(x, y):
    """Aberto no rosto, fechado ate o pescoco dos lados e atras."""
    t = min(1.0, max(0.0, (y + 0.10) / 0.16))
    return 0.150 - 0.36 * t * t * (3 - 2 * t)


def piso_capacete(x, y):
    return 0.105 - 0.20 * (y + 0.20)


def faixa_testa(m, cor, nome="faixa_testa", pontas=True):
    """Faixa amarrada na testa, com o no e duas pontas ao vento atras."""
    objs = [faixa(m, nome, (0, -0.004, 0.868), 0.236, 0.212, 0.026, cor, n=12)]
    if pontas:
        objs.append(esfera(m, nome + "_no", (0.110, 0.185, 0.868), (0.042, 0.038, 0.040), cor, cortes=0))
        for k, (dx, dy, dz) in enumerate(((0.19, 0.24, -0.13), (0.07, 0.29, -0.20))):
            objs.append(tubo(m, f"{nome}_ponta_{k}", [(0.115, 0.200, 0.865), (0.115 + dx * 0.55, 0.200 + dy * 0.55, 0.865 + dz * 0.4),
                                                      (0.115 + dx, 0.200 + dy, 0.865 + dz)], [0.032, 0.030, 0.004], cor))
    return objs


def oculos(m, aro="ouro", tira="marrom_escuro", nome="oculos"):
    """Oculos redondos de aro grosso, com a tira passando por tras da cabeca."""
    objs = []
    for k, (x, y, z) in enumerate(OLHOS):
        sx = 1 if x > 0 else -1
        objs.append(toro(m, f"{nome}_aro_{k}", (x + 0.004 * sx, y - 0.036, z), 0.078, 0.017, aro, normal=(0.10 * sx, -1, 0), seg=12, lados=5))
    objs.append(tubo(m, nome + "_ponte", [(-0.020, -0.236, 0.800), (0.020, -0.236, 0.800)], 0.012, aro, nivel=0))
    objs.append(tubo(m, nome + "_tira", arco((0, 0.0, 0.795), 0.272, 0.240, -20, 200, n=8), 0.013, tira))
    return objs


# ---------------------------------------------------------------- tronco, costas e pes
def lenco(m, cor="azul", nome="lenco"):
    """O lenco do piloto: anel no pescoco, no e duas pontas."""
    objs = [m.obj(nome, poli.membro(arco((0, -0.005, 0.548), 0.150, 0.132, -60, 280, n=12, inclina=-0.012), [0.034] * 13, cor=cor), nivel=1)]
    no = poli.gaiola((0.046, 0.040, 0.046), cortes=0, cor=cor)
    poli.mover(no, (0.075, -0.135, 0.535))
    objs.append(m.obj(nome + "_no", no, nivel=1))
    for k, dx in enumerate((0.030, -0.012)):
        objs.append(m.obj(f"{nome}_ponta_{k}", poli.membro(
            [(0.075, -0.142, 0.525), (0.075 + dx * 0.6, -0.168, 0.465), (0.075 + dx, -0.172, 0.395)],
            [0.030, 0.027, 0.004], cor=cor), nivel=1))
    return objs


def cachecol(m, cor, nome="cachecol"):
    """Cachecol comprido, com as pontas voando para o lado esquerdo (aparece na vista do jogo)."""
    objs = [faixa(m, nome, (0, -0.005, 0.548), 0.156, 0.138, 0.040, cor, g0=-60, g1=280, n=12, inclina=-0.012)]
    objs.append(esfera(m, nome + "_no", (-0.085, -0.130, 0.538), (0.050, 0.044, 0.050), cor, cortes=0))
    for k, (dx, dy, dz) in enumerate(((-0.36, 0.10, -0.06), (-0.33, 0.22, -0.17))):
        objs.append(tubo(m, f"{nome}_ponta_{k}", [(-0.095, -0.135, 0.535), (-0.095 + dx * 0.5, -0.135 + dy * 0.4, 0.535 + dz * 0.3),
                                                  (-0.095 + dx, -0.135 + dy, 0.535 + dz)], [0.040, 0.038, 0.004], cor))
    return objs


def bandoleira(m, cor, nome="bandoleira"):
    """Faixa atravessada no tronco, do ombro esquerdo ao quadril direito."""
    return [faixa(m, nome, (0, -0.005, 0.400), 0.186, 0.170, 0.022, cor, n=12, tomba=-0.105)]


def cinto(m, cor, fivela="ouro", nome="cinto"):
    objs = [faixa(m, nome, (0, -0.010, 0.300), 0.186, 0.172, 0.022, cor, n=12)]
    objs += [bloco(m, nome + "_fivela", (0.060, 0.020, 0.050), (0, -0.186, 0.300), fivela, chanfro=0.006)]
    return objs


def capa(m, cor, nome="capa", comp=0.50, largura=0.56, queda=0.24, borda=None):
    """Capa ao vento: presa no pescoco, aberta em leque para tras. Fica quase deitada para aparecer
    na vista do jogo, que olha de cima. comp = quanto vai para tras; queda = quanto desce."""
    grade = []
    for i in range(5):
        t = i / 4
        w, yc, z = 0.16 + (largura - 0.16) * t ** 0.8, 0.135 + comp * t, 0.535 - queda * t
        # as bordas abracam os ombros em cima e ondulam embaixo
        grade.append([(w * s, yc - 0.12 * s * s * (1 - 0.7 * t) + 0.030 * t * math.cos(s * math.pi * 2),
                       z + 0.035 * t * math.cos(s * math.pi * 2)) for s in (-1, -0.5, 0, 0.5, 1)])
    objs = [folha(m, nome, grade, cor, esp=0.022)]
    if borda:
        objs.append(tubo(m, nome + "_borda", grade[-1], 0.022, borda))
    return objs


def tenis(m, cor, sola="branco", nome="tenis"):
    """Tenis sobre os pes do macaco, com a sola de outra cor."""
    objs = []
    for k, (x, y, z) in enumerate(PES):
        bm = poli.gaiola((0.086, 0.134, 0.056), cortes=1, cor=cor)
        poli.pintar([f for f in bm.faces if f.calc_center_median().z < -0.012], sola)
        for v in bm.verts:
            v.co.z = max(v.co.z, -0.044)
        poli.mover(bm, (x, y, z + 0.004))
        objs.append(m.obj(f"{nome}_{k}", bm, nivel=1))
    return objs


def aljava(m, corpo="marrom", penas=("vermelho", "amarelo", "vermelho"), detalhe="marrom_escuro", nome="aljava"):
    """Aljava atras do ombro direito, com as penas das flechas aparecendo por cima da cabeca."""
    base, direcao = Vector((0.150, 0.215, 0.360)), Vector((0.42, 0.10, 1)).normalized()
    objs = [cilindro(m, nome, base, direcao, [(0, 0), (0.060, 0.010), (0.070, 0.300), (0.078, 0.310), (0.078, 0.350), (0.060, 0.352), (0, 0.330)],
                     cores=[corpo, corpo, detalhe, detalhe, detalhe, "tinta"], seg=10)]
    lado = direcao.cross(Vector((0, 1, 0))).normalized()
    for k, cor in enumerate(penas):
        p = base + direcao * 0.345 + lado * (0.034 * (k - 1)) + Vector((0, 0.020 * (k % 2), 0))
        objs.append(cone(m, f"{nome}_pena_{k}", p, direcao, 0.030, 0.150 + 0.03 * (k % 2), cor, seg=4, fechado=True))
    return objs


# ---------------------------------------------------------------- armas
def dardo(m, nome, pos, direcao=(0, -1, 0), frente=0.20, tras=0.10, ponta="cinza", ponta_escala=1.0, pena_escala=1.0,
          penas=("vermelho", "amarelo"), haste="marrom_escuro"):
    """O dardo do piloto: haste, ponta de metal e penas cruzadas. Sai em 'pos', apontado para 'direcao'."""
    mat = em(pos, direcao)
    objs = [m.obj(nome + "_haste", poli.membro([(0, tras, 0), (0, -frente, 0)], [0.016, 0.016], cor=haste), nivel=1, matriz=mat)]
    pt = poli.torno([(0, 0), (0.044 * ponta_escala, 0.025), (0.030 * ponta_escala, 0.085), (0, 0.205 * (0.6 + 0.4 * ponta_escala))], seg=12, cor=ponta)
    objs.append(m.obj(nome + "_ponta", poli.orientar(pt, (0, -1, 0), (0, 0.015 - frente, 0)), nivel=0, vivo=True, matriz=mat))
    e = pena_escala
    for sufixo, tam, cor in (("_pena_v", (0.012, 0.115 * e, 0.105 * e), penas[0]), ("_pena_h", (0.105 * e, 0.115 * e, 0.012), penas[1])):
        bm = poli.caixa(tam, cor, chanfro=0.004, seg=1)
        for v in bm.verts:  # afina para a frente, em forma de pena
            if v.co.y < 0:
                v.co.x *= 0.25
                v.co.z *= 0.25
        poli.mover(bm, (0, tras - 0.025, 0))
        objs.append(m.obj(nome + sufixo, bm, nivel=0, vivo=True, matriz=mat))
    return objs


def besta(m, pos, direcao=(0, -1, 0), escala=1.0, madeira="marrom", arco_cor="marrom_escuro", detalhe="cinza", ponta="cinza",
          luneta=False, duplo=False, nome="besta"):
    """Besta: coronha, arco deitado, corda e o virote armado."""
    mat = em(pos, direcao, escala)
    objs = [bloco(m, nome + "_coronha", (0.056, 0.42, 0.060), (0, -0.10, 0), madeira, chanfro=0.014, matriz=mat)]
    objs.append(bloco(m, nome + "_trilho", (0.030, 0.30, 0.016), (0, -0.16, 0.036), detalhe, chanfro=0.004, matriz=mat))
    arcos = [(-0.285, 0.0)] + ([(-0.225, 0.045)] if duplo else [])
    for k, (y0, z0) in enumerate(arcos):
        pts = [(-0.27, y0 + 0.105, z0), (-0.16, y0 + 0.030, z0), (0, y0, z0), (0.16, y0 + 0.030, z0), (0.27, y0 + 0.105, z0)]
        objs.append(tubo(m, f"{nome}_arco_{k}", pts, [0.010, 0.024, 0.030, 0.024, 0.010], arco_cor, matriz=mat))
        objs.append(tubo(m, f"{nome}_corda_{k}", [(-0.262, y0 + 0.105, z0), (0, -0.030, z0 + 0.010), (0.262, y0 + 0.105, z0)], 0.007, "bege", nivel=0, matriz=mat))
    objs.append(tubo(m, nome + "_virote", [(0, -0.02, 0.052), (0, -0.36, 0.052)], 0.010, "bege", nivel=0, matriz=mat))
    pt = poli.torno([(0, 0), (0.034, 0.020), (0.022, 0.060), (0, 0.140)], seg=8, cor=ponta)
    objs.append(m.obj(nome + "_ponta", poli.orientar(pt, (0, -1, 0), (0, -0.345, 0.052)), nivel=0, vivo=True, matriz=mat))
    if luneta:
        lun = poli.torno([(0, 0), (0.030, 0.004), (0.030, 0.050), (0.022, 0.056), (0.022, 0.150), (0.034, 0.156), (0.034, 0.210), (0, 0.200)],
                         seg=10, cores=[detalhe, detalhe, "tinta", "tinta", detalhe, detalhe, "ciano"])
        objs.append(m.obj(nome + "_luneta", poli.orientar(lun, (0, -1, 0), (0, 0.06, 0.092)), nivel=0, vivo=True, matriz=mat))
    return objs
