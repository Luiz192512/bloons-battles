"""Servidor do modo Batalha (lockstep).

O servidor nao simula o jogo: ele ORDENA os comandos. A cada tick (15 por
segundo) ele esvazia a fila compartilhada e transmite "0K<tick>|cmd|cmd".
Os dois clientes aplicam os mesmos comandos, na mesma ordem, no mesmo tick,
e como a simulacao e deterministica os estados ficam identicos.

Uso:
    python -m bloons.servidor.servidor [porta]
"""

from __future__ import annotations

import logging
import random
import socket
import sys
import threading
import time

from bloons.comum import constantes as C
from bloons.comum import protocolo as P
from bloons.jogo.herois_def import HEROIS
from bloons.jogo.mapas import MAPAS
from bloons.servidor.partida import Sala

log = logging.getLogger("servidor")

TICKS_POR_SEGUNDO = 15


class Servidor:
    def __init__(self, host: str = C.HOST_PADRAO, porta: int = C.PORTA_PADRAO) -> None:
        self.sala = Sala()
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sock.bind((host, porta))
        self.porta = self._sock.getsockname()[1]
        self._clientes: dict[int, socket.socket] = {}
        self._lock_clientes = threading.Lock()  # protege o dicionario de sockets
        self._lock_envio: dict[int, threading.Lock] = {}  # um sendall por vez por socket
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
            socks = list(self._clientes.values())
            self._clientes.clear()
        for s in socks:
            try:
                s.close()
            except OSError:
                pass

    # ---------- envio ----------

    def enviar(self, numero: int, linha: str) -> None:
        with self._lock_clientes:
            s = self._clientes.get(numero)
            trava = self._lock_envio.get(numero)
        if s is None or trava is None:
            return
        try:
            with trava:
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
            conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            threading.Thread(target=self._atender, args=(conn, endereco), daemon=True).start()

    def _relogio(self) -> None:
        """Thread consumidora: fecha um tick a cada 1/15 s e transmite."""
        intervalo = 1.0 / TICKS_POR_SEGUNDO
        proximo = time.perf_counter()
        while self._rodando.is_set() and self.sala.em_jogo():
            numero, comandos = self.sala.fechar_tick()
            self.transmitir(P.tick(numero, comandos))
            proximo += intervalo
            espera = proximo - time.perf_counter()
            if espera > 0:
                time.sleep(espera)
            else:
                proximo = time.perf_counter()

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
                        heroi = msg.args["heroi"] if msg.args["heroi"] in HEROIS else "quincy"
                        mapa = msg.args["mapa"] if msg.args["mapa"] in MAPAS else "prado"
                        numero = self.sala.reservar_vaga(heroi, mapa)
                        if numero is None:
                            conn.sendall(P.erro(0, P.ERRO_SALA_CHEIA).encode("ascii"))
                            return
                        with self._lock_clientes:
                            self._clientes[numero] = conn
                            self._lock_envio[numero] = threading.Lock()
                        self.enviar(numero, P.boas_vindas(numero))
                        log.info("jogador %d entrou de %s (%s)", numero, endereco,
                                 heroi)
                        dados_inicio = self.sala.iniciar()
                        if dados_inicio:
                            mapa, h1, h2 = dados_inicio
                            seed = random.randrange(1, 10 ** 6)
                            self.transmitir(P.inicio(seed, mapa, h1, h2))
                            threading.Thread(target=self._relogio, name="relogio",
                                             daemon=True).start()
                            log.info("partida iniciada: mapa %s, %s x %s", mapa, h1, h2)
                        continue

                    self._tratar(numero, msg)
        finally:
            if numero is not None:
                with self._lock_clientes:
                    self._clientes.pop(numero, None)
                    self._lock_envio.pop(numero, None)
                estava_em_jogo = self.sala.remover(numero)
                log.info("jogador %d saiu", numero)
                if estava_em_jogo and self.sala.encerrar():
                    self.transmitir(P.fim_jogo(self.sala.oponente(numero)))
            try:
                conn.close()
            except OSError:
                pass

    def _tratar(self, numero: int, msg: P.Mensagem) -> None:
        if msg.origem != numero:
            self.enviar(numero, P.erro(numero, P.ERRO_INVALIDA))
            return
        cmd = msg.comando
        if cmd == P.PING:
            self.enviar(numero, P.ping(P.SERVIDOR))
        elif cmd in P.COMANDOS_JOGO:
            if not self.sala.em_jogo():
                self.enviar(numero, P.erro(numero, P.ERRO_FORA_DE_HORA))
                return
            self.sala.enfileirar(numero, msg.args["cmd"])
        elif cmd == P.HASH:
            iguais = self.sala.registrar_hash(numero, msg.args["tick"], msg.args["hash"])
            if iguais is False:
                log.warning("dessincronia no tick %d", msg.args["tick"])
                self.transmitir(P.dessinc(msg.args["tick"]))
        elif cmd == P.FIM_JOGO:
            if self.sala.encerrar():
                self.transmitir(P.fim_jogo(self.sala.oponente(numero)))


def main() -> None:
    logging.basicConfig(level=logging.INFO, format="%(asctime)s [%(name)s] %(message)s")
    porta = int(sys.argv[1]) if len(sys.argv) > 1 else C.PORTA_PADRAO
    servidor = Servidor(host="0.0.0.0", porta=porta)
    servidor.iniciar()
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        servidor.parar()


if __name__ == "__main__":
    main()
