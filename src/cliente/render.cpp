#include "cliente/render.hpp"

#include <algorithm>
#include <cmath>
#include <map>

#include "cliente/arte.hpp"
#include "cliente/modelos3d.hpp"
#include "cliente/som.hpp"
#include "cliente/ui.hpp"
#include "rlgl.h"

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
        {"raio_perdicao", rgb(235, 40, 40)},
    };
    auto it = m.find(vis);
    return it == m.end() ? padrao : it->second;
}

// Tamanho da caixa do sprite de uma torre no mapa
float tamanho_torre(const Torre& t) {
    float tam = t.dfn->heroi ? 74.0f : static_cast<float>(t.dfn->raio) * 3.2f;
    if (t.temporaria) tam *= t.mae ? 0.6f : 0.8f;  // escolta do Comanche menor ainda
    return tam;
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
            if (++pops <= 40) efeitos_.push_back({"pop", e, 0, 0.3});
        } else if (tipo == "crit") {
            efeitos_.push_back({"crit", e, 0, 0.6});
        } else if (tipo == "explosao") {
            efeitos_.push_back({"explosao", e, 0, 0.45});
            if (sons_) som::tocar("explosao", 90);
        } else if (tipo == "raio") {
            // os raios da Dartling duram mais que o intervalo entre tiros (0,2 s): na tela o feixe nao pisca
            const bool continuo = e.s == "raio_plasma" || e.s == "raio_perdicao";
            efeitos_.push_back({"raio", e, 0, continuo ? 0.24 : 0.09});
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
            efeitos_.push_back({"nivel", e, 0, 0.8});
            if (sons_) som::tocar("upgrade");
        } else if (tipo == "habilidade") {
            Evento t = e;
            t.cor = {255, 255, 255};
            efeitos_.push_back({"texto", t, 0, 1.4});
            if (e.s == "Tsar Bomba") efeitos_.push_back({"aviao", e, 0, 1.3});
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
    animador_.observar(pista_, ui::tempo());
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
    for (auto& p : pista_.projeteis) {
        // rastro por upgrade: chamas no Kylie da Dominacao M.O.A.B. e verde na Carga Permanente
        if (p->torre && p->torre->chave == "bumerangue" && (p->torre->caminhos[2] >= 5 || p->torre->caminhos[1] >= 5)) {
            const bool fogo = p->torre->caminhos[2] >= 5;
            const float v = static_cast<float>(std::hypot(p->vx, p->vy)) + 0.001f;
            const float ux = static_cast<float>(p->vx) / v, uy = static_cast<float>(p->vy) / v;
            for (int i = 1; i <= 5; ++i) {
                const float px = static_cast<float>(p->x) - ux * i * 9, py = static_cast<float>(p->y) - uy * i * 9;
                const Color c = fogo ? (i < 3 ? rgb(255, 220, 90) : rgb(255, 110, 30)) : rgb(110, 255, 120);
                DrawCircleV({px, py}, (fogo ? 12.0f : 8.0f) - i * 1.4f, ui::com_alfa(c, 210 - i * 36));
            }
        }
        arte::projetil(*p);
    }
    // dinheiro caido no chao: balanca, e pisca nos ultimos 3 s antes de sumir
    for (const Coletavel& c : pista_.coletaveis) {
        const double t = ui::tempo();
        if (c.vida < 3.0 && std::fmod(t, 0.3) < 0.12) continue;
        const float x = static_cast<float>(c.x), y = static_cast<float>(c.y) + 3 * std::sin(static_cast<float>(t) * 4 + c.id);
        DrawEllipse(static_cast<int>(x), static_cast<int>(c.y) + 14, 14, 5, ui::com_alfa(ui::TINTA, 70));
        if (c.visual == "banana") {
            arte::projetil_vivo("banana", x, y, 40, 0, t);
        } else {
            // caixa de suprimentos (desenho provisorio): caixote de madeira com a moeda em cima
            const Rectangle r{x - 16, y - 10, 32, 24};
            ui::ret(ui::inflar(r, 3, 3), ui::TINTA, 6);
            ui::ret(r, rgb(190, 130, 70), 4);
            DrawRectangleRec({r.x, r.y + 9, r.width, 4}, rgb(120, 78, 40));
            arte::projetil_vivo("moeda", x, y - 14, 30, 0, t);
        }
    }
    desenhar_habilidades_em_uso();
    desenhar_efeitos();
}

