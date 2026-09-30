#include "cliente/caneta.hpp"

#include <algorithm>
#include <cmath>

#include "cliente/ui.hpp"
#include "rlgl.h"

namespace bl::spr {

namespace {

constexpr float PI_F = 3.14159265358979f;

unsigned char u8(float v) { return static_cast<unsigned char>(std::clamp(std::lround(v), 0L, 255L)); }

void elipse_cheia(float cx, float cy, float rx, float ry, Color c) {
    if (rx <= 0 || ry <= 0 || c.a == 0) return;
    const int n = 44;
    rlBegin(RL_TRIANGLES);
    rlColor4ub(c.r, c.g, c.b, c.a);
    for (int i = 0; i < n; ++i) {
        const float a0 = 2 * PI_F * i / n, a1 = 2 * PI_F * (i + 1) / n;
        rlVertex2f(cx, cy);
        rlVertex2f(cx + std::cos(a1) * rx, cy + std::sin(a1) * ry);
        rlVertex2f(cx + std::cos(a0) * rx, cy + std::sin(a0) * ry);
    }
    rlEnd();
}

float area2(const std::vector<Vector2>& v) {
    float a = 0;
    for (size_t i = 0; i < v.size(); ++i) {
        const Vector2& p = v[i];
        const Vector2& q = v[(i + 1) % v.size()];
        a += p.x * q.y - q.x * p.y;
    }
    return a;
}

float cruz(Vector2 a, Vector2 b, Vector2 c) { return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x); }

bool dentro_tri(Vector2 p, Vector2 a, Vector2 b, Vector2 c) {
    const float d1 = cruz(a, b, p), d2 = cruz(b, c, p), d3 = cruz(c, a, p);
    const bool neg = d1 < 0 || d2 < 0 || d3 < 0, pos = d1 > 0 || d2 > 0 || d3 > 0;
    return !(neg && pos);
}

// Triangulacao por remocao de orelhas: vale para qualquer poligono simples (concavo ou nao).
void poligono_cheio(const std::vector<Vector2>& pts, Color c) {
    if (pts.size() < 3 || c.a == 0) return;
    std::vector<Vector2> v = pts;
    if (area2(v) < 0) std::reverse(v.begin(), v.end());  // sentido anti-horario (em y para baixo)
    rlBegin(RL_TRIANGLES);
    rlColor4ub(c.r, c.g, c.b, c.a);
    int guarda = 0;
    while (v.size() > 3 && guarda++ < 400) {
        bool cortou = false;
        for (size_t i = 0; i < v.size(); ++i) {
            const Vector2 a = v[(i + v.size() - 1) % v.size()], b = v[i], d = v[(i + 1) % v.size()];
            if (cruz(a, b, d) <= 0) continue;  // vertice reflexo
            bool vazio = true;
            for (size_t k = 0; k < v.size() && vazio; ++k) {
                const Vector2& p = v[k];
                if ((p.x == a.x && p.y == a.y) || (p.x == b.x && p.y == b.y) || (p.x == d.x && p.y == d.y)) continue;
                if (dentro_tri(p, a, b, d)) vazio = false;
            }
            if (!vazio) continue;
            rlVertex2f(a.x, a.y);
            rlVertex2f(d.x, d.y);
            rlVertex2f(b.x, b.y);
            v.erase(v.begin() + static_cast<long>(i));
            cortou = true;
            break;
        }
        if (!cortou) break;  // poligono estranho: termina em leque
    }
    Vector2 m{0, 0};
    for (auto& p : v) m.x += p.x / v.size(), m.y += p.y / v.size();
    for (size_t i = 0; i < v.size(); ++i) {
        const Vector2& p = v[i];
        const Vector2& q = v[(i + 1) % v.size()];
        if (v.size() == 3) {
            if (i) break;
            rlVertex2f(v[0].x, v[0].y);
            rlVertex2f(v[2].x, v[2].y);
            rlVertex2f(v[1].x, v[1].y);
            break;
        }
        rlVertex2f(m.x, m.y);
        rlVertex2f(q.x, q.y);
        rlVertex2f(p.x, p.y);
    }
    rlEnd();
}

void tracado(const std::vector<Vector2>& v, float w, Color c, bool fechado) {
    if (c.a == 0 || w <= 0) return;
    const size_t n = fechado ? v.size() : v.size() - 1;
    for (size_t i = 0; i < n; ++i) DrawLineEx(v[i], v[(i + 1) % v.size()], w, c);
    for (auto& p : v) DrawCircleV(p, w / 2, c);
}

}  // namespace

Color esc(Color c, float k) { return {u8(c.r * k), u8(c.g * k), u8(c.b * k), c.a}; }
Color clar(Color c, float k) {
    return {u8(c.r + (255 - c.r) * k), u8(c.g + (255 - c.g) * k), u8(c.b + (255 - c.b) * k), c.a};
}
Color al(Color c, float a) { return {c.r, c.g, c.b, u8(a * 255)}; }

