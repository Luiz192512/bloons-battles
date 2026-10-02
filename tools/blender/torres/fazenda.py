"""Fazenda de Bananas: construcao sem macaco. Canteiro de terra com bananeiras (tudo na malha base).

Caminho 1 (producao): mais bananeiras; do tier 3 em diante, plantacao, centro de pesquisa e central.
Caminho 2 (banco): cesto e bananas douradas; do tier 3 em diante, o banco.
Caminho 3 (mercado): caixote de coleta e cerca; do tier 3 em diante, barraca, mercado e o predio.

Tudo sai de _fazenda, que le os tres tiers (regra da torre) e vale em qualquer combinacao.
"""
import math

from mathutils import Vector

import pecas

ENQUADRE = ((0, 0.0, 0.35), 1.9)


def _bananeira(m, nome, pos, altura=0.420, banana="amarelo"):
    p = Vector(pos)
    topo = p + Vector((0, 0, altura))
    objs = [pecas.tubo(m, nome, [p, p + Vector((0.010, 0, altura * 0.5)), topo], [0.036, 0.030, 0.024], "marrom")]
    for k in range(5):
        a = math.radians(72 * k + 15)
        d = Vector((math.cos(a), math.sin(a), 0.25))
        objs.append(pecas.cone(m, f"{nome}_folha_{k}", topo - d * 0.020, d, 0.062, 0.240, "verde", seg=4, fechado=True))
    for k in range(3):
        a = math.radians(120 * k + 60)
        objs.append(pecas.cone(m, f"{nome}_banana_{k}", topo + Vector((0.040 * math.cos(a), 0.040 * math.sin(a), -0.050)), (math.cos(a) * 0.5, math.sin(a) * 0.5, -1), 0.026, 0.110, banana, seg=4, fechado=True))
    return objs


def _predio(m, nome, pos, tam, cor, teto, domo=None, colunas=None):
    x, y = pos
    lx, ly, lz = tam
    objs = [pecas.bloco(m, nome, tam, (x, y, lz / 2 + 0.030), cor, chanfro=0.020),
            pecas.bloco(m, nome + "_teto", (lx * 1.10, ly * 1.10, 0.050), (x, y, lz + 0.050), teto, chanfro=0.014),
            pecas.bloco(m, nome + "_porta", (lx * 0.22, 0.020, lz * 0.42), (x, y - ly / 2 - 0.004, lz * 0.21 + 0.030), "marrom_escuro", chanfro=0.006)]
    if domo:
        objs.append(pecas.esfera(m, nome + "_domo", (x, y, lz + 0.075), (lx * 0.30, ly * 0.30, lx * 0.26), domo, cortes=1))
    if colunas:
        for k in range(4):
            cx = x + lx * (k / 3 - 0.5) * 0.86
            objs.append(pecas.tubo(m, f"{nome}_coluna_{k}", [(cx, y - ly / 2 - 0.030, 0.030), (cx, y - ly / 2 - 0.030, lz + 0.030)], 0.022, colunas, nivel=0))
    return objs