void RenderPista::desenhar_torre(const Torre& t, bool sel) {
    if (t.chave == "as") {
        // pista de pouso no chao, onde o As foi colocado (desenho provisorio, com as pecas da interface)
        const float px = static_cast<float>(t.cx), py = static_cast<float>(t.cy);
        const Rectangle r{px - 30, py - 17, 60, 34};
        if (sel) ui::ret(ui::inflar(r, 5, 5), ui::AMARELO, 10);
        ui::ret(ui::inflar(r, 3, 3), ui::TINTA, 9);
        ui::ret(r, rgb(250, 200, 50), 6);
        ui::ret(ui::inflar(r, -4, -4), rgb(72, 74, 84), 4);
        for (int k = 0; k < 4; ++k) DrawRectangleRec({px - 22 + k * 13.0f, py - 2, 7, 4}, ui::BRANCO);
    }
    if (t.chave == "heli") {
        // heliponto no chao, onde o Heli foi colocado (desenho provisorio)
        const float px = static_cast<float>(t.cx), py = static_cast<float>(t.cy);
        const Rectangle r{px - 22, py - 22, 44, 44};
        if (sel) ui::ret(ui::inflar(r, 5, 5), ui::AMARELO, 12);
        ui::ret(ui::inflar(r, 3, 3), ui::TINTA, 11);
        ui::ret(r, rgb(250, 200, 50), 8);
        ui::ret(ui::inflar(r, -4, -4), rgb(44, 52, 110), 6);
        ui::texto("H", px, py - 1, 22, rgb(235, 70, 60), 3, ui::Ancora::CENTER);
    }
    const float tam = tamanho_torre(t);
    const float x = static_cast<float>(t.x), y = static_cast<float>(t.y);
    const anim::Quadro q = animador_.quadro(t.id, ui::tempo());
    if (q.habilidade && q.brilho > 0.01f) {
        // brilho da habilidade por baixo da torre: disco e raios girando
        const float r = tam * 0.62f * (0.8f + 0.3f * q.brilho);
        DrawCircleV({x, y}, r, ui::com_alfa(q.cor, static_cast<int>(90 * q.brilho)));
        const float giro = static_cast<float>(ui::tempo()) * 2.2f;
        for (int k = 0; k < 10; ++k) {
            const float a = giro + k * PI_F / 5, l = r * 1.35f;
            const Vector2 p0{x + std::cos(a - 0.09f) * r * 0.5f, y + std::sin(a - 0.09f) * r * 0.5f};
            const Vector2 p1{x + std::cos(a + 0.09f) * r * 0.5f, y + std::sin(a + 0.09f) * r * 0.5f};
            DrawTriangle(p0, {x + std::cos(a) * l, y + std::sin(a) * l}, p1, ui::com_alfa(ui::BRANCO, static_cast<int>(120 * q.brilho)));
        }
    }
    // anel no chao, em volta do pe do boneco 3/4 (selecao em amarelo, turbo em laranja)
    const float pe = y + arte::pe_torre_mapa(tam);
    auto anel_chao = [&](Color cor, float folga) {
        const float rx = tam * 0.32f + folga;
        rlPushMatrix();
        rlTranslatef(x, pe, 0);
        rlScalef(1, 0.4f, 1);
        DrawRing({0, 0}, rx - 6, rx + 2, 0, 360, 48, ui::com_alfa(ui::TINTA, 150));
        DrawRing({0, 0}, rx - 5, rx, 0, 360, 48, cor);
        rlPopMatrix();
    };
    if (sel) anel_chao(ui::AMARELO, 6);
    if (t.chave == "bumerangue" && t.caminhos[1] >= 5) anel_chao(rgb(110, 255, 120), sel ? 13.0f : 5.0f);  // Carga Permanente
    if (t.turbo < 1.0) anel_chao(rgb(255, 170, 40), sel ? 14.0f : 6.0f);
    // modelo 3D quando a pasta assets/modelos existe; senao, o sprite 2D
    auto desenhar = [&](const std::string& chave, const std::array<int, 3>& cam, float tm) {
        if (m3d::tem(chave) && t.dfn->mov == Mov::FIXO)  // sombra no chao (quem voa ja tem a base desenhada)
            DrawEllipse(static_cast<int>(x), static_cast<int>(y + tm * 0.22f), tm * 0.36f, tm * 0.17f, ui::com_alfa(ui::TINTA, 70));
        // o Templo do Sol e uma construcao: nao gira com a mira
        const anim::TipoMira mira = chave == "super" && cam[0] >= 4 ? anim::TipoMira::FIXA : anim::tipo_mira(chave);
        if (!m3d::torre(chave, cam, x, y + tm * 0.2f, tm * 0.95f, static_cast<float>(t.ang), mira))
            arte::torre_mapa(chave, chave == t.chave ? arte::visual(t) : arte::Visual{}, x, y, tm, &q);
    };
    if (t.disfarce == "super" || t.disfarce == "plasma") {
        // Fa-Clube: o Dardo vira Super Macaco; no Plasma, com um anel roxo (arte propria ainda nao existe)
        if (t.disfarce == "plasma") anel_chao(rgb(190, 90, 255), 10);
        desenhar("super", {0, 0, 0}, tam * 1.1f);
    } else if (t.disfarce == "monstro") {
        // Transformacao: desenho provisorio, a propria torre maior com um anel roxo
        anel_chao(rgb(150, 60, 220), 10);
        desenhar(t.chave, t.caminhos, tam * 1.3f);
    } else {
        desenhar(t.chave, t.caminhos, tam);
    }
    if (t.submerso) {
        // submerso: agua por cima do casco e o radar verde varrendo o alcance
        DrawEllipse(static_cast<int>(x), static_cast<int>(y + tam * 0.1f), tam * 0.46f, tam * 0.34f, rgb(40, 120, 210, 150));
        const float alc = static_cast<float>(t.alcance());
        for (int k = 0; k < 2; ++k) {
            const float f = static_cast<float>(std::fmod(ui::tempo() / 1.35 + k * 0.5, 1.0));
            DrawRing({x, y}, alc * f - 3, alc * f, 0, 360, 72, rgb(90, 255, 120, static_cast<int>(200 * (1 - f))));
        }
        DrawCircleV({x, y}, alc, rgb(90, 255, 120, 18));
    }
    if (t.dfn->heroi) ui::tecla_centro(std::to_string(t.nivel), x + tam * 0.3f, pe - 4);
}

