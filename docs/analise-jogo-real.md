# Análise de mecânicas e balanceamento: Bloons TD Battles 2 (jogo real)

**Versão analisada:** 4.13, a mais recente citada pela Blooncyclopedia [B0]. **Data da pesquisa:** 30/09/2026.

Mesma estrutura de `docs/analise-mecanicas.md` (a análise do clone), agora para o jogo real da Ninja Kiwi.

**Fontes:** Blooncyclopedia (bloonswiki.com), mais atualizada, usada como fonte principal; Bloons Wiki (bloons.fandom.com), para status base e upgrades sem página detalhada. A coluna **Conf.** diz de onde veio cada linha:

- **wiki**: número da Blooncyclopedia.
- **fandom**: número da Bloons Wiki, que pode estar desatualizado em algumas versões.
- **desc.**: só o texto do jogo, sem número.
- **n/d**: não encontrado.

Nenhum número foi inventado. Os valores das colunas "calc." foram calculados a partir dos números da fonte (dano × acertos ÷ recarga).

## 0. Principais achados

1. **O clone acerta a estrutura, mas erra muitos números e várias mecânicas-chave:**
   - **Cola** pega chumbo no jogo real.
   - **Alquimista** dá buff temporário em uma torre por vez.
   - **Bloon Trap** tem limite de 500 de RBE e não rende nada com bloons enviados.
   - **Necromante** reanima bloons de verdade.
   - **Morteiro e Dartling** miram onde o jogador escolhe.
   - **Buffs** que acumulam têm teto: Shinobi até 15×, Poplust até 5×, desconto no máximo 20%.
2. **Base econômica do Battles igual à do clone:** $650 iniciais, eco inicial de 250 a cada 6 s e 150 vidas.
3. **Economia diferente do clone:** os envios são **muito mais baratos** (MOAB $900 a 1.000, ZOMG $4.500, B.A.D. $15.000) e liberados bem antes (MOAB na R17, B.A.D. na R30). O jogo também tem **modificadores de envio** (regen ×1,6 na R8, camo ×2 na R12, fortificado ×2 na R18) e envios "Tight" em massa.
4. **Partida curta e com fim definido:** 40 rodadas normais e **Morte Súbita** até a 50 (o B.A.D. natural). Não há as 100 rodadas nem dificuldades de modo solo. O clone repete o modo clássico com 100 rodadas.
5. **Bloons diferentes:**
   - Chumbo, zebra e roxo são **mais rápidos** que no clone.
   - O chumbo também é imune a **energia**.
   - O DDT solta **4** cerâmicas (não 6) e é imune a quase tudo.
   - O B.A.D. tem **12.500** de vida (não 20.000).
   - Chumbo e cerâmica fortificados têm 6 e 30 de vida.
   - A vida dos dirigíveis **cresce a cada rodada depois da R25**.
6. **35 mapas, com dificuldade medida em "RBS"** (segundos que um bloon vermelho leva para atravessar), de 13,8 (Street Party) a 36,4 (Docks). O acesso a cada mapa depende da arena do jogador.
7. **Mecânicas feitas para o PvP**, que o clone não tem:
   - Seguros contra vazamento: Bomb Blitz, Legend of the Night, Elite Defender.
   - Escala com a pressão do oponente: Avatar of Wrath e Heart of Vengeance.
   - Renda alternativa por tempo ("alt-eco"): Supply Drop, Jungle's Bounty, Support Chinook.
   - Sabotagem do oponente: Jericho.
   - Sacrifício de torres: Sun Temple, Monkeyopolis.

## 1. Como ler as métricas

Mesmo método do documento do clone:

- **1 alvo/s**: dano por golpe × golpes por segundo em um alvo.
- **Área/s**: projéteis × pierce × dano ÷ recarga (teto teórico).

As métricas só aparecem quando a fonte traz dano, pierce e recarga. Muitas páginas de upgrade do BTDB2 descrevem só as mudanças, e aí a métrica fica de fora. **Alcance em unidades do jogo** (o Dardo tem 32; o clone usa 128 px, ou seja, ~×4). **Velocidade em unidades por segundo** (vermelho = 25).

## 2. Mecânicas gerais

| Sistema | Como funciona no BTDB2 | Fonte |
|---|---|---|
| Partida | 2 jogadores, mapa espelhado; $650 iniciais, **eco de 250** paga a cada 6 s (4,2 s no Speed Battle), **150 vidas** | [B0] |
| Fim | Perde quem zera as vidas. Depois da R40, perde quem tem menos vidas; empate (ou os dois com 150+) vai para a Morte Súbita (vidas viram 1) até a R50, quando **toda renda para** | [B0] [F4] |
| Próxima rodada | Começa 2 s depois dos dois lados limparem, 4 s depois de um lado limpar ou no tempo-limite (rodada × 1,5 + 8,5 s depois do último bloon nascer); mínimo de 5,5 s | [F5] |
| Tipos de dano | Afiado, estilhaço, gelo, energia, plasma, fogo, explosão, ácido, normal, imparável. Imunidades vêm das "propriedades": preto (explosão), branco (gelo), roxo (energia, plasma, fogo), chumbo (afiado, estilhaço, gelo, energia) | [B] |
| Congelado | Bloon congelado não é estourado por afiado (padrão dos dardos) | [B] |
| Camo, regen, fortificado | Regen volta uma camada a cada **2,6 s**. Fortificado: chumbo 6 de vida, cerâmica 30, dirigíveis ×2 | [F3] |
| Dirigíveis | Vida cresce a cada rodada depois da R25. Rodadas 30+: zebra solta só 1 preto e chumbo só 1 preto | [B] |
| Cruzamento | Padrão 5-2-0; várias interações de caminho cruzado estão documentadas por upgrade | [B] |
| Linha de visão | Obstáculos bloqueiam tiros; várias torres ignoram isso com upgrades (Guided Magic, mísseis, raios) | [B] |
| Heróis | Sobem de nível com a rodada e com sacrifícios; **o nível também pode ser comprado com dinheiro** (custo acumulado nas tabelas) | [F] |
| Arenas | 9 arenas. A primeira (Red Bloon Camp) proíbe modificadores de envio; as mais altas trocam o conjunto de mapas | [F6] |
| Modos e eventos | Liga, Casual, Privado, Clã. Eventos: Bananza (dinheiro ×2, Morte Súbita na R101), No Pain No Gain (preço conforme as vidas), Jump Start (começa numa rodada entre 10 e 30) | [B0] |

## 3. Bloons

Velocidade em unidades por segundo e em relação ao vermelho (25 u/s). Fonte: páginas de cada bloon na Blooncyclopedia [B].

| Bloon | Vida (fort.) | Vel. | ×vermelho | RBE (fort.) | Imunidades | Filhos |
|---|---|---|---|---|---|---|
| Vermelho | 1 | 25 | 1,0 | 1 | - | - |
| Azul | 1 | 35 | 1,4 | 2 | - | 1 vermelho |
| Verde | 1 | 45 | 1,8 | 3 | - | 1 azul |
| Amarelo | 1 | 80 | 3,2 | 4 | - | 1 verde |
| Rosa | 1 | 87,5 | 3,5 | 5 | - | 1 amarelo |
| Preto | 1 | 45 | 1,8 | 11 | explosão | 2 rosas |
| Branco | 1 | 50 | 2,0 | 11 | gelo | 2 rosas |
| Roxo | 1 | 85 | 3,4 | 11 | energia, plasma, fogo | 2 rosas |
| Chumbo | 1 (6) | 45 | 1,8 | 23 (28) | afiado, estilhaço, gelo, **energia** | 2 pretos (1 na R30+) |
| Zebra | 1 | 75 | 3,0 | 23 | explosão, gelo | preto + branco (só preto na R30+) |
| Arco-íris | 1 | 55 | 2,2 | 47 | - | 2 zebras |
| Cerâmica | 10 (30) | 62,5 | 2,5 | 104 (124) | - | 2 arco-íris |
| MOAB | 200 (400) | 25 | 1,0 | 616 (936) | - | 4 cerâmicas |
| BFB | 700 (1.400) | 8,5 | 0,34 | 3.164 (5.144) | - | 4 MOAB |
| ZOMG | 4.000 (8.000) | 7 | 0,28 | 16.656 (28.576) | - | 4 BFB |
| DDT | 400 (800) | 68,75 | 2,75 | 816 (1.336) | afiado, estilhaço, gelo, energia, explosão; **camo** | **4** cerâmicas camo+regen |
| B.A.D. | **12.500** (25.000) | 4,5 | 0,18 | 48.260 (86.160) | não sofre empurrão, lentidão nem atordoamento | 2 ZOMG + 3 DDT |

Na R30, o B.A.D. real tem ~80.600 de RBE, por causa do crescimento da vida dos dirigíveis [B].

## 4. Torres

Ordem da loja. Os custos são os da Blooncyclopedia (versão atual). Onde a Bloons Wiki traz outro valor, ficou o da Blooncyclopedia.
### Macaco Dardo (Dart Monkey) · $200 · Primária

**Base:** 1 dardo a cada 0,95 s, 1 de dano, pierce 2, alcance 32, [afiado]. Não estoura chumbo nem congelado. **1 alvo: 1,05/s.** Fonte: [B1].

| Up | Nome | $ (acum.) | Mecânica | 1 alvo/s (calc.) | Conf. |
|---|---|---|---|---|---|
| 1-1 | Sharp Shots | 100 (300) | +1 pierce (+6 no Crossbow Master) | 1,05 | wiki |
| 1-2 | Razor Sharp Shots | 250 (550) | +3 pierce (total 6; +8 no Crossbow Master) | 1,05 | wiki |
| 1-3 | Spike-o-pult | 450 (1.000) | Bola de espinhos: pierce 30, +1 em cerâmica, estoura congelado, quica em obstáculo; recarga 1,15 s; +4,8 alcance | 0,87 | wiki |
| 1-4 | Juggernaut | 1.800 (2.800) | Bola gigante: 2 de dano, +2 em cerâmica, pierce 60, estoura chumbo, recarga ~1 s | ~2,0 | wiki |
| 1-5 | Ultra-Juggernaut | 13.500 (16.300) | 5 de dano, pierce 200, +16 cerâmica, +5 fortificado, +30 chumbo; ao acabar se divide 2× em 6 juggernauts (pierce 100) | ~5 (+fragm.) | wiki |
| 2-1 | Quick Shots | 100 (300) | Recarga ×0,85 (0,81 s) | 1,24 | wiki |
| 2-2 | Very Quick Shots | 100 (400) | Recarga ×0,78 (~0,63 s) | 1,59 | wiki |
| 2-3 | Triple Shot | 350 (750) | 3 dardos em leque de 30° | 1,59 (área 9,5) | wiki |
| 2-4 | Super Monkey Fan Club | 9.000 (9.750) | Recarga 0,477 s, 3 de dano. **Hab.** (50 s): transforma ele + 9 dardos (até 2-4-2) em Super Macacos por 12 s (1 dano, pierce 2, a cada 0,05 s, alcance mín. 40) | 6,3 (+20/s por macaco na hab.) | wiki |
| 2-5 | Plasma Monkey Fan Club | 45.000 (54.750) | **Hab.** (50 s): até 20 dardos viram Macacos Plasma por 15 s (2 dano, pierce 5, a cada 0,025 s, [plasma]) | +80/s por macaco na hab. | wiki |
| 3-1 | Long Range Darts | 90 (290) | +8 alcance (+25%) | 1,05 | wiki |
| 3-2 | Enhanced Eyesight | 200 (490) | +8 alcance, +10% vel. do projétil, **camo** | 1,05 | wiki |
| 3-3 | Crossbow | 600 (1.090) | Besta: 3 de dano, pierce 7, alcance 56 | 3,2 | wiki |
| 3-4 | Sharp Shooter | 2.300 (3.390) | 6 de dano, recarga 0,75 s; **crítico de 50 a cada 7 tiros**; ricocheteia 1 vez | ~16 | wiki |
| 3-5 | Crossbow Master | 27.000 (30.390) | 9 de dano, pierce 13, alcance 76, recarga 0,13 s, [normal]; crítico de 75 a cada 5 tiros | ~171 | wiki |

**Análise.**
- Base barata e sem camo; o 3-2 é a primeira detecção de camo por $490.
- O 3-5 é um dos maiores DPS de alvo único por custo do jogo.
- O caminho 2 só vale com vários Dardos por perto, porque o Fan Club transforma os outros Dardos.
- Na versão 4.6 a Ninja Kiwi passou custo do Triple Shot para a base, subindo o Dardo de $150 para $200, e enfraqueceu a Besta [B1].

### Macaco Bumerangue (Boomerang Monkey) · $325 · Primária

**Base:** 1 bumerangue a cada 1,2 s, 1 de dano, pierce 4, alcance 40, faz curva e volta. **1 alvo: 0,83/s.** Fonte: [F1].

