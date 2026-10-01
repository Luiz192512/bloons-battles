"""As torres do piloto em malha poligonal: dardo (macaco), bomba (maquina) e bucaneiro (macaco no barco)."""
import math

from mathutils import Matrix, Vector

import macaco_poli
import poli


def _anel(centro, rx, ry, g0, g1, n=10, inclina=0.0):
    pts = []
    for i in range(n + 1):
        a = math.radians(g0 + (g1 - g0) * i / n)
        pts.append((centro[0] + rx * math.cos(a), centro[1] + ry * math.sin(a), centro[2] + inclina * math.sin(a)))
    return pts


def dardo(c):
    partes, piv = macaco_poli.construir(c, pelagem="topete")
    # lenco azul: anel no pescoco, no e duas pontas
    extras = [poli.objeto("lenco", poli.membro(_anel((0, -0.005, 0.548), 0.150, 0.132, -60, 280, n=12, inclina=-0.012),
                                                  [0.034] * 13, cor="azul"), c, nivel=1)]
    no = poli.gaiola((0.046, 0.040, 0.046), cortes=0, cor="azul")
    poli.mover(no, (0.075, -0.135, 0.535))
    extras.append(poli.objeto("lenco_no", no, c, nivel=1))
    for k, dx in enumerate((0.030, -0.012)):
        extras.append(poli.objeto(f"lenco_ponta_{k}", poli.membro(
            [(0.075, -0.142, 0.525), (0.075 + dx * 0.6, -0.168, 0.465), (0.075 + dx, -0.172, 0.395)],
            [0.030, 0.027, 0.004], cor="azul"), c, nivel=1))
    partes["corpo"] = poli.juntar("corpo", [partes["corpo"]] + extras, c)

    # dardo na mao: haste, ponta de metal e penas cruzadas
    mx, my, mz = macaco_poli.MAO
    z = mz + 0.02
    arma = [poli.objeto("haste", poli.membro([(mx, my + 0.10, z), (mx, my - 0.20, z)], [0.016, 0.016], cor="marrom_escuro"), c, nivel=1)]
    ponta = poli.torno([(0, 0), (0.044, 0.025), (0.030, 0.085), (0, 0.205)], seg=12, cor="cinza")
    arma.append(poli.objeto("ponta", poli.orientar(ponta, (0, -1, 0), (mx, my - 0.185, z)), c, nivel=0, vivo=True))
    for nome, tam, cor in (("pena_v", (0.012, 0.115, 0.105), "vermelho"), ("pena_h", (0.105, 0.115, 0.012), "amarelo")):
        bm = poli.caixa(tam, cor, chanfro=0.004, seg=1)
        for v in bm.verts:  # afina para a frente, em forma de pena
            if v.co.y < 0:
                v.co.x *= 0.25
                v.co.z *= 0.25
        poli.mover(bm, (mx, my + 0.075, z))
        arma.append(poli.objeto(nome, bm, c, nivel=0, vivo=True))
    partes["braco"] = poli.juntar("braco", [partes["braco"]] + arma, c)
    return partes, piv


def bomba(c):
    base = []
    bm = poli.caixa((0.40, 0.54, 0.15), "marrom", chanfro=0.045, seg=3)
    poli.mover(bm, (0, 0.03, 0.175))
    base.append(poli.objeto("carreta", bm, c, nivel=0, vivo=True))
    berco = poli.gaiola((0.150, 0.200, 0.085), cortes=1, cor="marrom_escuro")
    poli.mover(berco, (0, 0.04, 0.265))
    base.append(poli.objeto("berco", berco, c, nivel=1))
    roda = [(0, -0.048), (0.070, -0.052), (0.085, -0.060), (0.150, -0.050), (0.168, -0.020), (0.168, 0.020),
            (0.150, 0.050), (0.085, 0.060), (0.070, 0.052), (0.045, 0.060), (0, 0.130)]
    cores_roda = ["ouro", "tinta", "tinta", "tinta", "tinta", "tinta", "tinta", "tinta", "ouro", "ouro"]
    for sx in (-1, 1):
        r = poli.torno(roda, seg=20, cores=cores_roda)
        base.append(poli.objeto(f"roda_{sx}", poli.orientar(r, (sx, 0, 0), (0.262 * sx, 0.03, 0.168)), c, nivel=0, vivo=True))

    # cano: perfil torneado com culatra redonda, aneis e a boca furada
    cano = [(0, 0), (0.105, 0.012), (0.160, 0.060), (0.178, 0.130), (0.178, 0.215), (0.192, 0.225), (0.192, 0.265),
            (0.176, 0.275), (0.156, 0.500), (0.170, 0.510), (0.170, 0.560), (0.154, 0.570), (0.146, 0.690),
            (0.172, 0.705), (0.172, 0.770), (0.128, 0.775), (0.116, 0.640), (0, 0.620)]
    cores = ["cinza_escuro"] * 5 + ["ouro"] + ["cinza_escuro"] * 3 + ["azul"] + ["cinza_escuro"] * 2 + ["tinta"] * 5
    torreta = [poli.objeto("cano", poli.orientar(poli.torno(cano, seg=24, cores=cores), (0, -1, 0.20), (0, 0.300, 0.335)),
                           c, nivel=0, vivo=True)]
    torreta.append(poli.objeto("pavio", poli.membro([(0, 0.215, 0.500), (0.012, 0.250, 0.585), (0.034, 0.262, 0.650)],
                                                    [0.014, 0.013, 0.011], cor="bege"), c, nivel=1))
    for nome, cor, direcao, comp in (("faisca", "amarelo", (0.3, 0.2, 1), 0.050), ("faisca2", "vermelho", (1, -0.4, 0.1), 0.045),
                                     ("faisca3", "amarelo", (-0.2, 1, 0.2), 0.045)):
        f = poli.torno([(0, -comp), (0.022, 0), (0, comp)], seg=6, cor=cor)
        torreta.append(poli.objeto(nome, poli.orientar(f, direcao, (0.036, 0.264, 0.660)), c, nivel=0, vivo=True))
    partes = {"base": poli.juntar("base", base, c), "torreta": poli.juntar("torreta", torreta, c)}
    return partes, {"base": (0, 0, 0), "torreta": (0, 0.05, 0.38)}


