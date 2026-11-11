"""Atirador de Cola: o macaco padrao de pelo liso, capacete amarelo, pistola de cola e tanque nas costas.

Caminho 1 (corrosiva): a cor da cola muda a cada tier; do tier 3 em diante, mascara, tanques e avental.
Caminho 2 (respingo): gota maior e poca no chao; do tier 3 em diante, mangueira grossa e aspersor.
Caminho 3 (grudenta): cinto de tubos e balde; do tier 3 em diante, lancador de cola no ombro.

A cor da cola e o tamanho da gota saem dos tiers (regra da torre) e valem em qualquer peca com cola.
"""
import math

from mathutils import Vector

import pecas

ENQUADRE = ((0, 0.02, 0.45), 1.7)
MX, MY, MZ = pecas.MAO
COLA = {0: "amarelo", 1: "laranja", 2: "verde", 3: "verde", 4: "roxo", 5: "rosa"}


def _cola(m):
    return COLA[m.tier[0]]


def _gota(m):
    return 1.0 + 0.45 * min(m.tier[1], 1)


def _pistola(m, escala=1.0, corpo="cinza", lado=1):
    """Pistola de cola: corpo, bico e a gota de cola na ponta, com a mangueira ate o tanque."""
    mao = pecas.MAO if lado > 0 else pecas.MAO_ESQ
    n = "pistola" if lado > 0 else "pistola_esq"
    mat = pecas.em((mao[0], mao[1] - 0.02, mao[2] + 0.04), (0.10 * -lado, -1, 0), escala)
    objs = [pecas.bloco(m, n, (0.080, 0.220, 0.100), (0, -0.090, 0), corpo, chanfro=0.022, matriz=mat)]
    objs.append(pecas.bloco(m, n + "_faixa", (0.086, 0.050, 0.106), (0, -0.060, 0), _cola(m), chanfro=0.010, matriz=mat))
    objs.append(pecas.cilindro(m, n + "_bico", (0, -0.190, 0), (0, -1, 0), [(0.038, 0), (0.030, 0.080), (0.040, 0.090), (0.040, 0.120), (0, 0.120)],
                               cor="cinza_escuro", seg=8, matriz=mat))
    objs.append(pecas.esfera(m, n + "_gota", (0, -0.335, -0.008), 0.046 * _gota(m), _cola(m), cortes=0, matriz=mat))
    if lado > 0:
        objs.append(pecas.tubo(m, n + "_mangueira", [(mao[0], mao[1] + 0.06, mao[2] + 0.03), (mao[0] + 0.03, 0.060, 0.330), (0.120, 0.190, 0.340)], 0.018, "cinza_escuro"))
    return objs


def _mangueira(m, escala=1.0):
    """Mangueira grossa com bocal largo, no lugar da pistola."""
    mat = pecas.em((MX, MY - 0.02, MZ + 0.04), (0, -1, 0), escala)
    objs = [pecas.cilindro(m, "bocal", (0, 0.060, 0), (0, -1, 0), [(0.040, 0), (0.044, 0.140), (0.090, 0.260), (0.090, 0.300), (0.070, 0.300), (0.040, 0.180), (0, 0.180)],
                           cores=["cinza", "cinza", "cinza_escuro", "cinza_escuro", "tinta", "tinta"], seg=10, matriz=mat)]
    objs.append(pecas.esfera(m, "bocal_gota", (0, -0.300, -0.006), 0.075 * _gota(m), _cola(m), cortes=0, matriz=mat))
    objs.append(pecas.tubo(m, "mangueira", [(MX, MY + 0.04, MZ + 0.04), (MX + 0.05, 0.080, 0.300), (0.150, 0.200, 0.330)], 0.036, "cinza_escuro"))
    return objs


def _lancador(m, escala, corpo, anel):
    """Lancador de cola apoiado no ombro direito, com um globo de cola na boca."""
    mat = pecas.em((MX - 0.02, MY + 0.06, MZ + 0.14), (0, -1, 0.10), escala)
    perfil = [(0, -0.160), (0.075, -0.150), (0.085, 0.0), (0.085, 0.250), (0.110, 0.270), (0.110, 0.360), (0.080, 0.360), (0.074, 0.200), (0, 0.200)]
    objs = [pecas.cilindro(m, "lancador", (0, 0, 0), (0, -1, 0), perfil, cores=[corpo, corpo, corpo, anel, anel, "tinta", "tinta", "tinta"], seg=12, matriz=mat)]
    objs.append(pecas.esfera(m, "lancador_globo", (0, -0.370, 0), 0.095 * _gota(m), _cola(m), cortes=1, matriz=mat))
    objs.append(pecas.toro(m, "lancador_cinta", (0, -0.060, 0), 0.090, 0.018, anel, normal=(0, 1, 0), seg=10, lados=4, matriz=mat))
    return objs


