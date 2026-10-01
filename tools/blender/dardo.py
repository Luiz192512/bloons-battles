"""Macaco Dardo (base): o macaco padrao com um lenco azul e um dardo na mao."""
import comum as c
from macaco import macaco

ALVO_PREVIA = (0, 0, 0.5)
DIST_PREVIA = 2.5


def construir():
    piv = macaco(pelagem="topete")
    # lenco no pescoco, na cor da categoria Primaria
    c.toro("lenco", (0, -0.01, 0.555), 0.150, 0.036, c.AZUL, "corpo", rot=(8, 0, 0))
    c.esfera("lenco_no", (0.085, -0.150, 0.535), (0.040, 0.034, 0.040), c.AZUL, "corpo")
    c.capsula("lenco_ponta", (0.090, -0.160, 0.525), (0.115, -0.185, 0.445), 0.024, c.AZUL, "corpo", raio2=0.012)
    # dardo: haste, ponta afilada de ponta macia e pena redonda
    mx, my, mz = piv["mao"]
    c.capsula("dardo_haste", (mx, my + 0.07, mz + 0.03), (mx, my - 0.22, mz + 0.03), 0.017, c.MARROM_ESCURO, "braco")
    c.capsula("dardo_ponta", (mx, my - 0.20, mz + 0.03), (mx, my - 0.33, mz + 0.03), 0.034, c.CINZA, "braco", raio2=0.008)
    c.esfera("dardo_pena_a", (mx, my + 0.085, mz + 0.03), (0.012, 0.050, 0.046), c.VERMELHO, "braco")
    c.esfera("dardo_pena_b", (mx, my + 0.085, mz + 0.03), (0.046, 0.050, 0.012), c.AMARELO, "braco")
    return piv
