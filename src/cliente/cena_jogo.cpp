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
constexpr float CARD_W = 72, CARD_H = 60, PASSO_X = 76, PASSO_Y = 64;
constexpr float GRADE_X = PAINEL_X + 8, GRADE_Y = 72;
constexpr float ENVIO_H = 104;
constexpr float UP_W = 384, UP_H = 320;
const char* const NOMES_MODO[5] = {"Primeiro", "Último", "Perto", "Forte", "Elite"};
// quantos modos de alvo a torre tem: o Atirador de Elite ganha o quinto
int modos_de_alvo(const Torre& t) { return t.chave == "sniper" && t.caminhos[1] >= 5 ? 5 : 4; }
const Color MADEIRA_BLOQ = rgb(58, 42, 30);

std::string numero_g(double v) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%g", std::fabs(v));
    std::string s = buf;
    for (char& c : s)
        if (c == '.') c = ',';
    if (v == 0) return "±0";
    return (v > 0 ? "+" : "-") + s;
}

// Tecla (KEY_*) -> letra minuscula
char letra(int k) { return (k >= KEY_A && k <= KEY_Z) ? static_cast<char>('a' + (k - KEY_A)) : 0; }

// Cor da faixa de categoria no topo do card de torre
Color cor_categoria(const DefTorre& d) {
    if (d.heroi) return ui::AMARELO;
    if (d.categoria == "militar") return rgb(143, 160, 74);
    if (d.categoria == "magica") return rgb(166, 95, 232);
    if (d.categoria == "suporte") return rgb(242, 169, 58);
    return ui::AZUL;  // primaria
}

// Retangulo arredondado com borda TINTA, labio e brilho (cards e botoes do painel).
void bloco(Rectangle r, Color face, Color lab, float raio, float sombra = 3, bool brilho = true) {
    if (sombra > 0) ui::ret(ui::mover(r, 0, sombra), rgb(22, 20, 26, 128), raio);
    ui::ret(r, ui::TINTA, raio);
    const Rectangle f = ui::inflar(r, -6, -6);
    ui::ret(f, lab, raio - 3);
    ui::ret({f.x, f.y, f.width, f.height - 5}, face, raio - 3);
    if (brilho) ui::ret({f.x + 3, f.y + 1, f.width - 6, 3}, rgb(255, 255, 255, 77), 2);
}

void seta(Vector2 c, float lado, bool direita, Color cor) {
    const float d = direita ? 1.0f : -1.0f;
    DrawTriangle({c.x + d * lado * 0.6f, c.y}, {c.x - d * lado * 0.4f, c.y - lado * 0.6f}, {c.x - d * lado * 0.4f, c.y + lado * 0.6f}, cor);
    DrawTriangle({c.x + d * lado * 0.6f, c.y}, {c.x - d * lado * 0.4f, c.y + lado * 0.6f}, {c.x - d * lado * 0.4f, c.y - lado * 0.6f}, cor);
}

// Texto cortado com "..." para caber na largura
std::string caber(const std::string& s, int tam, float largura, ui::Peso peso = ui::Peso::TITULO) {
    if (ui::medir(s, tam, peso).x <= largura) return s;
    std::string r = s;
    while (!r.empty()) {
        r.pop_back();
        while (!r.empty() && (static_cast<unsigned char>(r.back()) & 0xC0) == 0x80) r.pop_back();
        if (!r.empty() && (static_cast<unsigned char>(r.back()) & 0xC0) == 0xC0) r.pop_back();
        if (ui::medir(r + "...", tam, peso).x <= largura) return r + "...";
    }
    return r;
}

}  // namespace