def _tanques(m, n=1, raio=0.085, altura=0.300, tampa="cinza"):
    lados = {1: (0,), 2: (-1, 1), 3: (-1.6, 0, 1.6)}[n]
    objs = []
    for k, lado in enumerate(lados):
        objs += pecas.tanque(m, _cola(m), tampa=tampa, lado=lado, raio=raio, altura=altura * (1.15 if lado == 0 and n == 3 else 1.0), nome=f"tanque_{k}")
    return objs


def base(m):
    m.macaco(pelagem="lisa")
    m.por("chapeu", pecas.capacete(m, "amarelo", "laranja"))
    m.por("mao_ataque", _pistola(m))
    m.por("costas", _tanques(m))
    # lampada na frente do capacete, da cor da cola: mostra o tier do caminho 1 na vista do jogo
    m.somar("extra", pecas.esfera(m, "lampada", (0, -0.190, 0.940), (0.062, 0.050, 0.055), _cola(m), cortes=0), grupo="cabeca", preso=True)


# ---------------------------------------------------------------- caminho 1: corrosiva
def _mascara(m, cor="cinza_escuro", filtro="cinza"):
    objs = [pecas.cilindro(m, "mascara", (0, -0.215, 0.650), (0, -1, -0.10), [(0.110, 0), (0.105, 0.050), (0.070, 0.085), (0, 0.090)], cor=cor, seg=10)]
    for sx in (-1, 1):
        objs.append(pecas.cilindro(m, f"mascara_filtro_{sx}", (0.085 * sx, -0.235, 0.640), (0.9 * sx, -1, -0.2), [(0.045, 0), (0.045, 0.060), (0, 0.062)], cor=filtro, seg=8))
    return objs


def dissolvedor(m):
    m.por("mao_ataque", _pistola(m, 1.20))
    m.por("costas", _tanques(m, 2))
    m.por("rosto", _mascara(m))


def liquefator(m):
    m.por("costas", _tanques(m, 2, raio=0.100, altura=0.360, tampa="amarelo"))
    m.por("tronco", pecas.colete(m, "branco", barra=_cola(m), z0=0.200, z1=0.510))
    m.por("chapeu", pecas.capacete(m, "branco", _cola(m)))


def solucionador(m):
    m.por("mao_ataque", _pistola(m, 1.30, corpo="tinta"))
    m.por("mao_livre", _pistola(m, 1.30, corpo="tinta", lado=-1))
    m.por("costas", _tanques(m, 3, raio=0.095, altura=0.360, tampa="ouro"))
    m.por("tronco", pecas.colete(m, "tinta", barra=_cola(m), emblema=_cola(m), z0=0.200, z1=0.510))
    m.por("chapeu", pecas.capacete(m, "tinta", _cola(m), topo=_cola(m)))
    m.por("rosto", _mascara(m, "tinta", "ouro"))


# ---------------------------------------------------------------- caminho 2: respingo e mangueira
def poca(m):
    """Respingo de Cola: poca de cola no chao, em volta dos pes, com respingos."""
    objs = [pecas.esfera(m, "poca", (0.030, -0.080, 0.006), (0.330, 0.290, 0.022), _cola(m))]
    for k, (x, y, r) in enumerate(((0.400, -0.250, 0.060), (-0.360, 0.130, 0.075), (0.300, 0.260, 0.050), (-0.330, -0.300, 0.045))):
        objs.append(pecas.esfera(m, f"poca_respingo_{k}", (x, y, 0.006), (r, r * 0.85, 0.018), _cola(m), cortes=0))
    m.somar("extra", objs, preso=True)


def mangueira(m):
    m.por("mao_ataque", _mangueira(m))
    m.por("costas", _tanques(m, 1, raio=0.115, altura=0.340))


def ataque_de_cola(m):
    m.por("mao_ataque", _mangueira(m, 1.20))
    m.por("costas", _tanques(m, 1, raio=0.135, altura=0.400, tampa="vermelho"))
    m.somar("costas", pecas.toro(m, "tanque_manometro", (0, 0.215, 0.520), 0.150, 0.024, "vermelho", normal=(0, 0.12, 1), seg=12, lados=4))


