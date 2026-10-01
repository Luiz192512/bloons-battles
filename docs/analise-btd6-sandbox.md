# Análise do BTD6 jogado no Sandbox e comparação com o clone

Análise feita jogando o Bloons TD 6 no modo Sandbox (fase 1) e comparando com o código e as
capturas do clone (fase 2). Os números de custo, dano e pierce estão em
[analise-jogo-real.md](analise-jogo-real.md) e não se repetem aqui: o foco é o que só se vê
jogando (forma e comportamento do disparo, visual de cada tier e interface).

## 0. Resumo

Foram testadas no BTD6 as 22 torres que o clone também tem, cada uma nos 3 caminhos até o
tier 5, e ainda o Beast Handler (tiers 4). O backlog da seção 6 tem **3 itens P0, 15 P1 e 11 P2**.
O clone já reproduz bem a maior parte dos disparos (divisão do Ultra-Juggernaut, glaives em
órbita, leque do Triple Shot, cipós na trilha, zumbis, anéis de fogo e de tachinhas). As lacunas
estão em modos de alvo, feedback visual e mecânicas especiais de upgrade.

As 10 lacunas de maior impacto:

0. **Projéteis erram alvos distantes** (B28, P0): o Bucaneiro, por exemplo, só acerta um bloon
   vermelho que cruza a linha de tiro até 120 px, com alcance de 240.
1. **Dartling não mira no cursor** (B01, P0): no BTD6 a mira é o que define a torre.
2. **Mortar sem "Set Target"** (B02, P0): o clone mira no bloon; no BTD6 o jogador fixa o ponto.
3. **Modos de alvo próprios de cada torre ausentes** (B05, B06, B15, B19): rotas do Ás, Heli
   seguindo o mouse, Sniper Elite, Anti-Bloon com dois alvos.
4. **Sem texto "CRIT"** (B03): o crítico existe na conta, mas o jogador não o vê.
5. **Sem modo Sandbox** (B04): é a ferramenta que o BTD6 oferece para testar torres.
6. **Sacrifício do Sun Temple sem diálogo nem efeito** (B08).
7. **Fan Club e Total Transformation sem a transformação visual** (B07).
8. **Bananas não caem para coletar** (B09) e **Monkeyopolis sem a trava "Requires Banana Farm"**
   (B10).
9. **Habilidade do Maelstrom é só turbo** (B12): no BTD6 é a espiral de serras pelo mapa.
10. **Tier 5 sem troca de silhueta** (B27): no BTD6 o tier 5 vira outro objeto (catapulta, templo,
    robô, lançador com cara de tubarão); no clone continua o mesmo macaco com um acessório.

A coluna do clone vem do código e das capturas antigas, e a leitura do código resolveu os 8
itens que estavam como "conferir". Depois o acesso ao clone foi liberado e ele foi jogado por
pouco tempo no modo `--demo` (seção 3.1), o que rendeu o item B27.

## 1. Sessão

- **Jogo:** Bloons TD 6 da Steam, versão **56.3.11937** (rodapé da tela inicial).
- **Mapas:** Monkey Meadow (Beginner), Easy, Sandbox, para as torres de terra; In The Loop
  (Beginner), Easy, Sandbox, para Sub e Buccaneer, que precisam de água. Dinheiro e vidas aparecem
  como 9.999.999 e 999.999.
- **Data:** 30/09/2026.
- **Conta:** nível 108. Todos os upgrades testados estavam liberados (a tela de Upgrades do Dart
  mostra os 15 e o Paragon). Só o Desperado aparece com cadeado na loja.
- **Clone:** nas duas primeiras tentativas a permissão para controlar o `BloonsBattles.exe` foi
  negada; na terceira foi liberada e o clone foi jogado por pouco tempo no modo `--demo` (seção
  3.1), sem cobrir torre por torre. As colunas "clone" vêm da leitura de
  `src/jogo/dados.cpp`, `src/jogo/sim.cpp` e `src/cliente/cena_jogo.cpp` e das capturas que já
  estavam no repositório (`docs/design/capturas/*.png` e `btd6/demo_*`, `painel_*`, `vitrine_*`).
- **Capturas:** `docs/design/capturas/btd6/real_*.jpg`. As outras imagens dessa pasta
  (`demo_*`, `vitrine_*`, `painel_*`) são do clone, de sessões anteriores.

### Como o controle foi feito (limitação que afetou a sessão)

O jogo só aceita entrada quando está em foco, e o app do Claude retoma o foco entre as chamadas.
Cada sequência começa com um clique neutro no mapa. Os cliques em botões da interface se perdiam
com frequência (o clique longo de 0,5 s funcionou melhor). O que destravou a sessão foi a tela
**Hotkeys** do jogo (captura `real_ui_hotkeys.jpg`):

- **Upgrades:** Seta Esquerda, Seta Baixo e Seta Direita (caminhos 1, 2 e 3). Os `,` `.` `/` que
  o README do clone usa não são os atalhos desta instalação.
- **Envio de bloons no Sandbox:** Ctrl+1 a Ctrl+0 (vermelho a zebra), Ctrl+- (arco-íris), Ctrl+=
  (cerâmica), Ctrl+O (MOAB), Ctrl+P (BFB), Ctrl+[ (ZOMG), Ctrl+] (DDT), Ctrl+\ (BAD).
- **Torres:** Q a L e I (Beast Handler). Mermonkey, Skywarden e Desperado não têm tecla.
- **Outros:** Backspace vende, Tab troca o alvo, 1 a 0 ativam habilidades, a crase pausa.

Cada caminho foi testado assim: vender, pôr a torre nova no mesmo ponto, subir 3 tiers,
mandar cerâmicas, capturar, subir até o tier 5, mandar cerâmicas ou MOABs e capturar. O disparo
foi visto por sequências de capturas, e não em vídeo. Projéteis rápidos (dardo simples, bala do
Sniper) quase nunca aparecem em voo.

## 2. UI do BTD6 x clone

A coluna "Clone" vem das capturas do clone já existentes e do código (ver seção 1).

