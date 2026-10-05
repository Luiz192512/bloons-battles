# Modelos 3D das torres (Blender)

Os modelos são gerados por script, sem abrir a interface do Blender. Precisa do Blender 4.2 LTS
(`winget install BlenderFoundation.Blender.LTS.4.2`).

## Gerar

Na raiz do repositório:

```
"C:\Program Files\Blender Foundation\Blender 4.2\blender.exe" --background --python tools/blender/gerar.py -- dardo bomba bucaneiro
```

Sem nomes depois do `--`, gera as três torres do piloto. Saídas por torre:

| Arquivo | Para quê |
|---|---|
| `assets/modelos/<chave>.glb` | o modelo que o jogo carrega, uma malha por grupo animável |
| `assets/modelos/<chave>.json` | grupo e pivô de cada malha, na ordem do `.glb` |
| `docs/design/capturas/modelos/<chave>.png` | prévia na vista 3/4, para revisar o estilo |
| `dist/blender/<chave>.blend` | cena para retocar à mão (fora do git) |

## Regras de arte

- Formas redondas (esfera, cápsula, toro, cubo com cantos arredondados) misturadas com pontudas
  (`cone` e `lamina`): pontas de arma, penas, espinhos, cristas, proas e bandeiras (`comum.py`).
- Cor chapada por peça, gravada como cor de vértice. O jogo não usa textura.
- Todo macaco usa o mesmo corpo (`macaco.py`). Entre as torres mudam a cor e o estilo da
  pelagem (`lisa`, `topete`, `crista`, `tufos`, `barba`), a roupa e a arma. Só o Super Macaco
  usa `escala` maior.
- O modelo fica em pé na origem, com 1,0 de altura para o macaco, olhando para -Y.

## Classificação das torres

| Grupo | Torres | O que se modela |
|---|---|---|
| Macacos | dardo, bumerangue, gelo, cola, sniper, morteiro, dartling, mago, super, ninja, alquimista, druida, engenheiro | o macaco padrão com pelagem, roupa e arma |
| Máquinas e estruturas | bomba, tachinha, as, heli, fazenda, espinhos, vila | só a máquina ou a construção, sem macaco |
| Aquáticos | submarino | só o submarino |
| Aquáticos | bucaneiro | o macaco em cima do barco |

## Torre nova

1. Crie `tools/blender/<chave>.py` com uma função `construir()` que monta as peças e devolve o
   pivô de cada grupo (veja `dardo.py` para macaco e `bomba.py` para máquina).
2. Grupos animáveis: `base` (parado), `corpo`, `cauda`, `cabeca`, `torreta` (gira e recua) e
   `braco` (o braço do ataque, com a arma).
3. Rode o `gerar.py` com a chave nova e confira a prévia.

## Modelagem poligonal (fluxo atual)

Os modelos por primitivas (`gerar.py`, `comum.py`, `macaco.py`) foram substituídos por malha
poligonal: `poli.py` (ferramentas), `macaco_poli.py` (o macaco padrão) e `torres_poli.py` (as
torres). Gere uma torre por execução, sem interface, com o Blender 5.2:

```
"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe" --background --python tools/blender/gerar_poli.py -- dardo
```

Saídas: `assets/modelos/<chave>.glb` e `.json`, a fonte em `assets/modelos/fonte/<chave>.blend`
e as folhas de revisão em `docs/design/capturas/modelos/<chave>_*.png`.

Como cada parte é feita:
- Corpo, cabeça e braço do macaco: bloco de volumes fundido numa casca só, retopologia em quads
  (QuadriFlow) e cores cortadas na malha pelo contorno de cada mancha.
- Peças duras (cano, rodas, luneta, pontas): perfil torneado em quads (`torno`) ou caixa
  chanfrada (`caixa`), com quinas vivas.
- Roupas, pelagem e cauda: tubos de quads com subdivisão (`membro`) e gaiolas (`gaiola`).

Orçamento em triângulos, medido no `.glb`: **teto de 10.000 por torre** (decisão do dono).

