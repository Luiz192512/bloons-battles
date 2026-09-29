"""Desenho de uma pista (mapa, torres, bloons, projeteis e efeitos)."""

from __future__ import annotations

import math
import random

import pygame

from bloons.cliente import arte, som
from bloons.cliente.ui import texto, formatar
from bloons.jogo.mapas import ALTURA_MAPA, LARGURA_MAPA

COR_EFEITO = {
    "congelar": (170, 225, 255), "anel_fogo": (255, 130, 30), "radiacao": (120, 255, 90),
    "impacto": (150, 110, 70), "espadas": (230, 230, 240), "vento": (240, 250, 255),
    "orbita_glaive": (220, 220, 235), "relampago": (140, 220, 255), "raio_plasma": (255, 90, 220),
    "bala": (255, 240, 150), "psi": (200, 120, 255),
}


_SOMBRAS: dict[int, pygame.Surface] = {}


def _sombra(tam: int) -> pygame.Surface:
    s = _SOMBRAS.get(tam)
    if s is None:
        s = pygame.Surface((tam, tam // 2), pygame.SRCALPHA)
        pygame.draw.ellipse(s, (0, 0, 0, 70), s.get_rect())
        _SOMBRAS[tam] = s
    return s


class Efeito:
    __slots__ = ("tipo", "dados", "t", "dur")

    def __init__(self, tipo, dados, dur):
        self.tipo, self.dados, self.t, self.dur = tipo, dados, 0.0, dur


class RenderPista:
    def __init__(self, pista, chave_mapa: str, sons: bool = True):
        self.pista = pista
        self.chave_mapa = chave_mapa
        self.efeitos: list[Efeito] = []
        self.avisos: list[tuple] = []
        self.sons = sons
        self.flash = None
        self.rng = random.Random(1)

    # ---------------------------------------------------------- eventos da simulacao
    def consumir_eventos(self) -> None:
        ev = self.pista.eventos
        if not ev:
            return
        pops = 0
        for e in ev:
            tipo = e[0]
            if tipo == "pop":
                pops += 1
                if pops <= 40:
                    self.efeitos.append(Efeito("pop", (e[1], e[2]), 0.12))
            elif tipo == "explosao":
                self.efeitos.append(Efeito("explosao", (e[1], e[2], e[3], e[4]), 0.28))
                if self.sons:
                    som.tocar("explosao", 90)
            elif tipo == "raio":
                self.efeitos.append(Efeito("raio", e[1:], 0.09))
            elif tipo == "aura":
                self.efeitos.append(Efeito("aura", (e[1], e[2], e[3], e[4]), 0.3))
            elif tipo == "dinheiro":
                self.efeitos.append(Efeito("texto", (e[1], e[2], f"+${formatar(e[3])}", (120, 255, 90)), 1.2))
            elif tipo == "nivel":
                self.efeitos.append(Efeito("texto", (e[1], e[2], f"NÍVEL {e[3]}!", (255, 220, 60)), 1.6))
                if self.sons:
                    som.tocar("upgrade")
            elif tipo == "habilidade":
                self.efeitos.append(Efeito("texto", (e[1], e[2], e[3], (255, 255, 255)), 1.4))
                if self.sons:
                    som.tocar("habilidade")
            elif tipo == "flash":
                self.flash = [e[3], 0.0]
            elif tipo == "vazou":
                self.efeitos.append(Efeito("vazou", (e[1], e[2], e[3]), 0.5))
                if self.sons:
                    som.tocar("vazou", 200)
            elif tipo in ("colocar", "upgrade", "venda", "invocar"):
                self.efeitos.append(Efeito("anel", (e[1], e[2]), 0.35))
                if self.sons:
                    som.tocar({"colocar": "colocar", "upgrade": "upgrade", "venda": "venda",
                               "invocar": "colocar"}[tipo])
            elif tipo == "regen":
                self.efeitos.append(Efeito("regen", (e[1], e[2]), 0.3))
            elif tipo in ("fim_rodada", "eco", "envio"):
                self.avisos.append(e)
        if pops and self.sons:
            som.tocar("pop", 45)
        ev.clear()
        if len(self.efeitos) > 500:
            self.efeitos = self.efeitos[-500:]

    def atualizar(self, dt: float) -> None:
        for f in self.efeitos:
            f.t += dt
        self.efeitos = [f for f in self.efeitos if f.t < f.dur]
        if self.flash:
            self.flash[1] += dt
            if self.flash[1] > 0.35:
                self.flash = None

    # ---------------------------------------------------------- desenho completo
    def desenhar(self, tela, selecionada=None) -> None:
        p = self.pista
        tela.blit(arte.fundo_mapa(self.chave_mapa), (0, 0))
        for s in p.pilhas:
            arte.desenhar_pilha(tela, s)
        self._desenhar_bloons(tela)
        for t in sorted(p.torres.values(), key=lambda t: (t.dfn.mov != "fixo", t.y)):
            self._desenhar_torre(tela, t, t.id == selecionada)
        for pr in p.projeteis:
            arte.desenhar_projetil(tela, pr)
        self._desenhar_efeitos(tela)

    def _desenhar_torre(self, tela, t, sel: bool) -> None:
        tam = int(t.dfn.raio * 2.7) if not t.dfn.heroi else 58
        if t.temporaria:
            tam = int(tam * 0.8)
        tier = max(t.caminhos) if not t.dfn.heroi else (3 if t.nivel >= 10 else 0) + (2 if t.nivel >= 20 else 0)
        ang10 = int(round(t.ang / 10.0)) % 36
        spr = arte.sprite_torre_rot(t.chave, tam, tier, ang10)
        sombra = _sombra(tam)
        if t.dfn.mov == "fixo":
            tela.blit(sombra, (t.x - tam / 2, t.y + tam * 0.1))
        else:
            tela.blit(sombra, (t.x - tam / 2 + 14, t.y + 22))
        tela.blit(spr, spr.get_rect(center=(int(t.x), int(t.y))))
        if sel:
            pygame.draw.circle(tela, (255, 255, 255), (int(t.x), int(t.y)), tam // 2 + 4, 2)
        if t.dfn.heroi:
            r = texto(tela, str(t.nivel), (int(t.x) + tam // 3, int(t.y) + tam // 3), 14,
                      (255, 230, 90), ancora="center")
        if t.turbo < 1.0:
            pygame.draw.circle(tela, (255, 200, 60), (int(t.x), int(t.y)), tam // 2 + 2, 2)

    def _desenhar_bloons(self, tela) -> None:
        bloons = self.pista.bloons
        # dirigiveis por baixo, bloons pequenos por cima (desenhados de tras para frente)
        for b in sorted(bloons, key=lambda b: (not b.tipo.moab, b.d)):
            if not b.vivo:
                continue
            x, y = int(b.x), int(b.y)
            if b.tipo.moab:
                frac = b.vida / max(1, b.vida_max)
                dano = 0 if frac > 0.75 else 1 if frac > 0.5 else 2 if frac > 0.25 else 3
                spr = arte.sprite_dirigivel(b.tipo.nome, b.fort, dano, int(round(b.ang / 8.0)) % 45)
            else:
                dano = 0
                if b.tipo.nome == "ceramica":
                    frac = b.vida / max(1, b.vida_max)
                    dano = 0 if frac > 0.7 else 1 if frac > 0.4 else 2
                spr = arte.sprite_bloon(b.tipo.nome, b.camo, b.regen, b.fort, dano)
            tela.blit(spr, spr.get_rect(center=(x, y)))
            r = int(b.tipo.raio)
            if b.cong_t > 0:
                s = pygame.Surface((r * 3, r * 3), pygame.SRCALPHA)
                pygame.draw.circle(s, (190, 235, 255, 150), (r * 3 // 2, r * 3 // 2), int(r * 1.2))
                pygame.draw.circle(s, (240, 250, 255, 220), (r * 3 // 2, r * 3 // 2), int(r * 1.2), 2)
                tela.blit(s, (x - r * 3 // 2, y - r * 3 // 2))
            if b.cola_t > 0:
                pygame.draw.circle(tela, (150, 210, 60), (x + r // 3, y - r // 3), max(3, r // 2))
            if b.queima_t > 0:
                pygame.draw.circle(tela, (255, 140, 30), (x - r // 2, y + r // 3), max(2, r // 3))
                pygame.draw.circle(tela, (255, 220, 80), (x - r // 2, y + r // 3 - 2), max(1, r // 5))
            if b.atord_t > 0:
                for k in range(3):
                    a = pygame.time.get_ticks() / 150.0 + k * 2.1
                    pygame.draw.circle(tela, (255, 240, 90), (int(x + math.cos(a) * r), int(y - r - 4 + math.sin(a) * 3)), 3)

    def _desenhar_efeitos(self, tela) -> None:
        for f in self.efeitos:
            k = f.t / f.dur
            d = f.dados
            if f.tipo == "pop":
                x, y = d
                r = 10 + 6 * k
                pts = []
                for i in range(12):
                    a = i * math.pi / 6
                    rr = r if i % 2 == 0 else r * 0.55
                    pts.append((x + math.cos(a) * rr, y + math.sin(a) * rr))
                pygame.draw.polygon(tela, (255, 255, 255), pts)
                pygame.draw.polygon(tela, (30, 30, 30), pts, 2)
            elif f.tipo == "explosao":
                x, y, raio, _ = d
                s = pygame.Surface((int(raio * 2.4), int(raio * 2.4)), pygame.SRCALPHA)
                c = s.get_width() // 2
                a = int(220 * (1 - k))
                pygame.draw.circle(s, (255, 150, 40, a), (c, c), int(raio * (0.6 + 0.5 * k)))
                pygame.draw.circle(s, (255, 230, 120, a), (c, c), int(raio * 0.45 * (1 - k * 0.5)))
                tela.blit(s, (x - c, y - c))
            elif f.tipo == "raio":
                x1, y1, x2, y2 = d[:4]
                vis = d[4] if len(d) > 4 else "bala"
                cor = COR_EFEITO.get(vis, (255, 240, 150))
                if vis == "relampago":
                    pts = [(x1, y1)]
                    for i in range(1, 6):
                        t = i / 6
                        pts.append((x1 + (x2 - x1) * t + self.rng.uniform(-8, 8),
                                    y1 + (y2 - y1) * t + self.rng.uniform(-8, 8)))
                    pts.append((x2, y2))
                    pygame.draw.lines(tela, cor, False, pts, 3)
                    pygame.draw.lines(tela, (255, 255, 255), False, pts, 1)
                else:
                    larg = 6 if vis == "raio_plasma" else 2
                    pygame.draw.line(tela, cor, (x1, y1), (x2, y2), larg)
            elif f.tipo == "aura":
                x, y, raio, vis = d
                cor = COR_EFEITO.get(vis, (255, 255, 255))
                s = pygame.Surface((int(raio * 2 + 8), int(raio * 2 + 8)), pygame.SRCALPHA)
                c = s.get_width() // 2
                a = int(140 * (1 - k))
                if vis == "congelar":
                    pygame.draw.circle(s, (*cor, a // 2), (c, c), int(raio * (0.3 + 0.7 * k)))
                pygame.draw.circle(s, (*cor, a + 60 if a else 0), (c, c), int(raio * (0.3 + 0.7 * k)), 4)
                tela.blit(s, (x - c, y - c))
            elif f.tipo == "texto":
                x, y, txt, cor = d
                texto(tela, txt, (int(x), int(y - 30 * k - 20)), 18, cor, ancora="center")
            elif f.tipo == "anel":
                x, y = d
                pygame.draw.circle(tela, (255, 255, 255), (int(x), int(y)), int(10 + 30 * k), 3)
            elif f.tipo == "regen":
                x, y = d
                pygame.draw.circle(tela, (255, 120, 180), (int(x), int(y)), int(8 + 10 * k), 2)
            elif f.tipo == "vazou":
                x, y, perda = d
                texto(tela, f"-{perda}", (int(x), int(y - 30 * k)), 18, (255, 80, 80), ancora="center")
        if self.flash:
            cor, t = self.flash
            s = pygame.Surface((LARGURA_MAPA, ALTURA_MAPA), pygame.SRCALPHA)
            s.fill((*cor, int(150 * (1 - t / 0.35))))
            tela.blit(s, (0, 0))

    # ---------------------------------------------------------- miniatura (oponente)
    def desenhar_mini(self, tela, rect: pygame.Rect) -> None:
        esc = rect.w / LARGURA_MAPA
        tela.blit(arte.miniatura_mapa(self.chave_mapa, rect.w, rect.h), rect.topleft)
        p = self.pista
        for s in p.pilhas:
            arte.desenhar_pilha(tela, s, rect.x, rect.y, esc)
        for t in p.torres.values():
            tam = max(10, int(t.dfn.raio * 2.7 * esc * 1.3))
            spr = arte.sprite_torre(t.chave, tam, max(t.caminhos) if not t.dfn.heroi else 0)
            tela.blit(spr, spr.get_rect(center=(rect.x + int(t.x * esc), rect.y + int(t.y * esc))))
        for b in p.bloons:
            if not b.vivo:
                continue
            r = max(2, int(b.tipo.raio * esc * 1.2))
            cor = b.tipo.cor
            pygame.draw.circle(tela, (0, 0, 0), (rect.x + int(b.x * esc), rect.y + int(b.y * esc)), r + 1)
            pygame.draw.circle(tela, cor, (rect.x + int(b.x * esc), rect.y + int(b.y * esc)), r)
        for pr in p.projeteis[:200]:
            arte.desenhar_projetil(tela, pr, rect.x, rect.y, esc)
