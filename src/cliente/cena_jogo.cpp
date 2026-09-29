#include "cliente/cena_jogo.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "cliente/arte.hpp"
#include "cliente/som.hpp"

namespace bl {

using ui::Ancora;
using ui::rgb;

namespace {

constexpr float PAINEL_X = LARGURA_MAPA;
constexpr float PAINEL_W = ui::LARGURA - LARGURA_MAPA;
constexpr float CARD_W = 74, CARD_H = 64;
constexpr float GRADE_Y = 66;
constexpr float ENVIO_H = 104;
const char* const NOMES_MODO[4] = {"Primeiro", "Último", "Perto", "Forte"};

std::string curto(std::string s, size_t n) {
    // corta sem quebrar um caractere UTF-8 no meio
    if (s.size() <= n) return s;
    size_t k = n;
    while (k > 0 && (static_cast<unsigned char>(s[k]) & 0xC0) == 0x80) --k;
    return s.substr(0, k) + ".";
}

std::string numero_g(double v) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%+g", v);
    return buf;
}

// Tecla (KEY_*) -> letra minuscula
char letra(int k) { return (k >= KEY_A && k <= KEY_Z) ? static_cast<char>('a' + (k - KEY_A)) : 0; }

}  // namespace

CenaJogo::CenaJogo(App& a, std::unique_ptr<Controlador> controle)
    : Cena(a), ctl_(std::move(controle)), chave_mapa_(ctl_->partida->mapa.def.chave),
      render_(ctl_->pista(), chave_mapa_), mostrar_log_(ctl_->online()) {
    if (ctl_->online()) render_op_ = std::make_unique<RenderPista>(ctl_->oponente(), chave_mapa_, false);
    heroi_ = ctl_->pista().heroi_escolhido;
    std::vector<std::string> chaves;
    if (!heroi_.empty()) chaves.push_back(heroi_);
    for (auto& t : torres()) chaves.push_back(t.chave);
    for (size_t i = 0; i < chaves.size(); ++i) {
        float col = static_cast<float>(i % 3), lin = static_cast<float>(i / 3);
        cards_.push_back({chaves[i], {PAINEL_X + 8 + col * (CARD_W + 2), GRADE_Y + lin * (CARD_H + 2), CARD_W, CARD_H}});
    }
    botao_play_ = {PAINEL_X + 70, ui::ALTURA - 116.0f, 100, 100};
    rect_mini_ = {LARGURA_MAPA - 270.0f, 8, 262, 181};
}

// ============================================================== utilidades
void CenaJogo::aviso(const std::string& txt, bool erro) {
    msg_ = {txt, 1.8};
    if (erro) som::tocar("erro", 150);
}

bool CenaJogo::comando(const std::string& cmd) {
    char erro = ctl_->enviar(cmd);
    if (erro) {
        aviso(mensagem_erro(erro));
        return false;
    }
    return true;
}

std::vector<CenaJogo::Hab> CenaJogo::hab_lista() {
    std::vector<Hab> out;
    for (auto& [id, t] : pista().torres)
        for (size_t i = 0; i < t->st.habs.size() && out.size() < 9; ++i) out.push_back({t, static_cast<int>(i), &t->st.habs[i]});
    return out;
}

// ============================================================== eventos
void CenaJogo::evento(const ui::Evento& e) {
    if (ctl_->terminou() || menu_pausa_) {
        if (menu_pausa_ && e.tipo == ui::Evento::TECLA && e.tecla == KEY_ESCAPE) {
            pausar(false);
            return;
        }
        for (auto& [b, acao] : botoes_menu_) {
            if (b.clicou(e)) {
                acao();
                return;
            }
        }
        return;
    }
    if (e.tipo == ui::Evento::TECLA) {
        tecla(e.tecla);
    } else if (e.tipo == ui::Evento::CLIQUE_DIR) {
        colocando_.clear();
        selecionada_ = 0;
    } else if (e.tipo == ui::Evento::CLIQUE) {
        clique(e.pos);
    }
}

void CenaJogo::pausar(bool v) {
    menu_pausa_ = v;
    if (!ctl_->online()) ctl_->pausado = v;
}

void CenaJogo::tecla(int k) {
    if (k == KEY_ESCAPE) {
        if (!colocando_.empty() || selecionada_ || op_grande_) {
            colocando_.clear();
            selecionada_ = 0;
            op_grande_ = false;
        } else {
            pausar(true);
        }
        return;
    }
    if (k == KEY_F1) {
        mostrar_log_ = !mostrar_log_;
        return;
    }
    if (k == KEY_O && ctl_->online()) {
        op_grande_ = !op_grande_;
        return;
    }
    if (k == KEY_SPACE) {
        ctl_->botao_play();
        return;
    }
    if ((k == KEY_COMMA || k == KEY_PERIOD || k == KEY_SLASH) && selecionada_) {
        upar(k == KEY_COMMA ? 0 : k == KEY_PERIOD ? 1 : 2);
        return;
    }
    if ((k == KEY_BACKSPACE || k == KEY_DELETE) && selecionada_) {
        vender();
        return;
    }
    if (k == KEY_TAB && selecionada_) {
        if (TorreP t = pista().torre(selecionada_))
            comando("M" + std::to_string(t->id) + ":" + std::to_string((t->modo + 1) % 4));
        return;
    }
    if (k >= KEY_ONE && k <= KEY_NINE) {
        auto habs = hab_lista();
        size_t i = static_cast<size_t>(k - KEY_ONE);
        if (i < habs.size()) comando("B" + std::to_string(habs[i].t->id) + ":" + std::to_string(habs[i].idx));
        return;
    }
    const char c = letra(k);
    if (!c) return;
    if (c == 'u' && !heroi_.empty()) {
        escolher(heroi_);
        return;
    }
    for (auto& t : torres()) {
        if (t.tecla.size() == 1 && t.tecla[0] == c) {
            escolher(t.chave);
            return;
        }
    }
}

