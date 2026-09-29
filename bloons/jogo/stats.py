"""Calculo dos atributos de uma torre a partir da definicao + upgrades + nivel de heroi."""

from __future__ import annotations

import copy

from bloons.jogo.bloons_def import AFIADO
from bloons.jogo.herois_def import HEROIS
from bloons.jogo.torres_def import AUXILIARES, TORRES, DefTorre

PADRAO_ATAQUE = dict(
    tipo="projetil", cad=1.0, dano=1, pierce=1, dtype=AFIADO, vel=600.0, dist=250.0, n=1,
    spread=0.0, busca=False, boom=False, raio_proj=5.0, splash=0.0, sdano=0, spierce=0,
    sdtype=None, moab=0, cer=0, fort=0, lento=None, congela=0.0, cola=None, queima=None,
    atordoa=0.0, empurra=0.0, fragiliza=0, retira_camo=False, retira_regen=False, quica=0,
    frag=None, global_=False, visual="dardo", alvo=None, so_moab=False, moab_lento=False,
    moab_congela=False, moab_cola=False, moab_atordoa=False, raio_aura=0.0,
    pilha_pierce=0, pilha_vida=10.0, saltos=0, valor=0.0, buffs=None, fusivel=0.0,
    na_trilha=False, impreciso=0.0, linha=False, dur=0.0, base=None, nivel_inv=0,
)

_SOMA = {"dano", "pierce", "n", "splash", "sdano", "spierce", "quica", "moab", "cer",
         "fort", "dist", "raio_proj", "valor", "pilha_pierce", "saltos", "empurra",
         "fragiliza", "nivel_inv", "raio_aura"}
_MULT = {"cad", "vel", "pilha_vida", "impreciso"}
_NIVEL_TORRE = {"alcance", "alcance_x", "camo", "ouro", "desconto", "venda", "hab",
                "persegue"}


def novo_ataque(d: dict) -> dict:
    at = dict(PADRAO_ATAQUE)
    at.update(copy.deepcopy(d))
    if at["buffs"] is None:
        at["buffs"] = {}
    return at


class Stats:
    """Atributos efetivos (antes de buffs temporarios) de uma torre."""

    def __init__(self, dfn: DefTorre) -> None:
        self.alcance = dfn.alcance
        self.camo = dfn.camo
        self.ouro = 0.0
        self.desconto = 0.0
        self.venda = 0.7
        self.persegue = False
        self.habs: list[dict] = []
        self.ataques = [novo_ataque(a) for a in dfn.ataques]


def _mesclar_buffs(alvo: dict, novos: dict) -> None:
    for k, v in novos.items():
        if k == "cad":
            alvo["cad"] = alvo.get("cad", 1.0) * v
        elif isinstance(v, bool):
            alvo[k] = v
        else:
            alvo[k] = alvo.get(k, 0) + v


def aplicar(st: Stats, ef: dict) -> None:
    idx = ef.get("a", 0)
    if idx == "todos":
        alvos = st.ataques
    else:
        alvos = [st.ataques[idx]] if idx < len(st.ataques) else []

    for k, v in ef.items():
        if k == "a":
            continue
        if k in _NIVEL_TORRE:
            if k == "alcance":
                st.alcance += v
            elif k == "alcance_x":
                st.alcance *= v
            elif k == "ouro":
                st.ouro += v
            elif k == "desconto":
                st.desconto = max(st.desconto, v)
            elif k == "hab":
                st.habs.append(dict(v))
            else:
                setattr(st, k, v)
            continue
        if k == "novo":
            st.ataques.append(novo_ataque(v))
            continue
        if k == "subst":
            st.ataques[0] = novo_ataque(v)
            continue
        for at in alvos:
            if k == "buffs":
                _mesclar_buffs(at["buffs"], v)
            elif k == "valor_x":
                at["valor"] *= v
            elif k in _SOMA:
                at[k] = at[k] + v
            elif k in _MULT:
                at[k] = at[k] * v
            else:
                at[k] = v


def definicao(chave: str) -> DefTorre:
    if chave in TORRES:
        return TORRES[chave]
    if chave in HEROIS:
        return HEROIS[chave]
    return AUXILIARES[chave]


def calcular(chave: str, caminhos=(0, 0, 0), nivel: int = 0) -> Stats:
    dfn = definicao(chave)
    st = Stats(dfn)
    for p, tier in enumerate(caminhos):
        if dfn.caminhos:
            for i in range(tier):
                aplicar(st, dfn.caminhos[p][i].ef)
    if dfn.heroi:
        for n in range(2, nivel + 1):
            ef = dfn.niveis.get(n)
            if ef:
                aplicar(st, ef)
        if nivel >= 3 and dfn.hab3:
            st.habs.append(dict(dfn.hab3))
        if nivel >= 10 and dfn.hab10:
            st.habs.append(dict(dfn.hab10))
    return st


def pode_upar(caminhos, p: int) -> bool:
    novo = list(caminhos)
    novo[p] += 1
    if novo[p] > 5:
        return False
    if sum(1 for x in novo if x > 0) > 2:
        return False
    if sum(1 for x in novo if x > 2) > 1:
        return False
    return True


def arredondar_preco(valor: float) -> int:
    return max(5, int(round(valor / 5.0)) * 5)
