"""Menus: principal, escolha do modo solo, escolha da batalha e sala de espera."""

from __future__ import annotations

import math
import random
import socket

import pygame

from bloons.cliente import arte, som
from bloons.cliente import ui
from bloons.cliente.controle import ControladorBatalha, ControladorSolo
from bloons.cliente.rede import Conexao
from bloons.cliente.ui import AMARELO, BRANCO, VERDE, Botao, CampoTexto, texto
from bloons.comum import constantes as C
from bloons.comum import protocolo as P
from bloons.jogo.bloons_def import TIPOS
from bloons.jogo.herois_def import HEROIS, ORDEM_HEROIS
from bloons.jogo.mapas import MAPAS, ORDEM_MAPAS
from bloons.jogo.rodadas import DIFICULDADES


class FundoBloons:
    """Bloons subindo pelo ceu no fundo dos menus."""

    def __init__(self):
        self.rng = random.Random(5)
        tipos = ["vermelho", "azul", "verde", "amarelo", "rosa", "preto", "branco", "zebra",
                 "arco_iris", "ceramica", "roxo", "chumbo"]
        self.bloons = [[self.rng.uniform(0, ui.LARGURA), self.rng.uniform(0, ui.ALTURA),
                        self.rng.choice(tipos), self.rng.uniform(30, 80), self.rng.uniform(0, 6)]
                       for _ in range(26)]

    def desenhar(self, tela, dt):
        ui.fundo_gradiente(tela)
        for b in self.bloons:
            b[1] -= b[3] * dt
            b[4] += dt
            if b[1] < -40:
                b[1] = ui.ALTURA + 40
                b[0] = self.rng.uniform(0, ui.LARGURA)
            spr = arte.sprite_bloon(b[2], False, False, False, 0)
            x = b[0] + math.sin(b[4]) * 12
            tela.blit(spr, spr.get_rect(center=(int(x), int(b[1]))))
            pygame.draw.line(tela, (80, 80, 80), (x, b[1] + TIPOS[b[2]].raio + 3), (x + math.sin(b[4] * 2) * 4, b[1] + TIPOS[b[2]].raio + 26), 1)
        # grama no rodape
        pygame.draw.rect(tela, (98, 170, 58), (0, ui.ALTURA - 60, ui.LARGURA, 60))
        pygame.draw.rect(tela, (70, 140, 40), (0, ui.ALTURA - 60, ui.LARGURA, 6))


