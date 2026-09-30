# Análise de mecânicas e balanceamento

Análise de cada torre (base + 15 upgrades), dos 18 heróis, dos 17 bloons, das 4 dificuldades, dos 4 mapas e das 100 rodadas, feita a partir do código atual (`src/jogo/dados.cpp`, `stats.cpp`, `sim.cpp`, `rodadas.cpp`, `mapas.cpp`).

Os números das tabelas **não são estimados à mão**: saem de um programa que usa o próprio núcleo do jogo (`calcular()`, `rbe()`, `Caminho::comprimento`, `agenda_da_rodada()`), então batem com o que o jogo faz hoje.

## 0. Principais achados

**Bugs de mecânica** (o upgrade não faz o que a descrição promete). Detalhes na seção 8.

1. **Índice de ataque errado (`"a": N`) com cruzamento**:
   - Submarino 5-x-x (Energizador melhora a aura de camo, não o reator).
   - Ninja x-x-5 (Mestre Bombardeiro melhora a Bomba de Luz, não a Grudenta).
   - Vila x-0-5 (Macacópolis não dá os $5.000).
   - Bucaneiro 0-1/2-4/5 (a renda não sobe) e 4/5-1/2-0 (o Tiro Quente vai para os aviões).
   - Druida 2-5-0 (o Espírito da Floresta melhora o relâmpago).
2. **Auras e pilhas "utilitárias" com tipo afiado** (dano 0) são bloqueadas por imunidade. Por isso não revelam DDT (Submergir, Cintilar, Sinalizador), não tiram regen de chumbo (Vila 2-1, Espuma) e não desaceleram nem empurram chumbo e congelados (Vento Ártico, Corrente Descendente, Tornado). A cola também não gruda em chumbo.
3. **A Mira Infalível (Ás 3-3) não funciona**: o radial dispara sem alvo, então não há teleguiado.
4. **A Cola perde a corrosão** do caminho 1 se o caminho 3 for comprado, e o **Gelo 3-3** apaga o Permafrost e o dano [normal] do cruzamento.
5. **O Príncipe das Trevas (Mago 3-5)** transforma a aura de Cintilar em uma aura de dano 4 × 999 alvos, sem querer.
6. **Os buffs se acumulam sem limite** (várias vilas/alquimistas multiplicam a cadência, com piso de ×0,4). A Nau Capitânia aplica o buff **uma vez por ataque** (até ×0,61), e as Táticas Shinobi também se aplicam à própria torre.
7. **Upgrades sem efeito**: Cola 1-1 (Cola Encharcada) e Ás 3-2 (Rota Centralizada).

**Balanceamento**:

- **Mais fortes por custo**:
  - Engenheiro 3-4 Armadilha Bloon: $8,1 mil no total, 99 de dano por bloon com pierce 500.
  - Dartling 1-5 Raio da Perdição.
  - Super Macaco 1-x.
  - Ás 3-5.
  - Alquimista 1-x (buff permanente).
- **Mais fracos**: Cola sem cruzamento, Heli 2-x, Vila 3-4/3-5 (a Metrópole leva 50 rodadas para se pagar), Druida 2-4 e o Submarino 1-2 (mira global, mas projétil de 260 px).
- **Dificuldade dos mapas está invertida**: a Espiral ("Avançado") tem a trilha **77% mais longa** que o Prado e o melhor ponto de cobertura. A Encruzilhada ("Intermediário") é a mais difícil: duas trilhas de metade do tamanho, com os bloons alternando entre elas.
- **A curva das rodadas finais é irregular**: as rodadas geradas (81–84, 86–89, 91–94, 96–99) são **muito** mais pesadas que as rodadas "chefe". A 89 tem 70.940 de RBE e a 90 (6 DDT) só 6.144; a 99 tem 136.172 e a 100 (B.A.D.) só 56.384.
- **Qualquer M.O.A.B. que vaza (616 vidas) acaba a partida** em todas as dificuldades, inclusive no Fácil (200 vidas).

## 1. Como ler as métricas

- **1 alvo/s**: dano por segundo em um bloon isolado (dano do golpe + dano de área ÷ recarga). Contra alvos de 1 camada ela mede estouros/s.
- **Área/s**: potencial em grupo = projéteis × (pierce × dano + pierce da explosão × dano da explosão) ÷ recarga. Para aura é pierce × dano; para pilha é pierce da pilha × dano; para corrente é (saltos+1) × dano. É o teto teórico com a trilha cheia.
- **MOAB/s**: como "1 alvo", somando o bônus `moab` por golpe.
- Buffs de outras torres, habilidades e fragilização **não** entram nas métricas.
- Os custos são da dificuldade **Médio** (×1,0). Os outros usam ×0,85 / ×1,08 / ×1,2, arredondados para múltiplos de $5.

## 2. Mecânicas gerais do motor

| Sistema | Como funciona no código |
|---|---|
| Passo de simulação | Fixo em 1/30 s. O mapa tem 1040×720 px; o vermelho anda 95 px/s. |
| Tipos de dano | `afiado`, `explosão`, `gelo`, `energia`, `normal`. O tipo `normal` ignora todas as imunidades. O padrão de um ataque sem `dtype` é **afiado**. A explosão usa `sdtype` (padrão explosão). |
| Imunidade | Golpe bloqueado **gasta pierce igual** (conta como acerto) e não aplica nenhum efeito (cola, camo, regen, lentidão). |
| Congelado | Enquanto congelado, o bloon fica imune a **afiado**. MOAB congela metade do tempo e só com `moab_congela`. Branco e zebra não congelam. |
| Pierce | Cada bloon atingido gasta 1 pierce. O projétil não acerta o mesmo bloon duas vezes, e os filhos nascem marcados como já atingidos por ele. |
| Excesso de dano | O dano que sobra passa para os filhos (até 12 níveis), exceto para filhos MOAB. |
| Bônus | `moab` (em dirigíveis), `cer` (em cerâmica) e `fort` (em fortificados) somam dano fixo por golpe. A fragilização do bloon também soma em **todo** golpe de qualquer torre. |
| Status em MOAB | Atordoar ×0,4 da duração, empurrar ×0,35 da distância, congelar ×0,5. Lentidão e cola só com a flag `moab_*`. |
| Mira | Primeiro / último / perto / forte (forte = maior `rank`, depois o mais avançado). Ataques com `alvo: forte` sempre usam "forte". |
| Camo | Só é visto por torre com `camo` ou pelo buff de radar. `retira_camo` remove de vez. O DDT nasce camo e seus filhos herdam camo e regen. |
| Regen | A cada 3 s sem levar dano, o bloon sobe uma camada até o tipo original. A cerâmica regen **não** recupera vida; só os filhos voltam a ser cerâmica. |
| Fortificado | Só chumbo (4), cerâmica (20) e dirigíveis (vida ×2) têm versão fortificada. |
| Cruzamento | No máximo 2 caminhos abertos e só 1 acima do tier 2 (padrão 5-2-0). Os efeitos são aplicados **na ordem caminho 1 → 2 → 3**, e isso importa para `subst`, `valor_x`, `cola` e índices `a`. |
| Venda | 70% do investido (90% com Fazenda 3-2). |
| Buffs | Recalculados a cada 0,5 s, a partir de toda torre com `buffs` em algum ataque. **Se acumulam**: a cadência multiplica (piso ×0,4); alcance, dano, pierce, MOAB e ouro somam. O buff de dano só vale em ataques com dano > 0. |
| Habilidades | Começam com 50% da recarga já passada. |
| Economia solo | $650 inicial; **$1 por camada estourada** (não por ponto de vida); fim de rodada paga $100 + nº da rodada + rendas. |
| Heróis | XP = 1 por estouro + (20 + 2×rodada) no fim da rodada (solo) ou 10 + rodada (batalha). Nível 20 com 48.406 XP (fórmula 180·(n−1)^1,9). Só 1 herói por pista. |
| Vazamento | Tira vidas igual ao **RBE restante** do bloon (vida + filhos). |

## 3. Bloons

| Bloon | Vida (fort.) | Velocidade | RBE (fort.) | $ se estourado todo | Imunidades | Congela | Filhos |
|---|---|---|---|---|---|---|---|
| Vermelho | 1 | 1,00× = 95 px/s | 1 | 1 | – | sim | – |
| Azul | 1 | 1,40× = 133 | 2 | 2 | – | sim | 1 vermelho |
| Verde | 1 | 1,80× = 171 | 3 | 3 | – | sim | 1 azul |
| Amarelo | 1 | 3,20× = 304 | 4 | 4 | – | sim | 1 verde |
| Rosa | 1 | 3,50× = 332 | 5 | 5 | – | sim | 1 amarelo |
| Preto | 1 | 1,80× = 171 | 11 | 11 | explosão | sim | 2 rosas |
| Branco | 1 | 2,00× = 190 | 11 | 11 | gelo | **não** | 2 rosas |
| Roxo | 1 | 3,00× = 285 | 11 | 11 | energia | sim | 2 rosas |
| Chumbo | 1 (4) | 1,00× = 95 | 23 (26) | 23 | afiado + gelo | sim | 2 pretos |
| Zebra | 1 | 1,80× = 171 | 23 | 23 | explosão + gelo | **não** | preto + branco |
| Arco-íris | 1 | 2,20× = 209 | 47 | 47 | – | sim | 2 zebras |
| Cerâmica | 10 (20) | 2,50× = 238 | 104 (114) | 95 | – | sim | 2 arco-íris |
| M.O.A.B. | 200 (400) | 1,00× = 95 | 616 (856) | 381 | – | não | 4 cerâmicas |
| B.F.B. | 700 (1.400) | 0,25× = 24 | 3.164 (4.824) | 1.525 | – | não | 4 MOAB |
| Z.O.M.G. | 4.000 (8.000) | 0,18× = 17 | 16.656 (27.296) | 6.101 | – | não | 4 BFB |
| D.D.T. | 400 (800) | 2,75× = 261 | 1.024 (1.484) | 571 | afiado + explosão; **camo nativo** | não | 6 cerâmicas camo+regen |
| B.A.D. | 20.000 (40.000) | 0,18× = 17 | 56.384 (99.044) | 13.916 | – | não | 2 ZOMG + 3 DDT |

**Observações:**

- **Rosa e amarelo são os mais rápidos** (3,2–3,5×) e são eles que vazam no começo do jogo. Torres de projétil lento (Bomba, Morteiro) erram esses bloons.
- **BFB e ZOMG são 4–5× mais lentos que o MOAB** (valores do original). No Prado, um ZOMG leva 186 s para atravessar, então quase qualquer DPS o derruba. O DDT (261 px/s, camo e imune a afiado e explosão) é o dirigível mais perigoso.
- **O chumbo é imune a afiado e gelo** e, por causa do bug do tipo padrão, também à cola e às auras utilitárias. Para ele contam Bomba, Mago, Alquimista, Gelo 1-2 e toda torre [normal].
- O branco e o roxo nascidos de um regen voltam ao próprio tipo. Já um **preto regen filho de chumbo vira zebra** (e não chumbo), porque a tabela `regen_proximo` vai preto→zebra e só compara o `rank`. É um detalhe pequeno de fidelidade.

## 4. Torres

As tabelas mostram cada upgrade isolado no próprio caminho (ex.: a linha 1-3 é a torre 3-0-0). A coluna "Mecânica" é o efeito exato aplicado pelo código.

### Macaco Dardo — $200 · alcance 128 · primaria

**Base:** projétil cad 0.95s dano 1 pierce 2 [afiado]. Dano/s 1 alvo **1.1**, potencial em área **2.1**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Tiros Afiados | 140 (340) | +1 pierce | 1.1 | 3.2 | 1.1 |
| 1-2 | Tiros Super Afiados | 220 (560) | +2 pierce | 1.1 | 5.3 | 1.1 |
| 1-3 | Espinhopulta | 300 (860) | +18 pierce; recarga ×1.3 (-23% ataques/s); +6 raio do projétil; vel. projétil ×0.7 | 0.8 | 19 | 0.8 |
| 1-4 | Juggernaut | 1800 (2660) | +1 dano; +50 pierce; dano vira [normal]; +4 dano em cerâmica; +6 raio do projétil | 1.6 | 118 | 1.6 |
| 1-5 | Ultra-Juggernaut | 15000 (17660) | +3 dano; +100 pierce; +8 dano em cerâmica; +4 raio do projétil; 6 fragmentos (dano 2, pierce 30) | 4.0 | 700 | 4.0 |
| 2-1 | Tiros Rápidos | 100 (300) | recarga ×0.85 (+18% ataques/s) | 1.2 | 2.5 | 1.2 |
| 2-2 | Tiros Muito Rápidos | 190 (490) | recarga ×0.78 (+28% ataques/s) | 1.6 | 3.2 | 1.6 |
| 2-3 | Tiro Triplo | 400 (890) | +2 projéteis; leque 30° | 1.6 | 9.5 | 1.6 |
| 2-4 | Fã-Clube Super Macaco | 8000 (8890) | **Hab. Fã-Clube Super Macaco** (50s): torres a até alcance+60px do tipo dardo atacam 12.5× mais rápido por 15s | 1.6 | 9.5 | 1.6 |
| 2-5 | Fã-Clube Macaco Plasma | 45000 (53890) | +2 dano; +3 pierce; dano vira [normal]; **Hab. Fã-Clube Macaco Plasma** (45s): torres a até alcance+60px do tipo dardo atacam 25× mais rápido por 15s | 4.8 | 71 | 4.8 |
| 3-1 | Dardos de Longo Alcance | 90 (290) | +32 alcance; +60 distância do projétil | 1.1 | 2.1 | 1.1 |
| 3-2 | Visão Aprimorada | 200 (490) | +16 alcance; detecta camo | 1.1 | 2.1 | 1.1 |
| 3-3 | Besta | 575 (1065) | +16 alcance; +2 dano; +1 pierce | 3.2 | 9.5 | 3.2 |
| 3-4 | Atirador Afiado | 2000 (3065) | recarga ×0.6 (+67% ataques/s); +3 dano | 11 | 32 | 11 |
| 3-5 | Mestre da Besta | 25000 (28065) | recarga ×0.3 (+233% ataques/s); +5 dano; +4 pierce; +2 ricochete(s); +40 alcance | 64 | 450 | 64 |

**Análise.** Torre de entrada barata e eficiente até o meio do jogo. O caminho 1 troca cadência por pierce (a Espinhopulta ataca 23% mais devagar, mas com 23 de pierce) e vira dano em área de verdade no Juggernaut (dano [normal], acerta chumbo). No 1-5, os 6 fragmentos só nascem quando a bola **acaba a distância** (`fim_projetil`), não quando acerta, então perto de curvas fechadas eles podem sair fora da trilha. No caminho 2, o 2-4 e o 2-5 **não transformam** os dardos em Super Macacos: a habilidade só dá turbo (recarga ×0,08 / ×0,04, com piso de 0,02 s) aos Macacos Dardo no alcance+60 px, e o custo de $8.000 não compra nada de passivo. No 2-5, o +2 dano/[normal] é passivo. O caminho 3 é o melhor alvo único da torre (Mestre da Besta: 64 dano/s por $28 mil).

### Macaco Bumerangue — $325 · alcance 172 · primaria

