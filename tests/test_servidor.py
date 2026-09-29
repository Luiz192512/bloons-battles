import socket
import threading
import unittest

from bloons.comum import protocolo as P
from bloons.servidor.partida import Partida
from bloons.servidor.servidor import Servidor


def conectar(porta: int) -> tuple[socket.socket, P.LeitorDeLinhas]:
    s = socket.create_connection(("127.0.0.1", porta), timeout=3)
    return s, P.LeitorDeLinhas()


def ler(s: socket.socket, leitor: P.LeitorDeLinhas, n: int) -> list[str]:
    linhas: list[str] = []
    while len(linhas) < n:
        linhas += leitor.alimentar(s.recv(4096))
    return linhas


class TestServidor(unittest.TestCase):
    def setUp(self):
        self.servidor = Servidor(porta=0)  # porta livre escolhida pelo SO
        self.servidor.iniciar()

    def tearDown(self):
        self.servidor.parar()

    def test_dois_jogadores_iniciam_partida(self):
        a, la = conectar(self.servidor.porta)
        a.sendall(P.entrar().encode())
        self.assertEqual(ler(a, la, 1), ["0J1"])

        b, lb = conectar(self.servidor.porta)
        b.sendall(P.entrar().encode())
        self.assertEqual(ler(b, lb, 2), ["0J2", "0I"])
        self.assertEqual(ler(a, la, 1), ["0I"])

        c, lc = conectar(self.servidor.porta)
        c.sendall(P.entrar().encode())
        self.assertEqual(ler(c, lc, 1), ["0X0S"])  # sala cheia

        a.sendall(P.ping(1).encode())
        self.assertEqual(ler(a, la, 1), ["0P"])

        a.close()
        self.assertEqual(ler(b, lb, 1), ["0F2"])  # oponente saiu, jogador 2 vence
        for s in (b, c):
            s.close()


class TestExclusaoMutua(unittest.TestCase):
    def test_vagas_nao_se_repetem_com_threads_concorrentes(self):
        for _ in range(50):
            partida = Partida()
            barreira = threading.Barrier(8)
            obtidos: list = []
            lock = threading.Lock()

            def entrar():
                barreira.wait()
                numero = partida.reservar_vaga()
                with lock:
                    obtidos.append(numero)

            threads = [threading.Thread(target=entrar) for _ in range(8)]
            for t in threads:
                t.start()
            for t in threads:
                t.join()
            vagas = sorted(n for n in obtidos if n is not None)
            self.assertEqual(vagas, [1, 2])


if __name__ == "__main__":
    unittest.main()
