# Aparências dos heróis por estágio de nível

Levantamento para os modelos 3D dos 18 heróis (`tools/blender/herois.py`). Feito em 05/10/2026.

## Como ler

- **Estágio**: uma aparência do herói. O estágio 0 é o do nível 1. O arquivo do estágio `e` será
  `assets/modelos/<chave>/<e>-0-0.glb`.
- **No original (observado)**: o que o BTD6 mostra naquele trecho de níveis. Serve para saber a
  função da mudança, nunca como traje a copiar.
- **No nosso modelo**: o que vamos modelar, com o figurino do clone e as peças do montador.
- Níveis fundidos: quando o original muda em dois níveis próximos e uma das mudanças não se lê a
  48 px (um brinco, uma flecha a mais), as duas viram um estágio só. A coluna do original cita os
  dois níveis.

## Fontes

- Blooncyclopedia (bloonswiki.com), lida pela API em 05/10/2026: seção "Appearance" da página de
  cada herói (por exemplo "Quincy") e a galeria de retratos por nível da página "<Herói> (BTD6)",
  que dá as faixas de nível.
- Os retratos de cada faixa foram vistos um a um. Não estão no repositório.
- `src/jogo/dados.cpp`: o ataque e o que cada herói ganha por nível no clone.
- Identidade do nosso modelo: `tools/blender/herois.py` (que segue o herói 2D do clone).

## Tabela resumo

| Herói (chave) | Papel em uma frase | Estágios | Níveis de troca | Fonte | Não confirmado |
|---|---|---|---|---|---|
| Quincy (quincy) | arqueiro de capuz | 4 | 3, 10, 20 | "Quincy", Appearance | nada |
| Gwendolin (gwendolin) | lança-chamas | 4 | 3, 10, 20 | "Gwendolin", Appearance | nada |
| Striker Jones (striker) | comandante de bazuca | 4 | 3, 10, 20 | "Striker Jones", Appearance | nada |
| Obyn Guardião (obyn) | druida da floresta com cajado | 4 | 3, 10, 20 | "Obyn Greenfoot", Appearance | nada |
| Capitão Churchill (churchill) | tanque | 5 | 3, 5, 10, 20 | "Captain Churchill", Appearance | nada |
| Benjamin (benjamin) | hacker de laptop | 4 | 3, 10, 20 | "Benjamin", Appearance | nada |
| Ezili (ezili) | sacerdotisa de cajado e caveira | 5 | 3, 10, 16, 20 | "Ezili", Appearance | nada |
| Pat Fusty (pat) | gigante que bate com os punhos | 4 | 3, 10, 20 | "Pat Fusty", Appearance | nada |
| Adora (adora) | sacerdotisa do sol | 4 | 3, 10, 20 | "Adora", Appearance | nada |
| Almirante Brickell (brickell) | comandante naval de pistola | 4 | 3, 10, 20 | galeria de "Admiral Brickell (BTD6)" | o texto: a wiki marca a seção como incompleta. Descrição feita por mim a partir dos retratos |
| Etienne (etienne) | piloto de drones | 4 | 3, 10, 20 | "Etienne", Appearance | nada |
| Sauda (sauda) | espadachim de duas espadas | 4 | 3, 10, 20 | galeria de "Sauda (BTD6)" | o texto: a seção está vazia na wiki. Descrição feita por mim a partir dos retratos |
| Psi (psi) | psíquico que flutua | 4 | 3, 10, 20 | "Psi", Appearance | nada |
| Geraldo (geraldo) | mercador de mochila enorme | 4 | 3, 10, 20 | "Geraldo", Appearance | nada |
| Corvus (corvus) | mago sombrio com espíritos | 4 | 3, 10, 20 | "Corvus", Appearance | nada |
| Rosalia (rosalia) | engenheira de jetpack | 4 | 7, 13, 20 | "Rosalia", Appearance | nada |
| Dan D'Monke (dan) | esgrimista de capa | 4 | 3, 10, 20 | "Dan D'Monke", Appearance | nada |
| Silas (silas) | mago do gelo | 4 | 3, 10, 20 | "Silas", Appearance | nada |

