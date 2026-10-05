"""Os 18 herois, montados com as mesmas pecas das torres (pecas.py) sobre o macaco padrao.

Cada heroi e uma funcao que recebe a montagem (m) e veste o macaco: nao ha caminhos de upgrade,
entao so existe a variacao 0-0-0. O desenho segue os herois 2D do proprio jogo (sprites.cpp):
mesma cor de roupa, mesmo chapeu e mesma arma, sem copiar trajes de outro jogo.

Regras que valem aqui tambem: o corpo e o de todos os macacos (so Pat Fusty e maior), heroi de
capuz usa a cabeca sem orelhas (pecas.capuz ja troca) e a cor e chapada por peca.
"""
import math

import bpy
from mathutils import Matrix, Vector

import pecas

MX, MY, MZ = pecas.MAO
EX, EY, EZ = pecas.MAO_ESQ
ENQUADRE = {"churchill": ((0, 0.0, 0.45), 2.2), "pat": ((0, 0.02, 0.62), 2.2)}
ENQUADRE_PADRAO = ((0, 0.02, 0.50), 1.9)


# ---------------------------------------------------------------- pecas so dos herois
def _espada(m, nome, pos, direcao=(0.10, -0.55, 1), comp=0.50, lamina="aco", guarda="ouro", cabo="marrom_escuro"):
    """Espada em pe na mao: cabo, guarda e lamina de quatro faces."""
    mat = pecas.em(pos, direcao, de=(0, 0, 1))
    return [pecas.tubo(m, nome + "_cabo", [(0, 0, -0.090), (0, 0, 0.030)], 0.022, cabo, nivel=0, matriz=mat),
            pecas.bloco(m, nome + "_guarda", (0.170, 0.036, 0.030), (0, 0, 0.040), guarda, chanfro=0.008, matriz=mat),
            pecas.cilindro(m, nome + "_lamina", (0, 0, 0.050), (0, 0, 1), [(0, 0), (0.036, 0.004), (0.030, comp * 0.82), (0, comp)],
                           cor=lamina, seg=4, matriz=mat)]


def _arco(m):
    """Arco deitado na frente da mao, com a corda puxada e a flecha armada (aparece de cima)."""
    x, y, z = MX - 0.040, MY - 0.200, MZ + 0.040
    pts = [(x - 0.300, y + 0.130, z - 0.050), (x - 0.170, y + 0.020, z - 0.025), (x, y - 0.030, z), (x + 0.170, y + 0.020, z + 0.025),
           (x + 0.300, y + 0.130, z + 0.050)]
    objs = [pecas.tubo(m, "arco", pts, [0.012, 0.024, 0.030, 0.024, 0.012], "marrom_escuro")]
    objs.append(pecas.tubo(m, "arco_corda", [pts[0], (x, y + 0.150, z), pts[-1]], 0.007, "bege", nivel=0))
    objs += pecas.dardo(m, "flecha", (x, y + 0.020, z + 0.012), frente=0.260, tras=0.130, penas=("vermelho", "amarelo"))
    return objs


def _fones(m, cor, nome="fones"):
    """Fones de ouvido: arco por cima da cabeca e uma concha em cada orelha."""
    objs = [pecas.tubo(m, nome, [(-0.300, 0, 0.770), (-0.270, 0, 0.930), (0, 0, 1.020), (0.270, 0, 0.930), (0.300, 0, 0.770)], 0.024, "tinta")]
    for sx in (-1, 1):
        objs.append(pecas.esfera(m, f"{nome}_concha_{sx}", (0.318 * sx, 0.0, 0.765), (0.052, 0.090, 0.100), cor))
    return objs


def _boina(m, cor, detalhe):
    """Boina: a calota de cima do cranio, com a barra e o botao."""
    piso = lambda x, y: 0.165 - 0.10 * (y + 0.20)
    objs = [pecas.casca(m, "boina", cor, piso, raios=(0.318, 0.286, 0.278))]
    objs.append(pecas.barra_casca(m, "boina_barra", "tinta", piso, raios=(0.318, 0.286, 0.278)))
    objs.append(pecas.esfera(m, "boina_botao", (0.150, -0.150, 0.945), 0.034, detalhe, cortes=0))
    return objs


