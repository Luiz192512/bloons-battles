# Bloons TD Battles 1v1: plano do Trabalho 02 de SO

Nome para a planilha: **Bloons TD Battles (tower defense 1v1)**
Entrega: 15/11/2026 23:59 (cerca de 7 semanas a partir de 28/09/2026)

## 1. Como funciona o original

Dois jogadores defendem trilhas separadas com torres de macacos. Cada um gasta dinheiro para
enviar balões extras à trilha do adversário, o que também aumenta sua renda por rodada. Perde quem
zerar as vidas primeiro.

## 2. Versão da dupla (MVP)

**Entra:**
- 1 mapa, com a mesma trilha para os dois jogadores (justo e mais simples).
- 3 torres:
  - Dardo: barata, acerta 1 balão.
  - Canhão: dano em área.
  - Gelo: deixa os balões mais lentos.
- 3 balões em camadas, como no original:
  - Vermelho: 1 de vida.
  - Azul: 2 de vida, vira vermelho ao ser atingido.
  - Verde: 3 de vida, vira azul ao ser atingido.
- Rodadas automáticas a cada 15 s, que ficam mais fortes com o tempo.
- Envio de balões ao oponente, com custo e tempo de espera. Cada envio aumenta a renda de quem
  enviou (a mecânica central do Battles).
- 100 vidas por jogador, dinheiro inicial de 650.
- 1 upgrade por torre e venda de torre.

**Fica de fora:** heróis, árvores de upgrade, habilidades ativas, vários mapas, ranking online,
MOAB e balões camuflados.

**Extras se sobrar tempo:** modo espectador (terceiro cliente só assistindo), reconexão de
jogador e replay a partir do log de mensagens.

## 3. Arquitetura

```
 [Cliente 1: pygame]  <--socket TCP-->  [SERVIDOR]  <--socket TCP-->  [Cliente 2: pygame]
                                           |
                     thread aceitar | thread cliente 1 | thread cliente 2 | thread game loop
                                           |
                               MEMÓRIA COMPARTILHADA (struct Partida)
                               protegida por mutex (threading.Lock)
```

- **Servidor autoritativo:** só ele simula o jogo (movimento dos balões, tiros, dinheiro). Os
  clientes enviam intenções ("quero colocar torre") e desenham o estado recebido. Isso impede
  trapaça e dessincronização.
- **Threads no servidor:**
  - uma para aceitar conexões;
  - uma por cliente, que lê os comandos dele;
  - uma para o game loop, rodando a 20 ticks por segundo.
- **Threads no cliente:**
  - uma para receber mensagens, que atualiza o estado local;
  - a principal, com o loop do pygame (desenho e entrada do jogador).

## 4. Memória compartilhada

```python
class Jogador:
    dinheiro: int
    renda: int          # recebida a cada rodada
    vidas: int
    torres: list[Torre] # id, tipo, x, y, nivel, recarga

class Trilha:
    baloes: list[Balao] # id, tipo(camada), progresso 0..1, lentidao

class Partida:
    estado: str         # AGUARDANDO | EM_JOGO | FIM (equivale à variável "vez" do enunciado)
    tick: int
    rodada: int
    jogadores: [Jogador, Jogador]
    trilhas:   [Trilha, Trilha]

    lock_economia = Lock()           # dinheiro, renda e vidas
    lock_trilha   = [Lock(), Lock()] # um por trilha
```

**Regra anti deadlock:** quando uma operação precisa de dois locks, ela os pega sempre na mesma
ordem, `lock_economia` antes de `lock_trilha[i]`. Assim nunca há espera circular.

## 5. Exclusão mútua: as condições de corrida reais

| # | Seção crítica | Quem disputa | O que dá errado sem o lock | Solução |
|---|---|---|---|---|
| 1 | Lista de balões da trilha | A thread do jogador 1 insere os balões enviados na trilha do jogador 2, enquanto o game loop remove balões estourados | Lista corrompida, balão pulado ou estourado duas vezes, erro de iteração | `lock_trilha[i]` ao inserir e ao percorrer |
| 2 | Compra de torre ou envio de balões | Duas mensagens seguidas do mesmo jogador, e o game loop creditando renda | Verificar e depois agir: as duas compras veem "tenho 650" e o dinheiro fica negativo | Verificar e debitar dentro do mesmo `with lock_economia:` |
| 3 | Vidas | O game loop tira vidas quando um balão chega ao fim; a verificação de fim de jogo lê as vidas | O jogo declara vencedor errado ou continua depois de zerar | `lock_economia` e mudança do estado para `FIM` dentro dele |
| 4 | Estado no cliente | A thread de recepção escreve o snapshot enquanto a thread de desenho lê | Quadro desenhado com metade do estado velho e metade novo | Lock curto: a recepção troca a referência do snapshot e o desenho copia a referência |

