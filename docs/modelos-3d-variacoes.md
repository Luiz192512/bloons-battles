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
| bomba | 64 de 64 | 6.152 (2-0-5) | 2 | 6,0 MB | 1-0-0 muda pouco (cano 10% mais grosso); 4-0-0 e 5-0-0 se parecem de cima (o 5 é preto com cinta vermelha) |
| tachinha | 64 de 64 | 3.381 (2-0-5) | 2 | 5,3 MB | 0-0-1 e 0-0-2 (10 e 12 bicos) quase não se distinguem de 0-0-0 a 48 px; a engrenagem do topo lê como uma flor; 0-5-0 fica confuso de tantas lâminas |
| gelo | 64 de 64 | 8.468 (0-5-2) | 6 | 10,9 MB | os pingentes das costas (2-0-0) aparecem pouco de cima; azul claro, branco e ciano se misturam a 48 px; o tanque do canhão some atrás do corpo |
| cola | 64 de 64 | 7.898 (0-2-5) | 5 | 11,0 MB | 0-1-0 (gota maior) quase não se vê a 48 px; 0-3-0 e 0-4-0 se parecem de cima (o tanque fica atrás do corpo); 3-0-0 difere de 2-0-0 só pela máscara e pela pistola maior |
| sniper | 64 de 64 | 8.262 (0-2-5) | 5 | 10,8 MB | capacete e capuz verdes somem na grama; 1-0-0, 0-0-1 e 0-0-2 (ponteira e carregadores) são pequenos a 48 px |
| submarino | 64 de 64 | 1.304 (4-0-2) | 2 | 2,8 MB | 1-0-0 (periscópio mais alto) não se vê de cima; o reator fica escondido pelo radar; modelo simples, com folga grande no orçamento |
| bucaneiro | 64 de 64 | 9.304 (0-1-4) | 6 | 14,5 MB | a pilha de balas de uva (0-1-0, 0-2-0) fica atrás da vela na vista do jogo; as torres do destróier aparecem pouco; orçamento apertado, a base já usa 8.868 |
| as | 64 de 64 | 1.123 (0-2-5) | 2 | 3,1 MB | o abacaxi (0-1-0) é pequeno a 48 px; as bombas do 0-3-0 ficam sob a asa; modelo simples, com folga grande no orçamento |
| heli | 64 de 64 | 2.176 (0-2-5) | 2 | 4,4 MB | o corpo verde some um pouco na grama; os canhões e o tambor ficam sob o nariz e aparecem pouco de cima; no helicóptero de dois rotores a malha `torreta` tem um pivô só |
| morteiro | 64 de 64 | 8.726 (0-5-2) | 6 | 11,7 MB | o tubo da base é pequeno perto do macaco; 0-0-1 (mira ao lado do tubo) quase não se vê a 48 px; o macaco não encosta no morteiro |

Primárias concluídas (dardo, bumerangue, bomba, tachinha, gelo, cola). Militares, Mágicas e Suporte ainda não foram começadas.

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

## Bomba (Canhão Bomba)

Base: máquina sem macaco. Carreta de madeira com duas rodas (`base`) e o cano com o pavio aceso
(`torreta`). A torreta lê os três tiers: o caminho principal escolhe o tipo (canhão,
lança-mísseis ou feixe de canos) e os tiers cruzados enfeitam qualquer tipo.