def _folhas(m, cor, n=8):
    """Coroa de folhas em volta da cabeca."""
    objs = []
    for k in range(n):
        a = 2 * math.pi * k / n
        objs.append(pecas.cone(m, f"folha_{k}", (0.225 * math.cos(a), 0.205 * math.sin(a), 0.880), (0.55 * math.cos(a), 0.55 * math.sin(a), 1),
                               0.056, 0.150, cor, seg=4, fechado=True))
    return objs


def _manga_dir(m, cor, **kw):
    m.somar("extra", pecas.manga(m, 1, cor, **kw), grupo="braco", preso=True)


def _orbes(m, nome, cor, n, raio=0.52, z=0.80, tam=0.060, rabo=None):
    """Orbes flutuando em volta do heroi; rabo = cor de uma cauda de espirito."""
    objs = []
    for k in range(n):
        a = 2 * math.pi * (k + 0.5) / n
        p = Vector((raio * math.cos(a), raio * math.sin(a) * 0.85, z + 0.10 * math.sin(a * 2)))
        objs.append(pecas.esfera(m, f"{nome}_{k}", p, tam, cor, cortes=0))
        if rabo:
            objs.append(pecas.cone(m, f"{nome}_rabo_{k}", p + Vector((0, 0, -tam * 0.5)), (-math.sin(a) * 0.6, math.cos(a) * 0.6, -1), tam * 0.75, tam * 2.6,
                                   rabo, seg=5))
    return objs


# ---------------------------------------------------------------- os herois
def quincy(m):
    m.macaco(pelagem="lisa")
    m.por("chapeu", pecas.capuz(m, "verde_escuro", barra="ouro"))
    m.por("costas", pecas.aljava(m))
    m.por("tronco", pecas.cinto(m, "marrom_escuro") + pecas.bandoleira(m, "marrom"))
    m.por("mao_ataque", _arco(m))


def gwendolin(m):
    m.macaco(pelagem="crista", pelo_escuro="laranja")
    m.por("rosto", pecas.oculos(m, aro="amarelo", tira="marrom_escuro"))
    m.por("tronco", pecas.colete(m, "branco", barra="laranja", emblema="vermelho"))
    m.por("costas", pecas.tanque(m, "vermelho", tampa="cinza"))
    lanca = [pecas.cilindro(m, "lanca", (MX, MY + 0.120, MZ + 0.025), (0, -1, 0.05),
                            [(0, 0), (0.046, 0.004), (0.046, 0.220), (0.030, 0.240), (0.030, 0.360), (0.052, 0.380), (0.052, 0.430), (0, 0.430)],
                            cores=["cinza_escuro", "cinza_escuro", "cinza", "cinza", "vermelho", "vermelho", "tinta"], seg=10)]
    boca = (MX, MY - 0.300, MZ + 0.045)
    lanca.append(pecas.cone(m, "fogo", boca, (0, -1, 0.12), 0.085, 0.300, "laranja", seg=5, fechado=True))
    lanca.append(pecas.cone(m, "fogo_miolo", boca, (0, -1, 0.12), 0.050, 0.200, "amarelo", seg=5, fechado=True))
    m.por("mao_ataque", lanca)


def striker(m):
    m.macaco(pelagem="lisa")
    m.por("chapeu", _boina(m, "verde_escuro", "ouro"))
    m.por("tronco", pecas.colete(m, "verde_escuro", barra="tinta") + [pecas.estrela(m, "emblema", (0, -0.186, 0.410), 0.070, "ouro", normal=(0, -1, 0.1))]
          + pecas.bandoleira(m, "marrom_escuro"))
    bazuca = [pecas.cilindro(m, "bazuca", (MX - 0.040, MY + 0.360, MZ + 0.200), (0, -1, 0.04),
                             [(0, 0), (0.078, 0.004), (0.078, 0.060), (0.058, 0.070), (0.058, 0.560), (0.082, 0.580), (0.082, 0.680), (0.052, 0.680), (0, 0.610)],
                             cores=["verde_escuro", "verde_escuro", "cinza_escuro", "verde_escuro", "cinza_escuro", "cinza_escuro", "tinta", "tinta"], seg=10)]
    bazuca.append(pecas.bloco(m, "bazuca_punho", (0.040, 0.050, 0.130), (MX - 0.040, MY + 0.020, MZ + 0.110), "tinta", chanfro=0.008))
    m.por("mao_ataque", bazuca)


