#include "cliente/modelos3d.hpp"

#include <cmath>
#include <fstream>
#include <map>
#include <vector>

#include "cliente/ui.hpp"
#include "jogo/defs.hpp"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

namespace bl::m3d {
namespace {

// Enquadramento do sprite: quadrado de LADO unidades, centrado em (0, ALTO, 0), com RES px de lado.
constexpr float LADO = 3.4f, ALTO = 0.5f;
constexpr int RES = 320;
constexpr int PASSOS = 24;  // direcoes de mira guardadas (15 graus cada)
// Vista do jogo dos modelos (tools/blender/poli.py, VISTAS["jogo"]), no sistema do .glb (Y para cima)
const Vector3 DIR_CAMERA{0.0f, 1.0f, 0.70f};

const char* VS = R"(#version 330
in vec3 vertexPosition;
in vec3 vertexNormal;
in vec4 vertexColor;
uniform mat4 mvp;
uniform mat4 matModel;
out vec4 cor;
out vec3 normal;
void main() {
    cor = vertexColor;
    normal = normalize(mat3(matModel) * vertexNormal);
    gl_Position = mvp * vec4(vertexPosition, 1.0);
})";

// Cor de vertice (linear no .glb) com luz difusa simples, parecida com o sol das folhas de revisao
const char* FS = R"(#version 330
in vec4 cor;
in vec3 normal;
out vec4 finalColor;
void main() {
    vec3 luz = normalize(vec3(0.35, 0.80, 0.50));
    float d = max(dot(normalize(normal), luz), 0.0);
    vec3 c = pow(cor.rgb, vec3(1.0 / 2.2));
    finalColor = vec4(c * (0.60 + 0.50 * d), 1.0);
})";

struct Modelo {
    Model model{};
    bool ok = false;
    int torreta = -1;  // malha que gira sozinha nas maquinas
    Vector3 pivo_torreta{};
};

std::string pasta;
bool procurou = false;
Shader shader{};
Material material{};
std::map<std::string, Modelo> modelos;
std::map<std::string, RenderTexture2D> sprites;

const std::string& achar_pasta() {
    if (procurou) return pasta;
    procurou = true;
    const std::string exe = GetApplicationDirectory();
    for (const std::string& base : {exe, exe + "../", exe + "../../", exe + "../../../", std::string()}) {
        const std::string p = base + "assets/modelos";
        if (DirectoryExists(p.c_str())) {
            pasta = p;
            break;
        }
    }
    if (!pasta.empty()) {
        shader = LoadShaderFromMemory(VS, FS);
        material = LoadMaterialDefault();
        material.shader = shader;
    }
    return pasta;
}

Modelo& carregar(const std::string& chave, const std::string& var) {
    const std::string k = chave + "/" + var;
    auto it = modelos.find(k);
    if (it != modelos.end()) return it->second;
    Modelo m;
    const std::string base = achar_pasta() + "/" + k;
    if (!pasta.empty() && FileExists((base + ".glb").c_str())) {
        m.model = LoadModel((base + ".glb").c_str());
        m.ok = m.model.meshCount > 0;
        std::ifstream f(base + ".json");
        if (m.ok && f) {
            const J j = J::parse(f, nullptr, false);
            if (!j.is_discarded() && j.contains("malhas")) {
                int i = 0;
                for (const J& malha : j["malhas"]) {
                    if (malha.value("grupo", std::string()) == "torreta" && malha.contains("pivo")) {
                        m.torreta = i;
                        const J& p = malha["pivo"];
                        m.pivo_torreta = {p[0].get<float>(), p[1].get<float>(), p[2].get<float>()};
                    }
                    ++i;
                }
            }
        }
    }
    return modelos[k] = m;
}

