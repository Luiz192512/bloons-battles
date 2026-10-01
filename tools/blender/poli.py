"""Modelagem poligonal das torres: gaiola de quads (box modeling) mais subdivisao controlada.

Cada parte nasce de um cubo subdividido, ganha forma por extrusao e escala de faces, e so entao
recebe subdivisao. As cores entram como indice de material na gaiola (sobrevivem a subdivisao) e
viram cor de vertice no fim. Usado pelo conector do Blender e pelos scripts de geracao.
"""
import math

import bmesh
import bpy
from mathutils import Vector

# cores (sRGB 0..255); o indice e o "material" da face na gaiola
PALETA = {
    "pelo": (150, 96, 52), "pele": (240, 206, 160), "pelo_escuro": (108, 68, 36),
    "branco": (250, 248, 240), "tinta": (38, 28, 22), "azul": (60, 150, 230),
    "vermelho": (225, 60, 50), "amarelo": (255, 214, 50), "verde": (96, 196, 60),
    "marrom": (122, 78, 40), "marrom_escuro": (72, 44, 20), "bege": (238, 214, 160),
    "cinza": (120, 128, 140), "cinza_escuro": (58, 62, 72), "ouro": (240, 180, 40),
}
INDICE = {nome: i for i, nome in enumerate(PALETA)}
CORES = list(PALETA.values())


def cena(nome):
    """Cena propria, para nao mexer no que ja esta aberto no Blender."""
    c = bpy.data.scenes.get(nome) or bpy.data.scenes.new(nome)
    bpy.context.window.scene = c
    return c


def limpar_cena(c):
    for o in list(c.collection.all_objects):
        bpy.data.objects.remove(o, do_unlink=True)


def gaiola(raios, cortes=2, cor="pelo"):
    """Cubo subdividido e arredondado: a esfera de quads de onde sai cada parte."""
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    if cortes:
        bmesh.ops.subdivide_edges(bm, edges=bm.edges[:], cuts=cortes, use_grid_fill=True)
    for v in bm.verts:
        d = v.co.normalized()
        v.co = Vector((d.x * raios[0], d.y * raios[1], d.z * raios[2]))
    for f in bm.faces:
        f.material_index = INDICE[cor]
    bm.faces.ensure_lookup_table()
    return bm


def faces_em(bm, direcao, n=1):
    """As n faces cujo centro aponta mais para a direcao dada."""
    d = Vector(direcao).normalized()
    return sorted(bm.faces, key=lambda f: -f.calc_center_median().normalized().dot(d))[:n]


def extrudir(bm, faces, desloc=(0, 0, 0), escala=1.0, cor=None):
    """Extrusao de um grupo de faces, deslocando e escalando em torno do centro delas."""
    geo = bmesh.ops.extrude_face_region(bm, geom=faces)["geom"]
    novas = [g for g in geo if isinstance(g, bmesh.types.BMFace)]
    verts = list({v for f in novas for v in f.verts})
    centro = sum((v.co for v in verts), Vector()) / len(verts)
    if not isinstance(escala, (tuple, list)):
        escala = (escala, escala, escala)
    for v in verts:
        r = v.co - centro
        v.co = centro + Vector((r.x * escala[0], r.y * escala[1], r.z * escala[2])) + Vector(desloc)
    bmesh.ops.delete(bm, geom=[f for f in faces if f.is_valid], context="FACES_ONLY")
    if cor:
        for f in novas:
            f.material_index = INDICE[cor]
    bm.faces.ensure_lookup_table()
    return novas


def pintar(faces, cor):
    for f in faces:
        f.material_index = INDICE[cor]


def mover(bm, desloc):
    bmesh.ops.translate(bm, verts=bm.verts[:], vec=Vector(desloc))


def material():
    m = bpy.data.materials.get("bb_tinta")
    if m:
        return m
    m = bpy.data.materials.new("bb_tinta")
    m.use_nodes = True
    bsdf = m.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Roughness"].default_value = 0.6
    attr = m.node_tree.nodes.new("ShaderNodeVertexColor")
    attr.layer_name = "Col"
    m.node_tree.links.new(attr.outputs["Color"], bsdf.inputs["Base Color"])
    return m


