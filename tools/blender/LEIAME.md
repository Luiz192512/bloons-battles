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
"C:\Program Files\Blender Foundation\Blender 5.2lender.exe" --background --python tools/blender/gerar_poli.py -- dardo
```

Saídas: `assets/modelos/<chave>.glb` e `.json`, a fonte em `assets/modelos/fonte/<chave>.blend`
e as folhas de revisão em `docs/design/capturas/modelos/<chave>_*.png`.

Como cada parte é feita:
- Corpo, cabeça e braço do macaco: bloco de volumes fundido numa casca só, retopologia em quads
  (QuadriFlow) e cores cortadas na malha pelo contorno de cada mancha.
- Peças duras (cano, rodas, luneta, pontas): perfil torneado em quads (`torno`) ou caixa
  chanfrada (`caixa`), com quinas vivas.
- Roupas, pelagem e cauda: tubos de quads com subdivisão (`membro`) e gaiolas (`gaiola`).

Orçamento em triângulos, medido no `.glb` (decisão do dono: densidade alta):

| Classe | Orçamento | Piloto |
|---|---|---|
| Macaco com roupa e arma | até 6.000 | dardo: 5.760 |
| Máquina ou estrutura pequena | até 2.500 | bomba: 1.984 |
| Barco com macaco | até 8.000 | bucaneiro: 7.908 |

A exportação pelo Blender aberto (conector) com troca de contexto derrubou o Blender 5.2; por
isso a geração roda sem interface.