**Base:** projétil cad 1.2s dano 1 pierce 4 [afiado] volta. Dano/s 1 alvo **0.8**, potencial em área **3.3**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Bumerangues Melhorados | 200 (525) | +4 pierce | 0.8 | 6.7 | 0.8 |
| 1-2 | Glaives | 280 (805) | +6 pierce | 0.8 | 12 | 0.8 |
| 1-3 | Ricochete de Glaive | 1300 (2105) | +30 pierce; +6 ricochete(s) | 0.8 | 37 | 0.8 |
| 1-4 | M.O.A.R. Glaives | 3000 (5105) | +40 pierce; +1 dano; +3 dano em MOAB | 1.7 | 140 | 4.2 |
| 1-5 | Senhor das Glaives | 32500 (37605) | +5 dano; +10 dano em MOAB; **novo ataque**: aura cad 0.1s dano 4 pierce 100 [normal] raio 90 | 46 | 4490 | 57 |
| 2-1 | Arremesso Rápido | 175 (500) | recarga ×0.75 (+33% ataques/s) | 1.1 | 4.4 | 1.1 |
| 2-2 | Bumerangues Velozes | 250 (750) | vel. projétil ×1.3; recarga ×0.9 (+11% ataques/s) | 1.2 | 4.9 | 1.2 |
| 2-3 | Bumerangue Biônico | 1600 (2350) | recarga ×0.4 (+150% ataques/s); +2 dano em MOAB | 3.1 | 12 | 9.3 |
| 2-4 | Turbo Carga | 4000 (6350) | recarga ×0.8 (+25% ataques/s); **Hab. Turbo Carga** (45s): a própria torre ataca 4× mais rápido por 10s | 3.9 | 15 | 12 |
| 2-5 | Carga Permanente | 35000 (41350) | recarga ×0.35 (+186% ataques/s); +5 dano | 66 | 265 | 88 |
| 3-1 | Bumerangues de Longo Alcance | 100 (425) | +24 alcance; +40 distância do projétil | 0.8 | 3.3 | 0.8 |
| 3-2 | Bumerangues Incandescentes | 300 (725) | +1 dano; dano vira [normal] | 1.7 | 6.7 | 1.7 |
| 3-3 | Bumerangue Kylie | 1300 (2025) | +20 pierce; +220 distância do projétil | 1.7 | 40 | 1.7 |
| 3-4 | Prensa de M.O.A.B. | 2200 (4225) | +4 dano em MOAB; +40px de empurrão | 1.7 | 40 | 5.0 |
| 3-5 | Dominação M.O.A.B. | 50000 (54225) | +20 dano; +30 dano em MOAB; +80px de empurrão; +30 pierce | 18 | 990 | 47 |

**Análise.** O projétil faz uma curva (2,2 rad/s) e volta quando percorre metade da distância. Ao voltar pode acertar de novo bloons que ainda não foram atingidos (o conjunto `atingidos` é por projétil). O caminho 1 escala pierce e ricochete. No 1-5, a aura de glaives orbitando (dano 4, pierce 100, a cada 0,1 s, raio 90) é quase todo o dano da torre (4.490/s potencial). O caminho 2 é pura cadência: a Carga Permanente chega a 0,09 s de recarga com dano 6, e o Turbo Carga do 2-4 continua disponível como habilidade no 2-5. O caminho 3 dá empurrão em MOAB (efeito ×0,35 em dirigíveis). O Kylie ("segue a trilha") é só +20 pierce e +220 de distância; o bumerangue não segue a trilha no código.

### Canhão Bomba — $525 · alcance 160 · primaria

**Base:** projétil cad 1.5s dano 1 pierce 1 [afiado] | área r45 d1 p14 [explosão]. Dano/s 1 alvo **1.3**, potencial em área **10**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Bombas Maiores | 350 (875) | +12 raio da explosão; +10 pierce da explosão | 1.3 | 17 | 1.3 |
| 1-2 | Bombas Pesadas | 650 (1525) | +1 dano da explosão; +8 pierce da explosão | 2.0 | 43 | 2.0 |
| 1-3 | Bombas Muito Grandes | 1100 (2625) | +20 raio da explosão; +20 pierce da explosão; +1 dano da explosão | 2.7 | 105 | 2.7 |
| 1-4 | Impacto Bloon | 3600 (6225) | atordoa 1s; +1 dano da explosão | 3.3 | 139 | 3.3 |
| 1-5 | Esmaga Bloon | 55000 (61225) | +8 dano da explosão; +30 raio da explosão; +200 pierce da explosão; atordoa 2s; +10 dano em cerâmica; +10 dano em MOAB | 8.7 | 2017 | 15 |
| 2-1 | Recarga Rápida | 250 (775) | recarga ×0.75 (+33% ataques/s) | 1.8 | 13 | 1.8 |
| 2-2 | Lança-Mísseis | 400 (1175) | recarga ×0.85 (+18% ataques/s); vel. projétil ×1.6; +16 alcance | 2.1 | 16 | 2.1 |
| 2-3 | Destruidor de M.O.A.B. | 1100 (2275) | +15 dano em MOAB | 2.1 | 16 | 18 |
| 2-4 | Assassino de M.O.A.B. | 3200 (5475) | +15 dano em MOAB; **Hab. Míssil Assassino** (30s): 750 de dano em 1 alvo(s) mais forte(s) (só MOAB) | 2.1 | 16 | 33 |
| 2-5 | Eliminador de M.O.A.B. | 25000 (30475) | +100 dano em MOAB; **Hab. Míssil Eliminador** (10s): 4500 de dano em 1 alvo(s) mais forte(s) (só MOAB) | 2.1 | 16 | 138 |
| 3-1 | Alcance Extra | 200 (725) | +28 alcance; +40 distância do projétil | 1.3 | 10 | 1.3 |
| 3-2 | Bombas de Fragmentação | 300 (1025) | 8 fragmentos (dano 1, pierce 1) | 1.3 | 10 | 1.3 |
| 3-3 | Bombas de Cacho | 800 (1825) | 8 fragmentos (dano 1, pierce 1, explodem r30) | 1.3 | 10 | 1.3 |
| 3-4 | Cacho Recursivo | 2800 (4625) | +1 dano da explosão; 10 fragmentos (dano 1, pierce 1, explodem r34) | 2.0 | 19 | 2.0 |
| 3-5 | Blitz de Bombas | 23000 (27625) | +2 dano da explosão; **Hab. Blitz de Bombas** (60s): 1000 de dano [normal] em todo bloon na tela | 3.3 | 38 | 3.3 |

**Análise.** O impacto direto é afiado (dano 1, pierce 1) e o dano real vem da explosão [explosão]. **Pretos, zebras e DDT são imunes** a ela até o caminho 1 virar... nunca: nenhum upgrade muda o `sdtype`. A torre só acerta pretos e zebras com o buff da Vila (Agência de Inteligência). No caminho 2, o bônus MOAB (+15/+30/+130) entra **em cada bloon atingido pela explosão**, e o Eliminador ainda ganha um míssil de 4.500 a cada 10 s: é o melhor anti-MOAB por custo antes do fim do jogo. No caminho 3, os fragmentos só saem quando a bomba acerta (colisão com explosão) e são afiados (não pegam chumbo). O 3-5 Blitz tira 1.000 de todo bloon na tela a cada 60 s.

### Atirador de Tachinhas — $280 · alcance 92 · primaria

**Base:** radial cad 1.4s dano 1 pierce 1 n 8 [afiado]. Dano/s 1 alvo **0.7**, potencial em área **5.7**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Disparo Rápido | 150 (430) | recarga ×0.75 (+33% ataques/s) | 1.0 | 7.6 | 1.0 |
| 1-2 | Disparo Mais Rápido | 300 (730) | recarga ×0.66 (+52% ataques/s) | 1.4 | 12 | 1.4 |
| 1-3 | Tiros Quentes | 600 (1330) | dano vira [normal]; +1 dano | 2.9 | 23 | 2.9 |
| 1-4 | Anel de Fogo | 3500 (4830) | **substitui o ataque [0]** por: aura cad 0.3s dano 1 pierce 60 [energia] raio 110 | 3.3 | 200 | 3.3 |
| 1-5 | Anel Infernal | 45500 (50330) | +3 dano; +80 pierce; **novo ataque**: projétil cad 4s dano 700 pierce 1 [normal] | área r60 d30 p20 [explosão] global alvo forte | 196 | 2192 | 196 |
| 2-1 | Tachinhas de Longo Alcance | 100 (380) | +16 alcance; +30 distância do projétil | 0.7 | 5.7 | 0.7 |
| 2-2 | Tachinhas de Super Alcance | 225 (605) | +16 alcance; +30 distância do projétil | 0.7 | 5.7 | 0.7 |
| 2-3 | Atirador de Lâminas | 550 (1155) | +3 pierce; +1 dano | 1.4 | 46 | 1.4 |
| 2-4 | Turbilhão de Lâminas | 2700 (3855) | **Hab. Turbilhão** (20s): a própria torre ataca 20× mais rápido por 3s | 1.4 | 46 | 1.4 |
| 2-5 | Super Turbilhão | 15000 (18855) | +10 pierce; +2 dano; **Hab. Super Turbilhão** (20s): a própria torre ataca 25× mais rápido por 9s | 2.9 | 320 | 2.9 |
| 3-1 | Mais Tachinhas | 100 (380) | +2 projéteis | 0.7 | 7.1 | 0.7 |
| 3-2 | Ainda Mais Tachinhas | 300 (680) | +2 projéteis | 0.7 | 8.6 | 0.7 |
| 3-3 | Pulverizador de Tachinhas | 600 (1280) | +4 projéteis; recarga ×0.6 (+67% ataques/s) | 1.2 | 19 | 1.2 |
| 3-4 | Sobrecarga | 3200 (4480) | recarga ×0.33 (+203% ataques/s) | 3.6 | 58 | 3.6 |
| 3-5 | Zona das Tachinhas | 24000 (28480) | +20 projéteis; +4 pierce; recarga ×0.5 (+100% ataques/s); +1 dano; +30 alcance | 14 | 2597 | 14 |

**Análise.** O ataque radial só dispara se houver um alvo dentro do alcance (92 px, o menor do jogo). O 1-4 **substitui** o ataque por uma aura de fogo de raio 110 [energia]: roxos ficam imunes, e todo bônus comprado nos caminhos 2/3 antes (n, dist) é perdido ou vira efeito inútil (o `n` da aura não é usado). O 1-5 adiciona um meteoro **global** de 700 de dano no alvo mais forte a cada 4 s. O caminho 2 tem habilidades de turbo curtas. O 3-5 (36 tachinhas com pierce 5 a cada 0,14 s) tem potencial enorme, mas só no alcance curto.

### Macaco de Gelo — $500 · alcance 80 · primaria

**Base:** aura cad 2.4s dano 1 pierce 40 [gelo] congela 1.5s. Dano/s 1 alvo **0.4**, potencial em área **17**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Permafrost | 100 (600) | lentidão ×0.5 por 2.5s | 0.4 | 17 | 0.4 |
| 1-2 | Estalo Frio | 350 (950) | detecta camo; dano vira [normal] | 0.4 | 17 | 0.4 |
| 1-3 | Estilhaços de Gelo | 1500 (2450) | 3 fragmentos (dano 1, pierce 2) | 0.4 | 17 | 0.4 |
| 1-4 | Fragilização | 2200 (4650) | +1 fragilização (dano extra por golpe que o bloon passa a sofrer) | 0.4 | 17 | 0.4 |
| 1-5 | Super Frágil | 28000 (32650) | +4 fragilização (dano extra por golpe que o bloon passa a sofrer); +2 dano; +4 dano em MOAB | 1.2 | 50 | 2.9 |
| 2-1 | Congelamento Melhor | 225 (725) | recarga ×0.8 (+25% ataques/s); congela 2s | 0.5 | 21 | 0.5 |
| 2-2 | Congelamento Profundo | 350 (1075) | congela 2.5s; +1 dano | 1.0 | 42 | 1.0 |
| 2-3 | Vento Ártico | 2900 (3975) | +30 alcance; **novo ataque**: aura cad 0.2s pierce 999 lento ×0.4 0.3s (MOAB lento) | 1.0 | 42 | 1.0 |
| 2-4 | Nevasca | 3000 (6975) | **Hab. Nevasca** (30s): congela todos por 3s (não pega MOAB/branco/zebra) | 1.0 | 42 | 1.0 |
| 2-5 | Zero Absoluto | 26000 (32975) | **Hab. Zero Absoluto** (20s): congela todos por 10s (inclui MOAB, metade do tempo) | 1.0 | 42 | 1.0 |
| 3-1 | Raio Maior | 175 (675) | +16 alcance | 0.4 | 17 | 0.4 |
| 3-2 | Recongelar | 225 (900) | +20 pierce | 0.4 | 25 | 0.4 |
| 3-3 | Canhão Criogênico | 2000 (2900) | +60 alcance; **substitui o ataque [0]** por: projétil cad 1.2s dano 1 pierce 1 [gelo] | área r34 d1 p20 [gelo] congela 1.5s | 1.7 | 18 | 1.7 |
| 3-4 | Pingentes | 2000 (4900) | +1 dano da explosão; +2 dano em MOAB; +10 pierce da explosão | 2.5 | 51 | 4.2 |
| 3-5 | Empalar com Pingentes | 30000 (34900) | +30 dano da explosão; +30 dano em MOAB; congela 5s; congela MOAB | 28 | 801 | 54 |

**Análise.** A aura congela e causa 1 de dano [gelo]. **Brancos e zebras não congelam e são imunes a gelo**; chumbo é imune a gelo até o 1-2 (Estalo Frio muda o dano para [normal]). Bloon congelado fica **imune a afiado** e o golpe ainda gasta pierce de quem atirou: um Gelo ao lado de dardos/tachinhas atrapalha essas torres. O 1-3 (estilhaços) só dispara quando o bloon **morre** enquanto está congelado. A fragilização (1-4/1-5) soma dano fixo a **todo golpe** que o bloon recebe de qualquer torre, e rende muito com torres de cadência alta (Super, Dartling). O 2-3 adiciona uma aura de lentidão (×0,4, também em MOAB) com dano 0 e tipo afiado padrão. **Bug:** essa aura é bloqueada por chumbo, DDT e bloons congelados, então não desacelera esses alvos. O 3-3 **substitui** o ataque: com cruzamento 2-0-5 / 0-2-5, o Permafrost, o dano [normal] e o congelamento extra dos outros caminhos somem (só a detecção de camo fica).

### Atirador de Cola — $225 · alcance 184 · primaria