| Tela / elemento | BTD6 (o que foi visto) | Clone | Diferença | Prioridade | Captura |
|---|---|---|---|---|---|
| Tela inicial | Arte grande com vários macacos, logo, botão verde START, versão no rodapé e ID do usuário | Não há tela inicial: o jogo abre direto no menu (01_menu.png) | O BTD6 tem a tela de título antes do menu. No clone é dispensável | P2 | não salva (mostra nome e ID da conta) |
| Menu principal | Vila com construções clicáveis. Nível e barra de XP no topo à esquerda, Monkey Money no topo à direita, botões laterais redondos (config, conquistas, loja, eventos) e barra inferior com 6 botões grandes com selo "New" (Monkeys, Heroes, Legends, Play, Play Social, Powers, Knowledge) | Menu com logo, painel de madeira e 4 botões (Jogar Solo, Batalha: Hospedar, Batalha: Entrar, Sair) e macacos decorando | O menu do BTD6 é um hub com meta-progressão (nível, Monkey Money, loja). O clone é um trabalho de SO e não precisa disso | - | não salva (mostra o nome da conta) |
| Seleção de mapa | Grade 3x2 de cartões com miniatura, nome, medalhas por dificuldade e selo de rodada máxima. Setas laterais e bolinhas de página. Abas por dificuldade de mapa (Beginner, Intermediate, Advanced, Expert) com ícones de dardo, e ainda busca, Friends, Change Hero e Community | Tela 'Jogo Solo' em 3 passos numa página só: 4 mapas com etiqueta de dificuldade, 7 dificuldades e 18 heróis (menu_solo_modos.png) | O BTD6 separa mapa, dificuldade e modo em telas seguidas. A página única do clone é mais rápida. Faltam só as medalhas por mapa | P2 | real_ui_selecao_mapa.jpg |
| Dificuldade | 3 cartões (Easy, Medium, Hard) com retrato do macaco, medalha e recompensa em Monkey Money | Botões Fácil, Médio, Difícil e Impossível na mesma página, com vidas e rodadas | Equivalente | - | não salva (mostra nomes) |
| Modos da dificuldade | Descrição em texto no topo, e os modos ligados por setas azuis numa árvore: Standard, depois Primary Only e Deflation, e o Sandbox ao lado. Cada modo mostra a recompensa | CHIMPS, Meio Dinheiro e Deflação ficam na mesma fileira das dificuldades. Não há Sandbox nem Primary Only | Faltam o Sandbox (ferramenta de teste: enviar qualquer bloon, dinheiro e vidas infinitos) e os modos de restrição de categoria (Primary Only, Military Only...) | P1 | real_ui_modos_facil.jpg |
| HUD da partida | Coração com vidas e moedas com dinheiro no topo à esquerda, "Round N" e engrenagem no topo à direita. Sem barra de fundo: os números ficam direto sobre o mapa, com contorno | Vidas e dinheiro dentro de uma placa de madeira no topo esquerdo. Rodada 'N/60' e dificuldade numa placa no topo da loja (demo_solo.png) | O BTD6 não usa placa: números com contorno direto sobre o mapa. O clone mostra a rodada máxima, o BTD6 só no modo normal. Escolha de estilo | P2 | real_ui_loja.jpg |
| Loja de torres | Coluna à direita. Herói no topo (cartão amarelo), depois torres em grade de 2 colunas com cor de fundo por categoria (azul Primárias, verde Militares...). Preço embaixo, torre bloqueada com cadeado, botão UPGRADES e o nome da torre em destaque | Grade de 3 colunas com cartões creme, a tecla de atalho no canto e o preço embaixo. Herói no primeiro cartão com 'Em jogo' | O BTD6 usa 2 colunas, fundo colorido por categoria (azul, verde, roxo, amarelo) e não mostra a tecla. O clone mostra mais torres sem rolagem, mas sem a cor de categoria fica mais difícil achar o grupo | P2 | real_ui_loja.jpg |
| Prévia de colocação | A torre segue o cursor com o círculo de alcance cinza translúcido. Em posição inválida o círculo fica vermelho. Aparece um X vermelho para cancelar e o cartão escolhido fica vazio (azul claro) | Círculo de alcance branco; posição inválida em vermelho com um X; sem dinheiro em âmbar com o preço (cena_jogo.cpp, desenho da prévia) | Equivalente, e o clone ainda explica o motivo. O BTD6 tem Drag & Drop ligado nas configurações | - |  |
| Painel de upgrade | Painel bege sobreposto ao mapa, à esquerda da loja. Nome da torre, contador de estouros (estrela), retrato grande que muda a cada tier, botão "i" e seletor de alvo com setas (First...). Tem 3 linhas de caminho: à esquerda o upgrade atual com 5 marcadores (verdes quando comprados) e "Owned", à direita o próximo upgrade num botão verde com ícone, nome e preço. Caminho no máximo: botão cinza "Max Upgrades". Embaixo, o valor de venda e o botão vermelho Sell | Painel verde sobre o mapa, que troca de lado conforme a torre. Nome, estouros, código do caminho (ex.: 3-2-0), 15 marcadores numa faixa, 3 cartas com o próximo upgrade (azul com preço; cinza com cadeado e o motivo, ex.: 'Caminho fechado'), seletor de alvo com Tab e botão Vender com o valor | Mesma informação. O BTD6 mostra o upgrade já comprado de cada caminho ao lado do próximo e usa retrato grande da torre. O clone mostra o motivo do bloqueio, o BTD6 não | P2 | real_ui_painel_upgrade.jpg |
| Dica de upgrade | Ao passar o mouse no botão aparece à esquerda uma caixa escura com o nome em verde e a descrição curta | Dica com nome e descrição ao passar o mouse na carta (cena_jogo.cpp, 'Dica d{mouse, up.nome, ...}') | Equivalente | - | real_ui_tooltip_upgrade.jpg |
| Selo de camo | Com detecção de camo, um selo verde aparece ao lado do retrato no painel | Não encontrado no painel | Falta indicar no painel que a torre vê camo | P2 |  |
| Barra de habilidades | Canto inferior esquerdo: botão azul de recolher e um retrato quadrado por habilidade | Canto inferior esquerdo: círculos com o retrato, a recarga em segundos no centro e a tecla (1 a 4) num selo | Equivalente. O clone mostra a recarga em número, o que o BTD6 faz com o preenchimento do ícone | - |  |
| Crítico | Texto laranja "CRIT" grande sobre o bloon atingido | O crítico existe na simulação (crit_cada), mas nenhum evento visual é emitido. Não há texto 'CRIT' | Falta o feedback visual do crítico | P1 | real_dart_0-0-5.jpg |
| Painel do Sandbox | Troca com a loja pelo botão azul do canto. Campos Set Round (com botão verde para mandar a rodada), Spacing e Count. Três botões de propriedade (camo, regen, fortificado) e a grade com todos os bloons e dirigíveis, até o BAD. Na lateral do mapa: 4 botões (recarregar habilidades, reiniciar, apagar bloons, apagar torres) e o de Powers | Não existe | Ver 'Modos da dificuldade'. O painel do Sandbox acelera muito o teste de torres e o balanceamento | P1 | real_ui_sandbox_painel_bloons.jpg |
| Tela de Upgrades (no jogo) | Tela roxa com os 15 ícones em 3 linhas, XP da torre, Paragon à direita num cartão destacado e a descrição da torre | Não existe no jogo. A vitrine (tecla de desenvolvedor) mostra os sprites dos tiers, mas não os 15 upgrades com descrição | Uma tela para consultar os 15 upgrades de cada torre fora da partida ajudaria a planejar | P2 |  |

## 3. Torres

Legenda: "Disparo no BTD6" é o que apareceu nas capturas (forma, quantidade, trajetória,
impacto e ritmo). "Disparo no clone" é a descrição do upgrade e o tipo de ataque e sprite que o
código usa (`dados.cpp`), e não uma observação na tela.

### 3.1 Conferência com o clone rodando (modo `--demo`)

O clone foi aberto com `BloonsBattles.exe --demo`, que dá $200.000 e põe 12 torres no Prado dos
Macacos. Capturas em `docs/design/capturas/clone-vs-btd6/`. O que foi visto:

- **Controles:** `,` compra o caminho 1 de primeira, Backspace vende, Q pega o Dardo e Espaço
  inicia a rodada. O teclado responde melhor que o do BTD6.
- **Painel:** mostra o código do caminho (5-0-0), os 15 marcadores e "MÁXIMO" em amarelo no
  caminho completo. Depois do tier 5, os outros dois caminhos seguem à venda (Tiros Rápidos $100,
  Dardos de Longo Alcance $90), como no BTD6.
- **Macaco gira para o alvo:** o Dardo vira de lado quando os bloons entram pela esquerda.
- **Disparos vistos em voo:** flechas do Quincy (haste cinza, ponta vermelha), estrepes do Ninja
  no chão, bumerangue amarelo, chamas laranja do Mago e o tiro do Sniper como uma linha amarela
  com clarão no cano (parecido com o BTD6).
- **Ultra-Juggernaut:** a bola em voo e a divisão em 6 não apareceram nas capturas. As rodadas 2
  a 4 têm poucos bloons e as outras torres matam antes. Não observado.
- **Tier 5 mantém a silhueta:** o Dardo 5-0-0 é o mesmo macaco, com capacete e uma bola de
  espinhos na mão. No BTD6 ele vira uma catapulta. A vitrine (`11_vitrine_tiers_1.png` e
  `12_vitrine_tiers_2.png`) mostra o mesmo padrão nas outras torres: o Canhão continua um canhão
  preto nos três tier 5 (no BTD6: caveira azul, lançador tubarão, canhão azul e dourado), o
  Verdadeiro Deus Sol é um macaco dourado com asas (no BTD6: templo com estátua gigante) e o
  Morteiro é o mesmo disco. Virou o item B27.
- **F9 (disparo em loop):** anima só o braço da torre, sem soltar projétil. Serve para conferir a
  pose, não o projétil.
- **Sem Sandbox, testar é lento:** para ver um tier 5 atirando é preciso jogar dezenas de rodadas
  ou usar o `--demo`. Reforça o item B04.
- **AUTO:** o botão de rodada automática não ligou com o clique enviado pela automação. Pode ser
  só o clique; vale testar à mão.

### 3.2 Dois erros apontados pelo dono e confirmados no código

**Projéteis erram alvos distantes (B28, P0).** Em `src/jogo/sim.cpp`, o disparo calcula o ângulo
para a posição atual do bloon (`atan2(a->y - t.y, a->x - t.x)`) e o projétil segue reto, sem
antecipar o movimento. Como os bloons do clone são rápidos em relação aos projéteis (vermelho a
95 px/s, dardo a 900 px/s, dardo do Bucaneiro a 700 px/s), o bloon sai do lugar antes de o
projétil chegar. Conta feita com os números do código, para um bloon cruzando a linha de tiro:

| Bloon | Bucaneiro (700 px/s, alcance 240) acerta até | Dardo (900 px/s) acerta até |
|---|---|---|
| Vermelho | 120 px | 150 px |
| Azul | 80 px | 120 px |
| Verde | 70 px | 90 px |
| Cerâmica | 50 px | 70 px |
| Amarelo e rosa | 30 px | 40 px |

Ou seja, o Bucaneiro erra todo tiro em bloon vermelho na metade de fora do próprio alcance, e
quase todo tiro em amarelo ou rosa. Vale para todo ataque do tipo projétil sem `busca` (Dardo,
Bucaneiro, Ninja, Engenheiro, Super, Druida). Quando o bloon anda na direção do tiro o projétil
acerta, por isso o erro depende da posição da torre. No BTD6 as torres também miram onde o bloon
está, mas os projéteis são muito mais rápidos em relação aos bloons e o erro quase não aparece.

