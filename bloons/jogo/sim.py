"""Simulacao deterministica do jogo.

A mesma sequencia de comandos gera exatamente o mesmo estado em qualquer processo.
E isso que permite o modo Batalha em lockstep: o servidor so ordena os comandos e
cada cliente simula as duas pistas localmente.

Unidades: px e segundos. Passo fixo DT = 1/30 s.
"""

from __future__ import annotations

import math
import random
import zlib

from bloons.jogo import bloons_def as BD
from bloons.jogo.bloons_def import AFIADO, ENERGIA, EXPLOSAO, GELO, NORMAL, TIPOS
from bloons.jogo.herois_def import HEROIS, XP_NIVEL
from bloons.jogo.mapas import ALTURA_MAPA, LARGURA_MAPA, MAPAS, Mapa
from bloons.jogo.rodadas import (DIFICULDADES, ENVIOS_POR_CHAVE, agenda_da_rodada,
                                 duracao_rodada)
from bloons.jogo.stats import (arredondar_preco, calcular, definicao, novo_ataque,
                               pode_upar)
from bloons.jogo.torres_def import TORRES

DT = 1.0 / 30.0
CELULA = 64
MODOS_ALVO = ["primeiro", "ultimo", "perto", "forte"]
MAX_PILHAS_POR_TORRE = 30
MAX_EVENTOS = 400

# Batalha
VIDAS_BATALHA = 150
DINHEIRO_INICIAL = 650
ECO_INICIAL = 250.0
ECO_INTERVALO = 6.0
PAUSA_ENTRE_RODADAS = 5.0

ERRO_DINHEIRO = "D"
ERRO_POSICAO = "P"
ERRO_INVALIDO = "M"
ERRO_HEROI = "H"
ERRO_BLOQUEADO = "B"


class Bloon:
    __slots__ = ("id", "tipo", "d", "cam", "vida", "camo", "regen", "regen_orig", "fort",
                 "lento_f", "lento_t", "cong_t", "cola_f", "cola_t", "cola_dps", "queima_dps",
                 "queima_t", "atord_t", "frag", "regen_t", "x", "y", "ang", "vivo", "dot",
                 "vida_max")

    def __init__(self, bid, tipo, d, cam, camo=False, regen=False, fort=False, orig=None):
        self.id = bid
        self.tipo = tipo
        self.d = d
        self.cam = cam
        self.fort = fort and bool(tipo.vida_fortificado)
        self.vida = tipo.vida_fortificado if self.fort else tipo.vida
        self.vida_max = self.vida
        self.camo = camo or tipo.camo_nativo
        self.regen = regen
        self.regen_orig = orig or tipo.nome
        self.lento_f = 1.0
        self.lento_t = 0.0
        self.cong_t = 0.0
        self.cola_f = 1.0
        self.cola_t = 0.0
        self.cola_dps = 0.0
        self.queima_dps = 0.0
        self.queima_t = 0.0
        self.atord_t = 0.0
        self.frag = 0
        self.regen_t = 0.0
        self.dot = 0.0
        self.x = self.y = self.ang = 0.0
        self.vivo = True


class Projetil:
    __slots__ = ("id", "x", "y", "vx", "vy", "vel", "at", "torre", "pierce", "dist",
                 "raio", "atingidos", "alvo", "voltando", "ox", "oy", "ang", "vivo",
                 "fusivel", "quicos", "frag_filho")

    def __init__(self, pid, x, y, ang, at, torre, frag_filho=False):
        self.id = pid
        self.x, self.y = x, y
        self.ox, self.oy = x, y
        self.vel = at["vel"]
        rad = math.radians(ang)
        self.vx, self.vy = math.cos(rad) * self.vel, math.sin(rad) * self.vel
        self.ang = ang
        self.at = at
        self.torre = torre
        self.pierce = at["pierce"]
        self.dist = at["dist"]
        self.raio = at["raio_proj"]
        self.atingidos: set[int] = set()
        self.alvo = None
        self.voltando = False
        self.vivo = True
        self.fusivel = at["fusivel"]
        self.quicos = at["quica"]
        self.frag_filho = frag_filho


class Pilha:
    """Espinhos, estrepes, chamas ou poca de acido parados na trilha."""
    __slots__ = ("x", "y", "pierce", "at", "torre", "vida", "atingidos", "vivo", "visual")

    def __init__(self, x, y, pierce, at, torre, vida, visual):
        self.x, self.y = x, y
        self.pierce = pierce
        self.at = at
        self.torre = torre
        self.vida = vida
        self.atingidos: set[int] = set()
        self.vivo = True
        self.visual = visual


class Torre:
    def __init__(self, tid, chave, dono, x, y, custo, temporaria=0.0):
        self.id = tid
        self.chave = chave
        self.dfn = definicao(chave)
        self.dono = dono
        self.x, self.y = float(x), float(y)
        self.cx, self.cy = float(x), float(y)
        self.caminhos = [0, 0, 0]
        self.nivel = 1 if self.dfn.heroi else 0
        self.xp = 0.0
        self.pops = 0
        self.investido = custo
        self.modo = 0
        self.ang = -90.0
        self.turbo = 1.0
        self.turbo_t = 0.0
        self.temporaria = temporaria
        self.orbita = 0.0
        self.buff: dict = {}
        self.pontos_trilha: list[tuple[int, float]] = []
        self.recalcular()

    def recalcular(self, extra_nivel_inv: int = 0) -> None:
        st = calcular(self.chave, tuple(self.caminhos), self.nivel)
        if extra_nivel_inv:
            for at in st.ataques:
                at["dano"] += extra_nivel_inv
                at["pierce"] += 2 * extra_nivel_inv
                at["cad"] *= 0.8 ** extra_nivel_inv
        antigos = getattr(self, "recargas", [])
        self.st = st
        self.recargas = [antigos[i] if i < len(antigos) else 0.0 for i in range(len(st.ataques))]
        antigas_h = getattr(self, "hab_rec", [])
        self.hab_rec = [antigas_h[i] if i < len(antigas_h) else h["recarga"] * 0.5
                        for i, h in enumerate(st.habs)]

    @property
    def alcance(self) -> float:
        return self.st.alcance * (1.0 + self.buff.get("alcance_pct", 0.0))

    @property
    def detecta_camo(self) -> bool:
        return self.st.camo or self.buff.get("camo", False)


