#include "cliente/vitrine.hpp"

#include <algorithm>
#include <cmath>

#include "cliente/arte.hpp"
#include "cliente/cena_jogo.hpp"
#include "cliente/sprites.hpp"

namespace bl {

using ui::Ancora;
using ui::rgb;

namespace {

const char* const PAGINAS[] = {"Torres", "Tiers (1/2)", "Tiers (2/2)", "Heróis", "Bloons", "Dirigíveis",
                               "Projéteis", "Efeitos", "Habilidades", "Animações", "Mapas"};
constexpr int N_PAGINAS = 11;

struct HabInfo {
    std::string dono, efeito, nome;
    bool heroi;
    int cam, tier, nivel;
};

std::vector<HabInfo> todas_habs() {
    std::vector<HabInfo> v;
    for (const DefTorre& t : torres())
        for (int c = 0; c < 3; ++c)
            for (int k = 0; k < 5; ++k) {
                const J& ef = t.caminhos[c][k].ef;
                if (ef.contains("hab"))
                    v.push_back({t.chave, ef["hab"].value("tipo", std::string("invocar")), ef["hab"].value("nome", std::string()),
                                 false, c, k + 1, 1});
            }
    for (const DefTorre& h : herois())
        for (auto [nivel, hab] : {std::pair<int, const J*>{3, &h.hab3}, std::pair<int, const J*>{10, &h.hab10}})
            if (!hab->is_null())
                v.push_back({h.chave, hab->value("tipo", std::string("invocar")), hab->value("nome", std::string()), true, -1, 0, nivel});
    return v;
}

// Torre que mostra cada clipe na pagina de animacoes
struct Exemplo {
    std::string chave;
    arte::Visual v;
};
Exemplo exemplo(const anim::Clipe& c) {
    const std::string& n = c.nome;
    auto V = [](int cam, int tier) { arte::Visual v; v.cam = cam; v.tier = tier; return v; };
    if (n == "disparo: arremesso") return {"dardo", {}};
    if (n == "disparo: arco") return {"quincy", {}};
    if (n == "disparo: tiro") return {"sniper", {}};
    if (n == "disparo: magia") return {"mago", {}};
    if (n == "disparo: canhao") return {"bomba", {}};
    if (n == "disparo: pulso") return {"tachinha", {}};
    if (n.find("turbo em area") != std::string::npos) return {"vila", V(1, 3)};
    if (n.find("turbo") != std::string::npos) return {"bumerangue", V(1, 3)};
    if (n.find("dano global") != std::string::npos) return {"super", V(1, 3)};
    if (n.find("dano forte") != std::string::npos) return {"bomba", V(1, 3)};
    if (n.find("congela") != std::string::npos) return {"gelo", V(1, 3)};
    if (n.find("lentidao") != std::string::npos) return {"ninja", V(1, 3)};
    if (n.find("dinheiro") != std::string::npos) return {"sniper", V(1, 3)};
    if (n.find("espinhos global") != std::string::npos) return {"espinhos", V(1, 3)};
    if (n.find("espinhos") != std::string::npos) return {"obyn", {}};
    if (n.find("invocar") != std::string::npos) return {"mago", V(1, 3)};
    if (n.find("reverso") != std::string::npos) return {"super", V(2, 5)};
    return {"jericho", {}};
}

class CenaVitrine : public Cena {
public:
    CenaVitrine(App& a, int pagina) : Cena(a), pagina_(std::clamp(pagina, 0, N_PAGINAS - 1)), habs_(todas_habs()) {}

