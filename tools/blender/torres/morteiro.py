"""Macaco Morteiro: o macaco padrao de pelo liso e capacete cinza, com um projetil na mao direita e a esquerda apoiada no tubo do morteiro.

Caminho 1 (explosao): tubo cada vez mais grosso; do tier 3 em diante, bipe, carreta e o morteiro gigante.
Caminho 2 (cadencia): pilha de projeteis; do tier 3 em diante, projeteis de aco e bateria de tubos.
Caminho 3 (fogo): mira e chama na boca; do tier 3 em diante, sinalizador e morteiro incendiario.

O morteiro sai de _morteiro, que le os tres tiers (regra da torre) e vale em qualquer combinacao.
"""
from mathutils import Matrix, Vector

import pecas

ENQUADRE = ((0.05, -0.12, 0.45), 2.2)
MX, MY, MZ = pecas.MAO
PE = Vector((-0.180, -0.100, 0.050))     # onde o tubo apoia no chao (o conjunto com o macaco fica centrado)
EIXO = Vector((0, -0.38, 1)).normalized()


def _projetil(m, nome, pos, direcao, raio, cor, ponta):
    perfil = [(0, 0), (raio * 0.55, 0.010), (raio * 0.55, raio * 0.9), (raio, raio * 1.2), (raio, raio * 2.6), (raio * 0.6, raio * 3.5), (0, raio * 4.1)]
    return pecas.cilindro(m, nome, pos, direcao, perfil, cores=["cinza", "cinza", cor, cor, ponta, ponta], seg=8)


def _tubo(m, nome, pe, raio, comp, cor, anel, boca="tinta"):
    r, L = raio, comp
    perfil = [(0, 0), (r * 1.25, 0.020), (r * 1.05, 0.070), (r, 0.090), (r, L * 0.45), (r * 1.14, L * 0.46), (r * 1.14, L * 0.54), (r, L * 0.55), (r, L - 0.060),
              (r * 1.22, L - 0.050), (r * 1.22, L), (r * 0.82, L), (r * 0.78, L * 0.5), (0, L * 0.5)]
    cores = [cor, cor, cor, cor, anel, anel, anel, cor, anel, anel, boca, boca, boca]
    return pecas.cilindro(m, nome, pe, EIXO, perfil, cores=cores, seg=12)


