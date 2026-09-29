"""Notacao de mensagens do jogo.

Cada mensagem e uma linha ASCII terminada em '\\n', no formato:

    <origem><comando><argumentos>

origem: '0' servidor, '1' ou '2' jogador.
Exemplos:
    1J            jogador pede para entrar
    0J1           servidor: voce e o jogador 1
    0I            inicio da partida
    1T2@08,04     jogador 1 coloca torre tipo 2 na celula (8, 4)
    1U05          upgrade da torre 05
    1V05          venda da torre 05
    2S3x10        jogador 2 envia 10 baloes tipo 3
    0R07          rodada 7
    0X1D          erro do jogador 1: dinheiro insuficiente
    1P / 0P       heartbeat
    0F2           fim de jogo, jogador 2 venceu
"""

from __future__ import annotations

import re
from dataclasses import dataclass, field

SERVIDOR = 0
FIM_MSG = "\n"

# Comandos
ENTRAR = "J"
INICIO = "I"
TORRE = "T"
UPGRADE = "U"
VENDA = "V"
ENVIO = "S"
RODADA = "R"
ESTADO = "E"
CONFIRMA = "A"
ERRO = "X"
PING = "P"
FIM_JOGO = "F"

# Codigos de erro de 0X<jogador><codigo>
ERRO_DINHEIRO = "D"
ERRO_POSICAO = "P"
ERRO_ESPERA = "C"
ERRO_SALA_CHEIA = "S"
ERRO_INVALIDA = "M"

_RE_TORRE = re.compile(r"^(\d)@(\d{2}),(\d{2})$")
_RE_ID = re.compile(r"^(\d{2})$")
_RE_ENVIO = re.compile(r"^(\d)x(\d{1,3})$")


class MensagemInvalida(ValueError):
    pass


@dataclass(frozen=True)
class Mensagem:
    origem: int
    comando: str
    args: dict = field(default_factory=dict)


# ---------- montagem ----------

def _linha(origem: int, comando: str, corpo: str = "") -> str:
    return f"{origem}{comando}{corpo}{FIM_MSG}"


def entrar(jogador: int = 1) -> str:
    return _linha(jogador, ENTRAR)


def boas_vindas(jogador: int) -> str:
    return _linha(SERVIDOR, ENTRAR, str(jogador))


def inicio() -> str:
    return _linha(SERVIDOR, INICIO)


def torre(jogador: int, tipo: int, x: int, y: int) -> str:
    return _linha(jogador, TORRE, f"{tipo}@{x:02d},{y:02d}")


def upgrade(jogador: int, id_torre: int) -> str:
    return _linha(jogador, UPGRADE, f"{id_torre:02d}")


def venda(jogador: int, id_torre: int) -> str:
    return _linha(jogador, VENDA, f"{id_torre:02d}")


def envio(jogador: int, tipo: int, qtd: int) -> str:
    return _linha(jogador, ENVIO, f"{tipo}x{qtd}")


def rodada(numero: int) -> str:
    return _linha(SERVIDOR, RODADA, f"{numero:02d}")


def erro(jogador: int, codigo: str) -> str:
    return _linha(SERVIDOR, ERRO, f"{jogador}{codigo}")


def ping(origem: int) -> str:
    return _linha(origem, PING)


def fim_jogo(vencedor: int) -> str:
    return _linha(SERVIDOR, FIM_JOGO, str(vencedor))


# ---------- leitura ----------

def interpretar(linha: str) -> Mensagem:
    """Converte uma linha (sem '\\n') em Mensagem. Levanta MensagemInvalida."""
    linha = linha.strip()
    if len(linha) < 2 or not linha[0].isdigit():
        raise MensagemInvalida(linha)
    origem, cmd, corpo = int(linha[0]), linha[1], linha[2:]

    if cmd in (INICIO, PING):
        if corpo:
            raise MensagemInvalida(linha)
        return Mensagem(origem, cmd)

    if cmd == ENTRAR:
        if origem == SERVIDOR:
            if not corpo.isdigit():
                raise MensagemInvalida(linha)
            return Mensagem(origem, cmd, {"jogador": int(corpo)})
        return Mensagem(origem, cmd)

    if cmd == TORRE:
        m = _RE_TORRE.match(corpo)
        if not m:
            raise MensagemInvalida(linha)
        return Mensagem(origem, cmd, {"tipo": int(m[1]), "x": int(m[2]), "y": int(m[3])})

    if cmd in (UPGRADE, VENDA):
        m = _RE_ID.match(corpo)
        if not m:
            raise MensagemInvalida(linha)
        return Mensagem(origem, cmd, {"id": int(m[1])})

    if cmd == ENVIO:
        m = _RE_ENVIO.match(corpo)
        if not m or int(m[2]) == 0:
            raise MensagemInvalida(linha)
        return Mensagem(origem, cmd, {"tipo": int(m[1]), "qtd": int(m[2])})

    if cmd == RODADA:
        if not corpo.isdigit():
            raise MensagemInvalida(linha)
        return Mensagem(origem, cmd, {"numero": int(corpo)})

    if cmd == ERRO:
        if len(corpo) != 2 or not corpo[0].isdigit():
            raise MensagemInvalida(linha)
        return Mensagem(origem, cmd, {"jogador": int(corpo[0]), "codigo": corpo[1]})

    if cmd == FIM_JOGO:
        if not corpo.isdigit():
            raise MensagemInvalida(linha)
        return Mensagem(origem, cmd, {"vencedor": int(corpo)})

    if cmd in (ESTADO, CONFIRMA):
        # formato definido nas proximas semanas; repassa o corpo cru
        return Mensagem(origem, cmd, {"corpo": corpo})

    raise MensagemInvalida(linha)


class LeitorDeLinhas:
    """TCP e um fluxo de bytes: junta pedacos recebidos e devolve linhas completas."""

    def __init__(self) -> None:
        self._buffer = ""

    def alimentar(self, dados: bytes) -> list[str]:
        self._buffer += dados.decode("ascii", errors="replace")
        *linhas, self._buffer = self._buffer.split(FIM_MSG)
        return [l for l in linhas if l]