Total: 74 estágios (18 bases refeitas e 56 estágios novos).

Os níveis 3 e 10 coincidem com o desbloqueio das duas habilidades do herói no clone, então a
troca de visual também avisa que a habilidade chegou.

## Decisões para o dono antes de modelar

1. **Brickell de pistola.** O modelo atual segura um sabre, mas o ataque dela no clone é bala
   (`visual: bala`) e no original é pistola. Proposta: pistola na mão de ataque e o sabre na
   cintura. Se preferir manter o sabre na mão, os estágios trocam a pistola pelo sabre.
2. **Corvus com livro.** Hoje ele tem lança e três espíritos. Proposta: manter a lança e dar o
   livro de feitiços na mão livre a partir do estágio 1, porque o ataque dele é o espírito.
3. **Psi azul no nível 20.** No original o pelo muda de cor no último estágio. Proposta: trocar o
   lilás pelo ciano só no estágio 3. É a única troca de cor de pelo da lista.
4. **Forma mascarada do Dan.** No original ele alterna com o Macaco Mascarado. Fica fora deste
   levantamento: o clone desenha um modelo por herói.
5. **Barcos, cadeiras e tapetes.** No original Brickell fica num navio, Benjamin numa cadeira e
   Psi num tapete. Aqui todos ficam em pé no chão, como hoje. Só o Churchill tem veículo.

## Fichas

### Quincy

Identidade fixa: capuz verde-escuro com barra dourada, pelagem lisa, arco na mão e aljava nas costas.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Capuz cinza com detalhes laranja, arco composto, aljava, cinto com bolsas | Base refeita: arco recurvo maior com empunhadura, flecha armada, aljava com duas flechas, bandoleira e cinto | chapeu, costas, tronco, mao_ataque | é o arqueiro |
| 1 | 3 a 9 | Nível 3: uma flecha a mais na aljava. Nível 7: a pena dela ganha listras | Aljava cheia (cinco flechas de pena vermelha e amarela), braçadeira de couro no braço do arco, flecha armada de ponta vermelha | costas, tronco, mao_ataque | mais munição; a flecha explosiva |
| 2 | 10 a 19 | O capuz ganha duas asas | Duas penas grandes dos lados do capuz, arco longo com reforço dourado nas pontas, ombreira de couro | chapeu, mao_ataque, tronco | atirador de elite |
| 3 | 20 | Viseira que cobre o rosto com lentes laranja; arco laranja | Máscara de lente laranja, arco com lâminas douradas, três flechas armadas em leque, capa curta verde | rosto, mao_ataque, costas | o melhor arqueiro |

### Gwendolin

Identidade fixa: crista laranja em forma de chama, óculos na testa, colete branco, tanque vermelho nas costas e lança-chamas.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Cabelo de chama, macacão azul, lança-chamas cinza | Base refeita: lança-chamas com bocal e chama piloto curta, tanque pequeno, óculos | pelagem, rosto, tronco, costas, mao_ataque | cospe fogo |
| 1 | 3 a 9 | Mochila vermelha e um coquetel na mão | Frasco de fogo na mão livre, tanque duplo com mangueira até a arma | mao_livre, costas | o Coquetel de Fogo |
| 2 | 10 a 19 | Bocal com pintura de chamas amarela e laranja | Bocal largo laranja com anel amarelo, chama de três línguas, cinto com frascos | mao_ataque, tronco | fogo mais forte |
| 3 | 20 | Viseira de proteção com chamas; cabelo em rabo | Máscara de solda escura com faixa de chamas, crista mais alta, chama de miolo azul, aro de brasas no chão | rosto, pelagem, mao_ataque, extra | a Tempestade de Fogo no máximo |

### Striker Jones