| Modelo do piloto | Triângulos |
|---|---|
| dardo | 5.568 |
| bomba | 1.984 |
| bucaneiro | 8.872 |

A exportação pelo Blender aberto (conector) com troca de contexto derrubou o Blender 5.2; por
isso a geração roda sem interface.

## Variações de upgrade (montadas por partes)

Cada torre tem 64 variações (um caminho até 5, outro até 2, o terceiro em 0). Nenhuma é modelada
à mão: o montador `partes.py` compõe as peças que a torre declara em `torres/<chave>.py`.

```
"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe" --background --python tools/blender/gerar_variacoes.py -- dardo
python tools/blender/conferir_variacoes.py dardo
```

Saídas: `assets/modelos/<chave>/<a>-<b>-<c>.glb` e `.json`, e as folhas de contato
`docs/design/capturas/modelos/<chave>_variacoes.png` (8x8, vista do jogo) e
`<chave>_variacoes_48.png` (no tamanho do mapa). Com combinações depois da chave
(`-- dardo 3-2-0`), gera só essas e as vistas de revisão em `dist/modelos/<chave>/`.

| Arquivo | O que faz |
|---|---|
| `partes.py` | o montador: encaixes, regras de composição, exportação e medição do `.glb` |
| `pecas.py` | peças reaproveitadas (faixa, capa, casca de cabeça, óculos, tênis, dardo, besta...) |
| `torres/<chave>.py` | a torre: `base(m)`, `CAMINHOS` (peça de cada tier) e `ENQUADRE` |
| `gerar_variacoes.py` | gera as variações e as folhas de contato, sem interface |
| `conferir_variacoes.py` | confere triângulos, malhas, cor e `.json` de cada variação |

Torre nova no montador:

1. Escreva a tabela de peças em `docs/modelos-3d-variacoes.md` (o que cada tier acrescenta e em
   qual encaixe).
2. Em `torres/<chave>.py`, `base(m)` monta a variante 0-0-0 (`m.macaco(...)` para as torres com
   macaco, `m.por(encaixe, pecas)` para cada peça).
3. Em `CAMINHOS`, os tiers 1 e 2 são acessórios, `[(encaixe, funcao), ...]` em ordem de
   preferência; os tiers 3 a 5 são conjuntos, uma função que troca traje, arma e silhueta.
4. O caminho cruzado só entra com os tiers 1 e 2, no primeiro encaixe que o principal não ocupou.
   Se a peça do principal esconder um encaixe sem usá-lo, declare com `m.ocupar(encaixe)`.

Macaco de capuz não tem orelhas: `pecas.capuz` troca a cabeça pela versão sem orelhas
(`m.sem_orelhas()`), para o capuz encaixar justo.

As regras completas e o estado de cada torre estão em `docs/modelos-3d-variacoes.md`.

## Heróis

Os 18 heróis saem de `herois.py`: uma função por herói, que veste o macaco padrão com as peças de
`pecas.py` (mais algumas só deles: espada, arco, fones, boina, orbes). Herói não tem caminho de
upgrade, então só existe a variação `0-0-0`.

```
"C:\Program Files\Blender Foundation\Blender 5.2\blender.exe" --background --python tools/blender/gerar_herois.py
```

Com chaves depois do `--` (por exemplo `-- quincy pat`), gera só esses. Saídas:
`assets/modelos/<chave>/0-0-0.glb` e `.json`, e as folhas `docs/design/capturas/modelos/herois.png`
(vista do jogo) e `herois_3q.png` (três quartos), na ordem de `herois.HEROIS`.

Regras: o desenho segue o herói 2D do próprio jogo (`src/cliente/sprites.cpp`, `HS()`), sem copiar
traje de outro jogo. Só Pat Fusty é maior que os outros. O Capitão Churchill é um tanque: casco na
malha `base` e torre, canhão e cabeça na malha `torreta`, que o jogo gira para a mira. Psi e Silas
têm pelo de outra cor (lilás e gelo).
