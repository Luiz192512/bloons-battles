"""Pecas arredondadas, cores e exportacao dos modelos 3D do jogo (Blender 4.x, sem interface).

Convencoes:
- Unidade: o macaco padrao tem 1,0 de altura. O modelo fica em pe na origem, com Z para cima,
  olhando para -Y (a frente do Blender).
- So formas redondas: esfera, capsula, toro e cubo com cantos bem arredondados.
- Cor chapada por peca, gravada como cor de vertice (o jogo nao usa textura).
- Cada peca pertence a um grupo animavel (base, corpo, cabeca, braco, arma, torreta, cauda).
  Na exportacao, as pecas de um grupo viram uma malha so.
"""
import json
import math
import struct

import bpy
from mathutils import Vector

# Paleta do projeto (src/cliente/ui.hpp) e tons de apoio
TINTA = (38, 28, 22)
BRANCO = (250, 248, 240)
AMARELO = (255, 214, 50)
VERDE = (96, 196, 60)
VERMELHO = (225, 60, 50)
AZUL = (60, 150, 230)
MARROM = (122, 78, 40)
MARROM_ESCURO = (72, 44, 20)
BEGE = (238, 214, 160)
CINZA = (120, 128, 140)
CINZA_ESCURO = (58, 62, 72)
OURO = (240, 180, 40)


def limpar():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def _material():
    """Um material so, que le a cor do vertice (serve para a previa e para o .glb)."""
    m = bpy.data.materials.get("tinta")
    if m:
        return m
    m = bpy.data.materials.new("tinta")
    m.use_nodes = True
    nt = m.node_tree
    bsdf = nt.nodes["Principled BSDF"]
    bsdf.inputs["Roughness"].default_value = 0.55
    attr = nt.nodes.new("ShaderNodeVertexColor")
    attr.layer_name = "Col"
    nt.links.new(attr.outputs["Color"], bsdf.inputs["Base Color"])
    return m


def _registrar(obj, nome, cor, grupo):
    obj.name = nome
    obj["cor"] = list(cor)
    obj["grupo"] = grupo
    for p in obj.data.polygons:
        p.use_smooth = True
    obj.data.materials.append(_material())
    return obj


def esfera(nome, pos, escala, cor, grupo, rot=(0, 0, 0)):
    """Esfera ou elipsoide. escala = raio (numero) ou (rx, ry, rz); rot em graus."""
    if isinstance(escala, (int, float)):
        escala = (escala, escala, escala)
    bpy.ops.mesh.primitive_uv_sphere_add(segments=32, ring_count=16, radius=1, location=pos)
    o = bpy.context.object
    o.scale = escala
    o.rotation_euler = [math.radians(a) for a in rot]
    return _registrar(o, nome, cor, grupo)


def capsula(nome, p1, p2, raio, cor, grupo, raio2=None):
    """Capsula de p1 a p2. Com raio2, afina ate a ponta p2 (ponta sempre arredondada)."""
    p1, p2 = Vector(p1), Vector(p2)
    eixo = p2 - p1
    comp = eixo.length
    bpy.ops.mesh.primitive_uv_sphere_add(segments=24, ring_count=12, radius=1, location=(0, 0, 0))
    o = bpy.context.object
    r2 = raio if raio2 is None else raio2
    for v in o.data.vertices:
        if v.co.z > 1e-6:
            v.co.x *= r2
            v.co.y *= r2
            v.co.z = v.co.z * r2 + comp / 2
        else:
            v.co.x *= raio
            v.co.y *= raio
            v.co.z = v.co.z * raio - comp / 2
    o.rotation_mode = "QUATERNION"
    o.rotation_quaternion = Vector((0, 0, 1)).rotation_difference(eixo.normalized())
    o.location = (p1 + p2) / 2
    return _registrar(o, nome, cor, grupo)


def toro(nome, pos, raio, espessura, cor, grupo, rot=(0, 0, 0)):
    bpy.ops.mesh.primitive_torus_add(
        major_radius=raio, minor_radius=espessura, major_segments=40, minor_segments=14,
        location=pos, rotation=[math.radians(a) for a in rot])
    return _registrar(bpy.context.object, nome, cor, grupo)


def cubo(nome, pos, tamanho, cor, grupo, canto=None, rot=(0, 0, 0)):
    """Cubo com cantos arredondados. tamanho = (x, y, z); canto = raio do arredondamento."""
    bpy.ops.mesh.primitive_cube_add(size=1, location=pos)
    o = bpy.context.object
    o.scale = tamanho
    bpy.ops.object.transform_apply(scale=True)
    o.rotation_euler = [math.radians(a) for a in rot]
    if canto is None:
        canto = min(tamanho) * 0.45
    mod = o.modifiers.new("canto", "BEVEL")
    mod.width = canto
    mod.segments = 8
    mod.limit_method = "NONE"
    return _registrar(o, nome, cor, grupo)


