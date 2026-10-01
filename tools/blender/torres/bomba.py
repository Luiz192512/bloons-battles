"""Canhao Bomba: maquina sem macaco. Carreta com rodas (base) e o cano com o pavio (torreta).

Caminho 1 (bombas maiores): boca e cano cada vez mais grossos, ate o canhao preto e dourado.
Caminho 2 (misseis): pilha de bombas e missil na boca; do tier 3 em diante, lanca-misseis.
Caminho 3 (cacho): cano mais comprido e cravos; do tier 3 em diante, feixe de canos.

A torreta sai de _torreta, que le os tres tiers (regra da torre): o caminho principal escolhe o
tipo (canhao, lanca-misseis ou feixe) e os tiers cruzados enfeitam qualquer tipo (aro da boca,
cinta de aco, missil na boca, comprimento, cravos).
"""
import math

from mathutils import Matrix, Vector

import pecas
import poli

ENQUADRE = ((0, -0.05, 0.36), 1.8)
EIXO = Vector((0, -1, 0.20)).normalized()
CULATRA = Vector((0, 0.300, 0.335))


def carreta(m, cor="marrom", berco_cor="marrom_escuro", cubo="ouro", pneu="tinta", chapa=None):
    base = []
    bm = poli.caixa((0.40, 0.54, 0.15), cor, chanfro=0.045, seg=3)
    poli.mover(bm, (0, 0.03, 0.175))
    base.append(m.obj("carreta", bm, nivel=0, vivo=True))
    berco = poli.gaiola((0.150, 0.200, 0.085), cortes=1, cor=berco_cor)
    poli.mover(berco, (0, 0.04, 0.265))
    base.append(m.obj("berco", berco, nivel=1))
    roda = [(0, -0.048), (0.070, -0.052), (0.085, -0.060), (0.150, -0.050), (0.168, -0.020), (0.168, 0.020),
            (0.150, 0.050), (0.085, 0.060), (0.070, 0.052), (0.045, 0.060), (0, 0.130)]
    cores_roda = [cubo] + [pneu] * 7 + [cubo, cubo]
    for sx in (-1, 1):
        r = poli.torno(roda, seg=20, cores=cores_roda)
        base.append(m.obj(f"roda_{sx}", poli.orientar(r, (sx, 0, 0), (0.262 * sx, 0.03, 0.168)), nivel=0, vivo=True))
    if chapa:  # reforco de metal nos cantos da carreta
        for sx in (-1, 1):
            for k, y in enumerate((-0.20, 0.26)):
                base.append(pecas.bloco(m, f"chapa_{sx}_{k}", (0.070, 0.090, 0.170), (0.180 * sx, y, 0.175), chapa, chanfro=0.014))
    return base


def cano(m, raio=1.0, comp=1.0, cor="cinza_escuro", anel="ouro", cinta="azul", boca="tinta", nome="cano", origem=CULATRA, seg=24):
    """O cano do piloto, torneado; raio e comp escalam o perfil."""
    perfil = [(0, 0), (0.105, 0.012), (0.160, 0.060), (0.178, 0.130), (0.178, 0.215), (0.192, 0.225), (0.192, 0.265),
              (0.176, 0.275), (0.156, 0.500), (0.170, 0.510), (0.170, 0.560), (0.154, 0.570), (0.146, 0.690),
              (0.172, 0.705), (0.172, 0.770), (0.128, 0.775), (0.116, 0.640), (0, 0.620)]
    perfil = [(r * raio, h * comp) for r, h in perfil]
    cores = [cor] * 5 + [anel] + [cor] * 3 + [cinta] + [cor] * 2 + [boca] * 5
    obj = m.obj(nome, poli.orientar(poli.torno(perfil, seg=seg, cores=cores), EIXO, origem), nivel=0, vivo=True)
    return obj, {"o": Vector(origem), "L": 0.775 * comp, "r": 0.172 * raio, "furo": 0.128 * raio}


