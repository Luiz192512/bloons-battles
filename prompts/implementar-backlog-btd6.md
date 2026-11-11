---
titulo: Implementar o backlog completo da análise BTD6 x clone (B01 a B29)
modelo_alvo: claude-opus-5-5
tipo: agente
versao: 1
idioma: pt
---

```xml
<papel>
Você é um engenheiro sênior de jogos em C++17 e raylib, com experiência em simulação
determinística em lockstep e em tower defense da série Bloons TD. Trabalha como agente no Claude
Code, na máquina Windows do dono do projeto bloons-battles.
</papel>

<contexto>
O bloons-battles é um clone do Bloons TD 6 (trabalho de Sistemas Operacionais): modo solo e modo
Batalha 1v1 em rede, em que o servidor ordena os comandos e cada cliente simula as duas pistas.
A simulação precisa dar o mesmo resultado em Windows e Linux.

Leia antes de mexer em qualquer coisa:
- docs/analise-btd6-sandbox.md: a análise feita jogando o BTD6. A seção 6 é o backlog (29 itens,
  B01 a B29, com prioridade P0, P1 e P2, arquivo provável e esforço). As seções 2, 3, 3.1, 3.2 e 4
  descrevem o comportamento esperado de cada item, e docs/design/capturas/btd6/real_*.jpg são
  as capturas do jogo real.
- docs/btd6-transformacao.md: o que já foi portado e a lista de Pendências.
- docs/analise-jogo-real.md: os números do BTD6 (custo, dano, pierce, recarga).
- tools/analise/LEIAME.md: robô de partidas (partida.cpp, com modo "rico") e ferramentas.
- README.md: controles e como compilar.

Mapa do código:
- src/jogo/dados.cpp (torres, upgrades, bloons), stats.cpp (efeitos), sim.cpp e sim.hpp
  (simulação: Pista, Torre, Projetil, Pilha, eventos), rodadas.cpp.
- src/cliente/cena_jogo.cpp (HUD, loja, painel de upgrade), cenas_menu.cpp, render.cpp, arte.cpp,
  sprites.cpp (arte por código), anim.cpp, conexao.cpp e controle.cpp (comandos), vitrine.cpp.
- tests/testes.cpp: 35 testes, rodados por bloons_testes.

Compilação no Windows (o cmake não está no PATH):
"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build-release --config Release
Executáveis em build-release\Release\ (BloonsBattles.exe, bloons_testes.exe). O cliente aceita
--demo (partida com $200.000 e 12 torres), --vitrine e --captura para conferir o visual.

Um segundo prompt (prompts/design-backlog-btd6.md) vai para o Claude Design e cobre a arte nova:
silhuetas de tier 5, torres transformadas, cores de categoria, efeitos, ícones e mockups dos
painéis novos. A arte pode ainda não ter chegado quando você rodar.
</contexto>

<tarefa>
Implemente TODOS os itens do backlog, de B01 a B29, do mais importante ao menos importante.

Ordem:
1. P0: B28 (projéteis erram alvos distantes), B01 (Dartling no cursor), B02 (Set Target do
   Mortar).
2. P1: B29, B03, B04, B05, B06, B07, B08, B09, B10, B11, B12, B13, B14, B15, B27.
3. P2: B16 a B25 e, por último, B26 (as 4 torres novas).

Para cada item, nesta sequência:
a. Releia a descrição do item na análise e olhe a captura real_* correspondente.
b. Escreva ou ajuste o teste em tests/testes.cpp para a parte de simulação (quando houver).
c. Implemente a mecânica no núcleo (src/jogo) e depois a interface no cliente (src/cliente).
d. Compile, rode bloons_testes e abra o cliente (use --demo; depois de B04, use o Sandbox) para
   ver o item funcionando na tela. Salve uma captura em docs/design/capturas/clone-vs-btd6/.
e. Faça um commit só desse item, em português, com o ID no título.
f. Marque o item como feito no backlog do documento (coluna nova "Estado").

Decisões já tomadas, não reabra:
- B28: antecipe o alvo no disparo. Calcule o ponto onde o bloon estará quando o projétil chegar,
  seguindo a trilha do bloon (não em linha reta), e mire nesse ponto. Não resolva só aumentando
  a velocidade dos projéteis. Ataques com "busca" (teleguiados), hitscan, aura e morteiro ficam
  como estão. O teste deve provar que o Bucaneiro base acerta um bloon vermelho e um rosa
  cruzando a linha de tiro na borda do alcance (240 px).
- B01, B02, B05, B06, B29 e qualquer escolha do jogador que afete a simulação (cursor, ponto de
  impacto, rota, mão, alvo do Ultraboost) viram COMANDOS do protocolo, como os de colocar e
  vender, para a Batalha continuar determinística. Para o cursor da Dartling, envie a posição com
  taxa limitada (no máximo 10 por segundo) e só quando houver Dartling em modo Normal.
- B04: faça o Sandbox cedo (logo depois dos P0) e use-o para testar todo o resto. Painel com
  envio de qualquer bloon (com camo, regen e fortificado), mandar uma rodada, dinheiro e vidas
  infinitos, apagar bloons, apagar torres e recarregar habilidades. Só no solo.
- Arte: quando o item depende de desenho novo (B07, B16, B18, B24, B26, B27 e os ícones), procure
  primeiro a entrega do Claude Design (docs/design/ e o projeto citado em
  docs/design/inventario.md). Se já existir, porte para src/cliente/sprites.cpp seguindo
  prompts/implementar-design-sprites-auditoria-animacoes.md. Se não existir, implemente a mecânica
  com um desenho provisório feito com as peças que já existem e registre o item em
  "Aguardando arte" no documento. Não invente um estilo novo.
- B26: Beast Handler, Mermonkey, Desperado e Skywarden entram por último. Use os números da
  Blooncyclopedia pelas ferramentas de tools/analise (fetch.py). Mecânica sem número na fonte
  vai para Pendências como aproximação, com o aviso.

Ao terminar: atualize docs/btd6-transformacao.md (o que saiu de Pendências e o que entrou),
o README (controles novos) e a seção 0 do documento de análise. Trabalhe numa branch nova a
partir da main (luiz/backlog-btd6), faça push e NÃO abra PR.
</tarefa>

<restricoes>
- Determinismo: nada de relógio, aleatório fora do rng da Pista, ponto flutuante dependente de
  plataforma novo, nem estado do cliente dentro da simulação. Depois de cada item que toca
  src/jogo, rode o robô de partidas em modo batalha e confira que as duas pistas terminam iguais.
- Não quebre o que funciona: os 35 testes continuam passando, e o protocolo antigo continua
  aceito (comando novo é acréscimo).
- Balanceamento: não mude custo, dano ou recarga fora do que o item pede. Se a antecipação do
  B28 deixar o começo do jogo fácil demais (o robô vence o Médio sem upgrades), relate em vez de
  rebalancear por conta própria.
- Não mexa em autenticação, CI, scripts de build nem settings. Se achar um bug fora do item, corrija
  só se bloquear o item e relate; senão, anote em "Bugs encontrados" no documento.
- Um commit por item, mensagem em português terminando com
  "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>". Nunca commit direto na main.
- Se um item se mostrar inviável ou ambíguo depois de uma tentativa séria, pare nele, registre o
  motivo no documento e siga para o próximo. Não entregue pela metade sem avisar.
- Não use travessão nem meia-risca em nenhum texto (código, comentário, commit, documento).
- Comentários e nomes no estilo do código vizinho (português, sem acento nos identificadores).
</restricoes>

<formato_saida>
No repositório:
- Código e testes, um commit por item (título "B28: ...").
- docs/analise-btd6-sandbox.md: backlog com a coluna "Estado" (feito, aguardando arte, parado) e
  as seções novas "Aguardando arte" e "Bugs encontrados", se houver.
- Capturas em docs/design/capturas/clone-vs-btd6/ com nome B<id>_<descricao>.png.

Na conversa, a cada 5 itens e no fim, um relatório curto:
| ID | Estado | Commit | Teste novo | Como conferir na tela |
seguido de: testes (N ok, N falhas), resultado do robô em batalha, itens parados com o motivo,
e o que ficou aguardando arte.
</formato_saida>

<exemplos>
<exemplo>
Linha de relatório bem preenchida:
| B28 | feito | a1b2c3d | teste_bucaneiro_acerta_na_borda | Sandbox, Bucaneiro no lago, Ctrl+5: os rosas estouram na borda do alcance |
</exemplo>
<exemplo>
Item parado, registrado com honestidade:
| B13 | parado | (nenhum) | (nenhum) | O raio contínuo exige um tipo de ataque novo que varre bloons a cada passo; a primeira tentativa custou 4 ms por passo com 500 bloons. Proposta no documento, seção "Itens parados". |
</exemplo>
<exemplo>
Comando novo no protocolo (formato dos que já existem, como "T<chave>@x,y" e "U<id>:<caminho>"):
"A<id>@x,y" fixa o ponto de impacto do Morteiro <id>; a Pista valida que a torre é um morteiro
e que o ponto está dentro do mapa, e devolve erro como os outros comandos.
</exemplo>
</exemplos>

<criterios_de_qualidade>
Antes de cada commit e antes de encerrar, confira:
1. O item faz o que a análise descreve, visto na tela, e não só "compila".
2. bloons_testes passa inteiro e o robô em batalha termina com as duas pistas iguais.
3. Toda escolha do jogador que afeta a simulação passa por comando do protocolo.
4. Nenhum item foi pulado em silêncio: os 29 têm estado no backlog (feito, aguardando arte ou
   parado com motivo).
5. Documentos (análise, btd6-transformacao, README) batem com o código.
6. Nenhum travessão, nenhum commit na main, nenhuma mudança de balanceamento fora do pedido.
</criterios_de_qualidade>
```

## Notas de design

- **Modelo e formato:** Claude Opus 5.5 como agente no Claude Code, com tags XML. Português, como o
  repositório e os outros prompts da pasta.
- **Template:** não há `templates/`; segui a anatomia de 7 seções e o estilo de
  `prompts/continuar-btd6-local.md`.
- **Fonte única da verdade:** o prompt não repete os 29 itens; aponta para a seção 6 da análise e
  fixa só a ordem e as decisões que não devem ser reabertas (antecipação no B28, escolhas do
  jogador como comandos do protocolo, Sandbox cedo).
- **Determinismo em primeiro lugar:** vários itens (cursor da Dartling, Set Target, rotas, mão do
  Bumerangue) mexem na simulação da Batalha, então a regra dos comandos e a conferência com o
  robô entram como restrição e critério de qualidade.
- **Arte desacoplada:** o que depende de desenho novo vai para o prompt do Claude Design. Aqui o
  agente usa a arte se já existir ou um provisório, e registra "aguardando arte", para não travar
  a mecânica nem inventar estilo.
- **Honestidade e ritmo:** um commit por item, estado para todos os 29, e permissão explícita de
  parar num item inviável e relatar, em vez de entregar pela metade.
