"""Tela de jogo: mapa, loja de torres, painel de upgrades, habilidades e modo Batalha."""

from __future__ import annotations

import math

import pygame

from bloons.cliente import arte, som
from bloons.cliente import ui
from bloons.cliente.controle import MENSAGEM_ERRO
from bloons.cliente.render import RenderPista
from bloons.cliente.ui import (AMARELO, BRANCO, DINHEIRO, PRETO, VERDE, VERMELHO, Botao,
                               clarear, formatar, texto)
from bloons.jogo import sim as S
from bloons.jogo.bloons_def import TIPOS
from bloons.jogo.herois_def import XP_NIVEL
from bloons.jogo.mapas import LARGURA_MAPA
from bloons.jogo.rodadas import DIFICULDADES, ENVIOS
from bloons.jogo.stats import definicao
from bloons.jogo.torres_def import ORDEM_TORRES, TORRES

PAINEL_X = LARGURA_MAPA
PAINEL_W = ui.LARGURA - LARGURA_MAPA
CARD_W, CARD_H = 74, 64
GRADE_Y = 66
NOMES_MODO = ["Primeiro", "Último", "Perto", "Forte"]
TECLAS_UP = {pygame.K_COMMA: 0, pygame.K_PERIOD: 1, pygame.K_SLASH: 2}
ENVIO_H = 104


