# Variações de upgrade dos modelos 3D, montadas por partes

Relatório da tarefa `prompts/modelar-todas-variacoes-por-partes.md`. Cada torre tem 64 variações
(um caminho até 5, outro até 2, o terceiro em 0), e nenhuma é modelada à mão: todas saem do
montador `tools/blender/partes.py`, que compõe as peças declaradas em
`tools/blender/torres/<chave>.py`.

## Como gerar e conferir

```
"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe" --background --python tools/blender/gerar_variacoes.py -- dardo
python tools/blender/conferir_variacoes.py dardo
```

- Sem combinação depois da chave, gera as 64, a folha de contato
  (`docs/design/capturas/modelos/<chave>_variacoes.png`, 8x8, vista do jogo) e a mesma folha no
  tamanho do mapa (`<chave>_variacoes_48.png`, 48 px por unidade).
- Com combinações (`-- dardo 3-2-0 0-0-5`), gera só essas e as vistas de revisão em
  `dist/modelos/<chave>/` (fora do git).
- O `conferir_variacoes.py` lê os `.glb` e imprime a linha de estado da torre; sai com erro se
  alguma variação passar de 10.000 triângulos ou de 6 malhas, ficar sem cor ou se o `.json` não
  bater com a ordem das malhas.

## Regras de composição

- Encaixes: `pelagem`, `chapeu`, `rosto` (malha `cabeca`); `tronco`, `costas`, `mao_livre`,
  `pes` (malha `corpo`); `mao_ataque` (malha `braco`); `base`, `torreta`, `extra`.
- O caminho principal é o de tier mais alto (em empate, o de menor índice). Os tiers dele são
  aplicados em ordem, do 1 até o atingido.
- Os tiers 1 e 2 de cada caminho são acessórios, com uma lista de encaixes alternativos. Os
  tiers 3 a 5 são conjuntos (traje, arma, silhueta).
- O caminho cruzado só chega aos tiers 1 e 2. Cada tier entra no primeiro encaixe da lista que
  o principal não ocupou.
- As peças podem ler os tiers para se ajustar. Isso é regra da torre, não caso solto: no dardo,
  toda ponta de dardo ou de virote fica de aço quando o caminho 1 tem tier.

## Estado

| Torre | Variações geradas | Máx. de triângulos | Malhas | Tamanho total | Pendências |
|---|---|---|---|---|---|
| dardo | 64 de 64 | 8.929 (2-0-5) | 6 | 12,7 MB | 0-0-1 e 3-1-0 mudam pouco a 48 px; o capuz é largo para cobrir as orelhas |
| bumerangue | 64 de 64 | 8.876 (5-2-0) | 5 | 10,8 MB | 1-0-0 muda pouco (só o gume e as pontas de aço); o bumerangue lê como um arco; a mochila do 0-4-0 some na vista do jogo |
| bomba | 1 de 64 (só a base) | 1.984 | 2 | 0,07 MB | upgrades entram na Parte B |
| bucaneiro | 1 de 64 (só a base) | 8.868 | 5 | 0,23 MB | upgrades entram na Parte B; a base já usa 8.868 triângulos, sobra pouco para peças |

As outras torres ainda não foram começadas.

Tamanho no repositório: o dardo ocupa 12,7 MB (de 139 a 250 KB por variação), abaixo do limite
de 30 MB por torre. Projetando para as 22 torres, o total fica entre 250 e 330 MB de `.glb`.

Migração do piloto: a variante 0-0-0 de dardo, bomba e bucaneiro sai do montador com a mesma
contagem de triângulos e o mesmo tamanho de arquivo do piloto. Nas folhas de revisão, bomba e
bucaneiro saíram idênticas pixel a pixel; no dardo a maior diferença é de 4 em 255 em poucos
pixels (arredondamento de posição do dardo na mão).

## Dardo (Macaco Dardo)

Base: macaco padrão de topete, lenço azul (`tronco`) e um dardo na mão (`mao_ataque`).

