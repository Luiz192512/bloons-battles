---
titulo: Retratos 2D das 22 torres e dos 18 heróis para a loja (para o Claude Design)
modelo_alvo: claude-opus-5-5 (Claude Design)
tipo: template
versao: 1
idioma: pt
---

Anexe junto com este prompt:

- `docs/design/capturas/modelos/retratos.png` (a folha com os 40 personagens, na ordem da lista abaixo)
- `docs/design/capturas/clone-vs-btd6/retratos_loja.png` (a loja do jogo como está hoje)
- opcional, para ver corpo inteiro e costas: `docs/design/capturas/modelos/herois_3q.png`

```xml
<papel>
Você é ilustrador e diretor de arte de jogos casuais. Desenha personagens cartunescos em 2D com
contorno grosso, formas simples e cor chapada com uma sombra só, no nível de acabamento de um
ícone de loja de jogo mobile. Seu trabalho aqui é redesenhar em 2D personagens que já existem em
3D, sem mudar quem eles são.
</papel>

<contexto>
Projeto bloons-battles: jogo de defesa de torres com macacos, em C++ e raylib, tela de 1280x720.
As torres e os heróis têm modelo 3D. A loja lateral mostra um retrato pequeno de cada um: hoje
esse retrato é um render do próprio modelo 3D com contorno automático (folha anexa,
retratos.png). Ele identifica o personagem, mas tem cara de render, não de ilustração. Quero
trocar por arte 2D desenhada, mais bonita e mais legível em tamanho pequeno.

Como o retrato é usado no jogo:
- O jogo carrega um arquivo PNG por personagem, de 256 x 256 px, com fundo transparente.
- Na loja ele aparece com 44 px de lado, dentro de um cartão bege claro (#F3E2B8) com uma faixa
  fina da cor da categoria no topo, e a parte de baixo fica coberta pelo rodapé do preço. Veja
  retratos_loja.png.
- O mesmo arquivo aparece maior (84 px) no cartão do herói escolhido e em 46 a 48 px nas grades
  de seleção.
Por isso o desenho precisa ler bem a 44 px: rosto e chapéu grandes, poucos detalhes, silhueta
clara.
</contexto>

<tarefa>
Desenhe 40 retratos 2D, um para cada personagem da folha anexa, mantendo a identidade de cada
modelo 3D. Antes de desenhar, olhe o personagem na folha e anote o que o identifica: cor do
pelo, chapéu ou cabelo, o que está no rosto, roupa e o objeto na mão. Esses elementos têm de
estar no retrato, nas mesmas cores.

Ordem na folha (8 por linha, da esquerda para a direita, de cima para baixo) e nome do arquivo:

Torres
 1. dardo        macaco de topete com lenço azul e um dardo
 2. bumerangue   macaco de crista com bumerangue bege
 3. bomba        canhão preto sobre rodas, sem macaco
 4. tachinha     torre vermelha redonda com canos em volta, sem macaco
 5. gelo         macaco de pelo azul gelo
 6. cola         macaco de capacete amarelo com pistola de cola
 7. sniper       macaco de capacete verde com rifle
 8. submarino    submarino amarelo e azul, sem macaco
 9. bucaneiro    macaco num barco de vela branca e vermelha
10. as           avião vermelho e amarelo, sem macaco
11. heli         helicóptero verde, sem macaco
12. morteiro     macaco de capacete cinza ao lado do tubo do morteiro
13. dartling     macaco de faixa verde com metralhadora de dardos no tripé
14. mago         macaco de chapéu pontudo azul e túnica azul, com cajado
15. super        macaco de capa azul com estrela dourada no peito
16. ninja        macaco de capuz preto com borda vermelha e shuriken
17. alquimista   macaco de óculos redondos, avental branco e frasco roxo
18. druida       macaco de coroa de folhas verdes, túnica verde e cajado
19. fazenda      canteiro de terra com bananeiras, sem macaco
20. espinhos     fábrica cinza de teto vermelho que solta espinhos, sem macaco
21. vila         cabana de teto laranja com bandeira vermelha, sem macaco
22. engenheiro   macaco de capacete laranja, macacão azul, chave inglesa e pistola de pregos

Heróis
23. quincy       capuz verde com borda dourada, aljava e arco
24. gwendolin    crista laranja, óculos amarelos, jaleco branco e lança-chamas
25. striker      boina verde, colete verde com estrela dourada e bazuca no ombro
26. obyn         chifres claros, coroa de folhas, capa verde e cajado
27. churchill    tanque verde com a cabeça do macaco de capacete na escotilha
28. benjamin     fones azuis, óculos ciano e laptop
29. ezili        caveirinha branca na cabeça, capa e saia roxas, cajado de orbe verde
30. pat          macaco grande e forte, faixa vermelha na testa, sem arma
31. adora        coroa dourada com raios de sol, túnica branca e dourada, cetro
32. brickell     quepe azul-marinho com estrela, farda azul-marinho com dourado e sabre
33. etienne      fones vermelhos, lenço azul, controle remoto e um drone ao lado
34. sauda        faixa preta na testa e duas espadas
35. psi          pelo lilás, antena com bolinha rosa, saia roxa e orbes flutuando
36. geraldo      chapéu marrom de aba, mochila grande, colete verde e frasco rosa
37. corvus       capuz preto com borda ciano, lança de ponta ciano e espíritos em volta
38. rosalia      topete rosa, óculos dourados, cachecol rosa, jetpack e pistola
39. dan          chapéu preto com fita e pena vermelhas, capa vermelha e florete
40. silas        pelo azul gelo, chapéu pontudo branco, capa azul e cajado de cristal
</tarefa>

<estilo>
- Ilustração 2D vetorial, não render 3D: cor chapada, uma sombra chapada mais escura por forma
  e, no máximo, um brilho pequeno. Sem degradê suave, sem textura, sem luz volumétrica.
- Contorno de tinta escuro (#16141A) em volta de tudo, com 8 a 10 px de espessura no arquivo de
  256 px, mais fino (4 a 5 px) nas linhas de dentro. A 44 px o contorno de fora ainda precisa
  aparecer.
- Macacos em busto, de frente, levemente virados para a direita de quem olha: cabeça ocupando
  cerca de 60% da altura, ombros, e a mão que segura o objeto entrando no quadro. Olhos grandes
  e expressivos. A parte de baixo do busto pode ser cortada reta, porque o rodapé do preço
  cobre.
- Máquinas e construções (bomba, tachinha, submarino, as, heli, fazenda, espinhos, vila e o
  tanque do churchill) inteiras, em três quartos, ocupando o quadro.
- Mesmo rosto e mesma proporção de macaco nos 31 retratos de macaco: a diferença entre eles é
  pelo, chapéu, roupa e objeto. Só o pat é visivelmente maior e mais largo.
- Paleta viva e quente, a mesma dos modelos: pelo marrom (#96602D a #A86A34), pele creme
  (#F0CEA0), e as cores de roupa que aparecem na folha.
- Margem de uns 10 px transparentes em toda a volta, para o contorno não encostar na borda.
</estilo>

<restricoes>
- Não copie o traço nem os trajes dos personagens do Bloons TD 6 ou de outro jogo. A referência
  é só a folha anexa, que é arte própria deste projeto.
- Não invente acessório que o modelo não tem, nem troque cor de roupa ou de pelo.
- Sem texto, letra, número, moldura, fundo ou sombra no chão dentro do retrato.
- Fundo 100% transparente.
- Um personagem por arquivo, centralizado.
</restricoes>

<formato_de_saida>
1. Primeiro, uma folha de conferência única (8 colunas por 5 linhas, na ordem da lista, sobre
   fundo #F3E2B8) para eu aprovar o estilo antes da exportação.
2. Ao lado, a mesma folha reduzida, com cada retrato a 44 px, para conferir a leitura no tamanho
   da loja.
3. Depois da aprovação, 40 arquivos PNG de 256 x 256 px com transparência, com exatamente estes
   nomes (minúsculos, sem acento): dardo.png, bumerangue.png, bomba.png, tachinha.png, gelo.png,
   cola.png, sniper.png, submarino.png, bucaneiro.png, as.png, heli.png, morteiro.png,
   dartling.png, mago.png, super.png, ninja.png, alquimista.png, druida.png, fazenda.png,
   espinhos.png, vila.png, engenheiro.png, quincy.png, gwendolin.png, striker.png, obyn.png,
   churchill.png, benjamin.png, ezili.png, pat.png, adora.png, brickell.png, etienne.png,
   sauda.png, psi.png, geraldo.png, corvus.png, rosalia.png, dan.png, silas.png.
   Se a exportação em PNG não for possível, entregue um SVG por personagem com os mesmos nomes
   (viewBox 0 0 256 256, sem fundo).
</formato_de_saida>

<criterios_de_aceite>
- Olhando só o retrato a 44 px, dá para dizer qual é o personagem sem ler o nome.
- Cada retrato tem os elementos de identificação listados na tarefa, nas cores da folha.
- Os 40 parecem do mesmo artista: mesmo contorno, mesma sombra, mesmo rosto de macaco.
- Nenhum retrato parece render 3D.
</criterios_de_aceite>
```

## Como aplicar no jogo

Salve os 40 arquivos em `assets/retratos/`, por cima dos atuais, com os mesmos nomes. O jogo já
carrega dessa pasta (`m3d::retrato` em `src/cliente/modelos3d.cpp`); não precisa recompilar. Se
a entrega vier em SVG, converta para PNG de 256 px antes. Para voltar aos retratos gerados do
modelo 3D: `blender --background --python tools/blender/gerar_retratos.py`.
