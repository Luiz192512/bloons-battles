"""Macaco Bumerangue: o macaco padrao de crista, cinto laranja e um bumerangue de madeira na mao.

Caminho 1 (glaives): gume de aco, depois glaive; do tier 3 em diante, elmo e mais glaives.
Caminho 2 (rapido): faixa e tenis; do tier 3 em diante, braco mecanico, turbina e armadura.
Caminho 3 (alcance): bumerangue maior e em brasa; do tier 3 em diante, kylie pesado e chapeu de aba.

A arma sai de _armar, que le os tres tiers (regra da torre): o caminho 1 da o gume e a glaive, o
caminho 3 da o tamanho, a brasa e o kylie. Por isso os tiers 1 e 2 desses caminhos nao ocupam encaixe.
"""
import math

from mathutils import Matrix

import pecas

ENQUADRE = ((0, 0.03, 0.45), 1.8)
MX, MY, MZ = pecas.MAO


def _kylie(m, pos, escala, cor, faixa, cabeca=None, serra=None, duplo=False):
    """Kylie: bumerangue pesado, de cabo reto e cabeca virada, com faixas."""
    mat = pecas.em(pos, (0, -1, 0), escala) @ Matrix.Diagonal((1, 1, 0.6, 1))
    lados = (-1, 1) if duplo else (-1,)
    objs = [pecas.tubo(m, "kylie_cabo", [(0, 0.150, 0), (0, -0.060, 0), (0, -0.230, 0)], [0.030, 0.038, 0.046], cor, matriz=mat)]
    for sx in lados:
        objs.append(pecas.tubo(m, f"kylie_cabeca_{sx}", [(0, -0.200, 0), (0.060 * sx, -0.330, 0), (0.170 * sx, -0.420, 0)], [0.046, 0.054, 0.026], cor, matriz=mat))
        if cabeca:
            objs.append(pecas.bloco(m, f"kylie_peso_{sx}", (0.130, 0.110, 0.110), (0.075 * sx, -0.345, 0), cabeca, chanfro=0.020, matriz=mat))
    for k, y in enumerate((0.060, -0.060, -0.170)):
        objs.append(pecas.toro(m, f"kylie_faixa_{k}", (0, y, 0), 0.040, 0.014, faixa, normal=(0, 1, 0), seg=8, lados=4, matriz=mat))
    if serra:
        for k in range(4):
            objs.append(pecas.cone(m, f"kylie_dente_{k}", (0.030, 0.100 - 0.085 * k, 0), (1, -0.4, 0), 0.030, 0.085, serra, seg=4, matriz=mat))
    return objs


def _armar(m):
    """A arma da mao do ataque, conforme os tres tiers."""
    t1, _t2, t3 = m.tier
    esc = 1.35 if t3 >= 1 else 1.0
    brasa = t3 >= 2
    if t3 >= 3:
        cor, faixa = ("vermelho", "branco") if t3 < 5 else ("vermelho", "ouro")
        objs = _kylie(m, (MX, MY - 0.03, MZ + 0.03), 1.0 + 0.18 * (t3 - 3), cor, faixa, cabeca={3: None, 4: "aco", 5: "ouro"}[t3],
                      serra="aco" if t1 >= 2 else None, duplo=t3 >= 5)
        if t1 == 1:   # gume de aco ao longo do cabo
            objs.append(pecas.tubo(m, "kylie_gume", [(MX + 0.034, MY + 0.10, MZ + 0.03), (MX + 0.040, MY - 0.24, MZ + 0.03)], 0.014, "aco", nivel=0))
    elif t1 >= 2:
        lamina = "ouro" if t1 >= 5 else ("laranja" if brasa else "aco")
        miolo = "amarelo" if brasa else "vermelho"
        raio = 0.115 * esc * (1.25 if t1 >= 3 else 1.0)
        objs = pecas.glaive(m, "glaive", (MX + 0.01, MY - 0.03 - raio, MZ + 0.03), raio, lamina=lamina, miolo=miolo, laminas=8 if t1 >= 3 else 6)
    else:
        cor, pontas = ("laranja", "amarelo") if brasa else ("bege", "aco" if t1 else "laranja")
        # seguro por uma das pontas, aberto para fora do corpo
        objs = pecas.bumerangue(m, "bumerangue", (MX + 0.150 * esc, MY - 0.090 * esc, MZ + 0.03), direcao=(0.35, -1, 0), escala=esc, cor=cor, pontas=pontas,
                                gume="aco" if t1 else None)
    m.por("mao_ataque", objs)


def base(m):
    m.macaco(pelagem="crista")
    m.por("tronco", pecas.cinto(m, "laranja"))
    _armar(m)


