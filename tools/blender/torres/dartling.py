"""Atirador Dartling: o macaco padrao de barba e faixa verde na testa, atras de uma metralhadora de canos sobre tripe.

Caminho 1 (laser): canos mais longos e ponta acesa; do tier 3 em diante, canhao de laser e de plasma.
Caminho 2 (foguetes): mira e motor; do tier 3 em diante, casulos de foguete e misseis grandes.
Caminho 3 (canos): mais canos e de aco; do tier 3 em diante, boca larga e canos grossos.

A arma sai de _arma, que le os tres tiers (regra da torre) e vale em qualquer combinacao.
"""
import math

from mathutils import Matrix, Vector

import pecas

ENQUADRE = ((0, -0.12, 0.42), 2.0)
G = Vector((0.170, -0.300, 0.430))     # o eixo da arma passa por aqui, apontando para -Y
RECUO = Matrix.Translation((0, 0.180, 0))


def _arma(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    laser, fog, canos = (p == 0 and t1 >= 3), (p == 1 and t2 >= 3), (p == 2 and t3 >= 3)
    corpo, metal = "verde_escuro", "cinza_escuro"
    if laser:
        corpo, metal = {3: ("azul", "cinza"), 4: ("roxo", "cinza"), 5: ("tinta", "vermelho")}[t1]
    elif fog:
        corpo, metal = {3: ("verde_escuro", "cinza"), 4: ("verde_escuro", "amarelo"), 5: ("tinta", "vermelho")}[t2]
    elif canos:
        corpo, metal = {3: ("cinza_escuro", "aco"), 4: ("cinza_escuro", "aco"), 5: ("tinta", "ouro")}[t3]
    esc = 1.0 + (0.12 * (max(m.tier) - 2) if max(m.tier) >= 3 else 0)

    b = [pecas.cilindro(m, "tripe_cabeca", G - Vector((0, 0, 0.130)), (0, 0, 1), [(0, 0), (0.050, 0.004), (0.050, 0.060), (0, 0.064)], cor="cinza", seg=8)]
    for k, a in enumerate((90, 210, 330)):
        d = Vector((math.cos(math.radians(a)), math.sin(math.radians(a)), 0))
        b.append(pecas.tubo(m, f"tripe_{k}", [G - Vector((0, 0, 0.090)), Vector((G.x, G.y, 0)) + d * 0.300 + Vector((0, 0, 0.010))], 0.020, "cinza", nivel=0))
    m.por("base", b)
    m.pivo("base", (0, 0, 0))

    t = [pecas.bloco(m, "arma_corpo", (0.170 * esc, 0.250 * esc, 0.170 * esc), G + Vector((0, 0.060 * esc, 0)), corpo, chanfro=0.030, seg=2)]
    # manopla ate a mao direita do macaco, que segura a arma; a outra manopla fica livre
    mao = RECUO @ Vector(pecas.MAO)
    t.append(pecas.tubo(m, "manopla", [G + Vector((0.050, 0.150 * esc, 0)), mao + Vector((-0.010, -0.050, 0.010)), mao + Vector((-0.010, 0.010, 0.010))], 0.020, "tinta", nivel=0))
    t.append(pecas.tubo(m, "manopla_esq", [G + Vector((-0.050, 0.150 * esc, 0)), G + Vector((-0.070, 0.150 * esc + 0.110, -0.030))], 0.018, "tinta", nivel=0))
    t.append(pecas.bloco(m, "municao", (0.110, 0.150, 0.130), G + Vector((-0.150 * esc, 0.060, -0.030)), "amarelo" if not laser else "ciano", chanfro=0.016))
    frente = G + Vector((0, -0.060 * esc, 0))
    L = 0.340 * (1.25 if t1 >= 1 else 1.0)
    if laser:
        r, Lc = 0.060 * esc, {3: 0.480, 4: 0.560, 5: 0.680}[t1]
        luz = "ciano" if t1 < 5 else "vermelho"
        t.append(pecas.cilindro(m, "canhao_laser", frente, (0, -1, 0), [(r, 0), (r, Lc * 0.75), (r * 1.5, Lc * 0.80), (r * 1.5, Lc), (r * 0.8, Lc), (r * 0.7, Lc * 0.6), (0, Lc * 0.6)],
                                cores=[metal, metal, metal, luz, luz, luz], seg=10))
        for k in range({3: 2, 4: 3, 5: 4}[t1]):   # bobinas acesas
            t.append(pecas.toro(m, f"bobina_{k}", frente + Vector((0, -Lc * (0.15 + 0.17 * k), 0)), r * 1.25, r * 0.30, luz, normal=(0, 1, 0), seg=10, lados=4))
        if t1 >= 4:   # garras de foco na boca
            for k in range(3):
                a = math.radians(120 * k + 90)
                d = Vector((math.cos(a), 0, math.sin(a)))
                t.append(pecas.cone(m, f"garra_{k}", frente + Vector((0, -Lc, 0)) + d * r * 1.3, Vector((0, -1, 0)) + d * 0.15, r * 0.40, 0.200 if t1 == 4 else 0.300, metal, seg=4))
        boca = frente + Vector((0, -Lc, 0))
    elif fog:
        lados = (0,) if t2 == 3 else (-1, 1)
        for sx in lados:
            c = frente + Vector((0.130 * sx * esc, 0, 0.020))
            if t2 < 5:
                t.append(pecas.bloco(m, f"casulo_{sx}", (0.200, 0.340, 0.170), c + Vector((0, -0.170, 0)), corpo, chanfro=0.022))
                for k in range(6):
                    x, z = (k % 3 - 1) * 0.062, (k // 3 - 0.5) * 0.070
                    t.append(pecas.cilindro(m, f"foguete_{sx}_{k}", c + Vector((x, -0.320, z)), (0, -1, 0), [(0.024, 0), (0.024, 0.040), (0, 0.110)], cores=["branco", "vermelho"], seg=6))
            else:
                t.append(pecas.cilindro(m, f"missil_{sx}", c + Vector((0, 0.150, 0.040)), (0, -1, 0), [(0, 0), (0.075, 0.030), (0.075, 0.560), (0.050, 0.700), (0, 0.840)],
                                        cores=["cinza", "tinta", "vermelho", "vermelho"], seg=10))
                t.append(pecas.toro(m, f"missil_faixa_{sx}", c + Vector((0, -0.150, 0.040)), 0.076, 0.016, "amarelo", normal=(0, 1, 0), seg=10, lados=4))
        boca = frente + Vector((0, -0.360, 0))
    else:
        n = 4 if t3 == 0 else 6
        r_cano, r_anel = 0.020, 0.058
        if t3 >= 2:
            r_cano = 0.027
        if canos:
            n, r_cano, r_anel, L = {3: (1, 0.085, 0.0, 0.300), 4: (4, 0.048, 0.085, 0.420), 5: (6, 0.046, 0.105, 0.480)}[t3]
        cor_cano = "aco" if t3 >= 2 else "tinta"
        if canos and t3 == 5:
            cor_cano = "ouro"
        for k in range(n):
            a = 2 * math.pi * k / n + math.pi / n
            d = Vector((math.cos(a), 0, math.sin(a))) * r_anel
            if canos and t3 == 3:   # boca larga de chumbinho
                t.append(pecas.cilindro(m, "cano_largo", frente, (0, -1, 0), [(0.060, 0), (0.060, L * 0.6), (0.110, L), (0.085, L), (0.050, L * 0.5), (0, L * 0.5)],
                                        cores=[cor_cano, cor_cano, "tinta", "tinta", "tinta"], seg=10))
            else:
                t.append(pecas.tubo(m, f"cano_{k}", [frente + d, frente + d + Vector((0, -L, 0))], r_cano, cor_cano, nivel=0))
        if r_anel:
            for k, f in enumerate((0.35, 0.92)):
                t.append(pecas.toro(m, f"cano_anel_{k}", frente + Vector((0, -L * f, 0)), r_anel + r_cano * 0.4, r_cano * 0.7, metal, normal=(0, 1, 0), seg=10, lados=4))
        boca = frente + Vector((0, -L, 0))
    # enfeites dos tiers 1 e 2 (valem em qualquer arma)
    if t1 == 2 or (t1 >= 2 and not laser):   # Choque Laser: ponta acesa
        t.append(pecas.esfera(m, "ponta_laser", boca + Vector((0, -0.020, 0)), 0.055, "ciano", cortes=0))
    if t2 >= 1:   # Mira Avancada
        t.append(pecas.cilindro(m, "mira", G + Vector((0, 0.100, 0.085 * esc + 0.030)), (0, -1, 0), [(0, 0), (0.028, 0.004), (0.028, 0.160), (0.042, 0.166), (0.042, 0.220), (0, 0.210)],
                                cores=["cinza", "cinza", "cinza", "cinza", "ciano"], seg=8))
    if t2 >= 2:   # Giro de Cano Rapido: motor amarelo atras
        t.append(pecas.cilindro(m, "motor", G + Vector((0.100 * esc, 0.060, 0)), (1, 0, 0), [(0, 0), (0.070, 0.004), (0.070, 0.070), (0.040, 0.080), (0, 0.080)], cor="amarelo", seg=10))
    m.por("torreta", t)
    m.pivo("torreta", tuple(G))


def base(m):
    m.macaco(pelagem="barba")
    m.por("chapeu", pecas.faixa_testa(m, "verde_escuro"))
    m.matriz = RECUO
    _arma(m)


def laser(m):
    t = m.tier[0]
    m.por("rosto", pecas.viseira(m, "ciano" if t < 5 else "vermelho", "cinza_escuro"))
    if t >= 4:
        m.por("tronco", pecas.colete(m, "roxo" if t == 4 else "tinta", barra="ciano" if t == 4 else "vermelho"))
    _arma(m)


def foguetes(m):
    t = m.tier[1]
    m.por("chapeu", pecas.capacete(m, "verde_escuro" if t < 5 else "tinta", "amarelo" if t < 5 else "vermelho"))
    if t >= 4:
        m.por("tronco", pecas.colete(m, "verde_escuro" if t == 4 else "tinta", barra="amarelo" if t == 4 else "vermelho"))
    _arma(m)


def canos(m):
    t = m.tier[2]
    m.por("tronco", pecas.bandoleira(m, "amarelo" if t < 5 else "ouro"))
    if t >= 4:
        m.por("chapeu", pecas.capacete(m, "cinza_escuro" if t == 4 else "tinta", "aco" if t == 4 else "ouro"))
    _arma(m)


PARAMETRO = [(None, None)]   # o tier muda a arma, que le m.tier
CAMINHOS = [
    {1: PARAMETRO, 2: PARAMETRO, 3: laser, 4: laser, 5: laser},
    {1: PARAMETRO, 2: PARAMETRO, 3: foguetes, 4: foguetes, 5: foguetes},
    {1: PARAMETRO, 2: PARAMETRO, 3: canos, 4: canos, 5: canos},
]
