"""Macaco Dardo: o macaco padrao de topete, lenco azul e um dardo na mao.

Caminho 1 (afiados): dardos de aco de reserva na mao livre; do tier 3 em diante, catapulta.
Caminho 2 (rapidos): tenis e faixa de corrida; leque de tres dardos; capa do fa-clube.
Caminho 3 (alcance): dardo comprido e oculos; do tier 3 em diante, besta e capuz.
"""
import math

from mathutils import Matrix, Vector

import pecas

ENQUADRE = ((0, 0.03, 0.42), 1.7)
MX, MY, MZ = pecas.MAO
ZD = MZ + 0.02   # altura do dardo na mao


# ---------------------------------------------------------------- base e arma de mao
def _ponta(m):
    """Cor e tamanho da ponta de qualquer dardo da torre: o caminho 1 afia (regra da torre)."""
    if m.tier[1] >= 5:
        return "ciano", 1.25
    return ("aco", 1.0 + 0.08 * m.tier[0]) if m.tier[0] else ("cinza", 1.0)


def _dardo_mao(m, longo=False):
    cor, esc = _ponta(m)
    return pecas.dardo(m, "dardo", (MX, MY, ZD), frente=0.40 if longo else 0.20, ponta=cor, ponta_escala=esc,
                       pena_escala=1.5 if longo else 1.0)


def base(m):
    m.macaco(pelagem="topete")
    m.por("tronco", pecas.lenco(m, "azul"))
    m.por("mao_ataque", _dardo_mao(m))


# ---------------------------------------------------------------- caminho 1: afiados e catapulta
def reserva_1(m):
    """Um dardo de aco de reserva na mao esquerda."""
    x, y, z = pecas.MAO_ESQ
    m.por("mao_livre", pecas.dardo(m, "reserva", (x, y - 0.02, z + 0.02), direcao=(-0.10, -1, 0.10), ponta="aco", ponta_escala=1.08,
                                   penas=("cinza", "branco")))


def reserva_2(m):
    """Feixe de tres dardos de aco na mao esquerda e bracelete de espinhos."""
    x, y, z = pecas.MAO_ESQ
    objs = []
    for k, (dx, dz) in enumerate(((-0.34, 0.16), (-0.10, 0.10), (0.14, 0.16))):
        objs += pecas.dardo(m, f"reserva_{k}", (x, y - 0.02, z + 0.02), direcao=(dx, -1, dz), ponta="aco", ponta_escala=1.16,
                            penas=("cinza", "branco"))
    objs.append(pecas.faixa(m, "bracelete", (x + 0.012, y + 0.030, z + 0.075), 0.062, 0.062, 0.020, "cinza_escuro", n=8))
    for k in range(5):
        a = math.radians(72 * k + 20)
        d = Vector((math.cos(a), math.sin(a), 0.15))
        objs.append(pecas.cone(m, f"bracelete_espinho_{k}", Vector((x + 0.012, y + 0.030, z + 0.075)) + d * 0.060, d, 0.022, 0.075, "aco", seg=5))
    m.por("mao_livre", objs)


RECUO = 0.30   # o macaco fica atras da catapulta
CY = -0.30     # centro da carreta: o conjunto (catapulta mais macaco) fica centrado na origem


def _roda(m, nome, pos, sx, raio, pneu, cubo):
    w = 0.040
    perfil = [(0, -w * 1.5), (raio * 0.30, -w * 1.1), (raio * 0.34, -w), (raio * 0.90, -w), (raio, -w * 0.5), (raio, w * 0.5),
              (raio * 0.90, w), (raio * 0.34, w), (0, w)]
    return pecas.cilindro(m, nome, pos, (sx, 0, 0), perfil, cores=[cubo, cubo, pneu, pneu, pneu, pneu, pneu, pneu], seg=12)


