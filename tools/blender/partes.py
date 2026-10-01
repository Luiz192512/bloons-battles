"""Montador de variacoes por partes: cada torre declara pecas por caminho e tier, e as 64
combinacoes de upgrade saem da composicao dessas pecas em encaixes.

Uma torre e um modulo em tools/blender/torres/<chave>.py com:
    base(m)      as pecas da variante 0-0-0
    CAMINHOS     lista de 3 dicionarios {tier: peca}
    ENQUADRE     (alvo, tamanho) das folhas de revisao
A peca dos tiers 1 e 2 e um acessorio: uma lista de alternativas [(encaixe, funcao), ...].
Um acessorio (None, None) nao ocupa encaixe: o tier so muda a arma ou outra peca que le m.tier.
A peca dos tiers 3 a 5 e um conjunto: uma funcao que troca traje, arma e silhueta.

Regras de composicao (montar):
- o caminho principal e o de tier mais alto (em empate, o de menor indice); os tiers dele sao
  aplicados em ordem, do 1 ate o atingido (cumulativos);
- o caminho cruzado so chega aos tiers 1 e 2; cada tier soma um acessorio, na primeira
  alternativa cujo encaixe o principal nao ocupou;
- as funcoes de peca podem ler m.tier para ajustar a propria peca (por exemplo a ponta do dardo).
"""
import importlib
import json
import os
import struct

import bpy
from mathutils import Matrix, Vector

import comum
import macaco_poli
import poli

ENCAIXES = ["pelagem", "chapeu", "rosto", "tronco", "costas", "mao_ataque", "mao_livre", "pes", "base", "torreta", "extra"]
# malha (grupo animavel) em que cada encaixe entra, quando a peca nao diz outra
GRUPO = {"pelagem": "cabeca", "chapeu": "cabeca", "rosto": "cabeca", "tronco": "corpo", "costas": "corpo",
         "mao_ataque": "braco", "mao_livre": "corpo", "pes": "corpo", "base": "base", "torreta": "torreta", "extra": "base"}
# encaixes presos ao macaco: acompanham quando o macaco e movido ou escalado
DO_MACACO = {"pelagem", "chapeu", "rosto", "tronco", "costas", "mao_ataque", "mao_livre", "pes"}
TETO_TRIS = 10000
TETO_MALHAS = 6

_cache_macaco = {}


def combinacoes():
    """As 64 combinacoes validas: um caminho ate 5, outro ate 2 e o terceiro em 0."""
    todas = []
    for a in range(6):
        for b in range(6):
            for c3 in range(6):
                t = sorted((a, b, c3))
                if t[0] == 0 and t[1] <= 2:
                    todas.append((a, b, c3))
    return todas


def principal(tier):
    return max(range(3), key=lambda i: (tier[i], -i))


def carregar(chave):
    return importlib.import_module("torres." + chave)


def _macaco_malhas(c, pelo, pele):
    """Malhas do corpo padrao, feitas uma vez por execucao (a retopologia e a parte lenta)."""
    k = (pelo, pele)
    if k not in _cache_macaco:
        objs = {"corpo": macaco_poli._corpo(c, pelo, pele), "braco": macaco_poli._braco(c, pelo, pele),
                "cauda": macaco_poli._cauda(c, pelo),
                "cabeca": poli.juntar("cabeca", [macaco_poli._cabeca(c, pelo, pele)] + macaco_poli._rosto(c), c)}
        _cache_macaco[k] = {}
        for g, o in objs.items():
            me = o.data
            me.use_fake_user = True
            bpy.data.objects.remove(o, do_unlink=True)
            _cache_macaco[k][g] = me
    return _cache_macaco[k]


