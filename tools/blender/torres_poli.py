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


def bucaneiro(c):
    # casco: uma gaiola so, achatada em cima, afinada e erguida na proa, com o conves rebaixado
    bm = poli.gaiola((0.40, 0.76, 0.21), cortes=3, cor="marrom")
    for v in bm.verts:
        if v.co.z > 0.02:
            v.co.z = 0.02 + (v.co.z - 0.02) * 0.12
        t = max(0.0, -v.co.y / 0.76)
        v.co.x *= 1.0 - 0.80 * t ** 1.6
        v.co.z += 0.13 * t ** 2
        s = max(0.0, v.co.y / 0.76)
        v.co.x *= 1.0 - 0.25 * s ** 2
    bm.normal_update()
    topo = [f for f in bm.faces if f.normal.z > 0.75]
    topo = poli.extrudir(bm, topo, escala=(0.84, 0.90, 1.0), cor="marrom_escuro")
    poli.extrudir(bm, topo, desloc=(0, 0, -0.050), escala=0.97, cor="bege")
    poli.mover(bm, (0, 0, 0.21))
    base = [poli.objeto("casco", bm, c, nivel=1)]
    esporao = poli.torno([(0, 0), (0.048, 0.030), (0, 0.240)], seg=10, cor="ouro")
    base.append(poli.objeto("esporao", poli.orientar(esporao, (0, -1, 0.30), (0, -0.700, 0.330)), c, nivel=0, vivo=True))
    base.append(poli.objeto("mastro", poli.membro([(0, 0.400, 0.150), (0, 0.400, 1.120)], [0.030, 0.020], cor="marrom_escuro"), c, nivel=1))
    for nome, tam, cor, raio, dy in (("vela", (0.54, 0.030, 0.46), "branco", 0.27, 0.375), ("faixa", (0.545, 0.034, 0.105), "verde", 0.2725, 0.373)):
        v_bm = poli.caixa(tam, cor, chanfro=0.010, seg=1)
        for v in v_bm.verts:  # vela enfunada para a frente
            v.co.y -= 0.060 * (1 - (v.co.x / raio) ** 2)
        poli.mover(v_bm, (0, dy, 0.760))
        base.append(poli.objeto(nome, v_bm, c, nivel=0, vivo=True))
    band = poli.membro([(0.010, 0.400, 1.070), (0.150, 0.400, 1.070), (0.290, 0.400, 1.070)], [0.060, 0.035, 0.004], cor="vermelho")
    for v in band.verts:
        v.co.y = 0.400 + (v.co.y - 0.400) * 0.15
    base.append(poli.objeto("bandeira", band, c, nivel=1))
    canhao = [(0, 0), (0.050, 0.010), (0.058, 0.090), (0.048, 0.220), (0.062, 0.228), (0.062, 0.262), (0.036, 0.265),
              (0.032, 0.200), (0, 0.190)]
    for sx in (-1, 1):
        k = poli.torno(canhao, seg=14, cores=["cinza_escuro"] * 5 + ["tinta"] * 3)
        base.append(poli.objeto(f"canhao_{sx}", poli.orientar(k, (sx, 0, 0.06), (0.200 * sx, 0.130, 0.250)), c, nivel=0, vivo=True))

    # o macaco padrao, com lenco de pirata, tapa-olho e luneta, em pe no conves
    partes, piv = macaco_poli.construir(c, pelagem="tufos")
    lenco = poli.gaiola((0.275, 0.246, 0.232), cortes=2, cor="vermelho")
    for v in lenco.verts:
        v.co.z = max(v.co.z, 0.105)
    poli.mover(lenco, (0, 0.0, 0.740))
    cab = [poli.objeto("lenco", lenco, c, nivel=1)]
    no = poli.gaiola((0.052, 0.046, 0.046), cortes=0, cor="vermelho")
    poli.mover(no, (0.235, 0.060, 0.870))
    cab.append(poli.objeto("lenco_no", no, c, nivel=1))
    for k, dz in enumerate((0.0, -0.055)):
        cab.append(poli.objeto(f"lenco_ponta_{k}", poli.membro(
            [(0.245, 0.070, 0.865 + dz * 0.3), (0.300, 0.110, 0.810 + dz), (0.345, 0.150, 0.740 + dz * 1.6)],
            [0.030, 0.026, 0.004], cor="vermelho"), c, nivel=1))
    tapa = poli.gaiola((0.070, 0.022, 0.078), cortes=1, cor="tinta")
    poli.mover(tapa, (-0.086, -0.212, 0.792))
    cab.append(poli.objeto("tapa_olho", tapa, c, nivel=1))
    partes["cabeca"] = poli.juntar("cabeca", [partes["cabeca"]] + cab, c)
    mx, my, mz = macaco_poli.MAO
    luneta = poli.torno([(0, 0), (0.030, 0.006), (0.032, 0.120), (0.044, 0.128), (0.046, 0.262), (0.032, 0.265),
                         (0.028, 0.200), (0, 0.190)], seg=14,
                        cores=["ouro", "cinza_escuro", "ouro", "cinza_escuro", "ouro", "tinta", "tinta"])
    lun = poli.objeto("luneta", poli.orientar(luneta, (0, -1, 0.12), (mx, my + 0.075, mz + 0.020)), c, nivel=0, vivo=True)
    partes["braco"] = poli.juntar("braco", [partes["braco"], lun], c)

    m = Matrix.Translation((0, -0.090, 0.168)) @ Matrix.Scale(0.80, 4)
    for o in partes.values():
        o.data.transform(m)
    piv = {g: tuple(m @ Vector(p)) for g, p in piv.items()}
    partes["base"] = poli.juntar("base", base, c)
    piv["base"] = (0, 0, 0)
    return partes, piv


TORRES = {"dardo": dardo, "bomba": bomba, "bucaneiro": bucaneiro}
# alvo e tamanho do enquadramento das folhas de revisao
ENQUADRE = {"dardo": ((0, 0.03, 0.52), 1.25), "bomba": ((0, -0.02, 0.33), 1.15), "bucaneiro": ((0, -0.05, 0.55), 2.0)}
