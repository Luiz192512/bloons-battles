"""Constantes compartilhadas entre servidor e cliente."""

HOST_PADRAO = "127.0.0.1"
PORTA_PADRAO = 5050

MAX_JOGADORES = 2
TICKS_POR_SEGUNDO = 20
SNAPSHOTS_POR_SEGUNDO = 10

HEARTBEAT_INTERVALO = 2.0  # segundos entre pings
HEARTBEAT_TIMEOUT = 5.0    # segundos sem mensagem = desconectado

DINHEIRO_INICIAL = 650
VIDAS_INICIAIS = 100
RENDA_INICIAL = 250

# Estados da partida (equivalem a variavel "vez" do enunciado:
# existem nos dois processos e sao sincronizados pelas mensagens)
AGUARDANDO = "AGUARDANDO"
EM_JOGO = "EM_JOGO"
FIM = "FIM"
