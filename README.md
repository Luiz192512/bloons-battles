# Bloons TD Battles 1v1

Trabalho 02 de Sistemas Operacionais (ESOFT 4S, prof. Maurilio Campano Jr): jogo tower defense
1v1 inspirado em Bloons TD Battles 2, com comunicação entre processos por socket TCP, memória
compartilhada e exclusão mútua.

Entrega: 15/11/2026 23:59.

## Como rodar

```bash
python -m pip install -r requirements.txt
```

Jogador que hospeda (sobe o servidor e conecta), tecla **H** no menu:

```bash
python -m bloons.cliente.cliente
```

Segundo jogador, tecla **C** no menu (informe o IP de quem hospeda):

```bash
python -m bloons.cliente.cliente 192.168.0.10 5050
```

Servidor sozinho (opcional):

```bash
python -m bloons.servidor.servidor 5050
```

Testes:

```bash
python -m unittest discover -s tests -v
```

## Estrutura

```
bloons/
  comum/     protocolo.py (notação das mensagens), constantes.py
  servidor/  servidor.py (sockets e threads), partida.py (memória compartilhada + locks)
  cliente/   cliente.py (pygame), rede.py (thread de recepção e heartbeat)
assets/      sprites e sons (Kenney.nl, CC0)
docs/        plano.md (design completo), prompt-escolha-do-jogo.md
tests/
```

## Protocolo (resumo)

Uma mensagem por linha: `<origem><comando><argumentos>`, origem `0` = servidor, `1`/`2` = jogador.

| Mensagem | Significado |
|---|---|
| `1J` / `0J1` | pedido de entrada / você é o jogador 1 |
| `0I` | início da partida |
| `1T2@08,04` | jogador 1 coloca torre tipo 2 na célula 8,4 |
| `2S3x10` | jogador 2 envia 10 balões tipo 3 |
| `0X1D` | erro do jogador 1: dinheiro insuficiente |
| `1P` / `0P` | heartbeat |
| `0F2` | fim de jogo, jogador 2 venceu |

Tabela completa e as condições de corrida tratadas em [docs/plano.md](docs/plano.md).

## Progresso

- [x] Semana 1: protocolo com testes, servidor aceita 2 jogadores, lobby
- [ ] Semana 2: game loop no servidor, balões, snapshot `0E`
- [ ] Semana 3: cliente desenha snapshot, colocar torre com locks
- [ ] Semana 4: tiros, camadas, dinheiro, vidas, fim de jogo
- [ ] Semana 5: envio de balões, renda, rodadas, upgrade e venda
- [ ] Semana 6: UI final, sprites, sons, desconexão
- [ ] Semana 7: testes, executável (pyinstaller), vídeo, formulário