# secoes do casco, da popa a proa: (y, meia largura, fundo, borda)
CASCO = [(0.84, 0.26, 0.14, 0.40), (0.70, 0.36, 0.06, 0.36), (0.40, 0.44, 0.01, 0.30), (0.05, 0.47, 0.00, 0.27),
         (-0.30, 0.44, 0.00, 0.27), (-0.60, 0.34, 0.03, 0.30), (-0.84, 0.19, 0.10, 0.36), (-1.02, 0.05, 0.24, 0.43)]
CONVES = 0.075   # quanto o conves fica abaixo da borda
BORDA = 0.86     # largura interna da borda em relacao a externa


def _secao(y, w, fundo, borda):
    """Anel fechado da secao: casco em U por fora, borda grossa em cima e conves por dentro."""
    pts = []
    for g in (-90, -62, -34, 0, 34, 62, 90):
        a = math.radians(g)
        pts.append((w * math.sin(a), y, borda - (borda - fundo) * math.cos(a) ** 0.75))
    wi, d = w * BORDA, borda - CONVES
    pts += [(wi, y, borda), (wi * 0.97, y, d), (0, y, d), (-wi * 0.97, y, d), (-wi, y, borda)]
    return pts


def _barco(c):
    aneis = [_secao(*s) for s in CASCO]
    # por fora: faixa vermelha junto a borda e madeira no resto; borda escura; conves claro
    cores = ["vermelho", "marrom", "marrom", "marrom", "marrom", "vermelho", "marrom_escuro", "marrom_escuro",
             "bege", "bege", "marrom_escuro", "marrom_escuro"]
    pecas = [poli.objeto("casco", poli.loft(aneis, cores), c, nivel=1)]
    y_p, w_p, f_p, b_p = CASCO[-1]
    esporao = poli.torno([(0, 0), (0.050, 0.035), (0.030, 0.120), (0, 0.300)], seg=10, cor="ouro")
    pecas.append(poli.objeto("esporao", poli.orientar(esporao, (0, -1, 0.32), (0, y_p + 0.06, b_p - 0.07)), c, nivel=0, vivo=True))
    leme = poli.caixa((0.035, 0.20, 0.36), "marrom_escuro", chanfro=0.012, seg=1)
    for v in leme.verts:  # leme mais largo embaixo
        if v.co.z < 0:
            v.co.y *= 1.5
    poli.mover(leme, (0, 0.90, 0.20))
    pecas.append(poli.objeto("leme", leme, c, nivel=0, vivo=True))
    # mastro com cesto, vela enfunada com faixa, e bandeira triangular
    my = 0.44
    pecas.append(poli.objeto("mastro", poli.membro([(0, my, 0.18), (0, my, 0.80), (0, my, 1.42)], [0.034, 0.028, 0.020], cor="marrom_escuro"), c, nivel=1))
    cesto = poli.torno([(0, 0), (0.060, 0.005), (0.075, 0.070), (0.062, 0.075), (0.050, 0.030), (0, 0.030)], seg=12, cor="marrom")
    pecas.append(poli.objeto("cesto", poli.orientar(cesto, (0, 0, 1), (0, my, 1.26)), c, nivel=0, vivo=True))
    pecas.append(poli.objeto("verga", poli.membro([(-0.36, my - 0.03, 1.12), (0.36, my - 0.03, 1.12)], [0.020, 0.020], cor="marrom_escuro"), c, nivel=1))
    for nome, tam, cor, dz in (("vela", (0.66, 0.026, 0.56), "branco", 0.0), ("faixa", (0.664, 0.060, 0.12), "vermelho", 0.02)):
        bm = poli.caixa(tam, cor, chanfro=0.008, seg=1)
        import bmesh
        bmesh.ops.subdivide_edges(bm, edges=[e for e in bm.edges if abs(e.verts[0].co.x - e.verts[1].co.x) > 0.3], cuts=5)
        for v in bm.verts:  # barriga da vela para a frente, mais cheia embaixo
            v.co.y -= 0.11 * (1 - (v.co.x / 0.34) ** 2) * (0.75 - 0.5 * (v.co.z + dz) / 0.56)
        poli.mover(bm, (0, my - 0.045, 0.83 + dz))
        pecas.append(poli.objeto(nome, bm, c, nivel=0))
    band = poli.caixa((0.30, 0.012, 0.16), "tinta", chanfro=0.004, seg=1)
    for v in band.verts:
        if v.co.x > 0:
            v.co.z *= 0.05
    poli.mover(band, (0.165, my, 1.36))
    pecas.append(poli.objeto("bandeira", band, c, nivel=0, vivo=True))
    # dois canhoes por bordo, apoiados na borda
    canhao = [(0, 0), (0.052, 0.010), (0.060, 0.090), (0.050, 0.240), (0.066, 0.248), (0.066, 0.285), (0.038, 0.288),
              (0.034, 0.220), (0, 0.210)]
    for sx in (-1, 1):
        for k, y in enumerate((0.26, -0.16)):
            bm = poli.torno(canhao, seg=14, cores=["cinza_escuro"] * 5 + ["tinta"] * 3)
            pecas.append(poli.objeto(f"canhao_{sx}_{k}", poli.orientar(bm, (sx, 0, 0.10), (0.27 * sx, y, 0.285)), c, nivel=0, vivo=True))
    return poli.juntar("base", pecas, c)


