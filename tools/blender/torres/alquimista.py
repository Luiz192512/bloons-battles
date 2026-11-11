"""Alquimista: o macaco padrao de tufos, oculos de protecao, avental branco e um frasco de pocao na mao.

Caminho 1 (estimulante): frasco maior e segundo frasco; do tier 3 em diante, cinto de frascos e tanques.
Caminho 2 (transformacao): pocao verde e depois vermelha com fumaca; depois frasco instavel e o monstro.
Caminho 3 (ouro): cinto de frascos e poca de acido; do tier 3 em diante, ouro, cartola e capa.

O frasco da mao sai de _frasco, que le os tres tiers (regra da torre) e vale em qualquer combinacao.
"""
import math

from mathutils import Vector

import pecas

ENQUADRE = ((0, 0.02, 0.48), 1.8)
MX, MY, MZ = pecas.MAO


def _liquido(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    if p == 0 and t1 >= 3:
        return {3: "vermelho", 4: "laranja", 5: "ouro"}[t1]
    if p == 2 and t3 >= 3:
        return "ouro" if t3 < 5 else "rosa"
    return {0: "roxo", 1: "verde", 2: "vermelho", 3: "amarelo", 4: "roxo", 5: "verde"}[t2]


def _frasco(m):
    t1, t2, _t3 = m.tier
    esc = (1.35 if t1 >= 1 else 1.0) * (1.25 if max(m.tier) >= 3 else 1.0)
    pos = Vector((MX, MY - 0.03, MZ + 0.050))
    objs = [pecas.frasco(m, "frasco", pos, _liquido(m), escala=esc)]
    if t2 >= 2:   # fumaca (ou faiscas, no instavel) saindo do gargalo
        cor = "tinta" if t2 == 2 else ("vermelho" if t2 == 3 else "verde")
        for k, dx in enumerate((-0.4, 0.1, 0.5)):
            objs.append(pecas.cone(m, f"frasco_fumaca_{k}", pos + Vector((0, 0, 0.200 * esc)), (dx, 0.1 * k, 1), 0.026 * esc, (0.110 + 0.030 * k) * esc, cor, seg=4, fechado=True))
    m.por("mao_ataque", objs)


def base(m):
    m.macaco(pelagem="tufos")
    m.por("rosto", pecas.oculos(m, aro="cinza_escuro", tira="marrom_escuro"))
    m.por("tronco", pecas.colete(m, "branco", barra="roxo", z0=0.230, z1=0.515))
    _frasco(m)


# ---------------------------------------------------------------- caminho 1: estimulante
def frasco_esq(m):
    x, y, z = pecas.MAO_ESQ
    m.somar("mao_livre", pecas.frasco(m, "frasco_esq", (x - 0.01, y - 0.03, z + 0.050), "verde", escala=1.15))


def frasco_chao(m):
    m.somar("extra", pecas.frasco(m, "frasco_chao", (-0.430, -0.200, 0.0), "verde", escala=1.6), preso=True)


def _cinto_frascos(m, cor, n=4):
    objs = pecas.bandoleira(m, "marrom_escuro")
    for k in range(n):
        a = math.radians(-140 + 100 * k / max(1, n - 1))
        objs.append(pecas.frasco(m, f"cinto_frasco_{k}", (0.190 * math.cos(a), -0.005 + 0.176 * math.sin(a) - 0.024, 0.350 - 0.105 * math.cos(a)), cor, escala=0.55))
    return objs


def estimulante(m):
    t = m.tier[0]
    cor = _liquido(m)
    m.somar("tronco", _cinto_frascos(m, cor))
    if t >= 4:
        m.por("costas", pecas.tanque(m, cor, tampa="cinza", lado=-1 if t == 5 else 0, raio=0.095, altura=0.340) +
              (pecas.tanque(m, cor, tampa="cinza", lado=1, raio=0.095, altura=0.340, nome="tanque_2") if t == 5 else []))
    if t == 5:
        m.por("tronco", pecas.colete(m, "branco", barra="ouro", emblema="ouro", z0=0.230, z1=0.515) + _cinto_frascos(m, cor))
    _frasco(m)


# ---------------------------------------------------------------- caminho 2: transformacao
def transformacao(m):
    t = m.tier[1]
    m.pelagem("crista")   # o cabelo arrepia com a mistura instavel
    if t >= 4:   # o monstro: bracos e peito roxos, garras e chifres
        m.por("mao_livre", pecas.manga(m, -1, "roxo", luva="roxo"))
        m.por("extra", pecas.manga(m, 1, "roxo", luva="roxo"), grupo="braco", preso=True)
        m.por("chapeu", pecas.chifres(m, "bege", comp=0.160 if t == 4 else 0.260))
        m.por("tronco", pecas.colete(m, "roxo", barra="tinta", z0=0.230, z1=0.515, folga=1.0 if t == 4 else 1.12))
    if t == 5:
        m.por("costas", pecas.capa(m, "tinta", borda="verde", comp=0.42, largura=0.52, gola=False))
        m.por("rosto", pecas.viseira(m, "verde", "tinta"))
    _frasco(m)


# ---------------------------------------------------------------- caminho 3: ouro
def cinto(m):
    m.somar("tronco", _cinto_frascos(m, "verde", n=3))


def botas(m):
    m.por("pes", pecas.tenis(m, "roxo", sola="branco", nome="bota"))


def poca(m):
    objs = [pecas.esfera(m, "poca", (0.020, -0.420, 0.006), (0.260, 0.170, 0.020), "verde")]
    for k, (x, y, r) in enumerate(((0.320, -0.330, 0.050), (-0.300, -0.360, 0.060))):
        objs.append(pecas.esfera(m, f"poca_gota_{k}", (x, y, 0.006), (r, r * 0.8, 0.016), "verde", cortes=0))
    m.somar("extra", objs)


def ouro(m):
    t = m.tier[2]
    pilha = []
    for k, (x, y) in enumerate(((-0.460, 0.050), (-0.520, -0.130), (0.480, 0.120), (0.500, -0.100), (-0.380, -0.270))[:2 + (t - 3) * 2 - (1 if t == 5 else 0)]):
        pilha.append(pecas.cone(m, f"ouro_{k}", (x, y, 0), (0, 0, 1), 0.085, 0.140, "ouro", seg=5, fechado=True))
    m.somar("extra", pilha)
    if t >= 4:
        m.tirar("pelagem")
        m.por("chapeu", pecas.chapeu_aba(m, "tinta", fita="ouro", aba=0.300, copa=0.230, raio=0.215, tomba=0.25))
    if t == 5:
        m.por("costas", pecas.capa(m, "roxo", borda="ouro", comp=0.44, largura=0.52))
        m.por("tronco", pecas.colete(m, "roxo", barra="ouro", emblema="ouro", z0=0.230, z1=0.515))
    _frasco(m)


PARAMETRO = [(None, None)]   # o tier muda o frasco, que le m.tier
CAMINHOS = [
    {1: PARAMETRO, 2: [("mao_livre", frasco_esq), ("extra", frasco_chao)], 3: estimulante, 4: estimulante, 5: estimulante},
    {1: PARAMETRO, 2: PARAMETRO, 3: transformacao, 4: transformacao, 5: transformacao},
    {1: [("tronco", cinto), ("pes", botas)], 2: [("extra", poca)], 3: ouro, 4: ouro, 5: ouro},
]
