"""Fabrica de Espinhos: maquina sem macaco. Caixa da fabrica com funil e a pilha de espinhos (base) e a calha (torreta).

Caminho 1 (minas): pilha maior e em brasa; do tier 3 em diante, bolas espinhosas e minas.
Caminho 2 (producao): engrenagens; do tier 3 em diante, rolo triturador e lancadores no teto.
Caminho 3 (duracao): calha comprida e antena; do tier 3 em diante, espinhos reforcados e dourados.

Tudo sai de _fabrica, que le os tres tiers (regra da torre) e vale em qualquer combinacao.
"""
import math

from mathutils import Vector

import pecas

ENQUADRE = ((0, -0.10, 0.28), 1.7)


def _pilha(m, centro, n, cor, tam=1.0, nome="espinho"):
    """Pilha de espinhos: pontas para cima e para os lados."""
    objs = []
    for k in range(n):
        a = math.radians(360 * k / n + 20 * (k % 3))
        r = 0.110 * tam * (0.4 + 0.6 * ((k * 7) % 5) / 4)
        d = Vector((math.cos(a) * 0.5, math.sin(a) * 0.5, 1))
        objs.append(pecas.cone(m, f"{nome}_{k}", Vector(centro) + Vector((r * math.cos(a), r * math.sin(a), 0)), d, 0.026 * tam, 0.130 * tam, cor, seg=4, fechado=True))
    return objs


def _bola(m, nome, pos, raio, cor, espinho):
    objs = [pecas.esfera(m, nome, pos, raio, cor, cortes=0)]
    for k, d in enumerate(((1, 0, 0.3), (-1, 0, 0.3), (0, 1, 0.3), (0, -1, 0.3), (0, 0, 1), (0.7, 0.7, 0.6), (-0.7, -0.7, 0.6))):
        d = Vector(d).normalized()
        objs.append(pecas.cone(m, f"{nome}_ponta_{k}", Vector(pos) + d * raio * 0.75, d, raio * 0.30, raio * 0.80, espinho, seg=4))
    return objs


