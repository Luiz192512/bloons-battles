"""Mapas: trilhas (polilinhas), agua e obstaculos. Coordenadas em 1040x720."""

from __future__ import annotations

import bisect
import math
from dataclasses import dataclass, field

LARGURA_MAPA = 1040
ALTURA_MAPA = 720
LARGURA_TRILHA = 46


class Caminho:
    """Polilinha com consulta de posicao por distancia percorrida."""

    def __init__(self, pontos: list[tuple[float, float]]) -> None:
        self.pontos = pontos
        self.acum = [0.0]
        for (x1, y1), (x2, y2) in zip(pontos, pontos[1:]):
            self.acum.append(self.acum[-1] + math.hypot(x2 - x1, y2 - y1))
        self.comprimento = self.acum[-1]
        # amostras a cada 8 px para busca rapida de pontos proximos
        self.amostras: list[tuple[float, float, float]] = []
        d = 0.0
        while d <= self.comprimento:
            x, y, _ = self.posicao(d)
            self.amostras.append((d, x, y))
            d += 8.0

    def posicao(self, d: float) -> tuple[float, float, float]:
        """(x, y, angulo em graus) na distancia d."""
        if d <= 0:
            i = 0
        elif d >= self.comprimento:
            i = len(self.pontos) - 2
        else:
            i = bisect.bisect_right(self.acum, d) - 1
        i = max(0, min(i, len(self.pontos) - 2))
        (x1, y1), (x2, y2) = self.pontos[i], self.pontos[i + 1]
        seg = self.acum[i + 1] - self.acum[i] or 1.0
        t = (d - self.acum[i]) / seg
        ang = math.degrees(math.atan2(y2 - y1, x2 - x1))
        return x1 + (x2 - x1) * t, y1 + (y2 - y1) * t, ang

    def distancia_ponto(self, x: float, y: float) -> float:
        melhor = 1e9
        for (x1, y1), (x2, y2) in zip(self.pontos, self.pontos[1:]):
            dx, dy = x2 - x1, y2 - y1
            L2 = dx * dx + dy * dy or 1.0
            t = max(0.0, min(1.0, ((x - x1) * dx + (y - y1) * dy) / L2))
            px, py = x1 + dx * t, y1 + dy * t
            melhor = min(melhor, math.hypot(x - px, y - py))
        return melhor

    def distancias_no_raio(self, x: float, y: float, r: float) -> list[float]:
        r2 = r * r
        return [d for d, px, py in self.amostras
                if (px - x) ** 2 + (py - y) ** 2 <= r2 and 0 < d < self.comprimento]


@dataclass
class Mapa:
    chave: str
    nome: str
    dificuldade: str
    trilhas: list[list[tuple[float, float]]]
    agua: list[tuple[float, float, float]] = field(default_factory=list)       # circulos
    agua_ret: list[tuple[float, float, float, float]] = field(default_factory=list)
    obstaculos: list[tuple[float, float, float, str]] = field(default_factory=list)
    grama: tuple = (98, 170, 58)
    terra: tuple = (196, 160, 104)

    def __post_init__(self) -> None:
        self.caminhos = [Caminho(p) for p in self.trilhas]

    def eh_agua(self, x: float, y: float) -> bool:
        for cx, cy, r in self.agua:
            if (x - cx) ** 2 + (y - cy) ** 2 <= r * r:
                return True
        for rx, ry, rw, rh in self.agua_ret:
            if rx <= x <= rx + rw and ry <= y <= ry + rh:
                return True
        return False

    def na_trilha(self, x: float, y: float, raio: float) -> bool:
        return any(c.distancia_ponto(x, y) < LARGURA_TRILHA / 2 + raio for c in self.caminhos)

    def bloqueado(self, x: float, y: float, raio: float) -> bool:
        for ox, oy, orr, _ in self.obstaculos:
            if math.hypot(x - ox, y - oy) < orr + raio * 0.6:
                return True
        return False


MAPAS: dict[str, Mapa] = {}


def _registrar(m: Mapa) -> None:
    MAPAS[m.chave] = m


_registrar(Mapa(
    "prado", "Prado dos Macacos", "Iniciante",
    [[(-40, 330), (170, 330), (170, 120), (420, 120), (420, 575), (240, 575), (240, 440),
      (640, 440), (640, 190), (860, 190), (860, 610), (560, 610), (560, 760)]],
    obstaculos=[(60, 110, 34, "arvore"), (300, 300, 30, "arvore"), (980, 80, 36, "arvore"),
                (760, 350, 26, "pedra"), (80, 620, 40, "arvore"), (960, 470, 30, "arvore"),
                (330, 670, 24, "pedra")],
))

_registrar(Mapa(
    "lago", "Lago Sereno", "Iniciante",
    [[(-40, 150), (260, 150), (260, 60), (800, 60), (800, 150), (960, 150), (960, 620),
      (700, 620), (700, 520), (330, 520), (330, 640), (80, 640), (80, 330), (-40, 330)]],
    agua=[(530, 300, 150)],
    obstaculos=[(150, 470, 36, "arvore"), (1010, 40, 22, "pedra"), (560, 680, 26, "arvore"),
                (880, 400, 30, "arvore")],
    grama=(92, 168, 70),
))

_registrar(Mapa(
    "encruzilhada", "Encruzilhada", "Intermediário",
    [[(-40, 200), (300, 200), (300, 420), (700, 420), (700, 120), (1080, 120)],
     [(-40, 560), (400, 560), (400, 330), (820, 330), (820, 640), (1080, 640)]],
    agua_ret=[(480, 470, 170, 110)],
    obstaculos=[(150, 380, 34, "arvore"), (560, 240, 30, "pedra"), (950, 380, 40, "arvore"),
                (150, 60, 28, "arvore"), (600, 680, 22, "pedra")],
    grama=(110, 176, 64), terra=(186, 150, 96),
))

_registrar(Mapa(
    "espiral", "Espiral da Selva", "Avançado",
    [[(520, -40), (520, 60), (940, 60), (940, 660), (100, 660), (100, 140), (820, 140),
      (820, 560), (220, 560), (220, 250), (700, 250), (700, 450), (380, 450), (380, 360)]],
    agua=[(540, 350, 45)],
    obstaculos=[(40, 40, 30, "arvore"), (1000, 700, 30, "arvore"), (990, 300, 24, "pedra")],
    grama=(84, 150, 56), terra=(170, 130, 84),
))

ORDEM_MAPAS = list(MAPAS)
