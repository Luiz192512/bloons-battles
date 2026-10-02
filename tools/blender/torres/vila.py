"""Vila dos Macacos: construcao sem macaco. Cabana redonda de telhado em cone, com porta (malha base), e o mastro com a bandeira (malha torreta).

Caminho 1 (treino): mastro alto e tambores; do tier 3 em diante, alvo de treino, segunda cabana e balista.
Caminho 2 (inteligencia): antena e radar; do tier 3 em diante, predio de agencia e fortaleza.
Caminho 3 (comercio): placa de moeda e toldo; do tier 3 em diante, casas, predios e a cidade.

Tudo sai de _vila, que le os tres tiers (regra da torre) e vale em qualquer combinacao.
"""
import math

from mathutils import Vector

import pecas

ENQUADRE = ((0, 0.0, 0.45), 2.0)


def _cabana(m, nome, pos, raio, parede, telhado, alto=0.300):
    x, y = pos
    objs = [pecas.cilindro(m, nome, (x, y, 0), (0, 0, 1), [(0, 0), (raio, 0.004), (raio, alto), (raio * 1.25, alto + 0.010), (raio * 0.55, alto + raio * 0.95), (0, alto + raio * 1.45)],
                           cores=[parede, parede, telhado, telhado, telhado], seg=12)]
    objs.append(pecas.bloco(m, nome + "_porta", (raio * 0.50, 0.030, alto * 0.70), (x, y - raio + 0.004, alto * 0.35), "marrom_escuro", chanfro=0.010))
    return objs


def _predio(m, nome, pos, tam, cor, teto, faixa=None):
    x, y = pos
    lx, ly, lz = tam
    objs = [pecas.bloco(m, nome, tam, (x, y, lz / 2), cor, chanfro=0.020), pecas.bloco(m, nome + "_teto", (lx * 1.08, ly * 1.08, 0.040), (x, y, lz + 0.020), teto, chanfro=0.012)]
    if faixa:
        for k in range(max(1, int(lz / 0.2))):
            objs.append(pecas.bloco(m, f"{nome}_janela_{k}", (lx * 0.70, 0.016, 0.060), (x, y - ly / 2 - 0.002, 0.140 + 0.200 * k), faixa, chanfro=0.006))
    return objs


