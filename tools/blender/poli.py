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
    # cores das variacoes (entram depois das do piloto, que nao mudam)
    "aco": (168, 180, 198), "roxo": (132, 78, 196), "ciano": (90, 226, 240), "laranja": (245, 130, 40),
    "verde_escuro": (48, 120, 60), "rosa": (240, 120, 170),
    # Macaco de Gelo, a unica excecao de paleta: tons tirados de docs/design/capturas/btd6/real_ice_*.jpg
    "pelo_gelo": (130, 200, 236), "pele_gelo": (240, 250, 253), "pelo_gelo_escuro": (50, 157, 202),
    # herois (entram no fim para nao mudar o indice das cores acima): azul da marinha e o lilas de Psi
    "marinho": (36, 58, 110), "lilas": (176, 120, 210), "lilas_claro": (245, 225, 250), "lilas_escuro": (128, 78, 170),
}
INDICE = {nome: i for i, nome in enumerate(PALETA)}
CORES = list(PALETA.values())


def cena(nome):
    """Cena propria, para nao mexer no que ja esta aberto no Blender."""
    if bpy.context.window is None:  # sem interface: usa a cena que ja existe
        bpy.context.scene.name = nome
        return bpy.context.scene
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


def objeto(nome, bm, c, nivel=1, pos=(0, 0, 0), vivo=False):
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
    if vivo:  # pecas duras: quinas acima de 35 graus ficam vivas
        me.set_sharp_from_angle(angle=math.radians(35))
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
    col = me.color_attributes.get("Col")
    if col:  # sem cor ativa, a exportacao deixa a cor de fora
        me.color_attributes.active_color = col
        me.color_attributes.render_color_index = me.color_attributes.find("Col")
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


# ---------------------------------------------------------------- malha continua (bloco, fusao, retopologia)
from mathutils import Matrix  # noqa: E402


def bloco_esfera(bm, centro, raios):
    """Volume de bloco (so entra na fusao; nao vira peca final)."""
    if not isinstance(raios, (tuple, list)):
        raios = (raios, raios, raios)
    mat = Matrix.Translation(Vector(centro)) @ Matrix.Diagonal((raios[0], raios[1], raios[2], 1.0))
    bmesh.ops.create_uvsphere(bm, u_segments=24, v_segments=14, radius=1.0, matrix=mat)


def bloco_tubo(bm, pontos, raios):
    """Tubo de bloco ao longo dos pontos, com juntas redondas."""
    pts = [Vector(p) for p in pontos]
    for p, r in zip(pts, raios):
        bloco_esfera(bm, p, r)
    for i in range(len(pts) - 1):
        eixo = pts[i + 1] - pts[i]
        rot = Vector((0, 0, 1)).rotation_difference(eixo.normalized()).to_matrix().to_4x4()
        mat = Matrix.Translation((pts[i] + pts[i + 1]) / 2) @ rot
        bmesh.ops.create_cone(bm, cap_ends=True, segments=20, radius1=raios[i], radius2=raios[i + 1],
                              depth=eixo.length, matrix=mat)


def remalhar(nome, bm, c, faces, voxel=0.012, suave=6, simetria=True):
    """Funde os volumes numa casca so (voxel), alisa as juncoes e refaz a topologia em quads."""
    me = bpy.data.meshes.new(nome + "_bloco")
    bm.to_mesh(me)
    bm.free()
    o = bpy.data.objects.new(nome, me)
    c.collection.objects.link(o)
    m = o.modifiers.new("vox", "REMESH")
    m.mode = "VOXEL"
    m.voxel_size = voxel
    s = o.modifiers.new("alisar", "SMOOTH")
    s.factor = 0.5
    s.iterations = suave
    dg = bpy.context.evaluated_depsgraph_get()
    me2 = bpy.data.meshes.new_from_object(o.evaluated_get(dg))
    o.modifiers.clear()
    o.data = me2
    bpy.data.meshes.remove(me)
    for ob in bpy.context.view_layer.objects:
        ob.select_set(False)
    o.select_set(True)
    bpy.context.view_layer.objects.active = o
    bpy.ops.object.quadriflow_remesh(mode="FACES", target_faces=faces, use_mesh_symmetry=simetria,
                                     use_preserve_sharp=False, use_preserve_boundary=False, smooth_normals=True)
    o.data.name = nome
    for p in o.data.polygons:
        p.use_smooth = True
    o.data.materials.clear()
    o.data.materials.append(material())
    return o


