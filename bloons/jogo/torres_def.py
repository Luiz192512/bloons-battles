"""As 22 torres do Bloons TD Battles 2, cada uma com 3 caminhos de 5 upgrades.

Valores inspirados no jogo original (dificuldade Media), adaptados para esta simulacao.
Alcances do original multiplicados por ~4 (px de um mapa 1040x720).

Cada upgrade tem um dicionario de efeitos aplicado por bloons.jogo.stats.aplicar():
  aditivos:        dano, pierce, n, splash, sdano, spierce, quica, moab, cer, fort,
                   dist, raio_proj, valor, pilha_pierce, saltos, empurra, alcance
  multiplicativos: cad, vel, pilha_vida, alcance_x, valor_x, impreciso
  definicao:       dtype, sdtype, camo, busca, global_, visual, lento, congela, cola,
                   queima, atordoa, fragiliza, retira_camo, retira_regen, ouro, spread,
                   frag, fusivel, boom
  estruturais:     a (indice do ataque alvo, ou "todos"), novo (ataque novo),
                   subst (substitui o ataque), hab (habilidade), buffs (para torres buff)
"""

from __future__ import annotations

from dataclasses import dataclass, field

from bloons.jogo.bloons_def import AFIADO, ENERGIA, EXPLOSAO, GELO, NORMAL


@dataclass
class Upgrade:
    nome: str
    custo: int
    desc: str
    ef: dict


@dataclass
class DefTorre:
    chave: str
    nome: str
    custo: int
    tecla: str
    alcance: float
    ataques: list
    caminhos: list = field(default_factory=list)
    categoria: str = "primaria"
    agua: bool = False
    mov: str = "fixo"          # fixo | orbita | heli
    raio: float = 20
    camo: bool = False
    desc: str = ""
    cor: tuple = (140, 90, 40)
    heroi: bool = False


def U(nome: str, custo: int, desc: str = "", **ef) -> Upgrade:
    return Upgrade(nome, custo, desc, ef)


def A(tipo: str, **kw) -> dict:
    kw["tipo"] = tipo
    return kw


def H(nome: str, tipo: str, recarga: float, **kw) -> dict:
    return dict(nome=nome, tipo=tipo, recarga=recarga, **kw)


TORRES: dict[str, DefTorre] = {}


def _reg(t: DefTorre) -> None:
    TORRES[t.chave] = t


# ---------------------------------------------------------------- PRIMARIAS
_reg(DefTorre(
    "dardo", "Macaco Dardo", 200, "q", 128,
    [A("projetil", cad=0.95, dano=1, pierce=2, vel=900, dist=220, visual="dardo")],
    [[U("Tiros Afiados", 140, "Dardos estouram +1 bloon.", pierce=1),
      U("Tiros Super Afiados", 220, "Dardos estouram +2 bloons.", pierce=2),
      U("Espinhopulta", 300, "Arremessa bolas de espinhos com grande perfuração.",
        pierce=18, cad=1.3, raio_proj=6, vel=0.7, visual="bola_espinho"),
      U("Juggernaut", 1800, "Bola gigante que estoura chumbo e cerâmica.",
        dano=1, pierce=50, dtype=NORMAL, cer=4, raio_proj=6, visual="juggernaut"),
      U("Ultra-Juggernaut", 15000, "Se parte em 6 mini juggernauts.",
        dano=3, pierce=100, cer=8, raio_proj=4,
        frag=dict(n=6, dano=2, pierce=30, dtype=NORMAL, visual="bola_espinho"))],
     [U("Tiros Rápidos", 100, "Atira mais rápido.", cad=0.85),
      U("Tiros Muito Rápidos", 190, "Atira ainda mais rápido.", cad=0.78),
      U("Tiro Triplo", 400, "Atira 3 dardos por vez.", n=2, spread=30),
      U("Fã-Clube Super Macaco", 8000, "Habilidade: dardos próximos viram Super Macacos.",
        hab=H("Fã-Clube Super Macaco", "turbo_area", 50, dur=15, valor=0.08, filtro="dardo")),
      U("Fã-Clube Macaco Plasma", 45000, "Habilidade: vira Macacos Plasma.",
        dano=2, pierce=3, dtype=NORMAL,
        hab=H("Fã-Clube Macaco Plasma", "turbo_area", 45, dur=15, valor=0.04, filtro="dardo"))],
     [U("Dardos de Longo Alcance", 90, "Mais alcance.", alcance=32, dist=60),
      U("Visão Aprimorada", 200, "Mais alcance e detecta camo.", alcance=16, camo=True),
      U("Besta", 575, "Flechas mais fortes.", alcance=16, dano=2, pierce=1, visual="flecha"),
      U("Atirador Afiado", 2000, "Tiros críticos.", cad=0.6, dano=3),
      U("Mestre da Besta", 25000, "Rajadas de flechas que ricocheteiam.",
        cad=0.3, dano=5, pierce=4, quica=2, alcance=40)]],
    desc="Atira dardos. Barato e versátil.", cor=(150, 95, 45)))

_reg(DefTorre(
    "bumerangue", "Macaco Bumerangue", 325, "w", 172,
    [A("projetil", cad=1.2, dano=1, pierce=4, vel=520, dist=240, boom=True, raio_proj=8,
       visual="bumerangue")],
    [[U("Bumerangues Melhorados", 200, pierce=4),
      U("Glaives", 280, "Lâminas afiadas.", pierce=6, visual="glaive"),
      U("Ricochete de Glaive", 1300, "Glaives ricocheteiam entre bloons.", pierce=30, quica=6),
      U("M.O.A.R. Glaives", 3000, "Glaives que destroem M.O.A.B.s.", pierce=40, dano=1, moab=3),
      U("Senhor das Glaives", 32500, "Glaives orbitam o macaco.", dano=5, moab=10,
        novo=A("aura", cad=0.1, dano=4, pierce=100, dtype=NORMAL, raio_aura=90,
               visual="orbita_glaive"))],
     [U("Arremesso Rápido", 175, cad=0.75),
      U("Bumerangues Velozes", 250, vel=1.3, cad=0.9),
      U("Bumerangue Biônico", 1600, "Braço biônico.", cad=0.4, moab=2),
      U("Turbo Carga", 4000, "Habilidade: velocidade extrema.", cad=0.8,
        hab=H("Turbo Carga", "turbo", 45, dur=10, valor=0.25)),
      U("Carga Permanente", 35000, "Turbo permanente.", cad=0.35, dano=5)],
     [U("Bumerangues de Longo Alcance", 100, alcance=24, dist=40),
      U("Bumerangues Incandescentes", 300, "Estouram chumbo.", dano=1, dtype=NORMAL),
      U("Bumerangue Kylie", 1300, "Segue a trilha.", pierce=20, dist=220, visual="kylie"),
      U("Prensa de M.O.A.B.", 2200, "Empurra dirigíveis.", moab=4, empurra=40),
      U("Dominação M.O.A.B.", 50000, "Esmaga dirigíveis.", dano=20, moab=30, empurra=80,
        pierce=30)]],
    desc="Bumerangues vão e voltam.", cor=(170, 110, 50)))

