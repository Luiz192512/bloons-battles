"""Macaco Ninja: o macaco padrao de capuz preto com barra vermelha, cinto vermelho e uma shuriken na mao.

Caminho 1 (shurikens): sandalias e shuriken maior; do tier 3 em diante, mais shurikens e traje claro.
Caminho 2 (sabotagem): bomba de fumaca e viseira; do tier 3 em diante, estandartes nas costas.
Caminho 3 (bombas): shuriken acesa e estrepes; do tier 3 em diante, bombas cada vez maiores.

A shuriken sai de _shuriken, que le os tres tiers (regra da torre) e vale em qualquer combinacao.
"""
import math

from mathutils import Vector

import pecas

ENQUADRE = ((0, 0.02, 0.48), 1.8)
MX, MY, MZ = pecas.MAO


def _uma(m, nome, pos, normal=(0, 0, 1)):
    t1, _t2, t3 = m.tier
    raio = 0.085 if t1 < 2 else 0.112
    lamina = "ouro" if t1 >= 5 else "aco"
    return pecas.glaive(m, nome, pos, raio, normal=normal, lamina=lamina, miolo="ciano" if t3 >= 1 else "cinza_escuro", laminas=4 if t1 < 2 else 6)


def _shuriken(m):
    t1 = m.tier[0]
    objs = _uma(m, "shuriken", (MX + 0.02, MY - 0.130, MZ + 0.03))
    if t1 >= 4:   # leque de tres na mao
        for k, dx in enumerate((-0.170, 0.170)):
            objs += _uma(m, f"shuriken_{k}", (MX + 0.02 + dx, MY - 0.090, MZ + 0.05 + 0.03 * k))
    m.por("mao_ataque", objs)
    if t1 >= 3:   # Tiro Duplo: outra na mao esquerda
        x, y, z = pecas.MAO_ESQ
        m.por("mao_livre", _uma(m, "shuriken_esq", (x - 0.03, y - 0.130, z + 0.04)))


def base(m):
    m.macaco(pelagem="lisa")
    m.por("chapeu", pecas.capuz(m, "tinta", barra="vermelho"))
    m.por("tronco", pecas.cinto(m, "vermelho", fivela="tinta"))
    _shuriken(m)


# ---------------------------------------------------------------- caminho 1: shurikens
def sandalias(m):
    m.por("pes", pecas.tenis(m, "vermelho", sola="tinta", nome="sandalia"))


def mestre(m):
    t = m.tier[0]
    cor, barra = {3: ("azul", "branco"), 4: ("branco", "azul"), 5: ("branco", "ouro")}[t]
    m.por("chapeu", pecas.capuz(m, cor, barra=barra))
    if t >= 4:
        m.por("tronco", pecas.colete(m, cor, barra=barra, z0=0.270, z1=0.515))
    if t == 5:   # roda de shurikens nas costas
        m.por("costas", pecas.capa(m, "branco", borda="ouro", comp=0.40, largura=0.46, gola=False))
        for k in range(6):
            a = math.radians(60 * k + 30)
            m.somar("costas", pecas.glaive(m, f"roda_{k}", (0.330 * math.cos(a), 0.300, 0.760 + 0.330 * math.sin(a)), 0.075, normal=(0, 1, 0), lamina="ouro", miolo="vermelho", laminas=4))
    _shuriken(m)


# ---------------------------------------------------------------- caminho 2: sabotagem
def _fumaca(m, pos):
    return [pecas.esfera(m, "fumaca", pos, 0.070, "cinza", cortes=0), pecas.tubo(m, "fumaca_pavio", [Vector(pos) + Vector((0, 0, 0.060)), Vector(pos) + Vector((0.020, 0, 0.120))], 0.012, "bege", nivel=0)]


def fumaca_mao(m):
    x, y, z = pecas.MAO_ESQ
    m.somar("mao_livre", _fumaca(m, (x - 0.01, y - 0.03, z + 0.09)))


def fumaca_chao(m):
    m.somar("extra", _fumaca(m, (-0.430, -0.180, 0.070)), preso=True)


