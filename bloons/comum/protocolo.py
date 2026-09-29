"""Notacao de mensagens do jogo.

Cada mensagem e uma linha ASCII terminada em '\\n', no formato:

    <origem><comando><argumentos>

origem: '0' servidor, '1' ou '2' jogador.

Cliente -> servidor
    1Jquincy,prado    entrar com o heroi Quincy, sugerindo o mapa "prado"
    1Tdardo@230,250   colocar Macaco Dardo em (230, 250)
    1U12:0            upgrade da torre 12 no caminho 0 (superior)
    1V12              vender a torre 12
    1M12:3            modo de alvo da torre 12 (0 primeiro, 1 ultimo, 2 perto, 3 forte)
    1B12:0            usar a habilidade 0 da torre 12
    2Sr8              jogador 2 envia "8 Vermelhos" ao oponente
    1H450,123456      hash do estado da simulacao no tick 450 (deteccao de dessincronia)
    1F                desistir
    1P                heartbeat

Servidor -> clientes
    0J1                           voce e o jogador 1
    0I8231,prado,quincy,adora     inicio: semente, mapa, heroi do J1, heroi do J2
    0K451|1Tdardo@230,250|2Sr8    tick 451 com os comandos a aplicar, na ordem
    0D450                         estados divergiram no tick 450
    0F2                           fim de jogo, jogador 2 venceu
    0X1D                          erro do jogador 1 (codigo)
    0P                            heartbeat
"""

from __future__ import annotations

import re
from dataclasses import dataclass, field

SERVIDOR = 0
FIM_MSG = "\n"
SEP_TICK = "|"

# Comandos de controle
ENTRAR = "J"
INICIO = "I"
TICK = "K"
HASH = "H"
DESSINC = "D"
FIM_JOGO = "F"
ERRO = "X"
PING = "P"

# Comandos de jogo (repassados pelo servidor dentro do tick)
TORRE = "T"
UPGRADE = "U"
VENDA = "V"
MODO = "M"
HABILIDADE = "B"
ENVIO = "S"
COMANDOS_JOGO = {TORRE, UPGRADE, VENDA, MODO, HABILIDADE, ENVIO}

# Codigos de erro de 0X<jogador><codigo>
ERRO_SALA_CHEIA = "S"
ERRO_INVALIDA = "M"
ERRO_FORA_DE_HORA = "O"

_RE_JOGO = {
    TORRE: re.compile(r"^T([a-z_]{1,20})@(\d{1,4}),(\d{1,4})$"),
    UPGRADE: re.compile(r"^U(\d{1,5}):([0-2])$"),
    VENDA: re.compile(r"^V(\d{1,5})$"),
    MODO: re.compile(r"^M(\d{1,5}):([0-3])$"),
    HABILIDADE: re.compile(r"^B(\d{1,5}):(\d)$"),
    ENVIO: re.compile(r"^S([a-z0-9]{1,6})$"),
}
_RE_NOME = re.compile(r"^[a-z_]{1,20}$")


class MensagemInvalida(ValueError):
    pass


@dataclass(frozen=True)
class Mensagem:
    origem: int
    comando: str
    args: dict = field(default_factory=dict)


def comando_valido(cmd: str) -> bool:
    """Verifica a sintaxe de um comando de jogo (sem o digito de origem)."""
    r = _RE_JOGO.get(cmd[:1])
    return bool(r and r.match(cmd))


# ---------- montagem ----------

def _linha(origem: int, corpo: str) -> str:
    return f"{origem}{corpo}{FIM_MSG}"


def entrar(heroi: str, mapa: str, jogador: int = 1) -> str:
    return _linha(jogador, f"{ENTRAR}{heroi},{mapa}")


def boas_vindas(jogador: int) -> str:
    return _linha(SERVIDOR, f"{ENTRAR}{jogador}")


def inicio(seed: int, mapa: str, heroi1: str, heroi2: str) -> str:
    return _linha(SERVIDOR, f"{INICIO}{seed},{mapa},{heroi1},{heroi2}")


def tick(numero: int, comandos: list[str]) -> str:
    return _linha(SERVIDOR, TICK + SEP_TICK.join([str(numero), *comandos]))


def comando(jogador: int, cmd: str) -> str:
    return _linha(jogador, cmd)


def torre(jogador: int, chave: str, x: int, y: int) -> str:
    return comando(jogador, f"{TORRE}{chave}@{int(x)},{int(y)}")


def upgrade(jogador: int, id_torre: int, caminho: int) -> str:
    return comando(jogador, f"{UPGRADE}{id_torre}:{caminho}")


def venda(jogador: int, id_torre: int) -> str:
    return comando(jogador, f"{VENDA}{id_torre}")