class CenaJogo:
    def __init__(self, app, controle):
        self.app = app
        self.ctl = controle
        self.partida = controle.partida
        self.chave_mapa = self.partida.mapa.chave
        self.render = RenderPista(controle.pista, self.chave_mapa)
        self.render_op = None
        if controle.online:
            self.render_op = RenderPista(controle.oponente, self.chave_mapa, sons=False)
        self.selecionada: int | None = None
        self.colocando: str | None = None
        self.msg = ("", 0.0)
        self.banner = ("", 0.0)
        self.menu_pausa = False
        self.mostrar_log = controle.online
        self.mostrar_op = True
        self.op_grande = False
        self.dicas: list = []
        self.heroi = controle.pista.heroi_escolhido
        self.cards = self._montar_cards()
        self._botoes_up: list = []
        self._botoes_hab: list = []
        self._botoes_envio: list = []
        self._botoes_menu: list = []
        self._botao_play = pygame.Rect(PAINEL_X + 70, ui.ALTURA - 116, 100, 100)
        self._rect_mini = pygame.Rect(LARGURA_MAPA - 270, 8, 262, 181)
        self.avisos_op: list = []
        self.eco_texto = (0, 0.0)

    # ============================================================== utilidades
    @property
    def pista(self) -> S.Pista:
        return self.ctl.pista

    def _montar_cards(self):
        chaves = ([self.heroi] if self.heroi else []) + ORDEM_TORRES
        cards = []
        for i, chave in enumerate(chaves):
            col, lin = i % 3, i // 3
            r = pygame.Rect(PAINEL_X + 8 + col * (CARD_W + 2), GRADE_Y + lin * (CARD_H + 2), CARD_W, CARD_H)
            cards.append((chave, r))
        return cards

    def aviso(self, txt: str, erro: bool = True) -> None:
        self.msg = (txt, 1.8)
        if erro:
            som.tocar("erro", 150)

    def comando(self, cmd: str) -> bool:
        erro = self.ctl.enviar(cmd)
        if erro:
            self.aviso(MENSAGEM_ERRO.get(erro, "Não foi possível."))
            return False
        return True

    def _hab_lista(self):
        out = []
        for t in self.pista.torres.values():
            for i, h in enumerate(t.st.habs):
                out.append((t, i, h))
        return out[:9]

    # ============================================================== eventos
    def evento(self, e) -> None:
        if self.ctl.terminou or self.menu_pausa:
            if self.menu_pausa and e.type == pygame.KEYDOWN and e.key == pygame.K_ESCAPE:
                self._pausar(False)
                return
            for b, acao in self._botoes_menu:
                if b.clicou(e):
                    acao()
                    return
            return
        if e.type == pygame.KEYDOWN:
            self._tecla(e)
        elif e.type == pygame.MOUSEBUTTONDOWN:
            if e.button == 3:
                self.colocando = None
                self.selecionada = None
            elif e.button == 1:
                self._clique(e.pos)

    def _pausar(self, v: bool) -> None:
        self.menu_pausa = v
        if not self.ctl.online:
            self.ctl.pausado = v

    def _tecla(self, e) -> None:
        k = e.key
        if k == pygame.K_ESCAPE:
            if self.colocando or self.selecionada or self.op_grande:
                self.colocando = None
                self.selecionada = None
                self.op_grande = False
            else:
                self._pausar(True)
            return
        if k == pygame.K_F1:
            self.mostrar_log = not self.mostrar_log
            return
        if k == pygame.K_o and self.ctl.online:
            self.op_grande = not self.op_grande
            return
        if k == pygame.K_SPACE:
            self.ctl.botao_play()
            return
        if k in TECLAS_UP and self.selecionada:
            self._upar(TECLAS_UP[k])
            return
        if k in (pygame.K_BACKSPACE, pygame.K_DELETE) and self.selecionada:
            self._vender()
            return
        if k == pygame.K_TAB and self.selecionada:
            t = self.pista.torres.get(self.selecionada)
            if t:
                self.comando(f"M{t.id}:{(t.modo + 1) % 4}")
            return
        if pygame.K_1 <= k <= pygame.K_9:
            habs = self._hab_lista()
            i = k - pygame.K_1
            if i < len(habs):
                t, idx, _ = habs[i]
                self.comando(f"B{t.id}:{idx}")
            return
        nome = pygame.key.name(k)
        if nome == "u" and self.heroi:
            self._escolher(self.heroi)
            return
        for chave in ORDEM_TORRES:
            if TORRES[chave].tecla == nome:
                self._escolher(chave)
                return

    def _escolher(self, chave: str) -> None:
        dfn = definicao(chave)
        if dfn.heroi and self.pista.tem_heroi:
            self.aviso("Você já colocou seu herói.")
            return
        if self.pista.dinheiro < self.pista.custo(dfn.custo):
            self.aviso("Dinheiro insuficiente!")
            return
        self.colocando = None if self.colocando == chave else chave
        self.selecionada = None

    def _clique(self, pos) -> None:
        for lista in (self._botoes_up, self._botoes_hab, self._botoes_envio):
            for r, acao in lista:
                if r.collidepoint(pos):
                    acao()
                    return
        if self.op_grande:
            self.op_grande = False
            return
        if (self.ctl.online and self.mostrar_op and self._rect_mini.collidepoint(pos)
                and not self.colocando):
            self.op_grande = True
            return
        if pos[0] >= PAINEL_X:
            if self._botao_play.collidepoint(pos) and not self.ctl.online:
                self.ctl.botao_play()
                som.tocar("rodada")
                return
            for chave, r in self.cards:
                if r.collidepoint(pos):
                    self._escolher(chave)
                    return
            return
        x, y = pos
        if self.colocando:
            chave = self.colocando
            if self.comando(f"T{chave}@{int(x)},{int(y)}"):
                if not (pygame.key.get_mods() & pygame.KMOD_SHIFT) or definicao(chave).heroi:
                    self.colocando = None
            return
        melhor, md = None, 1e9
        for t in self.pista.torres.values():
            d = (t.x - x) ** 2 + (t.y - y) ** 2
            if d < (t.dfn.raio + 10) ** 2 and d < md:
                melhor, md = t, d
        self.selecionada = melhor.id if melhor else None

    def _upar(self, p: int) -> None:
        t = self.pista.torres.get(self.selecionada)
        if t is None or t.dfn.heroi or t.temporaria:
            return
        if self.pista.custo_upgrade(t, p) is None:
            self.aviso("Caminho bloqueado ou no máximo.")
            return
        self.comando(f"U{t.id}:{p}")

    def _vender(self) -> None:
        t = self.pista.torres.get(self.selecionada)
        if t is None or t.temporaria:
            return
        if self.comando(f"V{t.id}"):
            self.selecionada = None

    # ============================================================== atualizar
    def atualizar(self, dt: float) -> None:
        self.ctl.atualizar(dt)
        self.render.consumir_eventos()
        self.render.atualizar(dt)
        for av in self.render.avisos:
            if av[0] == "fim_rodada":
                if self.ctl.online:
                    self.banner = (f"Rodada {av[3]}", 1.6)
                    som.tocar("rodada")
                else:
                    self.banner = (f"Rodada {av[3]} completa!", 2.0)
            elif av[0] == "eco":
                self.eco_texto = (av[3], 1.2)
        self.render.avisos.clear()
        if self.render_op:
            self.render_op.consumir_eventos()
            self.render_op.atualizar(dt)
            for av in self.render_op.avisos:
                if av[0] == "envio":
                    self.avisos_op.append([f"Oponente enviou {av[3]}!", 2.5])
            self.render_op.avisos.clear()
        for a in self.avisos_op:
            a[1] -= dt
        self.avisos_op = [a for a in self.avisos_op if a[1] > 0][-4:]
        self.msg = (self.msg[0], self.msg[1] - dt)
        self.banner = (self.banner[0], self.banner[1] - dt)
        self.eco_texto = (self.eco_texto[0], self.eco_texto[1] - dt)
        if self.selecionada and self.selecionada not in self.pista.torres:
            self.selecionada = None
        if self.ctl.status:
            self.msg = (self.ctl.status, 3.0)
            self.ctl.status = ""

    # ============================================================== desenho
    def desenhar(self, tela) -> None:
        mouse = pygame.mouse.get_pos()
        self._botoes_up, self._botoes_hab, self._botoes_envio, self._botoes_menu = [], [], [], []
        self.dicas = []
        sel = self.pista.torres.get(self.selecionada) if self.selecionada else None
        self.render.desenhar(tela, self.selecionada)
        if sel is not None:
            self._circulo_alcance(tela, sel.x, sel.y, sel.alcance, True)
        if self.colocando and mouse[0] < PAINEL_X:
            self._previa(tela, mouse)
        self._hud_topo(tela)
        if self.ctl.online:
            self._painel_envios(tela, mouse)
            if self.mostrar_op:
                self._mini_oponente(tela)
        self._habilidades(tela, mouse)
        self._painel_lateral(tela, mouse)
        if sel is not None:
            self._painel_upgrade(tela, sel, mouse)
        if self.mostrar_log:
            self._painel_log(tela)
        if self.op_grande and self.render_op:
            self._oponente_grande(tela)
        self._mensagens(tela)
        for d in self.dicas[:1]:
            self._dica(tela, *d)
        if self.ctl.terminou:
            self._tela_fim(tela)
        elif self.menu_pausa:
            self._tela_pausa(tela)

    def _circulo_alcance(self, tela, x, y, r, valido):
        if r >= 5000:
            return
        r = int(r)
        s = pygame.Surface((r * 2 + 4, r * 2 + 4), pygame.SRCALPHA)
        cor = (0, 0, 0, 55) if valido else (255, 0, 0, 70)
        pygame.draw.circle(s, cor, (r + 2, r + 2), r)
        pygame.draw.circle(s, (255, 255, 255, 170) if valido else (255, 60, 60, 200), (r + 2, r + 2), r, 2)
        tela.blit(s, (x - r - 2, y - r - 2))

    def _previa(self, tela, mouse):
        dfn = definicao(self.colocando)
        x, y = mouse
        valido = (self.pista.posicao_valida(dfn, x, y)
                  and self.pista.dinheiro >= self.pista.custo(dfn.custo, x, y))
        self._circulo_alcance(tela, x, y, dfn.alcance if dfn.alcance < 5000 else 60, valido)
        tam = int(dfn.raio * 2.7) if not dfn.heroi else 58
        spr = arte.sprite_torre(self.colocando, tam, 0).copy()
        spr.set_alpha(210)
        tela.blit(spr, spr.get_rect(center=mouse))

    # ---------------------------------------------------------------- HUD
    def _hud_topo(self, tela):
        p = self.pista
        tela.blit(arte.icone_coracao(34), (10, 8))
        texto(tela, formatar(max(0, p.vidas)), (48, 25), 26, ancora="midleft")
        tela.blit(arte.icone_moeda(32), (10, 48))
        texto(tela, "$" + formatar(p.dinheiro), (48, 64), 26, DINHEIRO, ancora="midleft")
        if self.ctl.online:
            tela.blit(arte.icone_eco(28), (12, 88))
            texto(tela, f"+{formatar(p.eco)}", (48, 102), 20, (140, 255, 120), ancora="midleft")
            frac = 1.0 - self.partida.tempo_para_eco() / S.ECO_INTERVALO
            ui.barra(tela, (130, 96, 90, 12), frac, (120, 230, 90))
            if self.eco_texto[1] > 0:
                texto(tela, f"+${formatar(self.eco_texto[0])}", (230, 102), 18, (140, 255, 120), ancora="midleft")

    def _painel_lateral(self, tela, mouse):
        ui.painel_madeira(tela, (PAINEL_X, 0, PAINEL_W, ui.ALTURA), raio=0)
        p = self.partida
        cx = PAINEL_X + PAINEL_W // 2
        if self.ctl.online:
            texto(tela, f"Rodada {max(1, p.rodada)}", (cx, 20), 24, ancora="center")
            texto(tela, f"próxima em {int(p.tempo_para_rodada())}s", (cx, 46), 15, ancora="center")
        else:
            texto(tela, f"Rodada {max(1, p.rodada)}/{p.ultima_rodada}", (cx, 22), 24, ancora="center")
            texto(tela, DIFICULDADES[p.dificuldade][0], (cx, 48), 15, AMARELO, ancora="center")
        pista = self.pista
        for chave, r in self.cards:
            dfn = definicao(chave)
            custo = pista.custo(dfn.custo)
            pode = pista.dinheiro >= custo and not (dfn.heroi and pista.tem_heroi)
            sobre = r.collidepoint(mouse)
            cor = (236, 214, 150) if pode else (170, 150, 120)
            if dfn.heroi:
                cor = (255, 226, 120) if pode else (180, 160, 110)
            if self.colocando == chave:
                cor = (140, 230, 110)
            if sobre:
                cor = clarear(cor, 0.3)
            pygame.draw.rect(tela, (70, 42, 18), r.move(0, 3), border_radius=9)
            pygame.draw.rect(tela, cor, r, border_radius=9)
            pygame.draw.rect(tela, (90, 60, 30), r, 2, border_radius=9)
            spr = arte.sprite_torre(chave, 50, 0)
            tela.blit(spr, spr.get_rect(center=(r.centerx, r.y + 26)))
            if dfn.heroi and pista.tem_heroi:
                texto(tela, "EM JOGO", (r.centerx, r.bottom - 10), 11, (200, 255, 200), ancora="center")
            else:
                texto(tela, f"${formatar(custo)}", (r.centerx, r.bottom - 10), 13,
                      DINHEIRO if pode else (255, 110, 100), ancora="center")
            tecla = "U" if dfn.heroi else dfn.tecla.upper()
            texto(tela, tecla, (r.x + 5, r.y + 2), 10, BRANCO, 1)
            if sobre:
                self.dicas.append((mouse, dfn.nome, dfn.titulo if dfn.heroi else dfn.desc))
        if self.ctl.online:
            y = ui.ALTURA - 112
            texto(tela, "ECONOMIA", (cx, y), 18, ancora="center")
            texto(tela, f"+${formatar(pista.eco)} a cada {int(S.ECO_INTERVALO)}s", (cx, y + 26), 15,
                  (150, 255, 130), ancora="center")
            op = self.ctl.oponente
            texto(tela, f"Oponente: {max(0, op.vidas)} vidas", (cx, y + 52), 13, ancora="center")
            texto(tela, "O: ver oponente   F1: mensagens", (cx, y + 76), 11, (230, 230, 230), 1, ancora="center")
            texto(tela, "Esc: menu", (cx, y + 94), 11, (230, 230, 230), 1, ancora="center")
            return
        r = self._botao_play
        rapido = self.ctl.velocidade > 1
        cor = (255, 170, 40) if (p.em_rodada and rapido) else (80, 200, 60)
        pygame.draw.circle(tela, (20, 60, 15), (r.centerx, r.centery + 4), 46)
        pygame.draw.circle(tela, (30, 90, 20), r.center, 46)
        pygame.draw.circle(tela, cor, r.center, 40)
        pygame.draw.circle(tela, clarear(cor, 0.3), (r.centerx, r.centery - 10), 26)
        pygame.draw.circle(tela, cor, (r.centerx, r.centery + 4), 30)
        cxp, cyp = r.center
        if not p.em_rodada:
            pts = [(cxp - 12, cyp - 20), (cxp - 12, cyp + 20), (cxp + 20, cyp)]
            pygame.draw.polygon(tela, BRANCO, pts)
            pygame.draw.polygon(tela, PRETO, pts, 3)
        else:
            for dx in (-16, 4):
                pts = [(cxp + dx, cyp - 16), (cxp + dx, cyp + 16), (cxp + dx + 18, cyp)]
                pygame.draw.polygon(tela, BRANCO, pts)
                pygame.draw.polygon(tela, PRETO, pts, 3)
        texto(tela, "Espaço", (r.centerx, r.bottom + 6), 12, ancora="center")
        ra = pygame.Rect(PAINEL_X + 176, ui.ALTURA - 60, 58, 24)
        pygame.draw.rect(tela, (60, 40, 20), ra, border_radius=6)
        texto(tela, "AUTO: " + ("ON" if p.auto else "OFF"), ra.center, 10,
              (160, 255, 140) if p.auto else (230, 230, 230), 1, ancora="center")
        self._botoes_up.append((ra, self._alternar_auto))
        if r.collidepoint(mouse):
            self.dicas.append((mouse, "Iniciar rodada" if not p.em_rodada else "Acelerar",
                               "Começa a próxima rodada. Durante a rodada, alterna velocidade 1x/3x."))

    def _dica(self, tela, pos, titulo, desc, largura=250):
        linhas = ui.quebrar(desc or "", 14, largura - 20, peso="arial") if desc else []
        h = 34 + 18 * len(linhas)
        x = min(pos[0] + 16, ui.LARGURA - largura - 4)
        if pos[0] >= PAINEL_X:
            x = PAINEL_X - largura - 8
        y = max(4, min(pos[1] + 10, ui.ALTURA - h - 4))
        r = pygame.Rect(x, y, largura, h)
        pygame.draw.rect(tela, (30, 20, 10), r, border_radius=8)
        pygame.draw.rect(tela, (250, 236, 200), r.inflate(-4, -4), border_radius=7)
        texto(tela, titulo, (r.x + 10, r.y + 6), 15, (60, 30, 10), 0)
        for i, l in enumerate(linhas):
            texto(tela, l, (r.x + 10, r.y + 28 + i * 18), 14, (40, 30, 20), 0, peso="arial")

    # ---------------------------------------------------------------- upgrades
    def _painel_upgrade(self, tela, t, mouse):
        w = 300
        h = 500 if not t.dfn.heroi else 360
        x = 8 if t.x > LARGURA_MAPA / 2 else LARGURA_MAPA - w - 8
        y = 118
        if self.ctl.online and x > LARGURA_MAPA / 2:
            y = 200
        h = min(h, ui.ALTURA - y - (ENVIO_H + 6 if self.ctl.online else 8))
        ui.painel(tela, (x, y, w, h), cor=(86, 150, 50), borda=(60, 40, 20))
        p = self.pista
        nome = t.dfn.nome
        texto(tela, nome, (x + w // 2, y + 20), 18 if len(nome) < 20 else 15, ancora="center")
        spr = arte.sprite_torre(t.chave, 64, max(t.caminhos) if not t.dfn.heroi else 0)
        tela.blit(spr, (x + 12, y + 34))
        texto(tela, f"Estouros: {formatar(t.pops)}", (x + 84, y + 42), 14)
        rm = pygame.Rect(x + 84, y + 66, 196, 30)
        pygame.draw.rect(tela, (40, 80, 30), rm, border_radius=8)
        texto(tela, f"<  {NOMES_MODO[t.modo]}  >", rm.center, 15, ancora="center")
        self._botoes_up.append((rm, lambda: self.comando(f"M{t.id}:{(t.modo + 1) % 4}")))
        if rm.collidepoint(mouse):
            self.dicas.append((mouse, "Prioridade de alvo", "Clique ou Tab para trocar."))
        yy = y + 106
        if t.dfn.heroi:
            self._painel_heroi(tela, t, x, yy, w)
        elif t.temporaria:
            texto(tela, f"Temporária: {int(t.temporaria)}s", (x + w // 2, yy + 20), 16, ancora="center")
        else:
            for pth in range(3):
                if yy + pth * 96 + 90 < y + h - 56:
                    self._linha_upgrade(tela, t, pth, x + 10, yy + pth * 96, w - 20, mouse)
        if not t.temporaria:
            rv = pygame.Rect(x + 20, y + h - 52, w - 40, 40)
            b = Botao(rv, f"Vender  ${formatar(p.valor_venda(t))}", cor=(230, 120, 40), tam=18)
            b.desenhar(tela, mouse)
            self._botoes_up.append((rv, self._vender))

    def _linha_upgrade(self, tela, t, pth, x, y, w, mouse):
        p = self.pista
        tier = t.caminhos[pth]
        for k in range(5):
            r = pygame.Rect(x + k * 14, y + 4, 11, 11)
            pygame.draw.rect(tela, (30, 60, 20), r, border_radius=3)
            if k < tier:
                pygame.draw.rect(tela, (140, 255, 90), r.inflate(-2, -2), border_radius=3)
        if tier > 0:
            atual = t.dfn.caminhos[pth][tier - 1].nome
            if len(atual) > 24:
                atual = atual[:23] + "."
            texto(tela, atual, (x + 76, y + 10), 12, (230, 255, 220), 1, ancora="midleft")
        card = pygame.Rect(x, y + 20, w, 70)
        if tier >= 5:
            pygame.draw.rect(tela, (200, 160, 40), card, border_radius=10)
            texto(tela, "MÁXIMO", card.center, 20, ancora="center")
            return
        up = t.dfn.caminhos[pth][tier]
        custo = p.custo_upgrade(t, pth)
        if custo is None:
            pygame.draw.rect(tela, (70, 70, 70), card, border_radius=10)
            texto(tela, "Caminho fechado", (card.centerx, card.y + 22), 14, (220, 220, 220), ancora="center")
            texto(tela, up.nome, (card.centerx, card.y + 46), 13, (180, 180, 180), 1, ancora="center")
            return
        pode = p.dinheiro >= custo
        sobre = card.collidepoint(mouse)
        cor = (60, 160, 230) if pode else (150, 70, 60)
        if sobre:
            cor = clarear(cor, 0.2)
        pygame.draw.rect(tela, (20, 30, 50), card.move(0, 3), border_radius=10)
        pygame.draw.rect(tela, cor, card, border_radius=10)
        for i, l in enumerate(ui.quebrar(up.nome, 15, w - 24)[:2]):
            texto(tela, l, (card.centerx, card.y + 16 + i * 20), 15, ancora="center")
        texto(tela, f"${formatar(custo)}", (card.centerx, card.bottom - 12), 16,
              DINHEIRO if pode else (255, 140, 130), ancora="center")
        texto(tela, ",./"[pth], (card.right - 12, card.y + 6), 11, BRANCO, 1)
        self._botoes_up.append((card, lambda: self._upar(pth)))
        if sobre:
            self.dicas.append((mouse, up.nome, up.desc or "Melhora a torre."))

    def _painel_heroi(self, tela, t, x, y, w):
        texto(tela, f"Nível {t.nivel}", (x + w // 2, y + 12), 24, AMARELO, ancora="center")
        if t.nivel < 20:
            a, b = XP_NIVEL[t.nivel], XP_NIVEL[t.nivel + 1]
            ui.barra(tela, (x + 30, y + 40, w - 60, 16), (t.xp - a) / max(1, b - a), (120, 200, 255))
            texto(tela, f"XP {int(t.xp)}/{b}", (x + w // 2, y + 48), 12, ancora="center")
        dfn = t.dfn
        yy = y + 76
        texto(tela, dfn.titulo, (x + w // 2, yy), 13, (230, 255, 220), 1, ancora="center")
        for i, (nivel, h) in enumerate(((3, dfn.hab3), (10, dfn.hab10))):
            if h:
                ok = t.nivel >= nivel
                texto(tela, f"Nv {nivel}: {h['nome']}", (x + 20, yy + 26 + i * 26), 14,
                      BRANCO if ok else (170, 190, 170), 1)
        texto(tela, "Sobe de nível estourando bloons.", (x + w // 2, yy + 92), 11, (230, 230, 230), 1,
              ancora="center")

    # ---------------------------------------------------------------- habilidades
    def _habilidades(self, tela, mouse):
        habs = self._hab_lista()
        if not habs:
            return
        base_y = ui.ALTURA - (ENVIO_H + 40 if self.ctl.online else 40)
        for i, (t, idx, h) in enumerate(habs):
            cx, cy = 34 + i * 60, base_y
            rec = t.hab_rec[idx]
            pronto = rec <= 0
            pygame.draw.circle(tela, (30, 30, 30), (cx, cy + 3), 27)
            pygame.draw.circle(tela, (250, 220, 90) if pronto else (120, 120, 120), (cx, cy), 27)
            pygame.draw.circle(tela, (60, 120, 200), (cx, cy), 23)
            spr = arte.sprite_torre(t.chave, 40, 0)
            tela.blit(spr, spr.get_rect(center=(cx, cy)))
            if not pronto:
                frac = min(1.0, rec / h["recarga"])
                s = pygame.Surface((54, 54), pygame.SRCALPHA)
                pts = [(27, 27)] + [(27 + math.cos(-math.pi / 2 + a / 30 * 2 * math.pi * frac) * 26,
                                     27 + math.sin(-math.pi / 2 + a / 30 * 2 * math.pi * frac) * 26)
                                    for a in range(31)]
                pygame.draw.polygon(s, (0, 0, 0, 150), pts)
                tela.blit(s, (cx - 27, cy - 27))
                texto(tela, str(int(rec) + 1), (cx, cy), 16, ancora="center")
            texto(tela, str(i + 1), (cx + 18, cy + 18), 12, AMARELO, ancora="center")
            r = pygame.Rect(cx - 27, cy - 27, 54, 54)
            self._botoes_hab.append((r, lambda t=t, idx=idx: self.comando(f"B{t.id}:{idx}")))
            if r.collidepoint(mouse):
                self.dicas.append((mouse, h["nome"], f"{t.dfn.nome} (recarga {int(h['recarga'])}s)"))

    # ---------------------------------------------------------------- batalha
    def _painel_envios(self, tela, mouse):
        r = pygame.Rect(0, ui.ALTURA - ENVIO_H, LARGURA_MAPA, ENVIO_H)
        s = pygame.Surface(r.size, pygame.SRCALPHA)
        s.fill((50, 30, 12, 215))
        tela.blit(s, r)
        pygame.draw.line(tela, (30, 18, 6), r.topleft, r.topright, 4)
        p = self.pista
        rodada = self.partida.rodada
        bw, bh = 84, 45
        for i, env in enumerate(ENVIOS):
            col, lin = i % 12, i // 12
            b = pygame.Rect(4 + col * (bw + 2), r.y + 6 + lin * (bh + 4), bw, bh)
            bloqueado = rodada < env.rodada_min
            pode = not bloqueado and p.dinheiro >= env.custo
            cor = (90, 170, 60) if pode else (110, 90, 70) if not bloqueado else (60, 50, 40)
            if b.collidepoint(mouse) and not bloqueado:
                cor = clarear(cor, 0.2)
            pygame.draw.rect(tela, (20, 12, 4), b.move(0, 2), border_radius=7)
            pygame.draw.rect(tela, cor, b, border_radius=7)
            tela.blit(_icone_envio(env), (b.x + 2, b.y + 4))
            if env.qtd > 1:
                texto(tela, f"x{env.qtd}", (b.x + 28, b.y + 30), 11, BRANCO, 1)
            texto(tela, f"${formatar(env.custo)}", (b.right - 4, b.y + 12), 11,
                  DINHEIRO if pode else (255, 150, 140), 1, ancora="midright")
            eco = f"{env.eco:+g}" if env.eco else "0"
            texto(tela, eco, (b.right - 4, b.y + 32), 11,
                  (150, 255, 130) if env.eco > 0 else (255, 150, 140), 1, ancora="midright")
            if bloqueado:
                s2 = pygame.Surface(b.size, pygame.SRCALPHA)
                s2.fill((0, 0, 0, 120))
                tela.blit(s2, b)
                texto(tela, f"R{env.rodada_min}", b.center, 16, (220, 220, 220), ancora="center")
            else:
                self._botoes_envio.append((b, lambda env=env: self._enviar(env)))
            if b.collidepoint(mouse):
                self.dicas.append((mouse, env.nome, f"Custo ${env.custo}. Renda {eco} por ciclo de eco. "
                                                    f"Libera na rodada {env.rodada_min}."))

    def _enviar(self, env):
        if self.comando(f"S{env.chave}"):
            som.tocar("colocar", 60)

    def _mini_oponente(self, tela):
        r = self._rect_mini
        pygame.draw.rect(tela, (40, 25, 10), r.inflate(8, 30).move(0, 11), border_radius=8)
        self.render_op.desenhar_mini(tela, r)
        op = self.ctl.oponente
        texto(tela, f"OPONENTE   {max(0, op.vidas)} vidas", (r.x + 6, r.bottom + 4), 14)
        y = r.bottom + 30
        for txt, _ in self.avisos_op:
            texto(tela, txt, (r.right, y), 14, (255, 150, 130), ancora="topright")
            y += 20

    def _oponente_grande(self, tela):
        s = pygame.Surface((LARGURA_MAPA, ui.ALTURA), pygame.SRCALPHA)
        s.fill((0, 0, 0, 160))
        tela.blit(s, (0, 0))
        r = pygame.Rect(60, 40, 920, 637)
        pygame.draw.rect(tela, (40, 25, 10), r.inflate(12, 12), border_radius=10)
        self.render_op.desenhar_mini(tela, r)
        texto(tela, "Mapa do oponente (clique ou O para fechar)", (r.centerx, r.y - 22), 18, ancora="center")

    def _painel_log(self, tela):
        if not self.ctl.online:
            return
        linhas = self.ctl.log()[-12:]
        r = pygame.Rect(8, 130, 330, 26 + 17 * max(1, len(linhas)))
        s = pygame.Surface(r.size, pygame.SRCALPHA)
        s.fill((0, 0, 0, 170))
        tela.blit(s, r)
        texto(tela, f"Mensagens (F1)  tick {self.ctl.tick}", (r.x + 8, r.y + 4), 13, AMARELO, 1)
        for i, l in enumerate(linhas):
            cor = (160, 230, 255) if l.startswith(">") else (200, 255, 170)
            texto(tela, l, (r.x + 8, r.y + 24 + i * 17), 13, cor, 0, peso="consolas")

    # ---------------------------------------------------------------- mensagens e telas
    def _mensagens(self, tela):
        if self.msg[1] > 0 and self.msg[0]:
            texto(tela, self.msg[0], (LARGURA_MAPA // 2, 150), 24, (255, 120, 100), 3, ancora="center")
        if self.banner[1] > 0 and self.banner[0]:
            texto(tela, self.banner[0], (LARGURA_MAPA // 2, 260), 40, AMARELO, 4, ancora="center")
        if self.ctl.online and getattr(self.ctl, "dessinc", False):
            texto(tela, "DESSINCRONIZADO", (LARGURA_MAPA // 2, 110), 18, (255, 80, 80), ancora="center")

    def _sobreposicao(self, tela, titulo, cor):
        s = pygame.Surface((ui.LARGURA, ui.ALTURA), pygame.SRCALPHA)
        s.fill((0, 0, 0, 150))
        tela.blit(s, (0, 0))
        r = pygame.Rect(0, 0, 520, 420)
        r.center = (ui.LARGURA // 2, ui.ALTURA // 2)
        ui.painel(tela, r, cor=(86, 150, 50), borda=(60, 40, 20), raio=20, espessura=7)
        texto(tela, titulo, (r.centerx, r.y + 56), 48, cor, 4, ancora="center")
        return r

    def _tela_pausa(self, tela):
        r = self._sobreposicao(tela, "PAUSADO", BRANCO)
        mouse = pygame.mouse.get_pos()
        opcoes = [("Continuar", lambda: self._pausar(False), VERDE)]
        if not self.ctl.online:
            opcoes.append(("Reiniciar", self._reiniciar, (230, 160, 40)))
            opcoes.append(("Rodada automática: " + ("ON" if self.partida.auto else "OFF"),
                           self._alternar_auto, (60, 150, 230)))
        else:
            opcoes.append(("Desistir", self._desistir, VERMELHO))
        opcoes.append(("Menu principal", self.app.ir_menu, (150, 90, 50)))
        for i, (rot, acao, cor) in enumerate(opcoes):
            b = Botao((r.x + 90, r.y + 110 + i * 68, r.w - 180, 56), rot, cor, 20)
            b.desenhar(tela, mouse)
            self._botoes_menu.append((b, acao))

    def _alternar_auto(self):
        self.partida.auto = not self.partida.auto

    def _reiniciar(self):
        self.app.iniciar_jogo(self.ctl.reiniciar())

    def _desistir(self):
        self.ctl.desistir()
        self._pausar(False)

    def _tela_fim(self, tela):
        v = self.ctl.vencedor
        ganhou = v == self.ctl.meu
        if not self.ctl.online:
            titulo = "VITÓRIA!" if ganhou else "FIM DE JOGO"
        else:
            titulo = "VITÓRIA!" if ganhou else "EMPATE" if v == 0 else "DERROTA"
        r = self._sobreposicao(tela, titulo, AMARELO if ganhou else (255, 110, 100))
        p = self.pista
        linhas = [f"Rodada alcançada: {self.partida.rodada}",
                  f"Bloons estourados: {formatar(p.pops_total)}",
                  f"Vidas restantes: {max(0, p.vidas)}"]
        for i, l in enumerate(linhas):
            texto(tela, l, (r.centerx, r.y + 118 + i * 32), 20, ancora="center")
        mouse = pygame.mouse.get_pos()
        opcoes = []
        if not self.ctl.online:
            opcoes.append(("Jogar novamente", self._reiniciar, VERDE))
        opcoes.append(("Menu principal", self.app.ir_menu, (150, 90, 50)))
        for i, (rot, acao, cor) in enumerate(opcoes):
            b = Botao((r.x + 110, r.y + 240 + i * 66, r.w - 220, 54), rot, cor, 20)
            b.desenhar(tela, mouse)
            self._botoes_menu.append((b, acao))

    def sair(self) -> None:
        self.ctl.fechar()


_ICONES_ENVIO: dict = {}


def _icone_envio(env) -> pygame.Surface:
    s = _ICONES_ENVIO.get(env.chave)
    if s is None:
        if TIPOS[env.tipo].moab:
            spr = arte.sprite_dirigivel(env.tipo, env.fort, 0, 0)
            w = 40
            s = pygame.transform.smoothscale(spr, (w, max(10, int(w * spr.get_height() / spr.get_width()))))
        else:
            spr = arte.sprite_bloon(env.tipo, env.camo, env.regen, env.fort, 0)
            s = pygame.transform.smoothscale(spr, (34, 34))
        _ICONES_ENVIO[env.chave] = s
    return s
