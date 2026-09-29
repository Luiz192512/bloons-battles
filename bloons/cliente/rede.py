"""Conexao do cliente: thread de recepcao + heartbeat.

A thread de rede escreve na fila de mensagens e a thread do pygame le.
O lock protege essa fila compartilhada entre as duas threads.
"""

from __future__ import annotations

import socket
import threading

from bloons.comum import constantes as C
from bloons.comum import protocolo as P


class Conexao:
    def __init__(self) -> None:
        self._sock: socket.socket | None = None
        self._fila: list[P.Mensagem] = []
        self._lock_fila = threading.Lock()
        self._ativo = threading.Event()
        self.numero: int | None = None
        self.log: list[str] = []  # ultimas mensagens, para o painel F1

    def conectar(self, host: str, porta: int) -> None:
        self._sock = socket.create_connection((host, porta), timeout=3)
        self._sock.settimeout(None)
        self._ativo.set()
        threading.Thread(target=self._receber, name="rede-rx", daemon=True).start()
        threading.Thread(target=self._heartbeat, name="rede-hb", daemon=True).start()
        self.enviar(P.entrar())

    @property
    def ativo(self) -> bool:
        return self._ativo.is_set()

    def enviar(self, linha: str) -> None:
        if not self._sock:
            return
        # troca a origem provisoria pelo numero recebido do servidor
        if self.numero and linha[0] != "0":
            linha = str(self.numero) + linha[1:]
        if linha[1] != P.PING:
            self._registrar("> " + linha.strip())
        try:
            self._sock.sendall(linha.encode("ascii"))
        except OSError:
            self._ativo.clear()

    def pegar_mensagens(self) -> list[P.Mensagem]:
        with self._lock_fila:
            msgs, self._fila = self._fila, []
        return msgs

    def fechar(self) -> None:
        self._ativo.clear()
        if self._sock:
            try:
                self._sock.close()
            except OSError:
                pass

    def _registrar(self, texto: str) -> None:
        self.log.append(texto)
        del self.log[:-12]

    def _receber(self) -> None:
        leitor = P.LeitorDeLinhas()
        while self._ativo.is_set():
            try:
                dados = self._sock.recv(4096)
            except OSError:
                break
            if not dados:
                break
            for linha in leitor.alimentar(dados):
                try:
                    msg = P.interpretar(linha)
                except P.MensagemInvalida:
                    continue
                if msg.comando != P.PING:
                    self._registrar("< " + linha)
                if msg.comando == P.ENTRAR:
                    self.numero = msg.args["jogador"]
                with self._lock_fila:
                    self._fila.append(msg)
        self._ativo.clear()

    def _heartbeat(self) -> None:
        parar = threading.Event()
        while self._ativo.is_set():
            self.enviar(P.ping(self.numero or 1))
            parar.wait(C.HEARTBEAT_INTERVALO)
