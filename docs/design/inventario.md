# Inventário: Claude Design → bloons-battles

Projeto de design: `https://claude.ai/design/p/f8eb94b8-d2cd-4000-9f5f-2490a78409ef`
(lido pelo MCP do Claude Design). Arquivos lidos por inteiro:

- `Sprites.dc.html`: galeria com abas (torres, ícones de upgrade, heróis, bloons, dirigíveis,
  projéteis, efeitos, habilidades). Os desenhos vêm de `sprites2.js`, uma biblioteca de primitivas
  que gera ao mesmo tempo o SVG e a "receita" raylib de cada sprite. `dados_sprites.js` tem as
  torres, heróis e habilidades extraídos de `src/jogo/dados.cpp`; `sprites_ponte.js` junta a API
  antiga (`sprites.js`: ícones do HUD e fundo dos mapas) com a nova.
- `Auditoria Bloons TD Battles.dc.html`: análise das telas, design system (paleta, tipografia,
  medidas), componentes, mockups 1280×720 (importados de `Partida Nova.dc.html` e
  `Telas Menu.dc.html`, também lidos), guia de arte e backlog P0/P1/P2.
- `support.js`: runtime genérico dos arquivos `.dc.html` (templates `sc-for`/`sc-if`, React). Não
  tem dados de arte; só foi usado para entender como os outros dois montam as páginas.

## 1. Sprites (`Sprites.dc.html`)

A biblioteca `sprites2.js` foi portada para C++ quase linha a linha, com as mesmas coordenadas
(caixa 128×128; dirigíveis 220×120; projéteis centrados em 0,0).

| Elemento do design | Onde ficou | Observação |
|---|---|---|
| Caneta (`criarPen`: c, e, rr, poly, star, ln, sec, arc, ring, ringE, txt, g) | `src/cliente/caneta.{hpp,cpp}` | Contorno = mesma forma em TINTA desenhada maior por baixo, como no PREAMBULO do design. Polígonos triangulados por remoção de orelhas (o design fazia leque, que falha em formas côncavas). |
| Animações em loop dos grupos (gira, balanca, flutua, recuo, voa, pulsa, cresce, pisca, some) | `spr::Anim` + `Caneta::aplicar_anim` | Mesmas fórmulas (`onda`, `recuo`) do PREAMBULO. Novo tipo `BRACO` para o braço que ataca (segue a pose dos clipes). |
| 22 torres: base, T3 e T5 de cada caminho (`TS`, `MAQ`) | `sprites.cpp` (`TS()`, `MAQ()`, `macaco()`, `maquina()`) | Vista de cima no mapa; vista 3/4 nos cards, painel, menus. |
| 18 heróis nos níveis 1, 10 e 20 (`HS`, `tanque`) | `sprites.cpp` (`HS()`, `tanque()`) | Nível vem de `Torre::nivel`. |
| Itens na mão, chapéus, rostos, costas, aura, órbita | `item()`, `chapeu()`, `rosto2()`, `costas()`, `aura()`, `orbita()` | Todos os tipos do design. |
| 12 bloons + camo, regen, fortificado | `desenhar_bloon()` | Sombra em crescente = elipse escura + elipse clara deslocada (sem recorte), regen = coração fora da silhueta. Cerâmica ganhou rachaduras por dano (o jogo já tinha 0..2). |
| Dirigíveis MOAB, BFB, ZOMG, DDT, BAD, dano 0..4 e fortificado | `desenhar_dirigivel()` | O render agora usa os 5 estados de dano (antes 0..3). |
| 57 projéteis (`PROJ`) | `PROJ()` | Mais `fragmento` e `uva_fogo`, visuais do jogo que o design não desenhou (feitos com as mesmas peças). |
| Efeitos (estouro, explosão, dinheiro, nível, impacto, camo revelado) e estados (congelado, colado, queimando, atordoado) | `spr::efeito()`, `spr::estado_*()` | No jogo usam a fase do efeito (0→1), como pede a nota do design. |
| 75 ícones de habilidade (moldura dourada, retrato, selo do efeito) | `spr::habilidade()` + `GLIFO`, `COR_EFEITO` | Usados nos botões de habilidade e no painel do herói. |
| Ícones do HUD (`sprites.js`: coração, moeda, eco, rodada, oponente, cadeado, estrela) | `spr::icone()` | Caixa 32×32. |
| Fundo dos mapas (`sprites.js` `mapa`) | `arte.cpp` `desenhar_mapa()` | Trilha em 4 camadas (tinta, borda escura, terra, faixa clara), água com margem de areia, árvores/pedras com sombra e contorno; usa os dados reais de `mapas.cpp`. |
| Torres invocadas `sentinela` e `fenix` | `MAQ()` | Não existem no design; desenhadas com as mesmas peças (máquina com canhão; fênix com as asas "pena" e chamas). |