_reg(DefTorre(
    "bomba", "Canhão Bomba", 525, "e", 160,
    [A("projetil", cad=1.5, dano=1, pierce=1, vel=600, dist=260, visual="bomba", raio_proj=7,
       splash=45, sdano=1, spierce=14, sdtype=EXPLOSAO)],
    [[U("Bombas Maiores", 350, splash=12, spierce=10),
      U("Bombas Pesadas", 650, sdano=1, spierce=8),
      U("Bombas Muito Grandes", 1100, splash=20, spierce=20, sdano=1),
      U("Impacto Bloon", 3600, "Atordoa bloons.", atordoa=1.0, sdano=1),
      U("Esmaga Bloon", 55000, "Explosão devastadora.", sdano=8, splash=30, spierce=200,
        atordoa=2.0, cer=10, moab=10)],
     [U("Recarga Rápida", 250, cad=0.75),
      U("Lança-Mísseis", 400, "Mísseis rápidos.", cad=0.85, vel=1.6, alcance=16,
        visual="missil"),
      U("Destruidor de M.O.A.B.", 1100, moab=15),
      U("Assassino de M.O.A.B.", 3200, "Habilidade: míssil anti dirigível.", moab=15,
        hab=H("Míssil Assassino", "dano_forte", 30, valor=750, n=1, moab_so=True)),
      U("Eliminador de M.O.A.B.", 25000, moab=100,
        hab=H("Míssil Eliminador", "dano_forte", 10, valor=4500, n=1, moab_so=True))],
     [U("Alcance Extra", 200, alcance=28, dist=40),
      U("Bombas de Fragmentação", 300, "Soltam fragmentos.",
        frag=dict(n=8, dano=1, pierce=1, dtype=AFIADO, visual="fragmento")),
      U("Bombas de Cacho", 800, "Soltam mini bombas.",
        frag=dict(n=8, dano=1, pierce=1, splash=30, sdano=1, spierce=8, visual="bomba")),
      U("Cacho Recursivo", 2800, sdano=1,
        frag=dict(n=10, dano=1, pierce=1, splash=34, sdano=2, spierce=10, visual="bomba")),
      U("Blitz de Bombas", 23000, "Habilidade: bombardeio geral.", sdano=2,
        hab=H("Blitz de Bombas", "dano_global", 60, valor=1000))]],
    desc="Bombas explodem em área.", cor=(60, 60, 70)))

_reg(DefTorre(
    "tachinha", "Atirador de Tachinhas", 280, "r", 92,
    [A("radial", cad=1.4, dano=1, pierce=1, n=8, vel=520, dist=120, visual="tachinha")],
    [[U("Disparo Rápido", 150, cad=0.75),
      U("Disparo Mais Rápido", 300, cad=0.66),
      U("Tiros Quentes", 600, "Tachinhas de fogo estouram chumbo.", dtype=NORMAL, dano=1,
        visual="fogo"),
      U("Anel de Fogo", 3500, "Anel de chamas em volta.",
        subst=A("aura", cad=0.3, dano=1, pierce=60, dtype=ENERGIA, raio_aura=110,
                visual="anel_fogo")),
      U("Anel Infernal", 45500, "Meteoros e anel mais forte.", dano=3, pierce=80,
        novo=A("projetil", cad=4.0, dano=700, pierce=1, vel=1500, dist=900, dtype=NORMAL,
               splash=60, sdano=30, spierce=20, global_=True, visual="meteoro",
               raio_proj=16, alvo="forte"))],
     [U("Tachinhas de Longo Alcance", 100, alcance=16, dist=30),
      U("Tachinhas de Super Alcance", 225, alcance=16, dist=30),
      U("Atirador de Lâminas", 550, pierce=3, dano=1, visual="lamina"),
      U("Turbilhão de Lâminas", 2700, "Habilidade: redemoinho de lâminas.",
        hab=H("Turbilhão", "turbo", 20, dur=3, valor=0.05)),
      U("Super Turbilhão", 15000, pierce=10, dano=2,
        hab=H("Super Turbilhão", "turbo", 20, dur=9, valor=0.04))],
     [U("Mais Tachinhas", 100, n=2),
      U("Ainda Mais Tachinhas", 300, n=2),
      U("Pulverizador de Tachinhas", 600, n=4, cad=0.6),
      U("Sobrecarga", 3200, cad=0.33),
      U("Zona das Tachinhas", 24000, n=20, pierce=4, cad=0.5, dano=1, alcance=30)]],
    desc="Dispara tachinhas em 8 direções.", cor=(170, 170, 180)))

