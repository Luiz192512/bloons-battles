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


def toro(m, nome, centro, raio, tubo, cor, normal=(0, 0, 1), seg=14, lados=6, matriz=None):
    """Anel rigido (aro de oculos, halo, argola)."""
    perfil = [(raio + tubo * math.cos(2 * math.pi * k / lados), tubo * math.sin(2 * math.pi * k / lados)) for k in range(lados + 1)]
    bm = poli.torno(perfil, seg=seg, cor=cor)
    return m.obj(nome, poli.orientar(bm, normal, centro), nivel=0, matriz=matriz)


def esfera(m, nome, centro, raios, cor, cortes=1, nivel=1, matriz=None):
    if not isinstance(raios, (tuple, list)):
        raios = (raios, raios, raios)
    bm = poli.gaiola(raios, cortes=cortes, cor=cor)
    poli.mover(bm, centro)
    return m.obj(nome, bm, nivel=nivel, matriz=matriz)


def cone(m, nome, base, direcao, raio, comp, cor, seg=6, fechado=False, matriz=None):
    """Ponta viva: espinho, pena, chifre."""
    perfil = ([(0, 0)] if fechado else []) + [(raio, 0), (0, comp)]
    return m.obj(nome, poli.orientar(poli.torno(perfil, seg=seg, cor=cor), direcao, base), nivel=0, vivo=True, matriz=matriz)


def cilindro(m, nome, base, direcao, perfil, cor="cinza_escuro", cores=None, seg=12, matriz=None):
    """Peca torneada (perfil = [(raio, altura)]) apontada para a direcao dada."""
    return m.obj(nome, poli.orientar(poli.torno(perfil, seg=seg, cor=cor, cores=cores), direcao, base), nivel=0, vivo=True, matriz=matriz)


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
    """Casca que veste o cranio (lenco, capacete). piso(x, y) = altura minima, relativa ao centro
    da cabeca: e a barra da peca."""
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


def piso_capacete(x, y):
    return 0.105 - 0.20 * (y + 0.20)


# secoes do capuz, do rosto para a nuca: (y, meia largura, topo, barra de baixo)
CAPUZ = [(-0.205, 0.285, 0.975, 0.560), (-0.100, 0.365, 1.000, 0.545), (0.020, 0.425, 1.005, 0.535), (0.140, 0.400, 0.990, 0.530),
         (0.240, 0.300, 0.950, 0.550), (0.310, 0.160, 0.900, 0.620)]


def capuz(m, cor, barra=None, nome="capuz"):
    """Capuz de verdade: casca em volta da cabeca toda (cobre as orelhas e desce ate os ombros),
    aberta num oval em volta do rosto, com a borda enrolada e o bico caido para tras."""
    n = 12
    aneis = []
    for y, a, zt, zb in CAPUZ:
        zc, h = (zt + zb) / 2, (zt - zb) / 2
        aneis.append([(a * math.cos(2 * math.pi * k / n), y, zc + h * math.sin(2 * math.pi * k / n)) for k in range(n)])
    objs = [m.obj(nome, poli.loft(aneis, [cor] * n, fecha_inicio=False), nivel=1)]
    objs.append(tubo(m, nome + "_borda", aneis[0] + aneis[0][:1], 0.030, barra or cor))
    objs.append(tubo(m, nome + "_bico", [(0, 0.270, 0.900), (0, 0.420, 0.850), (0, 0.520, 0.700)], [0.090, 0.055, 0.004], cor))
    return objs


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