void CenaJogo::escolher(const std::string& chave) {
    const DefTorre& dfn = definicao(chave);
    if (dfn.heroi && pista().tem_heroi) {
        aviso("Você já colocou seu herói.");
        return;
    }
    if (pista().dinheiro < pista().custo(dfn.custo)) {
        aviso("Dinheiro insuficiente!");
        return;
    }
    colocando_ = colocando_ == chave ? "" : chave;
    selecionada_ = 0;
}

void CenaJogo::clique(Vector2 pos) {
    for (auto* lista : {&botoes_up_, &botoes_hab_, &botoes_envio_}) {
        for (auto& alvo : *lista) {
            if (ui::dentro(alvo.r, pos)) {
                Acao acao = alvo.acao;  // a acao pode mexer nas listas
                acao();
                return;
            }
        }
    }
    if (op_grande_) {
        op_grande_ = false;
        return;
    }
    if (ctl_->online() && mostrar_op_ && ui::dentro(rect_mini_, pos) && colocando_.empty()) {
        op_grande_ = true;
        return;
    }
    if (pos.x >= PAINEL_X) {
        if (ui::dentro(botao_play_, pos) && !ctl_->online()) {
            ctl_->botao_play();
            som::tocar("rodada");
            return;
        }
        for (auto& [chave, r] : cards_) {
            if (ui::dentro(r, pos)) {
                escolher(chave);
                return;
            }
        }
        return;
    }
    const int x = static_cast<int>(pos.x), y = static_cast<int>(pos.y);
    if (!colocando_.empty()) {
        const std::string chave = colocando_;
        if (comando("T" + chave + "@" + std::to_string(x) + "," + std::to_string(y)))
            if (!ui::shift() || definicao(chave).heroi) colocando_.clear();
        return;
    }
    const Torre* melhor = nullptr;
    double md = 1e9;
    for (auto& [id, t] : pista().torres) {
        double d = (t->x - x) * (t->x - x) + (t->y - y) * (t->y - y);
        if (d < (t->dfn->raio + 10) * (t->dfn->raio + 10) && d < md) melhor = t.get(), md = d;
    }
    selecionada_ = melhor ? melhor->id : 0;
}

void CenaJogo::upar(int p) {
    TorreP t = pista().torre(selecionada_);
    if (!t || t->dfn->heroi || t->temporaria) return;
    if (!pista().custo_upgrade(*t, p)) {
        aviso("Caminho bloqueado ou no máximo.");
        return;
    }
    comando("U" + std::to_string(t->id) + ":" + std::to_string(p));
}

void CenaJogo::vender() {
    TorreP t = pista().torre(selecionada_);
    if (!t || t->temporaria) return;
    if (comando("V" + std::to_string(t->id))) selecionada_ = 0;
}

void CenaJogo::alternar_auto() { ctl_->partida->automatico = !ctl_->partida->automatico; }

void CenaJogo::reiniciar() {
    if (auto* solo = dynamic_cast<ControladorSolo*>(ctl_.get())) app.iniciar_jogo(solo->reiniciar());
}

void CenaJogo::desistir() {
    ctl_->desistir();
    pausar(false);
}

void CenaJogo::enviar(const Envio& env) {
    if (comando("S" + env.chave)) som::tocar("colocar", 60);
}

// ============================================================== atualizar
void CenaJogo::atualizar(double dt) {
    ctl_->atualizar(dt);
    render_.consumir_eventos();
    render_.atualizar(dt);
    for (const Evento& av : render_.avisos) {
        if (av.tipo == "fim_rodada") {
            const int r = static_cast<int>(av.v);
            if (ctl_->online()) {
                banner_ = {"Rodada " + std::to_string(r), 1.6};
                som::tocar("rodada");
            } else {
                banner_ = {"Rodada " + std::to_string(r) + " completa!", 2.0};
            }
        } else if (av.tipo == "eco") {
            eco_texto_ = {av.v, 1.2};
        }
    }
    render_.avisos.clear();
    if (render_op_) {
        render_op_->consumir_eventos();
        render_op_->atualizar(dt);
        for (const Evento& av : render_op_->avisos)
            if (av.tipo == "envio") avisos_op_.push_back({"Oponente enviou " + av.s + "!", 2.5});
        render_op_->avisos.clear();
    }
    for (auto& a : avisos_op_) a.second -= dt;
    avisos_op_.erase(std::remove_if(avisos_op_.begin(), avisos_op_.end(), [](auto& a) { return a.second <= 0; }),
                     avisos_op_.end());
    if (avisos_op_.size() > 4) avisos_op_.erase(avisos_op_.begin(), avisos_op_.end() - 4);
    msg_.second -= dt;
    banner_.second -= dt;
    eco_texto_.second -= dt;
    if (selecionada_ && !pista().torre(selecionada_)) selecionada_ = 0;
    if (!ctl_->status.empty()) {
        msg_ = {ctl_->status, 3.0};
        ctl_->status.clear();
    }
}