Identidade fixa: boina verde com botão dourado, colete verde com estrela, bandoleira e bazuca no ombro.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Quepe e uniforme verdes, bigode, bazuca de boca larga | Base refeita: bazuca de tubo com punho e boca larga, boina, colete com estrela | chapeu, tronco, mao_ataque | artilheiro |
| 1 | 3 a 9 | Divisa de cabo na manga | Ombreiras com divisas douradas e cinturão de granadas | tronco | subiu de patente |
| 2 | 10 a 19 | Bazuca retangular com luneta, fones verdes, divisa de sargento | Lançador de caixa com luneta, fones por cima da boina | mao_ataque, chapeu | o Comando de Artilharia |
| 3 | 20 | Luneta com lente infravermelha, insígnia de estrela e louros | Lançador duplo, luneta de lente vermelha, medalhas no peito, mochila com foguetes à mostra | mao_ataque, tronco, costas | comandante máximo |

### Obyn Guardião

Identidade fixa: barba verde, chifres de galho com coroa de folhas, capa verde e cajado com orbe verde.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Pelo azul, galhos na cabeça, faixas nas mãos | Base refeita: chifres curtos, coroa de folhas, capa curta, cajado de orbe pequeno | chapeu, costas, tronco, mao_ataque | guardião da floresta |
| 1 | 3 a 9 | Folhas nos braços, galhos um pouco maiores | Chifres maiores com brotos, braçadeiras de folhas | chapeu, tronco | a floresta cresce nele |
| 2 | 10 a 19 | Elmo pequeno de madeira, folhas com brilho azul | Elmo de casca de árvore por baixo da coroa, orbe maior de brilho ciano, ombreiras de folhas | chapeu, mao_ataque, tronco | a Muralha de Árvores |
| 3 | 20 | Arbustos nos galhos, folhas nos ombros, elmo fechado, mãos brilhando | Copa de árvore sobre os chifres, capa longa de folhas, dois espíritos verdes em volta, aro de raízes no chão | chapeu, costas, extra | virou a própria floresta |

### Capitão Churchill

Identidade fixa: tanque verde com casco na base, torre com canhão e a cabeça do capitão de capacete na escotilha.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Tanque verde, canhão cinza, antena de ponta vermelha | Base refeita: casco com esteiras e rodas, torre, canhão, antena, capitão de capacete e óculos | base, torreta | é um tanque |
| 1 | 3 a 4 | Cano maior e pintado de verde | Canhão mais grosso e longo, com freio de boca | torreta | tiro mais pesado |
| 2 | 5 a 9 | Nível 5: metralhadora de mão. Nível 6: lentes vermelhas e visor de mira | Metralhadora na escotilha, óculos do capitão de lente vermelha | torreta | a metralhadora |
| 3 | 10 a 19 | Capacete camuflado, tanque mais robusto, segundo canhão atrás da torre | Casco mais largo com saias blindadas, canhão duplo, capacete de duas cores | base, torreta | a Barragem de MOAB |
| 4 | 20 | Tanque preto com detalhes vermelhos | Pintura escura com faixas e rodas vermelhas, lançador de foguetes atrás da torre | base, torreta | tanque de elite |

### Benjamin

Identidade fixa: topete, fones azuis, óculos ciano, colete cinza-escuro e laptop.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Camisa branca, gravata, laptop cinza | Base refeita: laptop aberto com a tela acesa, fones, colete | chapeu, rosto, tronco, mao_livre | o hacker |
| 1 | 3 a 9 | Nível 3: pulseira com telas. Nível 5: óculos na testa. Nível 7: antena, pratinho de satélite e cadeado no laptop | Laptop com antena e pratinho de satélite, pulseira de tela verde | mao_livre, tronco | invade mais longe |
| 2 | 10 a 19 | Óculos escuros no rosto, tablet, servidor grande atrás, fica de pé | Torre de servidor atrás dele com parabólica, tablet no lugar do laptop, óculos escuros | extra, mao_livre, rosto | o Desvio de Fundos |
| 3 | 20 | Visor verde, tela holográfica, traje preto e verde, segundo servidor | Dois servidores, tela curva verde na frente, visor verde, traje com linhas verdes | extra, mao_livre, rosto, tronco | dono da rede |

### Ezili