### Cache em RenderTexture

`arte.cpp` desenha cada sprite em camadas: a Caneta tem um modo que conta os grupos animados e
separa o sprite em segmentos parados (cada um vira uma RenderTexture em resolução dobrada, com
margem de 25% para chapéus e auras) e grupos animados (braço de ataque, hélices, órbitas,
coração do regen...), que são desenhados ao vivo entre as texturas, na mesma ordem. Ícones,
retratos e habilidades ficam numa textura só.

### Aproximações

- Degradês e opacidade de grupo do SVG viram alfa multiplicado nas cores do grupo (`pisca`, `some`).
- Texto dentro de sprite (`$` da fazenda T5, `+$` do efeito) usa a fonte Lilita One já carregada
  (tamanho 24) escalada; o design usava texto SVG.
- Heli T5 do caminho 3: no design a órbita de drones ficava deslocada (grupo aninhado somava
  64,70 duas vezes); aqui foi centralizada no helicóptero.
- `rr(..., 'none', b3)` (moldura sem preenchimento da fazenda) vira `DrawRectangleRoundedLinesEx`.

## 2. Auditoria (`Auditoria Bloons TD Battles.dc.html`)

IDs A01..A20 seguem o backlog da auditoria (seção 5). Os achados da análise (seção 1) estão
ligados ao item que os resolve. Nenhum item muda mecânica, balanceamento, atalhos ou protocolo.

