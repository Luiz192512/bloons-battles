"""Macaco Engenheiro: o macaco padrao de topete, capacete laranja de obra, macacao azul, pistola de pregos e chave inglesa.

Caminho 1 (sentinelas): torretas ao lado; do tier 3 em diante, engrenagem nas costas e torretas melhores.
Caminho 2 (apoio): antena e chave maior; do tier 3 em diante, canhao de espuma e mochila de sobrecarga.
Caminho 3 (pregos): prego grande e cinto de pregos; do tier 3 em diante, duas pistolas e a armadilha.

As ferramentas saem de _ferramentas, que le os tres tiers (regra da torre) e vale em qualquer combinacao.
"""
import math

from mathutils import Vector

import pecas

ENQUADRE = ((0, 0.0, 0.45), 1.9)


def _pistola(m, lado, corpo="cinza", prego=1.0):
    """Pistola de pregos: corpo, cabo e o prego na ponta."""
    mao = pecas.MAO if lado > 0 else pecas.MAO_ESQ
    n = "pistola" if lado > 0 else "pistola_esq"
    mat = pecas.em((mao[0], mao[1] - 0.02, mao[2] + 0.05), (0.10 * -lado, -1, 0))
    return [pecas.bloco(m, n, (0.075, 0.230, 0.110), (0, -0.080, 0), corpo, chanfro=0.022, matriz=mat),
            pecas.bloco(m, n + "_carga", (0.060, 0.070, 0.110), (0, -0.030, -0.090), "amarelo", chanfro=0.012, matriz=mat),
            pecas.cilindro(m, n + "_prego", (0, -0.190, 0.010), (0, -1, 0), [(0.034 * prego, 0), (0.034 * prego, 0.014), (0.012 * prego, 0.018), (0.012 * prego, 0.120 * prego), (0, 0.170 * prego)],
                           cor="aco", seg=6, matriz=mat)]


def _chave(m, escala=1.0, cor="cinza"):
    """Chave inglesa na mao esquerda."""
    x, y, z = pecas.MAO_ESQ
    mat = pecas.em((x - 0.01, y - 0.03, z + 0.03), (-0.2, -0.5, 1), escala, de=(0, 0, 1))
    objs = [pecas.bloco(m, "chave", (0.040, 0.020, 0.280), (0, 0, 0.080), cor, chanfro=0.008, matriz=mat)]
    for sx in (-1, 1):
        objs.append(pecas.bloco(m, f"chave_boca_{sx}", (0.034, 0.024, 0.090), (0.040 * sx, 0, 0.250), cor, chanfro=0.008, matriz=mat))
    return objs


def _torreta(m, nome, pos, cor, cano="cinza_escuro", tam=1.0, luz=None):
    """Sentinela: tripe curto, caixa e cano."""
    p = Vector(pos)
    objs = [pecas.cilindro(m, nome + "_pe", p, (0, 0, 1), [(0, 0), (0.090 * tam, 0.004), (0.050 * tam, 0.040), (0.030 * tam, 0.130 * tam), (0, 0.130 * tam)], cor="cinza", seg=8),
            pecas.bloco(m, nome, (0.150 * tam, 0.170 * tam, 0.110 * tam), p + Vector((0, 0, 0.180 * tam)), cor, chanfro=0.020),
            pecas.tubo(m, nome + "_cano", [p + Vector((0, -0.060 * tam, 0.185 * tam)), p + Vector((0, -0.270 * tam, 0.185 * tam))], 0.024 * tam, cano, nivel=0)]
    if luz:
        objs.append(pecas.esfera(m, nome + "_luz", p + Vector((0, 0.020, 0.255 * tam)), 0.034 * tam, luz, cortes=0))
    return objs