| Caminho | Tier | Upgrade (nome no clone) | Encaixe | Peça | O que comunica |
|---|---|---|---|---|---|
| 1 | 1 | Bombas Maiores | torreta (parâmetro) | cano 10% mais grosso; como cruzado, aro dourado grosso na boca | explosão maior |
| 1 | 2 | Bombas Pesadas | torreta (parâmetro) | cano 18% mais grosso com cinta de aço; como cruzado, cinta de aço | mais dano |
| 1 | 3 | Bombas Muito Grandes | torreta | canhão curto e bem mais grosso, de cinta vermelha | bomba que empurra |
| 1 | 4 | Impacto Bloon | torreta, base | canhão maior com coroa de pontas amarelas na boca; carreta com chapas de aço | atordoa |
| 1 | 5 | Esmaga Bloon | torreta, base | canhão enorme preto de anéis dourados e pontas de ouro; carreta de aço com ouro | esmaga tudo |
| 2 | 1 | Recarga Rápida | extra | pilha de três bombas de reserva ao lado da carreta | recarrega rápido |
| 2 | 2 | Lança-Mísseis | torreta (parâmetro) | nariz vermelho de míssil saindo da boca de cada cano | míssil |
| 2 | 3 | Destruidor de M.O.A.B. | torreta, base | tubo lança-mísseis verde com o míssil na boca; carreta verde | caça dirigível |
| 2 | 4 | Assassino de M.O.A.B. | torreta, base | dois tubos lado a lado com anéis amarelos | mais dano em dirigível |
| 2 | 5 | Eliminador de M.O.A.B. | torreta, base | um míssil gigante preto de faixas amarelas e nariz vermelho, com aletas, sobre um trilho | o míssil definitivo |
| 3 | 1 | Alcance Extra | torreta (parâmetro) | cano 22% mais comprido (vale para qualquer tipo) | alcança mais longe |
| 3 | 2 | Bombas de Fragmentação | torreta (parâmetro) | coroa de cravos amarelos em volta do cano | solta fragmentos |
| 3 | 3 | Bombas de Cacho | torreta | feixe de três canos finos presos por um aro | mini bombas |
| 3 | 4 | Cacho Recursivo | torreta, base | feixe de cinco canos com cintas ciano e aro azul; chapas na carreta | cacho de cachos |
| 3 | 5 | Blitz de Bombas | torreta, base | feixe de sete canos vermelhos com ouro | chuva de bombas |

Composições resolvidas: em **2-0-5** cada um dos sete canos ganha o aro dourado e a cinta de aço;
em **0-2-4** cada cano do feixe leva o nariz de míssil; em **5-0-2** os cravos amarelos entram
atrás da coroa de pontas.

## Tachinha (Atirador de Tachinhas)

Base: máquina sem macaco. Prato de metal (`base`) e tambor vermelho de tampa dourada com oito
bicos em volta (`torreta`, gira no jogo). A torreta lê os três tiers, então engrenagens, bicos
compridos e número de bicos valem em qualquer combinação.

| Caminho | Tier | Upgrade (nome no clone) | Encaixe | Peça | O que comunica |
|---|---|---|---|---|---|
| 1 | 1 | Disparo Rápido | torreta (parâmetro) | engrenagem dourada no topo | mecanismo mais rápido |
| 1 | 2 | Disparo Mais Rápido | torreta (parâmetro) | segunda engrenagem, menor, por cima | ainda mais rápido |
| 1 | 3 | Tiros Quentes | torreta | bicos e engrenagens em laranja e cinta em brasa no tambor | tachinhas quentes |
| 1 | 4 | Anel de Fogo | torreta, base | tambor preto de tampa laranja; anel de doze chamas em volta do prato | anel de fogo |
| 1 | 5 | Anel Infernal | torreta, base | tampa amarela; dezesseis chamas maiores, vermelhas e amarelas | inferno |
| 2 | 1 | Tachinhas de Longo Alcance | torreta (parâmetro) | bicos 30% mais compridos | alcança mais longe |
| 2 | 2 | Tachinhas de Super Alcance | torreta (parâmetro) | bicos 60% mais compridos | mais alcance ainda |
| 2 | 3 | Atirador de Lâminas | torreta | tambor azul de tampa cinza; lâminas de aço deitadas no lugar dos bicos | lâminas |
| 2 | 4 | Turbilhão de Lâminas | torreta | lâminas maiores e uma lâmina grande girando no topo | turbilhão |
| 2 | 5 | Super Turbilhão | torreta | tambor preto, duas fileiras de lâminas douradas e lâmina dourada maior no topo | super turbilhão |
| 3 | 1 | Mais Tachinhas | torreta (parâmetro) | dez bicos | mais tachinhas |
| 3 | 2 | Ainda Mais Tachinhas | torreta (parâmetro) | doze bicos | ainda mais |
| 3 | 3 | Pulverizador de Tachinhas | torreta | dezesseis bicos num tambor mais alto | pulveriza |
| 3 | 4 | Sobrecarga | torreta | duas bobinas ciano em volta do tambor e antena | sobrecarga |
| 3 | 5 | Zona das Tachinhas | torreta, base | tambor dourado de tampa vermelha, duas fileiras de dezesseis bicos; prato maior de aro dourado | zona coberta |