def arco(nome, centro, raio, ang0, ang1, espessura, cor, grupo, plano="XZ", n=7, afina=1.0):
    """Curva feita de capsulas (cauda, alca, corda). Angulos em graus no plano dado."""
    pts = []
    for i in range(n + 1):
        a = math.radians(ang0 + (ang1 - ang0) * i / n)
        c, s = math.cos(a) * raio, math.sin(a) * raio
        d = {"XZ": (c, 0, s), "YZ": (0, c, s), "XY": (c, s, 0)}[plano]
        pts.append(Vector(centro) + Vector(d))
    pecas = []
    for i in range(n):
        r_a = espessura * (1 + (afina - 1) * i / n)
        r_b = espessura * (1 + (afina - 1) * (i + 1) / n)
        pecas.append(capsula(f"{nome}_{i}", pts[i], pts[i + 1], r_a, cor, grupo, raio2=r_b))
    return pecas


def _pintar(obj):
    r, g, b = obj["cor"]
    attr = obj.data.color_attributes.get("Col") or obj.data.color_attributes.new("Col", "BYTE_COLOR", "CORNER")
    for d in attr.data:
        d.color_srgb = (r / 255, g / 255, b / 255, 1.0)
    obj.data.color_attributes.active_color = attr
    obj.data.color_attributes.render_color_index = obj.data.color_attributes.find("Col")


def juntar_grupos(ordem):
    """Aplica modificadores, pinta e junta as pecas de cada grupo. Devolve os grupos que existem."""
    feitos = []
    for g in ordem:
        objs = [o for o in bpy.context.scene.objects if o.type == "MESH" and o.get("grupo") == g]
        if not objs:
            continue
        bpy.ops.object.select_all(action="DESELECT")
        for o in objs:
            o.select_set(True)
        bpy.context.view_layer.objects.active = objs[0]
        bpy.ops.object.convert(target="MESH")
        for o in objs:
            _pintar(o)
        if len(objs) > 1:
            bpy.ops.object.join()
        junto = bpy.context.view_layer.objects.active
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        junto.name = f"{len(feitos)}_{g}"
        junto.data.name = junto.name
        feitos.append(g)
    return feitos


def exportar_glb(caminho):
    bpy.ops.object.select_all(action="SELECT")
    base = dict(filepath=caminho, export_format="GLB", export_yup=True, export_apply=True)
    for extra in (dict(export_vertex_color="ACTIVE", export_all_vertex_colors=False),
                  dict(export_colors=True), dict()):
        try:
            bpy.ops.export_scene.gltf(**base, **extra)
            return
        except TypeError:
            continue


def malhas_do_glb(caminho):
    """Nomes das malhas na ordem dos nos do arquivo (a ordem em que a raylib carrega)."""
    with open(caminho, "rb") as f:
        f.read(12)
        tam, _tipo = struct.unpack("<II", f.read(8))
        doc = json.loads(f.read(tam))
    nomes = []
    for no in doc.get("nodes", []):
        if "mesh" in no:
            m = doc["meshes"][no["mesh"]]
            nomes += [m.get("name", "")] * len(m["primitives"])
    return nomes


def para_gltf(v):
    """Ponto do Blender (Z para cima) no sistema do .glb (Y para cima)."""
    return [round(v[0], 4), round(v[2], 4), round(-v[1], 4)]


def previa(caminho, alvo=(0, 0, 0.5), distancia=3.2, fundo=(98, 170, 58)):
    """Imagem na vista 3/4 parecida com a do jogo, sobre um verde de grama."""
    cena = bpy.context.scene
    cam = bpy.data.objects.new("camera", bpy.data.cameras.new("camera"))
    cena.collection.objects.link(cam)
    direcao = Vector((0.55, -1.0, 0.85)).normalized()
    cam.location = Vector(alvo) + direcao * distancia
    cam.rotation_euler = (Vector(alvo) - cam.location).to_track_quat("-Z", "Y").to_euler()
    cam.data.lens = 60
    cena.camera = cam

    sol = bpy.data.objects.new("sol", bpy.data.lights.new("sol", "SUN"))
    sol.data.energy = 3.2
    sol.data.angle = math.radians(12)
    sol.rotation_euler = (math.radians(48), math.radians(8), math.radians(32))
    cena.collection.objects.link(sol)

    mundo = bpy.data.worlds.new("mundo")
    mundo.use_nodes = True
    bg = mundo.node_tree.nodes["Background"]
    bg.inputs["Color"].default_value = tuple((c / 255) ** 2.2 for c in fundo) + (1.0,)
    bg.inputs["Strength"].default_value = 1.0
    cena.world = mundo

    cena.render.resolution_x = cena.render.resolution_y = 720
    cena.render.filepath = caminho
    cena.view_settings.view_transform = "Standard"
    for motor in ("BLENDER_EEVEE_NEXT", "BLENDER_EEVEE", "CYCLES"):
        try:
            cena.render.engine = motor
            if motor == "CYCLES":
                cena.cycles.samples = 48
            bpy.ops.render.render(write_still=True)
            return motor
        except Exception as e:  # sem GPU no modo sem interface: tenta o proximo motor
            print("previa: falhou com", motor, e)
    return None
