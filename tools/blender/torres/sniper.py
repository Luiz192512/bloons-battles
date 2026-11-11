"""Macaco Atirador (sniper): o macaco padrao de pelo liso, capacete verde e um rifle comprido.

Caminho 1 (dano): ponteira de aco e cano grosso; do tier 3 em diante, luneta grande, capuz e capa.
Caminho 2 (apoio): oculos de visao noturna e boca de estilhacos; depois boina, caixa de suprimentos.
Caminho 3 (cadencia): carregadores; do tier 3 em diante, tambor, faixa, cinto de municao e armadura.

O rifle sai de _rifle, que le os tres tiers (regra da torre) e vale em qualquer combinacao.
"""
import pecas

ENQUADRE = ((0, -0.10, 0.45), 1.9)
MX, MY, MZ = pecas.MAO


def _rifle(m):
    t1, t2, t3 = m.tier
    esc = 1.0 + (0.14 if t1 >= 3 else 0) + (0.12 if t1 >= 4 else 0) + (0.14 if t1 >= 5 else 0)
    corpo = "tinta" if (t1 >= 5 or t2 >= 5) else ("cinza_escuro" if t3 >= 4 else "marrom")
    metal = "ouro" if t2 >= 5 else ("vermelho" if t1 >= 5 else "cinza_escuro")
    mat = pecas.em((MX, MY + 0.02, MZ + 0.03), (0, -1, 0.04), esc)
    r_cano = 0.028 if t1 >= 2 else 0.017
    objs = [pecas.bloco(m, "rifle_coronha", (0.050, 0.220, 0.095), (0, 0.130, -0.012), corpo, chanfro=0.016, matriz=mat),
            pecas.bloco(m, "rifle_corpo", (0.052, 0.280, 0.070), (0, -0.090, 0), corpo, chanfro=0.014, matriz=mat),
            pecas.tubo(m, "rifle_cano", [(0, -0.200, 0.010), (0, -0.640, 0.010)], r_cano, metal, nivel=0, matriz=mat)]
    if t1 >= 1:   # Jaqueta Metalica: ponteira de aco
        objs.append(pecas.cilindro(m, "rifle_ponteira", (0, -0.600, 0.010), (0, -1, 0), [(r_cano * 1.7, 0), (r_cano * 1.7, 0.090), (r_cano * 1.1, 0.100), (0, 0.100)],
                                   cor="aco", seg=8, matriz=mat))
    if t1 >= 4:   # freio de boca e bipe
        objs.append(pecas.bloco(m, "rifle_freio", (0.110, 0.060, 0.050), (0, -0.720, 0.010), "aco" if t1 == 4 else "ouro", chanfro=0.010, matriz=mat))
        for sx in (-1, 1):
            objs.append(pecas.tubo(m, f"rifle_bipe_{sx}", [(0, -0.480, 0), (0.100 * sx, -0.520, -0.200)], 0.012, metal, nivel=0, matriz=mat))
    r_lun = 0.036 if t1 >= 3 else 0.022
    objs.append(pecas.cilindro(m, "rifle_luneta", (0, 0.040, 0.050 + r_lun), (0, -1, 0),
                               [(0, 0), (r_lun, 0.004), (r_lun, 0.060), (r_lun * 0.7, 0.070), (r_lun * 0.7, 0.180), (r_lun * 1.25, 0.190), (r_lun * 1.25, 0.250), (0, 0.240)],
                               cores=["cinza_escuro"] * 6 + ["ciano"], seg=8, matriz=mat))
    if t2 >= 2:   # Tiro de Estilhacos: boca em leque
        for k, dx in enumerate((-0.5, 0, 0.5)):
            objs.append(pecas.cone(m, f"rifle_estilhaco_{k}", (0, -0.680, 0.010), (dx, -1, 0), 0.026, 0.110, "laranja", seg=4, matriz=mat))
    if t3 >= 3:   # tambor de municao (dois no tier 5)
        for k, y in enumerate((-0.090, -0.190)[:2 if t3 >= 5 else 1]):
            objs.append(pecas.cilindro(m, f"rifle_tambor_{k}", (-0.030, y, -0.085), (1, 0, 0), [(0, 0), (0.070, 0.004), (0.070, 0.056), (0, 0.060)],
                                       cor="amarelo" if t3 < 5 else "ouro", seg=10, matriz=mat))
    elif t3 >= 1:   # carregadores
        for k in range(t3):
            objs.append(pecas.bloco(m, f"rifle_carregador_{k}", (0.036, 0.062, 0.120), (0, -0.060 - 0.085 * k, -0.085), "amarelo", chanfro=0.008, matriz=mat))
    m.por("mao_ataque", objs)