def pavio(m, sobe=0.0):
    """Pavio aceso na culatra, com tres faiscas."""
    mat = Matrix.Translation((0, 0, sobe))
    objs = [m.obj("pavio", poli.membro([(0, 0.215, 0.500), (0.012, 0.250, 0.585), (0.034, 0.262, 0.650)],
                                       [0.014, 0.013, 0.011], cor="bege"), nivel=1, matriz=mat if sobe else None)]
    for nome, cor, direcao, comp in (("faisca", "amarelo", (0.3, 0.2, 1), 0.050), ("faisca2", "vermelho", (1, -0.4, 0.1), 0.045),
                                     ("faisca3", "amarelo", (-0.2, 1, 0.2), 0.045)):
        f = poli.torno([(0, -comp), (0.022, 0), (0, comp)], seg=6, cor=cor)
        objs.append(m.obj(nome, poli.orientar(f, direcao, (0.036, 0.264, 0.660)), nivel=0, vivo=True, matriz=mat if sobe else None))
    return objs


def _lancador(m, raio, comp, cor, anel, origem, nome="lancador", seg=14):
    """Tubo de lanca-misseis com o missil apontando na boca."""
    r, L = raio, comp
    tubo = [(0, 0), (r * 0.90, 0.010), (r, 0.050), (r, L * 0.30), (r * 1.07, L * 0.31), (r * 1.07, L * 0.37), (r, L * 0.38), (r, L - 0.050),
            (r * 1.10, L - 0.040), (r * 1.10, L), (r * 0.84, L), (r * 0.80, L * 0.55), (0, L * 0.55)]
    cores = [cor, cor, cor, anel, anel, anel, cor, anel, anel, "tinta", "tinta", "tinta"]
    objs = [m.obj(nome, poli.orientar(poli.torno(tubo, seg=seg, cores=cores), EIXO, origem), nivel=0, vivo=True)]
    missil = [(r * 0.72, L * 0.55), (r * 0.72, L + 0.050), (r * 0.56, L + 0.160), (0, L + 0.300)]
    objs.append(m.obj(nome + "_missil", poli.orientar(poli.torno(missil, seg=seg, cores=["branco", "vermelho", "vermelho"]), EIXO, origem), nivel=0, vivo=True))
    return objs, {"o": Vector(origem), "L": L, "r": r * 1.10, "furo": 0.0}


def _missil_gigante(m, origem):
    """Missil unico, exposto sobre o trilho: preto, de faixas amarelas e nariz vermelho, com aletas."""
    r, L = 0.185, 1.00
    corpo = [(0, 0), (r * 0.70, 0.010), (r, 0.090), (r, 0.300), (r * 1.04, 0.305), (r * 1.04, 0.365), (r, 0.370), (r, 0.640), (r * 1.04, 0.645),
             (r * 1.04, 0.705), (r, 0.710), (r, 0.800), (r * 0.72, 0.930), (0, 1.100)]
    cores = ["cinza", "tinta", "tinta", "amarelo", "amarelo", "amarelo", "tinta", "amarelo", "amarelo", "amarelo", "tinta", "vermelho", "vermelho"]
    objs = [m.obj("missil", poli.orientar(poli.torno(corpo, seg=16, cores=cores), EIXO, origem), nivel=0, vivo=True)]
    lado = EIXO.cross(Vector((1, 0, 0))).normalized()
    for k, d in enumerate((Vector((1, 0, 0)), Vector((-1, 0, 0)), lado, -lado)):
        objs.append(pecas.cone(m, f"missil_aleta_{k}", Vector(origem) + EIXO * 0.060 + d * r * 0.80, d + EIXO * -0.5, 0.085, 0.230, "vermelho", seg=4, fechado=True))
    objs.append(pecas.bloco(m, "trilho", (0.130, 0.700, 0.060), Vector(origem) + EIXO * 0.36 + Vector((0, 0, -0.215)), "cinza_escuro", chanfro=0.014,
                            matriz=None))
    return objs, {"o": Vector(origem), "L": L * 0.80, "r": r * 1.04, "furo": 0.0}