_reg(DefTorre(
    "gelo", "Macaco de Gelo", 500, "t", 80,
    [A("aura", cad=2.4, dano=1, pierce=40, dtype=GELO, congela=1.5, visual="congelar")],
    [[U("Permafrost", 100, "Bloons ficam lentos depois.", lento=(0.5, 2.5)),
      U("Estalo Frio", 350, "Congela camo e chumbo.", camo=True, dtype=NORMAL),
      U("Estilhaços de Gelo", 1500, "Bloons congelados soltam estilhaços.",
        frag=dict(n=3, dano=1, pierce=2, dtype=AFIADO, visual="fragmento_gelo")),
      U("Fragilização", 2200, "Bloons recebem dano extra.", fragiliza=1),
      U("Super Frágil", 28000, fragiliza=4, dano=2, moab=4)],
     [U("Congelamento Melhor", 225, cad=0.8, congela=2.0),
      U("Congelamento Profundo", 350, congela=2.5, dano=1),
      U("Vento Ártico", 2900, "Aura que desacelera tudo.", alcance=30,
        novo=A("aura", cad=0.2, dano=0, pierce=999, lento=(0.4, 0.3), moab_lento=True,
               visual="vento")),
      U("Nevasca", 3000, "Habilidade: congela todos os bloons.",
        hab=H("Nevasca", "congelar_global", 30, dur=3)),
      U("Zero Absoluto", 26000,
        hab=H("Zero Absoluto", "congelar_global", 20, dur=10, moab=True))],
     [U("Raio Maior", 175, alcance=16),
      U("Recongelar", 225, pierce=20),
      U("Canhão Criogênico", 2000, "Dispara bolas de gelo.", alcance=60,
        subst=A("projetil", cad=1.2, dano=1, pierce=1, vel=700, dist=300, dtype=GELO,
                congela=1.5, splash=34, sdano=1, spierce=20, sdtype=GELO,
                visual="gelo_bola", raio_proj=8)),
      U("Pingentes", 2000, sdano=1, moab=2, spierce=10),
      U("Empalar com Pingentes", 30000, sdano=30, moab=30, congela=5, moab_congela=True)]],
    desc="Congela bloons em volta.", cor=(150, 210, 240)))

_reg(DefTorre(
    "cola", "Atirador de Cola", 225, "y", 184,
    [A("projetil", cad=1.0, dano=0, pierce=1, vel=700, dist=260, cola=(0.5, 11, 0),
       visual="cola")],
    [[U("Cola Encharcada", 200, "Cola passa pelas camadas."),
      U("Cola Corrosiva", 300, "Cola corrói bloons.", cola=(0.5, 11, 0.5)),
      U("Dissolvedor de Bloons", 2500, cola=(0.5, 11, 2)),
      U("Liquefator de Bloons", 5000, cola=(0.45, 11, 10)),
      U("Solucionador de Bloons", 22000, cola=(0.4, 11, 50), splash=40, spierce=6)],
     [U("Globos Maiores", 100, pierce=1),
      U("Respingo de Cola", 1600, splash=40, spierce=6),
      U("Mangueira de Cola", 3250, cad=0.3),
      U("Ataque de Cola", 3500, "Habilidade: cola todos os bloons.",
        hab=H("Ataque de Cola", "lentidao", 40, dur=11, valor=0.5)),
      U("Tempestade de Cola", 15000,
        hab=H("Tempestade de Cola", "lentidao", 30, dur=15, valor=0.3, dano=5))],
     [U("Cola Mais Grudenta", 120, cola=(0.5, 22, 0)),
      U("Cola Mais Forte", 400, cola=(0.35, 22, 0)),
      U("Cola de M.O.A.B.", 3400, "Cola dirigíveis.", moab_cola=True),
      U("Cola Implacável", 3000, pierce=3, splash=30, spierce=4),
      U("Super Cola", 35000, "Paralisa dirigíveis.", atordoa=2.0, moab_atordoa=True,
        cola=(0.2, 30, 5))]],
    desc="Cola desacelera bloons.", cor=(150, 200, 60)))

_reg(DefTorre(
    "sniper", "Macaco Atirador", 350, "z", 9999,
    [A("hitscan", cad=1.59, dano=2, pierce=1, dtype=AFIADO, global_=True, visual="bala")],
    [[U("Jaqueta Metálica", 350, "Estoura chumbo.", dtype=NORMAL, dano=2),
      U("Calibre Grosso", 1300, dano=3),
      U("Precisão Mortal", 3000, dano=11, cer=15),
      U("Mutilar M.O.A.B.", 5000, "Atordoa dirigíveis.", atordoa=3.0, moab_atordoa=True,
        dano=12),
      U("Aleijar M.O.A.B.", 34000, atordoa=7.0, fragiliza=5, dano=20)],
     [U("Óculos de Visão Noturna", 300, "Detecta camo.", camo=True),
      U("Tiro de Estilhaços", 450,
        frag=dict(n=5, dano=1, pierce=1, dtype=AFIADO, visual="fragmento")),
      U("Bala Ricochete", 3200, quica=3),
      U("Lançamento de Suprimentos", 7200, "Habilidade: caixa de dinheiro.",
        hab=H("Suprimentos", "dinheiro", 60, valor=1000)),
      U("Atirador de Elite", 13000, cad=0.5,
        hab=H("Suprimentos de Elite", "dinheiro", 50, valor=2000))],
     [U("Disparo Rápido", 400, cad=0.7),
      U("Disparo Mais Rápido", 400, cad=0.7),
      U("Semiautomático", 3500, cad=0.33),
      U("Rifle Automático", 4750, cad=0.5, dano=2, moab=3),
      U("Defensor de Elite", 14000, cad=0.5, dano=2)]],
    categoria="militar", desc="Alcance infinito.", cor=(80, 110, 60)))