**Base:** projétil cad 1s pierce 1 cola ×0.5 11s. Dano/s 1 alvo **0.0**, potencial em área **0.0**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Cola Encharcada | 200 (425) | *(sem efeito no código)* | 0.0 | 0.0 | 0.0 |
| 1-2 | Cola Corrosiva | 300 (725) | cola ×0.5 por 11s, corrói 0.5 dano/s | 0.0 | 0.0 | 0.0 |
| 1-3 | Dissolvedor de Bloons | 2500 (3225) | cola ×0.5 por 11s, corrói 2 dano/s | 0.0 | 0.0 | 0.0 |
| 1-4 | Liquefator de Bloons | 5000 (8225) | cola ×0.45 por 11s, corrói 10 dano/s | 0.0 | 0.0 | 0.0 |
| 1-5 | Solucionador de Bloons | 22000 (30225) | cola ×0.4 por 11s, corrói 50 dano/s; +40 raio da explosão; +6 pierce da explosão | 0.0 | 0.0 | 0.0 |
| 2-1 | Globos Maiores | 100 (325) | +1 pierce | 0.0 | 0.0 | 0.0 |
| 2-2 | Respingo de Cola | 1600 (1925) | +40 raio da explosão; +6 pierce da explosão | 0.0 | 0.0 | 0.0 |
| 2-3 | Mangueira de Cola | 3250 (5175) | recarga ×0.3 (+233% ataques/s) | 0.0 | 0.0 | 0.0 |
| 2-4 | Ataque de Cola | 3500 (8675) | **Hab. Ataque de Cola** (40s): todos os bloons a ×0.5 de velocidade por 11s | 0.0 | 0.0 | 0.0 |
| 2-5 | Tempestade de Cola | 15000 (23675) | **Hab. Tempestade de Cola** (30s): todos os bloons a ×0.3 de velocidade por 15s + 5 de dano em todos | 0.0 | 0.0 | 0.0 |
| 3-1 | Cola Mais Grudenta | 120 (345) | cola ×0.5 por 22s | 0.0 | 0.0 | 0.0 |
| 3-2 | Cola Mais Forte | 400 (745) | cola ×0.35 por 22s | 0.0 | 0.0 | 0.0 |
| 3-3 | Cola de M.O.A.B. | 3400 (4145) | cola pega MOAB | 0.0 | 0.0 | 0.0 |
| 3-4 | Cola Implacável | 3000 (7145) | +3 pierce; +30 raio da explosão; +4 pierce da explosão | 0.0 | 0.0 | 0.0 |
| 3-5 | Super Cola | 35000 (42145) | atordoa 2s; atordoa MOAB; cola ×0.2 por 30s, corrói 5 dano/s | 0.0 | 0.0 | 0.0 |

**Análise.** O dano base é 0 e só aplica cola (lentidão). Como o tipo padrão é afiado, **chumbo e DDT são imunes à cola**, e bloons congelados também. O 1-1 Cola Encharcada **não tem efeito** no código. O 1-2 a 1-5 dão corrosão (DoT de 0,5→50 dano/s enquanto a cola dura 11 s), e esse dano não gera o ouro extra de outras torres. **Bug de cruzamento:** o caminho 3 redefine a `cola` inteira, então 5-0-1 e 5-0-2 **perdem toda a corrosão do caminho 1** (0-0-5 também sobrescreve para 5 dano/s). O 1-5 adiciona área de cola. O 3-3 faz a cola pegar MOAB, e o 3-5 atordoa MOAB (efeito ×0,4 em dirigíveis).

### Macaco Atirador — $350 · alcance global · militar

**Base:** hitscan cad 1.59s dano 2 pierce 1 [afiado] global. Dano/s 1 alvo **1.3**, potencial em área **1.3**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Jaqueta Metálica | 350 (700) | dano vira [normal]; +2 dano | 2.5 | 2.5 | 2.5 |
| 1-2 | Calibre Grosso | 1300 (2000) | +3 dano | 4.4 | 4.4 | 4.4 |
| 1-3 | Precisão Mortal | 3000 (5000) | +11 dano; +15 dano em cerâmica | 11 | 11 | 11 |
| 1-4 | Mutilar M.O.A.B. | 5000 (10000) | atordoa 3s; atordoa MOAB; +12 dano | 19 | 19 | 19 |
| 1-5 | Aleijar M.O.A.B. | 34000 (44000) | atordoa 7s; +5 fragilização (dano extra por golpe que o bloon passa a sofrer); +20 dano | 31 | 31 | 31 |
| 2-1 | Óculos de Visão Noturna | 300 (650) | detecta camo | 1.3 | 1.3 | 1.3 |
| 2-2 | Tiro de Estilhaços | 450 (1100) | 5 fragmentos (dano 1, pierce 1) | 1.3 | 1.3 | 1.3 |
| 2-3 | Bala Ricochete | 3200 (4300) | +3 ricochete(s) | 1.3 | 5.0 | 1.3 |
| 2-4 | Lançamento de Suprimentos | 7200 (11500) | **Hab. Suprimentos** (60s): +$1000 | 1.3 | 5.0 | 1.3 |
| 2-5 | Atirador de Elite | 13000 (24500) | recarga ×0.5 (+100% ataques/s); **Hab. Suprimentos de Elite** (50s): +$2000 | 2.5 | 10 | 2.5 |
| 3-1 | Disparo Rápido | 400 (750) | recarga ×0.7 (+43% ataques/s) | 1.8 | 1.8 | 1.8 |
| 3-2 | Disparo Mais Rápido | 400 (1150) | recarga ×0.7 (+43% ataques/s) | 2.6 | 2.6 | 2.6 |
| 3-3 | Semiautomático | 3500 (4650) | recarga ×0.33 (+203% ataques/s) | 7.8 | 7.8 | 7.8 |
| 3-4 | Rifle Automático | 4750 (9400) | recarga ×0.5 (+100% ataques/s); +2 dano; +3 dano em MOAB | 31 | 31 | 54 |
| 3-5 | Defensor de Elite | 14000 (23400) | recarga ×0.5 (+100% ataques/s); +2 dano | 93 | 93 | 140 |

**Análise.** É hitscan global: acerta instantaneamente o alvo pelo modo de mira, sem projétil. Afiado na base, então não pega chumbo até o 1-1. O caminho 1 é dano bruto por tiro (50 no 1-5, mais fragiliza +5 e atordoamento de 7 s em MOAB com ×0,4). O caminho 2 vira economia: os suprimentos dão $1.000 a cada 60 s no 2-4 e as duas habilidades somam no 2-5. O caminho 3 é cadência (6 dano a cada 0,064 s no 3-5 = 93/s em alvo único). Ricochete e fragmentos do caminho 2 continuam afiados.

### Submarino Macaco — $325 · alcance 168 · militar · só água

**Base:** projétil cad 0.75s dano 1 pierce 2 [afiado] teleguiado. Dano/s 1 alvo **1.3**, potencial em área **2.7**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Alcance Maior | 130 (455) | +40 alcance | 1.3 | 2.7 | 1.3 |
| 1-2 | Inteligência Avançada | 500 (955) | alcance global | 1.3 | 2.7 | 1.3 |
| 1-3 | Submergir e Apoiar | 500 (1455) | detecta camo; **novo ataque**: aura cad 0.5s pierce 999 tira camo | 1.3 | 2.7 | 1.3 |
| 1-4 | Reator de Bloontônio | 2500 (3955) | **novo ataque**: aura cad 0.3s dano 1 pierce 100 [normal] | 4.7 | 336 | 4.7 |
| 1-5 | Energizador | 32000 (35955) | +3 dano; +300 pierce; recarga ×0.6 (+67% ataques/s) (no ataque [1]) | 15 | 13326 | 15 |
| 2-1 | Dardos Farpados | 450 (775) | +3 pierce | 1.3 | 6.7 | 1.3 |
| 2-2 | Dardos Aquecidos | 300 (1075) | dano vira [normal]; +1 dano | 2.7 | 13 | 2.7 |
| 2-3 | Míssil Balístico | 1300 (2375) | **novo ataque**: projétil cad 1.5s dano 3 pierce 1 [afiado] | área r40 d1 p10 [explosão] +5 MOAB teleguiado global | 5.3 | 22 | 8.7 |
| 2-4 | Capacidade de Primeiro Ataque | 13000 (15375) | **Hab. Primeiro Ataque** (60s): 10000 de dano em 1 alvo(s) mais forte(s) + explosão r120 d700 | 5.3 | 22 | 8.7 |
| 2-5 | Ataque Preventivo | 32000 (47375) | **novo ataque**: projétil cad 5s dano 1000 pierce 1 [normal] teleguiado global só MOAB alvo forte | 205 | 222 | 209 |
| 3-1 | Canhões Gêmeos | 450 (775) | recarga ×0.5 (+100% ataques/s) | 2.7 | 5.3 | 2.7 |
| 3-2 | Dardos de Explosão Aérea | 1000 (1775) | 3 fragmentos (dano 1, pierce 1) | 2.7 | 5.3 | 2.7 |
| 3-3 | Canhões Triplos | 1100 (2875) | recarga ×0.66 (+52% ataques/s) | 4.0 | 8.1 | 4.0 |
| 3-4 | Dardos Perfurantes | 3000 (5875) | +2 dano; +3 dano em MOAB; +2 dano em cerâmica | 12 | 24 | 24 |
| 3-5 | Comandante Submarino | 25000 (30875) | +8 dano; +6 pierce; recarga ×0.5 (+100% ataques/s) | 89 | 711 | 113 |

**Análise.** Só pode ficar na água; o Prado não tem água. O 1-2 dá mira global, mas o projétil **continua com distância 260**: alvos longe somem antes de chegar, então o upgrade quase não serve para fora do raio. O 1-3 cria uma aura que tira camo (dano 0, tipo afiado). **Bug:** ela é bloqueada pela imunidade do **DDT** (camo nativo, imune a afiado), então não revela DDT. **Bug no 1-5:** o Energizador aplica `a:1`, que é a aura de revelar camo, e não o reator (índice 2). Na prática a aura de revelar passa a causar 3 de dano afiado com pierce 1.299 a cada 0,3 s, e o reator fica igual. O caminho 2 tem o Ataque Preventivo: um míssil de 1.000 só em MOAB a cada 5 s, o melhor anti-MOAB da torre.

### Macaco Bucaneiro — $500 · alcance 240 · militar · só água

**Base:** projétil cad 1s dano 1 pierce 4 n 2 leque 360° [afiado]. Dano/s 1 alvo **1.0**, potencial em área **8.0**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Disparo Rápido | 275 (775) | recarga ×0.75 (+33% ataques/s) | 1.3 | 11 | 1.3 |
| 1-2 | Tiro Duplo | 450 (1225) | +2 projéteis | 1.3 | 21 | 1.3 |
| 1-3 | Destróier | 2950 (4175) | recarga ×0.2 (+400% ataques/s) | 6.7 | 107 | 6.7 |
| 1-4 | Porta-Aviões | 6000 (10175) | **novo ataque**: radial cad 0.6s dano 2 pierce 5 n 4 [afiado] global | 10 | 173 | 10 |
| 1-5 | Nau Capitânia | 40000 (50175) | +3 dano; +5 pierce; aura de buff: recarga ×0.85 +10% alcance (em todos os ataques) | 35 | 1293 | 35 |
| 2-1 | Tiro de Uva | 550 (1050) | **novo ataque**: projétil cad 1s dano 1 pierce 1 n 5 leque 40° [afiado] | 2.0 | 13 | 2.0 |
| 2-2 | Tiro Quente | 500 (1550) | dano vira [normal]; +1 dano (no ataque [1]) | 3.0 | 18 | 3.0 |
| 2-3 | Navio Canhão | 900 (2450) | **novo ataque**: projétil cad 1.3s dano 2 pierce 1 [afiado] | área r40 d2 p16 [explosão] | 6.1 | 44 | 6.1 |
| 2-4 | Macacos Piratas | 4500 (6950) | +4 dano em MOAB; **Hab. Arpão** (60s): 4000 de dano em 1 alvo(s) mais forte(s) (só MOAB) | 6.1 | 44 | 10 |
| 2-5 | Senhor Pirata | 21000 (27950) | +10 dano em MOAB; **Hab. Arpões do Senhor Pirata** (60s): 20000 de dano em 3 alvo(s) mais forte(s) (só MOAB) | 6.1 | 44 | 20 |
| 3-1 | Longo Alcance | 180 (680) | +40 alcance | 1.0 | 8.0 | 1.0 |
| 3-2 | Ninho do Corvo | 400 (1080) | detecta camo | 1.0 | 8.0 | 1.0 |
| 3-3 | Navio Mercante | 2300 (3380) | **novo ataque**: renda $200/rodada | 1.0 | 8.0 · $200/rod. | 1.0 |
| 3-4 | Comércio Favorecido | 5500 (8880) | +$300/rodada (no ataque [1]) | 1.0 | 8.0 · $500/rod. | 1.0 |
| 3-5 | Império Comercial | 23000 (31880) | +$900/rodada; aura de buff: +1 dano (no ataque [1]) | 1.0 | 8.0 · $1400/rod. | 1.0 |

**Análise.** Dispara 2 projéteis opostos (360°) mirando o alvo. O Porta-Aviões (1-4) é um radial "global": atira se existe **qualquer** bloon no mapa, mas os aviões saem da posição do navio com distância 300, então não cobrem o mapa. **Bugs de índice (`a:1`) com cruzamento:** em 4-1-0/4-2-0/5-2-0 o Tiro Quente (2-2) vai para os aviões, e as uvas continuam afiadas. Em 0-1-4/0-2-5 o Comércio Favorecido e o Império Comercial vão para as uvas, e a renda fica em $200 em vez de $500/$1.400. A Nau Capitânia (1-5) coloca buff em **todos** os ataques, e como `recalcular_buffs` soma o buff por ataque, as torres vizinhas recebem **recarga ×0,85 elevado ao número de ataques** (até ×0,61 no 5-2-0), além de +10% de alcance por ataque.

### Macaco Ás — $800 · alcance 200 · militar · voa em órbita

**Base:** radial cad 1.68s dano 1 pierce 5 n 8 [afiado]. Dano/s 1 alvo **0.6**, potencial em área **24**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Tiro Rápido | 650 (1450) | recarga ×0.7 (+43% ataques/s) | 0.9 | 34 | 0.9 |
| 1-2 | Muito Mais Dardos | 650 (2100) | +4 projéteis | 0.9 | 51 | 0.9 |
| 1-3 | Avião de Caça | 1000 (3100) | **novo ataque**: projétil cad 1s dano 3 pierce 1 [afiado] | área r25 d1 p5 [explosão] +3 MOAB teleguiado global | 4.9 | 59 | 7.9 |
| 1-4 | Operação: Tempestade de Dardos | 3000 (6100) | recarga ×0.4 (+150% ataques/s); +4 projéteis | 6.1 | 178 | 9.1 |
| 1-5 | Retalhador Celeste | 24000 (30100) | +8 projéteis; +5 pierce; recarga ×0.5 (+100% ataques/s); +2 dano | 17 | 3069 | 20 |
| 2-1 | Abacaxi Explosivo | 200 (1000) | **novo ataque**: queda cad 3s dano 1 [afiado] | área r50 d1 p20 [explosão] pavio 1.5s | 0.9 | 30 | 0.9 |
| 2-2 | Avião Espião | 350 (1350) | detecta camo | 0.9 | 30 | 0.9 |
| 2-3 | Ás Bombardeiro | 900 (2250) | **novo ataque**: queda cad 1.5s dano 1 [afiado] | área r60 d2 p30 [explosão] pavio 0.3s | 2.3 | 70 | 2.3 |
| 2-4 | Marco Zero | 18000 (20250) | **Hab. Marco Zero** (45s): 700 de dano [normal] em todo bloon na tela | 2.3 | 70 | 2.3 |
| 2-5 | Tsar Bomba | 30000 (50250) | **Hab. Tsar Bomba** (60s): 3000 de dano [normal] em todo bloon na tela + atordoa 8s | 2.3 | 70 | 2.3 |
| 3-1 | Dardos Mais Afiados | 500 (1300) | +3 pierce | 0.6 | 38 | 0.6 |
| 3-2 | Rota Centralizada | 300 (1600) | *(sem efeito no código)* | 0.6 | 38 | 0.6 |
| 3-3 | Mira Infalível | 2200 (3800) | teleguiado | 0.6 | 38 | 0.6 |
| 3-4 | Espectro | 24000 (27800) | **novo ataque**: projétil cad 0.05s dano 2 pierce 3 [normal] teleguiado global | 41 | 158 | 41 |
| 3-5 | Fortaleza Voadora | 90000 (117800) | +3 dano; +8 projéteis; recarga ×0.5 (+100% ataques/s) (em todos os ataques) | 205 | 6010 | 205 |

