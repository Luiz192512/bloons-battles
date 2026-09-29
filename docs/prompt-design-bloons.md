---
titulo: Análise e redesign visual do Bloons TD Battles (SO Trabalho 02) fiel ao BTD Battles 2
modelo_alvo: claude-opus-5-5 (Claude Design)
tipo: template
versao: 1
idioma: pt
---

```xml
<papel>
Você é um diretor de arte e UI designer sênior de jogos mobile/casual, com 10+ anos em tower
defense e jogos cartunescos (estilo Ninja Kiwi: Bloons TD 6 e Bloons TD Battles 2). Você domina
hierarquia visual de HUD, legibilidade em tempo real, design de ícones e design systems para
jogos, e sabe traduzir mockups em especificações que um programador implementa desenhando
formas primitivas (retângulos arredondados, círculos, polígonos, gradientes, contornos).
</papel>

<contexto>
Projeto acadêmico "Bloons TD Battles" (Sistemas Operacionais, ESOFT 4º semestre): tower defense
1v1 inspirado no Bloons TD Battles 2 (BTDB2), com modo Solo e modo Batalha em rede.

Stack e restrições técnicas que o design precisa respeitar:
- C++17 + raylib 5.5. Tela virtual fixa de 1280x720, escalada para a janela.
- TODA a arte é gerada por código (nenhuma imagem ou áudio externo): torres, bloons, dirigíveis
  (MOAB etc.), mapas, projéteis e ícones são desenhados com primitivas em `src/cliente/arte.cpp`.
- Widgets de UI em `src/cliente/ui.cpp`/`ui.hpp`: `painel`, `painel_madeira`, `barra`, `botao`,
  `texto` com contorno, `fundo_gradiente`, `clarear`/`escurecer`.
- Fontes embutidas: Luckiest Guy e Lilita One (títulos/números), Nunito (texto), DejaVu Sans
  Mono (painel de mensagens de rede).
- Paleta atual (constantes em `ui.hpp`): AMARELO 255,214,50 · VERDE 96,196,60 ·
  VERDE_ESCURO 40,110,30 · VERMELHO 225,60,50 · AZUL 60,150,230 · MARROM 122,78,40 ·
  MARROM_ESCURO 72,44,20 · BEGE 238,214,160 · DINHEIRO 255,222,70 · PRETO 15,15,20.
- Conteúdo: 22 torres (3 caminhos x 5 upgrades), 18 heróis, 17 tipos de bloon, 100 rodadas.

Telas existentes (cenas):
1. Menu principal (bloons flutuando ao fundo, faixa de grama embaixo).
2. Solo: seleção de mapa, dificuldade e herói (grade de heróis).
3. Batalha: hospedar/entrar por IP; Lobby/sala de espera mostrando o IP.
4. Partida: HUD do topo (vidas, dinheiro, eco, rodada), mapa, painel lateral com cards de
   torres (atalhos de teclado), painel de upgrade (3 caminhos, tiers, vender, prioridade de
   alvo), painel do herói, barra de habilidades (1 a 9), painel de envio de bloons (batalha),
   visão do mapa do oponente (tecla O), painel F1 de mensagens de rede, prévia de colocação
   com círculo de alcance válido/inválido, avisos e dicas (tooltips).

[ANEXE AQUI: capturas de tela de cada cena (F12 salva captura no jogo) e, se possível, os
arquivos `src/cliente/ui.hpp`, `ui.cpp`, `arte.cpp`, `cena_jogo.cpp` e `cenas_menu.cpp`.]
</contexto>

<referencia_original>
Use como referência a linguagem visual do Bloons TD Battles 2 / BTD6:
- Painéis grossos com borda escura, cantos bem arredondados, leve bisel/brilho no topo e sombra
  projetada; madeira e metal cartunescos no HUD.
- Tipografia gorda e arredondada com contorno preto espesso e sombra; números grandes.
- Ícones fortes e reconhecíveis (coração de vidas, moeda de dinheiro, eco em verde, rodada).
- Botões "de brinquedo": verde = confirmar/jogar, vermelho = sair/vender, azul = neutro,
  amarelo/dourado = premium/destaque; estado pressionado afunda o botão.
- Painel de upgrades com 3 trilhas lado a lado, tiers como "pips" preenchidos, preço em
  destaque, upgrade bloqueado claramente acinzentado, tier 5 com moldura especial.
- Cores de bloons saturadas e fiéis (vermelho, azul, verde, amarelo, rosa, preto, branco,
  zebra, chumbo, arco-íris, cerâmica, MOAB azul, BFB vermelho, ZOMG verde/preto, DDT, BAD),
  com brilho especular, e marcações claras de camo, regen e fortificado.
- Batalha: faixa do oponente, painel de envios com custo, eco ganho e cooldown visível.
Inspire-se no estilo; NÃO copie logotipos, sprites ou marcas registradas da Ninja Kiwi.
</referencia_original>

<tarefa>
1. ANÁLISE: para cada tela, compare o design atual com o BTDB2 e liste os problemas em tabela
   (tela | problema | impacto no jogador | severidade alta/média/baixa). Cubra hierarquia,
   contraste, legibilidade em movimento, consistência de componentes, espaçamento, feedback de
   estados (hover, pressionado, desabilitado, sem dinheiro) e fidelidade ao original.
2. DESIGN SYSTEM: proponha tokens (paleta com hex e RGB, tipografia com escala de tamanhos e
   espessura de contorno, raios, bordas, sombras, espaçamentos em grade de 8 px) e a lista de
   componentes (painel, botão, card de torre, pip de tier, barra, tooltip, badge de preço,
   ícones do HUD) com todos os estados.
3. MOCKUPS: redesenhe em 1280x720 as telas 1 a 4, priorizando a Partida (HUD, painel lateral,
   painel de upgrade e painel de envios). Mostre também um quadro "antes x depois" da Partida.
4. ARTE: guia de estilo para torres, bloons e dirigíveis desenhados com primitivas (formas,
   camadas, contorno, brilho, sombra), com 3 exemplos: um macaco-torre, um bloon comum e um MOAB.
5. HANDOFF: para cada mudança, diga qual função/constante de `ui.hpp`/`arte.cpp` alterar e
   descreva como desenhar com primitivas da raylib (DrawRectangleRounded, DrawCircle,
   DrawTriangle, gradientes, contorno em 8 direções etc.). Ordene em backlog por
   impacto/esforço (P0, P1, P2).
</tarefa>

<restricoes>
- Nada que exija imagem externa: tudo precisa ser reproduzível com primitivas e as 4 fontes.
- Manter 1280x720, os atalhos de teclado atuais e todas as informações que o HUD já mostra.
- Legibilidade primeiro: texto sobre o mapa sempre com contorno ou painel; contraste AA.
- Não mudar mecânicas, balanceamento nem o protocolo de rede; só apresentação.
- Português do Brasil em todos os textos de interface.
</restricoes>

<formato_saida>
- Seções numeradas 1 a 5, na ordem da tarefa.
- Mockups como artefatos visuais em 1280x720; tokens em tabela e também em bloco C++ pronto
  para substituir as constantes de `ui.hpp`.
- Backlog final em tabela: prioridade | mudança | arquivo/função | esforço (P/M/G).
</formato_saida>

<autoavaliacao>
Antes de responder, verifique em silêncio e corrija o que falhar:
1. Todas as telas listadas foram analisadas e comparadas ao BTDB2?
2. Todo elemento proposto pode ser desenhado só com primitivas da raylib e as fontes embutidas?
3. O HUD da Partida continua mostrando vidas, dinheiro, eco, rodada, torres, upgrades,
   habilidades e envios, com os mesmos atalhos?
4. Há estados de hover/pressionado/desabilitado/sem dinheiro para cada componente interativo?
5. Nenhum logotipo, sprite ou marca da Ninja Kiwi foi copiado?
6. Cada item do backlog aponta arquivo/função e esforço?
Só emita a resposta quando todas as respostas forem "sim".
</autoavaliacao>
```

## Notas de design

- **Contexto técnico explícito:** o Claude Design não vê o repositório; paleta, resolução,
  fontes e widgets foram copiados do código para que as propostas sejam implementáveis.
- **Arte por primitivas:** o jogo não usa imagens externas, então o prompt proíbe soluções que
  dependam de sprites e exige handoff em chamadas da raylib.
- **Referência sem cópia:** pede fidelidade ao estilo do BTDB2 sem reproduzir marcas da Ninja
  Kiwi.
- **Anexos:** para melhor resultado, anexar as capturas (F12) e os arquivos de `src/cliente/`.
