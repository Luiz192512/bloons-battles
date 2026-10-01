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
    # dardo: haste, ponta afilada de ponta macia e pena redonda
    mx, my, mz = piv["mao"]
    c.capsula("dardo_haste", (mx, my + 0.07, mz + 0.03), (mx, my - 0.22, mz + 0.03), 0.017, c.MARROM_ESCURO, "braco")
    c.cone("dardo_ponta", (mx, my - 0.20, mz + 0.03), (mx, my - 0.37, mz + 0.03), 0.040, c.CINZA, "braco")
    for k, cor in enumerate((c.VERMELHO, c.AMARELO, c.VERMELHO)):
        c.lamina(f"dardo_pena_{k}", (mx, my + 0.11, mz + 0.03), (mx, my - 0.02, mz + 0.03), 0.11, 0.012, cor, "braco",
                 giro=k * 60)
    # bico do lenco
    c.lamina("lenco_bico", (0.090, -0.165, 0.50), (0.135, -0.200, 0.38), 0.07, 0.014, c.AZUL, "corpo", giro=20)
    return piv
