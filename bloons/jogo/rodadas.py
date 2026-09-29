"""Rodadas (inspiradas no modo classico) e envios de bloons do modo Batalha."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class Grupo:
    tipo: str
    qtd: int
    espaco: float        # segundos entre bloons
    inicio: float = 0.0  # atraso do grupo em segundos
    camo: bool = False
    regen: bool = False
    fort: bool = False


def G(tipo, qtd, espaco=0.5, inicio=0.0, c=False, r=False, f=False) -> Grupo:
    return Grupo(tipo, qtd, espaco, inicio, c, r, f)


# fmt: off
_RODADAS: dict[int, list[Grupo]] = {
    1: [G("vermelho", 20, 0.9)],
    2: [G("vermelho", 35, 0.55)],
    3: [G("vermelho", 25, 0.6), G("azul", 5, 1.0, 8)],
    4: [G("vermelho", 35, 0.45), G("azul", 18, 0.8, 4)],
    5: [G("vermelho", 5, 0.8), G("azul", 27, 0.5, 2)],
    6: [G("vermelho", 15, 0.6), G("azul", 15, 0.6, 3), G("verde", 4, 1.2, 8)],
    7: [G("vermelho", 20, 0.5), G("azul", 25, 0.5, 4), G("verde", 5, 0.9, 12)],
    8: [G("vermelho", 10, 0.5), G("azul", 20, 0.5, 2), G("verde", 14, 0.6, 8)],
    9: [G("verde", 30, 0.55)],
    10: [G("azul", 102, 0.17)],
    11: [G("vermelho", 10, 0.5), G("azul", 10, 0.5, 2), G("verde", 12, 0.5, 5),
         G("amarelo", 2, 1.5, 9)],
    12: [G("azul", 15, 0.5), G("verde", 10, 0.6, 4), G("amarelo", 5, 1.0, 8)],
    13: [G("azul", 50, 0.3), G("verde", 23, 0.5, 6)],
    14: [G("vermelho", 49, 0.2), G("azul", 15, 0.4, 5), G("verde", 10, 0.5, 9),
         G("amarelo", 9, 0.7, 13)],
    15: [G("vermelho", 20, 0.3), G("verde", 15, 0.5, 4), G("amarelo", 12, 0.6, 9),
         G("rosa", 5, 1.0, 14)],
    16: [G("verde", 20, 0.4), G("amarelo", 8, 0.8, 5)],
    17: [G("amarelo", 8, 0.8, r=True)],
    18: [G("verde", 80, 0.22)],
    19: [G("verde", 10, 0.4), G("amarelo", 4, 0.8, 3), G("amarelo", 5, 0.8, 7, r=True),
         G("rosa", 7, 0.7, 11)],
    20: [G("preto", 6, 1.0)],
    21: [G("amarelo", 14, 0.5), G("rosa", 40, 0.3, 5)],
    22: [G("branco", 16, 0.8)],
    23: [G("preto", 7, 0.8), G("branco", 7, 0.8, 4)],
    24: [G("verde", 1, 1.0, c=True), G("azul", 20, 0.4, 2)],
    25: [G("amarelo", 31, 0.4, r=True), G("roxo", 10, 0.8, 8)],
    26: [G("rosa", 23, 0.4), G("zebra", 4, 1.2, 8)],
    27: [G("vermelho", 100, 0.1), G("azul", 60, 0.12, 5), G("verde", 45, 0.15, 10),
         G("amarelo", 45, 0.2, 15)],
    28: [G("chumbo", 6, 1.2)],
    29: [G("amarelo", 48, 0.3), G("rosa", 12, 0.6, 8, r=True)],
    30: [G("chumbo", 9, 1.0)],
    31: [G("preto", 8, 0.6), G("branco", 8, 0.6, 3), G("zebra", 4, 1.0, 7, r=True)],
    32: [G("preto", 25, 0.35), G("branco", 28, 0.35, 5)],
    33: [G("vermelho", 13, 0.3), G("amarelo", 20, 0.4, 3, c=True)],
    34: [G("amarelo", 140, 0.12), G("zebra", 5, 1.0, 10)],
    35: [G("rosa", 35, 0.25), G("preto", 30, 0.3, 5), G("branco", 25, 0.3, 10),
         G("arco_iris", 5, 1.2, 15)],
    36: [G("rosa", 81, 0.15)],
    37: [G("preto", 20, 0.4), G("branco", 20, 0.4, 4), G("zebra", 15, 0.5, 9, r=True),
         G("branco", 10, 0.6, 14, c=True)],
    38: [G("rosa", 42, 0.2), G("branco", 17, 0.4, 4), G("chumbo", 14, 0.6, 8),
         G("zebra", 10, 0.5, 13), G("ceramica", 4, 1.5, 17)],
    39: [G("preto", 10, 0.4), G("branco", 10, 0.4, 2), G("chumbo", 20, 0.4, 6),
         G("arco_iris", 18, 0.5, 11, r=True)],
    40: [G("moab", 1, 1.0)],
    41: [G("preto", 60, 0.2), G("zebra", 60, 0.2, 6)],
    42: [G("arco_iris", 6, 0.8, r=True), G("arco_iris", 4, 0.8, 6, c=True)],
    43: [G("arco_iris", 10, 0.6), G("ceramica", 7, 0.9, 6)],
    44: [G("zebra", 50, 0.25)],
    45: [G("rosa", 200, 0.08), G("ceramica", 8, 0.8, 8, r=True)],
    46: [G("preto", 10, 0.5, c=True), G("moab", 1, 1.0, 6)],
    47: [G("rosa", 70, 0.15, c=True), G("ceramica", 12, 0.6, 8)],
    48: [G("rosa", 120, 0.1, r=True), G("arco_iris", 50, 0.25, 10)],
    49: [G("verde", 343, 0.05), G("zebra", 20, 0.3, 10), G("arco_iris", 30, 0.3, 14),
         G("ceramica", 15, 0.6, 20)],
    50: [G("chumbo", 20, 0.4, f=True), G("moab", 2, 2.0, 6)],
    51: [G("arco_iris", 28, 0.3, c=True), G("ceramica", 10, 0.5, 8)],
    52: [G("ceramica", 25, 0.4, r=True), G("moab", 2, 2.0, 8)],
    53: [G("rosa", 80, 0.1, c=True), G("moab", 3, 1.5, 6)],
    54: [G("ceramica", 35, 0.35), G("moab", 2, 2.0, 10)],
    55: [G("ceramica", 45, 0.3, r=True)],
    56: [G("arco_iris", 40, 0.25, c=True), G("moab", 3, 1.5, 8)],
    57: [G("ceramica", 40, 0.3), G("moab", 4, 1.2, 10)],
    58: [G("chumbo", 30, 0.3, f=True), G("ceramica", 25, 0.3, 8, f=True)],
    59: [G("ceramica", 50, 0.25, r=True), G("moab", 3, 1.5, 12)],
    60: [G("bfb", 1, 1.0)],
    61: [G("zebra", 120, 0.08, c=True), G("moab", 5, 1.0, 8)],
    62: [G("ceramica", 60, 0.2, c=True), G("moab", 4, 1.2, 10)],
    63: [G("chumbo", 50, 0.2), G("ceramica", 75, 0.15, 6)],
    64: [G("moab", 9, 0.8)],
    65: [G("zebra", 80, 0.1), G("arco_iris", 60, 0.15, 6), G("ceramica", 40, 0.25, 12),
         G("bfb", 1, 1.0, 18)],
    66: [G("ceramica", 50, 0.2, f=True), G("moab", 4, 1.0, 8)],
    67: [G("moab", 6, 0.8), G("ceramica", 40, 0.25, 4, c=True)],
    68: [G("moab", 4, 1.0, f=True), G("bfb", 1, 1.0, 6)],
    69: [G("chumbo", 60, 0.15, f=True), G("ceramica", 50, 0.2, 8, r=True)],
    70: [G("arco_iris", 200, 0.05), G("moab", 4, 1.0, 10)],
    71: [G("ceramica", 70, 0.15, c=True, r=True), G("moab", 5, 0.8, 10)],
    72: [G("chumbo", 50, 0.2), G("moab", 8, 0.6, 6, f=True)],
    73: [G("ceramica", 100, 0.1, f=True)],
    74: [G("bfb", 3, 2.0), G("moab", 8, 0.6, 4)],
    75: [G("ceramica", 80, 0.12, c=True), G("bfb", 2, 2.0, 10)],
    76: [G("ceramica", 120, 0.08, f=True, r=True)],
    77: [G("moab", 14, 0.5, f=True), G("bfb", 2, 2.0, 8)],
    78: [G("ceramica", 150, 0.07, f=True), G("bfb", 3, 1.5, 10)],
    79: [G("moab", 20, 0.4, f=True), G("bfb", 3, 1.5, 8)],
    80: [G("zomg", 1, 1.0), G("bfb", 2, 2.0, 5)],
    85: [G("zomg", 2, 2.0), G("ceramica", 120, 0.08, 4, f=True)],
    90: [G("ddt", 6, 1.0)],
    95: [G("zomg", 4, 1.5, f=True), G("ddt", 10, 0.6, 6)],
    100: [G("bad", 1, 1.0)],
}
# fmt: on


def grupos_da_rodada(r: int) -> list[Grupo]:
    if r in _RODADAS:
        return _RODADAS[r]
    # rodadas livres/intermediarias acima de 80: mistura crescente
    k = r - 80
    return [
        G("ceramica", 80 + 6 * k, 0.08, f=k > 3, r=True),
        G("moab", 6 + k, 0.5, 4, f=True),
        G("bfb", 2 + k // 3, 1.5, 8, f=k > 6),
        G("zomg", k // 6, 2.0, 12),
        G("ddt", k // 4, 0.8, 14),
    ]


def agenda_da_rodada(r: int) -> list[tuple[float, Grupo]]:
    """Lista (tempo_s, grupo) de cada bloon da rodada, ordenada por tempo."""
    eventos = []
    for g in grupos_da_rodada(r):
        for i in range(g.qtd):
            eventos.append((g.inicio + i * g.espaco, g))
    eventos.sort(key=lambda e: e[0])
    return eventos


def duracao_rodada(r: int) -> float:
    ag = agenda_da_rodada(r)
    return ag[-1][0] if ag else 0.0


# Dificuldades do modo solo: (vidas, multiplicador de custo, ultima rodada)
DIFICULDADES = {
    "facil": ("Fácil", 200, 0.85, 40),
    "medio": ("Médio", 150, 1.0, 60),
    "dificil": ("Difícil", 100, 1.08, 80),
    "impossivel": ("Impossível", 1, 1.2, 100),
}


@dataclass(frozen=True)
class Envio:
    chave: str
    nome: str
    tipo: str
    qtd: int
    espaco: float
    custo: int
    eco: float
    rodada_min: int
    camo: bool = False
    regen: bool = False
    fort: bool = False


# Envios do modo Batalha (inspirados no Battles 2): custo, efeito na renda (eco) e desbloqueio
ENVIOS: list[Envio] = [
    Envio("r8", "8 Vermelhos", "vermelho", 8, 0.12, 25, 1.0, 1),
    Envio("b6", "6 Azuis", "azul", 6, 0.14, 30, 1.2, 1),
    Envio("g5", "5 Verdes", "verde", 5, 0.16, 40, 1.4, 2),
    Envio("y4", "4 Amarelos", "amarelo", 4, 0.18, 45, 1.6, 4),
    Envio("p4", "4 Rosas", "rosa", 4, 0.18, 55, 1.8, 6),
    Envio("yr", "6 Amarelos Regen", "amarelo", 6, 0.2, 70, 2.2, 7, regen=True),
    Envio("gc", "6 Verdes Camo", "verde", 6, 0.2, 80, 2.4, 8, camo=True),
    Envio("k4", "4 Pretos", "preto", 4, 0.3, 90, 2.6, 9),
    Envio("w4", "4 Brancos", "branco", 4, 0.3, 90, 2.6, 9),
    Envio("u4", "4 Roxos", "roxo", 4, 0.3, 110, 2.8, 11),
    Envio("l3", "3 Chumbos", "chumbo", 3, 0.4, 150, 3.2, 12),
    Envio("z3", "3 Zebras", "zebra", 3, 0.4, 150, 3.2, 13),
    Envio("ra2", "2 Arco-íris", "arco_iris", 2, 0.5, 220, 3.6, 15),
    Envio("rar", "3 Arco-íris Regen", "arco_iris", 3, 0.4, 350, 4.0, 17, regen=True),
    Envio("c2", "2 Cerâmicas", "ceramica", 2, 0.5, 400, 4.0, 18),
    Envio("cc", "3 Cerâmicas Camo", "ceramica", 3, 0.5, 700, 4.5, 21, camo=True),
    Envio("cf", "3 Cerâmicas Fortificadas", "ceramica", 3, 0.5, 850, 4.5, 23, fort=True),
    Envio("m1", "M.O.A.B.", "moab", 1, 1.0, 1500, 0.0, 25),
    Envio("mf", "M.O.A.B. Fortificado", "moab", 1, 1.0, 2400, 0.0, 28, fort=True),
    Envio("bfb", "B.F.B.", "bfb", 1, 1.0, 6000, -10.0, 32),
    Envio("ddt", "D.D.T.", "ddt", 1, 1.0, 6500, -10.0, 34),
    Envio("zomg", "Z.O.M.G.", "zomg", 1, 1.0, 18000, -30.0, 38),
    Envio("bad", "B.A.D.", "bad", 1, 1.0, 60000, -80.0, 45),
]
ENVIOS_POR_CHAVE = {e.chave: e for e in ENVIOS}
