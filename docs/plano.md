# Bloons TD Battles: documento do trabalho

Trabalho 02 de Sistemas Operacionais (prof. Maurilio Campano Jr, ESOFT 4S).
Nome na planilha: **Bloons TD Battles (tower defense 1v1)**. Entrega: 15/11/2026 23:59.

## 1. O jogo

Tower defense inspirado no Bloons TD Battles 2. Bloons (balões) percorrem uma trilha e o jogador
coloca macacos (torres) para estourá-los. Tem dois modos:

- **Solo:** 4 mapas, 4 dificuldades (40, 60, 80 ou 100 rodadas), rodadas iniciadas pelo jogador,
  velocidade 1x/3x e rodada automática.
- **Batalha (2 jogadores em rede):** cada jogador defende a própria pista e gasta dinheiro para
  **enviar bloons ao oponente**. Cada envio aumenta a renda (eco), paga a cada 6 s. Perde quem
  zerar as 150 vidas primeiro.

Conteúdo:

| Item | Quantidade |
|---|---|
| Torres (macacos) | 22, com 3 caminhos de 5 upgrades cada (330 upgrades) e regra de caminhos cruzados |
| Heróis | 18, com níveis 1 a 20 e habilidades nos níveis 3 e 10 |
| Bloons | 17 tipos (vermelho ao B.A.D.), com camo, regeneração e fortificado |
| Rodadas | 100 |
| Envios do modo Batalha | 23 |
| Habilidades ativas | turbo, dano global, míssil em dirigível, congelar tudo, dinheiro, invocação etc. |

Toda a arte é desenhada por código (não há imagens externas).

Implementação em **C++17 com raylib** (janela, desenho e som), `std::thread`/`std::mutex` para
concorrência e sockets TCP nativos (Winsock2 no Windows, POSIX no Linux). O executável do
Windows é um `.exe` único, sem DLLs extras.

## 2. Arquitetura: lockstep determinístico

```
 [Cliente 1: raylib]                 [SERVIDOR]                  [Cliente 2: raylib]
  simula as 2 pistas   --comandos-->  ordena comandos  <--comandos--  simula as 2 pistas
                       <---ticks----  15 ticks/s       ----ticks--->
```

- O servidor **não simula** o jogo. Ele só recebe comandos, os coloca numa fila compartilhada e,
  15 vezes por segundo, transmite um **tick** com todos os comandos daquele intervalo.
- Cada cliente aplica os comandos do tick na mesma ordem e avança 2 passos da simulação
  (30 passos/s). A simulação é **determinística** (sem relógio, sem aleatório sem semente),
  então os dois computadores chegam exatamente ao mesmo estado.
- A cada 45 ticks cada cliente envia um **hash** do estado. Se os hashes diferirem, o servidor
  avisa os dois (`0D`): detecção de dessincronia.
- A variável de controle pedida no enunciado (como a "vez") é o **estado da sala**
  (`AGUARDANDO`, `EM_JOGO`, `FIM`) e o **número do tick**, que existem nos dois processos e
  só avançam pelas mensagens do servidor.

Threads:

| Processo | Thread | Papel |
|---|---|---|
| Servidor | aceitar | aceita conexões |
| Servidor | uma por cliente | lê comandos e hashes (produtora da fila) |
| Servidor | relógio | fecha um tick a cada 1/15 s e transmite (consumidora da fila) |
| Cliente | rede-rx | recebe mensagens e as põe na fila local (produtora) |
| Cliente | rede-hb | heartbeat a cada 2 s |
| Cliente | principal (raylib) | consome a fila, simula e desenha |

## 3. Notação de mensagens

Uma mensagem por linha ASCII terminada em `\n`: `<origem><comando><argumentos>`.
Origem `0` = servidor, `1`/`2` = jogador.

