# Bloons TD Battles

Trabalho 02 de Sistemas Operacionais (ESOFT 4S, prof. Maurilio Campano Jr): tower defense
inspirado no Bloons TD Battles 2, com modo solo e modo Batalha em rede usando sockets TCP,
threads, memória compartilhada e exclusão mútua.

- 22 torres com 3 caminhos de 5 upgrades, 18 heróis, 17 tipos de bloon, 100 rodadas.
- Modo Batalha 1v1 em lockstep: o servidor ordena os comandos e cada cliente simula as duas pistas.
- Arte toda desenhada por código, sem imagens externas.

Design completo, notação das mensagens e respostas do formulário: [docs/plano.md](docs/plano.md).

## Como rodar

Requer Python 3.11+.

```bash
python -m pip install -r requirements.txt
```

```bash
python jogar.py
```

- **Solo:** "Jogar Solo", escolha mapa, dificuldade e herói.
- **Batalha no mesmo PC:** abra o jogo duas vezes. Na primeira, "Batalha: Hospedar". Na segunda,
  "Batalha: Entrar" com IP `127.0.0.1`.
- **Batalha em dois PCs:** quem hospeda vê o IP na sala de espera; o outro digita esse IP.
  Libere a porta 5050 no firewall do Windows se pedir.

Servidor avulso (opcional):

```bash
python -m bloons.servidor.servidor 5050
```

## Controles

| Tecla | Ação |
|---|---|
| Clique no card / Q W E R T Y Z X C V B N M A S D F G H J K L | escolher torre |
| U | herói |
| Clique no mapa (Shift mantém a torre selecionada) | colocar |
| Clique na torre | abrir upgrades |
| `,` `.` `/` | upgrade nos caminhos 1, 2 e 3 |
| Tab | prioridade de alvo |
| Backspace | vender |
| 1 a 9 | habilidades |
| Espaço | iniciar rodada / acelerar (solo) |
| O | ver o mapa do oponente (batalha) |
| F1 | painel de mensagens trocadas (batalha) |
| Esc | cancelar / menu |

## Testes

```bash
python -m unittest discover -s tests -v
```

Cobrem o protocolo, o servidor com clientes reais, a exclusão mútua com threads concorrentes e o
determinismo da simulação.

## Entrega

```bash
python -m pip install pyinstaller
```

```bash
python empacotar.py
```

Gera em `entrega/` o zip só com o código e o zip com o executável (`BloonsBattles.exe`).

## Estrutura

```
bloons/
  comum/     protocolo.py (notação das mensagens), constantes.py
  servidor/  servidor.py (sockets e threads), partida.py (memória compartilhada + locks)
  jogo/      sim.py (simulação determinística), torres_def.py, herois_def.py,
             bloons_def.py, rodadas.py, mapas.py, stats.py
  cliente/   cliente.py (app), cena_jogo.py, cenas_menu.py, render.py, arte.py, ui.py,
             controle.py (solo e batalha), rede.py (threads de rede), som.py
docs/        plano.md, prompt-escolha-do-jogo.md
tests/
```