| Up | Nome | $ (acum.) | Mecânica | 1 alvo/s (calc.) | Conf. |
|---|---|---|---|---|---|
| 1-1 | Improved Rangs | 200 (525) | +3 pierce (7); +8 no Kylie, +100 no Heavy Kylie | 0,83 | wiki |
| 1-2 | Glaives | 200 (725) | +3 pierce (10); +10 no Kylie, +120 e +50% empurrão no Heavy Kylie | 0,83 | wiki |
| 1-3 | Glaive Ricochet | 1.400 (2.125) | Glaive em linha reta que salta de bloon em bloon (62,5 u), pierce 40 | 0,83 | wiki |
| 1-4 | M.O.A.R Glaives | 3.400 (5.525) | Recarga 0,6 s (2× mais rápido), pierce 80, salto 187,5 u | 1,7 | wiki |
| 1-5 | Glaive Lord | 30.000 (35.525) | Anel de glaives orbitando (raio 30, não aumenta com buff): 2 de dano a 1.000 bloons a cada 0,1 s, +5 cerâmica/MOAB/fortificado, camo. Glaive lançada: 8 de dano e sangramento de 100/s por 15,1 s em MOAB | 20 (anel) + 13 | wiki |
| 2-1 | Faster Throwing | 175 (500) | Recarga ×0,75 (0,9 s) | 1,1 | wiki |
| 2-2 | Faster Rangs | 250 (750) | Recarga ×0,75 de novo e projétil mais rápido | 1,5 | fandom |
| 2-3 | Bionic Boomerang | 1.100 (1.850) | Recarga 0,238 s, +1 em MOAB | 4,2 | wiki |
| 2-4 | Turbo Charge | 4.500 (6.350) | **Hab.** (45 s, sem recarga inicial): 7× velocidade (0,034 s), +1 dano e camo por 8 s | 4,2 (59 na hab.) | wiki |
| 2-5 | Perma Charge | 33.000 (39.350) | Permanente: recarga 0,034 s, 4 de dano. Hab.: +10 dano por 15 s | ~118 | wiki |
| 3-1 | Long Range Rangs | 100 (425) | +15% alcance (+6,45) e arco mais largo | 0,83 | wiki |
| 3-2 | Red Hot Rangs | 300 (725) | +1 dano, [normal] (chumbo e congelado), +1 em chumbo | 1,7 | wiki |
| 3-3 | Kylie Boomerang | 1.100 (1.825) | Kylie em linha reta, pierce 18, reacerta o mesmo alvo a cada 0,3 s | 0,83+ | wiki |
| 3-4 | MOAB Press | 3.400 (5.225) | Novo Heavy Kylie a cada 10 s contra MOAB: 5 de dano em MOAB, pierce 200, empurra o dirigível (não em BAD) | +0,5 MOAB | wiki |
| 3-5 | MOAB Domination | 52.000 (57.225) | Kylie com 12 de dano a cada 0,6 s. Heavy Kylie a cada 5 s: 50 em MOAB, pierce 300, atordoa 0,25 s, pega BAD e explode (100 de dano, r50, queima 50/s por 4 s) | 20 | wiki |

**Análise.** No BTDB2 o Bumerangue é o anti-pierce clássico. O 4-0-2 M.O.A.R. Glaives é citado como defesa padrão contra as investidas da R11 (roxos e zebras em grupo). O 3-x controla dirigíveis com empurrão, e o 2-5 vira DPS bruto.

### Canhão Bomba (Bomb Shooter) · $525 · Primária

**Base:** bomba a cada 1,5 s, explosão com 1 de dano, pierce 14, raio 12, alcance 40, [explosão] (estoura chumbo, mas não preto). **1 alvo: 0,67/s; área 9,3/s.** Fontes: [B], [F].

| Up | Nome | $ (acum.) | Mecânica | 1 alvo/s (calc.) | Conf. |
|---|---|---|---|---|---|
| 1-1 | Bigger Bombs | 350 (875) | Raio 12→18, +6 pierce (20) | 0,67 | wiki |
| 1-2 | Heavy Bombs | 650 (1.525) | +1 dano (2), +10 pierce (30) | 1,3 | wiki |
| 1-3 | Really Big Bombs | 1.200 (2.725) | 3 de dano, pierce 50, raio 27 | 2,0 | wiki |
| 1-4 | Bloon Impact | 3.600 (6.325) | Atordoa bloons comuns por 1 s; +3 alcance | 2,0 | wiki |
| 1-5 | Bloon Crush | 55.000 (61.325) | 30 de dano por explosão, [normal], atordoa **MOAB** e bloons por 1,75 s | 20 | wiki |
| 2-1 | Faster Reload | 250 (775) | Recarga ×0,75 (1,125 s) | 0,89 | wiki |
| 2-2 | Missile Launcher | 400 (1.175) | Recarga 0,825 s, +4 alcance, projétil +50% | 1,2 | wiki |
| 2-3 | MOAB Mauler | 900 (2.075) | +18 em MOAB (19); +5 alcance | 23 em MOAB | wiki |
| 2-4 | MOAB Assassin | 3.200 (5.275) | +40 em MOAB (41). **Hab.** (30 s): 750 no dirigível mais forte + explosão de 3 | 50 em MOAB | wiki |
| 2-5 | MOAB Eliminator | 25.000 (30.275) | +199 em MOAB (200). Hab.: 4.500 a cada 10 s | 242 em MOAB (+450/s na hab.) | wiki |
| 3-1 | Extra Range | 200 (725) | +7 alcance (47) | 0,67 | wiki |
| 3-2 | Frag Bombs | 300 (1.025) | 8 fragmentos afiados de 1 pierce; +2 alcance; **a bomba passa a estourar preto, zebra e DDT** | 0,67 | wiki |
| 3-3 | Cluster Bombs | 900 (1.925) | Fragmentos viram mini explosões (1 dano, pierce 8, raio 15) | 0,67 (área ~52) | wiki |
| 3-4 | Recursive Cluster | 3.200 (5.125) | A cada 2 tiros, explosões terciárias com 6× o pierce do cluster (48) | área ~150 | wiki |
| 3-5 | Bomb Blitz | 38.000 (43.125) | +3 dano em tudo, recursivo em todo tiro. **Passiva** (40 s): ao perder vida, 3.000 de dano em tudo na tela e destrói MOAB e menores | 5,3 | wiki |

**Análise.** A Bomba do BTDB2 **passa a acertar pretos no 3-2**, diferente do clone. O 2-5 (+199 em MOAB por explosão) é o anti-dirigível padrão. O 3-5 é um "seguro contra vazamento" passivo, sem equivalente no clone.

### Atirador de Tachinhas (Tack Shooter) · $280 · Primária

**Base:** 8 tachinhas (45° entre elas) a cada 1,4 s, 1 de dano, pierce 1, alcance 23, [afiado]. **1 alvo: 0,71/s; área 5,7/s.** Fonte: [B].

| Up | Nome | $ (acum.) | Mecânica | 1 alvo/s (calc.) | Conf. |
|---|---|---|---|---|---|
| 1-1 | Faster Shooting | 150 (430) | Recarga ×0,75 (1,05 s) | 0,95 | wiki |
| 1-2 | Even Faster Shooting | 400 (830) | Recarga ×0,6 (0,63 s) | 1,6 | wiki |
| 1-3 | Hot Shots | 700 (1.530) | +1 dano, estoura chumbo e congelado | 3,2 | wiki |
| 1-4 | Ring of Fire | 3.500 (5.030) | Anel de fogo: 3 de dano, pierce 45, a cada 0,47 s; não pega roxo | 6,3 | wiki |
| 1-5 | Inferno Ring | 35.500 (40.530) | Anel: 11 de dano, +11 em MOAB, alcance 35, a cada 0,1 s. Meteoro a cada 4 s: 900 no alvo + 150 em área + queimadura de 150/s por 4 s | 110 (+225 meteoro) | wiki |
| 2-1 | Long Range Tacks | 100 (380) | +4 alcance (27); +10 pierce no anel | 0,71 | wiki |
| 2-2 | Super Range Tacks | 225 (605) | +4 alcance (31), +1 pierce | 0,71 | wiki |
| 2-3 | Blade Shooter | 600 (1.205) | Lâminas: pierce 3, projétil maior, recarga 1,19 s | 0,84 | wiki |
| 2-4 | Blade Maelstrom | 3.850 (5.055) | **Hab.** (20 s): 2 lâminas em espiral a cada 0,033 s (1 dano, pierce 200) por 3 s | 0,84 (+60 na hab.) | wiki |
| 2-5 | Super Maelstrom | 15.000 (20.055) | Hab.: 4 lâminas, 2 de dano, pierce 500, [normal], 9 s | +240 na hab. | wiki |
| 3-1 | More Tacks | 100 (380) | 10 tachinhas (+1 dano no anel, +400 no meteoro) | 0,71 | wiki |
| 3-2 | Even More Tacks | 100 (480) | 12 tachinhas (+1 dano no anel, meteoro vai a 1.800) | 0,71 | wiki |
| 3-3 | Tack Sprayer | 450 (930) | 16 tachinhas, recarga 1,05 s | 0,95 | wiki |
| 3-4 | Overdrive | 3.400 (4.330) | 3× velocidade (0,35 s), +1 pierce | 2,9 | wiki |
| 3-5 | The Tack Zone | 24.000 (28.330) | 32 tachinhas, recarga 0,26 s, alcance 30, 2 de dano, pierce 4 (0-2-5: pierce 10, alcance 50) | 7,6 (área 490) | wiki |

**Análise.** O caminho 3 barato (Even More Tacks por $100) é o grande pulo de custo/benefício. O 1-5 é um dos melhores T5 contra tudo. Os caminhos cruzados mudam números de verdade (por exemplo, o 0-2-5 tem 10 de pierce).

### Macaco de Gelo (Ice Monkey) · $450 · Primária

**Base:** congela em área (raio 20) até 40 bloons por 1,5 s a cada 2,4 s, 1 de dano, [gelo]. Não mira nem afeta MOAB e pode ficar na água. **1 alvo: 0,42/s; área 16,7/s.** Fonte: [B].

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Permafrost | 100 (550) | Bloons descongelados ficam 50% mais lentos (no T5 também MOAB, 25%, menos BAD) | wiki |
| 1-2 | Cold Snap | 350 (900) | Congela e dana **chumbo** e mira **camo** (com prioridade de camo) | wiki |
| 1-3 | Ice Shards | 2.000 (2.900) | Camada congelada que estoura solta 3 estilhaços (1 dano, pierce 3) | wiki |
| 1-4 | Embrittlement | 2.700 (5.600) | +5 alcance; estoura tudo, pega MOAB (dano sem congelar), **remove camo e regen**, e o bloon leva +1 de dano e perde imunidades por 2 s | wiki |
| 1-5 | Super Brittle | 34.000 (39.600) | O debuff passa a +4 de dano; recarga ×0,9 (2,16 s); tira o camo do DDT | wiki |
| 2-1 | Enhanced Freeze | 225 (675) | Congela 2,2 s; recarga ×0,75 (1,8 s) | wiki |
| 2-2 | Deep Freeze | 350 (1.025) | Congelamento atravessa +1 camada | wiki |
| 2-3 | Arctic Wind | 2.900 (3.925) | Aura de raio 35 que deixa bloons (não dirigíveis, inclusive chumbo e branco) 60% mais lentos; pierce 100; +10 alcance; **deixa colocar torres de terra na água** | wiki |
| 2-4 | Snowstorm | 4.000 (7.925) | **Hab.** (8 s): congela tudo na tela (3 s em camo, branco e MOAB), 1 de dano em todos e gelos +50% mais rápidos | wiki |
| 2-5 | Absolute Zero | 18.000 (25.925) | Pierce 300, alcance 40, lentidão de 80%, mira MOAB; hab. congela tudo menos BAD e dá +100% de velocidade aos gelos | wiki |
| 3-1 | Larger Radius | 100 (550) | +7 alcance (+35%) | wiki |
| 3-2 | Re-Freeze | 200 (750) | Pode mirar e recongelar bloons já congelados | wiki |
| 3-3 | Cryo Cannon | 1.300 (2.050) | Canhão: a cada 1,15 s, alcance 46, explosão r20 com pierce 30; ganha modos de mira | wiki |
| 3-4 | Icicles | 2.900 (4.950) | Congelados ganham pingentes (2 de dano, +6 em MOAB, pierce 3); mira MOAB | wiki |
| 3-5 | Icicle Impale | 32.000 (36.950) | +68 em MOAB (70), **congela MOAB**, recarga 0,86 s; dirigíveis ficam na velocidade de um ZOMG, e o ZOMG fica 50% mais lento | wiki |

**Análise.** O Gelo do BTDB2 é suporte e controle. O 2-3 é único: **transforma água em terreno construível**. O 1-4 tira camo, regen e imunidades de tudo em volta. O 3-5 é um controle de dirigível forte. Nenhum caminho é DPS puro.

### Atirador de Cola (Glue Gunner) · $100 (wiki) · Primária