**Bumerangue sem troca de mão (B29, P1).** Em `Pista::mover_bumerangue` a curva é sempre para o
mesmo lado (`ang + 2.2 * DT`, comentário "curva suave para a direita"). No BTD6 o painel do
Boomerang tem um botão de mão (o ícone branco de mão ao lado do retrato, visível em
`real_ui_painel_lado_esquerdo.jpg`) que inverte o lado do arco. Na primeira versão deste
documento o ícone foi descrito só como "selo branco", sem identificar a função.

### Dart Monkey (Macaco Dardo)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Dart Monkey | Macaco Dardo | 1 dardo com pena vermelha e amarela, reto e rápido (não aparece em voo nas capturas). Fica cravado na cerâmica ao acertar. O macaco gira o corpo inteiro para o alvo e arremessa com o braço. Ritmo médio | projétil, sprite 'dardo' | Sem diferença relevante no código | - |
| 1-0-0 / 2-0-0 | Sharp Shots, Razor Sharp Shots | Tiros Afiados / Tiros Super Afiados | Sem mudança visual no mapa. O retrato muda (dardo mais afiado) | Dardos estouram +2 bloons | Sem diferença relevante no código | - |
| 3-0-0 | Spike-o-pult | Espinhopulta | O modelo vira um macaco numa catapulta. Lança uma bola de espinhos grande e mais lenta. Alcance maior | Bolas de espinhos: 2 de dano, pierce 18, quicam em obstáculos (sprite 'bola_espinho') | Sem diferença relevante no código | - |
| 4-0-0 | Juggernaut | Juggernaut | Bola de espinhos maior (não observada em voo) | Bola gigante: pierce 60, estoura chumbo, +3 em cerâmica e +2 em fortificado (sprite 'juggernaut') | Sem diferença relevante no código | - |
| 5-0-0 | Ultra-Juggernaut | Ultra-Juggernaut | Catapulta preta com detalhes laranja. Bola preta grande com anel laranja brilhante que, ao acertar, se divide em 6 bolas menores (pretas com anel amarelo e rastro laranja) que se espalham em estrela e ricocheteiam pela pista. Ritmo lento | 5 de dano, pierce 210; se divide em 6 juggernauts (sprite 'juggernaut') | Mesma divisão em 6 bolas quando a bola principal acaba (`Pista::fim_projetil`). O clone não faz as bolas quicarem nos obstáculos, apesar da descrição do Spike-o-pult dizer isso | P2 |
| 0-1-0 / 0-2-0 | Quick Shots, Very Quick Shots | Tiros Rápidos / Tiros Muito Rápidos | Bandana verde no retrato. No mapa, só atira mais rápido | Atira ainda mais rápido | Sem diferença relevante no código | - |
| 0-3-0 | Triple Shot | Tiro Triplo | Bandana vermelha. 3 dardos num leque estreito (de 20 a 30 graus), com pena vermelha | Atira 3 dardos por vez, mais rápido | Sem diferença relevante no código | - |
| 0-4-0 | Super Monkey Fan Club | Fã-Clube Super Macaco | Mesmo tiro. Ganha a habilidade: o Dart (e até 10 Darts próximos) vira Super Monkey por 15 s, com roupa azul, capa vermelha, máscara e um brilho verde no chão | Atira 2x mais rápido. Habilidade: até 10 dardos viram Super Macacos por 15 s (habilidade turbo_area) | O clone só acelera os Darts (turbo em área) e não troca o visual para Super Monkey (pendência) | P1 |
| 0-5-0 | Plasma Monkey Fan Club | Fã-Clube Macaco Plasma | Não comprado: o clique falhou pelo problema de entrada. Não observado | Habilidade: até 20 dardos viram Macacos Plasma por 15 s (habilidade turbo_area) | Idem: turbo em área, sem o visual de Plasma Monkey (pendência) | P1 |
| 0-0-1 / 0-0-2 | Long Range Darts, Enhanced Eyesight | Dardos de Longo Alcance / Visão Aprimorada | Só alcance maior (o círculo cresce). O tier 2 dá camo | +8 de alcance e detecta camo | Sem diferença relevante no código | - |
| 0-0-3 | Crossbow | Besta | Macaco de roupa roxa com uma besta | Besta: 3 de dano, pierce 4, alcance 60 (sprite 'flecha') | Sem diferença relevante no código | - |
| 0-0-4 | Sharp Shooter | Atirador Afiado | Capuz roxo e besta maior. Críticos periódicos | 6 de dano, 2x mais rápido e crítico de 50 a cada 10 tiros | O clone calcula o crítico, mas não mostra o texto CRIT | P1 |
| 0-0-5 | Crossbow Master | Mestre da Besta | Capuz preto e besta grande dourada e preta. Virote como um traço fino e claro, quase instantâneo, com estilhaços brancos no impacto e "CRIT" laranja nos críticos. Alcance muito maior. Ritmo rápido | 8 de dano, pierce 8, alcance 80, 2x mais rápido, estoura tudo; crítico de 80 a cada 5 tiros | Crítico sem o texto CRIT na tela | P1 |

Habilidades: Super Monkey Fan Club (0-4-0), vista acima. Plasma Monkey Fan Club: não observada.

Cruzamentos testados: nenhum.

### Boomerang Monkey (Macaco Bumerangue)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Boomerang Monkey | Macaco Bumerangue | Macaco com faixa vermelha e amarela. O bumerangue sai em arco largo e volta (não visto em voo no tier 0) | projétil, sprite 'bumerangue' | Sem diferença relevante no código | - |
| 2-0-0 | Glaives | Glaives | O retrato troca o bumerangue por uma glaive (lâmina de aço com centro azul) | Glaives: pierce 13 (sprite 'glaive') | Sem diferença relevante no código | - |
| 4-0-0 | M.O.A.B. Glaives | M.O.A.R. Glaives | Capacete cinza e glaives amarelas | 3x mais rápido, pierce 60 | Sem diferença relevante no código | - |
| 5-0-0 | Glaive Lord | Senhor das Glaives | Capuz roxo. 3 glaives orbitam o macaco o tempo todo, com rastro roxo curvo, e rasgam tudo que passa no raio curto. Os bloons somem na entrada antes de aparecerem na captura | Glaives orbitam o macaco: 4 de dano a cada 0,05 s em até 200 bloons (ataque aura em volta; sprite 'orbita_glaive') | Sem diferença relevante no código | - |
| 0-3-0 | Bionic Boomerang | Bumerangue Biônico | Braço robótico cinza com luz verde. Bumerangue marrom rápido em arco, com rastro branco curto | Ataca 4x mais rápido, +1 em M.O.A.B | Sem diferença relevante no código | - |
| 0-4-0 | Turbo Charge | Turbo Carga | Habilidade que acelera por alguns segundos (não observada) | Habilidade: 5x mais rápido por 10 s (habilidade turbo) | Sem diferença relevante no código | - |
| 0-5-0 | Perma Charge | Carga Permanente | Braço robótico enorme com bobinas verdes brilhantes. Os bumerangues saem verdes, com várias imagens fantasma (borrão de velocidade) | Turbo permanente, 4 de dano | O clone só aumenta a cadência; falta o rastro verde de velocidade | P2 |
| 0-0-3 | Kylie Boomerang | Bumerangue Kylie | Chapéu de explorador. Cerâmicas atingidas ficam com marcas alaranjadas de calor (do Red Hot Rangs, 0-0-2) | Kylie em linha reta, pierce 18 (sprite 'kylie') | Confirmado pelo código: o Bumerangues Incandescentes (0-0-2) só muda dano e tipo de dano, sem visual de calor no bumerangue nem nos bloons | P2 |
| 0-0-5 | MOAB Domination | Dominação M.O.A.B. | Casaco listrado de tigre e bumerangue em chamas (laranja e vermelho). O acerto em MOAB solta lascas brancas e azuis | Kylie com 12 de dano, pierce 54 e 2x mais rápido | O Kylie do clone não pega fogo (não há visual de chama) | P2 |

Habilidades: Turbo Charge (0-4-0), não ativada.