class Pista:
    """O mapa de um jogador: bloons, torres, projeteis e economia."""

    def __init__(self, dono: int, mapa: Mapa, seed: int, vidas: int, dinheiro: int,
                 mult_custo: float = 1.0):
        self.dono = dono
        self.mapa = mapa
        self.rng = random.Random(seed * 7919 + dono)
        self.vidas = vidas
        self.dinheiro = float(dinheiro)
        self.eco = ECO_INICIAL
        self.mult_custo = mult_custo
        self.bloons: list[Bloon] = []
        self.projeteis: list[Projetil] = []
        self.pilhas: list[Pilha] = []
        self.torres: dict[int, Torre] = {}
        self.fila: list[tuple[float, str, bool, bool, bool]] = []  # (t, tipo, camo, regen, fort)
        self.tempo = 0.0
        self.prox_id = 1
        self.prox_torre = 1
        self.grade: dict[tuple[int, int], list[Bloon]] = {}
        self.eventos: list[tuple] = []
        self.heroi_escolhido: str | None = None
        self.tem_heroi = False
        self.pops_total = 0
        self.vazou = 0
        self.buff_t = 0.0
        self.lentidao_global_t = 0.0
        self.lentidao_global_f = 1.0
        self.oponente: Pista | None = None
        self.proximo_cam = 0

    # ------------------------------------------------------------ utilidades
    def _nid(self) -> int:
        self.prox_id += 1
        return self.prox_id

    def evento(self, *e) -> None:
        if len(self.eventos) < MAX_EVENTOS:
            self.eventos.append(e)

    def custo(self, base: int, x: float | None = None, y: float | None = None) -> int:
        desc = 0.0
        if x is not None:
            for t in self.torres.values():
                if t.st.desconto and (t.x - x) ** 2 + (t.y - y) ** 2 <= t.alcance ** 2:
                    desc = max(desc, t.st.desconto)
        return arredondar_preco(base * self.mult_custo * (1.0 - desc))

    def custo_upgrade(self, torre: Torre, p: int) -> int | None:
        dfn = torre.dfn
        if dfn.heroi or not dfn.caminhos or not pode_upar(torre.caminhos, p):
            return None
        return self.custo(dfn.caminhos[p][torre.caminhos[p]].custo, torre.x, torre.y)

    def valor_venda(self, torre: Torre) -> int:
        return int(torre.investido * torre.st.venda)

    # ------------------------------------------------------------ bloons
    def criar_bloon(self, nome, d=0.0, cam=None, camo=False, regen=False, fort=False,
                    orig=None) -> Bloon:
        if cam is None:
            cam = self.proximo_cam % len(self.mapa.caminhos)
            self.proximo_cam += 1
        b = Bloon(self._nid(), TIPOS[nome], d, cam, camo, regen, fort, orig)
        self._posicionar(b)
        self.bloons.append(b)
        return b

    def _posicionar(self, b: Bloon) -> None:
        b.x, b.y, b.ang = self.mapa.caminhos[b.cam].posicao(b.d)

    def agendar(self, nome, atraso, camo=False, regen=False, fort=False) -> None:
        self.fila.append((self.tempo + atraso, nome, camo, regen, fort))

    def rbe_restante(self, b: Bloon) -> int:
        filhos = sum(BD.rbe(f, b.fort) for f in b.tipo.filhos)
        return max(1, int(b.vida)) + filhos

    def aplicar_dano(self, b: Bloon, dano: float, at: dict, torre: Torre | None,
                     proj: Projetil | None = None, dtype: str | None = None) -> bool:
        """Aplica um acerto. Devolve True se consumiu pierce."""
        if not b.vivo:
            return False
        dtype = dtype or at["dtype"]
        if torre is not None and torre.buff.get("dtype_normal"):
            dtype = NORMAL
        tp = b.tipo
        if dtype != NORMAL and dtype in tp.imune:
            self.evento("bloqueio", b.x, b.y)
            return True
        if b.cong_t > 0 and dtype == AFIADO:
            return True
        if at["retira_camo"]:
            b.camo = False
        if at["retira_regen"]:
            b.regen = False
        if at["fragiliza"]:
            b.frag = max(b.frag, at["fragiliza"])
        # efeitos
        if at["congela"] and (tp.congela or (tp.moab and at["moab_congela"])):
            b.cong_t = max(b.cong_t, at["congela"] * (0.5 if tp.moab else 1.0))
        if at["lento"] and (not tp.moab or at["moab_lento"]):
            f, t = at["lento"]
            b.lento_f, b.lento_t = min(b.lento_f if b.lento_t > 0 else 1.0, f), max(b.lento_t, t)
        if at["cola"] and (not tp.moab or at["moab_cola"]):
            f, t, dps = at["cola"]
            b.cola_f, b.cola_t, b.cola_dps = f, t, max(b.cola_dps, dps)
        if at["queima"]:
            dps, t = at["queima"]
            b.queima_dps, b.queima_t = max(b.queima_dps, dps), max(b.queima_t, t)
        if at["atordoa"] and (not tp.moab or at["moab_atordoa"]):
            b.atord_t = max(b.atord_t, at["atordoa"] * (0.4 if tp.moab else 1.0))
        if at["empurra"]:
            b.d = max(0.0, b.d - at["empurra"] * (0.35 if tp.moab else 1.0))
        if dano <= 0:
            return True
        extra = b.frag
        if tp.moab:
            extra += at["moab"] + (torre.buff.get("moab", 0) if torre else 0)
        if tp.nome == "ceramica":
            extra += at["cer"]
        if b.fort:
            extra += at["fort"]
        b.vida -= dano + extra
        b.regen_t = 0.0
        if b.vida <= 0:
            self.estourar(b, -b.vida, dtype, torre, proj)
        return True

    def estourar(self, b: Bloon, excesso: float, dtype: str, torre: Torre | None,
                 proj: Projetil | None, profundidade: int = 0) -> None:
        b.vivo = False
        ouro = 0.0
        if torre is not None:
            torre.pops += 1
            ouro = torre.st.ouro + torre.buff.get("ouro", 0.0)
            if b.tipo.nome == "chumbo" and torre.st.ouro:
                ouro += 2
        self.dinheiro += 1 + ouro
        self.pops_total += 1
        self._xp(1.0)
        self.evento("pop", b.x, b.y, b.tipo.nome)
        tp = b.tipo
        n = len(tp.filhos)
        if not n:
            return
        passo = 22.0 if tp.moab else 7.0
        filho_camo = b.camo or tp.nome == "ddt"
        filho_regen = b.regen or tp.nome == "ddt"
        for i, nome in enumerate(tp.filhos):
            d = max(0.0, b.d + (i - (n - 1) / 2) * passo)
            c = self.criar_bloon(nome, d, b.cam, filho_camo, filho_regen, b.fort,
                                 b.regen_orig if b.regen else None)
            c.lento_f, c.lento_t = b.lento_f, b.lento_t
            c.queima_dps, c.queima_t = b.queima_dps, b.queima_t
            if proj is not None:
                proj.atingidos.add(c.id)
            if excesso > 0 and not c.tipo.moab and profundidade < 12:
                if dtype == NORMAL or dtype not in c.tipo.imune:
                    c.vida -= excesso
                    if c.vida <= 0:
                        self.estourar(c, -c.vida, dtype, torre, proj, profundidade + 1)

    def _xp(self, v: float) -> None:
        if not self.tem_heroi:
            return
        for t in self.torres.values():
            if t.dfn.heroi and t.nivel < 20:
                t.xp += v
                while t.nivel < 20 and t.xp >= XP_NIVEL[t.nivel + 1]:
                    t.nivel += 1
                    t.recalcular()
                    self.evento("nivel", t.x, t.y, t.nivel)

    # ------------------------------------------------------------ comandos
    def colocar_torre(self, chave: str, x: float, y: float) -> str | None:
        heroi = chave in HEROIS
        if chave not in TORRES and not heroi:
            return ERRO_INVALIDO
        if heroi and (self.tem_heroi or chave != self.heroi_escolhido):
            return ERRO_HEROI
        dfn = definicao(chave)
        if not self.posicao_valida(dfn, x, y):
            return ERRO_POSICAO
        custo = self.custo(dfn.custo, x, y)
        if self.dinheiro < custo:
            return ERRO_DINHEIRO
        self.dinheiro -= custo
        t = Torre(self.prox_torre, chave, self.dono, x, y, custo)
        self.prox_torre += 1
        self._preparar_trilha(t)
        self.torres[t.id] = t
        if heroi:
            self.tem_heroi = True
        self.buff_t = 0.0
        self.evento("colocar", x, y)
        return None

    def posicao_valida(self, dfn, x: float, y: float) -> bool:
        r = dfn.raio
        if not (r * 0.5 <= x <= LARGURA_MAPA - r * 0.5 and r * 0.5 <= y <= ALTURA_MAPA - r * 0.5):
            return False
        voa = dfn.mov in ("orbita", "heli")
        if not voa:
            if self.mapa.na_trilha(x, y, r * 0.8):
                return False
            if self.mapa.bloqueado(x, y, r):
                return False
            agua = self.mapa.eh_agua(x, y)
            if dfn.agua != agua:
                return False
        elif self.mapa.eh_agua(x, y) and not dfn.agua:
            pass
        for t in self.torres.values():
            if t.temporaria:
                continue
            if (t.cx - x) ** 2 + (t.cy - y) ** 2 < (t.dfn.raio + r) ** 2 * 0.8:
                return False
        return True

    def _preparar_trilha(self, t: Torre) -> None:
        pts = []
        for ci, cam in enumerate(self.mapa.caminhos):
            for d in cam.distancias_no_raio(t.x, t.y, max(40.0, t.alcance)):
                pts.append((ci, d))
        t.pontos_trilha = pts

    def upar(self, tid: int, p: int) -> str | None:
        t = self.torres.get(tid)
        if t is None or t.temporaria or not 0 <= p <= 2:
            return ERRO_INVALIDO
        custo = self.custo_upgrade(t, p)
        if custo is None:
            return ERRO_BLOQUEADO
        if self.dinheiro < custo:
            return ERRO_DINHEIRO
        self.dinheiro -= custo
        t.investido += custo
        t.caminhos[p] += 1
        t.recalcular()
        self._preparar_trilha(t)
        self.buff_t = 0.0
        self.evento("upgrade", t.x, t.y)
        return None

    def vender(self, tid: int) -> str | None:
        t = self.torres.get(tid)
        if t is None or t.temporaria:
            return ERRO_INVALIDO
        self.dinheiro += self.valor_venda(t)
        del self.torres[tid]
        if t.dfn.heroi:
            self.tem_heroi = False
        self.buff_t = 0.0
        self.evento("venda", t.x, t.y)
        return None

    def mudar_modo(self, tid: int, modo: int) -> str | None:
        t = self.torres.get(tid)
        if t is None or not 0 <= modo < len(MODOS_ALVO):
            return ERRO_INVALIDO
        t.modo = modo
        return None

    def usar_habilidade(self, tid: int, idx: int) -> str | None:
        t = self.torres.get(tid)
        if t is None or not 0 <= idx < len(t.st.habs) or t.hab_rec[idx] > 0:
            return ERRO_INVALIDO
        h = t.st.habs[idx]
        t.hab_rec[idx] = h["recarga"]
        self._executar_habilidade(t, h)
        self.evento("habilidade", t.x, t.y, h["nome"])
        return None

    # ------------------------------------------------------------ habilidades
    def _executar_habilidade(self, t: Torre, h: dict) -> None:
        tipo = h["tipo"]
        at = novo_ataque(dict(dtype=NORMAL, atordoa=h.get("atordoa", 0.0),
                              queima=h.get("queima"), congela=h.get("congela", 0.0),
                              moab_atordoa=True, moab_congela=bool(h.get("congela"))))
        if tipo == "turbo":
            t.turbo, t.turbo_t = h["valor"], h["dur"]
        elif tipo == "turbo_area":
            filtro = set(h.get("filtro", "").split(",")) - {""}
            for o in self.torres.values():
                if filtro and o.chave not in filtro:
                    continue
                if h.get("global_") or (o.x - t.x) ** 2 + (o.y - t.y) ** 2 <= (t.alcance + 60) ** 2:
                    o.turbo, o.turbo_t = min(o.turbo, h["valor"]), max(o.turbo_t, h["dur"])
        elif tipo == "dano_global":
            for b in list(self.bloons):
                self.aplicar_dano(b, h["valor"], at, t)
            self.evento("flash", 0, 0, (255, 255, 255))
        elif tipo == "dano_forte":
            alvos = [b for b in self.bloons if b.vivo and (b.tipo.moab or not h.get("moab_so"))]
            alvos.sort(key=lambda b: (b.tipo.rank, b.d), reverse=True)
            for b in alvos[:h.get("n", 1)]:
                self.evento("raio", t.x, t.y, b.x, b.y)
                bx, by = b.x, b.y
                self.aplicar_dano(b, h["valor"], at, t)
                if h.get("splash"):
                    self._explosao(bx, by, h["splash"], h.get("sdano", 1), 999, at, t, NORMAL)
        elif tipo == "lentidao":
            self.lentidao_global_f, self.lentidao_global_t = h["valor"], h["dur"]
            if h.get("dano"):
                for b in list(self.bloons):
                    self.aplicar_dano(b, h["dano"], at, t)
            self.evento("flash", 0, 0, (150, 220, 90))
        elif tipo == "congelar_global":
            for b in self.bloons:
                if b.tipo.congela or h.get("moab"):
                    b.cong_t = max(b.cong_t, h["dur"] * (0.5 if b.tipo.moab else 1.0))
            self.evento("flash", 0, 0, (180, 230, 255))
        elif tipo == "dinheiro":
            self.dinheiro += h["valor"]
            self.evento("dinheiro", t.x, t.y, h["valor"])
        elif tipo == "roubo":
            self.dinheiro += h["valor"]
            if self.oponente is not None:
                self.oponente.dinheiro = max(0.0, self.oponente.dinheiro - h["valor"])
            self.evento("dinheiro", t.x, t.y, h["valor"])
        elif tipo == "invocar":
            self._invocar(t, h.get("base", "sentinela"), h.get("dur", 15.0),
                          h.get("nivel"))
        elif tipo == "reverso":
            for b in self.bloons:
                b.d = max(0.0, b.d - h["valor"] * (0.5 if b.tipo.moab else 1.0))
            self.evento("flash", 0, 0, (40, 40, 60))
        elif tipo in ("spikes_global", "spikes_local"):
            at_p = novo_ataque(dict(dano=h.get("dano", 1), dtype=NORMAL))
            if tipo == "spikes_global":
                for cam in self.mapa.caminhos:
                    d = 60.0
                    while d < cam.comprimento:
                        x, y, _ = cam.posicao(d)
                        self.pilhas.append(Pilha(x, y, h["valor"], at_p, t, 20.0, "espinhos"))
                        d += 140.0
            else:
                ci, d = self._ponto_trilha_mais_avancado(t)
                x, y, _ = self.mapa.caminhos[ci].posicao(d)
                self.pilhas.append(Pilha(x, y, h["valor"], at_p, t, h.get("dur", 10.0),
                                         "espinheiro"))

    def _ponto_trilha_mais_avancado(self, t: Torre) -> tuple[int, float]:
        if t.pontos_trilha:
            return max(t.pontos_trilha, key=lambda p: p[1])
        cam = self.mapa.caminhos[0]
        melhor = min(cam.amostras, key=lambda a: (a[1] - t.x) ** 2 + (a[2] - t.y) ** 2)
        return 0, melhor[0]

    def _invocar(self, t: Torre, base: str, dur: float, nivel=None) -> None:
        ang = self.rng.uniform(0, 2 * math.pi)
        dist = 40 + self.rng.uniform(0, 30)
        x = min(max(t.x + math.cos(ang) * dist, 20), LARGURA_MAPA - 20)
        y = min(max(t.y + math.sin(ang) * dist, 20), ALTURA_MAPA - 20)
        s = Torre(self.prox_torre, base, self.dono, x, y, 0, temporaria=dur)
        self.prox_torre += 1
        if nivel:
            s.caminhos = list(nivel)
            s.recalcular()
        if base == "sentinela":
            extra = 0
            for at in t.st.ataques:
                if at["tipo"] == "invocar":
                    extra = at["nivel_inv"]
            if extra:
                s.recalcular(extra)
        self._preparar_trilha(s)
        self.torres[s.id] = s
        self.evento("invocar", x, y)

    # ------------------------------------------------------------ passo
    def passo(self) -> None:
        self.tempo += DT
        self._spawns()
        self._grade()
        self.buff_t -= DT
        if self.buff_t <= 0:
            self._recalcular_buffs()
            self.buff_t = 0.5
        if self.lentidao_global_t > 0:
            self.lentidao_global_t -= DT
        for t in list(self.torres.values()):
            self._torre(t)
        self._projeteis()
        self._pilhas()
        self._mover_bloons()
        if any(not b.vivo for b in self.bloons):
            self.bloons = [b for b in self.bloons if b.vivo]

    def _spawns(self) -> None:
        if not self.fila:
            return
        restantes = []
        for item in self.fila:
            if item[0] <= self.tempo:
                _, nome, camo, regen, fort = item
                self.criar_bloon(nome, 0.0, None, camo, regen, fort)
            else:
                restantes.append(item)
        self.fila = restantes

    def _grade(self) -> None:
        g: dict[tuple[int, int], list[Bloon]] = {}
        for b in self.bloons:
            if b.vivo:
                g.setdefault((int(b.x) // CELULA, int(b.y) // CELULA), []).append(b)
        self.grade = g

    def _vizinhos(self, x: float, y: float, r: float):
        c0, c1 = int(x - r) // CELULA, int(x + r) // CELULA
        l0, l1 = int(y - r) // CELULA, int(y + r) // CELULA
        g = self.grade
        for cx in range(c0, c1 + 1):
            for cy in range(l0, l1 + 1):
                lst = g.get((cx, cy))
                if lst:
                    yield from lst

    def _recalcular_buffs(self) -> None:
        for t in self.torres.values():
            t.buff = {}
        for f in self.torres.values():
            for at in f.st.ataques:
                if not at["buffs"]:
                    continue
                r2 = f.alcance ** 2
                for t in self.torres.values():
                    if t is f and at["tipo"] == "buff":
                        continue
                    if (t.x - f.x) ** 2 + (t.y - f.y) ** 2 <= r2:
                        for k, v in at["buffs"].items():
                            if k == "cad":
                                t.buff["cad"] = t.buff.get("cad", 1.0) * v
                            elif isinstance(v, bool):
                                t.buff[k] = v
                            else:
                                t.buff[k] = t.buff.get(k, 0) + v
        for t in self.torres.values():
            if "cad" in t.buff:
                t.buff["cad"] = max(0.4, t.buff["cad"])

    # ------------------------------------------------------------ torres
    def _alvo(self, t: Torre, at: dict, alcance: float):
        so_moab = at["so_moab"]
        camo = t.detecta_camo
        glob = at["global_"] or alcance >= 5000
        r2 = alcance * alcance
        melhor = None
        modo = at["alvo"] or MODOS_ALVO[t.modo]
        chave_melhor = None
        for b in self.bloons:
            if not b.vivo or (b.camo and not camo) or (so_moab and not b.tipo.moab):
                continue
            dx, dy = b.x - t.x, b.y - t.y
            dist2 = dx * dx + dy * dy
            if not glob and dist2 > r2:
                continue
            if modo == "primeiro":
                k = b.d
            elif modo == "ultimo":
                k = -b.d
            elif modo == "perto":
                k = -dist2
            else:
                k = b.tipo.rank * 100000 + b.d
            if chave_melhor is None or k > chave_melhor:
                chave_melhor, melhor = k, b
        return melhor

    def _mover_torre(self, t: Torre) -> None:
        mov = t.dfn.mov
        if mov == "orbita":
            t.orbita += DT * 1.3
            raio = 110.0
            t.x = t.cx + math.cos(t.orbita) * raio
            t.y = t.cy + math.sin(t.orbita) * raio * 0.75
            t.ang = math.degrees(t.orbita) + 90
        elif mov == "heli":
            alvo = None
            limite = 9999 if t.st.persegue else 260
            melhor = None
            for b in self.bloons:
                if b.vivo and (not b.camo or t.detecta_camo):
                    if (b.x - t.cx) ** 2 + (b.y - t.cy) ** 2 <= limite ** 2:
                        if melhor is None or b.d > melhor.d:
                            melhor = b
            if melhor is not None:
                alvo = (melhor.x, melhor.y)
            else:
                alvo = (t.cx, t.cy)
            dx, dy = alvo[0] - t.x, alvo[1] - t.y
            dist = math.hypot(dx, dy)
            if dist > 30:
                v = 260.0 * DT
                t.x += dx / dist * min(v, dist)
                t.y += dy / dist * min(v, dist)

    def _torre(self, t: Torre) -> None:
        if t.temporaria:
            t.temporaria -= DT
            if t.temporaria <= 0:
                del self.torres[t.id]
                return
        if t.dfn.mov != "fixo":
            self._mover_torre(t)
        if t.turbo_t > 0:
            t.turbo_t -= DT
            if t.turbo_t <= 0:
                t.turbo = 1.0
        for i in range(len(t.hab_rec)):
            if t.hab_rec[i] > 0:
                t.hab_rec[i] -= DT
        buff = t.buff
        mult_cad = t.turbo * buff.get("cad", 1.0)
        alcance = t.alcance
        for i, at in enumerate(t.st.ataques):
            tipo = at["tipo"]
            if tipo in ("buff", "renda"):
                continue
            t.recargas[i] -= DT
            if t.recargas[i] > 0:
                continue
            cad = max(0.02, at["cad"] * mult_cad)
            if tipo == "invocar":
                t.recargas[i] = cad
                self._invocar(t, at["base"] or "sentinela", at["dur"] or 20.0)
                continue
            if tipo == "pilha":
                if t.pontos_trilha and self.bloons_ou_fila():
                    meus = sum(1 for p in self.pilhas if p.torre is t)
                    if meus < MAX_PILHAS_POR_TORRE:
                        ci, d = t.pontos_trilha[self.rng.randrange(len(t.pontos_trilha))]
                        x, y, _ = self.mapa.caminhos[ci].posicao(d)
                        x += self.rng.uniform(-10, 10)
                        y += self.rng.uniform(-10, 10)
                        self.pilhas.append(Pilha(x, y, at["pilha_pierce"] + buff.get("pierce", 0),
                                                 at, t, at["pilha_vida"], at["visual"]))
                        t.recargas[i] = cad
                continue
            if tipo == "queda":
                if self.bloons:
                    x, y = t.x, t.y
                    if at["na_trilha"]:
                        alvo = self._alvo(t, at, 9999)
                        if alvo is not None:
                            x, y = alvo.x, alvo.y
                    p = Projetil(self._nid(), x, y, 0, at, t)
                    p.vx = p.vy = 0.0
                    p.fusivel = at["fusivel"] or 0.5
                    self.projeteis.append(p)
                    t.recargas[i] = cad
                continue
            if tipo == "aura":
                raio = at["raio_aura"] or alcance
                self._aura(t, at, raio)
                t.recargas[i] = cad
                continue
            if tipo == "radial":
                glob = at["global_"] or t.dfn.mov == "orbita"
                if glob:
                    ok = bool(self.bloons)
                else:
                    ok = self._alvo(t, at, alcance) is not None
                if ok:
                    n = int(at["n"])
                    base = t.ang if t.dfn.mov == "orbita" else 0.0
                    for k in range(n):
                        self._disparar(t, at, base + k * 360.0 / n)
                    t.recargas[i] = cad
                    self.evento("tiro", t.x, t.y, t.chave)
                continue
            alvo = self._alvo(t, at, alcance)
            if alvo is None:
                continue
            t.recargas[i] = cad
            ang = math.degrees(math.atan2(alvo.y - t.y, alvo.x - t.x))
            if i == 0 and t.dfn.mov == "fixo":
                t.ang = ang
            if tipo == "hitscan":
                self._hitscan(t, at, alvo)
            elif tipo == "cadeia":
                self._cadeia(t, at, alvo)
            elif tipo == "morteiro":
                for _ in range(int(at["n"])):
                    im = at["impreciso"]
                    x = alvo.x + self.rng.uniform(-im, im)
                    y = alvo.y + self.rng.uniform(-im, im)
                    p = Projetil(self._nid(), x, y, 0, at, t)
                    p.vx = p.vy = 0.0
                    p.fusivel = 0.7
                    self.projeteis.append(p)
                    self.evento("morteiro", t.x, t.y, x, y)
            else:
                n = int(at["n"])
                spread = at["spread"]
                if n <= 1:
                    self._disparar(t, at, ang, alvo)
                elif spread >= 360:
                    for k in range(n):
                        self._disparar(t, at, ang + k * 360.0 / n, alvo)
                else:
                    passo = spread / (n - 1) if n > 1 else 0
                    for k in range(n):
                        self._disparar(t, at, ang - spread / 2 + k * passo, alvo)
                self.evento("tiro", t.x, t.y, t.chave)

    def bloons_ou_fila(self) -> bool:
        return bool(self.bloons or self.fila)

    def _ataque_efetivo(self, t: Torre, at: dict) -> dict:
        b = t.buff
        if not b or not any(k in b for k in ("dano", "pierce", "dtype_normal")):
            return at
        ef = dict(at)
        ef["dano"] = at["dano"] + (b.get("dano", 0) if at["dano"] > 0 else 0)
        ef["pierce"] = at["pierce"] + b.get("pierce", 0)
        if at["sdano"]:
            ef["sdano"] = at["sdano"] + b.get("dano", 0)
        if b.get("dtype_normal"):
            ef["dtype"] = NORMAL
            ef["sdtype"] = NORMAL
        return ef

    def _disparar(self, t: Torre, at: dict, ang: float, alvo: Bloon | None = None) -> None:
        at = self._ataque_efetivo(t, at)
        p = Projetil(self._nid(), t.x, t.y, ang, at, t)
        if at["busca"] and alvo is not None:
            p.alvo = alvo
        self.projeteis.append(p)

    def _aura(self, t: Torre, at: dict, raio: float) -> None:
        at = self._ataque_efetivo(t, at)
        pierce = at["pierce"]
        r2 = raio * raio
        acertou = False
        camo = t.detecta_camo or at["retira_camo"]
        for b in list(self._vizinhos(t.x, t.y, raio)):
            if pierce <= 0:
                break
            if not b.vivo or (b.camo and not camo):
                continue
            if (b.x - t.x) ** 2 + (b.y - t.y) ** 2 <= r2:
                if self.aplicar_dano(b, at["dano"], at, t):
                    pierce -= 1
                    acertou = True
                    if at["frag"] and b.cong_t > 0 and not b.vivo:
                        self._fragmentar(b.x, b.y, at["frag"], t)
        if acertou and at["visual"] != "nenhum":
            self.evento("aura", t.x, t.y, raio, at["visual"])

    def _hitscan(self, t: Torre, at: dict, alvo: Bloon) -> None:
        at = self._ataque_efetivo(t, at)
        self.evento("raio", t.x, t.y, alvo.x, alvo.y, at["visual"])
        if at["linha"]:
            dx, dy = alvo.x - t.x, alvo.y - t.y
            L = math.hypot(dx, dy) or 1.0
            ux, uy = dx / L, dy / L
            pierce = at["pierce"]
            atingidos = []
            for b in self.bloons:
                if not b.vivo:
                    continue
                px, py = b.x - t.x, b.y - t.y
                proj = px * ux + py * uy
                if proj < 0:
                    continue
                if abs(px * uy - py * ux) <= 18 + b.tipo.raio * 0.5:
                    atingidos.append((proj, b))
            atingidos.sort(key=lambda e: e[0])
            for _, b in atingidos[:int(pierce)]:
                self.aplicar_dano(b, at["dano"], at, t)
            return
        x, y = alvo.x, alvo.y
        self.aplicar_dano(alvo, at["dano"], at, t)
        if at["splash"]:
            self._explosao(x, y, at["splash"], at["sdano"], at["spierce"], at, t, at["sdtype"])
        if at["frag"]:
            self._fragmentar(x, y, at["frag"], t)
        feitos = {alvo.id}
        ultimo = (x, y)
        for _ in range(int(at["quica"])):
            prox = self._mais_proximo(ultimo[0], ultimo[1], 160, feitos, t.detecta_camo)
            if prox is None:
                break
            feitos.add(prox.id)
            self.evento("raio", ultimo[0], ultimo[1], prox.x, prox.y, at["visual"])
            ultimo = (prox.x, prox.y)
            self.aplicar_dano(prox, at["dano"], at, t)

    def _cadeia(self, t: Torre, at: dict, alvo: Bloon) -> None:
        at = self._ataque_efetivo(t, at)
        feitos = {alvo.id}
        ultimo = (t.x, t.y)
        atual = alvo
        for _ in range(int(at["saltos"]) + 1):
            self.evento("raio", ultimo[0], ultimo[1], atual.x, atual.y, "relampago")
            ultimo = (atual.x, atual.y)
            self.aplicar_dano(atual, at["dano"], at, t)
            prox = self._mais_proximo(ultimo[0], ultimo[1], 140, feitos, True)
            if prox is None:
                break
            feitos.add(prox.id)
            atual = prox

    def _mais_proximo(self, x, y, r, excluir, camo):
        melhor, md = None, r * r
        for b in self._vizinhos(x, y, r):
            if not b.vivo or b.id in excluir or (b.camo and not camo):
                continue
            d2 = (b.x - x) ** 2 + (b.y - y) ** 2
            if d2 < md:
                melhor, md = b, d2
        return melhor

    def _explosao(self, x, y, raio, dano, pierce, at, torre, dtype) -> None:
        self.evento("explosao", x, y, raio, at["visual"])
        dtype = dtype or EXPLOSAO
        if torre is not None and torre.buff.get("dtype_normal"):
            dtype = NORMAL
        r2 = raio * raio
        n = int(pierce)
        for b in list(self._vizinhos(x, y, raio + 40)):
            if n <= 0:
                break
            if not b.vivo:
                continue
            rr = raio + b.tipo.raio * 0.6
            if (b.x - x) ** 2 + (b.y - y) ** 2 <= rr * rr:
                if self.aplicar_dano(b, dano, at, torre, dtype=dtype):
                    n -= 1

    def _fragmentar(self, x, y, frag: dict, torre: Torre) -> None:
        at = novo_ataque(dict(tipo="projetil", vel=520, dist=110, **frag))
        n = int(frag.get("n", 6))
        for k in range(n):
            p = Projetil(self._nid(), x, y, k * 360.0 / n, at, torre, frag_filho=True)
            self.projeteis.append(p)

    # ------------------------------------------------------------ projeteis
    def _projeteis(self) -> None:
        vivos = []
        for p in self.projeteis:
            if not p.vivo:
                continue
            at = p.at
            if p.fusivel > 0:
                if p.vx == 0 and p.vy == 0:
                    p.fusivel -= DT
                    if p.fusivel <= 0:
                        self._explosao(p.x, p.y, at["splash"] or 40, at["sdano"] or 1,
                                       at["spierce"] or 20, at, p.torre, at["sdtype"])
                        if at["queima"]:
                            self.pilhas.append(Pilha(p.x, p.y, 30, novo_ataque(
                                dict(dano=at["queima"][0], dtype=NORMAL)), p.torre,
                                at["queima"][1], "chamas"))
                        if at["frag"]:
                            self._fragmentar(p.x, p.y, at["frag"], p.torre)
                        p.vivo = False
                        continue
                    vivos.append(p)
                    continue
            # teleguiado
            if p.alvo is not None:
                if not p.alvo.vivo:
                    p.alvo = self._mais_proximo(p.x, p.y, 300, p.atingidos, True)
                if p.alvo is not None:
                    dx, dy = p.alvo.x - p.x, p.alvo.y - p.y
                    L = math.hypot(dx, dy) or 1.0
                    k = 0.25
                    p.vx = p.vx * (1 - k) + dx / L * p.vel * k
                    p.vy = p.vy * (1 - k) + dy / L * p.vel * k
                    n = math.hypot(p.vx, p.vy) or 1.0
                    p.vx, p.vy = p.vx / n * p.vel, p.vy / n * p.vel
            if at["boom"]:
                self._mover_bumerangue(p)
            else:
                p.x += p.vx * DT
                p.y += p.vy * DT
                p.dist -= p.vel * DT
            p.ang = math.degrees(math.atan2(p.vy, p.vx))
            if p.dist <= 0 or not (-60 < p.x < LARGURA_MAPA + 60 and -60 < p.y < ALTURA_MAPA + 60):
                self._fim_projetil(p)
                continue
            self._colidir(p)
            if p.vivo:
                vivos.append(p)
        self.projeteis = vivos

    def _mover_bumerangue(self, p: Projetil) -> None:
        if not p.voltando:
            p.x += p.vx * DT
            p.y += p.vy * DT
            p.dist -= p.vel * DT
            if p.dist <= p.at["dist"] * 0.5:
                p.voltando = True
            else:
                # curva suave para a direita
                ang = math.atan2(p.vy, p.vx) + 2.2 * DT
                p.vx, p.vy = math.cos(ang) * p.vel, math.sin(ang) * p.vel
        else:
            tx, ty = p.torre.x, p.torre.y
            dx, dy = tx - p.x, ty - p.y
            L = math.hypot(dx, dy) or 1.0
            k = 0.18
            p.vx = p.vx * (1 - k) + dx / L * p.vel * k
            p.vy = p.vy * (1 - k) + dy / L * p.vel * k
            n = math.hypot(p.vx, p.vy) or 1.0
            p.vx, p.vy = p.vx / n * p.vel, p.vy / n * p.vel
            p.x += p.vx * DT
            p.y += p.vy * DT
            p.dist -= p.vel * DT * 0.5
            if L < 20:
                p.dist = 0

    def _fim_projetil(self, p: Projetil) -> None:
        p.vivo = False
        at = p.at
        if at["frag"] and not p.frag_filho and at["visual"] in ("juggernaut", "bola_espinho"):
            self._fragmentar(p.x, p.y, at["frag"], p.torre)

    def _colidir(self, p: Projetil) -> None:
        at = p.at
        r = p.raio
        for b in list(self._vizinhos(p.x, p.y, r + 90)):
            if not b.vivo or b.id in p.atingidos:
                continue
            rr = r + b.tipo.raio
            if (b.x - p.x) ** 2 + (b.y - p.y) ** 2 > rr * rr:
                continue
            p.atingidos.add(b.id)
            bx, by = b.x, b.y
            consumiu = self.aplicar_dano(b, at["dano"], at, p.torre, p)
            if at["splash"]:
                self._explosao(bx, by, at["splash"], at["sdano"], at["spierce"], at, p.torre,
                               at["sdtype"])
                if at["frag"] and not p.frag_filho:
                    self._fragmentar(bx, by, at["frag"], p.torre)
            elif at["frag"] and not p.frag_filho and not b.vivo \
                    and at["visual"] not in ("juggernaut", "bola_espinho"):
                self._fragmentar(bx, by, at["frag"], p.torre)
            if consumiu:
                p.pierce -= 1
            if p.quicos > 0 and p.pierce > 0:
                prox = self._mais_proximo(bx, by, 200, p.atingidos, p.torre.detecta_camo)
                if prox is not None:
                    p.quicos -= 1
                    dx, dy = prox.x - bx, prox.y - by
                    L = math.hypot(dx, dy) or 1.0
                    p.vx, p.vy = dx / L * p.vel, dy / L * p.vel
                    p.dist = max(p.dist, 220)
            if p.pierce <= 0:
                p.vivo = False
                return

    # ------------------------------------------------------------ pilhas
    def _pilhas(self) -> None:
        if not self.pilhas:
            return
        vivas = []
        for s in self.pilhas:
            s.vida -= DT
            if s.vida <= 0 or s.pierce <= 0:
                at = s.at
                if at["splash"] and at["tipo"] == "pilha":
                    self._explosao(s.x, s.y, at["splash"], at["sdano"], at["spierce"], at,
                                   s.torre, NORMAL)
                continue
            for b in self._vizinhos(s.x, s.y, 30):
                if s.pierce <= 0:
                    break
                if not b.vivo or b.id in s.atingidos:
                    continue
                rr = 14 + b.tipo.raio
                if (b.x - s.x) ** 2 + (b.y - s.y) ** 2 <= rr * rr:
                    s.atingidos.add(b.id)
                    if self.aplicar_dano(b, s.at["dano"], s.at, s.torre):
                        s.pierce -= 1
            vivas.append(s)
        self.pilhas = vivas

    # ------------------------------------------------------------ movimento dos bloons
    def _mover_bloons(self) -> None:
        base = BD.VELOCIDADE_BASE * DT
        glob = self.lentidao_global_f if self.lentidao_global_t > 0 else 1.0
        caminhos = self.mapa.caminhos
        for b in self.bloons:
            if not b.vivo:
                continue
            # dano continuo (cola corrosiva, fogo)
            if b.queima_t > 0 or (b.cola_t > 0 and b.cola_dps):
                b.dot += (b.queima_dps if b.queima_t > 0 else 0) * DT
                if b.cola_t > 0:
                    b.dot += b.cola_dps * DT
                if b.dot >= 1.0:
                    dano = int(b.dot)
                    b.dot -= dano
                    self._dano_continuo(b, dano)
                    if not b.vivo:
                        continue
            if b.queima_t > 0:
                b.queima_t -= DT
            if b.cong_t > 0:
                b.cong_t -= DT
                continue
            if b.atord_t > 0:
                b.atord_t -= DT
                continue
            v = base * b.tipo.velocidade * glob
            if b.lento_t > 0:
                b.lento_t -= DT
                v *= b.lento_f
            if b.cola_t > 0:
                b.cola_t -= DT
                v *= b.cola_f
            b.d += v
            cam = caminhos[b.cam]
            if b.d >= cam.comprimento:
                b.vivo = False
                perda = self.rbe_restante(b)
                self.vidas -= perda
                self.vazou += perda
                self.evento("vazou", b.x, b.y, perda)
                continue
            b.x, b.y, b.ang = cam.posicao(b.d)
            if b.regen:
                b.regen_t += DT
                if b.regen_t >= 3.0:
                    b.regen_t = 0.0
                    self._regenerar(b)

    def _dano_continuo(self, b: Bloon, dano: int) -> None:
        at = _AT_DOT
        self.aplicar_dano(b, dano, at, None)

    def _regenerar(self, b: Bloon) -> None:
        prox = BD.REGEN_PROXIMO.get(b.tipo.nome)
        if prox is None:
            return
        orig = TIPOS[b.regen_orig]
        if TIPOS[prox].rank > orig.rank:
            return
        if b.tipo.nome == "rosa" and orig.nome in ("branco", "roxo"):
            prox = orig.nome
        b.tipo = TIPOS[prox]
        b.vida = b.tipo.vida_fortificado if b.fort and b.tipo.vida_fortificado else b.tipo.vida
        self.evento("regen", b.x, b.y)

    # ------------------------------------------------------------ rodada
    def pagar_renda(self) -> None:
        for t in self.torres.values():
            for at in t.st.ataques:
                if at["tipo"] == "renda" and at["valor"]:
                    self.dinheiro += at["valor"]
                    self.evento("dinheiro", t.x, t.y, int(at["valor"]))

    def hash(self) -> int:
        dados = (int(self.dinheiro), self.vidas, len(self.bloons), len(self.torres),
                 int(sum(b.d for b in self.bloons)), self.pops_total, len(self.projeteis))
        return zlib.crc32(repr(dados).encode())


_AT_DOT = novo_ataque(dict(dano=1, dtype=NORMAL))


class Partida:
    """Uma partida: solo (1 pista) ou batalha (2 pistas)."""

    def __init__(self, modo: str, mapa: str, seed: int = 1, dificuldade: str = "medio",
                 herois: dict[int, str] | None = None):
        self.modo = modo
        self.mapa = MAPAS[mapa]
        self.seed = seed
        self.tick = 0
        self.tempo = 0.0
        self.rodada = 0
        self.em_rodada = False
        self.fim = False
        self.vencedor: int | None = None
        self.auto = False
        herois = herois or {}
        if modo == "solo":
            _, vidas, mult, ultima = DIFICULDADES[dificuldade]
            self.dificuldade = dificuldade
            self.ultima_rodada = ultima
            self.pistas = {1: Pista(1, self.mapa, seed, vidas, DINHEIRO_INICIAL, mult)}
        else:
            self.dificuldade = "medio"
            self.ultima_rodada = 10 ** 9
            self.pistas = {
                1: Pista(1, self.mapa, seed, VIDAS_BATALHA, DINHEIRO_INICIAL),
                2: Pista(2, self.mapa, seed, VIDAS_BATALHA, DINHEIRO_INICIAL),
            }
            self.pistas[1].oponente = self.pistas[2]
            self.pistas[2].oponente = self.pistas[1]
            self.prox_rodada_t = 3.0
            self.prox_eco_t = ECO_INTERVALO
            self.envio_rec: dict[tuple[int, str], float] = {}
        for j, h in herois.items():
            if j in self.pistas and h in HEROIS:
                self.pistas[j].heroi_escolhido = h
        self.ultimos_erros: dict[int, str] = {}

    # ------------------------------------------------------------ comandos
    def aplicar(self, jogador: int, cmd: str) -> str | None:
        """Aplica um comando textual (sem origem): T, U, V, M, B, S, N."""
        pista = self.pistas.get(jogador)
        if pista is None or self.fim or not cmd:
            return ERRO_INVALIDO
        c, corpo = cmd[0], cmd[1:]
        try:
            if c == "T":
                chave, xy = corpo.split("@")
                x, y = xy.split(",")
                erro = pista.colocar_torre(chave, int(x), int(y))
            elif c == "U":
                tid, p = corpo.split(":")
                erro = pista.upar(int(tid), int(p))
            elif c == "V":
                erro = pista.vender(int(corpo))
            elif c == "M":
                tid, m = corpo.split(":")
                erro = pista.mudar_modo(int(tid), int(m))
            elif c == "B":
                tid, i = corpo.split(":")
                erro = pista.usar_habilidade(int(tid), int(i))
            elif c == "S":
                erro = self.enviar(jogador, corpo)
            elif c == "N":
                erro = self.iniciar_rodada()
            else:
                erro = ERRO_INVALIDO
        except (ValueError, KeyError):
            erro = ERRO_INVALIDO
        if erro:
            self.ultimos_erros[jogador] = erro
        return erro

    def enviar(self, jogador: int, chave: str) -> str | None:
        if self.modo != "batalha":
            return ERRO_INVALIDO
        env = ENVIOS_POR_CHAVE.get(chave)
        if env is None:
            return ERRO_INVALIDO
        if self.rodada < env.rodada_min:
            return ERRO_BLOQUEADO
        if self.envio_rec.get((jogador, chave), 0.0) > self.tempo:
            return ERRO_BLOQUEADO
        pista = self.pistas[jogador]
        if pista.dinheiro < env.custo:
            return ERRO_DINHEIRO
        pista.dinheiro -= env.custo
        pista.eco = max(0.0, pista.eco + env.eco)
        self.envio_rec[(jogador, chave)] = self.tempo + 0.6
        alvo = pista.oponente
        for k in range(env.qtd):
            alvo.agendar(env.tipo, 0.3 + k * env.espaco, env.camo, env.regen, env.fort)
        pista.evento("envio", 0, 0, env.nome)
        return None

    def iniciar_rodada(self) -> str | None:
        if self.modo != "solo" or self.em_rodada or self.fim:
            return ERRO_INVALIDO
        self.rodada += 1
        self.em_rodada = True
        pista = self.pistas[1]
        for t, g in agenda_da_rodada(self.rodada):
            pista.agendar(g.tipo, t, g.camo, g.regen, g.fort)
        return None

    # ------------------------------------------------------------ passo
    def passo(self) -> None:
        if self.fim:
            return
        self.tick += 1
        self.tempo += DT
        for p in self.pistas.values():
            p.passo()
        if self.modo == "solo":
            self._passo_solo()
        else:
            self._passo_batalha()

    def _passo_solo(self) -> None:
        pista = self.pistas[1]
        if pista.vidas <= 0:
            self.fim, self.vencedor = True, 0
            return
        if self.em_rodada and not pista.fila and not pista.bloons:
            self.em_rodada = False
            pista.dinheiro += 100 + self.rodada
            pista.pagar_renda()
            pista._xp(20 + self.rodada * 2)
            pista.evento("fim_rodada", 0, 0, self.rodada)
            if self.rodada >= self.ultima_rodada:
                self.fim, self.vencedor = True, 1
                return
            if self.auto:
                self.iniciar_rodada()

    def _passo_batalha(self) -> None:
        if self.tempo >= self.prox_eco_t:
            self.prox_eco_t += ECO_INTERVALO
            for p in self.pistas.values():
                p.dinheiro += p.eco
                p.evento("eco", 0, 0, int(p.eco))
        if self.tempo >= self.prox_rodada_t:
            self.rodada += 1
            ag = agenda_da_rodada(self.rodada)
            for p in self.pistas.values():
                for t, g in ag:
                    p.agendar(g.tipo, t, g.camo, g.regen, g.fort)
                p.pagar_renda()
                p._xp(10 + self.rodada)
                p.evento("fim_rodada", 0, 0, self.rodada)
            self.prox_rodada_t = self.tempo + duracao_rodada(self.rodada) + PAUSA_ENTRE_RODADAS
        v1, v2 = self.pistas[1].vidas, self.pistas[2].vidas
        if v1 <= 0 or v2 <= 0:
            self.fim = True
            if v1 <= 0 and v2 <= 0:
                self.vencedor = 1 if v1 > v2 else 2 if v2 > v1 else 0
            else:
                self.vencedor = 2 if v1 <= 0 else 1

    def tempo_para_rodada(self) -> float:
        if self.modo != "batalha":
            return 0.0
        return max(0.0, self.prox_rodada_t - self.tempo)

    def tempo_para_eco(self) -> float:
        if self.modo != "batalha":
            return 0.0
        return max(0.0, self.prox_eco_t - self.tempo)

    def hash(self) -> int:
        h = self.tick
        for j in sorted(self.pistas):
            h = zlib.crc32(str(self.pistas[j].hash()).encode(), h)
        return h
