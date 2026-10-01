"""Macaco Bucaneiro (base): o macaco padrao em cima de um barco redondo, com lenco de pirata."""
import comum as c
from macaco import macaco

ALVO_PREVIA = (0, 0, 0.55)
DIST_PREVIA = 4.6


def construir():
    # barco (grupo base): casco em forma de tigela comprida, borda, conves e canhoes laterais
    c.esfera("casco", (0, 0, 0.12), (0.42, 0.72, 0.22), c.MARROM, "base")
    c.esfera("conves", (0, 0, 0.305), (0.33, 0.60, 0.060), c.BEGE, "base")
    c.toro("borda", (0, 0, 0.255), 0.50, 0.040, c.MARROM_ESCURO, "base").scale = (0.80, 1.36, 1.0)
    c.esfera("proa", (0, -0.70, 0.27), (0.060, 0.085, 0.060), c.OURO, "base")
    for lado, sx in (("e", -1), ("d", 1)):
        c.capsula(f"canhao_{lado}", (0.26 * sx, 0.10, 0.39), (0.47 * sx, 0.10, 0.40), 0.058, c.CINZA_ESCURO, "base",
                  raio2=0.048)
        c.toro(f"canhao_boca_{lado}", (0.495 * sx, 0.10, 0.401), 0.040, 0.014, c.TINTA, "base", rot=(0, 90, 0))
    # mastro curto com vela arredondada e bandeira (atras do macaco)
    c.capsula("mastro", (0, 0.36, 0.24), (0, 0.36, 1.10), 0.028, c.MARROM_ESCURO, "base")
    c.cubo("vela", (0, 0.335, 0.74), (0.50, 0.035, 0.46), c.BRANCO, "base", canto=0.016)
    c.cubo("vela_faixa", (0, 0.330, 0.74), (0.505, 0.030, 0.10), c.VERDE, "base", canto=0.013)
    c.esfera("bandeira", (0.075, 0.36, 1.10), (0.085, 0.010, 0.050), c.VERMELHO, "base")

    # o macaco em pe no conves, na frente do mastro
    piv = macaco(pelo=(128, 84, 50), pelagem="tufos", origem=(0, -0.10, 0.345), escala=0.80, prefixo="pirata")
    ox, oy, oz = 0, -0.10, 0.345
    e = 0.80
    # lenco de pirata na cabeca (meia esfera vermelha) com no lateral, e tapa-olho
    c.esfera("lenco", (ox, oy - 0.005 * e, oz + 0.815 * e), (0.240 * e, 0.222 * e, 0.150 * e), c.VERMELHO, "cabeca")
    c.esfera("lenco_no", (ox + 0.215 * e, oy + 0.04 * e, oz + 0.800 * e), (0.050 * e, 0.045 * e, 0.045 * e), c.VERMELHO, "cabeca")
    c.capsula("lenco_ponta", (ox + 0.225 * e, oy + 0.05 * e, oz + 0.790 * e), (ox + 0.265 * e, oy + 0.09 * e, oz + 0.690 * e),
              0.026 * e, c.VERMELHO, "cabeca", raio2=0.012 * e)
    c.esfera("tapa_olho", (ox - 0.075 * e, oy - 0.222 * e, oz + 0.775 * e), (0.052 * e, 0.018 * e, 0.060 * e), c.TINTA, "cabeca")
    # luneta na mao do ataque
    mx, my, mz = piv["mao"]
    c.capsula("luneta", (mx, my + 0.04, mz + 0.02), (mx, my - 0.20, mz + 0.05), 0.030, c.OURO, "braco", raio2=0.040)

    piv["base"] = (0, 0, 0)
    return piv