Identidade fixa: pelo com mechas roxas, faixa escura com crânio na testa, capa e saia roxas, cajado.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Pelo roxo, túnica violeta, um bastão simples | Base refeita: cajado de madeira com orbe pequeno, capa curta, crânio na faixa | chapeu, costas, tronco, mao_ataque | a feiticeira |
| 1 | 3 a 9 | Nível 3: pingente de coração no bastão. Nível 7: cabeça de caveira no bastão | Cajado com caveira no topo e pingente | mao_ataque | magia mais sombria |
| 2 | 10 a 15 | Cabelo mais escuro, olhos da caveira acesos em verde | Olhos da caveira acesos, garra maior no cajado, colar de ossos | mao_ataque, tronco | o Feitiço de MOAB |
| 3 | 16 a 19 | Cicatrizes e pupilas verdes, faixas brancas nas mãos, caveira branca | Faixas brancas nos braços, caveira branca maior, dois fios de fumaça verde em volta | tronco, mao_ataque, extra | poder transbordando |
| 4 | 20 | Máscara de madeira, rabo de cabelo longo, segunda caveira no bastão | Máscara de madeira com olhos verdes, cajado de duas caveiras, capa longa de barra verde, três espíritos | rosto, mao_ataque, costas, extra | sacerdotisa suprema |

### Pat Fusty

Identidade fixa: o único maior que os outros, pelagem de tufos, faixa na testa e punhos grandes.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Macaco grande e gordo, sem roupa, de cara fechada | Base refeita: punhos maiores, ombreiras de couro, cinto, faixa vermelha | chapeu, tronco, mao_ataque, mao_livre | bate com as mãos |
| 1 | 3 a 9 | Nível 3: boca aberta. Nível 5: mãos enfaixadas de branco | Punhos enfaixados em bege claro | mao_ataque, mao_livre | o Rugido |
| 2 | 10 a 19 | Faixas vermelhas, bandana azul, pelo eriçado | Faixas vermelhas nos punhos, tufos maiores nos ombros, bandana de pontas longas, cinturão de fivela dourada grande | mao_ataque, mao_livre, pelagem, chapeu, tronco | o Grande Aperto |
| 3 | 20 | Faixas amarelas com luvas pretas, bandana preta, marca escura no rosto, topete e olhos vermelhos | Luvas escuras com faixas amarelas, bandana escura, crista vermelha, ombros de pelo escuro, aro de impacto no chão | mao_ataque, mao_livre, chapeu, pelagem, extra | fúria máxima |

### Adora

Identidade fixa: pelo dourado, coroa de ouro com joia vermelha, raios de sol atrás da cabeça, capa amarela e cajado dourado.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Pelo quase branco, capa branca com barra laranja, tiara dourada | Base refeita: tiara simples, cinco raios curtos, capa curta, cajado de orbe pequeno | chapeu, costas, tronco, mao_ataque | serva do sol |
| 1 | 3 a 9 | Nível 3: segundo rabo de cabelo e tiara detalhada. Nível 7: cristal vermelho e luvas vermelhas | Joia vermelha maior na tiara, luvas vermelhas, sete raios | chapeu, tronco | o Longo Braço de Luz |
| 2 | 10 a 19 | Sem pupilas, ombros da capa maiores, três emblemas na tiara | Ombreiras douradas, nove raios longos, orbe maior no cajado, uma bola de luz em volta | costas, chapeu, mao_ataque, extra | a Bola de Luz |
| 3 | 20 | Mais rabos de cabelo, tiara em forma de coroa, luvas amarelas, tornozeleiras | Coroa alta, halo completo atrás da cabeça, capa longa branca e ouro, três bolas de luz, aro de luz no chão | chapeu, costas, extra | quase uma deusa do sol |

### Almirante Brickell

Identidade fixa: quepe marinho com estrela dourada, casaco marinho de barra dourada e luvas brancas. A arma depende da decisão 1.

