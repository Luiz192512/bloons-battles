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
| Heróis | 18 (Jericho é do BTDB2) | 17 do clone existem no BTD6; Dan D'Monke falta | **Aplicado**: níveis 2 a 20, habilidades e XP do BTD6; Jericho trocado por Dan D'Monke |
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
9. **Crítico a cada N tiros** (`crit_cada`, `crit_max`, `crit_dano`, `crit_mais`), com contador por ataque na torre e sorteio pelo gerador da pista (determinístico). Fonte [B1], páginas "Sharp Shooter (BTD6)", "Crossbow Master (BTD6)" e "Robo Monkey (BTD6)":
   - Dardo 3-4: a cada 10º tiro, crítico de 50 de dano no lugar do normal.
   - Dardo 3-5: a cada 5º tiro, crítico de 80.
   - Super 2-3: a cada 15 a 20 tiros, crítico com +9 de dano.
10. **Buffs com escopo e sem acúmulo** (`escopo`, `global_`, `sem_si`, `acumula` nos buffs). Fontes do mesmo tipo de torre não somam; vale o melhor valor de cada campo. Fonte [B1], páginas das upgrades citadas:
    - Vila 1-3 a 1-5: o buff de Primárias (+1 pierce e +10% de alcance, +5 de alcance no 1-4, +3 pierce no 1-5) só vale para torres Primárias. Jungle Drums continua para todas.
    - Druida 3-4 (Poplust): +15% de velocidade e de pierce só nos outros Druidas, acumulando até 5 vezes.
    - Ninja 2-3 (Shinobi Tactics): Ninjas no alcance, inclusive ele, ×0,92 de recarga e +8% de pierce, acumulando até 20 vezes.
    - Bucaneiro 1-5 (Carrier Flagship): torres na água e Ases no mapa todo, ×0,8 de recarga. A página da wiki diz 20% no resumo e 15% na seção do buff; ficou 20%.
    - Sniper 2-5 (Elite Sniper): os outros Snipers do mapa atacam com ×0,75 de recarga.
11. **Poções do Alquimista por torre** (`pocao`, com tiros, segundos e bloqueio):
    - Acidic Mixture Dip (1-2): a cada 10 s, numa torre sorteada no alcance (prefere as sem a poção). Dá chumbo, +1 em cerâmica e +1 em M.O.A.B. por 10 ataques, acumulando até 30.
    - Berserker Brew (1-3): a cada 8 s, na torre mais próxima. Dá +1 de dano, +2 pierce, +10% de alcance e ×0,9 de recarga por 25 tiros ou 5 s; a torre só recebe outra depois de 5 s.
    - Stronger Stimulant (1-4): +1 de dano, +3 pierce, +15% de alcance, ×0,85 de recarga, por 40 tiros ou 12 s.
    - Permanent Brew (1-5): as poções novas ficam para sempre.
12. **Freeplay do BTD6 a partir da R81** (só no solo). Fonte [B1], "Ceramic Bloon (BTD6)", seção Super Ceramic Bloons, e "Freeplay":
    - As cerâmicas viram Super Cerâmicas: 60 de vida (120 fortificadas) e $87 ao estourar a camada, multiplicado pela renda da rodada.
    - Bloons que não são dirigíveis soltam um filho só; o vazamento em vidas segue essa regra.
    - Depois de vencer, a tela de vitória oferece "Continuar em freeplay", que segue sem última rodada.
13. **Heróis com os níveis 2 a 20 do BTD6** (páginas "<Herói> (BTD6)" da [B1]). Ataque principal, passivas e habilidades seguem as tabelas de nível, com alcance em unidades ×4 e raio de explosão ×2,5, como nas torres.
    - Os efeitos de nível podem mudar campos das habilidades (`h3` e `h10`), por exemplo o Rapid Shot do Quincy com 12 s no nível 13 e ×4 no 15, e a MOAB Barrage do Churchill com 19.200 por alvo e recarga de 30 s no 20.
    - Buffs dos heróis com escopo: Striker (Bombas e Morteiros do mapa, x0,9 e x0,81), Obyn (Druidas e torres Mágicas), Etienne (+10% e +20% de alcance, UAV com camo no mapa), Brickell (+1 pierce na água), Gwendolin (Heat It Up com +1 pierce e chumbo).
    - Habilidades novas no motor: `recarregar` (Artillery Command zera a recarga das Bombas e Morteiros), `sem_regen` (Heartstopper) e `pct` (MOAB Hex tira parte da vida máxima). O `turbo_area` agora aceita `buffs` temporários (Rallying Roar, Biohack, Naval Tactics, Long Arm of Light) e `n` (as torres mais próximas).
    - XP oficial (módulo "BTD6 hero xp" e página "Experience"): 180 a 17.280 por nível; no solo, só as rodadas dão XP (20r+20, 40r-380 e 90r-2880), ×1,1 a ×1,3 conforme o mapa, e o freeplay depois de vencer corta 70% até a R100 e 90% depois. Na Batalha continua a XP por estouro.
    - **Jericho saiu** (é do Battles 2) e entrou **Dan D'Monke** ($650, escala de XP 1,425), com a arte do Jericho e espada até ter arte própria. Decisão do dono em 30/09/2026.