def tempestade(m):
    m.por("mao_ataque", _mangueira(m, 1.35))
    m.por("costas", _tanques(m, 1, raio=0.150, altura=0.430, tampa="ouro"))
    asp = [pecas.tubo(m, "aspersor_haste", [(0, 0.240, 0.740), (0, 0.160, 1.050), (0, 0.020, 1.180)], 0.020, "cinza")]
    asp.append(pecas.cilindro(m, "aspersor", (0, 0.020, 1.160), (0, 0, 1), [(0, 0), (0.090, 0.004), (0.110, 0.040), (0, 0.050)], cor="ouro", seg=10))
    for k in range(8):   # gotas saindo do aspersor, em volta, por cima da cabeca
        a = math.radians(45 * k)
        d = Vector((math.cos(a), math.sin(a), 0))
        asp.append(pecas.esfera(m, f"aspersor_gota_{k}", Vector((0, 0.020, 1.150)) + d * 0.300 + Vector((0, 0, -0.050 * (k % 2))), 0.050, _cola(m), cortes=0))
    m.somar("extra", asp, grupo="cabeca", preso=True)


# ---------------------------------------------------------------- caminho 3: grudenta e lancador
def cinto_de_tubos(m):
    objs = pecas.bandoleira(m, "marrom_escuro")
    for k, a in enumerate((-135, -105, -75, -45)):
        a = math.radians(a)
        x, y = 0.186 * math.cos(a), -0.005 + 0.170 * math.sin(a)
        objs.append(pecas.cilindro(m, f"tubo_cola_{k}", (x, y - 0.014, 0.360 - 0.105 * math.cos(a)), (0.2, -0.15, 1), [(0.028, 0), (0.028, 0.080), (0.012, 0.100), (0, 0.100)],
                                   cor=_cola(m), seg=6))
    m.somar("tronco", objs)


def latas_no_chao(m):
    objs = []
    for k, (x, y) in enumerate(((-0.440, 0.060), (-0.500, -0.110))):
        objs.append(pecas.cilindro(m, f"lata_{k}", (x, y, 0), (0, 0, 1), [(0, 0), (0.075, 0.004), (0.075, 0.150), (0.060, 0.156), (0, 0.156)],
                                   cores=["cinza", "cinza", "cinza", _cola(m)], seg=10))
    m.somar("extra", objs, preso=True)


def _balde(m, pos):
    return [pecas.cilindro(m, "balde", pos, (0, 0, 1), [(0, 0), (0.075, 0.004), (0.100, 0.160), (0.090, 0.160), (0.085, 0.130), (0, 0.130)],
                           cores=["cinza", "cinza", "cinza", _cola(m), _cola(m)], seg=10)]


def balde(m):
    x, y, z = pecas.MAO_ESQ
    m.somar("mao_livre", _balde(m, (x - 0.01, y - 0.03, z - 0.190)) + [pecas.toro(m, "balde_alca", (x - 0.01, y - 0.03, z - 0.030), 0.085, 0.010, "cinza_escuro", normal=(0, 1, 0), seg=8, lados=4)])


def balde_no_chao(m):
    m.somar("extra", _balde(m, (-0.470, -0.280, 0)), preso=True)


def cola_de_moab(m):
    m.por("mao_ataque", _lancador(m, 1.0, "cinza_escuro", "laranja"))
    m.por("chapeu", pecas.capacete(m, "laranja", "tinta"))


def implacavel(m):
    m.por("mao_ataque", _lancador(m, 1.22, "cinza_escuro", "vermelho"))
    m.por("chapeu", pecas.capacete(m, "vermelho", "tinta", topo="amarelo"))
    m.somar("tronco", pecas.colete(m, "cinza_escuro", barra="vermelho", z0=0.200, z1=0.510))


def super_cola(m):
    m.por("mao_ataque", _lancador(m, 1.45, "tinta", "ouro"))
    m.por("chapeu", pecas.capacete(m, "tinta", "ouro", topo="ouro"))
    m.por("costas", _tanques(m, 2, raio=0.110, altura=0.380, tampa="ouro"))


PARAMETRO = [(None, None)]   # o tier muda a cor da cola ou a gota, que as pecas leem em m.tier
CAMINHOS = [
    {1: PARAMETRO, 2: PARAMETRO, 3: dissolvedor, 4: liquefator, 5: solucionador},
    {1: PARAMETRO, 2: [("extra", poca)], 3: mangueira, 4: ataque_de_cola, 5: tempestade},
    {1: [("tronco", cinto_de_tubos), ("extra", latas_no_chao)], 2: [("mao_livre", balde), ("extra", balde_no_chao)], 3: cola_de_moab, 4: implacavel, 5: super_cola},
]