def viseira(m):
    m.por("rosto", pecas.viseira(m, "verde", "tinta"))


def _estandarte(m, lado, cor, nome):
    x = 0.130 * lado
    return [pecas.tubo(m, nome, [(x, 0.200, 0.300), (x * 1.6, 0.260, 1.250)], 0.016, "marrom_escuro", nivel=0),
            pecas.bloco(m, nome + "_pano", (0.180, 0.016, 0.340), (x * 1.5 + 0.100 * lado, 0.255, 1.050), cor, chanfro=0.006)]


def sabotagem(m):
    t = m.tier[1]
    cor, barra = ("verde_escuro", "verde") if t < 5 else ("tinta", "verde")
    m.por("chapeu", pecas.capuz(m, cor, barra=barra))
    bandeira = "verde" if t < 5 else "ciano"
    m.por("costas", _estandarte(m, 1, bandeira, "estandarte") + (_estandarte(m, -1, bandeira, "estandarte_2") if t >= 4 else []))
    if t >= 4:
        m.por("tronco", pecas.colete(m, cor, barra=barra, z0=0.270, z1=0.515))


# ---------------------------------------------------------------- caminho 3: bombas
def estrepes(m):
    objs = []
    for k, (x, y) in enumerate(((-0.200, -0.460), (0.0, -0.540), (0.200, -0.470), (-0.100, -0.360), (0.110, -0.380))):
        for j, d in enumerate(((0, 0, 1), (0.9, 0, -0.3), (-0.5, 0.8, -0.3), (-0.5, -0.8, -0.3))):
            objs.append(pecas.cone(m, f"estrepe_{k}_{j}", (x, y, 0.030), d, 0.016, 0.060, "aco", seg=4))
    m.somar("extra", objs)


def bombas(m):
    t = m.tier[2]
    cor, pino, raio = {3: ("branco", "amarelo", 0.085), 4: ("rosa", "roxo", 0.110), 5: ("roxo", "ouro", 0.135)}[t]
    x, y, z = pecas.MAO_ESQ
    c = Vector((x - 0.03, y - 0.05, z + 0.03 + raio))
    bomba = [pecas.esfera(m, "bomba", c, raio, cor), pecas.tubo(m, "bomba_pavio", [c + Vector((0, 0, raio * 0.9)), c + Vector((0.030, 0, raio * 1.6))], 0.014, "bege", nivel=0)]
    for k in range(6 if t >= 4 else 0):   # pinos da bomba grudenta
        a = math.radians(60 * k)
        d = Vector((math.cos(a), math.sin(a), 0.2))
        bomba.append(pecas.cone(m, f"bomba_pino_{k}", c + d * raio * 0.85, d, raio * 0.22, raio * 0.55, pino, seg=4))
    m.por("mao_livre", bomba)
    cinto = pecas.bandoleira(m, "marrom_escuro")
    for k, a in enumerate((-130, -90, -50)):
        a = math.radians(a)
        cinto.append(pecas.esfera(m, f"cinto_bomba_{k}", (0.190 * math.cos(a), -0.005 + 0.176 * math.sin(a) - 0.020, 0.400 - 0.105 * math.cos(a)), 0.042, cor, cortes=0))
    m.por("tronco", cinto)
    if t >= 4:
        m.por("chapeu", pecas.capuz(m, "laranja", barra="tinta" if t == 4 else "ouro"))
        m.por("costas", pecas.mochila(m, "marrom_escuro", detalhe="tinta"))
    _shuriken(m)


PARAMETRO = [(None, None)]   # o tier muda a shuriken, que le m.tier
CAMINHOS = [
    {1: [("pes", sandalias)], 2: PARAMETRO, 3: mestre, 4: mestre, 5: mestre},
    {1: [("mao_livre", fumaca_mao), ("extra", fumaca_chao)], 2: [("rosto", viseira)], 3: sabotagem, 4: sabotagem, 5: sabotagem},
    {1: PARAMETRO, 2: [("extra", estrepes)], 3: bombas, 4: bombas, 5: bombas},
]
