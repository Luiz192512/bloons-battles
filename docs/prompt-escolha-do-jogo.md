---
titulo: Sugestão de jogos PvP com UI gráfica não utilizados (SO Trabalho 02)
modelo_alvo: claude-opus-5-5
tipo: template
versao: 3
idioma: pt
---

```xml
<papel>
Você é um professor sênior de Sistemas Operacionais e game developer com experiência em jogos
multiplayer 2D (sprites, game loop, netcode cliente/servidor). Há mais de 10 anos você orienta
trabalhos acadêmicos de comunicação entre processos (sockets, threads, memória compartilhada,
exclusão mútua) e sabe dimensionar um jogo com interface gráfica para caber no prazo de uma dupla.
</papel>

<contexto>
Uma dupla de Engenharia de Software (ESOFT, 4º semestre) precisa escolher um jogo para o
Trabalho 02 do 2º bimestre de Sistemas Operacionais. O jogo não pode repetir nenhum jogo já
indicado por outra dupla.

A dupla quer um jogo COM IMAGENS E INTERFACE GRÁFICA (sprites, telas, HUD, animações simples),
no estilo de jogos PvP online famosos como:
- Bloons TD Battles 2 (tower defense 1v1: cada jogador defende sua trilha e envia inimigos para
  a trilha do adversário);
- Pokémon Showdown (batalha competitiva por turnos com escolha simultânea de ações).
Ou seja: jogos de videogame competitivos ou cooperativos conhecidos, e não jogos de tabuleiro ou
baralho tradicionais.

<requisitos>
R1. Jogo para pelo menos 2 usuários, cada um em um processo distinto.
R2. Comunicação entre processos por socket (cliente/servidor local; em C, winsock2.h e pthreads.h,
    a partir do código base do professor). Bibliotecas de rede de alto nível que escondam o socket
    (ex.: Netcode do Unity, Photon) NÃO devem ser usadas.
R3. Memória compartilhada entre os usuários, com gerenciamento de exclusão mútua (o formulário
    pergunta "Como é feito o gerenciamento da exclusão mútua?").
R4. Ações do jogo controladas por códigos dentro das mensagens; estado como "vez" existe nos dois
    processos e é sincronizado pelas mensagens.
R5. Notação própria de mensagens criada pela dupla (exemplo do enunciado: "2Cc3" no xadrez).
R6. Linguagens aceitas: C, C++, C#, Java, Python, Node.js, Go (outras só com consulta prévia).
R7. O enunciado não exige interface bonita, mas a dupla QUER UI gráfica com imagens. A UI não pode
    consumir o tempo necessário para R2 a R5, que valem a nota.
R8. Jogo não pode ser igual ao de outra dupla (copiados ou duplicados são zerados).
R9. Entrega até 15/11/2026 23:59, com vídeo de gameplay de 1 minuto e código com e sem executável.

Critérios de avaliação (3,0 pontos): originalidade, uso da troca de mensagens, uso do
gerenciamento de memória, técnicas implementadas, criatividade na solução, organização do código,
vídeo de gameplay.
</requisitos>

<jogos_ja_utilizados>
Batalha Naval; Dama; Blackjack 1x1; Duelo de Pênaltis; Termo; Wumpus 1x1; Lig 4 (Connect Four);
Jogo da Forca; Pedra, papel ou tesoura; Pedra-papel-tesoura-lagarto-Spock; Ping-Pong; Pong; Sinuca;
Pokémon/Combate por turno; Dúvido (baralho); Jogo de luta; Cara a Cara; Uno; Campo Minado 1v1;
RPG de turno coop; Truco; Fodinha; Duelo de Justa; Xadrez; Dominó; Puzzle Coop;
Survival horror/jogo de tiro; RPG survival; Jogo da Velha v2; Sobrevivência;
Jogo da memória e tabuleiro Ouija; Corrida de cavalo; Adivinhe o Número; Pife; BinaWar;
Quem sou eu?; Slither (Cobrinha)/4 seguidos; Escape Room; Bomber Friends (Bomberman);
Nem a Pato; Top 10; Quem é o impostor; Jeopardy; Corrida 1v1v1; Caça ao Tesouro; Onitama.
</jogos_ja_utilizados>

<perfil_da_dupla opcional="true">
{{LINGUAGEM PREFERIDA, EXPERIÊNCIA COM ENGINE/BIBLIOTECA GRÁFICA, TEMPO DISPONÍVEL.}}
</perfil_da_dupla>
</contexto>

<tarefa>
1. Sugira 8 jogos de videogame FAMOSOS, no estilo de Bloons TD Battles 2 e Pokémon Showdown
   (PvP ou coop online, com visual), que NÃO estejam na lista e atendam a R1 a R9.
2. Para cada um, defina uma versão reduzida viável para a dupla (o "MVP" que cabe no prazo),
   deixando claro o que fica de fora do jogo original.
3. Descreva a notação de mensagens, a memória compartilhada, a seção crítica e a stack gráfica.
4. Ordene do mais recomendado ao menos recomendado (nota esperada x risco de prazo) e termine com
   uma recomendação única.
</tarefa>

<restricoes>
- Só jogos famosos e nomeados. Nada de gêneros genéricos nem jogos inventados.
- Proibido repetir jogo da lista ou mecânica central de um deles. Conflitos já conhecidos:
  qualquer jogo de Pokémon ou batalha de monstros por turnos (= "Pokémon/Combate por turno");
  jogos de luta como Street Fighter e Smash (= "Jogo de luta"); Bomberman (= Bomber Friends);
  Slither.io e Agar.io (= Slither); Among Us (= "Quem é o impostor"); Mario Kart e corridas
  (= "Corrida 1v1v1"); shooters (= "Survival horror/jogo de tiro"); Air Hockey (≈ Pong).
- Pokémon Showdown serve só como REFERÊNCIA de estilo (escolha simultânea, log de batalha,
  UI competitiva). Não o sugira.
- Se houver semelhança parcial com a lista, declare em "Conflito com a lista" em vez de omitir.
- Stack gráfica deve usar socket puro por baixo (ex.: Python + pygame + socket; Java + libGDX ou
  JavaFX + java.net; C# + MonoGame + System.Net.Sockets; C + raylib + winsock2; Node.js + Phaser
  no navegador + WebSocket/net). Não recomende Unity/Godot com rede de alto nível.
- Assets: recomende sprites gratuitos com licença livre (ex.: Kenney.nl, OpenGameArt, itch.io
  free) ou desenhados pela dupla. Não sugira extrair sprites oficiais de jogos comerciais.
- Não gere código completo; trechos de mensagem de exemplo são permitidos.
- Português do Brasil, tom direto, sem travessões.
</restricoes>

<formato_saida>
## Tabela resumo
| # | Jogo | Estilo | Tempo real ou turno | Dificuldade (1 a 5) | Stack sugerida |

## Detalhes
### 1. <Nome do jogo>
- **Como funciona o original:** 1 a 2 frases.
- **Versão da dupla (MVP):** o que entra e o que fica de fora.
- **Conflito com a lista:** "Nenhum" ou o risco explicado.
- **Telas e UI:** telas (lobby, partida, fim de jogo), HUD e elementos visuais principais.
- **Notação de mensagens:** formato + 2 exemplos (ex.: `1T3@12,5` = jogador 1 coloca torre tipo 3
  na posição 12,5) e significado de cada campo.
- **Memória compartilhada:** estruturas compartilhadas (ex.: estado da partida, dinheiro, vidas).
- **Exclusão mútua:** seção crítica, condição de corrida evitada e primitiva usada.
- **Stack e assets:** linguagem, biblioteca gráfica e fonte de sprites.
- **Diferencial para a nota:** 1 técnica extra (ex.: espectador, reconexão, replay pelo log).
- **Risco principal:** ...

## Recomendação final
**<Nome do jogo>**: justificativa em até 3 frases.
</formato_saida>

<exemplos>
<exemplo>
<saida_parcial>
### 1. Bloons TD Battles 2
- **Como funciona o original:** Dois jogadores defendem trilhas separadas com torres e gastam
  dinheiro para enviar balões extras à trilha do adversário; perde quem zerar as vidas.
- **Versão da dupla (MVP):** 1 mapa, 3 tipos de torre, 3 tipos de balão, sem heróis e sem upgrades
  em árvore.
- **Conflito com a lista:** Nenhum; não há tower defense na planilha.
- **Telas e UI:** lobby (aguardando oponente), tela dividida com as duas trilhas, HUD com
  dinheiro, vidas e botões de envio de balões, tela de vitória.
- **Notação de mensagens:** `<jogador><ação><dados>`. `1T2@08,04` = jogador 1 coloca torre
  tipo 2 em (8,4); `2S3x10` = jogador 2 envia 10 balões tipo 3; `0V1` = servidor anuncia
  vitória do jogador 1.
- **Memória compartilhada:** struct da partida com dinheiro, vidas, lista de balões ativos por trilha.
- **Exclusão mútua:** a thread que recebe "enviar balões" e a thread do game loop que remove
  balões estourados alteram a mesma lista; um mutex por trilha evita inserção e remoção
  simultâneas (lista corrompida ou balão contado duas vezes).
- **Stack e assets:** Python + pygame + socket; sprites do pacote "Tower Defense" do Kenney.nl.
- **Diferencial para a nota:** modo espectador conectado como terceiro cliente.
- **Risco principal:** sincronizar o tempo das ondas nos dois clientes.
</saida_parcial>
</exemplo>
<contraexemplo motivo="conflito com a lista">
"Pokémon Showdown": rejeitado, a planilha já tem "Pokémon/Combate por turno".
</contraexemplo>
</exemplos>

<autoavaliacao>
Antes de responder, verifique em silêncio e corrija o que falhar:
1. Os 8 jogos são videogames famosos, no estilo PvP/coop com visual, e reconhecíveis pelo nome?
2. Nenhum repete jogo ou mecânica central da lista (incluindo os conflitos listados)?
3. Cada MVP cabe no prazo de uma dupla sem sacrificar R2 a R5?
4. Cada jogo tem notação com exemplos, memória compartilhada e seção crítica com condição de
   corrida real?
5. A stack usa socket de verdade e os assets são de licença livre?
6. Há uma única recomendação final, em até 3 frases?
Só emita a resposta quando todas as respostas forem "sim".
</autoavaliacao>
```

## Notas de design

- **Versão 3:** mudou o público-alvo de jogos de tabuleiro para videogames PvP/coop com UI
  gráfica, usando Bloons TD Battles 2 e Pokémon Showdown como referência de estilo.
- **Pokémon Showdown só como referência:** a planilha já tem "Pokémon/Combate por turno", então o
  prompt o proíbe como sugestão e usa isso como contraexemplo.
- **Seção "MVP":** jogos comerciais são grandes demais para uma dupla; exigir o recorte evita
  sugestões inviáveis no prazo.
- **Stack com socket puro:** R2 exige socket explícito; engines com rede pronta esconderiam
  justamente o que é avaliado.
- **Assets livres:** evita problema de direitos autorais no vídeo de gameplay entregue.
- **Tags XML para Claude:** para GPT/Gemini, trocar por headers Markdown.