**Base:** cola a cada ~0,9 a 1 s, dano 0, pierce 1, alcance 40, [ácido]. Desacelera 50% por 11 s e atravessa até 3 camadas. **Estoura chumbo** (ácido) [F]. Fontes: [B], [F].

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Glue Soak | 150 (250) | A cola atravessa **todas** as camadas (não os dirigíveis) | wiki |
| 1-2 | Corrosive Glue | 500 (750) | 1 de dano a cada 2 s enquanto colado; passa a colar MOAB (só DoT) | wiki |
| 1-3 | Bloon Dissolver | 2.300 (3.050) | 1 de dano a cada 0,575 s (2 em cerâmica e MOAB) | fandom |
| 1-4 | Bloon Liquefier | 4.500 (7.550) | 1 de dano a cada 0,1 s (3 em cerâmica), +1 pierce | fandom |
| 1-5 | The Bloon Solver | 19.000 (26.550) | 3 de dano a cada 0,1 s (bem mais em cerâmica e MOAB); tiro vira respingo de pierce 6 a cada 0,4 s | fandom |
| 2-1 | Bigger Globs | 100 (200) | +1 pierce | fandom |
| 2-2 | Glue Splatter | 700 (900) | O tiro vira respingo que cola 10 bloons | wiki |
| 2-3 | Glue Hose | 2.200 (3.100) | Recarga −70% (0,3 s) | wiki |
| 2-4 | Glue Strike | 4.000 (7.100) | **Hab.** (40 s): cola tudo na tela por 11 s (24 s com 3-1); os bloons levam **+2 de dano de tudo** e perdem a imunidade de chumbo | wiki |
| 2-5 | Glue Storm | 18.000 (25.100) | A hab. recola tudo a cada 2 s, com o dobro de lentidão e duração | fandom |
| 3-1 | Stickier Glue | 120 (220) | Cola dura 24 s | fandom |
| 3-2 | Stronger Glue | 400 (620) | Lentidão de 75% | fandom |
| 3-3 | MOAB Glue | 3.200 (3.820) | Cola MOAB a ×0,625 de velocidade por 24 s (não o BAD); soma com outras colas | wiki |
| 3-4 | Relentless Glue | 3.000 (6.820) | Bloon colado que estoura solta um respingo que atordoa (1 s em bloons, 0,35 s em MOAB); **camo** | wiki |
| 3-5 | Super Glue | 28.000 (34.820) | Prende MOAB, DDT e bloons no lugar; BFB e ZOMG ficam quase parados; +6 pierce | fandom |

**Análise.**
- A cola do jogo real **pega chumbo**, é barata ($100) e o 2-4 funciona como um amplificador de dano global (+2 em tudo). No clone é o contrário: a cola é imune ao chumbo e não tem esse debuff.
- O 1-5 é um DPS contínuo contra cerâmica e dirigível.

### Macaco Atirador (Sniper Monkey) · $300 · Militar

**Base:** tiro instantâneo em qualquer distância (sem atravessar obstáculos) a cada 1,35 s, 2 de dano, 1 alvo, [afiado]. Sem chumbo, congelado nem camo. **1 alvo: 1,48/s.** Fonte: [B].

| Up | Nome | $ (acum.) | Mecânica | 1 alvo/s (calc.) | Conf. |
|---|---|---|---|---|---|
| 1-1 | Full Metal Jacket | 250 (550) | 4 de dano, [normal] (chumbo e congelado) | 3,0 | wiki |
| 1-2 | Large Calibre | 750 (1.300) | 7 de dano | 5,2 | wiki |
| 1-3 | Deadly Precision | 2.800 (4.100) | 30 de dano, +15 em cerâmica | 22 | wiki |
| 1-4 | Maim MOAB | 5.000 (9.100) | 45 de dano; atordoa MOAB 2 s, BFB 1 s, ZOMG 0,45 s, DDT 0,5 s (BAD 0) | 33 | wiki |
| 1-5 | Cripple MOAB | 34.000 (43.100) | 280 de dano; atordoamento e "cripple" (+5 de dano de tudo): MOAB 7 s, BFB 6 s, ZOMG 3 s | 207 | wiki |
| 2-1 | Night Vision Goggles | 200 (500) | **Camo**, +1 de dano em camo | 1,48 | wiki |
| 2-2 | Shrapnel Shot | 300 (800) | 5 estilhaços em cone de 45° (1 dano, pierce 2) | 1,48 | wiki |
| 2-3 | Bouncing Bullet | 2.600 (3.400) | A bala quica 5 vezes (6 alvos, até 50 u) | 1,48 (área 8,9) | wiki |
| 2-4 | Supply Drop | 5.800 (9.200) | **Hab.** (50 s, 20 s iniciais): caixa de **$2.000**; bala [imparável] | 1,48 | wiki |
| 2-5 | Elite Sniper | 14.000 (23.200) | Recarga ×0,4; todo quique solta estilhaço; **todos os Snipers** ganham recarga ×0,8; caixa de $4.000 e dispara todas as caixas juntas | 3,7 | wiki |
| 3-1 | Fast Firing | 300 (600) | Recarga ×0,7 (1,015 s); −5 s na caixa | 2,0 | wiki |
| 3-2 | Even Faster Firing | 300 (900) | Recarga ×0,7 (0,71 s) | 2,8 | wiki |
| 3-3 | Semi-Automatic | 2.100 (3.000) | Recarga 0,22 s | 9,1 | wiki |
| 3-4 | Full Auto Rifle | 3.800 (6.800) | Recarga 0,118 s, estoura chumbo, +2 em MOAB | 17 (34 em MOAB) | wiki |
| 3-5 | Elite Defender | 14.500 (21.300) | Recarga 0,059 s; até +100% conforme o bloon mais avançado; passiva de 4× velocidade por 7 s ao vazar | 34 a 68 (até ~270) | wiki |

**Análise.**
- O caminho 2 é a principal renda alternativa por tempo do jogo (o 2-4 dá $2.000 a cada 50 s), chamada de "alt-eco" [F2].
- O 1-5 enfraquece dirigíveis para o time inteiro (+5 de dano).
- O Sniper é a única torre, segundo a wiki, sem jeito fácil de parar Tight Leads [F3].

### Submarino Macaco (Monkey Sub) · $325 · Militar · só água

**Base:** dardo teleguiado a cada 0,75 s, 1 de dano, pierce 2, alcance 42. **1 alvo: 1,33/s.** Fonte: [B].

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Longer Range | 130 (455) | +10 alcance | wiki |
| 1-2 | Advanced Intel | 500 (955) | Mira qualquer bloon no alcance de **qualquer torre sua** (camo só se essa torre vê camo) | fandom |
| 1-3 | Submerge and Support | 950 (1.905) | Modo "Submerso": para de atacar e emite um pulso a cada 1,2 s que **remove camo** de 75 bloons | wiki |
| 1-4 | Bloontonium Reactor | 2.800 (4.705) | Submerso: pulso radioativo de 1 dano (pierce 30) e pulso de remover camo (pierce 150) a cada 0,4 s; torres de água no alcance recarregam habilidades 15% mais rápido | wiki |
| 1-5 | Energizer | 32.000 (36.705) | Todas as habilidades da tela −20% de recarga (−50% nas torres de água no alcance); heróis no alcance +75% de XP; pulso com 5 de dano, +10 em cerâmica, pierce 1.000 | wiki |
| 2-1 | Barbed Darts | 300 (625) | +3 pierce (5) | wiki |
| 2-2 | Heat-tipped Darts | 300 (925) | [imparável] (chumbo e congelado) | wiki |
| 2-3 | Ballistic Missile | 1.600 (2.525) | Míssil (ignora linha de visão): 2 de dano, +5 em cerâmica e MOAB, pierce 50, a cada 1,1 s | wiki |
| 2-4 | First Strike Capability | 15.000 (17.525) | **Hab.** (60 s): 7.000 no bloon mais forte + explosão de 200 (pierce 80) | wiki |
| 2-5 | Pre-emptive Strike | 30.000 (47.525) | Míssil automático de 1.000 em **cada dirigível que entra no mapa**; hab. vai a 11.000 com recarga de 30 s | wiki |
| 3-1 | Twin Guns | 300 (625) | Recarga do dardo ÷2 (0,375 s); acelera os outros ataques | wiki |
| 3-2 | Airburst Darts | 800 (1.425) | O dardo se divide em 3 (1 dano, pierce 2) ao acertar | wiki |
| 3-3 | Triple Guns | 800 (2.225) | Recarga 0,25 s | wiki |
| 3-4 | Armor Piercing Darts | 3.200 (5.425) | 3 de dano, +3 em MOAB; fragmentos com 2 de dano e pierce 5 | wiki |
| 3-5 | Sub Commander | 25.000 (30.425) | Subs no alcance: dano ×2, +4 pierce, +20 alcance; o Commander ataca 3× mais rápido | fandom |

**Análise.**
- O 1-5 é suporte global: −20% na recarga de **todas** as habilidades e XP de herói.
- O 1-3 é o removedor de camo mais barato para a torre ter a mira do 1-2.
- O 2-5 anula dirigíveis enviados em sequência (1.000 em cada um que entra).

### Macaco Bucaneiro (Monkey Buccaneer) · $500 · Militar · só água

**Base:** dardo para os dois lados a cada 1,0 s, 1 de dano, pierce 4, alcance 60. **1 alvo: 1,0/s.** Fontes: [B], [F].

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Faster Shooting | 350 (850) | Recarga ×0,75; a hab. do 2-4 cai para 20 s | fandom |
| 1-2 | Double Shot | 550 (1.400) | 2 dardos (10 uvas com o 2-1) | wiki |
| 1-3 | Destroyer | 2.200 (3.600) | Dardo 5× mais rápido (0,15 s); uvas 3× mais rápidas | wiki |
| 1-4 | Aircraft Carrier | 6.400 (10.000) | Aviões com dardos à frente, radial e mísseis anti-MOAB | fandom |
| 1-5 | Carrier Flagship | 25.000 (35.000) | Tudo estoura qualquer bloon e dá mais dano; torres no convés ignoram parte da linha de visão; **torres de água, Ases e o UCAV** atacam +18% mais rápido | fandom |
| 2-1 | Grape Shot | 550 (1.050) | 5 uvas em leque de 90° a cada 1,3 s (1 dano, pierce 1) | wiki |
| 2-2 | Hot Shot | 500 (1.550) | Uvas de fogo: chumbo e congelado (não roxo), queimam 1 a cada 1,5 s por 3,1 s | wiki |
| 2-3 | Cannon Ship | 900 (2.450) | Canhão a cada 1,2 s: bomba de fragmentação (1 dano, pierce 28) | wiki |
| 2-4 | Monkey Pirates | 5.250 (7.700) | 3 bombas de 2 de dano. **Hab.** (30 s): **puxa e destrói** 1 MOAB/BFB/DDT (o mais forte), rendendo $400/$800/$1.200 | wiki |
| 2-5 | Pirate Lord | 21.000 (28.700) | Hab.: 3 ganchos (ZOMG usa 2, rende $1.200); uvas com 5 de dano, +5 em cerâmica; dardo e uva 2× mais rápidos | wiki |
| 3-1 | Long Range | 300 (800) | Alcance 60→71, +1 pierce, projéteis +25% | wiki |
| 3-2 | Crow's Nest | 400 (1.200) | **Camo** e +1 de dano em camo em todos os ataques | wiki |
| 3-3 | Merchantman | 2.000 (3.200) | **$400 por rodada** (+5% por Central Market, até +50%) | wiki |
| 3-4 | Favored Trades | 5.300 (8.500) | $1.200 por rodada; +10% no valor de venda das torres no alcance (teto 95%) | wiki |
| 3-5 | Trade Empire | 24.000 (32.500) | $3.000 por rodada; até 10 outros Merchantmen ou Favored Trades ganham +10% de renda por navio (até +100%) e +1 de dano, +1 em cerâmica e +1 em MOAB | wiki |

**Análise.**
- O 2-4 e o 2-5 são "instakill" de dirigível, que a wiki cita como resposta clássica a envios de dirigível fortificado [F3].
- O caminho 3 é renda por rodada (retorno do 3-3: 8 rodadas).
- O 1-5 é um buff de velocidade para torres de água.

### Macaco Ás (Monkey Ace) · $750 · Militar · voa