**Análise.** Voa em órbita elíptica (raio 110×82) em volta do ponto onde foi posto, e o radial dispara sempre que existe bloon no mapa. O abacaxi (2-1) cai **onde o avião está**, não na trilha; só o Ás Bombardeiro (2-3) mira a trilha. O 3-2 Rota Centralizada **não tem efeito**. **Bug no 3-3:** a Mira Infalível dá `busca` ao radial, mas o radial chama `disparar` sem alvo, então os dardos **nunca** ficam teleguiados. O 3-4 Espectro (dardos globais teleguiados a cada 0,05 s) é o salto real. A Fortaleza Voadora aplica +8 projéteis também no Espectro (9 por disparo a cada 0,025 s): é a segunda maior DPS da loja.

### Piloto de Helicóptero — $1600 · alcance 170 · militar · helicóptero móvel

**Base:** projétil cad 0.57s dano 1 pierce 3 n 2 leque 10° [afiado]. Dano/s 1 alvo **3.5**, potencial em área **11**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Dardos Quádruplos | 800 (2400) | +2 projéteis; leque 20° | 1.8 | 21 | 1.8 |
| 1-2 | Perseguição | 500 (2900) | persegue bloons no mapa todo | 1.8 | 21 | 1.8 |
| 1-3 | Hélices Navalha | 1750 (4650) | **novo ataque**: aura cad 0.5s dano 2 pierce 20 [afiado] raio 55 | 5.8 | 101 | 5.8 |
| 1-4 | Apache Dardeiro | 19600 (24250) | recarga ×0.35 (+186% ataques/s); +2 projéteis; **novo ataque**: projétil cad 1s dano 5 pierce 1 [afiado] | área r35 d2 p10 [explosão] +5 MOAB | 16 | 195 | 21 |
| 1-5 | Apache Prime | 45000 (69250) | dano vira [normal]; +4 dano (em todos os ataques) | 48 | 720 | 53 |
| 2-1 | Jatos Maiores | 300 (1900) | +20 alcance | 3.5 | 11 | 3.5 |
| 2-2 | IFR | 600 (2500) | detecta camo | 3.5 | 11 | 3.5 |
| 2-3 | Corrente Descendente | 2000 (4500) | **novo ataque**: aura cad 1.2s pierce 6 raio 80 empurra 90px | 3.5 | 11 | 3.5 |
| 2-4 | Chinook de Apoio | 12000 (16500) | **Hab. Entrega** (60s): +$1000 | 3.5 | 11 | 3.5 |
| 2-5 | Operações Especiais | 35000 (51500) | **Hab. Fuzileiro** (60s): invoca sniper por 20s | 3.5 | 11 | 3.5 |
| 3-1 | Dardos Rápidos | 250 (1850) | vel. projétil ×1.3 | 3.5 | 11 | 3.5 |
| 3-2 | Disparo Rápido | 350 (2200) | recarga ×0.8 (+25% ataques/s) | 4.4 | 13 | 4.4 |
| 3-3 | Empurrão de M.O.A.B. | 3000 (5200) | **novo ataque**: aura cad 0.5s pierce 99 raio 90 lento ×0.5 0.5s (MOAB lento) | 4.4 | 13 | 4.4 |
| 3-4 | Defesa Comanche | 8500 (13700) | +4 projéteis; recarga ×0.7 (+43% ataques/s) | 19 | 56 | 19 |
| 3-5 | Comandante Comanche | 35000 (48700) | +4 dano; +6 projéteis; +4 pierce | 188 | 1316 | 188 |

**Análise.** Segue o bloon mais avançado a até 260 px do ponto de origem (ou do mapa inteiro com Perseguição, 1-2) a 260 px/s. O 1-1 dobra os dardos mas abre o leque para 20°: ganha em grupos, porém contra 1 alvo isolado quase não melhora (a métrica de alvo único cai porque os dardos se espalham). As Hélices Navalha (1-3) e a Corrente Descendente (2-3) usam tipo afiado padrão, então não pegam chumbo e congelados. O empurrão da corrente (dano 0) é bloqueado da mesma forma. O caminho 2 é utilitário: dinheiro no 2-4 e um sniper 4-0-3 temporário no 2-5. O caminho 3 soma n: 12 dardos no 3-5.

### Macaco Morteiro — $750 · alcance global · militar

**Base:** morteiro cad 2s dano 1 [afiado] | área r40 d1 p40 [explosão] global imprecisão 40. Dano/s 1 alvo **0.5**, potencial em área **20**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Explosão Maior | 500 (1250) | +12 raio da explosão; +10 pierce da explosão | 0.5 | 25 | 0.5 |
| 1-2 | Destruidor de Bloons | 500 (1750) | +1 dano da explosão | 1.0 | 50 | 1.0 |
| 1-3 | Choque de Projéteis | 900 (2650) | atordoa 0.5s; +10 raio da explosão | 1.0 | 50 | 1.0 |
| 1-4 | A Grande | 7000 (9650) | +30 raio da explosão; +3 dano da explosão; +60 pierce da explosão | 2.5 | 275 | 2.5 |
| 1-5 | A Maior de Todas | 35000 (44650) | +60 raio da explosão; +20 dano da explosão; +200 pierce da explosão | 12 | 3875 | 12 |
| 2-1 | Recarga Rápida | 300 (1050) | recarga ×0.75 (+33% ataques/s) | 0.7 | 27 | 0.7 |
| 2-2 | Recarga Veloz | 500 (1550) | recarga ×0.75 (+33% ataques/s) | 0.9 | 36 | 0.9 |
| 2-3 | Projéteis Pesados | 900 (2450) | +1 dano da explosão; explosão vira [normal] | 1.8 | 71 | 1.8 |
| 2-4 | Bateria de Artilharia | 5500 (7950) | +2 projéteis | 5.3 | 213 | 5.3 |
| 2-5 | Choque e Pavor | 30000 (37950) | **Hab. Choque e Pavor** (60s): 50 de dano [normal] em todo bloon na tela + atordoa 8s | 5.3 | 213 | 5.3 |
| 3-1 | Precisão Aumentada | 200 (950) | imprecisão ×0.5 | 0.5 | 20 | 0.5 |
| 3-2 | Coisas Queimando | 500 (1450) | fogo 1 dano/s por 3s | 0.5 | 20 | 0.5 |
| 3-3 | Sinalizador | 600 (2050) | remove camo; detecta camo | 0.5 | 20 | 0.5 |
| 3-4 | Projéteis Estilhaçantes | 11000 (13050) | +2 fragilização (dano extra por golpe que o bloon passa a sofrer); +5 dano em cerâmica; +5 dano em MOAB | 0.5 | 20 | 3.0 |
| 3-5 | Bloonflagração | 40000 (53050) | fogo 20 dano/s por 3s; +5 dano da explosão; explosão vira [normal] | 3.0 | 120 | 5.5 |

**Análise.** A descrição diz "local escolhido", mas o código **mira sozinho** no bloon do modo de alvo (`alvo()` global) com imprecisão de ±40 px e pavio de 0,7 s. Em bloons rápidos (rosa a 332 px/s) o tiro erra muito. A explosão é [explosão] até o 2-3 (Projéteis Pesados → [normal]). No 3-3, o Sinalizador tira camo, mas **a explosão é bloqueada pela imunidade do DDT**, então não revela DDT. O fogo (3-2) deixa uma pilha de chamas de pierce 30 no chão. O 1-5 tem a maior explosão do jogo (r152, dano 25, pierce 310).

### Atirador Dartling — $850 · alcance global · militar

**Base:** projétil cad 0.2s dano 1 pierce 1 leque 20° [afiado] global. Dano/s 1 alvo **5.0**, potencial em área **5.0**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Disparo Focado | 250 (1100) | leque 6° | 5.0 | 5.0 | 5.0 |
| 1-2 | Choque Laser | 1200 (2300) | dano vira [energia]; +1 pierce; fogo 1 dano/s por 1s | 5.0 | 10 | 5.0 |
| 1-3 | Canhão Laser | 3000 (5300) | +1 dano; +3 pierce | 10 | 50 | 10 |
| 1-4 | Acelerador de Plasma | 11000 (16300) | **substitui o ataque [0]** por: hitscan cad 0.2s dano 3 pierce 100 [normal] global linha | 15 | 1500 | 15 |
| 1-5 | Raio da Perdição | 90000 (106300) | +27 dano; +200 pierce; recarga ×0.5 (+100% ataques/s); +10 dano em MOAB | 300 | 90000 | 400 |
| 2-1 | Mira Avançada | 300 (1150) | teleguiado | 5.0 | 5.0 | 5.0 |
| 2-2 | Giro de Cano Rápido | 950 (2100) | recarga ×0.7 (+43% ataques/s) | 7.1 | 7.1 | 7.1 |
| 2-3 | Cápsulas de Foguete Hidra | 5000 (7100) | +25 raio da explosão; +1 dano da explosão; +6 pierce da explosão | 14 | 50 | 14 |
| 2-4 | Tempestade de Foguetes | 6000 (13100) | **Hab. Tempestade de Foguetes** (40s): a própria torre ataca 5× mais rápido por 8s | 14 | 50 | 14 |
| 2-5 | M.A.D. | 58000 (71100) | +40 dano em MOAB; +6 dano da explosão; +20 raio da explosão | 57 | 307 | 343 |
| 3-1 | Giro Mais Rápido | 150 (1000) | vel. projétil ×1.2 | 5.0 | 5.0 | 5.0 |
| 3-2 | Dardos Poderosos | 1200 (2200) | +1 dano; +1 pierce; vel. projétil ×1.5 | 10 | 20 | 10 |
| 3-3 | Chumbinho | 3200 (5400) | +5 projéteis; leque 30°; +1 dano | 15 | 180 | 15 |
| 3-4 | Sistema de Negação de Área | 6000 (11400) | +4 projéteis; +2 pierce | 15 | 600 | 15 |
| 3-5 | Zona de Exclusão Bloon | 42000 (53400) | +6 projéteis; +3 dano; +3 pierce; recarga ×0.6 (+67% ataques/s) | 50 | 5600 | 50 |

**Análise.** Global, mas o projétil só vai na direção do alvo com leque aleatório de 20° (o 1-1 reduz para 6°). O caminho 1 vira energia no 1-2 (roxos ficam imunes) e depois [normal]. O Acelerador de Plasma (1-4) **substitui** o ataque por um raio em linha (largura 18 px + metade do raio do bloon) que acerta até 100 bloons na reta. O **1-5 Raio da Perdição** (30 dano, pierce 300, a cada 0,1 s) é a torre mais forte do jogo: ~300 de dano/s por alvo e 90.000/s de potencial. Como tudo na linha é atingido, o alvo único subestima muito o poder real. O caminho 2 tem M.A.D. (+40 MOAB na explosão). O caminho 3 é uma espingarda de n alto.

### Macaco Mago — $375 · alcance 160 · magica

**Base:** projétil cad 1.1s dano 1 pierce 2 [energia]. Dano/s 1 alvo **0.9**, potencial em área **1.8**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Magia Guiada | 150 (525) | teleguiado | 0.9 | 1.8 | 0.9 |
| 1-2 | Explosão Arcana | 600 (1125) | +1 dano | 1.8 | 3.6 | 1.8 |
| 1-3 | Maestria Arcana | 1300 (2425) | recarga ×0.5 (+100% ataques/s); +3 pierce; +20 alcance | 3.6 | 18 | 3.6 |
| 1-4 | Espinho Arcano | 10900 (13325) | +10 dano em MOAB; +2 dano | 7.3 | 36 | 25 |
| 1-5 | Arquimago | 32000 (45325) | +5 dano; +5 pierce; recarga ×0.6 (+67% ataques/s) (em todos os ataques) | 27 | 273 | 58 |
| 2-1 | Bola de Fogo | 300 (675) | **novo ataque**: projétil cad 2.2s dano 1 pierce 1 [energia] | área r30 d1 p12 [energia] | 1.8 | 7.7 | 1.8 |
| 2-2 | Muralha de Fogo | 900 (1575) | **novo ataque**: pilha cad 5.5s dano 1 pierce 15 vida 5s [energia] | 2.0 | 10 | 2.0 |
| 2-3 | Sopro do Dragão | 3000 (4575) | **novo ataque**: projétil cad 0.1s dano 1 pierce 5 n 3 leque 25° [energia] | 12 | 160 | 12 |
| 2-4 | Invocar Fênix | 4000 (8575) | **Hab. Fênix** (45s): invoca fenix por 20s | 12 | 160 | 12 |
| 2-5 | Lorde Fênix | 50000 (58575) | **novo ataque**: projétil cad 0.1s dano 5 pierce 10 n 2 leque 30° [normal] global | 62 | 1160 | 62 |
| 3-1 | Magia Intensa | 300 (675) | +2 pierce; vel. projétil ×1.2 | 0.9 | 3.6 | 0.9 |
| 3-2 | Sentido Macaco | 300 (975) | detecta camo | 0.9 | 3.6 | 0.9 |
| 3-3 | Cintilar | 1700 (2675) | **novo ataque**: aura cad 1s pierce 999 tira camo | 0.9 | 3.6 | 0.9 |
| 3-4 | Necromante | 2000 (4675) | **novo ataque**: pilha cad 2s dano 2 pierce 8 vida 6s [normal] | 1.9 | 12 | 1.9 |
| 3-5 | Príncipe das Trevas | 24000 (28675) | +4 dano; +20 pierce por pilha (em todos os ataques) | 12 | 4098 | 12 |

**Análise.** Tipo [energia] na base: **roxos são imunes** até o Lorde Fênix/Arquimago. O caminho 1 é bom alvo único; o Arquimago aplica em todos os ataques. O caminho 2 empilha 4 ataques novos (bola de fogo, muralha, sopro, fênix), e o Sopro do Dragão (2-3) é o salto de DPS. **Bug no 3-5:** o Príncipe das Trevas aplica +4 dano em **todos** os ataques, inclusive na aura de Cintilar (dano 0, pierce 999, tipo afiado). Ela vira uma aura de 4 dano × 999 alvos a cada 1 s em todo o alcance, e isso explica os 4.098/s. O Necromante é só uma pilha de "zumbis" na trilha; não reanima bloons.