def _fabrica(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    minas, prod, dura = (p == 0 and t1 >= 3), (p == 1 and t2 >= 3), (p == 2 and t3 >= 3)
    corpo, teto, metal = "cinza_escuro", "vermelho", "cinza"
    if minas:
        corpo, teto = {3: ("azul", "cinza"), 4: ("cinza_escuro", "amarelo"), 5: ("tinta", "vermelho")}[t1]
    elif prod:
        corpo, teto = {3: ("verde_escuro", "cinza"), 4: ("verde_escuro", "amarelo"), 5: ("vermelho", "tinta")}[t2]
    elif dura:
        corpo, teto = {3: ("marrom", "cinza"), 4: ("tinta", "vermelho"), 5: ("branco", "ouro")}[t3]
    b = [pecas.bloco(m, "fabrica", (0.520, 0.420, 0.340), (0, 0.180, 0.170), corpo, chanfro=0.030, seg=2),
         pecas.bloco(m, "fabrica_teto", (0.560, 0.460, 0.060), (0, 0.180, 0.360), teto, chanfro=0.020),
         pecas.cilindro(m, "funil", (0, 0.220, 0.380), (0, 0, 1), [(0.070, 0), (0.070, 0.050), (0.150, 0.170), (0.130, 0.170), (0.055, 0.060), (0, 0.060)], cores=[metal, metal, metal, "tinta", "tinta"], seg=10)]
    comp = 0.300 + (0.160 if t3 >= 1 else 0)   # Alcance Longo: calha mais comprida
    calha = [pecas.bloco(m, "calha", (0.200, comp, 0.040), (0, -0.030 - comp / 2, 0.100), metal, chanfro=0.012)]
    for sx in (-1, 1):   # a calha e malha propria (torreta), para o jogo poder sacudir quando solta espinhos
        calha.append(pecas.bloco(m, f"calha_borda_{sx}", (0.026, comp, 0.070), (0.100 * sx, -0.030 - comp / 2, 0.120), metal, chanfro=0.008))
    saida = Vector((0, -0.110 - comp, 0.020))
    # o que sai da fabrica: espinhos, bolas ou minas
    cor_esp = "laranja" if t1 >= 2 else "aco"
    if dura:
        cor_esp = {3: "aco", 4: "vermelho", 5: "ouro"}[t3]
    if minas:
        raio = {3: 0.075, 4: 0.095, 5: 0.135}[t1]
        cor_b, ponta = {3: ("cinza", "aco"), 4: ("vermelho", "amarelo"), 5: ("tinta", "vermelho")}[t1]
        for k, (dx, dy) in enumerate(((0, 0), (-0.210, 0.060), (0.210, 0.060))[:3 if t1 < 5 else 1]):
            b += _bola(m, f"mina_{k}", saida + Vector((dx, dy, raio)), raio, cor_b, ponta)
        if t1 >= 4:   # faixas de aviso na fabrica
            for k in range(4):
                b.append(pecas.bloco(m, f"aviso_{k}", (0.060, 0.012, 0.120), (-0.180 + 0.120 * k, -0.034, 0.200), "amarelo" if k % 2 == 0 else "tinta", chanfro=0.004))
    else:
        tam = 1.0 + (0.35 if t1 >= 1 else 0) + (0.25 if dura else 0) + (0.35 if (dura and t3 == 5) else 0)
        b += _pilha(m, saida, 9 if t1 == 0 else 14, cor_esp, tam)
    # caminho 2: engrenagens na lateral; depois o rolo triturador e os lancadores
    for k in range(min(t2, 2)):
        c = Vector((0.270, 0.100 + 0.170 * k, 0.200 + 0.050 * k))
        b.append(pecas.cilindro(m, f"engrenagem_{k}", c, (1, 0, 0), [(0, 0), (0.075, 0.004), (0.075, 0.040), (0.030, 0.046), (0, 0.046)], cor="amarelo", seg=10))
        for j in range(6):
            a = math.radians(60 * j)
            b.append(pecas.bloco(m, f"engrenagem_{k}_dente_{j}", (0.040, 0.036, 0.036), c + Vector((0.020, 0.084 * math.cos(a), 0.084 * math.sin(a))), "amarelo", chanfro=0.006))
    if prod:
        b.append(pecas.cilindro(m, "rolo", (-0.200, -0.060, 0.160), (1, 0, 0), [(0, 0), (0.080, 0.004), (0.080, 0.400), (0, 0.404)], cor="aco", seg=10))
        for k in range(10):
            a = math.radians(72 * k)
            b.append(pecas.cone(m, f"rolo_dente_{k}", (-0.160 + 0.080 * (k % 5), -0.060 + 0.070 * math.cos(a), 0.160 + 0.070 * math.sin(a)), (0, math.cos(a), math.sin(a)), 0.026, 0.070, "aco", seg=4))
        if t2 >= 4:   # lancadores no teto
            n = 4 if t2 == 4 else 8
            for k in range(n):
                a = math.radians(360 * k / n + 22)
                d = Vector((math.cos(a) * 0.6, math.sin(a) * 0.6, 1))
                b.append(pecas.cilindro(m, f"lancador_{k}", Vector((0, 0.220, 0.470)) + Vector((math.cos(a), math.sin(a), 0)) * 0.130, d, [(0.034, 0), (0.034, 0.170), (0.020, 0.170), (0, 0.090)],
                                        cores=["cinza", "tinta", "tinta"], seg=6))
    # caminho 3, tier 2: antena de luz ciano
    if t3 >= 2:
        b.append(pecas.tubo(m, "antena", [(-0.200, 0.340, 0.380), (-0.200, 0.340, 0.640)], 0.012, metal, nivel=0))
        b.append(pecas.esfera(m, "antena_luz", (-0.200, 0.340, 0.670), 0.045, "ciano", cortes=0))
    if dura:   # rebites de reforco nos cantos
        for sx in (-1, 1):
            for sy in (-1, 1):
                b.append(pecas.bloco(m, f"reforco_{sx}_{sy}", (0.060, 0.060, 0.360), (0.250 * sx, 0.180 + 0.200 * sy, 0.180), "cinza" if t3 < 5 else "ouro", chanfro=0.012))
    m.por("base", b)
    m.pivo("base", (0, 0, 0))
    m.por("torreta", calha)
    m.pivo("torreta", (0, -0.030, 0.100))


def base(m):
    _fabrica(m)


PARAMETRO = [(None, None)]   # o tier muda a fabrica, que le m.tier
CAMINHOS = [{1: PARAMETRO, 2: PARAMETRO, 3: _fabrica, 4: _fabrica, 5: _fabrica} for _ in range(3)]