def capa(m, cor, nome="capa", comp=0.50, largura=0.56, queda=0.24, borda=None, gola=True):
    """Capa de pano: cai dos ombros, abre em leque para tras e faz pregas. Termina quase deitada
    para aparecer na vista do jogo, que olha de cima. comp = quanto vai para tras; queda = quanto desce."""
    grade, colunas = [], [k / 3 - 1 for k in range(7)]
    for i in range(5):
        t = i / 4
        w = 0.17 + (largura - 0.17) * t ** 0.7
        yc = 0.135 + comp * t ** 1.3                       # sai devagar do pescoco e so depois voa
        z = 0.535 - queda * math.sin(t * math.pi / 2)      # cai logo no comeco, como pano pesado
        linha = []
        for s in colunas:
            prega = 0.050 * t * math.cos(s * math.pi * 3)  # pregas que crescem para a barra
            linha.append((w * s, yc - 0.12 * s * s * (1 - 0.7 * t) + prega * 0.5, z + prega - 0.05 * t * s * s))
        grade.append(linha)
    objs = [folha(m, nome, grade, cor, esp=0.020)]
    if borda:
        objs.append(tubo(m, nome + "_borda", grade[-1], 0.020, borda))
    if gola:  # gola no pescoco e o broche que prende a capa
        objs.append(faixa(m, nome + "_gola", (0, -0.005, 0.548), 0.152, 0.134, 0.032, cor, n=12, inclina=-0.012))
        objs.append(esfera(m, nome + "_broche", (0, -0.150, 0.535), (0.040, 0.026, 0.040), borda or "ouro", cortes=0))
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
    base, direcao = Vector((0.170, 0.300, 0.360)), Vector((0.46, 0.14, 1)).normalized()
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


# ---------------------------------------------------------------- mais pecas de cabeca
def capacete(m, cor, barra, topo=None, crista=None, nome="capacete"):
    """Capacete redondo com barra; topo = cor de uma ponta no alto; crista = cor de uma fileira de laminas."""
    objs = [casca(m, nome, cor, piso_capacete)]
    objs.append(barra_casca(m, nome + "_barra", barra, piso_capacete))
    if topo:
        objs.append(cone(m, nome + "_ponta", (0, 0.0, 0.985), (0, 0.1, 1), 0.050, 0.150, topo, seg=6))
    if crista:
        for k in range(4):
            y = -0.150 + k * 0.105
            objs.append(cone(m, f"{nome}_crista_{k}", (0, y, 0.972 - 0.06 * abs(k - 1.3) ** 1.5), (0, 0.35, 1), 0.052, 0.170 - 0.02 * abs(k - 1.5), crista, seg=4))
    return objs


def chapeu_aba(m, cor, fita=None, aba=0.37, copa=0.15, raio=0.235, z=0.880, tomba=0.10, nome="chapeu"):
    """Chapeu de aba larga com copa baixa, um pouco tombado para tras."""
    perfil = [(0, 0), (aba, 0.004), (aba, 0.026), (raio + 0.012, 0.034), (raio, 0.070), (raio * 0.92, copa), (raio * 0.60, copa + 0.030), (0, copa + 0.034)]
    cores = [cor, cor, cor, fita or cor, cor, cor, cor]
    return [cilindro(m, nome, (0, 0.010 + tomba * 0.12, z - tomba * 0.03), (0, tomba, 1), perfil, cores=cores, seg=14)]


def chapeu_cone(m, cor, fita=None, aba=0.34, altura=0.42, raio=0.225, z=0.885, tomba=0.30, nome="chapeu"):
    """Chapeu pontudo de aba (mago, bruxo), tombado para tras para o rosto aparecer de cima."""
    c = Vector((0, 0.010 + tomba * 0.12, z - tomba * 0.03))
    eixo = Vector((0, tomba, 1)).normalized()
    objs = [cilindro(m, nome + "_aba", c, eixo, [(0, 0), (aba, 0.004), (aba, 0.024), (raio, 0.034), (raio, 0.075), (0, 0.075)],
                     cores=[cor, cor, cor, fita or cor, cor], seg=14)]
    objs.append(tubo(m, nome + "_copa", [c + eixo * 0.060, c + eixo * altura * 0.55 + Vector((0, 0.030, 0)), c + eixo * altura + Vector((0, 0.130, -0.020))], [raio * 0.80, raio * 0.42, 0.004], cor))
    return objs


def coroa(m, cor="ouro", joia="vermelho", raio=0.200, z=0.915, pontas=5, nome="coroa"):
    """Coroa: aro com pontas e uma joia na frente."""
    objs = [cilindro(m, nome, (0, 0, z), (0, 0, 1), [(raio * 0.94, 0), (raio, 0.004), (raio * 1.04, 0.070), (raio * 0.92, 0.070), (raio * 0.90, 0.004)], cor=cor, seg=12)]
    for k in range(pontas):
        a = 2 * math.pi * (k + 0.5) / pontas - math.pi / 2
        objs.append(cone(m, f"{nome}_ponta_{k}", (raio * 0.98 * math.cos(a), raio * 0.98 * math.sin(a), z + 0.060), (0.2 * math.cos(a), 0.2 * math.sin(a), 1), 0.045, 0.110, cor, seg=4))
    objs.append(esfera(m, nome + "_joia", (0, -raio * 1.02, z + 0.038), 0.030, joia, cortes=0))
    return objs


