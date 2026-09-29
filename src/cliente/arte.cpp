#include "cliente/arte.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <random>
#include <unordered_map>
#include <vector>

#include "cliente/ui.hpp"
#include "jogo/mapas.hpp"
#include "rlgl.h"

namespace bl::arte {

namespace {

constexpr float PI_F = 3.14159265358979f;
constexpr float SUPER = 2.0f;  // superamostragem das texturas em cache

Color C(int r, int g, int b, int a = 255) { return ui::rgb(r, g, b, a); }
Color sombra(Color c, float f = 0.7f) { return ui::escurecer(c, f); }
Color clarear(Color c, float f = 0.35f) { return ui::clarear(c, f); }

const Color PRETO_A = C(20, 20, 24);
const Color BRANCO_A = C(255, 255, 255);
const Color PELO = C(140, 88, 44);
const Color ROSTO = C(236, 196, 150);

// ------------------------------------------------------------------ primitivas (estilo pygame.draw)
void circ(float x, float y, float r, Color c) {
    if (r > 0) DrawCircleV({x, y}, r, c);
}
void circ_l(float x, float y, float r, float esp, Color c) {
    if (r > 0) DrawRing({x, y}, std::max(0.0f, r - esp), r, 0, 360, 48, c);
}
// elipse inscrita no retangulo (x, y, w, h)
void elip(float x, float y, float w, float h, Color c) {
    if (w <= 0 || h <= 0) return;
    const Vector2 m{x + w / 2, y + h / 2};
    const int n = 40;
    Vector2 ant{m.x + w / 2, m.y};
    for (int i = 1; i <= n; ++i) {
        float a = 2 * PI_F * i / n;
        Vector2 p{m.x + std::cos(a) * w / 2, m.y + std::sin(a) * h / 2};
        DrawTriangle(m, ant, p, c);
        ant = p;
    }
}
void elip_l(float x, float y, float w, float h, float esp, Color c) {
    const float cx = x + w / 2, cy = y + h / 2;
    const int n = 40;
    for (int i = 0; i < n; ++i) {
        float a0 = 2 * PI_F * i / n, a1 = 2 * PI_F * (i + 1) / n;
        Vector2 p0{cx + std::cos(a0) * (w / 2 - esp / 2), cy + std::sin(a0) * (h / 2 - esp / 2)};
        Vector2 p1{cx + std::cos(a1) * (w / 2 - esp / 2), cy + std::sin(a1) * (h / 2 - esp / 2)};
        DrawLineEx(p0, p1, esp, c);
    }
}
void rect(float x, float y, float w, float h, Color c, float raio = 0) { ui::ret({x, y, w, h}, c, raio); }
void linha(float x1, float y1, float x2, float y2, float esp, Color c) {
    DrawLineEx({x1, y1}, {x2, y2}, esp, c);
    if (esp >= 3) {
        circ(x1, y1, esp / 2 - 0.5f, c);
        circ(x2, y2, esp / 2 - 0.5f, c);
    }
}
void linhas(const std::vector<Vector2>& pts, float esp, Color c) {
    for (size_t i = 1; i < pts.size(); ++i) linha(pts[i - 1].x, pts[i - 1].y, pts[i].x, pts[i].y, esp, c);
}
// poligono preenchido (leque a partir do centroide: serve para convexos e estrelas)
void poli(const std::vector<Vector2>& pts, Color c) {
    if (pts.size() < 3) return;
    Vector2 m{0, 0};
    for (auto& p : pts) m.x += p.x, m.y += p.y;
    m.x /= pts.size();
    m.y /= pts.size();
    for (size_t i = 0; i < pts.size(); ++i) DrawTriangle(m, pts[i], pts[(i + 1) % pts.size()], c);
}
void poli_l(const std::vector<Vector2>& pts, float esp, Color c) {
    for (size_t i = 0; i < pts.size(); ++i) {
        const Vector2& a = pts[i];
        const Vector2& b = pts[(i + 1) % pts.size()];
        DrawLineEx(a, b, esp, c);
    }
}
// arco da elipse inscrita em (x, y, w, h), angulos em radianos no sentido anti-horario (como o pygame)
void arco(float x, float y, float w, float h, float a0, float a1, float esp, Color c) {
    const float cx = x + w / 2, cy = y + h / 2;
    const int n = 24;
    std::vector<Vector2> pts;
    for (int i = 0; i <= n; ++i) {
        float a = a0 + (a1 - a0) * i / n;
        pts.push_back({cx + std::cos(a) * (w / 2 - esp / 2), cy - std::sin(a) * (h / 2 - esp / 2)});
    }
    linhas(pts, esp, c);
}

// Preenche linha a linha a interseccao de uma elipse com faixas definidas por fn(y) -> [x0, x1].
// Usado para recortar listras, faixas e manchas no formato do bloon.
void por_linhas(float cx, float cy, float rx, float ry,
                const std::function<void(float y, float xl, float xr)>& fn) {
    const float passo = 1.0f / SUPER;
    for (float y = cy - ry; y < cy + ry; y += passo) {
        float t = (y + passo / 2 - cy) / ry;
        if (t * t >= 1) continue;
        float meio = rx * std::sqrt(1 - t * t);
        fn(y, cx - meio, cx + meio);
    }
}
void faixa(float y, float x0, float x1, Color c) {
    if (x1 > x0) DrawRectangleRec({x0, y, x1 - x0, 1.0f / SUPER + 0.01f}, c);
}

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
    // o canal alfa acumula direito (padrao da raylib multiplica o alfa por ele mesmo)
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
    DrawTexturePro(s.rt.texture, src, dst, {dst.width / 2, dst.height / 2}, rotacao, C(255, 255, 255, alfa));
}

// ================================================================== MACACOS
struct Estilo {
    Color pelo, rosto;
    const char* chapeu;
    const char* item;
    Color cor;
};

const std::map<std::string, Estilo>& estilos() {
    static const std::map<std::string, Estilo> m = {
        {"dardo", {PELO, ROSTO, nullptr, "dardo", BLANK}},
        {"bumerangue", {PELO, ROSTO, "faixa", "bumerangue", C(220, 60, 50)}},
        {"gelo", {C(150, 205, 235), C(230, 245, 255), "gorro", nullptr, C(80, 150, 210)}},
        {"cola", {PELO, ROSTO, "oculos", "arma_cola", C(120, 190, 60)}},
        {"sniper", {PELO, ROSTO, "capacete", "rifle", C(80, 110, 60)}},
        {"mago", {PELO, ROSTO, "chapeu_mago", "varinha", C(110, 60, 170)}},
        {"super", {PELO, ROSTO, "mascara_super", nullptr, C(40, 90, 200)}},
        {"ninja", {PELO, ROSTO, "ninja", "shuriken", C(180, 30, 30)}},
        {"alquimista", {PELO, ROSTO, "chapeu_alq", "pocao", C(110, 60, 130)}},
        {"druida", {PELO, ROSTO, "coroa_folhas", nullptr, C(60, 140, 50)}},
        {"engenheiro", {PELO, ROSTO, "capacete_obra", "pregadora", C(240, 200, 40)}},
        // herois
        {"quincy", {C(150, 95, 45), ROSTO, "capuz", "arco", C(60, 120, 60)}},
        {"gwendolin", {C(190, 80, 40), ROSTO, "oculos", "lanca_chamas", C(230, 120, 30)}},
        {"striker", {C(110, 80, 50), ROSTO, "boina", "lanca_foguete", C(60, 90, 50)}},
        {"obyn", {C(60, 110, 90), C(200, 230, 210), "coroa_folhas", "cajado", C(40, 150, 90)}},
        {"churchill", {C(110, 80, 50), ROSTO, "tanque", nullptr, C(80, 100, 70)}},
        {"benjamin", {C(90, 70, 60), ROSTO, "fones", "laptop", C(40, 40, 50)}},
        {"ezili", {C(110, 60, 80), ROSTO, "caveira", "cajado", C(150, 40, 110)}},
        {"pat", {C(150, 110, 70), ROSTO, nullptr, nullptr, BLANK}},
        {"adora", {C(200, 160, 90), ROSTO, "coroa_sol", nullptr, C(240, 200, 60)}},
        {"brickell", {C(110, 80, 50), ROSTO, "quepe", "pistola", C(40, 60, 130)}},
        {"etienne", {C(120, 90, 60), ROSTO, "boina", "controle", C(70, 110, 160)}},
        {"sauda", {C(160, 100, 60), ROSTO, "faixa", "espadas", C(200, 60, 60)}},
        {"psi", {C(150, 110, 170), C(230, 210, 240), "psi", nullptr, C(180, 100, 220)}},
        {"geraldo", {C(120, 80, 50), ROSTO, "chapeu_alq", "sacola", C(140, 90, 40)}},
        {"corvus", {C(50, 50, 80), C(180, 180, 210), "capuz", "cajado", C(40, 40, 90)}},
        {"rosalia", {C(170, 100, 110), ROSTO, "oculos", "lanca_foguete", C(220, 100, 130)}},
        {"jericho", {C(120, 85, 55), ROSTO, "chapeu_cowboy", "pistola", C(110, 70, 40)}},
        {"silas", {C(140, 200, 230), C(230, 245, 255), "gorro", "cajado", C(90, 160, 220)}},
    };
    return m;
}

bool eh(const char* a, const char* b) { return a && std::string(a) == b; }

void circulo_borda(Color c, float x, float y, float r, float borda = 2) {
    circ(x, y, r + borda, sombra(c, 0.55f));
    circ(x, y, r, c);
}

// Macaco visto de cima, olhando para cima (-y). u = unidade (raio da cabeca).
void macaco_base(float cx, float cy, float u, Color pelo, Color rosto) {
    // corpo e bracos
    circulo_borda(pelo, cx, cy + u * 0.55f, u * 0.8f);
    circulo_borda(pelo, cx - u * 0.75f, cy - u * 0.2f, u * 0.32f);
    circulo_borda(pelo, cx + u * 0.75f, cy - u * 0.2f, u * 0.32f);
    // orelhas
    for (int sx : {-1, 1}) {
        circulo_borda(pelo, cx + sx * u * 0.95f, cy - u * 0.1f, u * 0.3f);
        circ(cx + sx * u * 0.95f, cy - u * 0.1f, u * 0.17f, rosto);
    }
    // cabeca
    circulo_borda(pelo, cx, cy, u);
    // rosto
    elip(cx - u * 0.62f, cy - u * 0.75f, u * 1.24f, u * 1.05f, rosto);
    // olhos
    for (int sx : {-1, 1}) {
        circ(cx + sx * u * 0.25f, cy - u * 0.4f, std::max(2.0f, u * 0.18f), BRANCO_A);
        circ(cx + sx * u * 0.25f, cy - u * 0.45f, std::max(1.0f, u * 0.1f), PRETO_A);
    }
    // focinho
    elip(cx - u * 0.32f, cy - u * 0.2f, u * 0.64f, u * 0.38f, sombra(rosto, 0.9f));
    circ(cx - u * 0.1f, cy - u * 0.1f, std::max(1.0f, u * 0.05f), sombra(rosto, 0.5f));
    circ(cx + u * 0.1f, cy - u * 0.1f, std::max(1.0f, u * 0.05f), sombra(rosto, 0.5f));
}

void chapeu(float cx, float cy, float u, const char* tipo, Color cor) {
    if (!tipo) return;
    if (eh(tipo, "faixa")) {
        rect(cx - u, cy - u * 0.75f, 2 * u, u * 0.28f, cor, 3);
    } else if (eh(tipo, "gorro")) {
        elip(cx - u, cy - u * 1.05f, 2 * u, u * 0.9f, cor);
        circ(cx, cy - u * 1.05f, u * 0.25f, BRANCO_A);
    } else if (eh(tipo, "oculos")) {
        for (int sx : {-1, 1}) circ_l(cx + sx * u * 0.28f, cy - u * 0.42f, u * 0.24f, 3, cor);
    } else if (eh(tipo, "capacete") || eh(tipo, "capacete_obra")) {
        elip(cx - u * 1.08f, cy - u * 1.12f, u * 2.16f, u * 1.2f, sombra(cor, 0.6f));
        elip(cx - u, cy - u * 1.08f, 2 * u, u * 1.08f, cor);
        if (eh(tipo, "capacete_obra")) rect(cx - u * 0.12f, cy - u * 1.08f, u * 0.24f, u * 0.9f, sombra(cor, 0.8f));
    } else if (eh(tipo, "chapeu_mago")) {
        elip(cx - u * 1.2f, cy - u * 0.8f, u * 2.4f, u * 0.7f, sombra(cor, 0.6f));
        poli({{cx - u * 1.1f, cy - u * 0.45f}, {cx + u * 1.1f, cy - u * 0.45f}, {cx, cy - u * 2.0f}}, cor);
        circ(cx + u * 0.2f, cy - u * 1.1f, std::max(2.0f, u * 0.13f), C(255, 230, 90));
    } else if (eh(tipo, "mascara_super")) {
        rect(cx - u * 0.62f, cy - u * 0.62f, u * 1.24f, u * 0.34f, cor, 4);
        poli({{cx - u, cy + u * 0.3f}, {cx + u, cy + u * 0.3f}, {cx + u * 1.3f, cy + u * 1.6f},
              {cx - u * 1.3f, cy + u * 1.6f}},
             C(220, 40, 40));
    } else if (eh(tipo, "ninja")) {
        elip(cx - u, cy - u, 2 * u, u * 1.3f, C(30, 30, 36));
        rect(cx - u * 0.55f, cy - u * 0.62f, u * 1.1f, u * 0.36f, ROSTO, 4);
        for (int sx : {-1, 1}) circ(cx + sx * u * 0.25f, cy - u * 0.45f, std::max(1.0f, u * 0.1f), PRETO_A);
        rect(cx - u, cy - u * 0.2f, 2 * u, u * 0.22f, cor);
    } else if (eh(tipo, "chapeu_alq")) {
        elip(cx - u * 1.15f, cy - u * 0.95f, u * 2.3f, u * 0.6f, sombra(cor, 0.7f));
        rect(cx - u * 0.6f, cy - u * 1.6f, u * 1.2f, u * 0.9f, cor, 4);
    } else if (eh(tipo, "coroa_folhas") || eh(tipo, "coroa_sol")) {
        for (int k = 0; k < 7; ++k) {
            float a = PI_F + k * PI_F / 6;
            float x = cx + std::cos(a) * u * 0.9f;
            float y = cy - u * 0.3f + std::sin(a) * u * 0.9f;
            if (eh(tipo, "coroa_sol")) poli({{x, y}, {x + 4, y + 8}, {x - 4, y + 8}}, cor);
            else elip(x - 5, y - 4, 10, 8, cor);
        }
    } else if (eh(tipo, "capuz")) {
        arco(cx - u * 1.15f, cy - u * 1.15f, u * 2.3f, u * 2.3f, 0.2f, PI_F - 0.2f, std::max(3.0f, u * 0.35f), cor);
    } else if (eh(tipo, "boina") || eh(tipo, "quepe") || eh(tipo, "chapeu_cowboy")) {
        float largura = eh(tipo, "chapeu_cowboy") ? 1.4f : 1.0f;
        elip(cx - u * largura, cy - u * 1.15f, u * largura * 2, u * 0.8f, cor);
        if (eh(tipo, "quepe")) rect(cx - u * 0.2f, cy - u * 0.95f, u * 0.4f, u * 0.2f, C(230, 200, 60));
    } else if (eh(tipo, "fones")) {
        arco(cx - u * 1.1f, cy - u * 1.1f, u * 2.2f, u * 1.6f, 0, PI_F, 4, cor);
        for (int sx : {-1, 1}) circ(cx + sx * u * 1.0f, cy - u * 0.2f, u * 0.3f, cor);
    } else if (eh(tipo, "caveira")) {
        circ(cx, cy - u * 0.95f, u * 0.38f, C(240, 240, 230));
        circ(cx - 3, cy - u * 1.0f, 2, PRETO_A);
        circ(cx + 3, cy - u * 1.0f, 2, PRETO_A);
    } else if (eh(tipo, "psi")) {
        circ_l(cx, cy - u * 0.2f, u * 1.25f, 2, C(240, 200, 255));
        circ(cx, cy - u * 0.95f, u * 0.22f, cor);
    }
}

// Objeto nas maos, apontando para cima.
void item(float cx, float cy, float u, const char* it, Color cor) {
    if (!it) return;
    const float hx = cx + u * 0.75f, hy = cy - u * 0.45f;
    const bool tem_cor = cor.a != 0;
    if (eh(it, "dardo") || eh(it, "pregadora")) {
        linha(hx, hy, hx, hy - u * 0.9f, std::max(2.0f, u / 6), C(90, 90, 100));
        poli({{hx - 3, hy - u * 0.9f}, {hx + 3, hy - u * 0.9f}, {hx, hy - u * 1.2f}}, C(200, 200, 210));
        if (eh(it, "pregadora")) rect(hx - 5, hy - 8, 10, 14, C(230, 190, 40), 2);
    } else if (eh(it, "bumerangue")) {
        linhas({{hx - 8, hy - 2}, {hx, hy - 12}, {hx + 8, hy - 2}}, 4, C(240, 200, 40));
    } else if (eh(it, "arma_cola")) {
        rect(hx - 4, hy - u * 0.9f, 8, u * 0.9f, C(90, 90, 90));
        circ(hx, hy - u * 0.2f, u * 0.28f, C(130, 200, 60));
    } else if (eh(it, "rifle")) {
        linha(cx, cy - u * 0.3f, cx, cy - u * 2.0f, std::max(3.0f, u / 4), C(60, 45, 30));
        linha(cx, cy - u * 1.3f, cx, cy - u * 2.3f, std::max(2.0f, u / 6), C(40, 40, 40));
    } else if (eh(it, "varinha") || eh(it, "cajado")) {
        linha(hx, hy + 6, hx, hy - u * 1.1f, 3, C(110, 70, 30));
        circ(hx, hy - u * 1.15f, std::max(3.0f, u * 0.2f), tem_cor ? cor : C(160, 90, 220));
    } else if (eh(it, "shuriken")) {
        poli({{hx, hy - 9}, {hx + 3, hy - 3}, {hx + 9, hy}, {hx + 3, hy + 3}, {hx, hy + 9}, {hx - 3, hy + 3},
              {hx - 9, hy}, {hx - 3, hy - 3}},
             C(170, 170, 180));
    } else if (eh(it, "pocao")) {
        circ(hx, hy - 4, u * 0.28f, C(120, 220, 90));
        rect(hx - 2, hy - 4 - u * 0.45f, 4, u * 0.2f, C(200, 200, 220));
    } else if (eh(it, "arco")) {
        arco(hx - 10, hy - u * 1.3f, 20, u * 1.4f, -0.3f, PI_F + 0.3f, 3, C(110, 70, 30));
    } else if (eh(it, "lanca_chamas") || eh(it, "lanca_foguete") || eh(it, "pistola") || eh(it, "controle") ||
               eh(it, "laptop") || eh(it, "sacola")) {
        rect(hx - 5, hy - u * 0.9f, 10, u * 0.9f, tem_cor ? cor : C(80, 80, 90), 3);
    } else if (eh(it, "espadas")) {
        for (int sx : {-1, 1}) {
            float x = cx + sx * u * 0.75f;
            linha(x, hy, x, hy - u * 1.3f, 3, C(220, 220, 230));
        }
    }
}

// Torres que nao sao um macaco simples.
bool torre_especial(float cx, float cy, float u, const std::string& chave) {
    if (chave == "tachinha") {
        std::vector<Vector2> pts, sombra_pts;
        for (int k = 0; k < 8; ++k) {
            float a = k * PI_F / 4 + PI_F / 8;
            pts.push_back({cx + std::cos(a) * u * 1.1f, cy + std::sin(a) * u * 1.1f});
            sombra_pts.push_back({pts.back().x + 2, pts.back().y + 2});
        }
        poli(sombra_pts, C(90, 90, 100));
        poli(pts, C(175, 175, 185));
        for (int k = 0; k < 8; ++k) {
            float a = k * PI_F / 4;
            linha(cx, cy, cx + std::cos(a) * u * 1.3f, cy + std::sin(a) * u * 1.3f, 3, C(60, 60, 70));
        }
        circ(cx, cy, u * 0.45f, C(220, 60, 50));
        return true;
    }
    if (chave == "bomba") {
        macaco_base(cx, cy + u * 0.4f, u * 0.7f, PELO, ROSTO);
        rect(cx - u * 0.45f, cy - u * 1.4f, u * 0.9f, u * 1.6f, C(30, 30, 36), 5);
        rect(cx - u * 0.55f, cy - u * 1.5f, u * 1.1f, u * 0.35f, C(70, 70, 80), 4);
        return true;
    }
    if (chave == "submarino") {
        elip(cx - u * 0.65f, cy - u * 1.5f, u * 1.3f, u * 3.0f, C(160, 130, 20));
        elip(cx - u * 0.55f, cy - u * 1.4f, u * 1.1f, u * 2.8f, C(240, 205, 40));
        circ(cx, cy - u * 0.2f, u * 0.4f, C(160, 130, 20));
        circ(cx, cy - u * 0.2f, u * 0.25f, C(120, 200, 230));
        return true;
    }
    if (chave == "bucaneiro") {
        poli({{cx, cy - u * 1.9f}, {cx + u * 0.8f, cy - u * 0.8f}, {cx + u * 0.8f, cy + u * 1.5f},
              {cx - u * 0.8f, cy + u * 1.5f}, {cx - u * 0.8f, cy - u * 0.8f}},
             C(90, 55, 25));
        poli({{cx, cy - u * 1.6f}, {cx + u * 0.62f, cy - u * 0.7f}, {cx + u * 0.62f, cy + u * 1.3f},
              {cx - u * 0.62f, cy + u * 1.3f}, {cx - u * 0.62f, cy - u * 0.7f}},
             C(140, 90, 45));
        rect(cx - u * 0.9f, cy - u * 0.5f, u * 1.8f, u * 0.5f, C(245, 240, 225));
        macaco_base(cx, cy + u * 0.6f, u * 0.45f, PELO, ROSTO);
        return true;
    }
    if (chave == "as" || chave == "fenix") {
        Color corpo = chave == "as" ? C(240, 200, 40) : C(250, 120, 30);
        poli({{cx - u * 1.8f, cy}, {cx + u * 1.8f, cy}, {cx, cy - u * 0.5f}}, sombra(corpo));
        poli({{cx - u * 1.7f, cy - 2}, {cx + u * 1.7f, cy - 2}, {cx, cy - u * 0.6f}}, corpo);
        elip(cx - u * 0.35f, cy - u * 1.4f, u * 0.7f, u * 2.6f, sombra(corpo, 0.85f));
        poli({{cx - u * 0.7f, cy + u * 1.1f}, {cx + u * 0.7f, cy + u * 1.1f}, {cx, cy + u * 0.7f}}, corpo);
        circ(cx, cy - u * 0.6f, u * 0.22f, C(120, 200, 230));
        return true;
    }
    if (chave == "heli") {
        elip(cx - u * 0.6f, cy - u * 1.0f, u * 1.2f, u * 1.8f, C(60, 90, 50));
        rect(cx - 3, cy + u * 0.5f, 6, u * 1.3f, C(60, 90, 50));
        circ(cx, cy - u * 0.5f, u * 0.3f, C(120, 200, 230));
        linha(cx - u * 1.7f, cy - u * 0.2f, cx + u * 1.7f, cy + u * 0.2f, 3, C(40, 40, 40));
        linha(cx - u * 0.2f, cy - u * 1.7f, cx + u * 0.2f, cy + u * 1.3f, 3, C(40, 40, 40));
        return true;
    }
    if (chave == "morteiro") {
        macaco_base(cx - u * 0.5f, cy + u * 0.3f, u * 0.6f, PELO, ROSTO);
        circ(cx + u * 0.5f, cy - u * 0.2f, u * 0.7f, C(70, 90, 60));
        circ(cx + u * 0.5f, cy - u * 0.2f, u * 0.45f, C(30, 30, 30));
        return true;
    }
    if (chave == "dartling") {
        macaco_base(cx, cy + u * 0.5f, u * 0.65f, PELO, ROSTO);
        rect(cx - u * 0.5f, cy - u * 1.6f, u * 1.0f, u * 1.5f, C(70, 80, 90), 4);
        for (int k : {-1, 0, 1}) linha(cx + k * 6, cy - u * 1.6f, cx + k * 6, cy - u * 2.1f, 3, C(30, 30, 30));
        return true;
    }
    if (chave == "fazenda") {
        rect(cx - u * 1.3f, cy - u * 0.2f, u * 2.6f, u * 1.3f, C(120, 80, 40));
        poli({{cx - u * 1.5f, cy - u * 0.2f}, {cx + u * 1.5f, cy - u * 0.2f}, {cx, cy - u * 1.3f}}, C(190, 60, 40));
        for (int k = 0; k < 3; ++k) {
            float x = cx - u * 0.9f + k * u * 0.9f;
            arco(x - 6, cy + u * 0.1f, 12, 18, 0.5f, 2.6f, 4, C(250, 220, 50));
        }
        circ(cx + u * 1.1f, cy - u * 1.0f, u * 0.5f, C(60, 150, 50));
        return true;
    }
    if (chave == "espinhos") {
        rect(cx - u * 1.1f, cy - u * 1.1f, u * 2.2f, u * 2.2f, C(90, 90, 100), 6);
        rect(cx - u, cy - u, 2 * u, 2 * u, C(150, 150, 160), 6);
        circ(cx, cy, u * 0.6f, C(70, 70, 80));
        for (int k = 0; k < 8; ++k) {
            float a = k * PI_F / 4;
            linha(cx, cy, cx + std::cos(a) * u * 0.55f, cy + std::sin(a) * u * 0.55f, 2, C(40, 40, 40));
        }
        return true;
    }
    if (chave == "vila") {
        circ(cx, cy, u * 1.35f, C(120, 80, 40));
        std::vector<Vector2> hex;
        for (int k = 0; k < 6; ++k) hex.push_back({cx + std::cos(k * PI_F / 3) * u * 1.3f, cy + std::sin(k * PI_F / 3) * u * 1.3f});
        poli(hex, C(200, 170, 90));
        circ(cx, cy, u * 0.5f, C(170, 130, 60));
        circ(cx, cy - u * 0.2f, u * 0.2f, C(220, 60, 50));
        return true;
    }
    if (chave == "sentinela") {
        circ(cx, cy, u * 0.9f, C(60, 60, 70));
        circ(cx, cy, u * 0.7f, C(220, 180, 40));
        rect(cx - 3, cy - u * 1.4f, 6, u * 1.2f, C(60, 60, 70));
        return true;
    }
    if (chave == "churchill") {
        rect(cx - u * 1.3f, cy - u * 1.2f, u * 2.6f, u * 2.4f, C(40, 50, 40), 8);
        rect(cx - u * 1.1f, cy - u, u * 2.2f, 2 * u, C(90, 110, 80), 8);
        rect(cx - 4, cy - u * 2.0f, 8, u * 1.6f, C(60, 70, 55));
        macaco_base(cx, cy, u * 0.55f, C(110, 80, 50), ROSTO);
        return true;
    }
    return false;
}

void retrato(const std::string& chave, float cx, float cy, float u) {
    if (torre_especial(cx, cy, u, chave)) return;
    auto it = estilos().find(chave);
    Estilo e = it != estilos().end() ? it->second : Estilo{PELO, ROSTO, nullptr, nullptr, BLANK};
    if (chave == "pat") u *= 1.25f;
    macaco_base(cx, cy, u, e.pelo, e.rosto);
    chapeu(cx, cy, u, e.chapeu, e.cor);
    item(cx, cy, u, e.item, e.cor);
}

const Color COR_TIER[6] = {BLANK, BLANK, BLANK, C(205, 127, 50), C(200, 205, 215), C(255, 215, 60)};

// ================================================================== BLOONS
void desenhar_bloon(float cx, float cy, float r, const TipoBloon& t, bool camo, bool regen, bool fort, int dano) {
    const Color c = cor(t.cor);
    const float w = std::floor(r * 1.75f), h = std::floor(r * 2.1f);
    const float rx = cx - w / 2, ry = cy - h / 2;
    const std::string& tipo = t.nome;
    // no
    poli({{cx - 3, ry + h - 2}, {cx + 3, ry + h - 2}, {cx, ry + h + 4}}, sombra(c, 0.6f));
    elip(rx - 1.5f, ry - 1.5f, w + 3, h + 3, tipo != "preto" ? sombra(c, 0.45f) : C(70, 70, 80));
    if (tipo == "arco_iris") {
        const Color faixas[6] = {C(230, 40, 40), C(250, 140, 20), C(250, 220, 30),
                                 C(60, 190, 60), C(40, 140, 235), C(150, 60, 200)};
        por_linhas(cx, cy, w / 2, h / 2, [&](float y, float xl, float xr) {
            int i = std::clamp(static_cast<int>((y - ry) / (h / 6)), 0, 5);
            faixa(y, xl, xr, faixas[i]);
        });
    } else {
        elip(rx, ry, w, h, c);
    }
    if (tipo == "zebra") {
        // listras diagonais recortadas no formato do bloon
        const float esp = std::max(3.0f, std::floor(r / 4));
        const float m = (h / 3) / w;  // inclinacao
        const float meia = esp / 2 * std::sqrt(1 + m * m) / m;
        por_linhas(cx, cy, w / 2, h / 2, [&](float y, float xl, float xr) {
            for (int k = -2; k < 4; ++k) {
                float xc = rx + (y - ry - k * h / 3) / m;
                faixa(y, std::max(xl, xc - meia), std::min(xr, xc + meia), C(25, 25, 30));
            }
        });
    }
    if (tipo == "chumbo") elip(rx + w / 6 - w / 8, ry + h / 6 - h / 8, w - w / 3, h - h / 3, C(170, 175, 185));
    if (tipo == "ceramica") {
        elip(rx + w / 5, ry + h / 5, w - w / 2.5f, h - h / 2.5f, C(205, 140, 80));
        for (int k = 0; k < dano; ++k) {
            float a = 0.8f + k * 1.7f;
            linha(cx, cy, cx + std::cos(a) * r, cy + std::sin(a) * r, 2, C(80, 40, 15));
        }
    }
    // brilho
    const Color brilho = tipo != "preto" ? C(255, 255, 255, 150) : C(120, 120, 130);
    elip(cx - w / 3, cy - h / 3, std::max(3.0f, std::floor(w / 4)), std::max(4.0f, std::floor(h / 3)), brilho);
    if (fort) {
        elip_l(rx, ry, w, h, 3, C(120, 120, 130));
        linha(rx + 2, cy, rx + w - 2, cy, 3, C(120, 120, 130));
        linha(cx, ry + 2, cx, ry + h - 2, 3, C(120, 120, 130));
    }
    if (camo) {
        std::mt19937 rng(static_cast<unsigned>(std::hash<std::string>{}(tipo) & 0xffff));
        const Color manchas[3] = {C(60, 90, 40), C(110, 130, 60), C(80, 60, 30)};
        for (int i = 0; i < 7; ++i) {
            float x = rx + static_cast<float>(rng() % static_cast<unsigned>(w + 1));
            float y = ry + static_cast<float>(rng() % static_cast<unsigned>(h + 1));
            Color mc = manchas[rng() % 3];
            float mw = 10 + static_cast<float>(rng() % 7), mh = 8;
            // mancha eliptica recortada pela elipse do bloon
            const float mcx = x - 5 + mw / 2, mcy = y - 4 + mh / 2;
            por_linhas(mcx, mcy, mw / 2, mh / 2, [&](float yy, float xl, float xr) {
                float t = (yy - cy) / (h / 2);
                if (t * t >= 1) return;
                float meio = w / 2 * std::sqrt(1 - t * t);
                faixa(yy, std::max(xl, cx - meio), std::min(xr, cx + meio), mc);
            });
        }
    }
    if (regen) {
        for (int k = 0; k < 10; ++k) {
            float a = k * PI_F / 5;
            circ(cx + std::cos(a) * w * 0.55f, cy + std::sin(a) * h * 0.5f, 2, C(255, 110, 170));
        }
    }
}

void desenhar_dirigivel(const TipoBloon& t, bool fort, int dano) {
    const float comp = std::floor(static_cast<float>(t.raio) * 2.6f);
    const float alt = std::floor(static_cast<float>(t.raio) * 1.35f);
    const float ox = 15, oy = 15;
    const Color c = cor(t.cor);
    const Color escuro = sombra(c, 0.55f);
    const std::string& tipo = t.nome;
    // aletas
    for (int sy : {-1, 1})
        poli({{ox + 6, oy + alt / 2}, {ox - 8, oy + alt / 2 + sy * alt / 1.6f}, {ox + comp / 4, oy + alt / 2}}, escuro);
    elip(ox - 2, oy - 2, comp + 4, alt + 4, escuro);
    elip(ox, oy, comp, alt, c);
    const Color faixa_c = (tipo == "moab" || tipo == "bfb" || tipo == "bad") ? C(240, 240, 245) : C(30, 30, 30);
    rect(ox + comp / 5, oy + alt / 2 - alt / 10, comp * 3 / 5, alt / 5, faixa_c);
    elip(ox + comp / 6, oy + alt / 8, comp / 2, alt / 5, clarear(c, 0.4f));
    // "olho" na frente
    circ(ox + comp - comp / 6, oy + alt / 2, std::max(4.0f, alt / 7), C(250, 250, 250));
    circ(ox + comp - comp / 6 + 2, oy + alt / 2, std::max(2.0f, alt / 14), C(20, 20, 20));
    if (tipo == "ddt") {
        std::mt19937 rng(7);
        for (int i = 0; i < 10; ++i) {
            float x = ox + 10 + static_cast<float>(rng() % static_cast<unsigned>(comp - 19));
            float y = oy + 6 + static_cast<float>(rng() % static_cast<unsigned>(alt - 11));
            circ(x, y, 3 + static_cast<float>(rng() % 5), C(70, 80, 60));
        }
    }
    if (fort) {
        for (int k = 1; k < 5; ++k) {
            float x = ox + k * comp / 5;
            linha(x, oy + 4, x, oy + alt - 4, 3, C(150, 150, 160));
        }
    }
    for (int k = 0; k < dano; ++k) {
        float x0 = ox + comp / 3 + k * comp / 7;
        linhas({{x0, oy + 6}, {x0 + 6, oy + alt / 3}, {x0 - 4, oy + alt / 2}, {x0 + 5, oy + alt - 8}}, 2,
               C(20, 20, 20));
    }
}

// ================================================================== MAPA
void desenhar_mapa(const Mapa& mp) {
    const DefMapa& m = mp.def;
    const Color grama = cor(m.grama), terra = cor(m.terra);
    rect(0, 0, LARGURA_MAPA, ALTURA_MAPA, grama);
    std::mt19937 rng(static_cast<unsigned>(m.chave.size() * 31 + 7));
    auto uni = [&](int a, int b) { return a + static_cast<int>(rng() % static_cast<unsigned>(b - a + 1)); };
    auto ruido = [&](Color c, int n, int raio) {
        for (int i = 0; i < n; ++i) {
            float x = static_cast<float>(uni(0, LARGURA_MAPA)), y = static_cast<float>(uni(0, ALTURA_MAPA));
            circ(x, y, static_cast<float>(uni(raio / 2, raio)), c);
        }
    };
    ruido(sombra(grama, 0.93f), 220, 34);
    ruido(clarear(grama, 0.08f), 180, 26);
    for (int i = 0; i < 900; ++i) {
        float x = static_cast<float>(uni(0, LARGURA_MAPA)), y = static_cast<float>(uni(0, ALTURA_MAPA));
        linha(x, y, x + uni(-3, 3), y - uni(4, 8), 2, sombra(grama, 0.75f));
    }
    // agua
    for (auto& [cx, cy, r0] : m.agua) {
        float x = static_cast<float>(cx), y = static_cast<float>(cy), r = static_cast<float>(r0);
        circ(x, y, r + 10, C(70, 140, 60));
        circ(x, y, r + 5, C(230, 215, 160));
        circ(x, y, r, C(40, 130, 210));
        circ(x, y, r * 0.8f, C(70, 160, 230));
        for (int k = 0; k < 8; ++k) {
            float a = k * 0.8f;
            float px = x + std::cos(a) * r * 0.5f, py = y + std::sin(a) * r * 0.5f;
            arco(px - 14, py - 5, 28, 10, 0.3f, 2.8f, 2, C(150, 200, 245));
        }
    }
    for (auto& [rx, ry, rw, rh] : m.agua_ret) {
        float x = static_cast<float>(rx), y = static_cast<float>(ry), w = static_cast<float>(rw),
              h = static_cast<float>(rh);
        rect(x - 5, y - 5, w + 10, h + 10, C(230, 215, 160), 18);
        rect(x, y, w, h, C(40, 130, 210), 14);
        rect(x + 10, y + 10, w - 20, h - 20, C(70, 160, 230), 12);
    }
    // trilha
    for (const Caminho& cam : mp.caminhos) {
        const float larguras[3] = {static_cast<float>(LARGURA_TRILHA) + 10, static_cast<float>(LARGURA_TRILHA),
                                   static_cast<float>(LARGURA_TRILHA) - 18};
        const Color cores[3] = {sombra(terra, 0.65f), terra, clarear(terra, 0.12f)};
        for (int k = 0; k < 3; ++k) {
            for (size_t i = 1; i < cam.pontos.size(); ++i)
                DrawLineEx({static_cast<float>(cam.pontos[i - 1].x), static_cast<float>(cam.pontos[i - 1].y)},
                           {static_cast<float>(cam.pontos[i].x), static_cast<float>(cam.pontos[i].y)}, larguras[k],
                           cores[k]);
            for (auto& p : cam.pontos) circ(static_cast<float>(p.x), static_cast<float>(p.y), larguras[k] / 2, cores[k]);
        }
        for (size_t i = 0; i < cam.amostras.size(); i += 3) {
            if (rng() % 2) {
                auto& a = cam.amostras[i];
                circ(static_cast<float>(a.x) + uni(-14, 14), static_cast<float>(a.y) + uni(-14, 14),
                     static_cast<float>(uni(1, 3)), sombra(terra, 0.8f));
            }
        }
    }
    // obstaculos
    for (auto& o : m.obstaculos) {
        float ox = static_cast<float>(o.x), oy = static_cast<float>(o.y), r = static_cast<float>(o.r);
        if (o.tipo == "arvore") {
            circ(ox + 6, oy + 8, r, C(40, 70, 30));
            for (int k = 0; k < 6; ++k) {
                float a = k * PI_F / 3;
                circ(ox + std::cos(a) * r * 0.45f, oy + std::sin(a) * r * 0.45f, r * 0.6f, C(46, 110, 40));
            }
            circ(ox - 4, oy - 4, r * 0.55f, C(60, 135, 50));
            circ(ox - 10, oy - 10, r * 0.25f, C(90, 165, 70));
        } else {
            std::vector<Vector2> pts, sp, bp;
            for (int k = 0; k < 6; ++k)
                pts.push_back({ox + std::cos(k * 1.1f) * r * (0.8f + 0.2f * (k % 2)), oy + std::sin(k * 1.1f) * r * 0.75f});
            for (auto& p : pts) sp.push_back({p.x + 4, p.y + 5}), bp.push_back({p.x * 0.6f + ox * 0.4f - 4, p.y * 0.6f + oy * 0.4f - 4});
            poli(sp, C(90, 90, 95));
            poli(pts, C(150, 150, 155));
            poli(bp, C(185, 185, 190));
        }
    }
}

// ================================================================== PROJETEIS
const std::map<std::string, Color>& cores_proj() {
    static const std::map<std::string, Color> m = {
        {"dardo", C(70, 70, 80)},        {"flecha", C(110, 70, 30)},        {"prego", C(120, 120, 130)},
        {"tachinha", C(90, 90, 100)},    {"lamina", C(210, 210, 220)},      {"bala", C(240, 210, 80)},
        {"magia", C(190, 90, 255)},      {"fogo", C(255, 140, 30)},         {"laser", C(255, 50, 50)},
        {"plasma", C(255, 90, 220)},     {"sol", C(255, 230, 80)},          {"escuro", C(60, 20, 90)},
        {"shuriken", C(180, 180, 190)},  {"pocao", C(130, 220, 80)},        {"espinho", C(80, 140, 40)},
        {"cola", C(150, 210, 60)},       {"gelo_bola", C(170, 230, 255)},   {"uva", C(130, 50, 150)},
        {"uva_fogo", C(255, 110, 40)},   {"bala_canhao", C(40, 40, 45)},    {"bomba", C(30, 30, 35)},
        {"missil", C(200, 200, 205)},    {"abacaxi", C(230, 200, 40)},      {"meteoro", C(255, 100, 20)},
        {"tornado", C(220, 230, 240)},   {"balista", C(120, 80, 40)},       {"drone", C(90, 110, 140)},
        {"espirito", C(120, 255, 200)},  {"luz", C(255, 250, 180)},         {"maldicao", C(150, 40, 150)},
        {"juggernaut", C(110, 110, 115)}, {"bola_espinho", C(120, 120, 125)}, {"glaive", C(230, 230, 240)},
        {"kylie", C(240, 200, 40)},      {"bumerangue", C(240, 200, 40)},   {"fragmento", C(60, 60, 60)},
        {"fragmento_gelo", C(190, 240, 255)}, {"aviaozinho", C(230, 200, 40)}, {"flash", C(255, 255, 255)},
    };
    return m;
}

const std::map<std::string, Color>& cores_pilha() {
    static const std::map<std::string, Color> m = {
        {"espinhos", C(120, 120, 130)}, {"bola_espinho", C(110, 110, 115)}, {"estrepe", C(80, 80, 90)},
        {"chamas", C(255, 120, 30)},    {"acido", C(140, 220, 60)},         {"cipo", C(50, 140, 50)},
        {"zumbi", C(120, 160, 120)},    {"espuma", C(230, 240, 255)},       {"armadilha", C(90, 70, 50)},
        {"espinheiro", C(60, 110, 40)},
    };
    return m;
}

bool em(const std::string& v, std::initializer_list<const char*> lista) {
    for (const char* s : lista)
        if (v == s) return true;
    return false;
}

}  // namespace

