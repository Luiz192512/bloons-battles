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
| Tab | prioridade de alvo |
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

Opções de linha de comando para conferir o visual:

| Opção | O que faz |
|---|---|
| `--vitrine [0-10]` | galeria de sprites; a página 9 toca as animações e mostra as curvas de cada clipe |
| `--demo solo` / `--demo batalha` | partida local já montada, com torres, upgrades e bloons |
| `--tela solo\|hospedar\|entrar\|entrar-erro\|lobby` | abre direto uma tela de menu |
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

## Testes

```bash
ctest --test-dir build --output-on-failure
```

São 23 testes: protocolo, dados das torres, regras de upgrade, simulação e determinismo,
exclusão mútua com threads concorrentes e o servidor com clientes TCP reais.

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
