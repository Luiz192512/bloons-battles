---
titulo: Macacos na vista 3/4 também no mapa, girando para mirar
modelo_alvo: claude-opus-5-5
tipo: agente
versao: 1
idioma: pt
---

```xml
<papel>
Você é um engenheiro sênior de jogos em C++17 e raylib, especialista em arte procedural e
animação 2D de personagens (sprites desenhados só com primitivas). Trabalha como agente no
Claude Code, no repositório bloons-battles, na branch luiz/design-sprites-auditoria (PR #4).
</papel>

<contexto>
O PR #4 portou os sprites do Claude Design para C++:
- src/cliente/sprites.cpp desenha cada torre/herói em duas vistas (enum spr::Vista):
  CIMA (vista de cima, usada hoje nos macacos posicionados no mapa, girada inteira pelo
  ângulo Torre::ang + 90) e FRENTE (vista 3/4 de frente, usada em cards, painel de upgrade,
  menus, retrato do herói e vitrine).
- src/cliente/arte.cpp: arte::torre() desenha a vista CIMA no mapa, com cache em camadas
  (segmentos parados em RenderTexture, grupos animados ao vivo); arte::torre_icone() e
  arte::torre_viva() desenham a vista FRENTE.
- src/cliente/render.cpp: RenderPista::desenhar_torre() chama arte::torre() com a rotação e o
  anim::Quadro (clipes de disparo/habilidade de src/cliente/anim.cpp).
- A vista FRENTE tem o braço de ataque num grupo Anim::BRACO (segue a pose dos clipes), sombra
  própria e, nas máquinas, base extrudada + arte de cima achatada (escala Y 0,78).

Pedido do dono do projeto: a vista 3/4 ficou muito mais bonita que a de cima. No mapa os
macacos devem aparecer SÓ na vista 3/4. Eles precisam continuar "girando" para mirar no alvo,
com animação ou mudança visual que deixe isso claro, mas o resultado NÃO pode ficar mais feio
que a vista 3/4 parada. A vista de cima deve continuar no código (e na vitrine), só deixa de
ser usada no mapa.

Você não sabe ainda como cada sprite 3/4 se comporta espelhado ou inclinado: confira na vitrine
e em capturas antes de decidir os limites.
</contexto>

<tarefa>
1. Troque a vista usada no mapa: desenhar_torre() e a miniatura do oponente passam a usar a
   vista FRENTE, mantendo o cache em camadas (crie a variante 3/4 de arte::torre com os mesmos
   segmentos em RenderTexture + grupos vivos). Ajuste tamanho e ponto de apoio para o pé do
   macaco ficar na posição da torre e a sombra no chão.
2. Implemente a mira sem deitar o sprite. Combine, nesta ordem de prioridade:
   a. Virar para o lado do alvo: espelhamento horizontal quando o alvo está à esquerda, com
      uma animação curta de virada (escala X passando por ~0,15 e voltando, 0,12 a 0,18 s,
      curva SAI_VOLTA) e histerese para não ficar trocando de lado quando o alvo está quase
      na vertical.
   b. O braço de ataque aponta para o alvo: giro do grupo do braço limitado a uma faixa que
      não quebre o desenho (comece em -60° a +35° e ajuste olhando a vitrine), somado à pose
      dos clipes de disparo.
   c. Inclinação leve do corpo na direção do alvo (no máximo 6° a 8°), suavizada no tempo.
   d. Máquinas e veículos (bomba, sentinela, morteiro, churchill, submarino, bucaneiro, ás,
      heli, fazenda, espinhos, vila, tachinha): decida por torre se a torreta/cano gira
      (grupo animado próprio) ou se só espelha; tachinha, vila, fazenda e espinhos não giram.
   Todo esse estado (lado atual, ângulo suavizado, tempo da virada) fica no cliente, junto do
   anim::Animador, e só LÊ Torre::ang e a posição da torre.
3. Mantenha a vista CIMA intacta em sprites.cpp e visível na vitrine (--vitrine), marcada
   como "não usada no mapa". Adicione à vitrine uma página ou seção que mostre a mira 3/4 em
   loop (alvo girando em volta do macaco), para comparar os 22 macacos e os 18 heróis.
4. Verifique: build com cmake, ctest (24 casos), capturas com --demo solo, --demo batalha e
   --vitrine antes e depois, lado a lado, em docs/design/capturas/. Se algum macaco ficar
   pior espelhado ou inclinado, reduza os limites dele ou desligue aquela transformação só
   para ele, e registre a exceção.
5. Commit (mensagem em português, terminando com
   "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"), push na mesma branch e
   atualize a descrição do PR #4 com uma seção nova.
</tarefa>

<restricoes>
- Não altere src/jogo nem src/servidor: a mira é só visual e o modo Batalha precisa continuar
  determinístico. Nada do cliente (GetTime, animador, estado de virada) entra na simulação.
- Não gire o sprite 3/4 inteiro pelo ângulo de mira (fica deitado e feio); rotação do corpo
  só dentro do limite de inclinação.
- Sem imagens externas; tudo com primitivas e cache em RenderTexture, como já está.
- Não apague a vista de cima nem as funções dela.
- Não piore o desempenho de forma visível: no --demo batalha, com F3, o FPS deve continuar
  perto de 60; se cair, reduza o que é desenhado ao vivo.
- Não use travessão nem meia-risca em nenhum texto (código, commit, PR, docs).
- Não declare pronto sem as capturas e a saída do ctest.
</restricoes>

<formato_saida>
Seção nova na descrição do PR #4, em Markdown:

## Macacos 3/4 no mapa
Resumo em 2 a 3 frases.

| Torre/herói | Vira de lado | Braço mira | Inclina | Torreta gira | Exceção e motivo |
|---|---|---|---|---|---|

Parágrafo: onde fica o estado da mira no cliente e por que não afeta o determinismo.

Verificação: comandos e resultado do build e do ctest; capturas antes/depois com links.

Na conversa, responda com: link do PR, o que mudou em 3 a 5 linhas, lista de exceções e o
caminho das capturas antes/depois.
</formato_saida>

<exemplos>
<exemplo>
Situação: Macaco Dardo, alvo passa de cima-direita para cima-esquerda.
Esperado: com histerese, ele só vira quando o alvo passa uns 10° do eixo vertical; a virada
dura ~0,15 s (escala X 1 → 0,15 → -1 com volta); o braço continua apontando para o alvo.
Linha da tabela:
| Macaco Dardo | sim | sim (-60° a +35°) | 6° | não se aplica | nenhuma |
</exemplo>
<exemplo>
Situação: Vila dos Macacos (não ataca).
Linha da tabela:
| Vila dos Macacos | não | não | não | não | construção parada; espelhar mudaria a bandeira sem motivo |
</exemplo>
</exemplos>

<criterios_de_qualidade>
Antes de encerrar, confira:
1. Nenhuma torre no mapa usa mais a vista CIMA; a vitrine ainda mostra a vista CIMA.
2. Em todas as capturas "depois", nenhum macaco aparece deitado, cortado ou com braço
   quebrado; compare com a vista 3/4 parada dos cards.
3. Dá para saber para onde cada macaco mira olhando uma captura parada.
4. git diff em src/jogo e src/servidor está vazio; ctest passa com 24 casos.
5. Toda exceção da tabela tem motivo; nenhuma linha diz só "ajustado".
6. Nenhum travessão ou meia-risca no diff, no commit e no PR.
</criterios_de_qualidade>
```

## Notas de design

- **Pergunta respondida antes de gerar**: a vista 3/4 não pode girar inteira sem ficar deitada, então perguntei como mirar. O dono pediu "girar, com animação ou mudança visual, sem ficar mais feio" e manter a vista de cima guardada; o prompt traduz isso em espelhar com animação de virada, braço apontando, inclinação limitada e torretas por máquina.
- **Contexto concreto do PR #4**: nomes reais (`spr::Vista`, `arte::torre`, `desenhar_torre`, `Anim::BRACO`, cache em camadas) para o agente não recriar a arquitetura nem apagar a vista de cima.
- **"Não pode ficar mais feio" virou critério verificável**: capturas antes/depois, limites ajustados olhando a vitrine, exceções por torre com motivo e checagem de FPS.
- **Determinismo protegido**: o estado da mira fica no cliente e só lê `Torre::ang`; `src/jogo` e `src/servidor` com diff vazio é critério de aceite.
- **XML tags e português**: alvo é Claude Code no mesmo repositório em português; os exemplos fixam o formato da tabela do PR e o comportamento com histerese.