// ================================================================== interface publica
void torre(const std::string& chave, float x, float y, int tam, int tier, float rotacao, unsigned char alfa) {
    const std::string k = "t:" + chave + ":" + std::to_string(tam) + ":" + std::to_string(tier);
    const Sprite& s = gerar(k, static_cast<float>(tam), static_cast<float>(tam), [&] {
        const float c = std::floor(tam / 2.0f);
        const float u = std::max(4.0f, std::floor(tam * 0.2f));
        if (tier >= 3) {
            Color ct = COR_TIER[std::min(tier, 5)];
            circ(c, c, std::floor(tam * 0.47f), ui::com_alfa(ct, 110));
            circ_l(c, c, std::floor(tam * 0.47f), 2, ct);
        }
        retrato(chave, c, c, u);
    });
    desenhar_sprite(s, x, y, 1, rotacao, alfa);
}

void bloon(const TipoBloon& t, bool camo, bool regen, bool fort, int dano, float x, float y, float escala) {
    const std::string k = "b:" + t.nome + ":" + std::to_string(camo) + std::to_string(regen) + std::to_string(fort) +
                          std::to_string(dano);
    const float r = std::floor(static_cast<float>(t.raio));
    const float tam = r * 3 + 8;
    const Sprite& s = gerar(k, tam, tam, [&] {
        desenhar_bloon(std::floor(tam / 2), std::floor(tam / 2), r, t, camo, regen, fort, dano);
    });
    desenhar_sprite(s, x, y, escala, 0, 255);
}

