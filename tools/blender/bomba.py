"""Canhao Bomba (base): maquina sem macaco. Carreta redonda com rodas e um cano gordo."""
import comum as c

ALVO_PREVIA = (0, 0, 0.32)
DIST_PREVIA = 2.3


def construir():
    # carreta e rodas (grupo base: nao gira)
    c.cubo("carreta", (0, 0.02, 0.17), (0.40, 0.50, 0.16), c.MARROM, "base", canto=0.07)
    for lado, sx in (("e", -1), ("d", 1)):
        c.toro(f"roda_{lado}", (0.245 * sx, 0.03, 0.15), 0.115, 0.045, c.TINTA, "base", rot=(0, 90, 0))
        c.esfera(f"calota_{lado}", (0.255 * sx, 0.03, 0.15), (0.035, 0.075, 0.075), c.OURO, "base")
    c.esfera("apoio", (0, 0.02, 0.27), (0.15, 0.17, 0.07), c.MARROM_ESCURO, "base")

    # cano (grupo torreta: gira para o alvo e recua no disparo), apontado para -Y e levemente para cima
    c.capsula("cano", (0, 0.20, 0.36), (0, -0.30, 0.46), 0.165, c.CINZA_ESCURO, "torreta", raio2=0.135)
    c.esfera("culatra", (0, 0.24, 0.355), (0.175, 0.17, 0.175), c.CINZA_ESCURO, "torreta")
    c.toro("boca", (0, -0.385, 0.478), 0.118, 0.034, c.TINTA, "torreta", rot=(79, 0, 0))
    c.toro("faixa", (0, -0.05, 0.410), 0.158, 0.026, c.AZUL, "torreta", rot=(79, 0, 0))
    c.toro("anel", (0, 0.13, 0.374), 0.168, 0.020, c.OURO, "torreta", rot=(79, 0, 0))
    # pavio com brasa
    c.capsula("pavio", (0, 0.30, 0.50), (0.03, 0.34, 0.60), 0.014, c.BEGE, "torreta")
    c.esfera("brasa", (0.035, 0.345, 0.615), 0.026, c.AMARELO, "torreta")

    return {"base": (0, 0, 0), "torreta": (0, 0.05, 0.38)}
