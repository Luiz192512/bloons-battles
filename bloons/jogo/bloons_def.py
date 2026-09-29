"""Tipos de bloons: vida, velocidade, filhos, imunidades e RBE."""

from __future__ import annotations

from dataclasses import dataclass, field

# Tipos de dano
AFIADO = "afiado"      # dardos, tachinhas, laminas
EXPLOSAO = "explosao"  # bombas, morteiro
GELO = "gelo"          # congelamento
ENERGIA = "energia"    # magia, fogo, plasma, laser
NORMAL = "normal"      # estoura qualquer coisa

VELOCIDADE_BASE = 95.0  # px/s do bloon vermelho


@dataclass(frozen=True)
class TipoBloon:
    nome: str
    rotulo: str
    cor: tuple
    raio: float
    velocidade: float           # multiplicador da base
    vida: int = 1
    filhos: tuple = ()
    imune: frozenset = frozenset()
    moab: bool = False          # classe MOAB (dirigiveis)
    congela: bool = True        # pode ser congelado
    camo_nativo: bool = False
    rank: int = 0
    vida_fortificado: int = 0


def _t(nome, rotulo, cor, raio, vel, rank, **kw) -> TipoBloon:
    return TipoBloon(nome, rotulo, cor, raio, vel, rank=rank, **kw)


TIPOS: dict[str, TipoBloon] = {}
for tb in [
    _t("vermelho", "Vermelho", (225, 38, 38), 12, 1.0, 1),
    _t("azul", "Azul", (40, 140, 235), 13, 1.4, 2, filhos=("vermelho",)),
    _t("verde", "Verde", (60, 190, 60), 13.5, 1.8, 3, filhos=("azul",)),
    _t("amarelo", "Amarelo", (250, 220, 30), 14, 3.2, 4, filhos=("verde",)),
    _t("rosa", "Rosa", (250, 110, 170), 14.5, 3.5, 5, filhos=("amarelo",)),
    _t("preto", "Preto", (30, 30, 34), 9.5, 1.8, 6, filhos=("rosa", "rosa"),
       imune=frozenset({EXPLOSAO})),
    _t("branco", "Branco", (245, 245, 245), 9.5, 2.0, 6, filhos=("rosa", "rosa"),
       imune=frozenset({GELO}), congela=False),
    _t("roxo", "Roxo", (150, 60, 200), 13, 3.0, 6, filhos=("rosa", "rosa"),
       imune=frozenset({ENERGIA})),
    _t("chumbo", "Chumbo", (125, 130, 140), 14, 1.0, 7, filhos=("preto", "preto"),
       imune=frozenset({AFIADO, GELO}), vida_fortificado=4),
    _t("zebra", "Zebra", (230, 230, 230), 14, 1.8, 7, filhos=("preto", "branco"),
       imune=frozenset({EXPLOSAO, GELO}), congela=False),
    _t("arco_iris", "Arco-íris", (255, 140, 0), 15, 2.2, 8, filhos=("zebra", "zebra")),
    _t("ceramica", "Cerâmica", (170, 100, 45), 15.5, 2.5, 9, vida=10,
       filhos=("arco_iris", "arco_iris"), vida_fortificado=20),
    _t("moab", "M.O.A.B.", (50, 110, 220), 42, 1.0, 10, vida=200,
       filhos=("ceramica",) * 4, moab=True, congela=False, vida_fortificado=400),
    _t("bfb", "B.F.B.", (200, 40, 40), 55, 0.25, 11, vida=700,
       filhos=("moab",) * 4, moab=True, congela=False, vida_fortificado=1400),
    _t("zomg", "Z.O.M.G.", (40, 120, 40), 66, 0.18, 12, vida=4000,
       filhos=("bfb",) * 4, moab=True, congela=False, vida_fortificado=8000),
    _t("ddt", "D.D.T.", (45, 45, 50), 40, 2.75, 12, vida=400,
       filhos=("ceramica",) * 6, moab=True, congela=False, camo_nativo=True,
       imune=frozenset({AFIADO, EXPLOSAO}), vida_fortificado=800),
    _t("bad", "B.A.D.", (120, 40, 150), 80, 0.18, 13, vida=20000,
       filhos=("zomg", "zomg", "ddt", "ddt", "ddt"), moab=True, congela=False,
       vida_fortificado=40000),
]:
    TIPOS[tb.nome] = tb

ORDEM = list(TIPOS)

# Proximo tipo quando um bloon regenerado (regrow) cresce uma camada
REGEN_PROXIMO = {
    "vermelho": "azul", "azul": "verde", "verde": "amarelo", "amarelo": "rosa",
    "rosa": "preto", "preto": "zebra", "branco": "zebra", "roxo": "zebra",
    "zebra": "arco_iris", "arco_iris": "ceramica",
}


def _rbe(nome: str, fort: bool = False) -> int:
    t = TIPOS[nome]
    vida = t.vida_fortificado if fort and t.vida_fortificado else t.vida
    return vida + sum(_rbe(f, fort) for f in t.filhos)


RBE = {n: _rbe(n) for n in TIPOS}
RBE_FORT = {n: _rbe(n, True) for n in TIPOS}


def rbe(nome: str, fortificado: bool = False) -> int:
    return RBE_FORT[nome] if fortificado else RBE[nome]