def base(m):
    m.macaco(pelagem="lisa")
    m.por("chapeu", pecas.capacete(m, "verde_escuro", "marrom_escuro"))
    _rifle(m)


# ---------------------------------------------------------------- caminho 1: dano
def precisao_mortal(m):
    m.por("chapeu", pecas.capuz(m, "verde_escuro"))


def mutilar(m):
    m.por("costas", pecas.capa(m, "verde_escuro", borda="marrom_escuro", comp=0.44, largura=0.52, gola=False))


def aleijar(m):
    m.por("chapeu", pecas.capuz(m, "tinta", barra="vermelho"))
    m.por("costas", pecas.capa(m, "tinta", borda="vermelho", comp=0.48, largura=0.56, gola=False))
    m.por("tronco", pecas.colete(m, "tinta", barra="vermelho", emblema="ouro"))


# ---------------------------------------------------------------- caminho 2: apoio
def oculos(m):
    m.por("rosto", pecas.oculos(m, aro="verde", tira="tinta"))


def _boina(m, cor, fita):
    objs = [pecas.casca(m, "boina", cor, pecas.piso_lenco), pecas.barra_casca(m, "boina_fita", fita, pecas.piso_lenco)]
    objs.append(pecas.esfera(m, "boina_aba", (0.150, -0.060, 0.930), (0.150, 0.130, 0.050), cor, cortes=0))
    return objs


def _caixa(m, cor, fita):
    """Caixa de suprimentos com paraquedas dobrado em cima, ao lado do macaco."""
    c = (-0.520, 0.060, 0.110)
    objs = [pecas.bloco(m, "caixa", (0.220, 0.220, 0.200), c, cor, chanfro=0.020),
            pecas.bloco(m, "caixa_fita_x", (0.230, 0.050, 0.210), c, fita, chanfro=0.008),
            pecas.bloco(m, "caixa_fita_y", (0.050, 0.230, 0.210), c, fita, chanfro=0.008),
            pecas.esfera(m, "caixa_paraquedas", (c[0], c[1], 0.230), (0.120, 0.120, 0.070), "branco", cortes=1)]
    return objs


def ricochete(m):
    m.por("chapeu", _boina(m, "vermelho", "tinta"))
    m.por("tronco", pecas.bandoleira(m, "marrom_escuro"))


def suprimentos(m):
    m.somar("extra", _caixa(m, "marrom", "amarelo"), preso=True)


def elite(m):
    m.por("chapeu", _boina(m, "tinta", "ouro"))
    m.por("extra", _caixa(m, "ouro", "vermelho"), preso=True)
    m.por("costas", pecas.capa(m, "azul", borda="ouro", comp=0.44, largura=0.52))


# ---------------------------------------------------------------- caminho 3: cadencia
def semiautomatico(m):
    m.por("chapeu", pecas.faixa_testa(m, "vermelho"))


def automatico(m):
    m.por("tronco", pecas.colete(m, "cinza_escuro", barra="ouro") + pecas.bandoleira(m, "ouro"))


def defensor(m):
    m.por("chapeu", pecas.capacete(m, "cinza_escuro", "ouro", topo="vermelho"))
    m.somar("tronco", pecas.manga(m, -1, "cinza_escuro", luva="tinta", ombreira="ouro"))
    m.somar("extra", pecas.manga(m, 1, "cinza_escuro", luva="tinta", ombreira="ouro"), grupo="braco", preso=True)


PARAMETRO = [(None, None)]   # o tier muda o rifle, que le m.tier
CAMINHOS = [
    {1: PARAMETRO, 2: PARAMETRO, 3: precisao_mortal, 4: mutilar, 5: aleijar},
    {1: [("rosto", oculos)], 2: PARAMETRO, 3: ricochete, 4: suprimentos, 5: elite},
    {1: PARAMETRO, 2: PARAMETRO, 3: semiautomatico, 4: automatico, 5: defensor},
]
