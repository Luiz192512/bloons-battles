---
titulo: Arte e interface que faltam no clone em relação ao BTD6 (para o Claude Design)
modelo_alvo: claude-opus-5-5 (Claude Design)
tipo: template
versao: 1
idioma: pt
---

```xml
<papel>
Você é um diretor de arte e UI designer sênior de jogos casuais, especialista no estilo da Ninja
Kiwi (Bloons TD 6). Desenha sprites cartunescos com contorno grosso e formas simples, e entrega
tudo de um jeito que um programador reproduz com primitivas (círculos, retângulos arredondados,
polígonos, arcos), sem imagens externas.
</papel>

<contexto>
Projeto bloons-battles: clone do Bloons TD 6 em C++17 e raylib, tela virtual de 1280x720. TODA a
arte é desenhada por código. Você já fez a primeira leva neste mesmo projeto do Claude Design:
- Sprites.dc.html, com a biblioteca sprites2.js (primitivas c, e, rr, poly, star, ln, sec, arc,
  ring, ringE, txt, g, que geram o SVG e a "receita" raylib ao mesmo tempo) e dados_sprites.js.
- Auditoria Bloons TD Battles.dc.html, com o design system (paleta, tipografia, medidas),
  componentes e os mockups de Partida e Menus.
Mantenha exatamente esse sistema: caixa 128x128 para torres (220x120 para dirigíveis, projéteis
centrados em 0,0), contorno em TINTA desenhado por baixo, mesma paleta e as fontes Lilita One e
Nunito. As torres têm vista de cima no mapa e vista 3/4 nos cartões e painéis.

Uma análise comparou o clone com o BTD6 jogado de verdade. O que falta de arte e de interface
está abaixo. Estou anexando o documento docs/analise-btd6-sandbox.md e as capturas do jogo real
(real_*.jpg) e do clone (11_vitrine_tiers_1.png, 12_vitrine_tiers_2.png, demo_solo.png,
painel_super.png). Use as capturas do BTD6 como referência de FORMA e LEITURA, não para copiar
traço: o resultado tem de parecer com o resto do clone.

Hoje, no clone, o tier 5 de cada torre é o mesmo macaco (ou a mesma máquina) com um acessório.
No BTD6 o tier 5 vira outro objeto, reconhecível de longe. Essa é a maior diferença visual.
</contexto>

<tarefa>
Produza, em ordem de prioridade, os seis pacotes abaixo.

PACOTE 1. Silhuetas de tier (o mais importante)
Para as 22 torres, redesenhe o tier 5 dos 3 caminhos (66 sprites) e os tiers 3 e 4 quando o
BTD6 muda o objeto. Cada um nas duas vistas (cima e 3/4). Referências obrigatórias:
- Dardo: catapulta (Espinhopulta, Juggernaut, Ultra-Juggernaut preto e laranja); bandana e três
  dardos (Tiro Triplo); besta e capuz (Besta, Atirador Afiado, Mestre da Besta preto e dourado).
- Bumerangue: glaives orbitando e capuz roxo (Senhor das Glaives); braço robótico (Biônico) e
  braço enorme com bobinas verdes (Carga Permanente); chapéu de explorador (Kylie) e casaco
  listrado com bumerangue em chamas (Dominação M.O.A.B.).
- Canhão: canhão vermelho de boca larga (Bombas Muito Grandes), azul-marinho com caveira e
  espetos (Esmaga Bloon); lançador de mísseis amarelo com cara de tubarão (Destruidor) e preto e
  verde (Eliminador); canhão verde (Cacho) e azul-marinho e dourado (Blitz).
- Tachinhas: chama no topo e blindagem (Anel Infernal); espiral azul com serras (Super
  Turbilhão); torre preta com caveira e muitos canos (Zona das Tachinhas).
- Gelo, Cola, Sniper (roupa de folhas, armadura de lentes verdes, capacete preto), Sub, Bucaneiro
  (porta-aviões, navio pirata, cargueiro de contêineres), Ás (caça, jato furtivo, bombardeiro
  cinza, fortaleza de quatro motores), Heli (Apache, Poperações, Comanche com escolta), Morteiro
  (cano verde brilhante, bateria de três canos, vermelho e dourado), Dartling (canhão laser
  vermelho, lançador duplo roxo e verde, torre cilíndrica azul).
- Mago (Arquimago branco, Fênix, Príncipe das Trevas), Super (Avatar do Sol, TEMPLO do Sol,
  Verdadeiro Deus Sol como estátua gigante sobre o templo; Robô e Anti-Bloon; Cavaleiro e Lenda da
  Noite), Ninja, Alquimista, Druida (tempestade, árvore com chifres, fera vermelha).
- Fazenda (central tecnológica, banco, templo de mármore), Espinhos (minas vermelhas, fábrica
  roxa, fábrica amarela), Vila (castelo com balista, prédio com antena, prefeitura), Engenheiro.
A seção 3 do documento descreve cada um como foi visto.

PACOTE 2. Torres transformadas e estados
- Dardo transformado em Super Macaco e em Macaco Plasma (habilidade Fã-Clube), e o Alquimista em
  forma de monstro (Transformação Total).
- Cola por upgrade: amarela (base), verde (corrosiva), rosa (Super Cola), cobrindo o bloon e
  escorrendo; estrelas de atordoamento.
- Bloon "super frágil" (gelo com faixas roxas) e bloon encolhendo (Poção de Encolher).

PACOTE 3. Projéteis e efeitos novos
- Texto "CRIT" (laranja, grande, com contorno, sobe e some em 0,6 s).
- Espiral de serras do Turbilhão; raio contínuo vermelho do Raio da Perdição (com brilho na base
  e na ponta); anel de fogo que se expande; clarão amarelo em estrela do Esmaga Bloon; rastro
  verde de velocidade do bumerangue; Kylie em chamas; míssil com cara de tubarão.
- Banana e caixa de bananas caídas no chão (com brilho de "clique para coletar"), caixa de
  suprimentos do Sniper caindo de paraquedas, avião da Tsar Bomba, mini-Comanche, radar verde do
  Submarino submerso, marcador de alvo do Morteiro e mira arrastável do Ás.

PACOTE 4. Interface da partida (mockups 1280x720 mais especificação)
- Loja com cor por categoria: Primárias, Militares, Mágicas e Suporte, cada uma com seu tom nos
  cartões, legível com a tecla e o preço que já existem. Defina os 4 tons na paleta.
- Painel de upgrade com os controles novos: selo de camo, botão de mão do Bumerangue, botão
  "Definir alvo" do Morteiro, seletor de rota do Ás (Círculo, Infinito, Oito, Rota Centralizada),
  modos do Heli (Perseguir, Seguir o mouse, Travar no lugar, Patrulha), modo do Dartling (Normal,
  Travado), modo Elite do Sniper, dois seletores de alvo do Anti-Bloon, botão Submergir, botão de
  escolher torre do Ultraimpulso, contador de lápide do Necromante e de dinheiro gerado.
- Carta de upgrade condicional: cinza, com o ícone do requisito e o texto (ex.: "Requer Fazenda
  de Bananas").
- Diálogo de confirmação do sacrifício (Templo do Sol e Verdadeiro Deus Sol), com Cancelar e
  Confirmar.

PACOTE 5. Telas novas (mockups mais especificação)
- Painel do Sandbox no lugar da loja: campos Rodada, Espaçamento e Quantidade, três alternadores
  (camo, regen, fortificado), grade com todos os bloons e dirigíveis, e os botões de recarregar
  habilidades, apagar bloons, apagar torres e reiniciar. Com o botão que alterna loja e painel.
- Entrada "Sandbox" e os modos de restrição (Só Primárias, Só Militares, Só Mágicas) na tela
  Jogo Solo, sem estourar a fileira de dificuldades que já tem 7 botões.
- Tela de consulta de upgrades: os 15 upgrades de uma torre em 3 linhas, com ícone, nome, preço e
  descrição.

PACOTE 6. As 4 torres novas (por último)
Beast Handler, Mermonkey, Desperado e Skywarden: base, tier 3 e tier 5 dos 3 caminhos, nas duas
vistas, mais os projéteis e as feras ou ajudantes de cada uma, e o ícone do cartão da loja.
</tarefa>

<restricoes>
- Só primitivas da sprites2.js. Nada de imagem, textura, filtro de SVG ou degradê que a receita
  raylib não reproduza (degradê vira no máximo 2 faixas de cor).
- Cada sprite com até cerca de 40 primitivas e legível a 48 px (tamanho no mapa). O tier 5 tem de
  ser reconhecido só pela silhueta, em preto.
- Os três tier 5 de uma torre têm de ser diferentes entre si e do tier 3.
- Não mude sprites, cores ou componentes que já existem e não foram pedidos. Cor nova só as 4
  de categoria e as da cola; declare cada uma na paleta com nome e RGB.
- Textos de interface em português, com os nomes dos upgrades que o clone já usa (estão no
  documento, coluna "Nome no clone").
- Não copie arte do BTD6: mesma ideia e mesma leitura, traço do clone.
- Não use travessão nem meia-risca em nenhum texto.
</restricoes>

<formato_saida>
No projeto do Claude Design:
1. Sprites.dc.html atualizado, com abas novas "Tiers novos", "Transformações", "Efeitos novos" e
   "Torres novas". Cada sprite com chave estável no padrão que o código usa:
   torre: <chave>_c<caminho>t<tier> (ex.: dardo_c1t5), nas vistas "cima" e "34";
   projétil e efeito: nome curto em minúsculas sem acento (ex.: crit, serra, raio_perdicao).
2. Interface BTD6.dc.html, com os mockups dos pacotes 4 e 5 em 1280x720 e, para cada componente,
   medidas em px, cores da paleta, estados (normal, sobre, pressionado, bloqueado) e fonte.
3. Uma página "Inventário de entrega" com a tabela:
   | Chave | Pacote | Item do backlog (B..) | Vistas | Animação (se houver) | Observação |
   e a lista do que NÃO foi entregue, com o motivo.
Para cada animação, diga o grupo animado, o tipo (gira, balanca, flutua, recuo, pulsa, cresce,
pisca, some) e a duração.
</formato_saida>

<exemplos>
<exemplo>
Linha do inventário bem preenchida:
| dardo_c1t5 | 1 | B27 | cima, 34 | braço da catapulta: recuo, 0,35 s | Catapulta preta com aro laranja; o macaco fica atrás, só a cabeça aparece na vista de cima |
</exemplo>
<exemplo>
Especificação de componente bem preenchida:
"Selo de camo: círculo de 22 px, fundo VERDE_ESCURO, contorno TINTA de 3 px, ícone de olho em
BEGE. Fica no canto superior direito do retrato do painel, 6 px para dentro. Só aparece quando a
torre detecta camo. Sem estado de sobre."
</exemplo>
<exemplo>
Diferença de silhueta bem resolvida:
"Canhão c2t3 (Destruidor de M.O.A.B.): deixa de ser canhão sobre rodas e vira um tubo de míssil
amarelo deitado sobre base listrada, com olhos e dentes de tubarão na ponta. Em silhueta preta,
lê-se como um peixe sobre um disco, diferente do canhão redondo da base."
</exemplo>
</exemplos>

<criterios_de_qualidade>
Antes de entregar, confira:
1. Teste da silhueta: cada tier 5 em preto, a 48 px, é distinguível do tier base e dos outros
   dois tier 5 da mesma torre.
2. Todo sprite usa só primitivas da sprites2.js e tem a receita raylib gerada.
3. As chaves seguem o padrão e não colidem com as que já existem.
4. Os mockups cabem em 1280x720 sem sobrepor o HUD, a loja e a barra de habilidades atuais.
5. Todo controle novo do painel tem os 4 estados especificados.
6. O inventário lista tudo o que foi feito e tudo o que ficou de fora, sem lacuna silenciosa.
7. Nada do que já existia foi alterado sem pedido, e nenhum texto tem travessão.
</criterios_de_qualidade>
```

## Notas de design

- **Modelo e formato:** Claude Design (Opus 5.5), tags XML, em português, no mesmo molde de
  `docs/prompt-design-bloons.md`, que gerou a primeira leva de arte.
- **Continuidade do sistema:** o prompt amarra a entrega à `sprites2.js`, às caixas, à paleta e às
  chaves que o código já usa, para o porte para `src/cliente/sprites.cpp` ser mecânico.
- **Prioridade pela maior lacuna:** as silhuetas de tier 5 (B27) vêm primeiro, com referências por
  torre tiradas das capturas reais; as 4 torres novas ficam por último.
- **Só o que é desenho:** mecânica fica no prompt de implementação. Aqui entram sprites, efeitos,
  cores de categoria e mockups dos painéis e telas novas (Sandbox, sacrifício, upgrade condicional,
  consulta de upgrades).
- **Entrega verificável:** teste da silhueta em preto a 48 px, inventário com a chave e o item do
  backlog de cada sprite, e lista explícita do que não foi entregue.
- **Anexos necessários:** mande junto `docs/analise-btd6-sandbox.md` e as capturas citadas no
  contexto; sem elas o Claude Design não tem a referência de forma.
