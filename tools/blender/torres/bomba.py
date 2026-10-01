"""Canhao Bomba: maquina sem macaco. Carreta com rodas (base) e o cano com o pavio (torreta)."""
import poli

ENQUADRE = ((0, -0.02, 0.33), 1.15)


def carreta(m):
    base = []
    bm = poli.caixa((0.40, 0.54, 0.15), "marrom", chanfro=0.045, seg=3)
    poli.mover(bm, (0, 0.03, 0.175))
    base.append(m.obj("carreta", bm, nivel=0, vivo=True))
    berco = poli.gaiola((0.150, 0.200, 0.085), cortes=1, cor="marrom_escuro")
    poli.mover(berco, (0, 0.04, 0.265))
    base.append(m.obj("berco", berco, nivel=1))
    roda = [(0, -0.048), (0.070, -0.052), (0.085, -0.060), (0.150, -0.050), (0.168, -0.020), (0.168, 0.020),
            (0.150, 0.050), (0.085, 0.060), (0.070, 0.052), (0.045, 0.060), (0, 0.130)]
    cores_roda = ["ouro", "tinta", "tinta", "tinta", "tinta", "tinta", "tinta", "tinta", "ouro", "ouro"]
    for sx in (-1, 1):
        r = poli.torno(roda, seg=20, cores=cores_roda)
        base.append(m.obj(f"roda_{sx}", poli.orientar(r, (sx, 0, 0), (0.262 * sx, 0.03, 0.168)), nivel=0, vivo=True))
    return base


def cano(m):
    """Cano torneado com culatra redonda, aneis e a boca furada, mais o pavio aceso."""
    perfil = [(0, 0), (0.105, 0.012), (0.160, 0.060), (0.178, 0.130), (0.178, 0.215), (0.192, 0.225), (0.192, 0.265),
              (0.176, 0.275), (0.156, 0.500), (0.170, 0.510), (0.170, 0.560), (0.154, 0.570), (0.146, 0.690),
              (0.172, 0.705), (0.172, 0.770), (0.128, 0.775), (0.116, 0.640), (0, 0.620)]
    cores = ["cinza_escuro"] * 5 + ["ouro"] + ["cinza_escuro"] * 3 + ["azul"] + ["cinza_escuro"] * 2 + ["tinta"] * 5
    torreta = [m.obj("cano", poli.orientar(poli.torno(perfil, seg=24, cores=cores), (0, -1, 0.20), (0, 0.300, 0.335)), nivel=0, vivo=True)]
    torreta.append(m.obj("pavio", poli.membro([(0, 0.215, 0.500), (0.012, 0.250, 0.585), (0.034, 0.262, 0.650)],
                                              [0.014, 0.013, 0.011], cor="bege"), nivel=1))
    for nome, cor, direcao, comp in (("faisca", "amarelo", (0.3, 0.2, 1), 0.050), ("faisca2", "vermelho", (1, -0.4, 0.1), 0.045),
                                     ("faisca3", "amarelo", (-0.2, 1, 0.2), 0.045)):
        f = poli.torno([(0, -comp), (0.022, 0), (0, comp)], seg=6, cor=cor)
        torreta.append(m.obj(nome, poli.orientar(f, direcao, (0.036, 0.264, 0.660)), nivel=0, vivo=True))
    return torreta


def base(m):
    m.por("base", carreta(m))
    m.por("torreta", cano(m))
    m.pivo("base", (0, 0, 0))
    m.pivo("torreta", (0, 0.05, 0.38))


CAMINHOS = [{}, {}, {}]   # os upgrades entram na Parte B
