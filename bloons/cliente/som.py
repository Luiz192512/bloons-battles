"""Efeitos sonoros sintetizados na hora (sem arquivos de audio)."""

from __future__ import annotations

import array
import math
import random

import pygame

TAXA = 22050
_sons: dict[str, pygame.mixer.Sound] = {}
_ultimo: dict[str, int] = {}
ativo = True


def _gerar(nome: str, dur: float, fn) -> None:
    n = int(TAXA * dur)
    buf = array.array("h", (int(max(-1.0, min(1.0, fn(i / TAXA, i / n))) * 12000) for i in range(n)))
    _sons[nome] = pygame.mixer.Sound(buffer=buf.tobytes())


def iniciar() -> None:
    global ativo
    try:
        pygame.mixer.init(TAXA, -16, 1, 512)
    except pygame.error:
        ativo = False
        return
    rng = random.Random(3)
    _gerar("pop", 0.07, lambda t, f: (rng.random() * 2 - 1) * (1 - f) ** 3
           + 0.6 * math.sin(2 * math.pi * 900 * t) * (1 - f) ** 4)
    _gerar("colocar", 0.18, lambda t, f: math.sin(2 * math.pi * (300 + 500 * f) * t) * (1 - f))
    _gerar("upgrade", 0.35, lambda t, f: (math.sin(2 * math.pi * 520 * t) + math.sin(2 * math.pi * 780 * t)
                                          + math.sin(2 * math.pi * 1040 * t * (1 + f))) / 3 * (1 - f))
    _gerar("venda", 0.25, lambda t, f: math.sin(2 * math.pi * (900 - 500 * f) * t) * (1 - f))
    _gerar("erro", 0.2, lambda t, f: (1 if math.sin(2 * math.pi * 140 * t) > 0 else -1) * 0.5 * (1 - f))
    _gerar("explosao", 0.3, lambda t, f: (rng.random() * 2 - 1) * (1 - f) ** 2)
    _gerar("rodada", 0.5, lambda t, f: math.sin(2 * math.pi * (440 if f < 0.5 else 660) * t) * (1 - f) * 0.8)
    _gerar("vazou", 0.3, lambda t, f: math.sin(2 * math.pi * (200 - 120 * f) * t) * (1 - f))
    _gerar("habilidade", 0.5, lambda t, f: math.sin(2 * math.pi * (300 + 900 * f) * t) * (1 - f) * 0.8)
    _sons["pop"].set_volume(0.35)
    _sons["explosao"].set_volume(0.4)


def tocar(nome: str, intervalo_ms: int = 0) -> None:
    if not ativo or nome not in _sons:
        return
    agora = pygame.time.get_ticks()
    if intervalo_ms and agora - _ultimo.get(nome, -99999) < intervalo_ms:
        return
    _ultimo[nome] = agora
    _sons[nome].play()