_reg(DefTorre(
    "submarino", "Submarino Macaco", 325, "x", 168,
    [A("projetil", cad=0.75, dano=1, pierce=2, vel=800, dist=260, busca=True, visual="dardo")],
    [[U("Alcance Maior", 130, alcance=40),
      U("Inteligência Avançada", 500, "Ataca qualquer bloon no mapa.", global_=True),
      U("Submergir e Apoiar", 500, "Revela bloons camo em volta.", camo=True,
        novo=A("aura", cad=0.5, dano=0, pierce=999, retira_camo=True, visual="nenhum")),
      U("Reator de Bloontônio", 2500, "Radiação estoura tudo em volta.",
        novo=A("aura", cad=0.3, dano=1, pierce=100, dtype=NORMAL, visual="radiacao")),
      U("Energizador", 32000, "Radiação intensa.", a=1, dano=3, pierce=300, cad=0.6)],
     [U("Dardos Farpados", 450, pierce=3),
      U("Dardos Aquecidos", 300, dtype=NORMAL, dano=1),
      U("Míssil Balístico", 1300, "Mísseis de longo alcance.",
        novo=A("projetil", cad=1.5, dano=3, pierce=1, vel=900, dist=2000, busca=True,
               global_=True, moab=5, splash=40, sdano=1, spierce=10, visual="missil")),
      U("Capacidade de Primeiro Ataque", 13000, "Habilidade: míssil nuclear.",
        hab=H("Primeiro Ataque", "dano_forte", 60, valor=10000, n=1, splash=120,
              sdano=700)),
      U("Ataque Preventivo", 32000, "Míssil em cada dirigível.",
        novo=A("projetil", cad=5.0, dano=1000, pierce=1, vel=1500, dist=3000, busca=True,
               global_=True, dtype=NORMAL, alvo="forte", so_moab=True, visual="missil",
               raio_proj=10))],
     [U("Canhões Gêmeos", 450, cad=0.5),
      U("Dardos de Explosão Aérea", 1000,
        frag=dict(n=3, dano=1, pierce=1, dtype=AFIADO, visual="dardo")),
      U("Canhões Triplos", 1100, cad=0.66),
      U("Dardos Perfurantes", 3000, dano=2, moab=3, cer=2),
      U("Comandante Submarino", 25000, dano=8, pierce=6, cad=0.5)]],
    categoria="militar", agua=True, desc="Só na água.", cor=(230, 200, 40)))

_reg(DefTorre(
    "bucaneiro", "Macaco Bucaneiro", 500, "c", 240,
    [A("projetil", cad=1.0, dano=1, pierce=4, n=2, spread=360, vel=700, dist=300,
       visual="dardo")],
    [[U("Disparo Rápido", 275, cad=0.75),
      U("Tiro Duplo", 450, n=2),
      U("Destróier", 2950, cad=0.2),
      U("Porta-Aviões", 6000, "Aviões atacam em todo o mapa.",
        novo=A("radial", cad=0.6, dano=2, pierce=5, n=4, vel=700, dist=300,
               visual="aviaozinho", global_=True)),
      U("Nau Capitânia", 40000, a="todos", dano=3, pierce=5,
        buffs=dict(cad=0.85, alcance_pct=0.1))],
     [U("Tiro de Uva", 550, "Dispara uvas.",
        novo=A("projetil", cad=1.0, dano=1, pierce=1, n=5, spread=40, vel=700, dist=260,
               visual="uva")),
      U("Tiro Quente", 500, a=1, dtype=NORMAL, dano=1, visual="uva_fogo"),
      U("Navio Canhão", 900, "Balas de canhão explosivas.",
        novo=A("projetil", cad=1.3, dano=2, pierce=1, vel=650, dist=320, splash=40,
               sdano=2, spierce=16, sdtype=EXPLOSAO, visual="bala_canhao", raio_proj=8)),
      U("Macacos Piratas", 4500, "Habilidade: arpão derruba um dirigível.", moab=4,
        hab=H("Arpão", "dano_forte", 60, valor=4000, n=1, moab_so=True)),
      U("Senhor Pirata", 21000, moab=10,
        hab=H("Arpões do Senhor Pirata", "dano_forte", 60, valor=20000, n=3, moab_so=True))],
     [U("Longo Alcance", 180, alcance=40),
      U("Ninho do Corvo", 400, "Detecta camo.", camo=True),
      U("Navio Mercante", 2300, "Gera dinheiro por rodada.",
        novo=A("renda", valor=200, visual="moeda")),
      U("Comércio Favorecido", 5500, a=1, valor=300),
      U("Império Comercial", 23000, a=1, valor=900, buffs=dict(dano=1))]],
    categoria="militar", agua=True, raio=26, desc="Só na água.", cor=(120, 80, 40)))

_reg(DefTorre(
    "as", "Macaco Ás", 800, "v", 200,
    [A("radial", cad=1.68, dano=1, pierce=5, n=8, vel=700, dist=240, visual="dardo")],
    [[U("Tiro Rápido", 650, cad=0.7),
      U("Muito Mais Dardos", 650, n=4),
      U("Avião de Caça", 1000, "Mísseis teleguiados.",
        novo=A("projetil", cad=1.0, dano=3, pierce=1, vel=900, dist=2000, busca=True,
               global_=True, moab=3, splash=25, sdano=1, spierce=5, visual="missil")),
      U("Operação: Tempestade de Dardos", 3000, cad=0.4, n=4),
      U("Retalhador Celeste", 24000, n=8, pierce=5, cad=0.5, dano=2)],
     [U("Abacaxi Explosivo", 200, "Solta abacaxis explosivos.",
        novo=A("queda", cad=3.0, splash=50, sdano=1, spierce=20, fusivel=1.5,
               visual="abacaxi")),
      U("Avião Espião", 350, "Detecta camo.", camo=True),
      U("Ás Bombardeiro", 900, "Bombas na trilha.",
        novo=A("queda", cad=1.5, splash=60, sdano=2, spierce=30, fusivel=0.3,
               visual="bomba", na_trilha=True)),
      U("Marco Zero", 18000, "Habilidade: bomba gigante.",
        hab=H("Marco Zero", "dano_global", 45, valor=700)),
      U("Tsar Bomba", 30000,
        hab=H("Tsar Bomba", "dano_global", 60, valor=3000, atordoa=8))],
     [U("Dardos Mais Afiados", 500, pierce=3),
      U("Rota Centralizada", 300, "Voa em círculo menor."),
      U("Mira Infalível", 2200, "Dardos teleguiados.", busca=True),
      U("Espectro", 24000, "Chuva de dardos e bombas.",
        novo=A("projetil", cad=0.05, dano=2, pierce=3, vel=900, dist=1200, busca=True,
               global_=True, dtype=NORMAL, visual="dardo")),
      U("Fortaleza Voadora", 90000, a="todos", dano=3, n=8, cad=0.5)]],
    categoria="militar", mov="orbita", raio=24, desc="Voa em círculos atirando.",
    cor=(230, 200, 40)))