def _fazenda(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    prod, banco, merc = (p == 0 and t1 >= 3), (p == 1 and t2 >= 3), (p == 2 and t3 >= 3)
    banana = "ouro" if t2 >= 2 else "amarelo"
    b = [pecas.cilindro(m, "canteiro", (0, 0, 0), (0, 0, 1), [(0, 0), (0.640, 0.004), (0.640, 0.030), (0.560, 0.050), (0, 0.056)], cores=["verde_escuro", "verde_escuro", "marrom", "marrom"], seg=18)]
    # bananeiras: 2 na base; 3 e 4 com os tiers 1 e 2 do caminho 1; a plantacao tem 6
    n = {0: 2, 1: 3, 2: 4}.get(t1, 4)
    lugares = [(-0.300, -0.140), (0.300, -0.140), (0.0, -0.360), (0.0, 0.300), (-0.380, 0.220), (0.380, 0.220)]
    if prod:
        n = 6 if t1 == 3 else 4
    if banco or merc or (prod and t1 >= 4):   # o predio ocupa o fundo: as arvores ficam na frente
        lugares = [(-0.380, -0.200), (0.380, -0.200), (-0.140, -0.400), (0.140, -0.400), (-0.500, 0.060), (0.500, 0.060)]
    for k in range(n):
        b += _bananeira(m, f"bananeira_{k}", (lugares[k][0], lugares[k][1], 0.040), altura=0.420 if not prod else 0.500, banana=banana)
    if prod and t1 >= 4:   # centro de pesquisa e central: laboratorio com domo e caixas
        b += _predio(m, "laboratorio", (0, 0.200), (0.520, 0.380, 0.300 if t1 == 4 else 0.420), "branco", "ciano" if t1 == 4 else "ouro", domo="ciano")
        for k, (x, y) in enumerate(((-0.050, -0.140), (0.110, -0.170), (0.030, -0.150))[:2 if t1 == 4 else 3]):
            b.append(pecas.bloco(m, f"caixa_{k}", (0.130, 0.130, 0.120), (x, y, 0.110 + 0.120 * (1 if k == 2 else 0)), "amarelo", chanfro=0.014))
        if t1 == 5:
            b.append(pecas.cilindro(m, "chamine", (0.180, 0.300, 0.450), (0, 0, 1), [(0.050, 0), (0.042, 0.220), (0.052, 0.230), (0.052, 0.260), (0, 0.260)], cores=["cinza", "vermelho", "vermelho", "tinta"], seg=8))
    if banco:
        alt = {3: 0.300, 4: 0.380, 5: 0.520}[t2]
        b += _predio(m, "banco", (0, 0.180), (0.560, 0.400, alt), "bege", "azul" if t2 < 5 else "ouro", domo="ouro" if t2 >= 4 else None, colunas="branco")
        if t2 == 5:   # moeda grande no alto
            b.append(pecas.cilindro(m, "moeda", (0, 0.160, alt + 0.300), (0, -1, 0.15), [(0, 0), (0.130, 0.004), (0.130, 0.034), (0, 0.038)], cor="ouro", seg=14))
    if merc:
        if t3 == 5:   # predio alto de vidro
            b += _predio(m, "predio", (0, 0.200), (0.400, 0.340, 0.780), "azul", "cinza", colunas=None)
            for k in range(3):
                b.append(pecas.bloco(m, f"predio_faixa_{k}", (0.410, 0.350, 0.030), (0, 0.200, 0.220 + 0.200 * k), "ciano", chanfro=0.006))
            b.append(pecas.cone(m, "predio_antena", (0, 0.200, 0.860), (0, 0, 1), 0.030, 0.260, "ouro", seg=5))
        for k in range({3: 1, 4: 2, 5: 2}[t3]):   # barracas de toldo listrado
            x = 0.0 if t3 == 3 else (-0.330 + 0.660 * k)
            y = 0.180 if t3 < 5 else -0.060
            b.append(pecas.bloco(m, f"barraca_{k}", (0.340, 0.220, 0.160), (x, y, 0.120), "marrom", chanfro=0.016))
            for j in range(4):
                b.append(pecas.bloco(m, f"toldo_{k}_{j}", (0.090, 0.300, 0.030), (x - 0.135 + 0.090 * j, y - 0.020, 0.330), "vermelho" if j % 2 == 0 else "branco", chanfro=0.008))
            for sx in (-1, 1):
                b.append(pecas.tubo(m, f"barraca_haste_{k}_{sx}", [(x + 0.160 * sx, y - 0.110, 0.040), (x + 0.160 * sx, y - 0.110, 0.320)], 0.012, "marrom_escuro", nivel=0))
    # caminho 2, tiers 1 e 2: cesto de bananas na frente
    if t2 >= 1 and not banco:
        b.append(pecas.cilindro(m, "cesto", (0.180, -0.480, 0.040), (0, 0, 1), [(0, 0), (0.070, 0.004), (0.100, 0.100), (0.085, 0.100), (0.070, 0.050), (0, 0.050)], cor="marrom_escuro", seg=10))
        for k in range(3):
            a = math.radians(120 * k)
            b.append(pecas.cone(m, f"cesto_banana_{k}", (0.180 + 0.030 * math.cos(a), -0.480 + 0.030 * math.sin(a), 0.100), (math.cos(a) * 0.6, math.sin(a) * 0.6, 1), 0.026, 0.110, banana, seg=4, fechado=True))
    # caminho 3, tiers 1 e 2: caixote de coleta e cerca branca
    if t3 >= 1 and not merc:
        b.append(pecas.bloco(m, "caixote", (0.170, 0.150, 0.130), (-0.200, -0.480, 0.105), "laranja", chanfro=0.014))
    if t3 >= 2:
        for k in range(12):
            a = math.radians(30 * k + 15)
            b.append(pecas.bloco(m, f"cerca_{k}", (0.036, 0.036, 0.150), (0.600 * math.cos(a), 0.600 * math.sin(a), 0.100), "branco", chanfro=0.008))
        b.append(pecas.toro(m, "cerca_travessa", (0, 0, 0.130), 0.600, 0.014, "branco", seg=18, lados=4))
    m.por("base", b)
    m.pivo("base", (0, 0, 0))


def base(m):
    _fazenda(m)


PARAMETRO = [(None, None)]   # o tier muda a fazenda, que le m.tier
CAMINHOS = [{1: PARAMETRO, 2: PARAMETRO, 3: _fazenda, 4: _fazenda, 5: _fazenda} for _ in range(3)]
