"""Estado da partida: a memoria compartilhada entre as threads do servidor.

Regra anti deadlock: quando precisar dos dois, adquira SEMPRE
lock_economia antes de lock_trilha[i].
"""

from __future__ import annotations

import threading
from dataclasses import dataclass, field

from bloons.comum import constantes as C


@dataclass
class Jogador:
    numero: int
    dinheiro: int = C.DINHEIRO_INICIAL
    renda: int = C.RENDA_INICIAL
    vidas: int = C.VIDAS_INICIAIS
    torres: list = field(default_factory=list)


@dataclass
class Trilha:
    baloes: list = field(default_factory=list)


class Partida:
    def __init__(self) -> None:
        self.estado = C.AGUARDANDO
        self.tick = 0
        self.rodada = 0
        self.jogadores: dict[int, Jogador] = {}
        self.trilhas = {1: Trilha(), 2: Trilha()}

        self.lock_sala = threading.Lock()       # entrada e saida de jogadores, estado
        self.lock_economia = threading.Lock()   # dinheiro, renda, vidas
        self.lock_trilha = {1: threading.Lock(), 2: threading.Lock()}

    def reservar_vaga(self) -> int | None:
        """Devolve o numero do novo jogador ou None se a sala estiver cheia.

        Secao critica: dois clientes conectando ao mesmo tempo nao podem
        receber o mesmo numero.
        """
        with self.lock_sala:
            if self.estado != C.AGUARDANDO:
                return None
            for numero in range(1, C.MAX_JOGADORES + 1):
                if numero not in self.jogadores:
                    self.jogadores[numero] = Jogador(numero)
                    return numero
            return None

    def sala_cheia(self) -> bool:
        with self.lock_sala:
            return len(self.jogadores) == C.MAX_JOGADORES

    def iniciar(self) -> bool:
        """Muda para EM_JOGO uma unica vez. Devolve True para quem iniciou."""
        with self.lock_sala:
            if self.estado == C.AGUARDANDO and len(self.jogadores) == C.MAX_JOGADORES:
                self.estado = C.EM_JOGO
                return True
            return False

    def remover(self, numero: int) -> None:
        with self.lock_sala:
            self.jogadores.pop(numero, None)
            if self.estado == C.EM_JOGO:
                self.estado = C.FIM

    def oponente(self, numero: int) -> int:
        return 2 if numero == 1 else 1
