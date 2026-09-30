#include "cliente/sprites.hpp"

#include <array>
#include <cmath>
#include <functional>
#include <map>
#include <optional>
#include <vector>

namespace bl::spr {

namespace {

constexpr float PI_F = 3.14159265358979f;
float rad(float g) { return g * PI_F / 180; }

const Color ROSTO = H(0xF2C99A), ROSTO_E = H(0xD9A273);
const Color BRANCO = H(0xFFFFFF);

using Desenho = std::function<void(Caneta&)>;

// ================================================================ projeteis (centro 0,0, +-28)
const std::map<std::string, Desenho>& PROJ() {
    static const std::map<std::string, Desenho> m = {
        {"dardo", [](Caneta& p) {
             p.rr(-2.5f, -18, 5, 30, 2, H(0x6B7480), b3);
             p.poly({{-6, -16}, {6, -16}, {0, -28}}, H(0xDDE2E8), b3);
             p.poly({{-7, 14}, {7, 14}, {0, 5}}, H(0xE8312A), 2);
         }},
        {"flecha", [](Caneta& p) {
             p.rr(-2, -20, 4, 38, 2, H(0x8A5A2E), b3);
             p.poly({{-6, -18}, {6, -18}, {0, -30}}, H(0xC7CDD6), b3);
             p.poly({{-6, 22}, {0, 12}, {6, 22}, {0, 17}}, H(0xF2F2F5), 2);
         }},
        {"bola_espinho", [](Caneta& p) {
             p.g(G().anim(Anim::gira(0.8f)), [&] {
                 p.star(0, 0, 20, 13, 12, H(0x8E97A4), b3);
                 p.c(0, 0, 11, H(0x6B7480), b3);
                 p.c(-3, -3, 3.5f, H(0xFFFFFF, 0x66));
             });
         }},
        {"juggernaut", [](Caneta& p) {
             p.g(G().anim(Anim::gira(1)), [&] {
                 p.star(0, 0, 28, 19, 14, H(0x6B7480), B);
                 p.c(0, 0, 17, H(0x3E4550), b3);
                 p.c(-5, -5, 5, H(0xFFFFFF, 0x55));
             });
         }},
        {"bumerangue", [](Caneta& p) {
             p.g(G().anim(Anim::gira(0.5f)), [&] { p.ln({{-16, 8}, {0, -12}, {16, 8}}, 7, H(0xF2C21A), b3); });
         }},
        {"glaive", [](Caneta& p) {
             p.g(G().anim(Anim::gira(0.4f)), [&] {
                 p.star(0, 0, 22, 8, 4, H(0xC7CDD6), b3, -45);
                 p.c(0, 0, 5, H(0x6B7480), 2);
             });
         }},
        {"kylie", [](Caneta& p) {
             p.g(G().anim(Anim::gira(0.5f)), [&] { p.ln({{-22, 10}, {0, -18}, {22, 10}}, 9, H(0xC98A0E), b3); });
         }},
        {"bomba", [](Caneta& p) {
             p.c(0, 2, 14, H(0x26242C), b3);
             p.c(-5, -3, 4, H(0xFFFFFF, 0x44));
             p.rr(-4, -16, 8, 6, 2, H(0x55555E), 2);
             p.ln({{0, -16}, {5, -22}}, 2.5f, H(0xC98A0E));
             p.g(g_em(6, -23).anim(Anim::pisca(0.3f, 1, 0.2f)), [&] { p.star(0, 0, 6, 2.5f, 6, H(0xFFD632)); });
         }},
        {"missil", [](Caneta& p) {
             p.g(g_em(0, 18).anim(Anim::pisca(0.5f, 1, 0.15f)), [&] {
                 p.poly({{-5, 0}, {5, 0}, {0, 14}}, H(0xF7941D), 2);
                 p.poly({{-2.5f, 0}, {2.5f, 0}, {0, 8}}, H(0xFFD632));
             });
             p.poly({{-5, 12}, {-11, 20}, {-5, 18}}, H(0xE8312A), 2);
             p.poly({{5, 12}, {11, 20}, {5, 18}}, H(0xE8312A), 2);
             p.rr(-5, -14, 10, 32, 5, H(0xE0E4EA), b3);
             p.poly({{-5, -12}, {5, -12}, {0, -24}}, H(0xE8312A), b3);
         }},
        {"tachinha", [](Caneta& p) {
             p.poly({{-3, -8}, {3, -8}, {0, -26}}, H(0xC7CDD6), 2);
             p.c(0, 0, 10, H(0xFF6FAE), b3);
             p.c(-3, -3, 3, H(0xFFFFFF, 0x77));
         }},
        {"fogo", [](Caneta& p) {
             p.g(G().anim(Anim::pulsa(0.9f, 1.12f, 0.3f)), [&] {
                 p.poly({{0, -24}, {12, -4}, {10, 10}, {0, 16}, {-10, 10}, {-12, -4}}, H(0xF7941D), b3);
                 p.poly({{0, -12}, {6, 2}, {0, 10}, {-6, 2}}, H(0xFFD632));
             });
         }},
        {"anel_fogo", [](Caneta& p) {
             p.g(G().anim(Anim::pulsa(0.8f, 1.1f, 0.4f)), [&] {
                 p.ring(0, 0, 15, 23, H(0xF7941D));
                 p.ring(0, 0, 18, 20, H(0xFFD632));
             });
         }},
        {"lamina", [](Caneta& p) {
             p.g(G().anim(Anim::gira(0.5f)), [&] { p.poly({{0, -24}, {5, -4}, {0, 20}, {-5, -4}}, H(0xDDE2E8), b3); });
         }},
        {"fragmento_gelo", [](Caneta& p) {
             p.poly({{0, -20}, {8, -4}, {0, 18}, {-8, -4}}, H(0x8FD3FF), b3);
             p.poly({{0, -12}, {3, -4}, {0, 4}, {-3, -4}}, BRANCO);
         }},
        {"gelo_bola", [](Caneta& p) {
             p.c(0, 0, 15, H(0xBFE6FF), b3);
             p.star(0, 0, 10, 3.5f, 6, BRANCO);
         }},
        {"cola", [](Caneta& p) {
             p.poly({{0, -18}, {12, 4}, {8, 14}, {-8, 14}, {-12, 4}}, H(0x9BE66E), b3);
             p.c(-4, 4, 3, H(0xFFFFFF, 0x88));
         }},
        {"bala", [](Caneta& p) {
             p.rr(-4, -10, 8, 20, 4, H(0xC98A0E), b3);
             p.rr(-4, -10, 8, 7, 4, H(0x8E97A4));
         }},
        {"bala_canhao", [](Caneta& p) {
             p.c(0, 0, 13, H(0x3E4550), b3);
             p.c(-4, -4, 4, H(0xFFFFFF, 0x44));
         }},
        {"uva", [](Caneta& p) {
             for (Vector2 q : {Vector2{-6, -2}, Vector2{6, -2}, Vector2{0, 8}, Vector2{0, -11}}) p.c(q.x, q.y, 7, H(0x8B3FD9), 2);
             p.ln({{0, -17}, {4, -24}}, 2.5f, H(0x3F9A3A));
         }},
        {"abacaxi", [](Caneta& p) {
             p.poly({{-8, -12}, {0, -28}, {8, -12}}, H(0x3F9A3A), 2);
             p.e(0, 4, 12, 16, H(0xE0A020), b3);
             p.ln({{-8, -4}, {8, 12}}, 1.5f, H(0x8A5A2E));
             p.ln({{8, -4}, {-8, 12}}, 1.5f, H(0x8A5A2E));
         }},
        {"plasma", [](Caneta& p) {
             p.g(G().anim(Anim::pulsa(0.85f, 1.1f, 0.4f)), [&] {
                 p.c(0, 0, 17, al(H(0xB47CFF), 0.4f));
                 p.c(0, 0, 10, H(0xB47CFF), b3);
                 p.c(0, 0, 5, BRANCO);
             });
         }},
        {"laser", [](Caneta& p) {
             p.rr(-5, -26, 10, 52, 5, al(H(0xFF4D5E), 0.45f));
             p.rr(-2, -24, 4, 48, 2, H(0xFFD1D5));
         }},
        {"raio_plasma", [](Caneta& p) {
             p.rr(-6, -26, 12, 52, 6, al(H(0xB47CFF), 0.45f));
             p.rr(-2.5f, -24, 5, 48, 2.5f, H(0xF0DCFF));
         }},
        {"magia", [](Caneta& p) {
             p.g(G().anim(Anim::gira(1.2f)), [&] {
                 p.star(0, 0, 16, 7, 5, H(0xB47CFF), b3);
                 p.c(0, 0, 5, BRANCO);
             });
         }},
        {"chamas", [](Caneta& p) {
             p.g(G().anim(Anim::pisca(0.6f, 1, 0.2f)), [&] {
                 p.e(0, 0, 20, 12, H(0xF7941D), b3);
                 p.e(4, 0, 12, 7, H(0xFFD632));
             });
         }},
        {"sol", [](Caneta& p) {
             p.g(G().anim(Anim::gira(3)), [&] { p.star(0, 0, 20, 12, 10, H(0xFFD632), b3); });
             p.c(0, 0, 10, H(0xFFF3B0), 2);
         }},
        {"escuro", [](Caneta& p) {
             p.g(G().anim(Anim::pulsa(0.9f, 1.1f, 0.5f)), [&] {
                 p.c(0, 0, 16, al(H(0x6A3FC4), 0.5f));
                 p.c(0, 0, 9, H(0x26242C), b3);
                 p.c(0, 0, 4, H(0xB47CFF));
             });
         }},
        {"shuriken", [](Caneta& p) {
             p.g(G().anim(Anim::gira(0.35f)), [&] {
                 p.star(0, 0, 18, 5, 4, H(0xC7CDD6), b3);
                 p.c(0, 0, 3.5f, H(0x3E4550));
             });
         }},
        {"estrepe", [](Caneta& p) {
             p.star(0, 0, 15, 4, 3, H(0x8E97A4), b3);
             p.c(0, 0, 3, H(0x6B7480));
         }},
        {"pocao", [](Caneta& p) {
             p.c(0, 4, 12, H(0xE8312A), b3);
             p.rr(-4, -14, 8, 10, 2, H(0xC7E8F5), 2);
             p.c(-4, 0, 3, H(0xFFFFFF, 0x88));
         }},
        {"acido", [](Caneta& p) {
             p.e(0, 4, 16, 10, H(0xCFFF6E), b3);
             p.c(-6, 1, 3, H(0x9BE66E));
             p.c(5, 6, 2.5f, H(0x9BE66E));
         }},
        {"espinho", [](Caneta& p) {
             for (Vector2 q : {Vector2{-8, -6}, Vector2{8, -6}, Vector2{0, 8}}) p.star(q.x, q.y, 9, 3, 4, H(0x8E97A4), 2);
         }},
        {"espinhos", [](Caneta& p) {
             p.star(0, 0, 20, 8, 8, H(0x8E97A4), b3);
             p.c(0, 0, 6, H(0x6B7480));
         }},
        {"relampago", [](Caneta& p) {
             p.g(G().anim(Anim::pisca(0.4f, 1, 0.15f)), [&] {
                 p.poly({{4, -26}, {-10, 2}, {0, 2}, {-4, 26}, {10, -4}, {0, -4}}, H(0xFFD632), b3);
             });
         }},
        {"tornado", [](Caneta& p) {
             p.g(G().anim(Anim::balanca(10, 0.4f)), [&] {
                 p.e(0, -14, 18, 5, H(0xDDE2E8), b3);
                 p.e(0, -4, 13, 4.5f, H(0xDDE2E8), b3);
                 p.e(0, 5, 9, 4, H(0xDDE2E8), b3);
                 p.e(0, 13, 5, 3, H(0xDDE2E8), b3);
             });
         }},
        {"cipo", [](Caneta& p) {
             p.ln({{-18, 12}, {-6, 0}, {6, 4}, {18, -10}}, 5, H(0x3F9A3A), 2);
             p.e(-6, -5, 6, 3.5f, H(0x62C23A), 1.5f, -30);
             p.e(8, -1, 6, 3.5f, H(0x62C23A), 1.5f, 30);
         }},
        {"banana", [](Caneta& p) {
             p.g(G().anim(Anim::flutua(-4, 1)), [&] {
                 p.arc(0, -12, 18, 40, 140, 8, H(0xFFD632), b3);
                 p.c(-14, 0, 2.5f, H(0x6E4523));
             });
         }},
        {"moeda", [](Caneta& p) {
             p.g(G().anim(Anim::flutua(-4, 1)), [&] {
                 p.c(0, 0, 14, H(0xFFD632), b3);
                 p.ring(0, 0, 8, 10, H(0xC98A0E));
                 p.rr(-1.5f, -6, 3, 12, 1.5f, H(0xC98A0E));
             });
         }},
        {"prego", [](Caneta& p) {
             p.rr(-2, -14, 4, 28, 1, H(0x8E97A4), 2);
             p.rr(-7, -16, 14, 4, 2, H(0x6B7480), 2);
         }},
        {"espuma", [](Caneta& p) {
             p.c(-6, 2, 9, H(0xFFB3D9), 2);
             p.c(6, -2, 10, H(0xFFB3D9), 2);
             p.c(2, 8, 7, H(0xFFD6EB), 2);
         }},
        {"drone", [](Caneta& p) {
             p.rr(-10, -6, 20, 12, 5, H(0x55555E), b3);
             for (Vector2 q : {Vector2{-16, -10}, Vector2{16, -10}, Vector2{-16, 10}, Vector2{16, 10}})
                 p.g(g_em(q.x, q.y).anim(Anim::gira(0.15f)), [&] { p.rr(-7, -1.5f, 14, 3, 1.5f, H(0xC7CDD6)); });
             p.c(0, 0, 3, H(0xFF4D5E));
         }},
        {"psi", [](Caneta& p) {
             p.g(G().anim(Anim::pulsa(0.8f, 1.1f, 0.6f)), [&] {
                 p.ring(0, 0, 12, 16, H(0xF0C8FF));
                 p.ring(0, 0, 4, 7, H(0xB47CFF));
             });
         }},
        {"luz", [](Caneta& p) {
             p.g(G().anim(Anim::pulsa(0.9f, 1.1f, 0.5f)), [&] {
                 p.star(0, 0, 22, 6, 4, H(0xFFF3B0));
                 p.c(0, 0, 8, BRANCO, 2);
             });
         }},
        {"espirito", [](Caneta& p) {
             p.g(G().anim(Anim::flutua(-4, 1.2f)), [&] {
                 p.poly({{0, -18}, {12, -6}, {12, 12}, {6, 7}, {0, 14}, {-6, 7}, {-12, 12}, {-12, -6}},
                        al(H(0x8FD3FF), 0.9f), b3);
                 p.c(-4, -5, 2.5f, TINTA);
                 p.c(4, -5, 2.5f, TINTA);
             });
         }},
        {"maldicao", [](Caneta& p) {
             p.g(G().anim(Anim::gira(2)), [&] {
                 p.ring(0, 0, 11, 15, H(0x8B3FD9));
                 p.star(0, 0, 9, 4, 5, H(0x9BE66E));
             });
         }},
        {"meteoro", [](Caneta& p) {
             p.poly({{-10, -6}, {22, -26}, {6, 4}}, al(H(0xF7941D), 0.7f));
             p.c(-6, 6, 12, H(0x8A5A2E), b3);
             p.c(-9, 3, 3, H(0x5A3418));
         }},
        {"radiacao", [](Caneta& p) {
             p.c(0, 0, 15, H(0xFFD632), b3);
             for (float a : {0.0f, 120.0f, 240.0f}) p.sec(0, 0, 12, a - 30, a + 30, TINTA);
             p.c(0, 0, 4, H(0xFFD632), 1.5f);
         }},
        {"aviaozinho", [](Caneta& p) { p.poly({{0, -18}, {14, 14}, {0, 6}, {-14, 14}}, H(0xF2F2F5), b3); }},
        {"vento", [](Caneta& p) {
             p.arc(0, 0, 14, 200, 340, 4, H(0xDDF3FF), 2);
             p.arc(4, 9, 10, 200, 340, 4, H(0xDDF3FF), 2);
         }},
        {"espadas", [](Caneta& p) {
             p.g(G().anim(Anim::gira(0.8f)), [&] {
                 p.rr(-2.5f, -24, 5, 48, 2, H(0xDDE2E8), 2, 45);
                 p.rr(-2.5f, -24, 5, 48, 2, H(0xDDE2E8), 2, -45);
             });
         }},
        {"flash", [](Caneta& p) {
             p.g(G().anim(Anim::pulsa(0.7f, 1.15f, 0.3f)), [&] {
                 p.star(0, 0, 22, 9, 8, BRANCO, b3);
                 p.c(0, 0, 6, H(0xFFF3B0));
             });
         }},
        {"armadilha", [](Caneta& p) {
             p.c(0, 0, 14, H(0x6B7480), b3);
             p.star(0, 0, 14, 9, 10, H(0x8E97A4));
             p.g(G().anim(Anim::pisca(0.3f, 1, 0.6f)), [&] { p.c(0, 0, 5, H(0xE8312A), 2); });
         }},
        {"zumbi", [](Caneta& p) {
             p.c(0, 0, 14, H(0x8FBF45), b3);
             p.c(-5, -3, 3, TINTA);
             p.c(5, -3, 3, TINTA);
             p.ln({{-5, 6}, {5, 6}}, 2, TINTA);
         }},
        {"balista", [](Caneta& p) {
             p.rr(-3, -22, 6, 44, 2, H(0x6E4523), b3);
             p.poly({{-8, -20}, {8, -20}, {0, -32}}, H(0x8E97A4), b3);
         }},
        {"impacto", [](Caneta& p) {
             p.g(G().anim(Anim::pulsa(0.6f, 1.1f, 0.3f)), [&] { p.star(0, 0, 20, 8, 8, H(0xFFD632), b3); });
         }},
        // visuais do jogo que o design nao desenhou: variacoes das pecas acima
        {"fragmento", [](Caneta& p) { p.poly({{0, -12}, {6, 2}, {-5, 7}}, H(0x8E97A4), 2); }},
        {"uva_fogo", [](Caneta& p) {
             for (Vector2 q : {Vector2{-6, -2}, Vector2{6, -2}, Vector2{0, 8}, Vector2{0, -11}}) p.c(q.x, q.y, 7, H(0xF7941D), 2);
             p.c(0, -2, 4, H(0xFFD632));
         }},
    };
    return m;
}

void proj(Caneta& p, const std::string& k) {
    auto it = PROJ().find(k);
    (it != PROJ().end() ? it->second : PROJ().at("magia"))(p);
}

// ================================================================ itens na mao (mao em 0,0 apontando -y)
struct Item {
    const char* t = nullptr;
    Color c = NENHUMA;
    bool g = false, plasma = false, bolha = false, mira = false, duplo = false;
};
Item It(const char* t, uint32_t c = 0, bool tem_cor = false) {
    Item i;
    i.t = t;
    if (tem_cor) i.c = H(c);
    return i;
}
Item Ic(const char* t, uint32_t c) { return It(t, c, true); }
Item Ig(Item i) { i.g = true; return i; }

Color ou(Color c, uint32_t padrao) { return c.a ? c : H(padrao); }

void item(Caneta& p, const Item& s) {
    if (!s.t) return;
    const std::string t = s.t;
    const bool g = s.g;
    if (t == "dardo") {
        p.rr(-2.5f, -34, 5, 30, 2, H(0x6B7480), b3);
        p.poly({{-6, -32}, {6, -32}, {0, -44}}, H(0xDDE2E8), b3);
    } else if (t == "triplo") {
        for (float r : {-20.0f, 0.0f, 20.0f})
            p.g(G().gira(r), [&] {
                p.rr(-2, -34, 4, 28, 2, s.plasma ? H(0xB47CFF) : H(0x6B7480), 2);
                p.poly({{-5, -32}, {5, -32}, {0, -42}}, s.plasma ? H(0xF0DCFF) : H(0xDDE2E8), 2);
            });
    } else if (t == "bolaEspinho") {
        const float r = g ? 18.0f : 10.0f;
        p.star(0, -r - 6, r + 7, r, 12, g ? H(0xFFD632) : H(0x8E97A4), b3);
        p.c(0, -r - 6, r * 0.7f, H(0x6B7480), 2);
        p.c(-r * 0.25f, -r - 9, r * 0.25f, H(0xFFFFFF, 0x66));
    } else if (t == "besta") {
        p.g(G().esc(g ? 1.3f : 1), [&] {
            p.rr(-3, -36, 6, 34, 2, H(0x6E4523), b3);
            p.arc(0, -10, 20, 205, 335, 5, H(0x8A5A2E), b3);
            p.ln({{-18, -18.5f}, {0, -8}, {18, -18.5f}}, 1.5f, H(0xF3E2B8));
            p.poly({{-4, -34}, {4, -34}, {0, -42}}, H(0xC7CDD6), 2);
        });
    } else if (t == "arco") {
        p.g(G().esc(g ? 1.25f : 1), [&] {
            p.arc(0, -4, 24, 200, 340, 5, H(0x6E4523), b3);
            p.ln({{-22.5f, -12}, {22.5f, -12}}, 1.5f, H(0xF3E2B8));
            p.rr(-1.5f, -38, 3, 30, 1, H(0x8A5A2E));
            p.poly({{-4, -36}, {4, -36}, {0, -44}}, H(0xC7CDD6), 2);
        });
    } else if (t == "bumerangue") {
        p.ln({{-14, -8}, {0, -24}, {14, -8}}, 7, H(0xF2C21A), b3);
    } else if (t == "glaive") {
        p.star(0, -20, 17, 6, 4, H(0xC7CDD6), b3, -45);
        p.c(0, -20, 4, H(0x6B7480));
    } else if (t == "kylie") {
        if (g) p.ln({{-24, -2}, {0, -40}, {24, -2}}, 11, H(0xE8312A), b3);
        else p.ln({{-18, -4}, {0, -32}, {18, -4}}, 9, H(0xC98A0E), b3);
    } else if (t == "varinha") {
        p.rr(-2, -30, 4, 30, 2, H(0x6E4523), 2);
        p.star(0, -34, 9, 4, 5, ou(s.c, 0xB47CFF), b3);
    } else if (t == "cajado") {
        const float L = g ? 58.0f : 48.0f;
        p.rr(-3, -L, 6, L + 4, 3, H(0x6E4523), b3);
        p.c(0, -L - 2, g ? 10.0f : 8.0f, ou(s.c, 0xB47CFF), b3);
        p.c(-3, -L - 5, 2.5f, H(0xFFFFFF, 0xAA));
    } else if (t == "espada") {
        const float L = g ? 50.0f : 40.0f;
        p.rr(-4, -L - 4, 8, L, 3, H(0xDDE2E8), b3);
        p.rr(-11, -8, 22, 6, 3, H(0xC98A0E), b3);
        p.rr(-2.5f, -4, 5, 10, 2, H(0x6E4523), 2);
    } else if (t == "lanca") {
        p.rr(-2.5f, -50, 5, 56, 2, H(0x3E4550), b3);
        p.poly({{-7, -46}, {7, -46}, {0, -64}}, ou(s.c, 0xC7CDD6), b3);
    } else if (t == "frasco") {
        const Color c = ou(s.c, 0x62C23A);
        p.g(G().esc(g ? 1.35f : 1), [&] {
            p.rr(-4, -32, 8, 10, 2, H(0xC7E8F5), 2);
            p.c(0, -14, 11, c, b3);
            p.c(-4, -18, 3, H(0xFFFFFF, 0xAA));
            if (s.bolha) p.g(g_em(3, -26).anim(Anim::flutua(-8, 0.8f)), [&] { p.c(0, 0, 3, c, 1.5f); });
        });
    } else if (t == "shuriken") {
        p.g(g_em(0, -16).anim(Anim::gira(0.6f)), [&] {
            p.star(0, 0, 13, 4, 4, H(0xC7CDD6), b3);
            p.c(0, 0, 3, H(0x3E4550));
        });
    } else if (t == "kunai") {
        p.rr(-2, -8, 4, 10, 1, H(0x1C1B20), 2);
        p.poly({{-5, -8}, {5, -8}, {0, -34}}, H(0xC7CDD6), b3);
    } else if (t == "rifle") {
        const float L = g ? 62.0f : 52.0f;
        p.rr(-3, -L, 6, L + 2, 2, H(0x3E4550), b3);
        p.rr(-5, -24, 10, 10, 2, H(0x6E4523), 2);
        if (s.mira) {
            p.rr(-7, -40, 5, 14, 2, H(0x26242C), 2);
            p.c(-4.5f, -40, 2, H(0x8FD3FF));
        }
    } else if (t == "pistola") {
        p.rr(-4, -26, 8, 24, 2, H(0x3E4550), b3);
        p.rr(-3, -8, 6, 6, 1, H(0x6E4523));
    } else if (t == "minigun") {
        std::vector<float> xs = s.duplo ? std::vector<float>{-10, 10} : std::vector<float>{0};
        for (float x : xs) {
            p.rr(x - 8, -40, 16, 36, 4, H(0x55555E), b3);
            for (float dx : {-5.0f, 0.0f, 5.0f}) p.rr(x + dx - 1.8f, -52, 3.6f, 14, 1, H(0x3E4550), 1.5f);
            p.c(x, -12, 6, H(0x8E97A4), 2);
        }
    } else if (t == "laser") {
        p.g(G().esc(g ? 1.3f : 1), [&] {
            p.rr(-7, -44, 14, 40, 4, H(0x3E4550), b3);
            p.g(g_em(0, -46).anim(Anim::pulsa(0.8f, 1.2f, 0.5f)), [&] {
                p.c(0, 0, 10, al(H(0xFF4D5E), 0.4f));
                p.c(0, 0, 5.5f, H(0xFF4D5E), 2);
            });
        });
    } else if (t == "lancaFoguete") {
        p.g(G().esc(g ? 1.3f : 1), [&] {
            p.rr(-7, -48, 14, 46, 4, H(0x5A6A3C), b3);
            p.poly({{-5, -48}, {5, -48}, {0, -60}}, H(0xE8312A), 2);
            p.rr(-9, -14, 18, 5, 2, H(0x3E4A2A));
        });
    } else if (t == "mangueira") {
        const Color c = ou(s.c, 0x9BE66E);
        p.rr(-4, -30, 8, 26, 3, H(0x8E97A4), b3);
        p.poly({{-6, -30}, {6, -30}, {3, -40}, {-3, -40}}, H(0x55555E), 2);
        p.g(g_em(0, -46).anim(Anim::pisca(0.4f, 1, 0.5f)), [&] { p.c(0, 0, 5, c, 2); });
    } else if (t == "bomba") {
        p.c(0, -14, 11, H(0x26242C), b3);
        p.c(-4, -18, 3, H(0xFFFFFF, 0x44));
        p.ln({{4, -24}, {9, -30}}, 2.5f, H(0xC98A0E));
        p.g(g_em(10, -31).anim(Anim::pisca(0.3f, 1, 0.2f)), [&] { p.star(0, 0, 5, 2, 6, H(0xFFD632)); });
    } else if (t == "chave") {
        p.g(G().esc(g ? 1.4f : 1), [&] {
            p.rr(-3, -34, 6, 32, 2, H(0x8E97A4), b3);
            p.sec(0, -38, 9, 90, 390, H(0x8E97A4), b3);
            p.c(0, -42, 3.5f, H(0xF3E2B8));
        });
    } else if (t == "cetroSol") {
        p.rr(-2.5f, -40, 5, 42, 2, H(0xC98A0E), b3);
        p.g(g_em(0, -44).anim(Anim::gira(4)), [&] { p.star(0, 0, 12, 6, 8, H(0xFFD632), b3); });
        p.c(0, -44, 5, BRANCO);
    } else if (t == "cetroTrevas") {
        p.rr(-2.5f, -40, 5, 42, 2, H(0x26242C), b3);
        p.star(0, -44, 11, 5, 4, H(0x6A3FC4), b3);
        p.c(0, -44, 4, H(0xFF4D5E));
    } else if (t == "laptop") {
        p.rr(-16, -24, 32, 20, 3, H(0x3E4550), b3);
        p.g(G().anim(Anim::pisca(0.6f, 1, 0.9f)), [&] { p.rr(-13, -21, 26, 14, 2, H(0x3AE6C8)); });
    } else if (t == "controle") {
        p.ln({{6, -16}, {10, -30}}, 2, H(0x3E4550));
        p.rr(-12, -18, 24, 14, 5, H(0x3E4550), b3);
        p.c(-5, -11, 2.5f, H(0xE8312A));
        p.c(5, -11, 2.5f, H(0x62C23A));
    } else if (t == "lancaChamas") {
        p.rr(-6, -38, 12, 36, 3, H(0x8E97A4), b3);
        p.g(g_em(0, -40).anim(Anim::pulsa(0.8f, 1.2f, 0.25f)), [&] {
            p.poly({{-6, 0}, {6, 0}, {0, -18}}, H(0xF7941D), 2);
            p.poly({{-3, 0}, {3, 0}, {0, -9}}, H(0xFFD632));
        });
    } else if (t == "cristal") {
        p.poly({{0, -36}, {8, -22}, {0, -8}, {-8, -22}}, H(0x8FD3FF), b3);
        p.poly({{0, -30}, {3, -22}, {0, -14}, {-3, -22}}, BRANCO);
    } else if (t == "lancaGelo") {
        p.rr(-2.5f, -20, 5, 22, 2, H(0x3AA0E6), 2);
        p.poly({{0, -66}, {10, -30}, {0, -18}, {-10, -30}}, H(0x8FD3FF), b3);
        p.poly({{0, -56}, {4, -32}, {0, -24}, {-4, -32}}, BRANCO);
    } else if (t == "canhaoGelo") {
        p.rr(-8, -40, 16, 38, 5, H(0x8FD3FF), b3);
        p.c(0, -40, 8, BRANCO, b3);
        p.c(0, -40, 4, H(0x3AA0E6));
    } else if (t == "canhaoCola") {
        p.rr(-9, -44, 18, 42, 6, H(0x62C23A), b3);
        p.c(0, -44, 9, H(0x9BE66E), b3);
        p.c(0, -44, 4, H(0x2B7A1E));
    } else if (t == "chama") {
        p.g(g_em(0, -18).anim(Anim::pulsa(0.85f, 1.15f, 0.3f)), [&] {
            p.poly({{0, -16}, {9, -2}, {7, 8}, {-7, 8}, {-9, -2}}, H(0xF7941D), b3);
            p.poly({{0, -6}, {4, 2}, {-4, 2}}, H(0xFFD632));
        });
    } else if (t == "cipo") {
        p.ln({{0, 0}, {-6, -14}, {4, -26}, {-2, -38}}, 4, H(0x3F9A3A), 2);
        p.e(-6, -16, 5, 3, H(0x62C23A), 1.5f, -30);
        p.e(4, -28, 5, 3, H(0x62C23A), 1.5f, 30);
    }
}

// ================================================================ macaco (vista de cima / frente)
struct Cabeca {
    float x, y, r;
};
struct Ancora {
    Cabeca H;
    Vector2 orelhas[2], olhos[2], maoD, maoE, ombros[2];
};
const Ancora AC{{64, 50, 29}, {{35, 48}, {93, 48}}, {{56, 42}, {72, 42}}, {96, 68}, {32, 68}, {{46, 76}, {82, 76}}};
const Ancora AF{{64, 46, 27}, {{37, 44}, {91, 44}}, {{56, 43}, {72, 43}}, {97, 72}, {33, 90}, {{48, 72}, {80, 72}}};

void chapeu(Caneta& p, const std::string& t, const Cabeca& Hd, Color c, float yb, bool g);

void capacete(Caneta& p, const Cabeca& Hd, Color c, float yb) {
    p.sec(Hd.x, yb, Hd.r * 1.02f, 180, 360, c, B);
    p.rr(Hd.x - Hd.r * 1.12f, yb - 4, Hd.r * 2.24f, 8, 4, esc(c, 0.75f), b3);
    p.e(Hd.x - Hd.r * 0.35f, yb - Hd.r * 0.62f, 7, 3.5f, H(0xFFFFFF, 0x55), 0, -20);
}

void chapeu(Caneta& p, const std::string& t, const Cabeca& Hd, Color c, float yb, bool g) {
    const float x = Hd.x, r = Hd.r;
    if (t == "faixa") {
        p.rr(x - r * 0.98f, yb - 2, r * 1.96f, 9, 4.5f, c, b3);
    } else if (t == "bandana") {
        p.ln({{x + r * 0.9f, yb + 2}, {x + r * 1.35f, yb + 12}}, 6, c, b3);
        p.ln({{x + r * 0.9f, yb + 2}, {x + r * 1.45f, yb - 2}}, 6, c, b3);
        p.rr(x - r * 0.98f, yb - 2, r * 1.96f, 9, 4.5f, c, b3);
    } else if (t == "capuz") {
        p.e(x - r - 1, Hd.y + 2, 6, 14, c, b3);
        p.e(x + r + 1, Hd.y + 2, 6, 14, c, b3);
        p.sec(x, yb + 3, r + 5, 180, 360, c, B);
        p.e(x - r * 0.3f, yb - r * 0.55f, 7, 3.5f, H(0xFFFFFF, 0x33), 0, -20);
    } else if (t == "capacete") {
        capacete(p, Hd, c, yb);
    } else if (t == "elmo") {
        p.rr(x - 3.5f, yb - r * 1.25f, 7, r * 0.9f, 3.5f, H(0xE8312A), b3);
        capacete(p, Hd, c, yb);
    } else if (t == "gorro") {
        p.sec(x, yb, r * 0.98f, 180, 360, c, B);
        p.rr(x - r, yb - 5, r * 2, 9, 4.5f, BRANCO, b3);
        p.c(x, yb - r * 0.98f - 2, 7, BRANCO, b3);
    } else if (t == "mago") {
        const float hg = r * (g ? 2.3f : 1.6f);
        p.e(x, yb, r * (g ? 1.4f : 1.15f), 7, esc(c, 0.6f), B);
        p.poly({{x - r * 0.7f, yb}, {x + r * 0.7f, yb}, {x + 10, yb - hg}}, c, B);
        p.rr(x - r * 0.62f, yb - 9, r * 1.24f, 6, 3, g ? H(0xFFD632) : clar(c, 0.3f));
        p.star(x + 3, yb - hg * 0.5f, g ? 8.0f : 6.0f, g ? 3.4f : 2.6f, 5, H(0xFFD632), 2);
    } else if (t == "coroa") {
        const Color cc = c.a ? c : H(0xFFD632);
        const float w = r * 1.3f, y = yb + 3;
        p.poly({{x - w / 2, y}, {x - w / 2, y - 16}, {x - w / 4, y - 7}, {x, y - 20}, {x + w / 4, y - 7}, {x + w / 2, y - 16},
                {x + w / 2, y}},
               cc, b3);
        p.c(x, y - 5, 3, H(0xE8312A));
        p.c(x - w / 3, y - 4, 2.2f, H(0x3A96E6));
        p.c(x + w / 3, y - 4, 2.2f, H(0x3A96E6));
    } else if (t == "coroaGelo") {
        const Color cc = c.a ? c : H(0x8FD3FF);
        const float y = yb + 3;
        p.poly({{x - 20, y}, {x - 16, y - 16}, {x - 9, y - 6}, {x, y - 24}, {x + 9, y - 6}, {x + 16, y - 16}, {x + 20, y}}, cc, b3);
        p.poly({{x - 3, y - 4}, {x, y - 16}, {x + 3, y - 4}}, BRANCO);
    } else if (t == "coroaFogo") {
        p.g(g_em(x, yb + 2).anim(Anim::pulsa(0.92f, 1.08f, 0.35f)), [&] {
            const float dxs[3] = {-14, 0, 14};
            for (int i = 0; i < 3; ++i)
                p.poly({{dxs[i] - 8, 0}, {dxs[i], i == 1 ? -26.0f : -18.0f}, {dxs[i] + 8, 0}}, H(0xF7941D), b3);
            for (float dx : dxs) p.poly({{dx - 4, 0}, {dx, -10}, {dx + 4, 0}}, H(0xFFD632));
        });
    } else if (t == "halo") {
        p.g(g_em(x, yb - 14).anim(Anim::flutua(-3, 1.6f)), [&] {
            p.ringE(0, 0, r * 0.8f, 6, 9, TINTA);
            p.ringE(0, 0, r * 0.8f, 6, 5, H(0xFFD632));
        });
    } else if (t == "boina") {
        p.e(x - 5, yb + 1, r * 0.9f, r * 0.42f, c, B, -8);
        p.c(x - 5, yb - r * 0.3f, 3, esc(c, 0.7f));
    } else if (t == "quepe") {
        p.sec(x, yb, r * 0.95f, 180, 360, c, B);
        p.rr(x - r * 1.05f, yb - 4, r * 2.1f, 8, 4, H(0x1C1B20), b3);
        p.star(x, yb - r * 0.45f, 6, 2.6f, 5, H(0xFFD632), 1.5f);
    } else if (t == "cowboy") {
        p.e(x, yb + 2, r * 1.4f, 8.5f, esc(c, 0.8f), B);
        p.sec(x, yb, r * 0.72f, 180, 360, c, B);
        p.rr(x - r * 0.72f, yb - 6, r * 1.44f, 5, 2, H(0xC98A0E));
    } else if (t == "cartola") {
        p.rr(x - r * 0.6f, yb - r * 1.25f, r * 1.2f, r * 1.25f, 4, c, B);
        p.e(x, yb + 1, r * 1.05f, 6.5f, c, B);
        p.rr(x - r * 0.6f, yb - 12, r * 1.2f, 7, 0, H(0xFFD632));
    } else if (t == "fones") {
        p.arc(x, Hd.y, r + 3, 200, 340, 5, c, b3);
        p.rr(x - r - 8, Hd.y - 9, 10, 18, 4, c, b3);
        p.rr(x + r - 2, Hd.y - 9, 10, 18, 4, c, b3);
    } else if (t == "caveira") {
        p.e(x - 14, yb - 12, 3.5f, 11, H(0x8B3FD9), 2, -25);
        p.e(x + 14, yb - 12, 3.5f, 11, H(0x62C23A), 2, 25);
        p.c(x, yb - 6, 11, H(0xEEEEEE), b3);
        p.c(x - 4, yb - 7, 2.8f, TINTA);
        p.c(x + 4, yb - 7, 2.8f, TINTA);
    } else if (t == "folhas") {
        for (int i = 0; i < 7; ++i) {
            const float a = 180.0f + i * 30;
            p.e(x + std::cos(rad(a)) * r * 0.95f, Hd.y - 4 + std::sin(rad(a)) * r * 0.95f, 7.5f, 4.5f, c, 2, a + 90);
        }
    } else if (t == "chifres") {
        p.poly({{x - r * 0.6f, yb + 3}, {x - r * 0.28f, yb + 1}, {x - r * 0.95f, yb - 24}}, c, b3);
        p.poly({{x + r * 0.6f, yb + 3}, {x + r * 0.28f, yb + 1}, {x + r * 0.95f, yb - 24}}, c, b3);
    } else if (t == "antena") {
        p.ln({{x + 8, yb}, {x + 14, yb - 20}}, 3, H(0x8E97A4), 2);
        p.g(g_em(x + 14, yb - 22).anim(Anim::pisca(0.4f, 1, 0.8f)), [&] { p.c(0, 0, 4.5f, c, 2); });
    } else if (t == "nuvem") {
        p.g(g_em(x, yb - 16).anim(Anim::flutua(-3, 2)), [&] {
            p.c(-13, 3, 9, H(0xC7CDD6), b3);
            p.c(13, 3, 9, H(0xC7CDD6), b3);
            p.c(0, -3, 12, H(0xDDE2E8), b3);
            p.rr(-20, 2, 40, 10, 5, H(0xDDE2E8));
            p.g(G().anim(Anim::pisca(0, 1, 0.7f)), [&] {
                p.poly({{3, 8}, {-5, 20}, {0, 20}, {-3, 30}, {6, 16}, {1, 16}}, H(0xFFD632), 2);
            });
        });
    } else if (t == "sol") {
        p.g(g_em(x, yb - 8).anim(Anim::gira(8)), [&] { p.star(0, 0, 18, 10, 10, H(0xFFD632), b3); });
        p.c(x, yb - 8, 8, H(0xFFF3B0), b3);
    } else if (t == "ninja") {
        p.ln({{x + r * 0.9f, yb + 4}, {x + r * 1.4f, yb + 14}}, 5, c, b3);
        p.sec(x, yb + 2, r + 1, 180, 360, H(0x1C1B20), B);
        p.rr(x - r, yb - 3, r * 2, 7, 3.5f, c, b3);
    } else if (t == "alq") {
        p.rr(x - r * 0.5f, yb - r * 0.72f, r, r * 0.74f, 5, c, B);
        p.e(x, yb + 1, r * 1.05f, 6, esc(c, 0.7f), B);
        p.rr(x - r * 0.5f, yb - 8, r, 5, 0, H(0x62C23A));
    } else if (t == "morcego") {
        p.poly({{x - r * 0.75f, yb + 4}, {x - r * 0.3f, yb}, {x - r * 0.7f, yb - 20}}, c, b3);
        p.poly({{x + r * 0.75f, yb + 4}, {x + r * 0.3f, yb}, {x + r * 0.7f, yb - 20}}, c, b3);
    }
}

void rosto2(Caneta& p, const std::string& t, const Ancora& A, Color c) {
    const Cabeca& Hd = A.H;
    const float y = A.olhos[0].y;
    if (t == "oculos") {
        for (auto& o : A.olhos) p.ring(o.x, o.y, 5.5f, 8.5f, c);
        p.ln({{A.olhos[0].x + 8, A.olhos[0].y}, {A.olhos[1].x - 8, A.olhos[1].y}}, 2.5f, c);
    } else if (t == "goggles") {
        p.rr(Hd.x - Hd.r, y - 4, Hd.r * 2, 8, 4, esc(c, 0.6f));
        for (auto& o : A.olhos) {
            p.c(o.x, o.y, 8, c, b3);
            p.c(o.x, o.y, 5.5f, H(0x8FD3FF));
            p.c(o.x - 2, o.y - 2, 1.8f, BRANCO);
        }
    } else if (t == "visor") {
        p.rr(Hd.x - Hd.r * 0.72f, y - 6, Hd.r * 1.44f, 12, 6, c, b3);
        p.g(G().anim(Anim::pisca(0.3f, 1, 1)), [&] { p.rr(Hd.x - Hd.r * 0.6f, y - 3, Hd.r * 1.2f, 3.5f, 1.7f, H(0xFFFFFF, 0xBB)); });
    } else if (t == "mascara") {
        p.rr(Hd.x - Hd.r * 0.72f, y - 6.5f, Hd.r * 1.44f, 13, 6.5f, c, b3);
        for (auto& o : A.olhos) {
            p.c(o.x, o.y, 3.8f, BRANCO);
            p.c(o.x + 0.5f, o.y + 0.5f, 2, TINTA);
        }
    } else if (t == "mascaraNinja") {
        p.e(Hd.x, y + 13, Hd.r * 0.66f, 8, H(0x1C1B20), b3);
    } else if (t == "mascaraGas") {
        p.e(Hd.x, y + 13, 12, 9, c, b3);
        p.c(Hd.x, y + 14, 4.5f, H(0x3E4550), 2);
    }
}

void costas(Caneta& p, const std::string& t, bool F, Color c, const std::string& x) {
    if (t == "aljava") {
        p.rr(78, 54, 12, 40, 5, H(0x6E4523), b3, 25);
        for (Vector2 q : {Vector2{86, 50}, Vector2{91, 53}, Vector2{81, 49}})
            p.poly({{q.x - 3, q.y + 4}, {q.x + 3, q.y + 4}, {q.x, q.y - 4}}, H(0xE8312A), 1.5f);
    } else if (t == "jetpack") {
        for (float xx : {48.0f, 80.0f})
            p.g(g_em(xx, 102).anim(Anim::pisca(0.5f, 1, 0.25f)), [&] {
                p.poly({{-6, 0}, {6, 0}, {0, 18}}, H(0xF7941D), b3);
                p.poly({{-3, 0}, {3, 0}, {0, 10}}, H(0xFFD632));
            });
        p.rr(40, 70, 16, 32, 7, c, B);
        p.rr(72, 70, 16, 32, 7, c, B);
    } else if (t == "mochila") {
        p.rr(42, F ? 66.0f : 72.0f, 44, 36, 9, c, B);
        p.rr(48, F ? 72.0f : 78.0f, 32, 6, 3, esc(c, 0.7f));
    } else if (t == "tanque") {
        p.rr(46, F ? 62.0f : 68.0f, 36, 42, 12, c, B);
        p.c(64, F ? 66.0f : 72.0f, 5, H(0xC7CDD6), b3);
        p.e(56, F ? 72.0f : 78.0f, 4, 8, H(0xFFFFFF, 0x55));
    } else if (t == "mochilaBombas") {
        for (float xx : {48.0f, 64.0f, 80.0f}) {
            p.c(xx, F ? 70.0f : 98.0f, 9, H(0x26242C), b3);
            p.c(xx - 3, F ? 67.0f : 95.0f, 2.5f, H(0xFFFFFF, 0x44));
        }
    } else if (t == "frascos") {
        p.ln({{42, 70}, {86, 104}}, 6, H(0x6E4523), b3);
        p.c(50, 76, 5.5f, H(0xE8312A), 2);
        p.c(64, 87, 5.5f, H(0x62C23A), 2);
        p.c(78, 98, 5.5f, H(0x3A96E6), 2);
    } else if (t == "asas") {
        for (float sd : {-1.0f, 1.0f})
            p.g(g_em(64 + sd * 14, 76).anim(Anim::balanca(sd * -12, 0.9f)), [&] {
                if (x == "morcego") {
                    p.poly({{0, -2}, {sd * 50, -32}, {sd * 46, -10}, {sd * 54, 4}, {sd * 36, 2}, {sd * 32, 18}, {sd * 14, 10}}, c, B);
                } else {
                    p.poly({{0, -6}, {sd * 48, -36}, {sd * 58, -20}, {sd * 52, -4}, {sd * 42, 8}, {sd * 26, 14}, {0, 8}}, c, B);
                    p.poly({{sd * 6, -2}, {sd * 40, -26}, {sd * 46, -16}, {sd * 22, 4}}, clar(c, 0.45f));
                }
            });
    }
}

void aura(Caneta& p, Color c) {
    p.c(64, 70, 58, al(c, 0.16f));
    p.g(g_em(64, 70).anim(Anim::pulsa(0.9f, 1.05f, 1.4f)), [&] { p.ring(0, 0, 51, 56, al(c, 0.75f)); });
}

void orbita(Caneta& p, bool F, const std::string& tipo, int n, float r = 54, float dur = 3) {
    p.g(g_em(64, 70).esc(1, F ? 0.5f : 1), [&] {
        p.g(G().anim(Anim::gira(dur)), [&] {
            for (int i = 0; i < n; ++i) {
                const float a = i * PI_F * 2 / n;
                p.g(g_em(std::cos(a) * r, std::sin(a) * r).esc(0.45f), [&] { proj(p, tipo); });
            }
        });
    });
}

// Estilo de um macaco; Object.assign do design vira mesclar() com campos opcionais.
struct Estilo {
    std::optional<Color> pelo, rosto, olhos, aura, capa, ombro;
    std::optional<bool> braco_metal, gordo;
    std::optional<Item> item, item2;
    std::optional<std::pair<std::string, Color>> rosto2;
    struct Chapeu {
        std::string t;
        Color c = NENHUMA;
        bool g = false;
    };
    std::optional<Chapeu> chapeu;
    struct Costas {
        std::string t;
        Color c = NENHUMA;
        std::string x;
    };
    std::optional<Costas> costas;
    struct Orbita {
        std::string t;
        int n = 0;
        float r = 54, dur = 3;
    };
    std::optional<Orbita> orbita;