Vector2 tamanho_dirigivel(const TipoBloon& t) {
    return {std::floor(static_cast<float>(t.raio) * 2.6f) + 30, std::floor(static_cast<float>(t.raio) * 1.35f) + 30};
}

void dirigivel(const TipoBloon& t, bool fort, int dano, float x, float y, float ang, float escala) {
    const std::string k = "d:" + t.nome + ":" + std::to_string(fort) + std::to_string(dano);
    Vector2 tam = tamanho_dirigivel(t);
    const Sprite& s = gerar(k, tam.x, tam.y, [&] { desenhar_dirigivel(t, fort, dano); });
    desenhar_sprite(s, x, y, escala, ang, 255);
}

void fundo_mapa(const std::string& chave, Rectangle destino) {
    const Sprite& s = gerar("m:" + chave, LARGURA_MAPA, ALTURA_MAPA, [&] { desenhar_mapa(mapa(chave)); });
    Rectangle src{0, 0, static_cast<float>(s.rt.texture.width), -static_cast<float>(s.rt.texture.height)};
    DrawTexturePro(s.rt.texture, src, destino, {0, 0}, 0, WHITE);
}

void icone_coracao(float x, float y, int tam) {
    const Sprite& s = gerar("i:coracao:" + std::to_string(tam), static_cast<float>(tam), static_cast<float>(tam), [&] {
        const float t = static_cast<float>(tam), r = std::floor(t / 4);
        const Color cores[2] = {C(120, 10, 20), C(230, 40, 60)};
        const float encs[2] = {0, 2};
        for (int i = 0; i < 2; ++i) {
            float enc = encs[i];
            circ(std::floor(t / 2) - r + 1, std::floor(t / 3) + 1, r + 1 - enc, cores[i]);
            circ(std::floor(t / 2) + r - 1, std::floor(t / 3) + 1, r + 1 - enc, cores[i]);
            poli({{2 + enc, std::floor(t / 3) + 2}, {t - 2 - enc, std::floor(t / 3) + 2}, {std::floor(t / 2), t - 3 - enc}},
                 cores[i]);
        }
        circ(std::floor(t / 2) - r, std::floor(t / 3) - 2, std::max(2.0f, std::floor(r / 3)), C(255, 170, 180));
    });
    desenhar_sprite(s, x + tam / 2.0f, y + tam / 2.0f, 1, 0, 255);
}