### Super Macaco — $2500 · alcance 200 · magica

**Base:** projétil cad 0.045s dano 1 pierce 1 [afiado]. Dano/s 1 alvo **22**, potencial em área **22**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Rajadas Laser | 2500 (5000) | dano vira [energia]; +1 pierce | 22 | 44 | 22 |
| 1-2 | Rajadas de Plasma | 4500 (9500) | dano vira [normal]; +1 dano; +1 pierce | 44 | 133 | 44 |
| 1-3 | Avatar do Sol | 20000 (29500) | +2 projéteis; leque 20°; +2 dano; +2 pierce | 89 | 1333 | 89 |
| 1-4 | Templo do Sol | 100000 (129500) | +8 dano; +5 pierce; +10 dano em MOAB | 267 | 8000 | 489 |
| 1-5 | Verdadeiro Deus Sol | 500000 (629500) | +20 dano; +10 pierce; +30 dano em MOAB | 711 | 42667 | 1600 |
| 2-1 | Super Alcance | 1000 (3500) | +40 alcance; +60 distância do projétil | 22 | 22 | 22 |
| 2-2 | Alcance Épico | 1400 (4900) | +40 alcance; +60 distância do projétil | 22 | 22 | 22 |
| 2-3 | Robô Macaco | 7000 (11900) | +1 projéteis; leque 12°; +1 dano | 89 | 89 | 89 |
| 2-4 | Terror Tecnológico | 19000 (30900) | **Hab. Aniquilação** (45s): 2000 de dano [normal] em todo bloon na tela | 89 | 89 | 89 |
| 2-5 | O Anti-Bloon | 80000 (110900) | +4 dano; **Hab. Erradicação** (45s): 5000 de dano [normal] em todo bloon na tela | 267 | 267 | 267 |
| 3-1 | Repulsão | 3000 (5500) | +20px de empurrão | 22 | 22 | 22 |
| 3-2 | Ultravisão | 1200 (6700) | detecta camo; +10 alcance | 22 | 22 | 22 |
| 3-3 | Cavaleiro das Trevas | 5500 (12200) | dano vira [normal]; +2 dano em MOAB | 22 | 22 | 67 |
| 3-4 | Campeão das Trevas | 60000 (72200) | +4 dano; +6 dano em MOAB; +3 pierce | 111 | 444 | 289 |
| 3-5 | Lenda da Noite | 240000 (312200) | +10 dano; **Hab. Noite Eterna** (60s): recua todos os bloons 600px (MOAB metade) | 333 | 1333 | 511 |

**Análise.** Recarga de 0,045 s (22 ataques/s) na base: o maior alvo único barato. Tudo que soma dano escala muito (buff +1 de dano da Vila/Alquimista **dobra** a DPS base). O caminho 1 escala sem limite: o Verdadeiro Deus Sol faz 711/s em alvo único e 1.600/s em MOAB, com potencial de 42.667/s. O caminho 2 vira alcance e habilidades globais (Aniquilação 2.000 / Erradicação 5.000 em todos na tela). O caminho 3 fica [normal] no 3-3 e empurra 20 px por golpe (×0,35 em MOAB). A Noite Eterna recua todos os bloons 600 px (MOAB 300).

### Macaco Ninja — $400 · alcance 160 · magica · detecta camo

**Base:** projétil cad 0.7s dano 1 pierce 2 [afiado] teleguiado. Dano/s 1 alvo **1.4**, potencial em área **2.9**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Disciplina Ninja | 300 (700) | +28 alcance; recarga ×0.8 (+25% ataques/s) | 1.8 | 3.6 | 1.8 |
| 1-2 | Shurikens Afiadas | 350 (1050) | +2 pierce | 1.8 | 7.1 | 1.8 |
| 1-3 | Tiro Duplo | 850 (1900) | +1 projéteis; leque 10° | 3.6 | 14 | 3.6 |
| 1-4 | Bloonjitsu | 2750 (4650) | +3 projéteis; leque 30° | 1.8 | 36 | 1.8 |
| 1-5 | Grão-Mestre Ninja | 35000 (39650) | +3 projéteis; +2 dano; recarga ×0.5 (+100% ataques/s) | 11 | 343 | 11 |
| 2-1 | Distração | 350 (750) | +25px de empurrão | 1.4 | 2.9 | 1.4 |
| 2-2 | Contraespionagem | 500 (1250) | remove camo | 1.4 | 2.9 | 1.4 |
| 2-3 | Táticas Shinobi | 900 (2150) | recarga ×0.92 (+9% ataques/s); aura de buff: recarga ×0.92 | 1.6 | 3.1 | 1.6 |
| 2-4 | Sabotagem Bloon | 5200 (7350) | **Hab. Sabotagem** (60s): todos os bloons a ×0.5 de velocidade por 15s | 1.6 | 3.1 | 1.6 |
| 2-5 | Grande Sabotador | 22000 (29350) | **Hab. Grande Sabotagem** (60s): todos os bloons a ×0.5 de velocidade por 20s + 300 de dano em todos | 1.6 | 3.1 | 1.6 |
| 3-1 | Shuriken Teleguiada | 250 (650) | +1 pierce | 1.4 | 4.3 | 1.4 |
| 3-2 | Estrepes | 400 (1050) | **novo ataque**: pilha cad 4s dano 1 pierce 6 vida 10s [afiado] | 1.7 | 5.8 | 1.7 |
| 3-3 | Bomba de Luz | 2750 (3800) | **novo ataque**: projétil cad 4s dano 1 pierce 1 [afiado] | área r55 d1 p50 [explosão] atordoa 1s | 2.2 | 19 | 2.2 |
| 3-4 | Bomba Grudenta | 4500 (8300) | **novo ataque**: projétil cad 3s dano 500 pierce 1 [normal] teleguiado só MOAB alvo forte | 169 | 185 | 169 |
| 3-5 | Mestre Bombardeiro | 40000 (48300) | +4500 dano; recarga ×0.6 (+67% ataques/s) (no ataque [2]) | 2044 | 2069 | 2044 |

**Análise.** Detecta camo na base e tem shuriken teleguiada. As Táticas Shinobi (2-3) dão recarga ×0,92 na própria torre **e** um buff ×0,92 que também se aplica a ela (o buff só pula a própria torre quando o ataque é do tipo BUFF), então a torre ganha ×0,846. Os Shinobis se acumulam entre si sem limite, até o piso de ×0,4. **Bug no 3-5:** o Mestre Bombardeiro aplica `a:2`, que é a **Bomba de Luz** (índice 2), e não a Bomba Grudenta (índice 3). A bomba de luz passa a causar 4.501 de dano afiado (não pega chumbo nem DDT) em qualquer bloon, e a grudenta continua com 500.

### Alquimista — $550 · alcance 180 · magica

**Base:** projétil cad 2s dano 1 pierce 1 [normal] | área r30 d1 p15 [explosão]. Dano/s 1 alvo **1.0**, potencial em área **8.0**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Poções Maiores | 250 (800) | +10 raio da explosão; +10 pierce da explosão | 1.0 | 13 | 1.0 |
| 1-2 | Mistura Ácida | 350 (1150) | **novo ataque**: buff: moab+1 | 1.0 | 13 | 1.0 |
| 1-3 | Poção do Berserker | 1250 (2400) | aura de buff: recarga ×0.9 +10% alcance +2 pierce +1 dano (no ataque [1]) | 1.0 | 13 | 1.0 |
| 1-4 | Estimulante Forte | 3000 (5400) | aura de buff: recarga ×0.85 +1 dano (no ataque [1]) | 1.0 | 13 | 1.0 |
| 1-5 | Poção Permanente | 60000 (65400) | aura de buff: recarga ×0.8 +5 pierce +2 dano (no ataque [1]) | 1.0 | 13 | 1.0 |
| 2-1 | Ácido Forte | 250 (800) | fogo 1 dano/s por 4s | 1.0 | 8.0 | 1.0 |
| 2-2 | Poções Perecíveis | 475 (1275) | +4 dano em MOAB; +2 dano em cerâmica | 1.0 | 8.0 | 3.0 |
| 2-3 | Mistura Instável | 3000 (4275) | +10 dano em MOAB; +10 raio da explosão | 1.0 | 8.0 | 8.0 |
| 2-4 | Tônico Transformador | 4500 (8775) | **Hab. Transformação** (60s): a própria torre ataca 5× mais rápido por 20s | 1.0 | 8.0 | 8.0 |
| 2-5 | Transformação Total | 45000 (53775) | **Hab. Transformação Total** (40s): torres a até alcance+60px atacam 3.33× mais rápido por 20s | 1.0 | 8.0 | 8.0 |
| 3-1 | Arremesso Rápido | 650 (1200) | recarga ×0.75 (+33% ataques/s) | 1.3 | 11 | 1.3 |
| 3-2 | Poça de Ácido | 450 (1650) | **novo ataque**: pilha cad 3s dano 1 pierce 10 vida 8s [normal] | 1.7 | 14 | 1.7 |
| 3-3 | Chumbo em Ouro | 1000 (2650) | +$0.5 por bloon estourado (+$2 extra em chumbo) | 1.7 | 14 | 1.7 |
| 3-4 | Borracha em Ouro | 2750 (5400) | +$1 por bloon estourado (+$2 extra em chumbo) | 1.7 | 14 | 1.7 |
| 3-5 | Mestre Alquimista | 40000 (45400) | +10 dano; +30 dano em MOAB; +5 pierce | 8.3 | 57 | 28 |

**Análise.** A poção é [normal] no impacto, mas a explosão é [explosão] (pretos e zebras imunes). O caminho 1 é suporte: um buff **permanente** de dano/pierce/cadência em todas as torres no alcance (180), diferente do original, onde o Berserker é temporário. Como buff de dano só entra em ataques com dano > 0 (e no `sdano`), o ganho é enorme em torres de cadência alta. O caminho 3 dá ouro por estouro (+$0,5 / +$1,5 por bloon estourado pela própria torre, e +$2 em chumbo). O 3-5 descreve "encolhe bloons", mas só dá dano, MOAB e pierce.

### Druida — $400 · alcance 140 · magica

**Base:** projétil cad 1.1s dano 1 pierce 1 n 5 leque 40° [afiado]. Dano/s 1 alvo **0.9**, potencial em área **4.5**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Espinhos Duros | 250 (650) | +1 pierce; dano vira [normal] | 0.9 | 9.1 | 0.9 |
| 1-2 | Coração do Trovão | 1000 (1650) | **novo ataque**: cadeia cad 2.3s dano 1 pierce 1 [energia] saltos 6 | 1.3 | 12 | 1.3 |
| 1-3 | Druida da Tempestade | 1650 (3300) | **novo ataque**: projétil cad 2.5s pierce 30 empurra 120px | 1.3 | 12 | 1.3 |
| 1-4 | Bola de Relâmpago | 4500 (7800) | +8 saltos da corrente; +2 dano; recarga ×0.6 (+67% ataques/s) (no ataque [1]) | 3.1 | 42 | 3.1 |
| 1-5 | Supertempestade | 60000 (67800) | +8 dano; +10 pierce; +10 dano em MOAB (em todos os ataques) | 19 | 738 | 40 |
| 2-1 | Enxame de Espinhos | 250 (650) | +3 projéteis | 0.9 | 7.3 | 0.9 |
| 2-2 | Coração de Carvalho | 350 (1000) | remove regen | 0.9 | 7.3 | 0.9 |
| 2-3 | Druida da Selva | 950 (1950) | **novo ataque**: pilha cad 3s dano 2 pierce 8 vida 6s [normal] | 1.6 | 13 | 1.6 |
| 2-4 | Recompensa da Selva | 5000 (6950) | **novo ataque**: renda $250/rodada | 1.6 | 13 · $250/rod. | 1.6 |
| 2-5 | Espírito da Floresta | 35000 (41950) | +6 dano; +40 pierce por pilha; recarga ×0.3 (+233% ataques/s) (no ataque [1]) | 9.8 | 434 · $250/rod. | 9.8 |
| 3-1 | Alcance Druídico | 100 (500) | +40 alcance | 0.9 | 4.5 | 0.9 |
| 3-2 | Coração da Vingança | 300 (800) | recarga ×0.85 (+18% ataques/s) | 1.1 | 5.3 | 1.1 |
| 3-3 | Druida da Ira | 600 (1400) | recarga ×0.8 (+25% ataques/s); +1 dano | 2.7 | 13 | 2.7 |
| 3-4 | Luxúria de Estouros | 2500 (3900) | +3 pierce; aura de buff: recarga ×0.85 | 2.7 | 53 | 2.7 |
| 3-5 | Avatar da Ira | 45000 (48900) | +6 dano; +6 pierce; +4 projéteis | 11 | 963 | 11 |

**Análise.** Espalha 5 espinhos em leque de 40°. O 1-2 cria um relâmpago em cadeia (salta 140 px e ignora camo nos saltos). O tornado do 1-3 empurra 120 px, mas com dano 0 e tipo afiado padrão ele **não empurra chumbo nem bloons congelados**. **Bug de cruzamento:** o Espírito da Floresta (2-5) aplica `a:1`. Em 1-5-0 é o cipó (correto), mas em **2-5-0** o índice 1 é o relâmpago: o cipó não é melhorado e o relâmpago ganha +6 dano e recarga ×0,3. A Luxúria de Estouros (3-4) coloca buff no ataque principal, então também se aplica à própria torre.

### Fazenda de Bananas — $1250 · alcance 100 · suporte

**Base:** renda $80/rodada. Dano/s 1 alvo **0.0**, potencial em área **0.0**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Produção Aumentada | 500 (1750) | +$20/rodada | 0.0 | 0.0 · $100/rod. | 0.0 |
| 1-2 | Produção Maior | 600 (2350) | +$40/rodada | 0.0 | 0.0 · $140/rod. | 0.0 |
| 1-3 | Plantação de Bananas | 3000 (5350) | +$140/rodada | 0.0 | 0.0 · $280/rod. | 0.0 |
| 1-4 | Centro de Pesquisa de Bananas | 19000 (24350) | +$800/rodada | 0.0 | 0.0 · $1080/rod. | 0.0 |
| 1-5 | Central de Bananas | 100000 (124350) | +$3000/rodada | 0.0 | 0.0 · $4080/rod. | 0.0 |
| 2-1 | Bananas Duradouras | 300 (1550) | +$10/rodada | 0.0 | 0.0 · $90/rod. | 0.0 |
| 2-2 | Bananas Valiosas | 800 (2350) | renda ×1.25 | 0.0 | 0.0 · $112/rod. | 0.0 |
| 2-3 | Banco Macaco | 3650 (6000) | +$250/rodada | 0.0 | 0.0 · $362/rod. | 0.0 |
| 2-4 | Empréstimo do FMI | 7200 (13200) | **Hab. Empréstimo** (90s): +$5000 | 0.0 | 0.0 · $362/rod. | 0.0 |
| 2-5 | Macaconomia | 100000 (113200) | **Hab. Macaconomia** (60s): +$10000 | 0.0 | 0.0 · $362/rod. | 0.0 |
| 3-1 | Coleta Fácil | 250 (1500) | +$10/rodada | 0.0 | 0.0 · $90/rod. | 0.0 |
| 3-2 | Salvamento de Bananas | 200 (1700) | venda por 90% | 0.0 | 0.0 · $90/rod. | 0.0 |
| 3-3 | Mercado | 2900 (4600) | +$200/rodada | 0.0 | 0.0 · $290/rod. | 0.0 |
| 3-4 | Mercado Central | 15000 (19600) | +$600/rodada | 0.0 | 0.0 · $890/rod. | 0.0 |
| 3-5 | Wall Street dos Macacos | 60000 (79600) | +$4000/rodada | 0.0 | 0.0 · $4890/rod. | 0.0 |

