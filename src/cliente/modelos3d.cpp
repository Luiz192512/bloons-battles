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
    // grupo animavel e pivo de cada malha, na ordem do .glb
    std::vector<std::string> grupos;
    std::vector<Vector3> pivos;
};

std::string pasta;
bool procurou = false;
Shader shader{};
Material material{};
std::map<std::string, Modelo> modelos;
std::map<std::string, RenderTexture2D> sprites;
RenderTexture2D quadro_vivo{};  // onde sai a torre no meio de uma animacao (um desenho por vez)
std::map<std::string, Texture2D> retratos;  // id 0 = arquivo nao existe

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
                    Vector3 pv{};
                    if (malha.contains("pivo")) {
                        const J& p = malha["pivo"];
                        pv = {p[0].get<float>(), p[1].get<float>(), p[2].get<float>()};
                    }
                    m.grupos.push_back(malha.value("grupo", std::string()));
                    m.pivos.push_back(pv);
                    if (m.grupos.back() == "torreta" && malha.contains("pivo")) {
                        m.torreta = i;
                        m.pivo_torreta = pv;
                    }
                    ++i;
                }
            }
        }
    }
    return modelos[k] = m;
}

// Movimento de uma malha dentro do modelo (antes do giro da mira), para o instante do clipe.
// O modelo olha para +Z: girar em X sobe e desce o braco; andar em Z avanca e recua.
Matrix pose_da_malha(const Modelo& m, int i, const spr::Pose* pose) {
    if (!pose || !pose->ativa || i >= static_cast<int>(m.grupos.size())) return MatrixIdentity();
    const std::string& g = m.grupos[static_cast<size_t>(i)];
    const Vector3 p = m.pivos[static_cast<size_t>(i)];
    auto em_volta = [&](float graus) {
        return MatrixMultiply(MatrixMultiply(MatrixTranslate(-p.x, -p.y, -p.z), MatrixRotateX(graus * DEG2RAD)),
                              MatrixTranslate(p.x, p.y, p.z));
    };
    if (g == "braco")  // o mesmo sinal do clipe 2D: positivo arma para tras, negativo golpeia para a frente e para cima
        return MatrixMultiply(em_volta(pose->giro * 2.4f), MatrixTranslate(0, 0, pose->estica * 0.03f - pose->recuo * 0.08f));
    if (g == "cabeca") return em_volta(pose->giro * -0.25f);
    if (g == "torreta") return MatrixTranslate(0, 0, -pose->recuo * 0.24f);
    return MatrixIdentity();
}

// Renderiza o modelo na vista do jogo, girado em 'giro' radianos em volta do eixo vertical
void desenhar_em(RenderTexture2D rt, const Modelo& m, float giro, bool so_torreta, const spr::Pose* pose) {
    rlDrawRenderBatchActive();
    rlDisableScissorTest();
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
        Matrix t = pose_da_malha(m, i, pose);
        if (!so_torreta) {
            t = MatrixMultiply(t, rot);
        } else if (i == m.torreta) {
            const Vector3 p = m.pivo_torreta;
            t = MatrixMultiply(MatrixMultiply(MatrixMultiply(MatrixTranslate(-p.x, -p.y, -p.z), t), rot), MatrixTranslate(p.x, p.y, p.z));
        } else {
            t = MatrixIdentity();
        }
        DrawMesh(m.model.meshes[i], material, t);
    }
    EndMode3D();
    EndTextureMode();
    ui::restaurar_tela();
}

RenderTexture2D gerar(const Modelo& m, float giro, bool so_torreta) {
    RenderTexture2D rt = LoadRenderTexture(RES, RES);
    desenhar_em(rt, m, giro, so_torreta, nullptr);
    SetTextureFilter(rt.texture, TEXTURE_FILTER_BILINEAR);
    return rt;
}

}  // namespace

bool disponivel() { return !achar_pasta().empty(); }

