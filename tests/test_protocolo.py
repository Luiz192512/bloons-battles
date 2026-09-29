import unittest

from bloons.comum import protocolo as P


class TestMontagem(unittest.TestCase):
    def test_exemplos_do_plano(self):
        self.assertEqual(P.torre(1, 2, 8, 4), "1T2@08,04\n")
        self.assertEqual(P.envio(2, 3, 10), "2S3x10\n")
        self.assertEqual(P.upgrade(1, 5), "1U05\n")
        self.assertEqual(P.erro(1, P.ERRO_DINHEIRO), "0X1D\n")
        self.assertEqual(P.fim_jogo(2), "0F2\n")
        self.assertEqual(P.rodada(7), "0R07\n")


class TestInterpretar(unittest.TestCase):
    def test_ida_e_volta(self):
        casos = [
            (P.torre(1, 2, 8, 4), P.TORRE, {"tipo": 2, "x": 8, "y": 4}),
            (P.envio(2, 3, 10), P.ENVIO, {"tipo": 3, "qtd": 10}),
            (P.venda(1, 5), P.VENDA, {"id": 5}),
            (P.boas_vindas(2), P.ENTRAR, {"jogador": 2}),
            (P.erro(1, "D"), P.ERRO, {"jogador": 1, "codigo": "D"}),
            (P.fim_jogo(1), P.FIM_JOGO, {"vencedor": 1}),
        ]
        for linha, cmd, args in casos:
            with self.subTest(linha=linha):
                msg = P.interpretar(linha)
                self.assertEqual(msg.comando, cmd)
                self.assertEqual(msg.args, args)

    def test_invalidas(self):
        for linha in ["", "X", "1Z", "1T2@8,4", "2S3x0", "1U5", "0Fx", "1Pextra"]:
            with self.subTest(linha=linha):
                with self.assertRaises(P.MensagemInvalida):
                    P.interpretar(linha)


class TestLeitorDeLinhas(unittest.TestCase):
    def test_junta_pedacos_do_tcp(self):
        leitor = P.LeitorDeLinhas()
        self.assertEqual(leitor.alimentar(b"1T2@0"), [])
        self.assertEqual(leitor.alimentar(b"8,04\n2S3"), ["1T2@08,04"])
        self.assertEqual(leitor.alimentar(b"x10\n1P\n"), ["2S3x10", "1P"])


if __name__ == "__main__":
    unittest.main()
