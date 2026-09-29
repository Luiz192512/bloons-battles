"""Memoria compartilhada do servidor.

Varias threads acessam este objeto ao mesmo tempo:
  - uma thread por cliente (entra na sala, enfileira comandos, envia hashes);
  - a thread do relogio (esvazia a fila a cada tick e transmite).

Cada regiao tem seu proprio lock. Regra anti deadlock: nunca segurar dois
locks ao mesmo tempo; quando precisar dos dois, adquira sempre na ordem
lock_sala -> lock_fila -> lock_hash.
"""

from __future__ import annotations

import threading
from dataclasses import dataclass

from bloons.comum import constantes as C


@dataclass
class InfoJogador:
    numero: int
    heroi: str
    mapa: str


class Sala:
    def __init__(self) -> None:
        self.estado = C.AGUARDANDO
        self.jogadores: dict[int, InfoJogador] = {}
        self.lock_sala = threading.Lock()   # jogadores e estado

        self.tick = 0
        self._fila: list[str] = []          # comandos aguardando o proximo tick
        self.lock_fila = threading.Lock()

        self._hashes: dict[int, dict[int, int]] = {}  # tick -> jogador -> hash
        self.lock_hash = threading.Lock()

    # ---------- sala ----------

    def reservar_vaga(self, heroi: str, mapa: str) -> int | None:
        """Numero do novo jogador, ou None se a sala estiver cheia.

        Secao critica: dois clientes conectando ao mesmo tempo nao podem
        receber o mesmo numero.
        """
        with self.lock_sala:
            if self.estado != C.AGUARDANDO:
                return None
            for numero in range(1, C.MAX_JOGADORES + 1):
                if numero not in self.jogadores:
                    self.jogadores[numero] = InfoJogador(numero, heroi, mapa)
                    return numero
            return None

    def iniciar(self) -> tuple[str, str, str] | None:
        """Muda para EM_JOGO uma unica vez. Devolve (mapa, heroi1, heroi2) a quem iniciou."""
        with self.lock_sala:
            if self.estado == C.AGUARDANDO and len(self.jogadores) == C.MAX_JOGADORES:
                self.estado = C.EM_JOGO
                j1, j2 = self.jogadores[1], self.jogadores[2]
                return j1.mapa, j1.heroi, j2.heroi
            return None

    def em_jogo(self) -> bool:
        with self.lock_sala:
            return self.estado == C.EM_JOGO

    def encerrar(self) -> bool:
        """Marca FIM. Devolve True so para a primeira thread que encerrar."""
        with self.lock_sala:
            if self.estado == C.FIM:
                return False
            self.estado = C.FIM
            return True

    def remover(self, numero: int) -> bool:
        """Remove o jogador. Devolve True se a partida estava em andamento."""
        with self.lock_sala:
            self.jogadores.pop(numero, None)
            return self.estado == C.EM_JOGO

    @staticmethod
    def oponente(numero: int) -> int:
        return 2 if numero == 1 else 1

    # ---------- fila de comandos (produtor/consumidor) ----------

    def enfileirar(self, jogador: int, cmd: str) -> None:
        with self.lock_fila:
            self._fila.append(f"{jogador}{cmd}")

    def fechar_tick(self) -> tuple[int, list[str]]:
        """Troca a fila por uma vazia e avanca o tick, atomicamente."""
        with self.lock_fila:
            comandos, self._fila = self._fila, []
            self.tick += 1
            return self.tick, comandos

    # ---------- deteccao de dessincronia ----------

    def registrar_hash(self, jogador: int, tick: int, valor: int) -> bool | None:
        """Guarda o hash. Quando os dois chegam, devolve True se forem iguais."""
        with self.lock_hash:
            por_jogador = self._hashes.setdefault(tick, {})
            por_jogador[jogador] = valor
            if len(por_jogador) < 2:
                return None
            del self._hashes[tick]
            # limpa ticks antigos que um jogador nunca respondeu
            for t in [t for t in self._hashes if t < tick - 3000]:
                del self._hashes[t]
            return len(set(por_jogador.values())) == 1