A wiki não descreve as mudanças em texto (seção marcada como incompleta). A coluna do original foi escrita por mim olhando os cinco retratos da galeria. Tentado: página "Admiral Brickell", página "Admiral Brickell (BTD6)" e a galeria.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Quepe azul com âncora, casaco azul, pistola erguida, navio cinza atrás | Base refeita: pistola de cano curto na mão, sabre na cintura, quepe com estrela, casaco | chapeu, tronco, mao_ataque | comandante naval |
| 1 | 3 a 9 | Nível 3: bandeira amarela e azul. Nível 7: pistola de cano maior e radar no navio | Mastro curto nas costas com flâmula amarela e azul, dragonas douradas | costas, tronco | as Táticas Navais |
| 2 | 10 a 19 | Quepe com faixa dourada, navio maior | Pistola de cano longo, quepe com faixa dourada e âncora, uma mina naval ao pé | mao_ataque, chapeu, extra | a Mega Mina |
| 3 | 20 | Uniforme e quepe brancos, pistola dourada, navio preto e dourado | Uniforme branco com ouro, pistola dourada, capa marinho, duas minas | tronco, chapeu, mao_ataque, costas, extra | almirante de gala |

### Etienne

Identidade fixa: topete, fones vermelhos, colete branco, lenço azul, controle remoto e drone.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Capacete de piloto, óculos redondos, cachecol listrado, controle | Base refeita: controle com antena, um drone de quatro hélices, fones | chapeu, tronco, mao_ataque, extra | pilota drones |
| 1 | 3 a 9 | Nível 3: bolsa verde e antena extra. Nível 7: controle maior e óculos infravermelhos | Bolsa a tiracolo, controle de duas antenas, dois drones | tronco, mao_ataque, extra | o Enxame de Drones |
| 2 | 10 a 19 | Broche de bandeira e antena vermelha no capacete, controle amarelo | Controle grande amarelo, antena de bola vermelha nos fones, óculos, três drones | mao_ataque, chapeu, rosto, extra | o drone de ataque |
| 3 | 20 | Capacete e camisa azuis, laptop amarelo com adesivos | Console amarelo, jaqueta azul, um drone grande no alto e três pequenos | mao_ataque, tronco, extra | frota completa |

### Sauda

Identidade fixa: faixa escura na testa, cinto vermelho, ombreiras de aço e uma espada em cada mão.

A seção de aparência da wiki está vazia. A coluna do original foi escrita por mim olhando os quatro retratos da galeria. Tentado: página "Sauda", página "Sauda (BTD6)" e a galeria.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Faixa azul na cabeça, coque, duas espadas largas, blusa vermelha e preta | Base refeita: duas espadas de lâmina larga, faixa, cinto, ombreiras | chapeu, tronco, mao_ataque, mao_livre | luta de perto com duas espadas |
| 1 | 3 a 9 | Enfeite dourado no coque, sandálias | Enfeite dourado na faixa, guardas douradas nas espadas | chapeu, mao_ataque, mao_livre | a Espada Saltadora |
| 2 | 10 a 19 | Flor vermelha na faixa, espada grande nas costas com borla, faixa escura esvoaçante | Terceira espada grande nas costas com borla, faixa de pontas longas, lâminas mais largas | costas, chapeu, mao_ataque, mao_livre | a Investida |
| 3 | 20 | Enfeite de coroa dourado, flor amarela, espadas maiores, mais borlas, saia clara | Tiara dourada, lâminas longas com base dourada, saia branca, borlas duplas, aro de corte no chão | chapeu, mao_ataque, mao_livre, tronco, extra | mestra da espada |

### Psi

Identidade fixa: pelo lilás, antena de ponta rosa, gema na testa, saia roxa e orbes em volta.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Pelo laranja, roupa vinho, flutua sobre um tapete com incensos | Base refeita: dois orbes, aro de aura no chão, antena e gema | chapeu, tronco, extra | poder da mente |
| 1 | 3 a 9 | Nível 3: olhos fechados, marcas e mãos com brilho amarelo. Nível 8: cachecol turquesa e venda | Venda roxa nos olhos, gema acesa maior, quatro orbes | rosto, chapeu, extra | a Explosão Psíquica |
| 2 | 10 a 19 | Venda e tapete azuis, velas, brilho azul, capa | Capa roxa, seis orbes ciano, aro duplo, quatro velas no chão | costas, extra | o Grito Psiônico |
| 3 | 20 | Pelo azul vivo, pele azul escura, olhos brancos, sem venda, chamas turquesa | Pelo ciano (decisão 3), sem venda, coroa de orbes sobre a cabeça, chamas ciano no chão | pelagem, rosto, chapeu, extra | mente pura |