def _morteiro(m):
    t1, t2, t3 = m.tier
    p = max(range(3), key=lambda i: (m.tier[i], -i))
    raio = 0.085 * {0: 1.0, 1: 1.18, 2: 1.30, 3: 1.48, 4: 1.80, 5: 2.15}[t1 if p == 0 or t1 <= 2 else 0]
    comp = 0.460 * {0: 1.0, 1: 1.0, 2: 1.05, 3: 1.12, 4: 1.25, 5: 1.40}[t1 if p == 0 or t1 <= 2 else 0]
    cor, anel = "cinza_escuro", "ouro"
    if t1 >= 2:
        anel = "vermelho"
    if p == 0 and t1 >= 3:
        cor, anel = {3: ("cinza_escuro", "amarelo"), 4: ("tinta", "ouro"), 5: ("vermelho", "ouro")}[t1]
    elif p == 1 and t2 >= 3:
        cor, anel = {3: ("cinza", "aco"), 4: ("verde_escuro", "amarelo"), 5: ("tinta", "vermelho")}[t2]
    elif p == 2 and t3 >= 3:
        cor, anel = {3: ("cinza_escuro", "vermelho"), 4: ("marrom_escuro", "laranja"), 5: ("vermelho", "amarelo")}[t3]
    n = {4: 3, 5: 5}.get(t2, 1) if p == 1 else 1
    desloc = [Vector(((k - (n - 1) / 2) * raio * 2.2, 0.050 * abs(k - (n - 1) / 2), 0)) for k in range(n)]
    grosso = [1.0] * n
    if n == 5:   # bateria de cinco: quatro tubos nos cantos e um maior no centro; o ultimo e o canto de tras, a direita
        d = raio * 2.1
        desloc = [Vector((0, 0, 0)), Vector((-d, -d, 0)), Vector((d, -d, 0)), Vector((-d, d, 0)), Vector((d, d, 0))]
        grosso = [1.55, 1.0, 1.0, 1.0, 1.0]

    largura = (raio * 2.2 * (n - 1) / 2 if n != 5 else raio * 2.1) + raio * 2.4
    # o macaco fica ao lado do ultimo tubo, com a mao esquerda apoiada nele
    ultimo = PE + desloc[-1]
    mao = pecas.MAO_ESQ
    m.matriz = Matrix.Translation((ultimo.x + raio * 0.75 - mao[0], ultimo.y - 0.067 + raio * 0.30 - mao[1], 0))
    b = [pecas.cilindro(m, "placa", PE - Vector((0, 0, 0.050)), (0, 0, 1), [(0, 0), (largura, 0.004), (largura, 0.040), (largura * 0.8, 0.056), (0, 0.056)], cor="cinza", seg=12,
                        matriz=Matrix.Translation(PE) @ Matrix.Diagonal((1, 0.75 if n == 3 else 1, 1, 1)) @ Matrix.Translation(-PE))]
    meio = PE + EIXO * comp * 0.60
    for sx in (-1, 1):   # bipe
        b.append(pecas.tubo(m, f"bipe_{sx}", [meio + Vector((sx * raio, 0, 0)), Vector((PE.x + sx * (largura + 0.060), meio.y - 0.200, 0.010))], 0.016, "cinza", nivel=0))
    if p == 0 and t1 >= 4:   # carreta com rodas para o morteiro pesado
        for sx in (-1, 1):
            b.append(pecas.cilindro(m, f"roda_{sx}", (PE.x + sx * (largura + 0.030), PE.y + 0.060, 0.130), (sx, 0, 0), [(0, -0.040), (0.050, -0.045), (0.130, -0.040), (0.130, 0.040), (0.050, 0.045), (0, 0.070)],
                                    cores=["ouro", "tinta", "tinta", "tinta", "ouro"], seg=12))
    # caminho 2: pilha de projeteis ao lado
    if t2 >= 1:
        aco = p == 1 and t2 >= 3
        pilha = [(-0.400, -0.180), (-0.490, -0.300), (-0.360, -0.330)] + ([(-0.500, -0.130), (-0.430, -0.440), (-0.560, -0.420)] if t2 >= 2 else [])
        for k, (x, y) in enumerate(pilha):
            b.append(_projetil(m, f"reserva_{k}", (PE.x + x - largura + 0.200, PE.y + y + 0.300, 0), (0, 0, 1), 0.050 if not aco else 0.060, "verde_escuro" if not aco else "aco", "vermelho" if not aco else "ouro"))
    m.por("base", b)
    m.pivo("base", (0, 0, 0))

    t = []
    for k, d in enumerate(desloc):
        t.append(_tubo(m, f"tubo_{k}", PE + d, raio * grosso[k], comp * (1.0 + 0.25 * (grosso[k] - 1)), cor, anel))
    boca = PE + EIXO * comp
    if t3 >= 1:   # Precisao Aumentada: mira ao lado do tubo
        t.append(pecas.cilindro(m, "mira", PE + EIXO * comp * 0.45 + Vector((-raio - 0.040 - largura + raio * 2.4, 0, 0)), EIXO, [(0, 0), (0.026, 0.004), (0.026, 0.150), (0.038, 0.156), (0.038, 0.210), (0, 0.200)],
                                cores=["cinza", "cinza", "cinza", "cinza", "ciano"], seg=8))
    if t3 >= 2:   # fogo na boca (maior nos tiers altos)
        h = 0.130 if not (p == 2 and t3 >= 4) else (0.200 if t3 == 4 else 0.280)
        for d in desloc:
            for k, (dx, dy, cor_f) in enumerate(((0, 0, "laranja"), (0.6, 0.2, "amarelo"), (-0.6, -0.2, "amarelo"), (0.1, -0.7, "vermelho"))[:2 if h < 0.15 else 4]):
                t.append(pecas.cone(m, f"chama_{len(t)}", boca + d + Vector((dx, dy, 0)) * raio * 0.5, EIXO + Vector((dx, dy, 0)) * 0.5, raio * 0.55, h * (1.0 if k == 0 else 0.7), cor_f, seg=5))
    if p == 2 and t3 >= 3:   # sinalizador: antena com luz
        t.append(pecas.tubo(m, "sinalizador", [PE + Vector((-largura - 0.030, 0.120, 0)), PE + Vector((-largura - 0.030, 0.120, comp * 0.95))], 0.012, "cinza", nivel=0))
        t.append(pecas.esfera(m, "sinalizador_luz", PE + Vector((-largura - 0.030, 0.120, comp * 0.95 + 0.040)), 0.050, "vermelho" if t3 < 5 else "amarelo", cortes=0))
    m.por("torreta", t)
    m.pivo("torreta", tuple(PE))

    # o projetil na mao do macaco acompanha o calibre e o fogo
    cor_p, ponta = ("laranja", "amarelo") if t3 >= 2 else (("aco", "ouro") if (p == 1 and t2 >= 3) else ("verde_escuro", "vermelho"))
    m.por("mao_ataque", _projetil(m, "projetil", (MX, MY - 0.02, MZ + 0.020), (0, -0.3, 1), min(0.050 * raio / 0.085, 0.095), cor_p, ponta))