**Base:** 8 dardos em volta a cada 1,5 s, 1 de dano, pierce 5. Voa em rota (círculo, oito ou infinito). **Área: 26,7/s.** Fontes: [B], [F].

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Rapid Fire | 550 (1.300) | Recarga ×0,75 (1,125 s); bombas ×0,6 | wiki |
| 1-2 | Lots More Darts | 600 (1.900) | 12 dardos por rajada | wiki |
| 1-3 | Fighter Plane | 1.000 (2.900) | Dupla de mísseis só contra MOAB a cada 3 s (20 de dano, pierce 4, teleguiados) | wiki |
| 1-4 | Operation: Dart Storm | 3.000 (5.900) | 16 dardos, recarga 0,54 s, pierce 9; mísseis com 26 de dano a cada 1,5 s | wiki |
| 1-5 | Sky Shredder | 40.000 (45.900) | 32 dardos a cada 0,27 s, 3 de dano, +2 em cerâmica, pierce 14; mísseis com 250 de dano | wiki |
| 2-1 | Exploding Pineapple | 200 (950) | Abacaxi a cada 2 s onde o avião está (1 dano, pierce 20, r25; não pega preto) | wiki |
| 2-2 | Spy Plane | 350 (1.300) | **Camo** em todos os ataques | wiki |
| 2-3 | Bomber Ace | 1.100 (2.400) | 4 bombas na trilha a cada 1,7 s (3 de dano, pierce 10) | wiki |
| 2-4 | Ground Zero | 15.000 (17.400) | **Hab.** (35 s, sem recarga inicial): 700 de dano em tudo na tela; bombas com 10 de dano e pierce 30 | wiki |
| 2-5 | Tsar Bomba | 30.000 (47.400) | Hab.: 3.000 de dano em tudo e atordoa 8 s; bombas pegam qualquer bloon | wiki |
| 3-1 | Sharper Darts | 450 (1.200) | Pierce 5→9 (+12 nos abacaxis, +7 nas bombas) | wiki |
| 3-2 | Advanced Navigation | 300 (1.500) | Escolhe o centro de cada rota de voo | wiki |
| 3-3 | Neva-Miss Targeting | 1.900 (3.400) | Dardos teleguiados em qualquer distância | wiki |
| 3-4 | Spectre | 24.000 (27.400) | Rajada a cada 0,04 s alternando dardos (2 de dano, +2 em MOAB e cerâmica, pierce 15) e bombas (4 de dano, pierce 30, r20) | wiki |
| 3-5 | Flying Fortress | 75.000 (102.400) | 3 projéteis por disparo (primeiro, último, forte) a cada 0,03 s; dardos com 12 de dano, bombas com 6; pega todo tipo | wiki |

