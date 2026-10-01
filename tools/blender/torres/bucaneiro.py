"""Bucaneiro: o macaco padrao, de lenco de pirata, tapa-olho e luneta, em pe no conves do barco."""
import math

import bmesh
from mathutils import Matrix

import macaco_poli
import pecas
import poli

ENQUADRE = ((0, -0.05, 0.68), 2.5)

# secoes do casco, da popa a proa: (y, meia largura, fundo, borda)
CASCO = [(0.84, 0.26, 0.14, 0.40), (0.70, 0.36, 0.06, 0.36), (0.40, 0.44, 0.01, 0.30), (0.05, 0.47, 0.00, 0.27),
         (-0.30, 0.44, 0.00, 0.27), (-0.60, 0.34, 0.03, 0.30), (-0.84, 0.19, 0.10, 0.36), (-1.02, 0.05, 0.24, 0.43)]
CONVES = 0.075   # quanto o conves fica abaixo da borda
BORDA = 0.86     # largura interna da borda em relacao a externa
NO_CONVES = Matrix.Translation((0, -0.160, 0.195)) @ Matrix.Scale(0.80, 4)   # o macaco em pe no conves


def _secao(y, w, fundo, borda):
    """Anel fechado da secao: casco em U por fora, borda grossa em cima e conves por dentro."""
    pts = []
    for g in (-90, -62, -34, 0, 34, 62, 90):
        a = math.radians(g)
        pts.append((w * math.sin(a), y, borda - (borda - fundo) * math.cos(a) ** 0.75))
    wi, d = w * BORDA, borda - CONVES
    pts += [(wi, y, borda), (wi * 0.97, y, d), (0, y, d), (-wi * 0.97, y, d), (-wi, y, borda)]
    return pts


def barco(m):
    aneis = [_secao(*s) for s in CASCO]
    # por fora: faixa vermelha junto a borda e madeira no resto; borda escura; conves claro
    cores = ["vermelho", "marrom", "marrom", "marrom", "marrom", "vermelho", "marrom_escuro", "marrom_escuro",
             "bege", "bege", "marrom_escuro", "marrom_escuro"]
    p = [m.obj("casco", poli.loft(aneis, cores), nivel=1)]
    y_p, _w_p, _f_p, b_p = CASCO[-1]
    esporao = poli.torno([(0, 0), (0.050, 0.035), (0.030, 0.120), (0, 0.300)], seg=10, cor="ouro")
    p.append(m.obj("esporao", poli.orientar(esporao, (0, -1, 0.32), (0, y_p + 0.06, b_p - 0.07)), nivel=0, vivo=True))
    leme = poli.caixa((0.035, 0.20, 0.36), "marrom_escuro", chanfro=0.012, seg=1)
    for v in leme.verts:  # leme mais largo embaixo
        if v.co.z < 0:
            v.co.y *= 1.5
    poli.mover(leme, (0, 0.90, 0.20))
    p.append(m.obj("leme", leme, nivel=0, vivo=True))
    # mastro com cesto, vela enfunada com faixa, e bandeira triangular
    my = 0.44
    p.append(m.obj("mastro", poli.membro([(0, my, 0.18), (0, my, 0.80), (0, my, 1.42)], [0.034, 0.028, 0.020], cor="marrom_escuro"), nivel=1))
    cesto = poli.torno([(0, 0), (0.060, 0.005), (0.075, 0.070), (0.062, 0.075), (0.050, 0.030), (0, 0.030)], seg=12, cor="marrom")
    p.append(m.obj("cesto", poli.orientar(cesto, (0, 0, 1), (0, my, 1.26)), nivel=0, vivo=True))
    p.append(m.obj("verga", poli.membro([(-0.36, my - 0.03, 1.12), (0.36, my - 0.03, 1.12)], [0.020, 0.020], cor="marrom_escuro"), nivel=1))
    for nome, tam, cor, dz in (("vela", (0.66, 0.026, 0.56), "branco", 0.0), ("faixa", (0.664, 0.060, 0.12), "vermelho", 0.02)):
        bm = poli.caixa(tam, cor, chanfro=0.008, seg=1)
        bmesh.ops.subdivide_edges(bm, edges=[e for e in bm.edges if abs(e.verts[0].co.x - e.verts[1].co.x) > 0.3], cuts=5)
        for v in bm.verts:  # barriga da vela para a frente, mais cheia embaixo
            v.co.y -= 0.11 * (1 - (v.co.x / 0.34) ** 2) * (0.75 - 0.5 * (v.co.z + dz) / 0.56)
        poli.mover(bm, (0, my - 0.045, 0.83 + dz))
        p.append(m.obj(nome, bm, nivel=0))
    band = poli.caixa((0.30, 0.012, 0.16), "tinta", chanfro=0.004, seg=1)
    for v in band.verts:
        if v.co.x > 0:
            v.co.z *= 0.05
    poli.mover(band, (0.165, my, 1.36))
    p.append(m.obj("bandeira", band, nivel=0, vivo=True))
    # dois canhoes por bordo, apoiados na borda
    canhao = [(0, 0), (0.052, 0.010), (0.060, 0.090), (0.050, 0.240), (0.066, 0.248), (0.066, 0.285), (0.038, 0.288),
              (0.034, 0.220), (0, 0.210)]
    for sx in (-1, 1):
        for k, y in enumerate((0.26, -0.16)):
            bm = poli.torno(canhao, seg=14, cores=["cinza_escuro"] * 5 + ["tinta"] * 3)
            p.append(m.obj(f"canhao_{sx}_{k}", poli.orientar(bm, (sx, 0, 0.10), (0.27 * sx, y, 0.285)), nivel=0, vivo=True))
    return p


