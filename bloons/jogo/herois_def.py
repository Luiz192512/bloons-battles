"""Herois: sobem de nivel (1 a 20) com XP e liberam habilidades nos niveis 3 e 10."""

from __future__ import annotations

from dataclasses import dataclass, field

from bloons.jogo.bloons_def import AFIADO, ENERGIA, EXPLOSAO, GELO, NORMAL
from bloons.jogo.torres_def import A, DefTorre, H

# XP acumulado necessario para chegar a cada nivel (indice = nivel)
XP_NIVEL = [0, 0] + [int(180 * (n - 1) ** 1.9) for n in range(2, 21)]


@dataclass
class DefHeroi(DefTorre):
    niveis: dict = field(default_factory=dict)   # nivel -> efeitos
    hab3: dict | None = None
    hab10: dict | None = None
    titulo: str = ""


def _padrao(extra: dict) -> dict:
    """Progressao comum a todos + efeitos especificos por nivel."""
    base = {
        2: dict(alcance=8), 4: dict(pierce=1), 5: dict(cad=0.9), 6: dict(dano=1),
        7: dict(alcance=10), 8: dict(pierce=2), 9: dict(cad=0.9), 11: dict(dano=1),
        12: dict(pierce=2), 13: dict(alcance=10), 14: dict(cad=0.85), 15: dict(dano=2),
        16: dict(pierce=3), 17: dict(cad=0.85), 18: dict(dano=2), 19: dict(pierce=4),
        20: dict(dano=4, cad=0.8),
    }
    for nivel, ef in extra.items():
        base[nivel] = {**base.get(nivel, {}), **ef}
    return base


HEROIS: dict[str, DefHeroi] = {}


def _reg(h: DefHeroi) -> None:
    HEROIS[h.chave] = h


_reg(DefHeroi(
    "quincy", "Quincy", 540, "u", 160,
    [A("projetil", cad=0.95, dano=1, pierce=3, vel=900, dist=260, visual="flecha")],
    titulo="Arqueiro Orgulhoso", cor=(150, 95, 45), heroi=True,
    niveis=_padrao({3: dict(quica=1), 7: dict(n=1, spread=10), 12: dict(camo=True),
                    16: dict(moab=3)}),
    hab3=H("Tiro Rápido", "turbo", 45, dur=7, valor=0.33),
    hab10=H("Tempestade de Flechas", "dano_global", 60, valor=12)))

_reg(DefHeroi(
    "gwendolin", "Gwendolin", 725, "u", 150,
    [A("projetil", cad=0.9, dano=1, pierce=2, vel=800, dist=240, dtype=ENERGIA,
       queima=(1, 2), visual="fogo")],
    titulo="Cientista Piromaníaca", cor=(200, 80, 40), heroi=True,
    niveis=_padrao({6: dict(dtype=NORMAL), 11: dict(queima=(3, 3)),
                    16: dict(buffs=dict(dano=1))}),
    hab3=H("Coquetel de Fogo", "spikes_local", 20, valor=30, dano=1, dur=8),
    hab10=H("Tempestade de Fogo", "dano_global", 60, valor=40, queima=(5, 6))))

_reg(DefHeroi(
    "striker", "Striker Jones", 750, "u", 170,
    [A("projetil", cad=1.3, dano=1, pierce=1, vel=650, dist=280, visual="bomba",
       splash=35, sdano=1, spierce=10, sdtype=EXPLOSAO, raio_proj=7)],
    titulo="Comandante de Artilharia", cor=(80, 100, 60), heroi=True,
    niveis=_padrao({4: dict(sdtype=NORMAL), 8: dict(splash=10, sdano=1),
                    14: dict(sdano=2), 18: dict(sdano=4)}),
    hab3=H("Projétil de Concussão", "dano_forte", 20, valor=40, n=1, atordoa=4),
    hab10=H("Comando de Artilharia", "turbo_area", 60, dur=10, valor=0.5,
            filtro="bomba,morteiro", global_=True)))

_reg(DefHeroi(
    "obyn", "Obyn Guardião", 650, "u", 160,
    [A("projetil", cad=1.35, dano=2, pierce=4, vel=700, dist=260, busca=True,
       dtype=ENERGIA, visual="espirito")],
    titulo="Guardião da Floresta", cor=(40, 120, 90), heroi=True,
    niveis=_padrao({2: dict(buffs=dict(pierce=1)), 11: dict(buffs=dict(pierce=2, dano=1))}),
    hab3=H("Espinheiros", "spikes_local", 25, valor=60, dano=1, dur=12),
    hab10=H("Muralha de Árvores", "spikes_local", 50, valor=2000, dano=1, dur=15)))