def _bola(m, raio, cor, espinho, pos, faixa=None):
    """Bola de espinhos: esfera com doze pontas."""
    objs = [pecas.esfera(m, "bola", pos, raio, cor)]
    if faixa:
        objs.append(pecas.toro(m, "bola_faixa", pos, raio * 0.99, raio * 0.13, faixa, normal=(0, 0.35, 1), seg=12, lados=4))
    f = (1 + 5 ** 0.5) / 2
    dirs = [(0, s1, s2 * f) for s1 in (-1, 1) for s2 in (-1, 1)] + [(s1, s2 * f, 0) for s1 in (-1, 1) for s2 in (-1, 1)] +            [(s2 * f, 0, s1) for s1 in (-1, 1) for s2 in (-1, 1)]
    for k, d in enumerate(dirs):
        d = Vector(d).normalized()
        objs.append(pecas.cone(m, f"bola_espinho_{k}", Vector(pos) + d * raio * 0.80, d, raio * 0.30, raio * 0.75, espinho, seg=5))
    return objs


def _catapulta(m, madeira, viga, pneu, cubo, raio_bola, bola, espinho, braco, chapa=None, faixa=None):
    """Catapulta: carreta de vigas com quatro rodas (base) e o braco com a colher e a bola (torreta)."""
    b = []
    frente, tras, pe = CY - 0.30, CY + 0.30, CY - 0.10   # travessas e o pe dos montantes
    for sx in (-1, 1):
        b.append(pecas.bloco(m, f"longarina_{sx}", (0.075, 0.70, 0.085), (0.215 * sx, CY, 0.135), madeira, chanfro=0.016))
        b.append(pecas.bloco(m, f"montante_{sx}", (0.065, 0.075, 0.36), (0.215 * sx, pe, 0.345), viga, chanfro=0.014))
        for k, y in enumerate((frente + 0.06, tras - 0.06)):
            b += [_roda(m, f"roda_{sx}_{k}", (0.262 * sx, y, 0.112), sx, 0.112, pneu, cubo)]
    for k, y in enumerate((frente, tras)):
        b.append(pecas.bloco(m, f"travessa_{k}", (0.44, 0.075, 0.075), (0, y, 0.135), madeira, chanfro=0.016))
    b.append(pecas.bloco(m, "batente", (0.50, 0.070, 0.070), (0, pe, 0.535), viga, chanfro=0.016))
    if chapa:  # reforco de metal nas pontas das longarinas e no batente
        for sx in (-1, 1):
            for k, y in enumerate((frente, tras)):
                b.append(pecas.bloco(m, f"chapa_{sx}_{k}", (0.090, 0.130, 0.100), (0.215 * sx, y, 0.135), chapa, chanfro=0.012))
            b.append(pecas.bloco(m, f"chapa_batente_{sx}", (0.085, 0.085, 0.090), (0.215 * sx, pe, 0.535), chapa, chanfro=0.012))
    m.por("base", b)

    eixo, colher = Vector((0, pe, 0.210)), Vector((0, tras - 0.05, 0.300))
    # braco grosso e de cor clara, para aparecer de cima entre as longarinas; contrapeso na frente
    peso = eixo + Vector((0, -0.150, -0.050))
    t = [pecas.tubo(m, "braco_catapulta", [peso, eixo, (eixo + colher) / 2 + Vector((0, 0, 0.012)), colher], [0.044, 0.050, 0.046, 0.042], braco)]
    t.append(pecas.bloco(m, "contrapeso", (0.170, 0.120, 0.120), peso, chapa or "cinza", chanfro=0.020))
    t.append(pecas.tubo(m, "eixo_catapulta", [(-0.20, pe, 0.210), (0.20, pe, 0.210)], 0.026, chapa or viga, nivel=0))
    r = raio_bola
    t += [pecas.cilindro(m, "colher", colher + Vector((0, 0.01, -0.035)), (0, 0, 1), [(0, 0), (r * 0.85, 0.004), (r * 1.30, 0.070), (r * 1.16, 0.072), (r * 0.74, 0.022), (0, 0.022)],
                         cor=braco, seg=10)]
    t += _bola(m, r, bola, espinho, colher + Vector((0, 0.01, r * 0.92)), faixa)
    m.por("torreta", t)
    m.pivo("torreta", eixo)
    m.pivo("base", (0, 0, 0))
    # a mao do ataque segura a alavanca do disparo
    m.por("mao_ataque", [pecas.tubo(m, "alavanca", [(MX, MY - 0.010, MZ + 0.130), (MX + 0.010, MY - 0.050, 0.250), (MX + 0.020, MY - 0.110, 0.060)], 0.020, viga),
                         pecas.esfera(m, "alavanca_pomo", (MX, MY - 0.005, MZ + 0.160), 0.046, "vermelho", cortes=0)])
    m.tirar("mao_livre")
    m.ocupar("pes")   # os pes somem atras da maquina: o acessorio dos pes vai para a carreta
    m.matriz = Matrix.Translation((0, RECUO, 0))