### Bomb Shooter (Canhão)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Bomb Shooter | Canhão Bomba | Canhão preto sobre rodas que gira para o alvo | projétil, sprite 'bomba' | Sem diferença relevante no código | - |
| 3-0-0 | Really Big Bombs | Bombas Muito Grandes | Canhão vermelho de boca larga. Explosão em nuvem de fumaça cinza-escura (várias bolas) com lascas marrons da cerâmica | 4 de dano, pierce 80, empurra bloons | Sem diferença relevante no código | - |
| 5-0-0 | Bloon Crush | Esmaga Bloon | Canhão azul-marinho com borda vermelha, caveira vermelha e espetos. Explosão como um clarão amarelo em estrela com lascas escuras | 24 de dano, estoura tudo e atordoa M.O.A.B.s por 2 s | Usa a explosão padrão; o BTD6 tem clarão amarelo em estrela | P2 |
| 0-3-0 | MOAB Mauler | Destruidor de M.O.A.B. | Vira um lançador de mísseis amarelo com cara de tubarão, sobre base listrada de preto e amarelo. Explosão pequena e amarela | +15 de dano em M.O.A.B | Sem diferença relevante no código | - |
| 0-5-0 | MOAB Eliminator | Eliminador de M.O.A.B. | Lançador preto e verde com cara de tubarão verde e boca roxa brilhante. Míssil com traço laranja | +99 em M.O.A.B. Habilidade: 4.500 a cada 10 s (habilidade dano_forte) | Sem diferença relevante no código | - |
| 0-0-3 | Cluster Bombs | Bombas de Cacho | Canhão verde com boca amarela (as bombinhas não foram pegas na captura) | Soltam mini bombas (sprite 'bomba') | Sem diferença relevante no código | - |
| 0-0-5 | Bomb Blitz | Blitz de Bombas | Canhão azul-marinho com faixas douradas e base verde. Cada tiro espalha dezenas de bombinhas pretas com faixa amarela num leque largo sobre a pista, com fumaça | +3 de dano e ataca mais rápido (habilidade dano_global) | Confirmado pelo código: o clone solta 8 mini bombas por tiro, e as mini bombas não se dividem de novo (`frag_filho` barra a recursão), apesar do nome "Cacho Recursivo". O BTD6 mostra dezenas | P2 |

Habilidades: MOAB Eliminator (0-5-0) ativada sem dirigível no alcance, sem efeito visível.

### Tack Shooter (Atirador de Tachinhas)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Tack Shooter | Atirador de Tachinhas | Torre sem macaco, redonda, que não gira. Tachinhas em raios | radial (vários em círculo), 8 por vez, sprite 'tachinha' | Sem diferença relevante no código | - |
| 3-0-0 | Hot Shots | Tiros Quentes | Corpo amarelo e laranja com emblema de chama. 8 tachinhas de ponta vermelha saem ao mesmo tempo em 8 direções fixas, retas, até a borda do alcance | Tachinhas quentes: 3 de dano, estouram chumbo (sprite 'fogo') | Sem diferença relevante no código | - |
| 5-0-0 | Inferno Ring | Anel Infernal | Torre vermelha e laranja com chama viva no topo e blindagem cinza. Anéis de fogo translúcidos (borda laranja e amarela) se expandem do centro até o alcance, sem parar | Anel mais forte e meteoros de 700 de dano (ataque projétil; sprite 'meteoro') | Equivalente: o efeito de aura do clone desenha um anel que cresce de 30% a 100% do raio e some (`render.cpp`, efeito "aura") | - |
| 0-3-0 | Blade Shooter | Atirador de Lâminas | Torre azul com estrela de lâminas preta. Lâminas em raios | Lâminas: pierce 8 e alcance 46 (sprite 'lamina') | Sem diferença relevante no código | - |
| 0-5-0 | Super Maelstrom | Super Turbilhão | Topo com espiral azul e anel de dentes de serra. A habilidade lança dezenas de serras prateadas de centro verde numa espiral que cobre boa parte do mapa | 5 de dano, +5 em cerâmica, estoura tudo. Habilidade de 9 s (habilidade turbo) | A habilidade do clone é um turbo de 9 s; o BTD6 mostra a espiral de serras cobrindo o mapa | P1 |
| 0-0-3 | Tack Sprayer | Pulverizador de Tachinhas | Torre vermelha com X preto. Anel denso de tachinhas pretas pequenas | 16 tachinhas, +1 pierce | Sem diferença relevante no código | - |
| 0-0-5 | The Tack Zone | Zona das Tachinhas | Torre preta blindada com caveira branca de olhos vermelhos e vários canos de ponta vermelha. Anel contínuo de tachinhas na borda do alcance | 32 tachinhas, mais rápido e mais alcance | Sem diferença relevante no código | - |

Habilidades: Blade Maelstrom e Super Maelstrom (espiral de serras), vistas. Overdrive (0-0-4): não observado.

### Ice Monkey (Macaco de Gelo)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Ice Monkey | Macaco de Gelo | Macaco branco e azul, peludo. Pulso de gelo em área em volta de si, sem projétil | aura em volta, sprite 'congelar' | Sem diferença relevante no código | - |
| 3-0-0 | Ice Shards | Estilhaços de Gelo | Coroa de estilhaços de gelo | Bloons congelados soltam 3 estilhaços ao estourar (sprite 'fragmento_gelo') | Sem diferença relevante no código | - |
| 5-0-0 | Super Brittle | Super Frágil | Armadura roxa, coroa e punhos de gelo. Na habilidade, os bloons ficam cobertos de gelo branco com faixas roxas | Bloons recebem +4 de dano de tudo; ataca mais rápido | O clone aplica o dano extra sem o visual roxo de bloon quebradiço | P2 |
| 0-3-0 | Arctic Wind | Vento Ártico | Macaco peludo branco. Aura permanente de vento com riscos de neve girando no raio | Aura que desacelera bloons em 40% (ataque aura em volta; sprite 'vento') | Sem diferença relevante no código | - |
| 0-5-0 | Absolute Zero | Zero Absoluto | Corpo de cristal de gelo azul brilhante e tempestade de neve em volta. A habilidade congela todos os bloons do mapa, que ficam com uma camada de gelo e neve no topo | +10 de alcance e pierce 300 (habilidade congelar_global) | Sem diferença relevante no código | - |
| 0-0-3 | Cryo Cannon | Canhão Criogênico | Óculos e canhão de gelo no ombro. Tiro que congela: as cerâmicas atingidas ficam brancas com brilho azul | Canhão criogênico: 2x mais rápido, alcance 46 (ataque projétil; sprite 'gelo_bola') | Sem diferença relevante no código | - |
| 0-0-5 | Icicle Impale | Empalar com Pingentes | Canhão enorme de cristal de gelo. O disparo em MOAB não foi pego na captura | 50 de dano em M.O.A.B. e congela dirigíveis | Sem diferença relevante no código | - |

Habilidades: Snowstorm e Absolute Zero (congela o mapa), Super Brittle, vistas.

Estado "congelado" no BTD6: o bloon continua visível, com uma capa branca de gelo por cima. Não
fica azul sólido.

### Glue Gunner (Atirador de Cola)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Glue Gunner | Atirador de Cola | Macaco com pistola de cola amarela. Gota de cola que respinga. O bloon colado fica coberto de cola amarela escorrendo | projétil, sprite 'cola' | Sem diferença relevante no código | - |
| 3-0-0 | Bloon Dissolver | Dissolvedor de Bloons | Capacete azul com máscara de gás e tanque verde. Cola verde corrosiva | Corrosão a cada 0,5 s; atira 2x mais rápido | Sem diferença relevante no código | - |
| 5-0-0 | Bloon Solver | Solucionador de Bloons | Roupa de proteção verde e pistola com cola verde brilhante | 2 globos por tiro, pierce 4, 2x mais rápido | Sem diferença relevante no código | - |
| 0-3-0 | Glue Hose | Mangueira de Cola | Óculos amarelos e tanque escuro. Jatos rápidos: vários bloons ficam amarelos, com respingos voando | 3x mais rápido e +12 de alcance | Sem diferença relevante no código | - |
| 0-5-0 | Glue Storm | Tempestade de Cola | Macaco laranja sobre base escura com vários canos, como um polvo. Bolhas grandes de cola amarela sobre a fila. A habilidade cobre o mapa de cola | Habilidade: cola todos os bloons por 20 s (habilidade lentidao) | A habilidade do clone só desacelera; o BTD6 cobre de cola os bloons do mapa | P2 |
| 0-0-3 | MOAB Glue | Cola de M.O.A.B. | Capacete rosa com óculos pretos e pistola rosa | Cola dirigíveis | Sem diferença relevante no código | - |
| 0-0-5 | Super Glue | Super Cola | Armadura preta. Cola rosa: os bloons ficam cobertos de rosa, com estrelinhas brancas (atordoados, parados) | Para bloons comuns e desacelera muito os dirigíveis | Cola de cor única no clone; o BTD6 muda a cor (verde corrosiva, rosa na Super Glue) e mostra estrelas de atordoamento | P2 |

Estado "colado": a cor da cola depende do upgrade (amarela, verde ou rosa) e cobre o bloon por
cima, escorrendo.