def modo(jogador: int, id_torre: int, m: int) -> str:
    return comando(jogador, f"{MODO}{id_torre}:{m}")


def habilidade(jogador: int, id_torre: int, idx: int) -> str:
    return comando(jogador, f"{HABILIDADE}{id_torre}:{idx}")


def envio(jogador: int, chave: str) -> str:
    return comando(jogador, f"{ENVIO}{chave}")


def hash_estado(jogador: int, numero_tick: int, valor: int) -> str:
    return _linha(jogador, f"{HASH}{numero_tick},{valor}")


def dessinc(numero_tick: int) -> str:
    return _linha(SERVIDOR, f"{DESSINC}{numero_tick}")


def desistir(jogador: int) -> str:
    return _linha(jogador, FIM_JOGO)


def fim_jogo(vencedor: int) -> str:
    return _linha(SERVIDOR, f"{FIM_JOGO}{vencedor}")


def erro(jogador: int, codigo: str) -> str:
    return _linha(SERVIDOR, f"{ERRO}{jogador}{codigo}")


def ping(origem: int) -> str:
    return _linha(origem, PING)


# ---------- leitura ----------

def interpretar(linha: str) -> Mensagem:
    """Converte uma linha (sem '\\n') em Mensagem. Levanta MensagemInvalida."""
    linha = linha.strip()
    if len(linha) < 2 or not linha[0].isdigit():
        raise MensagemInvalida(linha)
    origem, cmd, corpo = int(linha[0]), linha[1], linha[2:]
    servidor = origem == SERVIDOR

    if cmd == PING:
        if corpo:
            raise MensagemInvalida(linha)
        return Mensagem(origem, cmd)

    if cmd == ENTRAR:
        if servidor:
            if corpo not in ("1", "2"):
                raise MensagemInvalida(linha)
            return Mensagem(origem, cmd, {"jogador": int(corpo)})
        partes = corpo.split(",")
        if len(partes) != 2 or not all(_RE_NOME.match(p) for p in partes):
            raise MensagemInvalida(linha)
        return Mensagem(origem, cmd, {"heroi": partes[0], "mapa": partes[1]})

    if cmd == INICIO and servidor:
        partes = corpo.split(",")
        if len(partes) != 4 or not partes[0].isdigit():
            raise MensagemInvalida(linha)
        return Mensagem(origem, cmd, {"seed": int(partes[0]), "mapa": partes[1],
                                      "herois": {1: partes[2], 2: partes[3]}})

    if cmd == TICK and servidor:
        partes = corpo.split(SEP_TICK)
        if not partes[0].isdigit():
            raise MensagemInvalida(linha)
        comandos = []
        for c in partes[1:]:
            if len(c) < 2 or c[0] not in "12" or not comando_valido(c[1:]):
                raise MensagemInvalida(linha)
            comandos.append((int(c[0]), c[1:]))
        return Mensagem(origem, cmd, {"tick": int(partes[0]), "comandos": comandos})

    if cmd in COMANDOS_JOGO and not servidor:
        if not comando_valido(linha[1:]):
            raise MensagemInvalida(linha)
        return Mensagem(origem, cmd, {"cmd": linha[1:]})

    if cmd == HASH and not servidor:
        partes = corpo.split(",")
        if len(partes) != 2 or not all(p.isdigit() for p in partes):
            raise MensagemInvalida(linha)
        return Mensagem(origem, cmd, {"tick": int(partes[0]), "hash": int(partes[1])})

    if cmd == DESSINC and servidor:
        if not corpo.isdigit():
            raise MensagemInvalida(linha)
        return Mensagem(origem, cmd, {"tick": int(corpo)})

    if cmd == FIM_JOGO:
        if servidor:
            if not corpo.isdigit():
                raise MensagemInvalida(linha)
            return Mensagem(origem, cmd, {"vencedor": int(corpo)})
        if corpo:
            raise MensagemInvalida(linha)
        return Mensagem(origem, cmd)

    if cmd == ERRO and servidor:
        if len(corpo) != 2 or not corpo[0].isdigit():
            raise MensagemInvalida(linha)
        return Mensagem(origem, cmd, {"jogador": int(corpo[0]), "codigo": corpo[1]})

    raise MensagemInvalida(linha)


class LeitorDeLinhas:
    """TCP e um fluxo de bytes: junta pedacos recebidos e devolve linhas completas."""

    LIMITE = 1 << 16

    def __init__(self) -> None:
        self._buffer = ""

    def alimentar(self, dados: bytes) -> list[str]:
        self._buffer += dados.decode("ascii", errors="replace")
        *linhas, self._buffer = self._buffer.split(FIM_MSG)
        if len(self._buffer) > self.LIMITE:  # linha absurda: descarta
            self._buffer = ""
        return [l for l in linhas if l]
