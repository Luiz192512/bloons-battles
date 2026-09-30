#include "cliente/arte.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <random>
#include <unordered_map>
#include <vector>

#include "cliente/sprites.hpp"
#include "cliente/ui.hpp"
#include "jogo/mapas.hpp"
#include "rlgl.h"

namespace bl::arte {

namespace {

using spr::Caneta;
using spr::H;

constexpr float PI_F = 3.14159265358979f;
constexpr float SUPER = 2.0f;  // superamostragem das texturas em cache
constexpr float MARGEM = 0.25f;  // folga em volta da caixa (chapeus e auras passam da caixa)

using Desenho = std::function<void(Caneta&)>;

// ------------------------------------------------------------------ cache de texturas
struct Sprite {
    RenderTexture2D rt;
    float w, h;  // tamanho em px da tela
};
std::unordered_map<std::string, Sprite> cache;

const Sprite& gerar(const std::string& chave, float w, float h, const std::function<void()>& desenho) {
    auto it = cache.find(chave);
    if (it != cache.end()) return it->second;
    rlDrawRenderBatchActive();
    rlDisableScissorTest();
    Sprite s{LoadRenderTexture(static_cast<int>(std::ceil(w * SUPER)), static_cast<int>(std::ceil(h * SUPER))), w, h};
    BeginTextureMode(s.rt);
    ClearBackground(BLANK);
    // o canal alfa acumula direito (o padrao da raylib multiplica o alfa por ele mesmo)
    rlSetBlendFactorsSeparate(RL_SRC_ALPHA, RL_ONE_MINUS_SRC_ALPHA, RL_ONE, RL_ONE_MINUS_SRC_ALPHA, RL_FUNC_ADD,
                              RL_FUNC_ADD);
    BeginBlendMode(BLEND_CUSTOM_SEPARATE);
    Camera2D cam{{0, 0}, {0, 0}, 0, SUPER};
    BeginMode2D(cam);
    desenho();
    EndMode2D();
    EndBlendMode();
    EndTextureMode();
    SetTextureFilter(s.rt.texture, TEXTURE_FILTER_BILINEAR);
    ui::restaurar_tela();
    return cache[chave] = s;
}

void desenhar_sprite(const Sprite& s, float x, float y, float escala, float rotacao, unsigned char alfa) {
    Rectangle src{0, 0, static_cast<float>(s.rt.texture.width), -static_cast<float>(s.rt.texture.height)};
    Rectangle dst{x, y, s.w * escala, s.h * escala};
    DrawTexturePro(s.rt.texture, src, dst, {dst.width / 2, dst.height / 2}, rotacao, Color{255, 255, 255, alfa});
}

// ------------------------------------------------------------------ sprites em camadas
// Um sprite numa caixa bw x bh desenhado com lado "px" (px da tela por unidade da caixa = px / bw).
struct Camadas {
    std::vector<const Sprite*> seg;  // segmento parado k (nullptr = vazio)
    int grupos = 0;                  // grupos animados entre os segmentos
    float bw = 128, bh = 128, esc = 1;
};
std::unordered_map<std::string, Camadas> cache_camadas;

const Camadas& camadas(const std::string& chave, float bw, float bh, float esc, const Desenho& desenho) {
    auto it = cache_camadas.find(chave);
    if (it != cache_camadas.end()) return it->second;
    Camadas c;
    c.bw = bw, c.bh = bh, c.esc = esc;
    Caneta cont;
    cont.modo = Caneta::Modo::CONTAR;
    cont.reiniciar();
    // o passe de contagem nao desenha nada, mas usa a pilha de matrizes
    rlPushMatrix();
    desenho(cont);
    rlPopMatrix();
    c.grupos = cont.grupos_anim();
    const auto& por_seg = cont.prim_por_segmento();
    const float w = bw * (1 + 2 * MARGEM) * esc, h = bh * (1 + 2 * MARGEM) * esc;
    for (int k = 0; k <= c.grupos; ++k) {
        if (k >= static_cast<int>(por_seg.size()) || por_seg[k] == 0) {
            c.seg.push_back(nullptr);
            continue;
        }
        const Sprite& s = gerar(chave + "#" + std::to_string(k), w, h, [&] {
            rlPushMatrix();
            rlScalef(esc, esc, 1);
            rlTranslatef(bw * MARGEM, bh * MARGEM, 0);
            Caneta p;
            p.modo = Caneta::Modo::ESTATICO;
            p.alvo = k;
            p.reiniciar();
            desenho(p);
            rlPopMatrix();
        });
        c.seg.push_back(&s);
    }
    return cache_camadas[chave] = c;
}

// Desenha as camadas intercalando as texturas com os grupos animados ao vivo.
// (x, y) = centro da caixa na tela; escala e rotacao aplicadas em volta desse centro.
void desenhar_camadas(const Camadas& c, float x, float y, float escala, float rotacao, unsigned char alfa,
                      const Desenho& desenho, double t, const spr::Pose& pose, bool em_jogo) {
    for (int k = 0; k <= c.grupos; ++k) {
        if (c.seg[k]) desenhar_sprite(*c.seg[k], x, y, escala, rotacao, alfa);
        if (k == c.grupos) break;
        rlPushMatrix();
        rlTranslatef(x, y, 0);
        if (rotacao != 0) rlRotatef(rotacao, 0, 0, 1);
        rlScalef(c.esc * escala, c.esc * escala, 1);
        rlTranslatef(-c.bw / 2, -c.bh / 2, 0);
        Caneta p;
        p.modo = Caneta::Modo::VIVO;
        p.alvo = k;
        p.t = static_cast<float>(t);
        p.pose = pose;
        p.em_jogo = em_jogo;
        p.reiniciar(alfa / 255.0f);
        desenho(p);
        rlPopMatrix();
    }
}

// Mesmo que desenhar_camadas, mas com uma transformacao qualquer (caixa -> tela) aplicada pela
// funcao 'xf': as texturas sao desenhadas nas coordenadas da caixa, entao espelho e rotacao em
// volta de qualquer ponto funcionam igual para as partes em cache e as ao vivo.
void desenhar_camadas_xf(const Camadas& c, const std::function<void()>& xf, unsigned char alfa, const Desenho& desenho,
                         double t, const spr::Pose& pose) {
    const float mx = c.bw * MARGEM, my = c.bh * MARGEM;
    for (int k = 0; k <= c.grupos; ++k) {
        rlPushMatrix();
        xf();
        if (c.seg[k]) {
            const Texture2D& tx = c.seg[k]->rt.texture;
            DrawTexturePro(tx, {0, 0, static_cast<float>(tx.width), -static_cast<float>(tx.height)},
                           {-mx, -my, c.bw + 2 * mx, c.bh + 2 * my}, {0, 0}, 0, Color{255, 255, 255, alfa});
        }
        if (k < c.grupos) {
            Caneta p;
            p.modo = Caneta::Modo::VIVO;
            p.alvo = k;
            p.t = static_cast<float>(t);
            p.pose = pose;
            p.em_jogo = true;
            p.reiniciar(alfa / 255.0f);
            desenho(p);
        }
        rlPopMatrix();
    }
}

// Tudo ao vivo (sem cache): menus animados, efeitos e vitrine.
void desenhar_vivo(float bw, float bh, float x, float y, float esc, float rotacao, const Desenho& desenho, double t,
                   const spr::Pose& pose = {}, bool em_jogo = false, float alfa = 1) {
    rlPushMatrix();
    rlTranslatef(x, y, 0);
    if (rotacao != 0) rlRotatef(rotacao, 0, 0, 1);
    rlScalef(esc, esc, 1);
    rlTranslatef(-bw / 2, -bh / 2, 0);
    Caneta p;
    p.modo = Caneta::Modo::TUDO;
    p.t = static_cast<float>(t);
    p.pose = pose;
    p.em_jogo = em_jogo;
    p.reiniciar(alfa);
    desenho(p);
    rlPopMatrix();
}

// Sprite inteiro numa textura so (icones e retratos parados).
const Sprite& inteiro(const std::string& chave, float bw, float bh, float esc, const Desenho& desenho) {
    const float w = bw * (1 + 2 * MARGEM) * esc, h = bh * (1 + 2 * MARGEM) * esc;
    return gerar(chave, w, h, [&] {
        rlPushMatrix();
        rlScalef(esc, esc, 1);
        rlTranslatef(bw * MARGEM, bh * MARGEM, 0);
        Caneta p;
        p.reiniciar();
        desenho(p);
        rlPopMatrix();
    });
}

std::string num(float v) { return std::to_string(static_cast<int>(std::lround(v))); }

Desenho desenho_torre(const std::string& chave, const Visual& v, spr::Vista vista) {
    return [chave, v, vista](Caneta& p) { spr::torre(p, chave, vista, v.cam, v.tier, v.nivel); };
}

// ================================================================== MAPA
void desenhar_mapa(const Mapa& mp) {
    const DefMapa& m = mp.def;
    const Color grama = cor(m.grama), terra = cor(m.terra);
    Caneta p;
    p.reiniciar();
    DrawRectangle(0, 0, LARGURA_MAPA, ALTURA_MAPA, grama);
    std::mt19937 rng(static_cast<unsigned>(m.chave.size() * 31 + 7));
    auto uni = [&](float a, float b) { return a + (b - a) * static_cast<float>(rng() % 10000) / 10000.0f; };
    // manchas claras e tufos de grama
    for (int i = 0; i < 34; ++i)
        p.e(uni(0, LARGURA_MAPA), uni(0, ALTURA_MAPA), uni(26, 56), uni(12, 22), spr::al(spr::clar(grama, 0.12f), 0.7f));
    for (int i = 0; i < 140; ++i) {
        const float x = uni(0, LARGURA_MAPA), y = uni(0, ALTURA_MAPA);
        p.ln({{x, y}, {x + 3, y - 7}, {x + 6, y}, {x + 9, y - 5}}, 2, spr::esc(grama, 0.8f));
    }
    // agua com margem de areia e contorno
    for (auto& [cx, cy, r0] : m.agua) {
        const float x = static_cast<float>(cx), y = static_cast<float>(cy), r = static_cast<float>(r0);
        p.c(x, y, r + 6, H(0xE6D3A0), 1.5f);
        p.c(x, y, r, H(0x3AA0E6));
        p.e(x - r * 0.3f, y - r * 0.35f, r * 0.4f, r * 0.12f, spr::al(spr::H(0xFFFFFF), 0.35f));
    }
    for (auto& [rx, ry, rw, rh] : m.agua_ret) {
        const float x = static_cast<float>(rx), y = static_cast<float>(ry), w = static_cast<float>(rw), h = static_cast<float>(rh);
        p.rr(x - 6, y - 6, w + 12, h + 12, 18, H(0xE6D3A0), 1.5f);
        p.rr(x, y, w, h, 14, H(0x3AA0E6));
    }
    // trilha: tinta, borda escura, terra e faixa clara no meio
    const float lt = static_cast<float>(LARGURA_TRILHA);
    const float larguras[4] = {lt + 14, lt + 8, lt, 14};
    const Color cores[4] = {spr::TINTA, spr::esc(terra, 0.72f), terra, spr::al(spr::clar(terra, 0.25f), 0.6f)};
    for (int k = 0; k < 4; ++k)
        for (const Caminho& cam : mp.caminhos) {
            std::vector<Vector2> v;
            for (auto& q : cam.pontos) v.push_back({static_cast<float>(q.x), static_cast<float>(q.y)});
            p.ln(v, larguras[k], cores[k]);
        }
    // obstaculos com sombra e contorno
    for (auto& o : m.obstaculos) {
        const float x = static_cast<float>(o.x), y = static_cast<float>(o.y), r = static_cast<float>(o.r);
        p.e(x + 5, y + r * 0.7f, r, r * 0.4f, H(0, 0x33));
        if (o.tipo == "arvore") {
            p.c(x, y, r, H(0x2E7D2A), 1.5f);
            p.c(x - r * 0.25f, y - r * 0.25f, r * 0.6f, H(0x3F9A3A));
            p.c(x - r * 0.4f, y - r * 0.4f, r * 0.22f, H(0x6DC24A));
        } else {
            p.e(x, y, r, r * 0.75f, H(0x8E97A4), 1.5f);
            p.e(x - r * 0.3f, y - r * 0.3f, r * 0.35f, r * 0.2f, H(0xC7CDD6));
        }
    }
}

}  // namespace

// ================================================================== interface publica
Visual visual_caminhos(const std::array<int, 3>& caminhos) {
    Visual v;
    int melhor = 0;
    for (int i = 0; i < 3; ++i)
        if (caminhos[i] > melhor) melhor = caminhos[i], v.cam = i;
    v.tier = melhor >= 5 ? 5 : melhor >= 3 ? 3 : 0;
    if (!v.tier) v.cam = -1;
    return v;
}

Visual visual(const Torre& t) {
    if (t.dfn->heroi) {
        Visual v;
        v.nivel = t.nivel;
        return v;
    }
    return visual_caminhos(t.caminhos);
}

namespace {
// Na caixa 3/4 o pe do macaco fica em (64, 116) e o corpo em volta de (64, 92): esse ponto vai
// sobre a posicao da torre, para o boneco ficar em pe onde foi colocado.
constexpr float PE_Y = 116, CORPO_Y = 92;
}  // namespace

float pe_torre_mapa(float tam) { return (PE_Y - CORPO_Y) * tam / 128; }

void torre_mapa(const std::string& chave, const Visual& v, float x, float y, float tam, const anim::Quadro* q,
                unsigned char alfa) {
    const std::string k = "tf:" + chave + ":" + std::to_string(v.cam) + ":" + std::to_string(v.tier) + ":" +
                          std::to_string(v.nivel) + ":" + num(tam);
    const Desenho d = desenho_torre(chave, v, spr::Vista::FRENTE);
    const float s = tam / 128;
    const Camadas& c = camadas(k, 128, 128, s, d);
    anim::Quadro neutro;
    const anim::Quadro& qq = q ? *q : neutro;
    const float esc = qq.pose.ativa ? qq.escala : 1.0f;
    const float salto = qq.pose.ativa ? qq.salto : 0.0f;
    // o corpo recua um pouco para o lado contrario ao que esta virado
    const float recuo = qq.pose.ativa ? -qq.corpo_recuo * (qq.sx < 0 ? -1.0f : 1.0f) : 0.0f;
    const float pe_x = x + recuo, pe_y = y + pe_torre_mapa(tam) + salto;
    auto xf = [&] {
        rlTranslatef(pe_x, pe_y, 0);
        if (qq.inclina != 0) rlRotatef(qq.inclina, 0, 0, 1);
        rlScalef(s * esc * qq.sx, s * esc, 1);
        rlTranslatef(-64, -PE_Y, 0);
    };
    desenhar_camadas_xf(c, xf, alfa, d, ui::tempo(), qq.pose);
}

void torre(const std::string& chave, const Visual& v, float x, float y, float tam, float rotacao, const anim::Quadro* q,
           unsigned char alfa) {
    const std::string k = "tc:" + chave + ":" + std::to_string(v.cam) + ":" + std::to_string(v.tier) + ":" +
                          std::to_string(v.nivel) + ":" + num(tam);
    const Desenho d = desenho_torre(chave, v, spr::Vista::CIMA);
    const Camadas& c = camadas(k, 128, 128, tam / 128, d);
    float esc = 1;
    spr::Pose pose;
    if (q && q->pose.ativa) {
        pose = q->pose;
        esc = q->escala * (1 - q->salto * 0.012f);
        // o corpo recua para tras (em relacao a mira)
        const float a = rotacao * PI_F / 180;
        x += -std::sin(a) * q->corpo_recuo;
        y += std::cos(a) * q->corpo_recuo;
    }
    desenhar_camadas(c, x, y, esc, rotacao, alfa, d, ui::tempo(), pose, true);
}

void torre_icone(const std::string& chave, const Visual& v, float x, float y, float tam, unsigned char alfa) {
    const std::string k = "ti:" + chave + ":" + std::to_string(v.cam) + ":" + std::to_string(v.tier) + ":" +
                          std::to_string(v.nivel) + ":" + num(tam);
    const Sprite& s = inteiro(k, 128, 128, tam / 128, desenho_torre(chave, v, spr::Vista::FRENTE));
    desenhar_sprite(s, x, y, 1, 0, alfa);
}

void torre_viva(const std::string& chave, const Visual& v, float x, float y, float tam, double t, const anim::Quadro* q,
                bool cima) {
    spr::Pose pose;
    float esc = tam / 128;
    if (q && q->pose.ativa) {
        pose = q->pose;
        esc *= q->escala;
        y += q->salto;
    }
    desenhar_vivo(128, 128, x, y, esc, 0, desenho_torre(chave, v, cima ? spr::Vista::CIMA : spr::Vista::FRENTE), t, pose,
                  q && q->pose.ativa);
}

// ------------------------------------------------------------------ bloons
float raio_bloon_px(const TipoBloon& t) { return std::floor(static_cast<float>(t.raio)); }

void bloon(const TipoBloon& t, bool camo, bool regen, bool fort, int dano, float x, float y, float escala) {
    const float r = raio_bloon_px(t);
    const float s = 0.9f * r / 30;  // o corpo do bloon tem raio 30 na caixa
    const std::string k = "b:" + t.nome + ":" + std::to_string(camo) + std::to_string(regen) + std::to_string(fort) +
                          std::to_string(dano) + ":" + num(r);
    const std::string nome = t.nome;
    const Desenho d = [=](Caneta& p) { spr::bloon(p, nome, camo, regen, fort, false, dano); };
    const Camadas& c = camadas(k, 128, 128, s, d);
    // o centro do corpo esta 6 unidades acima do centro da caixa
    desenhar_camadas(c, x, y + 6 * s * escala, escala, 0, 255, d, ui::tempo(), {}, true);
}

void bloon_vivo(const std::string& tipo, bool camo, bool regen, bool fort, float x, float y, float tam, double t) {
    desenhar_vivo(128, 128, x, y, tam / 128, 0, [=](Caneta& p) { spr::bloon(p, tipo, camo, regen, fort, true); }, t);
}

void estado_bloon(int estado, float x, float y, float raio, double t) {
    // o bloon dos estados do design tem corpo com raio ~23 em volta de (64, 66)
    const float s = raio / 23;
    desenhar_vivo(128, 128, x, y - 2 * s, s, 0, [estado](Caneta& p) {
        if (estado == 0) spr::estado_congelado(p);
        else if (estado == 1) spr::estado_colado(p);
        else if (estado == 2) spr::estado_queimando(p);
        else spr::estado_atordoado(p);
    }, t);
}

// ------------------------------------------------------------------ dirigiveis
namespace {
float escala_dirigivel(const TipoBloon& t) {
    return 1.3f * static_cast<float>(t.raio) / spr::raios_dirigivel(t.nome).x;
}
}  // namespace

Vector2 tamanho_dirigivel(const TipoBloon& t) {
    const float s = escala_dirigivel(t);
    return {220 * s, 120 * s};
}

void dirigivel(const TipoBloon& t, bool fort, int dano, float x, float y, float ang, float escala) {
    const float s = escala_dirigivel(t);
    const std::string k = "d:" + t.nome + ":" + std::to_string(fort) + std::to_string(dano);
    const std::string nome = t.nome;
    const Desenho d = [=](Caneta& p) { spr::dirigivel(p, nome, dano, fort, false); };
    const Camadas& c = camadas(k, 220, 120, s, d);
    // o corpo esta centrado em x = 112 (a caixa em 110): puxa 2 unidades para tras
    const float a = ang * PI_F / 180, dx = -2 * s * escala;
    desenhar_camadas(c, x + std::cos(a) * dx, y + std::sin(a) * dx, escala, ang, 255, d, ui::tempo(), {}, true);
}

void dirigivel_vivo(const std::string& tipo, int dano, bool fort, float x, float y, float largura, double t) {
    desenhar_vivo(220, 120, x, y, largura / 220, 0, [=](Caneta& p) { spr::dirigivel(p, tipo, dano, fort, true); }, t);
}

// ------------------------------------------------------------------ mapa e icones
void fundo_mapa(const std::string& chave, Rectangle destino) {
    const Sprite& s = gerar("m:" + chave, LARGURA_MAPA, ALTURA_MAPA, [&] { desenhar_mapa(mapa(chave)); });
    Rectangle src{0, 0, static_cast<float>(s.rt.texture.width), -static_cast<float>(s.rt.texture.height)};
    DrawTexturePro(s.rt.texture, src, destino, {0, 0}, 0, WHITE);
}

void icone(const std::string& nome, float x, float y, float tam) {
    const Sprite& s = inteiro("i:" + nome + ":" + num(tam), 32, 32, tam / 32, [nome](Caneta& p) { spr::icone(p, nome); });
    desenhar_sprite(s, x + tam / 2, y + tam / 2, 1, 0, 255);
}
void icone_coracao(float x, float y, int tam) { icone("coracao", x, y, static_cast<float>(tam)); }
void icone_moeda(float x, float y, int tam) { icone("moeda", x, y, static_cast<float>(tam)); }
void icone_eco(float x, float y, int tam) { icone("eco", x, y, static_cast<float>(tam)); }

void habilidade(const std::string& dono, bool heroi, int cam, int tier, int nivel, const std::string& efeito, float x,
                float y, float tam) {
    const std::string k = "h:" + dono + ":" + std::to_string(heroi) + std::to_string(cam) + std::to_string(tier) +
                          std::to_string(nivel) + efeito + ":" + num(tam);
    const Sprite& s = inteiro(k, 128, 128, tam / 128, [=](Caneta& p) { spr::habilidade(p, dono, heroi, cam, tier, nivel, efeito); });
    desenhar_sprite(s, x, y, 1, 0, 255);
}

// ------------------------------------------------------------------ projeteis e pilhas
namespace {
Desenho desenho_proj(const std::string& visual) {
    return [visual](Caneta& p) { p.g(spr::g_em(32, 32), [&] { spr::projetil(p, visual); }); };
}
}  // namespace

void projetil(const Projetil& p, float ox, float oy, float escala) {
    const std::string& v = p.at->visual;
    if (v.empty() || v == "nenhum") return;
    const float x = ox + static_cast<float>(p.x) * escala, y = oy + static_cast<float>(p.y) * escala;
    if (escala < 0.5f) {
        DrawCircleV({x, y}, 2, spr::TINTA);
        return;
    }
    // o desenho do projetil tem raio ~28; no mapa fica um pouco maior que o raio de colisao
    const float s = std::clamp((static_cast<float>(p.raio) + 9) / 28, 0.35f, 1.2f);
    const Desenho d = desenho_proj(v);
    const Camadas& c = camadas("p:" + v + ":" + num(s * 100), 64, 64, s, d);
    desenhar_camadas(c, x, y, escala, static_cast<float>(p.ang) + 90, 255, d, ui::tempo(), {}, true);
}

void projetil_vivo(const std::string& visual, float x, float y, float tam, float ang, double t) {
    desenhar_vivo(64, 64, x, y, tam / 64, ang, desenho_proj(visual), t);
}

void pilha(const Pilha& s, float ox, float oy, float escala) {
    const float x = std::floor(ox + static_cast<float>(s.x) * escala), y = std::floor(oy + static_cast<float>(s.y) * escala);
    std::string v = s.visual == "espinheiro" ? "espinho" : s.visual;
    if (!spr::tem_projetil(v)) v = "espinhos";
    if (escala < 0.5f) {
        DrawCircleV({x, y}, 2, H(0x8E97A4));
        return;
    }
    const Desenho d = desenho_proj(v);
    const Camadas& c = camadas("s:" + v, 64, 64, 0.55f, d);
    desenhar_camadas(c, x, y, escala, 0, 255, d, ui::tempo(), {}, true);
}

void efeito(const std::string& chave, float x, float y, float tam, double t) {
    desenhar_vivo(128, 128, x, y, tam / 128, 0, [chave](Caneta& p) { spr::efeito(p, chave); }, t);
}

void liberar() {
    for (auto& [k, s] : cache) UnloadRenderTexture(s.rt);
    cache.clear();
    cache_camadas.clear();
}

}  // namespace bl::arte