void RenderPista::desenhar_habilidades_em_uso() {
    const double agora = ui::tempo();
    for (auto& [id, tp] : pista_.torres) {
        const anim::Quadro q = animador_.quadro(id, agora);
        if (!q.habilidade || q.onda_alfa <= 0.01f) continue;
        const float x = static_cast<float>(tp->x), y = static_cast<float>(tp->y);
        const float r = 20 + q.onda * 190;
        DrawCircleV({x, y}, r, ui::com_alfa(q.cor, static_cast<int>(40 * q.onda_alfa)));
        DrawRing({x, y}, r - 9, r + 2, 0, 360, 72, ui::com_alfa(ui::TINTA, static_cast<int>(150 * q.onda_alfa)));
        DrawRing({x, y}, r - 7, r, 0, 360, 72, ui::com_alfa(q.cor, static_cast<int>(230 * q.onda_alfa)));
        DrawRing({x, y}, r - 7, r - 5, 0, 360, 72, ui::com_alfa(ui::BRANCO, static_cast<int>(160 * q.onda_alfa)));
    }
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
            int dano = frac > 0.8 ? 0 : frac > 0.6 ? 1 : frac > 0.4 ? 2 : frac > 0.2 ? 3 : 4;
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
        if (b->cong_t > 0) arte::estado_bloon(0, x, y, r, agora);
        if (b->cola_t > 0) {
            arte::estado_bloon(1, x, y, r, agora);
            // cola corrosiva em verde e Super Cola (quase para o bloon) em rosa, por cima da cola amarela
            if (b->cola_f <= 0.15) DrawCircleV({x, y - r * 0.15f}, r * 0.8f, rgb(255, 105, 180, 150));
            else if (b->cola_dps > 0) DrawCircleV({x, y - r * 0.15f}, r * 0.8f, rgb(110, 220, 70, 150));
        }
        // Super Fragil: o bloon fica roxo enquanto recebe o dano extra
        if (b->frag >= 4) DrawCircleV({x, y - r * 0.15f}, r * 0.85f, rgb(170, 70, 235, 150));
        if (b->queima_t > 0) arte::estado_bloon(2, x, y, r, agora);
        if (b->atord_t > 0) arte::estado_bloon(3, x, y, r, agora);
    }
}