| ID | Prior. | Item | Arquivo(s) | Status |
|---|---|---|---|---|
| A01 | P0 | Painel de upgrade esconde o 3º caminho (2 caminhos com torre à esquerda na Batalha) → 3 colunas lado a lado, 384×320 | `cena_jogo.cpp` `painel_upgrade`, `linha_upgrade` | feito |
| A02 | P0 | Recarga de envio (0,6 s) invisível → barra escura + filete amarelo no botão | `cena_jogo.cpp` `painel_envios`; `jogo/sim.{hpp,cpp}` `Partida::recarga_envio` (só leitura) | feito |
| A03 | P0 | Tokens de cor/medida + `contorno_auto()` (contorno ~10% da altura) | `ui.hpp`, `ui.cpp` `texto` | feito |
| A04 | P0 | Botão sem estado pressionado; desabilitado só troca a cor → 4 estados (sobe 2 px, afunda 3 px sem sombra, cinza sem brilho) | `ui.cpp` `Botao::desenhar` | feito |
| A05 | P0 | Card de torre: sem dinheiro pouco visível, tecla 13 px contorno 1 → bege apagado, ícone a 45%, rodapé vermelho, keycap | `cena_jogo.cpp` `painel_lateral`; `ui::tecla` | feito |
| A06 | P0 | HUD de vidas/dinheiro/eco direto sobre o mapa → placa de madeira escura | `cena_jogo.cpp` `hud_topo`; `ui::placa` | feito |
| A07 | P1 | Pips 11×11 sem contorno → 19×12 com contorno, tier 5 dourado | `ui::pips` | feito |
| A08 | P1 | Custo e eco do envio no mesmo estilo → pílula de preço + eco colorido | `painel_envios` | feito |
| A09 | P1 | Solo: dificuldade não escolhida parecia desabilitada; herói escolhido quase invisível → verde × bege; herói com fundo dourado e borda amarela | `cenas_menu.cpp` `CenaSolo`, `GradeHerois` | feito |
| A10 | P1 | Rodada longe dos recursos → placa de rodada no topo do painel lateral | `painel_lateral` | feito |
| A11 | P1 | Prévia não diferencia posição inválida de falta de dinheiro → vermelho + X; âmbar + preço | `cena_jogo.cpp` `previa` | feito |
| A12 | P1 | Mini mapa do oponente e avisos rosa soltos → faixa com ícone, vidas e tecla O; avisos RECEBENDO/ENVIADO em placas | `mini_oponente` | feito |
| A13 | P1 | Lobby: IP em texto solto e pontos que mudam a largura → cartão de IP e porta; pontos com largura fixa | `CenaLobby::desenhar` | feito |
| A14 | P1 | Campo de texto: foco só clareia; erro rosa sobre azul; porta abaixo da grade ao hospedar → anel amarelo, erro em placa vermelha com "!", porta como passo 2 | `ui::CampoTexto`, `ui::placa_erro`, `CenaBatalha` | feito |
| A15 | P2 | Menu sem placa, "BATTLES" laranja solto, macacos flutuando → painel de madeira, faixa vermelha "BATTLES", macacos no chão | `CenaMenu`, `titulo`, `FundoBloons` | feito |
| A16 | P2 | Solo/Batalha sem ordem → painéis numerados 1-2-3 e atalhos Enter/Esc nos botões | `cenas_menu.cpp` | feito |
| A17 | P2 | Habilidade pronta sem destaque → anel amarelo pulsando, keycap com o número | `habilidades` | feito |
| A18 | P2 | Sombra em crescente e contorno final nos sprites | `sprites.cpp` | feito (conforme Sprites.dc.html, ver conflitos) |
| A19 | P2 | Faixa de categoria nos cards (primária, militar, mágica, suporte, herói) | `painel_lateral` (`cor_categoria`) | feito |
| A20 | P2 | Log F1 aberto por padrão sobre a trilha → começa fechado | `CenaJogo` (construtor) | feito |
| A21 | análise | Vender em laranja; prioridade sem indicar Tab → Vender vermelho; ALVO com setas e keycap Tab | `rodape_upgrade` | feito |
| A22 | análise | Tier 5 = retângulo "MÁXIMO"; caminho fechado cinza liso; atalhos `,./` ilegíveis → moldura dourada tripla, listras + cadeado, keycaps | `linha_upgrade` | feito |
| A23 | análise | Mapa escolhido só troca a moldura → ✓ verde, borda amarela e elevação | `desenhar_mapas` | feito |
| A24 | análise | Avisos só texto em y=150; banner sem placa → placa vermelha e placa de rodada | `mensagens` | feito |
| A25 | análise | Painel lateral, dica e envios no mesmo design system (madeira com veio, pergaminho, TINTA) | `ui::painel_madeira`, `dica` | feito |

### Conflitos com o sprite novo (resolvidos a favor de `Sprites.dc.html`)

- A18: o guia de arte da auditoria pedia a sombra em crescente com `por_linhas()` (recorte linha a
  linha). O `Sprites.dc.html` faz a sombra com uma elipse escura e outra clara deslocada por cima,
  sem recorte. Foi seguido o Sprites.
- A17: o mockup da auditoria põe o retrato da torre num círculo azul no botão de habilidade; o
  `Sprites.dc.html` define o ícone de habilidade (moldura dourada, fundo na cor do efeito, selo).
  Foi usado o ícone do Sprites, com os estados da auditoria (anel pulsando, setor de recarga,
  keycap).

### Correção extra encontrada ao conferir as capturas