    void evento(const ui::Evento& e) override {
        if (e.tipo == ui::Evento::TECLA) {
            if (e.tecla == KEY_ESCAPE) app.ir_menu();
            if (e.tecla == KEY_RIGHT || e.tecla == KEY_PAGE_DOWN) pagina_ = (pagina_ + 1) % N_PAGINAS;
            if (e.tecla == KEY_LEFT || e.tecla == KEY_PAGE_UP) pagina_ = (pagina_ + N_PAGINAS - 1) % N_PAGINAS;
            if (e.tecla == KEY_DOWN) sel_ = (sel_ + 1) % static_cast<int>(anim::todos_os_clipes().size());
            if (e.tecla == KEY_UP) sel_ = (sel_ + static_cast<int>(anim::todos_os_clipes().size()) - 1) % static_cast<int>(anim::todos_os_clipes().size());
        } else if (e.tipo == ui::Evento::CLIQUE) {
            for (size_t i = 0; i < alvos_.size(); ++i)
                if (ui::dentro(alvos_[i], e.pos)) sel_ = static_cast<int>(i);
        }
    }
    void atualizar(double dt) override { t_ += dt; }
    void desenhar() override;

private:
    void cabecalho();
    void torres_base();
    void tiers(int parte);
    void pag_herois();
    void pag_bloons();
    void pag_dirigiveis();
    void pag_projeteis();
    void pag_efeitos();
    void pag_habilidades();
    void pag_animacoes();
    void pag_mapas();
    void celula(Rectangle r, Color fundo);
    anim::Quadro quadro_loop(const anim::Clipe& c, double atraso, float pausa = 0.5f) const;

