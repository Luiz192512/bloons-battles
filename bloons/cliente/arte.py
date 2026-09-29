"""Arte do jogo desenhada por codigo (sem imagens externas).

Tudo e desenhado uma vez e guardado em cache: macacos por tipo/tier/tamanho,
bloons por tipo/propriedades, dirigiveis por angulo e o fundo de cada mapa.
"""

from __future__ import annotations

import math
import random
from functools import lru_cache

import pygame

from bloons.jogo.bloons_def import TIPOS
from bloons.jogo.mapas import ALTURA_MAPA, LARGURA_MAPA, LARGURA_TRILHA, MAPAS

PRETO = (20, 20, 24)
BRANCO = (255, 255, 255)
PELO = (140, 88, 44)
PELO_ESCURO = (98, 60, 28)
ROSTO = (236, 196, 150)


def _sombra(cor, f=0.7):
    return tuple(max(0, int(c * f)) for c in cor[:3])


def _clarear(cor, f=0.35):
    return tuple(min(255, int(c + (255 - c) * f)) for c in cor[:3])


# ======================================================================= MACACOS
# Cada torre: (pelo, rosto, chapeu, item, cor_chapeu)
ESTILO = {
    "dardo": (PELO, ROSTO, None, "dardo", None),
    "bumerangue": (PELO, ROSTO, "faixa", "bumerangue", (220, 60, 50)),
    "gelo": ((150, 205, 235), (230, 245, 255), "gorro", None, (80, 150, 210)),
    "cola": (PELO, ROSTO, "oculos", "arma_cola", (120, 190, 60)),
    "sniper": (PELO, ROSTO, "capacete", "rifle", (80, 110, 60)),
    "mago": (PELO, ROSTO, "chapeu_mago", "varinha", (110, 60, 170)),
    "super": (PELO, ROSTO, "mascara_super", None, (40, 90, 200)),
    "ninja": (PELO, ROSTO, "ninja", "shuriken", (180, 30, 30)),
    "alquimista": (PELO, ROSTO, "chapeu_alq", "pocao", (110, 60, 130)),
    "druida": (PELO, ROSTO, "coroa_folhas", None, (60, 140, 50)),
    "engenheiro": (PELO, ROSTO, "capacete_obra", "pregadora", (240, 200, 40)),
    "sentinela": (None, None, None, None, None),
}

HEROI_ESTILO = {
    "quincy": ((150, 95, 45), ROSTO, "capuz", "arco", (60, 120, 60)),
    "gwendolin": ((190, 80, 40), ROSTO, "oculos", "lanca_chamas", (230, 120, 30)),
    "striker": ((110, 80, 50), ROSTO, "boina", "lanca_foguete", (60, 90, 50)),
    "obyn": ((60, 110, 90), (200, 230, 210), "coroa_folhas", "cajado", (40, 150, 90)),
    "churchill": ((110, 80, 50), ROSTO, "tanque", None, (80, 100, 70)),
    "benjamin": ((90, 70, 60), ROSTO, "fones", "laptop", (40, 40, 50)),
    "ezili": ((110, 60, 80), ROSTO, "caveira", "cajado", (150, 40, 110)),
    "pat": ((150, 110, 70), ROSTO, None, None, None),
    "adora": ((200, 160, 90), ROSTO, "coroa_sol", None, (240, 200, 60)),
    "brickell": ((110, 80, 50), ROSTO, "quepe", "pistola", (40, 60, 130)),
    "etienne": ((120, 90, 60), ROSTO, "boina", "controle", (70, 110, 160)),
    "sauda": ((160, 100, 60), ROSTO, "faixa", "espadas", (200, 60, 60)),
    "psi": ((150, 110, 170), (230, 210, 240), "psi", None, (180, 100, 220)),
    "geraldo": ((120, 80, 50), ROSTO, "chapeu_alq", "sacola", (140, 90, 40)),
    "corvus": ((50, 50, 80), (180, 180, 210), "capuz", "cajado", (40, 40, 90)),
    "rosalia": ((170, 100, 110), ROSTO, "oculos", "lanca_foguete", (220, 100, 130)),
    "jericho": ((120, 85, 55), ROSTO, "chapeu_cowboy", "pistola", (110, 70, 40)),
    "silas": ((140, 200, 230), (230, 245, 255), "gorro", "cajado", (90, 160, 220)),
}


def _circulo(s, cor, c, r, borda=2):
    pygame.draw.circle(s, _sombra(cor, 0.55), c, r + borda)
    pygame.draw.circle(s, cor, c, r)


def _macaco_base(s, cx, cy, u, pelo, rosto):
    """Macaco visto de cima, olhando para cima (-y). u = unidade (raio da cabeca)."""
    # corpo e bracos
    _circulo(s, pelo, (cx, cy + int(u * 0.55)), int(u * 0.8))
    _circulo(s, pelo, (cx - int(u * 0.75), cy - int(u * 0.2)), int(u * 0.32))
    _circulo(s, pelo, (cx + int(u * 0.75), cy - int(u * 0.2)), int(u * 0.32))
    # orelhas
    for sx in (-1, 1):
        _circulo(s, pelo, (cx + sx * int(u * 0.95), cy - int(u * 0.1)), int(u * 0.3))
        pygame.draw.circle(s, rosto, (cx + sx * int(u * 0.95), cy - int(u * 0.1)), int(u * 0.17))
    # cabeca
    _circulo(s, pelo, (cx, cy), int(u))
    # rosto
    pygame.draw.ellipse(s, rosto, (cx - int(u * 0.62), cy - int(u * 0.75), int(u * 1.24), int(u * 1.05)))
    # olhos
    for sx in (-1, 1):
        pygame.draw.circle(s, BRANCO, (cx + sx * int(u * 0.25), cy - int(u * 0.4)), max(2, int(u * 0.18)))
        pygame.draw.circle(s, PRETO, (cx + sx * int(u * 0.25), cy - int(u * 0.45)), max(1, int(u * 0.1)))
    # focinho
    pygame.draw.ellipse(s, _sombra(rosto, 0.9), (cx - int(u * 0.32), cy - int(u * 0.2), int(u * 0.64), int(u * 0.38)))
    pygame.draw.circle(s, _sombra(rosto, 0.5), (cx - int(u * 0.1), cy - int(u * 0.1)), max(1, int(u * 0.05)))
    pygame.draw.circle(s, _sombra(rosto, 0.5), (cx + int(u * 0.1), cy - int(u * 0.1)), max(1, int(u * 0.05)))