// ============================================================== desenho
void CenaJogo::desenhar() {
    const Vector2 mouse = ui::mouse();
    botoes_up_.clear();
    botoes_hab_.clear();
    botoes_envio_.clear();
    botoes_menu_.clear();
    dicas_.clear();
    TorreP sel = selecionada_ ? pista().torre(selecionada_) : nullptr;
    render_.desenhar(selecionada_);
    if (sel) circulo_alcance(static_cast<float>(sel->x), static_cast<float>(sel->y), sel->alcance(), true);
    if (!colocando_.empty() && mouse.x < PAINEL_X) previa(mouse);
    hud_topo();
    if (ctl_->online()) {
        painel_envios(mouse);
        if (mostrar_op_) mini_oponente();
    }
    habilidades(mouse);
    painel_lateral(mouse);
    if (sel) painel_upgrade(sel, mouse);
    if (mostrar_log_) painel_log();
    if (op_grande_ && render_op_) oponente_grande();
    mensagens();
    if (!dicas_.empty()) dica(dicas_.front());
    if (ctl_->terminou()) tela_fim();
    else if (menu_pausa_) tela_pausa();
}

void CenaJogo::circulo_alcance(float x, float y, double r, bool valido) {
    if (r >= 5000) return;
    const float rr = std::floor(static_cast<float>(r));
    DrawCircleV({x, y}, rr, valido ? rgb(0, 0, 0, 55) : rgb(255, 0, 0, 70));
    DrawRing({x, y}, rr - 2, rr, 0, 360, 72, valido ? rgb(255, 255, 255, 170) : rgb(255, 60, 60, 200));
}

void CenaJogo::previa(Vector2 mouse) {
    const DefTorre& dfn = definicao(colocando_);
    const bool valido = pista().posicao_valida(dfn, mouse.x, mouse.y) &&
                        pista().dinheiro >= pista().custo(dfn.custo, mouse.x, mouse.y);
    circulo_alcance(mouse.x, mouse.y, dfn.alcance < 5000 ? dfn.alcance : 60, valido);
    const int tam = dfn.heroi ? 58 : static_cast<int>(dfn.raio * 2.7);
    arte::torre(colocando_, mouse.x, mouse.y, tam, 0, 0, 210);
}

// ---------------------------------------------------------------- HUD
void CenaJogo::hud_topo() {
    Pista& p = pista();
    arte::icone_coracao(10, 8, 34);
    ui::texto(ui::formatar(std::max(0, p.vidas)), 48, 25, 26, ui::BRANCO, 2, Ancora::MIDLEFT);
    arte::icone_moeda(10, 48, 32);
    ui::texto("$" + ui::formatar(p.dinheiro), 48, 64, 26, ui::DINHEIRO, 2, Ancora::MIDLEFT);
    if (ctl_->online()) {
        arte::icone_eco(12, 88, 28);
        ui::texto("+" + ui::formatar(p.eco), 48, 102, 20, rgb(140, 255, 120), 2, Ancora::MIDLEFT);
        const float frac = static_cast<float>(1.0 - ctl_->partida->tempo_para_eco() / ECO_INTERVALO);
        ui::barra({130, 96, 90, 12}, frac, rgb(120, 230, 90));
        if (eco_texto_.second > 0)
            ui::texto("+$" + ui::formatar(eco_texto_.first), 230, 102, 18, rgb(140, 255, 120), 2, Ancora::MIDLEFT);
    }
}