def _enfeitar(m, canos, t1, t2, t3, principal):
    """Enfeites dos tiers cruzados, em cada cano: aro da boca, cinta de aco, missil na boca e cravos."""
    objs = []
    for i, c in enumerate(canos):
        boca, meio = c["o"] + EIXO * c["L"], c["o"] + EIXO * c["L"] * 0.42
        if principal != 0 and t1 >= 1:   # Bombas Maiores: aro grosso e dourado na boca
            objs.append(pecas.toro(m, f"aro_boca_{i}", boca - EIXO * 0.030, c["r"] * 1.08, c["r"] * 0.20, "ouro", normal=EIXO, seg=14, lados=5))
        if principal != 0 and t1 >= 2:   # Bombas Pesadas: cinta de aco no meio do cano
            objs.append(pecas.toro(m, f"cinta_{i}", meio, c["r"] * 0.98, c["r"] * 0.16, "aco", normal=EIXO, seg=14, lados=5))
        if t2 == 2 and c["furo"]:   # Lanca-Misseis: nariz vermelho de missil na boca
            objs.append(pecas.cilindro(m, f"nariz_{i}", boca - EIXO * 0.060, EIXO, [(c["furo"] * 0.95, 0), (c["furo"] * 0.95, 0.070), (c["furo"] * 0.60, 0.160), (0, 0.270)],
                                       cores=["branco", "vermelho", "vermelho"], seg=12))
        if t3 == 2:   # Bombas de Fragmentacao: coroa de cravos amarelos
            pos = c["o"] + EIXO * c["L"] * 0.72
            u = Vector((1, 0, 0))
            w = EIXO.cross(u).normalized()
            for k in range(8):
                a = 2 * math.pi * k / 8
                d = u * math.cos(a) + w * math.sin(a)
                objs.append(pecas.cone(m, f"cravo_{i}_{k}", pos + d * c["r"] * 0.86, d, c["r"] * 0.22, c["r"] * 0.50, "amarelo", seg=4))
    return objs


