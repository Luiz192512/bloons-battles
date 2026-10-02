"""Bucaneiro: o macaco padrao, de lenco de pirata, tapa-olho e luneta, em pe no conves do barco.

Caminho 1 (marinha): flamula e canhao de proa; do tier 3 em diante, destroier e porta-avioes.
Caminho 2 (pirata): pilha de balas de uva; do tier 3 em diante, canhao grande, velas e chapeu de pirata.
Caminho 3 (comercio): bandeira alta e ninho do corvo; do tier 3 em diante, carga, bau e ouro.

O barco sai de _navio, que le os tres tiers (regra da torre): o caminho principal escolhe o tipo
e as cores, e os tiers cruzados somam enfeites pequenos (a base ja usa 8.868 triangulos).
"""
import math

import bmesh
from mathutils import Matrix, Vector

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
MASTRO_Y = 0.44


def _secao(y, w, fundo, borda):
    """Anel fechado da secao: casco em U por fora, borda grossa em cima e conves por dentro."""
    pts = []
    for g in (-90, -62, -34, 0, 34, 62, 90):
        a = math.radians(g)
        pts.append((w * math.sin(a), y, borda - (borda - fundo) * math.cos(a) ** 0.75))
    wi, d = w * BORDA, borda - CONVES
    pts += [(wi, y, borda), (wi * 0.97, y, d), (0, y, d), (-wi * 0.97, y, d), (-wi, y, borda)]
    return pts


def barco(m, madeira="marrom", faixa="vermelho", borda="marrom_escuro", conves="bege", vela="branco", vela_faixa="vermelho", bandeira="tinta",
          esporao="ouro", mastro=True, canhoes=True):
    """O barco do piloto, com as cores e as partes (mastro, canhoes de bordo) por parametro."""
    aneis = [_secao(*s) for s in CASCO]
    # por fora: faixa junto a borda e madeira no resto; borda escura; conves claro
    cores = [faixa, madeira, madeira, madeira, madeira, faixa, borda, borda, conves, conves, borda, borda]
    p = [m.obj("casco", poli.loft(aneis, cores), nivel=1)]
    y_p, _w_p, _f_p, b_p = CASCO[-1]
    ponta = poli.torno([(0, 0), (0.050, 0.035), (0.030, 0.120), (0, 0.300)], seg=10, cor=esporao)
    p.append(m.obj("esporao", poli.orientar(ponta, (0, -1, 0.32), (0, y_p + 0.06, b_p - 0.07)), nivel=0, vivo=True))
    leme = poli.caixa((0.035, 0.20, 0.36), borda, chanfro=0.012, seg=1)
    for v in leme.verts:  # leme mais largo embaixo
        if v.co.z < 0:
            v.co.y *= 1.5
    poli.mover(leme, (0, 0.90, 0.20))
    p.append(m.obj("leme", leme, nivel=0, vivo=True))
    if mastro:   # mastro com cesto, vela enfunada com faixa, e bandeira triangular
        my = MASTRO_Y
        p.append(m.obj("mastro", poli.membro([(0, my, 0.18), (0, my, 0.80), (0, my, 1.42)], [0.034, 0.028, 0.020], cor="marrom_escuro"), nivel=1))
        cesto = poli.torno([(0, 0), (0.060, 0.005), (0.075, 0.070), (0.062, 0.075), (0.050, 0.030), (0, 0.030)], seg=12, cor="marrom")
        p.append(m.obj("cesto", poli.orientar(cesto, (0, 0, 1), (0, my, 1.26)), nivel=0, vivo=True))
        p.append(m.obj("verga", poli.membro([(-0.36, my - 0.03, 1.12), (0.36, my - 0.03, 1.12)], [0.020, 0.020], cor="marrom_escuro"), nivel=1))
        for nome, tam, cor, dz in (("vela", (0.66, 0.026, 0.56), vela, 0.0), ("faixa", (0.664, 0.060, 0.12), vela_faixa, 0.02)):
            bm = poli.caixa(tam, cor, chanfro=0.008, seg=1)
            bmesh.ops.subdivide_edges(bm, edges=[e for e in bm.edges if abs(e.verts[0].co.x - e.verts[1].co.x) > 0.3], cuts=5)
            for v in bm.verts:  # barriga da vela para a frente, mais cheia embaixo
                v.co.y -= 0.11 * (1 - (v.co.x / 0.34) ** 2) * (0.75 - 0.5 * (v.co.z + dz) / 0.56)
            poli.mover(bm, (0, my - 0.045, 0.83 + dz))
            p.append(m.obj(nome, bm, nivel=0))
        band = poli.caixa((0.30, 0.012, 0.16), bandeira, chanfro=0.004, seg=1)
        for v in band.verts:
            if v.co.x > 0:
                v.co.z *= 0.05
        poli.mover(band, (0.165, my, 1.36))
        p.append(m.obj("bandeira", band, nivel=0, vivo=True))
    if canhoes:   # dois canhoes por bordo, apoiados na borda
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


