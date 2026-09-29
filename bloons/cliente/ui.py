"""Componentes de interface no estilo Bloons: texto com contorno, botoes e paineis."""

from __future__ import annotations

from functools import lru_cache

import pygame

LARGURA, ALTURA = 1280, 720

AMARELO = (255, 214, 50)
VERDE = (96, 196, 60)
VERDE_ESCURO = (40, 110, 30)
VERMELHO = (225, 60, 50)
AZUL = (60, 150, 230)
MARROM = (122, 78, 40)
MARROM_ESCURO = (72, 44, 20)
BEGE = (238, 214, 160)
BRANCO = (255, 255, 255)
PRETO = (15, 15, 20)
CINZA = (130, 130, 140)
DINHEIRO = (255, 222, 70)


@lru_cache(maxsize=32)
def fonte(tamanho: int, peso: str = "arialblack") -> pygame.font.Font:
    return pygame.font.SysFont(peso, tamanho)


@lru_cache(maxsize=2048)
def texto_surf(txt: str, tam: int, cor=BRANCO, contorno: int = 2, cor_contorno=PRETO,
               peso: str = "arialblack") -> pygame.Surface:
    f = fonte(tam, peso)
    base = f.render(txt, True, cor)
    if contorno <= 0:
        return base
    borda = f.render(txt, True, cor_contorno)
    w, h = base.get_width() + contorno * 2, base.get_height() + contorno * 2
    s = pygame.Surface((w, h), pygame.SRCALPHA)
    for dx in range(-contorno, contorno + 1):
        for dy in range(-contorno, contorno + 1):
            if dx * dx + dy * dy <= contorno * contorno + 1:
                s.blit(borda, (dx + contorno, dy + contorno))
    s.blit(base, (contorno, contorno))
    return s


def texto(tela, txt, pos, tam=20, cor=BRANCO, contorno=2, ancora="topleft", peso="arialblack",
          cor_contorno=PRETO) -> pygame.Rect:
    s = texto_surf(str(txt), tam, cor, contorno, cor_contorno, peso)
    r = s.get_rect(**{ancora: pos})
    tela.blit(s, r)
    return r


def quebrar(txt: str, tam: int, largura: int, peso="arialblack") -> list[str]:
    f = fonte(tam, peso)
    linhas, atual = [], ""
    for palavra in txt.split():
        teste = (atual + " " + palavra).strip()
        if f.size(teste)[0] <= largura:
            atual = teste
        else:
            if atual:
                linhas.append(atual)
            atual = palavra
    if atual:
        linhas.append(atual)
    return linhas