_reg(DefTorre(
    "heli", "Piloto de Helicóptero", 1600, "b", 170,
    [A("projetil", cad=0.57, dano=1, pierce=3, n=2, spread=10, vel=850, dist=260,
       visual="dardo")],
    [[U("Dardos Quádruplos", 800, n=2, spread=20),
      U("Perseguição", 500, "Persegue os bloons.", persegue=True),
      U("Hélices Navalha", 1750, "Hélices estouram bloons.",
        novo=A("aura", cad=0.5, dano=2, pierce=20, raio_aura=55, visual="nenhum")),
      U("Apache Dardeiro", 19600, "Metralhadoras e mísseis.", cad=0.35, n=2,
        novo=A("projetil", cad=1.0, dano=5, pierce=1, vel=900, dist=600, moab=5,
               splash=35, sdano=2, spierce=10, visual="missil")),
      U("Apache Prime", 45000, a="todos", dtype=NORMAL, dano=4, visual="plasma")],
     [U("Jatos Maiores", 300, alcance=20),
      U("IFR", 600, "Detecta camo.", camo=True),
      U("Corrente Descendente", 2000, "Empurra bloons para trás.",
        novo=A("aura", cad=1.2, dano=0, pierce=6, empurra=90, raio_aura=80,
               visual="vento")),
      U("Chinook de Apoio", 12000, "Habilidade: entrega dinheiro.",
        hab=H("Entrega", "dinheiro", 60, valor=1000)),
      U("Operações Especiais", 35000, "Habilidade: fuzileiro de elite.",
        hab=H("Fuzileiro", "invocar", 60, dur=20, base="sniper", nivel=(4, 0, 3)))],
     [U("Dardos Rápidos", 250, vel=1.3),
      U("Disparo Rápido", 350, cad=0.8),
      U("Empurrão de M.O.A.B.", 3000, "Desacelera dirigíveis.",
        novo=A("aura", cad=0.5, dano=0, pierce=99, lento=(0.5, 0.5), moab_lento=True,
               raio_aura=90, visual="nenhum")),
      U("Defesa Comanche", 8500, "Mini comanches ajudam.", n=4, cad=0.7),
      U("Comandante Comanche", 35000, dano=4, n=6, pierce=4)]],
    categoria="militar", mov="heli", raio=26, desc="Helicóptero que se move.",
    cor=(80, 120, 70)))

_reg(DefTorre(
    "morteiro", "Macaco Morteiro", 750, "n", 9999,
    [A("morteiro", cad=2.0, dano=1, splash=40, sdano=1, spierce=40, sdtype=EXPLOSAO,
       impreciso=40, global_=True, visual="bala_canhao")],
    [[U("Explosão Maior", 500, splash=12, spierce=10),
      U("Destruidor de Bloons", 500, sdano=1),
      U("Choque de Projéteis", 900, "Atordoa bloons.", atordoa=0.5, splash=10),
      U("A Grande", 7000, splash=30, sdano=3, spierce=60),
      U("A Maior de Todas", 35000, splash=60, sdano=20, spierce=200)],
     [U("Recarga Rápida", 300, cad=0.75),
      U("Recarga Veloz", 500, cad=0.75),
      U("Projéteis Pesados", 900, sdano=1, sdtype=NORMAL),
      U("Bateria de Artilharia", 5500, "3 projéteis por vez.", n=2),
      U("Choque e Pavor", 30000, "Habilidade: atordoa tudo.",
        hab=H("Choque e Pavor", "dano_global", 60, valor=50, atordoa=8))],
     [U("Precisão Aumentada", 200, impreciso=0.5),
      U("Coisas Queimando", 500, "Deixa fogo.", queima=(1, 3)),
      U("Sinalizador", 600, "Revela camo.", retira_camo=True, camo=True),
      U("Projéteis Estilhaçantes", 11000, fragiliza=2, cer=5, moab=5),
      U("Bloonflagração", 40000, queima=(20, 3), sdano=5, sdtype=NORMAL)]],
    categoria="militar", desc="Atira no local escolhido.", cor=(90, 110, 70)))

_reg(DefTorre(
    "dartling", "Atirador Dartling", 850, "m", 9999,
    [A("projetil", cad=0.2, dano=1, pierce=1, vel=1200, dist=1400, spread=20,
       global_=True, visual="dardo")],
    [[U("Disparo Focado", 250, spread=6),
      U("Choque Laser", 1200, dtype=ENERGIA, pierce=1, queima=(1, 1), visual="laser"),
      U("Canhão Laser", 3000, dano=1, pierce=3, visual="laser"),
      U("Acelerador de Plasma", 11000, "Raio contínuo.",
        subst=A("hitscan", cad=0.2, dano=3, pierce=100, dtype=NORMAL, global_=True,
                visual="raio_plasma", linha=True)),
      U("Raio da Perdição", 90000, dano=27, pierce=200, cad=0.5, moab=10)],
     [U("Mira Avançada", 300, busca=True),
      U("Giro de Cano Rápido", 950, cad=0.7),
      U("Cápsulas de Foguete Hidra", 5000, splash=25, sdano=1, spierce=6, visual="missil"),
      U("Tempestade de Foguetes", 6000, "Habilidade: chuva de foguetes.",
        hab=H("Tempestade de Foguetes", "turbo", 40, dur=8, valor=0.2)),
      U("M.A.D.", 58000, "Mísseis anti dirigível.", moab=40, sdano=6, splash=20)],
     [U("Giro Mais Rápido", 150, vel=1.2),
      U("Dardos Poderosos", 1200, dano=1, pierce=1, vel=1.5),
      U("Chumbinho", 3200, n=5, spread=30, dano=1),
      U("Sistema de Negação de Área", 6000, n=4, pierce=2),
      U("Zona de Exclusão Bloon", 42000, n=6, dano=3, pierce=3, cad=0.6)]],
    categoria="militar", desc="Metralhadora de dardos.", cor=(70, 90, 110)))