# ---------------------------------------------------------------- pecas dos upgrades
def _canhao_grande(m, nome, pos, direcao, escala, cor="cinza_escuro", anel="ouro"):
    perfil = [(0, 0), (0.070, 0.012), (0.090, 0.100), (0.078, 0.330), (0.100, 0.340), (0.100, 0.400), (0.060, 0.402), (0.054, 0.300), (0, 0.290)]
    perfil = [(r * escala, h * escala) for r, h in perfil]
    return pecas.cilindro(m, nome, pos, direcao, perfil, cores=[cor, cor, cor, anel, anel, "tinta", "tinta", "tinta"], seg=12)


def _aviao(m, nome, pos, cor, asa):
    """Aviao pequeno pousado no conves, apontado para a proa."""
    mat = Matrix.Translation(Vector(pos))
    return [pecas.tubo(m, nome, [(0, 0.150, 0), (0, -0.060, 0.010), (0, -0.170, 0)], [0.020, 0.044, 0.026], cor, matriz=mat),
            pecas.bloco(m, nome + "_asa", (0.340, 0.085, 0.016), (0, -0.040, 0.020), asa, chanfro=0.006, matriz=mat),
            pecas.bloco(m, nome + "_cauda", (0.130, 0.050, 0.014), (0, 0.140, 0.012), asa, chanfro=0.005, matriz=mat),
            pecas.bloco(m, nome + "_leme", (0.014, 0.060, 0.070), (0, 0.140, 0.045), asa, chanfro=0.005, matriz=mat)]


