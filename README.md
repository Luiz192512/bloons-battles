# Bloons TD Battles

Trabalho 02 de Sistemas Operacionais (ESOFT 4S, prof. Maurilio Campano Jr): tower defense
inspirado no Bloons TD Battles 2, com modo solo e modo Batalha em rede usando sockets TCP,
threads, memória compartilhada e exclusão mútua.

- **C++17 + [raylib](https://www.raylib.com/)**, com `std::thread` e `std::mutex` para a
  concorrência e sockets nativos (Winsock2 no Windows, POSIX no Linux).
- 22 torres com 3 caminhos de 5 upgrades, 18 heróis, 17 tipos de bloon e 100 rodadas.
- Modo Batalha 1v1 em lockstep: o servidor ordena os comandos e cada cliente simula as duas
  pistas. A simulação é determinística, inclusive entre Windows e Linux.
- Arte e sons gerados por código, sem imagens nem áudio externos. As fontes vão embutidas no
  executável.

Design completo, notação das mensagens e respostas do formulário: [docs/plano.md](docs/plano.md).

## Como jogar

Baixe `BloonsBattles.exe` na página
[Releases](https://github.com/Luiz192512/bloons-battles/releases) e abra. É um arquivo só: não
precisa instalar nada (nem o Visual C++ Redistributable).

- **Solo:** "Jogar Solo", escolha mapa, dificuldade e herói.
- **Batalha no mesmo PC:** abra o jogo duas vezes. Na primeira, "Batalha: Hospedar". Na
  segunda, "Batalha: Entrar" com IP `127.0.0.1`.
- **Batalha em dois PCs:** quem hospeda vê o próprio IP na sala de espera e o outro digita esse
  IP. Os dois precisam estar na mesma rede.
- **Coop em LAN:** em "Batalha: Hospedar", clique em **Modo: Coop** antes de hospedar. O outro
  jogador entra normalmente por "Batalha: Entrar" e o modo vem do anfitrião. Os dois defendem a
  mesma pista, com vidas e dinheiro comuns; qualquer um coloca, melhora e vende qualquer torre, e
  qualquer um inicia a rodada com Espaço. Só há um herói (o de quem hospeda). Não há velocidade 3x
  nem rodada automática. Se o parceiro sai, a partida termina ("Parceiro saiu").

### Firewall do Windows (dois PCs)

No PC que **hospeda**, na primeira vez o Windows pergunta se o jogo pode usar a rede: marque
**Redes privadas** e clique em Permitir. Se a pergunta não aparecer ou o oponente não conseguir
conectar, rode `scripts\liberar_firewall.bat` como administrador (clique com o botão direito >
Executar como administrador). O script libera a porta TCP 5050. Confira também se a rede Wi-Fi
está marcada como **Privada** nas configurações do Windows.

## Controles

| Tecla | Ação |
|---|---|
| Clique no card / Q W E R T Y Z X C V B N M A S D F G H J K L | escolher torre |
| U | herói |
| Clique no mapa (Shift mantém a torre selecionada) | colocar |
| Clique na torre | abrir upgrades |
| `,` `.` `/` | upgrade nos caminhos 1, 2 e 3 |
| Tab | prioridade de alvo. Em algumas torres troca outra coisa: trava a mira da Dartling, pede o ponto do Morteiro, troca a rota do Ás e o modo de voo do Heli, saca o Banco |
| Botão ao lado do alvo | Bumerangue: mão do arremesso. Sub com Submergir: Tona ou Fundo. Robô Macaco: alvo do segundo braço. Engenheiro com Overclock: torre que recebe a habilidade (clique nela) |
| Cursor sobre a banana ou a caixa | coletar o dinheiro caído |
| F2 | no Sandbox, alterna a loja e o painel de envio de bloons |
| Backspace | vender |
| 1 a 9 | habilidades |
| Espaço | iniciar rodada / acelerar (solo) |
| O | ver o mapa do oponente (batalha) |
| F1 | painel de mensagens trocadas (batalha) |
| F3 | mostrar FPS |
| F9 / Shift+F9 | repetir a animação de disparo / habilidade da torre selecionada (só visual) |
| F12 | salvar captura de tela (`captura_AAAAMMDD_HHMMSS.png`) |
| Esc | cancelar / menu |

## Visual e ferramentas de arte

O visual segue o projeto do Claude Design (sprites e auditoria de interface). Tudo é desenhado
por código: os sprites em `src/cliente/sprites.cpp` (desenhados com a caneta de
`src/cliente/caneta.cpp` e guardados em RenderTextures por `arte.cpp`) e as animações de disparo
e habilidade em `src/cliente/anim.cpp`. O inventário do que foi aplicado está em
[docs/design/inventario.md](docs/design/inventario.md), com capturas em `docs/design/capturas/`.

As torres e os 18 heróis também têm modelo 3D (`assets/modelos/<chave>/<a>-<b>-<c>.glb`, gerados
por script no Blender, ver [tools/blender/LEIAME.md](tools/blender/LEIAME.md)). O jogo desenha o
modelo quando a pasta `assets/modelos` está ao lado do executável e cai no sprite 2D quando não
está.

Opções de linha de comando para conferir o visual:

| Opção | O que faz |
|---|---|
| `--vitrine [0-11]` | galeria de sprites; a página 9 toca as animações com as curvas de cada clipe e a 11 mostra a mira 3/4 |
| `--fps` | mostra o FPS desde o início |
| `--demo solo` / `--demo batalha` | partida local já montada, com torres, upgrades e bloons |
| `--tela solo\|macacos\|hospedar\|entrar\|entrar-erro\|lobby` | abre direto uma tela de menu |
| `--sandbox [mapa] [heroi=chave] [comandos]` | partida Sandbox já com os comandos aplicados (ex.: `Tsuper@600,500 U1:1 Xb:ceramica:20`) |
| `--sel id` / `--up caminho` / `--bloons` | seleciona a torre, abre a confirmação de upgrade ou o painel de bloons |
| `--captura arquivo.png [s] [n] [intervalo]` | salva n capturas depois de s segundos e fecha |

## Compilar

Precisa de CMake 3.16+, Git e um compilador C++17. A raylib 5.5 é baixada e compilada
automaticamente na primeira vez.

**Windows.** Se o PC ainda não tem as ferramentas, instale tudo com o winget (Git, CMake e
Visual Studio 2022 Build Tools com C++, cerca de 3 GB):

```bat
scripts\instalar_dependencias.bat
```

Depois compile. O script acha o Visual Studio sozinho, mesmo sem o CMake no PATH:

```bat
scripts\compilar.bat
```

O executável sai em `build\Release\BloonsBattles.exe` (ou `build\BloonsBattles.exe` com MinGW) e
já leva o runtime do C++ embutido, então roda em qualquer PC com Windows 10 ou 11.

**Linux** (Ubuntu/Debian):

```bash
sudo apt install build-essential cmake git libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
```

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

```bash
cmake --build build --parallel
```

```bash
./build/BloonsBattles
```

Servidor avulso, sem interface (opcional):

```bash
./build/bloons_servidor 5050
```

Monitor da sala (outro processo, no console): lê o placar que o servidor e os jogadores
publicam em memória compartilhada. Precisa rodar no mesmo PC de quem hospeda:

```bash
./build/bloons_monitor 5050
```

## Requisitos do trabalho: onde cada um é cumprido

| Requisito do enunciado | Onde está no projeto | Como conferir |
|---|---|---|
| **R1. Pelo menos 2 usuários, cada um em um processo distinto** | Cada jogador abre o próprio `BloonsBattles.exe`; o servidor roda dentro de quem hospeda ou separado (`bloons_servidor`). Modo Coop em LAN. | Abra o jogo duas vezes ([Como jogar](#como-jogar)). O teste `servidor_partida_completa` usa dois clientes TCP reais |
| **R2. Comunicação por socket, sem biblioteca de rede de alto nível** | [src/rede/socket.cpp](src/rede/socket.cpp): `socket`, `bind`, `listen`, `accept`, `connect`, `send`, `recv` direto (Winsock2 no Windows, POSIX no Linux). Servidor em [src/servidor/servidor.cpp](src/servidor/servidor.cpp), cliente em [src/cliente/conexao.cpp](src/cliente/conexao.cpp) | Painel de mensagens trocadas na partida (tecla F1) |
| **R3. Memória compartilhada com exclusão mútua** | **Entre threads:** classe `Sala` em [src/servidor/sala.hpp](src/servidor/sala.hpp) com `lock_sala_`, `lock_fila_` e `lock_hash_` (`std::mutex`). **Entre processos:** [src/ipc/memoria.cpp](src/ipc/memoria.cpp) (`CreateFileMapping` + `CreateMutex` no Windows, `shm_open` + `sem_open` no Linux) e o placar em [src/ipc/placar.cpp](src/ipc/placar.cpp), lido pelo `bloons_monitor` | `bloons_monitor 5050` com uma partida aberta. Testes `mutex_*` e `ipc_*`. Explicação completa em [docs/plano.md](docs/plano.md), seção 4 |
| **R4. Ações por códigos nas mensagens; estado da "vez" nos dois processos** | Cada ação é um código na mensagem (`T` torre, `U` upgrade, `V` venda, `M` alvo, `B` habilidade, `S` envio, `N` rodada, `A` mira, `O` opção, `C` coleta). O estado da sala (`AGUARDANDO`, `EM_JOGO`, `FIM`) e o número do tick existem no servidor e nos dois clientes e só avançam pelas mensagens `0K...` (lockstep, 15 ticks/s). Hash do estado a cada 3 s detecta dessincronia | [docs/plano.md](docs/plano.md), seção 2. Testes `sim_determinismo_*` e `servidor_*` |
| **R5. Notação própria de mensagens** | Uma linha ASCII `<origem><comando><argumentos>`, por exemplo `1Tdardo@230,250` e `0K451\|1Tdardo@230,250\|2Sr8`. Definida e validada em [src/comum/protocolo.hpp](src/comum/protocolo.hpp) e [protocolo.cpp](src/comum/protocolo.cpp) | Tabela completa em [docs/plano.md](docs/plano.md), seção 3. Testes `protocolo_*` |
| **R6. Linguagem aceita** | C++17 (com raylib só para janela, desenho e som) | [CMakeLists.txt](CMakeLists.txt) |
| **R7. A interface não pode consumir o tempo de R2 a R5** | Arte e sons gerados por código ([src/cliente/](src/cliente/)); rede, threads e memória compartilhada são módulos separados da interface ([src/rede](src/rede), [src/servidor](src/servidor), [src/ipc](src/ipc)) e têm testes próprios | `ctest` roda tudo sem abrir janela |
| **R8. Jogo diferente do de outra dupla** | Tower defense inspirado no Bloons TD Battles 2, com 22 torres, 18 heróis, envio de bloons e modo Coop | Decisão do professor |
| **R9. Entrega até 15/11/2026 23:59, vídeo de 1 min, código com e sem executável** | Roteiro do vídeo em [docs/plano.md](docs/plano.md) seção 5; zips gerados por `scripts\publicar_release.bat` ou `scripts/empacotar.sh` ([Entrega](#entrega)) | Release do GitHub com `BloonsBattles.exe`, `BloonsBattles_codigo.zip` e `BloonsBattles_com_executavel.zip` |

Respostas do formulário ("Como e quais informações são trocadas entre os processos?" e "Como é feito o gerenciamento da exclusão mútua?"): [docs/plano.md](docs/plano.md), seção 6.

## Testes

```bash
ctest --test-dir build --output-on-failure
```

São 65 testes: protocolo (inclusive o coop), dados das torres, regras de upgrade, simulação e determinismo,
exclusão mútua com threads concorrentes, o servidor com clientes TCP reais e a memória
compartilhada entre processos (inclusive abrindo o `bloons_monitor` como processo separado).

## Entrega

**No Windows**, o script abaixo compila, roda os testes, gera os dois zips de entrega em
`entrega/` e publica tudo numa Release do GitHub (usa o GitHub CLI, não depende do GitHub Actions):

```bat
scripts\publicar_release.bat v1.0
```

**No Linux ou no WSL** (`sudo apt install mingw-w64 zip`):

```bash
scripts/empacotar.sh
```

Gera em `entrega/` o zip só com o código e o zip com os executáveis do Windows.

## Estrutura

```
src/
  comum/     protocolo.hpp/.cpp (notação das mensagens)
  rede/      socket.hpp/.cpp (TCP igual no Windows e no Linux)
  ipc/       memoria.cpp (memória compartilhada com nome + mutex com nome, entre processos),
             placar.cpp (placar da sala), main_monitor.cpp (bloons_monitor)
  servidor/  servidor.cpp (threads aceitar, cliente e relógio), sala.cpp (memória compartilhada + mutex),
             main_servidor.cpp
  jogo/      sim.cpp (simulação determinística), stats.cpp (upgrades), dados.cpp (torres, heróis,
             bloons, mapas e rodadas), mapas.cpp, rodadas.cpp, defs.cpp
  cliente/   app.cpp (janela e cenas), cena_jogo.cpp, cenas_menu.cpp, render.cpp, arte.cpp,
             ui.cpp, controle.cpp (solo e batalha), conexao.cpp (threads de rede), som.cpp
tests/       testes.cpp
assets/      fontes (embutidas no executável na compilação)
third_party/ nlohmann/json (efeitos dos upgrades, licença MIT)
cmake/       embutir.cmake, mingw-w64.cmake
scripts/     compilar.bat, instalar_dependencias.bat, ambiente.bat, publicar_release.bat,
             empacotar.sh, liberar_firewall.bat
docs/        plano.md, prompt-escolha-do-jogo.md
```