float onda(float x) {
    x -= std::floor(x);
    return x < 0.5f ? x * 2 : 2 - x * 2;
}
float recuo(float x) {
    x -= std::floor(x);
    return x < 0.1f ? x / 0.1f : 1 - (x - 0.1f) / 0.9f;
}

// ---------------------------------------------------------------- passe
void Caneta::reiniciar(float alfa) {
    cont_anim_ = 0;
    prof_anim_ = 0;
    grupo_atual_ = -1;
    alfa_ = alfa;
    por_seg_.assign(1, 0);
}

bool Caneta::desenha() {
    switch (modo) {
        case Modo::TUDO: return true;
        case Modo::CONTAR:
            if (prof_anim_ == 0) {
                if (por_seg_.size() <= static_cast<size_t>(cont_anim_)) por_seg_.resize(cont_anim_ + 1, 0);
                ++por_seg_[cont_anim_];
            }
            return false;
        case Modo::ESTATICO: return prof_anim_ == 0 && cont_anim_ == alvo;
        case Modo::VIVO: return prof_anim_ > 0 && grupo_atual_ == alvo;
    }
    return false;
}

Color Caneta::cor(Color c) const { return {c.r, c.g, c.b, u8(c.a * alfa_)}; }

void Caneta::aplicar_anim(const Anim& a) {
    const float tt = t + a.atraso;
    const float f = a.dur > 0 ? tt / a.dur : 0;
    const float fase = f - std::floor(f);
    switch (a.t) {
        case Anim::NENHUMA: break;
        case Anim::GIRA: rlRotatef(std::fmod(tt * 360 / a.dur * a.dir, 360.0f), 0, 0, 1); break;
        case Anim::BALANCA: rlRotatef(a.amp * onda(f), 0, 0, 1); break;
        case Anim::FLUTUA: rlTranslatef(a.dx * onda(f), a.dy * onda(f), 0); break;
        case Anim::RECUO: rlTranslatef(0, a.dy * (em_jogo ? pose.recuo : recuo(f)), 0); break;
        case Anim::VOA: rlTranslatef(a.dx * fase, a.dy * fase, 0); break;
        case Anim::PULSA: {
            const float s = a.s0 + (a.s1 - a.s0) * onda(f);
            rlScalef(s, s, 1);
            break;
        }
        case Anim::CRESCE: {
            const float s = a.s0 + (a.s1 - a.s0) * fase;
            rlScalef(s, s, 1);
            break;
        }
        case Anim::PISCA: alfa_ *= a.a0 + (a.a1 - a.a0) * onda(f); break;
        case Anim::SOME: alfa_ *= fase < 0.4f ? 1 : 1 - (fase - 0.4f) / 0.6f; break;
        case Anim::BRACO:
            if (pose.ativa) {
                rlTranslatef(0, -pose.estica, 0);
                rlRotatef(pose.giro, 0, 0, 1);
            } else {
                rlRotatef(a.amp * onda(f), 0, 0, 1);
            }
            break;
    }
}

void Caneta::g(const G& gr, const std::function<void()>& fn) {
    const bool anim = gr.a.t != Anim::NENHUMA;
    const bool raiz = anim && prof_anim_ == 0;
    int meu = -1;
    if (raiz) {
        meu = cont_anim_++;
        if (modo == Modo::CONTAR || modo == Modo::ESTATICO) return;  // o grupo vai ao vivo
        if (modo == Modo::VIVO && meu != alvo) return;
    }
    rlPushMatrix();
    if (gr.x != 0 || gr.y != 0) rlTranslatef(gr.x, gr.y, 0);
    if (gr.rot != 0) rlRotatef(gr.rot, 0, 0, 1);
    if (gr.sx != 1 || gr.sy != 1) rlScalef(gr.sx, gr.sy, 1);
    const float alfa_antes = alfa_;
    const int grupo_antes = grupo_atual_;
    if (raiz) grupo_atual_ = meu;
    if (anim) {
        ++prof_anim_;
        if (modo == Modo::TUDO || modo == Modo::VIVO) aplicar_anim(gr.a);
    }
    fn();
    if (anim) --prof_anim_;
    grupo_atual_ = grupo_antes;
    alfa_ = alfa_antes;
    rlPopMatrix();
}

// ---------------------------------------------------------------- primitivas
void Caneta::c(float x, float y, float r, Color fill, float b) {
    if (!desenha()) return;
    if (b > 0) DrawCircleV({x, y}, r + b, cor(TINTA));
    if (fill.a) DrawCircleV({x, y}, r, cor(fill));
}

void Caneta::e(float x, float y, float rx, float ry, Color fill, float b, float rot) {
    if (!desenha()) return;
    rlPushMatrix();
    rlTranslatef(x, y, 0);
    if (rot != 0) rlRotatef(rot, 0, 0, 1);
    if (b > 0) elipse_cheia(0, 0, rx + b, ry + b, cor(TINTA));
    elipse_cheia(0, 0, rx, ry, cor(fill));
    rlPopMatrix();
}

