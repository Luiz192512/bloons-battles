"""Constantes compartilhadas entre servidor e cliente."""

HOST_PADRAO = "127.0.0.1"
PORTA_PADRAO = 5050

MAX_JOGADORES = 2

HEARTBEAT_INTERVALO = 2.0  # segundos entre pings
HEARTBEAT_TIMEOUT = 8.0    # segundos sem mensagem = desconectado

# Estados da sala (equivalem a variavel "vez" do enunciado:
# existem nos dois processos e sao sincronizados pelas mensagens)
AGUARDANDO = "AGUARDANDO"
EM_JOGO = "EM_JOGO"
FIM = "FIM"
