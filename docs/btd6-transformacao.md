# Transformação do clone em Bloons TD 6

Este documento registra a troca da base do clone, do Bloons TD Battles 2 (BTDB2) para o Bloons TD 6 (BTD6).

**Como foi feito:**
- **Prompt:** `prompts/transformar-em-btd6.md`.
- **Dados:** vieram do BTD6 atual e foram aplicados em `src/jogo/`.
- **Modo Batalha:** continua existindo, com os mesmos envios e a mesma eco.

## Versão e fontes

A pesquisa foi feita em 30/09/2026, com os dados que a Blooncyclopedia mostra hoje.

- **[B1] Blooncyclopedia**, lida pela API em 30/09/2026 (bloonswiki.com):
  - Páginas "(BTD6)" de 27 torres, 391 upgrades (custo por upgrade na predefinição "BTD6 upgrade info"), 18 heróis e 25 bloons.
  - Rodadas oficiais: `JSON:Bloons TD 6/Rounds/DefaultRoundSet`.
  - Renda por rodada: `JSON:Bloons TD 6/Rounds/DefaultIncomeSet`.
- **[F1] Bloons Wiki (fandom)**:
  - Status base das torres: infobox das páginas "Nome (BTD6)".
  - Aumento de vida e velocidade depois da R80: "Late Game and Freeplay (BTD6)".

## Diagnóstico por sistema

| Sistema | Clone antes | BTD6 real | Situação |
|---|---|---|---|
| Dificuldades | Fácil 200 vidas ×0,85 até R40; Médio 150 ×1,0 R60; Difícil 100 ×1,08 R80; Impossível 1 ×1,2 R100 | Igual | Já batia |
| Dinheiro inicial e bônus de rodada | $650; $100 + rodada | Igual | Já batia |
| Dinheiro por estouro | $1 sempre | ×1 até R50; ×0,5 até R60; ×0,2 até R85; ×0,1 até R100; ×0,05 até R120; ×0,04 até R140; ×0,02 depois | **Aplicado** |
| Rodadas | Conjunto próprio de 1 a 100 + gerador | 140 rodadas oficiais do BTD6 | **Aplicado** (140 rodadas exatas; gerador só depois da R140) |
| Vida e velocidade depois da R80 | Nenhuma rampa | Dirigíveis +2%/rodada (R81 a 100), +5% (101 a 124), +15% (125 a 150), +35% (151+); velocidade +2%/rodada a partir da R81 e salto para 1,6× na R101 | **Aplicado** no modo solo |
| Bloons | Quase igual ao BTD6 | Chumbo também imune a energia; DDT com 4 cerâmicas, imune a afiado, gelo, energia e explosão, velocidade 2,64× | **Aplicado** |
| Custos das torres | Valores do BTDB2 misturados | Ex.: Bomba $375, Mago $250, Morteiro $600, Engenheiro $350 | **Aplicado** (22 torres) |
| Custos dos upgrades | Valores próprios | 330 custos oficiais | **Aplicado** (330 upgrades) |
| Status base das torres | Parcial | Ex.: Tachinha a cada 1,12 s; Mago pierce 3; Ninja 0,62 s; Espinhos a cada 1,75 s com duração de 50 s; explosão do Morteiro com 2 de dano e pierce 25 | **Aplicado** |
| Efeitos dos upgrades | Números do BTDB2 ou inventados | Números da wiki para cada upgrade | **Aplicado** onde o motor expressa (ver pendências) |
| Heróis | 18 (Jericho é do BTDB2) | 17 do clone existem no BTD6; Dan D'Monke falta | Custos e escala de XP **aplicados**; Jericho vira pendência |
| Mapas | 4 próprios | Mapas do BTD6 por categoria | Mantidos (arte própria) |

## O que foi aplicado

- **Custos:** base das 22 torres, 330 upgrades e 17 heróis (script sobre a predefinição da wiki).
- **Rodadas:** as 140 do BTD6 viraram `RODADAS` em `dados.cpp`, com o espaçamento de cada grupo calculado pela duração oficial.
- **Economia:**
  - O dinheiro por estouro segue a tabela oficial por rodada, só no modo solo (`mult_renda_da_rodada`).
  - Depois da R80, a vida dos dirigíveis e a velocidade dos bloons sobem pela rampa do freeplay (`mult_vida_moab`, `mult_velocidade`), também só no solo.
- **Bloons:** chumbo e DDT corrigidos como na tabela acima.
- **Torres:** cada upgrade foi reescrito com os números da wiki. Alguns exemplos:
  - O Dardo 3-5 tem 8 de dano, pierce 8, alcance 80 e recarga ×0,5.
  - O Bumerangue 1-5 ganha uma aura de 4 de dano a cada 0,05 s em até 200 bloons.
  - A Bomba 2-5 dá +99 em MOAB e 4.500 na habilidade a cada 10 s.
  - O Bucaneiro 3-3 a 3-5 rende $200, $500 e $800 por rodada.
  - A Fazenda vai de $80 a $7.000 por rodada no caminho 1, e o 3-5 dá +$4.000 por rodada.
  - O Engenheiro 3-4 prende até 500 de RBE.