def obyn(m):
    m.macaco(pelagem="barba", pelo_escuro="verde_escuro")
    m.por("chapeu", pecas.chifres(m, "bege", comp=0.260, abre=1.15) + _folhas(m, "verde"))
    m.por("costas", pecas.capa(m, "verde_escuro", borda="verde", comp=0.46))
    m.por("tronco", pecas.cinto(m, "marrom", fivela="verde"))
    cajado, _topo = pecas.cajado(m, cor="marrom", orbe="verde", raio=0.075, garra="verde_escuro")
    m.por("mao_ataque", cajado)


def churchill(m):
    """Tanque: casco e esteiras na base; a torre gira com o canhao e com a cabeca do capitao na escotilha."""
    m.macaco(pelagem="lisa")
    for g in ("corpo", "cauda", "braco"):
        bpy.data.objects.remove(m.fixas.pop(g), do_unlink=True)
    casco = [pecas.bloco(m, "casco", (0.820, 1.060, 0.250), (0, 0.020, 0.260), "verde_escuro", chanfro=0.060, seg=2)]
    for sx in (-1, 1):
        casco.append(pecas.bloco(m, f"esteira_{sx}", (0.220, 1.180, 0.290), (0.470 * sx, 0.020, 0.160), "cinza_escuro", chanfro=0.090, seg=2))
        for k in range(4):
            casco.append(pecas.esfera(m, f"roda_{sx}_{k}", (0.585 * sx, -0.420 + 0.290 * k, 0.150), (0.030, 0.110, 0.110), "cinza", cortes=0))
    casco.append(pecas.estrela(m, "casco_estrela", (0, -0.300, 0.392), 0.110, "branco", normal=(0, -0.05, 1)))
    m.por("base", casco)
    pivo = Vector((0, 0.100, 0.380))
    torre = [pecas.cilindro(m, "torre", pivo, (0, 0, 1), [(0, 0), (0.340, 0.004), (0.360, 0.070), (0.310, 0.210), (0.200, 0.250), (0, 0.250)],
                            cores=["verde_escuro", "verde", "verde", "verde_escuro", "cinza_escuro"], seg=12)]
    torre.append(pecas.cilindro(m, "canhao", pivo + Vector((0, -0.300, 0.130)), (0, -1, 0.03),
                                [(0, 0), (0.075, 0.004), (0.062, 0.520), (0.090, 0.530), (0.090, 0.640), (0.055, 0.640), (0, 0.580)],
                                cores=["cinza_escuro", "cinza_escuro", "cinza", "cinza", "tinta", "tinta"], seg=10))
    # a cabeca do capitao sai da escotilha e gira junto com a torre
    sobe = Matrix.Translation((0, 0.130, 0.120)) @ Matrix.Scale(0.80, 4)
    cabeca = m.fixas.pop("cabeca")
    for o in [cabeca] + pecas.capacete(m, "verde_escuro", "tinta", topo="ouro"):
        o.data.transform(sobe)
        torre.append(o)
    m.por("torreta", torre)
    m.pivo("torreta", pivo)


def benjamin(m):
    m.macaco(pelagem="topete")
    m.por("chapeu", _fones(m, "azul"))
    m.por("rosto", pecas.oculos(m, aro="ciano", tira="tinta"))
    m.por("tronco", pecas.colete(m, "cinza_escuro", barra="azul", emblema="ciano"))
    laptop = [pecas.bloco(m, "laptop", (0.330, 0.210, 0.024), (0, -0.300, 0.330), "cinza", chanfro=0.008),
              pecas.bloco(m, "laptop_tela", (0.330, 0.024, 0.220), (0, -0.410, 0.440), "cinza_escuro", chanfro=0.008),
              pecas.esfera(m, "laptop_logo", (0, -0.426, 0.450), (0.050, 0.012, 0.050), "ciano", cortes=0)]
    m.por("mao_livre", laptop)


