#include "cliente/cenas_menu.hpp"

#include <cmath>

#include "cliente/arte.hpp"
#include "cliente/som.hpp"
#include "rlgl.h"

namespace bl {

using ui::Ancora;
using ui::rgb;
namespace P = proto;

void titulo(float y, float escala) {
    const float cx = ui::LARGURA / 2.0f;
    const int tam = static_cast<int>(86 * escala);
    // sombra dura em baixo e contorno de tinta
    ui::texto("BLOONS TD", cx, y + 8 * escala, tam, ui::TINTA, 8, Ancora::CENTER, ui::Peso::LOGO);
    ui::texto("BLOONS TD", cx, y, tam, ui::AMARELO, 8, Ancora::CENTER, ui::Peso::LOGO);
    // faixa vermelha inclinada com "BATTLES"
    const float w = 360 * escala, h = 78 * escala, fy = y + 78 * escala;
    rlPushMatrix();
    rlTranslatef(cx, fy, 0);
    rlRotatef(-3, 0, 0, 1);
    const Rectangle r{-w / 2, -h / 2, w, h};
    ui::ret(ui::mover(r, 0, 6), ui::TINTA, 14);
    ui::ret(r, ui::TINTA, 14);
    const Rectangle f = ui::inflar(r, -8, -8);
    ui::ret(f, ui::VERMELHO_ESCURO, 10);
    ui::ret({f.x, f.y, f.width, f.height - 6}, ui::VERMELHO, 10);
    ui::ret({f.x + 6, f.y + 2, f.width - 12, 4}, rgb(255, 255, 255, 77), 2);
    ui::texto("BATTLES", 0, 2 * escala, static_cast<int>(51 * escala), ui::BRANCO, 6, Ancora::CENTER, ui::Peso::LOGO);
    rlPopMatrix();
}

Rectangle painel_passo(Rectangle r, const std::string& rotulo) {
    ui::ret(ui::mover(r, 0, 6), rgb(22, 20, 26, 115), 20);
    ui::ret(r, ui::TINTA, 20);
    const Rectangle m = ui::inflar(r, -8, -8);
    ui::ret(m, ui::MADEIRA_ESCURA, 16);
    const Rectangle f = ui::inflar(m, -8, -8);
    ui::ret(f, ui::MADEIRA, 12);
    ui::texto(rotulo, r.x + 16, r.y + 26, 17, ui::BEGE, 4, Ancora::MIDLEFT);
    return f;
}

void info_heroi_card(const DefTorre& h, Rectangle r) {
    ui::ret(r, ui::TINTA, 14);
    ui::ret(ui::inflar(r, -6, -6), ui::MADEIRA_ESCURA, 11);
    const Vector2 c{r.x + 16 + 48, r.y + r.height / 2};
    DrawCircleV(c, 48, ui::TINTA);
    DrawCircleV(c, 44, rgb(247, 215, 116));
    arte::torre_viva(h.chave, {}, c.x, c.y + 4, 86, ui::tempo());
    const float x = r.x + 132;
    ui::texto(h.nome, x, r.y + 30, 23, ui::AMARELO, 5, Ancora::MIDLEFT);
    ui::texto(h.titulo, x, r.y + 58, 14, ui::BEGE, 0, Ancora::MIDLEFT, ui::Peso::TEXTO);
    ui::pilula("$" + ui::formatar(h.custo), x + 34, r.y + 84, ui::TINTA, ui::DINHEIRO, 12);
    std::string habs;
    if (!h.hab3.is_null()) habs += "Nv 3: " + h.hab3["nome"].get<std::string>();
    if (!h.hab10.is_null()) habs += "   ·   Nv 10: " + h.hab10["nome"].get<std::string>();
    ui::texto(habs, x, r.y + 112, 13, ui::BRANCO, 0, Ancora::MIDLEFT, ui::Peso::TEXTO);
}

// ====================================================================== MENU
CenaMenu::CenaMenu(App& a) : Cena(a) {
    const float x = 470, w = 340;
    botoes_ = {
        {{x, 318, w, 72}, "Jogar Solo", ui::VERDE, 28},
        {{x, 402, w, 62}, "Batalha: Hospedar", ui::AZUL, 22},
        {{x, 476, w, 62}, "Batalha: Entrar", ui::AZUL, 22},
        {{x, 558, w, 52}, "Sair", ui::VERMELHO, 19},
    };
}

void CenaMenu::evento(const ui::Evento& e) {
    for (size_t i = 0; i < botoes_.size(); ++i) {
        if (!botoes_[i].clicou(e)) continue;
        som::tocar("colocar");
        if (i == 0) app.trocar(std::make_unique<CenaSolo>(app));
        else if (i == 1) app.trocar(std::make_unique<CenaBatalha>(app, true));
        else if (i == 2) app.trocar(std::make_unique<CenaBatalha>(app, false));
        else app.sair();
    }
}

void CenaMenu::desenhar() {
    app.fundo.desenhar(dt_);
    titulo(110);
    // macacos no chao (com a base de grama), animados como no design
    const char* chaves[] = {"dardo", "super", "ninja", "mago"};
    const Vector2 pos[] = {{190, 420}, {1090, 420}, {300, 560}, {980, 560}};
    for (int i = 0; i < 4; ++i) {
        const Vector2 p = pos[i];
        DrawEllipse(static_cast<int>(p.x), static_cast<int>(p.y + 51), 73.0f, 20.0f, ui::TINTA);
        DrawEllipse(static_cast<int>(p.x), static_cast<int>(p.y + 51), 70.0f, 17.0f, ui::VERDE_ESCURO);
        arte::torre_viva(chaves[i], {}, p.x, p.y, 150, ui::tempo() + i * 0.37);
    }
    // botoes num painel de madeira (hierarquia: jogar em verde, batalha em azul, sair em vermelho)
    ui::painel_menu({452, 300, 376, 330}, 22);
    for (auto& b : botoes_) b.desenhar();
    ui::texto("Trabalho 02 de Sistemas Operacionais: comunicação entre processos", ui::LARGURA / 2.0f,
              ui::ALTURA - 30, 14, ui::BRANCO, 2, Ancora::CENTER, ui::Peso::TEXTO);
}

// ====================================================================== SELECAO DE HEROI
Rectangle GradeHerois::rect(size_t i) const {
    const float col = static_cast<float>(i % colunas_), lin = static_cast<float>(i / colunas_);
    return {x_ + col * (tam_ + 6), y_ + lin * (tam_ + 6), tam_, tam_};
}

void GradeHerois::evento(const ui::Evento& e) {
    if (e.tipo != ui::Evento::CLIQUE) return;
    for (size_t i = 0; i < herois().size(); ++i) {
        if (ui::dentro(rect(i), e.pos)) {
            escolhido = herois()[i].chave;
            som::tocar("colocar");
        }
    }
}

const DefTorre& GradeHerois::desenhar() const {
    const Vector2 m = ui::mouse();
    for (size_t i = 0; i < herois().size(); ++i) {
        const Rectangle r = rect(i);
        const bool sel = herois()[i].chave == escolhido;
        const bool sobre = ui::dentro(r, m);
        ui::ret(ui::mover(r, 0, 3), rgb(22, 20, 26, 128), 12);
        // escolhido: fundo dourado e borda amarela grossa (antes a diferenca era ~10% de luminancia)
        if (sel) ui::ret(ui::inflar(r, 4, 4), ui::TINTA, 14);
        ui::ret(r, sel ? ui::AMARELO : ui::TINTA, 12);
        ui::ret(ui::inflar(r, sel ? -8.0f : -6.0f, sel ? -8.0f : -6.0f), sel ? rgb(247, 215, 116) : sobre ? rgb(255, 246, 218) : ui::BEGE, 8);
        arte::torre_icone(herois()[i].chave, {}, r.x + r.width / 2, r.y + r.height / 2 + 1, 50);
    }
    return *achar_heroi(escolhido);
}

namespace {

void desenhar_mapas(float x0, float largura, const std::string& selecionado, bool detalhes) {
    const Color DIF[3] = {rgb(155, 230, 110), ui::AMARELO, rgb(255, 138, 92)};
    const float w = (largura - 54) / 4;
    for (size_t i = 0; i < mapas().size(); ++i) {
        const DefMapa& m = mapas()[i];
        const bool sel = m.chave == selecionado;
        Rectangle r{x0 + i * (w + 18), 124, w, 172};
        if (sel) r.y -= 4;
        ui::ret(ui::mover(r, 0, sel ? 8.0f : 4.0f), rgb(22, 20, 26, 140), 14);
        ui::ret(ui::inflar(r, sel ? 2.0f : 0.0f, sel ? 2.0f : 0.0f), sel ? ui::AMARELO : ui::TINTA, 14);
        const Rectangle f = ui::inflar(r, sel ? -8.0f : -6.0f, sel ? -8.0f : -6.0f);
        arte::fundo_mapa(m.chave, f);
        DrawRectangleRec({f.x, f.y + f.height - 38, f.width, 38}, rgb(22, 20, 26, 209));
        ui::texto(m.nome, f.x + 10, f.y + f.height - 19, 14, ui::BRANCO, 0, Ancora::MIDLEFT);
        if (detalhes || true) {
            const int k = m.dificuldade == "Iniciante" ? 0 : m.dificuldade == "Intermediário" ? 1 : 2;
            const float wd = ui::medir(m.dificuldade, 9).x + 18;
            const Rectangle rp{f.x + f.width - 10 - wd, f.y + f.height - 29, wd, 20};
            ui::ret(rp, ui::TINTA, 10);
            ui::ret(ui::inflar(rp, -4, -4), DIF[k], 8);
            ui::texto(m.dificuldade, rp.x + rp.width / 2, rp.y + 9, 9, ui::TINTA, 0, Ancora::CENTER);
        }
        if (sel) {
            const Vector2 c{f.x + f.width - 22, f.y + 22};
            DrawCircleV(c, 16, ui::TINTA);
            DrawCircleV(c, 13, ui::VERDE);
            DrawLineEx({c.x - 7, c.y}, {c.x - 2, c.y + 6}, 4, ui::BRANCO);
            DrawLineEx({c.x - 2, c.y + 6}, {c.x + 7, c.y - 6}, 4, ui::BRANCO);
        }
    }
}

void cabecalho(const std::string& t, const std::string& sub) {
    const Rectangle tr = ui::texto(t, 24, 44, 30, ui::BRANCO, 6, Ancora::MIDLEFT);
    const float w = ui::medir(sub, 14, ui::Peso::TEXTO).x + 24;
    const Rectangle r{tr.x + tr.width + 14, 30, w, 28};
    ui::ret(r, ui::TINTA, 14);
    ui::texto(sub, r.x + 12, r.y + 14, 14, ui::BEGE, 0, Ancora::MIDLEFT, ui::Peso::TEXTO);
}

}  // namespace

// ====================================================================== SOLO
namespace {
struct Restricao {
    const char* chave;
    const char* nome;
};
const Restricao RESTRICOES[4] = {{"", "Todas as torres"}, {"primaria", "Só Primárias"}, {"militar", "Só Militares"}, {"magica", "Só Mágicas"}};
}  // namespace

CenaSolo::CenaSolo(App& a) : Cena(a) {
    voltar_ = {{24, 646, 180, 56}, "Voltar", ui::AZUL, 19};
    voltar_.atalho = "Esc";
    jogar_ = {{ui::LARGURA - 304.0f, 638, 280, 68}, "JOGAR!", ui::VERDE, 26};
    jogar_.atalho = "Enter";
}

void CenaSolo::evento(const ui::Evento& e) {
    herois_.evento(e);
    if (e.tipo == ui::Evento::CLIQUE) {
        for (size_t i = 0; i < mapas().size(); ++i)
            if (ui::dentro(rect_mapa(i), e.pos)) mapa_ = mapas()[i].chave;
        for (size_t i = 0; i < DIFICULDADES.size(); ++i)
            if (ui::dentro(rect_dif(i), e.pos)) dif_ = DIFICULDADES[i].chave;
        for (size_t i = 0; i < 4; ++i)
            if (ui::dentro(rect_restricao(i), e.pos)) restricao_ = RESTRICOES[i].chave;
    }
    if (voltar_.clicou(e) || (e.tipo == ui::Evento::TECLA && e.tecla == KEY_ESCAPE)) app.ir_menu();
    if (jogar_.clicou(e) || (e.tipo == ui::Evento::TECLA && e.tecla == KEY_ENTER)) {
        som::tocar("rodada");
        int seed = std::uniform_int_distribution<int>(1, 999999)(app.rng);
        app.iniciar_jogo(std::make_unique<ControladorSolo>(mapa_, dif_, herois_.escolhido, seed, restricao_));
    }
}

void CenaSolo::desenhar() {
    ui::fundo_gradiente(rgb(90, 170, 70), rgb(47, 107, 40));
    cabecalho("Jogo Solo", "3 passos: mapa, dificuldade e herói");
    painel_passo({24, 80, 1232, 236}, "1 · Mapa");
    desenhar_mapas(40, 1200, mapa_, true);
    painel_passo({24, 332, 1232, 104}, "2 · Dificuldade");
    const Vector2 m = ui::mouse();
    for (size_t i = 0; i < DIFICULDADES.size(); ++i) {
        const Dificuldade& d = DIFICULDADES[i];
        const bool sel = d.chave == dif_;
        const bool sobre = ui::dentro(rect_dif(i), m);
        const Rectangle r = ui::mover(rect_dif(i), 0, sobre && !sel ? -2.0f : 0);
        // nao selecionada = bege (antes era marrom-cinza e parecia desabilitada)
        const Color face = sel ? ui::VERDE : sobre ? rgb(255, 246, 218) : ui::BEGE;
        const Color lab = sel ? ui::VERDE_ESCURO : rgb(217, 191, 134);
        ui::ret(ui::mover(r, 0, 4), rgb(22, 20, 26, 128), 12);
        ui::ret(r, ui::TINTA, 12);
        const Rectangle f = ui::inflar(r, -6, -6);
        ui::ret(f, lab, 9);
        ui::ret({f.x, f.y, f.width, f.height - 5}, face, 9);
        ui::ret({f.x + 4, f.y + 2, f.width - 8, 3}, rgb(255, 255, 255, 77), 2);
        ui::texto(d.nome, r.x + r.width / 2, r.y + 20, 15, sel ? ui::BRANCO : ui::MADEIRA_ESCURA, sel ? 4 : 0, Ancora::CENTER);
        const std::string vidas = std::to_string(d.vidas) + (d.vidas == 1 ? " vida" : " vidas");
        ui::texto(d.sandbox ? std::string("dinheiro e vidas infinitos")
                            : vidas + " · R" + std::to_string(d.primeira_rodada) + " a R" + std::to_string(d.ultima_rodada),
                  r.x + r.width / 2, r.y + 40, 11, sel ? ui::BRANCO : rgb(110, 69, 35), 0, Ancora::CENTER, ui::Peso::TEXTO);
    }
    for (size_t i = 0; i < 4; ++i) {
        const bool sel = restricao_ == RESTRICOES[i].chave;
        const Rectangle r = rect_restricao(i);
        ui::ret(r, ui::TINTA, 12);
        ui::ret(ui::inflar(r, -3, -3), sel ? ui::VERDE : ui::dentro(r, m) ? rgb(255, 246, 218) : ui::BEGE, 9);
        ui::texto(RESTRICOES[i].nome, r.x + r.width / 2, r.y + 12, 11, sel ? ui::BRANCO : ui::MADEIRA_ESCURA, sel ? 3 : 0,
                  Ancora::CENTER);
    }
    painel_passo({24, 452, 1232, 176}, "3 · Herói");
    const DefTorre& h = herois_.desenhar();
    info_heroi_card(h, {634, 468, 606, 144});
    voltar_.desenhar();
    jogar_.desenhar();
}

// ====================================================================== BATALHA
CenaBatalha::CenaBatalha(App& a, bool hospedar) : Cena(a), hospedar_(hospedar) {
    ip_ = {{40, 150, 380, 56}, P::HOST_PADRAO, "IP de quem hospeda"};
    porta_ = {{438, 150, 150, 56}, std::to_string(P::PORTA_PADRAO), "Porta"};
    porta_.max_len = 5;
    // hospedar: a porta e o passo 2, antes do heroi (antes ficava solta abaixo da grade)
    if (hospedar_) {
        porta_.rect = {40, 370, 150, 56};
        porta_.rotulo.clear();  // o titulo do passo ja diz "Porta"
    }
    else ip_.ativo = true;
    voltar_ = {{24, 646, 180, 56}, "Voltar", ui::AZUL, 19};
    voltar_.atalho = "Esc";
    ir_ = {{ui::LARGURA - 304.0f, 638, 280, 68}, hospedar ? "HOSPEDAR" : "CONECTAR", ui::VERDE, 24};
    ir_.atalho = "Enter";
    if (!hospedar_) herois_ = GradeHerois{40, 380, 9, 58};
}

void CenaBatalha::evento(const ui::Evento& e) {
    herois_.evento(e);
    if (!hospedar_) ip_.evento(e);
    porta_.evento(e);
    if (hospedar_ && e.tipo == ui::Evento::CLIQUE)
        for (size_t i = 0; i < mapas().size(); ++i)
            if (ui::dentro(rect_mapa(i), e.pos)) mapa_ = mapas()[i].chave;
    if (voltar_.clicou(e) || (e.tipo == ui::Evento::TECLA && e.tecla == KEY_ESCAPE && !ip_.ativo && !porta_.ativo))
        app.ir_menu();
    if (ir_.clicou(e) || (e.tipo == ui::Evento::TECLA && e.tecla == KEY_ENTER && !ip_.ativo && !porta_.ativo)) conectar();
}

void CenaBatalha::conectar() {
    int porta;
    ip_.erro = porta_.erro = false;
    try {
        porta = std::stoi(porta_.valor);
    } catch (const std::exception&) {
        erro_ = "Porta inválida.";
        porta_.erro = true;
        return;
    }
    std::shared_ptr<Servidor> servidor;
    std::string host = ip_.valor.empty() ? P::HOST_PADRAO : ip_.valor;
    if (hospedar_) {
        try {
            // escuta em todas as interfaces: o oponente vem pela rede local
            servidor = std::make_shared<Servidor>("0.0.0.0", porta);
            servidor->iniciar();
        } catch (const std::exception& ex) {
            erro_ = "Não foi possível abrir a porta " + std::to_string(porta) + ": " + ex.what();
            porta_.erro = true;
            return;
        }
        host = "127.0.0.1";
    }
    auto con = std::make_shared<Conexao>();
    try {
        con->conectar(host, porta, herois_.escolhido, mapa_);
    } catch (const std::exception& ex) {
        erro_ = "Falha ao conectar em " + host + ":" + std::to_string(porta) + " (" + ex.what() + ")";
        ip_.erro = !hospedar_;
        if (servidor) servidor->parar();
        return;
    }
    app.trocar(std::make_unique<CenaLobby>(app, con, servidor, porta));
}

void CenaBatalha::desenhar() {
    ui::fundo_gradiente(rgb(58, 120, 200), rgb(29, 63, 116));
    if (hospedar_) {
        cabecalho("Hospedar batalha", "Você será o servidor da partida");
        painel_passo({24, 80, 1232, 236}, "1 · Mapa");
        desenhar_mapas(40, 1200, mapa_, true);
        painel_passo({24, 332, 1232, 104}, "2 · Porta");
        porta_.desenhar();
        ui::texto("Porta TCP que ficará aberta. Libere no firewall se o oponente estiver em outro PC.", 214, 398, 14,
                  ui::BEGE, 0, Ancora::MIDLEFT, ui::Peso::TEXTO);
        painel_passo({24, 452, 1232, 176}, "3 · Herói");
        const DefTorre& h = herois_.desenhar();
        info_heroi_card(h, {634, 468, 606, 144});
        if (!erro_.empty()) ui::placa_erro(erro_, 700, 380, 540);
    } else {
        cabecalho("Entrar em uma batalha", "Conecte-se a quem hospeda");
        painel_passo({24, 80, 1232, 236}, "1 · Conexão");
        ip_.desenhar();
        porta_.desenhar();
        float y = 142;
        if (!erro_.empty()) y += ui::placa_erro(erro_, 606, 142, 634) + 10;
        ui::texto("O mapa é escolhido por quem hospeda.", 606, y + 10, 14, ui::BEGE, 0, Ancora::MIDLEFT, ui::Peso::TEXTO);
        ui::texto("Digite o IP e a porta e aperte Enter (ou clique em CONECTAR).", 40, 250, 13, ui::BEGE, 0, Ancora::MIDLEFT,
                  ui::Peso::TEXTO);
        painel_passo({24, 332, 1232, 290}, "2 · Herói");
        const DefTorre& h = herois_.desenhar();
        info_heroi_card(h, {634, 376, 606, 144});
    }
    voltar_.desenhar();
    ir_.desenhar();
}

// ====================================================================== SALA DE ESPERA
CenaLobby::CenaLobby(App& a, std::shared_ptr<Conexao> con, std::shared_ptr<Servidor> servidor, int porta)
    : Cena(a), con_(std::move(con)), servidor_(std::move(servidor)), porta_(porta) {
    if (servidor_) ips_ = rede::ips_locais();
    cancelar_ = {{520, 590, 240, 60}, "Cancelar", ui::VERMELHO, 20};
    cancelar_.atalho = "Esc";
}

void CenaLobby::evento(const ui::Evento& e) {
    if (cancelar_.clicou(e) || (e.tipo == ui::Evento::TECLA && e.tecla == KEY_ESCAPE)) app.ir_menu();  // sair() fecha a conexao
}

void CenaLobby::encerrar() {
    con_->fechar();
    if (servidor_) servidor_->parar();
}

void CenaLobby::atualizar(double dt) {
    t_ += dt;
    if (iniciou_) return;
    auto msgs = con_->pegar_mensagens();
    for (size_t i = 0; i < msgs.size(); ++i) {
        const P::Mensagem& msg = msgs[i];
        if (msg.comando == P::ENTRAR) {
            status_ = "Você é o jogador " + std::to_string(msg.jogador) + ". Aguardando oponente";
        } else if (msg.comando == P::ERRO && msg.codigo == P::ERRO_SALA_CHEIA) {
            status_ = "Sala cheia!";
        } else if (msg.comando == P::INICIO) {
            std::vector<P::Mensagem> pendentes(msgs.begin() + static_cast<long>(i) + 1, msgs.end());
            auto ctl = std::make_unique<ControladorBatalha>(
                con_, con_->numero(), msg.seed, msg.mapa,
                std::map<int, std::string>{{1, msg.herois[1]}, {2, msg.herois[2]}}, servidor_, std::move(pendentes),
                porta_);
            iniciou_ = true;
            som::tocar("rodada");
            app.iniciar_jogo(std::move(ctl));
            return;
        }
    }
    if (!con_->ativo()) status_ = "Conexão encerrada.";
}

void CenaLobby::desenhar() {
    app.fundo.desenhar(0.016);
    titulo(78, 0.67f);
    const Rectangle r{290, 236, 700, servidor_ ? 330.0f : 150.0f};
    ui::painel_menu(r, 24);
    // status com os pontos em largura fixa (o texto centralizado nao "treme" mais)
    const bool aguardando = status_.rfind("Você é o jogador", 0) == 0;
    const std::string txt = aguardando ? "Aguardando oponente" : status_;
    const float wt = ui::medir(txt, 21).x;
    const float wj = aguardando ? ui::medir("JOGADOR 1", 12).x + 24 : 0;
    const float total = wj + (aguardando ? 12 : 0) + wt + 12 + 48;
    float x = r.x + r.width / 2 - total / 2;
    const float cy = r.y + 44;
    if (aguardando) {
        const Rectangle pj{x, cy - 15, wj, 30};
        ui::ret(pj, ui::TINTA, 15);
        ui::ret(ui::inflar(pj, -6, -6), ui::AZUL, 12);
        ui::texto("JOGADOR " + std::to_string(con_->numero()), pj.x + pj.width / 2, cy, 12, ui::BRANCO, 0, Ancora::CENTER);
        x += wj + 12;
    }
    ui::texto(txt, x, cy, 21, ui::BRANCO, 5, Ancora::MIDLEFT);
    x += wt + 12;
    for (int i = 0; i < 3; ++i) {
        const bool aceso = i < (static_cast<int>(t_ * 2) % 4);
        DrawCircleV({x + 6 + i * 16.0f, cy + 4}, 7, ui::TINTA);
        DrawCircleV({x + 6 + i * 16.0f, cy + 4}, 5, aceso ? ui::BRANCO : rgb(255, 255, 255, 90));
    }
    if (servidor_) {
        ui::texto("Seu oponente deve usar \"Batalha: Entrar\" com:", r.x + r.width / 2, r.y + 90, 17, ui::BEGE, 0,
                  Ancora::CENTER, ui::Peso::TEXTO);
        // cartao com IP e porta: o que precisa ser ditado ao amigo
        const Rectangle c{r.x + 36, r.y + 116, r.width - 72, 112};
        ui::ret(ui::inflar(c, 6, 6), ui::TINTA, 18);
        ui::ret(c, ui::PERGAMINHO, 14);
        for (float k = c.x + 12; k < c.x + c.width - 12; k += 14) {
            DrawRectangleRec({k, c.y + 5, 8, 3}, ui::MADEIRA_ESCURA);
            DrawRectangleRec({k, c.y + c.height - 8, 8, 3}, ui::MADEIRA_ESCURA);
        }
        const std::string ip = ips_.empty() ? "127.0.0.1" : ips_.front();
        const float mx = c.x + c.width * 0.62f;
        ui::texto("IP", (c.x + mx) / 2, c.y + 26, 13, rgb(110, 69, 35), 0, Ancora::CENTER, ui::Peso::TEXTO);
        ui::texto(ip, (c.x + mx) / 2, c.y + 66, 38, ui::TINTA, 0, Ancora::CENTER);
        DrawRectangleRec({mx, c.y + 24, 3, 64}, rgb(217, 191, 134));
        ui::texto("PORTA", (mx + c.x + c.width) / 2, c.y + 26, 13, rgb(110, 69, 35), 0, Ancora::CENTER, ui::Peso::TEXTO);
        ui::texto(std::to_string(porta_), (mx + c.x + c.width) / 2, c.y + 66, 38, ui::TINTA, 0, Ancora::CENTER);
        std::string outros;
        for (size_t i = 1; i < ips_.size(); ++i) outros += (outros.empty() ? "" : ", ") + ips_[i];
        const std::string rodape = (outros.empty() ? "" : "Outros IPs desta máquina: " + outros + "  ·  ") +
                                   std::string("No mesmo computador, use 127.0.0.1");
        ui::texto(rodape, r.x + r.width / 2, r.y + 262, 14, ui::BEGE, 0, Ancora::CENTER, ui::Peso::TEXTO);
    } else {
        ui::texto("Conectado. A partida começa quando os dois jogadores entrarem.", r.x + r.width / 2, r.y + 96, 15, ui::BEGE,
                  0, Ancora::CENTER, ui::Peso::TEXTO);
    }
    cancelar_.desenhar();
}

void CenaLobby::sair() {
    if (!iniciou_) encerrar();
}

}  // namespace bl