- **Heróis:** escala de XP por herói (1,0; 1,425; 1,5; 1,71) e o ataque base da Gwendolin (0,5 s, alcance 38).
- **Nomes:** caminho 3 da Fábrica de Espinhos e Druida 1-5 ("Monarca das Tempestades") agora seguem a ordem e os nomes do BTD6.

## Mecânicas novas no motor

1. **Upgrade com lista de efeitos:** um upgrade pode aplicar vários efeitos em ataques diferentes (por exemplo, o 2-3 do Bucaneiro troca o dardo por bomba e dá +2 de dano às uvas).
2. **Mira de upgrade por tipo ou visual:** `"a": "uva"`, `"a": "renda"`, `"a": "queda"`. Isso substitui os índices fixos e **corrige os bugs #1 a #5** de `docs/analise-mecanicas.md` (Energizer, Mestre Bombardeiro, Macacópolis, Bucaneiro, Druida).
3. **Remover camo e regen não depende mais da imunidade:** Signal Flare, Shimmer e a espuma pegam o DDT, como no BTD6 (bug #6).
4. **Teleguiado no ataque radial:** a Mira Infalível do Ás funciona (bug #7).
5. **Armadilha por RBE:** a Bloon Trap engole bloons inteiros até encher a capacidade em RBE e paga $ por RBE (+100%; a XXXL paga +200% e prende dirigíveis, menos o BAD).
6. **Empréstimo com dívida:** o IMF Loan dá $9.000 e metade da renda seguinte paga a dívida.
7. **Lead to Gold:** paga $50 por chumbo estourado (`ouro_chumbo`).
8. **Rampa de freeplay** (vida e velocidade) e **renda por estouro por rodada**.

## Pendências

Ficaram de fora porque não há fonte com o número ou porque o motor ainda não tem a mecânica:

- **Críticos:** Dardo 3-4 e 3-5 e Super 2-3 ganharam só o dano normal.
- **Transformar torres:** Fan Club (Dardo 2-4 e 2-5) e Total Transformation viraram turbo de velocidade em área.
- **Sacrifícios:** Sun Temple e True Sun God, e a Monkeyopolis, que sacrifica fazendas. Esta última ficou sem efeito.
- **Passivos de vazamento:**
  - Legend of the Night (Super 3-5) ficou sem efeito.
  - A passiva da Bomb Blitz virou a habilidade de dano global que já existia.
- **Buffs com escopo por categoria:**
  - Vila 1-3 a 1-5 dá o buff para todas as torres, não só as Primárias.
  - Druida 3-4 dá o buff para todas as torres, não só os Druidas.
  - O Carrier Flagship dá o buff para as torres no alcance, não só as de água.
  - O bônus do Elite Sniper para todos os Snipers não foi aplicado.
- **Mecânicas sem suporte:**
  - Coleta manual de bananas e juros reais do Monkey Bank (virou ×1,15 na renda).
  - Mira no cursor do Dartling e do Morteiro.
  - Cemitério do Necromante.
  - Shrink Potion (Alquimista 3-5 sem efeito).
  - Rota Centralizada (Ás 3-2 sem efeito).
  - Smart Spikes (Espinhos 3-2 sem efeito).
  - Velocidade do Heli (2-1 sem efeito).
  - Mini-Comanches (Heli 3-4 e 3-5 viraram só dano).
  - Aumento de 20% de renda da Monkey City.
  - +15 vidas do Wall Street.
- **Números aproximados**, com a direção certa mas sem o número exato na fonte:
  - Distraction do Ninja.
  - Tornado e supertempestade do Druida.
  - Pop and Awe (160 de dano total).
  - Cooldown do Pre-emptive Strike.
  - Stun do Relentless Glue.
  - Unstable Concoction (20 de dano em MOAB).
  - Recarga de algumas habilidades que a wiki não listava (mantidas como estavam).
- **Buffs de alquimista e vila** continuam permanentes e acumulando entre si (no BTD6 o Berserker Brew é temporário e por torre).
- **Heróis:** os níveis 2 a 20 e as habilidades seguem o molde genérico do clone. Jericho é do BTDB2 e não existe no BTD6. Dan D'Monke falta porque precisa de arte.
- **Torres novas do BTD6** (Beast Handler, Desperado, Mermonkey, Skywarden) não foram criadas porque precisam de arte e mecânicas próprias.
- **Super Cerâmicas depois da R80** (vida maior e menos filhos) não foram aplicadas.
- **Modos do BTD6:** CHIMPS, Half Cash e Deflate não foram criados; só as 4 dificuldades.
- **Categorias de dificuldade dos mapas:** o jogo usa Beginner, Intermediate, Advanced e Expert. Os 4 mapas do clone continuam com os rótulos próprios.

## Verificação

Comandos:

```
cmake -S . -B build -DBLOONS_CLIENTE=OFF && cmake --build build
./build/bloons_testes
```

Resultado:

- **Testes:** `27 ok, 0 falha(s)`.
- **Cliente:** `src/cliente/vitrine.cpp` compila com os headers da raylib (`g++ -fsyntax-only`). O build do cliente não roda neste ambiente por falta das bibliotecas de janela do sistema.
- **Partidas automáticas** com um robô simples em quatro mapas e dificuldades, no modo solo e no modo batalha, rodaram sem erro. Uma rodada de estresse com dinheiro ilimitado chegou à rodada final; o resultado está na descrição do commit.