def _ferramentas(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    # mao do ataque: pistola de pregos (prego maior com Pregos Enormes) ou canhao de espuma
    if p == 1 and t2 >= 3:
        mx, my, mz = pecas.MAO
        mat = pecas.em((mx, my - 0.02, mz + 0.05), (0, -1, 0), 1.0 + 0.12 * (t2 - 3))
        m.por("mao_ataque", [pecas.cilindro(m, "espuma", (0, 0.080, 0), (0, -1, 0), [(0, 0), (0.050, 0.010), (0.056, 0.200), (0.095, 0.300), (0.095, 0.340), (0.070, 0.340), (0.040, 0.200), (0, 0.200)],
                                            cores=["branco", "branco", "ciano", "ciano", "tinta", "tinta", "tinta"], seg=10, matriz=mat),
                             pecas.esfera(m, "espuma_bolha", (0, -0.300, 0), 0.070, "branco", cortes=0, matriz=mat)])
    else:
        m.por("mao_ataque", _pistola(m, 1, corpo="cinza" if not (p == 2 and t3 == 5) else "ouro", prego=1.6 if t3 >= 1 else 1.0))
    # mao livre: chave inglesa (maior e vermelha com Desconstrucao) ou a segunda pistola (Arma Dupla)
    if p == 2 and t3 >= 3:
        m.por("mao_livre", _pistola(m, -1, corpo="cinza" if t3 < 5 else "ouro", prego=1.6))
    else:
        m.por("mao_livre", _chave(m, 1.35 if t2 >= 2 else 1.0, "vermelho" if t2 >= 2 else "cinza"))
    # Area de Servico Maior: antena no capacete
    if t2 >= 1:
        m.somar("extra", [pecas.tubo(m, "antena", [(0.120, 0.060, 0.960), (0.150, 0.080, 1.200)], 0.012, "cinza", nivel=0),
                          pecas.esfera(m, "antena_luz", (0.152, 0.082, 1.225), 0.040, "ciano", cortes=0)], grupo="cabeca", preso=True)
    # Pino: cinto de pregos atravessado
    if t3 >= 2:
        cinto = pecas.bandoleira(m, "marrom_escuro")
        for k, a in enumerate((-135, -105, -75, -45)):
            a = math.radians(a)
            cinto.append(pecas.cone(m, f"cinto_prego_{k}", (0.188 * math.cos(a), -0.005 + 0.172 * math.sin(a) - 0.014, 0.370 - 0.105 * math.cos(a)), (0.2, -0.15, 1), 0.016, 0.110, "aco", seg=4, fechado=True))
        m.somar("tronco", cinto)
    # sentinelas ao lado
    n = {0: 0, 1: 1, 2: 2, 3: 2, 4: 3, 5: 3}[t1]
    lugares = [(0.520, -0.120, 0), (-0.540, -0.160, 0), (0.300, 0.480, 0)]
    cores = ["verde_escuro"] * 3
    tam, luz = 1.0, None
    if p == 0 and t1 >= 4:
        cores, luz = (["vermelho", "azul", "amarelo"], "branco") if t1 == 4 else (["ouro"] * 3, "ciano")
        tam = 1.15 if t1 == 4 else 1.40
    for k in range(n):
        m.somar("extra", _torreta(m, f"sentinela_{k}", lugares[k], cores[k], tam=tam, luz=luz, cano="cinza_escuro" if luz != "ciano" else "ciano"))


def base(m):
    m.macaco(pelagem="topete")
    m.tirar("pelagem")
    m.por("chapeu", pecas.capacete(m, "laranja", "branco"))
    m.por("tronco", pecas.colete(m, "azul", barra="amarelo", z0=0.250, z1=0.500))
    _ferramentas(m)


def engrenagens(m):
    t = m.tier[0]
    c = Vector((0, 0.235, 0.440))
    raio = 0.150 if t < 5 else 0.185
    roda = [pecas.cilindro(m, "engrenagem", c, (0, 1, 0.1), [(0, 0), (raio, 0.004), (raio, 0.050), (raio * 0.4, 0.058), (0, 0.058)], cor="amarelo" if t < 5 else "ouro", seg=12)]
    for k in range(8):
        a = math.radians(45 * k)
        roda.append(pecas.bloco(m, f"engrenagem_dente_{k}", (0.060, 0.050, 0.060), c + Vector((raio * 1.08 * math.cos(a), 0.028, raio * 1.08 * math.sin(a))), "amarelo" if t < 5 else "ouro", chanfro=0.010))
    m.por("costas", roda)


def apoio(m):
    t = m.tier[1]
    if t >= 4:
        m.por("costas", pecas.mochila(m, "cinza" if t == 4 else "ouro", luz="ciano", tam=1.10 if t == 4 else 1.35))
        m.por("chapeu", pecas.capacete(m, "laranja" if t == 4 else "ouro", "ciano", topo="ciano"))
    else:
        m.por("costas", pecas.tanque(m, "branco", tampa="ciano"))
    if t == 5:
        m.por("rosto", pecas.viseira(m, "ciano", "cinza_escuro"))


def pregos(m):
    t = m.tier[2]
    if t >= 4:   # armadilha no chao, na frente: aro com dentes e o miolo aceso
        raio = 0.200 if t == 4 else 0.300
        c = Vector((0, -0.560, 0))
        arm = [pecas.cilindro(m, "armadilha", c, (0, 0, 1), [(0, 0), (raio, 0.004), (raio, 0.050), (raio * 0.75, 0.060), (raio * 0.70, 0.030), (0, 0.030)],
                              cores=["cinza_escuro", "cinza_escuro" if t == 4 else "ouro", "cinza", "verde", "verde"], seg=14)]
        for k in range(8):
            a = math.radians(45 * k)
            d = Vector((math.cos(a), math.sin(a), 0))
            arm.append(pecas.cone(m, f"armadilha_dente_{k}", c + d * raio * 0.88 + Vector((0, 0, 0.040)), -d * 0.5 + Vector((0, 0, 1)), 0.030, 0.110, "aco" if t == 4 else "ouro", seg=4))
        m.somar("extra", arm)
    if t == 5:
        m.por("chapeu", pecas.capacete(m, "ouro", "branco", topo="vermelho"))


PARAMETRO = [(None, None)]   # o tier muda as ferramentas, que saem de _ferramentas (le m.tier)
CAMINHOS = [
    {1: PARAMETRO, 2: PARAMETRO, 3: engrenagens, 4: engrenagens, 5: engrenagens},
    {1: PARAMETRO, 2: PARAMETRO, 3: apoio, 4: apoio, 5: apoio},
    {1: PARAMETRO, 2: PARAMETRO, 3: pregos, 4: pregos, 5: pregos},
]