**Análise.** A renda é paga no fim de cada rodada (solo) ou no início (batalha), sem coletar bananas. Retorno do investimento (rodadas para se pagar): base 15,6 · 1-3 8,4 · 1-4 19,8 · 1-5 29,7 · 2-3 16,6 · 3-4 22,0 · 3-5 16,3. A Bananas Valiosas (2-2) multiplica o que os caminhos aplicados **antes** já somaram (ordem: caminho 1, depois 2, depois 3), então rende mais em 5-2-0 ($5.112) do que em 0-2-5 ($4.922). O 3-2 sobe a venda de 70% para 90%.

### Fábrica de Espinhos — $1000 · alcance 136 · suporte

**Base:** pilha cad 2.2s dano 1 pierce 5 vida 40s [afiado]. Dano/s 1 alvo **0.5**, potencial em área **2.3**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Pilhas Maiores | 800 (1800) | +5 pierce por pilha | 0.5 | 4.5 | 0.5 |
| 1-2 | Espinhos Incandescentes | 600 (2400) | dano vira [normal] | 0.5 | 4.5 | 0.5 |
| 1-3 | Bolas Espinhosas | 2300 (4700) | +1 dano; +3 dano em cerâmica | 0.9 | 9.1 | 0.9 |
| 1-4 | Minas Espinhosas | 10000 (14700) | +40 raio da explosão; +5 dano da explosão; +40 pierce da explosão | 0.9 | 9.1 | 0.9 |
| 1-5 | Super Minas | 150000 (164700) | +80 raio da explosão; +400 dano da explosão; +200 pierce da explosão | 0.9 | 9.1 | 0.9 |
| 2-1 | Produção Rápida | 600 (1600) | recarga ×0.75 (+33% ataques/s) | 0.6 | 3.0 | 0.6 |
| 2-2 | Produção Mais Rápida | 800 (2400) | recarga ×0.75 (+33% ataques/s) | 0.8 | 4.0 | 0.8 |
| 2-3 | Triturador de M.O.A.B. | 2500 (4900) | +2 dano em MOAB | 0.8 | 4.0 | 2.4 |
| 2-4 | Tempestade de Espinhos | 5000 (9900) | **Hab. Tempestade de Espinhos** (40s): pilhas de 60 espinhos (dano 2) a cada 140px de toda trilha, 20s | 0.8 | 4.0 | 2.4 |
| 2-5 | Tapete de Espinhos | 40000 (49900) | recarga ×0.6 (+67% ataques/s); **Hab. Tapete de Espinhos** (30s): pilhas de 150 espinhos (dano 4) a cada 140px de toda trilha, 20s | 1.3 | 6.7 | 4.0 |
| 3-1 | Espinhos Duradouros | 150 (1150) | duração da pilha ×2 | 0.5 | 2.3 | 0.5 |
| 3-2 | Espinhos Mortais | 400 (1550) | +1 dano | 0.9 | 4.5 | 0.9 |
| 3-3 | Longo Alcance | 1400 (2950) | +40 alcance; +5 pierce por pilha | 0.9 | 9.1 | 0.9 |
| 3-4 | Espinhominador | 12500 (15450) | +3 dano; +15 pierce por pilha; duração da pilha ×2 | 2.3 | 57 | 2.3 |
| 3-5 | Perma-Espinho | 30000 (45450) | +10 dano; +40 pierce por pilha; duração da pilha ×4 | 6.8 | 443 | 6.8 |

**Análise.** Coloca pilhas em pontos aleatórios da trilha dentro do alcance (até 30 pilhas por torre, só se houver bloon na tela ou na fila). Cada pilha acerta cada bloon uma única vez. O 1-4/1-5 fazem a pilha **explodir** quando acaba a vida ou o pierce (explosão [normal], r120, dano 405 no 1-5). O 2-4/2-5 espalham pilhas por toda a trilha (a cada 140 px). A duração no caminho 3 multiplica (×2, ×2, ×4 → 640 s no 3-5), mas o teto de 30 pilhas limita o ganho.

### Vila dos Macacos — $1200 · alcance 160 · suporte

**Base:** buff: alc+10%. Dano/s 1 alvo **0.0**, potencial em área **0.0**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Raio Maior | 400 (1600) | +40 alcance | 0.0 | 0.0 | 0.0 |
| 1-2 | Tambores da Selva | 1500 (3100) | aura de buff: recarga ×0.85 | 0.0 | 0.0 | 0.0 |
| 1-3 | Treinamento Primário | 800 (3900) | aura de buff: +10% alcance +1 pierce | 0.0 | 0.0 | 0.0 |
| 1-4 | Mentoria Primária | 2500 (6400) | aura de buff: +1 dano | 0.0 | 0.0 | 0.0 |
| 1-5 | Especialização Primária | 25000 (31400) | **novo ataque**: projétil cad 4s dano 1000 pierce 5 [normal] teleguiado global alvo forte | 250 | 1250 | 250 |
| 2-1 | Bloqueador de Crescimento | 250 (1450) | **novo ataque**: aura cad 0.5s pierce 999 tira regen | 0.0 | 0.0 | 0.0 |
| 2-2 | Radar | 2000 (3450) | aura de buff: camo | 0.0 | 0.0 | 0.0 |
| 2-3 | Agência de Inteligência Macaco | 7500 (10950) | aura de buff: dano normal | 0.0 | 0.0 | 0.0 |
| 2-4 | Chamado às Armas | 20000 (30950) | **Hab. Chamado às Armas** (45s): torres a até alcance+60px atacam 1.52× mais rápido por 12s | 0.0 | 0.0 | 0.0 |
| 2-5 | Defesa da Pátria | 40000 (70950) | **Hab. Defesa da Pátria** (60s): todas as torres atacam 2× mais rápido por 20s | 0.0 | 0.0 | 0.0 |
| 3-1 | Negócios Macacos | 500 (1700) | 10% de desconto p/ torres no alcance | 0.0 | 0.0 | 0.0 |
| 3-2 | Comércio Macaco | 500 (2200) | 15% de desconto p/ torres no alcance | 0.0 | 0.0 | 0.0 |
| 3-3 | Cidade Macaco | 10000 (12200) | aura de buff: +$0.5/estouro | 0.0 | 0.0 | 0.0 |
| 3-4 | Metrópole Macaco | 13000 (25200) | **novo ataque**: renda $500/rodada | 0.0 | 0.0 · $500/rod. | 0.0 |
| 3-5 | Macacópolis | 75000 (100200) | +$5000/rodada (no ataque [2]) | 0.0 | 0.0 · $500/rod. | 0.0 |

**Análise.** O buff é recalculado a cada 0,5 s. Buffs de várias vilas/alquimistas **se acumulam** (cadência multiplica, com piso de ×0,4; alcance, dano e pierce somam), ao contrário do original, em que não se acumulam. O 2-3 (MIB) faz toda torre no alcance causar dano [normal], inclusive na explosão, e remove todas as imunidades. **Bug no 3-5:** a Macacópolis aplica `a:2`, que só existe se o caminho 2 tiver pelo menos o 2-1. Em **0-0-5, 1-0-5 e 2-0-5 os +$5.000 somem** e a renda continua em $500. O desconto (3-1/3-2) vale para colocar e upar torres dentro do alcance da vila, e o maior desconto vence (não soma). A aura de remover regen (2-1) tem tipo afiado e é bloqueada por chumbo.

### Macaco Engenheiro — $400 · alcance 160 · suporte

**Base:** projétil cad 0.7s dano 1 pierce 3 [afiado]. Dano/s 1 alvo **1.4**, potencial em área **4.3**/s.

| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |
|---|---|---|---|---|---|---|
| 1-1 | Torreta Sentinela | 500 (900) | **novo ataque**: invocar sentinela a cada 10s (dura 25s, +0 nív.) | 1.4 | 4.3 | 1.4 |
| 1-2 | Engenharia Rápida | 400 (1300) | recarga ×0.6 (+67% ataques/s) (no ataque [1]) | 1.4 | 4.3 | 1.4 |
| 1-3 | Engrenagens | 575 (1875) | +1 nível nas torretas (cada nível: +1 dano, +2 pierce, recarga ×0,8) (no ataque [1]) | 1.4 | 4.3 | 1.4 |
| 1-4 | Especialista em Sentinelas | 2500 (4375) | +1 nível nas torretas (cada nível: +1 dano, +2 pierce, recarga ×0,8) (no ataque [1]) | 1.4 | 4.3 | 1.4 |
| 1-5 | Campeão das Sentinelas | 32000 (36375) | +2 nível nas torretas (cada nível: +1 dano, +2 pierce, recarga ×0,8); recarga ×0.5 (+100% ataques/s) (no ataque [1]) | 1.4 | 4.3 | 1.4 |
| 2-1 | Área de Serviço Maior | 250 (650) | +24 alcance | 1.4 | 4.3 | 1.4 |
| 2-2 | Desconstrução | 350 (1000) | +1 dano em MOAB; +1 dano em fortificado | 1.4 | 4.3 | 2.9 |
| 2-3 | Espuma Purificadora | 800 (1800) | **novo ataque**: pilha cad 4s dano 1 pierce 10 vida 8s [afiado] tira camo tira regen | 1.7 | 6.8 | 3.1 |
| 2-4 | Overclock | 13500 (15300) | **Hab. Overclock** (45s): torres a até alcance+60px atacam 1.67× mais rápido por 30s | 1.7 | 6.8 | 3.1 |
| 2-5 | Ultraimpulso | 105000 (120300) | aura de buff: recarga ×0.7; **Hab. Ultraimpulso** (30s): torres a até alcance+60px atacam 2.5× mais rápido por 45s | 1.7 | 6.8 | 3.1 |
| 3-1 | Pregos Enormes | 450 (850) | +5 pierce; dano vira [normal] | 1.4 | 11 | 1.4 |
| 3-2 | Pino | 450 (1300) | lentidão ×0.6 por 1s | 1.4 | 11 | 1.4 |
| 3-3 | Arma Dupla | 3700 (5000) | recarga ×0.5 (+100% ataques/s) | 2.9 | 23 | 2.9 |
| 3-4 | Armadilha Bloon | 3100 (8100) | **novo ataque**: pilha cad 8s dano 99 pierce 500 vida 30s [normal] | 15 | 6210 | 15 |
| 3-5 | Armadilha XXXL | 54000 (62100) | +4 dano; +2000 pierce por pilha; +500 dano em MOAB (em todos os ataques) | 27 | 32302 | 1518 |

**Análise.** O 1-1 invoca uma torreta temporária (25 s) a cada 10 s, em posição aleatória a 40–70 px, **sem checar trilha nem água**. Cada nível extra (1-3/1-4/1-5) dá +1 dano, +2 pierce e recarga ×0,8 à torreta. A Espuma (2-3) é uma pilha de tipo afiado: **não tira camo nem regen de chumbo e DDT**. A Armadilha Bloon (3-4) é uma pilha de 500 pierce com **99 de dano por bloon** a cada 8 s ($3.100), e é de longe o upgrade com melhor custo/benefício do jogo (6.210/s de potencial por $8.100 no total). A Armadilha XXXL aplica +500 MOAB em todos os ataques, inclusive nos pregos.

## 5. Heróis

Os heróis sobem de nível sozinhos com XP, e os bônus por nível seguem quase sempre o mesmo molde: +alcance nos níveis 2/7/13, +pierce, recarga ×0,9/×0,85/×0,8 e +dano no 6/11/15/18/20. A habilidade 1 libera no nível 3 e a 2 no nível 10.