    int pagina_;
    int sel_ = 0;
    double t_ = 0;
    std::vector<HabInfo> habs_;
    std::vector<Rectangle> alvos_;
};

void CenaVitrine::celula(Rectangle r, Color fundo) {
    ui::ret(ui::mover(r, 0, 3), rgb(22, 20, 26, 90), 12);
    ui::ret(r, ui::TINTA, 12);
    ui::ret(ui::inflar(r, -5, -5), fundo, 9);
}

anim::Quadro CenaVitrine::quadro_loop(const anim::Clipe& c, double atraso, float pausa) const {
    const double per = c.dur + pausa;
    const double f = std::fmod(t_ + atraso, per);
    if (f > c.dur) return {};
    anim::Quadro q = anim::avaliar(c, static_cast<float>(f));
    return q;
}

void CenaVitrine::cabecalho() {
    DrawRectangle(0, 0, ui::LARGURA, 60, ui::MADEIRA_ESCURA);
    DrawRectangle(0, 60, ui::LARGURA, 4, ui::TINTA);
    ui::texto("Sprites", 20, 30, 24, ui::AMARELO, 5, Ancora::MIDLEFT, ui::Peso::LOGO);
    float x = 150;
    for (int i = 0; i < N_PAGINAS; ++i) {
        const float w = ui::medir(PAGINAS[i], 11).x + 18;
        const Rectangle r{x, 14, w, 32};
        ui::ret(r, ui::TINTA, 9);
        ui::ret(ui::inflar(r, -5, -5), i == pagina_ ? ui::AMARELO : ui::MADEIRA, 7);
        ui::texto(PAGINAS[i], r.x + w / 2, r.y + 15, 11, i == pagina_ ? ui::TINTA : ui::BRANCO, 0, Ancora::CENTER);
        x += w + 6;
    }
    ui::tecla("<", ui::LARGURA - 110.0f, 21, 9);
    ui::tecla(">", ui::LARGURA - 86.0f, 21, 9);
    ui::tecla("Esc", ui::LARGURA - 58.0f, 21, 9);
}

void CenaVitrine::desenhar() {
    ui::fundo_gradiente(ui::PERGAMINHO, rgb(233, 221, 190));
    alvos_.clear();
    switch (pagina_) {
        case 0: torres_base(); break;
        case 1: tiers(0); break;
        case 2: tiers(1); break;
        case 3: pag_herois(); break;
        case 4: pag_bloons(); break;
        case 5: pag_dirigiveis(); break;
        case 6: pag_projeteis(); break;
        case 7: pag_efeitos(); break;
        case 8: pag_habilidades(); break;
        case 9: pag_animacoes(); break;
        default: pag_mapas(); break;
    }
    cabecalho();
}

// As 22 torres: de cima (animada, atirando em loop) e o icone de frente.
void CenaVitrine::torres_base() {
    const auto& ts = torres();
    for (size_t i = 0; i < ts.size(); ++i) {
        const float x = 12 + (i % 11) * 115.0f, y = 76 + (i / 11) * 318.0f;
        celula({x, y, 108, 306}, rgb(109, 190, 69));
        const anim::Quadro q = quadro_loop(anim::clipe_disparo(ts[i].chave), i * 0.13, 0.9f);
        arte::torre_viva(ts[i].chave, {}, x + 54, y + 70, 104, t_, &q, true);
        arte::torre_icone(ts[i].chave, {}, x + 54, y + 190, 96);
        ui::texto(ts[i].nome, x + 54, y + 262, 10, ui::BRANCO, 3, Ancora::CENTER);
        ui::tecla_centro(std::string(1, static_cast<char>(std::toupper(ts[i].tecla[0]))), x + 54, y + 286, 9);
    }
}

// Base + tiers 3 e 5 de cada caminho (icone de frente, como no painel de upgrade).
void CenaVitrine::tiers(int parte) {
    const auto& ts = torres();
    const Color CAM[3] = {ui::AZUL, ui::VERDE, rgb(247, 148, 29)};
    for (int lin = 0; lin < 11; ++lin) {
        const DefTorre& t = ts[parte * 11 + lin];
        const float y = 72 + lin * 58.0f;
        ui::texto(t.nome, 16, y + 28, 12, ui::MADEIRA_ESCURA, 0, Ancora::MIDLEFT);
        for (int k = 0; k < 7; ++k) {
            const int cam = k == 0 ? -1 : (k - 1) / 2, tier = k == 0 ? 0 : (k % 2 ? 3 : 5);
            const float x = 220 + k * 150.0f;
            celula({x, y + 2, 140, 54}, k == 0 ? rgb(109, 190, 69) : tier == 5 ? ui::AMARELO : CAM[cam]);
            arte::Visual v;
            v.cam = cam, v.tier = tier;
            arte::torre_icone(t.chave, v, x + 30, y + 29, 52);
            const std::string rot = k == 0 ? "Base" : "C" + std::to_string(cam + 1) + " T" + std::to_string(tier);
            ui::texto(rot, x + 60, y + 18, 10, ui::BRANCO, 3, Ancora::MIDLEFT);
            if (k) ui::texto(t.caminhos[cam][tier - 1].nome, x + 60, y + 38, 9, ui::TINTA, 0, Ancora::MIDLEFT, ui::Peso::TEXTO);
        }
    }
}

void CenaVitrine::pag_herois() {
    const auto& hs = herois();
    for (size_t i = 0; i < hs.size(); ++i)
        for (int k = 0; k < 3; ++k) {
            const int nivel = k == 0 ? 1 : k == 1 ? 10 : 20;
            const float x = 12 + (i % 9) * 140.0f, y = 74 + ((i / 9) * 3 + k) * 106.0f;
            celula({x, y, 132, 100}, rgb(247, 215, 116));
            arte::Visual v;
            v.nivel = nivel;
            const anim::Quadro q = quadro_loop(anim::clipe_disparo(hs[i].chave), i * 0.21 + k * 0.4, 1.2f);
            arte::torre_viva(hs[i].chave, v, x + 44, y + 50, 84, t_ + i * 0.3, &q);
            ui::texto(hs[i].nome, x + 124, y + 20, 9, ui::MADEIRA_ESCURA, 0, Ancora::MIDRIGHT);
            ui::texto("Nv " + std::to_string(nivel), x + 124, y + 80, 11, ui::BRANCO, 3, Ancora::MIDRIGHT);
        }
}

void CenaVitrine::pag_bloons() {
    const char* tipos[] = {"vermelho", "azul", "verde", "amarelo", "rosa", "preto", "branco", "roxo", "chumbo", "zebra", "arco_iris", "ceramica"};
    const char* mods[] = {"normal", "camo", "regen", "fortificado", "todos"};
    for (int m = 0; m < 5; ++m) ui::texto(mods[m], 16, 132 + m * 124.0f, 11, ui::MADEIRA_ESCURA, 0, Ancora::MIDLEFT);
    for (int i = 0; i < 12; ++i)
        for (int m = 0; m < 5; ++m) {
            const float x = 110 + i * 97.0f, y = 76 + m * 124.0f;
            celula({x, y, 92, 118}, rgb(174, 224, 255));
            const bool camo = m == 1 || m == 4, regen = m == 2 || m == 4, fort = m == 3 || m == 4;
            arte::bloon_vivo(tipos[i], camo, regen, fort, x + 46, y + 60, 100, t_ + m * 0.3);
        }
}

void CenaVitrine::pag_dirigiveis() {
    const char* tipos[] = {"moab", "bfb", "zomg", "ddt", "bad"};
    for (int i = 0; i < 5; ++i)
        for (int k = 0; k < 6; ++k) {
            const float x = 12 + k * 210.0f, y = 74 + i * 128.0f;
            celula({x, y, 202, 122}, rgb(174, 224, 255));
            arte::dirigivel_vivo(tipos[i], k < 5 ? k : 0, k == 5, x + 101, y + 62, 186, t_);
            ui::texto(k == 5 ? "fortificado" : k == 0 ? "intacto" : "dano " + std::to_string(k), x + 12, y + 16, 9,
                      ui::TINTA, 0, Ancora::MIDLEFT);
        }
}

void CenaVitrine::pag_projeteis() {
    const char* ps[] = {"dardo", "flecha", "bola_espinho", "juggernaut", "bumerangue", "glaive", "kylie", "bomba", "missil", "tachinha",
                        "fogo", "anel_fogo", "lamina", "fragmento_gelo", "gelo_bola", "cola", "bala", "bala_canhao", "uva", "abacaxi",
                        "plasma", "laser", "raio_plasma", "magia", "chamas", "sol", "escuro", "shuriken", "estrepe", "pocao",
                        "acido", "espinho", "espinhos", "relampago", "tornado", "cipo", "banana", "moeda", "prego", "espuma",
                        "drone", "psi", "luz", "espirito", "maldicao", "meteoro", "radiacao", "aviaozinho", "vento", "espadas",
                        "flash", "armadilha", "zumbi", "balista", "impacto", "fragmento", "uva_fogo"};
    const int n = static_cast<int>(sizeof ps / sizeof ps[0]);
    for (int i = 0; i < n; ++i) {
        const float x = 12 + (i % 12) * 105.0f, y = 74 + (i / 12) * 128.0f;
        celula({x, y, 98, 122}, rgb(109, 190, 69));
        arte::projetil_vivo(ps[i], x + 49, y + 52, 110, 0, t_);
        ui::texto(ps[i], x + 49, y + 106, 9, ui::BRANCO, 3, Ancora::CENTER);
    }
}

void CenaVitrine::pag_efeitos() {
    const char* efs[] = {"estouro", "explosao", "dinheiro", "nivel", "impacto", "camo_revelado"};
    const float durs[] = {0.7f, 0.9f, 1.4f, 0.8f, 0.5f, 1.2f};
    for (int i = 0; i < 6; ++i) {
        const float x = 12 + (i % 5) * 250.0f, y = 76 + (i / 5) * 320.0f;
        celula({x, y, 240, 300}, rgb(109, 190, 69));
        if (i == 5) arte::bloon_vivo("verde", true, false, false, x + 120, y + 140, 180, t_);
        arte::efeito(efs[i], x + 120, y + 140, 220, std::fmod(t_, durs[i] + 0.3));
        ui::texto(efs[i], x + 120, y + 280, 12, ui::BRANCO, 3, Ancora::CENTER);
    }
    const char* est[] = {"congelado", "colado", "queimando", "atordoado"};
    const char* cores[] = {"vermelho", "azul", "amarelo", "rosa"};
    for (int k = 0; k < 4; ++k) {
        const float x = 12 + (k + 1) * 250.0f, y = 396;
        celula({x, y, 240, 300}, rgb(174, 224, 255));
        arte::bloon_vivo(cores[k], false, false, false, x + 120, y + 150, 170, 0);
        arte::estado_bloon(k, x + 120, y + 146, 38, t_);
        ui::texto(est[k], x + 120, y + 280, 12, ui::BRANCO, 3, Ancora::CENTER);
    }
}

void CenaVitrine::pag_habilidades() {
    for (size_t i = 0; i < habs_.size(); ++i) {
        const HabInfo& h = habs_[i];
        const float x = 12 + (i % 15) * 84.0f, y = 72 + (i / 15) * 128.0f;
        celula({x, y, 80, 122}, ui::MADEIRA_ESCURA);
        arte::habilidade(h.dono, h.heroi, h.cam, h.tier, h.nivel, h.efeito, x + 40, y + 44, 70);
        auto linhas = ui::quebrar(h.nome, 8, 74);
        for (size_t k = 0; k < linhas.size() && k < 2; ++k)
            ui::texto(linhas[k], x + 40, y + 92 + k * 13.0f, 8, ui::BRANCO, 2, Ancora::CENTER);
    }
}

// A ferramenta de animacao: cada clipe tocando em loop numa torre, e as curvas do clipe escolhido.
void CenaVitrine::pag_animacoes() {
    const auto& clipes = anim::todos_os_clipes();
    const int n = static_cast<int>(clipes.size());
    for (int i = 0; i < n; ++i) {
        const anim::Clipe& c = *clipes[i];
        const float x = 12 + (i % 6) * 122.0f, y = 74 + (i / 6) * 216.0f;
        const Rectangle r{x, y, 116, 208};
        alvos_.push_back(r);
        celula(r, i == sel_ ? rgb(200, 240, 168) : rgb(109, 190, 69));
        if (i == sel_) ui::ret_linha(ui::inflar(r, 4, 4), ui::AMARELO, 3, 14);
        const Exemplo ex = exemplo(c);
        const anim::Quadro q = quadro_loop(c, i * 0.17, 0.7f);
        const float cx = x + 58, cy = y + 86;
        if (!c.efeito.empty() && q.pose.ativa) {
            const Color cor = spr::cor_efeito(c.efeito);
            const float rr = 12 + q.onda * 60;
            DrawCircleV({cx, cy}, 50 * (0.8f + 0.3f * q.brilho), ui::com_alfa(cor, static_cast<int>(80 * q.brilho)));
            if (q.onda_alfa > 0.01f) {
                DrawRing({cx, cy}, rr - 6, rr, 0, 360, 48, ui::com_alfa(cor, static_cast<int>(230 * q.onda_alfa)));
                DrawRing({cx, cy}, rr - 7, rr + 1, 0, 360, 48, ui::com_alfa(ui::TINTA, static_cast<int>(90 * q.onda_alfa)));
            }
        }
        arte::torre_viva(ex.chave, ex.v, cx, cy, 104, t_, &q, true);
        auto linhas = ui::quebrar(c.nome, 9, 108);
        for (size_t k = 0; k < linhas.size() && k < 2; ++k)
            ui::texto(linhas[k], x + 58, y + 162 + k * 15.0f, 9, ui::BRANCO, 3, Ancora::CENTER);
        char buf[32];
        std::snprintf(buf, sizeof buf, "%.2f s", c.dur);
        ui::texto(buf, x + 58, y + 194, 9, ui::TINTA, 0, Ancora::CENTER, ui::Peso::MONO);
    }
    // curvas do clipe selecionado
    const anim::Clipe& c = *clipes[std::clamp(sel_, 0, n - 1)];
    const Rectangle painel{752, 74, 516, 634};
    celula(painel, ui::PERGAMINHO);
    ui::texto(c.nome, painel.x + 16, painel.y + 22, 15, ui::MADEIRA_ESCURA, 0, Ancora::MIDLEFT);
    char buf[64];
    std::snprintf(buf, sizeof buf, "duração %.2f s   (setas: trocar clipe)", c.dur);
    ui::texto(buf, painel.x + 16, painel.y + 44, 12, ui::TEXTO_ESCURO, 0, Ancora::MIDLEFT, ui::Peso::TEXTO);
    const float gx = painel.x + 110, gw = painel.width - 130;
    float y = painel.y + 70;
    const double per = c.dur + 0.7;
    const float cabeca = static_cast<float>(std::min(1.0, std::fmod(t_ + sel_ * 0.17, per) / c.dur));
    for (int k = 0; k < anim::N_CANAIS; ++k) {
        const anim::Trilha& tr = c.trilhas[k];
        if (tr.chaves.empty()) continue;
        float vmin = 1e9f, vmax = -1e9f;
        for (int s = 0; s <= 60; ++s) {
            const float v = tr.valor(c.dur * s / 60.0f, anim::padrao_canal(static_cast<anim::Canal>(k)));
            vmin = std::min(vmin, v), vmax = std::max(vmax, v);
        }
        if (vmax - vmin < 1e-3f) vmax = vmin + 1;
        const float h = 44;
        ui::texto(anim::nome_canal(static_cast<anim::Canal>(k)), painel.x + 16, y + h / 2, 11, ui::MADEIRA_ESCURA, 0, Ancora::MIDLEFT);
        ui::ret({gx - 4, y - 2, gw + 8, h + 4}, rgb(233, 221, 190), 6);
        auto ponto = [&](float t) {
            const float v = tr.valor(t, anim::padrao_canal(static_cast<anim::Canal>(k)));
            return Vector2{gx + gw * t / c.dur, y + h - h * (v - vmin) / (vmax - vmin)};
        };
        Vector2 ant = ponto(0);
        for (int s = 1; s <= 80; ++s) {
            const Vector2 p = ponto(c.dur * s / 80.0f);
            DrawLineEx(ant, p, 2.5f, ui::AZUL_ESCURO);
            ant = p;
        }
        for (const anim::Chave& ch : tr.chaves) {
            const Vector2 p = ponto(ch.t);
            DrawCircleV(p, 5, ui::TINTA);
            DrawCircleV(p, 3.5f, ui::AMARELO);
        }
        y += h + 12;
    }
    const float px = gx + gw * cabeca;
    DrawLineEx({px, painel.y + 64}, {px, y - 6}, 2, ui::VERMELHO);
    // legenda das curvas usadas
    std::vector<std::string> curvas;
    for (int k = 0; k < anim::N_CANAIS; ++k)
        for (const anim::Chave& ch : c.trilhas[k].chaves) {
            const std::string nome = anim::nome_curva(ch.curva);
            if (std::find(curvas.begin(), curvas.end(), nome) == curvas.end()) curvas.push_back(nome);
        }
    std::string leg = "curvas: ";
    for (size_t i = 0; i < curvas.size(); ++i) leg += (i ? ", " : "") + curvas[i];
    ui::texto(leg, painel.x + 16, painel.y + painel.height - 20, 11, ui::TEXTO_ESCURO, 0, Ancora::MIDLEFT, ui::Peso::TEXTO);
}

void CenaVitrine::pag_mapas() {
    for (size_t i = 0; i < mapas().size() && i < 4; ++i) {
        const float x = 16 + (i % 2) * 630.0f, y = 74 + (i / 2) * 320.0f;
        ui::ret({x - 4, y - 4, 612, 308}, ui::TINTA, 12);
        arte::fundo_mapa(mapas()[i].chave, {x, y, 604, 300});
        ui::texto(mapas()[i].nome, x + 12, y + 20, 14, ui::BRANCO, 4, Ancora::MIDLEFT);
    }
}

// ---------------------------------------------------------------- demo
class ControladorDemo : public Controlador {
public:
    explicit ControladorDemo(bool batalha) : batalha_(batalha) {
        partida = std::make_unique<Partida>(batalha ? "batalha" : "solo", "prado", 7, "medio",
                                            std::map<int, std::string>{{1, "quincy"}, {2, "obyn"}});
    }
    bool online() const override { return batalha_; }
    char enviar(const std::string& cmd) override { return partida->aplicar(1, cmd); }
    void botao_play() override {
        if (!partida->em_rodada) partida->aplicar(1, "N");
    }
    void atualizar(double dt) override {
        if (pausado || partida->fim) return;
        acc_ += std::min(dt, 0.1);
        while (acc_ >= DT) {
            partida->passo();
            acc_ -= DT;
        }
        // a demo usa as habilidades assim que ficam prontas (pelo mesmo comando do jogador)
        relogio_ += dt;
        if (relogio_ < 0.5) return;
        for (auto& [id, t] : pista().torres)
            for (size_t i = 0; i < t->hab_rec.size(); ++i)
                if (t->hab_rec[i] <= 0) partida->aplicar(1, "B" + std::to_string(id) + ":" + std::to_string(i));
    }
    bool terminou() const override { return partida->fim; }
    int vencedor() const override { return partida->vencedor; }
    std::vector<std::string> log() override {
        return {"> S cc           envio", "< ok  S cc       -$700", "< T 18232  j2  U 41:1", "> U 12:1         +Tiro",
                "< T 18234  j2  S z3"};
    }
    long tick_rede() const override { return 18234; }

private:
    bool batalha_;
    double acc_ = 0, relogio_ = 0;
};

}  // namespace

std::unique_ptr<Cena> criar_vitrine(App& app, int pagina) { return std::make_unique<CenaVitrine>(app, pagina); }

std::unique_ptr<Cena> criar_demo(App& app, bool batalha) {
    auto ctl = std::make_unique<ControladorDemo>(batalha);
    Partida& pt = *ctl->partida;
    // dinheiro de sobra so nesta partida de demonstracao (nao existe no jogo normal)
    for (auto& [j, p] : pt.pistas) p->dinheiro = 200000;
    struct Colocar {
        const char* chave;
        int x, y;
        std::vector<int> ups;
    };
    const std::vector<Colocar> lista = {
        {"dardo", 290, 225, {0, 0, 0, 1, 1}}, {"bumerangue", 100, 430, {1, 1, 1}}, {"tachinha", 330, 508, {0, 0, 0}},
        {"ninja", 520, 330, {0, 0, 2, 2}},    {"bomba", 490, 520, {1, 1, 1, 1}},   {"sniper", 60, 210, {1, 1, 1, 1}},
        {"quincy", 540, 250, {}},             {"mago", 740, 110, {1, 1, 1, 1}},    {"super", 745, 520, {0, 0, 0}},
        {"dartling", 960, 300, {}},           {"gelo", 330, 380, {1, 1, 1, 1}},    {"engenheiro", 150, 520, {0}},
    };
    for (int j : {1, 2}) {
        Pista& p = pt.pista(j);
        for (auto& c : lista) {
            if (j == 2 && std::string(c.chave) == "quincy") continue;
            if (pt.aplicar(j, "T" + std::string(c.chave) + "@" + std::to_string(c.x) + "," + std::to_string(c.y))) continue;
            int id = 0;
            for (auto& [tid, t] : p.torres) id = std::max(id, tid);
            for (int u : c.ups) pt.aplicar(j, "U" + std::to_string(id) + ":" + std::to_string(u));
        }
        if (j == 1 && !batalha) break;
    }
    if (!batalha) pt.aplicar(1, "N");
    // avanca a simulacao para ja ter bloons na tela
    const int passos = 30 * 40;
    for (int i = 0; i < passos; ++i) {
        if (batalha && i % 90 == 0) {
            pt.aplicar(2, "Sg5");
            pt.aplicar(1, "Sr8");
        }
        pt.passo();
    }
    int alvo = 0;
    for (auto& [id, t] : pt.pista(1).torres)
        if (t->chave == "dardo") alvo = id;
    auto cena = std::make_unique<CenaJogo>(app, std::move(ctl));
    cena->selecionar(alvo);
    return cena;
}

}  // namespace bl