- O F12 da raylib multiplica o tamanho da captura pela escala de DPI do Windows e lê fora do
  framebuffer: em telas com escala de 125% a imagem saía 1600×900 com uma faixa preta. O jogo
  agora faz a própria captura (`ui::capturar`), salva `captura_AAAAMMDD_HHMMSS.png`, e o CMake
  desliga `SUPPORT_SCREEN_CAPTURE`/`SUPPORT_GIF_RECORDING` da raylib (o Ctrl+F12 de GIF saiu junto).

### Pendências

Nenhum item ficou pendente. Diferença consciente em relação ao mockup: no mini mapa do oponente os
bloons continuam como círculos coloridos (no mockup eram sprites de 9 px), para não pesar a
miniatura com centenas de bloons.

## 3. Animações de disparo e habilidade

Ferramenta: `src/cliente/anim.{hpp,cpp}`. Cada animação é um clipe com trilhas de keyframes por
canal e uma curva de suavização por trecho (linear, entra/sai quad, sai cúbica, entra/sai cúbica,
sai com volta, sai elástica). A vitrine (`BloonsBattles --vitrine 9`) toca todos os clipes em
loop numa torre e desenha as curvas de cada canal com a cabeça de leitura; foi nela que os tempos
foram ajustados. Na partida, F9 repete o disparo da torre selecionada e Shift+F9 a habilidade.

| Clipe | Torres | Duração | Canais |
|---|---|---|---|
| disparo: arremesso | dardo, bumerangue, ninja, sauda, pat, alquimista, cola, brickell | 0,28 s | giro (antecipa +26°, golpe −34°, volta com overshoot), estica, corpo |
| disparo: arco | quincy | 0,32 s | estica (puxa −9 e solta), giro, flash |
| disparo: tiro | sniper, dartling, engenheiro, striker, rosalia, jericho | 0,22 s | estica (coice −8), giro, flash na ponta, corpo |
| disparo: magia | mago, druida, gelo, super e demais heróis | 0,34 s | giro, estica, flash, escala 1,04 |
| disparo: canhão | bomba, sentinela, churchill, morteiro, submarino, bucaneiro | 0,45 s | recuo dos canos, escala 0,93, corpo |
| disparo: pulso | tachinha, espinhos, heli, ás, fênix | 0,30 s | escala 1,1, recuo |
| habilidade: 12 clipes (um por efeito: turbo, turbo em área, dano global, dano forte, congela tudo, lentidão, dinheiro, espinhos, espinhos global, invocar, reverso, roubo) | quem tem a habilidade | 0,7 a 1,1 s | agacha (escala 0,88), pico (escala até 1,25, pulo, braço erguido −70°, clarão), onda de choque e brilho na cor do efeito, volta elástica |

Gatilho, sem tocar na simulação: o `anim::Animador` guarda, por torre, os valores do quadro
anterior de `Torre::recargas` e `Torre::hab_rec`. Recarga que sobe = a torre atirou; `hab_rec`
que reinicia = usou a habilidade. O estado das animações vive no `RenderPista` (cliente) e usa o
relógio do render; `src/jogo` não inclui nada do cliente.

## 4. Capturas (`docs/design/capturas/`)

Geradas com `BloonsBattles --captura` (a mesma captura do F12):

| Arquivo | Como gerar |
|---|---|
| `01_menu.png` | `BloonsBattles --captura 01_menu.png 2` |
| `02_solo.png`, `03a_hospedar.png`, `03b_entrar_erro.png`, `03c_lobby.png` | `--tela solo \| hospedar \| entrar-erro \| lobby` |
| `04_partida_solo.png`, `05_partida_batalha.png`, `06_pausa.png` | `--demo solo`, `--demo batalha`, `--demo solo --pausa` |
| `07_habilidade_em_uso_sequencia.png` | 6 quadros de `--demo solo` (0,1 s entre eles): habilidades disparadas pelo comando normal `B`, com brilho e onda |
| `10_...` a `20_...` | `--vitrine 0..10` (torres, tiers, heróis, bloons, dirigíveis, projéteis, efeitos, habilidades, animações, mapas) |