## Gelo (Macaco de Gelo)

Base: o macaco padrão na única paleta diferente. As cores foram tiradas do print
`real_ice_0-0-0.jpg` e registradas em `PALETA` (`poli.py`): `pelo_gelo` (130, 200, 236),
`pele_gelo` (240, 250, 253) e `pelo_gelo_escuro` (50, 157, 202), esta última para o topete.
Topete, cachecol branco (`tronco`) e um cristal de gelo na mão (`mao_ataque`).

| Caminho | Tier | Upgrade (nome no clone) | Encaixe | Peça | O que comunica |
|---|---|---|---|---|---|
| 1 | 1 | Permafrost | extra | anel de gelo ciano no chão, com pontas brancas | o frio fica no chão |
| 1 | 2 | Estalo Frio | costas (alternativa: extra) | três pingentes de gelo saindo das costas; ou pingentes fincados no chão ao lado | gelo mais forte |
| 1 | 3 | Estilhaços de Gelo | mao_ataque, tronco | dois cristais menores junto do cristal da mão; ombreiras de cristal | estilhaços |
| 1 | 4 | Fragilização | chapeu, tronco | coroa de cristais ciano; colete ciano | fragiliza |
| 1 | 5 | Super Frágil | chapeu, costas, torreta | coroa branca maior; capa branca; quatro cristais grandes em órbita (malha `torreta`) | super frágil |
| 2 | 1 | Congelamento Melhor | mao_livre | segundo cristal, azul, na mão esquerda | congela mais |
| 2 | 2 | Congelamento Profundo | pes | botas azuis de sola branca | congela fundo |
| 2 | 3 | Vento Ártico | chapeu, extra | gorro azul de barra branca e pompom; anel largo de vento no chão | aura de vento |
| 2 | 4 | Nevasca | costas, chapeu | capa branca de borda azul; floco de neve branco sobre a cabeça | nevasca |
| 2 | 5 | Zero Absoluto | costas, chapeu, extra | capa azul; floco ciano maior; oito pontas de gelo em volta, no chão | zero absoluto |
| 3 | 1 | Raio Maior | mao_ataque (alternativa: extra) | cristal da mão 45% maior; com a mão ocupada, cristal grande fincado no chão | raio maior |
| 3 | 2 | Recongelar | rosto | viseira ciano | recongela |
| 3 | 3 | Canhão Criogênico | mao_ataque, costas | canhão azul de boca ciano com mangueira; tanque ciano nas costas | canhão de gelo |
| 3 | 4 | Pingentes | mao_ataque, costas, chapeu | canhão maior com pingente na boca; dois tanques; capacete azul | atira pingentes |
| 3 | 5 | Empalar com Pingentes | mao_ataque, chapeu, tronco | canhão preto e dourado com pingente grande; capacete e colete pretos com ouro | empala dirigíveis |

## Cola (Atirador de Cola)

Base: macaco padrão de pelo liso, capacete amarelo de barra laranja com uma lâmpada na frente
(`chapeu`, `extra`), pistola de cola com mangueira (`mao_ataque`) e tanque nas costas (`costas`).
A cor da cola e o tamanho da gota saem dos tiers e valem em toda peça que tem cola (lâmpada,
faixa da pistola, gota, tanque, poça, balde): amarelo, laranja (1), verde (2 e 3), roxo (4), rosa (5).