void Caneta::rr(float x, float y, float w, float h, float raio, Color fill, float b, float rot) {
    if (!desenha()) return;
    rlPushMatrix();
    rlTranslatef(x + w / 2, y + h / 2, 0);
    if (rot != 0) rlRotatef(rot, 0, 0, 1);
    auto R = [&](float e) { return Rectangle{-w / 2 - e, -h / 2 - e, w + 2 * e, h + 2 * e}; };
    auto rd = [&](float e) { return std::min(1.0f, (raio + e) * 2 / (std::min(w, h) + 2 * e)); };
    if (fill.a == 0 && b > 0) {
        // "fill: none" com contorno: so a moldura
        DrawRectangleRoundedLinesEx(R(-b), rd(-b), 10, 2 * b, cor(TINTA));
    } else {
        if (b > 0) DrawRectangleRounded(R(b), rd(b), 10, cor(TINTA));
        if (raio <= 0) DrawRectangleRec(R(0), cor(fill));
        else DrawRectangleRounded(R(0), rd(0), 10, cor(fill));
    }
    rlPopMatrix();
}

void Caneta::poly(std::initializer_list<Vector2> pts, Color fill, float b) { poly(std::vector<Vector2>(pts), fill, b); }

void Caneta::poly(const std::vector<Vector2>& pts, Color fill, float b) {
    if (!desenha()) return;
    if (b > 0) {
        tracado(pts, 2 * b, cor(TINTA), true);
        poligono_cheio(pts, cor(TINTA));
    }
    poligono_cheio(pts, cor(fill));
}

void Caneta::star(float x, float y, float ro, float ri, int n, Color fill, float b, float rot) {
    std::vector<Vector2> v;
    for (int i = 0; i < n * 2; ++i) {
        const float a = (rot + i * 180.0f / n) * PI_F / 180, r = i % 2 ? ri : ro;
        v.push_back({x + std::cos(a) * r, y + std::sin(a) * r});
    }
    poly(v, fill, b);
}

void Caneta::ln(std::initializer_list<Vector2> pts, float w, Color c, float b) { ln(std::vector<Vector2>(pts), w, c, b); }

void Caneta::ln(const std::vector<Vector2>& pts, float w, Color c, float b) {
    if (!desenha() || pts.size() < 2) return;
    if (b > 0) tracado(pts, w + 2 * b, cor(TINTA), false);
    tracado(pts, w, cor(c), false);
}

void Caneta::sec(float x, float y, float r, float a0, float a1, Color fill, float b) {
    if (!desenha()) return;
    if (b > 0) DrawCircleSector({x, y}, r + b, a0 - 2, a1 + 2, 36, cor(TINTA));
    DrawCircleSector({x, y}, r, a0, a1, 36, cor(fill));
}

void Caneta::arc(float x, float y, float r, float a0, float a1, float w, Color c, float b) {
    if (!desenha()) return;
    auto ponta = [&](float a) { return Vector2{x + std::cos(a * PI_F / 180) * r, y + std::sin(a * PI_F / 180) * r}; };
    if (b > 0) {
        DrawRing({x, y}, r - w / 2 - b, r + w / 2 + b, a0, a1, 36, cor(TINTA));
        DrawCircleV(ponta(a0), w / 2 + b, cor(TINTA));
        DrawCircleV(ponta(a1), w / 2 + b, cor(TINTA));
    }
    DrawRing({x, y}, r - w / 2, r + w / 2, a0, a1, 36, cor(c));
    DrawCircleV(ponta(a0), w / 2, cor(c));
    DrawCircleV(ponta(a1), w / 2, cor(c));
}

void Caneta::ring(float x, float y, float ri, float ro, Color c) {
    if (!desenha()) return;
    DrawRing({x, y}, ri, ro, 0, 360, 48, cor(c));
}

void Caneta::ringE(float x, float y, float rx, float ry, float w, Color c) {
    if (!desenha()) return;
    rlPushMatrix();
    rlTranslatef(x, y, 0);
    rlScalef(1, ry / rx, 1);
    DrawRing({0, 0}, rx - w / 2, rx + w / 2, 0, 360, 48, cor(c));
    rlPopMatrix();
}

void Caneta::txt(float x, float y, const char* s, float tam, Color c, float b) {
    if (!desenha()) return;
    // a fonte do titulo (Lilita One) no tamanho 24 ja esta carregada; o resto e escala
    constexpr int BASE = 24;
    const float px = BASE * 1.32f;
    const float k = tam / px;
    rlPushMatrix();
    rlTranslatef(x, y, 0);
    rlScalef(k, k, 1);
    ui::texto(s, 0, 0, BASE, cor(c), static_cast<int>(std::lround(b / k * 0.5f)), ui::Ancora::CENTER, ui::Peso::TITULO,
              cor(TINTA));
    rlPopMatrix();
}

}  // namespace bl::spr