| Caminho | Tier | Upgrade (nome no clone) | Encaixe | Peça | O que comunica |
|---|---|---|---|---|---|
| 1 | 1 | Tiros Afiados | mao_livre | um dardo de aço de reserva na mão esquerda; a ponta do dardo da mão do ataque vira aço claro | dardo que fura mais |
| 1 | 2 | Tiros Super Afiados | mao_livre | feixe de três dardos de aço e bracelete de espinhos | ainda mais perfuração |
| 1 | 3 | Espinhopulta | base, torreta, chapeu | catapulta de madeira com quatro rodas (base); braço claro e grosso com contrapeso, colher e bola de espinhos (torreta); o macaco vai para trás da máquina e segura a alavanca do disparo; capacete de couro | lança bolas de espinhos |
| 1 | 4 | Juggernaut | base, torreta, chapeu | a mesma catapulta com chapas de aço, bola maior e escura; capacete de aço com ponta | bola gigante que fura chumbo |
| 1 | 5 | Ultra-Juggernaut | base, torreta, chapeu | catapulta de aço escuro com ouro, bola enorme preta de espinhos dourados e faixa vermelha; capacete dourado com crista vermelha | a bola que se divide |
| 2 | 1 | Tiros Rápidos | pes (alternativa: extra) | tênis vermelhos de sola branca; na catapulta, listras vermelhas na carreta | velocidade |
| 2 | 2 | Tiros Muito Rápidos | chapeu (alternativa: tronco) | faixa vermelha na testa com pontas ao vento; com o chapéu ocupado, cachecol vermelho no lugar do lenço | mais velocidade |
| 2 | 3 | Tiro Triplo | mao_ataque, tronco | leque de três dardos na mão; bandoleira amarela com dardos | três dardos por tiro |
| 2 | 4 | Fã-Clube Super Macaco | costas, tronco | capa azul com pregas, gola e broche, de borda amarela; estrela dourada no peito no lugar do lenço e da bandoleira | fã de herói, habilidade |
| 2 | 5 | Fã-Clube Macaco Plasma | costas, tronco, extra | capa roxa maior de borda ciano; estrela ciano; aro de energia sobre a cabeça; pontas dos dardos em ciano | plasma |
| 3 | 1 | Dardos de Longo Alcance | mao_ataque (alternativas: costas, mao_livre) | dardo de haste comprida e penas maiores; com a mão ocupada, vai atravessado nas costas | alcança mais longe |
| 3 | 2 | Visão Aprimorada | rosto | óculos redondos de aro dourado | enxerga camo |
| 3 | 3 | Besta | mao_ataque, chapeu | besta curta de madeira; capuz roxo que cobre as orelhas e emoldura o rosto | tiro forte |
| 3 | 4 | Atirador Afiado | mao_ataque, chapeu, costas | besta maior com luneta; capuz com barra e pena amarelas; aljava | precisão, crítico |
| 3 | 5 | Mestre da Besta | mao_ataque, chapeu, costas | besta grande preta e dourada de arco duplo; capuz preto com ouro; aljava dourada e capa curta preta | o melhor atirador |

Composições resolvidas:

- **3-2-0**: principal é o caminho 1 no tier 3 (catapulta: ocupa base, torreta, chapeu e pes). Do
  caminho 2 entram as listras vermelhas na carreta (tier 1, porque os pés ficam escondidos) e o
  cachecol vermelho (tier 2, porque o capacete ocupa o chapéu).
- **0-2-5**: principal é o caminho 3 no tier 5 (besta, capuz, aljava e capa). Do caminho 2
  entram os tênis e o cachecol vermelho (o capuz ocupa o chapéu).
- **5-0-1**: o dardo comprido não cabe na mão (a catapulta tirou a arma) e vai nas costas.
- **1-1-0**: empate, vale o caminho 1 como principal: dardo de reserva na mão esquerda, mais os
  tênis do caminho 2.

Ajustes feitos depois da primeira revisão do dono:

- Catapulta: o braço ficou grosso e de cor clara (bege, aço ou ouro conforme o tier), ganhou
  contrapeso na frente e colher mais larga, e a mão direita do macaco segura a alavanca do disparo.
- Capa: deixou de ser uma folha plana. Cai dos ombros, abre em leque para trás e faz pregas, com
  gola e broche no pescoço.
- Capuz: virou capuz de verdade (`pecas.capuz`), uma casca em volta da cabeça toda, que cobre as
  orelhas e abre num oval em volta do rosto.
