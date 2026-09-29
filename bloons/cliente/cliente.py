"""Cliente pygame: menu, lobby e tela de partida (esqueleto da semana 1).

Uso:
    python -m bloons.cliente.cliente
Teclas: H hospedar (sobe o servidor e conecta), C conectar, F1 log, ESC sair.
"""

from __future__ import annotations

import sys

import pygame

from bloons.comum import constantes as C
from bloons.comum import protocolo as P
from bloons.cliente.rede import Conexao

LARGURA, ALTURA = 1100, 680
FUNDO = (24, 30, 46)
TEXTO = (235, 238, 245)
DESTAQUE = (255, 196, 64)
ERRO = (240, 90, 90)

MENU, LOBBY, PARTIDA, FIM = "menu", "lobby", "partida", "fim"


class Jogo:
    def __init__(self, host: str, porta: int) -> None:
        pygame.init()
        pygame.display.set_caption("Bloons TD Battles 1v1")
        self.tela = pygame.display.set_mode((LARGURA, ALTURA))
        self.relogio = pygame.time.Clock()
        self.fonte = pygame.font.SysFont("arial", 22)
        self.fonte_grande = pygame.font.SysFont("arial", 56, bold=True)
        self.fonte_log = pygame.font.SysFont("consolas", 16)

        self.host, self.porta = host, porta
        self.cena = MENU
        self.conexao = Conexao()
        self.servidor = None
        self.aviso = ""
        self.mostrar_log = True
        self.vencedor: int | None = None

    # ---------- acoes ----------

    def hospedar(self) -> None:
        from bloons.servidor.servidor import Servidor

        try:
            self.servidor = Servidor(host="0.0.0.0", porta=self.porta)
            self.servidor.iniciar()
        except OSError as e:
            self.aviso = f"Nao foi possivel abrir a porta {self.porta}: {e}"
            return
        self.conectar("127.0.0.1")

    def conectar(self, host: str | None = None) -> None:
        try:
            self.conexao.conectar(host or self.host, self.porta)
            self.cena = LOBBY
            self.aviso = ""
        except OSError as e:
            self.aviso = f"Falha ao conectar em {host or self.host}:{self.porta} ({e})"

    # ---------- loop ----------

    def rodar(self) -> None:
        while True:
            for evento in pygame.event.get():
                if evento.type == pygame.QUIT:
                    return self.sair()
                if evento.type == pygame.KEYDOWN:
                    if evento.key == pygame.K_ESCAPE:
                        return self.sair()
                    if evento.key == pygame.K_F1:
                        self.mostrar_log = not self.mostrar_log
                    if self.cena == MENU and evento.key == pygame.K_h:
                        self.hospedar()
                    if self.cena == MENU and evento.key == pygame.K_c:
                        self.conectar()

            self.processar_rede()
            self.desenhar()
            pygame.display.flip()
            self.relogio.tick(60)

    def processar_rede(self) -> None:
        for msg in self.conexao.pegar_mensagens():
            if msg.comando == P.INICIO:
                self.cena = PARTIDA
            elif msg.comando == P.FIM_JOGO:
                self.vencedor = msg.args["vencedor"]
                self.cena = FIM
            elif msg.comando == P.ERRO and msg.args["codigo"] == P.ERRO_SALA_CHEIA:
                self.aviso = "Sala cheia."
                self.cena = MENU
        if self.cena in (LOBBY, PARTIDA) and not self.conexao.ativo:
            self.aviso = "Conexao perdida."
            self.cena = MENU

    # ---------- desenho ----------

    def texto(self, s: str, pos, cor=TEXTO, fonte=None, centro=False) -> None:
        img = (fonte or self.fonte).render(s, True, cor)
        rect = img.get_rect(center=pos) if centro else img.get_rect(topleft=pos)
        self.tela.blit(img, rect)

    def desenhar(self) -> None:
        self.tela.fill(FUNDO)
        cx = LARGURA // 2
        if self.cena == MENU:
            self.texto("BLOONS TD BATTLES 1v1", (cx, 180), DESTAQUE, self.fonte_grande, True)
            self.texto("[H] Hospedar partida    [C] Conectar", (cx, 320), centro=True)
            self.texto(f"Servidor: {self.host}:{self.porta}", (cx, 360), centro=True)
        elif self.cena == LOBBY:
            pontos = "." * (pygame.time.get_ticks() // 400 % 4)
            self.texto(f"Aguardando oponente{pontos}", (cx, 280), DESTAQUE, self.fonte_grande, True)
            self.texto(f"Voce e o jogador {self.conexao.numero or '?'}", (cx, 360), centro=True)
        elif self.cena == PARTIDA:
            self.texto(f"Jogador {self.conexao.numero}", (20, 16), DESTAQUE)
            self.texto(
                f"$ {C.DINHEIRO_INICIAL}   Vidas {C.VIDAS_INICIAIS}   Renda +{C.RENDA_INICIAL}",
                (220, 16),
            )
            pygame.draw.rect(self.tela, (46, 110, 60), (20, 60, 740, 580), border_radius=8)
            pygame.draw.rect(self.tela, (46, 110, 60), (780, 60, 300, 240), border_radius=8)
            self.texto("Sua trilha", (390, 340), centro=True)
            self.texto("Oponente", (930, 180), centro=True)
        elif self.cena == FIM:
            venceu = self.vencedor == self.conexao.numero
            self.texto("VITORIA" if venceu else "DERROTA", (cx, 300),
                       DESTAQUE if venceu else ERRO, self.fonte_grande, True)

        if self.aviso:
            self.texto(self.aviso, (cx, ALTURA - 40), ERRO, centro=True)
        if self.mostrar_log and self.cena != MENU:
            self.desenhar_log()

    def desenhar_log(self) -> None:
        painel = pygame.Rect(780, 320, 300, 320)
        pygame.draw.rect(self.tela, (12, 14, 22), painel, border_radius=8)
        self.texto("Mensagens (F1)", (painel.x + 10, painel.y + 8), DESTAQUE, self.fonte_log)
        for i, linha in enumerate(self.conexao.log[-14:]):
            self.texto(linha, (painel.x + 10, painel.y + 32 + i * 20), fonte=self.fonte_log)

    def sair(self) -> None:
        self.conexao.fechar()
        if self.servidor:
            self.servidor.parar()
        pygame.quit()


def main() -> None:
    host = sys.argv[1] if len(sys.argv) > 1 else C.HOST_PADRAO
    porta = int(sys.argv[2]) if len(sys.argv) > 2 else C.PORTA_PADRAO
    Jogo(host, porta).rodar()


if __name__ == "__main__":
    main()