# ---------------------------------------------------------------- MAGICAS
_reg(DefTorre(
    "mago", "Macaco Mago", 375, "a", 160,
    [A("projetil", cad=1.1, dano=1, pierce=2, dtype=ENERGIA, vel=700, dist=240,
       visual="magia")],
    [[U("Magia Guiada", 150, "Magia teleguiada.", busca=True),
      U("Explosão Arcana", 600, dano=1),
      U("Maestria Arcana", 1300, cad=0.5, pierce=3, alcance=20),
      U("Espinho Arcano", 10900, moab=10, dano=2),
      U("Arquimago", 32000, a="todos", dano=5, pierce=5, cad=0.6)],
     [U("Bola de Fogo", 300, "Lança bolas de fogo.",
        novo=A("projetil", cad=2.2, dano=1, pierce=1, vel=600, dist=260, dtype=ENERGIA,
               splash=30, sdano=1, spierce=12, sdtype=ENERGIA, visual="fogo", raio_proj=8)),
      U("Muralha de Fogo", 900, "Chamas na trilha.",
        novo=A("pilha", cad=5.5, dano=1, pilha_pierce=15, pilha_vida=5, dtype=ENERGIA,
               visual="chamas")),
      U("Sopro do Dragão", 3000, "Lança chamas.",
        novo=A("projetil", cad=0.1, dano=1, pierce=5, n=3, spread=25, vel=500, dist=170,
               dtype=ENERGIA, visual="fogo")),
      U("Invocar Fênix", 4000, "Habilidade: fênix de fogo.",
        hab=H("Fênix", "invocar", 45, dur=20, base="fenix")),
      U("Lorde Fênix", 50000, "Fênix permanente.",
        novo=A("projetil", cad=0.1, dano=5, pierce=10, n=2, spread=30, vel=700, dist=600,
               global_=True, dtype=NORMAL, visual="fogo"))],
     [U("Magia Intensa", 300, pierce=2, vel=1.2),
      U("Sentido Macaco", 300, "Detecta camo.", camo=True),
      U("Cintilar", 1700, "Revela camo em volta.",
        novo=A("aura", cad=1.0, dano=0, pierce=999, retira_camo=True, visual="nenhum")),
      U("Necromante", 2000, "Bloons mortos voltam como aliados.",
        novo=A("pilha", cad=2.0, dano=2, pilha_pierce=8, pilha_vida=6, dtype=NORMAL,
               visual="zumbi")),
      U("Príncipe das Trevas", 24000, a="todos", dano=4, pilha_pierce=20)]],
    categoria="magica", desc="Magia poderosa.", cor=(120, 70, 170)))

_reg(DefTorre(
    "super", "Super Macaco", 2500, "s", 200,
    [A("projetil", cad=0.045, dano=1, pierce=1, vel=1100, dist=280, visual="dardo")],
    [[U("Rajadas Laser", 2500, dtype=ENERGIA, pierce=1, visual="laser"),
      U("Rajadas de Plasma", 4500, dtype=NORMAL, dano=1, pierce=1, visual="plasma"),
      U("Avatar do Sol", 20000, n=2, spread=20, dano=2, pierce=2, visual="sol"),
      U("Templo do Sol", 100000, dano=8, pierce=5, moab=10),
      U("Verdadeiro Deus Sol", 500000, dano=20, pierce=10, moab=30)],
     [U("Super Alcance", 1000, alcance=40, dist=60),
      U("Alcance Épico", 1400, alcance=40, dist=60),
      U("Robô Macaco", 7000, n=1, spread=12, dano=1),
      U("Terror Tecnológico", 19000, "Habilidade: aniquilação.",
        hab=H("Aniquilação", "dano_global", 45, valor=2000)),
      U("O Anti-Bloon", 80000, dano=4,
        hab=H("Erradicação", "dano_global", 45, valor=5000))],
     [U("Repulsão", 3000, empurra=20),
      U("Ultravisão", 1200, camo=True, alcance=10),
      U("Cavaleiro das Trevas", 5500, dtype=NORMAL, moab=2, visual="escuro"),
      U("Campeão das Trevas", 60000, dano=4, moab=6, pierce=3),
      U("Lenda da Noite", 240000, "Habilidade: manda bloons de volta.", dano=10,
        hab=H("Noite Eterna", "reverso", 60, valor=600))]],
    categoria="magica", raio=22, desc="Muito rápido e forte.", cor=(40, 90, 200)))

_reg(DefTorre(
    "ninja", "Macaco Ninja", 400, "d", 160,
    [A("projetil", cad=0.7, dano=1, pierce=2, vel=900, dist=240, busca=True,
       visual="shuriken")],
    [[U("Disciplina Ninja", 300, alcance=28, cad=0.8),
      U("Shurikens Afiadas", 350, pierce=2),
      U("Tiro Duplo", 850, n=1, spread=10),
      U("Bloonjitsu", 2750, n=3, spread=30),
      U("Grão-Mestre Ninja", 35000, n=3, dano=2, cad=0.5)],
     [U("Distração", 350, "Empurra bloons.", empurra=25),
      U("Contraespionagem", 500, "Remove camo.", retira_camo=True),
      U("Táticas Shinobi", 900, cad=0.92, buffs=dict(cad=0.92)),
      U("Sabotagem Bloon", 5200, "Habilidade: bloons lentos.",
        hab=H("Sabotagem", "lentidao", 60, dur=15, valor=0.5)),
      U("Grande Sabotador", 22000,
        hab=H("Grande Sabotagem", "lentidao", 60, dur=20, valor=0.5, dano=300))],
     [U("Shuriken Teleguiada", 250, pierce=1),
      U("Estrepes", 400, "Espalha estrepes na trilha.",
        novo=A("pilha", cad=4.0, dano=1, pilha_pierce=6, pilha_vida=10, visual="estrepe")),
      U("Bomba de Luz", 2750, "Atordoa bloons.",
        novo=A("projetil", cad=4.0, dano=1, pierce=1, vel=600, dist=260, splash=55,
               sdano=1, spierce=50, atordoa=1.0, visual="flash")),
      U("Bomba Grudenta", 4500, "Bomba em dirigíveis.",
        novo=A("projetil", cad=3.0, dano=500, pierce=1, vel=900, dist=500, busca=True,
               so_moab=True, alvo="forte", dtype=NORMAL, visual="bomba")),
      U("Mestre Bombardeiro", 40000, a=2, dano=4500, cad=0.6)]],
    categoria="magica", camo=True, desc="Detecta camo.", cor=(170, 30, 30)))