def ezili(m):
    m.macaco(pelagem="lisa", pelo_escuro="roxo")
    cranio = [pecas.esfera(m, "cranio", (0, -0.060, 1.010), (0.125, 0.115, 0.100), "branco")]
    for sx in (-1, 1):
        cranio.append(pecas.esfera(m, f"cranio_olho_{sx}", (0.048 * sx, -0.160, 1.015), (0.030, 0.020, 0.034), "tinta", cortes=0))
    m.por("chapeu", pecas.faixa_testa(m, "tinta", pontas=False) + cranio)
    m.por("costas", pecas.capa(m, "roxo", borda="verde", comp=0.50))
    m.por("tronco", pecas.saia(m, "roxo", barra="tinta"))
    cajado, _topo = pecas.cajado(m, cor="tinta", orbe="verde", raio=0.070, garra="branco")
    m.por("mao_ataque", cajado)


def pat(m):
    m.macaco(pelagem="tufos")
    m.matriz = Matrix.Scale(1.32, 4)
    m.por("chapeu", pecas.faixa_testa(m, "vermelho"))
    m.por("tronco", pecas.cinto(m, "marrom_escuro") + pecas.manga(m, -1, "pelo", anel="ouro", ombreira="marrom_escuro"))
    _manga_dir(m, "pelo", anel="ouro", ombreira="marrom_escuro")
    m.por("mao_livre", pecas.esfera(m, "punho_e", pecas.MAO_ESQ, 0.092, "pele"))
    m.por("mao_ataque", pecas.esfera(m, "punho_d", pecas.MAO, 0.092, "pele"))


def adora(m):
    m.macaco(pelagem="lisa", pelo_escuro="ouro")
    sol = pecas.coroa(m, cor="ouro", joia="vermelho")
    for k in range(9):   # raios do sol atras da cabeca
        a = math.pi * k / 8
        sol.append(pecas.cone(m, f"raio_{k}", (0.300 * math.cos(a), 0.130, 0.800 + 0.300 * math.sin(a)), (math.cos(a), 0.15, math.sin(a)), 0.052, 0.190,
                              "amarelo", seg=4, fechado=True))
    m.por("chapeu", sol)
    m.por("costas", pecas.capa(m, "amarelo", borda="ouro", comp=0.48))
    m.por("tronco", pecas.colete(m, "branco", barra="ouro", emblema="amarelo") + pecas.saia(m, "branco", barra="ouro"))
    cajado, _topo = pecas.cajado(m, cor="ouro", orbe="amarelo", raio=0.085, garra="ouro")
    m.por("mao_ataque", cajado)


def brickell(m):
    m.macaco(pelagem="lisa")
    m.por("chapeu", pecas.chapeu_aba(m, "marinho", fita="ouro", aba=0.290, copa=0.130, tomba=0.05)
          + [pecas.estrela(m, "quepe_estrela", (0, -0.240, 0.985), 0.050, "ouro", normal=(0, -1, 0.25))])
    m.por("tronco", pecas.colete(m, "marinho", barra="ouro", emblema="ouro") + pecas.manga(m, -1, "marinho", luva="branco", ombreira="ouro"))
    _manga_dir(m, "marinho", luva="branco", ombreira="ouro")
    m.por("mao_ataque", _espada(m, "sabre", (MX, MY - 0.020, MZ + 0.040), lamina="aco", guarda="ouro", cabo="marinho"))


