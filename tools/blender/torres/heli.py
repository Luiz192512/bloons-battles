"""Piloto de Helicoptero: so o helicoptero, sem macaco. Cabine, cauda e esquis (base) e o rotor (torreta, gira no jogo).

Caminho 1 (ataque): mais canhoes e farol; do tier 3 em diante, rotor de laminas e helicoptero de ataque.
Caminho 2 (apoio): jatos e radar; do tier 3 em diante, rotor largo e helicoptero de dois rotores com carga.
Caminho 3 (comanche): canhoes longos e tambor de municao; depois ariete, casco furtivo e escolta.

Tudo sai de _heli, que le os tres tiers (regra da torre) e vale em qualquer combinacao.
"""
import math

from mathutils import Matrix, Vector

import pecas

ENQUADRE = ((0, 0.10, 0.35), 2.0)


def _rotor(m, nome, centro, raio, pas, cor, ponta, larg=0.075):
    objs = [pecas.cilindro(m, nome + "_cubo", Vector(centro) - Vector((0, 0, 0.080)), (0, 0, 1), [(0.030, 0), (0.030, 0.070), (0.060, 0.075), (0.060, 0.100), (0, 0.110)], cor="cinza_escuro", seg=8)]
    for k in range(pas):
        rot = Matrix.Translation(Vector(centro)) @ Matrix.Rotation(2 * math.pi * k / pas + 0.4, 4, "Z")
        objs.append(pecas.bloco(m, f"{nome}_pa_{k}", (raio * 0.78, larg, 0.014), (raio * 0.45, 0, 0.010), cor, chanfro=0.006, matriz=rot))
        objs.append(pecas.bloco(m, f"{nome}_ponta_{k}", (raio * 0.20, larg, 0.016), (raio * 0.92, 0, 0.010), ponta, chanfro=0.006, matriz=rot))
    return objs


def _mini(m, nome, pos, cor):
    """Helicoptero pequeno de escolta."""
    mat = Matrix.Translation(Vector(pos)) @ Matrix.Scale(0.42, 4)
    objs = [pecas.esfera(m, nome, (0, -0.05, 0), (0.200, 0.290, 0.190), cor, matriz=mat),
            pecas.tubo(m, nome + "_cauda", [(0, 0.150, 0.030), (0, 0.700, 0.080)], [0.065, 0.030], cor, nivel=0, matriz=mat)]
    for k in range(2):
        rot = mat @ Matrix.Translation((0, 0, 0.260)) @ Matrix.Rotation(math.pi / 2 * k + 0.5, 4, "Z")
        objs.append(pecas.bloco(m, f"{nome}_pa_{k}", (1.000, 0.090, 0.020), (0, 0, 0), "cinza_escuro", chanfro=0.008, matriz=rot))
    return objs


