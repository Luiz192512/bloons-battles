"""Macaco As: so o aviao, sem macaco. Fuselagem, asas e cauda (base) e as helices (torreta, giram no jogo).

Caminho 1 (caca): metralhadoras nas asas; do tier 3 em diante, caca a jato com misseis.
Caminho 2 (bombardeiro): abacaxi e radar; do tier 3 em diante, bombardeiro com bombas cada vez maiores.
Caminho 3 (canhoneira): ponta de aco e alvos nas asas; do tier 3 em diante, sensor, mais motores e asa enorme.

Tudo sai de _aviao, que le os tres tiers (regra da torre) e vale em qualquer combinacao.
"""
from mathutils import Matrix, Vector

import pecas
import poli

ENQUADRE = ((0, 0.0, 0.30), 2.0)
Z0 = 0.300


def _asa(m, nome, sx, meia, corda, recuo, cor, ponta, y=-0.060, z=Z0, esp=0.034):
    """Meia asa: caixa afilada, com a ponta de outra cor; recuo = quanto a ponta fica para tras."""
    corta = Matrix(((1, 0, 0, 0), (recuo * sx, 1, 0, 0), (0, 0, 1, 0), (0, 0, 0, 1)))
    mat = Matrix.Translation((0, y, z)) @ corta
    objs = [pecas.bloco(m, f"{nome}_{sx}", (meia * 0.80, corda, esp), (sx * meia * 0.40, 0, 0), cor, chanfro=0.012, matriz=mat),
            pecas.bloco(m, f"{nome}_ponta_{sx}", (meia * 0.22, corda * 0.80, esp), (sx * meia * 0.90, 0, 0), ponta, chanfro=0.012, matriz=mat)]
    return objs


def _helice(m, nome, pos, raio, cor="cinza_escuro", cubo="amarelo"):
    objs = [pecas.cone(m, nome + "_cubo", pos, (0, -1, 0), raio * 0.24, raio * 0.42, cubo, seg=6, fechado=True)]
    for k, d in enumerate(((1, 0, 0.25), (-1, 0, -0.25))):
        objs.append(pecas.cone(m, f"{nome}_pa_{k}", Vector(pos) + Vector((0, -0.020, 0)), d, raio * 0.20, raio, cor, seg=4, fechado=True))
    return objs


def _bomba(m, nome, pos, raio, cor, faixa):
    perfil = [(0, 0), (raio * 0.75, raio * 0.30), (raio, raio * 1.10), (raio, raio * 1.60), (raio * 0.70, raio * 2.40), (raio * 0.30, raio * 2.90), (raio * 0.55, raio * 3.40), (0, raio * 3.30)]
    return pecas.cilindro(m, nome, pos, (0, 1, 0), perfil, cores=[cor, cor, faixa, cor, cor, faixa, faixa], seg=10)