def _vila(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    treino, intel, comercio = (p == 0 and t1 >= 3), (p == 1 and t2 >= 3), (p == 2 and t3 >= 3)
    parede, telhado = "bege", "laranja"
    if treino:
        telhado = {3: "azul", 4: "azul", 5: "vermelho"}[t1]
    movel, pivo = [], (0, 0, 0)   # malha propria (torreta): o mastro com a bandeira, para o jogo poder tremular
    b = [pecas.cilindro(m, "chao", (0, 0, 0), (0, 0, 1), [(0, 0), (0.660, 0.004), (0.660, 0.024), (0, 0.030)], cor="marrom", seg=18)]
    topo = 0.950   # altura do alto do telhado, onde vai o mastro
    if intel:   # predio de agencia e fortaleza
        cor, teto = {3: ("cinza", "cinza_escuro"), 4: ("cinza", "vermelho"), 5: ("cinza_escuro", "ouro")}[t2]
        alt = {3: 0.460, 4: 0.520, 5: 0.640}[t2]
        b += _predio(m, "agencia", (0, 0.060), (0.560, 0.460, alt), cor, teto, faixa="ciano")
        topo = alt + 0.040
        if t2 == 5:   # torres de canto e muralha
            for sx in (-1, 1):
                for sy in (-1, 1):
                    b.append(pecas.cilindro(m, f"torre_{sx}_{sy}", (0.400 * sx, 0.060 + 0.340 * sy, 0), (0, 0, 1), [(0, 0), (0.090, 0.004), (0.090, 0.480), (0.115, 0.490), (0.115, 0.560), (0, 0.560)],
                                            cores=["cinza", "cinza", "ouro", "ouro", "cinza_escuro"], seg=8))
    elif comercio:   # casas, predios e a cidade
        b += _cabana(m, "cabana", (-0.280, 0.120), 0.200, parede, "laranja", alto=0.240)
        b += _cabana(m, "cabana_2", (0.300, -0.120), 0.170, parede, "vermelho", alto=0.200)
        alturas = {3: (0.340,), 4: (0.520, 0.380), 5: (0.820, 0.560, 0.420)}[t3]
        for k, alt in enumerate(alturas):
            x, y = ((0.200, 0.260), (-0.120, 0.380), (0.440, 0.300))[k]
            b += _predio(m, f"predio_{k}", (x, y), (0.240, 0.220, alt), "azul" if k == 0 else "cinza", "ouro" if (t3 == 5 and k == 0) else "cinza_escuro", faixa="ciano" if t3 >= 4 else "branco")
        topo = alturas[0] + 0.040
        if t3 == 5:
            movel.append(pecas.cone(m, "antena_cidade", (0.200, 0.260, 0.860), (0, 0, 1), 0.030, 0.240, "ouro", seg=5))
            pivo = (0.200, 0.260, 0.860)
    else:
        b += _cabana(m, "cabana", (0, 0.060), 0.340, parede, telhado, alto=0.300)
        topo = 0.300 + 0.340 * 1.45
        if treino and t1 >= 4:   # segunda cabana, menor
            b += _cabana(m, "cabana_2", (0.440, -0.240), 0.190, parede, telhado, alto=0.220)
    cx, cy = (0, 0.060) if not comercio else (0.200, 0.260)
    # mastro com bandeira (mais alto com Raio Maior)
    if not (comercio and t3 == 5):
        alto = 0.200 + (0.220 if t1 >= 1 else 0)
        pivo = (cx, cy, topo - 0.040)
        movel.append(pecas.tubo(m, "mastro", [(cx, cy, topo - 0.040), (cx, cy, topo + alto)], 0.014, "marrom_escuro", nivel=0))
        movel.append(pecas.cone(m, "bandeira", (cx, cy, topo + alto - 0.060), (1, 0.1, 0), 0.060, 0.240 + (0.080 if t1 >= 1 else 0), "vermelho" if not intel else "azul", seg=4, fechado=True))
    # caminho 1: tambores, alvo de treino e balista
    if t1 >= 2:
        for k, x in enumerate((-0.380, -0.520)):
            b.append(pecas.cilindro(m, f"tambor_{k}", (x, -0.360 + 0.110 * k, 0.020), (0, 0, 1), [(0, 0), (0.070, 0.004), (0.085, 0.080), (0.070, 0.150), (0.066, 0.156), (0, 0.158)],
                                    cores=["marrom", "marrom", "marrom", "bege", "bege"], seg=10))
    if treino:
        b.append(pecas.tubo(m, "alvo_haste", [(0.420, 0.240, 0.020), (0.420, 0.240, 0.300)], 0.016, "marrom", nivel=0))
        b.append(pecas.cilindro(m, "alvo", (0.420, 0.220, 0.330), (0, -1, 0), [(0, 0), (0.120, 0.002), (0.120, 0.020), (0.070, 0.024), (0.070, 0.028), (0.028, 0.032), (0, 0.036)],
                                cores=["branco", "branco", "vermelho", "branco", "vermelho", "vermelho"], seg=12))
        if t1 == 5:   # balista gigante em cima do telhado
            c = Vector((0, 0.060, topo - 0.150))
            b.append(pecas.bloco(m, "balista", (0.070, 0.560, 0.070), c + Vector((0, -0.100, 0)), "marrom_escuro", chanfro=0.014))
            b.append(pecas.tubo(m, "balista_arco", [c + Vector((-0.400, -0.180, 0)), c + Vector((-0.200, -0.300, 0)), c + Vector((0, -0.340, 0)), c + Vector((0.200, -0.300, 0)), c + Vector((0.400, -0.180, 0))],
                                [0.012, 0.030, 0.036, 0.030, 0.012], "ouro"))
            b.append(pecas.cone(m, "balista_flecha", c + Vector((0, -0.340, 0.050)), (0, -1, 0), 0.036, 0.220, "aco", seg=5, fechado=True))
    # caminho 2, tiers 1 e 2: antena roxa e prato de radar
    if t2 >= 1:
        b.append(pecas.tubo(m, "bloqueador", [(-0.300, 0.300, 0.020), (-0.300, 0.300, 0.560)], 0.016, "cinza", nivel=0))
        b.append(pecas.esfera(m, "bloqueador_luz", (-0.300, 0.300, 0.600), 0.055, "roxo", cortes=0))
    if t2 >= 2:
        b.append(pecas.cilindro(m, "radar", (-0.300, 0.280, 0.400), (0, -0.7, 1), [(0, 0), (0.130, 0.060), (0.120, 0.070), (0, 0.020)], cores=["cinza", "branco", "branco"], seg=10))
    if intel and t2 >= 4:   # corneta do chamado as armas
        b.append(pecas.cilindro(m, "corneta", (0.200, -0.180, topo + 0.040), (0, -1, 0.3), [(0.020, 0), (0.030, 0.100), (0.090, 0.200), (0.080, 0.200), (0, 0.090)], cores=["ouro", "ouro", "tinta", "tinta"], seg=8))
    # caminho 3, tiers 1 e 2: placa de moeda e toldo de comercio
    if t3 >= 1 and not comercio:
        b.append(pecas.tubo(m, "placa_haste", [(0.460, -0.300, 0.020), (0.460, -0.300, 0.300)], 0.014, "marrom_escuro", nivel=0))
        b.append(pecas.cilindro(m, "placa_moeda", (0.460, -0.320, 0.360), (0, -1, 0), [(0, 0), (0.090, 0.002), (0.090, 0.024), (0, 0.028)], cor="ouro", seg=12))
    if t3 >= 2 and not comercio:
        for j in range(4):
            b.append(pecas.bloco(m, f"toldo_{j}", (0.080, 0.200, 0.026), (-0.120 + 0.080 * j, -0.380, 0.260), "verde" if j % 2 == 0 else "branco", chanfro=0.008))
        b.append(pecas.bloco(m, "balcao", (0.320, 0.110, 0.130), (0, -0.400, 0.090), "marrom", chanfro=0.014))
    m.por("base", b)
    m.pivo("base", (0, 0, 0))
    m.por("torreta", movel)
    m.pivo("torreta", pivo)


def base(m):
    _vila(m)


PARAMETRO = [(None, None)]   # o tier muda a vila, que le m.tier
CAMINHOS = [{1: PARAMETRO, 2: PARAMETRO, 3: _vila, 4: _vila, 5: _vila} for _ in range(3)]