def base(m):
    m.macaco(pelagem="lisa")
    m.por("chapeu", pecas.capacete(m, "cinza", "vermelho"))
    _morteiro(m)


def explosao(m):
    t = m.tier[0]
    m.por("tronco", pecas.colete(m, {3: "cinza_escuro", 4: "tinta", 5: "vermelho"}[t], barra="amarelo" if t == 3 else "ouro"))
    if t >= 4:
        m.por("chapeu", pecas.capacete(m, "tinta" if t == 4 else "ouro", "ouro" if t == 4 else "vermelho", topo="ouro" if t == 4 else "vermelho"))
    _morteiro(m)


def cadencia(m):
    t = m.tier[1]
    m.por("tronco", pecas.bandoleira(m, "marrom_escuro" if t == 3 else "ouro"))
    if t >= 4:
        m.por("chapeu", pecas.capacete(m, "verde_escuro" if t == 4 else "tinta", "amarelo" if t == 4 else "vermelho"))
        m.somar("tronco", pecas.colete(m, "verde_escuro" if t == 4 else "tinta", barra="amarelo" if t == 4 else "vermelho"))
    _morteiro(m)


def fogo(m):
    t = m.tier[2]
    m.por("rosto", pecas.oculos(m, aro="laranja", tira="tinta"))
    if t >= 4:
        m.por("chapeu", pecas.capacete(m, "laranja" if t == 4 else "vermelho", "tinta" if t == 4 else "amarelo", topo=None if t == 4 else "amarelo"))
        m.por("tronco", pecas.colete(m, "marrom_escuro" if t == 4 else "vermelho", barra="laranja" if t == 4 else "amarelo"))
    _morteiro(m)


PARAMETRO = [(None, None)]   # o tier muda o morteiro, que le m.tier
CAMINHOS = [
    {1: PARAMETRO, 2: PARAMETRO, 3: explosao, 4: explosao, 5: explosao},
    {1: PARAMETRO, 2: PARAMETRO, 3: cadencia, 4: cadencia, 5: cadencia},
    {1: PARAMETRO, 2: PARAMETRO, 3: fogo, 4: fogo, 5: fogo},
]