# ---------------------------------------------------------------- caminho 1: glaives
def ricochete(m):
    _armar(m)
    m.tirar("pelagem")
    m.por("chapeu", pecas.capacete(m, "cinza", "cinza_escuro", crista="aco"))


def moar_glaives(m):
    x, y, z = pecas.MAO_ESQ
    brasa = m.tier[2] >= 2
    lamina, miolo = ("laranja", "amarelo") if brasa else ("aco", "vermelho")
    m.por("mao_livre", pecas.glaive(m, "glaive_esq", (x - 0.02, y - 0.150, z + 0.03), 0.135, lamina=lamina, miolo=miolo, laminas=8))
    m.por("costas", pecas.glaive(m, "glaive_costas", (0, 0.270, 0.740), 0.200, normal=(0, 1, 0.15), lamina=lamina, miolo=miolo, laminas=8))


def senhor_das_glaives(m):
    _armar(m)
    m.tirar("pelagem")
    m.por("chapeu", pecas.capacete(m, "ouro", "vermelho", crista="aco"))
    x, y, z = pecas.MAO_ESQ
    m.por("mao_livre", pecas.glaive(m, "glaive_esq", (x - 0.02, y - 0.150, z + 0.03), 0.135, lamina="ouro", miolo="vermelho", laminas=8))
    m.por("costas", pecas.capa(m, "vermelho", borda="ouro", comp=0.42, largura=0.50))
    orbita = []
    for k in range(3):   # tres glaives grandes girando em volta do macaco (malha propria: gira no jogo)
        a = math.radians(90 + 120 * k)
        orbita += pecas.glaive(m, f"orbita_{k}", (0.60 * math.cos(a), 0.60 * math.sin(a), 0.450), 0.150, lamina="ouro", miolo="vermelho", laminas=8)
    m.por("torreta", orbita)
    m.pivo("torreta", (0, 0, 0.450))


# ---------------------------------------------------------------- caminho 2: rapido e bionico
def faixa(m):
    m.por("chapeu", pecas.faixa_testa(m, "laranja"))


def cachecol(m):
    m.somar("tronco", pecas.cachecol(m, "laranja"))


def munhequeira(m):
    m.somar("mao_livre", pecas.faixa(m, "munhequeira", (-0.258, -0.030, 0.290), 0.060, 0.060, 0.030, "laranja", n=8))


def tenis(m):
    m.por("pes", pecas.tenis(m, "laranja"))


def bionico(m):
    m.por("extra", pecas.manga(m, 1, "cinza", luva="cinza_escuro", anel="ciano", ombreira="cinza_escuro"), grupo="braco", preso=True)
    m.por("rosto", pecas.monoculo(m, "ciano", "cinza"))


def turbo(m):
    m.por("costas", pecas.mochila(m, "cinza", luz="ciano"))
    m.somar("mao_livre", pecas.manga(m, -1, "cinza", luva="cinza_escuro", anel="ciano", ombreira="cinza_escuro"))


def carga_permanente(m):
    m.por("tronco", pecas.colete(m, "cinza", barra="cinza_escuro", emblema="ciano"))
    m.tirar("pelagem")
    m.por("chapeu", pecas.capacete(m, "cinza", "ciano", topo="ciano"))
    m.por("costas", pecas.mochila(m, "cinza", luz="laranja", tam=1.30))


# ---------------------------------------------------------------- caminho 3: alcance e kylie
def kylie(m):
    _armar(m)
    m.tirar("pelagem")
    m.por("chapeu", pecas.chapeu_aba(m, "marrom", fita="vermelho", aba=0.34, tomba=0.30))


def prensa(m):
    _armar(m)
    m.por("tronco", pecas.colete(m, "marrom_escuro", barra="bege"))


def dominacao(m):
    _armar(m)
    m.por("chapeu", pecas.chapeu_aba(m, "tinta", fita="ouro", aba=0.37, tomba=0.30))
    m.por("tronco", pecas.colete(m, "tinta", barra="ouro", emblema="vermelho"))
    m.por("costas", pecas.capa(m, "vermelho", borda="ouro", comp=0.42, largura=0.50))


PARAMETRO = [(None, None)]   # o tier muda a arma, que le m.tier
CAMINHOS = [
    {1: PARAMETRO, 2: PARAMETRO, 3: ricochete, 4: moar_glaives, 5: senhor_das_glaives},
    {1: [("chapeu", faixa), ("tronco", cachecol), ("mao_livre", munhequeira)], 2: [("pes", tenis)], 3: bionico, 4: turbo, 5: carga_permanente},
    {1: PARAMETRO, 2: PARAMETRO, 3: kylie, 4: prensa, 5: dominacao},
]