def etienne(m):
    m.macaco(pelagem="topete")
    m.por("chapeu", _fones(m, "vermelho"))
    m.por("tronco", pecas.colete(m, "branco", barra="azul", emblema="vermelho") + pecas.lenco(m, "azul"))
    controle = [pecas.bloco(m, "controle", (0.150, 0.080, 0.050), (MX - 0.010, MY - 0.040, MZ + 0.040), "cinza_escuro", chanfro=0.012),
                pecas.tubo(m, "controle_antena", [(MX + 0.040, MY - 0.040, MZ + 0.060), (MX + 0.060, MY - 0.040, MZ + 0.230)], 0.008, "aco", nivel=0),
                pecas.esfera(m, "controle_luz", (MX + 0.060, MY - 0.040, MZ + 0.240), 0.022, "vermelho", cortes=0)]
    m.por("mao_ataque", controle)
    c = Vector((-0.440, -0.060, 1.060))
    drone = [pecas.esfera(m, "drone", c, (0.100, 0.100, 0.055), "cinza_escuro"), pecas.esfera(m, "drone_olho", c + Vector((0, -0.085, -0.010)), 0.030, "vermelho", cortes=0)]
    for k in range(4):
        a = math.pi / 4 + math.pi / 2 * k
        p = c + Vector((0.170 * math.cos(a), 0.170 * math.sin(a), 0.020))
        drone.append(pecas.tubo(m, f"drone_braco_{k}", [c, p], 0.014, "cinza", nivel=0))
        drone.append(pecas.cilindro(m, f"drone_helice_{k}", p, (0, 0, 1), [(0, 0), (0.085, 0.002), (0.085, 0.012), (0, 0.014)], cor="aco", seg=10))
    m.somar("extra", drone)


def sauda(m):
    m.macaco(pelagem="lisa")
    m.por("chapeu", pecas.faixa_testa(m, "tinta"))
    m.por("tronco", pecas.cinto(m, "vermelho", fivela="ouro") + pecas.manga(m, -1, "pelo", ombreira="aco"))
    _manga_dir(m, "pelo", ombreira="aco")
    m.por("mao_ataque", _espada(m, "espada_d", (MX, MY - 0.020, MZ + 0.040), direcao=(0.25, -0.60, 1), guarda="vermelho"))
    m.por("mao_livre", _espada(m, "espada_e", (EX, EY - 0.020, EZ + 0.060), direcao=(-0.35, -0.60, 1), guarda="vermelho"))


def psi(m):
    m.macaco(pelagem="lisa", pelo="lilas", pele="lilas_claro", pelo_escuro="lilas_escuro")
    antena = [pecas.tubo(m, "antena", [(0, 0.020, 0.980), (0, 0.040, 1.120)], 0.012, "lilas_escuro", nivel=0),
              pecas.esfera(m, "antena_ponta", (0, 0.040, 1.140), 0.040, "rosa", cortes=0),
              pecas.esfera(m, "gema", (0, -0.250, 0.905), (0.040, 0.024, 0.040), "ciano", cortes=0)]
    m.por("chapeu", antena)
    m.por("tronco", pecas.saia(m, "roxo", barra="rosa"))
    m.somar("extra", _orbes(m, "orbe", "rosa", 4) + pecas.aro_chao(m, "aura", 0.600, "rosa"))


def geraldo(m):
    m.macaco(pelagem="barba")
    m.por("chapeu", pecas.chapeu_aba(m, "marrom", fita="ouro", aba=0.330, tomba=0.38))
    m.por("tronco", pecas.colete(m, "verde_escuro", barra="marrom_escuro") + pecas.bandoleira(m, "marrom_escuro"))
    mochila = pecas.mochila(m, "marrom", detalhe="marrom_escuro", tam=1.35)
    mochila.append(pecas.tubo(m, "tapete", [(-0.230, 0.250, 0.640), (0.230, 0.250, 0.640)], 0.062, "vermelho", nivel=0))
    mochila.append(pecas.tubo(m, "luneta", [(0.130, 0.300, 0.400), (0.200, 0.340, 0.760)], 0.022, "ouro", nivel=0))
    m.por("costas", mochila)
    m.por("mao_ataque", pecas.frasco(m, "pocao", (MX, MY - 0.030, MZ + 0.020), "rosa", escala=1.15))