| Herói | $ | Alcance | Ataque nível 1 | Nv1 1alvo/área | Nv10 1alvo/área | Nv20 1alvo/área (MOAB) | Habilidades (nv3 / nv10) |
|---|---|---|---|---|---|---|---|
| **Quincy** (Arqueiro Orgulhoso) | 540 | 160 | projétil cad 0.95s dano 1 pierce 3 [afiado] | 1.1 / 3.2 | 5.2 / 31 | 49 / 742 (63) | **Hab. Tiro Rápido** (45s): a própria torre ataca 3.03× mais rápido por 7s · **Hab. Tempestade de Flechas** (60s): 12 de dano [normal] em todo bloon na tela |
| **Gwendolin** (Cientista Piromaníaca) | 725 | 150 | projétil cad 0.9s dano 1 pierce 2 [energia] fogo 1dps 2s | 1.1 / 2.2 | 2.7 / 14 | 26 / 365 (26) | **Hab. Coquetel de Fogo** (20s): pilha de 30 pierce (dano 1) no ponto mais avançado da trilha no alcance, 8s · **Hab. Tempestade de Fogo** (60s): 40 de dano [normal] em todo bloon na tela + fogo |
| **Striker Jones** (Comandante de Artilharia) | 750 | 170 | projétil cad 1.3s dano 1 pierce 1 [afiado] | área r35 d1 p10 [explosão] | 1.5 / 8.5 | 3.8 / 27 | 31 / 366 (31) | **Hab. Projétil de Concussão** (20s): 40 de dano em 1 alvo(s) mais forte(s) + atordoa 4s · **Hab. Comando de Artilharia** (60s): todas as torres do tipo bomba,morteiro atacam 2× mais rápido por 10s |
| **Obyn Guardião** (Guardião da Floresta) | 650 | 160 | projétil cad 1.35s dano 2 pierce 4 [energia] teleguiado | 1.5 / 5.9 | 2.7 / 19 | 19 / 304 (19) | **Hab. Espinheiros** (25s): pilha de 60 pierce (dano 1) no ponto mais avançado da trilha no alcance, 12s · **Hab. Muralha de Árvores** (50s): pilha de 2000 pierce (dano 1) no ponto mais avançado da trilha no alcance, 15s |
| **Capitão Churchill** (Tanque Blindado) | 2000 | 170 | projétil cad 0.6s dano 3 pierce 1 [normal] | área r25 d2 p6 [normal] | 8.3 / 25 | 12 / 58 | 53 / 644 (206) | **Hab. Projéteis Perfurantes** (40s): a própria torre ataca 2.5× mais rápido por 8s · **Hab. Barragem M.O.A.B.** (60s): 500 de dano em 5 alvo(s) mais forte(s) (só MOAB) |
| **Benjamin** (Hacker) | 1200 | 100 | renda $60/rodada | $60/rod. | $780/rod. | $2530/rod. (0.0) | **Hab. Sifão de Fundos** (30s): +$250 · **Hab. Invasão Bancária** (60s): +$1500 |
| **Ezili** (Sacerdotisa Vodu) | 600 | 150 | projétil cad 1s dano 1 pierce 2 [normal] fogo 1dps 3s | 1.0 / 2.0 | 2.5 / 12 | 23 / 329 (34) | **Hab. Para-Coração** (30s): todos os bloons a ×0.7 de velocidade por 8s · **Hab. Maldição M.O.A.B.** (50s): 1500 de dano em 3 alvo(s) mais forte(s) (só MOAB) |
| **Pat Fusty** (Macaco Gigante) | 800 | 80 | aura cad 1.5s dano 2 pierce 10 [normal] | 1.3 / 13 | 2.5 / 32 | 17 / 376 (31) | **Hab. Rugido de Incentivo** (45s): torres a até alcance+60px atacam 1.43× mais rápido por 10s · **Hab. Grande Aperto** (60s): 5000 de dano em 1 alvo(s) mais forte(s) (só MOAB) |
| **Adora** (Sacerdotisa do Sol) | 1000 | 170 | projétil cad 0.8s dano 2 pierce 4 [energia] teleguiado | 2.5 / 10 | 4.6 / 97 | 32 / 1538 (53) | **Hab. Braço Longo da Luz** (40s): a própria torre ataca 2.5× mais rápido por 10s · **Hab. Bola de Luz** (60s): invoca fenix por 15s |
| **Almirante Brickell** (Comandante Naval) | 900 | 180 | projétil cad 0.4s dano 1 pierce 3 [afiado] | 2.5 / 7.5 | 6.2 / 37 | 59 / 881 (59) | **Hab. Táticas Navais** (45s): todas as torres atacam 2× mais rápido por 10s · **Hab. Mega Mina** (60s): pilha de 40 pierce (dano 1500) no ponto mais avançado da trilha no alcance, 30s |
| **Etienne** (Especialista em Drones) | 850 | global | projétil cad 0.5s dano 1 pierce 3 [afiado] teleguiado global | 2.0 / 6.0 | 4.9 / 59 | 47 / 2115 (73) | **Hab. Enxame de Drones** (45s): a própria torre ataca 3.33× mais rápido por 15s · **Hab. UCAV** (60s): invoca fenix por 20s |
| **Sauda** (Espadachim) | 600 | 70 | aura cad 0.6s dano 2 pierce 8 [normal] +2 cerâm. | 3.3 / 27 | 6.2 / 68 | 43 / 854 (78) | **Hab. Espada Saltitante** (20s): 100 de dano em 8 alvo(s) mais forte(s) · **Hab. Investida da Espada** (45s): 60 de dano [normal] em todo bloon na tela |
| **Psi** (Macaco Psíquico) | 1200 | global | hitscan cad 1.4s dano 3 pierce 1 [normal] global | 2.1 / 2.1 | 3.5 / 3.5 | 20 / 20 (58) | **Hab. Explosão Psíquica** (25s): 300 de dano em 3 alvo(s) mais forte(s) · **Hab. Grito Psiônico** (60s): 150 de dano [normal] em todo bloon na tela + atordoa 4s |
| **Geraldo** (Comerciante Místico) | 725 | 150 | projétil cad 0.8s dano 1 pierce 2 [afiado] | 1.2 / 2.5 | 3.1 / 15 | 29 / 411 (29) | **Hab. Torreta Atiradora** (40s): invoca sentinela por 25s · **Hab. Armadilha de Lâminas** (50s): pilha de 400 pierce (dano 4) no ponto mais avançado da trilha no alcance, 20s |
| **Corvus** (Guardião das Almas) | 1150 | 170 | projétil cad 0.6s dano 2 pierce 4 [energia] teleguiado | 3.3 / 13 | 6.2 / 43 | 43 / 683 (78) | **Hab. Lança Espiritual** (25s): 200 de dano em 2 alvo(s) mais forte(s) · **Hab. Colheita de Almas** (60s): 80 de dano [normal] em todo bloon na tela |
| **Rosalia** (Engenheira com Jetpack) | 800 | 170 | projétil cad 0.8s dano 2 pierce 3 [energia] | 2.5 / 7.5 | 4.6 / 28 | 37 / 523 (37) | **Hab. Propulsores** (40s): a própria torre ataca 2.5× mais rápido por 10s · **Hab. Tempestade de Foguetes** (60s): 50 de dano [normal] em todo bloon na tela |
| **Jericho** (Bandoleiro (exclusivo do Battles)) | 750 | 160 | projétil cad 0.8s dano 1 pierce 2 [afiado] | 1.2 / 2.5 | 3.1 / 15 | 29 / 411 (29) | **Hab. Proteger a Missão** (30s): a própria torre ataca 2× mais rápido por 8s · **Hab. Salteador** (60s): +$600 e tira o mesmo do oponente |
| **Silas** (Mago do Gelo) | 700 | 140 | projétil cad 0.9s dano 1 pierce 3 [gelo] lento ×0.6 1.5s | 1.1 / 3.3 | 2.7 / 16 | 26 / 392 (26) | **Hab. Raio Congelante** (25s): 60 de dano em 4 alvo(s) mais forte(s) + congela 3s · **Hab. Tempestade Glacial** (60s): congela todos por 6s (inclui MOAB, metade do tempo) |

**Análise dos heróis:**

- **Benjamin** é o mais forte em economia: $2.530 por rodada no nível 20, mais $250 a cada 30 s e $1.500 a cada 60 s. Não tem ataque, então precisa de defesa em volta.
- **Churchill** ($2.000) é o melhor anti-MOAB passivo (206 MOAB/s no nível 20) e ainda tem uma barragem de 5×500.
- **Etienne e Psi** são globais e detectam camo desde o nível 1, o que cobre a fraqueza de camo sem Vila Radar.
- **Adora** chega a 1.538/s de potencial em área (3 projéteis teleguiados de pierce 16).
- **Brickell** só pode ficar na água: **não dá para usar no Prado**, e na Espiral o lago tem raio 45.
- **O Salteador do Jericho** no solo só dá +$600, porque não existe oponente para roubar.
- **Etienne (UCAV) e Adora (Bola de Luz)** invocam uma Fênix (dano 5 × 2 projéteis a cada 0,1 s, global) por 15–20 s: são as habilidades de nível 10 mais fortes.
- **Sauda e Pat** têm alcance de 70–80 e dependem de ficar colados na trilha.


## 6. Mapas

| Mapa | Rótulo | Trilhas | Comprimento | Área livre em terra | Água | Melhor ponto (px de trilha a r128 / r160) | Tempo de travessia (vermelho / cerâmica / MOAB / DDT) |
|---|---|---|---|---|---|---|---|
| Prado dos Macacos | Iniciante | 1 | 3.180 px | 65% | nenhuma | 712 / 840 | 33,5 s / 13,4 s / 33,5 s / 12,2 s |
| Lago Sereno | Iniciante | 1 | 3.180 px | 58% | 10% (círculo r150 no centro) | 360 / 592 | 33,5 s / 13,4 s / 33,5 s / 12,2 s |
| Encruzilhada | Intermediário | **2** (alternadas) | 1.640 + 1.660 px | 65% | 3% (retângulo 170×110) | 600 / 808 (somando as 2) | **17,5 s / 7,0 s / 17,5 s / 6,4 s** |
| Espiral da Selva | Avançado | 1 | **5.620 px** | 44% | 1% (círculo r45) | 584 / 872 | **59,2 s / 23,7 s / 59,2 s / 21,5 s** |

**Análise:**

- **Prado**: é o mapa mais fácil de verdade. A trilha dá voltas em "S", e perto de (340, 490) uma torre de alcance 128 cobre 712 px de trilha, 22% do total. O único ponto contra é não ter água: Submarino, Bucaneiro e Brickell ficam fora.
- **Lago**: a trilha tem o mesmo comprimento do Prado, mas contorna o lago pela borda. O melhor ponto cobre só **metade** do que cobre no Prado (360 px) e há menos terra livre. É um pouco mais difícil que o Prado, então o rótulo "Iniciante" está no limite. O lago grande favorece torres de água.
- **Encruzilhada**: os bloons alternam de trilha (`proximo_cam % 2`), então cada trilha recebe metade da rodada, mas cada bloon **atravessa em metade do tempo**. A defesa precisa cobrir dois caminhos, e torres de alcance curto (Tachinha, Gelo, Sauda, Pat) valem metade. As trilhas se cruzam perto de (400, 330–420), onde uma torre cobre as duas. **É o mapa mais difícil.**
- **Espiral**: a trilha é 77% mais longa que a do Prado e tem o **melhor ponto do jogo** a r160 (872 px, em volta de (670, 180)). A área livre é menor (44%), mas o tempo extra de travessia compensa com folga, então o mapa é **mais fácil** que o Prado apesar do rótulo "Avançado". O lago r45 cabe no máximo 1–2 torres de água.
- **Recomendação:** reordenar para Prado (Iniciante) → Espiral (Iniciante/Intermediário) → Lago (Intermediário) → Encruzilhada (Avançado), ou encurtar a Espiral e adicionar obstáculos no miolo dela.

## 7. Dificuldades e rodadas

### 7.1 Dificuldades (modo solo)

| Dificuldade | Vidas | Custo | Última rodada | Chefe final | RBE total até o fim | $ acumulado aprox. até o fim* |
|---|---|---|---|---|---|---|
| Fácil | 200 | ×0,85 | 40 | 1 M.O.A.B. | 11.528 | ~$16,7 mil |
| Médio | 150 | ×1,00 | 60 | 1 B.F.B. | 71.885 | ~$70,9 mil |
| Difícil | 100 | ×1,08 | 80 | 1 Z.O.M.G. + 2 B.F.B. | 321.145 | ~$241 mil |
| Impossível | **1** | ×1,20 | 100 | 1 B.A.D. | 1.769.097 | ~$896 mil |

\* $650 + $1 por camada estourada + ($100 + rodada) no fim de cada rodada, sem contar rendas. A renda é a mesma em todas as dificuldades: a dificuldade muda só vidas, preço e duração.

**Análise:**

- **As vidas quase não fazem diferença**, porque 1 cerâmica vazada tira 104, 1 MOAB tira 616 e 1 BFB tira 3.164. No Fácil (200 vidas), um único MOAB vazado na rodada 40 **perde a partida**. No Difícil (100), uma cerâmica já mata. Na prática todas as dificuldades a partir da rodada 38 são "sem vazamento".
- Com o preço ×1,2 do Impossível, a Armadilha Bloon (3-4) sai por ~$9,7 mil e o Raio da Perdição por ~$110 mil, então as rodadas 90+ exigem a economia de 2–3 Fazendas 5-x-x ou do Benjamin.
- **Sugestões:** dar vidas mínimas que absorvam um MOAB no Fácil (≥ 650) ou limitar a perda por vazamento de dirigível; e dar um bônus de renda ou de dinheiro inicial nas dificuldades fáceis.

### 7.2 Curva de rodadas

Marcos: 1º azul na R3, verde na R6, amarelo na R11, rosa na R15, **regen na R17**, preto na R20, branco na R22, **camo na R24**, roxo na R25, zebra na R26, **chumbo na R28**, arco-íris na R35, cerâmica na R38, **MOAB na R40**, **fortificado na R50**, BFB na R60, ZOMG na R80, DDT na R90 e B.A.D. na R100.

**Análise:**

- **Picos isolados no começo**: a R10 (102 azuis em 17 s) tem 2× o RBE da R9, e a R27 (250 bloons, 535 RBE) e a R32 (583) são os picos antes da R35. Entre elas há rodadas bem mais leves (R28 com 138, R33 com 93).
- **Buraco na R40 e na R60**: são rodadas de um único dirigível, com RBE/s altíssimo (o dirigível entra todo de uma vez), mas curtas. Elas testam só dano em MOAB.
- **As rodadas 81–99 geradas por fórmula são mais pesadas que as rodadas chefe**: R89 = 70.940 contra R90 = 6.144 (6 DDT); R99 = 136.172 contra R100 = 56.384. **O chefe final é mais fácil que a rodada anterior.** Sugestão: fazer as rodadas chefe somarem ao gerador (por exemplo, B.A.D. + a mistura da R99) ou reduzir o crescimento do gerador (`80 + 6k` cerâmicas).
- **Camo aparece cedo e sozinho**: a R24 tem 1 verde camo e a R33 tem 20 amarelos camo. Isso força detecção de camo antes da R25, e só o Dardo 3-2 ($490), o Ninja e os heróis globais resolvem barato.
- **Fortificados** (R50+) só afetam chumbo, cerâmica e dirigíveis, e só o Engenheiro 2-2 tem bônus `fort`.

<details><summary>Tabela completa das 100 rodadas (RBE, duração e propriedades: C=camo, R=regen, F=fortificado, M=dirigível)</summary>