def _cortar(bm, f):
    """Abre arestas ao longo do contorno f(p) = 0, para a borda da cor seguir uma linha limpa."""
    val = {v: f(v.co) for v in bm.verts}
    novos = set()
    for e in bm.edges[:]:
        a, b = e.verts
        fa, fb = val[a], val[b]
        if fa * fb < 0 and min(abs(fa), abs(fb)) > 1e-4:
            _, nv = bmesh.utils.edge_split(e, a, fa / (fa - fb))
            val[nv] = 0.0
            novos.add(nv)
    for face in bm.faces[:]:
        marc = [v for v in face.verts if v in novos]
        if len(marc) == 2 and not any(marc[1] in e.verts for e in marc[0].link_edges):
            try:
                bmesh.utils.face_split(face, marc[0], marc[1])
            except ValueError:
                pass


def colorir(o, regioes, base="pelo"):
    """Pinta por regiao. regioes = [(cor, f)], com f(p) < 0 dentro da mancha; vale a primeira que
    contem a face. A malha e cortada no contorno de cada mancha, entao a borda sai nitida."""
    me = o.data
    bm = bmesh.new()
    bm.from_mesh(me)
    for _, f in regioes:
        _cortar(bm, f)
    bm.faces.ensure_lookup_table()
    cores = []
    for face in bm.faces:
        p = face.calc_center_median()
        cores.append(next((cor for cor, f in regioes if f(p) < 0), base))
        face.smooth = True
    bm.to_mesh(me)
    bm.free()
    attr = me.color_attributes.get("Col")
    if attr:
        me.color_attributes.remove(attr)
    attr = me.color_attributes.new("Col", "BYTE_COLOR", "CORNER")
    for poly, cor in zip(me.polygons, cores):
        r, g, b = PALETA[cor]
        for li in poly.loop_indices:
            attr.data[li].color_srgb = (r / 255, g / 255, b / 255, 1.0)
    me.color_attributes.active_color = attr


def membro(pontos, raios, cor="pelo"):
    """Tubo de quads de secao quadrada (vira redondo na subdivisao). Raio perto de zero = ponta."""
    bm = bmesh.new()
    pts = [Vector(p) for p in pontos]
    n = len(pts)
    ref = Vector((1, 0, 0))
    aneis = []
    for i, p in enumerate(pts):
        t = (pts[min(i + 1, n - 1)] - pts[max(i - 1, 0)]).normalized()
        a = ref - t * ref.dot(t)
        if a.length < 1e-5:
            a = t.orthogonal()
        a.normalize()
        b = t.cross(a)
        ref = a
        aneis.append([bm.verts.new(p + (a * ca + b * cb) * raios[i]) for ca, cb in ((1, 1), (-1, 1), (-1, -1), (1, -1))])
    for i in range(n - 1):
        for k in range(4):
            bm.faces.new((aneis[i][k], aneis[i][(k + 1) % 4], aneis[i + 1][(k + 1) % 4], aneis[i + 1][k]))
    bm.faces.new(aneis[0][::-1])
    bm.faces.new(aneis[-1])
    for f in bm.faces:
        f.material_index = INDICE[cor]
    return bm