void CenaJogo::painel_lateral(Vector2 mouse) {
    ui::painel_madeira({PAINEL_X, 0, PAINEL_W, ui::ALTURA}, 0);
    Partida& p = *ctl_->partida;
    const float cx = PAINEL_X + PAINEL_W / 2;
    if (ctl_->online()) {
        ui::texto("Rodada " + std::to_string(std::max(1, p.rodada)), cx, 20, 24, ui::BRANCO, 2, Ancora::CENTER);
        ui::texto("próxima em " + std::to_string(static_cast<int>(p.tempo_para_rodada())) + "s", cx, 46, 15,
                  ui::BRANCO, 2, Ancora::CENTER);
    } else {
        ui::texto("Rodada " + std::to_string(std::max(1, p.rodada)) + "/" + std::to_string(p.ultima_rodada), cx, 22, 24,
                  ui::BRANCO, 2, Ancora::CENTER);
        ui::texto(achar_dificuldade(p.dificuldade)->nome, cx, 48, 15, ui::AMARELO, 2, Ancora::CENTER);
    }
    Pista& pista_ = pista();
    for (auto& [chave, r] : cards_) {
        const DefTorre& dfn = definicao(chave);
        const int custo = pista_.custo(dfn.custo);
        const bool pode = pista_.dinheiro >= custo && !(dfn.heroi && pista_.tem_heroi);
        const bool sobre = ui::dentro(r, mouse);
        Color c = pode ? rgb(236, 214, 150) : rgb(170, 150, 120);
        if (dfn.heroi) c = pode ? rgb(255, 226, 120) : rgb(180, 160, 110);
        if (colocando_ == chave) c = rgb(140, 230, 110);
        if (sobre) c = ui::clarear(c, 0.3f);
        ui::ret(ui::mover(r, 0, 3), rgb(70, 42, 18), 9);
        ui::ret(r, c, 9);
        ui::ret_linha(r, rgb(90, 60, 30), 2, 9);
        arte::torre(chave, r.x + r.width / 2, r.y + 26, 50);
        if (dfn.heroi && pista_.tem_heroi)
            ui::texto("EM JOGO", r.x + r.width / 2, r.y + r.height - 10, 11, rgb(200, 255, 200), 2, Ancora::CENTER);
        else
            ui::texto("$" + ui::formatar(custo), r.x + r.width / 2, r.y + r.height - 10, 13,
                      pode ? ui::DINHEIRO : rgb(255, 110, 100), 2, Ancora::CENTER);
        std::string tecla = dfn.heroi ? "U" : std::string(1, static_cast<char>(std::toupper(dfn.tecla[0])));
        ui::texto(tecla, r.x + 5, r.y + 2, 10, ui::BRANCO, 1);
        if (sobre) dicas_.push_back({mouse, dfn.nome, dfn.heroi ? dfn.titulo : dfn.desc});
    }
    if (ctl_->online()) {
        const float y = ui::ALTURA - 112.0f;
        ui::texto("ECONOMIA", cx, y, 18, ui::BRANCO, 2, Ancora::CENTER);
        ui::texto("+$" + ui::formatar(pista_.eco) + " a cada " + std::to_string(static_cast<int>(ECO_INTERVALO)) + "s", cx,
                  y + 26, 15, rgb(150, 255, 130), 2, Ancora::CENTER);
        ui::texto("Oponente: " + std::to_string(std::max(0, ctl_->oponente().vidas)) + " vidas", cx, y + 52, 13,
                  ui::BRANCO, 2, Ancora::CENTER);
        ui::texto("O: ver oponente   F1: mensagens", cx, y + 76, 11, rgb(230, 230, 230), 1, Ancora::CENTER);
        ui::texto("Esc: menu", cx, y + 94, 11, rgb(230, 230, 230), 1, Ancora::CENTER);
        return;
    }
    const Rectangle r = botao_play_;
    const Vector2 c{r.x + r.width / 2, r.y + r.height / 2};
    const bool rapido = ctl_->velocidade > 1;
    const Color cor = (p.em_rodada && rapido) ? rgb(255, 170, 40) : rgb(80, 200, 60);
    DrawCircleV({c.x, c.y + 4}, 46, rgb(20, 60, 15));
    DrawCircleV(c, 46, rgb(30, 90, 20));
    DrawCircleV(c, 40, cor);
    DrawCircleV({c.x, c.y - 10}, 26, ui::clarear(cor, 0.3f));
    DrawCircleV({c.x, c.y + 4}, 30, cor);
    auto triangulo = [](Vector2 a, Vector2 b, Vector2 d) {
        DrawTriangle(a, b, d, WHITE);
        DrawLineEx(a, b, 3, ui::PRETO);
        DrawLineEx(b, d, 3, ui::PRETO);
        DrawLineEx(d, a, 3, ui::PRETO);
    };
    if (!p.em_rodada) {
        triangulo({c.x - 12, c.y - 20}, {c.x - 12, c.y + 20}, {c.x + 20, c.y});
    } else {
        for (float dx : {-16.0f, 4.0f}) triangulo({c.x + dx, c.y - 16}, {c.x + dx, c.y + 16}, {c.x + dx + 18, c.y});
    }
    ui::texto("Espaço", c.x, r.y + r.height + 6, 12, ui::BRANCO, 2, Ancora::CENTER);
    const Rectangle ra{PAINEL_X + 176, ui::ALTURA - 60.0f, 58, 24};
    ui::ret(ra, rgb(60, 40, 20), 6);
    ui::texto(std::string("AUTO: ") + (p.automatico ? "ON" : "OFF"), ra.x + ra.width / 2, ra.y + ra.height / 2, 10,
              p.automatico ? rgb(160, 255, 140) : rgb(230, 230, 230), 1, Ancora::CENTER);
    botoes_up_.push_back({ra, [this] { alternar_auto(); }});
    if (ui::dentro(r, mouse))
        dicas_.push_back({mouse, !p.em_rodada ? "Iniciar rodada" : "Acelerar",
                          "Começa a próxima rodada. Durante a rodada, alterna velocidade 1x/3x."});
}

