"""Servidor autoritativo: aceita 2 jogadores e distribui as mensagens.

Uso:
    python -m bloons.servidor.servidor [porta]
"""

from __future__ import annotations

import logging
import socket
import sys
import threading
import time

from bloons.comum import constantes as C
from bloons.comum import protocolo as P
from bloons.servidor.partida import Partida

log = logging.getLogger("servidor")


class Servidor:
    def __init__(self, host: str = C.HOST_PADRAO, porta: int = C.PORTA_PADRAO) -> None:
        self.partida = Partida()
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sock.bind((host, porta))
        self.porta = self._sock.getsockname()[1]
        self._clientes: dict[int, socket.socket] = {}
        self._lock_clientes = threading.Lock()  # protege o dicionario de sockets
        self._rodando = threading.Event()

    # ---------- ciclo de vida ----------

    def iniciar(self) -> None:
        self._sock.listen()
        self._rodando.set()
        threading.Thread(target=self._aceitar, name="aceitar", daemon=True).start()
        log.info("ouvindo na porta %d", self.porta)

    def parar(self) -> None:
        self._rodando.clear()
        try:
            self._sock.close()
        except OSError:
            pass
        with self._lock_clientes:
            for s in self._clientes.values():
                try:
                    s.close()
                except OSError:
                    pass
            self._clientes.clear()

    # ---------- envio ----------

    def enviar(self, numero: int, linha: str) -> None:
        with self._lock_clientes:
            s = self._clientes.get(numero)
        if s is None:
            return
        try:
            s.sendall(linha.encode("ascii"))
        except OSError:
            pass

    def transmitir(self, linha: str) -> None:
        with self._lock_clientes:
            destinos = list(self._clientes)
        for numero in destinos:
            self.enviar(numero, linha)

    # ---------- threads ----------

    def _aceitar(self) -> None:
        while self._rodando.is_set():
            try:
                conn, endereco = self._sock.accept()
            except OSError:
                break
            threading.Thread(
                target=self._atender, args=(conn, endereco), daemon=True
            ).start()

    def _atender(self, conn: socket.socket, endereco) -> None:
        conn.settimeout(C.HEARTBEAT_TIMEOUT)
        leitor = P.LeitorDeLinhas()
        numero: int | None = None
        try:
            while self._rodando.is_set():
                try:
                    dados = conn.recv(4096)
                except socket.timeout:
                    log.info("jogador %s sem resposta, desconectando", numero)
                    break
                except OSError:  # conexao resetada (WinError 10054) ou socket fechado
                    break
                if not dados:
                    break
                for linha in leitor.alimentar(dados):
                    try:
                        msg = P.interpretar(linha)
                    except P.MensagemInvalida:
                        if numero:
                            self.enviar(numero, P.erro(numero, P.ERRO_INVALIDA))
                        continue

                    if numero is None:
                        if msg.comando != P.ENTRAR:
                            continue
                        numero = self.partida.reservar_vaga()
                        if numero is None:
                            conn.sendall(P.erro(0, P.ERRO_SALA_CHEIA).encode("ascii"))
                            return
                        with self._lock_clientes:
                            self._clientes[numero] = conn
                        self.enviar(numero, P.boas_vindas(numero))
                        log.info("jogador %d entrou de %s", numero, endereco)
                        if self.partida.iniciar():
                            self.transmitir(P.inicio())
                            log.info("partida iniciada")
                        continue

                    self._tratar(numero, msg)
        finally:
            if numero is not None:
                with self._lock_clientes:
                    self._clientes.pop(numero, None)
                self.partida.remover(numero)
                log.info("jogador %d saiu", numero)
                oponente = self.partida.oponente(numero)
                self.enviar(oponente, P.fim_jogo(oponente))
            try:
                conn.close()
            except OSError:
                pass

    def _tratar(self, numero: int, msg: P.Mensagem) -> None:
        if msg.origem != numero:
            self.enviar(numero, P.erro(numero, P.ERRO_INVALIDA))
            return
        if msg.comando == P.PING:
            self.enviar(numero, P.ping(P.SERVIDOR))
            return
        # Semanas seguintes: torre, upgrade, venda, envio de baloes
        log.info("jogador %d: %s %s", numero, msg.comando, msg.args)


def main() -> None:
    logging.basicConfig(level=logging.INFO, format="%(asctime)s [%(name)s] %(message)s")
    porta = int(sys.argv[1]) if len(sys.argv) > 1 else C.PORTA_PADRAO
    servidor = Servidor(porta=porta)
    servidor.iniciar()
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        servidor.parar()


if __name__ == "__main__":
    main()