def corvus(m):
    m.macaco(pelagem="lisa")
    m.por("chapeu", pecas.capuz(m, "tinta", barra="ciano"))
    m.por("costas", pecas.capa(m, "tinta", borda="ciano", comp=0.52))
    m.por("tronco", pecas.cinto(m, "cinza_escuro", fivela="ciano"))
    x, y = MX + 0.010, MY - 0.030
    lanca = [pecas.tubo(m, "lanca", [(x, y, 0.020), (x, y, 0.980)], 0.020, "cinza_escuro", nivel=0),
             pecas.toro(m, "lanca_anel", (x, y, 0.960), 0.040, 0.014, "aco", seg=8, lados=4),
             pecas.cone(m, "lanca_ponta", (x, y, 0.970), (0, 0, 1), 0.058, 0.270, "ciano", seg=4)]
    m.por("mao_ataque", lanca)
    m.somar("extra", _orbes(m, "espirito", "ciano", 3, raio=0.50, z=0.86, tam=0.065, rabo="azul"))


def rosalia(m):
    m.macaco(pelagem="topete", pelo_escuro="rosa")
    m.por("rosto", pecas.oculos(m, aro="ouro", tira="marrom_escuro"))
    m.por("tronco", pecas.cachecol(m, "rosa") + pecas.cinto(m, "marrom_escuro"))
    m.por("costas", pecas.mochila(m, "aco", detalhe="cinza_escuro", luz="laranja", tam=1.15))
    m.por("pes", pecas.tenis(m, "rosa"))
    pistola = [pecas.bloco(m, "pistola", (0.046, 0.200, 0.056), (MX, MY - 0.120, MZ + 0.050), "cinza_escuro", chanfro=0.010),
               pecas.bloco(m, "pistola_cabo", (0.042, 0.056, 0.100), (MX, MY - 0.030, MZ - 0.010), "marrom_escuro", chanfro=0.008),
               pecas.esfera(m, "pistola_boca", (MX, MY - 0.225, MZ + 0.050), 0.030, "laranja", cortes=0)]
    m.por("mao_ataque", pistola)


def dan(m):
    m.macaco(pelagem="barba")
    m.por("chapeu", pecas.chapeu_aba(m, "tinta", fita="vermelho", aba=0.340, tomba=0.38)
          + [pecas.cone(m, "pena", (0.190, 0.100, 0.930), (0.5, 0.6, 1), 0.040, 0.260, "vermelho", seg=4, fechado=True)])
    m.por("costas", pecas.capa(m, "vermelho", borda="ouro", comp=0.50))
    m.por("tronco", pecas.colete(m, "tinta", barra="ouro", emblema="vermelho"))
    m.por("mao_ataque", _espada(m, "florete", (MX, MY - 0.020, MZ + 0.040), comp=0.58, guarda="ouro", cabo="vermelho"))


def silas(m):
    m.macaco(pelagem="topete", pelo="pelo_gelo", pele="pele_gelo", pelo_escuro="pelo_gelo_escuro")
    m.por("chapeu", pecas.chapeu_cone(m, "branco", fita="ciano"))
    m.por("costas", pecas.capa(m, "azul", borda="branco", comp=0.48))
    m.por("tronco", pecas.cinto(m, "azul", fivela="ciano"))
    cajado, topo = pecas.cajado(m, cor="aco", orbe="ciano", raio=0.070, garra="branco")
    for k in range(3):   # cristais de gelo em volta do orbe
        a = 2 * math.pi * k / 3
        cajado.append(pecas.cone(m, f"cristal_{k}", topo + Vector((0.050 * math.cos(a), 0.050 * math.sin(a), 0.030)), (math.cos(a), math.sin(a), 0.9), 0.036, 0.150,
                                 "branco", seg=4))
    m.por("mao_ataque", cajado)


HEROIS = {
    "quincy": quincy, "gwendolin": gwendolin, "striker": striker, "obyn": obyn, "churchill": churchill, "benjamin": benjamin,
    "ezili": ezili, "pat": pat, "adora": adora, "brickell": brickell, "etienne": etienne, "sauda": sauda,
    "psi": psi, "geraldo": geraldo, "corvus": corvus, "rosalia": rosalia, "dan": dan, "silas": silas,
}