def viseira(m, cor="ciano", aro="cinza_escuro", nome="viseira"):
    """Viseira inteirica na frente dos olhos."""
    objs = [tubo(m, nome, arco((0, -0.030, 0.795), 0.215, 0.215, 215, 325, n=6), 0.050, cor)]
    objs.append(tubo(m, nome + "_tira", arco((0, 0.0, 0.795), 0.272, 0.240, -30, 210, n=8), 0.016, aro))
    return objs


def monoculo(m, cor="ciano", aro="cinza", lado=1, nome="monoculo"):
    """Olho mecanico: lente acesa com aro, sobre um dos olhos."""
    x, y, z = OLHOS[0 if lado > 0 else 1]
    objs = [cilindro(m, nome, (x + 0.004 * lado, y - 0.030, z), (0.10 * lado, -1, 0), [(0, 0), (0.085, 0.002), (0.085, 0.030), (0.060, 0.034), (0, 0.040)],
                     cores=[aro, aro, aro, cor], seg=10)]
    objs.append(tubo(m, nome + "_tira", arco((0, 0.0, 0.800), 0.272, 0.240, -20, 200, n=8), 0.013, aro))
    return objs


# ---------------------------------------------------------------- mais pecas de corpo
BRACO_DIR = [macaco_poli.OMBRO, (0.245, -0.040, 0.405), (0.275, -0.125, 0.412)]
BRACO_ESQ = [(-0.150, 0.0, 0.475), (-0.235, -0.010, 0.375), (-0.262, -0.035, 0.265)]


def manga(m, lado, cor, luva=None, anel=None, ombreira=None, nome="manga"):
    """Manga (ou braco de metal) sobre o braco: lado 1 = direito (o do ataque), -1 = esquerdo."""
    pts = BRACO_DIR if lado > 0 else BRACO_ESQ
    mao = MAO if lado > 0 else MAO_ESQ
    nome = f"{nome}_{'d' if lado > 0 else 'e'}"
    objs = [tubo(m, nome, pts, [0.072, 0.064, 0.060], cor)]
    if luva:
        objs.append(esfera(m, nome + "_luva", mao, 0.078, luva))
    if anel:
        a, b = Vector(pts[1]), Vector(pts[2])
        objs.append(toro(m, nome + "_anel", a + (b - a) * 0.45, 0.064, 0.016, anel, normal=b - a, seg=10, lados=4))
    if ombreira:
        objs.append(esfera(m, nome + "_ombreira", Vector(pts[0]) + Vector((0.035 * lado, 0, 0.030)), (0.095, 0.085, 0.075), ombreira))
    return objs


def colete(m, cor, barra=None, emblema=None, z0=0.255, z1=0.520, folga=1.0, nome="colete"):
    """Colete (ou peitoral) em volta do tronco: barril cortado em cima e embaixo."""
    bm = poli.gaiola((0.200 * folga, 0.182 * folga, 0.230), cortes=2, cor=cor)
    for v in bm.verts:
        v.co.z = min(max(v.co.z, z0 - 0.385), z1 - 0.385)
    poli.mover(bm, (0, -0.005, 0.385))
    objs = [m.obj(nome, bm, nivel=1)]
    if barra:
        objs.append(faixa(m, nome + "_barra", (0, -0.008, z0 + 0.030), 0.196 * folga, 0.180 * folga, 0.022, barra, n=12))
    if emblema:
        objs.append(esfera(m, nome + "_emblema", (0, -0.176 * folga, 0.420), (0.050, 0.024, 0.050), emblema, cortes=0))
    return objs