Trecho central (caso 2):

```python
def comprar_torre(p, jog, tipo, x, y):
    with p.lock_economia:                 # seção crítica
        custo = CUSTO_TORRE[tipo]
        if p.jogadores[jog].dinheiro < custo:
            return "X1D"                  # erro: dinheiro insuficiente
        p.jogadores[jog].dinheiro -= custo
        p.jogadores[jog].torres.append(Torre(tipo, x, y))
    return None
```

## 6. Notação de mensagens

TCP com texto ASCII e uma mensagem por linha (terminada em `\n`), porque o TCP é um fluxo contínuo
de bytes. A leitura acumula os bytes num buffer e separa as mensagens pelas quebras de linha.

**Formato:** `<origem><comando><argumentos>`. A origem é `0` para o servidor, e `1` ou `2` para
os jogadores.

| Mensagem | Sentido | Significado |
|---|---|---|
| `1J` | cliente para servidor | Pedido de entrada na partida |
| `0J1` | servidor para cliente | Você é o jogador 1 |
| `0I` | servidor para todos | Início da partida (estado `EM_JOGO`) |
| `1T2@08,04` | cliente para servidor | Jogador 1 coloca torre tipo 2 (Canhão) na célula 8,4 |
| `1U05` | cliente para servidor | Jogador 1 faz upgrade da torre de id 05 |
| `1V05` | cliente para servidor | Jogador 1 vende a torre 05 |
| `2S3x10` | cliente para servidor | Jogador 2 envia 10 balões tipo 3 (Verde) à trilha do adversário |
| `0R07` | servidor para todos | Começou a rodada 7 |
| `0E1234\|650,12,98\|420,9,87\|1:1,0.42;1:3,0.10;2:2,0.87` | servidor para todos, 10 vezes por segundo | Snapshot: tick 1234; J1 com 650 de dinheiro, 12 de renda e 98 vidas; J2 com 420, 9 e 87; balões no formato trilha:tipo,progresso |
| `0A1T2@08,04#05` | servidor para todos | Confirmação: torre 05 criada para o jogador 1 |
| `0X1D` | servidor para jogador | Erro do jogador 1: dinheiro insuficiente (`D`), posição inválida (`P`), em espera (`C`) |
| `1P` / `0P` | nos dois sentidos | Heartbeat a cada 2 s; 5 s sem resposta = desconexão |
| `0F2` | servidor para todos | Fim de jogo, jogador 2 venceu |

## 7. Telas e interface

1. **Menu:** logo, campo de IP e porta, botões "Hospedar" (sobe o servidor e conecta) e
   "Conectar".
2. **Lobby:** "Aguardando oponente..." com animação simples; mostra jogador 1 e jogador 2
   conectados.
3. **Partida:**
   - Esquerda (70%): sua trilha em tamanho grande, com grade para posicionar torres, alcance
     da torre ao passar o mouse e a torre fantasma antes de confirmar.
   - Direita (30%): miniatura da trilha do oponente, ao vivo.
   - HUD no topo: dinheiro, renda (+12), suas vidas e as do oponente, rodada e timer da próxima
     rodada.
   - Painel inferior de torres com ícone, custo e tecla (1, 2, 3).
   - Painel lateral de envio de balões com botões de custo, espera visível e aumento de renda.
   - Log das últimas mensagens trocadas (liga e desliga com F1). Ótimo para o vídeo e para
     mostrar a comunicação ao professor.
4. **Fim:** "VITÓRIA" ou "DERROTA", estatísticas (balões estourados e enviados, dinheiro gasto) e
   botão de revanche.

## 8. Stack e assets