def _capacete(m, cor, barra, topo=None, crista=None):
    m.tirar("pelagem")
    objs = [pecas.casca(m, "capacete", cor, pecas.piso_capacete)]
    objs.append(pecas.barra_casca(m, "capacete_barra", barra, pecas.piso_capacete))
    if topo:
        objs.append(pecas.cone(m, "capacete_ponta", (0, 0.0, 0.985), (0, 0.1, 1), 0.050, 0.150, topo, seg=6))
    if crista:
        for k in range(4):
            y = -0.150 + k * 0.105
            objs.append(pecas.cone(m, f"capacete_crista_{k}", (0, y, 0.972 - 0.06 * abs(k - 1.3) ** 1.5), (0, 0.35, 1), 0.052, 0.170 - 0.02 * abs(k - 1.5), crista, seg=4))
    m.por("chapeu", objs)


def espinhopulta(m):
    _catapulta(m, "marrom", "marrom_escuro", "tinta", "ouro", 0.105, "cinza", "aco", "bege")
    _capacete(m, "marrom_escuro", "bege")


def juggernaut(m):
    _catapulta(m, "marrom", "marrom_escuro", "tinta", "cinza", 0.140, "cinza_escuro", "aco", "aco", chapa="cinza")
    _capacete(m, "cinza", "cinza_escuro", topo="aco")


def ultra_juggernaut(m):
    _catapulta(m, "cinza_escuro", "tinta", "tinta", "ouro", 0.175, "tinta", "ouro", "ouro", chapa="ouro", faixa="vermelho")
    _capacete(m, "ouro", "vermelho", crista="vermelho")


# ---------------------------------------------------------------- caminho 2: rapidos e fa-clube
def tenis(m):
    m.por("pes", pecas.tenis(m, "vermelho"))


def listras_corrida(m):
    """Alternativa do tenis quando os pes estao escondidos (catapulta): listras vermelhas na carreta."""
    objs = []
    for sx in (-1, 1):
        objs.append(pecas.bloco(m, f"listra_{sx}", (0.050, 0.44, 0.030), (0.215 * sx, CY, 0.190), "vermelho", chanfro=0.008))
        objs.append(pecas.cone(m, f"listra_ponta_{sx}", (0.215 * sx, CY - 0.22, 0.190), (0, -1, 0), 0.034, 0.110, "vermelho", seg=4))
    m.somar("extra", objs)


def faixa_corrida(m):
    m.por("chapeu", pecas.faixa_testa(m, "vermelho"))


def cachecol_corrida(m):
    """Alternativa da faixa quando o chapeu esta ocupado: troca o lenco por um cachecol vermelho."""
    m.por("tronco", pecas.cachecol(m, "vermelho"))


def tiro_triplo(m):
    cor, esc = _ponta(m)
    objs = []
    for k, g in enumerate((-24, 0, 24)):
        a = math.radians(g)
        objs += pecas.dardo(m, f"dardo_{k}", (MX + 0.03 * math.sin(a), MY, ZD), direcao=(math.sin(a), -math.cos(a), 0), ponta=cor, ponta_escala=esc)
    m.por("mao_ataque", objs)
    bando = pecas.bandoleira(m, "amarelo")
    for k, a in enumerate((-125, -95, -65)):
        a = math.radians(a)
        x, y = 0.186 * math.cos(a), -0.005 + 0.170 * math.sin(a)
        bando.append(pecas.cone(m, f"bandoleira_dardo_{k}", (x, y - 0.012, 0.385 - 0.105 * math.cos(a) / 1.0), (0.25, -0.2, 1), 0.026, 0.085, "cinza", seg=5, fechado=True))
    m.somar("tronco", bando)


def fa_clube(m):
    # o traje do fa-clube troca o lenco e a bandoleira: fica so a capa (com gola) e a estrela
    m.por("costas", pecas.capa(m, "azul", borda="amarelo"))
    m.por("tronco", pecas.estrela(m, "emblema", (0, -0.166, 0.360), 0.090, "ouro", normal=(0, -1, 0.15)))