class Montagem:
    """O estado de uma variacao em montagem: as malhas fixas do macaco e as pecas de cada encaixe."""

    def __init__(self, c, chave, tier):
        self.c, self.chave, self.tier = c, chave, tuple(tier)
        self.encaixes = {e: [] for e in ENCAIXES}   # encaixe -> [(grupo, objeto, preso)]
        self.fixas = {}                             # grupo -> malha do corpo padrao
        self.pivos = {}
        self.ocupados = set()
        self.matriz = Matrix.Identity(4)            # onde o macaco fica (barco, atras da catapulta)
        self.cores = {"pelo": "pelo", "pele": "pele", "pelo_escuro": "pelo_escuro"}
        self._principal = False

    # ---- macaco padrao
    def macaco(self, pelagem="topete", pelo="pelo", pele="pele", pelo_escuro="pelo_escuro"):
        self.cores = {"pelo": pelo, "pele": pele, "pelo_escuro": pelo_escuro}
        for g, me in _macaco_malhas(self.c, pelo, pele).items():
            o = bpy.data.objects.new(g, me.copy())
            self.c.collection.objects.link(o)
            self.fixas[g] = o
        self.pivos.update(macaco_poli.PIVOS)
        self.pelagem(pelagem)

    def pelagem(self, estilo):
        antes = self._principal
        self._principal = False   # trocar o pelo nao ocupa o encaixe
        self.por("pelagem", macaco_poli._pelagem(self.c, estilo, self.cores["pelo_escuro"]))
        self._principal = antes

    def mover_macaco(self, matriz):
        self.matriz = matriz @ self.matriz

    # ---- pecas
    def obj(self, nome, bm, nivel=1, vivo=False, matriz=None):
        o = poli.objeto(nome, bm, self.c, nivel=nivel, vivo=vivo)
        if matriz is not None:
            o.data.transform(matriz)
        return o

    def _lista(self, objs):
        return list(objs) if isinstance(objs, (list, tuple)) else [objs]

    def somar(self, encaixe, objs, grupo=None, preso=None):
        """Acrescenta pecas ao encaixe."""
        g = grupo or GRUPO[encaixe]
        p = (encaixe in DO_MACACO) if preso is None else preso
        self.encaixes[encaixe] += [(g, o, p) for o in self._lista(objs)]
        if self._principal and encaixe != "extra":   # extra aceita varias pecas: nunca fica ocupado
            self.ocupados.add(encaixe)

    def por(self, encaixe, objs, grupo=None, preso=None):
        """Troca o que estiver no encaixe pelas pecas dadas."""
        self.tirar(encaixe)
        self.somar(encaixe, objs, grupo, preso)

    def tirar(self, encaixe, prefixo=None):
        """Esvazia o encaixe (ou so as pecas cujo nome comeca com o prefixo)."""
        fica = []
        for g, o, p in self.encaixes[encaixe]:
            if prefixo and not o.name.startswith(prefixo):
                fica.append((g, o, p))
                continue
            me = o.data
            bpy.data.objects.remove(o, do_unlink=True)
            if me.users == 0:
                bpy.data.meshes.remove(me)
        self.encaixes[encaixe] = fica
        if self._principal and encaixe != "extra":   # extra aceita varias pecas: nunca fica ocupado
            self.ocupados.add(encaixe)

    def ocupar(self, *encaixes):
        self.ocupados.update(encaixes)

    def tem(self, encaixe):
        return bool(self.encaixes[encaixe])

    def pivo(self, grupo, ponto):
        self.pivos[grupo] = tuple(ponto)

    # ---- fecho
    def fechar(self):
        """Junta tudo em ate 6 malhas (uma por grupo) e devolve (partes, pivos)."""
        partes, pivos = {}, {}
        for g in poli.ORDEM:
            objs = []
            if g in self.fixas:
                self.fixas[g].data.transform(self.matriz)
                objs.append(self.fixas[g])
            for e in ENCAIXES:
                for gr, o, preso in self.encaixes[e]:
                    if gr == g:
                        if preso:
                            o.data.transform(self.matriz)
                        objs.append(o)
            if objs:
                partes[g] = poli.juntar(g, objs, self.c)
                p = Vector(self.pivos.get(g, (0, 0, 0)))
                pivos[g] = tuple(self.matriz @ p) if g in self.fixas else tuple(p)
        return partes, pivos