# ---------------------------------------------------------------- renders de revisao
def preparar_render(c, fundo=(0.12, 0.40, 0.05)):
    def pegar(nome, criar):
        o = c.objects.get(nome)
        if not o:
            o = bpy.data.objects.new(nome, criar())
            c.collection.objects.link(o)
        return o

    cam = pegar("bb_cam_" + c.name, lambda: bpy.data.cameras.new("bb_cam"))
    cam.data.type = "ORTHO"
    c.camera = cam
    sol = pegar("bb_sol_" + c.name, lambda: bpy.data.lights.new("bb_sol", "SUN"))
    sol.data.energy = 3.0
    sol.data.angle = math.radians(20)
    sol.rotation_euler = (math.radians(50), 0, math.radians(30))
    w = bpy.data.worlds.get("bb_mundo") or bpy.data.worlds.new("bb_mundo")
    w.use_nodes = True
    nt = w.node_tree
    bg = nt.nodes["Background"]
    if not nt.nodes.get("bb_mix"):
        lp = nt.nodes.new("ShaderNodeLightPath")
        mix = nt.nodes.new("ShaderNodeMixRGB")
        mix.name = "bb_mix"
        mix.inputs[1].default_value = (0.75, 0.75, 0.75, 1)
        nt.links.new(lp.outputs["Is Camera Ray"], mix.inputs[0])
        nt.links.new(mix.outputs[0], bg.inputs[0])
    nt.nodes["bb_mix"].inputs[2].default_value = (*fundo, 1)
    bg.inputs[1].default_value = 1.0
    c.world = w
    c.render.engine = "BLENDER_EEVEE"
    c.view_settings.view_transform = "Standard"
    c.render.image_settings.file_format = "PNG"
    return cam


VISTAS = {"frente": (0, -1, 0.12), "lado": (1, -0.02, 0.12), "tres_quartos": (0.6, -1, 0.35),
          "costas": (-0.5, 1, 0.3), "jogo": (0, -0.70, 1.0)}


def render(c, caminho, vista, alvo=(0, 0, 0.5), largura=1.5, altura=1.5, px=600):
    """Render ortografico. px = pixels por unidade (48 = tamanho da torre no mapa)."""
    cam = preparar_render(c)
    a = Vector(alvo)
    cam.location = a + Vector(VISTAS[vista]).normalized() * 8
    cam.rotation_euler = (a - cam.location).to_track_quat("-Z", "Y").to_euler()
    cam.data.ortho_scale = max(largura, altura)
    c.render.resolution_x = int(largura * px)
    c.render.resolution_y = int(altura * px)
    c.render.resolution_percentage = 100
    c.render.filepath = caminho
    bpy.ops.render.render(write_still=True)


# ---------------------------------------------------------------- pecas duras (maquinas, barcos, armas)
def caixa(tam, cor, chanfro=0.02, seg=2):
    """Caixa com as quinas chanfradas."""
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    for v in bm.verts:
        v.co = Vector((v.co.x * tam[0], v.co.y * tam[1], v.co.z * tam[2]))
    bmesh.ops.bevel(bm, geom=bm.edges[:], offset=chanfro, segments=seg, profile=0.5, affect="EDGES")
    for f in bm.faces:
        f.material_index = INDICE[cor]
    return bm


def torno(perfil, seg=20, cores=None, cor="cinza_escuro"):
    """Peca de revolucao em quads: perfil = [(raio, altura)] ao longo de Z. Raio 0 nas pontas fecha
    a peca (ponta viva). cores = uma cor por trecho do perfil."""
    bm = bmesh.new()
    aneis = []
    for r, h in perfil:
        if r < 1e-6:
            aneis.append([bm.verts.new((0, 0, h))])
        else:
            aneis.append([bm.verts.new((r * math.cos(2 * math.pi * k / seg), r * math.sin(2 * math.pi * k / seg), h))
                          for k in range(seg)])
    for i in range(len(perfil) - 1):
        a, b = aneis[i], aneis[i + 1]
        ci = INDICE[cores[i] if cores else cor]
        if len(a) == 1 and len(b) == 1:
            continue
        for k in range(seg):
            k2 = (k + 1) % seg
            if len(a) == 1:
                f = bm.faces.new((a[0], b[k], b[k2]))
            elif len(b) == 1:
                f = bm.faces.new((a[k], a[k2], b[0]))
            else:
                f = bm.faces.new((a[k], a[k2], b[k2], b[k]))
            f.material_index = ci
    return bm