def objeto(nome, bm, c, nivel=1, pos=(0, 0, 0)):
    """Fecha a gaiola: recalcula normais, subdivide 'nivel' vezes e grava a cor por vertice."""
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces[:])
    if nivel:
        # subdivisao de Catmull-Clark aplicada na malha (e a contagem que vai para o jogo)
        me_tmp = bpy.data.meshes.new(nome + "_gaiola")
        bm.to_mesh(me_tmp)
        o_tmp = bpy.data.objects.new(nome + "_gaiola", me_tmp)
        c.collection.objects.link(o_tmp)
        mod = o_tmp.modifiers.new("sub", "SUBSURF")
        mod.levels = nivel
        dg = bpy.context.evaluated_depsgraph_get()
        me = bpy.data.meshes.new_from_object(o_tmp.evaluated_get(dg))
        bpy.data.objects.remove(o_tmp, do_unlink=True)
        bpy.data.meshes.remove(me_tmp)
    else:
        me = bpy.data.meshes.new(nome)
        bm.to_mesh(me)
    bm.free()
    me.name = nome
    attr = me.color_attributes.new("Col", "BYTE_COLOR", "CORNER")
    for poly in me.polygons:
        r, g, b = CORES[poly.material_index]
        for li in poly.loop_indices:
            attr.data[li].color_srgb = (r / 255, g / 255, b / 255, 1.0)
        poly.use_smooth = True
        poly.material_index = 0
    me.materials.clear()
    me.materials.append(material())
    o = bpy.data.objects.new(nome, me)
    o.location = pos
    c.collection.objects.link(o)
    return o


def juntar(nome, objs, c):
    """Junta varias malhas numa so (uma parte movel = uma malha no jogo)."""
    bm = bmesh.new()
    for o in objs:
        tmp = o.data.copy()
        tmp.transform(o.matrix_world)
        bm.from_mesh(tmp)
        bpy.data.meshes.remove(tmp)
    me = bpy.data.meshes.new(nome)
    bm.to_mesh(me)
    bm.free()
    for p in me.polygons:
        p.use_smooth = True
    me.materials.append(material())
    for o in objs:
        bpy.data.objects.remove(o, do_unlink=True)
    novo = bpy.data.objects.new(nome, me)
    c.collection.objects.link(novo)
    return novo


def triangulos(objs):
    return sum(len(p.vertices) - 2 for o in objs for p in o.data.polygons)


def quads_pct(objs):
    faces = [p for o in objs for p in o.data.polygons]
    return round(100 * sum(len(p.vertices) == 4 for p in faces) / max(1, len(faces)))


# ---------------------------------------------------------------- pecas do macaco padrao
def cabeca_gaiola():
    bm = gaiola((0.235, 0.215, 0.215))
    # mascara do rosto: a fileira do meio e o centro de baixo da frente
    frente = faces_em(bm, (0, -1, 0), 9)
    for f in frente:
        cz = f.calc_center_median()
        if (abs(cz.x) < 0.05 and cz.z < 0.06) or (abs(cz.z) < 0.06):
            f.material_index = INDICE["pele"]
    # focinho: o centro e o centro de baixo saem para a frente
    foc = [f for f in frente if abs(f.calc_center_median().x) < 0.05 and f.calc_center_median().z < 0.06]
    extrudir(bm, foc, desloc=(0, -0.055, -0.01), escala=(0.80, 1.0, 0.72), cor="pele")
    # orelhas: a face do meio de cada lado sai, abre em disco e afunda no centro
    for sx in (-1, 1):
        lado = faces_em(bm, (sx, 0, 0.15), 1)
        aba = extrudir(bm, lado, desloc=(0.035 * sx, 0, 0.02), escala=(1.0, 0.75, 1.05))
        aba = extrudir(bm, aba, desloc=(0.040 * sx, 0, 0), escala=(1.0, 1.75, 1.55))
        aba = extrudir(bm, aba, desloc=(0.012 * sx, -0.01, 0), escala=(1.0, 0.70, 0.70), cor="pele")
        extrudir(bm, aba, desloc=(-0.016 * sx, 0, 0), escala=0.85, cor="pele")
    mover(bm, (0, -0.01, 0.74))
    return bm


def olho_gaiola(sx):
    bm = gaiola((0.050, 0.030, 0.060), cortes=1, cor="branco")
    pup = faces_em(bm, (-0.25 * sx, -1, -0.1), 1)
    extrudir(bm, pup, desloc=(0, -0.004, 0), escala=0.62, cor="tinta")
    mover(bm, (0.078 * sx, -0.196, 0.790))
    return bm


def tronco_gaiola():
    bm = gaiola((0.185, 0.165, 0.205))
    for f in faces_em(bm, (0, -1, -0.1), 9):
        cz = f.calc_center_median()
        if abs(cz.x) < 0.05 and cz.z < 0.07:
            f.material_index = INDICE["pele"]
    mover(bm, (0, 0, 0.38))
    return bm