def painel(tela, rect, cor=(86, 150, 50), borda=MARROM, raio=14, espessura=5, sombra=True):
    rect = pygame.Rect(rect)
    if sombra:
        s = pygame.Surface((rect.w, rect.h), pygame.SRCALPHA)
        pygame.draw.rect(s, (0, 0, 0, 90), s.get_rect(), border_radius=raio)
        tela.blit(s, rect.move(4, 5))
    pygame.draw.rect(tela, borda, rect, border_radius=raio)
    interno = rect.inflate(-espessura * 2, -espessura * 2)
    pygame.draw.rect(tela, cor, interno, border_radius=max(2, raio - espessura))
    # brilho no topo
    brilho = pygame.Rect(interno.x + 4, interno.y + 3, interno.w - 8, max(4, interno.h // 6))
    s = pygame.Surface(brilho.size, pygame.SRCALPHA)
    pygame.draw.rect(s, (255, 255, 255, 40), s.get_rect(), border_radius=max(2, raio - espessura))
    tela.blit(s, brilho)
    return interno


def painel_madeira(tela, rect, raio=10):
    """Painel lateral de madeira, como a loja de torres."""
    rect = pygame.Rect(rect)
    pygame.draw.rect(tela, MARROM_ESCURO, rect, border_radius=raio)
    interno = rect.inflate(-8, -8)
    pygame.draw.rect(tela, (150, 100, 55), interno, border_radius=raio)
    for k in range(interno.y + 6, interno.bottom, 22):
        pygame.draw.line(tela, (135, 88, 45), (interno.x + 4, k), (interno.right - 4, k), 2)
    return interno


class Botao:
    def __init__(self, rect, rotulo: str, cor=VERDE, tam=24, habilitado=True, dica=""):
        self.rect = pygame.Rect(rect)
        self.rotulo = rotulo
        self.cor = cor
        self.tam = tam
        self.habilitado = habilitado
        self.dica = dica

    def desenhar(self, tela, mouse=None) -> None:
        mouse = mouse or pygame.mouse.get_pos()
        cor = self.cor if self.habilitado else CINZA
        sobre = self.habilitado and self.rect.collidepoint(mouse)
        r = self.rect.move(0, -2) if sobre else self.rect
        pygame.draw.rect(tela, (0, 0, 0), r.move(0, 4), border_radius=12)
        pygame.draw.rect(tela, tuple(max(0, c - 70) for c in cor), r, border_radius=12)
        interno = r.inflate(-6, -6)
        pygame.draw.rect(tela, tuple(min(255, c + (25 if sobre else 0)) for c in cor), interno,
                         border_radius=10)
        s = pygame.Surface((interno.w - 8, interno.h // 2 - 2), pygame.SRCALPHA)
        pygame.draw.rect(s, (255, 255, 255, 55), s.get_rect(), border_radius=8)
        tela.blit(s, (interno.x + 4, interno.y + 3))
        texto(tela, self.rotulo, r.center, self.tam, ancora="center")

    def clicou(self, evento) -> bool:
        return (self.habilitado and evento.type == pygame.MOUSEBUTTONDOWN and evento.button == 1
                and self.rect.collidepoint(evento.pos))


class CampoTexto:
    def __init__(self, rect, valor="", rotulo="", max_len=40):
        self.rect = pygame.Rect(rect)
        self.valor = valor
        self.rotulo = rotulo
        self.ativo = False
        self.max_len = max_len

    def evento(self, e) -> None:
        if e.type == pygame.MOUSEBUTTONDOWN and e.button == 1:
            self.ativo = self.rect.collidepoint(e.pos)
        elif e.type == pygame.KEYDOWN and self.ativo:
            if e.key == pygame.K_BACKSPACE:
                self.valor = self.valor[:-1]
            elif e.key in (pygame.K_RETURN, pygame.K_TAB):
                self.ativo = False
            elif e.unicode and e.unicode.isprintable() and len(self.valor) < self.max_len:
                self.valor += e.unicode

    def desenhar(self, tela) -> None:
        if self.rotulo:
            texto(tela, self.rotulo, (self.rect.x, self.rect.y - 30), 18)
        pygame.draw.rect(tela, MARROM_ESCURO, self.rect, border_radius=8)
        pygame.draw.rect(tela, BEGE if self.ativo else (220, 200, 150), self.rect.inflate(-6, -6),
                         border_radius=6)
        cursor = "|" if self.ativo and pygame.time.get_ticks() // 500 % 2 else ""
        texto(tela, self.valor + cursor, (self.rect.x + 12, self.rect.centery), 20, PRETO, 0,
              ancora="midleft", peso="arial")


def barra(tela, rect, frac, cor=VERDE, fundo=(40, 40, 40)):
    rect = pygame.Rect(rect)
    pygame.draw.rect(tela, PRETO, rect.inflate(4, 4), border_radius=6)
    pygame.draw.rect(tela, fundo, rect, border_radius=5)
    if frac > 0:
        r = rect.copy()
        r.w = max(4, int(rect.w * min(1.0, frac)))
        pygame.draw.rect(tela, cor, r, border_radius=5)


def fundo_gradiente(tela, cima=(90, 180, 240), baixo=(170, 225, 255)):
    h = tela.get_height()
    for y in range(0, h, 4):
        t = y / h
        cor = tuple(int(cima[i] + (baixo[i] - cima[i]) * t) for i in range(3))
        pygame.draw.rect(tela, cor, (0, y, tela.get_width(), 4))


def formatar(n: float) -> str:
    n = int(n)
    return f"{n:,}".replace(",", ".")


def clarear(cor, f=0.25):
    return tuple(min(255, int(c + (255 - c) * f)) for c in cor[:3])