def lenco_pirata(m, cor="vermelho", barra="tinta"):
    """Lenco que cobre o cranio da testa ate a nuca, com a barra escura, o no e duas pontas."""
    cab = [pecas.casca(m, "lenco", cor, pecas.piso_lenco, cortes=3)]
    aro = [(0.268 * math.cos(math.radians(g)), -0.236 * math.sin(math.radians(g)) + 0.004,
            0.735 + 0.150 - 0.34 * (-0.236 * math.sin(math.radians(g)) + 0.20) + 0.004) for g in range(0, 361, 30)]
    cab.append(m.obj("lenco_barra", poli.membro(aro, [0.020] * len(aro), cor=barra), nivel=1))
    no = poli.gaiola((0.060, 0.052, 0.052), cortes=0, cor=cor)
    poli.mover(no, (0.150, 0.215, 0.790))
    cab.append(m.obj("lenco_no", no, nivel=1))
    for k, (dx, dz) in enumerate(((0.10, -0.16), (0.02, -0.20))):
        bm = poli.membro([(0.160, 0.235, 0.785), (0.160 + dx * 0.6, 0.285, 0.785 + dz * 0.5), (0.160 + dx, 0.300, 0.785 + dz)],
                         [0.036, 0.034, 0.004], cor=cor)
        cab.append(m.obj(f"lenco_ponta_{k}", bm, nivel=1))
    return cab


def tapa_olho(m):
    tapa = poli.gaiola((0.072, 0.024, 0.080), cortes=1, cor="tinta")
    poli.mover(tapa, (-0.086, -0.214, 0.792))
    return [m.obj("tapa_olho", tapa, nivel=1)]


def luneta(m):
    mx, my, mz = macaco_poli.MAO
    perfil = poli.torno([(0, 0), (0.030, 0.006), (0.032, 0.120), (0.044, 0.128), (0.046, 0.262), (0.032, 0.265),
                         (0.028, 0.200), (0, 0.190)], seg=14,
                        cores=["ouro", "cinza_escuro", "ouro", "cinza_escuro", "ouro", "tinta", "tinta"])
    return [m.obj("luneta", poli.orientar(perfil, (0, -1, 0.12), (mx, my + 0.075, mz + 0.020)), nivel=0, vivo=True)]


def base(m):
    m.macaco(pelagem="tufos")
    m.por("chapeu", lenco_pirata(m))
    m.por("rosto", tapa_olho(m))
    m.por("mao_ataque", luneta(m))
    m.matriz = NO_CONVES
    m.por("base", barco(m))
    m.pivo("base", (0, 0, 0))


CAMINHOS = [{}, {}, {}]   # os upgrades entram na Parte B
