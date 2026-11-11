"""Macaco de Gelo: o macaco padrao na unica paleta diferente (pelo azul claro, pele quase branca),
de topete, cachecol branco e um cristal de gelo na mao.

Caminho 1 (fragil): anel de gelo no chao e pingentes; do tier 3 em diante, cristais e coroa.
Caminho 2 (congelamento): segundo cristal e botas; do tier 3 em diante, gorro, floco e capa.
Caminho 3 (canhao): cristal maior e viseira; do tier 3 em diante, canhao criogenico com tanque.
"""
import math

from mathutils import Vector

import pecas
import poli

ENQUADRE = ((0, 0.02, 0.45), 1.7)
MX, MY, MZ = pecas.MAO
CORES = dict(pelo="pelo_gelo", pele="pele_gelo", pelo_escuro="pelo_gelo_escuro")


def _cristal(m, nome, pos, raio, altura, cor="ciano", direcao=(0, 0, 1)):
    """Cristal de gelo: prisma de seis lados com as duas pontas vivas."""
    bm = poli.torno([(0, -altura * 0.35), (raio, 0), (raio * 0.85, altura * 0.45), (0, altura)], seg=6, cor=cor)
    return m.obj(nome, poli.orientar(bm, direcao, pos), nivel=0, vivo=True)


def _cristal_mao(m):
    """O cristal da mao do ataque; o caminho 3 aumenta (regra da torre)."""
    e = 1.45 if m.tier[2] >= 1 else 1.0
    objs = [_cristal(m, "cristal", (MX, MY - 0.03, MZ + 0.07), 0.060 * e, 0.200 * e)]
    if m.tier[0] >= 3:   # estilhacos: dois cristais menores ao lado
        for k, (dx, dy) in enumerate(((0.075, -0.02), (-0.055, -0.07))):
            objs.append(_cristal(m, f"cristal_{k}", (MX + dx, MY - 0.03 + dy, MZ + 0.06), 0.038 * e, 0.130 * e, cor="branco", direcao=(dx * 4, dy * 2, 1)))
    return objs


def base(m):
    m.macaco(pelagem="topete", **CORES)
    m.por("tronco", pecas.lenco(m, "branco"))
    m.por("mao_ataque", _cristal_mao(m))


# ---------------------------------------------------------------- caminho 1: fragil
def anel_de_gelo(m):
    objs = [pecas.toro(m, "anel_gelo", (0, 0, 0.030), 0.390, 0.036, "ciano", seg=18, lados=4)]
    for k in range(6):
        a = math.radians(60 * k + 30)
        d = Vector((math.cos(a), math.sin(a), 0))
        objs.append(pecas.cone(m, f"anel_gelo_ponta_{k}", d * 0.390, d * 0.5 + Vector((0, 0, 1)), 0.040, 0.110, "branco", seg=4))
    m.somar("extra", objs)


def pingentes_costas(m):
    objs = []
    for k, (x, h) in enumerate(((-0.130, 0.300), (0.0, 0.400), (0.130, 0.300))):
        objs.append(pecas.cone(m, f"pingente_{k}", (x, 0.190, 0.520), (x * 1.6, 0.55, 1), 0.060, h, "ciano" if k % 2 == 0 else "branco", seg=5, fechado=True))
    m.somar("costas", objs)


def pingentes_chao(m):
    objs = []
    for k, (x, y, h) in enumerate(((-0.460, 0.120, 0.260), (-0.520, -0.040, 0.180), (-0.420, 0.270, 0.170))):
        objs.append(pecas.cone(m, f"pingente_chao_{k}", (x, y, 0), (-0.15, 0.05, 1), 0.065, h, "ciano" if k != 1 else "branco", seg=5, fechado=True))
    m.somar("extra", objs)