| R | Composição | RBE | Duração | RBE/s | Props |
|---|---|---|---|---|---|
| 1 | 20 Vermelho | 20 | 17.1s | 1 | - |
| 2 | 35 Vermelho | 35 | 18.7s | 2 | - |
| 3 | 25 Vermelho, 5 Azul | 35 | 14.4s | 2 | - |
| 4 | 35 Vermelho, 18 Azul | 71 | 17.6s | 4 | - |
| 5 | 5 Vermelho, 27 Azul | 59 | 15.0s | 4 | - |
| 6 | 15 Vermelho, 15 Azul, 4 Verde | 57 | 11.6s | 5 | - |
| 7 | 20 Vermelho, 25 Azul, 5 Verde | 85 | 16.0s | 5 | - |
| 8 | 10 Vermelho, 20 Azul, 14 Verde | 92 | 15.8s | 6 | - |
| 9 | 30 Verde | 90 | 16.0s | 6 | - |
| 10 | 102 Azul | 204 | 17.2s | 12 | - |
| 11 | 10 Vermelho, 10 Azul, 12 Verde, 2 Amarelo | 74 | 10.5s | 7 | - |
| 12 | 15 Azul, 10 Verde, 5 Amarelo | 80 | 12.0s | 7 | - |
| 13 | 50 Azul, 23 Verde | 169 | 17.0s | 10 | - |
| 14 | 49 Vermelho, 15 Azul, 10 Verde, 9 Amarelo | 145 | 18.6s | 8 | - |
| 15 | 20 Vermelho, 15 Verde, 12 Amarelo, 5 Rosa | 138 | 18.0s | 8 | - |
| 16 | 20 Verde, 8 Amarelo | 92 | 10.6s | 9 | - |
| 17 | 8 Amarelo regen | 32 | 5.6s | 6 | R |
| 18 | 80 Verde | 240 | 17.4s | 14 | - |
| 19 | 10 Verde, 4 Amarelo, 5 Amarelo regen, 7 Rosa | 101 | 15.2s | 7 | R |
| 20 | 6 Preto | 66 | 5.0s | 13 | - |
| 21 | 14 Amarelo, 40 Rosa | 256 | 16.7s | 15 | - |
| 22 | 16 Branco | 176 | 12.0s | 15 | - |
| 23 | 7 Preto, 7 Branco | 154 | 8.8s | 18 | - |
| 24 | 1 Verde camo, 20 Azul | 43 | 9.6s | 4 | C |
| 25 | 31 Amarelo regen, 10 Roxo | 234 | 15.2s | 15 | R |
| 26 | 23 Rosa, 4 Zebra | 207 | 11.6s | 18 | - |
| 27 | 100 Vermelho, 60 Azul, 45 Verde, 45 Amarelo | 535 | 23.8s | 22 | - |
| 28 | 6 Chumbo | 138 | 6.0s | 23 | - |
| 29 | 48 Amarelo, 12 Rosa regen | 252 | 14.6s | 17 | R |
| 30 | 9 Chumbo | 207 | 8.0s | 26 | - |
| 31 | 8 Preto, 8 Branco, 4 Zebra regen | 268 | 10.0s | 27 | R |
| 32 | 25 Preto, 28 Branco | 583 | 14.4s | 40 | - |
| 33 | 13 Vermelho, 20 Amarelo camo | 93 | 10.6s | 9 | C |
| 34 | 140 Amarelo, 5 Zebra | 675 | 16.7s | 40 | - |
| 35 | 35 Rosa, 30 Preto, 25 Branco, 5 Arco-íris | 1015 | 19.8s | 51 | - |
| 36 | 81 Rosa | 405 | 12.0s | 34 | - |
| 37 | 20 Preto, 20 Branco, 15 Zebra regen, 10 Branco camo | 895 | 19.4s | 46 | CR |
| 38 | 42 Rosa, 17 Branco, 14 Chumbo, 10 Zebra, 4 Cerâmica | 1365 | 21.5s | 63 | - |
| 39 | 10 Preto, 10 Branco, 20 Chumbo, 18 Arco-íris regen | 1526 | 19.5s | 78 | R |
| 40 | 1 M.O.A.B. | 616 | 0.0s | 616 | M |
| 41 | 60 Preto, 60 Zebra | 2040 | 17.8s | 115 | - |
| 42 | 6 Arco-íris regen, 4 Arco-íris camo | 470 | 8.4s | 56 | CR |
| 43 | 10 Arco-íris, 7 Cerâmica | 1198 | 11.4s | 105 | - |
| 44 | 50 Zebra | 1150 | 12.2s | 94 | - |
| 45 | 200 Rosa, 8 Cerâmica regen | 1832 | 15.9s | 115 | R |
| 46 | 10 Preto camo, 1 M.O.A.B. | 726 | 6.0s | 121 | CM |
| 47 | 70 Rosa camo, 12 Cerâmica | 1598 | 14.6s | 109 | C |
| 48 | 120 Rosa regen, 50 Arco-íris | 2950 | 22.2s | 133 | R |
| 49 | 343 Verde, 20 Zebra, 30 Arco-íris, 15 Cerâmica | 4459 | 28.4s | 157 | - |
| 50 | 20 Chumbo fort, 2 M.O.A.B. | 1752 | 8.0s | 219 | FM |
| 51 | 28 Arco-íris camo, 10 Cerâmica | 2356 | 12.5s | 188 | C |
| 52 | 25 Cerâmica regen, 2 M.O.A.B. | 3832 | 10.0s | 383 | RM |
| 53 | 80 Rosa camo, 3 M.O.A.B. | 2248 | 9.0s | 250 | CM |
| 54 | 35 Cerâmica, 2 M.O.A.B. | 4872 | 12.0s | 406 | M |
| 55 | 45 Cerâmica regen | 4680 | 13.2s | 355 | R |
| 56 | 40 Arco-íris camo, 3 M.O.A.B. | 3728 | 11.0s | 339 | CM |
| 57 | 40 Cerâmica, 4 M.O.A.B. | 6624 | 13.6s | 487 | M |
| 58 | 30 Chumbo fort, 25 Cerâmica fort | 3630 | 15.2s | 239 | F |
| 59 | 50 Cerâmica regen, 3 M.O.A.B. | 7048 | 15.0s | 470 | RM |
| 60 | 1 B.F.B. | 3164 | 0.0s | 3164 | M |
| 61 | 120 Zebra camo, 5 M.O.A.B. | 5840 | 12.0s | 487 | CM |
| 62 | 60 Cerâmica camo, 4 M.O.A.B. | 8704 | 13.6s | 640 | CM |
| 63 | 50 Chumbo, 75 Cerâmica | 8950 | 17.1s | 523 | - |
| 64 | 9 M.O.A.B. | 5544 | 6.4s | 866 | M |
| 65 | 80 Zebra, 60 Arco-íris, 40 Cerâmica, 1 B.F.B. | 11984 | 21.8s | 551 | M |
| 66 | 50 Cerâmica fort, 4 M.O.A.B. | 8164 | 11.0s | 742 | FM |
| 67 | 6 M.O.A.B., 40 Cerâmica camo | 7856 | 13.8s | 571 | CM |
| 68 | 4 M.O.A.B. fort, 1 B.F.B. | 6588 | 6.0s | 1098 | FM |
| 69 | 60 Chumbo fort, 50 Cerâmica regen | 6760 | 17.8s | 380 | RF |
| 70 | 200 Arco-íris, 4 M.O.A.B. | 11864 | 13.0s | 913 | M |
| 71 | 70 Cerâmica camo regen, 5 M.O.A.B. | 10360 | 13.2s | 785 | CRM |
| 72 | 50 Chumbo, 8 M.O.A.B. fort | 7998 | 10.2s | 784 | FM |
| 73 | 100 Cerâmica fort | 11400 | 9.9s | 1152 | F |
| 74 | 3 B.F.B., 8 M.O.A.B. | 14420 | 8.2s | 1759 | M |
| 75 | 80 Cerâmica camo, 2 B.F.B. | 14648 | 12.0s | 1221 | CM |
| 76 | 120 Cerâmica regen fort | 13680 | 9.5s | 1437 | RF |
| 77 | 14 M.O.A.B. fort, 2 B.F.B. | 18312 | 10.0s | 1831 | FM |
| 78 | 150 Cerâmica fort, 3 B.F.B. | 26592 | 13.0s | 2046 | FM |
| 79 | 20 M.O.A.B. fort, 3 B.F.B. | 26612 | 11.0s | 2419 | FM |
| 80 | 1 Z.O.M.G., 2 B.F.B. | 22984 | 7.0s | 3283 | M |
| 81 | 86 Cerâmica regen, 7 M.O.A.B. fort, 2 B.F.B. | 21264 | 9.5s | 2238 | RFM |
| 82 | 92 Cerâmica regen, 8 M.O.A.B. fort, 2 B.F.B. | 22744 | 9.5s | 2394 | RFM |
| 83 | 98 Cerâmica regen, 9 M.O.A.B. fort, 3 B.F.B. | 27388 | 11.0s | 2490 | RFM |
| 84 | 104 Cerâmica regen fort, 10 M.O.A.B. fort, 3 B.F.B., 1 D.D.T. | 30932 | 14.0s | 2209 | CRFM |
| 85 | 2 Z.O.M.G., 120 Cerâmica fort | 46992 | 13.5s | 3476 | FM |
| 86 | 116 Cerâmica regen fort, 12 M.O.A.B. fort, 4 B.F.B., 1 Z.O.M.G., 1 D.D.T. | 53832 | 14.0s | 3845 | CRFM |
| 87 | 122 Cerâmica regen fort, 13 M.O.A.B. fort, 4 B.F.B. fort, 1 Z.O.M.G., 1 D.D.T. | 62012 | 14.0s | 4429 | CRFM |
| 88 | 128 Cerâmica regen fort, 14 M.O.A.B. fort, 4 B.F.B. fort, 1 Z.O.M.G., 2 D.D.T. | 64576 | 14.8s | 4363 | CRFM |
| 89 | 134 Cerâmica regen fort, 15 M.O.A.B. fort, 5 B.F.B. fort, 1 Z.O.M.G., 2 D.D.T. | 70940 | 14.8s | 4793 | CRFM |
| 90 | 6 D.D.T. | 6144 | 5.0s | 1229 | CM |
| 91 | 146 Cerâmica regen fort, 17 M.O.A.B. fort, 5 B.F.B. fort, 1 Z.O.M.G., 2 D.D.T. | 74020 | 14.8s | 5001 | CRFM |
| 92 | 152 Cerâmica regen fort, 18 M.O.A.B. fort, 6 B.F.B. fort, 2 Z.O.M.G., 3 D.D.T. | 98064 | 15.6s | 6286 | CRFM |
| 93 | 158 Cerâmica regen fort, 19 M.O.A.B. fort, 6 B.F.B. fort, 2 Z.O.M.G., 3 D.D.T. | 99604 | 15.6s | 6385 | CRFM |
| 94 | 164 Cerâmica regen fort, 20 M.O.A.B. fort, 6 B.F.B. fort, 2 Z.O.M.G., 3 D.D.T. | 101144 | 15.6s | 6484 | CRFM |
| 95 | 4 Z.O.M.G. fort, 10 D.D.T. | 119424 | 11.4s | 10476 | CFM |
| 96 | 176 Cerâmica regen fort, 22 M.O.A.B. fort, 7 B.F.B. fort, 2 Z.O.M.G., 4 D.D.T. | 110072 | 17.0s | 6475 | CRFM |
| 97 | 182 Cerâmica regen fort, 23 M.O.A.B. fort, 7 B.F.B. fort, 2 Z.O.M.G., 4 D.D.T. | 111612 | 17.0s | 6565 | CRFM |
| 98 | 188 Cerâmica regen fort, 24 M.O.A.B. fort, 8 B.F.B. fort, 3 Z.O.M.G., 4 D.D.T. | 134632 | 18.5s | 7277 | CRFM |
| 99 | 194 Cerâmica regen fort, 25 M.O.A.B. fort, 8 B.F.B. fort, 3 Z.O.M.G., 4 D.D.T. | 136172 | 18.5s | 7361 | CRFM |
| 100 | 1 B.A.D. | 56384 | 0.0s | 56384 | M |

</details>

### 7.3 Modo Batalha

- O modo tem 150 vidas cada e $650 iniciais, sem rodada final. As rodadas vêm sozinhas: a duração da agenda + 5 s de pausa.
- **A eco começa em 250 a cada 6 s (~$2.500 por minuto)**. Os envios mexem pouco nisso: 8 vermelhos custam $25 e somam +1,0 de eco, então se pagam em 150 s. Envios caros se pagam cada vez mais devagar (2 cerâmicas: 600 s; 3 cerâmicas fortificadas: 1.133 s), e dirigíveis **tiram** eco (BFB −10, ZOMG −30, B.A.D. −80).
- Eficiência de ataque (RBE por $): vermelhos 0,32; cerâmica 0,52; MOAB 0,41; BFB 0,53; **ZOMG 0,93**; **B.A.D. 0,94**; DDT 0,16 (mas é camo e imune). No fim do jogo, ZOMG e B.A.D. são as melhores ameaças por dinheiro.
- O teto de uma pista é a RBE/s que ela consegue estourar, e os envios têm recarga de só 0,6 s. Quem guardar dinheiro pode mandar 3 B.A.D. seguidos (~169 mil de RBE) a partir da R45.

## 8. Lista de bugs encontrados (com correção sugerida)

| # | Onde | Problema | Correção sugerida |
|---|---|---|---|
| 1 | Submarino 1-5 Energizador | `a:1` aponta para a aura de revelar camo (1-3); o reator é o índice 2 | Trocar para `a:2` |
| 2 | Ninja 3-5 Mestre Bombardeiro | `a:2` é a Bomba de Luz; a Grudenta é o índice 3 | Trocar para `a:3` |
| 3 | Vila 3-5 Macacópolis | `a:2` só existe com Vila 0-1+-5; em x-0-5 os +$5.000 somem | Resolver o alvo pelo `tipo` (ex.: `"a": "renda"`) em vez de índice |
| 4 | Bucaneiro 2-2, 3-4, 3-5 | `a:1` muda de alvo conforme o cruzamento (aviões ou uvas) | Mesmo esquema de alvo por nome ou tipo |
| 5 | Druida 2-5 | `a:1` é o relâmpago quando o caminho 1 ≥ 2 | Mesmo esquema |
| 6 | Tipo padrão dos ataques utilitários | Aura/pilha de dano 0 usa `afiado` e é bloqueada por imunidade (DDT, chumbo, congelado), então não revela camo, não tira regen, não desacelera, não empurra e não cola | Usar `dtype: normal` nesses ataques, ou aplicar os efeitos sem dano antes da checagem de imunidade |
| 7 | Ás 3-3 Mira Infalível | `busca` no radial não funciona (`disparar` sem alvo) | Passar o alvo mais próximo de cada dardo ou mirar no `alvo()` |
| 8 | Mago 3-5 Príncipe das Trevas | `a:todos` dá dano à aura de Cintilar (pierce 999) | Pular ataques com dano 0 no `todos`, ou usar índices |
| 9 | Cola 3-1/3-2/3-5 | Redefinem `cola` inteira e apagam a corrosão do caminho 1 | Separar `cola_dps` em um efeito aditivo próprio |
| 10 | Gelo 3-3 | O `subst` apaga Permafrost, [normal] e congelamento dos outros caminhos | Aplicar o `subst` antes dos outros caminhos ou copiar os efeitos |
| 11 | Buffs | Buffs de várias fontes se acumulam sem limite; a Nau Capitânia aplica 1× por ataque; o Shinobi/Luxúria se aplicam à própria torre | Guardar só o maior buff de cada tipo, contar cada torre-fonte 1×, e pular a própria torre |
| 12 | Cola 1-1, Ás 3-2 | Upgrades sem efeito | Dar efeito (ex.: +pierce da cola nas camadas / órbita menor) |
| 13 | Submarino 1-2 | Mira global com projétil de distância 260 | Somar `dist` ou mudar para hitscan |
| 14 | Engenheiro 1-x | A torreta invocada nasce na trilha ou na água | Tentar posições válidas com `posicao_valida` |
| 15 | Morteiro | A descrição diz "local escolhido", mas mira sozinho | Ajustar o texto ou implementar a mira manual |
| 16 | Regen | O preto regen filho de chumbo vira zebra | Tratar os filhos de chumbo como no roxo e no branco |

## 9. Resumo de balanceamento das torres

| Faixa | Torres / upgrades |
|---|---|
| **Fortes demais** | Engenheiro 3-4/3-5, Dartling 1-5, Super 1-4/1-5, Ás 3-4/3-5, Alquimista 1-3+ (permanente e acumulável), Submarino 1-5 (por bug), Ninja 3-5 (por bug), Mago 3-5 (por bug) |
| **Bem posicionadas** | Dardo, Bumerangue 1-x/2-x, Bomba 2-x, Tachinha 1-x, Sniper, Bucaneiro 1-x, Heli 1-x/3-x, Mago 1-x/2-x, Druida 1-x, Fazenda, Espinhos, Vila 1-x/2-x |
| **Fracas** | Cola (sem pegar chumbo, com bugs de cruzamento), Gelo 3-x com cruzamento, Heli 2-x, Vila 3-4 (retorno em 50 rodadas), Druida 2-4 (28 rodadas), Submarino 1-2, Ás 3-3, Alquimista 3-5 (descrição promete mais do que faz) |

Os números podem ser gerados de novo com um pequeno programa que usa o `bloons_nucleo` (`calcular()`, `rbe()`, `agenda_da_rodada()`, `mapa().caminhos`). Esse programa não foi versionado.