def _torreta(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    comp = 1.22 if t3 >= 1 else 1.0   # Alcance Extra: cano mais comprido
    objs, canos, tem_pavio, sobe = [], [], True, 0.0
    if p == 0 and t1 >= 1:
        raio = {1: 1.10, 2: 1.18, 3: 1.34, 4: 1.44, 5: 1.60}[t1]
        cor, anel, cinta, boca = {1: ("cinza_escuro", "ouro", "azul", "tinta"), 2: ("cinza_escuro", "ouro", "aco", "tinta"),
                                  3: ("cinza_escuro", "ouro", "vermelho", "tinta"), 4: ("cinza_escuro", "amarelo", "vermelho", "tinta"),
                                  5: ("tinta", "ouro", "vermelho", "tinta")}[t1]
        o, c = cano(m, raio, comp * (0.95 if t1 >= 3 else 1.0), cor, anel, cinta, boca, origem=CULATRA + Vector((0, 0.02, 0.17)) * (raio - 1))
        objs.append(o)
        canos.append(c)
        sobe = 0.33 * (raio - 1)
        boca_pos = c["o"] + EIXO * c["L"]
        if t1 >= 2:   # cinta de aco
            objs.append(pecas.toro(m, "cinta", c["o"] + EIXO * c["L"] * 0.42, c["r"] * 0.98, c["r"] * 0.14, "aco", normal=EIXO, seg=16, lados=5))
        if t1 >= 4:   # coroa de pontas na boca
            u, w = Vector((1, 0, 0)), EIXO.cross(Vector((1, 0, 0))).normalized()
            for k in range(8):
                a = 2 * math.pi * k / 8
                d = u * math.cos(a) + w * math.sin(a)
                objs.append(pecas.cone(m, f"boca_ponta_{k}", boca_pos - EIXO * 0.040 + d * c["r"] * 0.92, d + EIXO * 0.6, c["r"] * 0.24, c["r"] * 0.66,
                                       "amarelo" if t1 == 4 else "ouro", seg=4))
    elif p == 1 and t2 >= 3:
        tem_pavio = False
        if t2 == 3:
            o, c = _lancador(m, 0.150, 0.80 * comp, "verde_escuro", "cinza_escuro", CULATRA)
            objs += o
            canos.append(c)
        elif t2 == 4:
            for sx in (-1, 1):
                o, c = _lancador(m, 0.125, 0.84 * comp, "verde_escuro", "amarelo", CULATRA + Vector((0.135 * sx, 0, 0.02)), nome=f"lancador_{sx}")
                objs += o
                canos.append(c)
            objs.append(pecas.bloco(m, "lancador_ponte", (0.300, 0.220, 0.090), CULATRA + EIXO * 0.28 + Vector((0, 0, -0.02)), "cinza_escuro", chanfro=0.020))
        else:
            o, c = _missil_gigante(m, CULATRA + Vector((0, 0.10, 0.06)))
            objs += o
            canos.append(c)
    elif p == 2 and t3 >= 3:
        n, raio, cor, anel, cinta = {3: (3, 0.56, "cinza_escuro", "ouro", "azul"), 4: (5, 0.50, "cinza_escuro", "azul", "ciano"),
                                     5: (7, 0.46, "vermelho", "ouro", "tinta")}[t3]
        u, w = Vector((1, 0, 0)), EIXO.cross(Vector((1, 0, 0))).normalized()
        centros = [Vector((0, 0, 0))] if n > 3 else []
        centros += [(u * math.cos(a) + w * math.sin(a)) * (0.115 if n == 3 else 0.185 * (raio / 0.50))
                    for a in (2 * math.pi * k / (n - len(centros)) + math.pi / 2 for k in range(n - len(centros)))]
        for i, d in enumerate(centros):
            o, c = cano(m, raio, comp * 0.92, cor, anel, cinta, nome=f"cano_{i}", origem=CULATRA + Vector((0, 0, 0.06)) + d, seg=12)
            objs.append(o)
            canos.append(c)
        objs.append(pecas.toro(m, "feixe_cinta", CULATRA + Vector((0, 0, 0.06)) + EIXO * 0.30, (0.115 if n == 3 else 0.185 * raio / 0.50) + 0.200 * raio, 0.028, anel, normal=EIXO, seg=14, lados=4))
        sobe = 0.10 + 0.02 * n
    else:
        o, c = cano(m, 1.0, comp)
        objs.append(o)
        canos.append(c)
    objs += _enfeitar(m, canos, t1, t2, t3, p if max(m.tier) else -1)
    if tem_pavio:
        objs += pavio(m, sobe)
    m.por("torreta", objs)


def _carreta(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    if p == 0 and t1 >= 4:
        m.por("base", carreta(m, "cinza_escuro" if t1 == 5 else "marrom", "tinta" if t1 == 5 else "marrom_escuro", "ouro", "tinta", chapa="ouro" if t1 == 5 else "cinza"))
    elif p == 1 and t2 >= 3:
        m.por("base", carreta(m, "verde_escuro", "cinza_escuro", "amarelo" if t2 >= 4 else "cinza", "tinta", chapa="vermelho" if t2 == 5 else None))
    elif p == 2 and t3 >= 4:
        m.por("base", carreta(m, "marrom", "marrom_escuro", "ciano" if t3 == 4 else "vermelho", "tinta", chapa="cinza" if t3 == 4 else "ouro"))
    else:
        m.por("base", carreta(m))


def base(m):
    _carreta(m)
    _torreta(m)
    m.pivo("base", (0, 0, 0))
    m.pivo("torreta", (0, 0.05, 0.38))


def refazer(m):
    """Conjunto dos tiers 3 a 5: a carreta e a torreta saem de novo, lendo os tiers."""
    _carreta(m)
    _torreta(m)


def pilha_de_bombas(m):
    """Recarga Rapida: bombas de reserva empilhadas ao lado da carreta."""
    objs = []
    for k, (x, y, z) in enumerate(((0.470, 0.130, 0.085), (0.470, -0.050, 0.085), (0.470, 0.040, 0.225))):
        objs.append(pecas.esfera(m, f"reserva_{k}", (x, y, z), 0.088, "tinta"))
        objs.append(pecas.tubo(m, f"reserva_pavio_{k}", [(x, y, z + 0.080), (x + 0.020, y + 0.010, z + 0.130)], [0.012, 0.010], "bege", nivel=0))
    m.somar("extra", objs)


PARAMETRO = [(None, None)]   # o tier muda a torreta, que le m.tier
CAMINHOS = [
    {1: PARAMETRO, 2: PARAMETRO, 3: refazer, 4: refazer, 5: refazer},
    {1: [("extra", pilha_de_bombas)], 2: PARAMETRO, 3: refazer, 4: refazer, 5: refazer},
    {1: PARAMETRO, 2: PARAMETRO, 3: refazer, 4: refazer, 5: refazer},
]