### Sniper Monkey (Atirador de Elite)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Sniper Monkey | Macaco Atirador | Macaco deitado com rifle longo. Alcance no mapa inteiro, o círculo só marca a posição | tiro instantâneo, sprite 'bala' | Sem diferença relevante no código | - |
| 3-0-0 | Deadly Precision | Precisão Mortal | Boina preta e rifle com faixas vermelhas. Tiro instantâneo: um risco de luz branco e amarelo sai do cano, sem projétil em voo | 20 de dano, +50 em cerâmica | Sem diferença relevante no código | - |
| 5-0-0 | Cripple MOAB | Aleijar M.O.A.B. | Roupa de camuflagem de folhas (ghillie) e rifle grande. O MOAB atingido fica com estrelas brancas de atordoamento | 140 de dano, explosão de 28 e +5 de dano em dirigíveis atingidos | Equivalente: o clone desenha o estado atordoado em todo bloon com `atord_t > 0`, inclusive dirigíveis (`render.cpp`, `arte::estado_bloon`) | - |
| 0-5-0 | Elite Sniper | Atirador de Elite | Armadura preta com lentes verdes. Novo modo de alvo "Elite" no painel. O acerto solta estilhaços brancos. Habilidade Supply Drop (caixa que cai e dá dinheiro) | Muito mais rápido; os outros Snipers do mapa atacam 33% mais rápido. Habilidade: caixa de $3.000 (habilidade dinheiro) | Sem o modo de alvo Elite; a caixa de suprimentos é dinheiro direto, sem a caixa caindo no mapa | P1 |
| 0-0-5 | Elite Defender | Defensor de Elite | Capacete preto e rifle com detalhes laranja. Tiro muito rápido, com clarão laranja e amarelo no cano | 2x mais rápido, +4 em M.O.A.B | Sem diferença relevante no código | - |

### Monkey Ace (Ás)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Monkey Ace | Macaco Ás | A torre é uma pista de pouso amarela e preta no chão. O avião (biplano verde) voa sozinho pelo mapa numa rota escolhida no painel (Circle, Infinite, Figure Eight...) e solta 8 dardos em raio | radial (vários em círculo), 8 por vez, sprite 'dardo', movimento ORBITA | O clone voa em círculo fixo (Mov::ORBITA). Falta a pista de pouso como torre e a escolha de rota no painel (Circle, Infinite, Figure Eight) | P1 |
| 3-0-0 | Fighter Plane | Avião de Caça | Caça vermelho e azul, dardos pretos em raio | Mísseis anti M.O.A.B. (18 de dano) (ataque projétil; sprite 'missil') | Sem diferença relevante no código | - |
| 5-0-0 | Sky Shredder | Retalhador Celeste | Jato furtivo preto com faixas vermelhas. Dezenas de dardos pretos em todas as direções, que cobrem boa parte do mapa | 32 dardos com 3 de dano, 2x mais rápido | Sem diferença relevante no código | - |
| 0-3-0 | Bomber Ace | Ás Bombardeiro | Avião cinza-esverdeado com bombas vermelhas | Bombardeio na trilha: bombas de 3 de dano | Sem diferença relevante no código | - |
| 0-5-0 | Tsar Bomba | Tsar Bomba | Bombardeiro pesado cinza com insígnias azul e amarela. A habilidade chama um avião que passa sobre o mapa | Habilidade: bomba de 3.000 e atordoa 8 s (habilidade dano_global) | Tsar Bomba é dano global; falta o avião cruzando o mapa | P2 |
| 0-0-3 | Neva-Miss Targeting | Mira Infalível | Avião vermelho. Libera a rota "Centered Path", que mostra uma mira amarela arrastável no mapa. Dardos teleguiados: pontos amarelos brilhantes que fazem curva até o bloon | Dardos teleguiados | Rota Centralizada sem efeito (pendência); não existe a mira amarela arrastável | P1 |
| 0-0-5 | Flying Fortress | Fortaleza Voadora | Bombardeiro preto enorme de quatro motores, insígnias vermelhas e brancas, mísseis de ponta vermelha e muitos dardos teleguiados | 3 projéteis por disparo, 50% mais rápido, +14 em M.O.A.B | Sem diferença relevante no código | - |

O avião é grande: o sprite cobre duas faixas da pista.

### Heli Pilot (Piloto de Helicóptero)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Heli Pilot | Piloto de Helicóptero | A torre é um heliponto azul com H vermelho no chão. O helicóptero voa e persegue os bloons. Modos de alvo próprios: Pursuit, Follow Mouse, Lock in Place, Patrol Points | projétil, 2 por vez, sprite 'dardo', movimento HELI | O clone persegue (Mov::HELI), mas não tem os modos Follow Mouse, Lock in Place e Patrol Points | P1 |
| 5-0-0 | Apache Prime | Apache Prime | Helicóptero de ataque preto e roxo. Rajadas de plasma amarelo | Lasers: 6 de dano e pierce 23 (sprite 'plasma') | Sem diferença relevante no código | - |
| 0-5-0 | Special Poperations | Operações Especiais | Helicóptero preto com luzes azuis. Várias habilidades (soltar fuzileiro, empurrar MOAB) que aparecem numa grade 2x2 flutuante no canto do mapa, além da barra inferior | Habilidade: fuzileiro de elite (habilidade invocar) | Invoca fuzileiro; falta a grade 2x2 de habilidades extra | P2 |
| 0-0-5 | Comanche Commander | Comandante Comanche | Helicóptero bege com dois mini-helicópteros escoltando. Foguetes vermelhos com rastro de fumaça marrom | Dardos com 4 de dano | Mini-Comanches não existem no clone (pendência) | P2 |

### Mortar Monkey (Morteiro)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Mortar Monkey | Macaco Morteiro | Tubo de morteiro. Botão verde "Set Target" no painel escolhe o ponto; sem escolher, mira perto da entrada. Explosão: nuvem branca com clarão laranja e lascas escuras caindo | morteiro, sprite 'bala_canhao' | O clone mira no bloon com imprecisão. Falta o botão Set Target para fixar o ponto de impacto (pendência) | P0 |
| 5-0-0 | The Biggest One | A Maior de Todas | Morteiro preto com cano verde brilhante. Explosão muito maior | 25 de dano, +30 em cerâmica e M.O.A.B., pierce 200 | Sem diferença relevante no código | - |
| 0-5-0 | Pop and Awe | Choque e Pavor | Bateria de três canos dourados e vermelhos. Os projéteis sobem em arco como bolas escuras (visíveis no ar) | Habilidade: atordoa tudo e causa 20 de dano por segundo por 8 s (habilidade dano_global) | Pop and Awe aproximado em 160 de dano (pendência) | P2 |
| 0-0-5 | Bloon-cineration | Bloonflagração | Morteiro vermelho e dourado. Os bloons atingidos pegam fogo: chamas laranja e azuladas por cima deles | Fogo intenso | Sem diferença relevante no código | - |

### Dartling Gunner (Metralhadora)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Dartling Gunner | Atirador Dartling | Metralhadora giratória que aponta para o cursor do mouse (modo de alvo "Normal") | projétil, sprite 'dardo' | O clone mira no bloon; no BTD6 a Dartling aponta para o cursor (pendência) | P0 |
| 5-0-0 | Ray of Doom | Raio da Perdição | Canhão pesado vermelho e dourado. Raio laser vermelho grosso e contínuo que atravessa o mapa inteiro na direção do cursor | Raio de 30 de dano, pierce 999 | Confirmado pelo código: cada tiro instantâneo vira uma linha de 6 px da torre até o alvo que dura 0,09 s, disparada a cada 0,2 s. Fica um raio piscando até o bloon, e não um feixe contínuo que atravessa o mapa | P1 |
| 0-5-0 | M.A.D | M.A.D. | Lançador duplo roxo e verde. Míssil verde com rastro verde | Mega mísseis: +450 em M.O.A.B | Sem diferença relevante no código | - |
| 0-0-5 | Bloon Exclusion Zone | Zona de Exclusão Bloon | Torre cilíndrica azul e dourada. Rajada de bolinhas escuras | 6 canos: 12 projéteis de 6 de dano | Sem diferença relevante no código | - |

### Wizard Monkey (Mago)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Wizard Monkey | Macaco Mago | Macaco com chapéu pontudo. Raio mágico roxo | projétil, sprite 'magia' | Sem diferença relevante no código | - |
| 5-0-0 | Archmage | Arquimago | Túnica e chapéu brancos, cajado com orbe azul | Raio de 8 de dano, 2x mais rápido; ganha sopro de fogo e cintilar (ataque aura em volta/projétil; sprite 'fogo') | Sem diferença relevante no código | - |
| 0-3-0 | Dragon's Breath | Sopro do Dragão | Túnica vermelha com emblema de chama. Jato de fogo contínuo em cone curto, laranja e amarelo, com risco vermelho | Lança-chamas: 2 de dano a cada 0,135 s (ataque projétil; sprite 'fogo') | Sem diferença relevante no código | - |
| 0-5-0 | Wizard Lord Phoenix | Lorde Fênix | Visual vermelho e laranja com asas. Fogo em rajadas | Fênix permanente (ataque projétil; sprite 'fogo') | Sem diferença relevante no código | - |
| 0-0-5 | Prince of Darkness | Príncipe das Trevas | Capuz roxo escuro e orbe verde. O painel ganha um contador de lápide (cemitério de bloons) e dois botões de alvo extras. Os bloons revividos andam para trás na pista, brancos e fantasmagóricos, com rastro roxo | 4x mais rápido e mais alcance; zumbis mais fortes | Os zumbis saem como pilha na trilha; faltam o cemitério e o contador de lápide no painel (pendência) | P1 |

