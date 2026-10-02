"""Macaco Mago: o macaco padrao de chapeu pontudo azul, tunica azul e cajado com orbe.

Caminho 1 (arcano): orbe roxo cada vez maior; do tier 3 em diante, traje roxo e dourado e cajado com garras.
Caminho 2 (fogo): bola de fogo na mao e aro de chamas; depois traje vermelho e a fenix.
Caminho 3 (sombras): orbes pequenos e monoculo; depois faiscas, traje preto e lapides.

O cajado sai de _cajado, que le os tres tiers (regra da torre) e vale em qualquer combinacao.
"""
import math

from mathutils import Vector

import pecas

ENQUADRE = ((0, 0.03, 0.50), 1.8)


def _cajado(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    orbe, raio, cor, garra = "azul", 0.070, "marrom", None
    if t1 >= 1:
        orbe, raio = "roxo", 0.070 + 0.018 * min(t1, 2)
    if p == 0 and t1 >= 3:
        cor, garra, raio = "ouro", "ouro", {3: 0.110, 4: 0.125, 5: 0.150}[t1]
        orbe = "roxo" if t1 < 5 else "ciano"
    elif p == 1 and t2 >= 3:
        cor, garra, orbe, raio = "marrom_escuro", "ouro", "laranja", {3: 0.095, 4: 0.105, 5: 0.120}[t2]
    elif p == 2 and t3 >= 4:
        cor, garra, orbe, raio = "tinta", "aco", "verde", {4: 0.100, 5: 0.125}[t3]
    objs, topo = pecas.cajado(m, cor=cor, orbe=orbe, raio=raio, garra=garra)
    if t1 >= 2:   # Explosao Arcana: anel em volta do orbe
        objs.append(pecas.toro(m, "cajado_anel", topo, raio * 1.45, raio * 0.16, "ouro", normal=(0.3, 0, 1), seg=12, lados=4))
    if p == 0 and t1 >= 4:   # Espinho Arcano: ponta no alto
        objs.append(pecas.cone(m, "cajado_espinho", topo + Vector((0, 0, raio * 0.8)), (0, 0, 1), raio * 0.45, raio * 2.2, "ouro" if t1 == 4 else "branco", seg=5))
    if p == 1 and t2 >= 3:   # chama sobre o orbe
        objs += pecas.chama(m, "cajado_chama", topo + Vector((0, 0, raio * 0.6)), raio * 2.2)
    if t3 >= 1:   # Magia Intensa: dois orbes pequenos ao lado do grande
        for k, sx in enumerate((-1, 1)):
            objs.append(pecas.esfera(m, f"cajado_orbe_{k}", topo + Vector((sx * raio * 1.9, 0, raio * 0.3)), raio * 0.42, "ciano", cortes=0))
    m.por("mao_ataque", objs)


def _traje(m, cor, fita, barra, capa=None, chapeu=True):
    if chapeu:
        m.tirar("pelagem")
        m.por("chapeu", pecas.chapeu_cone(m, cor, fita=fita))
    m.por("tronco", pecas.colete(m, cor, barra=fita, z0=0.300, z1=0.515) + pecas.saia(m, cor, barra=barra))
    if capa:
        m.por("costas", pecas.capa(m, capa, borda=fita, comp=0.42, largura=0.50))


def base(m):
    m.macaco(pelagem="lisa")
    _traje(m, "azul", "ouro", "ouro")
    _cajado(m)


# ---------------------------------------------------------------- caminho 1: arcano
def arcano(m):
    t = m.tier[0]
    _traje(m, "roxo", "ouro", "ouro", capa={3: None, 4: "roxo", 5: "branco"}[t])
    if t == 5:
        m.somar("chapeu", pecas.estrela(m, "chapeu_estrela", (0, -0.180, 1.010), 0.070, "ouro", normal=(0, -1, 0.3)))
    _cajado(m)


# ---------------------------------------------------------------- caminho 2: fogo
def bola_de_fogo(m):
    x, y, z = pecas.MAO_ESQ
    m.somar("mao_livre", [pecas.esfera(m, "bola_fogo", (x - 0.02, y - 0.04, z + 0.10), 0.075, "laranja", cortes=0)] + pecas.chama(m, "bola_fogo_chama", (x - 0.02, y - 0.04, z + 0.15), 0.150))


def muralha(m):
    """Muralha de Fogo: fileira de chamas no chao, na frente do mago."""
    objs = []
    for k in range(5):
        objs += pecas.chama(m, f"muralha_{k}", ((k - 2) * 0.150, -0.460 - 0.030 * abs(k - 2), 0.0), 0.200 if k % 2 == 0 else 0.150)
    m.somar("extra", objs)


def _fenix(m, escala, cor, asa):
    """Fenix pousada ao lado, de asas abertas."""
    c = Vector((-0.560, 0.080, 0.300 * escala))
    objs = [pecas.esfera(m, "fenix", c, (0.100 * escala, 0.130 * escala, 0.110 * escala), cor),
            pecas.esfera(m, "fenix_cabeca", c + Vector((0, -0.130, 0.110)) * escala, 0.070 * escala, cor, cortes=0),
            pecas.cone(m, "fenix_bico", c + Vector((0, -0.190, 0.110)) * escala, (0, -1, -0.2), 0.030 * escala, 0.090 * escala, "amarelo", seg=4)]
    for sx in (-1, 1):
        objs.append(pecas.cone(m, f"fenix_asa_{sx}", c + Vector((0.060 * sx, 0, 0.040)) * escala, (sx, 0.3, 0.6), 0.090 * escala, 0.320 * escala, asa, seg=4, fechado=True))
    objs += pecas.chama(m, "fenix_cauda", c + Vector((0, 0.120, 0.020)) * escala, 0.200 * escala, cores=(asa, "amarelo"))
    objs.append(pecas.cilindro(m, "fenix_poleiro", (c.x, c.y, 0), (0, 0, 1), [(0, 0), (0.060, 0.004), (0.020, 0.030), (0.020, c.z - 0.090 * escala), (0, c.z - 0.090 * escala)], cor="marrom_escuro", seg=6))
    return objs


def fogo(m):
    t = m.tier[1]
    _traje(m, "vermelho", "ouro", "laranja", capa={3: None, 4: "laranja", 5: "ouro"}[t])
    if t >= 4:
        m.somar("extra", _fenix(m, 1.0 if t == 4 else 1.45, "laranja" if t == 4 else "ouro", "vermelho"))
    _cajado(m)


# ---------------------------------------------------------------- caminho 3: sombras
def monoculo(m):
    m.por("rosto", pecas.monoculo(m, "roxo", "ouro"))


def _lapides(m, n, cor):
    objs = []
    for k, (x, y) in enumerate(((-0.500, 0.250), (0.520, 0.200), (-0.560, -0.150), (0.560, -0.180), (0.0, 0.560))[:n]):
        objs.append(pecas.bloco(m, f"lapide_{k}", (0.130, 0.050, 0.210), (x, y, 0.100), cor, chanfro=0.022, seg=2))
    return objs


def sombras(m):
    t = m.tier[2]
    if t == 3:   # Cintilar: faiscas em volta do chapeu
        for k in range(6):
            a = math.radians(60 * k)
            m.somar("chapeu", pecas.cone(m, f"faisca_{k}", (0.330 * math.cos(a), 0.330 * math.sin(a), 0.960), (math.cos(a), math.sin(a), 0.5), 0.030, 0.100, "ciano", seg=4, fechado=True))
    else:
        _traje(m, "tinta", "verde" if t == 4 else "roxo", "verde" if t == 4 else "roxo", capa="tinta" if t == 4 else "roxo")
        m.somar("extra", _lapides(m, 3 if t == 4 else 5, "cinza"))
        if t == 5:
            m.somar("chapeu", pecas.coroa(m, "ouro", "verde", raio=0.150, z=0.975, pontas=5, nome="coroa_trevas"))
    _cajado(m)


PARAMETRO = [(None, None)]   # o tier muda o cajado, que le m.tier
CAMINHOS = [
    {1: PARAMETRO, 2: PARAMETRO, 3: arcano, 4: arcano, 5: arcano},
    {1: [("mao_livre", bola_de_fogo)], 2: [("extra", muralha)], 3: fogo, 4: fogo, 5: fogo},
    {1: PARAMETRO, 2: [("rosto", monoculo)], 3: sombras, 4: sombras, 5: sombras},
]
