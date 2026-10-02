"""Druida: o macaco padrao de barba, coroa de folhas, tunica verde e cajado de madeira com um broto na ponta.

Caminho 1 (tempestade): ponta de espinho e raio no cajado; depois traje azul, tornado e nuvem.
Caminho 2 (selva): espinhos no cajado e bolota; depois flores, cesto de bananas, galhos e cipos.
Caminho 3 (ira): cajado mais alto e oculos vermelhos; depois traje vermelho, chifres e fogo.

O cajado sai de _cajado, que le os tres tiers (regra da torre) e vale em qualquer combinacao.
"""
import math

from mathutils import Vector

import pecas

ENQUADRE = ((0, 0.03, 0.52), 1.9)


def _cajado(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    orbe, raio, altura = "verde", 0.070, 0.960 + (0.180 if t3 >= 1 else 0)
    if p == 0 and t1 >= 4:
        orbe, raio = "ciano", 0.105 if t1 == 4 else 0.125
    elif p == 2 and t3 >= 3:
        orbe, raio = "vermelho", 0.085 + 0.012 * (t3 - 3)
    objs, topo = pecas.cajado(m, cor="marrom", orbe=orbe, raio=raio, altura=altura, garra="marrom_escuro")
    if t1 >= 1:   # Espinhos Duros: ponta de aco
        objs.append(pecas.cone(m, "cajado_espinho", topo + Vector((0, 0, raio * 0.7)), (0, 0, 1), raio * 0.40, raio * 2.0, "aco", seg=5))
    if t1 >= 2:   # Coracao do Trovao: raio amarelo saindo da ponta
        for k, (d, o) in enumerate((((0.9, 0, 0.5), (0, 0, 0)), ((0.3, 0, -1), (0.130, 0, 0.070)), ((1, 0, 0.3), (0.170, 0, -0.060)))):
            objs.append(pecas.cone(m, f"cajado_raio_{k}", topo + Vector(o) + Vector((raio, 0, raio)), d, 0.022, 0.150, "amarelo", seg=4, fechado=True))
    if t2 >= 1:   # Enxame de Espinhos: coroa de espinhos verdes abaixo do broto
        for k in range(6):
            a = math.radians(60 * k)
            d = Vector((math.cos(a), math.sin(a), 0.35))
            objs.append(pecas.cone(m, f"cajado_farpa_{k}", topo + Vector((0, 0, -raio * 1.6)) + d * 0.020, d, 0.020, 0.100, "verde_escuro", seg=4))
    m.por("mao_ataque", objs)


def _traje(m, cor, barra):
    m.por("tronco", pecas.colete(m, cor, barra=barra, z0=0.300, z1=0.515) + pecas.saia(m, cor, barra=barra))


def base(m):
    m.macaco(pelagem="barba")
    m.por("chapeu", pecas.coroa(m, "verde", "vermelho", raio=0.215, z=0.895, pontas=6))
    _traje(m, "verde_escuro", "marrom")
    _cajado(m)


# ---------------------------------------------------------------- caminho 1: tempestade
def tempestade(m):
    t = m.tier[0]
    _traje(m, "azul", "branco" if t < 5 else "ouro")
    funil = []   # tornado ao lado: aneis que encolhem para baixo
    for k in range(5):
        funil.append(pecas.toro(m, f"tornado_{k}", (-0.540 + 0.020 * math.sin(k * 1.7), 0.060, 0.080 + 0.130 * k), 0.060 + 0.045 * k, 0.034, "cinza" if k % 2 == 0 else "branco", seg=10, lados=4))
    m.somar("extra", funil)
    if t == 5:   # nuvem de tempestade sobre a cabeca, com raios
        nuvem = [pecas.esfera(m, f"nuvem_{k}", (x, 0.060, 1.330 + z), r, "cinza_escuro", cortes=0) for k, (x, z, r) in enumerate(((-0.150, 0, 0.130), (0.040, 0.040, 0.160), (0.210, 0, 0.120)))]
        for k, x in enumerate((-0.140, 0.180)):
            nuvem.append(pecas.cone(m, f"nuvem_raio_{k}", (x, 0.020, 1.270), (0.2 - 0.4 * k, -0.3, -1), 0.030, 0.170, "amarelo", seg=4, fechado=True))
        m.somar("extra", nuvem, grupo="cabeca", preso=True)
        m.por("costas", pecas.capa(m, "roxo", borda="ouro", comp=0.42, largura=0.50, gola=False))
        m.por("chapeu", pecas.coroa(m, "ouro", "ciano", raio=0.215, z=0.895, pontas=6))
    _cajado(m)


# ---------------------------------------------------------------- caminho 2: selva
def bolota(m):
    m.somar("tronco", [pecas.esfera(m, "bolota", (0, -0.190, 0.430), (0.052, 0.036, 0.060), "marrom", cortes=0),
                       pecas.esfera(m, "bolota_capa", (0, -0.190, 0.470), (0.058, 0.040, 0.032), "marrom_escuro", cortes=0)])


def muda(m):
    """Alternativa da bolota: uma muda de carvalho no chao, ao lado."""
    m.somar("extra", [pecas.tubo(m, "muda", [(0.480, -0.100, 0), (0.490, -0.100, 0.200)], 0.020, "marrom", nivel=0),
                      pecas.esfera(m, "muda_copa", (0.490, -0.100, 0.270), (0.110, 0.110, 0.090), "verde", cortes=0)], preso=True)


def selva(m):
    t = m.tier[1]
    m.por("chapeu", pecas.coroa(m, "verde", "rosa", raio=0.215, z=0.895, pontas=6) +
          [pecas.esfera(m, f"flor_{k}", (0.230 * math.cos(math.radians(a)), 0.230 * math.sin(math.radians(a)), 0.950), 0.045, "rosa" if k % 2 == 0 else "amarelo", cortes=0)
           for k, a in enumerate((200, 250, 290, 340))])
    m.por("mao_livre", pecas.manga(m, -1, "verde", anel="verde_escuro"))   # cipos enrolados no braco
    if t >= 4:   # cesto de bananas
        cesto = [pecas.cilindro(m, "cesto", (-0.500, -0.060, 0), (0, 0, 1), [(0, 0), (0.090, 0.004), (0.130, 0.130), (0.115, 0.130), (0.090, 0.060), (0, 0.060)], cor="marrom", seg=10)]
        for k in range(4):
            a = math.radians(90 * k + 20)
            cesto.append(pecas.cone(m, f"banana_{k}", (-0.500 + 0.050 * math.cos(a), -0.060 + 0.050 * math.sin(a), 0.080), (math.cos(a) * 0.7, math.sin(a) * 0.7, 1), 0.030, 0.150, "amarelo", seg=4, fechado=True))
        m.somar("extra", cesto)
    if t == 5:   # espirito da floresta: galhos na cabeca, capa de folhas e cipos no chao
        m.somar("chapeu", pecas.chifres(m, "marrom", comp=0.300, abre=1.2, nome="galho"))
        m.por("costas", pecas.capa(m, "verde", borda="verde_escuro", comp=0.44, largura=0.54, gola=False))
        m.somar("extra", pecas.aro_chao(m, "cipo", 0.600, "verde_escuro", pontas="verde", n=8, altura=0.130))
    _cajado(m)


# ---------------------------------------------------------------- caminho 3: ira
def oculos(m):
    m.por("rosto", pecas.oculos(m, aro="vermelho", tira="tinta"))


def ira(m):
    t = m.tier[2]
    cor, barra = ("vermelho", "laranja") if t < 5 else ("tinta", "vermelho")
    _traje(m, cor, barra)
    if t >= 4:
        m.por("chapeu", pecas.coroa(m, "vermelho", "amarelo", raio=0.215, z=0.895, pontas=6) + pecas.chifres(m, "vermelho" if t == 4 else "tinta", comp=0.180 + 0.080 * (t - 4)))
        m.somar("extra", pecas.aro_chao(m, "furia", 0.560, "laranja", pontas="vermelho" if t == 5 else None, n=8, altura=0.180))
    if t == 5:
        m.somar("chapeu", pecas.chama(m, "furia_chama", (0, 0.020, 0.990), 0.260, cores=("vermelho", "laranja")))
    _cajado(m)


PARAMETRO = [(None, None)]   # o tier muda o cajado, que le m.tier
CAMINHOS = [
    {1: PARAMETRO, 2: PARAMETRO, 3: tempestade, 4: tempestade, 5: tempestade},
    {1: PARAMETRO, 2: [("tronco", bolota), ("extra", muda)], 3: selva, 4: selva, 5: selva},
    {1: PARAMETRO, 2: [("rosto", oculos)], 3: ira, 4: ira, 5: ira},
]