def _aplicar(m, peca, cruzado):
    if callable(peca):
        if cruzado:
            raise ValueError("caminho cruzado so aceita acessorio (lista de alternativas)")
        peca(m)
        return None
    for encaixe, f in peca:
        if encaixe is None:   # o tier so muda um parametro que as pecas da torre leem em m.tier
            if f:
                f(m)
            return None
        if not cruzado or encaixe not in m.ocupados:
            f(m)
            return encaixe
    raise ValueError(f"{m.chave} {m.tier}: acessorio sem encaixe livre")


def montar(c, chave, a, b, c3):
    """Monta a variacao a-b-c3 da torre na cena e devolve (partes, pivos)."""
    tier = (a, b, c3)
    if tier not in combinacoes():
        raise ValueError(f"combinacao invalida: {a}-{b}-{c3}")
    torre = carregar(chave)
    m = Montagem(c, chave, tier)
    torre.base(m)
    p = principal(tier)
    m._principal = True
    for t in range(1, tier[p] + 1):
        _aplicar(m, torre.CAMINHOS[p][t], cruzado=False)
    m._principal = False
    for q in range(3):
        if q != p:
            for t in range(1, tier[q] + 1):
                _aplicar(m, torre.CAMINHOS[q][t], cruzado=True)
    return m.fechar()


def limpar(c):
    """Apaga a variacao anterior (o cache do macaco fica)."""
    for o in list(c.collection.all_objects):
        bpy.data.objects.remove(o, do_unlink=True)
    for me in list(bpy.data.meshes):
        if me.users == 0 and not me.use_fake_user:
            bpy.data.meshes.remove(me)


def medir_glb(caminho):
    """(malhas, triangulos) lidos do arquivo: e a conta que vale para o orcamento."""
    with open(caminho, "rb") as f:
        f.read(12)
        tam, _tipo = struct.unpack("<II", f.read(8))
        doc = json.loads(f.read(tam))
    malhas = tris = 0
    for me in doc.get("meshes", []):
        for pr in me["primitives"]:
            malhas += 1
            n = doc["accessors"][pr["indices"]]["count"] if "indices" in pr else doc["accessors"][pr["attributes"]["POSITION"]]["count"]
            tris += n // 3
    tem_cor = all("COLOR_0" in pr["attributes"] for me in doc.get("meshes", []) for pr in me["primitives"])
    return malhas, tris, tem_cor


def exportar(c, chave, rotulo, partes, pivos, raiz):
    """Grava assets/modelos/<chave>/<rotulo>.glb e .json (grupo e pivo de cada malha)."""
    objs = []
    for g in poli.ORDEM:
        if g in partes:
            o = partes[g]
            o.name = o.data.name = f"{chave}_{len(objs)}_{g}"
            objs.append(o)
    bpy.context.view_layer.update()
    for ob in c.objects:
        ob.select_set(ob in objs)
    bpy.context.view_layer.objects.active = objs[0]
    pasta = os.path.join(raiz, "assets", "modelos", chave)
    os.makedirs(pasta, exist_ok=True)
    glb = os.path.join(pasta, rotulo + ".glb")
    bpy.ops.export_scene.gltf(filepath=glb, export_format="GLB", export_yup=True, export_apply=True, use_selection=True,
                              export_vertex_color="ACTIVE", export_all_vertex_colors=False)
    info = []
    for nome in comum.malhas_do_glb(glb):
        g = nome.rsplit("_", 1)[-1].split(".")[0]
        info.append({"grupo": g, "pivo": comum.para_gltf(pivos.get(g, (0, 0, 0)))})
    with open(os.path.join(pasta, rotulo + ".json"), "w", encoding="utf-8") as f:
        json.dump({"malhas": info}, f, indent=1)
    malhas, tris, tem_cor = medir_glb(glb)
    return {"malhas": malhas, "grupos": [i["grupo"] for i in info], "tris": tris, "cor": tem_cor, "bytes": os.path.getsize(glb)}
