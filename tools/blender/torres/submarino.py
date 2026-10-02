"""Submarino: so o submarino, sem macaco. Casco com helice e lemes (base) e a torre com periscopio e canhoes (torreta).

Caminho 1 (sensores): periscopio alto e radar; do tier 3 em diante, aneis de sonar e reator.
Caminho 2 (misseis): ponta farpada e aquecida na proa; do tier 3 em diante, tubos de missil no conves.
Caminho 3 (canhoes): dois e tres canhoes na torre; depois canhoes longos, blindagem e insignia.

Tudo sai de _montar, que le os tres tiers (regra da torre) e vale em qualquer combinacao.
"""
import math

from mathutils import Matrix, Vector

import pecas
import poli

ENQUADRE = ((0, 0.0, 0.22), 1.7)
TORRE = Vector((0, 0.060, 0.270))


def _casco(m, cor, faixa, quilha):
    perfil = [(0, 0), (0.090, 0.020), (0.180, 0.120), (0.225, 0.300), (0.235, 0.520), (0.235, 0.600), (0.215, 0.800), (0.150, 0.980), (0.060, 1.090), (0, 1.110)]
    cores = [cor, cor, cor, faixa, cor, faixa, cor, cor, cor]
    bm = poli.torno(perfil, seg=16, cores=cores)
    for f in bm.faces:   # a barriga do casco tem outra cor
        c = f.calc_center_median()
        if c.x * 0 + c.y < -0.090 and f.material_index == poli.INDICE[cor]:
            f.material_index = poli.INDICE[quilha]
    poli.orientar(bm, (0, -1, 0), (0, 0.555, 0.170))
    return m.obj("casco", bm, nivel=0, matriz=Matrix.Translation((0, 0, 0.170)) @ Matrix.Diagonal((1, 1, 0.82, 1)) @ Matrix.Translation((0, 0, -0.170)))