### Geraldo

Identidade fixa: barba, chapéu de aba marrom com fita dourada, colete verde, mochila grande e poção na mão.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Bigode, faixa listrada, poncho vermelho, mochila turquesa enorme | Base refeita: mochila média com tapete enrolado e luneta, poção | chapeu, tronco, costas, mao_ataque | mercador viajante |
| 1 | 3 a 9 | Nível 3: brincos, regata azul, vara com lanterna na mochila. Nível 7: tira de pano branca | Vara com lanterna acesa presa à mochila, segunda poção no cinto | costas, tronco | a loja ganhou itens |
| 2 | 10 a 19 | Mochila bem maior com faixa amarela, faixa da testa azul escura | Mochila bem maior com panela, rolo e bandeirola, pena no chapéu | costas, chapeu | loja quase completa |
| 3 | 20 | Emblema verde, poncho com franja e mangas, mais brincos, barba pontuda | Mochila gigante com duas lanternas, gema verde no chapéu, poncho vermelho de barra dourada, poção brilhando | costas, chapeu, tronco, mao_ataque | todos os itens |

### Corvus

Identidade fixa: capuz escuro com barra ciano, capa escura, lança de ponta ciano e espíritos em volta.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Pelo azul, colete preto, livro de feitiços, um espírito verde | Base refeita: lança, capuz, capa curta, um espírito | chapeu, costas, tronco, mao_ataque, extra | invoca espíritos |
| 1 | 3 a 9 | Nível 3: marcas azul claro e mecha cinza. Nível 7: olheiras roxas, topete em gancho | Livro de feitiços na mão livre (decisão 2), dois espíritos | mao_livre, extra | a Colheita de Almas |
| 2 | 10 a 19 | Nível 10: cinco pontas turquesa na testa. Nível 15: dentes e marcas turquesa, luva e pés brilhando | Coroa de cinco pontas ciano sobre o capuz, três espíritos maiores, lança de duas lâminas | chapeu, extra, mao_ataque | o Ritual Sombrio |
| 3 | 20 | Pelo longo, luva rosa com garras, levita | Chifres ciano, capa longa rasgada, livro aceso, um espírito grande atrás e três pequenos, aro no chão | chapeu, costas, mao_livre, extra | senhor das almas |

### Rosalia

Identidade fixa: topete rosa, óculos, cachecol rosa, mochila a jato, tênis rosa e pistola de laser.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 6 | Cabelo laranja em dois coques, regata, jetpack de dois propulsores. Nível 2: luvas. Nível 5: alças azuis e anel na arma | Base refeita: mochila a jato com luz, pistola de laser, óculos | pelagem, rosto, tronco, costas, pes, mao_ataque | voa e atira laser |
| 1 | 7 a 12 | Visor transparente, aletas no jetpack | Aletas no jato, visor no lugar dos óculos, anel na boca da pistola | costas, rosto, mao_ataque | o Impulso de Voo |
| 2 | 13 a 19 | Macacão vestido, coques espetados, fivelas | Jato de dois propulsores com chama ciano, macacão cinza-escuro, dois tufos espetados | costas, tronco, pelagem | a Carga Cinética |
| 3 | 20 | Visor rosa, coques maiores, detalhes rosa, peitoral de metal com emblema azul | Jato com asas e detalhes rosa, peitoral com gema ciano, pistola de cano duplo, chamas longas | costas, tronco, mao_ataque | engenharia no auge |

No original a Rosalia não muda no nível 3. As luvas do nível 2 e as alças do nível 5 não se leem a 48 px e ficaram no estágio 0.

