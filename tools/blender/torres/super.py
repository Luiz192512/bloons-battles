"""Super Macaco: o macaco padrao, o unico maior (escala 1,25), de topete, traje azul com estrela e capa.

Caminho 1 (sol): viseira de laser e de plasma; do tier 3 em diante, traje dourado, disco do sol e o templo.
Caminho 2 (robo): capa maior e aura; do tier 3 em diante, bracos e capacete de metal, canhoes de ombro.
Caminho 3 (trevas): luvas e oculos; do tier 3 em diante, armadura preta, laminas e o aro escuro.

Rosto, capa, luvas e aura saem de _corpo, que le os tres tiers (regra da torre).
"""
import math

from mathutils import Matrix, Vector

import pecas

ENQUADRE = ((0, 0.05, 0.88), 2.9)
ESCALA = 1.25
MX, MY, MZ = pecas.MAO


def _corpo(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    sol, robo, trevas = (p == 0 and t1 >= 3), (p == 1 and t2 >= 3), (p == 2 and t3 >= 3)
    # rosto: viseira de laser (vermelha) ou de plasma (roxa); Ultravisao troca a tira por ciano ou vira oculos
    cor = {0: None, 1: "vermelho", 2: "roxo"}.get(t1, "amarelo")
    if robo:
        cor = "verde" if t2 < 5 else "vermelho"
    if cor:
        m.por("rosto", pecas.viseira(m, cor, "ciano" if t3 >= 2 else "cinza_escuro"))
    elif t3 >= 2:
        m.por("rosto", pecas.oculos(m, aro="ciano", tira="tinta"))
    # capa: cresce com Super Alcance; cada caminho principal tem a sua
    if not robo:
        capa, borda = "azul", "amarelo"
        if sol:
            capa, borda = ("amarelo", "laranja") if t1 < 5 else ("branco", "ouro")
        elif trevas:
            capa, borda = ("tinta", "roxo") if t3 < 5 else ("roxo", "ciano")
        maior = 0.12 if t2 >= 1 else 0.0
        m.por("costas", pecas.capa(m, capa, borda=borda, comp=0.46 + maior, largura=0.54 + maior))
    # Repulsao: luvas grandes nas duas maos
    if t3 >= 1 and not robo:
        luva = "laranja" if not trevas else "roxo"
        m.por("mao_livre", pecas.esfera(m, "luva_e", pecas.MAO_ESQ, 0.088, luva))
        m.por("extra", pecas.esfera(m, "luva_d", pecas.MAO, 0.088, luva), grupo="braco", preso=True)
    # Alcance Epico: aro de energia no chao
    if t2 >= 2 and not robo:
        m.somar("extra", pecas.aro_chao(m, "aura", 0.660, "amarelo" if not trevas else "roxo"))


def _dardo(m):
    m.por("mao_ataque", pecas.dardo(m, "dardo", (MX, MY, MZ + 0.02)))


def base(m):
    m.macaco(pelagem="topete")
    m.matriz = Matrix.Scale(ESCALA, 4)
    m.por("tronco", pecas.colete(m, "azul", barra="amarelo", z0=0.280, z1=0.515) + [pecas.estrela(m, "emblema", (0, -0.186, 0.410), 0.075, "ouro", normal=(0, -1, 0.1))])
    _dardo(m)
    _corpo(m)


# ---------------------------------------------------------------- caminho 1: sol
def _disco_sol(m, raio, cor, raios_cor, z=0.800):
    """Disco do sol em pe, atras da cabeca, com raios."""
    c = Vector((0, 0.290, z))
    objs = [pecas.cilindro(m, "sol", c, (0, 1, 0), [(0, 0), (raio, 0.004), (raio, 0.030), (0, 0.034)], cor=cor, seg=14)]
    for k in range(10):
        a = 2 * math.pi * k / 10
        d = Vector((math.cos(a), 0, math.sin(a)))
        objs.append(pecas.cone(m, f"sol_raio_{k}", c + d * raio * 0.95 + Vector((0, 0.015, 0)), d, raio * 0.20, raio * 0.55, raios_cor, seg=4, fechado=True))
    return objs


def _templo(m, degraus, cor, detalhe):
    """Templo em degraus; o macaco fica de pe no alto."""
    objs, z, lado = [], 0.0, 1.150 if degraus == 3 else 1.300
    for k in range(degraus):
        alt = 0.170
        objs.append(pecas.bloco(m, f"templo_{k}", (lado, lado, alt), (0, 0.050, z + alt / 2), cor if k % 2 == 0 else detalhe, chanfro=0.030))
        z, lado = z + alt, lado - 0.260
    if degraus > 3:   # obeliscos nos cantos
        for sx in (-1, 1):
            for sy in (-1, 1):
                objs.append(pecas.cone(m, f"obelisco_{sx}_{sy}", (0.560 * sx, 0.050 + 0.560 * sy, 0.170), (0, 0, 1), 0.070, 0.420, detalhe, seg=4, fechado=True))
    return objs, z


def sol(m):
    t = m.tier[0]
    cor, barra = ("amarelo", "laranja") if t < 5 else ("branco", "ouro")
    m.por("tronco", pecas.colete(m, cor, barra=barra, emblema="laranja" if t < 5 else "ouro", z0=0.280, z1=0.515))
    m.tirar("pelagem")
    m.por("chapeu", pecas.coroa(m, "ouro", "laranja", raio=0.215, z=0.905, pontas=7))
    m.somar("costas", _disco_sol(m, 0.300 if t == 3 else 0.360, "ouro", "laranja" if t < 5 else "branco"))
    if t >= 4:
        blocos, topo = _templo(m, 3 if t == 4 else 4, "ouro", "bege" if t == 4 else "branco")
        m.por("base", blocos)
        m.pivo("base", (0, 0, 0))
        m.matriz = Matrix.Translation((0, 0.050, topo)) @ Matrix.Scale(ESCALA, 4)


# ---------------------------------------------------------------- caminho 2: robo
def robo(m):
    t = m.tier[1]
    metal, luz = ("cinza", "verde") if t == 3 else (("verde_escuro", "verde") if t == 4 else ("tinta", "vermelho"))
    m.tirar("pelagem")
    m.por("chapeu", pecas.capacete(m, metal, luz, topo=luz))
    m.por("mao_livre", pecas.manga(m, -1, metal, luva="cinza_escuro", anel=luz, ombreira=metal))
    m.por("extra", pecas.manga(m, 1, metal, luva="cinza_escuro", anel=luz, ombreira=metal), grupo="braco", preso=True)
    m.por("tronco", pecas.colete(m, metal, barra=luz, emblema=luz, z0=0.280, z1=0.515))
    m.tirar("costas")
    if t >= 4:
        m.por("costas", pecas.mochila(m, metal, luz=luz, tam=1.15))
        for sx in (-1, 1):   # canhoes de ombro
            L = 0.300 if t == 4 else 0.420
            m.somar("costas", pecas.cilindro(m, f"canhao_ombro_{sx}", (0.200 * sx, 0.080, 0.640), (0, -1, 0.05), [(0.050, 0), (0.050, L - 0.050), (0.068, L - 0.044), (0.068, L), (0.034, L), (0.030, L * 0.5), (0, L * 0.5)],
                                             cores=["cinza_escuro", "cinza_escuro", luz, luz, "tinta", "tinta"], seg=8))
    if t == 5:
        m.somar("chapeu", pecas.chifres(m, "vermelho", comp=0.220))


# ---------------------------------------------------------------- caminho 3: trevas
def trevas(m):
    t = m.tier[2]
    m.por("tronco", pecas.colete(m, "tinta", barra="roxo", emblema="roxo" if t < 5 else "ciano", z0=0.280, z1=0.515))
    m.tirar("pelagem")
    m.por("chapeu", pecas.capacete(m, "tinta", "roxo") + pecas.chifres(m, "tinta" if t < 5 else "roxo", comp=0.150 + 0.050 * (t - 3)))
    raio = 0.110 + 0.025 * (t - 3)
    m.por("mao_ataque", pecas.glaive(m, "lamina", (MX + 0.01, MY - 0.05 - raio, MZ + 0.03), raio, lamina="aco", miolo="roxo", laminas=6))
    if t >= 4:
        x, y, z = pecas.MAO_ESQ
        m.somar("costas", pecas.glaive(m, "lamina_e", (x - 0.02, y - 0.06 - raio, z + 0.05), raio, lamina="aco", miolo="roxo", laminas=6))
    if t == 5:
        m.somar("extra", pecas.aro_chao(m, "vortice", 0.560, "roxo", pontas="ciano", n=8, altura=0.200))


PARAMETRO = [(None, None)]   # o tier muda rosto, capa, luvas ou aura, que saem de _corpo (le m.tier)
CAMINHOS = [
    {1: PARAMETRO, 2: PARAMETRO, 3: sol, 4: sol, 5: sol},
    {1: PARAMETRO, 2: PARAMETRO, 3: robo, 4: robo, 5: robo},
    {1: PARAMETRO, 2: PARAMETRO, 3: trevas, 4: trevas, 5: trevas},
]