def orientar(bm, direcao, origem=(0, 0, 0)):
    """Leva o eixo Z da peca para a direcao dada e a base para a origem."""
    rot = Vector((0, 0, 1)).rotation_difference(Vector(direcao).normalized()).to_matrix().to_4x4()
    bmesh.ops.transform(bm, matrix=Matrix.Translation(Vector(origem)) @ rot, verts=bm.verts[:])
    return bm


def loft(aneis, cores, fecha_inicio=True, fecha_fim=True):
    """Casca em quads ligando aneis fechados de pontos (todos com a mesma contagem).
    cores = uma cor por trecho do anel. As pontas fecham em leque."""
    bm = bmesh.new()
    vs = [[bm.verts.new(p) for p in anel] for anel in aneis]
    n = len(aneis[0])
    for i in range(len(vs) - 1):
        for k in range(n):
            k2 = (k + 1) % n
            f = bm.faces.new((vs[i][k], vs[i][k2], vs[i + 1][k2], vs[i + 1][k]))
            f.material_index = INDICE[cores[k]]
    for anel, fecha in ((vs[0], fecha_inicio), (vs[-1], fecha_fim)):
        if not fecha:
            continue
        centro = bm.verts.new(sum((v.co for v in anel), Vector()) / n)
        for k in range(n):
            f = bm.faces.new((anel[k], anel[(k + 1) % n], centro))
            f.material_index = INDICE[cores[k]]
    return bm


# ---------------------------------------------------------------- exportacao
ORDEM = ["base", "corpo", "cauda", "cabeca", "torreta", "braco"]


def exportar(c, chave, partes, pivos, raiz):
    """Grava assets/modelos/<chave>.glb e .json (grupo e pivo de cada malha) e a fonte .blend."""
    import json
    import os

    import comum

    objs = []
    for g in ORDEM:
        if g in partes:
            o = partes[g]
            o.name = o.data.name = f"{chave}_{len(objs)}_{g}"
            objs.append(o)
    bpy.context.view_layer.update()
    for ob in c.objects:
        ob.select_set(ob in objs)
    bpy.context.view_layer.objects.active = objs[0]
    pasta = os.path.join(raiz, "assets", "modelos")
    os.makedirs(os.path.join(pasta, "fonte"), exist_ok=True)
    glb = os.path.join(pasta, chave + ".glb")
    base = dict(filepath=glb, export_format="GLB", export_yup=True, export_apply=True, use_selection=True)
    for extra in (dict(export_vertex_color="ACTIVE", export_all_vertex_colors=False), dict(export_colors=True), dict()):
        try:
            # exporta so o que esta selecionado; rode pelo gerar_poli.py (sem interface, uma cena so)
            bpy.ops.export_scene.gltf(**base, **extra)
            break
        except TypeError:
            continue
    info = []
    for nome in comum.malhas_do_glb(glb):
        g = nome.rsplit("_", 1)[-1].split(".")[0]
        info.append({"grupo": g, "pivo": comum.para_gltf(pivos.get(g, (0, 0, 0)))})
    with open(os.path.join(pasta, chave + ".json"), "w", encoding="utf-8") as f:
        json.dump({"malhas": info}, f, indent=1)
    # so a cena do modelo vai para a fonte; o arquivo aberto no Blender nao e salvo nem alterado
    bpy.data.libraries.write(os.path.join(pasta, "fonte", chave + ".blend"), {c}, fake_user=True)
    return {"malhas": [i["grupo"] for i in info], "bytes": os.path.getsize(glb)}