### Super Monkey (Super Macaco)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Super Monkey | Super Macaco | Roupa azul, capa vermelha e bandana. Dardos muito rápidos | projétil, sprite 'dardo' | Sem diferença relevante no código | - |
| 3-0-0 | Sun Avatar | Avatar do Sol | Estátua dourada brilhante | 3 raios de sol por tiro, +4 pierce (sprite 'sol') | Sem diferença relevante no código | - |
| 4-0-0 | Sun Temple | Templo do Sol | Ao comprar, abre o diálogo "Temple Sacrifice" (Cancel / Do It) avisando que destrói as torres próximas. Vira um templo dourado grande | Raio de sol único: 5 de dano, pierce 20 | Sem o diálogo de sacrifício e sem o efeito do sacrifício (pendência) | P1 |
| 5-0-0 | True Sun God | Verdadeiro Deus Sol | Outro diálogo de confirmação ("Do you wish to summon the TRUE SUN GOD?"). Estátua dourada gigante de um deus sentado sobre o templo | Raio de 15 de dano | Sem o segundo diálogo e sem o bônus do sacrifício (pendência) | P1 |
| 0-3-0 | Robo Monkey | Robô Macaco | Robô cinza com dois canhões roxos nos braços. Dois fluxos de tiro, traços amarelos, "CRIT" frequente | Atira com os 2 braços; crítico de +9 de dano a cada 15 a 20 tiros | Crítico sem o texto CRIT na tela | P1 |
| 0-5-0 | The Anti-Bloon | O Anti-Bloon | Robô preto com canhões vermelhos. O painel tem dois seletores de alvo, um por braço (ex.: "Strong" e "First") | 5 de dano, pierce 12. Habilidade de 10.400 (habilidade dano_global) | Um seletor de alvo só; o Anti-Bloon do BTD6 tem um por braço | P2 |
| 0-0-3 | Dark Knight | Cavaleiro das Trevas | Cavaleiro de capa escura. Lâminas escuras em forma de morcego | Lâminas: pierce 4, +2 em M.O.A.B (sprite 'escuro') | Sem diferença relevante no código | - |
| 0-0-5 | Legend of the Night | Lenda da Noite | Máscara preta com asas, morcegos voando pelo mapa | Buraco negro que evita vazamentos por 8 s | Legend of the Night sem efeito no clone (pendência) | P1 |

### Ninja Monkey

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Ninja Monkey | Macaco Ninja | Roupa vermelha. Shuriken cinza de quatro pontas, que gira | projétil, sprite 'shuriken' | Sem diferença relevante no código | - |
| 5-0-0 | Grandmaster Ninja | Grão-Mestre Ninja | Chapéu de palha preto e capa vermelha. Muitas shurikens por vez | 8 shurikens de 2 de dano, 2x mais rápido | Sem diferença relevante no código | - |
| 0-5-0 | Grand Saboteur | Grande Sabotador | Roupa preta com lançador vermelho. Contador "x1" flutuando sobre a torre | Sabotagem de 30 s (habilidade lentidao) | A sabotagem só desacelera; falta o contador sobre a torre | P2 |
| 0-0-5 | Master Bomber | Mestre Bombardeiro | Capacete escuro com chifres. Shurikens e bombas grudentas | Bombas grudentas de 3.000, 2x mais frequentes e sem limite de alcance | Sem diferença relevante no código | - |

### Alchemist

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Alchemist | Alquimista | Macaco ruivo de óculos. Frasco jogado que quebra com respingo de ácido rosa | projétil, sprite 'pocao' | Sem diferença relevante no código | - |
| 5-0-0 | Permanent Brew | Poção Permanente | Barril grande nas costas. Buffs de poção nas torres próximas | As poções novas (estimulante e ácido) ficam permanentes | Sem diferença relevante no código | - |
| 0-5-0 | Total Transformation | Transformação Total | Vira um monstro azul e roxo peludo (habilidade) | (sem descrição) (habilidade turbo_area) | Total Transformation virou turbo em área, sem a forma de monstro (pendência) | P1 |
| 0-0-5 | Bloon Master Alchemist | Mestre Alquimista | Visual vermelho e dourado. A poção vermelha acerta o MOAB, espirra líquido vermelho e encolhe o dirigível até virar bloon vermelho | Poção que encolhe bloons em vermelhos | Shrink Potion sem efeito (pendência) | P1 |

### Druid

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Druid | Druida | Macaco claro com folhas. Espinhos em leque | projétil, 5 por vez, sprite 'espinho' | Sem diferença relevante no código | - |
| 5-0-0 | Monarch of Storms | Monarca das Tempestades | Capuz branco e nuvem de tempestade. Raios roxos | Relâmpagos de 30 e 10 de dano; supertempestade de 100 (ataque projétil; sprite 'tornado') | Tornado e supertempestade com números aproximados (pendência) | P2 |
| 0-5-0 | Spirit of the Forest | Espírito da Floresta | Druida em forma de árvore verde brilhante com chifres. Cipós espinhosos azul-esverdeados crescem sobre a pista e ferem o que passa | Cipós grossos na trilha | Sem diferença relevante no código | - |
| 0-0-5 | Avatar of Wrath | Avatar da Ira | Fera vermelha e preta, raivosa. Espinhos vermelhos | +3 de dano e 2x mais rápido | Sem diferença relevante no código | - |

### Monkey Sub (Submarino), no mapa In The Loop

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Monkey Sub | Submarino Macaco | Só na água. Submarino pequeno com dardos teleguiados | projétil, sprite 'dardo' | Sem diferença relevante no código | - |
| 5-0-0 | Energizer | Energizador | Submarino preto com brilho verde. Botão próprio "Submerge" no painel: submerso, mostra um radar verde com cone de varredura girando | Dardos com 5 de dano | Submergir é uma aura permanente no clone; faltam o botão Submerge e o radar verde | P2 |
| 0-5-0 | Pre-emptive Strike | Ataque Preventivo | Submarino de batalha vermelho. Mísseis | Míssil de 750 em cada dirigível; habilidade mais rápida (ataque projétil; sprite 'missil'; habilidade dano_forte) | Sem diferença relevante no código | - |
| 0-0-5 | Sub Commander | Comandante Submarino | Submarino branco e azul com macaco capitão | Dobra o dano e dá +4 pierce | Sem diferença relevante no código | - |

### Monkey Buccaneer (Navio), no mapa In The Loop

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Monkey Buccaneer | Macaco Bucaneiro | Só na água. Navio que atira dardos para os dois lados | projétil, 2 por vez, sprite 'dardo' | Sem diferença relevante no código | - |
| 5-0-0 | Carrier Flagship | Nau Capitânia | Porta-aviões cinza com pista. Aviõezinhos pretos e vermelhos voam em volta soltando dardos | Torres na água e Ases em todo o mapa atacam 25% mais rápido | Confirmado pelo código: não há colocação de torres sobre o navio | P2 |
| 0-5-0 | Pirate Lord | Senhor Pirata | Navio pirata roxo-escuro com detalhes verdes. Balas de canhão pretas, explosão amarela em estrela, ganchos para puxar dirigíveis | 35% mais rápido; uvas com 8 de dano (habilidade dano_forte) | Sem diferença relevante no código | - |
| 0-0-5 | Trade Empire | Império Comercial | Cargueiro vermelho com contêineres coloridos. Contador "x1" sobre o navio | Gera $800 por rodada | Sem diferença relevante no código | - |

### Banana Farm (Fazenda)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Banana Farm | Fazenda de Bananas | Bananeira num caixote. Sem seletor de alvo no painel. As bananas caem no chão perto da fazenda e são coletadas | renda por rodada, sprite 'banana' | O clone paga a renda direto; no BTD6 as bananas caem no chão e são coletadas (pendência) | P1 |
| 5-0-0 | Banana Central | Central de Bananas | Prédio tecnológico azul e branco com cúpula cheia de bananas | 5 caixas de $1.200 ($7.000 por rodada) | Sem diferença relevante no código | - |
| 0-5-0 | Monkey-Nomics | Macaconomia | Prédio de banco amarelo e cinza. Habilidade de dinheiro | Habilidade: $9.000 sem dívida (habilidade dinheiro) | Banco sem juros reais nem saque (pendência) | P2 |
| 0-0-5 | Monkey Wall Street | Wall Street dos Macacos | Templo de mármore branco com palmeiras e moedas | +$4.000 no fim de cada rodada | Faltam as +15 vidas do Wall Street (pendência) | P2 |