    Estilo& Pelo(uint32_t c) { pelo = H(c); return *this; }
    Estilo& Rosto(uint32_t c) { rosto = H(c); return *this; }
    Estilo& Olhos(uint32_t c) { olhos = H(c); return *this; }
    Estilo& Aura(uint32_t c) { aura = H(c); return *this; }
    Estilo& Capa(uint32_t c) { capa = H(c); return *this; }
    Estilo& SemCapa() { capa = NENHUMA; return *this; }
    Estilo& Ombro(uint32_t c) { ombro = H(c); return *this; }
    Estilo& BracoMetal() { braco_metal = true; return *this; }
    Estilo& Gordo() { gordo = true; return *this; }
    Estilo& Mao(Item i) { item = i; return *this; }
    Estilo& Mao2(Item i) { item2 = i; return *this; }
    Estilo& Rosto2(const char* t, uint32_t c = 0) { rosto2 = std::make_pair(std::string(t), H(c)); return *this; }
    Estilo& Chap(const char* t, uint32_t c = 0, bool g = false) {
        chapeu = Chapeu{t, c ? H(c) : NENHUMA, g};
        return *this;
    }
    Estilo& Cost(const char* t, uint32_t c = 0, const char* x = "") {
        costas = Costas{t, c ? H(c) : NENHUMA, x};
        return *this;
    }
    Estilo& Orb(const char* t, int n, float r = 54, float dur = 3) {
        orbita = Orbita{t, n, r, dur};
        return *this;
    }
};

void mesclar(Estilo& a, const Estilo& b) {
#define M(campo) \
    if (b.campo) a.campo = b.campo
    M(pelo); M(rosto); M(olhos); M(aura); M(capa); M(ombro); M(braco_metal); M(gordo);
    M(item); M(item2); M(rosto2); M(chapeu); M(costas); M(orbita);
#undef M
}

void macaco(Caneta& p, Vista v, const Estilo& s) {
    const bool F = v == Vista::FRENTE;
    const Ancora& A = F ? AF : AC;
    const Cabeca& Hd = A.H;
    const Color pelo = s.pelo.value_or(H(0x8A5A2E));
    const Color ro = s.rosto.value_or(ROSTO);
    const Color braco = s.braco_metal.value_or(false) ? H(0x8E97A4) : pelo;
    auto corpo = [&] {
        if (s.aura) aura(p, *s.aura);
        p.e(64, F ? 118.0f : 113.0f, 32, 8, H(0, 0x33));
        if (s.capa && s.capa->a) {
            const Color cp = *s.capa;
            p.g(g_em(64, 66).anim(Anim::balanca(3, 2)), [&] {
                if (F) p.poly({{-20, -2}, {20, -2}, {36, 54}, {-36, 54}}, cp, B);
                else p.poly({{-22, 0}, {22, 0}, {38, 56}, {-38, 56}}, cp, B);
            });
        }
        if (s.costas) costas(p, s.costas->t, F, s.costas->c, s.costas->x);
        if (F) {
            p.ln({{80, 100}, {98, 98}, {106, 84}, {98, 74}}, 6, pelo, b3);
            p.e(53, 106, 10, 9, pelo, B);
            p.e(75, 106, 10, 9, pelo, B);
            p.e(52, 112, 8, 4, ro);
            p.e(76, 112, 8, 4, ro);
            p.e(64, 86, 21, 22, pelo, B);
            p.e(64, 91, 12, 13, ro);
        } else {
            p.ln({{64, 104}, {76, 114}, {88, 110}, {92, 100}}, 6, pelo, b3);
            p.c(64, 88, 23, pelo, B);
            p.e(58, 80, 9, 5, H(0xFFFFFF, 0x26), 0, -20);
        }
        if (s.ombro)
            for (auto& o : A.ombros) {
                p.e(o.x, o.y, 11, 8, *s.ombro, b3);
                p.e(o.x - 3, o.y - 3, 4, 2, H(0xFFFFFF, 0x66));
            }
        // braco e mao esquerda
        if (F) p.ln({{47, 76}, {33, 90}}, 9, pelo, b3);
        else p.ln({{48, 80}, {32, 68}}, 9, pelo, b3);
        p.g(g_em(A.maoE.x, A.maoE.y).gira(F ? -20.0f : 8.0f), [&] {
            if (s.item2) item(p, *s.item2);
            p.c(0, 0, 7.5f, pelo, b3);
        });
        // cabeca
        for (auto& o : A.orelhas) {
            p.c(o.x, o.y, F ? 10.0f : 11.0f, pelo, B);
            p.c(o.x, o.y, F ? 5.5f : 6.0f, ro);
        }
        p.c(Hd.x, Hd.y, Hd.r, pelo, B);
        const bool olho_cor = s.olhos.has_value();
        const Color pup = olho_cor ? esc(*s.olhos, 0.75f) : TINTA;
        if (F) {
            p.c(55, 43, 10.5f, ro);
            p.c(73, 43, 10.5f, ro);
            p.e(64, 53, 15, 10.5f, ro);
            for (auto& o : A.olhos) {
                if (olho_cor) p.c(o.x, o.y, 9, al(*s.olhos, 0.35f));
                p.e(o.x, o.y, 4.8f, 6.2f, BRANCO);
                p.c(o.x + 0.8f, o.y + 1, 3, pup);
                p.c(o.x + 1.8f, o.y - 0.7f, 1.1f, BRANCO);
            }
            p.e(64, 51, 3.2f, 2.2f, esc(pelo, 0.5f));
            p.ln({{58.5f, 57}, {64, 60}, {69.5f, 57}}, 2.2f, TINTA);
            p.e(52, 29, 8, 4, H(0xFFFFFF, 0x4D), 0, -20);
        } else {
            p.e(64, 46, 19, 15, ro);
            for (auto& o : A.olhos) {
                if (olho_cor) p.c(o.x, o.y, 9, al(*s.olhos, 0.35f));
                p.c(o.x, o.y, 5.2f, BRANCO);
                p.c(o.x, o.y - 1.2f, 2.8f, pup);
            }
            p.e(64, 54, 8, 5, ROSTO_E);
            p.e(52, 33, 9, 4.5f, H(0xFFFFFF, 0x4D), 0, -20);
        }
        if (s.rosto2) rosto2(p, s.rosto2->first, A, s.rosto2->second);
        if (s.chapeu) chapeu(p, s.chapeu->t, Hd, s.chapeu->c, Hd.y - Hd.r * 0.45f, s.chapeu->g);
        // braco direito + item (ataque): no jogo segue a pose dos clipes de disparo/habilidade
        if (F) p.ln({{81, 76}, {97, 72}}, 9, braco, b3);
        else p.ln({{80, 80}, {96, 68}}, 9, braco, b3);
        p.g(g_em(A.maoD.x, A.maoD.y).gira(F ? 18.0f : -8.0f), [&] {
            p.g(G().anim(Anim::braco(-22, 1.1f)), [&] {
                if (s.item) item(p, *s.item);
                p.c(0, 0, 8, braco, b3);
                if (p.pose.flash > 0.01f) {
                    const float k = p.pose.flash;
                    p.star(0, -50, 13 * k + 4, 5 * k + 2, 8, al(H(0xFFF3B0), 0.9f * k));
                    p.c(0, -50, 5 * k, al(BRANCO, k));
                }
            });
        });
        if (s.orbita) orbita(p, F, s.orbita->t, s.orbita->n, s.orbita->r, s.orbita->dur);
    };
    if (s.gordo.value_or(false)) p.g(g_em(64, 84).esc(1.14f), [&] { p.g(g_em(-64, -84), corpo); });
    else corpo();
}

// ================================================================ maquinas
struct Maquina {
    bool voa = false;
    std::function<void(Caneta&, Color, int, int)> pe;
    std::function<void(Caneta&, int, int)> d;
};

const std::map<std::string, Maquina>& MAQ() {
    static const std::map<std::string, Maquina> m = {
        {"bomba",
         {false, [](Caneta& p, Color c, int, int) { p.c(64, 72, 32, c, B); },
          [](Caneta& p, int cam, int t) {
              const float big = cam == 0 ? (t == 5 ? 1.45f : t == 3 ? 1.22f : 1) : 1;
              if (cam == 0 && t == 5) p.star(64, 72, 44, 34, 16, H(0xC7CDD6), B);
              p.c(64, 72, 32, H(0x6B7480), B);
              for (int i = 0; i < 8; ++i) {
                  const float a = i * PI_F / 4;
                  p.c(64 + std::cos(a) * 26, 72 + std::sin(a) * 26, 2.2f, H(0x3E4550));
              }
              std::vector<float> canos = cam == 2 ? (t == 5 ? std::vector<float>{-21, -7, 7, 21}
                                                    : t == 3 ? std::vector<float>{-14, 0, 14}
                                                             : std::vector<float>{0})
                                                  : (cam == 1 && t == 5 ? std::vector<float>{-9, 9} : std::vector<float>{0});
              const float w = (canos.size() > 2 ? 12.0f : canos.size() == 2 ? 14.0f : 16.0f) * big, L = 44 * big;
              p.g(g_em(64, 72).anim(Anim::recuo(6, 1.2f)), [&] {
                  for (float dx : canos) {
                      const Color col = cam == 1 && t >= 3 ? H(0x8E1E16) : H(0x3E4550);
                      p.rr(dx - w / 2, -L, w, L, 5, col, B);
                      if (cam == 1 && t >= 3) {
                          p.rr(dx - w / 2, -L * 0.72f, w, 5, 0, H(0xF2F2F5));
                          p.rr(dx - w / 2, -L * 0.45f, w, 5, 0, H(0xF2F2F5));
                      }
                      p.rr(dx - w / 2 - 3, -L - 4, w + 6, 10, 4, H(0x26242C), B);
                  }
              });
              if (cam == 1 && t >= 3) p.rr(84, 40, 7, 22, 3, H(0x26242C), b3);
              if (cam == 2 && t == 5) p.ln({{34, 92}, {64, 102}, {94, 92}}, 7, H(0xC98A0E), b3);
              p.c(64, 72, 20, H(0x55555E), B);
              p.c(64, 72, 9, H(0x26242C));
              p.e(56, 64, 6, 3.5f, H(0xFFFFFF, 0x55), 0, -30);
          }}},
        {"tachinha",
         {false, [](Caneta& p, Color c, int, int) { p.c(64, 64, 40, c, B); },
          [](Caneta& p, int cam, int t) {
              const int n = cam == 2 ? (t == 5 ? 16 : 12) : 8;
              const bool lam = cam == 1 && t >= 3;
              const Color spike = cam == 0 && t >= 3 ? H(0xE8312A) : lam ? H(0xDDE2E8) : H(0xC7CDD6);
              const Color dome = cam == 0 && t >= 3   ? H(0xF7941D)
                                 : cam == 1 && t == 5 ? H(0xFFD632)
                                 : cam == 2 && t >= 3 ? H(0x8B3FD9)
                                                      : H(0xFF6FAE);
              if (cam == 0 && t == 5) {
                  p.c(64, 64, 60, al(H(0xF7941D), 0.2f));
                  p.g(g_em(64, 64).anim(Anim::gira(4)), [&] {
                      for (int i = 0; i < 8; ++i) {
                          const float a = i * PI_F / 4;
                          p.g(g_em(std::cos(a) * 52, std::sin(a) * 52).esc(0.42f), [&] { proj(p, "fogo"); });
                      }
                  });
              }
              if (cam == 2 && t == 5) p.star(64, 64, 52, 36, n, H(0x6B7480), B, -90 + 180.0f / n);
              p.g(g_em(64, 64).anim(cam == 1 && t == 5 ? Anim::gira(0.6f) : Anim::pulsa(1, 1.05f, 0.5f)),
                  [&] { p.star(0, 0, lam ? 52.0f : 42.0f, lam ? 20.0f : 26.0f, n, spike, B); });
              p.c(64, 64, 24, dome, B);
              p.c(64, 64, 10, clar(dome, 0.4f), b3);
              p.e(56, 55, 6, 3.5f, H(0xFFFFFF, 0x66), 0, -30);
          }}},
        {"submarino",
         {false, [](Caneta& p, Color c, int cam, int t) { p.e(64, 64, 21, cam == 2 && t == 5 ? 52.0f : 46.0f, c, B); },
          [](Caneta& p, int cam, int t) {
              const Color hull = cam == 0 && t == 5 ? H(0x3AA0E6) : cam == 2 && t == 5 ? H(0xFFD21F) : H(0xE0B020);
              const float L = cam == 2 && t == 5 ? 52.0f : 46.0f;
              if (cam == 0 && t == 5) aura(p, H(0x8FFFF0));
              p.rr(38, 64 + L - 16, 52, 9, 4, esc(hull, 0.7f), b3);
              if (cam == 2 && t >= 3)
                  for (float dx : {-10.0f, 0.0f, 10.0f}) p.rr(64 + dx - 2.5f, 64 - L - 8, 5, 18, 2, H(0x3E4550), b3);
              p.e(64, 64, 21, L, hull, B);
              p.e(57, 50, 5, L * 0.45f, H(0xFFFFFF, 0x40));
              if (cam == 1 && t >= 3) {
                  p.rr(56, 76, 16, 10, 3, H(0x8E1E16), b3);
                  p.rr(56, 90, 16, 10, 3, H(0x8E1E16), b3);
              }
              if (cam == 1 && t == 5) {
                  p.rr(58, 74, 12, 32, 6, H(0xE8312A), b3);
                  p.poly({{58, 78}, {70, 78}, {64, 68}}, H(0xF2F2F5), b3);
              }
              p.rr(52, 42, 24, 30, 9, esc(hull, 0.62f), B);
              p.c(64, 46, 4.5f, H(0x8FD3FF), b3);
              if (cam == 0 && t >= 3)
                  p.g(g_em(64, 60).anim(Anim::gira(2)), [&] {
                      p.e(0, 0, 15, 5, H(0xC7CDD6), b3);
                      p.c(0, 0, 3, H(0x3E4550));
                  });
              if (cam == 0 && t == 5)
                  p.g(g_em(64, 92).anim(Anim::pulsa(0.8f, 1.2f, 0.7f)), [&] { p.c(0, 0, 8, H(0x8FFFF0), b3); });
              if (cam == 2 && t == 5) {
                  p.rr(62, 16, 3, 28, 1, H(0x6E4523));
                  p.g(g_em(65, 18).anim(Anim::balanca(8, 1)), [&] { p.poly({{0, 0}, {20, 6}, {0, 12}}, H(0xE8312A), b3); });
              }
          }}},
        {"bucaneiro",
         {false,
          [](Caneta& p, Color, int, int) {
              p.poly({{64, 10}, {84, 40}, {86, 94}, {78, 114}, {50, 114}, {42, 94}, {44, 40}}, H(0x3A3640), B);
          },
          [](Caneta& p, int cam, int t) {
              const Color hull = cam == 0 && t >= 3 ? H(0x6B7480) : cam == 1 && t == 5 ? H(0x3A2A1E) : H(0x8A5A2E);
              const Color deck = cam == 0 && t >= 3 ? H(0x9AA3AE) : H(0xC9A06A);
              p.g(g_em(64, 64).esc(cam == 0 && t == 5 ? 1.12f : 1).anim(Anim::balanca(3, 2.4f)), [&] {
                  if (cam == 1 && t >= 3) {
                      std::vector<float> ys = t == 5 ? std::vector<float>{-18, 0, 18, 34} : std::vector<float>{-4, 18};
                      for (float y : ys) {
                          p.rr(-31, y - 3, 11, 6, 2, H(0x26242C), b3);
                          p.rr(20, y - 3, 11, 6, 2, H(0x26242C), b3);
                      }
                  }
                  p.poly({{0, -54}, {20, -24}, {22, 30}, {14, 50}, {-14, 50}, {-22, 30}, {-20, -24}}, hull, B);
                  p.poly({{0, -44}, {14, -20}, {15, 28}, {9, 42}, {-9, 42}, {-15, 28}, {-14, -20}}, deck);
                  for (float y : {-6.0f, 10.0f, 26.0f}) p.ln({{-13, y}, {13, y}}, 1.5f, esc(deck, 0.8f));
                  if (cam == 0 && t >= 3) {
                      p.rr(-3, -46, 6, 18, 2, H(0x3E4550), b3);
                      p.c(0, -28, 7, H(0x55555E), b3);
                  }
                  if (cam == 2 && t >= 3) {
                      p.rr(-12, 14, 10, 10, 2, H(0xC98A0E), b3);
                      p.rr(2, 20, 10, 10, 2, H(0xC98A0E), b3);
                      if (t == 5) {
                          p.c(-6, 32, 4, H(0xFFD632), 2);
                          p.c(4, 34, 4, H(0xFFD632), 2);
                      }
                  }
                  std::vector<float> mastros = cam == 0 && t == 5 ? std::vector<float>{-24, -2, 20} : std::vector<float>{-2};
                  const Color vela = cam == 1 && t == 5 ? H(0x1C1B20) : cam == 2 && t == 5 ? H(0xFFD632) : H(0xF2F2F5);
                  for (float y : mastros) {
                      p.rr(-27, y - 5, 54, 10, 5, vela, b3);
                      p.c(0, y, 4, H(0x6E4523), 2);
                  }
                  if (cam == 1 && t == 5) {
                      p.c(-12, -2, 3.5f, H(0xF2F2F5));
                      p.c(12, -2, 3.5f, H(0xF2F2F5));
                  }
                  p.c(0, 34, 9, H(0x8A5A2E), b3);
                  p.e(0, 32, 6, 4.5f, ROSTO);
                  if (cam == 1 && t == 5) p.e(0, 28, 8, 3, H(0x1C1B20), 2);
              });
          }}},
        {"as",
         {true, nullptr,
          [](Caneta& p, int cam, int t) {
              const Color col = cam == 0   ? (t == 5 ? H(0xE8312A) : t == 3 ? H(0x8E97A4) : H(0xE0E4EA))
                                : cam == 1 ? (t >= 3 ? H(0x5A6A3C) : H(0xE0E4EA))
                                           : (t == 5 ? H(0x3E4550) : H(0xE0E4EA));
              p.g(g_em(64, 62).anim(Anim::balanca(6, 2)), [&] {
                  const float W = cam == 1 && t == 5 ? 60.0f : cam == 2 && t == 5 ? 56.0f : 50.0f;
                  const bool sw = cam == 0 && t >= 3;
                  if (sw) p.poly({{0, -18}, {W, 14}, {W, 22}, {0, 10}, {-W, 22}, {-W, 14}}, col, B);
                  else p.poly({{0, -12}, {W, -2}, {W, 8}, {0, 4}, {-W, 8}, {-W, -2}}, col, B);
                  if (cam == 2 && t == 5) {
                      for (float x : {-40.0f, -22.0f, 22.0f, 40.0f}) p.c(x, 2, 6, H(0x55555E), b3);
                  } else if (cam == 1 && t == 3) {
                      for (float x : {-30.0f, 30.0f}) p.c(x, 10, 6, H(0x26242C), b3);
                  }
                  if (cam == 1 && t == 5) {
                      p.e(0, 20, 9, 16, H(0x26242C), b3);
                      p.c(-3, 14, 2.5f, H(0xFFD632));
                  }
                  p.poly({{0, 30}, {16, 44}, {16, 48}, {-16, 48}, {-16, 44}}, col, B);
                  p.rr(-9, -44, 18, 88, 9, col, B);
                  p.rr(-9, 12, 18, 4, 0, esc(col, 0.75f));
                  p.e(0, -24, 5, 9, H(0x3A96E6), b3);
                  p.e(-1.5f, -27, 1.5f, 3, BRANCO);
                  if (cam == 0 && t == 5) {
                      p.rr(-26, -14, 4, 14, 2, H(0x26242C), 2);
                      p.rr(22, -14, 4, 14, 2, H(0x26242C), 2);
                  }
                  if (cam == 2 && t >= 3) p.c(0, -44, 5, H(0xFFD632), b3);
                  if (!sw) p.g(g_em(0, -47).anim(Anim::gira(0.15f)), [&] { p.rr(-14, -2, 28, 4, 2, H(0x55555E), 2); });
              });
          }}},
        {"heli",
         {true, nullptr,
          [](Caneta& p, int cam, int t) {
              const Color col = cam == 0 && t == 5   ? H(0x3E4A2A)
                                : cam == 1 && t == 5 ? H(0x26242C)
                                : cam == 2 && t == 5 ? H(0x2F80DA)
                                                     : H(0x62C23A);
              p.g(g_em(64, 60), [&] {
                  if (cam == 1 && t >= 3)
                      p.g(G().anim(Anim::cresce(0.6f, 1.2f, 1)),
                          [&] { p.g(G().anim(Anim::some(1)), [&] { p.ring(0, 0, 48, 52, al(BRANCO, 0.6f)); }); });
                  p.rr(-4, 10, 8, 46, 3, esc(col, 0.72f), B);
                  p.rr(-12, 50, 24, 6, 3, esc(col, 0.72f), b3);
                  if (cam == 0 && t == 5) {
                      p.rr(-31, -8, 10, 22, 4, H(0x55555E), b3);
                      p.rr(21, -8, 10, 22, 4, H(0x55555E), b3);
                  }
                  if (cam == 2 && t >= 3) p.rr(-16, -40, 32, 6, 3, H(0x8E97A4), b3);
                  if (cam == 1 && t == 5) p.ln({{10, 4}, {18, 30}}, 2, H(0xC9A06A));
                  p.e(0, -6, 18, 26, col, B);
                  p.e(0, -20, 10, 9, H(0x8FD3FF), b3);
                  p.e(-3, -23, 3, 2, BRANCO);
                  const int nb = cam == 2 && t == 5 ? 4 : 2;
                  const Color lam = cam == 0 && t >= 3 ? H(0xE8312A) : H(0x55555E);
                  p.g(g_em(0, -6).anim(Anim::gira(0.25f)), [&] {
                      for (int i = 0; i < nb; ++i) p.rr(-52, -3, 104, 6, 3, lam, 2, i * 180.0f / nb);
                  });
                  p.c(0, -6, 5, H(0x3E4550), b3);
                  if (cam == 2 && t == 5) p.g(g_em(-64, -70), [&] { orbita(p, false, "drone", 2, 44, 2.5f); });
              });
          }}},
        {"morteiro",
         {false, [](Caneta& p, Color c, int, int) { p.c(64, 66, 44, c, B); },
          [](Caneta& p, int cam, int t) {
              for (int i = 0; i < 10; ++i) {
                  const float a = i * 36.0f;
                  p.e(64 + std::cos(rad(a)) * 36, 66 + std::sin(rad(a)) * 36, 11, 7.5f, H(0xC9A06A), b3, a + 90);
              }
              p.c(64, 66, 30, H(0x6B7A3A), B);
              const int n = cam == 1 ? (t == 5 ? 4 : t == 3 ? 2 : 1) : 1;
              const float r = n > 1 ? 11.0f : cam == 0 ? (t == 5 ? 22.0f : t == 3 ? 18.0f : 15.0f) : 15.0f;
              std::vector<Vector2> pos = n == 1   ? std::vector<Vector2>{{0, 0}}
                                         : n == 2 ? std::vector<Vector2>{{-12, 0}, {12, 0}}
                                                  : std::vector<Vector2>{{-12, -12}, {12, -12}, {-12, 12}, {12, 12}};
              const Color tubo = cam == 2 && t == 5 ? H(0x8E1E16) : H(0x3E4550);
              p.g(g_em(64, 66).anim(Anim::pulsa(1, 0.9f, 1.6f)), [&] {
                  for (auto& q : pos) {
                      p.c(q.x, q.y, r, tubo, B);
                      p.c(q.x, q.y, r * 0.55f, TINTA);
                      if (cam == 0 && t == 5) p.ring(q.x, q.y, r - 3, r, H(0xFFD632));
                  }
              });
              if (cam == 2 && t >= 3)
                  p.g(g_em(64, 66).anim(Anim::pisca(0.2f, 0.9f, 0.6f)), [&] { p.c(0, 0, 8, H(0xFF4D5E)); });
              if (cam == 2 && t == 5) p.g(g_em(0, -4), [&] { orbita(p, false, "fogo", 6, 46, 3); });
          }}},
        {"fazenda",
         {false, [](Caneta& p, Color c, int, int) { p.rr(10, 14, 108, 100, 16, c, B); },
          [](Caneta& p, int cam, int t) {
              p.rr(10, 14, 108, 100, 16, H(0x8FBF45), B);
              for (int i = 0; i < 4; ++i) p.rr(18, 24 + i * 22.0f, 92, 10, 5, H(0xB98C57));
              std::vector<Vector2> arv;
              if (cam == 0 && t == 3) arv = {{28, 30}, {28, 60}, {28, 92}, {50, 30}, {50, 92}, {100, 100}};
              else if (!(cam == 0 && t == 5)) arv = {{28, 30}, {28, 64}, {28, 96}};
              for (auto& q : arv) {
                  p.c(q.x, q.y, 11, H(0x3F9A3A), b3);
                  p.c(q.x - 3, q.y - 3, 5, H(0x6DC24A));
                  p.e(q.x + 5, q.y + 4, 4, 2.2f, H(0xFFD632), 1.5f, 30);
              }
              if (cam == 0 && t == 5) {
                  p.rr(26, 28, 76, 60, 8, H(0x8E97A4), B);
                  p.c(40, 40, 7, H(0x6B7480), b3);
                  p.c(88, 40, 7, H(0x6B7480), b3);
                  p.rr(26, 94, 76, 10, 5, H(0x3E4550), b3);
                  p.g(G().anim(Anim::voa(40, 0, 1.4f)), [&] { p.e(40, 99, 5, 3, H(0xFFD632), 1.5f); });
              } else if (cam == 1 && t >= 3) {
                  if (t == 5) {
                      p.c(78, 64, 34, H(0xFFD632), B);
                      p.ring(78, 64, 22, 26, H(0xC98A0E));
                      p.txt(78, 64, "$", 30, BRANCO, 3);
                  } else {
                      p.rr(58, 44, 50, 44, 6, H(0x3A96E6), B);
                      p.c(83, 66, 12, H(0xFFD632), b3);
                      p.c(83, 66, 4, H(0xC98A0E));
                  }
              } else if (cam == 2 && t >= 3) {
                  if (t == 5) {
                      p.rr(52, 22, 56, 82, 8, H(0x3E4550), B);
                      for (int i = 0; i < 4; ++i)
                          for (int j = 0; j < 3; ++j) p.rr(60 + j * 16.0f, 32 + i * 16.0f, 10, 8, 2, H(0x8FD3FF));
                      p.c(80, 94, 7, H(0xFFD632), 2);
                  } else {
                      for (int i = 0; i < 5; ++i) p.rr(56 + i * 10.0f, 48, 10, 34, 0, i % 2 ? H(0xF2F2F5) : H(0xE8312A));
                      p.rr(56, 48, 50, 34, 4, NENHUMA, b3);
                  }
              } else {
                  p.rr(64, 52, 44, 44, 6, H(0xE8312A), B);
                  p.rr(64, 72, 44, 4, 0, H(0x8E1E16));
              }
              p.g(g_em(94, 104).anim(Anim::flutua(-5, 1)), [&] { p.e(0, 0, 6, 3.4f, H(0xFFD632), 2, 25); });
          }}},
        {"espinhos",
         {false, [](Caneta& p, Color c, int, int) { p.rr(26, 38, 76, 58, 10, c, B); },
          [](Caneta& p, int cam, int t) {
              const Color corpo = cam == 2 && t == 5 ? H(0xFFD632) : H(0x8E97A4);
              p.g(g_em(42, 30).anim(Anim::flutua(-10, 1.6f)), [&] {
                  p.g(G().anim(Anim::some(1.6f)), [&] {
                      p.c(0, 0, 8, al(H(0x8E97A4), 0.7f));
                      p.c(7, -5, 6, al(H(0xC7CDD6), 0.7f));
                  });
              });
              p.rr(26, 38, 76, 58, 10, corpo, B);
              p.rr(32, 44, 64, 6, 3, H(0xFFFFFF, 0x44));
              p.c(42, 42, 9, esc(corpo, 0.75f), B);
              p.star(64, 106, 16, 7, 8, H(0xC7CDD6), b3);
              if (cam == 0 && t == 3)
                  for (float x : {46.0f, 82.0f}) p.star(x, 106, 11, 7, 10, H(0x8E97A4), b3);
              if (cam == 0 && t == 5) {
                  p.c(88, 106, 12, H(0x3E4550), b3);
                  p.g(g_em(88, 106).anim(Anim::pisca(0.2f, 1, 0.5f)), [&] { p.c(0, 0, 4.5f, H(0xE8312A)); });
              }
              if (cam == 1 && t >= 3)
                  p.g(g_em(82, 70).anim(Anim::gira(0.5f)), [&] { p.star(0, 0, 18, 12, 12, H(0xDDE2E8), b3); });
              if (cam == 1 && t == 5) {
                  p.g(g_em(48, 70).anim(Anim::gira(0.5f, -1)), [&] { p.star(0, 0, 18, 12, 12, H(0xDDE2E8), b3); });
                  for (float x : {20.0f, 108.0f}) p.star(x, 106, 9, 4, 6, H(0x8E97A4), 2);
              }
              if (cam == 2 && t >= 3)
                  p.g(g_em(90, 60).anim(Anim::balanca(-35, 1.2f)), [&] {
                      p.ln({{0, 0}, {0, -34}}, 5, H(0x6E4523), b3);
                      p.e(0, -36, 8, 5, H(0x6E4523), b3);
                  });
          }}},
        {"vila",
         {false, [](Caneta& p, Color c, int, int) { p.c(64, 64, 40, c, B); },
          [](Caneta& p, int cam, int t) {
              Color c1 = H(0xE8312A), c2 = H(0xF3E2B8);
              if (cam == 2 && t == 5) c1 = H(0xFFD632), c2 = H(0xF2F2F5);
              else if (cam == 0 && t == 5) c1 = H(0xE8312A), c2 = H(0xFFD632);
              else if (cam == 1 && t >= 3) c1 = H(0x2F80DA), c2 = H(0xF3E2B8);
              if (cam == 2 && t >= 3) {
                  std::vector<Vector2> casas = t == 5 ? std::vector<Vector2>{{18, 20}, {104, 22}, {16, 100}, {108, 98}, {64, 8}, {64, 116}}
                                                      : std::vector<Vector2>{{18, 24}, {106, 100}, {20, 104}};
                  for (auto& q : casas) {
                      p.rr(q.x - 11, q.y - 9, 22, 18, 3, H(0xC9A06A), b3);
                      p.poly({{q.x - 13, q.y - 7}, {q.x, q.y - 17}, {q.x + 13, q.y - 7}}, H(0x8E1E16), 2);
                  }
              }
              if (cam == 1 && t == 5)
                  p.g(g_em(64, 64).anim(Anim::pulsa(0.95f, 1.04f, 1.2f)), [&] {
                      p.c(0, 0, 56, al(H(0x3A96E6), 0.15f));
                      p.ring(0, 0, 52, 57, al(H(0x3A96E6), 0.7f));
                  });
              p.c(64, 64, 40, c2, B);
              for (int a = 0; a < 360; a += 60) p.sec(64, 64, 40, static_cast<float>(a), a + 30.0f, c1);
              p.ring(64, 64, 38, 40.5f, TINTA);
              p.c(64, 64, 8, H(0xC98A0E), b3);
              if (cam == 0 && t >= 3) {
                  const bool alto = t == 5;
                  p.rr(62, alto ? 6.0f : 20.0f, 4, alto ? 58.0f : 44.0f, 2, H(0x6E4523), 2);
                  p.g(g_em(66, alto ? 8.0f : 22.0f).anim(Anim::balanca(8, 1)),
                      [&] { p.poly({{0, 0}, {alto ? 30.0f : 22.0f, 7}, {0, 14}}, alto ? H(0xFFD632) : H(0xE8312A), b3); });
              }
              if (cam == 1 && t >= 3)
                  p.g(g_em(96, 34).anim(Anim::balanca(25, 3)), [&] {
                      p.e(0, 0, 13, 6, H(0xC7CDD6), b3);
                      p.c(0, 0, 3, H(0x3E4550));
                  });
          }}},
        // torres invocadas (nao estao no design): mesma linguagem das maquinas
        {"sentinela",
         {false, [](Caneta& p, Color c, int, int) { p.c(64, 70, 30, c, B); },
          [](Caneta& p, int, int) {
              p.c(64, 70, 30, H(0x8E97A4), B);
              for (int i = 0; i < 6; ++i) {
                  const float a = i * PI_F / 3;
                  p.c(64 + std::cos(a) * 23, 70 + std::sin(a) * 23, 2.2f, H(0x3E4550));
              }
              p.g(g_em(64, 70).anim(Anim::recuo(5, 1)), [&] {
                  p.rr(-6, -46, 12, 40, 4, H(0x3E4550), B);
                  p.rr(-9, -50, 18, 9, 4, H(0x26242C), b3);
              });
              p.c(64, 70, 17, H(0xFFB21F), B);
              p.c(64, 70, 6, H(0x26242C));
              p.e(57, 63, 5, 3, H(0xFFFFFF, 0x66), 0, -30);
          }}},
        {"fenix",
         {true, nullptr,
          [](Caneta& p, int, int) {
              costas(p, "asas", false, H(0xF7941D), "pena");
              p.g(g_em(64, 100).anim(Anim::pulsa(0.85f, 1.15f, 0.3f)), [&] {
                  for (float dx : {-8.0f, 0.0f, 8.0f}) p.poly({{dx - 6, 0}, {dx, 22}, {dx + 6, 0}}, H(0xE8312A), b3);
              });
              p.e(64, 76, 16, 26, H(0xF7941D), B);
              p.e(64, 80, 9, 16, H(0xFFD632));
              p.c(64, 46, 13, H(0xF7941D), B);
              p.poly({{58, 36}, {64, 22}, {70, 36}}, H(0xFFD632), b3);
              p.poly({{60, 50}, {68, 50}, {64, 60}}, H(0xC98A0E), 2);
              p.c(59, 43, 2.5f, TINTA);
              p.c(69, 43, 2.5f, TINTA);
          }}},
    };
    return m;
}

void tanque(Caneta& p, int nivel) {
    if (nivel >= 20) aura(p, H(0xFFD632));
    p.rr(24, 24, 18, 82, 7, H(0x3E4550), B);
    p.rr(86, 24, 18, 82, 7, H(0x3E4550), B);
    for (int i = 0; i < 6; ++i) {
        p.ln({{27, 32 + i * 13.0f}, {39, 32 + i * 13.0f}}, 2, H(0x26242C));
        p.ln({{89, 32 + i * 13.0f}, {101, 32 + i * 13.0f}}, 2, H(0x26242C));
    }
    p.rr(36, 30, 56, 70, 10, H(0x5A6A3C), B);
    std::vector<float> canos = nivel >= 20 ? std::vector<float>{-6, 6} : std::vector<float>{0};
    const float L = nivel >= 10 ? 52.0f : 42.0f;
    p.g(g_em(64, 66).anim(Anim::recuo(6, 1.4f)), [&] {
        for (float x : canos) p.rr(x - 4, -L, 8, L, 3, nivel >= 20 ? H(0xC98A0E) : H(0x3E4A2A), B);
    });
    p.c(64, 66, nivel >= 10 ? 20.0f : 17.0f, H(0x6E7E4A), B);
    p.c(64, 66, 7, H(0x5A6A3C), b3);
    p.star(64, 66, 5, 2.2f, 5, H(0xFFD632));
}

void maquina(Caneta& p, Vista v, bool voador, const std::function<void(Caneta&, Color, int, int)>& pe,
             const std::function<void()>& d, int cam, int t) {
    if (v != Vista::FRENTE) {
        if (voador) p.e(72, 110, 34, 8, H(0, 0x30));
        d();
        return;
    }
    if (voador) {
        p.e(64, 116, 34, 7, H(0, 0x30));
        p.g(g_em(64, 58).esc(1, 0.8f), [&] { p.g(g_em(-64, -64), d); });
        return;
    }
    p.e(64, 118, 46, 9, H(0, 0x33));
    p.g(g_em(64, 76).esc(1, 0.78f), [&] { p.g(g_em(-64, -64), [&] { pe(p, H(0x3A3640), cam, t); }); });
    p.g(g_em(64, 64).esc(1, 0.78f), [&] { p.g(g_em(-64, -64), d); });
}

// ================================================================ torres "macaco": base + [T3, T5] por caminho
struct Macaco {
    Estilo b;
    Estilo c[3][2];
};

const std::map<std::string, Macaco>& TS() {
    static const std::map<std::string, Macaco> m = [] {
        std::map<std::string, Macaco> t;
        using E = Estilo;
        t["dardo"] = {E().Mao(It("dardo")),
                      {{E().Mao(It("bolaEspinho")), E().Mao(Ig(It("bolaEspinho"))).Chap("capacete", 0x8E97A4).Aura(0xFFD632)},
                       {E().Mao(It("triplo")).Chap("bandana", 0xE8312A),
                        [] {
                            Item i = It("triplo");
                            i.plasma = true;
                            return E().Pelo(0x7B4FD6).Olhos(0xE9D5FF).Rosto2("goggles", 0x3A96E6).Mao(i).Aura(0xB47CFF).Orb("plasma", 3);
                        }()},
                       {E().Mao(It("besta")).Rosto2("oculos", 0x3F7A2E),
                        E().Mao(Ig(It("besta"))).Chap("capuz", 0x3F7A2E).Capa(0x2B4A1E).Cost("aljava").Aura(0x9BE66E)}}};
        t["bumerangue"] = {E().Mao(It("bumerangue")).Chap("faixa", 0xE8312A),
                           {{E().Mao(It("glaive")), E().Mao(It("glaive")).Orb("glaive", 4).Chap("elmo", 0x8E97A4).Aura(0xC7CDD6)},
                            {E().BracoMetal().Ombro(0x8E97A4),
                             E().BracoMetal().Ombro(0x3A96E6).Rosto2("visor", 0x3AE6C8).Orb("relampago", 3).Aura(0x3AA0E6)},
                            {E().Mao(It("kylie")), E().Mao(Ig(It("kylie"))).Capa(0x8E1E16).Chap("coroa").Aura(0xE8312A)}}};
        t["gelo"] = {E().Pelo(0xBFE6FF).Rosto(0xEAF6FF).Chap("gorro", 0x2F8FE6).Mao(It("cristal")),
                     {{E().Orb("fragmento_gelo", 3), E().Orb("fragmento_gelo", 6).Chap("coroaGelo").Aura(0x8FD3FF)},
                      {E().Capa(0x8FD3FF), E().Pelo(0x8FD3FF).Olhos(0xFFFFFF).Chap("coroaGelo", 0xFFFFFF).Capa(0x3AA0E6).Aura(0x3AA0E6)},
                      {E().Mao(It("canhaoGelo")), E().Mao(It("lancaGelo")).Ombro(0x8FD3FF).Aura(0xDDF3FF)}}};
        t["cola"] = {E().Rosto2("goggles", 0x62C23A).Mao(Ic("frasco", 0x9BE66E)),
                     {{E().Cost("tanque", 0x9BE66E), E().Cost("tanque", 0x62C23A).Rosto2("mascaraGas", 0x6B7480).Aura(0x9BE66E)},
                      {E().Mao(Ic("mangueira", 0x9BE66E)).Cost("tanque", 0x9BE66E),
                       E().Mao(Ic("mangueira", 0x9BE66E)).Mao2(Ic("mangueira", 0x9BE66E)).Cost("tanque", 0x62C23A).Orb("cola", 4).Aura(0x62C23A)},
                      {E().Mao(Ic("frasco", 0xE8312A)), E().Mao(It("canhaoCola")).Chap("capacete", 0x62C23A).Aura(0xFFD632)}}};
        Item mira = It("rifle");
        mira.mira = true;
        t["sniper"] = {E().Pelo(0x6B7A3A).Chap("capacete", 0x4E5E2A).Mao(It("rifle")),
                       {{E().Mao(mira), E().Mao(Ig(mira)).Chap("boina", 0x1C1B20).Capa(0x4E5E2A).Aura(0xE8312A)},
                        {E().Rosto2("oculos", 0xC98A0E), E().Rosto2("goggles", 0x62C23A).Cost("mochila", 0x4E5E2A).Aura(0x62C23A)},
                        {E().Mao2(It("pistola")),
                         E().Chap("capacete", 0x26242C).Rosto2("visor", 0xFF4D5E).Ombro(0x4E5E2A).Mao(Ig(It("rifle"))).Mao2(It("pistola")).Aura(0xFFD632)}}};
        Item duplo = It("minigun");
        duplo.duplo = true;
        t["dartling"] = {E().Chap("capacete", 0xE0A020).Mao(It("minigun")),
                         {{E().Mao(It("laser")), E().Mao(Ig(It("laser"))).Rosto2("visor", 0xFF4D5E).Aura(0xFF4D5E)},
                          {E().Mao(It("lancaFoguete")), E().Mao(Ig(It("lancaFoguete"))).Ombro(0x5A6A3C).Cost("mochila", 0x5A6A3C).Aura(0xF7941D)},
                          {E().Mao(duplo), E().Mao(duplo).Chap("fones", 0x26242C).Orb("bala", 6).Aura(0xFFD632)}}};
        t["mago"] = {E().Chap("mago", 0x6A3FC4).Mao(Ic("varinha", 0xB47CFF)),
                     {{E().Mao(Ic("cajado", 0xB47CFF)).Orb("magia", 2),
                       E().Chap("mago", 0x6A3FC4, true).Mao(Ig(Ic("cajado", 0xFFD632))).Capa(0x4A2A8C).Orb("magia", 4).Aura(0xB47CFF)},
                      {E().Chap("mago", 0xC8302A).Mao(It("chama")),
                       E().Chap("coroaFogo").Cost("asas", 0xF7941D, "pena").Mao(It("chama")).Aura(0xF7941D)},
                      {E().Chap("mago", 0x1D2F5C),
                       E().Chap("capuz", 0x1C1B20).Olhos(0xFF4D5E).Capa(0x1C1B20).Mao(It("cetroTrevas")).Aura(0x6A3FC4)}}};
        t["super"] = {E().Rosto2("mascara", 0x2F8FE6).Capa(0xE8312A),
                      {{E().Pelo(0xE8B04A).Chap("sol").Aura(0xFFD632),
                        E().Pelo(0xFFE08A).Olhos(0xFFFFFF).Chap("halo").Cost("asas", 0xFFE08A, "pena").Capa(0xFFD632).Aura(0xFFD632)},
                       {E().Pelo(0x9AA3AE).Rosto(0xC7CDD6).Rosto2("visor", 0x3AE6C8).Chap("antena", 0x3AE6C8).SemCapa(),
                        E().Pelo(0x3E4550).Rosto(0x8E97A4).Rosto2("visor", 0xFF4D5E).Chap("antena", 0xFF4D5E).Ombro(0x26242C)
                            .Mao(It("laser")).Mao2(It("laser")).Aura(0xFF4D5E)},
                       {E().Rosto2("mascara", 0x1C1B20).Chap("morcego", 0x1C1B20).Capa(0x1C1B20),
                        E().Cost("asas", 0x1C1B20, "morcego").Olhos(0xFF4D5E).Capa(0x26242C).Aura(0x6A3FC4)}}};
        t["ninja"] = {E().Chap("ninja", 0xE8312A).Rosto2("mascaraNinja").Mao(It("shuriken")),
                      {{E().Mao2(It("shuriken")), E().Mao2(It("shuriken")).Orb("shuriken", 5).Chap("ninja", 0xFFD632).Capa(0x8E1E16).Aura(0xE8312A)},
                       {E().Chap("ninja", 0x2F8FE6), E().Chap("ninja", 0x1C1B20).Capa(0x1C1B20).Mao(It("kunai")).Olhos(0x8FD3FF).Aura(0x2F8FE6)},
                       {E().Mao(It("bomba")), E().Mao(It("bomba")).Cost("mochilaBombas").Chap("ninja", 0xF7941D).Aura(0xF7941D)}}};
        Item bolha = Ic("frasco", 0x9BE66E);
        bolha.bolha = true;
        Item bolha5 = Ig(Ic("frasco", 0xCFFF6E));
        bolha5.bolha = true;
        t["alquimista"] = {E().Chap("alq", 0x7A4E26).Mao(Ic("frasco", 0x62C23A)),
                           {{E().Mao(Ic("frasco", 0xE8312A)), E().Mao(Ig(Ic("frasco", 0xE8312A))).Cost("frascos").Aura(0xE8312A)},
                            {E().Mao(bolha), E().Pelo(0x4E7A2E).Olhos(0xCFFF6E).Ombro(0x62C23A).Mao(bolha5).Aura(0x9BE66E)},
                            {E().Mao(Ic("frasco", 0xFFD632)),
                             E().Chap("cartola", 0x6A3FC4).Capa(0x6A3FC4).Orb("pocao", 3).Mao(Ig(Ic("frasco", 0xFFD632))).Aura(0xFFD632)}}};
        t["druida"] = {E().Chap("folhas", 0x3F9A3A).Mao(Ic("cajado", 0x9BE66E)),
                       {{E().Chap("nuvem"), E().Chap("nuvem").Orb("tornado", 3).Capa(0x1D5A9C).Aura(0x3AA0E6)},
                        {E().Chap("folhas", 0x62C23A).Mao2(It("cipo")),
                         E().Chap("chifres", 0xC9A06A).Mao2(It("cipo")).Capa(0x2E7D2A).Orb("cipo", 3).Aura(0x62C23A)},
                        {E().Olhos(0xFF4D5E).Chap("chifres", 0x8A5A2E),
                         E().Pelo(0x5A3418).Olhos(0xFF4D5E).Chap("chifres", 0x3A2A1E).Capa(0x8E1E16).Aura(0xE8312A)}}};
        t["engenheiro"] = {E().Chap("capacete", 0xFFB21F).Mao(It("chave")),
                           {{E().Cost("mochila", 0xFFB21F), E().Ombro(0xFFB21F).Orb("drone", 3).Cost("mochila", 0xFFB21F).Aura(0xFFD632)},
                            {E().Mao(Ic("mangueira", 0xFF6FAE)),
                             E().Mao(Ic("mangueira", 0xFF6FAE)).Rosto2("visor", 0x3AE6C8).Orb("relampago", 3).Aura(0x3AE6C8)},
                            {E().Mao(It("pistola")).Mao2(It("pistola")),
                             E().Mao(Ig(It("chave"))).Cost("mochila", 0xC98A0E).Ombro(0x8E97A4).Aura(0xFFB21F)}}};
        return t;
    }();
    return m;
}

const std::map<std::string, std::array<Estilo, 3>>& HS() {
    static const std::map<std::string, std::array<Estilo, 3>> m = [] {
        std::map<std::string, std::array<Estilo, 3>> h;
        using E = Estilo;
        h["quincy"] = {E().Pelo(0x96602D).Chap("capuz", 0x3F7A2E).Mao(It("arco")), E().Cost("aljava"),
                       E().Capa(0x2B4A1E).Aura(0x9BE66E).Mao(Ig(It("arco")))};
        h["gwendolin"] = {E().Pelo(0xC85028).Rosto2("goggles", 0xFFD21F).Mao(It("lancaChamas")), E().Cost("tanque", 0xE8312A),
                          E().Orb("fogo", 4).Aura(0xF7941D)};
        h["striker"] = {E().Pelo(0x50643C).Chap("boina", 0x2B3A1E).Mao(It("lancaFoguete")), E().Ombro(0x2B3A1E),
                        E().Mao(Ig(It("lancaFoguete"))).Cost("mochila", 0x2B3A1E).Aura(0xFFD632)};
        h["obyn"] = {E().Pelo(0x287A5A).Chap("folhas", 0x9BE66E).Mao(Ic("cajado", 0x9BE66E)), E().Chap("chifres", 0xC9A06A),
                     E().Capa(0x2E7D2A).Orb("cipo", 3).Aura(0x62C23A)};
        h["benjamin"] = {E().Pelo(0x3C3C50).Chap("fones", 0x3A96E6).Mao(It("laptop")), E().Rosto2("oculos", 0x3AE6C8),
                         E().Orb("moeda", 4).Aura(0x3AE6C8)};
        h["ezili"] = {E().Pelo(0x6E285A).Chap("caveira").Mao(Ic("cajado", 0x9BE66E)), E().Capa(0x4A1E3E),
                      E().Olhos(0x9BE66E).Orb("maldicao", 3).Aura(0x8B3FD9)};
        h["pat"] = {E().Pelo(0x966E46).Gordo(), E().Ombro(0x6E4523), E().Chap("faixa", 0xE8312A).Aura(0xFFD632)};
        h["adora"] = {E().Pelo(0xE6C85A).Chap("sol").Mao(It("cetroSol")), E().Capa(0xFFD632),
                      E().Cost("asas", 0xFFF3B0, "pena").Chap("halo").Aura(0xFFD632)};
        h["brickell"] = {E().Pelo(0x46506E).Chap("quepe", 0x1D2F5C).Mao(It("espada")), E().Ombro(0xC98A0E),
                         E().Orb("armadilha", 3).Capa(0x1D2F5C).Aura(0x3A96E6)};
        h["etienne"] = {E().Pelo(0x3C5A8C).Chap("fones", 0xE8312A).Mao(It("controle")).Orb("drone", 1),
                        E().Orb("drone", 2).Rosto2("oculos", 0x26242C), E().Orb("drone", 3).Aura(0x3A96E6)};
        h["sauda"] = {E().Pelo(0xC8783C).Chap("bandana", 0x1C1B20).Mao(It("espada")), E().Mao2(It("espada")),
                      E().Mao(Ig(It("espada"))).Mao2(Ig(It("espada"))).Aura(0xE8312A)};
        h["psi"] = {E().Pelo(0x9A5AC8).Olhos(0xF0C8FF), E().Chap("antena", 0xF0C8FF), E().Orb("psi", 4).Aura(0xF0C8FF)};
        h["geraldo"] = {E().Pelo(0x784628).Chap("cowboy", 0x5A3A1E).Cost("mochila", 0x8A5A2E), E().Mao(Ic("frasco", 0xFF6FAE)),
                        E().Cost("mochila", 0xC98A0E).Mao(Ic("cajado", 0xFF6FAE)).Aura(0xFFD632)};
        h["corvus"] = {E().Pelo(0x28285A).Chap("capuz", 0x1C1B20).Mao(Ic("lanca", 0x8FD3FF)), E().Orb("espirito", 3),
                       E().Cost("asas", 0x1C1B20, "pena").Olhos(0x8FD3FF).Aura(0x6A3FC4)};
        h["rosalia"] = {E().Pelo(0xC85A78).Rosto2("goggles", 0xFFD632).Cost("jetpack", 0x8E97A4), E().Mao(It("pistola")),
                        E().Mao(It("laser")).Cost("jetpack", 0xFFD632).Aura(0xFF6FAE)};
        // Dan D'Monke usa por enquanto a arte do antigo Jericho, com espada no lugar da pistola
        h["dan"] = {E().Pelo(0x6E5032).Chap("cowboy", 0x2B2B2B).Mao(It("espada")), E().Capa(0x8E1E16),
                    E().Mao2(It("espada")).Aura(0xFFD632)};
        h["silas"] = {E().Pelo(0x78BEE6).Rosto(0xEAF6FF).Chap("gorro", 0xFFFFFF).Mao(Ic("cajado", 0x8FD3FF)), E().Chap("coroaGelo"),
                      E().Orb("fragmento_gelo", 4).Capa(0x3AA0E6).Aura(0x8FD3FF)};
        return h;
    }();
    return m;
}

// ================================================================ bloons
const std::map<std::string, Color>& COR_BLOON() {
    static const std::map<std::string, Color> m = {
        {"vermelho", H(0xE8312A)}, {"azul", H(0x2F8FE6)},  {"verde", H(0x62C23A)},     {"amarelo", H(0xFFD21F)},
        {"rosa", H(0xFF6FAE)},     {"preto", H(0x2E2C34)}, {"branco", H(0xF4F4F6)},    {"roxo", H(0x8B3FD9)},
        {"chumbo", H(0x8E97A4)},   {"zebra", H(0xF4F4F6)}, {"arco_iris", H(0xE8312A)}, {"ceramica", H(0xB8733A)},
    };
    return m;
}

const std::vector<Vector2>& CORACAO() {
    static const std::vector<Vector2> pts = [] {
        std::vector<Vector2> v;
        for (int i = 0; i < 16; ++i) {
            const float t = i / 16.0f * PI_F * 2;
            const float s = std::sin(t);
            v.push_back({0.5f * 16 * s * s * s,
                         -0.5f * (13 * std::cos(t) - 5 * std::cos(2 * t) - 2 * std::cos(3 * t) - std::cos(4 * t))});
        }
        return v;
    }();
    return pts;
}

void desenhar_bloon(Caneta& p, const std::string& tipo, bool camo, bool regen, bool fort, int dano) {
    auto it = COR_BLOON().find(tipo);
    const Color C = it != COR_BLOON().end() ? it->second : H(0xE8312A);
    p.ln({{64, 100}, {61, 110}, {66, 120}}, 2, H(0x3A3640));
    p.poly({{58, 93}, {70, 93}, {64, 102}}, esc(C, 0.6f), b3);
    // corpo: elipse escura com contorno + elipse clara deslocada (sombra em crescente sem recorte)
    p.e(64, 58, 30, 37, tipo == "branco" || tipo == "zebra" ? H(0xBFC4CE) : esc(C, 0.62f), B);
    p.e(62.5f, 55.5f, 28.2f, 33.5f, C);
    if (tipo == "zebra") {
        p.e(50, 50, 4, 20, TINTA, 0, 20);
        p.e(64, 58, 4.5f, 27, TINTA, 0, 20);
        p.e(78, 62, 3.5f, 17, TINTA, 0, 20);
    }
    if (tipo == "arco_iris") {
        p.e(62, 56, 22, 26, H(0xF7941D));
        p.e(62, 57, 16, 19, H(0xFFD21F));
        p.e(62, 58, 10.5f, 12.5f, H(0x62C23A));
        p.e(62, 58.5f, 5.5f, 6.5f, H(0x2F8FE6));
    }
    if (tipo == "chumbo") {
        p.e(62, 56, 19, 24, H(0xB4BCC7));
        for (Vector2 q : {Vector2{50, 40}, Vector2{76, 42}, Vector2{48, 72}, Vector2{76, 74}}) p.c(q.x, q.y, 2.2f, H(0x6B7480));
    }
    if (tipo == "ceramica") {
        p.e(62, 56, 18, 23, H(0xD79A5E));
        p.ringE(62, 56, 18, 23, 2.5f, H(0x7A4217));
        p.ln({{62, 33}, {58, 44}, {64, 50}}, 2, H(0x7A4217));
        // rachaduras quando a ceramica perde vida
        if (dano >= 1) p.ln({{44, 34}, {52, 46}, {46, 56}, {54, 68}}, 2.5f, TINTA);
        if (dano >= 2) p.ln({{82, 44}, {74, 56}, {84, 66}, {78, 80}}, 2.5f, TINTA);
    }
    if (camo) {
        const float m[5][6] = {{50, 46, 8, 5, 0, 20}, {72, 52, 9, 6, 1, -15}, {56, 72, 8, 5, 2, 10}, {75, 74, 6, 4, 0, 30}, {64, 32, 7, 4, 1, 0}};
        const Color cores[3] = {H(0x4E6B2E), H(0x7A8A3A), H(0x5C4424)};
        for (auto& q : m) p.e(q[0], q[1], q[2], q[3], cores[static_cast<int>(q[4])], 0, q[5]);
    }
    p.e(52, 40, 6.5f, 11, tipo == "preto" ? H(0xFFFFFF, 0x66) : H(0xFFFFFF, 0xD9), 0, 30);
    if (fort) {
        p.rr(59, 23, 10, 70, 5, H(0xA9B2BD), b3);
        p.rr(35, 52, 58, 10, 5, H(0xA9B2BD), b3);
        for (Vector2 q : {Vector2{42, 57}, Vector2{86, 57}, Vector2{64, 30}, Vector2{64, 86}}) p.c(q.x, q.y, 1.8f, TINTA);
    }
    if (regen) p.g(g_em(94, 26).anim(Anim::pulsa(0.9f, 1.12f, 0.8f)), [&] { p.poly(CORACAO(), H(0xFF5FA2), b3); });
}

// ================================================================ dirigiveis (220x120)
struct Dir {
    Color c, f;
    float rx, ry;
};
const std::map<std::string, Dir>& DIR() {
    static const std::map<std::string, Dir> m = {
        {"moab", {H(0x2F80DA), H(0xF2F2F5), 70, 32}}, {"bfb", {H(0xD8302A), H(0xF2F2F5), 76, 36}},
        {"zomg", {H(0x6FCB2E), H(0x1C1B20), 82, 40}}, {"ddt", {H(0x3C4A33), H(0x1C1B20), 72, 27}},
        {"bad", {H(0x7E36C4), H(0xF2F2F5), 90, 46}},
    };
    return m;
}

void desenhar_dirigivel(Caneta& p, const std::string& tipo, int dano, bool fort) {
    auto it = DIR().find(tipo);
    const Dir& D = it != DIR().end() ? it->second : DIR().at("moab");
    const Color c = D.c, fin = esc(c, 0.55f);
    const float rx = D.rx, ry = D.ry, x = 112, y = 60;
    const bool escura = D.f.r < 100;
    p.poly({{x - rx + 16, y - 6}, {x - rx - 12, y - ry - 12}, {x - rx + 6, y - ry - 12}, {x - rx + 36, y - ry * 0.5f}}, fin, B);
    p.poly({{x - rx + 16, y + 6}, {x - rx - 12, y + ry + 12}, {x - rx + 6, y + ry + 12}, {x - rx + 36, y + ry * 0.5f}}, fin, B);
    if (tipo == "bfb" || tipo == "bad" || tipo == "zomg") p.poly({{x - rx + 12, y - 5}, {x - rx - 18, y}, {x - rx + 12, y + 5}}, fin, B);
    p.e(x, y, rx, ry, esc(c, 0.62f), B);
    p.e(x - rx * 0.04f, y - ry * 0.1f, rx * 0.96f, ry * 0.88f, c);
    if (tipo == "ddt")
        for (auto q : {std::array<float, 3>{x - 30, y - 12, 7}, std::array<float, 3>{x + 4, y - 16, 5},
                       std::array<float, 3>{x + 30, y + 10, 7}, std::array<float, 3>{x - 8, y + 12, 5}})
            p.c(q[0], q[1], q[2], H(0x56663F));
    // faixas
    if (tipo == "zomg") {
        for (float dx : {-22.0f, 22.0f}) p.rr(x + dx - 6, y - ry * 0.82f, 12, ry * 1.64f, 6, D.f);
    } else if (tipo == "bad") {
        for (float k : {-0.3f, 0.22f}) p.rr(x - rx * 0.5f, y + ry * k - 5, rx, 10, 5, D.f, b3);
    } else {
        p.rr(x - rx * 0.45f, y - ry * 0.2f, rx * 0.9f, ry * 0.4f, 5, D.f, b3);
    }
    if (tipo != "zomg")
        for (int i = 0; i < 4; ++i)
            p.c(x - rx * 0.35f + i * rx * 0.23f, y + (tipo == "bad" ? ry * 0.22f : 0), 1.8f, escura ? H(0x6B6B75) : H(0x9AA3AE));
    p.e(x - rx * 0.2f, y - ry * 0.6f, rx * 0.4f, ry * 0.13f, H(0xFFFFFF, 0x80));
    if (fort)
        for (float k : {-0.4f, 0.0f, 0.4f}) {
            const float hh = ry * std::sqrt(1 - k * k) * 0.9f;
            p.rr(x + rx * k - 3.5f, y - hh, 7, hh * 2, 3, H(0xA9B2BD), b3);
        }
    if (dano >= 1) p.ln({{x - 12, y - ry * 0.78f}, {x - 5, y - ry * 0.45f}, {x - 14, y - ry * 0.15f}, {x - 7, y + ry * 0.1f}}, 2.5f, TINTA);
    if (dano >= 2) {
        p.e(x + rx * 0.35f, y + ry * 0.38f, rx * 0.14f, ry * 0.2f, esc(c, 0.42f));
        p.ln({{x + rx * 0.5f, y - ry * 0.7f}, {x + rx * 0.42f, y - ry * 0.35f}, {x + rx * 0.52f, y - ry * 0.1f}}, 2.5f, TINTA);
    }
    if (dano >= 3) {
        p.rr(x - rx * 0.6f, y + ry * 0.15f, rx * 0.18f, ry * 0.34f, 3, esc(c, 0.35f), 2);
        p.g(g_em(x + rx * 0.3f, y - ry * 0.95f).anim(Anim::flutua(-12, 1.4f)), [&] {
            p.g(G().anim(Anim::some(1.4f)), [&] {
                p.c(0, 0, 8, al(H(0x55555E), 0.75f));
                p.c(8, -6, 6, al(H(0x8E97A4), 0.7f));
            });
        });
    }
    if (dano >= 4) {
        p.poly({{x - rx * 0.15f, y + ry * 0.2f}, {x - rx * 0.02f, y + ry * 0.1f}, {x + rx * 0.1f, y + ry * 0.35f}, {x - rx * 0.05f, y + ry * 0.6f}},
               TINTA);
        p.g(g_em(x - rx * 0.02f, y + ry * 0.2f).anim(Anim::pulsa(0.8f, 1.2f, 0.3f)), [&] {
            p.poly({{-6, 0}, {6, 0}, {0, -16}}, H(0xF7941D), 2);
            p.poly({{-3, 0}, {3, 0}, {0, -8}}, H(0xFFD632));
        });
        p.g(g_em(x - rx * 0.45f, y - ry * 0.9f).anim(Anim::flutua(-14, 1.8f, 0, 0.6f)),
            [&] { p.g(G().anim(Anim::some(1.8f)), [&] { p.c(0, 0, 9, al(H(0x3E4550), 0.7f)); }); });
    }
    p.c(x + rx * 0.78f, y - ry * 0.1f, ry * 0.2f, tipo == "bad" ? H(0xFFD632) : BRANCO, b3);
    p.c(x + rx * 0.8f, y - ry * 0.1f, ry * 0.09f, tipo == "bad" ? H(0xE8312A) : TINTA);
}

// ================================================================ habilidades
const std::map<std::string, Color>& COR_EFEITO() {
    static const std::map<std::string, Color> m = {
        {"turbo", H(0x3A96E6)},        {"turbo_area", H(0x5CC43C)},     {"dano_global", H(0xE8312A)},
        {"dano_forte", H(0x8E1E16)},   {"congelar_global", H(0x8FD3FF)}, {"lentidao", H(0x8B3FD9)},
        {"dinheiro", H(0x2BA84A)},     {"spikes_local", H(0x8E97A4)},   {"spikes_global", H(0x6B7480)},
        {"invocar", H(0xF7941D)},      {"reverso", H(0x1C1B20)},        {"roubo", H(0x6E4523)},
    };
    return m;
}

void glifo(Caneta& p, const std::string& ef, float x, float y) {
    if (ef == "turbo") {
        p.poly({{x - 8, y + 2}, {x, y - 8}, {x + 8, y + 2}, {x + 4, y + 2}, {x, y - 2}, {x - 4, y + 2}}, H(0xFFD632));
        p.poly({{x - 8, y + 10}, {x, y}, {x + 8, y + 10}, {x + 4, y + 10}, {x, y + 6}, {x - 4, y + 10}}, H(0xFFD632));
    } else if (ef == "turbo_area") {
        p.ring(x, y, 9, 12, H(0x9BE66E));
        p.poly({{x, y - 8}, {x + 6, y}, {x + 2.5f, y}, {x + 2.5f, y + 7}, {x - 2.5f, y + 7}, {x - 2.5f, y}, {x - 6, y}}, BRANCO);
    } else if (ef == "dano_global") {
        p.star(x, y, 12, 5, 8, H(0xF7941D));
    } else if (ef == "dano_forte") {
        p.ring(x, y, 7, 10, H(0xFF4D5E));
        p.rr(x - 1.5f, y - 13, 3, 26, 1.5f, H(0xFF4D5E));
        p.rr(x - 13, y - 1.5f, 26, 3, 1.5f, H(0xFF4D5E));
    } else if (ef == "congelar_global") {
        for (float a : {0.0f, 60.0f, 120.0f}) p.rr(x - 1.8f, y - 12, 3.6f, 24, 1.8f, BRANCO, 0, a);
    } else if (ef == "lentidao") {
        p.ring(x, y, 9, 12, BRANCO);
        p.ln({{x, y}, {x, y - 7}, {x, y}, {x + 5, y + 3}}, 2.5f, BRANCO);
    } else if (ef == "dinheiro") {
        p.c(x, y, 11, H(0xFFD632));
        p.rr(x - 1.5f, y - 6, 3, 12, 1.5f, H(0xC98A0E));
    } else if (ef == "spikes_local") {
        p.star(x, y, 12, 4, 4, H(0xC7CDD6));
    } else if (ef == "spikes_global") {
        p.star(x - 5, y, 8, 3, 4, H(0xC7CDD6));
        p.star(x + 6, y + 3, 8, 3, 4, H(0xC7CDD6));
    } else if (ef == "reverso") {
        p.c(x, y, 11, H(0xFFD632));
        p.c(x + 5, y - 3, 9, H(0x16141A));
    } else if (ef == "roubo") {
        p.c(x, y + 2, 10, H(0xC9A06A));
        p.rr(x - 5, y - 11, 10, 6, 3, H(0xC9A06A));
        p.rr(x - 1.5f, y - 2, 3, 9, 1.5f, H(0x6E4523));
    } else {  // invocar
        p.star(x, y, 13, 3, 4, BRANCO);
        p.c(x, y, 3, H(0xFFD632));
    }
}

}  // namespace

// ================================================================ interface
bool eh_maquina(const std::string& chave) { return MAQ().count(chave) > 0 || chave == "churchill"; }
bool voa(const std::string& chave) {
    auto it = MAQ().find(chave);
    return it != MAQ().end() && it->second.voa;
}

void torre(Caneta& p, const std::string& chave, Vista v, int cam, int tier, int nivel) {
    if (chave == "churchill") {
        maquina(p, v, false, [](Caneta& pp, Color c, int, int) { pp.rr(24, 24, 80, 82, 10, c, B); }, [&] { tanque(p, nivel); }, 0, 0);
        return;
    }
    auto mq = MAQ().find(chave);
    if (mq != MAQ().end()) {
        const Maquina& M = mq->second;
        maquina(p, v, M.voa, M.pe, [&] { M.d(p, cam, tier); }, cam, tier);
        return;
    }
    auto hs = HS().find(chave);
    if (hs != HS().end()) {
        Estilo s = hs->second[0];
        if (nivel >= 10) mesclar(s, hs->second[1]);
        if (nivel >= 20) mesclar(s, hs->second[2]);
        macaco(p, v, s);
        return;
    }
    auto ts = TS().find(chave);
    const Macaco& T = ts != TS().end() ? ts->second : TS().at("dardo");
    Estilo s = T.b;
    if (cam >= 0 && cam < 3 && tier >= 3) mesclar(s, T.c[cam][0]);
    if (cam >= 0 && cam < 3 && tier == 5) mesclar(s, T.c[cam][1]);
    macaco(p, v, s);
}

void bloon(Caneta& p, const std::string& tipo, bool camo, bool regen, bool fort, bool flutuar, int dano) {
    if (flutuar) p.g(G().anim(Anim::flutua(-4, 1.8f)), [&] { desenhar_bloon(p, tipo, camo, regen, fort, dano); });
    else desenhar_bloon(p, tipo, camo, regen, fort, dano);
}

void dirigivel(Caneta& p, const std::string& tipo, int dano, bool fort, bool flutuar) {
    if (flutuar) p.g(G().anim(Anim::flutua(-3, 2.2f)), [&] { desenhar_dirigivel(p, tipo, dano, fort); });
    else desenhar_dirigivel(p, tipo, dano, fort);
}

Vector2 raios_dirigivel(const std::string& tipo) {
    auto it = DIR().find(tipo);
    const Dir& D = it != DIR().end() ? it->second : DIR().at("moab");
    return {D.rx, D.ry};
}

bool tem_projetil(const std::string& visual) { return PROJ().count(visual) > 0; }
void projetil(Caneta& p, const std::string& visual) { proj(p, visual); }

void efeito(Caneta& p, const std::string& k) {
    if (k == "estouro") {
        p.g(g_em(64, 64).anim(Anim::cresce(0.3f, 1.2f, 0.7f)), [&] {
            p.g(G().anim(Anim::some(0.7f)), [&] {
                p.star(0, 0, 40, 18, 10, BRANCO, B);
                p.star(0, 0, 24, 11, 10, H(0xFFD632));
            });
        });
        for (int i = 0; i < 6; ++i) {
            const float a = i * 60.0f + 20;
            p.g(g_em(64, 64).anim(Anim::voa(std::cos(rad(a)) * 48, std::sin(rad(a)) * 48, 0.7f)),
                [&] { p.g(G().anim(Anim::some(0.7f)), [&] { p.rr(-5, -3, 10, 6, 2, H(0xE8312A), 2, a); }); });
        }
    } else if (k == "explosao") {
        p.g(g_em(64, 64).anim(Anim::cresce(0.3f, 1.1f, 0.9f)), [&] {
            p.g(G().anim(Anim::some(0.9f)), [&] {
                p.c(0, 0, 44, H(0xF7941D), B);
                p.c(-4, -4, 30, H(0xFFD632));
                p.c(-6, -6, 14, BRANCO);
            });
        });
        const Vector2 fum[3] = {{40, 44}, {88, 50}, {60, 90}};
        for (int i = 0; i < 3; ++i)
            p.g(g_em(fum[i].x, fum[i].y).anim(Anim::flutua(-16, 0.9f, 0, i * 0.2f)), [&] { p.c(0, 0, 10, al(H(0x55555E), 0.7f)); });
    } else if (k == "dinheiro") {
        p.g(g_em(64, 80).anim(Anim::voa(0, -40, 1.4f)), [&] {
            p.g(G().anim(Anim::some(1.4f)), [&] {
                p.c(-22, 0, 14, H(0xFFD632), b3);
                p.rr(-23.5f, -6, 3, 12, 1.5f, H(0xC98A0E));
                p.txt(14, 0, "+$", 30, H(0xFFE046), 4);
            });
        });
    } else if (k == "nivel") {
        p.g(g_em(64, 64).anim(Anim::pulsa(0.9f, 1.1f, 0.6f)), [&] { p.ring(0, 0, 40, 46, al(H(0xFFD632), 0.7f)); });
        p.g(g_em(64, 70).anim(Anim::flutua(-10, 0.8f)), [&] {
            p.poly({{0, -34}, {24, -6}, {10, -6}, {10, 22}, {-10, 22}, {-10, -6}, {-24, -6}}, H(0x5CC43C), B);
            p.star(0, -4, 7, 3, 5, H(0xFFD632), 2);
        });
    } else if (k == "impacto") {
        p.g(g_em(64, 64).anim(Anim::cresce(0.4f, 1.1f, 0.4f)),
            [&] { p.g(G().anim(Anim::some(0.4f)), [&] { p.star(0, 0, 30, 10, 8, H(0xFFD632), b3); }); });
        for (int i = 0; i < 5; ++i) {
            const float a = -150.0f + i * 30;
            p.g(g_em(64, 64).anim(Anim::voa(std::cos(rad(a)) * 50, std::sin(rad(a)) * 50, 0.5f)),
                [&] { p.g(G().anim(Anim::some(0.5f)), [&] { p.c(0, 0, 3, H(0xFFE046), 1.5f); }); });
        }
    } else if (k == "camo_revelado") {
        p.g(g_em(64, 58).anim(Anim::cresce(0.4f, 1.3f, 1.2f)),
            [&] { p.g(G().anim(Anim::some(1.2f)), [&] { p.ring(0, 0, 36, 40, BRANCO); }); });
    }
}

void estado_congelado(Caneta& p) {
    p.rr(26, 18, 76, 94, 18, al(H(0xBFE6FF), 0.7f), b3);
    p.poly({{40, 26}, {52, 26}, {36, 60}}, H(0xFFFFFF, 0xAA));
    p.g(g_em(96, 22).anim(Anim::pisca(0.2f, 1, 0.8f)), [&] { p.star(0, 0, 9, 3, 4, BRANCO, 2); });
}

void estado_colado(Caneta& p) {
    p.e(58, 40, 22, 12, H(0x9BE66E), b3);
    const Vector2 gotas[3] = {{44, 50}, {60, 54}, {74, 48}};
    for (int i = 0; i < 3; ++i)
        p.g(g_em(gotas[i].x, gotas[i].y).anim(Anim::flutua(10, 1.2f, 0, i * 0.3f)), [&] { p.e(0, 0, 4, 7, H(0x9BE66E), 2); });
}

void estado_queimando(Caneta& p) {
    const Vector2 ch[3] = {{50, 30}, {64, 24}, {78, 32}};
    for (int i = 0; i < 3; ++i)
        p.g(g_em(ch[i].x, ch[i].y).anim(Anim::pulsa(0.8f, 1.2f, 0.3f, i * 0.1f)), [&] {
            p.poly({{-7, 6}, {7, 6}, {0, -14}}, H(0xF7941D), 2);
            p.poly({{-3, 6}, {3, 6}, {0, -4}}, H(0xFFD632));
        });
}

void estado_atordoado(Caneta& p) {
    p.g(g_em(64, 22).esc(1, 0.4f), [&] {
        p.g(G().anim(Anim::gira(1.2f)), [&] {
            for (float a : {0.0f, 120.0f, 240.0f})
                p.star(std::cos(rad(a)) * 26, std::sin(rad(a)) * 26, 9, 4, 5, H(0xFFD632), 2);
        });
    });
}

Color cor_efeito(const std::string& ef) {
    auto it = COR_EFEITO().find(ef);
    return it != COR_EFEITO().end() ? it->second : H(0x3A96E6);
}

void habilidade(Caneta& p, const std::string& dono, bool heroi, int cam, int tier, int nivel, const std::string& ef) {
    const Color bg = cor_efeito(ef);
    p.c(64, 64, 58, H(0xFFD632), B);
    p.c(64, 64, 50, bg);
    p.e(50, 32, 22, 9, H(0xFFFFFF, 0x33), 0, -20);
    p.g(g_em(64, 70).esc(0.72f), [&] {
        p.g(g_em(-64, -64), [&] {
            if (heroi) torre(p, dono, Vista::FRENTE, -1, 0, nivel >= 10 ? 10 : 1);
            else torre(p, dono, Vista::FRENTE, cam, tier >= 5 ? 5 : 3);
        });
    });
    p.c(102, 102, 19, TINTA);
    p.c(102, 102, 16, esc(bg, 0.8f));
    glifo(p, ef, 102, 102);
}

void icone(Caneta& p, const std::string& nome) {
    if (nome == "coracao") {
        std::vector<Vector2> v{{16, 28}, {4, 15}};
        for (int i = 1; i < 12; ++i) {
            const float a = rad(143.13f + 180.0f * i / 12);
            v.push_back({10 + std::cos(a) * 7.5f, 10.5f + std::sin(a) * 7.5f});
        }
        v.push_back({16, 6});
        for (int i = 1; i < 12; ++i) {
            const float a = rad(216.87f + 180.0f * i / 12);
            v.push_back({22 + std::cos(a) * 7.5f, 10.5f + std::sin(a) * 7.5f});
        }
        v.push_back({28, 15});
        p.poly(v, H(0xFF4D5E), 1.5f);
        p.e(10, 11, 3, 2, H(0xFFFFFF, 0xB3));
    } else if (nome == "moeda") {
        p.c(16, 16, 13, H(0xFFD632), 1.5f);
        p.ring(16, 16, 7.5f, 9.5f, H(0xC98A0E));
        p.rr(14.5f, 9, 3, 14, 1.5f, H(0xC98A0E));
        p.e(11, 10, 3, 2, H(0xFFFFFF, 0xB3));
    } else if (nome == "eco") {
        p.c(16, 16, 13, H(0x2BA84A), 1.5f);
        p.poly({{16, 6}, {25, 16}, {20, 16}, {20, 25}, {12, 25}, {12, 16}, {7, 16}}, BRANCO, 0.9f);
    } else if (nome == "rodada") {
        p.rr(6, 4, 3.5f, 25, 1.5f, H(0x8A5A2E), 1);
        p.poly({{9, 5}, {28, 10}, {9, 17}}, H(0xFFD632), 1.5f);
    } else if (nome == "cadeado") {
        p.ln({{10, 15}, {10, 11}}, 3, H(0xC7CDD6), 1.5f);
        p.ln({{22, 15}, {22, 11}}, 3, H(0xC7CDD6), 1.5f);
        p.arc(16, 11, 6, 180, 360, 3, H(0xC7CDD6), 1.5f);
        p.rr(6, 14, 20, 15, 4, H(0xFFD632), 1.5f);
        p.c(16, 21, 2.2f, TINTA);
    } else if (nome == "estrela") {
        p.poly({{16, 3}, {20, 12}, {29, 12}, {22, 18}, {25, 28}, {16, 22}, {7, 28}, {10, 18}, {3, 12}, {12, 12}}, H(0xFFD632), 1.5f);
    } else if (nome == "oponente") {
        p.c(16, 12, 7, H(0xE8312A), 1.5f);
        p.g(g_em(16, 29).esc(1, 0.83f), [&] { p.sec(0, 0, 12, 180, 360, H(0xE8312A), 1.5f); });
    }
}

}  // namespace bl::spr
