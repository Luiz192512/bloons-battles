#include "cliente/render.hpp"

#include <algorithm>
#include <cmath>
#include <map>

#include "cliente/arte.hpp"
#include "cliente/som.hpp"
#include "cliente/ui.hpp"

namespace bl {

using ui::rgb;

namespace {

constexpr float PI_F = 3.14159265358979f;

Color cor_efeito(const std::string& vis, Color padrao) {
    static const std::map<std::string, Color> m = {
        {"congelar", rgb(170, 225, 255)},  {"anel_fogo", rgb(255, 130, 30)}, {"radiacao", rgb(120, 255, 90)},
        {"impacto", rgb(150, 110, 70)},     {"espadas", rgb(230, 230, 240)},  {"vento", rgb(240, 250, 255)},
        {"orbita_glaive", rgb(220, 220, 235)}, {"relampago", rgb(140, 220, 255)},
        {"raio_plasma", rgb(255, 90, 220)}, {"bala", rgb(255, 240, 150)},     {"psi", rgb(200, 120, 255)},
    };
    auto it = m.find(vis);
    return it == m.end() ? padrao : it->second;
}

void sombra(float x, float y, float tam) {
    DrawEllipse(static_cast<int>(x + tam / 2), static_cast<int>(y + tam / 4), tam / 2, tam / 4, rgb(0, 0, 0, 70));
}

}  // namespace

RenderPista::RenderPista(Pista& pista, std::string chave_mapa, bool sons)
    : pista_(pista), chave_mapa_(std::move(chave_mapa)), sons_(sons) {}

// ---------------------------------------------------------- eventos da simulacao
void RenderPista::consumir_eventos() {
    auto& ev = pista_.eventos;
    if (ev.empty()) return;
    int pops = 0;
    for (const Evento& e : ev) {
        const std::string& tipo = e.tipo;
        if (tipo == "pop") {
            if (++pops <= 40) efeitos_.push_back({"pop", e, 0, 0.12});
        } else if (tipo == "explosao") {
            efeitos_.push_back({"explosao", e, 0, 0.28});
            if (sons_) som::tocar("explosao", 90);
        } else if (tipo == "raio") {
            efeitos_.push_back({"raio", e, 0, 0.09});
        } else if (tipo == "aura") {
            efeitos_.push_back({"aura", e, 0, 0.3});
        } else if (tipo == "dinheiro") {
            Evento t = e;
            t.s = "+$" + ui::formatar(e.v);
            t.cor = {120, 255, 90};
            efeitos_.push_back({"texto", t, 0, 1.2});
        } else if (tipo == "nivel") {
            Evento t = e;
            t.s = "NÍVEL " + std::to_string(static_cast<int>(e.v)) + "!";
            t.cor = {255, 220, 60};
            efeitos_.push_back({"texto", t, 0, 1.6});
            if (sons_) som::tocar("upgrade");
        } else if (tipo == "habilidade") {
            Evento t = e;
            t.cor = {255, 255, 255};
            efeitos_.push_back({"texto", t, 0, 1.4});
            if (sons_) som::tocar("habilidade");
        } else if (tipo == "flash") {
            flash_ = true;
            flash_cor_ = e.cor;
            flash_t_ = 0;
        } else if (tipo == "vazou") {
            efeitos_.push_back({"vazou", e, 0, 0.5});
            if (sons_) som::tocar("vazou", 200);
        } else if (tipo == "colocar" || tipo == "upgrade" || tipo == "venda" || tipo == "invocar") {
            efeitos_.push_back({"anel", e, 0, 0.35});
            if (sons_) som::tocar(tipo == "invocar" ? "colocar" : tipo);
        } else if (tipo == "regen") {
            efeitos_.push_back({"regen", e, 0, 0.3});
        } else if (tipo == "fim_rodada" || tipo == "eco" || tipo == "envio") {
            avisos.push_back(e);
        }
    }
    if (pops && sons_) som::tocar("pop", 45);
    ev.clear();
    if (efeitos_.size() > 500) efeitos_.erase(efeitos_.begin(), efeitos_.end() - 500);
}

void RenderPista::atualizar(double dt) {
    for (auto& f : efeitos_) f.t += dt;
    efeitos_.erase(std::remove_if(efeitos_.begin(), efeitos_.end(), [](const Efeito& f) { return f.t >= f.dur; }),
                   efeitos_.end());
    if (flash_) {
        flash_t_ += dt;
        if (flash_t_ > 0.35) flash_ = false;
    }
}

// ---------------------------------------------------------- desenho completo
void RenderPista::desenhar(int selecionada) {
    arte::fundo_mapa(chave_mapa_, {0, 0, LARGURA_MAPA, ALTURA_MAPA});
    for (auto& s : pista_.pilhas) arte::pilha(*s);
    desenhar_bloons();
    std::vector<const Torre*> ordem;
    for (auto& [id, t] : pista_.torres) ordem.push_back(t.get());
    std::stable_sort(ordem.begin(), ordem.end(), [](const Torre* a, const Torre* b) {
        bool va = a->dfn->mov != Mov::FIXO, vb = b->dfn->mov != Mov::FIXO;
        if (va != vb) return !va;
        return a->y < b->y;
    });
    for (const Torre* t : ordem) desenhar_torre(*t, t->id == selecionada);
    for (auto& p : pista_.projeteis) arte::projetil(*p);
    desenhar_efeitos();
}

void RenderPista::desenhar_torre(const Torre& t, bool sel) {
    int tam = t.dfn->heroi ? 58 : static_cast<int>(t.dfn->raio * 2.7);
    if (t.temporaria) tam = static_cast<int>(tam * 0.8);
    int tier;
    if (t.dfn->heroi) tier = (t.nivel >= 10 ? 3 : 0) + (t.nivel >= 20 ? 2 : 0);
    else tier = *std::max_element(t.caminhos.begin(), t.caminhos.end());
    const float x = static_cast<float>(t.x), y = static_cast<float>(t.y);
    if (t.dfn->mov == Mov::FIXO) sombra(x - tam / 2.0f, y + tam * 0.1f, static_cast<float>(tam));
    else sombra(x - tam / 2.0f + 14, y + 22, static_cast<float>(tam));
    // o desenho original gira em passos de 10 graus
    float ang10 = std::round(static_cast<float>(t.ang) / 10.0f) * 10.0f;
    arte::torre(t.chave, x, y, tam, tier, ang10 + 90);
    if (sel) DrawRing({x, y}, tam / 2.0f + 2, tam / 2.0f + 4, 0, 360, 48, WHITE);
    if (t.dfn->heroi)
        ui::texto(std::to_string(t.nivel), x + tam / 3.0f, y + tam / 3.0f, 14, rgb(255, 230, 90), 2, ui::Ancora::CENTER);
    if (t.turbo < 1.0) DrawRing({x, y}, tam / 2.0f, tam / 2.0f + 2, 0, 360, 48, rgb(255, 200, 60));
}

void RenderPista::desenhar_bloons() {
    // dirigiveis por baixo, bloons pequenos por cima (desenhados de tras para frente)
    std::vector<const Bloon*> ordem;
    for (auto& b : pista_.bloons)
        if (b->vivo) ordem.push_back(b.get());
    std::stable_sort(ordem.begin(), ordem.end(), [](const Bloon* a, const Bloon* b) {
        if (a->tipo->moab != b->tipo->moab) return a->tipo->moab;
        return a->d < b->d;
    });
    const double agora = ui::tempo();
    for (const Bloon* b : ordem) {
        const float x = std::floor(static_cast<float>(b->x)), y = std::floor(static_cast<float>(b->y));
        if (b->tipo->moab) {
            double frac = b->vida / std::max(1.0, b->vida_max);
            int dano = frac > 0.75 ? 0 : frac > 0.5 ? 1 : frac > 0.25 ? 2 : 3;
            float ang8 = std::round(static_cast<float>(b->ang) / 8.0f) * 8.0f;
            arte::dirigivel(*b->tipo, b->fort, dano, x, y, ang8);
        } else {
            int dano = 0;
            if (b->tipo->nome == "ceramica") {
                double frac = b->vida / std::max(1.0, b->vida_max);
                dano = frac > 0.7 ? 0 : frac > 0.4 ? 1 : 2;
            }
            arte::bloon(*b->tipo, b->camo, b->regen, b->fort, dano, x, y);
        }
        const float r = std::floor(static_cast<float>(b->tipo->raio));
        if (b->cong_t > 0) {
            DrawCircleV({x, y}, r * 1.2f, rgb(190, 235, 255, 150));
            DrawRing({x, y}, r * 1.2f - 2, r * 1.2f, 0, 360, 36, rgb(240, 250, 255, 220));
        }
        if (b->cola_t > 0) DrawCircleV({x + r / 3, y - r / 3}, std::max(3.0f, r / 2), rgb(150, 210, 60));
        if (b->queima_t > 0) {
            DrawCircleV({x - r / 2, y + r / 3}, std::max(2.0f, r / 3), rgb(255, 140, 30));
            DrawCircleV({x - r / 2, y + r / 3 - 2}, std::max(1.0f, r / 5), rgb(255, 220, 80));
        }
        if (b->atord_t > 0) {
            for (int k = 0; k < 3; ++k) {
                float a = static_cast<float>(agora * 1000 / 150.0) + k * 2.1f;
                DrawCircleV({x + std::cos(a) * r, y - r - 4 + std::sin(a) * 3}, 3, rgb(255, 240, 90));
            }
        }
    }
}

void RenderPista::desenhar_efeitos() {
    for (const Efeito& f : efeitos_) {
        const float k = static_cast<float>(f.t / f.dur);
        const Evento& d = f.dados;
        const float x = static_cast<float>(d.x), y = static_cast<float>(d.y);
        if (f.tipo == "pop") {
            const float r = 10 + 6 * k;
            std::vector<Vector2> pts;
            for (int i = 0; i < 12; ++i) {
                float a = i * PI_F / 6, rr = i % 2 == 0 ? r : r * 0.55f;
                pts.push_back({x + std::cos(a) * rr, y + std::sin(a) * rr});
            }
            for (size_t i = 0; i < pts.size(); ++i) DrawTriangle({x, y}, pts[i], pts[(i + 1) % pts.size()], WHITE);
            for (size_t i = 0; i < pts.size(); ++i) DrawLineEx(pts[i], pts[(i + 1) % pts.size()], 2, rgb(30, 30, 30));
        } else if (f.tipo == "explosao") {
            const float raio = static_cast<float>(d.v);
            const int a = static_cast<int>(220 * (1 - k));
            DrawCircleV({x, y}, raio * (0.6f + 0.5f * k), rgb(255, 150, 40, a));
            DrawCircleV({x, y}, raio * 0.45f * (1 - k * 0.5f), rgb(255, 230, 120, a));
        } else if (f.tipo == "raio") {
            const float x2 = static_cast<float>(d.x2), y2 = static_cast<float>(d.y2);
            const std::string vis = d.s.empty() ? "bala" : d.s;
            const Color c = cor_efeito(vis, rgb(255, 240, 150));
            if (vis == "relampago") {
                std::vector<Vector2> pts{{x, y}};
                std::uniform_real_distribution<float> u(-8, 8);
                for (int i = 1; i < 6; ++i) {
                    float t = i / 6.0f;
                    pts.push_back({x + (x2 - x) * t + u(rng_), y + (y2 - y) * t + u(rng_)});
                }
                pts.push_back({x2, y2});
                for (size_t i = 1; i < pts.size(); ++i) DrawLineEx(pts[i - 1], pts[i], 3, c);
                for (size_t i = 1; i < pts.size(); ++i) DrawLineEx(pts[i - 1], pts[i], 1, WHITE);
            } else {
                DrawLineEx({x, y}, {x2, y2}, vis == "raio_plasma" ? 6.0f : 2.0f, c);
            }
        } else if (f.tipo == "aura") {
            const float raio = static_cast<float>(d.v);
            const Color c = cor_efeito(d.s, WHITE);
            const int a = static_cast<int>(140 * (1 - k));
            const float rr = raio * (0.3f + 0.7f * k);
            if (d.s == "congelar") DrawCircleV({x, y}, rr, ui::com_alfa(c, a / 2));
            if (a) DrawRing({x, y}, std::max(0.0f, rr - 4), rr, 0, 360, 64, ui::com_alfa(c, a + 60));
        } else if (f.tipo == "texto") {
            ui::texto(d.s, x, y - 30 * k - 20, 18, arte::cor(d.cor), 2, ui::Ancora::CENTER);
        } else if (f.tipo == "anel") {
            const float r = 10 + 30 * k;
            DrawRing({x, y}, r - 3, r, 0, 360, 48, WHITE);
        } else if (f.tipo == "regen") {
            const float r = 8 + 10 * k;
            DrawRing({x, y}, r - 2, r, 0, 360, 32, rgb(255, 120, 180));
        } else if (f.tipo == "vazou") {
            ui::texto("-" + std::to_string(static_cast<int>(d.v)), x, y - 30 * k, 18, rgb(255, 80, 80), 2,
                      ui::Ancora::CENTER);
        }
    }
    if (flash_) {
        DrawRectangle(0, 0, LARGURA_MAPA, ALTURA_MAPA,
                      arte::cor(flash_cor_, static_cast<unsigned char>(150 * (1 - flash_t_ / 0.35))));
    }
}

// ---------------------------------------------------------- miniatura (oponente)
void RenderPista::desenhar_mini(Rectangle r) {
    const float esc = r.width / LARGURA_MAPA;
    arte::fundo_mapa(chave_mapa_, r);
    for (auto& s : pista_.pilhas) arte::pilha(*s, r.x, r.y, esc);
    for (auto& [id, t] : pista_.torres) {
        int tam = std::max(10, static_cast<int>(t->dfn->raio * 2.7 * esc * 1.3));
        int tier = t->dfn->heroi ? 0 : *std::max_element(t->caminhos.begin(), t->caminhos.end());
        arte::torre(t->chave, r.x + std::floor(static_cast<float>(t->x) * esc),
                    r.y + std::floor(static_cast<float>(t->y) * esc), tam, tier);
    }
    for (auto& b : pista_.bloons) {
        if (!b->vivo) continue;
        float rr = std::max(2.0f, std::floor(static_cast<float>(b->tipo->raio) * esc * 1.2f));
        Vector2 c{r.x + std::floor(static_cast<float>(b->x) * esc), r.y + std::floor(static_cast<float>(b->y) * esc)};
        DrawCircleV(c, rr + 1, BLACK);
        DrawCircleV(c, rr, arte::cor(b->tipo->cor));
    }
    const size_t n = std::min<size_t>(pista_.projeteis.size(), 200);
    for (size_t i = 0; i < n; ++i) arte::projetil(*pista_.projeteis[i], r.x, r.y, esc);
}

}  // namespace bl
