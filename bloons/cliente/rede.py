"""Conexao do cliente: thread de recepcao + thread de heartbeat.

A thread de rede (produtora) coloca mensagens na fila; a thread do pygame
(consumidora) retira a cada quadro. O lock protege essa fila e o log de
mensagens, que sao memoria compartilhada entre as duas threads.
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
        self._log: list[str] = []
        self._lock_log = threading.Lock()
        self._lock_envio = threading.Lock()
        self._ativo = threading.Event()
        self.numero: int | None = None
        self.bytes_env = 0
        self.bytes_rec = 0

    def conectar(self, host: str, porta: int, heroi: str, mapa: str) -> None:
        self._sock = socket.create_connection((host, porta), timeout=4)
        self._sock.settimeout(None)
        self._sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        self._ativo.set()
        threading.Thread(target=self._receber, name="rede-rx", daemon=True).start()
        threading.Thread(target=self._heartbeat, name="rede-hb", daemon=True).start()
        self.enviar(P.entrar(heroi, mapa))

    @property
    def ativo(self) -> bool:
        return self._ativo.is_set()

    def enviar(self, linha: str) -> None:
        if not self._sock:
            return
        # troca a origem provisoria pelo numero recebido do servidor
        if self.numero and linha[0] != "0":
            linha = str(self.numero) + linha[1:]
        if linha[1] not in (P.PING, P.HASH):
            self._registrar("> " + linha.strip())
        dados = linha.encode("ascii")
        try:
            with self._lock_envio:  # heartbeat e jogo podem enviar ao mesmo tempo
                self._sock.sendall(dados)
            self.bytes_env += len(dados)
        except OSError:
            self._ativo.clear()

    def comando(self, cmd: str) -> None:
        self.enviar(P.comando(self.numero or 1, cmd))

    def pegar_mensagens(self) -> list[P.Mensagem]:
        with self._lock_fila:
            msgs, self._fila = self._fila, []
        return msgs

    def log(self) -> list[str]:
        with self._lock_log:
            return list(self._log)

    def fechar(self) -> None:
        self._ativo.clear()
        if self._sock:
            try:
                self._sock.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            try:
                self._sock.close()
            except OSError:
                pass

    def _registrar(self, texto: str) -> None:
        if len(texto) > 60:
            texto = texto[:57] + "..."
        with self._lock_log:
            self._log.append(texto)
            del self._log[:-40]

    def _receber(self) -> None:
        leitor = P.LeitorDeLinhas()
        while self._ativo.is_set():
            try:
                dados = self._sock.recv(65536)
            except OSError:
                break
            if not dados:
                break
            self.bytes_rec += len(dados)
            for linha in leitor.alimentar(dados):
                try:
                    msg = P.interpretar(linha)
                except P.MensagemInvalida:
                    continue
                if msg.comando == P.PING:
                    continue
                if msg.comando != P.TICK or msg.args["comandos"]:
                    self._registrar("< " + linha)
                if msg.comando == P.ENTRAR:
                    self.numero = msg.args["jogador"]
                with self._lock_fila:
                    self._fila.append(msg)
        self._ativo.clear()

    def _heartbeat(self) -> None:
        espera = threading.Event()
        while self._ativo.is_set():
            self.enviar(P.ping(self.numero or 1))
            espera.wait(C.HEARTBEAT_INTERVALO)