| Caminho | Tier | Upgrade (nome no clone) | Encaixe | Peça | O que comunica |
|---|---|---|---|---|---|
| 1 | 1 | Cola Encharcada | cor da cola (parâmetro) | cola laranja | cola que atravessa |
| 1 | 2 | Cola Corrosiva | cor da cola (parâmetro) | cola verde | corrói |
| 1 | 3 | Dissolvedor de Bloons | mao_ataque, costas, rosto | pistola maior; dois tanques; máscara de gás | dissolve |
| 1 | 4 | Liquefator de Bloons | costas, tronco, chapeu | cola roxa; tanques maiores; avental e capacete brancos | liquefaz |
| 1 | 5 | Solucionador de Bloons | mao_ataque, mao_livre, costas, tronco, chapeu, rosto | cola rosa; duas pistolas pretas; três tanques; traje, capacete e máscara pretos | dois globos por tiro |
| 2 | 1 | Globos Maiores | gota (parâmetro) | gota 45% maior na ponta da arma | globo maior |
| 2 | 2 | Respingo de Cola | extra | poça de cola no chão, com respingos em volta | respinga |
| 2 | 3 | Mangueira de Cola | mao_ataque, costas | mangueira grossa de bocal largo; tanque maior | jato contínuo |
| 2 | 4 | Ataque de Cola | mao_ataque, costas | mangueira maior; tanque grande de tampa vermelha com aro | habilidade |
| 2 | 5 | Tempestade de Cola | mao_ataque, costas, extra | tanque enorme; aspersor dourado sobre a cabeça, com gotas em volta | tempestade |
| 3 | 1 | Cola Mais Grudenta | tronco (alternativa: extra) | cinto atravessado com tubos de cola; ou latas no chão | dura mais |
| 3 | 2 | Cola Mais Forte | mao_livre (alternativa: extra) | balde de cola na mão esquerda; ou balde no chão | mais lento |
| 3 | 3 | Cola de M.O.A.B. | mao_ataque, chapeu | lançador de cola no ombro, com globo na boca; capacete laranja | cola dirigível |
| 3 | 4 | Cola Implacável | mao_ataque, chapeu, tronco | lançador maior de anéis vermelhos; capacete vermelho de ponta; colete | atordoa |
| 3 | 5 | Super Cola | mao_ataque, chapeu, costas | lançador preto e dourado; capacete preto; dois tanques dourados | para tudo |

## Sniper (Macaco Atirador)

Base: macaco padrão de pelo liso, capacete verde (`chapeu`) e rifle comprido com luneta
(`mao_ataque`). O rifle lê os três tiers e vale em qualquer combinação.

| Caminho | Tier | Upgrade (nome no clone) | Encaixe | Peça | O que comunica |
|---|---|---|---|---|---|
| 1 | 1 | Jaqueta Metálica | rifle (parâmetro) | ponteira de aço na boca do cano | estoura chumbo |
| 1 | 2 | Calibre Grosso | rifle (parâmetro) | cano mais grosso | mais dano |
| 1 | 3 | Precisão Mortal | rifle, chapeu | rifle maior com luneta grande; capuz verde | tiro certeiro |
| 1 | 4 | Mutilar M.O.A.B. | rifle, costas | freio de boca e bipé; capa verde | atordoa dirigível |
| 1 | 5 | Aleijar M.O.A.B. | rifle, chapeu, costas, tronco | rifle preto e vermelho, ainda maior; capuz, capa e colete pretos com vermelho | aleija dirigível |
| 2 | 1 | Óculos de Visão Noturna | rosto | óculos de aro verde | detecta camo |
| 2 | 2 | Tiro de Estilhaços | rifle (parâmetro) | boca em leque de três pontas laranja | estilhaços |
| 2 | 3 | Bala Ricochete | chapeu, tronco | boina vermelha; bandoleira | ricochete |
| 2 | 4 | Lançamento de Suprimentos | extra | caixa de suprimentos com paraquedas ao lado | dinheiro |
| 2 | 5 | Atirador de Elite | chapeu, extra, costas | boina preta com ouro; caixa dourada; capa azul; rifle preto e dourado | elite |
| 3 | 1 | Disparo Rápido | rifle (parâmetro) | carregador amarelo | cadência |
| 3 | 2 | Disparo Mais Rápido | rifle (parâmetro) | segundo carregador | mais cadência |
| 3 | 3 | Semiautomático | rifle, chapeu | tambor de munição; faixa vermelha na testa | semiautomático |
| 3 | 4 | Rifle Automático | rifle, tronco | rifle cinza; colete com cinto de munição dourado | automático |
| 3 | 5 | Defensor de Elite | rifle, chapeu, tronco, extra | dois tambores dourados; capacete e braços blindados com ombreiras douradas | defesa de elite |