_reg(DefHeroi(
    "churchill", "Capitão Churchill", 2000, "u", 170,
    [A("projetil", cad=0.6, dano=3, pierce=1, vel=900, dist=300, dtype=NORMAL,
       splash=25, sdano=2, spierce=6, sdtype=NORMAL, visual="bala_canhao", raio_proj=8)],
    titulo="Tanque Blindado", cor=(70, 90, 60), heroi=True,
    niveis=_padrao({3: dict(moab=3), 8: dict(camo=True), 14: dict(moab=10),
                    20: dict(moab=30)}),
    hab3=H("Projéteis Perfurantes", "turbo", 40, dur=8, valor=0.4),
    hab10=H("Barragem M.O.A.B.", "dano_forte", 60, valor=500, n=5, moab_so=True)))

_reg(DefHeroi(
    "benjamin", "Benjamin", 1200, "u", 100,
    [A("renda", valor=60, visual="moeda")],
    titulo="Hacker", cor=(60, 60, 80), heroi=True,
    niveis={n: dict(valor=20 + 10 * n) for n in range(2, 21)},
    hab3=H("Sifão de Fundos", "dinheiro", 30, valor=250),
    hab10=H("Invasão Bancária", "dinheiro", 60, valor=1500)))

_reg(DefHeroi(
    "ezili", "Ezili", 600, "u", 150,
    [A("projetil", cad=1.0, dano=1, pierce=2, vel=700, dist=240, dtype=NORMAL,
       queima=(1, 3), visual="maldicao")],
    titulo="Sacerdotisa Vodu", cor=(110, 40, 90), heroi=True,
    niveis=_padrao({4: dict(retira_regen=True), 12: dict(queima=(5, 4), moab=5)}),
    hab3=H("Para-Coração", "lentidao", 30, dur=8, valor=0.7),
    hab10=H("Maldição M.O.A.B.", "dano_forte", 50, valor=1500, n=3, moab_so=True)))

_reg(DefHeroi(
    "pat", "Pat Fusty", 800, "u", 80,
    [A("aura", cad=1.5, dano=2, pierce=10, dtype=NORMAL, visual="impacto")],
    titulo="Macaco Gigante", cor=(150, 110, 70), heroi=True,
    niveis=_padrao({7: dict(atordoa=0.5), 13: dict(moab=10)}),
    hab3=H("Rugido de Incentivo", "turbo_area", 45, dur=10, valor=0.7),
    hab10=H("Grande Aperto", "dano_forte", 60, valor=5000, n=1, moab_so=True)))

_reg(DefHeroi(
    "adora", "Adora", 1000, "u", 170,
    [A("projetil", cad=0.8, dano=2, pierce=4, vel=900, dist=280, busca=True,
       dtype=ENERGIA, visual="luz")],
    titulo="Sacerdotisa do Sol", cor=(230, 200, 90), heroi=True,
    niveis=_padrao({5: dict(dtype=NORMAL), 10: dict(n=2, spread=20), 16: dict(moab=8)}),
    hab3=H("Braço Longo da Luz", "turbo", 40, dur=10, valor=0.4),
    hab10=H("Bola de Luz", "invocar", 60, dur=15, base="fenix")))

_reg(DefHeroi(
    "brickell", "Almirante Brickell", 900, "u", 180,
    [A("projetil", cad=0.4, dano=1, pierce=3, vel=900, dist=260, visual="bala")],
    titulo="Comandante Naval", cor=(40, 70, 130), heroi=True, agua=True,
    niveis=_padrao({6: dict(dtype=NORMAL), 13: dict(buffs=dict(cad=0.85))}),
    hab3=H("Táticas Navais", "turbo_area", 45, dur=10, valor=0.5, global_=True),
    hab10=H("Mega Mina", "spikes_local", 60, valor=40, dano=1500, dur=30)))

_reg(DefHeroi(
    "etienne", "Etienne", 850, "u", 9999,
    [A("projetil", cad=0.5, dano=1, pierce=3, vel=800, dist=900, busca=True,
       global_=True, visual="drone")],
    titulo="Especialista em Drones", cor=(80, 110, 150), heroi=True, camo=True,
    niveis=_padrao({8: dict(n=1), 15: dict(n=1, moab=6)}),
    hab3=H("Enxame de Drones", "turbo", 45, dur=15, valor=0.3),
    hab10=H("UCAV", "invocar", 60, dur=20, base="fenix")))