void CenaJogo::dica(const Dica& d, float largura) {
    const auto linhas = d.desc.empty() ? std::vector<std::string>{}
                                       : ui::quebrar(d.desc, 14, largura - 20, ui::Peso::TEXTO);
    const float h = 34 + 18.0f * linhas.size();
    float x = std::min(d.pos.x + 16, ui::LARGURA - largura - 4);
    if (d.pos.x >= PAINEL_X) x = PAINEL_X - largura - 8;
    const float y = std::max(4.0f, std::min(d.pos.y + 10, ui::ALTURA - h - 4));
    const Rectangle r{x, y, largura, h};
    ui::ret(r, rgb(30, 20, 10), 8);
    ui::ret(ui::inflar(r, -4, -4), rgb(250, 236, 200), 7);
    ui::texto(d.titulo, r.x + 10, r.y + 6, 15, rgb(60, 30, 10), 0);
    for (size_t i = 0; i < linhas.size(); ++i)
        ui::texto(linhas[i], r.x + 10, r.y + 28 + i * 18.0f, 14, rgb(40, 30, 20), 0, Ancora::TOPLEFT, ui::Peso::TEXTO);
}

// ---------------------------------------------------------------- upgrades
void CenaJogo::painel_upgrade(const TorreP& tp, Vector2 mouse) {
    const Torre& t = *tp;
    const float w = 300;
    float h = t.dfn->heroi ? 360.0f : 500.0f;
    const float x = t.x > LARGURA_MAPA / 2.0 ? 8.0f : LARGURA_MAPA - w - 8;
    float y = 118;
    if (ctl_->online() && x > LARGURA_MAPA / 2.0f) y = 200;
    h = std::min(h, ui::ALTURA - y - (ctl_->online() ? ENVIO_H + 6 : 8));
    ui::painel({x, y, w, h}, rgb(86, 150, 50), rgb(60, 40, 20));
    const std::string& nome = t.dfn->nome;
    ui::texto(nome, x + w / 2, y + 20, nome.size() < 20 ? 18 : 15, ui::BRANCO, 2, Ancora::CENTER);
    const int tier = t.dfn->heroi ? 0 : *std::max_element(t.caminhos.begin(), t.caminhos.end());
    arte::torre(t.chave, x + 12 + 32, y + 34 + 32, 64, tier);
    ui::texto("Estouros: " + ui::formatar(t.pops), x + 84, y + 42, 14);
    const Rectangle rm{x + 84, y + 66, 196, 30};
    ui::ret(rm, rgb(40, 80, 30), 8);
    ui::texto(std::string("<  ") + NOMES_MODO[t.modo] + "  >", rm.x + rm.width / 2, rm.y + rm.height / 2, 15, ui::BRANCO,
              2, Ancora::CENTER);
    const int id = t.id;
    botoes_up_.push_back({rm, [this, id] {
                              if (TorreP tt = pista().torre(id))
                                  comando("M" + std::to_string(id) + ":" + std::to_string((tt->modo + 1) % 4));
                          }});
    if (ui::dentro(rm, mouse)) dicas_.push_back({mouse, "Prioridade de alvo", "Clique ou Tab para trocar."});
    const float yy = y + 106;
    if (t.dfn->heroi) {
        painel_heroi(t, x, yy, w);
    } else if (t.temporaria) {
        ui::texto("Temporária: " + std::to_string(static_cast<int>(t.temporaria)) + "s", x + w / 2, yy + 20, 16,
                  ui::BRANCO, 2, Ancora::CENTER);
    } else {
        for (int pth = 0; pth < 3; ++pth)
            if (yy + pth * 96 + 90 < y + h - 56) linha_upgrade(tp, pth, x + 10, yy + pth * 96, w - 20, mouse);
    }
    if (!t.temporaria) {
        const Rectangle rv{x + 20, y + h - 52, w - 40, 40};
        ui::Botao b{rv, "Vender  $" + ui::formatar(pista().valor_venda(t)), rgb(230, 120, 40), 18};
        b.desenhar();
        botoes_up_.push_back({rv, [this] { vender(); }});
    }
}