def saia(m, cor, barra=None, z0=0.050, z1=0.330, largura=0.300, nome="saia"):
    """Barra de tunica ou manto: cone aberto da cintura ate perto do chao."""
    perfil = [(0.150, z1), (0.190, z1 - 0.030), (largura * 0.85, (z0 + z1) / 2), (largura, z0 + 0.030), (largura * 0.96, z0), (0, z0)]
    cores = [cor, cor, cor, barra or cor, cor]
    bm = poli.torno(perfil, seg=12, cores=cores)
    for v in bm.verts:
        v.co.y *= 0.90
    return [m.obj(nome, bm, nivel=0)]


def mochila(m, cor, detalhe="cinza_escuro", luz=None, tam=1.0, nome="mochila"):
    """Mochila nas costas com dois bocais para baixo; luz = cor do brilho dos bocais."""
    c = Vector((0, 0.215, 0.410))
    objs = [bloco(m, nome, (0.250 * tam, 0.130 * tam, 0.240 * tam), c, cor, chanfro=0.030, seg=2)]
    for sx in (-1, 1):
        b = c + Vector((0.075 * tam * sx, 0.030 * tam, -0.110 * tam))
        objs.append(cilindro(m, f"{nome}_bocal_{sx}", b, (0, 0.25, -1), [(0, 0), (0.040 * tam, 0.002), (0.058 * tam, 0.090 * tam), (0.040 * tam, 0.092 * tam), (0, 0.060 * tam)],
                             cores=[detalhe, detalhe, detalhe, luz or detalhe], seg=8))
        if luz:
            objs.append(cone(m, f"{nome}_jato_{sx}", b + Vector((0, 0.022, -0.085)) * tam, (0, 0.30, -1), 0.040 * tam, 0.150 * tam, luz, seg=5))
    return objs


def tanque(m, cor, tampa="cinza", lado=0, raio=0.085, altura=0.300, nome="tanque"):
    """Tanque cilindrico nas costas (cola, pocao, combustivel)."""
    base = (0.090 * lado, 0.215, 0.290)
    perfil = [(0, 0), (raio * 0.8, 0.004), (raio, 0.030), (raio, altura - 0.040), (raio * 0.75, altura), (raio * 0.40, altura + 0.010), (raio * 0.40, altura + 0.050), (0, altura + 0.050)]
    return [cilindro(m, nome, base, (0, 0.12, 1), perfil, cores=[cor, cor, cor, cor, tampa, tampa, tampa], seg=10)]


# ---------------------------------------------------------------- bumerangues e laminas
def bumerangue(m, nome, pos, direcao=(0, -1, 0), escala=1.0, cor="bege", pontas="laranja", gume=None):
    """Bumerangue em V, deitado, com as pontas pintadas; gume = cor de um fio de metal na borda de fora."""
    mat = em(pos, direcao, escala) @ Matrix.Diagonal((1, 1, 0.5, 1))
    v = [(-0.170, 0.075, 0), (-0.090, -0.035, 0), (0, -0.095, 0), (0.090, -0.035, 0), (0.170, 0.075, 0)]
    objs = [tubo(m, nome, v, [0.026, 0.040, 0.046, 0.040, 0.026], cor, matriz=mat)]
    for k, p in enumerate((v[0], v[-1])):
        objs.append(esfera(m, f"{nome}_ponta_{k}", p, (0.050, 0.050, 0.060), pontas, cortes=0, matriz=mat))
    if gume:
        fio = [(x * 1.04, y - 0.034, 0) for x, y, _z in v]
        objs.append(tubo(m, nome + "_gume", fio, [0.010, 0.018, 0.020, 0.018, 0.010], gume, matriz=mat))
    return objs


def glaive(m, nome, pos, raio=0.130, normal=(0, 0, 1), lamina="aco", miolo="vermelho", laminas=6):
    """Glaive: aro de metal com laminas em redemoinho e miolo colorido."""
    mat = em(pos, normal, de=(0, 0, 1))
    objs = [toro(m, nome, (0, 0, 0), raio, raio * 0.17, lamina, seg=12, lados=4, matriz=mat)]
    objs.append(esfera(m, nome + "_miolo", (0, 0, 0), (raio * 0.45, raio * 0.45, raio * 0.22), miolo, cortes=0, matriz=mat))
    objs.append(tubo(m, nome + "_raio", [(-raio, 0, 0), (raio, 0, 0)], raio * 0.10, lamina, nivel=0, matriz=mat))
    for k in range(laminas):
        a = 2 * math.pi * k / laminas
        objs.append(cone(m, f"{nome}_lamina_{k}", (raio * math.cos(a), raio * math.sin(a), 0), (math.cos(a + 0.9), math.sin(a + 0.9), 0),
                         raio * 0.24, raio * 0.70, lamina, seg=4, matriz=mat))
    return objs