def estilhacos(m):
    m.por("mao_ataque", _cristal_mao(m))
    objs = []
    for sx in (-1, 1):   # ombreiras de cristal
        for k, (dx, h) in enumerate(((0.0, 0.170), (0.065, 0.120))):
            objs.append(pecas.cone(m, f"ombreira_{sx}_{k}", ((0.170 + dx) * sx, 0.010, 0.500), (0.55 * sx, 0.0, 1), 0.050, h, "ciano" if k == 0 else "branco", seg=5,
                                   fechado=True))
    m.somar("tronco", objs)


def fragilizacao(m):
    m.tirar("pelagem")
    m.por("chapeu", pecas.coroa(m, "ciano", "branco", raio=0.215, z=0.900, pontas=6))
    m.somar("tronco", pecas.colete(m, "ciano", barra="branco", z0=0.290, z1=0.500))


def super_fragil(m):
    m.por("chapeu", pecas.coroa(m, "branco", "ciano", raio=0.235, z=0.890, pontas=8))
    m.por("costas", pecas.capa(m, "branco", borda="ciano", comp=0.44, largura=0.52, gola=False))
    orbita = []
    for k in range(4):   # quatro cristais grandes girando em volta (malha propria: gira no jogo)
        a = math.radians(45 + 90 * k)
        orbita.append(_cristal(m, f"orbita_{k}", (0.560 * math.cos(a), 0.560 * math.sin(a), 0.330), 0.085, 0.330, cor="ciano" if k % 2 == 0 else "branco"))
    m.por("torreta", orbita)
    m.pivo("torreta", (0, 0, 0.330))


# ---------------------------------------------------------------- caminho 2: congelamento
def segundo_cristal(m):
    x, y, z = pecas.MAO_ESQ
    m.somar("mao_livre", _cristal(m, "cristal_esq", (x - 0.02, y - 0.05, z + 0.08), 0.072, 0.240, cor="azul"))


def botas(m):
    m.por("pes", pecas.tenis(m, "azul", sola="branco", nome="bota"))


def _floco(m, z, raio, cor, nome="floco"):
    """Floco de neve deitado sobre a cabeca: seis bracos com pontas."""
    objs = [pecas.esfera(m, nome, (0, 0, z), (raio * 0.22, raio * 0.22, 0.020), cor, cortes=0)]
    for k in range(6):
        a = math.radians(60 * k)
        d = Vector((math.cos(a), math.sin(a), 0))
        objs.append(pecas.tubo(m, f"{nome}_braco_{k}", [Vector((0, 0, z)), Vector((0, 0, z)) + d * raio], 0.016, cor, nivel=0))
        objs.append(pecas.cone(m, f"{nome}_ponta_{k}", Vector((0, 0, z)) + d * raio * 0.80, d, raio * 0.22, raio * 0.42, cor, seg=4))
    return objs


def vento_artico(m):
    m.tirar("pelagem")
    gorro = [pecas.casca(m, "gorro", "azul", pecas.piso_capacete), pecas.barra_casca(m, "gorro_barra", "branco", pecas.piso_capacete, tubo_r=0.034)]
    gorro.append(pecas.esfera(m, "gorro_pompom", (0, 0.020, 1.000), 0.078, "branco", cortes=0))
    m.por("chapeu", gorro)
    aura = [pecas.toro(m, "aura", (0, 0, 0.022), 0.560, 0.026, "branco", seg=20, lados=4)]
    for k in range(8):
        a = math.radians(45 * k)
        d = Vector((math.cos(a), math.sin(a), 0))
        aura.append(pecas.cone(m, f"aura_sopro_{k}", d * 0.560, Vector((-d.y, d.x, 0.25)), 0.036, 0.170, "ciano", seg=4))
    m.somar("extra", aura)


def nevasca(m):
    m.por("costas", pecas.capa(m, "branco", borda="azul", comp=0.46, largura=0.54))
    m.somar("chapeu", _floco(m, 1.170, 0.200, "branco"))


