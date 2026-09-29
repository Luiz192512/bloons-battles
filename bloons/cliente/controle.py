"""Controladores: ligam a tela de jogo a uma partida solo (local) ou batalha (rede)."""

from __future__ import annotations

from bloons.comum import protocolo as P
from bloons.jogo import sim as S
from bloons.jogo.rodadas import ENVIOS_POR_CHAVE
from bloons.jogo.stats import definicao

PASSOS_POR_TICK = 2      # servidor a 15 ticks/s x 2 = 30 passos/s
HASH_A_CADA = 45         # ticks (3 s)

MENSAGEM_ERRO = {
    S.ERRO_DINHEIRO: "Dinheiro insuficiente!",
    S.ERRO_POSICAO: "Não pode colocar aqui!",
    S.ERRO_INVALIDO: "Ação inválida.",
    S.ERRO_HEROI: "Você só pode ter 1 herói.",
    S.ERRO_BLOQUEADO: "Bloqueado.",
}


class ControladorSolo:
    online = False

    def __init__(self, mapa: str, dificuldade: str, heroi: str, seed: int = 1234):
        self.args = (mapa, dificuldade, heroi, seed)
        self.partida = S.Partida("solo", mapa, seed, dificuldade, {1: heroi})
        self.meu = 1
        self.velocidade = 1
        self._acc = 0.0
        self.pausado = False
        self.status = ""

    @property
    def pista(self) -> S.Pista:
        return self.partida.pistas[1]

    def reiniciar(self) -> "ControladorSolo":
        return ControladorSolo(*self.args)

    def enviar(self, cmd: str) -> str | None:
        return self.partida.aplicar(1, cmd)

    def botao_play(self) -> None:
        p = self.partida
        if not p.em_rodada:
            p.aplicar(1, "N")
        else:
            self.velocidade = 3 if self.velocidade == 1 else 1

    def atualizar(self, dt: float) -> None:
        if self.pausado or self.partida.fim:
            return
        self._acc += min(dt, 0.1) * self.velocidade
        n = 0
        while self._acc >= S.DT and n < 12:
            self.partida.passo()
            self._acc -= S.DT
            n += 1

    @property
    def terminou(self) -> bool:
        return self.partida.fim

    @property
    def vencedor(self):
        return self.partida.vencedor

    def log(self) -> list[str]:
        return []

    def fechar(self) -> None:
        pass


class ControladorBatalha:
    """Lockstep: os comandos so sao aplicados quando voltam do servidor dentro de um tick."""

    online = True

    def __init__(self, conexao, numero: int, seed: int, mapa: str, herois: dict, servidor=None,
                 pendentes=None):
        self.conexao = conexao
        self._pendentes = list(pendentes or [])  # mensagens que chegaram junto com o inicio
        self.servidor = servidor
        self.meu = numero
        self.partida = S.Partida("batalha", mapa, seed, herois=herois)
        self.status = ""
        self.dessinc = False
        self.fim_remoto: int | None = None
        self.tick = 0
        self.velocidade = 1
        self.pausado = False

    @property
    def pista(self) -> S.Pista:
        return self.partida.pistas[self.meu]

    @property
    def oponente(self) -> S.Pista:
        return self.partida.pistas[2 if self.meu == 1 else 1]

    def checar(self, cmd: str) -> str | None:
        """Validacao local antecipada, so para dar retorno imediato ao jogador."""
        p = self.pista
        c, corpo = cmd[0], cmd[1:]
        try:
            if c == "T":
                chave, xy = corpo.split("@")
                x, y = (int(v) for v in xy.split(","))
                dfn = definicao(chave)
                if dfn.heroi and (p.tem_heroi or chave != p.heroi_escolhido):
                    return S.ERRO_HEROI
                if not p.posicao_valida(dfn, x, y):
                    return S.ERRO_POSICAO
                if p.dinheiro < p.custo(dfn.custo, x, y):
                    return S.ERRO_DINHEIRO
            elif c == "U":
                tid, cam = (int(v) for v in corpo.split(":"))
                t = p.torres.get(tid)
                if t is None:
                    return S.ERRO_INVALIDO
                custo = p.custo_upgrade(t, cam)
                if custo is None:
                    return S.ERRO_BLOQUEADO
                if p.dinheiro < custo:
                    return S.ERRO_DINHEIRO
            elif c == "S":
                env = ENVIOS_POR_CHAVE.get(corpo)
                if env is None:
                    return S.ERRO_INVALIDO
                if self.partida.rodada < env.rodada_min:
                    return S.ERRO_BLOQUEADO
                if p.dinheiro < env.custo:
                    return S.ERRO_DINHEIRO
        except (ValueError, KeyError):
            return S.ERRO_INVALIDO
        return None

    def enviar(self, cmd: str) -> str | None:
        if self.terminou:
            return S.ERRO_INVALIDO
        erro = self.checar(cmd)
        if erro:
            return erro
        self.conexao.comando(cmd)
        return None

    def botao_play(self) -> None:
        pass

    def atualizar(self, dt: float) -> None:
        msgs = self._pendentes + self.conexao.pegar_mensagens()
        self._pendentes = []
        for msg in msgs:
            if msg.comando == P.TICK:
                self.tick = msg.args["tick"]
                if self.partida.fim:
                    continue
                for jogador, cmd in msg.args["comandos"]:
                    self.partida.aplicar(jogador, cmd)
                for _ in range(PASSOS_POR_TICK):
                    self.partida.passo()
                if self.tick % HASH_A_CADA == 0:
                    self.conexao.enviar(P.hash_estado(self.meu, self.tick, self.partida.hash()))
            elif msg.comando == P.DESSINC:
                self.dessinc = True
                self.status = f"Dessincronia detectada no tick {msg.args['tick']}!"
            elif msg.comando == P.FIM_JOGO:
                self.fim_remoto = msg.args["vencedor"]
        if not self.conexao.ativo and not self.terminou:
            self.fim_remoto = 0
            self.status = "Conexão perdida."

    def desistir(self) -> None:
        self.conexao.enviar(P.desistir(self.meu))

    @property
    def terminou(self) -> bool:
        return self.partida.fim or self.fim_remoto is not None

    @property
    def vencedor(self):
        if self.partida.fim:
            return self.partida.vencedor
        return self.fim_remoto

    def log(self) -> list[str]:
        return self.conexao.log()

    def fechar(self) -> None:
        self.conexao.fechar()
        if self.servidor:
            self.servidor.parar()
