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

- Só formas redondas: esfera, cápsula, toro e cubo com cantos bem arredondados (`comum.py`).
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
