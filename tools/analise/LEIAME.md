# Ferramentas de análise e da transformação para BTD6

Estes são os scripts usados na sessão em nuvem. O objetivo é que outra conversa (local) continue o trabalho de onde ele parou.

## Estado atual (branch `luiz/sleepy-carson-royhhg`)

- `docs/analise-mecanicas.md`: análise do clone antes da transformação, com 16 bugs listados.
- `docs/analise-jogo-real.md`: análise do Bloons TD Battles 2 real e comparação com o clone.
- `prompts/analise-jogo-real-btdb2.md` e `prompts/transformar-em-btd6.md`: prompts usados.
- `docs/btd6-transformacao.md`: o que já foi aplicado do BTD6 e a **lista de pendências**. É por ela que a próxima conversa deve começar.
- Os testes (`bloons_testes`) passam: 35 ok.

## Arquivos

| Arquivo | Para que serve |
|---|---|
| `dump.cpp` + `md.inc` | Imprime status, métricas de dano/s, bloons, mapas e rodadas a partir do núcleo. Modos: `torres`, `cross`, `herois`, `bloons`, `mapas`, `rodadas`, `md`, `mdh` |
| `eco.cpp` | Dinheiro e RBE acumulados por rodada |
| `partida.cpp` | Robô que joga partidas automáticas. `partida <mapa> <dificuldade> <solo/batalha> [rico]`; o modo "rico" dá dinheiro e vidas infinitos, para teste de estresse |
| `fetch.py` | Baixa pela API MediaWiki as páginas que usam uma predefinição (por exemplo, "BTD6 upgrade info") da Blooncyclopedia |
| `show.py` | Mostra os textos dos upgrades de uma torre (espera o `torres.txt` gerado a partir do `fetch.py`) |
| `patch.py` | Aplica patches de upgrade em `src/jogo/dados.cpp` |
| `patches_*.txt` | Os patches do BTD6 já aplicados (servem de exemplo do formato) |

## Como compilar as ferramentas

```
cmake -S . -B build -DBLOONS_CLIENTE=OFF && cmake --build build
g++ -O2 -std=c++17 -Isrc -Ithird_party tools/analise/dump.cpp build/libbloons_nucleo.a -lpthread -o dump
g++ -O2 -std=c++17 -Isrc -Ithird_party tools/analise/partida.cpp build/libbloons_nucleo.a -lpthread -o partida
```

No Windows com Visual Studio (dentro de `scriptsmbiente.bat`), a biblioteca é `buildloons_nucleo.lib` e o runtime precisa ser estático:

```
cl /O2 /EHsc /std:c++17 /MT /D_USE_MATH_DEFINES /Isrc /Ithird_party toolsnalise\partida.cpp buildloons_nucleo.lib ws2_32.lib /Febuilderr\partida.exe
```

O robô aceita também as dificuldades novas (`chimps`, `metade`, `deflacao`).

## Formato dos patches (`python3 tools/analise/patch.py arquivo.txt`)

```
@torre p-t | descrição nova
{{"pierce", 1}}              <- efeito do upgrade, em C++, numa linha
@torre base
A("projetil", {...})         <- ataque base novo
```

Um efeito pode ser uma lista (`J::array({{...}}, {{...}})`) e pode mirar ataques pelo tipo ou pelo visual com `{"a", "uva"}`.

## Fontes (pela API, sem navegador)

- Blooncyclopedia: `https://www.bloonswiki.com/api.php`
  - Páginas "(BTD6)" e "(Battles 2)".
  - Rodadas oficiais: `JSON:Bloons TD 6/Rounds/DefaultRoundSet`.
- Bloons Wiki: `https://bloons.fandom.com/api.php`
  - Infobox das torres.
  - Página "Late Game and Freeplay (BTD6)".