def _aviao(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    jato = p == 0 and t1 >= 3
    bomb = p == 1 and t2 >= 3
    canh = p == 2 and t3 >= 3
    cor, ponta, cauda = "vermelho", "amarelo", "amarelo"
    meia, corda, recuo, esc = 0.560, 0.230, 0.10, 1.0
    if jato:
        cor, ponta, cauda = {3: ("cinza", "vermelho", "vermelho"), 4: ("azul", "branco", "branco"), 5: ("tinta", "ouro", "ouro")}[t1]
        meia, corda, recuo, esc = {3: (0.520, 0.300, 0.55, 1.05), 4: (0.580, 0.340, 0.65, 1.15), 5: (0.660, 0.420, 0.80, 1.28)}[t1]
    elif bomb:
        cor, ponta, cauda = {3: ("verde_escuro", "amarelo", "amarelo"), 4: ("verde_escuro", "laranja", "laranja"), 5: ("tinta", "vermelho", "vermelho")}[t2]
        meia, corda, recuo, esc = {3: (0.660, 0.260, 0.08, 1.15), 4: (0.720, 0.280, 0.08, 1.25), 5: (0.800, 0.300, 0.08, 1.38)}[t2]
    elif canh:
        cor, ponta, cauda = {3: ("azul", "branco", "branco"), 4: ("cinza_escuro", "azul", "azul"), 5: ("cinza", "ouro", "ouro")}[t3]
        meia, corda, recuo, esc = {3: (0.600, 0.240, 0.10, 1.05), 4: (0.740, 0.260, 0.06, 1.20), 5: (0.900, 0.290, 0.06, 1.35)}[t3]

    L = 1.000 * esc
    perfil = [(0, 0), (0.035 * esc, 0.020), (0.085 * esc, 0.200 * esc), (0.118 * esc, 0.550 * esc), (0.110 * esc, 0.800 * esc), (0.070 * esc, 0.940 * esc), (0, L)]
    nariz = Vector((0, -L / 2, Z0))
    b = [pecas.cilindro(m, "fuselagem", (0, L / 2, Z0), (0, -1, 0), perfil, cores=[cauda, cor, cor, cor, cor, ponta if jato else cor], seg=12)]
    b.append(pecas.esfera(m, "cabine", (0, -0.150 * esc, Z0 + 0.095 * esc), (0.070 * esc, 0.130 * esc, 0.060 * esc), "ciano", cortes=0))
    for sx in (-1, 1):
        b += _asa(m, "asa", sx, meia, corda, recuo, cor, ponta)
        b += _asa(m, "estabilizador", sx, 0.200 * esc, 0.120 * esc, 0.30, cauda, cauda, y=0.400 * esc, esp=0.026)
    deriva = (-0.075, 0.075) if (jato and t1 >= 4) else (0,)
    for k, x in enumerate(deriva):
        b.append(pecas.bloco(m, f"deriva_{k}", (0.026, 0.150 * esc, 0.190 * esc), (x, 0.410 * esc, Z0 + 0.110 * esc), cauda, chanfro=0.010))

    t = []
    # motores e helices: o jato nao tem helice; canhoneira e bombardeiro grandes tem motores nas asas
    motores = []
    if not jato:
        n_asa = {True: {3: 0, 4: 1, 5: 2}.get(t3, 0), False: 0}[canh] + (1 if (bomb and t2 == 5) else 0)
        if n_asa == 0:
            motores.append(nariz + Vector((0, -0.010, 0)))
        for k in range(n_asa):
            for sx in (-1, 1):
                x = sx * meia * (0.38 + 0.36 * k) if n_asa > 1 else sx * meia * 0.45
                motores.append(Vector((x, -0.060 - corda * 0.62 + recuo * abs(x), Z0)))
                b.append(pecas.cilindro(m, f"motor_{k}_{sx}", (x, -0.060 + corda * 0.30 + recuo * abs(x), Z0), (0, -1, 0), [(0, 0), (0.050, 0.030), (0.060, corda * 0.85), (0, corda * 0.92)],
                                        cor="cinza_escuro", seg=8))
        for k, pos in enumerate(motores):
            t += _helice(m, f"helice_{k}", pos, 0.170 * (esc if len(motores) == 1 else 0.85))
    else:   # bocais do jato na cauda
        for sx in ((-1, 1) if t1 >= 5 else (0,)):
            b.append(pecas.cilindro(m, f"bocal_{sx}", (0.060 * sx, L / 2 - 0.040, Z0), (0, 1, 0), [(0.060, 0), (0.075, 0.090), (0.050, 0.090), (0, 0.040)], cores=["cinza_escuro", "laranja", "laranja"], seg=8))

    borda = lambda x: -0.060 - corda * 0.50 + recuo * abs(x)   # noqa: E731  (y da borda de ataque da asa em x)
    # caminho 1: metralhadoras nas asas; no jato, misseis
    if t1 >= 1 and not jato:
        for k in range(1 if t1 == 1 else 2):
            for sx in (-1, 1):
                x = sx * meia * (0.30 + 0.22 * k)
                b.append(pecas.tubo(m, f"metralhadora_{k}_{sx}", [(x, borda(x) + 0.060, Z0 + 0.010), (x, borda(x) - 0.130, Z0 + 0.010)], 0.018, "tinta", nivel=0))
    if jato:
        for k in range({3: 1, 4: 2, 5: 3}[t1]):
            for sx in (-1, 1):
                x = sx * meia * (0.32 + 0.22 * k)
                b.append(pecas.cilindro(m, f"missil_{k}_{sx}", (x, borda(x) + 0.170, Z0 - 0.020), (0, -1, 0), [(0.026, 0), (0.026, 0.210), (0, 0.300)], cores=["branco", "vermelho"], seg=6))
    # caminho 2: abacaxi, radar e bombas
    if t2 >= 1 and not bomb:
        y = 0.180 * esc
        b.append(pecas.esfera(m, "abacaxi", (0, y, Z0 + 0.150 * esc), (0.070, 0.085, 0.070), "amarelo", cortes=0))
        b.append(pecas.cone(m, "abacaxi_folhas", (0, y + 0.060, Z0 + 0.170 * esc), (0, 1, 0.6), 0.050, 0.120, "verde", seg=5))
    if t2 >= 2:
        b.append(pecas.esfera(m, "radar", (0, 0.020 * esc, Z0 + 0.130 * esc), (0.085, 0.085, 0.045), "branco" if not bomb else "cinza", cortes=0))
    if bomb:
        if t2 == 3:
            for sx in (-1, 1):
                b.append(_bomba(m, f"bomba_{sx}", (sx * meia * 0.50, borda(sx * meia * 0.50) - 0.080, Z0 - 0.030), 0.060, "tinta", "amarelo"))
        else:
            r = 0.170 if t2 == 4 else 0.215
            b.append(_bomba(m, "bomba_grande", (0, -r * 2.2, Z0 - 0.105 * esc - r * 0.55), r, "tinta" if t2 == 4 else "vermelho", "amarelo" if t2 == 4 else "tinta"))
    # caminho 3: ponta de aco, alvos nas asas, sensor e canhoes laterais
    if t3 >= 1:
        b.append(pecas.cone(m, "ponta_aco", nariz + Vector((0, 0.020 if jato else -0.060, 0 if jato else -0.060)), (0, -1, 0), 0.036, 0.230, "aco", seg=6))
    if t3 >= 2:
        for sx in (-1, 1):
            x = sx * meia * 0.55
            b.append(pecas.cilindro(m, f"alvo_{sx}", (x, -0.060 + recuo * abs(x), Z0 + 0.016), (0, 0, 1), [(0.085, 0), (0.085, 0.008), (0.045, 0.010), (0.045, 0.014), (0, 0.016)],
                                    cores=["branco", "branco", "ciano", "ciano"], seg=10))
    if canh:
        b.append(pecas.esfera(m, "sensor", nariz + Vector((0, 0.110, -0.100 * esc)), 0.060, "ciano", cortes=0))
        if t3 >= 4:
            for k, y in enumerate((0.060, 0.220)):
                b.append(pecas.tubo(m, f"canhao_lateral_{k}", [(-0.090 * esc, y, Z0 - 0.020), (-0.330 * esc, y - 0.050, Z0 - 0.060)], 0.024, "tinta", nivel=0))
    m.por("base", b)
    m.pivo("base", (0, 0, Z0))
    if t:
        m.por("torreta", t)
        m.pivo("torreta", tuple(motores[0]))
    else:
        m.tirar("torreta")


def base(m):
    _aviao(m)


PARAMETRO = [(None, None)]   # o tier muda o aviao, que le m.tier
CAMINHOS = [{1: PARAMETRO, 2: PARAMETRO, 3: _aviao, 4: _aviao, 5: _aviao} for _ in range(3)]