**Análise.**
- O 3-5 tem um dos maiores DPS do jogo (~400/s só nos dardos, contando 3 projéteis por disparo).
- O 2-4 é o "nuke" de meio de jogo.
- A Mira Infalível do jogo real **funciona** (bug #7 do clone). A Rota Centralizada do clone foi trocada por Advanced Navigation.

### Piloto de Helicóptero (Heli Pilot) · $800 · Militar · voa

**Base:** dardos a cada ~0,5 s, 1 de dano, pierce 3, alcance 40; segue o cursor/ponto ou persegue. **1 alvo: ~2/s.** Fonte: [F] (infobox).

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Quad Darts | 600 (1.400) | 4 dardos por rajada em vez de 2 | desc. |
| 1-2 | Pursuit | 250 (1.650) | Modo Perseguir: fica na frente do bloon mais avançado | wiki |
| 1-3 | Razor Rotors | 1.800 (3.450) | Aura de hélices (raio 35): 2 de dano, pierce 13, a cada 0,55 s; pega chumbo e congelado | wiki |
| 1-4 | Apache Dartship | 17.500 (20.950) | Metralhadora de dardos + 4 mísseis teleguiados (3 de dano, +3 em cerâmica e MOAB); hélices mais fortes | fandom |
| 1-5 | Apache Prime | 45.000 (65.950) | Quad dart vira laser (6 de dano), metralhadora vira plasma (5 de dano), mísseis com 15 de dano | fandom |
| 2-1 | Bigger Jets | 200 (1.000) | Voa 75% mais rápido; melhora o MOAB Shove | fandom |
| 2-2 | IFR | 350 (1.350) | **Camo** e prioridade de camo | wiki |
| 2-3 | Downdraft | 3.200 (4.550) | Joga 1 bloon por vez de volta para a entrada (6,67 por segundo) | fandom |
| 2-4 | Support Chinook | 7.500 (12.050) | **Hab.:** reposiciona uma torre "leve" e solta uma caixa de dinheiro (sem vidas desde a 4.13) | wiki |
| 2-5 | Special Poperations | 30.000 (42.050) | Hab. de Fuzileiro (metralhadora de 6 de dano, pierce 20, camo); a caixa volta a dar vidas; heli +25% mais rápido | wiki |
| 3-1 | Faster Darts | 350 (1.150) | Projétil mais rápido, +alcance e +pierce do dardo | fandom |
| 3-2 | Faster Firing | 250 (1.400) | Recarga ×0,8 em todos os ataques | fandom |
| 3-3 | MOAB Shove | 4.000 (5.400) | Encosta nos dirigíveis e os empurra ou desacelera (mais forte com 2-1/2-2) | desc. |
| 3-4 | Comanche Defense | 7.500 (12.900) | Mini-Comanches (até 3) aparecem a cada 25% da trilha que os bloons avançam | fandom |
| 3-5 | Comanche Commander | 34.700 (47.600) | +1 de dano em tudo, mísseis muito mais fortes e 3 Comanches permanentes | fandom |

**Análise.** A torre tem mobilidade, e o caminho 2 é utilitário, com renda alternativa (Chinook). Um detalhe relevante: a caixa perdeu as vidas na 4.13 [B]. Os números exatos de vários upgrades não estão na wiki (páginas curtas).

### Macaco Morteiro (Mortar Monkey) · $700 · Militar

**Base:** a cada 2,0 s atira no **ponto escolhido** (alcance infinito), explosão de 1 de dano, pierce 40, [explosão]. **Área: 20/s.** Fonte: [F] (infobox).

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Bigger Blast | 350 (1.050) | Explosão maior | desc. |
| 1-2 | Bloon Buster | 500 (1.550) | Atravessa 2 camadas (2 de dano) | desc. |
| 1-3 | Shell Shock | 1.100 (2.650) | Raio 30→36; atordoamento de 0,4 s (r19) e onda de 1 de dano (r57) | wiki |
| 1-4 | The Big One | 7.300 (9.950) | 7 de dano, r56, pierce 100, pega preto | wiki |
| 1-5 | The Biggest One | 28.000 (37.950) | 50 de dano, r72, pierce 200, onda +20 em cerâmica, atordoa MOAB 0,35 s | wiki |
| 2-1 | Faster Reload | 350 (1.050) | Recarga 1,5 s | wiki |
| 2-2 | Rapid Reload | 400 (1.450) | Recarga 1,08 s | wiki |
| 2-3 | Heavy Shells | 800 (2.250) | [normal] (pega preto), +1 em chumbo, MOAB e fortificado, +2 em atordoado, +3 em cerâmica | wiki |
| 2-4 | Artillery Battery | 8.000 (10.250) | Recarga 0,27 s, +3 em BAD; **hab.** (60 s): 4× velocidade por 7 s | wiki |
| 2-5 | Pop and Awe | 27.000 (37.250) | Hab. (45 s): atordoa tudo 1 s (menos BAD) e dá 20 de dano + 20/s na tela toda por 10 s | wiki |
| 3-1 | Dynamic Targeting | 400 (1.100) | Mira automática em Denso ou Forte | wiki |
| 3-2 | Burny Stuff | 500 (1.600) | Queimadura de 1 a cada 1,4 s por 4,2 s | wiki |
| 3-3 | Signal Flare | 700 (2.300) | Explosão extra (r50, pierce 55) que **remove camo, inclusive do DDT**; torre vê camo | wiki |
| 3-4 | Shattering Shells | 8.500 (10.800) | Remove camo, regen e fortificado (até BFB) e o chumbo do DDT; queimadura de 5 | wiki |
| 3-5 | Blooncineration | 40.000 (50.800) | Fogo de 4 de dano a cada 0,1 s (pierce 40, pega tudo); 100 em MOAB e tira o fortificado de dirigíveis (menos BAD) | wiki |

**Análise.**
- **No jogo real o Morteiro mira onde o jogador escolhe** (3-1 adiciona mira automática). O clone mira sozinho (bug de descrição #15).
- O Signal Flare **revela DDT**, o que no clone falha (bug #6).
- O 3-4 remove propriedades em massa, e isso é central contra envios modificados.

### Atirador Dartling (Dartling Gunner) · $800 · Militar

**Base:** metralhadora que **mira no cursor**, a cada 0,2 s, 1 de dano, pierce 2, [afiado], com imprecisão. **1 alvo: 5/s.** Fonte: [F] (infobox).

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Focused Firing | 200 (1.000) | Espalhamento −60% | wiki |
| 1-2 | Laser Shock | 600 (1.600) | Choque (1 de dano após 1 s); bloons em choque levam +1 de outros Dartlings | wiki |
| 1-3 | Laser Cannon | 2.800 (4.400) | Laser: 2 de dano [energia] (chumbo sim, roxo não), pierce 5, sem imprecisão | wiki |
| 1-4 | Plasma Accelerator | 14.000 (18.400) | Raio contínuo até o alvo: a cada 0,2 s, pierce 40, 1 de dano (ponta 2, +10 em MOAB), choque nível 3 | wiki |
| 1-5 | Ray of Doom | 80.000 (98.400) | Raio infinito até um obstáculo: 35 de dano, +55 no 1º alvo, pierce 1.000, [normal]; choque de 20 | wiki |
| 2-1 | Advanced Targeting | 250 (1.050) | **Camo** | wiki |
| 2-2 | Faster Barrel Spin | 500 (1.550) | +50% velocidade (0,132 s) | wiki |
| 2-3 | Hydra Rocket Pods | 5.100 (6.650) | Mísseis com até 3 explosões (1 de dano, pierce 7, r8), pegam tudo | wiki |
| 2-4 | Rocket Storm | 5.500 (12.150) | **Hab.** (40 s): ondas de 10 mísseis a cada 0,5 s (5 de dano, pierce 7) por 8 s | wiki |
| 2-5 | M.A.D | 68.000 (80.150) | Mega mísseis a cada 0,396 s: 3 de dano na explosão e **+447 em MOAB** no impacto direto | wiki |
| 3-1 | Faster Swivel | 100 (900) | Gira 2× mais rápido (360°/s) | wiki |
| 3-2 | Powerful Darts | 800 (1.700) | +3 pierce (5), projétil mais rápido, [estilhaço] (pega congelado) | wiki |
| 3-3 | Buckshot | 3.400 (5.100) | Rajadas de chumbinho no lugar dos dardos | desc. |
| 3-4 | Bloon Area Denial System | 16.000 (21.100) | Canhão automático de 4 canos que mira sozinho | desc. |
| 3-5 | Bloon Exclusion Zone | 50.000 (71.100) | 6 canos e dano muito maior | desc. |

**Análise.**
- O 1-5 continua sendo um dos T5 mais fortes, com 35 de dano × pierce 1.000 em linha, e o M.A.D. é o anti-MOAB de fim de jogo.
- **No jogo real o Dartling mira no cursor**, enquanto o clone usa mira global automática.
- O caminho 3 aparece só com a descrição do jogo, porque as páginas da wiki estão incompletas.

### Macaco Mago (Wizard Monkey) · $300 · Mágica

**Base:** raio mágico a cada 1,1 s, 1 de dano, pierce 3, alcance 40, [energia]. Estoura congelado, mas não roxo nem chumbo. **1 alvo: 0,91/s.** Fonte: [B].

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Guided Magic | 100 (400) | Raios teleguiados, inclusive atrás de obstáculos | desc. |
| 1-2 | Arcane Blast | 250 (650) | 2 de dano, projétil +50% | wiki |
| 1-3 | Arcane Mastery | 1.300 (1.950) | Mais rápido (0,55 s), mais alcance e dano (3), estoura chumbo | desc./wiki |
| 1-4 | Arcane Spike | 9.000 (10.950) | Recarga 0,275 s, 8 de dano, +10 em MOAB, +4 em chumbo | wiki |
| 1-5 | Archmage | 32.000 (42.950) | 10 de dano, 18 em chumbo, 35 em MOAB, 2× mais rápido, mais pierce; Dragon's Breath e Shimmer melhorados | fandom |
| 2-1 | Fireball | 350 (650) | Bola de fogo a cada 2,6 s: explosão de 2 de dano, pierce 15 | wiki |
| 2-2 | Wall of Fire | 1.000 (1.650) | Muralha de fogo na trilha a cada 6,5 s: 1 de dano a cada 0,15 s (15 por tick, 100 no total), dura 4,5 s | wiki |
| 2-3 | Dragon's Breath | 3.000 (4.650) | Lança-chamas a cada 0,1 s (1 de dano, +1 em cerâmica, pierce 4, queima); bola de fogo com 9 de dano | wiki |
| 2-4 | Summon Phoenix | 4.000 (8.650) | **Hab.:** fênix poderosa por 20 s | desc. |
| 2-5 | Wizard Lord Phoenix | 50.000 (58.650) | Fênix permanente; a hab. vira Fênix de Lava por pouco tempo | desc./fandom |
| 3-1 | Intense Magic | 300 (600) | Raios mais fortes, rápidos e com mais pierce | desc. |
| 3-2 | Monkey Sense | 300 (900) | **Camo** e prioridade de camo | wiki |
| 3-3 | Shimmer | 1.500 (2.400) | A cada 1,5 s **remove camo** de até 200 bloons num raio de 70, ignorando obstáculos | wiki |
| 3-4 | Necromancer | 2.800 (5.200) | Bloons estourados num raio de 70 vão para o cemitério (até 450), e ele invoca bloons zumbis que andam ao contrário (2 de dano, pegam tudo) | wiki |
| 3-5 | Prince of Darkness | 28.000 (33.200) | Alcance 80, raio 4× mais rápido, cemitério de 3.000, invoca MOAB e BFB zumbis | wiki |

**Análise.** O Necromante do jogo real **reanima bloons estourados** de fato; no clone é só uma pilha. O 3-3 é o removedor de camo em área mais barato das torres mágicas.

### Super Macaco (Super Monkey) · $2.000 · Mágica

**Base:** 1 dardo a cada 0,045 s (22 por segundo), 1 de dano, pierce 1, alcance 50, [afiado]. **1 alvo: 22/s.** Fonte: [F] (infobox).

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Laser Blasts | 1.400 (3.400) | Laser: +1 pierce, estoura congelado (sem roxo e chumbo) | fandom |
| 1-2 | Plasma Blasts | 2.500 (5.900) | +1 pierce, +50% velocidade, estoura chumbo (sem roxo) | fandom |
| 1-3 | Sun Avatar | 13.000 (18.900) | 3 raios de sol por disparo, pierce 8 cada | fandom |
| 1-4 | Sun Temple | 80.000 (98.900) | **Sacrifica as torres no alcance** e ganha ataques conforme a categoria (até $20.000 por categoria e só 3 das 4 categorias) | wiki |
| 1-5 | True Sun God | 200.000 (298.900) | Deus Sol verdadeiro | desc. |
| 2-1 | Super Range | 1.000 (3.000) | +10 alcance, +1 pierce | fandom |
| 2-2 | Epic Range | 1.000 (4.000) | +12 alcance, +2 pierce, projétil +25% | fandom |
| 2-3 | Robo Monkey | 10.000 (14.000) | 2 braços com mira independente, +3 pierce, crítico de 10 a cada 15 tiros | fandom |
| 2-4 | Tech Terror | 20.000 (34.000) | **Hab.:** Aniquilação destrói a maioria dos bloons no raio | desc. |
| 2-5 | The Anti-Bloon | 90.000 (124.000) | Versão maior da Aniquilação | n/d |
| 3-1 | Knockback | 3.000 (5.000) | Empurra ou desacelera os bloons a cada golpe | desc. |
| 3-2 | Ultravision | 1.200 (6.200) | +alcance e **camo** | desc. |
| 3-3 | Dark Knight | 4.500 (10.700) | Lâminas sombrias com mais pierce e dano em MOAB; **hab.** Darkshift (teleporta perto) | desc. |
| 3-4 | Dark Champion | 32.000 (42.700) | Lâminas que pegam tudo; Darkshift para o mapa inteiro | desc. |
| 3-5 | Legend of the Night | 140.000 (182.700) | Se um bloon chega à saída, abre um buraco negro em todas as saídas por até 8 s, que apaga o que passa (fecha depois de 1 BAD); mais dano e pierce | fandom |

**Análise.** A base já faz 22/s em alvo único. O 1-4 **sacrifica torres**, mecânica que o clone não tem. O 3-5 é um seguro contra vazamento, como a Bomb Blitz. **Os números dos T3 a T5 não estão na wiki** (páginas curtas).

### Macaco Ninja (Ninja Monkey) · $350 · Mágica

**Base:** shuriken teleguiada a cada 0,62 a 0,70 s, 1 de dano, pierce 3, alcance 40, detecta **camo**. **1 alvo: ~1,5/s.** Fontes: [B], [F].

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Ninja Discipline | 150 (500) | Recarga ×0,7, +7 alcance | fandom |
| 1-2 | Sharp Shurikens | 200 (700) | Pierce 4 | desc. |
| 1-3 | Double Shot | 700 (1.400) | 2 shurikens (30°); estrepes com 5 de dano | wiki |
| 1-4 | Bloonjitsu | 2.750 (4.150) | 5 shurikens (45°) | wiki |
| 1-5 | Grandmaster Ninja | 35.000 (39.150) | 8 shurikens (60°), 4 de dano, recarga 0,217 s, alcance 57 | wiki |
| 2-1 | Distraction | 200 (550) | 25% de chance de jogar o bloon 100 a 150 u para trás (não dirigíveis) | wiki |
| 2-2 | Counter-Espionage | 375 (925) | Todo golpe **remove camo** | wiki |
| 2-3 | Shinobi Tactics | 2.100 (3.025) | Ninjas no alcance (inclusive ele): recarga −10% multiplicativo e +10% pierce, **acumula até 15×** (~2,87× velocidade) | wiki |
| 2-4 | Bloon Sabotage | 5.200 (8.225) | **Hab.** (60 s): bloons na tela e os que nascem ficam 50% mais lentos por 10 s (não BAD) | wiki |
| 2-5 | Grand Saboteur | 20.000 (28.225) | Duração maior; dirigíveis que entram com a hab. ativa nascem com −25% de vida; Shinobis no alcance +10 alcance e +2 em MOAB | fandom |
| 3-1 | Seeking Shuriken | 200 (550) | Shurikens buscam bloons | desc. |
| 3-2 | Caltrops | 250 (800) | Estrepes a cada 9 s (3 de dano, pierce 6, duram 25 s) | wiki |
| 3-3 | Flash Bomb | 1.800 (2.600) | A cada 3º ataque: bomba (r40, 1 de dano, pierce 40, atordoa 1 s, pega tudo) | wiki |
| 3-4 | Sticky Bomb | 4.600 (7.200) | Bomba só em MOAB a cada 4,5 s: gruda 3 s e explode (450 no alvo, 100 em volta) | wiki |
| 3-5 | Master Bomber | 40.000 (47.200) | Bomba negra a cada 1,8 s, alcance infinito: atordoa 1 s e dá 1.000 (200 em área); flash bomb com 10 de dano | wiki |

**Análise.**
- As Shinobi Tactics do jogo real **também se aplicam ao próprio ninja e acumulam de propósito**, até 15 vezes. O que no clone listei como bug #11 é, no jogo real, a mecânica intencional, mas com teto.
- O Master Bomber melhora a **bomba grudenta** (no clone melhora a bomba de luz, bug #2).

### Alquimista (Alchemist) · $550 · Mágica

**Base:** poção a cada 1,8 s, 1 de dano, pierce 12, raio 12, alcance 45, [ácido], mais corrosão de 1 a cada 2 s por 4 s. **Área: ~6,7/s.** Fonte: [F] (infobox).

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Larger Potions | 250 (800) | +6 pierce (18), raio +50% | wiki |
| 1-2 | Acidic Mixture Dip | 350 (1.150) | Joga poção nos macacos: +1 em cerâmica e MOAB e estouram chumbo por 10 tiros | fandom |
| 1-3 | Berserker Brew | 1.650 (2.800) | Poção no macaco mais próximo: +1 de dano, +10% alcance, +11% velocidade e +2 pierce, **temporário** | fandom |
| 1-4 | Stronger Stimulant | 2.800 (5.600) | +17,8% velocidade, +3 pierce, +15% alcance, dura mais | fandom |
| 1-5 | Permanent Brew | 55.000 (60.600) | Os buffs ficam **permanentes** enquanto o Alquimista existir | wiki |
| 2-1 | Stronger Acid | 250 (800) | Corrosão a cada 1,33 s | wiki |
| 2-2 | Perishing Potions | 475 (1.275) | 2 de dano, +3 em MOAB (+7 em MOAB fortificado); **tira o fortificado** de bloons comuns | wiki |
| 2-3 | Unstable Concoction | 4.000 (5.275) | Poção só em MOAB (a cada 4,5 s, alcance 67,5): quando ele estoura, explode com 10% da vida base | wiki/fandom |
| 2-4 | Transforming Tonic | 4.500 (9.775) | **Hab.** (60 s): vira monstro por 17,5 s (laser de 2 de dano, pierce 6, +60% alcance) | wiki |
| 2-5 | Total Transformation | 45.000 (54.775) | A hab. transforma até 5 macacos (até T3) em monstros de laser | fandom |
| 3-1 | Faster Throwing | 550 (1.100) | Recarga ×0,6 | fandom |
| 3-2 | Acid Pool | 450 (1.550) | Sem alvos, joga poças de ácido na trilha (1 de dano, pierce 5, 7 s, pegam camo) | wiki |
| 3-3 | Lead to Gold | 1.000 (2.550) | Dano extra em chumbo e **$50 por chumbo estourado** | fandom |
| 3-4 | Rubber to Gold | 2.500 (5.050) | Poção de ouro: os bloons afetados dão $1,5 por camada | fandom |
| 3-5 | Bloon Master Alchemist | 40.000 (45.050) | Poção que **encolhe até 200 bloons em vermelhos**, inclusive MOAB (que gasta mais pierce); sem dinheiro extra | fandom |

**Análise.**
- **O buff do jogo real é temporário** (por tiros ou tempo) e vai em **uma torre por vez**; só o 1-5 o torna permanente.
- No clone é uma aura permanente em área desde o 1-3, e isso explica por que lá fica forte demais.
- O 3-5 do jogo real encolhe mesmo os bloons; no clone é só dano.

### Druida (Druid) · $425 · Mágica

**Base:** 5 espinhos com espalhamento aleatório a cada 1,1 s, 1 de dano, pierce 1, alcance 35, [afiado]. **Área: 4,5/s.** Fonte: [B].

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Hard Thorns | 250 (675) | Pierce 2, [normal] (chumbo e congelado) | wiki |
| 1-2 | Heart of Thunder | 1.000 (1.675) | Relâmpago bifurcado de bloon em bloon (até 31 alvos) | fandom |
| 1-3 | Druid of the Storm | 1.500 (3.175) | Tornados que jogam 30 bloons para trás | fandom |
| 1-4 | Ball Lightning | 4.700 (7.875) | A cada 6 s, uma bola lenta que solta raios a cada 0,35 s (2 de dano, 5 bifurcações, 31 alvos) por 5 s | wiki |
| 1-5 | Superstorm | 55.000 (62.875) | Relâmpagos 3/5; supertempestade de 12 de dano que empurra **MOAB** 250 u e solta bolas de raio | wiki |
| 2-1 | Thorn Swarm | 350 (775) | 8 espinhos | wiki |
| 2-2 | Heart of Oak | 250 (1.025) | Remove regen; 33% de chance de tirar o fortificado | wiki |
| 2-3 | Druid of the Jungle | 700 (1.725) | Cipó pega o bloon mais forte do mapa (1 + 13% da vida a cada 0,15 s); cipó dourado a cada 14 s dá $100 | wiki |
| 2-4 | Jungle's Bounty | 4.000 (5.725) | **Hab.** (40 s, 15 s iniciais): **+$1.000**; +20 alcance | wiki |
| 2-5 | Spirit of the Forest | 35.000 (40.725) | Cipós grossos crescem pela trilha (até 4 de dano, +30 em cerâmica, +25 em MOAB perto do druida); hab. de vidas; o Bounty dispara em todos os druidas | wiki/desc. |
| 3-1 | Druidic Reach | 100 (525) | Alcance 35→45 | wiki |
| 3-2 | Heart of Vengeance | 400 (925) | +20% velocidade, +1% por vida perdida depois de comprar (até +80%) | wiki |
| 3-3 | Druid of Wrath | 650 (1.575) | +5% velocidade a cada 10 de dano (até +100%, some após 2 s sem dano) | wiki |
| 3-4 | Poplust | 2.500 (4.075) | Druidas no alcance (inclusive ele): +17% velocidade e +15% pierce, **acumula até 5×** | wiki |
| 3-5 | Avatar of Wrath | 52.000 (56.075) | +5 alcance, +3 de dano, recarga 0,55 s; +1 de dano a cada 3.000 de RBE no seu lado (até +26) | wiki |

**Análise.** O 2-4 é uma das três fontes de "alt-eco" do jogo (junto com Supply Drop e Support Chinook) [F2]. O 3-5 escala com a pressão de bloons, uma mecânica feita para o PvP. O Poplust acumula de propósito, com teto de 5.

### Fazenda de Bananas (Banana Farm) · $1.000 · Suporte

**Base:** 3 bananas por rodada, $40 cada (**$120 por rodada**), distribuídas ao longo da rodada. As bananas **precisam ser coletadas** e apodrecem em 15 s. Retorno: 8,3 rodadas. Fonte: [B].

| Up | Nome | $ (acum.) | Mecânica | $/rodada | Conf. |
|---|---|---|---|---|---|
| 1-1 | Increased Production | 550 (1.550) | 5 bananas | 200 | wiki |
| 1-2 | Greater Production | 550 (2.100) | 7 bananas | 280 | wiki |
| 1-3 | Banana Plantation | 2.700 (4.800) | 16 cachos de $20 | 320 | wiki |
| 1-4 | Banana Research Facility | 16.000 (20.800) | 5 caixas de $600 | 3.000 | wiki |
| 1-5 | Banana Central | 55.000 a 66.000 (86.800) | 5 caixas de $2.800; +20% nas Research Facilities e no Benjamin | 14.000 | wiki |
| 2-1 | Long Life Bananas | 200 (1.200) | Bananas duram 30 s | 120 | wiki |
| 2-2 | Valuable Bananas | 700 (1.900) | +25% no valor | 150 | wiki |
| 2-3 | Monkey Bank | 4.800 (6.700) | Guarda o dinheiro no banco: pacotes na rodada + $450 no fim + **20% de juros por rodada**, até $14.000 | ~600+ | wiki/fandom |
| 2-4 | IMF loan | 8.000 (14.700) | Banco de $18.000; **hab.** (90 s): empréstimo de **$15.000** (depois paga a dívida) | - | wiki |
| 2-5 | Monkey-Nomics | 45.000 (59.700) | Banco de $30.000; a hab. dá $15.000 **sem dívida** a cada 40 s | - | wiki |
| 3-1 | Quality Soil | 400 (1.400) | Upgrades dos caminhos 1 e 2 **15% mais baratos** | 120 | wiki |
| 3-2 | Banana Salvage | 240 (1.640) | Bananas perdidas rendem 50%; +5% na venda | 120 | wiki |
| 3-3 | Marketplace | 2.800 (4.440) | Dinheiro automático: 14× $40 | 560 | wiki |
| 3-4 | Central Market | 13.000 (17.440) | 14× $160; Merchantmen +5% por Central Market | 2.240 | wiki |
| 3-5 | Monkey Wall Street | 46.000 (63.440) | Igual ao Central Market + **$10.000 no fim de cada rodada** | 12.240 | wiki |

**Análise.**
- Retorno aproximado (rodadas para se pagar): 1-4 em 6,9; 1-5 em 6,2; 3-4 em 7,8; 3-5 em 5,2.
- A economia do BTDB2 é agressiva, e a Fazenda é a principal alternativa à eco. O 2-5 dá $15.000 a cada 40 s, cerca de $375/s.
- No jogo real a coleta é manual até o 3-3. O clone paga tudo automaticamente e com valores bem menores.

### Fábrica de Espinhos (Spike Factory) · $600 · Suporte

**Base:** pilha de 5 espinhos a cada 1,75 s, 1 de dano, alcance 34, duração 40 s, [afiado]. **Área: 2,9/s.** Fonte: [F] (infobox).

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Bigger Stacks | 600 (1.200) | Pilhas de 10 (+40 no Perma-Spike) | wiki |
| 1-2 | White Hot Spikes | 600 (1.800) | [normal] (chumbo e congelado) | wiki |
| 1-3 | Spiked Balls | 2.700 (4.500) | Bolas: 2 de dano, +6 em cerâmica, +1 em fortificado, pierce 14 | wiki |
| 1-4 | Spiked Mines | 9.500 (14.000) | Ao acabar, explodem (10 de dano, pierce 60) e queimam | fandom |
| 1-5 | Super Mines | 90.000 (104.000) | A cada 3,5 s: minas de 50 de dano (+20 em cerâmica); explosões de 20 a cada espinho gasto e explosão final de 1.000 (pierce 80) | fandom |
| 2-1 | Faster Production | 500 (1.100) | Recarga ×0,8 (1,4 s) | wiki |
| 2-2 | Even Faster Production | 500 (1.600) | Recarga ×0,7 (0,98 s) | wiki |
| 2-3 | MOAB SHREDR | 2.500 (4.100) | +4 em MOAB por espinho (25 por pilha) | wiki |
| 2-4 | Spike Storm | 6.200 (10.300) | **Hab.:** 200 pilhas espalhadas em toda a trilha em 1 s (duram 10 a 13 s) | wiki |
| 2-5 | Carpet of Spikes | 40.000 (50.300) | Spike Storm automático a cada 15 s, +3 de dano, recarga ÷2 | fandom |
| 3-1 | Long Reach | 150 (750) | Alcance 34→42; pilhas duram 75 s | wiki |
| 3-2 | Smart Spikes | 500 (1.250) | 4× velocidade por 2,5 s no início da rodada; modos Perto, Longe e Esperto | wiki |
| 3-3 | Long Life Spikes | 1.200 (2.450) | Pilhas duram 170 s; o turbo inicial dura 5 s | wiki |
| 3-4 | Deadly Spikes | 3.200 (5.650) | 3 de dano, 240 s | wiki |
| 3-5 | Perma-Spike | 30.000 (35.650) | A cada 3 s: 11 de dano, pierce 50 (90 no 1-0-5), 300 s | wiki |

**Análise.** O caminho 3 barato com início de rodada acelerado é característico do PvP: cobre o "vazamento do começo da rodada". O 2-4/2-5 cobre todas as trilhas, o que vale mais nos mapas de várias trilhas.

### Vila dos Macacos (Monkey Village) · $1.000 · Suporte

**Base:** raio de influência de 40; sem ataque e sem buff na base. Fontes: [B], [F].

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Bigger Radius | 400 (1.400) | +8 de raio (+20%) | wiki |
| 1-2 | Jungle Drums | 1.500 (2.900) | Torres no raio com recarga ×0,85 | fandom |
| 1-3 | Primary Training | 800 (3.700) | Primárias e Pat no raio: +10% alcance, +15% pierce (mín. +1), +25% vel. do projétil | fandom |
| 1-4 | Primary Mentoring | 2.500 (6.200) | Primárias: +5 alcance, −20% recarga de habilidades e **tier 1 grátis** | fandom |
| 1-5 | Primary Expertise | 25.000 (31.200) | Primárias: +40% pierce (mín. +3), tiers 1 e 2 grátis; a Vila ganha uma Mega Balista teleguiada | fandom |
| 2-1 | Grow Blocker | 250 (1.250) | Bloqueia o regen de até 200 bloons no raio | wiki |
| 2-2 | Radar Scanner | 1.700 (2.950) | **Camo** para as torres no raio | wiki |
| 2-3 | Monkey Intelligence Bureau | 5.000 (7.950) | Torres no raio estouram **qualquer tipo** de bloon | wiki |
| 2-4 | Call to Arms | 23.500 (31.450) | **Hab.** (45 s): +50% velocidade e +50% pierce no raio por 8 s | wiki |
| 2-5 | Homeland Defense | 45.000 (76.450) | Hab.: +100% velocidade e pierce em **todas as torres da tela** por 17 s | wiki |
| 3-1 | Monkey Business | 500 (1.500) | −5% em torres e upgrades até o T3 no raio (fandom diz −10%) | wiki |
| 3-2 | Monkey Commerce | 500 (2.000) | Mais −5%; acumula com até 3 Commerces (máx. −20%) | wiki |
| 3-3 | Monkeyconomy | 1.500 (3.500) | 4 caixas de $100 por rodada ($400) | wiki |
| 3-4 | Monkey City | 7.200 (10.700) | Caixas de $400 ($1.600 por rodada); +10 de raio | wiki |
| 3-5 | Monkeyopolis | 20.000 (30.700) | **Sacrifica Monkeyconomies e Cities no raio**: $600 por rodada a cada $2.000 sacrificados, mais $4.200 de base; **envios dão +20% de eco** e as penalidades de eco caem 20% | wiki |

**Análise.**
- **Os buffs da Vila têm tetos claros no jogo real.** O desconto chega a no máximo 20% (3 Commerces), e o Shinobi e o Poplust acumulam com limite.
- No clone, os buffs acumulam sem teto e a Macacópolis falha no cruzamento x-0-5 (bug #3).
- O 3-5 do jogo real também melhora a eco, e o clone ignora isso.

### Macaco Engenheiro (Engineer Monkey) · $400 · Suporte

**Base:** pregos a cada 0,7 s, 1 de dano, pierce 3, alcance 40, [afiado]. **1 alvo: 1,43/s.** Fonte: [F] (infobox).

| Up | Nome | $ (acum.) | Mecânica | Conf. |
|---|---|---|---|---|
| 1-1 | Sentry Gun | 400 (800) | Torreta a cada 10 s, dura 25 s (a cada 0,98 s, pierce 2, alcance 45) | wiki |
| 1-2 | Faster Engineering | 350 (1.150) | Torreta a cada 6 s; espuma e armadilha 40% mais rápidas | wiki |
| 1-3 | Sprockets | 500 (1.650) | Recarga −40% no Engenheiro e nas torretas | wiki |
| 1-4 | Sentry Expert | 3.250 (4.900) | 4 tipos de torreta (esmagar, explosão, gelo, energia) escolhidos conforme os bloons | wiki |
| 1-5 | Sentry Champion | 32.000 (36.900) | Torretas campeãs: plasma a cada 0,03 s (3 de dano, pierce 5); explodem ao sumir (260 de dano, pierce 100) | wiki |
| 2-1 | Larger Service Area | 250 (650) | Alcance 40→60 (+4 nas torretas; escolhe onde pôr a armadilha) | wiki |
| 2-2 | Deconstruction | 350 (1.000) | +1 em MOAB e fortificado (pregos e torretas) | wiki |
| 2-3 | Cleansing Foam | 800 (1.800) | Espuma a cada 2 s: **remove camo e regen**, dá 1 de dano em chumbo e DDT, e o **DDT perde a imunidade de chumbo** | wiki |
| 2-4 | Overclock | 13.500 (15.300) | **Hab.:** recarga −40% numa torre escolhida (+50% na Fazenda) | wiki |
| 2-5 | Ultraboost | 100.000 (115.300) | Cada overclock também dá um bônus pequeno e **permanente** | desc. |
| 3-1 | Oversize Nails | 450 (850) | +5 pierce (8), [estilhaço] (pega congelado) | wiki |
| 3-2 | Pin | 200 (1.050) | Prega bloons leves e atordoa a camada seguinte por 1 s | wiki |
| 3-3 | Double Gun | 350 (1.400) | Recarga ÷2 | wiki |
| 3-4 | Bloon Trap | 3.600 (5.000) | Armadilha na trilha que **mata na hora** até 500 de RBE (sem MOAB); cheia, rende até $500 com bloons naturais ($0 com enviados) | wiki |
| 3-5 | XXXL Trap | 54.000 (59.000) | Armadilha de 10.000 de RBE que pega MOAB, BFB, ZOMG e DDT (menos BAD); $0,5 por RBE (até $5.000) | wiki |

**Análise.**
- A **Bloon Trap do jogo real tem capacidade** (500 de RBE) e **não rende dinheiro com bloons enviados**.
- No clone ela é uma pilha de 99 de dano com pierce 500 que renova sozinha, e isso explica por que lá está desbalanceada.
- A Espuma do jogo real remove camo e regen **também do DDT e do chumbo**; no clone isso falha (bug #6).

## 5. Heróis

O jogo tem 12 heróis e 12 variantes ("Hero Alts") com mecânicas diferentes. Na coluna de custo, o valor entre parênteses é o preço de comprar o nível 10 direto com dinheiro, segundo a Bloons Wiki. Fontes: [B], [F].

| Herói | $ | Ataque no nível 1 | Hab. nível 3 | Hab. nível 10 | Destaque | Variantes |
|---|---|---|---|---|---|---|
| **Quincy** | 450 (6.460) | Flecha a cada 0,95 s, 1 de dano, pierce 4, quica (50 u), alcance 50 | Rapid Shot: 3× velocidade (45 s) | Storm of Arrows: área de 6 de dano (+6 em MOAB) por 3 s | Camo no nível 5 | Cyber Quincy (atravessa obstáculos) |
| **Gwendolin** | 700 (13.620) | Bolas de fogo, pierce 3, extra em chumbo e congelado; não pega roxo | Cocktail of Fire: muralha de fogo | Firestorm: queima tudo (15/s em MOAB) e dá dano extra global às torres | No nível 4, buffa torres em volta (pierce e dano em chumbo) | Scientist Gwendolin (sinergia com cola e ácido) |
| **Striker Jones** | 750 | Míssil a cada 1,2 s, 1 de dano, pierce 10, r15, alcance 55 | Concussive Shell: atordoa o mais forte | Artillery Command: dispara as habs. de todas as Bombas e Morteiros sem gastar recarga | Buffa Bombas e Morteiros | Biker Bones |
| **Obyn Greenfoot** | 650 (7.040) | Lobos espirituais teleguiados, 2 de dano, pierce 5; não pega roxo | Brambles: arbusto de pierce 40 na trilha | Wall of Trees: absorve 2.000 de RBE (8.000 no nível 20) e dá dinheiro | Buffa torres mágicas | Ocean Obyn |
| **Capitão Churchill** | 1.500 (9.690) | Obus a cada 0,7 s, até 3 explosões (1 de dano, pierce 15), alcance 63 | Armor Piercing Shells: mais explosões e pega tudo | MOAB Barrage: 250 por míssil em até 10 dirigíveis (800 no nível 20) | O herói mais caro e o que sobe mais devagar | Sentai Churchill |
| **Benjamin** | 950 | Sem ataque; **dinheiro por rodada** | Biohack: +2 de dano nas 6 torres mais próximas | Syphon Funding: rebaixa em 1 camada os bloons que nascem no seu lado | Melhora Bancos e Marketplaces | DJ Benjammin' (renda por torre perto) |
| **Ezili** | 600 (9.690) | Maldição a cada 1,2 s (DoT + área de 1); vê camo | Heartstopper: bloqueia regen e anula a imunidade do roxo | MOAB Hex: 4% da vida + 1 por segundo por 8,5 s e o dirigível morre sem soltar filhos | Pega BAD no nível 20 | Smudge Catt Ezili |
| **Pat Fusty** | 900 | Tapa a cada 1,1 s (3 de dano, 5 em cerâmica) + área | Rallying Roar: +dano nas torres perto (e velocidade nas Primárias) | Big Squeeze: esmaga o dirigível mais forte (não BAD); 6 de uma vez no nível 20 | Conta como Primária para a Vila | Fusty the Snowman |
| **Agente Jericho** | 850 | Pistola, 3 de dano; sem chumbo e camo | Seize Assets: **rouba dinheiro do oponente** | Bloon Decoy: MOAB falso no caminho de envio do oponente, que absorve projéteis | Barateia os modificadores de envio | Highwayman, Star Captain |
| **Adora** | 650 (9.690) | Raios teleguiados a cada 1,0 s, 1 de dano, pierce 5, alcance 45 | Long Arm of Light: 2× alcance e 2,4× pierce | Ball of Light: orbe de 3 de dano a cada 0,05 s (20 no nível 20) | Sacrifício de torres por XP; vira Deusa Sol com o True Sun God | Fateweaver Adora |
| **Etienne** | 650 | 2 drones de dardo (1 de dano, pierce 2) que perseguem dentro do alcance | Drone Swarm: +4 drones | UCAV: mísseis teleguiados (permanente no nível 20) | Buffa torres de voo | Beetienne |
| **Bonnie** | 700 | Dinamite na trilha a cada 3 s (1 de dano, pierce 20) | Mass Detonation | Caminhão que atordoa bloons | Carrinhos de "Bloonstone" que rendem dinheiro | - |

**Análise.**
- **Os heróis do BTDB2 são bem mais "de suporte e PvP" que os do clone:** Jericho rouba dinheiro e confunde envios, e Benjamin rebaixa os bloons do adversário.
- No clone, o Benjamin chega a $2.530 por rodada, e o Jericho não tem efeito no modo solo.
- **Nomes no clone:** "Sauda", "Psi", "Geraldo", "Corvus", "Rosalia" e "Silas" são heróis do BTD6 e **não existem no BTDB2**. Já Bonnie existe no BTDB2 e falta no clone.

## 6. Mapas

"RBS" = **segundos que um bloon vermelho leva para percorrer a trilha** (unidade da Blooncyclopedia). A disponibilidade depende da arena. Os mapas com duas trilhas "(natural / envio)" têm um caminho só para os bloons enviados, que costuma ser mais curto.

| Mapa | Disponível em | Entradas | Cruzamentos | Comprimento (RBS) | Removíveis ($) |
|---|---|---|---|---|---|
| Street Party | White Wasteland+ | 2 (natural/envio) | 0 | 18,7 / **13,8** | - |
| Inflection | ZOMG Superdome+ | 2 (natural/envio) | 0 | 16,4 / 14,2 | - |
| Glade | Todas | 2 | 0 | 21,7 / 14,3 (envio de MOAB) | 350 |
| Time's Up | White Wasteland+ | **4** | 0 | 15,9 (mais longa) | - |
| Ports | ZOMG Superdome+ | 1 | 1 | 16,3 / 22,5 | 350 |
| Koru | Todas | 1 | 1 | 25,9 / 17,7 | - |
| Basalt Columns | Todas | 2 | 0 | 18,1 | - |
| Building Site | White Wasteland+ | 1 | 1 | 18,3 / 22,0 | - |
| Pirate Cove | Lead Dungeon+ | 2 | 0 | 18,6 | 200 |
| Off-Tide | Todas | 1 | 0 | 19,7 | - |
| Garden | Todas | 2 | 0 | 26,7 / 20,3 | 500 |
| Precious Space | White Wasteland+ | 2 | 1 | 21,2 / 21,9 | (vende terreno) |
| Salmon Ladder | White Wasteland+ | 1 | 0 | 21,5 | - |
| Sun Palace | Todas | 1 | 2 | 24,2 / 22,7 | - |
| Star | Todas | 1 | 1 | 25,3 / 23,3 | - |
| COBRA Command | Todas | 1 | 2 | 24,3 | 350 |
| Castle Ruins | Todas | 2 | 0 | 24,7 | 350 |
| Mayan | até ZOMG Superdome | 2 | 0 | 24,7 | - |
| Sands of Time | Todas | 2 | 0 | 25,0 | - |
| Bloonstone Quarry | Todas | 1 | 2 | 25,8 | - |
| Oasis | Todas | 1 | 0 | 25,9 | 350 |
| Dino Graveyard | Todas | 1 | 0 | 26,1 | 350 |
| Banana Depot | White Wasteland+ | 1 | 0 | 26,9 | 250 a 500 (10 itens) |
| Bot Factory | Lead Dungeon+ | 2 | 0 | 27,5 | 400 |
| Bloontonium Mines | até BFB Colosseum | 2 | 0 | 32,1 / 27,9 | 500 |
| Island Base | só Guerra de Clãs | 2 | 1 | 29,3 / 28,0 | 300 a 800 |
| In the Wall | até BFB Colosseum | 2 | 0 | 29,7 | - |
| Thin Ice | até BFB Colosseum | 2 | 0 | 31,8 | 350 |
| Up On The Roof | até ZOMG Superdome | 1 | 0 | 33,6 | 350 |
| Docks | até MOAB Pit | 2 | 0 | **36,4** | 350 |
| Club Jammin', Magma Mixup, Neo Highway, Park, Splashdown | vários | - | - | n/d | - |

**Análise.**
- **Os mapas mais difíceis são os de trilha curta**, porque os envios chegam rápido: Street Party, Inflection, Glade (envio de MOAB em 14,3), Time's Up (4 entradas) e Ports. Eles ficam nas arenas altas (White Wasteland+ ou ZOMG Superdome+).
- **Os mais fáceis são os de trilha longa**, como Docks, Up On The Roof, Thin Ice e Bloontonium Mines, e ficam só nas arenas baixas. **A própria Ninja Kiwi usa o comprimento como régua de dificuldade.**
- O jogo não dá rótulo oficial de dificuldade; a comunidade agrupa os mapas pela arena [F6].
- **No clone, os 4 mapas têm trilhas de 17 a 59 s para o vermelho (convertendo os px).** O Prado (33,5 s) e a Espiral (59 s) seriam mapas "de iniciante" pela régua do jogo real, e a Encruzilhada (17,5 s, duas trilhas) seria de arena alta.
- Todos os mapas do BTDB2 têm água [F6]. O Prado do clone não tem.

## 7. Rodadas, economia e modo Batalha

### 7.1 Estrutura

- **Não há dificuldades Fácil, Médio, Difícil e Impossível.** A dificuldade vem de três coisas:
  - o mapa (o comprimento em RBS);
  - a arena, que libera modificadores de envio e troca o conjunto de mapas;
  - a pressão do oponente.
- Existe um modo solo, o **Hero Challenges**, contra um herói controlado pela IA que também envia bloons [F7]. A fonte não detalha regras numéricas desse modo, então os valores ficam como não encontrados.
- **Rodadas:** 40 normais e Morte Súbita até a 50 (B.A.D. natural na R50). A série segue, em geral, as rodadas pares do BTD6, com mudanças [F5]. No evento Bananza são 101 rodadas.

### 7.2 Curva das rodadas naturais

Os números são da Bloons Wiki [F5], que tem aviso de desatualizada: a versão 2.0 mudou os tempos mínimo e máximo das rodadas.

| R | Bloons | Duração aprox. |
|---|---|---|
| 1 | 35 vermelhos | 19 s |
| 5 | 57 azuis | 21,6 s |
| 10 | 6 pretos | 5,3 s |
| 11 | 25 brancos | 8 s |
| 12 | 60 azuis + verde camo (**1º camo**) | 9 s |
| 14 | 14 chumbos (**1º chumbo**) | 5 s |
| 15 | pretos, brancos, zebras e 2 zebras regen | 15,9 s |
| 16 | 15 pretos, 20 brancos, 20 roxos | 28 s |
| 17 | 160 amarelos + 18 arco-íris | 27 s |
| 18 | 140 rosas + 20 verdes camo regen | 21 s |
| 19 | rosas, brancos, zebras, 25 chumbos, 12 cerâmicas | 25 s |
| **20** | **MOAB** | 1 s |
| 23 | 10 cerâmicas fortificadas | 7 s |
| 24 | rosa regen, 60 roxos camo regen, 40 arco-íris, 6 cerâmicas fort. | 43,7 s |
| 25 a 29 | cerâmicas, chumbos fortificados, 2 a 5 MOAB | 16 a 44 s |
| **30** | **BFB** | 1 s |
| 31 | 250 roxos, 15 arco-íris camo, 5 MOAB, 2 MOAB fort. | 48,3 s |
| 35 | 120 brancos camo regen, 200 arco-íris, 4 MOAB | 41,1 s |
| 37 | 135 cerâmicas (várias fort. e camo regen) + BFB | 82,4 s |
| 39 | 80 roxos, 150 arco-íris, 147 cerâmicas + BFB | 90 s |
| **40** | **ZOMG** | 2 s |
| 41 a 44 | 10 a 50 MOAB, 5 a 15 BFB, 2 ZOMG | 15 a 36 s |
| 45 | 50 chumbos camo regen fort. + **3 DDT** | 11,9 s |
| 46 a 49 | MOAB fort., 25 a 30 BFB, 4 a 8 ZOMG | 15 a 35 s |
| **50** | **B.A.D.** | 1 s |

**Análise.**
- **Os marcos chegam muito antes que no clone:** camo na R12 (clone: R24), chumbo na R14 (R28), MOAB na R20 (R40), BFB na R30 (R60), ZOMG na R40 (R80), DDT na R45 (R90) e B.A.D. na R50 (R100).
- O BTDB2 comprime a curva do BTD6 em metade das rodadas. **O clone usa a curva clássica de 100 rodadas também no modo Batalha**, e o jogo fica bem mais lento.
- **O tempo entre rodadas depende de quem limpa primeiro** (2 s ou 4 s). Isso premia defesa rápida; o clone usa uma pausa fixa de 5 s.

### 7.3 Eco e envios

A **eco começa em 250** e é paga a cada 6 s. Cada envio muda a eco de forma permanente, e os envios de dirigível **reduzem** a eco.

**Modificadores de envio** (preço multiplicado): regen ×1,6 (a partir da R8), camo ×2 (R12), camo+regen ×3,2 (R12) e fortificado ×2 (R18, dobra a perda de eco em dirigíveis). Não existem na arena mais baixa [F3].

Tabela de envios da Bloons Wiki, com dados de antes da 4.0.2 [F3]:

| Envio | $ | Eco | Rodadas | Intervalo/bloon | Se paga em |
|---|---|---|---|---|---|
| 8 vermelhos agrupados | 20 | +1,0 | 1 a 11 | 0,1 s | 120 s |
| 5 azuis espaçados | 15 | +0,8 | 1 a 2 | 0,3 s | 113 s |
| 6 azuis agrupados | 24 | +1,1 | 3 a 12 | 0,1 s | 131 s |
| 5 verdes agrupados | 35 | +1,4 | 5 a 16 | 0,08 s | 150 s |
| 4 amarelos agrupados | 40 | +1,6 | 7 a 19 | 0,06 s | 150 s |
| 4 rosas agrupados | 60 | +2,3 | 9 a 50 | 0,05 s | 157 s |
| 5 brancos agrupados | 65 | +2,5 | 10 a 21 | 0,07 s | 156 s |
| 4 pretos agrupados | 75 | +2,9 | 10 a 25 | 0,08 s | 155 s |
| 5 roxos agrupados | 115 | +2,9 | 11 a 50 | 0,035 s | 238 s |
| 3 zebras agrupadas | 120 | +3,5 | 11 a 29 | 0,08 s | 206 s |
| 4 chumbos agrupados | 150 | +4,5 | 12 a 22 | 0,15 s | 200 s |
| **60 chumbos "Tight"** | 1.200 | +7,0 | 23 a 50 | 0,017 s | 1.029 s |
| 4 arco-íris agrupados | 250 | +6,0 | 13 a 50 | 0,1 s | 250 s |
| 1 cerâmica espaçada | 150 | +5,0 | 13 a 15 | 0,65 s | 180 s |
| 2 cerâmicas agrupadas | 350 | +5,0 | 16 a 27 | 0,12 s | 420 s |
| 30 cerâmicas "Tight" | 3.200 | 0 | 24 a 50 | 0,02 s | - |
| 1 MOAB espaçado / agrupado | 1.000 / 900 | 0 | 17 a 18 / 19 a 24 | 3 / 0,5 s | - |
| 15 MOAB "Tight" | 4.500 | −50 | 25 a 50 | 0,067 s | - |
| 1 BFB espaçado / agrupado | 1.600 / 1.300 | −25 | 20 a 21 / 22 a 26 | 3,5 / 0,6 s | - |
| 10 BFB "Tight" | 10.000 | −150 | 27 a 50 | 0,12 s | - |
| 1 ZOMG espaçado / agrupado | 5.500 / 4.500 | −100 | 22 a 23 / 26 a 29 | 6 / 1 s | - |
| 4 ZOMG "Tight" | 12.000 | −400 | 30 a 50 | 0,3 s | - |
| 1 DDT espaçado / 3 agrupados | 2.500 / 6.000 | −150 / −200 | 26 a 27 / 28 a 50 | 1,4 / 0,2 s | - |
| 1 B.A.D. espaçado / agrupado | 15.000 | −400 | 30 a 31 / 32 a 50 | 7 / 1 s | - |

**Análise.**
- A eco do BTDB2 **troca de "melhor envio" ao longo da partida**: cada envio tem janela de rodadas, e os envios antigos saem da lista.
- Segundo a wiki, desde a 4.0.2 o envio com o maior ganho de eco possível (sem olhar o custo) é cerâmicas agrupadas nas R16 a 22 e chumbos "Tight" da R23 em diante [F2].
- No clone não há janela de rodadas (os envios valem até o fim), o dirigível custa 3 a 4× mais e chega bem mais tarde, e não há modificadores de envio.
- **Monkeyopolis** e os eventos multiplicam a eco [F2].

## 8. Bugs e comportamentos estranhos documentados

| Onde | O que acontece | Fonte |
|---|---|---|
| Cryo Cannon (Gelo 3-3) | Com a prioridade de camo e o modo "Perto" ao mesmo tempo, a prioridade de camo não funciona | [B] |
| Quick Shots (Dardo 2-1) | O texto diz "15% mais rápido", mas é recarga ×0,85, ou seja, +17,6% de velocidade | [B] |
| Deep Freeze (Gelo 2-2) | O texto promete congelar por mais tempo, mas a duração não muda | [B] |
| Transforming Tonic (Alquimista 2-4) | O texto diz 20 s, mas dura 17,5 s | [F] |
| Support Chinook (Heli 2-4) | O texto ainda cita "vidas", mas a caixa deixou de dar vidas na 4.13 | [B] |
| Grouped Blues (envio) | A mudança de eco da 1.0.4 não entrou no jogo; foi corrigida na 1.0.5 | [F3] |

Não achei uma lista oficial de bugs ativos da versão 4.13. As notas de atualização de cada versão ficam na página "Update history" da wiki, que não foi lida por completo.

## 9. Resumo de balanceamento no meta atual

- **Torres citadas pela wiki como resposta padrão às investidas perigosas:**
  - R11 (roxos e zebras agrupados): 2-0-3 Cluster Bombs, Hydra Rocket Pods e 4-0-2 M.O.A.R. Glaives [F3].
  - Tight Leads: Ultra-Juggernaut, Glaive Lord, Recursive Cluster, Super Maelstrom, Snowstorm com Embrittlement, Bloon Solver, 5-2-0 Energizer, Carrier Flagship, Spectre, Apache/Comanche, The Big One ou Artillery Battery, Plasma Accelerator, Arcane Spike, Sun Avatar, Master Bomber com Shinobi, Lead to Gold, Spirit of the Forest, Spiked Mines e Sentry Champion. **Só o Sniper não tem resposta fácil** [F3].
- **Economia:** a base é a eco pelos envios. Por cima dela entram a renda alternativa por tempo (Supply Drop, Jungle's Bounty, Chinook), a renda por rodada (Fazenda, Merchantman, Monkeyopolis) e a renda por interação (Bloon Trap, Rubber to Gold) [F2].
- **Composições populares** citadas na página do Dardo: Dart-Farm-Sniper e Dart-Farm-Spac(tory) em Inflection e Glade [B1].
- **Instakill de dirigível** (ganchos do Monkey Pirates, Misdirection do Jericho) é o contra-jogo dos envios de dirigível. Por isso a wiki aconselha mandar 2 dirigíveis comuns em vez de 1 fortificado contra quem tem ganchos [F3].
- **Não encontrei** um ranking quantitativo de força por custo (taxa de vitória por torre) em fonte confiável; a classificação acima é qualitativa.

## 10. Comparação com o clone

A coluna "Clone" vem de `docs/analise-mecanicas.md` e do código em `src/jogo/`.

| Item | Jogo real (BTDB2 4.13) | Clone | Impacto | Ajuste sugerido no clone |
|---|---|---|---|---|
| Duração da partida | 40 rodadas + Morte Súbita até a 50; renda para na R50 | Batalha sem fim, com as 100 rodadas do modo clássico | **Alto**: partidas longas demais e dirigíveis tardios | Criar um conjunto de 50 rodadas para a Batalha (seção 7.2) e a regra de Morte Súbita |
| Marcos das rodadas | Camo R12, chumbo R14, MOAB R20, BFB R30, ZOMG R40, DDT R45, B.A.D. R50 | Camo R24, chumbo R28, MOAB R40, BFB R60, ZOMG R80, DDT R90, B.A.D. R100 | Alto | Mesmo conjunto novo |
| Tempo entre rodadas | 2 s (os dois limparam), 4 s (um limpou) ou o limite de r×1,5+8,5 s | Pausa fixa de 5 s depois da agenda | Médio | Implementar as 3 condições |
| Envios | Dirigíveis baratos e cedo (MOAB $900 na R17, B.A.D. $15.000 na R30); janela de rodadas; envios "Tight" | MOAB $1.500 na R25, B.A.D. $60.000 na R45; sem janela | **Alto**: a pressão ofensiva é bem menor | Copiar a tabela da seção 7.3 com janelas e intervalos |
| Modificadores de envio | Regen ×1,6 (R8), camo ×2 (R12), fortificado ×2 (R18) | Só variantes fixas ("6 verdes camo" etc.) | Médio | Três botões de modificador que multiplicam o preço |
| Base econômica | $650, eco de 250 a cada 6 s, 150 vidas | **Igual** | - | Manter |
| Velocidade dos bloons | Chumbo 1,8×, zebra 3,0×, roxo 3,4×, BFB 0,34×, ZOMG 0,28× | Chumbo 1,0×, zebra 1,8×, roxo 3,0×, BFB 0,25×, ZOMG 0,18× | Médio | Atualizar `criar_bloons()` |
| Vida dos bloons | B.A.D. 12.500; chumbo fort. 6; cerâmica fort. 30; dirigíveis ganham vida depois da R25 | B.A.D. 20.000; 4; 20; sem crescimento | Médio | Ajustar os valores e aplicar um fator de vida por rodada |
| DDT | 4 cerâmicas camo+regen; imune a afiado, estilhaço, gelo, energia e explosão | 6 cerâmicas; imune só a afiado e explosão | Médio | Corrigir filhos e imunidades |
| Imunidade do chumbo | Afiado, estilhaço, gelo **e energia** | Afiado e gelo | Médio: o Mago base não deveria pegar chumbo | Somar energia à máscara |
| Regen | A cada 2,6 s | A cada 3,0 s | Baixo | Trocar a constante |
| Tipos de dano | 10 tipos (estilhaço, plasma, fogo, ácido, imparável…) | 5 tipos | Médio | Pelo menos ácido (cola, alquimista) e plasma/fogo (roxo) |
| Cola e chumbo | A cola (ácido) **pega chumbo**; Glue Strike dá +2 de dano de tudo | A cola é bloqueada por chumbo (bug #6) | Alto na torre | dtype ácido/normal na cola; debuff de +2 |
| Buffs que acumulam | Shinobi até 15×, Poplust até 5×, desconto no máximo 20%; Jungle Drums não acumula | Tudo acumula sem teto (bug #11) | **Alto** | Teto por tipo de buff, como no jogo |
| Alquimista 1-3/1-4 | Poção temporária numa torre por vez (por tiros ou tempo); permanente só no 1-5 | Aura permanente em área | **Alto**: o clone deixa o Alquimista forte demais | Buff por torre com duração |
| Bloon Trap (Eng. 3-4) | Guarda até 500 de RBE; $1 por RBE natural e $0 por enviado; precisa esvaziar | Pilha de 99 de dano com pierce 500 que renova | **Alto**: é o upgrade mais forte do clone | Implementar a capacidade e o esvaziamento |
| Necromante (Mago 3-4) | Cemitério de bloons estourados e zumbis que andam ao contrário | Pilha fixa | Médio | Contador de estouros e bloons aliados |
| Morteiro e Dartling | Miram no ponto ou cursor escolhido | Miram sozinhos no alvo | Médio | Comando de mira no ponto (o Morteiro já tem um "alvo" possível) |
| Mira Infalível (Ás 3-3) | Dardos teleguiados em qualquer distância | Não funciona (bug #7) | Médio | Corrigir |
| Master Bomber (Ninja 3-5) | Melhora a **bomba grudenta** (1.000 de dano, alcance infinito) | Melhora a bomba de luz (bug #2) | Alto | Corrigir o índice |
| Energizer (Sub 1-5) | Buff global de recarga de habilidades e XP; pulso de 5 de dano | Melhora a aura de camo (bug #1) | Alto | Corrigir o índice e somar o buff de habilidades |
| Monkeyopolis (Vila 3-5) | Sacrifica Villages de economia; +20% de eco nos envios | +$5.000, que falha em x-0-5 (bug #3) | Médio | Mecânica de sacrifício e bônus de eco |
| Revelar camo | Shimmer, Signal Flare, Embrittlement, Espuma e Energizer revelam **DDT** | Bloqueado pela imunidade (bug #6) | Alto contra DDT | Efeito utilitário sem checar imunidade |
| Fazenda | Bananas coletáveis (manual até o 3-3); $120 na base, $14.000 no 1-5, Wall Street +$10.000 por rodada | Automático; $80 na base, $4.080 no 1-5 | Alto na economia | Recalibrar os valores |
| Custos | Ex.: Tachinha 1-5 $35.500, Dardo 3-5 $27.000, Super base $2.000 | Ex.: $45.500, $25.000, $2.500 | Médio | Copiar as colunas de custo da seção 4 |
| Heróis | 12 heróis + variantes; Bonnie existe; Sauda, Psi, Geraldo, Corvus, Rosalia e Silas **não existem** | 18 heróis, 6 deles do BTD6 | Médio | Trocar os 6 por Bonnie e variantes, ou marcar como "extra" |
| Mecânicas de PvP | Seguros contra vazamento, Jericho roubando dinheiro e confundindo envios, Avatar of Wrath escalando com o RBE do seu lado | Não existem | Médio | Implementar gradualmente |
| Mapas | 35 mapas com dificuldade por RBS (13,8 a 36,4), trilha de envio separada, água em todos, obstáculos removíveis pagos | 4 mapas; Prado sem água; mesma trilha para envios | Médio | Trilha de envio, água no Prado e obstáculos removíveis |

## Fontes

- **[B0]** Blooncyclopedia, "Bloons TD Battles 2" (versão mais recente 4.13): https://www.bloonswiki.com/Bloons_TD_Battles_2
- **[B1]** Blooncyclopedia, "Dart Monkey (Battles 2)" e o histórico de atualizações dela: https://www.bloonswiki.com/Dart_Monkey_(Battles_2)
- **[B]** Blooncyclopedia, páginas de torre, upgrade, herói, bloon e mapa em "(Battles 2)", lidas pela API em 30/09/2026 (25 torres, 335 upgrades, 24 heróis, 17 bloons, 35 mapas). Ex.: https://www.bloonswiki.com/Super_Monkey_Fan_Club_(Battles_2) , https://www.bloonswiki.com/BAD_(Battles_2) , https://www.bloonswiki.com/Glade . A unidade RBS é definida em https://www.bloonswiki.com/Module:BTDB2_map_info
- **[F]** Bloons Wiki (fandom), páginas "(BTDB2)" de torres e heróis (infobox e efeitos dos upgrades): https://bloons.fandom.com/wiki/Dart_Monkey_(BTDB2) e equivalentes
- **[F2]** Bloons Wiki, "Eco (BTDB2)" (tabela válida na 4.0.2): https://bloons.fandom.com/wiki/Eco_(BTDB2)
- **[F3]** Bloons Wiki, "Bloon Sends (BTDB2)": https://bloons.fandom.com/wiki/Bloon_Sends_(BTDB2)
- **[F4]** Bloons Wiki, "Sudden Death": https://bloons.fandom.com/wiki/Sudden_Death
- **[F5]** Bloons Wiki, "Rounds (BTDB2)" (aviso de desatualizada desde a 2.0): https://bloons.fandom.com/wiki/Rounds_(BTDB2)
- **[F6]** Bloons Wiki, "Bloons TD Battles 2" (arenas, modos e mapas por arena): https://bloons.fandom.com/wiki/Bloons_TD_Battles_2
- **[F7]** Bloons Wiki, "Bloons TD Battles 2", seção Hero Challenges (mesma página de [F6])

### Itens sem dado ("não encontrado" ou só descrição do jogo)

- **20 upgrades** estão só com a descrição do jogo ou sem dado (19 com descrição, 1 sem nada) e **2** têm dado parcial. São eles: Heli 1-1 e 3-3; Morteiro 1-1 e 1-2; Dartling 3-3 a 3-5; Mago 1-1, 2-4, 3-1 e 2-5 (parcial); Super 1-5, 2-4, 2-5 e 3-1 a 3-4; Ninja 1-2 e 3-1; Engenheiro 2-5.
- **5 mapas** estão sem comprimento publicado.
- **Regras numéricas do Hero Challenges** não foram encontradas.
- A Blooncyclopedia estava sem página detalhada para esses itens, e a Bloons Wiki só tinha texto genérico ou nada.
