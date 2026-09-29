import unittest

from bloons.jogo import bloons_def as BD
from bloons.jogo import sim as S
from bloons.jogo.herois_def import HEROIS
from bloons.jogo.stats import calcular, pode_upar
from bloons.jogo.torres_def import TORRES


def solo(**kw):
    p = S.Partida("solo", "prado", seed=1, herois={1: "quincy"}, **kw)
    return p, p.pistas[1]


class TestDados(unittest.TestCase):
    def test_22_torres_com_3x5_upgrades(self):
        self.assertEqual(len(TORRES), 22)
        for t in TORRES.values():
            self.assertEqual(len(t.caminhos), 3, t.chave)
            for c in t.caminhos:
                self.assertEqual(len(c), 5, t.chave)

    def test_herois(self):
        self.assertGreaterEqual(len(HEROIS), 17)
        for h in HEROIS.values():
            calcular(h.chave, nivel=20)

    def test_todas_as_combinacoes_validas_calculam(self):
        for chave in TORRES:
            for a in range(6):
                for b in range(3):
                    for p1, p2 in ((0, 1), (1, 2), (2, 0)):
                        cam = [0, 0, 0]
                        cam[p1], cam[p2] = a, b
                        calcular(chave, tuple(cam))

    def test_rbe(self):
        self.assertEqual(BD.rbe("vermelho"), 1)
        self.assertEqual(BD.rbe("rosa"), 5)
        self.assertEqual(BD.rbe("ceramica"), 104)
        self.assertEqual(BD.rbe("moab"), 616)


class TestRegrasDeUpgrade(unittest.TestCase):
    def test_caminhos_cruzados(self):
        self.assertTrue(pode_upar([4, 2, 0], 0))
        self.assertFalse(pode_upar([5, 0, 0], 0))
        self.assertFalse(pode_upar([2, 2, 0], 2))   # terceiro caminho
        self.assertFalse(pode_upar([3, 2, 0], 1))   # dois caminhos acima de 2
        self.assertTrue(pode_upar([2, 1, 0], 1))


class TestSimulacao(unittest.TestCase):
    def test_colocar_upar_vender(self):
        p, pi = solo()
        self.assertEqual(p.aplicar(1, "Tdardo@230,250"), None)
        self.assertEqual(pi.dinheiro, 650 - 200)
        self.assertEqual(p.aplicar(1, "Tdardo@230,250"), S.ERRO_POSICAO)     # em cima de outra
        self.assertEqual(p.aplicar(1, "Tdardo@170,200"), S.ERRO_POSICAO)     # na trilha
        self.assertEqual(p.aplicar(1, "Tsubmarino@600,500"), S.ERRO_POSICAO)  # sem agua
        self.assertEqual(p.aplicar(1, "Tsuper@600,500"), S.ERRO_DINHEIRO)
        self.assertEqual(p.aplicar(1, "Tadora@600,500"), S.ERRO_HEROI)       # heroi errado
        self.assertEqual(p.aplicar(1, "U1:0"), None)
        self.assertEqual(p.aplicar(1, "V1"), None)
        self.assertEqual(int(pi.dinheiro), 650 - 200 - 140 + int((200 + 140) * 0.7))

    def test_bloon_estoura_em_camadas_e_da_dinheiro(self):
        p, pi = solo()
        b = pi.criar_bloon("rosa", 100)
        dinheiro = pi.dinheiro
        at = S.novo_ataque(dict(dano=1))
        pi.aplicar_dano(b, 1, at, None)
        self.assertFalse(b.vivo)
        filhos = [x for x in pi.bloons if x.vivo]
        self.assertEqual([x.tipo.nome for x in filhos], ["amarelo"])
        self.assertEqual(pi.dinheiro, dinheiro + 1)

    def test_imunidades(self):
        p, pi = solo()
        chumbo = pi.criar_bloon("chumbo", 100)
        pi.aplicar_dano(chumbo, 5, S.novo_ataque(dict(dtype=BD.AFIADO)), None)
        self.assertTrue(chumbo.vivo)                      # dardo nao estoura chumbo
        pi.aplicar_dano(chumbo, 1, S.novo_ataque(dict(dtype=BD.EXPLOSAO)), None)
        self.assertFalse(chumbo.vivo)                     # bomba estoura
        preto = pi.criar_bloon("preto", 100)
        pi.aplicar_dano(preto, 1, S.novo_ataque(dict(dtype=BD.EXPLOSAO)), None)
        self.assertTrue(preto.vivo)                       # preto e imune a explosao

    def test_vazamento_tira_vidas_pelo_rbe(self):
        p, pi = solo()
        vidas = pi.vidas
        pi.criar_bloon("rosa", pi.mapa.caminhos[0].comprimento - 1)
        for _ in range(5):
            p.passo()
        self.assertEqual(pi.vidas, vidas - 5)

    def test_rodada_solo_completa(self):
        p, pi = solo()
        p.aplicar(1, "Tdardo@230,250")
        p.aplicar(1, "Tdardo@110,250")
        p.aplicar(1, "N")
        for _ in range(30 * 90):
            p.passo()
            if not p.em_rodada:
                break
        self.assertFalse(p.em_rodada)
        self.assertEqual(p.rodada, 1)
        self.assertEqual(pi.vidas, 150)

    def test_determinismo_batalha(self):
        def jogar():
            p = S.Partida("batalha", "encruzilhada", seed=9, herois={1: "quincy", 2: "obyn"})
            roteiro = {10: [(1, "Tdardo@150,300"), (2, "Ttachinha@150,300")],
                       60: [(1, "Tquincy@600,250"), (2, "Tobyn@600,250")],
                       200: [(1, "Sr8"), (2, "Sb6"), (1, "U1:0")]}
            for i in range(30 * 60):
                for j, c in roteiro.get(i, []):
                    p.aplicar(j, c)
                if i % 120 == 0 and i > 300:
                    p.aplicar(1, "Sg5")
                    p.aplicar(2, "Sr8")
                p.passo()
            return p.hash()
        self.assertEqual(jogar(), jogar())

    def test_envio_vai_para_o_oponente_e_aumenta_eco(self):
        p = S.Partida("batalha", "prado", seed=1)
        for _ in range(int(3.5 / S.DT)):
            p.passo()
        eco = p.pistas[1].eco
        self.assertIsNone(p.aplicar(1, "Sr8"))
        self.assertGreater(p.pistas[1].eco, eco)
        self.assertEqual(len(p.pistas[2].fila) - 0 > 0, True)
        self.assertEqual(p.aplicar(1, "Sbad"), S.ERRO_BLOQUEADO)  # ainda nao liberado


if __name__ == "__main__":
    unittest.main()