bool tem(const std::string& chave) { return disponivel() && carregar(chave, "0-0-0").ok; }

bool torre(const std::string& chave, const std::array<int, 3>& caminhos, float x, float y, float px, float ang,
           anim::TipoMira mira, unsigned char alfa, const spr::Pose* pose) {
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
    // no meio de um clipe a torre e desenhada na hora, com a pose; parada, sai do sprite guardado
    const bool viva = pose && pose->ativa &&
                      (std::fabs(pose->giro) > 0.5f || std::fabs(pose->estica) > 0.3f || pose->recuo > 0.01f);
    Texture2D tex{};
    if (viva) {
        if (!quadro_vivo.id) {
            quadro_vivo = LoadRenderTexture(RES, RES);
            SetTextureFilter(quadro_vivo.texture, TEXTURE_FILTER_BILINEAR);
        }
        desenhar_em(quadro_vivo, *m, passo * 2.0f * PI / PASSOS, so_torreta, pose);
        tex = quadro_vivo.texture;
    } else {
        auto it = sprites.find(k);
        if (it == sprites.end()) {
            if (sprites.size() > 600) liberar();  // partidas longas: recomeca o cache em vez de crescer sem fim
            it = sprites.emplace(k, gerar(*m, passo * 2.0f * PI / PASSOS, so_torreta)).first;
        }
        tex = it->second.texture;
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
            if (dx || dy) DrawTexturePro(tex, src, {base.x + dx * e, base.y + dy * e, lado, lado}, {0, 0}, 0, tinta);
    DrawTexturePro(tex, src, base, {0, 0}, 0, {255, 255, 255, alfa});
    if (viva) rlDrawRenderBatchActive();  // o quadro vivo e reaproveitado pela proxima torre
    return true;
}

bool retrato(const std::string& chave, float cx, float cy, float lado, unsigned char alfa) {
    if (!disponivel()) return false;
    auto it = retratos.find(chave);
    if (it == retratos.end()) {
        Texture2D t{};
        const std::string arq = pasta + "/../retratos/" + chave + ".png";
        if (FileExists(arq.c_str())) {
            t = LoadTexture(arq.c_str());
            GenTextureMipmaps(&t);  // o cartao da loja mostra o retrato bem menor que o arquivo
            SetTextureFilter(t, TEXTURE_FILTER_TRILINEAR);
        }
        it = retratos.emplace(chave, t).first;
    }
    const Texture2D& t = it->second;
    if (!t.id) return false;
    DrawTexturePro(t, {0, 0, static_cast<float>(t.width), static_cast<float>(t.height)}, {cx - lado / 2, cy - lado / 2, lado, lado},
                   {0, 0}, 0, {255, 255, 255, alfa});
    return true;
}

bool retrato_em(const std::string& chave, Rectangle caixa, unsigned char alfa) {
    static const char* INTEIRAS[] = {"bomba", "tachinha", "submarino", "bucaneiro", "as", "heli", "fazenda", "espinhos", "vila", "churchill"};
    bool inteira = false;
    for (const char* k : INTEIRAS) inteira |= chave == k;
    const float cx = caixa.x + caixa.width / 2;
    if (inteira) return retrato(chave, cx, caixa.y + caixa.height / 2, std::min(caixa.width, caixa.height * 1.5f), alfa);
    // a cabeca ocupa a metade de cima do retrato: o lado e o que faz ela caber na altura da caixa
    const float lado = std::min(caixa.width * 1.38f, caixa.height * 2.1f);
    ui::recortar(caixa);
    const bool ok = retrato(chave, cx, caixa.y + lado * 0.47f, lado, alfa);
    rlDrawRenderBatchActive();
    ui::fim_recorte();
    return ok;
}

void liberar() {
    for (auto& [k, rt] : sprites) UnloadRenderTexture(rt);
    sprites.clear();
}

}  // namespace bl::m3d