### Dan D'Monke

Identidade fixa: barba, chapéu de aba escuro com fita vermelha, capa vermelha, colete escuro e florete.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Boina rosa, capa rosa, camisa creme, florete prateado | Base refeita: florete fino de guarda em cesto, chapéu sem pena, capa curta | chapeu, costas, tronco, mao_ataque | esgrimista |
| 1 | 3 a 9 | Broche dourado com folhas na boina, capa de borda amarela, guarda dourada | Pena vermelha no chapéu, capa de borda dourada, guarda dourada | chapeu, costas, mao_ataque | ganhou estilo |
| 2 | 10 a 19 | Boina vira chapéu, luvas cinza, mecha branca, capa maior, florete dourado de guarda em flor | Chapéu de aba mais larga, capa longa, florete dourado mais comprido, luvas | chapeu, costas, mao_ataque, tronco | duelista famoso |
| 3 | 20 | Chapéu vermelho de faixa amarela, capa de penas vermelha, florete maior azul com listras | Capa de penas em camadas vermelha e ouro, duas penas no chapéu, florete de espiral azul, rosa no peito | costas, chapeu, mao_ataque, tronco | o grande galã |

### Silas

Identidade fixa: pelo de gelo, chapéu de cone branco com fita ciano, capa azul e cajado de gelo.

| Estágio | Níveis | No original (observado) | No nosso modelo | Encaixes | O que comunica |
|---|---|---|---|---|---|
| 0 | 1 a 2 | Pelo branco lavanda, capuz e cachecol roxos, chifres, emblema azul flutuando nas costas | Base refeita: cajado de orbe ciano sem cristais, chapéu, capa curta | chapeu, costas, tronco, mao_ataque | mago do gelo |
| 1 | 3 a 9 | Nível 3: emblema com aro de gelo, barra azul na roupa. Nível 7: emblema maior, pontas dos chifres congeladas | Três cristais em volta do orbe, floco pequeno flutuando nas costas | mao_ataque, extra | a Geladura |
| 2 | 10 a 19 | Emblema maior ainda, chifres todos congelados, cabelo mais longo | Floco maior de seis pontas, pontas de gelo no chapéu, ombreiras de cristal | extra, chapeu, tronco | a Cascata Congelada |
| 3 | 20 | Roupas azul claro, chifres maiores, olhos ciano, emblema em floco de neve | Capa branca e azul clara, floco grande, coroa de cristais no chapéu, estilhaços em volta, aro de geada no chão | costas, extra, chapeu | o inverno em pessoa |

## Peças novas

| Peça | Onde fica | Heróis que usam |
|---|---|---|
| Aro de efeito no chão em várias cores (já existe `pecas.aro_chao`) | pecas.py | gwendolin, obyn, pat, adora, sauda, psi, corvus, silas |
| Máscara ou viseira de rosto | pecas.py | quincy, gwendolin, ezili, benjamin |
| Ombreiras e dragonas soltas | pecas.py | quincy, striker, obyn, adora, brickell, silas |
| Venda nos olhos | herois.py | psi |
| Pistola (cano curto, longo, duplo) | herois.py | brickell, rosalia |
| Mina naval | herois.py | brickell |
| Mastro com flâmula | herois.py | brickell, geraldo |
| Lançador de caixa com luneta | herois.py | striker |
| Servidor com parabólica e tela curva | herois.py | benjamin |
| Caveira de cajado e máscara de madeira | herois.py | ezili |
| Drone grande | herois.py | etienne |
| Livro de feitiços | herois.py | corvus |
| Lanterna em vara | herois.py | geraldo |
| Floco de gelo flutuante e cristais | herois.py | silas |
| Halo e bola de luz | herois.py | adora |
| Capa de penas em camadas | herois.py | dan |
| Metralhadora, canhão duplo e lançador do tanque | herois.py | churchill |

## Estado

Preenchida nas Partes B e C.

| Herói | Estágios gerados | Máx. de triângulos | Malhas | Tamanho total | Pendências |
|---|---|---|---|---|---|