void icone_moeda(float x, float y, int tam) {
    const float c = std::floor(tam / 2.0f);
    circ(x + c, y + c, c - 1, C(160, 110, 10));
    circ(x + c, y + c, c - 3, C(250, 200, 40));
    circ_l(x + c, y + c, c - 7, 2, C(255, 230, 120));
    ui::texto("$", x + c, y + c + 1, static_cast<int>(tam * 0.5f), C(170, 120, 10), 0, ui::Ancora::CENTER);
}

void icone_eco(float x, float y, int tam) {
    const float t = static_cast<float>(tam), c = std::floor(t / 2);
    circ(x + c, y + c, c - 1, C(20, 110, 40));
    circ(x + c, y + c, c - 3, C(60, 190, 80));
    // seta para cima
    poli({{x + c, y + 4}, {x + t - 6, y + c}, {x + 6, y + c}}, C(240, 255, 240));
    rect(x + c - 3, y + c - 1, 6, c - 3, C(240, 255, 240));
}

void projetil(const Projetil& p, float ox, float oy, float escala) {
    const std::string& v = p.at->visual;
    const float x = ox + static_cast<float>(p.x) * escala, y = oy + static_cast<float>(p.y) * escala;
    auto it = cores_proj().find(v);
    const Color c = it != cores_proj().end() ? it->second : C(60, 60, 60);
    const float r = std::max(2.0f, std::floor(static_cast<float>(p.raio) * escala));
    if (escala < 0.5f) {
        circ(x, y, std::max(1.0f, std::floor(r / 2)), c);
        return;
    }
    const float a = static_cast<float>(p.ang) * PI_F / 180, ca = std::cos(a), sa = std::sin(a);
    if (em(v, {"dardo", "flecha", "prego", "tachinha", "bala", "fragmento", "laser", "espinho", "fragmento_gelo"})) {
        float comp = v == "laser" ? 18 : v == "bala" ? 8 : v == "tachinha" ? 9 : v == "fragmento" ? 7 : 13;
        float larg = v != "laser" ? 3 : 4;
        DrawLineEx({x - ca * comp, y - sa * comp}, {x + ca * 4, y + sa * 4}, larg, c);
        if (v == "dardo" || v == "flecha")
            DrawLineEx({x - ca * comp, y - sa * comp}, {x - ca * (comp - 4), y - sa * (comp - 4)}, 5, C(220, 60, 50));
    } else if (em(v, {"bumerangue", "kylie", "glaive", "shuriken"})) {
        float t = static_cast<float>(ui::tempo() * 1000 / 60.0);
        int bracos = v == "shuriken" ? 4 : v == "glaive" ? 3 : 2;
        for (int k = 0; k < bracos; ++k) {
            float ang = t + k * 2 * PI_F / bracos;
            DrawLineEx({x, y}, {x + std::cos(ang) * (r + 6), y + std::sin(ang) * (r + 6)}, 4, c);
        }
        circ(x, y, 3, sombra(c));
    } else if (v == "missil") {
        DrawLineEx({x - ca * 12, y - sa * 12}, {x + ca * 6, y + sa * 6}, 5, c);
        circ(x - ca * 14, y - sa * 14, 4, C(255, 160, 40));
    } else if (v == "juggernaut" || v == "bola_espinho") {
        circ(x, y, r + 2, C(70, 70, 75));
        circ(x, y, r, c);
        for (int k = 0; k < 8; ++k) {
            float ang = k * PI_F / 4;
            DrawLineEx({x + std::cos(ang) * r, y + std::sin(ang) * r},
                       {x + std::cos(ang) * (r + 5), y + std::sin(ang) * (r + 5)}, 3, C(60, 60, 60));
        }
    } else if (v == "tornado") {
        for (int k = 0; k < 4; ++k) elip_l(x - r + k * 2, y - r + k * 5, 2 * r - k * 4, 8, 2, c);
    } else {
        circ(x, y, r + 1, sombra(c, 0.6f));
        circ(x, y, r, c);
        if (em(v, {"fogo", "magia", "plasma", "sol", "luz", "espirito", "meteoro"}))
            circ(x, y, std::max(1.0f, std::floor(r / 2)), clarear(c, 0.6f));
        if ((v == "bomba" || v == "bala_canhao") && p.fusivel > 0) circ_l(x, y, r + 3, 2, C(255, 80, 40));
    }
}