# ---------------------------------------------------------------- cajados, frascos e magia
def cajado(m, cor="marrom", orbe="azul", raio=0.070, altura=0.960, garra=None, nome="cajado"):
    """Cajado em pe na mao do ataque, com um orbe no alto; garra = cor de tres pontas que seguram o orbe.
    Devolve (pecas, topo), com topo = centro do orbe."""
    x, y = MAO[0] + 0.010, MAO[1] - 0.030
    topo = Vector((x, y, altura + raio * 0.6))
    objs = [tubo(m, nome, [(x, y, 0.020), (x, y, altura * 0.5), (x, y, altura)], [0.022, 0.020, 0.026], cor)]
    objs.append(esfera(m, nome + "_orbe", topo, raio, orbe, cortes=1))
    if garra:
        for k in range(3):
            a = 2 * math.pi * k / 3 + 0.5
            d = Vector((math.cos(a), math.sin(a), 0))
            objs.append(cone(m, f"{nome}_garra_{k}", topo + d * raio * 0.95 - Vector((0, 0, raio * 0.7)), d * 0.25 + Vector((0, 0, 1)), raio * 0.32, raio * 1.9, garra, seg=4))
    return objs, topo


def frasco(m, nome, pos, liquido, escala=1.0, vidro="aco", rolha="marrom"):
    """Frasco de pocao: bojo redondo com o liquido colorido, gargalo e rolha."""
    perfil = [(0, 0), (0.050, 0.008), (0.072, 0.050), (0.060, 0.100), (0.026, 0.130), (0.026, 0.180), (0.036, 0.184), (0.036, 0.200), (0, 0.204)]
    perfil = [(r * escala, h * escala) for r, h in perfil]
    return cilindro(m, nome, pos, (0, 0, 1), perfil, cores=[liquido, liquido, liquido, vidro, vidro, rolha, rolha, rolha], seg=10)


def chama(m, nome, pos, altura, cores=("laranja", "amarelo"), raio=None):
    """Chama: uma lingua grande e duas menores."""
    r = raio or altura * 0.38
    objs = [cone(m, nome, pos, (0, 0, 1), r, altura, cores[0], seg=5, fechado=True)]
    for k, dx in enumerate((-1, 1)):
        objs.append(cone(m, f"{nome}_{k}", Vector(pos) + Vector((dx * r * 0.55, 0, 0)), (dx * 0.35, 0, 1), r * 0.60, altura * 0.62, cores[1], seg=4, fechado=True))
    return objs


def chifres(m, cor, comp=0.200, abre=0.9, nome="chifre"):
    """Dois chifres curvos no alto da cabeca."""
    objs = []
    for sx in (-1, 1):
        objs.append(tubo(m, f"{nome}_{sx}", [(0.150 * sx, 0.0, 0.900), ((0.150 + comp * 0.6 * abre) * sx, 0.010, 0.900 + comp * 0.55), ((0.150 + comp * 0.7 * abre) * sx, 0.0, 0.900 + comp * 1.2)],
                         [0.050, 0.036, 0.004], cor))
    return objs


def aro_chao(m, nome, raio, cor, pontas=None, n=8, tubo_r=0.028, altura=0.150):
    """Aro no chao em volta da torre (aura), com pontas opcionais de outra cor."""
    objs = [toro(m, nome, (0, 0, 0.024), raio, tubo_r, cor, seg=20, lados=4)]
    if pontas:
        for k in range(n):
            a = 2 * math.pi * (k + 0.5) / n
            d = Vector((math.cos(a), math.sin(a), 0))
            objs.append(cone(m, f"{nome}_ponta_{k}", d * raio, d * 0.4 + Vector((0, 0, 1)), 0.045, altura, pontas, seg=4, fechado=True))
    return objs