def _chapeu(s, cx, cy, u, tipo, cor):
    if tipo is None:
        return
    if tipo == "faixa":
        pygame.draw.rect(s, cor, (cx - u, cy - int(u * 0.75), 2 * u, int(u * 0.28)), border_radius=3)
    elif tipo == "gorro":
        pygame.draw.ellipse(s, cor, (cx - u, cy - int(u * 1.05), 2 * u, int(u * 0.9)))
        pygame.draw.circle(s, BRANCO, (cx, cy - int(u * 1.05)), int(u * 0.25))
    elif tipo == "oculos":
        for sx in (-1, 1):
            pygame.draw.circle(s, cor, (cx + sx * int(u * 0.28), cy - int(u * 0.42)), int(u * 0.24), 3)
    elif tipo in ("capacete", "capacete_obra"):
        pygame.draw.ellipse(s, _sombra(cor, 0.6), (cx - int(u * 1.08), cy - int(u * 1.12), int(u * 2.16), int(u * 1.2)))
        pygame.draw.ellipse(s, cor, (cx - u, cy - int(u * 1.08), 2 * u, int(u * 1.08)))
        if tipo == "capacete_obra":
            pygame.draw.rect(s, _sombra(cor, 0.8), (cx - int(u * 0.12), cy - int(u * 1.08), int(u * 0.24), int(u * 0.9)))
    elif tipo == "chapeu_mago":
        pts = [(cx - int(u * 1.1), cy - int(u * 0.45)), (cx + int(u * 1.1), cy - int(u * 0.45)), (cx, cy - int(u * 2.0))]
        pygame.draw.ellipse(s, _sombra(cor, 0.6), (cx - int(u * 1.2), cy - int(u * 0.8), int(u * 2.4), int(u * 0.7)))
        pygame.draw.polygon(s, cor, pts)
        pygame.draw.circle(s, (255, 230, 90), (cx + int(u * 0.2), cy - int(u * 1.1)), max(2, int(u * 0.13)))
    elif tipo == "mascara_super":
        pygame.draw.rect(s, cor, (cx - int(u * 0.62), cy - int(u * 0.62), int(u * 1.24), int(u * 0.34)), border_radius=4)
        pygame.draw.polygon(s, (220, 40, 40), [(cx - u, cy + int(u * 0.3)), (cx + u, cy + int(u * 0.3)), (cx + int(u * 1.3), cy + int(u * 1.6)), (cx - int(u * 1.3), cy + int(u * 1.6))])
    elif tipo == "ninja":
        pygame.draw.ellipse(s, (30, 30, 36), (cx - u, cy - u, 2 * u, int(u * 1.3)))
        pygame.draw.rect(s, ROSTO, (cx - int(u * 0.55), cy - int(u * 0.62), int(u * 1.1), int(u * 0.36)), border_radius=4)
        for sx in (-1, 1):
            pygame.draw.circle(s, PRETO, (cx + sx * int(u * 0.25), cy - int(u * 0.45)), max(1, int(u * 0.1)))
        pygame.draw.rect(s, cor, (cx - u, cy - int(u * 0.2), 2 * u, int(u * 0.22)))
    elif tipo == "chapeu_alq":
        pygame.draw.ellipse(s, _sombra(cor, 0.7), (cx - int(u * 1.15), cy - int(u * 0.95), int(u * 2.3), int(u * 0.6)))
        pygame.draw.rect(s, cor, (cx - int(u * 0.6), cy - int(u * 1.6), int(u * 1.2), int(u * 0.9)), border_radius=4)
    elif tipo in ("coroa_folhas", "coroa_sol"):
        for k in range(7):
            a = math.pi + k * math.pi / 6
            x = cx + math.cos(a) * u * 0.9
            y = cy - int(u * 0.3) + math.sin(a) * u * 0.9
            if tipo == "coroa_sol":
                pygame.draw.polygon(s, cor, [(x, y), (x + 4, y + 8), (x - 4, y + 8)])
            else:
                pygame.draw.ellipse(s, cor, (x - 5, y - 4, 10, 8))
    elif tipo == "capuz":
        pygame.draw.arc(s, cor, (cx - int(u * 1.15), cy - int(u * 1.15), int(u * 2.3), int(u * 2.3)), 0.2, math.pi - 0.2, max(3, int(u * 0.35)))
    elif tipo in ("boina", "quepe", "chapeu_cowboy"):
        largura = 1.4 if tipo == "chapeu_cowboy" else 1.0
        pygame.draw.ellipse(s, cor, (cx - int(u * largura), cy - int(u * 1.15), int(u * largura * 2), int(u * 0.8)))
        if tipo == "quepe":
            pygame.draw.rect(s, (230, 200, 60), (cx - int(u * 0.2), cy - int(u * 0.95), int(u * 0.4), int(u * 0.2)))
    elif tipo == "fones":
        pygame.draw.arc(s, cor, (cx - int(u * 1.1), cy - int(u * 1.1), int(u * 2.2), int(u * 1.6)), 0, math.pi, 4)
        for sx in (-1, 1):
            pygame.draw.circle(s, cor, (cx + sx * int(u * 1.0), cy - int(u * 0.2)), int(u * 0.3))
    elif tipo == "caveira":
        pygame.draw.circle(s, (240, 240, 230), (cx, cy - int(u * 0.95)), int(u * 0.38))
        pygame.draw.circle(s, PRETO, (cx - 3, cy - int(u * 1.0)), 2)
        pygame.draw.circle(s, PRETO, (cx + 3, cy - int(u * 1.0)), 2)
    elif tipo == "tanque":
        pass
    elif tipo == "psi":
        pygame.draw.circle(s, (240, 200, 255), (cx, cy - int(u * 0.2)), int(u * 1.25), 2)
        pygame.draw.circle(s, cor, (cx, cy - int(u * 0.95)), int(u * 0.22))