void pilha(const Pilha& s, float ox, float oy, float escala) {
    const float x = std::floor(ox + static_cast<float>(s.x) * escala), y = std::floor(oy + static_cast<float>(s.y) * escala);
    auto it = cores_pilha().find(s.visual);
    const Color c = it != cores_pilha().end() ? it->second : C(120, 120, 130);
    if (escala < 0.5f) {
        circ(x, y, 2, c);
        return;
    }
    if (em(s.visual, {"chamas", "acido", "espuma"})) {
        circ(x, y, 12, sombra(c, 0.7f));
        circ(x, y, 9, c);
    } else if (s.visual == "armadilha") {
        rect(x - 12, y - 12, 24, 24, c, 4);
        ui::ret_linha({x - 12, y - 12, 24, 24}, C(40, 40, 40), 2, 4);
    } else {
        for (int k = 0; k < 5; ++k) {
            float a = k * 1.25f;
            float px = x + std::cos(a) * 6, py = y + std::sin(a) * 6;
            std::vector<Vector2> tri{{px, py - 5}, {px + 4, py + 3}, {px - 4, py + 3}};
            poli(tri, c);
            poli_l(tri, 1, sombra(c, 0.6f));
        }
    }
}

void liberar() {
    for (auto& [k, s] : cache) UnloadRenderTexture(s.rt);
    cache.clear();
}

}  // namespace bl::arte
