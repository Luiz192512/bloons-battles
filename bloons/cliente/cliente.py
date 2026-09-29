"""Aplicativo pygame: janela, laco principal e troca de cenas.

Uso:
    python -m bloons.cliente.cliente
"""

from __future__ import annotations

import sys

import pygame

from bloons.cliente import som, ui


class App:
    def __init__(self) -> None:
        pygame.init()
        pygame.display.set_caption("Bloons TD Battles - SO 2026")
        self.tela = pygame.display.set_mode((ui.LARGURA, ui.ALTURA), pygame.SCALED | pygame.RESIZABLE)
        self.relogio = pygame.time.Clock()
        som.iniciar()
        from bloons.cliente.cenas_menu import CenaMenu, FundoBloons
        self.fundo = FundoBloons()
        self.cena = CenaMenu(self)
        self.rodando = True
        self.mostrar_fps = False

    def trocar(self, cena) -> None:
        antiga = self.cena
        self.cena = cena
        if antiga is not None and hasattr(antiga, "sair"):
            antiga.sair()

    def ir_menu(self) -> None:
        from bloons.cliente.cenas_menu import CenaMenu
        self.trocar(CenaMenu(self))

    def iniciar_jogo(self, controle) -> None:
        from bloons.cliente.cena_jogo import CenaJogo
        antiga = self.cena
        self.cena = CenaJogo(self, controle)
        # a cena antiga (lobby/jogo anterior) nao deve fechar a conexao nova
        if antiga is not None and antiga.__class__.__name__ == "CenaJogo" and antiga.ctl is not controle:
            antiga.sair()

    def sair(self) -> None:
        self.rodando = False

    def rodar(self) -> None:
        while self.rodando:
            dt = self.relogio.tick(60) / 1000.0
            for e in pygame.event.get():
                if e.type == pygame.QUIT:
                    self.rodando = False
                elif e.type == pygame.KEYDOWN and e.key == pygame.K_F3:
                    self.mostrar_fps = not self.mostrar_fps
                else:
                    self.cena.evento(e)
            self.cena.atualizar(dt)
            self.cena.desenhar(self.tela)
            if self.mostrar_fps:
                ui.texto(self.tela, f"{self.relogio.get_fps():.0f} fps", (6, ui.ALTURA - 24), 14)
            pygame.display.flip()
        if hasattr(self.cena, "sair"):
            self.cena.sair()
        pygame.quit()


def main() -> None:
    App().rodar()
    sys.exit(0)


if __name__ == "__main__":
    main()
