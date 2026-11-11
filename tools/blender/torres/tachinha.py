"""Atirador de Tachinhas: maquina sem macaco. Prato de metal (base) e tambor giratorio com bicos em volta (torreta).

Caminho 1 (fogo): engrenagens no topo; do tier 3 em diante, bicos em brasa e anel de chamas.
Caminho 2 (laminas): bicos mais compridos; do tier 3 em diante, laminas no lugar dos bicos.
Caminho 3 (mais tachinhas): mais bicos em volta; do tier 3 em diante, tambor mais alto e duas fileiras.

A torreta sai de _torreta, que le os tres tiers (regra da torre): engrenagens (caminho 1),
comprimento dos bicos (caminho 2) e numero de bicos (caminho 3) valem em qualquer combinacao.
"""
import math

from mathutils import Matrix, Vector

import pecas

ENQUADRE = ((0, 0, 0.26), 1.5)
R_TAMBOR = 0.215


def _prato(m, cor="cinza_escuro", aro="cinza", raio=0.340):
    perfil = [(0, 0), (raio, 0.004), (raio, 0.050), (raio * 0.92, 0.085), (raio * 0.70, 0.105), (0, 0.105)]
    return [pecas.cilindro(m, "prato", (0, 0, 0), (0, 0, 1), perfil, cores=[cor, aro, aro, cor, cor], seg=20)]


def _chamas(m, raio, altura, cores, n=12, nome="chama"):
    """Anel de chamas em volta do prato: pontas alternando duas cores."""
    objs = []
    for k in range(n):
        a = 2 * math.pi * k / n
        d = Vector((math.cos(a), math.sin(a), 0))
        h = altura * (1.0 if k % 2 == 0 else 0.68)
        objs.append(pecas.cone(m, f"{nome}_{k}", d * raio + Vector((0, 0, 0.030)), d * 0.45 + Vector((0, 0, 1)), 0.075, h, cores[k % 2], seg=5, fechado=True))
    return objs


def _engrenagem(m, z, raio, cor, nome):
    objs = [pecas.cilindro(m, nome, (0, 0, z), (0, 0, 1), [(0, 0), (raio, 0.002), (raio, 0.045), (raio * 0.45, 0.050), (0, 0.050)], cor=cor, seg=12)]
    for k in range(8):
        a = 2 * math.pi * k / 8
        objs.append(pecas.bloco(m, f"{nome}_dente_{k}", (0.050, 0.050, 0.044), (raio * 1.08 * math.cos(a), raio * 1.08 * math.sin(a), z + 0.023), cor, chanfro=0.008,
                                matriz=None))
    return objs


def _torreta(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    fogo, lamina, zona = (p == 0 and t1 >= 3), (p == 1 and t2 >= 3), (p == 2 and t3 >= 3)
    n = {0: 8, 1: 10, 2: 12, 3: 16, 4: 16, 5: 16}[t3]
    comp = 1.0 + 0.30 * min(t2, 2)
    alto = 0.090 if zona else 0.0
    cor, cap, bico = "vermelho", "ouro", "cinza"
    if fogo:
        cor, cap, bico = ("vermelho", "ouro", "laranja") if t1 == 3 else ("tinta", "laranja" if t1 == 4 else "amarelo", "laranja")
    elif lamina:
        cor, cap, bico = ("azul", "cinza", "aco") if t2 < 5 else ("tinta", "ouro", "ouro")
    elif zona and t3 == 5:
        cor, cap, bico = "ouro", "vermelho", "cinza_escuro"
    topo = 0.420 + alto
    perfil = [(0, 0.090), (R_TAMBOR * 0.90, 0.095), (R_TAMBOR, 0.130), (R_TAMBOR, topo - 0.040), (R_TAMBOR * 0.92, topo), (R_TAMBOR * 0.62, topo + 0.070),
              (R_TAMBOR * 0.25, topo + 0.105), (0, topo + 0.110)]
    objs = [pecas.cilindro(m, "tambor", (0, 0, 0), (0, 0, 1), perfil, cores=["cinza_escuro", cor, cor, cor, cap, cap, cap], seg=16)]
    fileiras = [0.265] if not (t3 == 5 or (lamina and t2 == 5)) else [0.225, 0.385 if zona else 0.345]
    if zona and t3 < 5:
        fileiras = [0.300]
    for f, z in enumerate(fileiras):
        for k in range(n):
            a = 2 * math.pi * (k + 0.5 * f) / n
            d = Vector((math.cos(a), math.sin(a), 0))
            pos = d * (R_TAMBOR - 0.020) + Vector((0, 0, z))
            if lamina:   # lamina deitada, saindo do tambor
                achata = Matrix.Translation((0, 0, z)) @ Matrix.Diagonal((1, 1, 0.22, 1)) @ Matrix.Translation((0, 0, -z))
                objs.append(pecas.cone(m, f"lamina_{f}_{k}", pos, d + Vector((-d.y, d.x, 0)) * 0.45, 0.085, (0.200 if t2 == 3 else 0.250) * comp / 1.6, bico, seg=4,
                                       fechado=True, matriz=achata))
            else:
                L = 0.120 * comp
                objs.append(pecas.cilindro(m, f"bico_{f}_{k}", pos, d, [(0.038, 0), (0.038, L * 0.70), (0.050, L * 0.74), (0.050, L), (0.026, L), (0.022, L * 0.5)],
                                           cores=[bico, bico, bico, "tinta", "tinta"], seg=6))
    # caminho 1: engrenagens no topo (rapidez); do tier 3 em diante, cinta em brasa
    for k in range(min(t1, 2)):
        objs += _engrenagem(m, topo + 0.085 + 0.060 * k, 0.110 - 0.030 * k, "ouro" if not fogo else "laranja", f"engrenagem_{k}")
    if fogo:
        objs.append(pecas.toro(m, "cinta_brasa", (0, 0, 0.180), R_TAMBOR * 1.02, 0.026, "laranja" if t1 < 5 else "amarelo", seg=16, lados=4))
    if lamina and t2 >= 4:   # lamina grande girando no topo
        objs += pecas.glaive(m, "lamina_topo", (0, 0, topo + 0.130), 0.170 if t2 == 4 else 0.215, lamina=bico, miolo="vermelho", laminas=8)
    if zona and t3 >= 4:   # sobrecarga: bobinas acesas e antena
        for k, z in enumerate((0.170, topo - 0.060)):
            objs.append(pecas.toro(m, f"bobina_{k}", (0, 0, z), R_TAMBOR * 1.04, 0.024, "ciano", seg=16, lados=4))
        objs.append(pecas.cone(m, "antena", (0, 0, topo + 0.090), (0, 0, 1), 0.030, 0.210, "ciano", seg=5))
    m.por("torreta", objs)
    m.pivo("torreta", (0, 0, 0.100))

    base = _prato(m, raio=0.340 if not (zona and t3 == 5) else 0.400, aro="cinza" if not (zona and t3 == 5) else "ouro",
                  cor="cinza_escuro" if not (fogo and t1 >= 4) else "tinta")
    if fogo and t1 >= 4:
        base += _chamas(m, 0.330, 0.230 if t1 == 4 else 0.330, ("laranja", "amarelo") if t1 == 4 else ("vermelho", "amarelo"), n=12 if t1 == 4 else 16)
    m.por("base", base)
    m.pivo("base", (0, 0, 0))


def base(m):
    _torreta(m)


PARAMETRO = [(None, None)]   # o tier muda a torreta, que le m.tier
CAMINHOS = [{1: PARAMETRO, 2: PARAMETRO, 3: _torreta, 4: _torreta, 5: _torreta} for _ in range(3)]