_reg(DefTorre(
    "alquimista", "Alquimista", 550, "f", 180,
    [A("projetil", cad=2.0, dano=1, pierce=1, vel=600, dist=260, dtype=NORMAL, splash=30,
       sdano=1, spierce=15, visual="pocao", raio_proj=7)],
    [[U("Poções Maiores", 250, splash=10, spierce=10),
      U("Mistura Ácida", 350, "Torres próximas causam mais dano em dirigíveis.",
        novo=A("buff", buffs=dict(moab=1))),
      U("Poção do Berserker", 1250, "Torres próximas ficam mais fortes.", a=1,
        buffs=dict(dano=1, cad=0.9, pierce=2, alcance_pct=0.1)),
      U("Estimulante Forte", 3000, a=1, buffs=dict(cad=0.85, dano=1)),
      U("Poção Permanente", 60000, a=1, buffs=dict(dano=2, pierce=5, cad=0.8))],
     [U("Ácido Forte", 250, queima=(1, 4)),
      U("Poções Perecíveis", 475, moab=4, cer=2),
      U("Mistura Instável", 3000, moab=10, splash=10),
      U("Tônico Transformador", 4500, "Habilidade: vira monstro.",
        hab=H("Transformação", "turbo", 60, dur=20, valor=0.2)),
      U("Transformação Total", 45000,
        hab=H("Transformação Total", "turbo_area", 40, dur=20, valor=0.3))],
     [U("Arremesso Rápido", 650, cad=0.75),
      U("Poça de Ácido", 450, "Poças na trilha.",
        novo=A("pilha", cad=3.0, dano=1, pilha_pierce=10, pilha_vida=8, dtype=NORMAL,
               visual="acido")),
      U("Chumbo em Ouro", 1000, "Chumbo estourado dá dinheiro extra.", ouro=0.5),
      U("Borracha em Ouro", 2750, ouro=1.0),
      U("Mestre Alquimista", 40000, "Encolhe bloons.", dano=10, moab=30, pierce=5)]],
    categoria="magica", desc="Poções e ácido.", cor=(110, 60, 130)))

_reg(DefTorre(
    "druida", "Druida", 400, "g", 140,
    [A("projetil", cad=1.1, dano=1, pierce=1, n=5, spread=40, vel=700, dist=200,
       visual="espinho")],
    [[U("Espinhos Duros", 250, pierce=1, dtype=NORMAL),
      U("Coração do Trovão", 1000, "Raios em cadeia.",
        novo=A("cadeia", cad=2.3, dano=1, pierce=1, saltos=6, dtype=ENERGIA,
               visual="relampago")),
      U("Druida da Tempestade", 1650, "Tornado empurra bloons.",
        novo=A("projetil", cad=2.5, dano=0, pierce=30, vel=300, dist=300, empurra=120,
               visual="tornado", raio_proj=18)),
      U("Bola de Relâmpago", 4500, a=1, saltos=8, dano=2, cad=0.6),
      U("Supertempestade", 60000, a="todos", dano=8, pierce=10, moab=10)],
     [U("Enxame de Espinhos", 250, n=3),
      U("Coração de Carvalho", 350, "Remove regeneração.", retira_regen=True),
      U("Druida da Selva", 950, "Cipós prendem bloons.",
        novo=A("pilha", cad=3.0, dano=2, pilha_pierce=8, pilha_vida=6, dtype=NORMAL,
               visual="cipo")),
      U("Recompensa da Selva", 5000, "Gera dinheiro.",
        novo=A("renda", valor=250, visual="moeda")),
      U("Espírito da Floresta", 35000, a=1, dano=6, pilha_pierce=40, cad=0.3)],
     [U("Alcance Druídico", 100, alcance=40),
      U("Coração da Vingança", 300, cad=0.85),
      U("Druida da Ira", 600, cad=0.8, dano=1),
      U("Luxúria de Estouros", 2500, pierce=3, buffs=dict(cad=0.85)),
      U("Avatar da Ira", 45000, dano=6, pierce=6, n=4)]],
    categoria="magica", desc="Poderes da natureza.", cor=(60, 130, 60)))

# ---------------------------------------------------------------- SUPORTE
_reg(DefTorre(
    "fazenda", "Fazenda de Bananas", 1250, "h", 100,
    [A("renda", valor=80, visual="banana")],
    [[U("Produção Aumentada", 500, valor=20),
      U("Produção Maior", 600, valor=40),
      U("Plantação de Bananas", 3000, valor=140),
      U("Centro de Pesquisa de Bananas", 19000, valor=800),
      U("Central de Bananas", 100000, valor=3000)],
     [U("Bananas Duradouras", 300, valor=10),
      U("Bananas Valiosas", 800, valor_x=1.25),
      U("Banco Macaco", 3650, valor=250),
      U("Empréstimo do FMI", 7200, "Habilidade: empréstimo.",
        hab=H("Empréstimo", "dinheiro", 90, valor=5000)),
      U("Macaconomia", 100000,
        hab=H("Macaconomia", "dinheiro", 60, valor=10000))],
     [U("Coleta Fácil", 250, valor=10),
      U("Salvamento de Bananas", 200, "Vende por 90%.", venda=0.9),
      U("Mercado", 2900, valor=200),
      U("Mercado Central", 15000, valor=600),
      U("Wall Street dos Macacos", 60000, valor=4000)]],
    categoria="suporte", raio=26, desc="Gera dinheiro a cada rodada.", cor=(240, 210, 60)))