- **Linguagem:** Python 3.12.
- **Gráfico:** `pygame-ce`.
- **Rede e concorrência:** `socket` (TCP) e `threading` (Lock, Thread) da biblioteca padrão, com
  socket puro, como exige o enunciado.
- **Executável:** `pyinstaller --onefile` (para o upload "com executável").
- **Assets:** Kenney.nl, pacote "Tower Defense (Top-Down)", licença CC0 (livre para uso). Os
  balões podem ser círculos coloridos com brilho desenhados pelo próprio pygame. Não usar sprites
  oficiais da Ninja Kiwi.
- **Alternativa em C:** se a dupla preferir partir do código base do professor, use raylib para
  a parte gráfica, winsock2 para a rede e `pthread_mutex_t` no lugar dos locks. A arquitetura e a
  notação continuam iguais.

Estrutura de pastas:

```
bloons-battles/
  servidor/  servidor.py  partida.py  simulacao.py  protocolo.py
  cliente/   cliente.py   rede.py     telas/ (menu, lobby, partida, fim)  render.py
  comum/     protocolo.py (parse e montagem das mensagens)  constantes.py
  assets/    sprites/  sons/
  README.md
```

## 9. Cronograma até 15/11

| Semana | Entrega |
|---|---|
| 29/09 a 05/10 | Protocolo em `protocolo.py` com testes; servidor aceita 2 clientes; lobby funcionando |
| 06/10 a 12/10 | Game loop no servidor: trilha, balões se movendo, snapshot `0E` |
| 13/10 a 19/10 | Cliente pygame desenha o snapshot; colocar torre (`T`) com validação e locks |
| 20/10 a 26/10 | Tiros, estouro de camadas, dinheiro, vidas, fim de jogo |
| 27/10 a 02/11 | Envio de balões (`S`), renda, rodadas, upgrade e venda |
| 03/11 a 09/11 | Interface final, sprites, sons, log F1, tratamento de desconexão |
| 10/11 a 15/11 | Testes, pyinstaller, vídeo de 1 min, formulário. **Entregar até dia 14** por segurança |

## 10. Roteiro do vídeo (1 min)

- **0 a 10 s:** menu; o jogador 1 hospeda e o jogador 2 conecta (duas janelas lado a lado).
- **10 a 30 s:** colocar torres, estourar a primeira rodada e mostrar o log F1 com as mensagens
  `1T2@08,04` e `0A...`.
- **30 a 45 s:** jogador 2 envia balões verdes; eles aparecem na trilha do jogador 1 e a renda
  sobe.
- **45 a 60 s:** vidas caindo, tela de vitória e derrota.

## 11. Rascunho das respostas do formulário

**Como e quais informações são trocadas entre os processos?**

> O jogo usa arquitetura cliente/servidor com sockets TCP. Cada mensagem é uma linha de texto no
> formato `<origem><comando><argumentos>`, com origem 0 = servidor e 1 ou 2 = jogador. Os clientes
> enviam intenções: colocar torre (`1T2@08,04`), upgrade (`1U05`), venda (`1V05`) e envio de
> balões ao oponente (`2S3x10`). O servidor, que é autoritativo, valida cada intenção e responde
> com confirmação (`0A`) ou erro (`0X1D`). Dez vezes por segundo ele transmite um snapshot (`0E`)
> com dinheiro, renda, vidas e posição dos balões dos dois jogadores. Há também mensagens de
> controle: entrada (`J`), início (`I`), rodada (`R`), heartbeat (`P`) e fim (`F`).

**Como é feito o gerenciamento da exclusão mútua?**

> O estado da partida fica numa estrutura compartilhada no servidor, acessada por várias threads:
> uma por cliente e uma do game loop. Usamos um lock para a economia (dinheiro, renda e vidas) e
> um lock por trilha de balões. A compra de torre verifica e debita o dinheiro dentro do mesmo
> lock, evitando que duas compras simultâneas deixem o saldo negativo. A inserção de balões
> enviados pelo oponente e a remoção de balões estourados pelo game loop são protegidas pelo lock
> da trilha. Para evitar deadlock, os locks são sempre adquiridos na mesma ordem (economia antes
> de trilha). No cliente, um lock curto protege a troca do snapshot entre a thread de rede e a
> thread de desenho.