| Mensagem | Sentido | Significado |
|---|---|---|
| `1Jquincy,prado` | C→S | entrar com o herói Quincy, sugerindo o mapa "prado" |
| `0J1` | S→C | você é o jogador 1 |
| `0I8231,prado,quincy,adora` | S→todos | início: semente, mapa, herói do J1, herói do J2 |
| `1Tdardo@230,250` | C→S | colocar Macaco Dardo em (230, 250) |
| `1U12:0` | C→S | upgrade da torre 12 no caminho 0 |
| `1V12` | C→S | vender a torre 12 |
| `1M12:3` | C→S | alvo da torre 12: 0 primeiro, 1 último, 2 perto, 3 forte |
| `1B12:0` | C→S | usar a habilidade 0 da torre 12 |
| `2Sr8` | C→S | jogador 2 envia "8 Vermelhos" ao oponente |
| `0K451\|1Tdardo@230,250\|2Sr8` | S→todos | tick 451 com os comandos a aplicar, em ordem |
| `1H450,123456` | C→S | hash do estado no tick 450 |
| `0D450` | S→todos | estados divergiram no tick 450 |
| `1F` | C→S | desistir |
| `0F2` | S→todos | fim de jogo, jogador 2 venceu |
| `0X1M` | S→C | erro do jogador 1 (M = mensagem inválida, S = sala cheia, O = fora de hora) |
| `1P` / `0P` | ambos | heartbeat (8 s sem nada = desconectado) |

O servidor valida a sintaxe de todo comando e recusa comando com origem diferente do jogador
daquela conexão (um cliente não consegue agir pelo outro).

## 4. Memória compartilhada e exclusão mútua

Memória compartilhada do servidor ([sala.cpp](../src/servidor/sala.cpp)), cada região
com seu próprio `std::mutex`:

| Região | Lock | Quem disputa | Condição de corrida evitada |
|---|---|---|---|
| Jogadores e estado da sala | `lock_sala` | threads de clientes conectando ao mesmo tempo | dois clientes recebendo o mesmo número; partida iniciada duas vezes; "fim" transmitido duas vezes |
| Fila de comandos | `lock_fila` | threads dos clientes (produtoras) e thread do relógio (consumidora) | comando perdido ou duplicado ao trocar a fila no meio de um `append`; comando entrando em dois ticks |
| Hashes por tick | `lock_hash` | as duas threads de cliente | os dois hashes chegando juntos e nenhum dos dois comparando (ou os dois) |
| Mapa de sockets | `lock_clientes_` | aceitar, clientes saindo, relógio transmitindo | iterar o dicionário enquanto outro remove |
| Envio por socket | um mutex por conexão | relógio e thread do cliente enviando ao mesmo socket | duas mensagens intercaladas no meio da linha |

No cliente ([conexao.cpp](../src/cliente/conexao.cpp)): a fila de mensagens recebidas e o log de
mensagens são protegidos por mutex entre a thread de rede e a thread da tela, e um mutex de envio
impede que o heartbeat e um comando se misturem no mesmo `send`.

Regras: seções críticas curtas (`std::lock_guard`, só troca de vetores, nunca I/O de rede com um
mutex de estado seguro) e nunca segurar dois locks de estado ao mesmo tempo, o que elimina deadlock.

Os testes provam isso: 8 threads disputam a sala 50 vezes e nunca repetem número; 2 produtoras
enfileiram 6.000 comandos enquanto a consumidora fecha ticks, e nenhum se perde, duplica ou troca
de ordem.

### Memória compartilhada entre processos (placar da sala)

Além das threads, há uma região de **memória compartilhada do sistema operacional**, com nome,
que vários **processos** da mesma máquina mapeiam ao mesmo tempo
([memoria.cpp](../src/ipc/memoria.cpp), [placar.cpp](../src/ipc/placar.cpp)):

| Sistema | Memória | Exclusão mútua entre processos |
|---|---|---|
| Windows | `CreateFileMapping` + `MapViewOfFile` (`Local\bloons_sala_<porta>`) | mutex com nome (`CreateMutex`) |
| Linux | `shm_open` + `mmap` (`/bloons_sala_<porta>`) | semáforo com nome (`sem_open`, valor inicial 1) |

A região guarda uma struct `Placar` de tamanho fixo (só tipos de largura fixa, para os bytes
significarem o mesmo em todos os processos):

| Quem | Escreve / lê | Campos |
|---|---|---|
| Processo que hospeda (servidor) | escreve | estado da sala, mapa, heróis, tick, total de comandos, vencedor |
| Cada cliente nesta máquina | escreve a própria linha a cada tick | vidas, dinheiro, eco, rodada, torres, bloons, estouros, hash, pid |
| `bloons_monitor` (terceiro processo) | só lê | mostra tudo no console a cada 0,5 s |

