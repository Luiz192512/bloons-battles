"""O macaco padrao: um corpo so para todas as torres.

Entre as torres mudam apenas a cor e o estilo da pelagem, a roupa e a arma. O unico macaco de
tamanho diferente e o Super Macaco (parametro escala).
"""
from mathutils import Vector

import comum as c

PELO = (150, 96, 52)
PELE = (240, 206, 160)


def macaco(pelo=PELO, pele=PELE, pelagem="lisa", origem=(0, 0, 0), escala=1.0, prefixo="m"):
    """Monta o macaco em pe sobre 'origem', olhando para -Y. Devolve os pivos dos grupos."""
    o = Vector(origem)
    e = escala

    def P(x, y, z):
        return o + Vector((x, y, z)) * e

    n = prefixo
    escuro = tuple(int(v * 0.72) for v in pelo)

    # pernas e pes (grupo corpo)
    for lado, sx in (("e", -1), ("d", 1)):
        c.capsula(f"{n}_perna_{lado}", P(0.085 * sx, 0, 0.26), P(0.10 * sx, -0.01, 0.07), 0.062 * e, pelo, "corpo")
        c.esfera(f"{n}_pe_{lado}", P(0.105 * sx, -0.045, 0.04), (0.075 * e, 0.11 * e, 0.045 * e), pele, "corpo")

    # tronco e barriga
    c.esfera(f"{n}_tronco", P(0, 0, 0.38), (0.185 * e, 0.165 * e, 0.20 * e), pelo, "corpo")
    c.esfera(f"{n}_barriga", P(0, -0.075, 0.36), (0.125 * e, 0.105 * e, 0.145 * e), pele, "corpo")

    # braco esquerdo solto ao lado do corpo
    c.capsula(f"{n}_braco_e", P(-0.17, 0, 0.47), P(-0.25, -0.02, 0.27), 0.052 * e, pelo, "corpo")
    c.esfera(f"{n}_mao_e", P(-0.26, -0.03, 0.235), 0.062 * e, pele, "corpo")

    # braco direito (o do ataque), dobrado para a frente
    ombro = P(0.17, 0, 0.47)
    c.capsula(f"{n}_braco_d", ombro, P(0.27, -0.10, 0.40), 0.052 * e, pelo, "braco")
    c.esfera(f"{n}_mao_d", P(0.285, -0.135, 0.405), 0.064 * e, pele, "braco")

    # cabeca grande, estilo boneco
    cab = P(0, -0.01, 0.74)
    c.esfera(f"{n}_cabeca", cab, (0.235 * e, 0.215 * e, 0.215 * e), pelo, "cabeca")
    c.esfera(f"{n}_rosto", P(0, -0.105, 0.725), (0.175 * e, 0.125 * e, 0.150 * e), pele, "cabeca")
    c.esfera(f"{n}_focinho", P(0, -0.185, 0.665), (0.105 * e, 0.075 * e, 0.070 * e), pele, "cabeca")
    c.esfera(f"{n}_nariz", P(0, -0.252, 0.685), (0.026 * e, 0.016 * e, 0.018 * e), c.TINTA, "cabeca")
    c.capsula(f"{n}_boca", P(-0.045, -0.243, 0.640), P(0.045, -0.243, 0.640), 0.009 * e, c.TINTA, "cabeca")
    for lado, sx in (("e", -1), ("d", 1)):
        c.esfera(f"{n}_olho_{lado}", P(0.075 * sx, -0.200, 0.775), (0.048 * e, 0.030 * e, 0.058 * e), c.BRANCO, "cabeca")
        c.esfera(f"{n}_pupila_{lado}", P(0.070 * sx, -0.226, 0.772), (0.024 * e, 0.014 * e, 0.030 * e), c.TINTA, "cabeca")
        c.esfera(f"{n}_orelha_{lado}", P(0.245 * sx, 0.01, 0.755), (0.050 * e, 0.085 * e, 0.095 * e), pelo, "cabeca")
        c.esfera(f"{n}_orelha_dentro_{lado}", P(0.262 * sx, -0.012, 0.755), (0.030 * e, 0.060 * e, 0.065 * e), pele, "cabeca")

    # pelagem: pecas de pelo por cima do corpo padrao
    if pelagem == "topete":
        for i, (x, y, z, r) in enumerate(((0, -0.06, 0.965, 0.060), (-0.055, -0.03, 0.950, 0.048),
                                          (0.055, -0.03, 0.950, 0.048))):
            c.esfera(f"{n}_topete_{i}", P(x, y, z), (r * e, r * 1.1 * e, r * 1.25 * e), escuro, "cabeca")
    elif pelagem == "crista":
        for i in range(5):
            c.esfera(f"{n}_crista_{i}", P(0, -0.10 + i * 0.055, 0.955 - abs(i - 2) * 0.012),
                     (0.035 * e, 0.045 * e, 0.065 * e), escuro, "cabeca")
    elif pelagem == "tufos":
        for i, sx in enumerate((-1, 1)):
            c.esfera(f"{n}_tufo_{i}", P(0.20 * sx, -0.02, 0.63), (0.070 * e, 0.060 * e, 0.075 * e), escuro, "cabeca")
    elif pelagem == "barba":
        c.esfera(f"{n}_barba", P(0, -0.165, 0.595), (0.125 * e, 0.075 * e, 0.075 * e), escuro, "cabeca")

    # cauda enrolada para tras
    base_cauda = P(0, 0.13, 0.26)
    c.arco(f"{n}_cauda", base_cauda + Vector((0, 0.16, 0.0)) * e, 0.16 * e, 180, 20, 0.032 * e, pelo, "cauda",
           plano="YZ", n=8, afina=0.75)

    return {
        "corpo": P(0, 0, 0.30),
        "cabeca": P(0, 0, 0.55),
        "braco": ombro,
        "cauda": base_cauda,
        "mao": P(0.285, -0.135, 0.405),
        "topo": P(0, 0, 0.96),
    }
