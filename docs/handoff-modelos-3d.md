# Passagem de contexto: modelos 3D das torres (01/10/2026)

Contexto para a conversa que vai executar `prompts/modelar-todas-variacoes-por-partes.md`.
Tudo está na branch `luiz/modelos-3d`.

## Onde estamos

- O jogo (C++17 e raylib) ainda desenha as torres em 2D por código. Nenhum modelo 3D é carregado
  pelo jogo ainda; a integração (carregar `.glb`, câmera, luz) é um passo futuro e não faz parte
  da tarefa atual.
- O piloto de modelos 3D está aprovado pelo dono: Dardo, Canhão Bomba e Bucaneiro na variante
  base (0-0-0), em malha poligonal. Folhas de revisão em `docs/design/capturas/modelos/`.
- Tarefa atual: gerar as 64 variações de upgrade de cada uma das 22 torres, montadas por partes.

## Decisões do dono (não reabrir)

1. Modelagem poligonal de verdade (malha contínua, quads), não personagem de primitivas
   empilhadas. As tentativas anteriores (Claude Design e `gerar.py` com esferas e cones) foram
   rejeitadas.
2. Formas redondas misturadas com pontudas.
3. Todo personagem é um macaco e todos usam o MESMO corpo. Entre as torres mudam o estilo da
   pelagem, a roupa e a arma. Só o Super Macaco é maior.
4. Paleta do macaco: padrão (pelo marrom, pele clara) em todas as torres, menos no Macaco de
   Gelo, que é azul claro como no jogo original (tons tirados de
   `docs/design/capturas/btd6/real_ice_*.jpg`).
5. Sem macaco: bomba, tachinha, as, heli, fazenda, espinhos, vila. Submarino é só o submarino.
   Bucaneiro é o macaco em cima do barco.
6. Teto de 10.000 triângulos por variação, medidos no `.glb`. O dono achou feias as densidades
   baixas; a que ele aprovou dá uns 5.500 triângulos num macaco vestido.
7. Rebranding do figurino: não copiar os trajes nem os personagens da Ninja Kiwi.
8. O ajuste fino de design fica para depois. Não mexer no estilo do piloto.

## Código que já existe

| Arquivo | O que faz |
|---|---|
| `tools/blender/poli.py` | ferramentas: `gaiola`, `extrudir`, `membro`, `torno`, `caixa`, `loft`, `remalhar`, `colorir`, `juntar`, `exportar`, `render` |
| `tools/blender/macaco_poli.py` | o macaco padrão (corpo, cauda, cabeca, braco) e as pelagens |
| `tools/blender/torres_poli.py` | dardo, bomba e bucaneiro (variante base) |
| `tools/blender/gerar_poli.py` | gera uma torre por execução, sem interface |
| `tools/blender/LEIAME.md` | regras de arte e fluxo |
| `tools/blender/{comum,macaco,dardo,bomba,bucaneiro,gerar}.py` | fluxo antigo por primitivas; só `comum.malhas_do_glb` e `comum.para_gltf` ainda são usados |

Triângulos do piloto: dardo 5.564, bomba 1.984, bucaneiro 8.868.

## Armadilhas já encontradas

- **Gerar sempre sem interface**, com o Blender 5.2:
  `"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe" --background --python tools/blender/gerar_poli.py -- dardo`.
  Pelo conector do Blender, a exportação com `temp_override` travou o Blender aberto e derrubou
  o Blender em linha de comando.
- No modo sem interface, o cubo, a câmera e a luz padrão entram na exportação se não forem
  apagados antes (o `gerar_poli.py` já apaga).
- Malha juntada (`poli.juntar`) precisa ter a cor `Col` marcada como ativa, senão o `.glb` sai sem
  cor (já tratado).
- A borda entre cores só fica limpa cortando a malha no contorno da mancha (`poli.colorir`);
  subdividir mais não resolve.
- Scripts longos em heredoc do bash quebram com aspas: escreva o arquivo e rode.
- O cmake não está no PATH; use o do Visual Studio (ver `prompts/implementar-backlog-btd6.md`).

## Referências

- `src/jogo/dados.cpp`: os 15 upgrades de cada torre (nomes em português).
- `docs/analise-btd6-sandbox.md`, seção 3: o que cada upgrade muda no jogo original.
- `docs/design/capturas/btd6/real_*.jpg`: capturas do BTD6 (tiers 3 e 5 de cada caminho).
- `dist/variacoes/`: prints por combinação. Fica fora do git e só existe na pasta principal do
  repositório (`C:\Users\forti\Documents\Projetos\faculdade\bloons-battles\dist\variacoes`); hoje
  tem o Dart completo e parte do Boomerang.

## Trabalho parado em outras frentes

- Branch `luiz/backlog-btd6`: correção B28 (projéteis antecipam o alvo) escrita e com teste, mas
  sem commit, e a última compilação foi interrompida. Não faz parte desta tarefa.
- `tools/analise/variacoes_btd6.ps1`: captura automática dos prints por combinação; a versão com
  pausa pelo mouse não foi testada.