## Pendências

Ficaram de fora porque não há fonte com o número ou porque o motor ainda não tem a mecânica:

- **Transformar torres:** Fan Club (Dardo 2-4 e 2-5) e Total Transformation viraram turbo de velocidade em área.
- **Sacrifícios:** Sun Temple e True Sun God, e a Monkeyopolis, que sacrifica fazendas. Esta última ficou sem efeito.
- **Passivos de vazamento:**
  - Legend of the Night (Super 3-5) ficou sem efeito.
  - A passiva da Bomb Blitz virou a habilidade de dano global que já existia.
- **Detalhes de buff ainda fora:**
  - Primary Training: +25% de velocidade do projétil nas Primárias.
  - Primary Mentoring e Expertise: tiers 1 e 2 grátis e recarga de habilidade 15% e 25% menor.
  - Berserker Brew com o cruzamento 3-2-0 (40 tiros ou 6 s).
  - AMD: +1 só em chumbo fortificado (o motor não separa chumbo fortificado de outros fortificados).
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
- **Heróis, aproximações** (a mecânica não existe no motor ou a wiki não dá o número):
  - Psi: a vibração psiônica destrói o bloon sem filhos depois de um tempo; aqui é dano alto (5, depois 7, 12 e 16, mais bônus em dirigível), com as restrições de alvo por tipo de dano.
  - Corvus: livro de feitiços e mana ficaram de fora; o espírito é um projétil teleguiado com recarga própria.
  - Geraldo: a loja não existe; as duas habilidades são itens da loja (Torreta e Armadilha de Lâminas).
  - Habilidades resumidas num golpe: Storm of Arrows, Dark Ritual, Sword Charge, Psionic Scream, Scatter Missile (soma dos mísseis) e MOAB Barrage (soma dos tiros).
  - Wall of Trees vira pilha com 2.500 (7.500 no 20) de pierce; Ball of Light e UCAV viram a Fênix invocada; Drone Swarm vira turbo.
  - Dan D'Monke: a segunda forma (Masqued Macaque), a rede, o escudo etéreo e a recuperação de vidas não entraram; Rabble Rouser dá +1 de dano em vez de ×1,5.
  - Passivas que dependem de sub-torre ou de mecânica própria: totem do Obyn, Pyrotechnics Expert da Gwendolin, Cyber Security e Trojan do Benjamin, Adrenaline Rush e granadas da Rosalia, Ice Walls e fragmentos do Silas, tapa do Pat.
  - Rapid Shot, Armor Piercing Shells e Transformation têm a duração do nível citado; o BTD6 soma 0,5 s por nível em algumas.
- **Torres novas do BTD6** (Beast Handler, Desperado, Mermonkey, Skywarden) não foram criadas porque precisam de arte e mecânicas próprias.
- **Freeplay, aproximações:**
  - Vazamento da Super Cerâmica: o motor conta 68 vidas (60 + arco-íris em fila); a wiki lista 65. O M.O.A.B. dá 472 contra 460.
  - As rodadas depois da R140 vêm de um gerador próprio; o BTD6 sorteia grupos de um conjunto fixo.
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
- **Partidas automáticas** (robô simples) sem erro em quatro mapas e nas 4 dificuldades do modo solo, e no modo batalha.
- **Estresse com dinheiro e vidas ilimitados:**
  - Solo no Impossível até a **rodada 170** (passa pelo freeplay depois da R140): 2,8 milhões de estouros, 11 s de execução.
  - Batalha na Encruzilhada até a **rodada 394**: 143 torres e 1.442 upgrades, sem travar.