CenaJogo::CenaJogo(App& a, std::unique_ptr<Controlador> controle)
    : Cena(a), ctl_(std::move(controle)), chave_mapa_(ctl_->partida->mapa.def.chave),
      render_(ctl_->pista(), chave_mapa_), mostrar_log_(false) {
    // o log de rede (F1) comeca fechado: ele tapava a trilha na Batalha
    if (ctl_->online()) render_op_ = std::make_unique<RenderPista>(ctl_->oponente(), chave_mapa_, false);
    heroi_ = ctl_->pista().heroi_escolhido;
    std::vector<std::string> chaves;
    if (!heroi_.empty()) chaves.push_back(heroi_);
    for (auto& t : torres()) chaves.push_back(t.chave);
    for (size_t i = 0; i < chaves.size(); ++i) {
        float col = static_cast<float>(i % 3), lin = static_cast<float>(i / 3);
        cards_.push_back({chaves[i], {GRADE_X + col * PASSO_X - 4, GRADE_Y + lin * PASSO_Y, CARD_W, CARD_H}});
    }
    botao_play_ = {PAINEL_X + 12, ui::ALTURA - 116.0f, 88, 88};
    rect_mini_ = {LARGURA_MAPA - 271.0f, 53, 256, 175};
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
    if (confirma_up_ >= 0 && !ctl_->terminou() && !menu_pausa_) {
        // dialogo do sacrificio aberto: so ele recebe o clique; Esc cancela e Enter confirma
        if (e.tipo == ui::Evento::TECLA && e.tecla == KEY_ESCAPE) confirma_up_ = -1;
        else if (e.tipo == ui::Evento::TECLA && e.tecla == KEY_ENTER) upar(confirma_up_);
        else
            for (auto& [b, acao] : botoes_menu_)
                if (b.clicou(e)) {
                    Acao a = acao;
                    a();
                    return;
                }
        return;
    }
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
            definindo_alvo_ = 0;
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
    if (k == KEY_F2 && ctl_->partida->sandbox) {
        painel_bloons_ = !painel_bloons_;
        return;
    }
    if (k == KEY_F9) {
        // depuracao das animacoes: repete o clipe na torre selecionada (so visual)
        depura_ = depura_ ? 0 : (ui::shift() ? 2 : 1);
        depura_t_ = 0;
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
        if (TorreP t = pista().torre(selecionada_); t && t->chave == "dartling")
            alternar_mira(t->id);
        else if (t && t->chave == "morteiro")
            definindo_alvo_ = definindo_alvo_ == t->id ? 0 : t->id;
        else if (t && t->chave == "as")
            proxima_rota(t->id);
        else if (t && t->chave == "heli")
            proximo_modo_heli(t->id);
        else if (t)
            comando("M" + std::to_string(t->id) + ":" + std::to_string((t->modo + 1) % modos_de_alvo(*t)));
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
    if (ctl_->online() && mostrar_op_ && ui::dentro(ui::inflar(rect_mini_, 6, 50), pos) && colocando_.empty()) {
        op_grande_ = true;
        return;
    }
    if (pos.x >= PAINEL_X) {
        if (ui::dentro(botao_play_, pos) && !ctl_->online()) {
            ctl_->botao_play();
            som::tocar("rodada");
            return;
        }
        if (ctl_->partida->sandbox && painel_bloons_) return;  // a loja esta escondida
        for (auto& [chave, r] : cards_) {
            if (ui::dentro(r, pos)) {
                escolher(chave);
                return;
            }
        }
        return;
    }
    const int x = static_cast<int>(pos.x), y = static_cast<int>(pos.y);
    if (definindo_alvo_) {
        const bool ok = comando("A" + std::to_string(definindo_alvo_) + "@" + std::to_string(x) + "," + std::to_string(y));
        // no Heli o clique marca a outra ponta da patrulha
        if (TorreP t = pista().torre(definindo_alvo_); ok && t && t->chave == "heli") {
            comando("O" + std::to_string(t->id) + ":2");
            modo_heli_[t->id] = 3;
        }
        definindo_alvo_ = 0;
        return;
    }
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
        // o As e o Heli tambem se selecionam pela base no chao, que fica parada
        if (t->dfn->mov != Mov::FIXO && !t->temporaria)
            d = std::min(d, (t->cx - x) * (t->cx - x) + (t->cy - y) * (t->cy - y));
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
    if (const std::string req = pista().requisito_upgrade(*t, p); !req.empty()) {
        aviso(req + ".");
        return;
    }
    // upgrade que destroi torres vizinhas: pede confirmacao antes de mandar o comando
    if (!pista().aviso_sacrificio(*t, p).empty() && confirma_up_ != p && pista().dinheiro >= *pista().custo_upgrade(*t, p)) {
        confirma_up_ = p;
        return;
    }
    confirma_up_ = -1;
    comando("U" + std::to_string(t->id) + ":" + std::to_string(p));
}

void CenaJogo::vender() {
    TorreP t = pista().torre(selecionada_);
    if (!t || t->temporaria) return;
    if (comando("V" + std::to_string(t->id))) selecionada_ = 0;
}

void CenaJogo::alternar_mira(int id) {
    if (!mira_travada_.erase(id)) mira_travada_.insert(id);
    mira_enviada_ = {-1, -1};  // ao destravar, manda o cursor de novo mesmo parado
}

// As: circulo, infinito, oito e, com o upgrade, a rota em volta de um ponto escolhido no mapa
void CenaJogo::proxima_rota(int id) {
    TorreP t = pista().torre(id);
    if (!t) return;
    if (definindo_alvo_ == id) {  // desistiu de escolher o centro: volta ao circulo
        definindo_alvo_ = 0;
        comando("O" + std::to_string(id) + ":0");
        return;
    }
    const int n = t->caminhos[2] >= 2 ? 4 : 3;
    const int prox = (t->rota + 1) % n;
    if (prox == 3) definindo_alvo_ = id;  // o clique no mapa manda o comando A, que ja liga a rota 3
    else comando("O" + std::to_string(id) + ":" + std::to_string(prox));
}

// Modo de voo mostrado no painel. A simulacao so sabe "procura bloons", "vai ao ponto" e "patrulha";
// seguir o mouse e travado sao o mesmo "vai ao ponto", com ou sem o cliente mandando o cursor.
int CenaJogo::modo_heli(const Torre& t) const {
    if (t.rota == 0) return 0;
    if (t.rota == 2) return 3;
    const auto it = modo_heli_.find(t.id);
    return it != modo_heli_.end() && it->second == 1 ? 1 : 2;
}

void CenaJogo::proximo_modo_heli(int id) {
    if (definindo_alvo_ == id) {  // desistiu da patrulha: volta ao automatico
        definindo_alvo_ = 0;
        modo_heli_[id] = 0;
        comando("O" + std::to_string(id) + ":0");
        return;
    }
    TorreP t = pista().torre(id);
    if (!t) return;
    const int prox = (modo_heli(*t) + 1) % 4;
    if (prox == 3) {
        definindo_alvo_ = id;  // o modo so muda quando o ponto for escolhido
        return;
    }
    modo_heli_[id] = prox;
    // 1 segue o cursor (os comandos A saem do atualizar); 2 trava onde esta, sem mandar mais nada
    if (prox != 2) comando("O" + std::to_string(id) + ":" + std::to_string(prox));
    mira_enviada_ = {-1, -1};
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
    if (comando("S" + env.chave)) {
        som::tocar("colocar", 60);
        avisos_op_.push_back({false, (env.qtd > 1 ? std::to_string(env.qtd) + " " : "") + env.nome, 2.0});
    }
}

// ============================================================== atualizar
void CenaJogo::atualizar(double dt) {
    // cursor para as Dartlings em modo Normal; a simulacao so conhece o ponto que chega por comando
    mira_espera_ -= dt;
    const Vector2 cur = ui::mouse();
    if (mira_espera_ <= 0 && !menu_pausa_ && !ctl_->terminou() && cur.x >= 0 && cur.x < PAINEL_X && cur.y >= 0 &&
        cur.y <= ALTURA_MAPA && (std::fabs(cur.x - mira_enviada_.x) > 6 || std::fabs(cur.y - mira_enviada_.y) > 6)) {
        bool enviou = false;
        for (auto& [id, t] : pista().torres) {
            const bool dartling = t->chave == "dartling" && !mira_travada_.count(id);
            const auto mh = modo_heli_.find(id);
            const bool heli = t->chave == "heli" && mh != modo_heli_.end() && mh->second == 1;
            if (!dartling && !heli) continue;
            ctl_->enviar("A" + std::to_string(id) + "@" + std::to_string(static_cast<int>(cur.x)) + "," +
                         std::to_string(static_cast<int>(cur.y)));
            enviou = true;
        }
        if (enviou) mira_espera_ = 0.1, mira_enviada_ = cur;
    }
    // passar o cursor por cima coleta bananas e caixas (Coleta Facil, caminho 3 da Fazenda, alarga o raio)
    if (!pista().coletaveis.empty() && cur.x < PAINEL_X) {
        double raio = 34;
        for (auto& [id, t] : pista().torres)
            if (t->chave == "fazenda" && t->caminhos[2] >= 1) raio = 85;
        for (const Coletavel& c : pista().coletaveis)
            if (std::hypot(c.x - cur.x, c.y - cur.y) <= raio && coleta_pedida_.insert(c.id).second)
                ctl_->enviar("C" + std::to_string(c.id));
    } else if (pista().coletaveis.empty()) {
        coleta_pedida_.clear();
    }
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
            if (av.tipo == "envio") avisos_op_.push_back({true, av.s, 2.5});
        render_op_->avisos.clear();
    }
    for (auto& a : avisos_op_) a.t -= dt;
    avisos_op_.erase(std::remove_if(avisos_op_.begin(), avisos_op_.end(), [](auto& a) { return a.t <= 0; }), avisos_op_.end());
    if (avisos_op_.size() > 4) avisos_op_.erase(avisos_op_.begin(), avisos_op_.end() - 4);
    msg_.second -= dt;
    banner_.second -= dt;
    eco_texto_.second -= dt;
    if (selecionada_ && !pista().torre(selecionada_)) selecionada_ = 0;
    if (!ctl_->status.empty()) {
        msg_ = {ctl_->status, 3.0};
        ctl_->status.clear();
    }
    if (depura_ && selecionada_) {
        depura_t_ -= dt;
        if (depura_t_ <= 0) {
            TorreP t = pista().torre(selecionada_);
            std::string efeito = "invocar";
            if (!t->st.habs.empty()) efeito = t->st.habs[0].value("tipo", efeito);
            if (depura_ == 1) {
                render_.animador().tocar_disparo(t->id, t->chave, ui::tempo());
                depura_t_ = anim::clipe_disparo(t->chave).dur + 0.35;
            } else {
                render_.animador().tocar_habilidade(t->id, efeito, ui::tempo());
                depura_t_ = anim::clipe_habilidade(efeito).dur + 0.5;
            }
        }
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
    if (sel && (sel->chave == "morteiro" ? sel->tem_mira
                : sel->chave == "heli"   ? sel->rota == 2
                                         : sel->chave == "as" && sel->rota == 3) ||
        (sel && definindo_alvo_ == sel->id)) {
        // marca do ponto de impacto: no ponto fixo, ou seguindo o cursor enquanto o jogador escolhe
        const bool esc = definindo_alvo_ == sel->id && mouse.x < PAINEL_X;
        const Vector2 c = esc ? mouse : Vector2{static_cast<float>(sel->mx), static_cast<float>(sel->my)};
        const Color cor = esc ? ui::AMARELO : ui::VERMELHO;
        DrawRing(c, 20, 24, 0, 360, 48, ui::TINTA);
        DrawRing(c, 21, 23, 0, 360, 48, cor);
        DrawLineEx({c.x - 30, c.y}, {c.x - 12, c.y}, 4, cor);
        DrawLineEx({c.x + 12, c.y}, {c.x + 30, c.y}, 4, cor);
        DrawLineEx({c.x, c.y - 30}, {c.x, c.y - 12}, 4, cor);
        DrawLineEx({c.x, c.y + 12}, {c.x, c.y + 30}, 4, cor);
    }
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
    if (depura_)
        ui::texto(depura_ == 1 ? "F9: disparo em loop" : "Shift+F9: habilidade em loop", LARGURA_MAPA / 2.0f, 700, 13,
                  ui::AMARELO, 2, Ancora::CENTER);
    if (ctl_->terminou()) tela_fim();
    else if (menu_pausa_) tela_pausa();
    else if (confirma_up_ >= 0) tela_sacrificio();
}

void CenaJogo::circulo_alcance(float x, float y, double r, bool valido) {
    if (r >= 5000) return;
    const float rr = std::floor(static_cast<float>(r));
    DrawCircleV({x, y}, rr, valido ? rgb(255, 255, 255, 46) : rgb(226, 59, 46, 70));
    DrawRing({x, y}, rr - 1.5f, rr + 1.5f, 0, 360, 72, rgb(22, 20, 26, 128));
    DrawRing({x, y}, rr - 4, rr - 1, 0, 360, 72, valido ? rgb(255, 255, 255, 217) : ui::VERMELHO);
}

void CenaJogo::previa(Vector2 mouse) {
    const DefTorre& dfn = definicao(colocando_);
    const bool pos_ok = pista().posicao_valida(dfn, mouse.x, mouse.y);
    const int custo = pista().custo(dfn.custo, mouse.x, mouse.y);
    const bool din_ok = pista().dinheiro >= custo;
    const float raio = static_cast<float>(dfn.alcance < 5000 ? dfn.alcance : 60);
    // motivo visivel: posicao invalida = vermelho com X; sem dinheiro = ambar com o preco
    if (pos_ok && din_ok) {
        circulo_alcance(mouse.x, mouse.y, raio, true);
    } else {
        const Color c = !pos_ok ? ui::VERMELHO : ui::AMARELO;
        DrawCircleV(mouse, raio, ui::com_alfa(c, 64));
        DrawRing(mouse, raio - 1.5f, raio + 1.5f, 0, 360, 72, rgb(22, 20, 26, 128));
        DrawRing(mouse, raio - 4, raio - 1, 0, 360, 72, c);
    }
    const float tam = dfn.heroi ? 74.0f : static_cast<float>(dfn.raio) * 3.2f;
    arte::torre_mapa(colocando_, {}, mouse.x, mouse.y, tam, nullptr, 215);
    if (!pos_ok) {
        for (float s : {1.0f, -1.0f}) {
            DrawLineEx({mouse.x - 12, mouse.y - 12 * s}, {mouse.x + 12, mouse.y + 12 * s}, 8, ui::TINTA);
            DrawLineEx({mouse.x - 12, mouse.y - 12 * s}, {mouse.x + 12, mouse.y + 12 * s}, 4, ui::VERMELHO);
        }
    } else if (!din_ok) {
        ui::pilula_preco(custo, mouse.x, mouse.y - tam * 0.5f - 10, false, 12);
    }
}

// ---------------------------------------------------------------- HUD
// Sandbox: envia qualquer bloon ou rodada e limpa a pista (comandos X da Partida)
void CenaJogo::painel_sandbox(Vector2 mouse) {
    const float x0 = PAINEL_X + 8, w = 224;
    auto botao = [&](Rectangle r, const std::string& txt, Color face, Color lab, int tam, Acao acao, bool ligado = false) {
        const bool sobre = ui::dentro(r, mouse);
        bloco(ui::mover(r, 0, sobre ? -1.0f : 0), ligado ? ui::VERDE : sobre ? ui::clarear(face, 0.15f) : face,
              ligado ? ui::VERDE_ESCURO : lab, 9, 2);
        ui::texto(txt, r.x + r.width / 2, r.y + r.height / 2 - 2, tam, ui::BRANCO, 2, Ancora::CENTER);
        botoes_up_.push_back({r, std::move(acao)});
        return sobre;
    };
    // quantidade
    static const int QTDS[] = {1, 5, 10, 20, 50, 100};
    auto passo_qtd = [this](int d) {
        int i = 0;
        while (i < 5 && QTDS[i] < sb_qtd_) ++i;
        sb_qtd_ = QTDS[std::clamp(i + d, 0, 5)];
    };
    float y = GRADE_Y;
    ui::texto("QUANTIDADE", x0 + 4, y + 15, 10, ui::BEGE, 0, Ancora::MIDLEFT);
    botao({x0 + 96, y, 34, 30}, "-", ui::AZUL, ui::AZUL_ESCURO, 16, [passo_qtd] { passo_qtd(-1); });
    ui::texto(std::to_string(sb_qtd_), x0 + 160, y + 14, 17, ui::BRANCO, 3, Ancora::CENTER);
    botao({x0 + w - 34, y, 34, 30}, "+", ui::AZUL, ui::AZUL_ESCURO, 16, [passo_qtd] { passo_qtd(1); });
    // propriedades
    y += 36;
    const char* nomes[3] = {"Camo", "Regen", "Fort"};
    bool* flags[3] = {&sb_camo_, &sb_regen_, &sb_fort_};
    for (int i = 0; i < 3; ++i) {
        bool* f = flags[i];
        botao({x0 + i * 76.0f, y, 72, 30}, nomes[i], ui::CINZA, ui::CINZA_ESCURO, 11, [f] { *f = !*f; }, *f);
    }
    // grade com todos os bloons e dirigiveis
    y += 38;
    const auto& tipos = bloons();
    for (size_t i = 0; i < tipos.size(); ++i) {
        const TipoBloon& tb = tipos[i];
        const Rectangle r{x0 + (i % 4) * 56.5f, y + (i / 4) * 52.0f, 54, 49};
        const bool sobre = ui::dentro(r, mouse);
        ui::ret(ui::mover(r, 0, sobre ? 4.0f : 2.0f), rgb(22, 20, 26, 128), 9);
        ui::ret(r, ui::TINTA, 9);
        ui::ret(ui::inflar(r, -4, -4), sobre ? rgb(255, 246, 218) : ui::BEGE, 6);
        const bool fort = sb_fort_ && tb.vida_fortificado > 0;
        if (tb.moab) {
            arte::dirigivel(tb, fort, 0, r.x + r.width / 2, r.y + r.height / 2, 0, 42 / arte::tamanho_dirigivel(tb).x);
        } else {
            arte::bloon(tb, sb_camo_, sb_regen_, fort, 0, r.x + r.width / 2, r.y + r.height / 2,
                        15.0f / arte::raio_bloon_px(tb));
        }
        const std::string nome = tb.nome;
        botoes_up_.push_back({r, [this, nome] {
                                  comando("Xb:" + nome + ":" + std::to_string(sb_qtd_) + ":" +
                                          (sb_camo_ ? "c" : "") + (sb_regen_ ? "r" : "") + (sb_fort_ ? "f" : ""));
                              }});
        if (sobre) dicas_.push_back({mouse, tb.rotulo, "Envia " + std::to_string(sb_qtd_) + " pela entrada."});
    }
    // rodada inteira
    y += static_cast<float>((tipos.size() + 3) / 4) * 52.0f + 6;
    ui::texto("RODADA", x0 + 4, y + 15, 10, ui::BEGE, 0, Ancora::MIDLEFT);
    botao({x0 + 58, y, 30, 30}, "-", ui::AZUL, ui::AZUL_ESCURO, 16,
          [this] { sb_rodada_ = std::max(1, sb_rodada_ - (ui::shift() ? 10 : 1)); });
    ui::texto(std::to_string(sb_rodada_), x0 + 108, y + 14, 16, ui::BRANCO, 3, Ancora::CENTER);
    botao({x0 + 128, y, 30, 30}, "+", ui::AZUL, ui::AZUL_ESCURO, 16,
          [this] { sb_rodada_ = std::min(200, sb_rodada_ + (ui::shift() ? 10 : 1)); });
    if (botao({x0 + 164, y, 60, 30}, "Enviar", ui::VERDE, ui::VERDE_ESCURO, 11,
              [this] { comando("Xr:" + std::to_string(sb_rodada_)); }))
        dicas_.push_back({mouse, "Enviar rodada", "Manda a rodada inteira. Shift nos botões pula de 10 em 10."});
    // limpeza e recarga (a ultima celula da direita fica para o botao Torres/Bloons)
    y += 36;
    botao({x0, y, 108, 28}, "Limpar bloons", ui::VERMELHO, ui::VERMELHO_ESCURO, 10, [this] { comando("Xl"); });
    botao({x0 + 116, y, 108, 28}, "Limpar torres", ui::VERMELHO, ui::VERMELHO_ESCURO, 10, [this] {
        if (comando("Xt")) selecionada_ = 0;
    });
    y += 32;
    botao({x0, y, 140, 28}, "Recarregar habilidades", ui::AZUL, ui::AZUL_ESCURO, 9, [this] { comando("Xh"); });
}

void CenaJogo::hud_topo() {
    Pista& p = pista();
    const std::string vidas = ui::formatar(std::max(0, p.vidas)), din = "$" + ui::formatar(p.dinheiro);
    const bool on = ctl_->online();
    const std::string eco = "+" + ui::formatar(p.eco);
    float w = 10 + 30 + 6 + ui::medir(vidas, 23).x + 18 + 30 + 6 + ui::medir(din, 23).x + 18;
    if (on) w += 28 + 6 + ui::medir(eco, 20).x + 8 + 62;
    // placa: os numeros deixam de sumir sobre o mapa
    ui::placa({12, 12, w, 52}, ui::MADEIRA_ESCURA, 16);
    float x = 22;
    arte::icone("coracao", x, 23, 30);
    x += 36;
    x += ui::texto(vidas, x, 38, 23, ui::BRANCO, 3, Ancora::MIDLEFT).width + 12;
    arte::icone("moeda", x, 23, 30);
    x += 36;
    x += ui::texto(din, x, 38, 23, ui::DINHEIRO, 3, Ancora::MIDLEFT).width + 12;
    if (on) {
        arte::icone("eco", x, 24, 28);
        x += 34;
        x += ui::texto(eco, x, 38, 20, ui::ECO, 3, Ancora::MIDLEFT).width + 8;
        const float frac = static_cast<float>(1.0 - ctl_->partida->tempo_para_eco() / ECO_INTERVALO);
        ui::barra({x + 2, 32, 54, 10}, frac, ui::ECO);
        if (eco_texto_.second > 0)
            ui::texto("+$" + ui::formatar(eco_texto_.first), 18, 76, 18, ui::ECO, 3, Ancora::MIDLEFT);
    }
}

void CenaJogo::painel_lateral(Vector2 mouse) {
    ui::painel_madeira({PAINEL_X, 0, PAINEL_W, ui::ALTURA}, 0);
    Partida& p = *ctl_->partida;
    // placa de rodada no topo do painel
    const Rectangle pr{PAINEL_X + 8, 8, 224, 56};
    ui::placa(pr, ui::MADEIRA_ESCURA, 12);
    arte::icone("rodada", pr.x + 12, pr.y + 12, 32);
    ui::texto("RODADA", pr.x + 50, pr.y + 13, 9, ui::BEGE, 0);
    const std::string rod = ctl_->online() ? std::to_string(std::max(1, p.rodada))
                   : (ctl_->partida->em_freeplay || p.sandbox) ? std::to_string(std::max(1, p.rodada))
                                                : std::to_string(std::max(1, p.rodada)) + "/" + std::to_string(p.ultima_rodada);
    ui::texto(rod, pr.x + 48, pr.y + 36, 23, ui::BRANCO, 4, Ancora::MIDLEFT);
    const float dx = pr.x + pr.width - 12;
    if (ctl_->online()) {
        const double falta = p.tempo_para_rodada();
        ui::texto("próxima em " + std::to_string(static_cast<int>(std::ceil(falta))) + " s", dx, pr.y + 18, 12,
                  ui::BEGE, 0, Ancora::MIDRIGHT, ui::Peso::TEXTO);
        ui::barra({dx - 72, pr.y + 33, 72, 8}, static_cast<float>(1 - std::min(1.0, falta / PAUSA_ENTRE_RODADAS)), ui::AMARELO);
    } else {
        ui::texto("dificuldade", dx, pr.y + 18, 12, ui::BEGE, 0, Ancora::MIDRIGHT, ui::Peso::TEXTO);
        const std::string nome = achar_dificuldade(p.dificuldade)->nome;
        const float wn = ui::medir(nome, 10).x + 16;
        const Rectangle rp{dx - wn, pr.y + 29, wn, 20};
        ui::ret(rp, ui::TINTA, 10);
        ui::ret(ui::inflar(rp, -4, -4), ui::AZUL, 8);
        ui::texto(nome, rp.x + rp.width / 2, rp.y + 9, 10, ui::BRANCO, 0, Ancora::CENTER);
    }
    Pista& pista_ = pista();
    if (p.sandbox) {
        // ultima celula da grade (vazia na loja): alterna entre a loja e o painel de bloons
        const Rectangle rt{GRADE_X + 2 * PASSO_X - 4, GRADE_Y + 7 * PASSO_Y, CARD_W, CARD_H};
        const bool sobre = ui::dentro(rt, mouse);
        bloco(ui::mover(rt, 0, sobre ? -2.0f : 0), sobre ? ui::clarear(ui::AZUL, 0.15f) : ui::AZUL, ui::AZUL_ESCURO, 10,
              sobre ? 5.0f : 3.0f);
        ui::tecla_centro("F2", rt.x + rt.width / 2, rt.y + 18 - (sobre ? 2 : 0), 8);
        ui::texto(painel_bloons_ ? "Torres" : "Bloons", rt.x + rt.width / 2, rt.y + 40 - (sobre ? 2 : 0), 11, ui::BRANCO, 2,
                  Ancora::CENTER);
        botoes_up_.push_back({rt, [this] { painel_bloons_ = !painel_bloons_; }});
        if (sobre) dicas_.push_back({mouse, "Sandbox", "Alterna entre a loja de torres e o painel de envio de bloons."});
    }
    if (p.sandbox && painel_bloons_) painel_sandbox(mouse);
    else for (auto& [chave, r0] : cards_) {
        const DefTorre& dfn = definicao(chave);
        const int custo = pista_.custo(dfn.custo);
        const bool em_jogo = dfn.heroi && pista_.tem_heroi;
        const bool pode = pista_.dinheiro >= custo && !em_jogo;
        const bool sobre = ui::dentro(r0, mouse);
        const bool colocando = colocando_ == chave;
        const Rectangle r = sobre && !colocando ? ui::mover(r0, 0, -2) : r0;
        Color face = pode ? ui::BEGE : ui::BEGE_APAGADO;
        if (em_jogo) face = rgb(247, 215, 116);
        if (sobre && pode) face = rgb(255, 246, 218);
        if (colocando) face = rgb(200, 240, 168);
        ui::ret(ui::mover(r, 0, sobre ? 5.0f : 3.0f), rgb(22, 20, 26, 128), 10);
        ui::ret(ui::inflar(r, colocando ? 2.0f : 0.0f, colocando ? 2.0f : 0.0f), colocando ? ui::VERDE_ESCURO : ui::TINTA, 10);
        const Rectangle f = ui::inflar(r, -5, -5);
        ui::ret(f, face, 7);
        DrawRectangleRec({f.x + 2, f.y, f.width - 4, 3}, cor_categoria(dfn));  // faixa de categoria
        const unsigned char alfa = em_jogo ? 140 : pode ? 255 : 115;
        arte::torre_icone(chave, {}, r.x + r.width / 2, r.y + 23, 40, alfa);
        // rodape do preco
        const Rectangle rod_r{f.x, r.y + r.height - 20, f.width, 17.5f};
        const Color fundo = em_jogo ? ui::VERDE_ESCURO : pode ? ui::MADEIRA_ESCURA : ui::VERMELHO_ESCURO;
        DrawRectangleRec({f.x, rod_r.y - 2, f.width, 2}, ui::TINTA);
        ui::ret(rod_r, fundo, 5);
        DrawRectangleRec({rod_r.x, rod_r.y, rod_r.width, 6}, fundo);
        ui::texto(em_jogo ? "EM JOGO" : "$" + ui::formatar(custo), r.x + r.width / 2, rod_r.y + 8, em_jogo ? 9 : 11,
                  em_jogo ? rgb(233, 255, 217) : pode ? ui::DINHEIRO : ui::PRECO_RUIM, 0, Ancora::CENTER);
        const std::string tecla = dfn.heroi ? "U" : std::string(1, static_cast<char>(std::toupper(dfn.tecla[0])));
        ui::tecla(tecla, r.x + 4, r.y + 5, 9);
        if (sobre) dicas_.push_back({mouse, dfn.nome, dfn.heroi ? dfn.titulo : dfn.desc});
    }
    if (ctl_->online()) {
        const float y = ui::ALTURA - 124.0f;
        const Rectangle re{PAINEL_X + 8, y, 224, 52};
        ui::placa(re, ui::ECO_ESCURO, 12);
        arte::icone("eco", re.x + 10, re.y + 10, 32);
        ui::texto("+$" + ui::formatar(pista_.eco), re.x + 50, re.y + 18, 18, ui::ECO, 4, Ancora::MIDLEFT);
        ui::texto("de renda a cada " + std::to_string(static_cast<int>(ECO_INTERVALO)) + " s", re.x + 50, re.y + 36, 12,
                  rgb(233, 255, 217), 0, Ancora::MIDLEFT, ui::Peso::TEXTO);
        const char* ks[3] = {"O", "F1", "Esc"};
        const char* ts[3] = {"Oponente", "Rede", "Menu"};
        for (int i = 0; i < 3; ++i) {
            const Rectangle b{PAINEL_X + 8 + i * 76.0f, y + 60, 72, 48};
            const bool sobre = ui::dentro(b, mouse);
            bloco(ui::mover(b, 0, sobre ? -2.0f : 0), sobre ? ui::clarear(ui::AZUL, 0.15f) : ui::AZUL, ui::AZUL_ESCURO, 10, sobre ? 5.0f : 3.0f);
            ui::tecla_centro(ks[i], b.x + b.width / 2, b.y + 15 - (sobre ? 2 : 0), 8);
            ui::texto(ts[i], b.x + b.width / 2, b.y + 34 - (sobre ? 2 : 0), 10, ui::BRANCO, 2, Ancora::CENTER);
            const int k = i;
            botoes_up_.push_back({b, [this, k] {
                                      if (k == 0) op_grande_ = !op_grande_;
                                      else if (k == 1) mostrar_log_ = !mostrar_log_;
                                      else pausar(true);
                                  }});
        }
        return;
    }
    // solo: botao de rodada, atalho e AUTO
    const Rectangle r = botao_play_;
    const Vector2 c{r.x + r.width / 2, r.y + r.height / 2};
    const bool rapido = ctl_->velocidade > 1;
    const bool sobre_play = ui::dentro(r, mouse);
    const Color cor = (p.em_rodada && rapido) ? rgb(255, 170, 40) : ui::VERDE;
    DrawCircleV({c.x, c.y + 5}, 44, rgb(22, 20, 26, 140));
    DrawCircleV(c, 44, ui::TINTA);
    DrawCircleV(c, 41, ui::labio(cor));
    DrawCircleV({c.x, c.y - 3}, 38, sobre_play ? ui::clarear(cor, 0.15f) : cor);
    DrawCircleSector({c.x, c.y - 3}, 36, 200, 340, 24, rgb(255, 255, 255, 80));
    auto triangulo = [](Vector2 a, Vector2 b, Vector2 d) {
        for (float e : {3.0f})
            for (auto [p0, p1] : {std::pair{a, b}, std::pair{b, d}, std::pair{d, a}}) DrawLineEx(p0, p1, e * 2, ui::TINTA);
        DrawTriangle(a, b, d, WHITE);
        DrawTriangle(a, d, b, WHITE);
    };
    if (!p.em_rodada) {
        triangulo({c.x - 10, c.y - 20}, {c.x - 10, c.y + 18}, {c.x + 22, c.y - 1});
    } else {
        for (float dx : {-16.0f, 4.0f}) triangulo({c.x + dx, c.y - 16}, {c.x + dx, c.y + 14}, {c.x + dx + 18, c.y - 1});
    }
    ui::tecla_centro("Espaço", c.x, r.y + r.height + 4, 8);
    const Rectangle ra{PAINEL_X + 118, ui::ALTURA - 104.0f, 104, 34};
    ui::ret(ra, ui::TINTA, 17);
    ui::ret(ui::inflar(ra, -6, -6), ui::MADEIRA_ESCURA, 12);
    ui::texto("AUTO", ra.x + 14, ra.y + ra.height / 2, 11, ui::BRANCO, 0, Ancora::MIDLEFT);
    const Rectangle sw{ra.x + ra.width - 44, ra.y + 6, 38, 22};
    ui::ret(sw, ui::TINTA, 11);
    ui::ret(ui::inflar(sw, -4, -4), p.automatico ? ui::VERDE : ui::CINZA, 9);
    const float bx = p.automatico ? sw.x + sw.width - 11 : sw.x + 11;
    DrawCircleV({bx, sw.y + 11}, 8, ui::TINTA);
    DrawCircleV({bx, sw.y + 11}, 6, ui::BRANCO);
    ui::texto("Rodada automática", ra.x + 2, ra.y + 48, 12, ui::BEGE, 0, Ancora::MIDLEFT, ui::Peso::TEXTO);
    ui::texto(p.automatico ? "ligada" : "desligada", ra.x + 2, ra.y + 64, 12, p.automatico ? ui::ECO : ui::BEGE, 0,
              Ancora::MIDLEFT, ui::Peso::TEXTO);
    botoes_up_.push_back({ra, [this] { alternar_auto(); }});
    if (sobre_play)
        dicas_.push_back({mouse, !p.em_rodada ? "Iniciar rodada" : "Acelerar",
                          "Começa a próxima rodada. Durante a rodada, alterna velocidade 1x/3x."});
}

void CenaJogo::dica(const Dica& d, float largura) {
    const auto linhas = d.desc.empty() ? std::vector<std::string>{} : ui::quebrar(d.desc, 15, largura - 24, ui::Peso::TEXTO);
    const bool com_preco = d.preco >= 0;
    const float h = 40 + 20.0f * linhas.size() + (com_preco ? 34 : 0);
    float x, y;
    if (d.ancora.width > 0) {
        // ao lado do painel de upgrade, com a setinha apontando para ele
        x = d.ancora.x > LARGURA_MAPA / 2.0f ? d.ancora.x - largura - 12 : d.ancora.x + d.ancora.width + 12;
        y = std::clamp(d.pos.y - 40, 8.0f, ui::ALTURA - h - 8);
    } else {
        x = std::min(d.pos.x + 16, ui::LARGURA - largura - 4);
        if (d.pos.x >= PAINEL_X) x = PAINEL_X - largura - 8;
        y = std::max(4.0f, std::min(d.pos.y + 10, ui::ALTURA - h - 4));
    }
    const Rectangle r{x, y, largura, h};
    ui::ret(ui::mover(r, 0, 5), rgb(22, 20, 26, 102), 12);
    ui::ret(r, ui::TINTA, 12);
    ui::ret(ui::inflar(r, -6, -6), ui::PERGAMINHO, 9);
    ui::texto(d.titulo, r.x + 12, r.y + 8, 15, ui::MADEIRA_ESCURA, 0);
    for (size_t i = 0; i < linhas.size(); ++i)
        ui::texto(linhas[i], r.x + 12, r.y + 32 + i * 20.0f, 15, ui::TEXTO_ESCURO, 0, Ancora::TOPLEFT, ui::Peso::TEXTO);
    if (com_preco) {
        const float yy = r.y + h - 34;
        for (float xx = r.x + 12; xx < r.x + largura - 12; xx += 8) DrawRectangleRec({xx, yy, 4, 2}, rgb(217, 191, 134));
        const Rectangle pp = ui::pilula_preco(d.preco, r.x + 12 + 30, yy + 16, d.pode, 12);
        const Rectangle kt = ui::tecla(d.tecla, pp.x + pp.width + 8, yy + 7);
        ui::texto(d.pode ? "para comprar" : "falta dinheiro", kt.x + kt.width + 8, yy + 16, 13, rgb(110, 69, 35), 0,
                  Ancora::MIDLEFT, ui::Peso::TEXTO);
    }
}

// ---------------------------------------------------------------- upgrades (3 trilhas lado a lado)
void CenaJogo::cabecalho_upgrade(const Torre& t, float x, float y, float w) {
    const Vector2 c{x + 27, y + 27};
    DrawCircleV(c, 27, ui::TINTA);
    DrawCircleV(c, 24, ui::PAINEL_VERDE_ESC);
    arte::torre_icone(t.chave, arte::visual(t), c.x, c.y + 2, 48);
    const std::string& nome = t.dfn->nome;
    const int tam = nome.size() < 18 ? 18 : 15;
    ui::texto(caber(nome, tam, w - 150), x + 66, y + 16, tam, ui::BRANCO, 4, Ancora::MIDLEFT);
    ui::texto("Estouros: " + ui::formatar(t.pops), x + 66, y + 40, 14, ui::TINTA, 0, Ancora::MIDLEFT, ui::Peso::TEXTO);
    std::string tag;
    if (t.dfn->heroi) tag = "Nv " + std::to_string(t.nivel);
    else tag = std::to_string(t.caminhos[0]) + "-" + std::to_string(t.caminhos[1]) + "-" + std::to_string(t.caminhos[2]);
    const float wt = ui::medir(tag, 13).x + 22;
    const Rectangle rt{x + w - wt, y + 13, wt, 28};
    ui::ret(rt, ui::TINTA, 14);
    ui::ret(ui::inflar(rt, -5, -5), ui::PAINEL_VERDE_ESC, 11);
    ui::texto(tag, rt.x + rt.width / 2, rt.y + 13, 13, ui::BRANCO, 0, Ancora::CENTER);
}

void CenaJogo::rodape_upgrade(const Torre& t, float x, float y, float w, Vector2 mouse) {
    // prioridade de alvo com seta e atalho Tab
    const float wv = t.temporaria ? 0.0f : 150.0f;
    const float wm = t.chave == "bumerangue" ? 52.0f : 0.0f;  // espaco do botao de mao
    const Rectangle ra{x, y, w - wv - (wv ? 8 : 0) - wm, 44};
    ui::ret(ra, ui::TINTA, 12);
    ui::ret(ui::inflar(ra, -6, -6), rgb(36, 82, 24), 9);
    ui::ret({ra.x + 3, ra.y + 3, ra.width - 6, ra.height - 10}, ui::PAINEL_VERDE_ESC, 9);
    seta({ra.x + 18, ra.y + 22}, 10, false, rgb(155, 230, 110));
    const Rectangle kt = ui::tecla("Tab", ra.x + ra.width - 42, ra.y + 13, 9);
    seta({kt.x - 14, ra.y + 22}, 10, true, rgb(155, 230, 110));
    const float cx = (ra.x + 28 + kt.x - 24) / 2;
    const int id = t.id;
    if (t.chave == "dartling") {
        // a Dartling nao escolhe bloon: aponta para o cursor (Normal) ou fica onde o jogador travou
        const bool travada = mira_travada_.count(id) > 0;
        ui::texto("MIRA", cx, ra.y + 12, 10, rgb(207, 239, 191), 0, Ancora::CENTER, ui::Peso::TEXTO);
        ui::texto(travada ? "Travada" : "No cursor", cx, ra.y + 28, 13, ui::BRANCO, 3, Ancora::CENTER);
        botoes_up_.push_back({ra, [this, id] { alternar_mira(id); }});
        if (ui::dentro(ra, mouse))
            dicas_.push_back({mouse, "Mira da Dartling", "Segue o cursor. Clique ou Tab para travar onde está."});
    } else if (t.chave == "heli") {
        // o Heli escolhe como voa: sozinho atras dos bloons, atras do cursor, parado ou em patrulha
        const int m = modo_heli(t);
        const bool esperando = definindo_alvo_ == id;
        const char* nomes[4] = {t.st.persegue ? "Perseguir" : "Automático", "Seguir o mouse", "Travado", "Patrulha"};
        ui::texto("VOO", cx, ra.y + 12, 10, rgb(207, 239, 191), 0, Ancora::CENTER, ui::Peso::TEXTO);
        ui::texto(esperando ? "Clique no mapa" : nomes[m], cx, ra.y + 28, 13, esperando ? ui::AMARELO : ui::BRANCO, 3,
                  Ancora::CENTER);
        botoes_up_.push_back({ra, [this, id] { proximo_modo_heli(id); }});
        if (ui::dentro(ra, mouse))
            dicas_.push_back({mouse, "Modo de voo", "Clique ou Tab para trocar. A patrulha pede um clique no mapa."});
    } else if (t.chave == "as") {
        // o As nao escolhe bloon: o jogador escolhe a rota do voo
        static const char* ROTAS[4] = {"Círculo", "Infinito", "Oito", "Centralizada"};
        const bool esperando = definindo_alvo_ == id;
        ui::texto("ROTA", cx, ra.y + 12, 10, rgb(207, 239, 191), 0, Ancora::CENTER, ui::Peso::TEXTO);
        ui::texto(esperando ? "Clique no mapa" : ROTAS[t.rota], cx, ra.y + 28, 13, esperando ? ui::AMARELO : ui::BRANCO, 3,
                  Ancora::CENTER);
        botoes_up_.push_back({ra, [this, id] { proxima_rota(id); }});
        if (ui::dentro(ra, mouse))
            dicas_.push_back({mouse, "Rota do voo", "Clique ou Tab para trocar. A Rota Centralizada pede o upgrade e um clique no mapa."});
    } else if (t.chave == "morteiro") {
        // o Morteiro bombardeia um ponto fixo, escolhido com um clique no mapa
        const bool esperando = definindo_alvo_ == id;
        ui::texto("ALVO", cx, ra.y + 12, 10, rgb(207, 239, 191), 0, Ancora::CENTER, ui::Peso::TEXTO);
        ui::texto(esperando ? "Clique no mapa" : t.tem_mira ? "Ponto fixo" : "Definir alvo", cx, ra.y + 28, 13,
                  esperando ? ui::AMARELO : ui::BRANCO, 3, Ancora::CENTER);
        botoes_up_.push_back({ra, [this, id] { definindo_alvo_ = definindo_alvo_ == id ? 0 : id; }});
        if (ui::dentro(ra, mouse))
            dicas_.push_back({mouse, "Alvo do Morteiro", "Clique aqui (ou Tab) e depois no mapa para fixar onde as bombas caem."});
    } else {
        ui::texto("ALVO", cx, ra.y + 12, 10, rgb(207, 239, 191), 0, Ancora::CENTER, ui::Peso::TEXTO);
        ui::texto(NOMES_MODO[t.modo], cx, ra.y + 28, 13, ui::BRANCO, 3, Ancora::CENTER);
        botoes_up_.push_back({ra, [this, id] {
                                  if (TorreP tt = pista().torre(id))
                                      comando("M" + std::to_string(id) + ":" +
                                              std::to_string((tt->modo + 1) % modos_de_alvo(*tt)));
                              }});
        if (ui::dentro(ra, mouse)) dicas_.push_back({mouse, "Prioridade de alvo", "Clique ou Tab para trocar."});
    }
    if (wm > 0) {
        // mao do arremesso: inverte o lado do arco do bumerangue
        const Rectangle rm{ra.x + ra.width + 8, y, 44, 44};
        ui::ret(rm, ui::TINTA, 12);
        ui::ret(ui::inflar(rm, -6, -6), rgb(36, 82, 24), 9);
        ui::ret({rm.x + 3, rm.y + 3, rm.width - 6, rm.height - 10}, ui::PAINEL_VERDE_ESC, 9);
        ui::texto("MÃO", rm.x + 22, rm.y + 12, 10, rgb(207, 239, 191), 0, Ancora::CENTER, ui::Peso::TEXTO);
        ui::texto(t.mao ? "Esq" : "Dir", rm.x + 22, rm.y + 28, 13, ui::BRANCO, 3, Ancora::CENTER);
        const int mao = t.mao ? 0 : 1;
        botoes_up_.push_back({rm, [this, id, mao] { comando("O" + std::to_string(id) + ":" + std::to_string(mao)); }});
        if (ui::dentro(rm, mouse))
            dicas_.push_back({mouse, "Mão do arremesso", "Troca o lado para onde o bumerangue faz a curva."});
    }
    if (t.temporaria) return;
    // vender em vermelho (convencao de sair/vender)
    const Rectangle rv{x + w - wv, y, wv, 44};
    const bool sobre = ui::dentro(rv, mouse);
    const Rectangle rb = ui::mover(rv, 0, sobre ? (IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? 3.0f : -2.0f) : 0);
    bloco(rb, sobre ? ui::clarear(ui::VERMELHO, 0.15f) : ui::VERMELHO, ui::VERMELHO_ESCURO, 12, sobre ? 6.0f : 4.0f);
    const std::string preco = "$" + ui::formatar(pista().valor_venda(t));
    const float wa = ui::medir("Vender", 13).x, wp = ui::medir(preco, 13).x;
    const float x0 = rb.x + rb.width / 2 - (wa + 8 + wp) / 2;
    ui::texto("Vender", x0, rb.y + 20, 13, ui::BRANCO, 3, Ancora::MIDLEFT);
    ui::texto(preco, x0 + wa + 8, rb.y + 20, 13, ui::DINHEIRO, 3, Ancora::MIDLEFT);
    botoes_up_.push_back({rv, [this] { vender(); }});
    if (sobre) dicas_.push_back({mouse, "Vender", "Backspace também vende a torre selecionada."});
}

void CenaJogo::painel_upgrade(const TorreP& tp, Vector2 mouse) {
    const Torre& t = *tp;
    // torre na metade esquerda -> painel a direita (e vice-versa); na Batalha fica abaixo do mini mapa
    const float x = t.x > LARGURA_MAPA / 2.0 ? 12.0f : LARGURA_MAPA - UP_W - 12;
    const float y = ctl_->online() ? 240.0f : 80.0f;
    const Rectangle r{x, y, UP_W, UP_H};
    ui::ret(ui::mover(r, 0, 6), rgb(22, 20, 26, 115), 18);
    ui::ret(r, ui::TINTA, 18);
    ui::ret(ui::inflar(r, -6, -6), ui::PAINEL_VERDE_ESC, 15);
    const Rectangle f = ui::inflar(r, -12, -12);
    ui::ret(f, ui::PAINEL_VERDE, 12);
    ui::ret({f.x + 4, f.y + 2, f.width - 8, 6}, rgb(255, 255, 255, 36), 3);
    const float px = x + 13, pw = UP_W - 26;
    cabecalho_upgrade(t, px, y + 12, pw);
    const float yy = y + 76;
    if (t.dfn->heroi) {
        painel_heroi(t, px, yy, pw);
    } else if (t.temporaria) {
        ui::texto("Temporária: " + std::to_string(static_cast<int>(t.temporaria)) + " s", px + pw / 2, yy + 70, 18,
                  ui::BRANCO, 3, Ancora::CENTER);
    } else {
        for (int pth = 0; pth < 3; ++pth) linha_upgrade(tp, pth, px + pth * 120.0f, yy, 112, mouse);
    }
    rodape_upgrade(t, px, y + UP_H - 12 - 44, pw, mouse);
    for (auto& d : dicas_)
        if (d.preco >= 0) d.ancora = r;
}

void CenaJogo::linha_upgrade(const TorreP& tp, int pth, float x, float y, float w, Vector2 mouse) {
    const Torre& t = *tp;
    Pista& p = pista();
    const int tier = t.caminhos[pth];
    ui::pips(x - 1, y, tier);
    const std::string atual = tier > 0 ? t.dfn->caminhos[pth][tier - 1].nome : "-";
    ui::texto(caber(atual, 12, w, ui::Peso::TEXTO), x, y + 26, 12, ui::BRANCO, 0, Ancora::MIDLEFT, ui::Peso::TEXTO);
    const Rectangle card{x, y + 38, w, 132};
    const Vector2 cc{card.x + card.width / 2, card.y + card.height / 2};
    const std::string atalho(1, ",./"[pth]);
    auto nome_card = [&](const std::string& nome, Color cor, float topo) {
        auto linhas = ui::quebrar(nome, 13, w - 14);
        for (size_t i = 0; i < linhas.size() && i < 3; ++i)
            ui::texto(linhas[i], cc.x, topo + i * 19.0f, 13, cor, 4, Ancora::CENTER);
    };
    if (tier >= 5) {
        // tier 5: moldura especial dourada
        ui::ret(ui::inflar(card, 12, 12), ui::TINTA, 16);
        ui::ret(ui::inflar(card, 6, 6), ui::BRANCO, 14);
        bloco(card, ui::AMARELO, ui::OURO_ESCURO, 12, 4);
        nome_card(t.dfn->caminhos[pth][4].nome, ui::BRANCO, card.y + 32);
        ui::pilula("MÁXIMO", cc.x, card.y + card.height - 26, ui::TINTA, ui::AMARELO, 13);
        return;
    }
    const Upgrade& up = t.dfn->caminhos[pth][tier];
    auto custo = p.custo_upgrade(t, pth);
    if (!custo) {
        // caminho fechado: listras cinza e cadeado
        ui::ret(card, ui::TINTA, 12);
        const Rectangle f = ui::inflar(card, -6, -6);
        ui::ret(f, ui::CINZA_ESCURO, 9);
        ui::recortar(ui::inflar(f, -4, -4));
        for (float k = -f.height; k < f.width + f.height; k += 20)
            DrawLineEx({f.x + k, f.y + f.height}, {f.x + k + f.height, f.y}, 8, rgb(74, 74, 82));
        ui::fim_recorte();
        arte::icone("cadeado", cc.x - 15, card.y + 22, 30);
        nome_card("Caminho fechado", rgb(216, 216, 222), card.y + 72);
        ui::texto(tier > 0 ? "limite de tier" : "2 caminhos já em uso", cc.x, card.y + 112, 11, rgb(199, 205, 214), 0,
                  Ancora::CENTER, ui::Peso::TEXTO);
        return;
    }
    if (const std::string req = p.requisito_upgrade(t, pth); !req.empty()) {
        // upgrade condicional: cinza, com o nome, o cadeado e o que falta
        bloco(card, ui::CINZA, ui::CINZA_ESCURO, 12, 4, false);
        arte::icone("cadeado", cc.x - 13, card.y + 10, 26);
        nome_card(up.nome, rgb(236, 236, 240), card.y + 52);
        const auto ls = ui::quebrar(req, 11, w - 12, ui::Peso::TEXTO);
        for (size_t i = 0; i < ls.size() && i < 3; ++i)
            ui::texto(ls[i], cc.x, card.y + 84 + i * 14.0f, 11, ui::AMARELO, 0, Ancora::CENTER, ui::Peso::TEXTO);
        botoes_up_.push_back({card, [this, pth] { upar(pth); }});
        if (ui::dentro(card, mouse)) dicas_.push_back({mouse, up.nome, up.desc + " " + req + "."});
        return;
    }
    const bool pode = p.dinheiro >= *custo;
    const bool sobre = ui::dentro(card, mouse);
    const Color face = pode ? (sobre ? rgb(90, 174, 240) : ui::AZUL) : rgb(108, 127, 148);
    const Color lab = pode ? ui::AZUL_ESCURO : rgb(62, 74, 87);
    const Rectangle rc = ui::mover(card, 0, sobre && pode ? -2.0f : 0);
    if (sobre && pode) ui::ret(ui::inflar(rc, 6, 6), ui::AMARELO, 15);
    bloco(rc, face, lab, 12, sobre ? 6.0f : 4.0f);
    ui::tecla(atalho, rc.x + rc.width - 23, rc.y + 5, 10);
    nome_card(up.nome, pode ? ui::BRANCO : rgb(228, 232, 238), rc.y + 34);
    ui::pilula_preco(*custo, cc.x, rc.y + rc.height - 36, pode, 13);
    if (!pode)
        ui::texto("faltam $" + ui::formatar(std::ceil(*custo - p.dinheiro)), cc.x, rc.y + rc.height - 16, 11, rgb(255, 217, 212), 0,
                  Ancora::CENTER, ui::Peso::TEXTO);
    botoes_up_.push_back({card, [this, pth] { upar(pth); }});
    if (sobre) {
        Dica d{mouse, up.nome, up.desc.empty() ? "Melhora a torre." : up.desc};
        d.preco = *custo;
        d.pode = pode;
        d.tecla = atalho;
        dicas_.push_back(d);
    }
}

void CenaJogo::painel_heroi(const Torre& t, float x, float y, float w) {
    ui::texto("Nível " + std::to_string(t.nivel), x, y + 12, 20, ui::AMARELO, 4, Ancora::MIDLEFT);
    if (t.nivel < 20) {
        const double a = XP_NIVEL[t.nivel] * t.dfn->xp_escala, b = XP_NIVEL[t.nivel + 1] * t.dfn->xp_escala;
        ui::barra({x + 120, y + 8, w - 130, 12}, static_cast<float>((t.xp - a) / std::max(1.0, b - a)), rgb(120, 200, 255));
        ui::texto("XP " + std::to_string(static_cast<int>(t.xp)) + "/" + std::to_string(static_cast<int>(b)), x + 120,
                  y + 32, 12, ui::BRANCO, 0, Ancora::MIDLEFT, ui::Peso::TEXTO);
    }
    const DefTorre& dfn = *t.dfn;
    ui::texto(dfn.titulo, x, y + 56, 14, rgb(233, 255, 217), 0, Ancora::MIDLEFT, ui::Peso::TEXTO);
    const std::pair<int, const J*> habs[2] = {{3, &dfn.hab3}, {10, &dfn.hab10}};
    for (int i = 0; i < 2; ++i) {
        const J& h = *habs[i].second;
        if (h.is_null()) continue;
        const bool ok = t.nivel >= habs[i].first;
        const float yy = y + 80 + i * 40.0f;
        const Rectangle rb{x, yy, w, 34};
        ui::ret(rb, ui::TINTA, 10);
        ui::ret(ui::inflar(rb, -5, -5), ok ? ui::PAINEL_VERDE_ESC : rgb(70, 90, 60), 8);
        arte::habilidade(dfn.chave, true, -1, 0, habs[i].first, h.value("tipo", std::string("invocar")), rb.x + 20,
                         rb.y + 17, 30);
        ui::texto("Nv " + std::to_string(habs[i].first) + ": " + h["nome"].get<std::string>(), rb.x + 42, rb.y + 17, 12,
                  ok ? ui::BRANCO : rgb(170, 190, 170), ok ? 3 : 0, Ancora::MIDLEFT);
    }
    ui::texto("Sobe de nível estourando bloons.", x, y + 172, 12, rgb(233, 255, 217), 0, Ancora::MIDLEFT, ui::Peso::TEXTO);
}

// ---------------------------------------------------------------- habilidades
void CenaJogo::habilidades(Vector2 mouse) {
    auto habs = hab_lista();
    if (habs.empty()) return;
    const float topo = ctl_->online() ? 540.0f : 640.0f;
    const double agora = ui::tempo();
    for (size_t i = 0; i < habs.size(); ++i) {
        const Hab& hb = habs[i];
        const float cx = 42 + i * 72.0f, cy = topo + 30;
        const double rec = hb.t->hab_rec[hb.idx];
        const bool pronto = rec <= 0;
        const double recarga = (*hb.h)["recarga"].get<double>();
        if (pronto) {
            // pronta: anel amarelo pulsando
            const int a = static_cast<int>(90 + 60 * std::sin(agora * 6));
            DrawRing({cx, cy}, 32, 37, 0, 360, 48, ui::com_alfa(ui::AMARELO, a));
        }
        DrawCircleV({cx, cy + 4}, 30, rgb(22, 20, 26, 128));
        const arte::Visual v = arte::visual(*hb.t);
        arte::habilidade(hb.t->chave, hb.t->dfn->heroi, v.cam < 0 ? 1 : v.cam, std::max(3, v.tier), hb.t->nivel,
                         hb.h->value("tipo", std::string("invocar")), cx, cy, 62);
        if (!pronto) {
            const float frac = static_cast<float>(std::min(1.0, rec / recarga));
            DrawCircleSector({cx, cy}, 27, -90, -90 + 360 * frac, 36, rgb(22, 20, 26, 178));
            DrawCircleV({cx, cy}, 27, rgb(140, 140, 150, 60));
            ui::texto(std::to_string(static_cast<int>(rec) + 1), cx, cy, 16, ui::BRANCO, 4, Ancora::CENTER);
        }
        const Rectangle kt{cx + 16, cy + 16, 22, 22};
        ui::ret(kt, ui::TINTA, 6);
        ui::ret({kt.x + 2, kt.y + kt.height - 3, kt.width - 4, 2}, rgb(58, 54, 64), 1);
        ui::texto(std::to_string(i + 1), kt.x + 11, kt.y + 10, 11, pronto ? ui::AMARELO : ui::BRANCO, 0, Ancora::CENTER);
        const Rectangle r{cx - 30, cy - 30, 60, 60};
        const int id = hb.t->id, idx = hb.idx;
        botoes_hab_.push_back({r, [this, id, idx] { comando("B" + std::to_string(id) + ":" + std::to_string(idx)); }});
        if (ui::dentro(r, mouse))
            dicas_.push_back({mouse, (*hb.h)["nome"].get<std::string>(),
                              hb.t->dfn->nome + " (recarga " + std::to_string(static_cast<int>(recarga)) + " s)"});
    }
}

// ---------------------------------------------------------------- batalha
void CenaJogo::painel_envios(Vector2 mouse) {
    const Rectangle r{0, ui::ALTURA - ENVIO_H, LARGURA_MAPA, ENVIO_H};
    DrawRectangleRec(r, ui::MADEIRA_ESCURA);
    DrawRectangleRec({r.x, r.y, r.width, 4}, ui::TINTA);
    DrawRectangleRec({r.x, r.y + 4, r.width, 3}, rgb(255, 255, 255, 36));
    Pista& p = pista();
    Partida& pt = *ctl_->partida;
    const int rodada = pt.rodada;
    const float bw = 82, bh = 44;
    for (size_t i = 0; i < ENVIOS.size(); ++i) {
        const Envio& env = ENVIOS[i];
        const float col = static_cast<float>(i % 12), lin = static_cast<float>(i / 12);
        const Rectangle b{6 + col * (bw + 4), r.y + 8 + lin * (bh + 4), bw, bh};
        const bool bloqueado = rodada < env.rodada_min;
        const bool pode = !bloqueado && p.dinheiro >= env.custo;
        const bool sobre = ui::dentro(b, mouse) && !bloqueado;
        Color c = bloqueado ? MADEIRA_BLOQ : pode ? ui::BEGE : ui::BEGE_APAGADO;
        if (sobre && pode) c = rgb(255, 246, 218);
        ui::ret(ui::mover(b, 0, 2), rgb(0, 0, 0, 102), 10);
        ui::ret(b, ui::TINTA, 10);
        const Rectangle f = ui::inflar(b, -5, -5);
        ui::ret(f, c, 8);
        ui::ret({f.x, f.y + f.height - 3, f.width, 3}, rgb(0, 0, 0, 46), 2);
        const TipoBloon& tb = tipo_bloon(env.tipo);
        const float icx = b.x + 17, icy = b.y + bh / 2;
        if (tb.moab) {
            const Vector2 tam = arte::tamanho_dirigivel(tb);
            arte::dirigivel(tb, env.fort, 0, icx, icy, 0, 30 / tam.x);
        } else {
            arte::bloon(tb, env.camo, env.regen, env.fort, 0, icx, icy - 2, 13 / arte::raio_bloon_px(tb));
        }
        if (bloqueado) {
            DrawRectangleRec(f, rgb(58, 42, 30, 170));
            arte::icone("cadeado", b.x + b.width - 44, b.y + 14, 16);
            ui::texto("R" + std::to_string(env.rodada_min), b.x + b.width - 8, b.y + bh / 2, 12, rgb(216, 216, 222), 3,
                      Ancora::MIDRIGHT);
        } else {
            // custo e renda separados: pilula de preco em cima, eco embaixo
            const std::string preco = "$" + ui::formatar(env.custo);
            const float wp = ui::medir(preco, 10).x + 10;
            const Rectangle pp{b.x + b.width - 5 - wp, b.y + 5, wp, 17};
            ui::ret(pp, pode ? ui::TINTA : ui::VERMELHO_ESCURO, 8.5f);
            ui::texto(preco, pp.x + pp.width / 2, pp.y + 8, 10, pode ? ui::DINHEIRO : ui::PRECO_RUIM, 0, Ancora::CENTER);
            const Color ce = env.eco > 0 ? ui::ECO_ESCURO : env.eco < 0 ? rgb(179, 38, 30) : rgb(110, 69, 35);
            ui::texto(numero_g(env.eco), b.x + b.width - 7, b.y + 32, 12, ce, 0, Ancora::MIDRIGHT, ui::Peso::TEXTO);
            if (env.qtd > 1) ui::texto("x" + std::to_string(env.qtd), b.x + 4, b.y + bh - 8, 9, ui::BRANCO, 3, Ancora::MIDLEFT);
            // recarga de 0,6 s entre envios iguais: barra escura + filete amarelo
            const double falta = pt.recarga_envio(ctl_->meu, env.chave);
            if (falta > 0) {
                const float frac = static_cast<float>(std::min(1.0, falta / 0.6));
                DrawRectangleRec({f.x, f.y, f.width * frac, f.height}, rgb(22, 20, 26, 115));
                DrawRectangleRec({f.x, f.y + f.height - 4, f.width * frac, 4}, ui::AMARELO);
            }
            botoes_envio_.push_back({b, [this, &env] { enviar(env); }});
        }
        if (ui::dentro(b, mouse))
            dicas_.push_back({mouse, env.nome,
                              "Custo $" + std::to_string(env.custo) + ". Renda " + numero_g(env.eco) +
                                  " por ciclo de eco. Libera na rodada " + std::to_string(env.rodada_min) + "."});
    }
}

void CenaJogo::mini_oponente() {
    const Rectangle r = rect_mini_;
    // faixa do oponente: icone, nome, vidas e atalho O
    const Rectangle fx{r.x - 3, 12, r.width + 6, 34};
    ui::placa(fx, ui::MADEIRA_ESCURA, 12);
    arte::icone("oponente", fx.x + 8, fx.y + 6, 22);
    ui::texto("OPONENTE", fx.x + 36, fx.y + 16, 13, ui::BRANCO, 3, Ancora::MIDLEFT);
    const Rectangle kt = ui::tecla("O", fx.x + fx.width - 28, fx.y + 8, 10);
    const std::string vidas = std::to_string(std::max(0, ctl_->oponente().vidas));
    const float wv = ui::medir(vidas, 15).x;
    ui::texto(vidas, kt.x - 8, fx.y + 16, 15, ui::BRANCO, 3, Ancora::MIDRIGHT);
    arte::icone("coracao", kt.x - 12 - wv - 22, fx.y + 7, 20);
    ui::ret(ui::mover(ui::inflar(r, 6, 6), 0, 4), rgb(22, 20, 26, 115), 10);
    ui::ret(ui::inflar(r, 6, 6), ui::TINTA, 10);
    render_op_->desenhar_mini(r);
    // avisos em placas, alinhados a direita, ao lado do mini mapa
    float y = 12;
    for (auto it = avisos_op_.rbegin(); it != avisos_op_.rend(); ++it) {
        const std::string rot = it->recebendo ? "RECEBENDO" : "ENVIADO";
        const float w = ui::medir(rot, 11).x + ui::medir(it->txt, 14, ui::Peso::TEXTO).x + 30;
        const Rectangle ra{r.x - 16 - w, y, w, 30};
        ui::placa(ra, it->recebendo ? ui::VERMELHO_ESCURO : ui::MADEIRA_ESCURA, 10);
        ui::texto(rot, ra.x + 10, ra.y + 14, 11, it->recebendo ? ui::AMARELO : ui::ECO, 3, Ancora::MIDLEFT);
        ui::texto(it->txt, ra.x + ra.width - 10, ra.y + 14, 14, ui::BRANCO, 0, Ancora::MIDRIGHT, ui::Peso::TEXTO);
        y += 36;
    }
}

void CenaJogo::oponente_grande() {
    DrawRectangle(0, 0, LARGURA_MAPA, ui::ALTURA, rgb(22, 20, 26, 170));
    const Rectangle r{60, 50, 920, 637};
    ui::ret(ui::mover(ui::inflar(r, 12, 12), 0, 6), rgb(22, 20, 26, 115), 14);
    ui::ret(ui::inflar(r, 12, 12), ui::TINTA, 14);
    render_op_->desenhar_mini(r);
    const Rectangle pl{r.x + r.width / 2 - 250, r.y - 40, 500, 34};
    ui::placa(pl, ui::MADEIRA_ESCURA, 12);
    ui::texto("Mapa do oponente", pl.x + 16, pl.y + 16, 14, ui::BRANCO, 3, Ancora::MIDLEFT);
    ui::texto("clique ou", pl.x + pl.width - 44, pl.y + 16, 12, ui::BEGE, 0, Ancora::MIDRIGHT, ui::Peso::TEXTO);
    ui::tecla("O", pl.x + pl.width - 34, pl.y + 8, 10);
}

void CenaJogo::painel_log() {
    if (!ctl_->online()) return;
    auto linhas = ctl_->log();
    if (linhas.size() > 12) linhas.erase(linhas.begin(), linhas.end() - 12);
    const Rectangle r{12, 76, 330, 34 + 17.0f * std::max<size_t>(1, linhas.size())};
    ui::ret(r, rgb(22, 20, 26, 224), 10);
    ui::tecla("F1", r.x + 10, r.y + 8, 9);
    ui::texto("MENSAGENS DE REDE", r.x + 44, r.y + 17, 12, ui::AMARELO, 0, Ancora::MIDLEFT);
    ui::texto("tick " + ui::formatar(static_cast<double>(ctl_->tick_rede())), r.x + r.width - 10, r.y + 17, 11,
              rgb(154, 163, 174), 0, Ancora::MIDRIGHT, ui::Peso::MONO);
    for (size_t i = 0; i < linhas.size(); ++i) {
        const Color c = linhas[i].rfind(">", 0) == 0 ? rgb(168, 224, 255) : rgb(200, 245, 176);
        ui::texto(linhas[i], r.x + 10, r.y + 30 + i * 17.0f, 12, c, 0, Ancora::TOPLEFT, ui::Peso::MONO);
    }
}

// ---------------------------------------------------------------- mensagens e telas
void CenaJogo::mensagens() {
    if (msg_.second > 0 && !msg_.first.empty()) {
        // aviso em placa vermelha (antes era texto rosa solto)
        const float w = ui::medir(msg_.first, 16).x + 40;
        const Rectangle r{LARGURA_MAPA / 2.0f - w / 2, 150, w, 40};
        ui::placa(r, ui::VERMELHO_ESCURO, 12);
        ui::texto(msg_.first, r.x + r.width / 2, r.y + 19, 16, ui::BRANCO, 3, Ancora::CENTER);
    }
    if (banner_.second > 0 && !banner_.first.empty()) {
        const float w = ui::medir(banner_.first, 32).x + 60;
        const Rectangle r{LARGURA_MAPA / 2.0f - w / 2, 236, w, 64};
        ui::placa(r, ui::MADEIRA_ESCURA, 16);
        ui::texto(banner_.first, r.x + r.width / 2, r.y + 30, 32, ui::AMARELO, 5, Ancora::CENTER);
    }
    if (ctl_->online() && ctl_->dessincronizado()) {
        const Rectangle r{LARGURA_MAPA / 2.0f - 110, 100, 220, 32};
        ui::placa(r, ui::VERMELHO_ESCURO, 10);
        ui::texto("DESSINCRONIZADO", r.x + r.width / 2, r.y + 15, 14, ui::BRANCO, 3, Ancora::CENTER);
    }
}

Rectangle CenaJogo::sobreposicao(const std::string& titulo, Color cor) {
    DrawRectangle(0, 0, ui::LARGURA, ui::ALTURA, rgb(22, 20, 26, 150));
    const Rectangle r{ui::LARGURA / 2.0f - 260, ui::ALTURA / 2.0f - 210, 520, 420};
    ui::painel_menu(r, 24);
    ui::texto(titulo, r.x + r.width / 2, r.y + 56, 44, cor, 6, Ancora::CENTER, ui::Peso::LOGO);
    return r;
}

void CenaJogo::tela_sacrificio() {
    TorreP t = selecionada_ ? pista().torre(selecionada_) : nullptr;
    const std::string txt = t ? pista().aviso_sacrificio(*t, confirma_up_) : "";
    if (txt.empty()) {
        confirma_up_ = -1;
        return;
    }
    DrawRectangle(0, 0, ui::LARGURA, ui::ALTURA, rgb(22, 20, 26, 150));
    const Rectangle r{ui::LARGURA / 2.0f - 290, ui::ALTURA / 2.0f - 140, 580, 280};
    ui::painel_menu(r, 24);
    ui::texto("SACRIFÍCIO", r.x + r.width / 2, r.y + 46, 34, ui::AMARELO, 6, Ancora::CENTER, ui::Peso::LOGO);
    const auto linhas = ui::quebrar(txt, 17, r.width - 70, ui::Peso::TEXTO);
    for (size_t i = 0; i < linhas.size() && i < 4; ++i)
        ui::texto(linhas[i], r.x + r.width / 2, r.y + 100 + i * 24.0f, 17, ui::BRANCO, 0, Ancora::CENTER, ui::Peso::TEXTO);
    ui::Botao nao{{r.x + 50, r.y + r.height - 78, 220, 54}, "Cancelar", ui::AMARELO, 20};
    ui::Botao sim{{r.x + r.width - 270, r.y + r.height - 78, 220, 54}, "Sacrificar", ui::VERDE, 20};
    nao.atalho = "Esc";
    sim.atalho = "Enter";
    nao.desenhar();
    sim.desenhar();
    botoes_menu_.push_back({nao, [this] { confirma_up_ = -1; }});
    const int p = confirma_up_;
    botoes_menu_.push_back({sim, [this, p] { upar(p); }});
}

void CenaJogo::tela_pausa() {
    const Rectangle r = sobreposicao("PAUSADO", ui::BRANCO);
    std::vector<std::tuple<std::string, Acao, Color>> opcoes;
    opcoes.emplace_back("Continuar", [this] { pausar(false); }, ui::VERDE);
    if (!ctl_->online()) {
        opcoes.emplace_back("Reiniciar", [this] { reiniciar(); }, ui::AMARELO);
        opcoes.emplace_back(std::string("Rodada automática: ") + (ctl_->partida->automatico ? "ON" : "OFF"),
                            [this] { alternar_auto(); }, ui::AZUL);
    } else {
        opcoes.emplace_back("Desistir", [this] { desistir(); }, ui::VERMELHO);
    }
    opcoes.emplace_back("Menu principal", [this] { app.ir_menu(); }, ui::VERMELHO);
    for (size_t i = 0; i < opcoes.size(); ++i) {
        auto& [rot, acao, cor] = opcoes[i];
        ui::Botao b{{r.x + 90, r.y + 110 + i * 68.0f, r.width - 180, 56}, rot, cor, 20};
        if (i == 0) b.atalho = "Esc";
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
    for (int i = 0; i < 3; ++i) ui::texto(linhas[i], r.x + r.width / 2, r.y + 118 + i * 32.0f, 20, ui::BRANCO, 3, Ancora::CENTER);
    std::vector<std::tuple<std::string, Acao, Color>> opcoes;
    if (!ctl_->online() && ganhou)
        opcoes.emplace_back("Continuar em freeplay", [this] { ctl_->partida->continuar_freeplay(); }, ui::AMARELO);
    if (!ctl_->online()) opcoes.emplace_back("Jogar novamente", [this] { reiniciar(); }, ui::VERDE);
    opcoes.emplace_back("Menu principal", [this] { app.ir_menu(); }, ui::AZUL);
    const bool tres = opcoes.size() > 2;  // com o freeplay os botoes ficam mais juntos para caber no painel
    for (size_t i = 0; i < opcoes.size(); ++i) {
        auto& [rot, acao, cor] = opcoes[i];
        ui::Botao b{{r.x + 110, r.y + (tres ? 222 : 240) + i * (tres ? 62.0f : 66.0f), r.width - 220, tres ? 50.0f : 54.0f},
                    rot, cor, 20};
        b.desenhar();
        botoes_menu_.push_back({b, acao});
    }
}

void CenaJogo::sair() { ctl_->fechar(); }

}  // namespace bl