def fa_clube_plasma(m):
    m.por("costas", pecas.capa(m, "roxo", comp=0.56, largura=0.66, borda="ciano"))
    m.por("tronco", pecas.estrela(m, "emblema", (0, -0.166, 0.360), 0.100, "ciano", normal=(0, -1, 0.15)))
    halo = [pecas.toro(m, "halo", (0, 0.0, 1.110), 0.215, 0.026, "ciano", seg=16, lados=5)]
    for k in range(4):
        a = math.radians(90 * k + 45)
        d = Vector((math.cos(a), math.sin(a), 0.25))
        halo.append(pecas.cone(m, f"halo_raio_{k}", Vector((0, 0, 1.110)) + d * 0.215, d, 0.030, 0.110, "branco", seg=4))
    m.por("extra", halo, grupo="cabeca", preso=True)


# ---------------------------------------------------------------- caminho 3: alcance e besta
def dardo_longo(m):
    m.por("mao_ataque", _dardo_mao(m, longo=True))


def dardo_costas(m):
    """Alternativa do dardo comprido quando a mao do ataque esta ocupada: vai atravessado nas costas."""
    cor, esc = _ponta(m)
    m.somar("costas", pecas.dardo(m, "dardo_costas", (0.060, 0.225, 0.560), direcao=(0.55, 0.0, 1), frente=0.42, tras=0.20, ponta=cor,
                                  ponta_escala=max(esc, 1.2), pena_escala=1.35))


def dardo_longo_reserva(m):
    """Segunda alternativa: o dardo comprido vai na mao livre."""
    x, y, z = pecas.MAO_ESQ
    cor, esc = _ponta(m)
    m.somar("mao_livre", pecas.dardo(m, "dardo_longo", (x, y - 0.02, z + 0.02), direcao=(-0.08, -1, 0.08), frente=0.36, ponta=cor, ponta_escala=esc,
                                     pena_escala=1.35))


def oculos(m):
    m.por("rosto", pecas.oculos(m))


def _capuz(m, cor, barra=None, pena=None):
    m.tirar("pelagem")
    objs = pecas.capuz(m, cor, barra)
    if pena:
        objs.append(pecas.cone(m, "capuz_pena", (0.300, -0.020, 0.930), (0.50, 0.15, 1), 0.050, 0.250, pena, seg=4, fechado=True))
    m.por("chapeu", objs)


def besta(m):
    cor, esc = _ponta(m)
    m.por("mao_ataque", pecas.besta(m, (MX, MY - 0.02, ZD + 0.01), ponta=cor))
    _capuz(m, "roxo")


def atirador_afiado(m):
    cor, esc = _ponta(m)
    m.por("mao_ataque", pecas.besta(m, (MX, MY - 0.04, ZD + 0.01), escala=1.28, detalhe="cinza_escuro", ponta=cor, luneta=True))
    _capuz(m, "roxo", barra="amarelo", pena="amarelo")
    m.por("costas", pecas.aljava(m))


def mestre_da_besta(m):
    cor, esc = _ponta(m)
    m.por("mao_ataque", pecas.besta(m, (MX, MY - 0.06, ZD + 0.01), escala=1.50, madeira="tinta", arco_cor="ouro", detalhe="ouro", ponta=cor,
                                    luneta=True, duplo=True))
    _capuz(m, "tinta", barra="ouro", pena="ouro")
    m.por("costas", pecas.aljava(m, corpo="ouro", detalhe="tinta", penas=("ouro", "branco", "ouro")) + pecas.capa(m, "tinta", comp=0.40, largura=0.50, queda=0.26, borda="ouro", gola=False))


CAMINHOS = [
    {1: [("mao_livre", reserva_1)], 2: [("mao_livre", reserva_2)], 3: espinhopulta, 4: juggernaut, 5: ultra_juggernaut},
    {1: [("pes", tenis), ("extra", listras_corrida)], 2: [("chapeu", faixa_corrida), ("tronco", cachecol_corrida)], 3: tiro_triplo, 4: fa_clube, 5: fa_clube_plasma},
    {1: [("mao_ataque", dardo_longo), ("costas", dardo_costas), ("mao_livre", dardo_longo_reserva)], 2: [("rosto", oculos)], 3: besta, 4: atirador_afiado, 5: mestre_da_besta},
]
