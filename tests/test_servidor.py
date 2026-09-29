import socket
import threading
import time
import unittest

from bloons.comum import protocolo as P
from bloons.servidor.partida import Sala
from bloons.servidor.servidor import Servidor


class Cliente:
    def __init__(self, porta: int):
        self.s = socket.create_connection(("127.0.0.1", porta), timeout=3)
        self.leitor = P.LeitorDeLinhas()
        self.buffer: list[str] = []

    def enviar(self, linha: str) -> None:
        self.s.sendall(linha.encode())

    def proxima(self, filtro=None) -> str:
        """Proxima linha (ignorando ticks vazios e pings, ou filtrando por comando)."""
        fim = time.time() + 3
        while time.time() < fim:
            while self.buffer:
                l = self.buffer.pop(0)
                if l in ("0P",):
                    continue
                if filtro is None and l.startswith("0K") and "|" not in l:
                    continue
                if filtro is None or l[1] == filtro:
                    return l
            self.buffer += self.leitor.alimentar(self.s.recv(4096))
        raise TimeoutError

    def fechar(self):
        self.s.close()


class TestServidor(unittest.TestCase):
    def setUp(self):
        self.servidor = Servidor(porta=0)
        self.servidor.iniciar()

    def tearDown(self):
        self.servidor.parar()

    def test_partida_completa(self):
        a = Cliente(self.servidor.porta)
        a.enviar(P.entrar("quincy", "lago"))
        self.assertEqual(a.proxima(), "0J1")

        b = Cliente(self.servidor.porta)
        b.enviar(P.entrar("heroi_inexistente", "prado"))
        self.assertEqual(b.proxima(), "0J2")
        inicio_b = P.interpretar(b.proxima())
        inicio_a = P.interpretar(a.proxima())
        self.assertEqual(inicio_a, inicio_b)
        self.assertEqual(inicio_a.args["mapa"], "lago")                 # mapa do jogador 1
        self.assertEqual(inicio_a.args["herois"], {1: "quincy", 2: "quincy"})  # heroi invalido -> padrao

        # terceiro cliente: sala cheia
        c = Cliente(self.servidor.porta)
        c.enviar(P.entrar("adora", "prado"))
        self.assertEqual(c.proxima(), "0X0S")

        # comandos chegam aos DOIS clientes dentro do mesmo tick, na mesma ordem
        a.enviar(P.torre(1, "dardo", 100, 100))
        b.enviar(P.envio(2, "r8"))
        vistos_a, vistos_b = [], []
        while len(vistos_a) < 2:
            vistos_a += P.interpretar(a.proxima()).args["comandos"]
        while len(vistos_b) < 2:
            vistos_b += P.interpretar(b.proxima()).args["comandos"]
        self.assertEqual(vistos_a, vistos_b)
        self.assertCountEqual(vistos_a, [(1, "Tdardo@100,100"), (2, "Sr8")])

        # cliente nao pode se passar pelo outro
        a.enviar(P.envio(2, "r8"))
        self.assertEqual(a.proxima(P.ERRO), "0X1M")

        # hashes diferentes -> dessincronia avisada aos dois
        a.enviar(P.hash_estado(1, 45, 111))
        b.enviar(P.hash_estado(2, 45, 222))
        self.assertEqual(a.proxima(P.DESSINC), "0D45")
        self.assertEqual(b.proxima(P.DESSINC), "0D45")

        # desistencia: o oponente vence
        a.enviar(P.desistir(1))
        self.assertEqual(b.proxima(P.FIM_JOGO), "0F2")
        for x in (a, b, c):
            x.fechar()

    def test_desconexao_da_vitoria_ao_oponente(self):
        a = Cliente(self.servidor.porta)
        a.enviar(P.entrar("quincy", "prado"))
        b = Cliente(self.servidor.porta)
        b.enviar(P.entrar("quincy", "prado"))
        b.proxima(P.INICIO)
        a.fechar()
        self.assertEqual(b.proxima(P.FIM_JOGO), "0F2")
        b.fechar()


class TestExclusaoMutua(unittest.TestCase):
    def test_vagas_nao_se_repetem_com_threads_concorrentes(self):
        for _ in range(50):
            sala = Sala()
            barreira = threading.Barrier(8)
            obtidos: list = []
            lock = threading.Lock()

            def entrar():
                barreira.wait()
                numero = sala.reservar_vaga("quincy", "prado")
                with lock:
                    obtidos.append(numero)

            threads = [threading.Thread(target=entrar) for _ in range(8)]
            for t in threads:
                t.start()
            for t in threads:
                t.join()
            self.assertEqual(sorted(n for n in obtidos if n is not None), [1, 2])

    def test_fila_de_comandos_nao_perde_nem_duplica(self):
        """Produtores (clientes) e consumidor (relogio) ao mesmo tempo."""
        sala = Sala()
        por_thread = 3000
        recebidos: list[str] = []
        parar = threading.Event()

        def produtor(j):
            for i in range(por_thread):
                sala.enfileirar(j, f"Sr{i}")

        def consumidor():
            while not parar.is_set():
                recebidos.extend(sala.fechar_tick()[1])
            recebidos.extend(sala.fechar_tick()[1])

        c = threading.Thread(target=consumidor)
        c.start()
        prods = [threading.Thread(target=produtor, args=(j,)) for j in (1, 2)]
        for t in prods:
            t.start()
        for t in prods:
            t.join()
        parar.set()
        c.join()
        self.assertEqual(len(recebidos), 2 * por_thread)
        self.assertEqual(len(set(recebidos)), 2 * por_thread)
        # a ordem de cada produtor e preservada
        so_1 = [int(r[3:]) for r in recebidos if r[0] == "1"]
        self.assertEqual(so_1, list(range(por_thread)))


if __name__ == "__main__":
    unittest.main()