### Spike Factory (Fábrica de Espinhos)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Spike Factory | Fábrica de Espinhos | Cúpula branca e amarela. Joga montes de espinhos pretos na pista, dentro do alcance | pilha na trilha, sprite 'espinhos' | Sem diferença relevante no código | - |
| 5-0-0 | Super Mines | Super Minas | Fábrica preta e vermelha com caveira. Minas vermelhas espinhosas | Super minas de 50 de dano | Sem diferença relevante no código | - |
| 0-5-0 | Carpet of Spikes | Tapete de Espinhos | Fábrica roxa. A habilidade cobre a pista inteira de espinhos | Tempestade de espinhos a cada 15 s (habilidade spikes_global) | Sem diferença relevante no código | - |
| 0-0-5 | Perma-Spike | Perma-Espinho | Fábrica amarela e preta, montes dourados. Os espinhos ficam na pista mesmo depois de vender a fábrica | Espinhos de 10 de dano, pierce 50, duram 300 s | Sem diferença relevante no código | - |

### Monkey Village (Vila)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 0-0-0 | Monkey Village | Vila dos Macacos | Cabana de palha. Aura de buff | buff nas vizinhas | Sem diferença relevante no código | - |
| 5-0-0 | Primary Expertise | Especialização Primária | Castelo de pedra com bandeiras vermelhas e balista que atira (tem seletor de alvo) | Primárias próximas: +3 pierce no total; balista gigante (ataque projétil; sprite 'balista') | Sem diferença relevante no código | - |
| 0-5-0 | Homeland Defense | Defesa da Pátria | Prédio moderno com antena parabólica. Habilidade | Habilidade: todas as torres 2x mais rápidas por 20 s (habilidade turbo_area) | Sem diferença relevante no código | - |
| 0-0-4 / 0-0-5 | Monkey City / Monkeyopolis | Metrópole Macaco / Macacópolis | Prefeitura de madeira. O botão do tier 5 fica cinza com o texto "Requires Banana Farm" e o ícone da fazenda: upgrade condicional | Sacrifica fazendas para gerar dinheiro | Monkeyopolis sem efeito e sem a trava Requires Banana Farm no botão (pendência) | P1 |

### Engineer Monkey (Engenheiro)

| Up | Nome BTD6 | Nome no clone | Disparo no BTD6 | Disparo no clone | Diferença | Prioridade |
|---|---|---|---|---|---|---|
| 5-0-0 | Sentry Champion | Campeão das Sentinelas | Armadura roxa e amarela. Espalha torretas-sentinela roxas em volta | Torretas campeãs | Sem diferença relevante no código | - |
| 0-5-0 | Ultraboost | Ultraimpulso | Visual azul e branco tecnológico. Botão extra de mira verde no painel para escolher a torre que recebe o buff | (sem descrição) (habilidade turbo_area) | Ultraboost é turbo em área; falta o botão para escolher a torre que recebe o buff | P2 |
| 0-0-5 | XXXL Trap | Armadilha XXXL | Capacete amarelo e armadilha azul grande. O painel mostra o dinheiro gerado ao lado dos estouros | Armadilha de 10.000 de RBE que prende dirigíveis | Sem diferença relevante no código | - |

### Beast Handler (ausente no clone)

| Up | Nome BTD6 | Disparo no BTD6 |
|---|---|---|
| 0-0-0 | Beast Handler | Macaco tribal com lança. O caminho 1 aparece cinza com "Needs Water" |
| 0-4-0 | Tyrannosaurus Rex | Dinossauro laranja que morde. O tier 5 mostra "Requires Beast 1/4" e o painel tem um contador roxo de poder de fera (16/64). Selo roxo com número sobre a torre |
| 0-0-4 | Giant Condor | Condor que voa, com uma mira verde de destino no mapa. Tier 5 também exige juntar feras |

### Não observados

- **Mermonkey e Skywarden:** não têm tecla de atalho e a rolagem da loja não respondeu.
  Nenhum dos dois existe no clone.
- **Desperado:** bloqueado na conta (cadeado na loja).
- **Paragons:** fora do escopo.

## 4. Mecânicas transversais

| Mecânica | BTD6 (visto) | Clone (código e capturas) | Diferença | Prioridade |
|---|---|---|---|---|
| Cerâmica | Ao estourar, solta as camadas em fila (arco-íris, zebra, filhos menores). Na fila colada do Sandbox isso fica bem visível | Cerâmica com rachaduras por dano (0 a 2) e filhos normais | Equivalente | - |
| Congelado | O bloon continua visível, com uma capa branca de gelo por cima. No Absolute Zero, todos os bloons do mapa ganham a capa | Sprite próprio de estado congelado (`spr::estado_*`) | Equivalente | - |
| Colado | A cola cobre o bloon e escorre. A cor muda com o upgrade: amarela (base), verde (corrosiva), rosa (Super Glue) | Um estado de cola só | Falta a cor por upgrade | P2 |
| Queimando | Chamas laranja e azuladas por cima do bloon | Estado queimando próprio | Equivalente | - |
| Atordoado | Estrelas brancas girando sobre o bloon ou o dirigível | Estado atordoado próprio | Equivalente | - |
| Crítico | Texto laranja "CRIT" sobre o alvo | Crítico calculado, sem texto. O `render.cpp` já tem o efeito de texto flutuante (usado em "+$" e "NÍVEL"), então basta um evento novo | Falta o texto | P1 |
| Estouro | Respingo branco e azul com lascas; cerâmica solta lascas marrons | Efeito de estouro próprio (evento "pop") | Equivalente | - |
| Dinheiro por estouro | Soma até no Sandbox | Soma (eventos "dinheiro") | Equivalente | - |
| Dirigíveis | MOAB grande (cobre quase duas faixas da pista), com 4 a 5 estados de dano | 5 estados de dano e fortificado | Equivalente | - |
| Pilhas na pista | Espinhos continuam na pista depois de vender a fábrica | `Pista::vender` não mexe nas pilhas, e cada pilha guarda a própria referência da torre | Igual | - |
| Modos de alvo | Além de First, Last, Close e Strong, cada torre tem os seus: rotas do Ás (Circle, Infinite, Figure Eight, Centered Path), Heli (Pursuit, Follow Mouse, Lock in Place, Patrol Points), Dartling no cursor (Normal, Locked), Mortar com "Set Target", Sniper "Elite", Anti-Bloon com um alvo por braço | Só os 4 genéricos (`MODOS_ALVO` em `sim.cpp`, `NOMES_MODO` em `cena_jogo.cpp`) | É a maior lacuna de jogabilidade | P0 |
| Diálogos de confirmação | Sun Temple e True Sun God pedem confirmação ("Temple Sacrifice") | Não há | Falta, junto com o sacrifício | P1 |
| Upgrade condicional | Monkeyopolis mostra "Requires Banana Farm" com o ícone da fazenda; Beast Handler mostra "Needs Water" e "Requires Beast 1/4" | Não há trava condicional | Falta | P1 |
| Painel troca de lado | Com a torre na metade direita, o painel abre à esquerda | Também troca de lado (`painel_super.png`) | Igual | - |
| Contadores extras no painel | Lápide do Necromancer, dinheiro gerado do XXXL Trap, poder de fera do Beast Handler | Só estouros | Falta o contador de cemitério e o de dinheiro gerado | P2 |

## 5. Torres e upgrades do BTD6 ausentes no clone

- **Torres novas:** Beast Handler, Mermonkey, Desperado e Skywarden. O dono adiou as quatro
  (Pendências de `docs/btd6-transformacao.md`, 30/09/2026).
- **Modo Sandbox:** painel para enviar qualquer bloon ou rodada, com dinheiro e vidas infinitos.
- **Modos de restrição:** Primary Only, Military Only e similares.
- **Paragons**, Powers, Insta Monkeys e Monkey Knowledge: fora do escopo de um trabalho de SO.
- **Mecânicas de upgrade sem efeito no clone** (todas já em Pendências): Legend of the Night,
  Shrink Potion, Monkeyopolis, Rota Centralizada, Mini-Comanches, coleta de bananas, juros do
  banco, +15 vidas do Wall Street, cemitério do Necromancer, transformação do Fan Club e da
  Total Transformation, sacrifício do Sun Temple.

## 6. Backlog priorizado

