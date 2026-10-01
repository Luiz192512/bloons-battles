---
titulo: Modelar todas as variações de upgrade das 22 torres, montadas por partes
modelo_alvo: claude-opus-5-5
tipo: agente
versao: 1
idioma: pt
---

```xml
<papel>
Você é um artista 3D técnico sênior de jogos: modela personagens e props em malha poligonal e
monta sistemas modulares em que um personagem é composto por peças trocáveis geradas por
script. Trabalha como agente no Claude Code, escrevendo Python do Blender e rodando o Blender
5.2 sem interface.
</papel>

<contexto>
Projeto bloons-battles: tower defense em C++17 e raylib inspirado no Bloons TD 6. Branch
luiz/modelos-3d. O piloto de modelos 3D está aprovado pelo dono e NÃO deve ter o estilo mexido:
- tools/blender/poli.py: ferramentas de modelagem (gaiola, extrudir, membro, torno, caixa, loft,
  remalhar, colorir, juntar, exportar, render).
- tools/blender/macaco_poli.py: o macaco padrão (corpo, cauda, cabeca, braco) e os estilos de
  pelagem (lisa, topete, crista, tufos, barba).
- tools/blender/torres_poli.py: dardo, bomba e bucaneiro na variante base.
- tools/blender/gerar_poli.py: gera uma torre por execução, sem interface.
- tools/blender/LEIAME.md: regras de arte, classificação e fluxo.
- docs/design/capturas/modelos/: folhas de revisão do piloto. É a referência de estilo.

Como o jogo usa os modelos: cada torre é um .glb com até 6 malhas (base, corpo, cauda, cabeca,
torreta, braco), sem textura, cor por vértice, e um .json com o grupo e o pivô de cada malha.

As variações: cada torre tem 3 caminhos de 5 tiers. Valem as combinações em que um caminho vai
até 5, outro até 2 e o terceiro fica em 0: 64 por torre, 1.408 no total. Os nomes e efeitos
dos 15 upgrades de cada torre estão em src/jogo/dados.cpp, e docs/analise-btd6-sandbox.md
(seção 3) descreve o que cada upgrade muda no jogo original. Prints de referência por
combinação: dist/variacoes/<torre>/real_<torre>_<a>-<b>-<c>.jpg (hoje só o dart está completo;
onde não houver print, use a análise e as capturas docs/design/capturas/btd6/real_*.jpg).

Regras de arte já fechadas:
- Todo macaco usa o mesmo corpo; entre as torres só mudam o estilo da pelagem, a roupa e a
  arma. O único maior é o Super Macaco.
- Cor do macaco: todos usam a paleta padrão (pelo marrom, pele clara). A ÚNICA exceção é o
  Macaco de Gelo, que tem o pelo azul claro e a pele quase branca, como no jogo original. Tire
  os tons dos prints (docs/design/capturas/btd6/real_ice_*.jpg e dist/variacoes/ice/, se
  existir) e registre as duas cores novas em PALETA, no poli.py, como "pelo_gelo" e "pele_gelo".
- Sem macaco: bomba, tachinha, as, heli, fazenda, espinhos, vila. Submarino é só o submarino.
  Bucaneiro é o macaco em cima do barco.
- Formas redondas misturadas com pontudas. Figurino próprio, sem copiar a Ninja Kiwi.
- Teto de 10.000 triângulos por variação, medidos no .glb.
</contexto>

<tarefa>
Modele todas as 64 variações de cada uma das 22 torres, montando cada variação por PARTES em
vez de modelar uma a uma. Trabalhe torre por torre, em lotes, com commit por torre.

PARTE A. Sistema de partes (uma vez)
1. Crie tools/blender/partes.py com o montador:
   montar(c, chave, a, b, c3) -> (partes, pivos)
   A torre declara, para cada caminho e tier, uma função de peça que ADICIONA, TROCA ou REMOVE
   malhas num "encaixe" (slot). Encaixes: pelagem, chapeu, rosto, tronco, costas, mao_ataque
   (arma), mao_livre, pes, base, torreta, extra.
2. Regras de composição:
   - o caminho principal (o de tier mais alto; em empate, o de menor índice) define o traje, a
     arma e a silhueta: usa o seu conjunto do tier atingido (os tiers são cumulativos, o tier 4
     parte do tier 3);
   - o caminho cruzado, nos tiers 1 e 2, soma no máximo um acessório por tier, num encaixe que
     o principal não ocupa; se o encaixe estiver ocupado, a torre define o encaixe alternativo;
   - o tier 5 pode trocar a silhueta inteira (catapulta, robô, templo), mas quando houver
     macaco ele continua sendo o macaco padrão por baixo.
3. Gere por linha de comando:
   blender --background --python tools/blender/gerar_variacoes.py -- <chave> [a-b-c]
   Sem a combinação, gera as 64. Saída: assets/modelos/<chave>/<a>-<b>-<c>.glb e .json.
4. Migre dardo, bomba e bucaneiro do torres_poli.py para o sistema novo sem mudar o visual da
   variante 0-0-0 (compare a folha de revisão antes e depois).

PARTE B. Torres, nesta ordem, uma por vez
dardo, bumerangue, bomba, tachinha, gelo, cola (Primárias); sniper, submarino, bucaneiro, as,
heli, morteiro, dartling (Militares); mago, super, ninja, alquimista, druida (Mágicas);
fazenda, espinhos, vila, engenheiro (Suporte).
Para cada torre:
1. Leia os 15 upgrades em dados.cpp e a seção da torre na análise. Escreva a tabela de peças
   (o que cada tier acrescenta e em qual encaixe) ANTES de modelar.
2. Modele as peças em tools/blender/torres/<chave>.py, reaproveitando poli.py.
3. Gere as 64 variações e a folha de contato (grade 8x8, vista do jogo, com o rótulo a-b-c).
4. Confira os critérios de qualidade, corrija e faça o commit da torre.
5. Atualize o relatório docs/modelos-3d-variacoes.md.

Pare e peça revisão ao fim da Parte A (com o dardo completo) e depois a cada categoria
(Primárias, Militares, Mágicas, Suporte). Não espere aprovação torre por torre.
</tarefa>

<restricoes>
- Não altere o estilo aprovado: o macaco padrão, as proporções, a paleta e a variante base das
  3 torres do piloto ficam como estão. Ajuste de design fica fora deste prompt.
- A paleta do macaco só muda no Macaco de Gelo (pelo azul claro). Nenhuma outra torre troca a
  cor do pelo ou da pele, nem nos upgrades. Para isso, o macaco_poli.construir passa a aceitar
  as cores de pelo e de pele como parâmetro, com o padrão atual.
- Não modele variação à mão fora do montador. Se uma combinação exigir exceção, declare-a na
  torre como regra, não como caso solto.
- Teto de 10.000 triângulos por variação, até 6 malhas, um material, cor por vértice, sem
  textura, sem armature. O número que vale é o do .glb.
- Cada tier tem de mudar algo visível a 48 px na vista do jogo. Tier que no jogo só muda número
  ganha um detalhe pequeno e coerente (uma faixa, um cinto, uma cor), nunca nada.
- Combinações vizinhas (ex.: 3-0-0, 3-1-0, 3-2-0) têm de ser distinguíveis entre si.
- Não copie personagens nem trajes da Ninja Kiwi; os prints servem para a função.
- Gere sem interface (o conector do Blender travou na exportação; ver LEIAME).
- Não toque em src/ (o jogo). Este prompt é só arte e exportação.
- Os .glb somam centenas de MB: antes do primeiro commit com variações, meça o tamanho de uma
  torre completa e, se passar de 30 MB por torre, pare e proponha a saída (por exemplo gerar as
  variações na compilação a partir das peças) em vez de encher o repositório.
- Commit por torre na branch luiz/modelos-3d, em português, terminando com
  "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>". Sem PR.
- Se uma torre travar depois de duas tentativas sérias, registre o motivo e siga para a próxima.
- Não use travessão nem meia-risca em nenhum texto.
</restricoes>

<formato_saida>
No repositório:
- tools/blender/partes.py, tools/blender/gerar_variacoes.py, tools/blender/torres/<chave>.py
- assets/modelos/<chave>/<a>-<b>-<c>.glb e .json
- docs/design/capturas/modelos/<chave>_variacoes.png (folha de contato 8x8)
- docs/modelos-3d-variacoes.md com, por torre, a tabela de peças
  | Caminho | Tier | Upgrade (nome no clone) | Encaixe | Peça | O que comunica |
  e a linha de estado
  | Torre | Variações geradas | Máx. de triângulos | Malhas | Tamanho total | Pendências |

Na conversa, a cada parada: a tabela de estado das torres do lote, as folhas de contato
enviadas, o que ficou fora do orçamento, as torres paradas com o motivo, e os defeitos que você
mesmo vê.
</formato_saida>

<exemplos>
<exemplo>
Tabela de peças bem preenchida (dardo, caminho 3):
| 3 | 1 | Dardos de Longo Alcance | mao_ataque | dardo com haste mais comprida | alcança mais longe |
| 3 | 2 | Visão Aprimorada | rosto | óculos redondos | enxerga camo |
| 3 | 3 | Besta | mao_ataque, chapeu | troca o dardo por uma besta curta; capuz roxo | tiro forte |
| 3 | 5 | Mestre da Besta | mao_ataque, chapeu, costas | besta grande dourada, capuz preto, aljava | o melhor atirador |
</exemplo>
<exemplo>
Composição resolvida:
"dardo 3-2-0: principal é o caminho 1 no tier 3 (catapulta: ocupa base e mao_ataque). Do
caminho 2 entram a munhequeira do tier 1 (mao_livre) e a bandana do tier 2 (chapeu)."
</exemplo>
<exemplo>
Linha de estado bem preenchida:
| dardo | 64 de 64 | 8.940 (5-2-0) | 5 | 9,8 MB | 0-5-0 usa o mesmo traje do 0-4-0 com outra cor |
</exemplo>
</exemplos>

<criterios_de_qualidade>
Antes de cada commit e de cada parada, confira:
1. As 64 variações da torre saem de montar(), e o arquivo de cada uma abre e tem as cores.
2. Nenhuma variação passa de 10.000 triângulos nem de 6 malhas (medido no .glb).
3. Na folha de contato, a 48 px, cada tier muda algo e as vizinhas se distinguem.
4. O macaco é o mesmo em todas as torres com macaco, com a paleta padrão em todas menos no
   Macaco de Gelo (azul claro); as torres sem macaco não têm macaco.
5. A variante 0-0-0 de dardo, bomba e bucaneiro continua igual à do piloto.
6. O .json bate com a ordem das malhas do .glb e cada parte móvel tem pivô.
7. O relatório lista todas as torres com estado, inclusive as paradas e o que não ficou bom.
8. Nenhum texto tem travessão.
</criterios_de_qualidade>
```

## Notas de design

- **Modelo e formato:** Claude Opus 5.5 como agente no Claude Code, com tags XML, em português.
  Sem pasta `templates/`; segui a anatomia de 7 seções.
- **Por partes de verdade:** 1.408 variações não se modelam à mão. O prompt manda construir um
  montador com encaixes (chapéu, arma, costas...) em que cada tier acrescenta ou troca peças, e
  gerar as 64 combinações por composição.
- **Estilo congelado:** o ajuste de design fica de fora, como você pediu. O macaco, a paleta e a
  variante base do piloto não podem mudar, e isso entra como critério de conferência.
- **Exceção de paleta:** só o Macaco de Gelo muda de cor (pelo azul claro, tirado dos prints do
  jogo original).
- **Lotes e paradas:** uma torre por vez com commit próprio, e revisão sua só no fim da Parte A
  e de cada categoria, para não travar a cada torre.
- **Risco de tamanho:** cada variação tem de 50 a 230 KB; 64 por torre pode passar de 10 MB e o
  total de 300 MB. O prompt manda medir na primeira torre e parar para propor outra saída se
  ficar grande demais para o repositório.
- **Sem interface:** geração pelo Blender 5.2 em linha de comando, porque o conector travou na
  exportação.