void CenaJogo::linha_upgrade(const TorreP& tp, int pth, float x, float y, float w, Vector2 mouse) {
    const Torre& t = *tp;
    Pista& p = pista();
    const int tier = t.caminhos[pth];
    for (int k = 0; k < 5; ++k) {
        const Rectangle r{x + k * 14.0f, y + 4, 11, 11};
        ui::ret(r, rgb(30, 60, 20), 3);
        if (k < tier) ui::ret(ui::inflar(r, -2, -2), rgb(140, 255, 90), 3);
    }
    if (tier > 0)
        ui::texto(curto(t.dfn->caminhos[pth][tier - 1].nome, 23), x + 76, y + 10, 12, rgb(230, 255, 220), 1,
                  Ancora::MIDLEFT);
    const Rectangle card{x, y + 20, w, 70};
    const Vector2 cc{card.x + card.width / 2, card.y + card.height / 2};
    if (tier >= 5) {
        ui::ret(card, rgb(200, 160, 40), 10);
        ui::texto("MÁXIMO", cc.x, cc.y, 20, ui::BRANCO, 2, Ancora::CENTER);
        return;
    }
    const Upgrade& up = t.dfn->caminhos[pth][tier];
    auto custo = p.custo_upgrade(t, pth);
    if (!custo) {
        ui::ret(card, rgb(70, 70, 70), 10);
        ui::texto("Caminho fechado", cc.x, card.y + 22, 14, rgb(220, 220, 220), 2, Ancora::CENTER);
        ui::texto(up.nome, cc.x, card.y + 46, 13, rgb(180, 180, 180), 1, Ancora::CENTER);
        return;
    }
    const bool pode = p.dinheiro >= *custo;
    const bool sobre = ui::dentro(card, mouse);
    Color c = pode ? rgb(60, 160, 230) : rgb(150, 70, 60);
    if (sobre) c = ui::clarear(c, 0.2f);
    ui::ret(ui::mover(card, 0, 3), rgb(20, 30, 50), 10);
    ui::ret(card, c, 10);
    auto linhas = ui::quebrar(up.nome, 15, w - 24);
    for (size_t i = 0; i < linhas.size() && i < 2; ++i)
        ui::texto(linhas[i], cc.x, card.y + 16 + i * 20.0f, 15, ui::BRANCO, 2, Ancora::CENTER);
    ui::texto("$" + ui::formatar(*custo), cc.x, card.y + card.height - 12, 16, pode ? ui::DINHEIRO : rgb(255, 140, 130),
              2, Ancora::CENTER);
    ui::texto(std::string(1, ",./"[pth]), card.x + card.width - 12, card.y + 6, 11, ui::BRANCO, 1);
    botoes_up_.push_back({card, [this, pth] { upar(pth); }});
    if (sobre) dicas_.push_back({mouse, up.nome, up.desc.empty() ? "Melhora a torre." : up.desc});
}

void CenaJogo::painel_heroi(const Torre& t, float x, float y, float w) {
    ui::texto("Nível " + std::to_string(t.nivel), x + w / 2, y + 12, 24, ui::AMARELO, 2, Ancora::CENTER);
    if (t.nivel < 20) {
        const double a = XP_NIVEL[t.nivel], b = XP_NIVEL[t.nivel + 1];
        ui::barra({x + 30, y + 40, w - 60, 16}, static_cast<float>((t.xp - a) / std::max(1.0, b - a)),
                  rgb(120, 200, 255));
        ui::texto("XP " + std::to_string(static_cast<int>(t.xp)) + "/" + std::to_string(static_cast<int>(b)), x + w / 2,
                  y + 48, 12, ui::BRANCO, 2, Ancora::CENTER);
    }
    const DefTorre& dfn = *t.dfn;
    const float yy = y + 76;
    ui::texto(dfn.titulo, x + w / 2, yy, 13, rgb(230, 255, 220), 1, Ancora::CENTER);
    const std::pair<int, const J*> habs[2] = {{3, &dfn.hab3}, {10, &dfn.hab10}};
    for (int i = 0; i < 2; ++i) {
        const J& h = *habs[i].second;
        if (h.is_null()) continue;
        const bool ok = t.nivel >= habs[i].first;
        ui::texto("Nv " + std::to_string(habs[i].first) + ": " + h["nome"].get<std::string>(), x + 20, yy + 26 + i * 26.0f,
                  14, ok ? ui::BRANCO : rgb(170, 190, 170), 1);
    }
    ui::texto("Sobe de nível estourando bloons.", x + w / 2, yy + 92, 11, rgb(230, 230, 230), 1, Ancora::CENTER);
}

// ---------------------------------------------------------------- habilidades
void CenaJogo::habilidades(Vector2 mouse) {
    auto habs = hab_lista();
    if (habs.empty()) return;
    const float base_y = ui::ALTURA - (ctl_->online() ? ENVIO_H + 40 : 40);
    for (size_t i = 0; i < habs.size(); ++i) {
        const Hab& hb = habs[i];
        const float cx = 34 + i * 60.0f, cy = base_y;
        const double rec = hb.t->hab_rec[hb.idx];
        const bool pronto = rec <= 0;
        DrawCircleV({cx, cy + 3}, 27, rgb(30, 30, 30));
        DrawCircleV({cx, cy}, 27, pronto ? rgb(250, 220, 90) : rgb(120, 120, 120));
        DrawCircleV({cx, cy}, 23, rgb(60, 120, 200));
        arte::torre(hb.t->chave, cx, cy, 40);
        const double recarga = (*hb.h)["recarga"].get<double>();
        if (!pronto) {
            const float frac = static_cast<float>(std::min(1.0, rec / recarga));
            DrawCircleSector({cx, cy}, 26, -90, -90 + 360 * frac, 30, rgb(0, 0, 0, 150));
            ui::texto(std::to_string(static_cast<int>(rec) + 1), cx, cy, 16, ui::BRANCO, 2, Ancora::CENTER);
        }
        ui::texto(std::to_string(i + 1), cx + 18, cy + 18, 12, ui::AMARELO, 2, Ancora::CENTER);
        const Rectangle r{cx - 27, cy - 27, 54, 54};
        const int id = hb.t->id, idx = hb.idx;
        botoes_hab_.push_back({r, [this, id, idx] { comando("B" + std::to_string(id) + ":" + std::to_string(idx)); }});
        if (ui::dentro(r, mouse))
            dicas_.push_back({mouse, (*hb.h)["nome"].get<std::string>(),
                              hb.t->dfn->nome + " (recarga " + std::to_string(static_cast<int>(recarga)) + "s)"});
    }
}