def _heli(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    ataque, apoio, coman = (p == 0 and t1 >= 3), (p == 1 and t2 >= 3), (p == 2 and t3 >= 3)
    cor, det, vidro = "verde", "amarelo", "ciano"
    if ataque:
        cor, det = {3: ("verde", "vermelho"), 4: ("cinza_escuro", "vermelho"), 5: ("tinta", "ciano")}[t1]
    elif apoio:
        cor, det = {3: ("verde", "branco"), 4: ("verde_escuro", "amarelo"), 5: ("tinta", "ouro")}[t2]
    elif coman:
        cor, det = {3: ("verde", "aco"), 4: ("azul", "cinza"), 5: ("azul", "ouro")}[t3]
    duplo = apoio and t2 >= 4   # dois rotores, corpo comprido
    z = 0.350
    b = []
    if duplo:
        b.append(pecas.esfera(m, "cabine", (0, 0.050, z), (0.230, 0.640, 0.200), cor, cortes=2))
        b.append(pecas.esfera(m, "torre_tras", (0, 0.480, z + 0.170), (0.110, 0.160, 0.130), cor, cortes=0))
        nariz = -0.560
    else:
        b.append(pecas.esfera(m, "cabine", (0, -0.050, z), (0.205, 0.300, 0.195) if not coman or t3 < 4 else (0.170, 0.340, 0.170), cor, cortes=2))
        b.append(pecas.tubo(m, "cauda", [(0, 0.150, z + 0.030), (0, 0.480, z + 0.060), (0, 0.760, z + 0.090)], [0.075, 0.048, 0.030], cor))
        b.append(pecas.bloco(m, "deriva", (0.026, 0.130, 0.200), (0, 0.770, z + 0.170), det, chanfro=0.010))
        b.append(pecas.cilindro(m, "rotor_cauda", (0.030, 0.770, z + 0.170), (1, 0, 0), [(0.085, 0), (0.085, 0.012), (0, 0.014)], cor="cinza_escuro", seg=8))
        nariz = -0.330
    b.append(pecas.esfera(m, "vidro", (0, nariz + 0.090, z + 0.045), (0.150, 0.130, 0.110), vidro, cortes=1))
    b.append(pecas.toro(m, "faixa", (0, 0.050, z), 0.200 if not duplo else 0.228, 0.022, det, normal=(0, 1, 0), seg=12, lados=4))
    for sx in (-1, 1):   # esquis
        y0, y1 = (-0.300, 0.220) if not duplo else (-0.500, 0.560)
        b.append(pecas.tubo(m, f"esqui_{sx}", [(0.170 * sx, y0, 0.060), (0.170 * sx, y1, 0.060)], 0.022, "cinza_escuro", nivel=0))
        for k, y in enumerate((y0 + 0.120, y1 - 0.120)):
            b.append(pecas.tubo(m, f"esqui_perna_{sx}_{k}", [(0.170 * sx, y, 0.060), (0.110 * sx, y, z - 0.130)], 0.016, "cinza_escuro", nivel=0))

    # canhoes sob o nariz: 2 na base, 4 com Dardos Quadruplos; mais longos e de aco com Dardos Rapidos
    n = 4 if t1 >= 1 else 2
    L = 0.200 + (0.110 if t3 >= 1 else 0)
    for k in range(n):
        x = (k - (n - 1) / 2) * 0.070
        b.append(pecas.tubo(m, f"canhao_{k}", [(x, nariz + 0.150, z - 0.150), (x, nariz + 0.150 - L, z - 0.160)], 0.020, "aco" if t3 >= 1 else "tinta", nivel=0))
    if t3 >= 2:   # tambor de municao
        b.append(pecas.cilindro(m, "tambor", (-0.060, nariz + 0.230, z - 0.170), (1, 0, 0), [(0, 0), (0.070, 0.004), (0.070, 0.120), (0, 0.124)], cor="amarelo", seg=10))
    if t1 >= 2:   # Perseguicao: farol no nariz
        b.append(pecas.esfera(m, "farol", (0, nariz - 0.010, z - 0.050), 0.058, "amarelo" if not (ataque and t1 == 5) else "ciano", cortes=0))
    if t2 >= 1:   # Jatos Maiores: bocais com chama dos lados
        for sx in (-1, 1):
            y = 0.200 if not duplo else 0.600
            b.append(pecas.cilindro(m, f"jato_{sx}", (0.200 * sx, y - 0.120, z + 0.020), (0, 1, 0), [(0.050, 0), (0.062, 0.140), (0.040, 0.140), (0, 0.090)], cores=["cinza_escuro", "laranja", "laranja"], seg=8))
            b.append(pecas.cone(m, f"jato_chama_{sx}", (0.200 * sx, y + 0.010, z + 0.020), (0, 1, 0), 0.040, 0.160, "laranja", seg=5))
    if ataque and t1 >= 4:   # asas curtas com casulos de foguete (ou de laser)
        for sx in (-1, 1):
            b.append(pecas.bloco(m, f"asa_{sx}", (0.300, 0.130, 0.030), (0.300 * sx, 0.020, z - 0.040), cor, chanfro=0.010))
            b.append(pecas.cilindro(m, f"casulo_{sx}", (0.400 * sx, 0.110, z - 0.080), (0, -1, 0), [(0, 0), (0.060, 0.010), (0.060, 0.240), (0.045, 0.240), (0, 0.200)],
                                    cores=["cinza", "cinza", det, det], seg=8))
    if apoio and t2 >= 4:   # caixa de carga pendurada ao lado
        cx = (0.520, 0.100, 0.130)
        b.append(pecas.bloco(m, "carga", (0.220, 0.220, 0.200), cx, "marrom" if t2 == 4 else "ouro", chanfro=0.020))
        b.append(pecas.bloco(m, "carga_fita", (0.230, 0.060, 0.210), cx, "amarelo" if t2 == 4 else "vermelho", chanfro=0.008))
        b.append(pecas.tubo(m, "carga_cabo", [(0.240, 0.100, z), (0.520, 0.100, 0.230)], 0.010, "tinta", nivel=0))
    if coman:   # ariete de aco no nariz e misseis
        b.append(pecas.cone(m, "ariete", (0, nariz + 0.060, z - 0.020), (0, -1, -0.05), 0.150, 0.260, det if t3 != 4 else "aco", seg=6))
        for sx in (-1, 1):
            b.append(pecas.cilindro(m, f"missil_{sx}", (0.230 * sx, 0.100, z - 0.060), (0, -1, 0), [(0.030, 0), (0.030, 0.220), (0, 0.320)], cores=["branco", "vermelho"], seg=6))
        if t3 == 5:
            b += _mini(m, "escolta_e", (-0.620, 0.300, 0.400), "azul") + _mini(m, "escolta_d", (0.620, 0.300, 0.400), "azul")
    m.por("base", b)
    m.pivo("base", (0, 0, z))

    # rotor principal (dois no helicoptero de carga)
    pas, raio, cor_pa, ponta = 2, 0.560, "cinza_escuro", "amarelo"
    if ataque:
        pas, raio, cor_pa, ponta = {3: (4, 0.640, "aco", "vermelho"), 4: (4, 0.640, "aco", "vermelho"), 5: (5, 0.700, "aco", "ciano")}[t1]
    elif apoio:
        pas, raio, cor_pa, ponta = {3: (6, 0.720, "cinza_escuro", "branco"), 4: (3, 0.560, "cinza_escuro", "amarelo"), 5: (4, 0.580, "cinza_escuro", "ouro")}[t2]
    elif coman:
        pas, ponta = 4 if t3 >= 4 else 3, det
    centros = [(0, 0, z + 0.290)] if not duplo else [(0, -0.380, z + 0.300), (0, 0.480, z + 0.400)]
    t = []
    for k, c in enumerate(centros):
        t += _rotor(m, f"rotor_{k}", c, raio, pas, cor_pa, ponta)
    if t2 >= 2:   # IFR: domo de radar por cima do rotor
        t.append(pecas.esfera(m, "radar", Vector(centros[0]) + Vector((0, 0, 0.075)), (0.085, 0.085, 0.060), "branco", cortes=0))
    m.por("torreta", t)
    m.pivo("torreta", centros[0])


def base(m):
    _heli(m)


PARAMETRO = [(None, None)]   # o tier muda o helicoptero, que le m.tier
CAMINHOS = [{1: PARAMETRO, 2: PARAMETRO, 3: _heli, 4: _heli, 5: _heli} for _ in range(3)]