_reg(DefTorre(
    "espinhos", "Fábrica de Espinhos", 1000, "j", 136,
    [A("pilha", cad=2.2, dano=1, pilha_pierce=5, pilha_vida=40, visual="espinhos")],
    [[U("Pilhas Maiores", 800, pilha_pierce=5),
      U("Espinhos Incandescentes", 600, dtype=NORMAL),
      U("Bolas Espinhosas", 2300, dano=1, cer=3, visual="bola_espinho"),
      U("Minas Espinhosas", 10000, "Explodem ao acabar.", splash=40, sdano=5, spierce=40),
      U("Super Minas", 150000, splash=80, sdano=400, spierce=200)],
     [U("Produção Rápida", 600, cad=0.75),
      U("Produção Mais Rápida", 800, cad=0.75),
      U("Triturador de M.O.A.B.", 2500, moab=2),
      U("Tempestade de Espinhos", 5000, "Habilidade: espinhos na trilha toda.",
        hab=H("Tempestade de Espinhos", "spikes_global", 40, valor=60, dano=2)),
      U("Tapete de Espinhos", 40000, cad=0.6,
        hab=H("Tapete de Espinhos", "spikes_global", 30, valor=150, dano=4))],
     [U("Espinhos Duradouros", 150, pilha_vida=2),
      U("Espinhos Mortais", 400, dano=1),
      U("Longo Alcance", 1400, alcance=40, pilha_pierce=5),
      U("Espinhominador", 12500, dano=3, pilha_pierce=15, pilha_vida=2),
      U("Perma-Espinho", 30000, dano=10, pilha_pierce=40, pilha_vida=4)]],
    categoria="suporte", desc="Espinhos na trilha.", cor=(130, 130, 140)))

_reg(DefTorre(
    "vila", "Vila dos Macacos", 1200, "k", 160,
    [A("buff", buffs=dict(alcance_pct=0.1))],
    [[U("Raio Maior", 400, alcance=40),
      U("Tambores da Selva", 1500, "Torres atacam mais rápido.", buffs=dict(cad=0.85)),
      U("Treinamento Primário", 800, buffs=dict(pierce=1, alcance_pct=0.1)),
      U("Mentoria Primária", 2500, buffs=dict(dano=1)),
      U("Especialização Primária", 25000, "Balista gigante.",
        novo=A("projetil", cad=4.0, dano=1000, pierce=5, vel=1200, dist=2500, busca=True,
               global_=True, dtype=NORMAL, alvo="forte", visual="balista",
               raio_proj=12))],
     [U("Bloqueador de Crescimento", 250, "Remove regeneração.",
        novo=A("aura", cad=0.5, dano=0, pierce=999, retira_regen=True, visual="nenhum")),
      U("Radar", 2000, "Torres próximas detectam camo.", buffs=dict(camo=True)),
      U("Agência de Inteligência Macaco", 7500, "Torres estouram tudo.",
        buffs=dict(dtype_normal=True)),
      U("Chamado às Armas", 20000, "Habilidade: torres mais rápidas.",
        hab=H("Chamado às Armas", "turbo_area", 45, dur=12, valor=0.66)),
      U("Defesa da Pátria", 40000,
        hab=H("Defesa da Pátria", "turbo_area", 60, dur=20, valor=0.5, global_=True))],
     [U("Negócios Macacos", 500, "Desconto em torres próximas.", desconto=0.1),
      U("Comércio Macaco", 500, desconto=0.15),
      U("Cidade Macaco", 10000, "Mais dinheiro por estouro.", buffs=dict(ouro=0.5)),
      U("Metrópole Macaco", 13000, novo=A("renda", valor=500, visual="moeda")),
      U("Macacópolis", 75000, a=2, valor=5000)]],
    categoria="suporte", raio=28, desc="Melhora torres próximas.", cor=(160, 110, 60)))

_reg(DefTorre(
    "engenheiro", "Macaco Engenheiro", 400, "l", 160,
    [A("projetil", cad=0.7, dano=1, pierce=3, vel=800, dist=240, visual="prego")],
    [[U("Torreta Sentinela", 500, "Cria torretas.",
        novo=A("invocar", cad=10.0, dur=25, base="sentinela")),
      U("Engenharia Rápida", 400, a=1, cad=0.6),
      U("Engrenagens", 575, a=1, nivel_inv=1),
      U("Especialista em Sentinelas", 2500, a=1, nivel_inv=1),
      U("Campeão das Sentinelas", 32000, a=1, nivel_inv=2, cad=0.5)],
     [U("Área de Serviço Maior", 250, alcance=24),
      U("Desconstrução", 350, moab=1, fort=1),
      U("Espuma Purificadora", 800, "Espuma remove camo e regeneração.",
        novo=A("pilha", cad=4.0, dano=1, pilha_pierce=10, pilha_vida=8, retira_camo=True,
               retira_regen=True, visual="espuma")),
      U("Overclock", 13500, "Habilidade: acelera torres.",
        hab=H("Overclock", "turbo_area", 45, dur=30, valor=0.6)),
      U("Ultraimpulso", 105000, buffs=dict(cad=0.7),
        hab=H("Ultraimpulso", "turbo_area", 30, dur=45, valor=0.4))],
     [U("Pregos Enormes", 450, pierce=5, dtype=NORMAL),
      U("Pino", 450, "Pregos desaceleram.", lento=(0.6, 1.0)),
      U("Arma Dupla", 3700, cad=0.5),
      U("Armadilha Bloon", 3100, "Armadilha na trilha.",
        novo=A("pilha", cad=8.0, dano=99, pilha_pierce=500, pilha_vida=30, dtype=NORMAL,
               visual="armadilha")),
      U("Armadilha XXXL", 54000, a="todos", dano=4, pilha_pierce=2000, moab=500)]],
    categoria="suporte", desc="Pregos e torretas.", cor=(230, 190, 40)))

ORDEM_TORRES = list(TORRES)

# Torres auxiliares (invocadas por habilidades/upgrades, nao compraveis)
AUXILIARES: dict[str, DefTorre] = {
    "sentinela": DefTorre("sentinela", "Torreta", 0, "", 120,
                          [A("projetil", cad=0.95, dano=1, pierce=2, vel=800, dist=200,
                             visual="prego")], raio=12, cor=(200, 170, 40)),
    "fenix": DefTorre("fenix", "Fênix", 0, "", 9999,
                      [A("projetil", cad=0.1, dano=5, pierce=10, n=2, spread=30, vel=700,
                         dist=700, global_=True, dtype=NORMAL, visual="fogo")],
                      mov="orbita", raio=22, cor=(250, 120, 30)),
}