// ---------------------------------------------------------------- batalha
void CenaJogo::painel_envios(Vector2 mouse) {
    const Rectangle r{0, ui::ALTURA - ENVIO_H, LARGURA_MAPA, ENVIO_H};
    DrawRectangleRec(r, rgb(50, 30, 12, 215));
    DrawLineEx({r.x, r.y}, {r.x + r.width, r.y}, 4, rgb(30, 18, 6));
    Pista& p = pista();
    const int rodada = ctl_->partida->rodada;
    const float bw = 84, bh = 45;
    for (size_t i = 0; i < ENVIOS.size(); ++i) {
        const Envio& env = ENVIOS[i];
        const float col = static_cast<float>(i % 12), lin = static_cast<float>(i / 12);
        const Rectangle b{4 + col * (bw + 2), r.y + 6 + lin * (bh + 4), bw, bh};
        const bool bloqueado = rodada < env.rodada_min;
        const bool pode = !bloqueado && p.dinheiro >= env.custo;
        Color c = pode ? rgb(90, 170, 60) : !bloqueado ? rgb(110, 90, 70) : rgb(60, 50, 40);
        if (ui::dentro(b, mouse) && !bloqueado) c = ui::clarear(c, 0.2f);
        ui::ret(ui::mover(b, 0, 2), rgb(20, 12, 4), 7);
        ui::ret(b, c, 7);
        const TipoBloon& tb = tipo_bloon(env.tipo);
        if (tb.moab) {
            Vector2 tam = arte::tamanho_dirigivel(tb);
            arte::dirigivel(tb, env.fort, 0, b.x + 2 + 20, b.y + 4 + 17, 0, 40 / tam.x);
        } else {
            const float lado = std::floor(static_cast<float>(tb.raio)) * 3 + 8;
            arte::bloon(tb, env.camo, env.regen, env.fort, 0, b.x + 2 + 17, b.y + 4 + 17, 34 / lado);
        }
        if (env.qtd > 1) ui::texto("x" + std::to_string(env.qtd), b.x + 28, b.y + 30, 11, ui::BRANCO, 1);
        ui::texto("$" + ui::formatar(env.custo), b.x + b.width - 4, b.y + 12, 11, pode ? ui::DINHEIRO : rgb(255, 150, 140),
                  1, Ancora::MIDRIGHT);
        const std::string eco = env.eco ? numero_g(env.eco) : "0";
        ui::texto(eco, b.x + b.width - 4, b.y + 32, 11, env.eco > 0 ? rgb(150, 255, 130) : rgb(255, 150, 140), 1,
                  Ancora::MIDRIGHT);
        if (bloqueado) {
            DrawRectangleRec(b, rgb(0, 0, 0, 120));
            ui::texto("R" + std::to_string(env.rodada_min), b.x + b.width / 2, b.y + b.height / 2, 16,
                      rgb(220, 220, 220), 2, Ancora::CENTER);
        } else {
            botoes_envio_.push_back({b, [this, &env] { enviar(env); }});
        }
        if (ui::dentro(b, mouse))
            dicas_.push_back({mouse, env.nome,
                              "Custo $" + std::to_string(env.custo) + ". Renda " + eco + " por ciclo de eco. Libera na rodada " +
                                  std::to_string(env.rodada_min) + "."});
    }
}

void CenaJogo::mini_oponente() {
    const Rectangle r = rect_mini_;
    ui::ret(ui::mover(ui::inflar(r, 8, 30), 0, 11), rgb(40, 25, 10), 8);
    render_op_->desenhar_mini(r);
    ui::texto("OPONENTE   " + std::to_string(std::max(0, ctl_->oponente().vidas)) + " vidas", r.x + 6, r.y + r.height + 4, 14);
    float y = r.y + r.height + 30;
    for (auto& [txt, t] : avisos_op_) {
        ui::texto(txt, r.x + r.width, y, 14, rgb(255, 150, 130), 2, Ancora::TOPRIGHT);
        y += 20;
    }
}

void CenaJogo::oponente_grande() {
    DrawRectangle(0, 0, LARGURA_MAPA, ui::ALTURA, rgb(0, 0, 0, 160));
    const Rectangle r{60, 40, 920, 637};
    ui::ret(ui::inflar(r, 12, 12), rgb(40, 25, 10), 10);
    render_op_->desenhar_mini(r);
    ui::texto("Mapa do oponente (clique ou O para fechar)", r.x + r.width / 2, r.y - 22, 18, ui::BRANCO, 2, Ancora::CENTER);
}