_reg(DefHeroi(
    "sauda", "Sauda", 600, "u", 70,
    [A("aura", cad=0.6, dano=2, pierce=8, dtype=NORMAL, cer=2, visual="espadas")],
    titulo="Espadachim", cor=(200, 120, 60), heroi=True,
    niveis=_padrao({9: dict(camo=True), 14: dict(cer=10, moab=10)}),
    hab3=H("Espada Saltitante", "dano_forte", 20, valor=100, n=8),
    hab10=H("Investida da Espada", "dano_global", 45, valor=60)))

_reg(DefHeroi(
    "psi", "Psi", 1200, "u", 9999,
    [A("hitscan", cad=1.4, dano=3, pierce=1, dtype=NORMAL, global_=True, visual="psi")],
    titulo="Macaco Psíquico", cor=(160, 90, 200), heroi=True, camo=True,
    niveis=_padrao({6: dict(moab=5), 14: dict(moab=20)}),
    hab3=H("Explosão Psíquica", "dano_forte", 25, valor=300, n=3),
    hab10=H("Grito Psiônico", "dano_global", 60, valor=150, atordoa=4)))

_reg(DefHeroi(
    "geraldo", "Geraldo", 725, "u", 150,
    [A("projetil", cad=0.8, dano=1, pierce=2, vel=900, dist=260, visual="bala")],
    titulo="Comerciante Místico", cor=(120, 70, 40), heroi=True,
    niveis=_padrao({5: dict(dtype=NORMAL), 11: dict(camo=True)}),
    hab3=H("Torreta Atiradora", "invocar", 40, dur=25, base="sentinela"),
    hab10=H("Armadilha de Lâminas", "spikes_local", 50, valor=400, dano=4, dur=20)))

_reg(DefHeroi(
    "corvus", "Corvus", 1150, "u", 170,
    [A("projetil", cad=0.6, dano=2, pierce=4, vel=800, dist=280, busca=True,
       dtype=ENERGIA, visual="espirito")],
    titulo="Guardião das Almas", cor=(40, 40, 80), heroi=True,
    niveis=_padrao({4: dict(camo=True), 12: dict(moab=10)}),
    hab3=H("Lança Espiritual", "dano_forte", 25, valor=200, n=2),
    hab10=H("Colheita de Almas", "dano_global", 60, valor=80)))

_reg(DefHeroi(
    "rosalia", "Rosalia", 800, "u", 170,
    [A("projetil", cad=0.8, dano=2, pierce=3, vel=1000, dist=300, dtype=ENERGIA,
       visual="laser")],
    titulo="Engenheira com Jetpack", cor=(200, 90, 120), heroi=True,
    niveis=_padrao({6: dict(dtype=NORMAL), 13: dict(splash=25, sdano=2, spierce=8)}),
    hab3=H("Propulsores", "turbo", 40, dur=10, valor=0.4),
    hab10=H("Tempestade de Foguetes", "dano_global", 60, valor=50)))

_reg(DefHeroi(
    "jericho", "Jericho", 750, "u", 160,
    [A("projetil", cad=0.8, dano=1, pierce=2, vel=950, dist=260, visual="bala")],
    titulo="Bandoleiro (exclusivo do Battles)", cor=(110, 80, 50), heroi=True,
    niveis=_padrao({6: dict(dtype=NORMAL), 12: dict(camo=True)}),
    hab3=H("Proteger a Missão", "turbo", 30, dur=8, valor=0.5),
    hab10=H("Salteador", "roubo", 60, valor=600)))

_reg(DefHeroi(
    "silas", "Silas", 700, "u", 140,
    [A("projetil", cad=0.9, dano=1, pierce=3, vel=800, dist=240, dtype=GELO,
       lento=(0.6, 1.5), visual="gelo_bola")],
    titulo="Mago do Gelo", cor=(120, 190, 230), heroi=True,
    niveis=_padrao({5: dict(dtype=NORMAL), 12: dict(congela=0.8)}),
    hab3=H("Raio Congelante", "dano_forte", 25, valor=60, n=4, congela=3),
    hab10=H("Tempestade Glacial", "congelar_global", 60, dur=6, moab=True)))

ORDEM_HEROIS = list(HEROIS)