## Submarino (Submarino Macaco)

Base: só o submarino, sem macaco. Casco amarelo de faixas azuis com hélice, leme e aletas
(`base`) e a torre com periscópio e um canhão (`torreta`). Tudo lê os três tiers.

| Caminho | Tier | Upgrade (nome no clone) | Encaixe | Peça | O que comunica |
|---|---|---|---|---|---|
| 1 | 1 | Alcance Maior | torreta (parâmetro) | periscópio mais alto | vê mais longe |
| 1 | 2 | Inteligência Avançada | torreta (parâmetro) | antena de radar branca na torre | ataca no mapa todo |
| 1 | 3 | Submergir e Apoiar | base | dois anéis de sonar ciano em volta do casco | pulso que tira camo |
| 1 | 4 | Reator de Bloontônio | base | casco cinza escuro de faixas e anéis verdes; reator verde no convés | radiação |
| 1 | 5 | Energizador | base | casco preto de faixas ciano; reator ciano maior | energiza |
| 2 | 1 | Dardos Farpados | base (parâmetro) | ponta de aço com farpas na proa | fura mais |
| 2 | 2 | Dardos Aquecidos | base (parâmetro) | a ponta da proa fica laranja | estoura chumbo |
| 2 | 3 | Míssil Balístico | base | dois tubos de míssil no convés, de nariz vermelho | mísseis |
| 2 | 4 | Capacidade de Primeiro Ataque | base | quatro mísseis; casco verde escuro de faixas amarelas | míssil gigante |
| 2 | 5 | Ataque Preventivo | base | seis mísseis; casco preto de faixas vermelhas | ataque preventivo |
| 3 | 1 | Canhões Gêmeos | torreta (parâmetro) | dois canhões na torre | atira em dobro |
| 3 | 2 | Dardos de Explosão Aérea | torreta (parâmetro) | ponta amarela em cada canhão | divide no ar |
| 3 | 3 | Canhões Triplos | torreta | três canhões | três canhões |
| 3 | 4 | Dardos Perfurantes | torreta, base | canhões de aço mais longos; casco cinza com placas de blindagem | perfura |
| 3 | 5 | Comandante Submarino | torreta, base | casco azul de faixas douradas; estrela dourada na torre | comando |

## Bucaneiro (Macaco Bucaneiro)

Base: a do piloto. Macaco padrão de tufos, lenço vermelho, tapa-olho e luneta, em pé no convés
de um veleiro com quatro canhões de bordo. O barco lê os três tiers; como a base já usa 8.868
triângulos, os enfeites cruzados são pequenos e os trajes dos tiers altos trocam o lenço (a peça
mais pesada da cabeça) por um chapéu mais leve.

| Caminho | Tier | Upgrade (nome no clone) | Encaixe | Peça | O que comunica |
|---|---|---|---|---|---|
| 1 | 1 | Disparo Rápido | base (parâmetro) | flâmula amarela no mastro | cadência |
| 1 | 2 | Tiro Duplo | base (parâmetro) | canhão de proa | tiro em dobro |
| 1 | 3 | Destróier | base, torreta, chapeu | casco de aço sem vela, chaminé e mastro de radar; duas torres de canhão duplo; quepe branco | navio de guerra |
| 1 | 4 | Porta-Aviões | base, chapeu | pista com linha amarela, ilha de comando e um avião pousado; o macaco sobe para a pista | aviões |
| 1 | 5 | Nau Capitânia | base, chapeu | casco azul e dourado, pista preta, ilha dourada mais alta e dois aviões; quepe com ouro | nau capitânia |
| 2 | 1 | Tiro de Uva | base (parâmetro) | pilha de balas roxas na popa | tiro em leque |
| 2 | 2 | Tiro Quente | base (parâmetro) | as balas ficam laranja | fogo |
| 2 | 3 | Navio Canhão | base, torreta | vela vermelha de faixa preta; canhão grande na proa (malha `torreta`) | bombas |
| 2 | 4 | Macacos Piratas | base, torreta, chapeu | vela preta de faixa amarela, bandeira vermelha, arpão na proa; chapéu preto de aba com pena | piratas |
| 2 | 5 | Senhor Pirata | base, torreta, chapeu | casco preto e dourado, vela vermelha, segundo canhão na popa; chapéu com ouro | senhor pirata |
| 3 | 1 | Longo Alcance | base (parâmetro) | bandeira azul comprida no alto do mastro | alcance |
| 3 | 2 | Ninho do Corvo | base (parâmetro) | cesto grande com luneta | vê camo |
| 3 | 3 | Navio Mercante | base | vela verde de faixa dourada; caixas de carga na popa | comércio |
| 3 | 4 | Comércio Favorecido | base, chapeu | casco branco de faixa azul; mais carga e baú de ouro; lenço verde | mais renda |
| 3 | 5 | Império Comercial | base, chapeu | casco branco e dourado, vela azul; pilhas de ouro; coroa | império |