def _item(s, cx, cy, u, item, cor):
    """Objeto nas maos, apontando para cima."""
    if item is None:
        return
    hx, hy = cx + int(u * 0.75), cy - int(u * 0.45)
    if item in ("dardo", "pregadora"):
        pygame.draw.line(s, (90, 90, 100), (hx, hy), (hx, hy - int(u * 0.9)), max(2, u // 6))
        pygame.draw.polygon(s, (200, 200, 210), [(hx - 3, hy - int(u * 0.9)), (hx + 3, hy - int(u * 0.9)), (hx, hy - int(u * 1.2))])
        if item == "pregadora":
            pygame.draw.rect(s, (230, 190, 40), (hx - 5, hy - 8, 10, 14), border_radius=2)
    elif item == "bumerangue":
        pygame.draw.lines(s, (240, 200, 40), False, [(hx - 8, hy - 2), (hx, hy - 12), (hx + 8, hy - 2)], 4)
    elif item == "arma_cola":
        pygame.draw.rect(s, (90, 90, 90), (hx - 4, hy - int(u * 0.9), 8, int(u * 0.9)))
        pygame.draw.circle(s, (130, 200, 60), (hx, hy - int(u * 0.2)), int(u * 0.28))
    elif item == "rifle":
        pygame.draw.line(s, (60, 45, 30), (cx, cy - int(u * 0.3)), (cx, cy - int(u * 2.0)), max(3, u // 4))
        pygame.draw.line(s, (40, 40, 40), (cx, cy - int(u * 1.3)), (cx, cy - int(u * 2.3)), max(2, u // 6))
    elif item in ("varinha", "cajado"):
        pygame.draw.line(s, (110, 70, 30), (hx, hy + 6), (hx, hy - int(u * 1.1)), 3)
        pygame.draw.circle(s, cor or (160, 90, 220), (hx, hy - int(u * 1.15)), max(3, int(u * 0.2)))
    elif item == "shuriken":
        pygame.draw.polygon(s, (170, 170, 180), [(hx, hy - 9), (hx + 3, hy - 3), (hx + 9, hy), (hx + 3, hy + 3), (hx, hy + 9), (hx - 3, hy + 3), (hx - 9, hy), (hx - 3, hy - 3)])
    elif item == "pocao":
        pygame.draw.circle(s, (120, 220, 90), (hx, hy - 4), int(u * 0.28))
        pygame.draw.rect(s, (200, 200, 220), (hx - 2, hy - 4 - int(u * 0.45), 4, int(u * 0.2)))
    elif item == "arco":
        pygame.draw.arc(s, (110, 70, 30), (hx - 10, hy - int(u * 1.3), 20, int(u * 1.4)), -0.3, math.pi + 0.3, 3)
    elif item in ("lanca_chamas", "lanca_foguete", "pistola", "controle", "laptop", "sacola"):
        pygame.draw.rect(s, cor or (80, 80, 90), (hx - 5, hy - int(u * 0.9), 10, int(u * 0.9)), border_radius=3)
    elif item == "espadas":
        for sx in (-1, 1):
            x = cx + sx * int(u * 0.75)
            pygame.draw.line(s, (220, 220, 230), (x, hy), (x, hy - int(u * 1.3)), 3)


def _desenhar_torre_especial(s, cx, cy, u, chave, cor):
    """Torres que nao sao um macaco simples."""
    if chave == "tachinha":
        pts = [(cx + math.cos(k * math.pi / 4 + math.pi / 8) * u * 1.1,
                cy + math.sin(k * math.pi / 4 + math.pi / 8) * u * 1.1) for k in range(8)]
        pygame.draw.polygon(s, (90, 90, 100), [(x + 2, y + 2) for x, y in pts])
        pygame.draw.polygon(s, (175, 175, 185), pts)
        for k in range(8):
            a = k * math.pi / 4
            pygame.draw.line(s, (60, 60, 70), (cx, cy), (cx + math.cos(a) * u * 1.3, cy + math.sin(a) * u * 1.3), 3)
        pygame.draw.circle(s, (220, 60, 50), (cx, cy), int(u * 0.45))
        return True
    if chave == "bomba":
        _macaco_base(s, cx, cy + int(u * 0.4), int(u * 0.7), PELO, ROSTO)
        pygame.draw.rect(s, (30, 30, 36), (cx - int(u * 0.45), cy - int(u * 1.4), int(u * 0.9), int(u * 1.6)), border_radius=5)
        pygame.draw.rect(s, (70, 70, 80), (cx - int(u * 0.55), cy - int(u * 1.5), int(u * 1.1), int(u * 0.35)), border_radius=4)
        return True
    if chave == "submarino":
        pygame.draw.ellipse(s, (160, 130, 20), (cx - int(u * 0.65), cy - int(u * 1.5), int(u * 1.3), int(u * 3.0)))
        pygame.draw.ellipse(s, (240, 205, 40), (cx - int(u * 0.55), cy - int(u * 1.4), int(u * 1.1), int(u * 2.8)))
        pygame.draw.circle(s, (160, 130, 20), (cx, cy - int(u * 0.2)), int(u * 0.4))
        pygame.draw.circle(s, (120, 200, 230), (cx, cy - int(u * 0.2)), int(u * 0.25))
        return True
    if chave == "bucaneiro":
        pygame.draw.polygon(s, (90, 55, 25), [(cx, cy - int(u * 1.9)), (cx + int(u * 0.8), cy - int(u * 0.8)), (cx + int(u * 0.8), cy + int(u * 1.5)), (cx - int(u * 0.8), cy + int(u * 1.5)), (cx - int(u * 0.8), cy - int(u * 0.8))])
        pygame.draw.polygon(s, (140, 90, 45), [(cx, cy - int(u * 1.6)), (cx + int(u * 0.62), cy - int(u * 0.7)), (cx + int(u * 0.62), cy + int(u * 1.3)), (cx - int(u * 0.62), cy + int(u * 1.3)), (cx - int(u * 0.62), cy - int(u * 0.7))])
        pygame.draw.rect(s, (245, 240, 225), (cx - int(u * 0.9), cy - int(u * 0.5), int(u * 1.8), int(u * 0.5)))
        _macaco_base(s, cx, cy + int(u * 0.6), int(u * 0.45), PELO, ROSTO)
        return True
    if chave in ("as", "fenix"):
        corpo = (240, 200, 40) if chave == "as" else (250, 120, 30)
        pygame.draw.polygon(s, _sombra(corpo), [(cx - int(u * 1.8), cy), (cx + int(u * 1.8), cy), (cx, cy - int(u * 0.5))])
        pygame.draw.polygon(s, corpo, [(cx - int(u * 1.7), cy - 2), (cx + int(u * 1.7), cy - 2), (cx, cy - int(u * 0.6))])
        pygame.draw.ellipse(s, _sombra(corpo, 0.85), (cx - int(u * 0.35), cy - int(u * 1.4), int(u * 0.7), int(u * 2.6)))
        pygame.draw.polygon(s, corpo, [(cx - int(u * 0.7), cy + int(u * 1.1)), (cx + int(u * 0.7), cy + int(u * 1.1)), (cx, cy + int(u * 0.7))])
        pygame.draw.circle(s, (120, 200, 230), (cx, cy - int(u * 0.6)), int(u * 0.22))
        return True
    if chave == "heli":
        pygame.draw.ellipse(s, (60, 90, 50), (cx - int(u * 0.6), cy - int(u * 1.0), int(u * 1.2), int(u * 1.8)))
        pygame.draw.rect(s, (60, 90, 50), (cx - 3, cy + int(u * 0.5), 6, int(u * 1.3)))
        pygame.draw.circle(s, (120, 200, 230), (cx, cy - int(u * 0.5)), int(u * 0.3))
        pygame.draw.line(s, (40, 40, 40), (cx - int(u * 1.7), cy - int(u * 0.2)), (cx + int(u * 1.7), cy + int(u * 0.2)), 3)
        pygame.draw.line(s, (40, 40, 40), (cx - int(u * 0.2), cy - int(u * 1.7)), (cx + int(u * 0.2), cy + int(u * 1.3)), 3)
        return True
    if chave == "morteiro":
        _macaco_base(s, cx - int(u * 0.5), cy + int(u * 0.3), int(u * 0.6), PELO, ROSTO)
        pygame.draw.circle(s, (70, 90, 60), (cx + int(u * 0.5), cy - int(u * 0.2)), int(u * 0.7))
        pygame.draw.circle(s, (30, 30, 30), (cx + int(u * 0.5), cy - int(u * 0.2)), int(u * 0.45))
        return True
    if chave == "dartling":
        _macaco_base(s, cx, cy + int(u * 0.5), int(u * 0.65), PELO, ROSTO)
        pygame.draw.rect(s, (70, 80, 90), (cx - int(u * 0.5), cy - int(u * 1.6), int(u * 1.0), int(u * 1.5)), border_radius=4)
        for k in (-1, 0, 1):
            pygame.draw.line(s, (30, 30, 30), (cx + k * 6, cy - int(u * 1.6)), (cx + k * 6, cy - int(u * 2.1)), 3)
        return True
    if chave == "fazenda":
        pygame.draw.rect(s, (120, 80, 40), (cx - int(u * 1.3), cy - int(u * 0.2), int(u * 2.6), int(u * 1.3)))
        pygame.draw.polygon(s, (190, 60, 40), [(cx - int(u * 1.5), cy - int(u * 0.2)), (cx + int(u * 1.5), cy - int(u * 0.2)), (cx, cy - int(u * 1.3))])
        for k in range(3):
            x = cx - int(u * 0.9) + k * int(u * 0.9)
            pygame.draw.arc(s, (250, 220, 50), (x - 6, cy + int(u * 0.1), 12, 18), 0.5, 2.6, 4)
        pygame.draw.circle(s, (60, 150, 50), (cx + int(u * 1.1), cy - int(u * 1.0)), int(u * 0.5))
        return True
    if chave == "espinhos":
        pygame.draw.rect(s, (90, 90, 100), (cx - int(u * 1.1), cy - int(u * 1.1), int(u * 2.2), int(u * 2.2)), border_radius=6)
        pygame.draw.rect(s, (150, 150, 160), (cx - u, cy - u, 2 * u, 2 * u), border_radius=6)
        pygame.draw.circle(s, (70, 70, 80), (cx, cy), int(u * 0.6))
        for k in range(8):
            a = k * math.pi / 4
            pygame.draw.line(s, (40, 40, 40), (cx, cy), (cx + math.cos(a) * u * 0.55, cy + math.sin(a) * u * 0.55), 2)
        return True
    if chave == "vila":
        pygame.draw.circle(s, (120, 80, 40), (cx, cy), int(u * 1.35))
        pygame.draw.polygon(s, (200, 170, 90), [(cx + math.cos(k * math.pi / 3) * u * 1.3, cy + math.sin(k * math.pi / 3) * u * 1.3) for k in range(6)])
        pygame.draw.circle(s, (170, 130, 60), (cx, cy), int(u * 0.5))
        pygame.draw.circle(s, (220, 60, 50), (cx, cy - int(u * 0.2)), int(u * 0.2))
        return True
    if chave == "sentinela":
        pygame.draw.circle(s, (60, 60, 70), (cx, cy), int(u * 0.9))
        pygame.draw.circle(s, (220, 180, 40), (cx, cy), int(u * 0.7))
        pygame.draw.rect(s, (60, 60, 70), (cx - 3, cy - int(u * 1.4), 6, int(u * 1.2)))
        return True
    if chave == "churchill":
        pygame.draw.rect(s, (40, 50, 40), (cx - int(u * 1.3), cy - int(u * 1.2), int(u * 2.6), int(u * 2.4)), border_radius=8)
        pygame.draw.rect(s, (90, 110, 80), (cx - int(u * 1.1), cy - u, int(u * 2.2), 2 * u), border_radius=8)
        pygame.draw.rect(s, (60, 70, 55), (cx - 4, cy - int(u * 2.0), 8, int(u * 1.6)))
        _macaco_base(s, cx, cy, int(u * 0.55), (110, 80, 50), ROSTO)
        return True
    return False


def desenhar_retrato(s: pygame.Surface, chave: str, cx: int, cy: int, u: int) -> None:
    """Desenha o macaco/torre na superficie s, olhando para cima."""
    if _desenhar_torre_especial(s, cx, cy, u, chave, None):
        return
    if chave in HEROI_ESTILO:
        pelo, rosto, chapeu, item, cor = HEROI_ESTILO[chave]
        if chave == "pat":
            u = int(u * 1.25)
    else:
        pelo, rosto, chapeu, item, cor = ESTILO.get(chave, (PELO, ROSTO, None, None, None))
    _macaco_base(s, cx, cy, u, pelo, rosto)
    _chapeu(s, cx, cy, u, chapeu, cor)
    _item(s, cx, cy, u, item, cor)


COR_TIER = {3: (205, 127, 50), 4: (200, 205, 215), 5: (255, 215, 60)}


@lru_cache(maxsize=512)
def sprite_torre(chave: str, tamanho: int, tier: int = 0) -> pygame.Surface:
    """Sprite quadrado (tamanho x tamanho) com transparencia, olhando para cima."""
    s = pygame.Surface((tamanho, tamanho), pygame.SRCALPHA)
    c = tamanho // 2
    u = max(4, int(tamanho * 0.2))
    if tier >= 3:
        cor = COR_TIER[min(tier, 5)]
        pygame.draw.circle(s, (*cor, 110), (c, c), int(tamanho * 0.47))
        pygame.draw.circle(s, (*cor, 255), (c, c), int(tamanho * 0.47), 2)
    desenhar_retrato(s, chave, c, c, u)
    return s


@lru_cache(maxsize=4096)
def sprite_torre_rot(chave: str, tamanho: int, tier: int, ang10: int) -> pygame.Surface:
    base = sprite_torre(chave, tamanho, tier)
    return pygame.transform.rotate(base, -(ang10 * 10) - 90)


# ======================================================================= BLOONS
def _desenhar_bloon(s, cx, cy, r, tipo, camo, regen, fort, dano):
    t = TIPOS[tipo]
    cor = t.cor
    w, h = int(r * 1.75), int(r * 2.1)
    rect = pygame.Rect(cx - w // 2, cy - h // 2, w, h)
    # no
    pygame.draw.polygon(s, _sombra(cor, 0.6), [(cx - 3, rect.bottom - 2), (cx + 3, rect.bottom - 2), (cx, rect.bottom + 4)])
    pygame.draw.ellipse(s, _sombra(cor, 0.45) if tipo != "preto" else (70, 70, 80), rect.inflate(3, 3))
    if tipo == "arco_iris":
        faixas = [(230, 40, 40), (250, 140, 20), (250, 220, 30), (60, 190, 60), (40, 140, 235), (150, 60, 200)]
        base = pygame.Surface((w, h), pygame.SRCALPHA)
        for i, fc in enumerate(faixas):
            pygame.draw.rect(base, fc, (0, i * h // 6, w, h // 6 + 1))
        mask = pygame.Surface((w, h), pygame.SRCALPHA)
        pygame.draw.ellipse(mask, (255, 255, 255, 255), (0, 0, w, h))
        base.blit(mask, (0, 0), special_flags=pygame.BLEND_RGBA_MIN)
        s.blit(base, rect.topleft)
    else:
        pygame.draw.ellipse(s, cor, rect)
    if tipo == "zebra":
        clip = s.get_clip()
        s.set_clip(rect)
        for k in range(-2, 4):
            pygame.draw.line(s, (25, 25, 30), (rect.left, rect.top + k * h // 3), (rect.right, rect.top + k * h // 3 + h // 3), max(3, r // 4))
        s.set_clip(clip)
    if tipo == "chumbo":
        pygame.draw.ellipse(s, (170, 175, 185), rect.inflate(-w // 3, -h // 3).move(-w // 8, -h // 8))
    if tipo == "ceramica":
        pygame.draw.ellipse(s, (205, 140, 80), rect.inflate(-w // 2.5, -h // 2.5))
        for k in range(dano):
            a = 0.8 + k * 1.7
            pygame.draw.line(s, (80, 40, 15), (cx, cy), (cx + math.cos(a) * r, cy + math.sin(a) * r), 2)
    # brilho
    if tipo not in ("preto",):
        pygame.draw.ellipse(s, (255, 255, 255, 150), (cx - w // 3, cy - h // 3, max(3, w // 4), max(4, h // 3)))
    else:
        pygame.draw.ellipse(s, (120, 120, 130), (cx - w // 3, cy - h // 3, max(3, w // 4), max(4, h // 3)))
    if fort:
        pygame.draw.ellipse(s, (120, 120, 130), rect, 3)
        pygame.draw.line(s, (120, 120, 130), (rect.left + 2, cy), (rect.right - 2, cy), 3)
        pygame.draw.line(s, (120, 120, 130), (cx, rect.top + 2), (cx, rect.bottom - 2), 3)
    if camo:
        rng = random.Random(hash(tipo) & 0xffff)
        clip = s.get_clip()
        s.set_clip(rect)
        for _ in range(7):
            x = rng.randint(rect.left, rect.right)
            y = rng.randint(rect.top, rect.bottom)
            pygame.draw.ellipse(s, rng.choice([(60, 90, 40), (110, 130, 60), (80, 60, 30)]), (x - 5, y - 4, 10 + rng.randint(0, 6), 8))
        s.set_clip(clip)
    if regen:
        for k in range(10):
            a = k * math.pi / 5
            pygame.draw.circle(s, (255, 110, 170), (int(cx + math.cos(a) * w * 0.55), int(cy + math.sin(a) * h * 0.5)), 2)


@lru_cache(maxsize=1024)
def sprite_bloon(tipo: str, camo: bool, regen: bool, fort: bool, dano: int = 0) -> pygame.Surface:
    t = TIPOS[tipo]
    r = int(t.raio)
    tam = r * 3 + 8
    s = pygame.Surface((tam, tam), pygame.SRCALPHA)
    _desenhar_bloon(s, tam // 2, tam // 2, r, tipo, camo, regen, fort, dano)
    return s


def _desenhar_dirigivel(tipo: str, fort: bool, dano: int) -> pygame.Surface:
    t = TIPOS[tipo]
    comp = int(t.raio * 2.6)
    alt = int(t.raio * 1.35)
    s = pygame.Surface((comp + 30, alt + 30), pygame.SRCALPHA)
    ox, oy = 15, 15
    cor = t.cor
    escuro = _sombra(cor, 0.55)
    # aletas
    for sy in (-1, 1):
        pygame.draw.polygon(s, escuro, [(ox + 6, oy + alt // 2), (ox - 8, oy + alt // 2 + sy * alt // 1.6), (ox + comp // 4, oy + alt // 2)])
    pygame.draw.ellipse(s, escuro, (ox - 2, oy - 2, comp + 4, alt + 4))
    pygame.draw.ellipse(s, cor, (ox, oy, comp, alt))
    faixa = (240, 240, 245) if tipo in ("moab", "bfb", "bad") else (30, 30, 30)
    pygame.draw.rect(s, faixa, (ox + comp // 5, oy + alt // 2 - alt // 10, comp * 3 // 5, alt // 5))
    pygame.draw.ellipse(s, _clarear(cor, 0.4), (ox + comp // 6, oy + alt // 8, comp // 2, alt // 5))
    # "olho" na frente
    pygame.draw.circle(s, (250, 250, 250), (ox + comp - comp // 6, oy + alt // 2), max(4, alt // 7))
    pygame.draw.circle(s, (20, 20, 20), (ox + comp - comp // 6 + 2, oy + alt // 2), max(2, alt // 14))
    if tipo == "ddt":
        rng = random.Random(7)
        for _ in range(10):
            pygame.draw.circle(s, (70, 80, 60), (ox + rng.randint(10, comp - 10), oy + rng.randint(6, alt - 6)), rng.randint(3, 7))
    if fort:
        for k in range(1, 5):
            x = ox + k * comp // 5
            pygame.draw.line(s, (150, 150, 160), (x, oy + 4), (x, oy + alt - 4), 3)
    for k in range(dano):
        x0 = ox + comp // 3 + k * comp // 7
        pygame.draw.lines(s, (20, 20, 20), False, [(x0, oy + 6), (x0 + 6, oy + alt // 3), (x0 - 4, oy + alt // 2), (x0 + 5, oy + alt - 8)], 2)
    return s


@lru_cache(maxsize=2048)
def sprite_dirigivel(tipo: str, fort: bool, dano: int, ang8: int) -> pygame.Surface:
    base = _desenhar_dirigivel(tipo, fort, dano)
    return pygame.transform.rotate(base, -ang8 * 8)


# ======================================================================= MAPA
def _ruido(s, rng, cor, n, raio):
    for _ in range(n):
        x, y = rng.randint(0, s.get_width()), rng.randint(0, s.get_height())
        r = rng.randint(raio // 2, raio)
        pygame.draw.circle(s, cor, (x, y), r)


@lru_cache(maxsize=16)
def fundo_mapa(chave: str) -> pygame.Surface:
    m = MAPAS[chave]
    s = pygame.Surface((LARGURA_MAPA, ALTURA_MAPA))
    s.fill(m.grama)
    rng = random.Random(len(chave) * 31 + 7)
    _ruido(s, rng, _sombra(m.grama, 0.93), 220, 34)
    _ruido(s, rng, _clarear(m.grama, 0.08), 180, 26)
    for _ in range(900):
        x, y = rng.randint(0, LARGURA_MAPA), rng.randint(0, ALTURA_MAPA)
        pygame.draw.line(s, _sombra(m.grama, 0.75), (x, y), (x + rng.randint(-3, 3), y - rng.randint(4, 8)), 2)
    # agua
    for cx, cy, r in m.agua:
        pygame.draw.circle(s, (70, 140, 60), (cx, cy), r + 10)
        pygame.draw.circle(s, (230, 215, 160), (cx, cy), r + 5)
        pygame.draw.circle(s, (40, 130, 210), (cx, cy), r)
        pygame.draw.circle(s, (70, 160, 230), (cx, cy), int(r * 0.8))
        for k in range(8):
            a = k * 0.8
            px, py = cx + math.cos(a) * r * 0.5, cy + math.sin(a) * r * 0.5
            pygame.draw.arc(s, (150, 200, 245), (px - 14, py - 5, 28, 10), 0.3, 2.8, 2)
    for rx, ry, rw, rh in m.agua_ret:
        pygame.draw.rect(s, (230, 215, 160), (rx - 5, ry - 5, rw + 10, rh + 10), border_radius=18)
        pygame.draw.rect(s, (40, 130, 210), (rx, ry, rw, rh), border_radius=14)
        pygame.draw.rect(s, (70, 160, 230), (rx + 10, ry + 10, rw - 20, rh - 20), border_radius=12)
    # trilha
    for cam in m.caminhos:
        pts = [(int(x), int(y)) for x, y in cam.pontos]
        for larg, cor in ((LARGURA_TRILHA + 10, _sombra(m.terra, 0.65)),
                          (LARGURA_TRILHA, m.terra),
                          (LARGURA_TRILHA - 18, _clarear(m.terra, 0.12))):
            pygame.draw.lines(s, cor, False, pts, larg)
            for p in pts:
                pygame.draw.circle(s, cor, p, larg // 2)
        for d, x, y in cam.amostras[::3]:
            if rng.random() < 0.5:
                pygame.draw.circle(s, _sombra(m.terra, 0.8), (int(x + rng.randint(-14, 14)), int(y + rng.randint(-14, 14))), rng.randint(1, 3))
    # obstaculos
    for ox, oy, r, tipo in m.obstaculos:
        if tipo == "arvore":
            pygame.draw.circle(s, (40, 70, 30), (ox + 6, oy + 8), r)
            for k in range(6):
                a = k * math.pi / 3
                pygame.draw.circle(s, (46, 110, 40), (int(ox + math.cos(a) * r * 0.45), int(oy + math.sin(a) * r * 0.45)), int(r * 0.6))
            pygame.draw.circle(s, (60, 135, 50), (ox - 4, oy - 4), int(r * 0.55))
            pygame.draw.circle(s, (90, 165, 70), (ox - 10, oy - 10), int(r * 0.25))
        else:
            pts = [(ox + math.cos(k * 1.1) * r * (0.8 + 0.2 * (k % 2)), oy + math.sin(k * 1.1) * r * 0.75) for k in range(6)]
            pygame.draw.polygon(s, (90, 90, 95), [(x + 4, y + 5) for x, y in pts])
            pygame.draw.polygon(s, (150, 150, 155), pts)
            pygame.draw.polygon(s, (185, 185, 190), [(x * 0.6 + ox * 0.4 - 4, y * 0.6 + oy * 0.4 - 4) for x, y in pts])
    return s


@lru_cache(maxsize=16)
def miniatura_mapa(chave: str, w: int, h: int) -> pygame.Surface:
    return pygame.transform.smoothscale(fundo_mapa(chave), (w, h))


# ======================================================================= ICONES
@lru_cache(maxsize=8)
def icone_coracao(tam: int) -> pygame.Surface:
    s = pygame.Surface((tam, tam), pygame.SRCALPHA)
    r = tam // 4
    for cor, enc in (((120, 10, 20), 0), ((230, 40, 60), 2)):
        pygame.draw.circle(s, cor, (tam // 2 - r + 1, tam // 3 + 1), r + 1 - enc)
        pygame.draw.circle(s, cor, (tam // 2 + r - 1, tam // 3 + 1), r + 1 - enc)
        pygame.draw.polygon(s, cor, [(2 + enc, tam // 3 + 2), (tam - 2 - enc, tam // 3 + 2), (tam // 2, tam - 3 - enc)])
    pygame.draw.circle(s, (255, 170, 180), (tam // 2 - r, tam // 3 - 2), max(2, r // 3))
    return s


@lru_cache(maxsize=8)
def icone_moeda(tam: int) -> pygame.Surface:
    s = pygame.Surface((tam, tam), pygame.SRCALPHA)
    c = tam // 2
    pygame.draw.circle(s, (160, 110, 10), (c, c), c - 1)
    pygame.draw.circle(s, (250, 200, 40), (c, c), c - 3)
    pygame.draw.circle(s, (255, 230, 120), (c, c), c - 7, 2)
    f = pygame.font.SysFont("arialblack", int(tam * 0.55))
    t = f.render("$", True, (170, 120, 10))
    s.blit(t, t.get_rect(center=(c, c + 1)))
    return s


@lru_cache(maxsize=4)
def icone_eco(tam: int) -> pygame.Surface:
    s = pygame.Surface((tam, tam), pygame.SRCALPHA)
    c = tam // 2
    pygame.draw.circle(s, (20, 110, 40), (c, c), c - 1)
    pygame.draw.circle(s, (60, 190, 80), (c, c), c - 3)
    pygame.draw.polygon(s, (240, 255, 240), [(c, 4), (tam - 6, c), (c + 3, c), (c + 3, tam - 4), (c - 3, tam - 4), (c - 3, c), (6, c)])
    return s


# ======================================================================= PROJETEIS
COR_PROJ = {
    "dardo": (70, 70, 80), "flecha": (110, 70, 30), "prego": (120, 120, 130),
    "tachinha": (90, 90, 100), "lamina": (210, 210, 220), "bala": (240, 210, 80),
    "magia": (190, 90, 255), "fogo": (255, 140, 30), "laser": (255, 50, 50),
    "plasma": (255, 90, 220), "sol": (255, 230, 80), "escuro": (60, 20, 90),
    "shuriken": (180, 180, 190), "pocao": (130, 220, 80), "espinho": (80, 140, 40),
    "cola": (150, 210, 60), "gelo_bola": (170, 230, 255), "uva": (130, 50, 150),
    "uva_fogo": (255, 110, 40), "bala_canhao": (40, 40, 45), "bomba": (30, 30, 35),
    "missil": (200, 200, 205), "abacaxi": (230, 200, 40), "meteoro": (255, 100, 20),
    "tornado": (220, 230, 240), "balista": (120, 80, 40), "drone": (90, 110, 140),
    "espirito": (120, 255, 200), "luz": (255, 250, 180), "maldicao": (150, 40, 150),
    "juggernaut": (110, 110, 115), "bola_espinho": (120, 120, 125), "glaive": (230, 230, 240),
    "kylie": (240, 200, 40), "bumerangue": (240, 200, 40), "fragmento": (60, 60, 60),
    "fragmento_gelo": (190, 240, 255), "aviaozinho": (230, 200, 40), "flash": (255, 255, 255),
}


def desenhar_projetil(tela, p, ox=0, oy=0, escala=1.0) -> None:
    v = p.at["visual"]
    x, y = ox + p.x * escala, oy + p.y * escala
    cor = COR_PROJ.get(v, (60, 60, 60))
    r = max(2, int(p.raio * escala))
    if escala < 0.5:
        pygame.draw.circle(tela, cor, (int(x), int(y)), max(1, r // 2))
        return
    a = math.radians(p.ang)
    ca, sa = math.cos(a), math.sin(a)
    if v in ("dardo", "flecha", "prego", "tachinha", "bala", "fragmento", "laser", "espinho", "fragmento_gelo"):
        comp = {"laser": 18, "bala": 8, "tachinha": 9, "fragmento": 7}.get(v, 13)
        larg = 3 if v != "laser" else 4
        pygame.draw.line(tela, cor, (x - ca * comp, y - sa * comp), (x + ca * 4, y + sa * 4), larg)
        if v in ("dardo", "flecha"):
            pygame.draw.line(tela, (220, 60, 50), (x - ca * comp, y - sa * comp), (x - ca * (comp - 4), y - sa * (comp - 4)), 5)
    elif v in ("bumerangue", "kylie", "glaive", "shuriken"):
        t = pygame.time.get_ticks() / 60.0
        pts = []
        braços = 4 if v == "shuriken" else 3 if v == "glaive" else 2
        for k in range(braços):
            ang = t + k * 2 * math.pi / braços
            pts.append((x + math.cos(ang) * (r + 6), y + math.sin(ang) * (r + 6)))
        for px, py in pts:
            pygame.draw.line(tela, cor, (x, y), (px, py), 4)
        pygame.draw.circle(tela, _sombra(cor), (int(x), int(y)), 3)
    elif v == "missil":
        pygame.draw.line(tela, cor, (x - ca * 12, y - sa * 12), (x + ca * 6, y + sa * 6), 5)
        pygame.draw.circle(tela, (255, 160, 40), (int(x - ca * 14), int(y - sa * 14)), 4)
    elif v in ("juggernaut", "bola_espinho"):
        pygame.draw.circle(tela, (70, 70, 75), (int(x), int(y)), r + 2)
        pygame.draw.circle(tela, cor, (int(x), int(y)), r)
        for k in range(8):
            ang = k * math.pi / 4
            pygame.draw.line(tela, (60, 60, 60), (x + math.cos(ang) * r, y + math.sin(ang) * r), (x + math.cos(ang) * (r + 5), y + math.sin(ang) * (r + 5)), 3)
    elif v == "tornado":
        for k in range(4):
            pygame.draw.ellipse(tela, cor, (x - r + k * 2, y - r + k * 5, 2 * r - k * 4, 8), 2)
    else:
        pygame.draw.circle(tela, _sombra(cor, 0.6), (int(x), int(y)), r + 1)
        pygame.draw.circle(tela, cor, (int(x), int(y)), r)
        if v in ("fogo", "magia", "plasma", "sol", "luz", "espirito", "meteoro"):
            pygame.draw.circle(tela, _clarear(cor, 0.6), (int(x), int(y)), max(1, r // 2))
        if v in ("bomba", "bala_canhao") and p.fusivel > 0:
            pygame.draw.circle(tela, (255, 80, 40), (int(x), int(y)), r + 3, 2)


COR_PILHA = {
    "espinhos": (120, 120, 130), "bola_espinho": (110, 110, 115), "estrepe": (80, 80, 90),
    "chamas": (255, 120, 30), "acido": (140, 220, 60), "cipo": (50, 140, 50),
    "zumbi": (120, 160, 120), "espuma": (230, 240, 255), "armadilha": (90, 70, 50),
    "espinheiro": (60, 110, 40),
}


def desenhar_pilha(tela, s, ox=0, oy=0, escala=1.0) -> None:
    x, y = int(ox + s.x * escala), int(oy + s.y * escala)
    cor = COR_PILHA.get(s.visual, (120, 120, 130))
    if escala < 0.5:
        pygame.draw.circle(tela, cor, (x, y), 2)
        return
    if s.visual in ("chamas", "acido", "espuma"):
        pygame.draw.circle(tela, _sombra(cor, 0.7), (x, y), 12)
        pygame.draw.circle(tela, cor, (x, y), 9)
    elif s.visual == "armadilha":
        pygame.draw.rect(tela, cor, (x - 12, y - 12, 24, 24), border_radius=4)
        pygame.draw.rect(tela, (40, 40, 40), (x - 12, y - 12, 24, 24), 2, border_radius=4)
    else:
        for k in range(5):
            a = k * 1.25
            px, py = x + math.cos(a) * 6, y + math.sin(a) * 6
            pygame.draw.polygon(tela, cor, [(px, py - 5), (px + 4, py + 3), (px - 4, py + 3)])
            pygame.draw.polygon(tela, _sombra(cor, 0.6), [(px, py - 5), (px + 4, py + 3), (px - 4, py + 3)], 1)