def _navio(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    topo = Vector((0, MASTRO_Y, 1.42))   # onde entram a flamula e a bandeira alta
    torreta = []
    if p == 0 and t1 >= 3:   # destroier e porta-avioes: casco de aco, sem vela
        casco, fx = ("cinza", "azul") if t1 < 5 else ("azul", "ouro")
        b = barco(m, madeira=casco, faixa=fx, borda="cinza_escuro", conves="cinza", esporao="aco" if t1 < 5 else "ouro", mastro=False, canhoes=False)
        if t1 == 3:
            b.append(pecas.cilindro(m, "chamine", (0, 0.440, 0.220), (0, 0.10, 1), [(0.110, 0), (0.095, 0.380), (0.105, 0.390), (0.105, 0.440), (0.070, 0.440), (0.065, 0.300), (0, 0.300)],
                                    cores=["cinza_escuro", "vermelho", "vermelho", "tinta", "tinta", "tinta"], seg=10))
            b.append(pecas.tubo(m, "mastro_radar", [(0, 0.640, 0.250), (0, 0.640, 0.950)], 0.018, "cinza_escuro", nivel=0))
            topo = Vector((0, 0.640, 0.950))
            for k, (y, d) in enumerate(((-0.640, -1), (0.740, 1))):
                torreta.append(pecas.esfera(m, f"torre_{k}", (0, y, 0.330), (0.105, 0.115, 0.075), "cinza_escuro", cortes=0))
                for sx in (-1, 1):
                    torreta.append(pecas.tubo(m, f"torre_cano_{k}_{sx}", [(0.040 * sx, y, 0.345), (0.040 * sx, y + 0.240 * d, 0.380)], 0.020, "tinta", nivel=0))
        else:
            cor_pista = "cinza_escuro" if t1 == 4 else "tinta"
            b.append(pecas.bloco(m, "pista", (0.860, 1.640, 0.050), (0, -0.060, 0.400), cor_pista, chanfro=0.020))
            b.append(pecas.bloco(m, "pista_linha", (0.050, 1.500, 0.054), (-0.120, -0.060, 0.400), "amarelo" if t1 == 4 else "ouro", chanfro=0.006))
            alto = 0.300 if t1 == 4 else 0.420
            b.append(pecas.bloco(m, "ilha", (0.170, 0.340, alto), (0.330, 0.330, 0.425 + alto / 2), "cinza" if t1 == 4 else "ouro", chanfro=0.024))
            b.append(pecas.tubo(m, "mastro_radar", [(0.330, 0.330, 0.425 + alto), (0.330, 0.330, 0.800 + alto)], 0.016, "cinza_escuro", nivel=0))
            topo = Vector((0.330, 0.330, 0.800 + alto))
            b += _aviao(m, "aviao_0", (-0.150, 0.480, 0.460), "vermelho", "branco")
            if t1 == 5:
                b += _aviao(m, "aviao_1", (-0.200, -0.560, 0.460), "vermelho", "branco")
            m.matriz = Matrix.Translation((0, 0, 0.150)) @ NO_CONVES   # o macaco sobe para a pista
    elif p == 1 and t2 >= 3:   # navio pirata
        cores = {3: dict(vela="vermelho", vela_faixa="tinta"), 4: dict(vela="tinta", vela_faixa="amarelo", bandeira="vermelho"),
                 5: dict(madeira="tinta", faixa="ouro", vela="vermelho", vela_faixa="ouro", bandeira="ouro")}[t2]
        b = barco(m, **cores)
        esc = {3: 1.0, 4: 1.12, 5: 1.25}[t2]
        torreta.append(_canhao_grande(m, "canhao_proa", (0, -0.560, 0.330), (0, -1, 0.18), esc, anel="ouro" if t2 != 4 else "vermelho"))
        if t2 >= 4:   # arpao na proa
            b.append(pecas.cone(m, "arpao", (0.140, -0.800, 0.400), (0.15, -1, 0.10), 0.030, 0.420, "aco", seg=5, fechado=True))
        if t2 == 5:
            torreta.append(_canhao_grande(m, "canhao_popa", (0, 0.700, 0.420), (0, 1, 0.25), 0.90))
    elif p == 2 and t3 >= 3:   # navio mercante
        cores = {3: dict(vela="verde", vela_faixa="ouro", bandeira="verde"), 4: dict(madeira="branco", faixa="azul", vela="verde", vela_faixa="ouro", bandeira="ouro"),
                 5: dict(madeira="branco", faixa="ouro", borda="ouro", vela="azul", vela_faixa="ouro", bandeira="ouro")}[t3]
        b = barco(m, **cores)
        for k, (x, y, s) in enumerate(((-0.150, 0.640, 0.150), (0.120, 0.660, 0.130), (0.0, 0.200, 0.140))[:2 if t3 == 3 else 3]):
            b.append(pecas.bloco(m, f"carga_{k}", (s, s, s), (x, y, 0.270 + s / 2), "marrom" if k != 1 else "laranja", chanfro=0.014))
        if t3 >= 4:   # bau de ouro
            b.append(pecas.bloco(m, "bau", (0.200, 0.150, 0.120), (0.0, -0.640, 0.330), "marrom_escuro", chanfro=0.016))
            b.append(pecas.esfera(m, "bau_ouro", (0.0, -0.640, 0.395), (0.085, 0.060, 0.045), "ouro", cortes=0))
        if t3 == 5:   # ouro empilhado na popa
            for k, (x, y) in enumerate(((-0.200, 0.280), (0.210, 0.250), (0.0, 0.760))):
                b.append(pecas.cone(m, f"ouro_{k}", (x, y, 0.250), (0, 0, 1), 0.100, 0.170, "ouro", seg=6, fechado=True))
    else:
        b = barco(m)
    # enfeites dos tiers cruzados e dos tiers 1 e 2
    if t1 >= 1:   # Disparo Rapido: flamula amarela
        b.append(pecas.cone(m, "flamula", topo + Vector((0, 0, -0.230)), (-1, 0.1, 0), 0.050, 0.340, "amarelo", seg=4, fechado=True))
    if t1 == 2:   # Tiro Duplo: canhao de proa
        b.append(_canhao_grande(m, "canhao_duplo", (0, -0.620, 0.345), (0, -1, 0.15), 0.62))
    if t2 >= 1 and not (p == 1 and t2 >= 3):   # Tiro de Uva (e Quente): pilha de balas na popa
        cor = "roxo" if t2 == 1 else "laranja"
        for k, (x, y, z) in enumerate(((-0.080, 0.700, 0.330), (0.080, 0.700, 0.330), (0, 0.620, 0.330), (0, 0.670, 0.420))):
            b.append(pecas.esfera(m, f"uva_{k}", (x, y, z + (0.150 if (p == 0 and t1 >= 4) else 0)), 0.062, cor, cortes=0))
    if t3 >= 1:   # Longo Alcance: bandeira azul comprida no alto
        b.append(pecas.cone(m, "bandeira_alta", topo + Vector((0, 0, 0.020)), (1, 0.1, 0.05), 0.060, 0.460, "azul", seg=4, fechado=True))
        b.append(pecas.tubo(m, "bandeira_alta_haste", [topo, topo + Vector((0, 0, 0.130))], 0.014, "marrom_escuro", nivel=0))
    if t3 == 2:   # Ninho do Corvo: cesto grande com luneta
        b.append(pecas.cilindro(m, "ninho", topo + Vector((0, 0, -0.200)), (0, 0, 1), [(0, 0), (0.110, 0.006), (0.135, 0.110), (0.115, 0.112), (0.095, 0.040), (0, 0.040)], cor="marrom", seg=10))
        b.append(pecas.cilindro(m, "ninho_luneta", topo + Vector((0.060, -0.080, -0.080)), (0.2, -1, 0.1), [(0.028, 0), (0.036, 0.150), (0, 0.150)], cores=["ouro", "ciano"], seg=8))
    m.por("base", b)
    m.pivo("base", (0, 0, 0))
    if torreta:
        m.por("torreta", torreta)
        m.pivo("torreta", (0, -0.560, 0.330))


def base(m):
    m.macaco(pelagem="tufos")
    m.por("chapeu", lenco_pirata(m))
    m.por("rosto", tapa_olho(m))
    m.por("mao_ataque", luneta(m))
    m.matriz = NO_CONVES
    _navio(m)


def marinha(m):
    """Quepe de marinheiro no lugar do lenco (tiers 3 a 5 do caminho 1)."""
    t = m.tier[0]
    m.por("chapeu", pecas.capacete(m, "branco", "azul" if t < 5 else "ouro", topo="ouro" if t == 5 else None))
    _navio(m)


def pirata(m):
    t = m.tier[1]
    if t >= 4:
        m.tirar("pelagem")
        m.por("chapeu", pecas.chapeu_aba(m, "tinta", fita="vermelho" if t == 4 else "ouro", aba=0.36, tomba=0.30)
              + [pecas.cone(m, "chapeu_pena", (0.200, 0.060, 0.960), (0.5, 0.4, 1), 0.045, 0.260, "vermelho" if t == 4 else "ouro", seg=4, fechado=True)])
    _navio(m)


def mercante(m):
    t = m.tier[2]
    if t == 5:
        m.por("chapeu", pecas.coroa(m, "ouro", "vermelho"))
    elif t == 4:
        m.por("chapeu", lenco_pirata(m, "verde", "ouro"))
    _navio(m)


PARAMETRO = [(None, None)]   # o tier muda o barco, que le m.tier
CAMINHOS = [
    {1: PARAMETRO, 2: PARAMETRO, 3: marinha, 4: marinha, 5: marinha},
    {1: PARAMETRO, 2: PARAMETRO, 3: pirata, 4: pirata, 5: pirata},
    {1: PARAMETRO, 2: PARAMETRO, 3: mercante, 4: mercante, 5: mercante},
]