## Ás (Macaco Ás)

Base: só o avião, sem macaco. Monomotor vermelho de pontas amarelas, com fuselagem, asas e cauda
(`base`) e a hélice (`torreta`, gira no jogo). Tudo lê os três tiers.

| Caminho | Tier | Upgrade (nome no clone) | Encaixe | Peça | O que comunica |
|---|---|---|---|---|---|
| 1 | 1 | Tiro Rápido | base (parâmetro) | uma metralhadora em cada asa | cadência |
| 1 | 2 | Muito Mais Dardos | base (parâmetro) | duas metralhadoras em cada asa | mais dardos |
| 1 | 3 | Avião de Caça | base | caça a jato cinza de asa enflechada, sem hélice, com um míssil por asa | mísseis |
| 1 | 4 | Operação: Tempestade de Dardos | base | caça azul maior, de duas derivas e dois mísseis por asa | tempestade |
| 1 | 5 | Retalhador Celeste | base | caça preto e dourado, o maior, com três mísseis por asa e dois bocais | retalhador |
| 2 | 1 | Abacaxi Explosivo | base (parâmetro) | abacaxi preso em cima da fuselagem | abacaxis |
| 2 | 2 | Avião Espião | base (parâmetro) | domo de radar branco | vê camo |
| 2 | 3 | Ás Bombardeiro | base | bombardeiro verde escuro de asa longa, com uma bomba por asa | bombardeio |
| 2 | 4 | Marco Zero | base | bombardeiro maior com uma bomba grande sob a barriga | bomba na tela toda |
| 2 | 5 | Tsar Bomba | base, torreta | bombardeiro preto e vermelho com bomba enorme e motores nas asas | a maior bomba |
| 3 | 1 | Dardos Mais Afiados | base (parâmetro) | ponta de aço no nariz | fura mais |
| 3 | 2 | Rota Centralizada | base (parâmetro) | alvos branco e ciano pintados nas asas | rota fixa |
| 3 | 3 | Mira Infalível | base | avião azul de pontas brancas com sensor ciano no nariz | teleguiado |
| 3 | 4 | Espectro | base, torreta | canhoneira cinza escura, com um motor por asa e canhões laterais | rajada |
| 3 | 5 | Fortaleza Voadora | base, torreta | asa enorme cinza e dourada, quatro motores | fortaleza |

## Heli (Piloto de Helicóptero)

Base: só o helicóptero, sem macaco. Cabine verde de vidro ciano, cauda e esquis (`base`) e o
rotor de duas pás (`torreta`, gira no jogo). Tudo lê os três tiers.

