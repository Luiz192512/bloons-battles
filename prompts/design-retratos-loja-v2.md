---
titulo: Retratos 2D da loja, segunda leva, com os ícones do BTD6 como régua de qualidade
modelo_alvo: claude-opus-5-5 (Claude Design)
tipo: template
versao: 2
idioma: pt
---

Anexos, nesta ordem:

1. `docs/design/capturas/btd6/ref_retratos_btd6.png`: a régua. Em cima à esquerda, o retrato grande do
   BTD6; à direita, a loja do BTD6; embaixo à esquerda, a leva anterior, REPROVADA.
2. `docs/design/capturas/modelos/herois_3q.png` e `docs/design/capturas/modelos/retratos.png`: os nossos
   modelos 3D, que dizem QUEM é cada personagem (roupa, chapéu, objeto).

```xml
<papel>
Você é ilustrador sênior de personagens para jogos casuais, com domínio do acabamento de ícone
de loja de tower defense: pose dinâmica de corpo inteiro, traço fino e variável, volume com
duas ou três faixas de sombra e luz. Você desenha em SVG à mão, forma por forma, e compara o
próprio desenho com uma referência antes de entregar.
</papel>

<contexto>
Projeto bloons-battles, clone de Bloons TD 6. A loja lateral mostra um retrato 2D de cada torre
e herói (PNG de 256 x 256 px com transparência, exibido a 44 px na loja e até 84 px nos menus).

Neste mesmo projeto do Claude Design você já entregou uma leva de 40 retratos (arquivo Retratos
Loja e pasta retratos/). Ela foi REPROVADA pelo dono do jogo. Ela aparece na parte de baixo, à
esquerda, da imagem ref_retratos_btd6.png. Os três motivos da reprovação:
1. Rosto genérico: dois pontos pretos e um sorriso, igual em todos, sem expressão.
2. Traço grosso demais: contorno preto pesado, com cara de adesivo ou clipart.
3. Formas e cores pobres: corpo em bolas, busto cortado, cor chapada sem volume.

A régua de qualidade agora são os ícones do próprio Bloons TD 6, na mesma imagem: o retrato
grande do macaco de dardo (em cima, à esquerda) e a coluna da loja (à direita). Observe neles, e
reproduza a LINGUAGEM, não os personagens:
- Corpo inteiro, em pose de ação, de três quartos: cabeça grande (perto de metade da altura),
  tronco pequeno em forma de pera, braços e pernas finos, mãos e pés grandes, rabo em curva.
- Cabeça ovalada e larga embaixo, com tufo de pelo em pontas no alto e orelhas pequenas.
- Olhos enormes, brancos, encostados um no outro, com íris castanha grande e pupila escura, e
  sobrancelha grossa que dá a expressão (decidido, bravo, concentrado, sereno). Máscara de pelo
  mais clara em volta dos olhos e do focinho.
- Traço fino, marrom bem escuro (não preto), mais grosso só na silhueta de fora.
- Volume: cada forma tem a cor base, uma faixa de sombra e uma de luz, com borda definida. O
  pelo vai de um marrom alaranjado na luz a um marrom mais fundo na sombra.
- Objetos e máquinas com o mesmo tratamento: canhão, torre de tachinhas e avião têm sombra, luz
  e reflexo, não são formas chapadas.

Os personagens continuam sendo os NOSSOS, vistos em herois_3q.png e retratos.png (modelos 3D do
jogo). A lista com o que identifica cada um está no arquivo design-retratos-loja.md, já anexado
neste projeto: use a mesma ordem, os mesmos 40 nomes de arquivo e os mesmos elementos de
identificação (cor de pelo, chapéu, roupa e objeto).
</contexto>

<tarefa>
Refaça os 40 retratos do zero, no nível de acabamento da régua do BTD6.

Trabalhe em três etapas, sem pular:

Etapa 1, piloto. Desenhe só quatro: dardo, ninja, bomba e quincy. Monte uma folha de comparação
com três colunas por personagem: o ícone equivalente do BTD6 recortado da régua (macaco de
dardo; para os outros, o ícone mais parecido), o seu desenho novo, e o desenho da leva
reprovada. Faça a autocrítica escrita, item por item, dos critérios de qualidade abaixo. Se
algum item falhar, redesenhe o piloto antes de seguir.

Etapa 2, os 40. Com o piloto aprovado nos critérios, desenhe os outros 36 no mesmo padrão. Os
31 macacos compartilham a mesma construção de cabeça e corpo, mas cada um tem pose e expressão
próprias, ligadas ao que ele faz (o sniper mira, o ninja se esgueira, o mago ergue o cajado, o
pat cruza os braços).

Etapa 3, folhas e exportação. Monte a folha 8 x 5 a 160 px e a mesma folha a 44 px, sobre fundo
#F3E2B8. Exporte os 40 PNG de 256 x 256 px com fundo transparente, só o personagem, para a pasta
retratos_v2/ do projeto, com os nomes do prompt anterior (dardo.png ... silas.png), e ofereça um
.zip retratos_v2.zip para baixar.
</tarefa>

<restricoes>
- Não copie nenhum personagem do Bloons TD 6. A régua serve para proporção, olho, traço e
  volume. Roupa, chapéu, cores e objetos são os dos nossos modelos 3D.
- Não reaproveite formas da leva reprovada. Nada de olho de bolinha preta, sorriso em arco,
  busto cortado em linha reta ou contorno preto grosso.
- Contorno de fora com no máximo 5 px no arquivo de 256 px; linhas internas com 2 a 3 px.
- Corpo inteiro para os 31 macacos, com os pés aparecendo. Máquinas e construções inteiras.
- Sem texto, número, moldura, fundo ou sombra no chão dentro do PNG. Fundo transparente.
- Um personagem por arquivo, ocupando de 85% a 95% do quadro, sem encostar na borda.
- Não entregue os 40 se o piloto não passar: pare na etapa 1 e diga o que não conseguiu.
</restricoes>

<formato_de_saida>
1. Página "Piloto": a folha de comparação de três colunas e a autocrítica.
2. Página "Folha": a grade 8 x 5 a 160 px e a grade a 44 px.
3. Pasta retratos_v2/ com os 40 PNG e o botão de download do retratos_v2.zip.
4. No chat, um resumo curto: o que mudou em relação à leva reprovada e qualquer personagem que
   ficou abaixo do padrão, pelo nome.
</formato_de_saida>

<exemplo>
Autocrítica esperada para um retrato do piloto (formato, não conteúdo fixo):
dardo: proporção cabeça/corpo ok (cabeça 48% da altura) · olhos brancos grandes com íris, ok ·
sobrancelha dá expressão decidida, ok · traço de fora 4 px marrom escuro, ok · pelo com base,
sombra e luz, ok · pose de arremesso de corpo inteiro, ok · a 44 px lê como macaco com dardo e
lenço azul, ok.
</exemplo>

<criterios_de_qualidade>
Antes de entregar cada etapa, confira e escreva o resultado:
1. Colocado ao lado do ícone do BTD6, o retrato parece do mesmo nível de acabamento, e não uma
   versão simplificada.
2. Nenhum dos três defeitos da leva reprovada aparece.
3. Olhos com branco, íris e pupila, e uma expressão reconhecível por personagem.
4. Toda forma grande tem pelo menos base e sombra; pelo, roupa e metal têm também luz.
5. O personagem é o do nosso modelo 3D: elementos de identificação presentes, nas cores certas.
6. A 44 px a silhueta e o elemento principal (chapéu ou arma) ainda leem.
7. Os 40 parecem do mesmo artista.
</criterios_de_qualidade>
```

## Notas de design

- Tags XML porque o alvo é um modelo Claude (Claude Design).
- A leva anterior entra como contraexemplo explícito, com os três motivos que o dono marcou
  (rosto genérico, traço grosso, formas e cores pobres), para o modelo não repetir a solução.
- A régua do BTD6 é descrita em palavras (proporção, olho, traço, volume), porque só anexar a
  imagem não bastou para o modelo perceber o que copiar da linguagem.
- Etapa de piloto com comparação lado a lado e autocrítica: gasta pouco e trava a exportação dos
  40 se o padrão não for atingido.
- Restrição de direito autoral mantida: a régua é de acabamento, os personagens são os nossos.