// Renderiza o modelo na vista do jogo, girado em 'giro' radianos em volta do eixo vertical
RenderTexture2D gerar(const Modelo& m, float giro, bool so_torreta) {
    rlDrawRenderBatchActive();
    rlDisableScissorTest();
    RenderTexture2D rt = LoadRenderTexture(RES, RES);
    BeginTextureMode(rt);
    ClearBackground(BLANK);
    Camera3D cam{};
    cam.target = {0, ALTO, 0};
    cam.position = Vector3Add(cam.target, Vector3Scale(Vector3Normalize(DIR_CAMERA), 12.0f));
    cam.up = {0, 1, 0};
    cam.fovy = LADO;
    cam.projection = CAMERA_ORTHOGRAPHIC;
    BeginMode3D(cam);
    const Matrix rot = MatrixRotateY(giro);
    for (int i = 0; i < m.model.meshCount; ++i) {
        Matrix t = MatrixIdentity();
        if (!so_torreta) {
            t = rot;
        } else if (i == m.torreta) {
            const Vector3 p = m.pivo_torreta;
            t = MatrixMultiply(MatrixMultiply(MatrixTranslate(-p.x, -p.y, -p.z), rot), MatrixTranslate(p.x, p.y, p.z));
        }
        DrawMesh(m.model.meshes[i], material, t);
    }
    EndMode3D();
    EndTextureMode();
    SetTextureFilter(rt.texture, TEXTURE_FILTER_BILINEAR);
    ui::restaurar_tela();
    return rt;
}

}  // namespace

bool disponivel() { return !achar_pasta().empty(); }

bool tem(const std::string& chave) { return disponivel() && carregar(chave, "0-0-0").ok; }

bool torre(const std::string& chave, const std::array<int, 3>& caminhos, float x, float y, float px, float ang,
           anim::TipoMira mira, unsigned char alfa) {
    if (!disponivel()) return false;
    std::string var = std::to_string(caminhos[0]) + "-" + std::to_string(caminhos[1]) + "-" + std::to_string(caminhos[2]);
    Modelo* m = &carregar(chave, var);
    if (!m->ok && var != "0-0-0") m = &carregar(chave, var = "0-0-0");
    if (!m->ok) return false;
    // o modelo olha para baixo na tela (90 graus); construcoes nao giram
    int passo = 0;
    if (mira != anim::TipoMira::FIXA) {
        const float voltas = (90.0f - ang) / 360.0f;
        passo = static_cast<int>(std::lround((voltas - std::floor(voltas)) * PASSOS)) % PASSOS;
    }
    const bool so_torreta = mira == anim::TipoMira::TORRETA && m->torreta >= 0;
    const std::string k = chave + "/" + var + "/" + std::to_string(passo);
    auto it = sprites.find(k);
    if (it == sprites.end()) {
        if (sprites.size() > 600) liberar();  // partidas longas: recomeca o cache em vez de crescer sem fim
        it = sprites.emplace(k, gerar(*m, passo * 2.0f * PI / PASSOS, so_torreta)).first;
    }
    // o pe do modelo (origem) fica ALTO unidades abaixo do centro do quadro, visto de cima
    const float lado = LADO * px;
    const float pe = ALTO * Vector3Normalize(DIR_CAMERA).z * px;
    const Rectangle src{0, 0, static_cast<float>(RES), -static_cast<float>(RES)};
    const Rectangle base{x - lado / 2, y - lado / 2 - pe, lado, lado};
    // contorno escuro, como nos sprites 2D: a silhueta em TINTA deslocada em volta
    const float e = std::max(1.0f, px * 0.03f);
    const Color tinta{20, 18, 24, alfa};
    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
            if (dx || dy) DrawTexturePro(it->second.texture, src, {base.x + dx * e, base.y + dy * e, lado, lado}, {0, 0}, 0, tinta);
    DrawTexturePro(it->second.texture, src, base, {0, 0}, 0, {255, 255, 255, alfa});
    return true;
}

void liberar() {
    for (auto& [k, rt] : sprites) UnloadRenderTexture(rt);
    sprites.clear();
}

}  // namespace bl::m3d