def titulo(tela, y=110):
    texto(tela, "BLOONS TD", (ui.LARGURA // 2, y), 84, AMARELO, 6, ancora="center")
    texto(tela, "BATTLES", (ui.LARGURA // 2, y + 82), 64, (255, 120, 60), 5, ancora="center")


# ====================================================================== MENU
class CenaMenu:
    def __init__(self, app):
        self.app = app
        self.fundo = app.fundo
        cx = ui.LARGURA // 2
        self.botoes = [
            (Botao((cx - 170, 300, 340, 64), "Jogar Solo", VERDE, 28), lambda: app.trocar(CenaSolo(app))),
            (Botao((cx - 170, 380, 340, 64), "Batalha: Hospedar", (60, 150, 230), 26),
             lambda: app.trocar(CenaBatalha(app, True))),
            (Botao((cx - 170, 460, 340, 64), "Batalha: Entrar", (60, 150, 230), 26),
             lambda: app.trocar(CenaBatalha(app, False))),
            (Botao((cx - 170, 560, 340, 56), "Sair", (180, 70, 50), 24), app.sair),
        ]

    def evento(self, e):
        for b, acao in self.botoes:
            if b.clicou(e):
                som.tocar("colocar")
                acao()

    def atualizar(self, dt):
        self._dt = dt

    def desenhar(self, tela):
        self.fundo.desenhar(tela, getattr(self, "_dt", 0.016))
        titulo(tela)
        for b, _ in self.botoes:
            b.desenhar(tela)
        for i, chave in enumerate(["dardo", "super", "ninja", "mago"]):
            spr = arte.sprite_torre(chave, 110, 0)
            x = 150 if i < 2 else ui.LARGURA - 150
            y = 330 + (i % 2) * 150
            tela.blit(spr, spr.get_rect(center=(x, y)))
        texto(tela, "Trabalho 02 de Sistemas Operacionais: comunicação entre processos",
              (ui.LARGURA // 2, ui.ALTURA - 30), 16, ancora="center")

    def sair(self):
        pass


# ====================================================================== SELECAO DE HEROI (comum)
class GradeHerois:
    def __init__(self, x, y, colunas=6, tam=64):
        self.x, self.y, self.colunas, self.tam = x, y, colunas, tam
        self.escolhido = "quincy"

    def rects(self):
        for i, chave in enumerate(ORDEM_HEROIS):
            col, lin = i % self.colunas, i // self.colunas
            yield chave, pygame.Rect(self.x + col * (self.tam + 8), self.y + lin * (self.tam + 8),
                                     self.tam, self.tam)

    def evento(self, e):
        if e.type == pygame.MOUSEBUTTONDOWN and e.button == 1:
            for chave, r in self.rects():
                if r.collidepoint(e.pos):
                    self.escolhido = chave
                    som.tocar("colocar")

    def desenhar(self, tela):
        for chave, r in self.rects():
            sel = chave == self.escolhido
            pygame.draw.rect(tela, (60, 40, 20), r.move(0, 3), border_radius=10)
            pygame.draw.rect(tela, (255, 226, 120) if sel else (236, 214, 150), r, border_radius=10)
            pygame.draw.rect(tela, (255, 255, 255) if sel else (90, 60, 30), r, 3 if sel else 2, border_radius=10)
            spr = arte.sprite_torre(chave, self.tam - 8, 0)
            tela.blit(spr, spr.get_rect(center=r.center))
        h = HEROIS[self.escolhido]
        return h


def _info_heroi(tela, h, x, y):
    texto(tela, h.nome, (x, y), 28, AMARELO)
    texto(tela, h.titulo, (x, y + 38), 16)
    texto(tela, f"Custo: ${h.custo}", (x, y + 64), 16, ui.DINHEIRO)
    if h.hab3:
        texto(tela, f"Nível 3: {h.hab3['nome']}", (x, y + 92), 15)
    if h.hab10:
        texto(tela, f"Nível 10: {h.hab10['nome']}", (x, y + 116), 15)


# ====================================================================== SOLO
class CenaSolo:
    def __init__(self, app):
        self.app = app
        self.mapa = "prado"
        self.dif = "medio"
        self.herois = GradeHerois(60, 410, colunas=9, tam=62)
        self.voltar = Botao((30, 640, 170, 54), "Voltar", (150, 90, 50), 22)
        self.jogar = Botao((ui.LARGURA - 260, 630, 230, 70), "JOGAR!", VERDE, 32)

    def _mapas(self):
        for i, chave in enumerate(ORDEM_MAPAS):
            yield chave, pygame.Rect(60 + i * 250, 90, 230, 150)

    def _difs(self):
        for i, chave in enumerate(DIFICULDADES):
            yield chave, pygame.Rect(60 + i * 250, 312, 230, 50)

    def evento(self, e):
        self.herois.evento(e)
        if e.type == pygame.MOUSEBUTTONDOWN and e.button == 1:
            for chave, r in self._mapas():
                if r.collidepoint(e.pos):
                    self.mapa = chave
            for chave, r in self._difs():
                if r.collidepoint(e.pos):
                    self.dif = chave
        if self.voltar.clicou(e):
            self.app.ir_menu()
        if self.jogar.clicou(e) or (e.type == pygame.KEYDOWN and e.key == pygame.K_RETURN):
            som.tocar("rodada")
            self.app.iniciar_jogo(ControladorSolo(self.mapa, self.dif, self.herois.escolhido,
                                                  seed=random.randrange(1, 10 ** 6)))

    def atualizar(self, dt):
        pass

    def desenhar(self, tela):
        ui.fundo_gradiente(tela, (70, 140, 60), (40, 90, 40))
        texto(tela, "Escolha o mapa", (60, 50), 26)
        for chave, r in self._mapas():
            sel = chave == self.mapa
            pygame.draw.rect(tela, (255, 230, 90) if sel else (60, 40, 20), r.inflate(10, 10), border_radius=10)
            tela.blit(arte.miniatura_mapa(chave, r.w, r.h), r)
            m = MAPAS[chave]
            texto(tela, m.nome, (r.centerx, r.bottom + 16), 16, ancora="center")
            texto(tela, m.dificuldade, (r.centerx, r.bottom - 14), 13, AMARELO, ancora="center")
        texto(tela, "Dificuldade", (60, 280), 20)
        for chave, r in self._difs():
            nome, vidas, _, ultima = DIFICULDADES[chave]
            b = Botao(r, f"{nome} ({ultima})", VERDE if chave == self.dif else (120, 110, 90), 18)
            b.desenhar(tela)
        texto(tela, "Escolha o herói", (60, 378), 20)
        h = self.herois.desenhar(tela)
        _info_heroi(tela, h, 740, 470)
        self.voltar.desenhar(tela)
        self.jogar.desenhar(tela)

    def sair(self):
        pass


# ====================================================================== BATALHA
class CenaBatalha:
    def __init__(self, app, hospedar: bool):
        self.app = app
        self.hospedar = hospedar
        self.mapa = "prado"
        self.herois = GradeHerois(60, 330, colunas=9, tam=62)
        self.ip = CampoTexto((60, 120, 300, 48), C.HOST_PADRAO, "IP de quem hospeda")
        self.porta = CampoTexto((390, 120, 140, 48), str(C.PORTA_PADRAO), "Porta")
        self.voltar = Botao((30, 640, 170, 54), "Voltar", (150, 90, 50), 22)
        self.ir = Botao((ui.LARGURA - 320, 630, 290, 70), "HOSPEDAR" if hospedar else "CONECTAR",
                        VERDE, 30)
        self.erro = ""

    def _mapas(self):
        for i, chave in enumerate(ORDEM_MAPAS):
            yield chave, pygame.Rect(60 + i * 250, 90, 230, 160)

    def evento(self, e):
        self.herois.evento(e)
        if not self.hospedar:
            self.ip.evento(e)
        self.porta.evento(e)
        if self.hospedar and e.type == pygame.MOUSEBUTTONDOWN and e.button == 1:
            for chave, r in self._mapas():
                if r.collidepoint(e.pos):
                    self.mapa = chave
        if self.voltar.clicou(e):
            self.app.ir_menu()
        if self.ir.clicou(e):
            self._conectar()

    def _conectar(self):
        try:
            porta = int(self.porta.valor)
        except ValueError:
            self.erro = "Porta inválida."
            return
        servidor = None
        host = self.ip.valor.strip() or C.HOST_PADRAO
        if self.hospedar:
            from bloons.servidor.servidor import Servidor
            try:
                servidor = Servidor(host="0.0.0.0", porta=porta)
                servidor.iniciar()
            except OSError as ex:
                self.erro = f"Não foi possível abrir a porta {porta}: {ex}"
                return
            host = "127.0.0.1"
        con = Conexao()
        try:
            con.conectar(host, porta, self.herois.escolhido, self.mapa)
        except OSError as ex:
            self.erro = f"Falha ao conectar em {host}:{porta} ({ex})"
            if servidor:
                servidor.parar()
            return
        self.app.trocar(CenaLobby(self.app, con, servidor, porta))

    def atualizar(self, dt):
        pass

    def desenhar(self, tela):
        ui.fundo_gradiente(tela, (50, 100, 170), (30, 60, 110))
        if self.hospedar:
            texto(tela, "Hospedar batalha: escolha o mapa", (60, 50), 26)
            for chave, r in self._mapas():
                sel = chave == self.mapa
                pygame.draw.rect(tela, (255, 230, 90) if sel else (30, 30, 50), r.inflate(10, 10), border_radius=10)
                tela.blit(arte.miniatura_mapa(chave, r.w, r.h), r)
                texto(tela, MAPAS[chave].nome, (r.centerx, r.bottom + 16), 16, ancora="center")
            self.porta.rect.topleft = (60, 520)
            self.porta.desenhar(tela)
        else:
            texto(tela, "Entrar em uma batalha", (60, 40), 26)
            self.ip.desenhar(tela)
            self.porta.rect.topleft = (390, 120)
            self.porta.desenhar(tela)
            texto(tela, "O mapa é escolhido por quem hospeda.", (60, 190), 16)
        texto(tela, "Escolha o herói", (60, 296), 20)
        h = self.herois.desenhar(tela)
        _info_heroi(tela, h, 740, 410)
        if self.erro:
            texto(tela, self.erro, (ui.LARGURA // 2, 600), 18, (255, 120, 100), ancora="center")
        self.voltar.desenhar(tela)
        self.ir.desenhar(tela)

    def sair(self):
        pass


def _ips_locais() -> list[str]:
    ips = set()
    try:
        for info in socket.getaddrinfo(socket.gethostname(), None, socket.AF_INET):
            ips.add(info[4][0])
    except OSError:
        pass
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ips.add(s.getsockname()[0])
        s.close()
    except OSError:
        pass
    return sorted(ip for ip in ips if not ip.startswith("127."))


class CenaLobby:
    def __init__(self, app, conexao, servidor, porta):
        self.app = app
        self.con = conexao
        self.servidor = servidor
        self.porta = porta
        self.ips = _ips_locais() if servidor else []
        self.status = "Conectando..."
        self.cancelar = Botao((ui.LARGURA // 2 - 120, 600, 240, 60), "Cancelar", (180, 70, 50), 24)
        self.t = 0.0
        self._iniciou = False

    def evento(self, e):
        if self.cancelar.clicou(e):
            self._encerrar()
            self.app.ir_menu()

    def _encerrar(self):
        self.con.fechar()
        if self.servidor:
            self.servidor.parar()

    def atualizar(self, dt):
        self.t += dt
        msgs = self.con.pegar_mensagens()
        for i, msg in enumerate(msgs):
            if msg.comando == P.ENTRAR:
                self.status = f"Você é o jogador {msg.args['jogador']}. Aguardando oponente"
            elif msg.comando == P.ERRO and msg.args["codigo"] == P.ERRO_SALA_CHEIA:
                self.status = "Sala cheia!"
            elif msg.comando == P.INICIO:
                ctl = ControladorBatalha(self.con, self.con.numero, msg.args["seed"], msg.args["mapa"],
                                         msg.args["herois"], self.servidor, pendentes=msgs[i + 1:])
                self._iniciou = True
                som.tocar("rodada")
                self.app.iniciar_jogo(ctl)
                return
        if not self.con.ativo and not self._iniciou:
            self.status = "Conexão encerrada."

    def desenhar(self, tela):
        self.app.fundo.desenhar(tela, 0.016)
        titulo(tela, 90)
        pontos = "." * (int(self.t * 2) % 4)
        texto(tela, self.status + pontos, (ui.LARGURA // 2, 330), 28, ancora="center")
        if self.servidor:
            texto(tela, "Seu oponente deve usar \"Batalha: Entrar\" com o IP:", (ui.LARGURA // 2, 400), 20, ancora="center")
            ips = ", ".join(self.ips) or "127.0.0.1"
            texto(tela, f"{ips}   porta {self.porta}", (ui.LARGURA // 2, 440), 28, AMARELO, ancora="center")
            texto(tela, "(no mesmo computador, use 127.0.0.1)", (ui.LARGURA // 2, 480), 16, ancora="center")
        self.cancelar.desenhar(tela)

    def sair(self):
        if not self._iniciou:
            self._encerrar()