| ID | Prioridade | Item | Onde no clone (arquivo: trecho) | Esforço | Já em Pendências? | Estado |
|---|---|---|---|---|---|---|
| B01 | P0 | Dartling aponta para o cursor (modo Normal) e pode travar a direção (Locked) | `src/jogo/sim.cpp`: `Pista::alvo` e o laço de disparo; `src/cliente/cena_jogo.cpp`: seletor de alvo; `src/cliente/conexao.cpp`: comando novo para mandar a posição do cursor na Batalha | M | sim | feito (teste e tela) |
| B02 | P0 | Mortar com ponto de impacto fixo: botão "Set Target" no painel e clique no mapa | `src/jogo/sim.cpp`: ramo `TipoAtaque::MORTEIRO`; `src/cliente/cena_jogo.cpp`: botão no painel; `src/cliente/conexao.cpp`: comando | M | sim | feito (teste; painel não visto na tela) |
| B28 | P0 | Projéteis miram a posição atual do bloon e erram alvos distantes ou rápidos (ver 3.2). Corrigir antecipando o alvo no disparo ou aumentando a velocidade dos projéteis | `src/jogo/sim.cpp`: cálculo de `ang` no laço de disparo e `Pista::disparar`; `src/jogo/dados.cpp`: `vel` dos ataques | M | não | feito (teste; não visto na tela) |
| B29 | P1 | Boomerang: botão no painel para trocar a mão (lado do arco) | `src/jogo/sim.cpp`: `Pista::mover_bumerangue`; `src/jogo/sim.hpp`: campo na `Torre`; `src/cliente/cena_jogo.cpp`: botão; `src/cliente/conexao.cpp`: comando | P | não | feito (teste; botão não visto na tela) |
| B03 | P1 | Texto "CRIT" nos acertos críticos | `src/jogo/sim.cpp`: `critico` emite um evento novo; `src/cliente/render.cpp`: texto flutuante | P | não | feito (teste; não visto na tela) |
| B04 | P1 | Modo Sandbox no solo: dinheiro e vidas infinitos, painel para mandar qualquer bloon ou rodada, apagar bloons e torres | `src/cliente/cenas_menu.cpp`: opção no Jogo Solo; `src/cliente/cena_jogo.cpp`: painel; `src/jogo/sim.cpp`: o modo "rico" do robô (`tools/analise/partida.cpp`) já tem a base | G | não | a fazer |
| B05 | P1 | Ás: pista de pouso como torre e rotas no painel (Circle, Infinite, Figure Eight, Centered Path com mira arrastável) | `src/jogo/sim.cpp`: `Mov::ORBITA`; `src/jogo/dados.cpp`: torre "as"; `src/cliente/sprites.cpp`: sprite da pista | M | em parte (Rota Centralizada) | a fazer |
| B06 | P1 | Heli: modos Follow Mouse, Lock in Place e Patrol Points | `src/jogo/sim.cpp`: `Mov::HELI`; `src/cliente/cena_jogo.cpp`: seletor | M | não | a fazer |
| B07 | P1 | Fan Club e Total Transformation trocam o visual das torres afetadas | `src/jogo/sim.cpp`: habilidade `turbo_area`; `src/cliente/arte.cpp`: desenho da torre transformada | M | sim | a fazer |
| B08 | P1 | Sun Temple e True Sun God: diálogo de sacrifício e o efeito de sacrificar torres próximas | `src/cliente/cena_jogo.cpp`: diálogo (usa o confirmar de `src/cliente/ui.hpp`); `src/jogo/sim.cpp`: compra do upgrade | M | sim | a fazer |
| B09 | P1 | Bananas caem no chão e são coletadas (com o Ez Collect automático) | `src/jogo/sim.cpp`: `TipoAtaque::RENDA`; `src/cliente/render.cpp` e `cena_jogo.cpp`: clique para coletar | M | sim | a fazer |
| B10 | P1 | Upgrade condicional: Monkeyopolis com "Requires Banana Farm" e efeito de sacrificar fazendas | `src/jogo/stats.cpp` e `sim.cpp`: regra de compra; `src/cliente/cena_jogo.cpp`: carta cinza com o motivo | M | sim | a fazer |
| B11 | P1 | Cemitério do Necromancer (bloons estourados alimentam os zumbis) e contador de lápide no painel | `src/jogo/sim.cpp`: pilha "zumbi"; `src/cliente/cena_jogo.cpp`: painel | M | sim | a fazer |
| B12 | P1 | Blade Maelstrom e Super Maelstrom: a habilidade lança a espiral de serras que se vê pelo mapa | `src/jogo/dados.cpp`: habilidade "Turbilhão" (hoje `turbo`); `src/jogo/sim.cpp`: tipo de habilidade novo que dispara projéteis em espiral | M | não | a fazer |
| B13 | P1 | Ray of Doom: raio contínuo que atravessa o mapa na direção do cursor (depende de B01) | `src/jogo/sim.cpp`: hitscan do Plasma Accelerator; `src/cliente/render.cpp`: feixe persistente | M | não | a fazer |
| B14 | P1 | Legend of the Night e Shrink Potion com efeito | `src/jogo/sim.cpp` e `src/jogo/dados.cpp` | M | sim | a fazer |
| B15 | P1 | Elite Sniper: modo de alvo "Elite" e caixa de suprimentos que cai no mapa | `src/jogo/sim.cpp`: `MODOS_ALVO` e habilidade `dinheiro`; `src/cliente/render.cpp`: caixa | M | não | a fazer |
| B27 | P1 | Tier 5 com silhueta própria: trocar o desenho inteiro da torre nos tier 5 (e nos tiers 3 e 4 mais marcantes), como a catapulta do Juggernaut, o templo do Deus Sol, o lançador de mísseis do MOAB Mauler e o robô do Robo Monkey | `src/cliente/sprites.cpp`: `TS()` e `MAQ()`; conferir na vitrine (`--vitrine`) | G | não | a fazer |
| B16 | P2 | Loja com cor de fundo por categoria (Primárias, Militares, Mágicas, Suporte) | `src/cliente/cena_jogo.cpp`: desenho dos cartões da loja | P | não | a fazer |
| B17 | P2 | Selo de camo no painel da torre | `src/cliente/cena_jogo.cpp`: painel de upgrade | P | não | a fazer |
| B18 | P2 | Cor da cola por upgrade (amarela, verde, rosa) e estrelas na Super Glue | `src/cliente/arte.cpp`: estado de cola; `src/jogo/dados.cpp`: visual por upgrade | P | não | a fazer |
| B19 | P2 | Anti-Bloon com um seletor de alvo por braço | `src/jogo/sim.cpp`: alvo por ataque; `src/cliente/cena_jogo.cpp` | M | não | a fazer |
| B20 | P2 | Sub: botão Submerge e radar verde | `src/jogo/dados.cpp`: submarino 1-3; `src/cliente/cena_jogo.cpp` e `render.cpp` | M | não | a fazer |
| B21 | P2 | Tela de consulta dos 15 upgrades de cada torre fora da partida | `src/cliente/cenas_menu.cpp` (pode reaproveitar a vitrine) | M | não | a fazer |
| B22 | P2 | Ultraboost: escolher a torre que recebe o buff | `src/jogo/sim.cpp`: habilidade `turbo_area`; `src/cliente/cena_jogo.cpp` | M | não | a fazer |
| B23 | P2 | Mini-Comanches, juros do banco e +15 vidas do Wall Street | `src/jogo/sim.cpp` e `src/jogo/dados.cpp` | M | sim | a fazer |
| B24 | P2 | Polimento de efeitos: clarão do Bloon Crush, anel expandindo no Inferno Ring, Kylie em chamas, rastro verde do Perma Charge, visual roxo do Super Brittle, avião do Tsar Bomba | `src/cliente/render.cpp`, `arte.cpp` e `sprites.cpp` | P cada | não | a fazer |
| B25 | P2 | Modos de restrição (Primary Only, Military Only) | `src/cliente/cenas_menu.cpp` e `src/jogo/sim.cpp` | M | não | a fazer |
| B26 | P2 | Beast Handler, Mermonkey, Desperado e Skywarden | torres novas em `src/jogo/dados.cpp` e arte em `src/cliente/sprites.cpp` | G | sim (adiado pelo dono) | a fazer |

Implementação em andamento na branch `luiz/backlog-btd6`, um commit por item. "Não visto na
tela" quer dizer que o item passa no teste automático, mas a automação de cliques não conseguiu
operar o clone para conferir o visual; vale uma olhada à mão.

## 7. O que não foi possível observar e por quê

- **O clone torre por torre:** o clone só foi jogado por pouco tempo no modo `--demo` (seção
  3.1). A comparação por torre usou o código e as capturas antigas. A leitura do código resolveu
  os 8 itens "conferir", mas o visual de cada disparo do clone (cores, tamanhos, animação) não
  foi visto em tela torre por torre.
- **Mermonkey e Skywarden:** sem tecla de atalho, e a rolagem da loja parou de responder.
- **Desperado:** bloqueado na conta.
- **Tiers 1, 2 e 4 da maioria das torres:** para caber no tempo, cada caminho foi capturado no
  tier 3 e no tier 5. Os tiers intermediários só aparecem no retrato do painel.
- **Projéteis muito rápidos em voo:** dardo simples, bala do Sniper e dardos do Ás aparecem
  pouco nas capturas em sequência.
- **Plasma Monkey Fan Club e habilidades que precisam de alvo no alcance** (MOAB Eliminator,
  Overdrive, Supply Drop): ativadas sem alvo ou sem efeito visível na captura.
- **Paragons:** fora do escopo.