def _montar(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    cor, faixa, quilha, metal = "amarelo", "azul", "laranja", "cinza"
    if p == 0 and t1 >= 4:
        cor, faixa, quilha, metal = ("cinza_escuro", "verde", "tinta", "cinza") if t1 == 4 else ("tinta", "ciano", "tinta", "aco")
    elif p == 1 and t2 >= 4:
        cor, faixa, quilha, metal = ("verde_escuro", "amarelo", "tinta", "cinza") if t2 == 4 else ("tinta", "vermelho", "tinta", "cinza")
    elif p == 2 and t3 >= 4:
        cor, faixa, quilha, metal = ("cinza", "azul", "cinza_escuro", "aco") if t3 == 4 else ("azul", "ouro", "tinta", "ouro")

    b = [_casco(m, cor, faixa, quilha)]
    for k in range(3):   # helice
        a = math.radians(120 * k + 30)
        b.append(pecas.cone(m, f"helice_{k}", (0, 0.585, 0.170), (math.cos(a), 0.25, math.sin(a)), 0.045, 0.140, metal, seg=4, fechado=True))
    b.append(pecas.bloco(m, "leme", (0.024, 0.150, 0.200), (0, 0.470, 0.300), faixa, chanfro=0.010))
    for sx in (-1, 1):
        b.append(pecas.bloco(m, f"aleta_{sx}", (0.180, 0.130, 0.024), (0.200 * sx, 0.440, 0.170), faixa, chanfro=0.010))
    # caminho 2: ponta na proa (farpada, depois aquecida) e tubos de missil no conves da frente
    if t2 >= 1:
        ponta = "laranja" if t2 >= 2 else "aco"
        b.append(pecas.cone(m, "proa_ponta", (0, -0.530, 0.170), (0, -1, 0), 0.070, 0.230, ponta, seg=6))
        for sx in (-1, 1):
            b.append(pecas.cone(m, f"proa_farpa_{sx}", (0.030 * sx, -0.600, 0.170), (0.9 * sx, 0.8, 0), 0.030, 0.110, ponta, seg=4))
    if t2 >= 3:
        n = {3: 2, 4: 4, 5: 6}[t2]
        for k in range(n):
            sx = -1 if k % 2 == 0 else 1
            y = -0.300 + 0.120 * (k // 2)
            b.append(pecas.cilindro(m, f"missil_{k}", (0.085 * sx, y, 0.250), (0.25 * sx, 0, 1), [(0.052, 0), (0.052, 0.090), (0.040, 0.094), (0.040, 0.150), (0, 0.240)],
                                    cores=["cinza_escuro", "cinza_escuro", "branco", "vermelho"], seg=8))
    # caminho 1: aneis de sonar e reator
    if t1 >= 3:
        for k, y in enumerate((-0.250, 0.300)):
            b.append(pecas.toro(m, f"sonar_{k}", (0, y, 0.170), 0.238, 0.022, "ciano" if t1 != 4 else "verde", normal=(0, 1, 0), seg=16, lados=4,
                                matriz=Matrix.Translation((0, 0, 0.170)) @ Matrix.Diagonal((1, 1, 0.82, 1)) @ Matrix.Translation((0, 0, -0.170))))
    if t1 >= 4:
        luz = "verde" if t1 == 4 else "ciano"
        b.append(pecas.esfera(m, "reator", (0, 0.300, 0.330), (0.110, 0.130, 0.090) if t1 == 4 else (0.140, 0.160, 0.120), luz))
        b.append(pecas.toro(m, "reator_aro", (0, 0.300, 0.335), 0.120 if t1 == 4 else 0.150, 0.020, metal, seg=12, lados=4))
    if p == 2 and t3 >= 4:   # blindagem: placas no conves
        for sx in (-1, 1):
            b.append(pecas.bloco(m, f"placa_{sx}", (0.050, 0.420, 0.070), (0.150 * sx, -0.130, 0.290), metal, chanfro=0.014))
    m.por("base", b)
    m.pivo("base", (0, 0, 0))

    # torre com periscopio, radar e canhoes
    t = [m.obj("torre", poli.orientar(poli.torno([(0.115, 0), (0.105, 0.150), (0.085, 0.180), (0, 0.185)], seg=12, cores=[cor, faixa, faixa]), (0, 0, 1), TORRE),
               nivel=0, matriz=Matrix.Translation(TORRE) @ Matrix.Diagonal((1, 1.45, 1, 1)) @ Matrix.Translation(-TORRE))]
    alto = 0.200 + (0.130 if t1 >= 1 else 0)
    px, py, pz = TORRE + Vector((0, 0.070, 0.170))
    t.append(pecas.tubo(m, "periscopio", [(px, py, pz), (px, py, pz + alto), (px, py - 0.070, pz + alto + 0.015)], 0.018, metal, nivel=0))
    t.append(pecas.esfera(m, "periscopio_lente", (px, py - 0.085, pz + alto + 0.015), 0.028, "ciano", cortes=0))
    if t1 >= 2:   # radar
        t.append(pecas.cilindro(m, "radar", TORRE + Vector((0, -0.060, 0.260)), (0, -0.5, 1), [(0, 0), (0.110, 0.050), (0.100, 0.060), (0, 0.020)], cores=[metal, "branco", "branco"], seg=10))
        t.append(pecas.tubo(m, "radar_haste", [TORRE + Vector((0, -0.060, 0.170)), TORRE + Vector((0, -0.060, 0.265))], 0.014, metal, nivel=0))
    n = 1 if t3 == 0 else (2 if t3 <= 2 else 3)
    L = 0.260 + (0.090 if (p == 2 and t3 >= 4) else 0)
    cano = "aco" if (p == 2 and t3 >= 4) else "cinza_escuro"
    for k in range(n):
        x = (k - (n - 1) / 2) * 0.085
        t.append(pecas.cilindro(m, f"canhao_{k}", TORRE + Vector((x, -0.110, 0.085)), (0, -1, 0.06), [(0.030, 0), (0.030, L - 0.040), (0.042, L - 0.036), (0.042, L), (0.020, L), (0.018, L * 0.5)],
                                cores=[cano, cano, cano, "tinta", "tinta"], seg=8))
        if t3 >= 2:   # explosao aerea: ponta amarela dividida
            t.append(pecas.cone(m, f"canhao_ponta_{k}", TORRE + Vector((x, -0.110 - L, 0.100)), (0, -1, 0.06), 0.024, 0.080, "amarelo", seg=4))
    if p == 2 and t3 == 5:   # comandante: insignia dourada na torre
        t.append(pecas.estrela(m, "insignia", TORRE + Vector((0, -0.020, 0.200)), 0.095, "ouro", normal=(0, -0.2, 1)))
    m.por("torreta", t)
    m.pivo("torreta", TORRE)


def base(m):
    _montar(m)


PARAMETRO = [(None, None)]   # o tier muda o submarino, que le m.tier
CAMINHOS = [{1: PARAMETRO, 2: PARAMETRO, 3: _montar, 4: _montar, 5: _montar} for _ in range(3)]