def bucaneiro(c):
    # o macaco padrao, com lenco de pirata, tapa-olho e luneta, em pe no conves
    partes, piv = macaco_poli.construir(c, pelagem="tufos")
    # lenco: casca que cobre o cranio da testa (acima dos olhos) ate a nuca, mais baixa atras
    lenco = poli.gaiola((0.300, 0.268, 0.262), cortes=3, cor="vermelho")
    for v in lenco.verts:
        v.co.z = max(v.co.z, 0.150 - 0.34 * (v.co.y + 0.20))
    poli.mover(lenco, (0, 0.0, 0.735))
    cab = [poli.objeto("lenco", lenco, c, nivel=1)]
    # barra do lenco: faixa mais escura contornando a testa
    barra = [(0.268 * math.cos(math.radians(g)), -0.236 * math.sin(math.radians(g)) + 0.004,
              0.735 + 0.150 - 0.34 * (-0.236 * math.sin(math.radians(g)) + 0.20) + 0.004) for g in range(0, 361, 30)]
    cab.append(poli.objeto("lenco_barra", poli.membro(barra, [0.020] * len(barra), cor="tinta"), c, nivel=1))
    no = poli.gaiola((0.060, 0.052, 0.052), cortes=0, cor="vermelho")
    poli.mover(no, (0.150, 0.215, 0.790))
    cab.append(poli.objeto("lenco_no", no, c, nivel=1))
    for k, (dx, dz) in enumerate(((0.10, -0.16), (0.02, -0.20))):
        bm = poli.membro([(0.160, 0.235, 0.785), (0.160 + dx * 0.6, 0.285, 0.785 + dz * 0.5), (0.160 + dx, 0.300, 0.785 + dz)],
                         [0.036, 0.034, 0.004], cor="vermelho")
        cab.append(poli.objeto(f"lenco_ponta_{k}", bm, c, nivel=1))
    tapa = poli.gaiola((0.072, 0.024, 0.080), cortes=1, cor="tinta")
    poli.mover(tapa, (-0.086, -0.214, 0.792))
    cab.append(poli.objeto("tapa_olho", tapa, c, nivel=1))
    partes["cabeca"] = poli.juntar("cabeca", [partes["cabeca"]] + cab, c)
    mx, my, mz = macaco_poli.MAO
    luneta = poli.torno([(0, 0), (0.030, 0.006), (0.032, 0.120), (0.044, 0.128), (0.046, 0.262), (0.032, 0.265),
                         (0.028, 0.200), (0, 0.190)], seg=14,
                        cores=["ouro", "cinza_escuro", "ouro", "cinza_escuro", "ouro", "tinta", "tinta"])
    lun = poli.objeto("luneta", poli.orientar(luneta, (0, -1, 0.12), (mx, my + 0.075, mz + 0.020)), c, nivel=0, vivo=True)
    partes["braco"] = poli.juntar("braco", [partes["braco"], lun], c)

    m = Matrix.Translation((0, -0.160, 0.195)) @ Matrix.Scale(0.80, 4)
    for o in partes.values():
        o.data.transform(m)
    piv = {g: tuple(m @ Vector(p)) for g, p in piv.items()}
    partes["base"] = _barco(c)
    piv["base"] = (0, 0, 0)
    return partes, piv


TORRES = {"dardo": dardo, "bomba": bomba, "bucaneiro": bucaneiro}
# alvo e tamanho do enquadramento das folhas de revisao
ENQUADRE = {"dardo": ((0, 0.03, 0.52), 1.25), "bomba": ((0, -0.02, 0.33), 1.15), "bucaneiro": ((0, -0.05, 0.68), 2.5)}
