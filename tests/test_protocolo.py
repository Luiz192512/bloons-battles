import unittest

from bloons.comum import protocolo as P


class TestMontagem(unittest.TestCase):
    def test_exemplos_da_documentacao(self):
        self.assertEqual(P.entrar("quincy", "prado"), "1Jquincy,prado\n")
        self.assertEqual(P.torre(1, "dardo", 230, 250), "1Tdardo@230,250\n")
        self.assertEqual(P.upgrade(1, 12, 0), "1U12:0\n")
        self.assertEqual(P.venda(1, 12), "1V12\n")
        self.assertEqual(P.modo(1, 12, 3), "1M12:3\n")
        self.assertEqual(P.habilidade(1, 12, 0), "1B12:0\n")
        self.assertEqual(P.envio(2, "r8"), "2Sr8\n")
        self.assertEqual(P.inicio(8231, "prado", "quincy", "adora"), "0I8231,prado,quincy,adora\n")
        self.assertEqual(P.tick(451, ["1Tdardo@230,250", "2Sr8"]), "0K451|1Tdardo@230,250|2Sr8\n")
        self.assertEqual(P.tick(452, []), "0K452\n")
        self.assertEqual(P.hash_estado(1, 450, 123456), "1H450,123456\n")
        self.assertEqual(P.dessinc(450), "0D450\n")
        self.assertEqual(P.fim_jogo(2), "0F2\n")
        self.assertEqual(P.desistir(1), "1F\n")


class TestInterpretar(unittest.TestCase):
    def test_ida_e_volta(self):
        casos = [
            (P.entrar("adora", "lago"), P.ENTRAR, {"heroi": "adora", "mapa": "lago"}),
            (P.boas_vindas(2), P.ENTRAR, {"jogador": 2}),
            (P.inicio(7, "prado", "quincy", "psi"), P.INICIO,
             {"seed": 7, "mapa": "prado", "herois": {1: "quincy", 2: "psi"}}),
            (P.tick(3, ["1Tdardo@1,2", "2U5:1"]), P.TICK,
             {"tick": 3, "comandos": [(1, "Tdardo@1,2"), (2, "U5:1")]}),
            (P.torre(1, "super", 10, 20), P.TORRE, {"cmd": "Tsuper@10,20"}),
            (P.envio(2, "moab"[:3]), P.ENVIO, {"cmd": "Smoa"}),
            (P.hash_estado(2, 90, 42), P.HASH, {"tick": 90, "hash": 42}),
            (P.dessinc(90), P.DESSINC, {"tick": 90}),
            (P.fim_jogo(1), P.FIM_JOGO, {"vencedor": 1}),
            (P.desistir(2), P.FIM_JOGO, {}),
            (P.erro(1, "M"), P.ERRO, {"jogador": 1, "codigo": "M"}),
        ]
        for linha, cmd, args in casos:
            with self.subTest(linha=linha):
                msg = P.interpretar(linha)
                self.assertEqual(msg.comando, cmd)
                self.assertEqual(msg.args, args)

    def test_invalidas(self):
        for linha in ["", "X", "1Z", "1Tdardo@x,4", "1U5:7", "1V", "1M3:9", "0Fx", "1Pextra",
                      "1J", "1Jso_um", "0K1|9Tdardo@1,1", "0K1|1Qqq", "1Tdardo@12345,1",
                      "0Ix,prado,a,b", "1H1"]:
            with self.subTest(linha=linha):
                with self.assertRaises(P.MensagemInvalida):
                    P.interpretar(linha)

    def test_cliente_nao_pode_falar_como_servidor(self):
        with self.assertRaises(P.MensagemInvalida):
            P.interpretar("1K5|1Sr8")


class TestLeitorDeLinhas(unittest.TestCase):
    def test_junta_pedacos_do_tcp(self):
        leitor = P.LeitorDeLinhas()
        self.assertEqual(leitor.alimentar(b"1Tdardo@2"), [])
        self.assertEqual(leitor.alimentar(b"30,250\n2S"), ["1Tdardo@230,250"])
        self.assertEqual(leitor.alimentar(b"r8\n1P\n"), ["2Sr8", "1P"])


if __name__ == "__main__":
    unittest.main()