| Caminho | Tier | Upgrade (nome no clone) | Encaixe | Peça | O que comunica |
|---|---|---|---|---|---|
| 1 | 1 | Dardos Quádruplos | base (parâmetro) | quatro canhões sob o nariz em vez de dois | quatro dardos |
| 1 | 2 | Perseguição | base (parâmetro) | farol amarelo no nariz | persegue |
| 1 | 3 | Hélices Navalha | torreta | rotor maior, de quatro pás de aço com pontas vermelhas | hélice que corta |
| 1 | 4 | Apache Dardeiro | base, torreta | casco cinza escuro com asas curtas e casulos de foguete | helicóptero de ataque |
| 1 | 5 | Apache Prime | base, torreta | casco preto com luz ciano, casulos de laser e rotor de cinco pás | lasers |
| 2 | 1 | Jatos Maiores | base (parâmetro) | dois bocais com chama dos lados | voa mais rápido |
| 2 | 2 | IFR | torreta (parâmetro) | domo de radar branco por cima do rotor | vê camo |
| 2 | 3 | Corrente Descendente | torreta | rotor bem largo, de seis pás com pontas brancas | sopra os bloons |
| 2 | 4 | Chinook de Apoio | base, torreta | corpo comprido verde escuro com dois rotores; caixa de carga pendurada | carga e dinheiro |
| 2 | 5 | Operações Especiais | base, torreta | o mesmo em preto e dourado, com caixa dourada | operações especiais |
| 3 | 1 | Dardos Rápidos | base (parâmetro) | canhões mais longos, de aço | alcance |
| 3 | 2 | Disparo Rápido | base (parâmetro) | tambor de munição amarelo | cadência |
| 3 | 3 | Empurrão de M.O.A.B. | base, torreta | aríete de aço no nariz e dois mísseis; rotor de três pás | empurra dirigível |
| 3 | 4 | Defesa Comanche | base, torreta | casco azul mais fino; rotor de quatro pás | comanche |
| 3 | 5 | Comandante Comanche | base | aríete dourado e dois helicópteros pequenos de escolta | comanda a escolta |

## Morteiro (Macaco Morteiro)

Base: macaco padrão de pelo liso, capacete cinza de barra vermelha (`chapeu`) e um projétil na
mão (`mao_ataque`), em pé atrás do morteiro: placa e bipé (`base`) e o tubo (`torreta`). O
morteiro lê os três tiers.

| Caminho | Tier | Upgrade (nome no clone) | Encaixe | Peça | O que comunica |
|---|---|---|---|---|---|
| 1 | 1 | Explosão Maior | torreta (parâmetro) | tubo 18% mais grosso | explosão maior |
| 1 | 2 | Destruidor de Bloons | torreta (parâmetro) | tubo 30% mais grosso, de anéis vermelhos | mais dano |
| 1 | 3 | Choque de Projéteis | torreta, tronco | tubo maior de anéis amarelos; colete | atordoa |
| 1 | 4 | A Grande | torreta, base, tronco, chapeu | tubo enorme preto e dourado sobre carreta com rodas; capacete e colete pretos | a grande |
| 1 | 5 | A Maior de Todas | torreta, base, tronco, chapeu | tubo ainda maior, vermelho e dourado; capacete dourado | a maior |
| 2 | 1 | Recarga Rápida | base (parâmetro) | pilha de três projéteis ao lado | recarga |
| 2 | 2 | Recarga Veloz | base (parâmetro) | pilha de seis projéteis | mais recarga |
| 2 | 3 | Projéteis Pesados | base, torreta, tronco | projéteis de aço com ponta dourada; tubo cinza; bandoleira | estoura tudo |
| 2 | 4 | Bateria de Artilharia | torreta, tronco, chapeu | três tubos verdes numa placa larga; capacete e colete verdes | bateria |
| 2 | 5 | Choque e Pavor | torreta, tronco, chapeu | cinco tubos pretos de anéis vermelhos; traje preto | choque e pavor |
| 3 | 1 | Precisão Aumentada | torreta (parâmetro) | mira com lente ciano ao lado do tubo | precisão, vê camo |
| 3 | 2 | Coisas Queimando | torreta (parâmetro) | chama na boca do tubo; projétil laranja na mão | deixa fogo |
| 3 | 3 | Sinalizador | torreta, rosto | antena com luz vermelha; óculos laranja | tira camo |
| 3 | 4 | Projéteis Estilhaçantes | torreta, chapeu, tronco | tubo marrom de anéis laranja com chama maior; capacete laranja e colete | queima mais |
| 3 | 5 | Bloonflagração | torreta, chapeu, tronco | tubo vermelho com chama grande; traje vermelho e amarelo | fogo intenso |