Cada acesso acontece dentro do mutex com nome (`Trava`, RAII). A seção crítica só copia bytes:
os valores são calculados antes de travar. O servidor nunca trava o placar enquanto segura um
mutex da `Sala` (primeiro copia da `Sala`, solta, depois publica), e publica **antes** de avisar
os clientes, para que quem recebe um tick já o encontre no placar. Isso elimina deadlock entre
processos.

Testes: dois mapeamentos da mesma região enxergam os mesmos bytes; 4 threads, cada uma com o seu
próprio mapeamento, fazem 5.000 incrementos cada "ler, somar, gravar" sob o mutex com nome sem perder
nenhum; e um teste abre o `bloons_monitor` como **outro processo** e confere que ele leu o placar
publicado pelo servidor.

## 5. Roteiro do vídeo (1 min)

- **0 a 10 s:** menu; o jogador 1 hospeda, escolhe mapa e herói; o jogador 2 entra pelo IP.
- **10 a 30 s:** colocar torres e upgrades com o painel F1 aberto, mostrando `1Tdardo@...` saindo
  e voltando dentro de `0K...`.
- **30 a 45 s:** enviar bloons ao oponente (aviso "Oponente enviou..." do outro lado) e abrir o
  mapa do oponente (tecla O).
- **45 a 60 s:** usar uma habilidade, vidas zerando, tela de vitória e derrota.

## 6. Respostas do formulário

**Como e quais informações são trocadas entre os processos?**

> Arquitetura cliente/servidor com sockets TCP, no modelo lockstep. Cada mensagem é uma linha de
> texto `<origem><comando><argumentos>` (origem 0 = servidor, 1 ou 2 = jogador). Os clientes
> enviam só as ações do jogador: colocar torre (`1Tdardo@230,250`), upgrade (`1U12:0`), venda
> (`1V12`), prioridade de alvo (`1M12:3`), habilidade (`1B12:0`) e envio de bloons ao oponente
> (`2Sr8`). O servidor põe as ações numa fila e, 15 vezes por segundo, transmite um tick com todas
> elas em ordem (`0K451|1Tdardo@230,250|2Sr8`). Os dois clientes aplicam os mesmos comandos no
> mesmo tick numa simulação determinística e chegam ao mesmo estado. Também trocamos entrada
> (`J`), início com semente, mapa e heróis (`I`), hash do estado para detectar dessincronia
> (`H`/`D`), fim de jogo (`F`), erros (`X`) e heartbeat (`P`).

**Como é feito o gerenciamento da exclusão mútua?**

> O servidor (C++, `std::thread`) tem uma memória compartilhada (classe `Sala`) acessada por várias threads: uma por
> cliente, que produz comandos, e a thread do relógio, que consome a fila a cada tick. Cada região
> tem seu `std::mutex`: `lock_sala` para entrada de jogadores e estado da partida, garantindo números
> únicos e início e fim uma só vez; `lock_fila` para a fila de comandos, cuja troca por uma lista
> vazia e o avanço do tick são atômicos, então nenhum comando se perde nem aparece em dois ticks;
> e `lock_hash` para a comparação de hashes. Há ainda um lock para o dicionário de sockets e um
> lock de envio por conexão, para mensagens não se intercalarem. Nunca seguramos dois locks de
> estado ao mesmo tempo, o que evita deadlock. No cliente, a fila de mensagens entre a thread de
> rede e a da tela também é protegida por mutex. Entre **processos**, o jogo usa memória
> compartilhada do sistema operacional com nome (`CreateFileMapping`/`MapViewOfFile` no Windows,
> `shm_open`/`mmap` no Linux) com um placar da sala: o processo que hospeda escreve o estado da
> partida, cada jogador na mesma máquina escreve a própria linha (vidas, dinheiro, rodada, hash) a
> cada tick, e um terceiro processo, o `bloons_monitor`, lê e mostra tudo. O acesso é protegido por
> um mutex com nome (`CreateMutex` no Windows, semáforo `sem_open` no Linux), com seção crítica que
> só copia bytes e sem nunca segurar esse mutex junto com os da sala. Testes automatizados com
> threads e processos concorrentes comprovam que não há perda, duplicação ou números repetidos.