void RenderPista::desenhar_efeitos() {
    for (const Efeito& f : efeitos_) {
        const float k = static_cast<float>(f.t / f.dur);
        const Evento& d = f.dados;
        const float x = static_cast<float>(d.x), y = static_cast<float>(d.y);
        if (f.tipo == "pop") {
            // estouro do design: estrela branca que cresce e some, com confete (fase 0..1 do efeito)
            arte::efeito("estouro", x, y, 46, k * 0.7);
        } else if (f.tipo == "crit") {
            // acerto critico: texto laranja que sobe e some (como o CRIT do BTD6)
            const unsigned char a = static_cast<unsigned char>(255 * (k < 0.6f ? 1.0f : (1.0f - k) / 0.4f));
            ui::texto("CRIT", x, y - 26 - 22 * k, 22, {255, 150, 30, a}, 4, ui::Ancora::CENTER);
        } else if (f.tipo == "explosao") {
            const float raio = static_cast<float>(d.v);
            arte::efeito("explosao", x, y, std::max(48.0f, raio * 2.6f), k * 0.9);
            if (d.s == "esmaga") {
                // Esmaga Bloon: clarao branco e onda de choque por cima da explosao
                const int a = static_cast<int>(230 * (1 - k));
                DrawCircleV({x, y}, raio * (1.2f + 1.4f * k), rgb(255, 255, 255, a / 2));
                DrawRing({x, y}, raio * (0.6f + 2.2f * k) - 7, raio * (0.6f + 2.2f * k), 0, 360, 64, rgb(255, 250, 200, a));
            }
        } else if (f.tipo == "nivel") {
            arte::efeito("nivel", x, y - 20, 70, k * 0.8);
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
                if (vis == "raio_plasma" || vis == "raio_perdicao") {
                    // feixe com borda escura, cor e miolo claro; o Raio da Perdicao e o mais grosso, com brilho na base
                    const float w = vis == "raio_perdicao" ? 18.0f : 8.0f;
                    DrawLineEx({x, y}, {x2, y2}, w + 4, ui::com_alfa(ui::TINTA, 120));
                    DrawLineEx({x, y}, {x2, y2}, w, c);
                    DrawLineEx({x, y}, {x2, y2}, w * 0.4f, rgb(255, 245, 235));
                    if (vis == "raio_perdicao") {
                        DrawCircleV({x, y}, w * 1.1f, ui::com_alfa(c, 170));
                        DrawCircleV({x, y}, w * 0.6f, rgb(255, 245, 235));
                    }
                } else {
                    DrawLineEx({x, y}, {x2, y2}, 2.0f, c);
                }
            }
        } else if (f.tipo == "aura") {
            const float raio = static_cast<float>(d.v);
            const Color c = cor_efeito(d.s, WHITE);
            const int a = static_cast<int>(140 * (1 - k));
            const float rr = raio * (0.3f + 0.7f * k);
            if (d.s == "anel_fogo") {
                // Anel de Fogo e Anel Infernal: tres aneis de fogo abrindo ate a borda do alcance
                for (int i = 0; i < 3; ++i) {
                    const float ki = std::clamp(static_cast<float>(k) * 1.25f - i * 0.12f, 0.0f, 1.0f);
                    const float ri = raio * ki, esp = 5.0f + raio * 0.05f;
                    const Color ci = i == 0 ? rgb(255, 235, 120) : i == 1 ? rgb(255, 140, 30) : rgb(220, 50, 20);
                    DrawRing({x, y}, std::max(0.0f, ri - esp), ri, 0, 360, 64, ui::com_alfa(ci, static_cast<int>(210 * (1 - ki))));
                }
            }
            if (d.s == "congelar") DrawCircleV({x, y}, rr, ui::com_alfa(c, a / 2));
            if (a) DrawRing({x, y}, std::max(0.0f, rr - 4), rr, 0, 360, 64, ui::com_alfa(c, a + 60));
        } else if (f.tipo == "texto") {
            ui::texto(d.s, x, y - 30 * k - 20, 18, arte::cor(d.cor), 2, ui::Ancora::CENTER);
        } else if (f.tipo == "aviao") {
            // Tsar Bomba: o bombardeiro cruza o mapa e solta a bomba no meio do caminho
            const float ax = -90.0f + (LARGURA_MAPA + 180.0f) * static_cast<float>(k), ay = ALTURA_MAPA * 0.42f;
            DrawEllipse(static_cast<int>(ax - 14), static_cast<int>(ay + 70), 60, 14, rgb(22, 20, 26, 70));  // sombra
            DrawTriangle({ax - 6, ay}, {ax - 40, ay + 48}, {ax + 16, ay}, ui::TINTA);
            DrawTriangle({ax - 6, ay}, {ax + 16, ay}, {ax - 40, ay - 48}, ui::TINTA);
            DrawTriangle({ax - 4, ay}, {ax - 34, ay + 40}, {ax + 10, ay}, rgb(92, 104, 82));
            DrawTriangle({ax - 4, ay}, {ax + 10, ay}, {ax - 34, ay - 40}, rgb(92, 104, 82));
            DrawEllipse(static_cast<int>(ax), static_cast<int>(ay), 58, 13, ui::TINTA);
            DrawEllipse(static_cast<int>(ax), static_cast<int>(ay), 54, 10, rgb(120, 134, 104));
            DrawTriangle({ax - 44, ay}, {ax - 62, ay + 18}, {ax - 34, ay}, ui::TINTA);
            DrawTriangle({ax - 44, ay}, {ax - 34, ay}, {ax - 62, ay - 18}, ui::TINTA);
            DrawCircleV({ax + 34, ay}, 6, rgb(150, 210, 240));
            if (k > 0.45 && k < 0.8) {
                const float q = static_cast<float>((k - 0.45) / 0.35);
                const float bx = LARGURA_MAPA * 0.5f, by = ay + 10 + q * 60;
                DrawEllipse(static_cast<int>(bx), static_cast<int>(by), 9 * (1 - q * 0.4f), 18 * (1 - q * 0.4f), ui::TINTA);
                DrawEllipse(static_cast<int>(bx), static_cast<int>(by), 6 * (1 - q * 0.4f), 15 * (1 - q * 0.4f), rgb(60, 62, 70));
            }
        } else if (f.tipo == "anel") {
            const float r = 10 + 30 * k;
            const int a = static_cast<int>(255 * (1 - k));
            DrawRing({x, y}, r - 5, r + 1, 0, 360, 48, ui::com_alfa(ui::TINTA, a / 2));
            DrawRing({x, y}, r - 4, r, 0, 360, 48, ui::com_alfa(ui::BRANCO, a));
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
        const float tam = std::max(14.0f, std::round(tamanho_torre(*t) * esc * 1.3f));
        const anim::Quadro q = animador_.quadro(t->id, ui::tempo());
        arte::torre_mapa(t->chave, arte::visual(*t), r.x + std::floor(static_cast<float>(t->x) * esc),
                         r.y + std::floor(static_cast<float>(t->y) * esc), tam, &q);
    }
    for (auto& b : pista_.bloons) {
        if (!b->vivo) continue;
        float rr = std::max(2.0f, std::floor(static_cast<float>(b->tipo->raio) * esc * 1.2f));
        Vector2 c{r.x + std::floor(static_cast<float>(b->x) * esc), r.y + std::floor(static_cast<float>(b->y) * esc)};
        DrawCircleV(c, rr + 1, ui::TINTA);
        DrawCircleV(c, rr, arte::cor(b->tipo->cor));
    }
    const size_t n = std::min<size_t>(pista_.projeteis.size(), 200);
    for (size_t i = 0; i < n; ++i) arte::projetil(*pista_.projeteis[i], r.x, r.y, esc);
}

}  // namespace bl