def zero_absoluto(m):
    m.por("costas", pecas.capa(m, "azul", borda="ciano", comp=0.50, largura=0.60))
    m.tirar("chapeu", prefixo="floco")
    m.somar("chapeu", _floco(m, 1.190, 0.290, "ciano"))
    pontas = []
    for k in range(8):
        a = math.radians(45 * k + 22.5)
        d = Vector((math.cos(a), math.sin(a), 0))
        pontas.append(pecas.cone(m, f"aura_gelo_{k}", d * 0.560, d * 0.4 + Vector((0, 0, 1)), 0.060, 0.230, "branco", seg=5, fechado=True))
    m.somar("extra", pontas)


# ---------------------------------------------------------------- caminho 3: canhao criogenico
def cristal_maior(m):
    m.por("mao_ataque", _cristal_mao(m))


def cristal_grande_chao(m):
    """Alternativa quando a mao do ataque esta ocupada: um cristal grande fincado no chao, a direita."""
    m.somar("extra", _cristal(m, "cristal_chao", (0.480, 0.060, 0.090), 0.090, 0.330))


def viseira(m):
    m.por("rosto", pecas.viseira(m, "ciano", "azul"))


def _canhao(m, escala, cor, anel, pingente=None):
    """Canhao criogenico segurado na mao, com a mangueira ate o tanque das costas."""
    mat = pecas.em((MX, MY + 0.02, MZ + 0.03), (0, -1, 0), escala)
    perfil = [(0, -0.100), (0.060, -0.090), (0.072, 0.020), (0.060, 0.250), (0.090, 0.270), (0.090, 0.380), (0.052, 0.380), (0.046, 0.250), (0, 0.250)]
    cores = [cor, cor, cor, anel, anel, "ciano", "ciano", "ciano"]
    objs = [pecas.cilindro(m, "canhao", (0, 0, 0), (0, -1, 0), perfil, cores=cores, seg=10, matriz=mat)]
    if pingente:
        objs.append(pecas.cone(m, "canhao_pingente", (0, -0.340, 0), (0, -1, 0), 0.050, pingente, "branco", seg=6, matriz=mat))
    objs.append(pecas.tubo(m, "mangueira", [(MX, MY + 0.10, MZ + 0.02), (MX + 0.02, 0.060, 0.330), (0.130, 0.190, 0.340)], 0.020, "cinza_escuro"))
    return objs


def canhao_criogenico(m):
    m.por("mao_ataque", _canhao(m, 1.0, "azul", "branco"))
    m.por("costas", pecas.tanque(m, "ciano", tampa="azul"))


def pingentes(m):
    m.por("mao_ataque", _canhao(m, 1.18, "azul", "branco", pingente=0.260))
    m.por("costas", pecas.tanque(m, "ciano", tampa="azul", lado=-1) + pecas.tanque(m, "ciano", tampa="azul", lado=1, nome="tanque_2"))
    m.tirar("pelagem")
    m.por("chapeu", pecas.capacete(m, "azul", "branco", topo="ciano"))


def empalar(m):
    m.por("mao_ataque", _canhao(m, 1.40, "tinta", "ouro", pingente=0.320))
    m.por("chapeu", pecas.capacete(m, "tinta", "ouro", topo="ciano"))
    m.somar("tronco", pecas.colete(m, "tinta", barra="ouro", emblema="ciano", z0=0.290, z1=0.500))


CAMINHOS = [
    {1: [("extra", anel_de_gelo)], 2: [("costas", pingentes_costas), ("extra", pingentes_chao)], 3: estilhacos, 4: fragilizacao, 5: super_fragil},
    {1: [("mao_livre", segundo_cristal)], 2: [("pes", botas)], 3: vento_artico, 4: nevasca, 5: zero_absoluto},
    {1: [("mao_ataque", cristal_maior), ("extra", cristal_grande_chao)], 2: [("rosto", viseira)], 3: canhao_criogenico, 4: pingentes, 5: empalar},
]