- Ponta de aço: cinza azulado em vez de quase branco, e só 8% maior por tier.
- Caminho 2, tiers 4 e 5: o traje do fã-clube troca o lenço e a bandoleira pela capa com gola e
  pela estrela; ficam a faixa da testa, a capa, a estrela e, no tier 5, o aro de energia.

O que ainda não ficou bom (para o ajuste de design):

- 0-0-1 muda pouco em relação a 0-0-0 a 48 px (só o dardo mais comprido).
- 3-0-0 e 3-1-0 se distinguem só pelas listras vermelhas na carreta, que são pequenas a 48 px.
- O capuz precisa ser largo para caber as orelhas, e a cabeça fica grande na vista do jogo.
- A capa, vista de lado, ainda é fina; as pregas aparecem melhor de costas e de cima.

## Bumerangue (Macaco Bumerangue)

Base: macaco padrão de crista, cinto laranja (`tronco`) e um bumerangue de madeira na mão
(`mao_ataque`). A arma lê os três tiers: o caminho 1 dá o gume e a glaive, o caminho 3 dá o
tamanho, a brasa e o kylie. Por isso os tiers 1 e 2 desses caminhos não ocupam encaixe e valem
em qualquer combinação.

| Caminho | Tier | Upgrade (nome no clone) | Encaixe | Peça | O que comunica |
|---|---|---|---|---|---|
| 1 | 1 | Bumerangues Melhorados | mao_ataque (parâmetro) | gume e pontas de aço no bumerangue | fura mais |
| 1 | 2 | Glaives | mao_ataque (parâmetro) | o bumerangue vira uma glaive: aro de lâminas com miolo vermelho | lâmina giratória |
| 1 | 3 | Ricochete de Glaive | mao_ataque, chapeu | glaive maior, de oito lâminas; elmo de aço com crista de lâminas | ricocheteia |
| 1 | 4 | M.O.A.R. Glaives | mao_livre, costas | segunda glaive na mão esquerda e uma grande nas costas | muito mais glaives |
| 1 | 5 | Senhor das Glaives | torreta, chapeu, costas | três glaives douradas em órbita (malha `torreta`, gira no jogo); elmo dourado; capa vermelha | glaives orbitando |
| 2 | 1 | Arremesso Rápido | chapeu (alternativas: tronco, mao_livre) | faixa laranja na testa; ou cachecol laranja; ou munhequeira | velocidade |
| 2 | 2 | Bumerangues Velozes | pes | tênis laranja | mais velocidade |
| 2 | 3 | Bumerangue Biônico | extra (malha braco), rosto | braço direito mecânico com anel ciano; olho mecânico ciano | braço robótico |
| 2 | 4 | Turbo Carga | costas, mao_livre | mochila turbina de jato ciano; braço esquerdo mecânico | habilidade turbo |
| 2 | 5 | Carga Permanente | tronco, chapeu, costas | peitoral e capacete de metal com luz ciano; turbina maior de jato laranja | turbo permanente |
| 3 | 1 | Bumerangues de Longo Alcance | mao_ataque (parâmetro) | arma 35% maior | alcança mais longe |
| 3 | 2 | Bumerangues Incandescentes | mao_ataque (parâmetro) | arma em brasa: laranja de pontas amarelas | estoura chumbo |
| 3 | 3 | Bumerangue Kylie | mao_ataque, chapeu | kylie vermelho de faixas brancas, de cabo reto e cabeça virada; chapéu de aba marrom | linha reta |
| 3 | 4 | Prensa de M.O.A.B. | mao_ataque, tronco | kylie maior com peso de aço; colete escuro | empurra dirigíveis |
| 3 | 5 | Dominação M.O.A.B. | mao_ataque, chapeu, tronco, costas | kylie duplo com pesos dourados; chapéu e colete pretos com ouro; capa vermelha | domina dirigíveis |

Composições resolvidas: em **2-0-5** o kylie ganha dentes de serra de aço (o caminho 1 afia a arma
que estiver na mão); em **0-1-4** a faixa da testa não cabe (chapéu) nem o cachecol (colete), e o
tier vira munhequeira; em **5-0-2** as glaives ficam em brasa.