void CenaJogo::painel_log() {
    if (!ctl_->online()) return;
    auto linhas = ctl_->log();
    if (linhas.size() > 12) linhas.erase(linhas.begin(), linhas.end() - 12);
    const Rectangle r{8, 130, 330, 26 + 17.0f * std::max<size_t>(1, linhas.size())};
    DrawRectangleRec(r, rgb(0, 0, 0, 170));
    ui::texto("Mensagens (F1)  tick " + std::to_string(ctl_->tick_rede()), r.x + 8, r.y + 4, 13, ui::AMARELO, 1);
    for (size_t i = 0; i < linhas.size(); ++i) {
        const Color c = linhas[i].rfind(">", 0) == 0 ? rgb(160, 230, 255) : rgb(200, 255, 170);
        ui::texto(linhas[i], r.x + 8, r.y + 24 + i * 17.0f, 13, c, 0, Ancora::TOPLEFT, ui::Peso::MONO);
    }
}

// ---------------------------------------------------------------- mensagens e telas
void CenaJogo::mensagens() {
    if (msg_.second > 0 && !msg_.first.empty())
        ui::texto(msg_.first, LARGURA_MAPA / 2.0f, 150, 24, rgb(255, 120, 100), 3, Ancora::CENTER);
    if (banner_.second > 0 && !banner_.first.empty())
        ui::texto(banner_.first, LARGURA_MAPA / 2.0f, 260, 40, ui::AMARELO, 4, Ancora::CENTER);
    if (ctl_->online() && ctl_->dessincronizado())
        ui::texto("DESSINCRONIZADO", LARGURA_MAPA / 2.0f, 110, 18, rgb(255, 80, 80), 2, Ancora::CENTER);
}

Rectangle CenaJogo::sobreposicao(const std::string& titulo, Color cor) {
    DrawRectangle(0, 0, ui::LARGURA, ui::ALTURA, rgb(0, 0, 0, 150));
    const Rectangle r{ui::LARGURA / 2.0f - 260, ui::ALTURA / 2.0f - 210, 520, 420};
    ui::painel(r, rgb(86, 150, 50), rgb(60, 40, 20), 20, 7);
    ui::texto(titulo, r.x + r.width / 2, r.y + 56, 48, cor, 4, Ancora::CENTER);
    return r;
}

void CenaJogo::tela_pausa() {
    const Rectangle r = sobreposicao("PAUSADO", ui::BRANCO);
    std::vector<std::tuple<std::string, Acao, Color>> opcoes;
    opcoes.emplace_back("Continuar", [this] { pausar(false); }, ui::VERDE);
    if (!ctl_->online()) {
        opcoes.emplace_back("Reiniciar", [this] { reiniciar(); }, rgb(230, 160, 40));
        opcoes.emplace_back(std::string("Rodada automática: ") + (ctl_->partida->automatico ? "ON" : "OFF"),
                            [this] { alternar_auto(); }, rgb(60, 150, 230));
    } else {
        opcoes.emplace_back("Desistir", [this] { desistir(); }, ui::VERMELHO);
    }
    opcoes.emplace_back("Menu principal", [this] { app.ir_menu(); }, rgb(150, 90, 50));
    for (size_t i = 0; i < opcoes.size(); ++i) {
        auto& [rot, acao, cor] = opcoes[i];
        ui::Botao b{{r.x + 90, r.y + 110 + i * 68.0f, r.width - 180, 56}, rot, cor, 20};
        b.desenhar();
        botoes_menu_.push_back({b, acao});
    }
}

void CenaJogo::tela_fim() {
    const int v = ctl_->vencedor();
    const bool ganhou = v == ctl_->meu;
    std::string titulo;
    if (!ctl_->online()) titulo = ganhou ? "VITÓRIA!" : "FIM DE JOGO";
    else titulo = ganhou ? "VITÓRIA!" : v == 0 ? "EMPATE" : "DERROTA";
    const Rectangle r = sobreposicao(titulo, ganhou ? ui::AMARELO : rgb(255, 110, 100));
    Pista& p = pista();
    const std::string linhas[3] = {"Rodada alcançada: " + std::to_string(ctl_->partida->rodada),
                                   "Bloons estourados: " + ui::formatar(p.pops_total),
                                   "Vidas restantes: " + std::to_string(std::max(0, p.vidas))};
    for (int i = 0; i < 3; ++i) ui::texto(linhas[i], r.x + r.width / 2, r.y + 118 + i * 32.0f, 20, ui::BRANCO, 2, Ancora::CENTER);
    std::vector<std::tuple<std::string, Acao, Color>> opcoes;
    if (!ctl_->online()) opcoes.emplace_back("Jogar novamente", [this] { reiniciar(); }, ui::VERDE);
    opcoes.emplace_back("Menu principal", [this] { app.ir_menu(); }, rgb(150, 90, 50));
    for (size_t i = 0; i < opcoes.size(); ++i) {
        auto& [rot, acao, cor] = opcoes[i];
        ui::Botao b{{r.x + 110, r.y + 240 + i * 66.0f, r.width - 220, 54}, rot, cor, 20};
        b.desenhar();
        botoes_menu_.push_back({b, acao});
    }
}

void CenaJogo::sair() { ctl_->fechar(); }

}  // namespace bl
