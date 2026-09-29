#include "cliente/cenas_menu.hpp"

#include <cmath>

#include "cliente/arte.hpp"
#include "cliente/som.hpp"

namespace bl {

using ui::Ancora;
using ui::rgb;
namespace P = proto;

void titulo(float y) {
    ui::texto("BLOONS TD", ui::LARGURA / 2.0f, y, 84, ui::AMARELO, 6, Ancora::CENTER, ui::Peso::LOGO);
    ui::texto("BATTLES", ui::LARGURA / 2.0f, y + 82, 64, rgb(255, 120, 60), 5, Ancora::CENTER, ui::Peso::LOGO);
}

// ====================================================================== MENU
CenaMenu::CenaMenu(App& a) : Cena(a) {
    const float cx = ui::LARGURA / 2.0f;
    botoes_ = {
        {{cx - 170, 300, 340, 64}, "Jogar Solo", ui::VERDE, 28},
        {{cx - 170, 380, 340, 64}, "Batalha: Hospedar", rgb(60, 150, 230), 26},
        {{cx - 170, 460, 340, 64}, "Batalha: Entrar", rgb(60, 150, 230), 26},
        {{cx - 170, 560, 340, 56}, "Sair", rgb(180, 70, 50), 24},
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
    titulo();
    for (auto& b : botoes_) b.desenhar();
    const char* chaves[] = {"dardo", "super", "ninja", "mago"};
    for (int i = 0; i < 4; ++i) {
        float x = i < 2 ? 150.0f : ui::LARGURA - 150.0f;
        float y = 330.0f + (i % 2) * 150;
        arte::torre(chaves[i], x, y, 110);
    }
    ui::texto("Trabalho 02 de Sistemas Operacionais: comunicação entre processos", ui::LARGURA / 2.0f,
              ui::ALTURA - 30, 16, ui::BRANCO, 2, Ancora::CENTER);
}

// ====================================================================== SELECAO DE HEROI
Rectangle GradeHerois::rect(size_t i) const {
    const float col = static_cast<float>(i % colunas_), lin = static_cast<float>(i / colunas_);
    return {x_ + col * (tam_ + 8), y_ + lin * (tam_ + 8), tam_, tam_};
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
    for (size_t i = 0; i < herois().size(); ++i) {
        const Rectangle r = rect(i);
        const bool sel = herois()[i].chave == escolhido;
        ui::ret(ui::mover(r, 0, 3), rgb(60, 40, 20), 10);
        ui::ret(r, sel ? rgb(255, 226, 120) : rgb(236, 214, 150), 10);
        ui::ret_linha(r, sel ? ui::BRANCO : rgb(90, 60, 30), sel ? 3.0f : 2.0f, 10);
        arte::torre(herois()[i].chave, r.x + r.width / 2, r.y + r.height / 2, static_cast<int>(tam_ - 8));
    }
    return *achar_heroi(escolhido);
}

namespace {

void info_heroi(const DefTorre& h, float x, float y) {
    ui::texto(h.nome, x, y, 28, ui::AMARELO);
    ui::texto(h.titulo, x, y + 38, 16);
    ui::texto("Custo: $" + std::to_string(h.custo), x, y + 64, 16, ui::DINHEIRO);
    if (!h.hab3.is_null()) ui::texto("Nível 3: " + h.hab3["nome"].get<std::string>(), x, y + 92, 15);
    if (!h.hab10.is_null()) ui::texto("Nível 10: " + h.hab10["nome"].get<std::string>(), x, y + 116, 15);
}

void desenhar_mapas(const std::string& selecionado, float h, bool detalhes, Color moldura) {
    for (size_t i = 0; i < mapas().size(); ++i) {
        const DefMapa& m = mapas()[i];
        Rectangle r{60 + i * 250.0f, 90, 230, h};
        ui::ret(ui::inflar(r, 10, 10), m.chave == selecionado ? rgb(255, 230, 90) : moldura, 10);
        arte::fundo_mapa(m.chave, r);
        ui::texto(m.nome, r.x + r.width / 2, r.y + r.height + 16, 16, ui::BRANCO, 2, Ancora::CENTER);
        if (detalhes) ui::texto(m.dificuldade, r.x + r.width / 2, r.y + r.height - 14, 13, ui::AMARELO, 2, Ancora::CENTER);
    }
}

}  // namespace

// ====================================================================== SOLO
CenaSolo::CenaSolo(App& a) : Cena(a) {
    voltar_ = {{30, 640, 170, 54}, "Voltar", rgb(150, 90, 50), 22};
    jogar_ = {{ui::LARGURA - 260.0f, 630, 230, 70}, "JOGAR!", ui::VERDE, 32};
}

void CenaSolo::evento(const ui::Evento& e) {
    herois_.evento(e);
    if (e.tipo == ui::Evento::CLIQUE) {
        for (size_t i = 0; i < mapas().size(); ++i)
            if (ui::dentro(rect_mapa(i), e.pos)) mapa_ = mapas()[i].chave;
        for (size_t i = 0; i < DIFICULDADES.size(); ++i)
            if (ui::dentro(rect_dif(i), e.pos)) dif_ = DIFICULDADES[i].chave;
    }
    if (voltar_.clicou(e)) app.ir_menu();
    if (jogar_.clicou(e) || (e.tipo == ui::Evento::TECLA && e.tecla == KEY_ENTER)) {
        som::tocar("rodada");
        int seed = std::uniform_int_distribution<int>(1, 999999)(app.rng);
        app.iniciar_jogo(std::make_unique<ControladorSolo>(mapa_, dif_, herois_.escolhido, seed));
    }
}

void CenaSolo::desenhar() {
    ui::fundo_gradiente(rgb(70, 140, 60), rgb(40, 90, 40));
    ui::texto("Escolha o mapa", 60, 50, 26);
    desenhar_mapas(mapa_, 150, true, rgb(60, 40, 20));
    ui::texto("Dificuldade", 60, 280, 20);
    for (size_t i = 0; i < DIFICULDADES.size(); ++i) {
        const Dificuldade& d = DIFICULDADES[i];
        ui::Botao b{rect_dif(i), d.nome + " (" + std::to_string(d.ultima_rodada) + ")",
                    d.chave == dif_ ? ui::VERDE : rgb(120, 110, 90), 18};
        b.desenhar();
    }
    ui::texto("Escolha o herói", 60, 378, 20);
    info_heroi(herois_.desenhar(), 740, 470);
    voltar_.desenhar();
    jogar_.desenhar();
}

// ====================================================================== BATALHA
CenaBatalha::CenaBatalha(App& a, bool hospedar) : Cena(a), hospedar_(hospedar) {
    ip_ = {{60, 120, 300, 48}, P::HOST_PADRAO, "IP de quem hospeda"};
    porta_ = {{390, 120, 140, 48}, std::to_string(P::PORTA_PADRAO), "Porta"};
    porta_.max_len = 5;
    if (hospedar_) porta_.rect = {60, 520, 140, 48};
    voltar_ = {{30, 640, 170, 54}, "Voltar", rgb(150, 90, 50), 22};
    ir_ = {{ui::LARGURA - 320.0f, 630, 290, 70}, hospedar ? "HOSPEDAR" : "CONECTAR", ui::VERDE, 30};
}

void CenaBatalha::evento(const ui::Evento& e) {
    herois_.evento(e);
    if (!hospedar_) ip_.evento(e);
    porta_.evento(e);
    if (hospedar_ && e.tipo == ui::Evento::CLIQUE)
        for (size_t i = 0; i < mapas().size(); ++i)
            if (ui::dentro(rect_mapa(i), e.pos)) mapa_ = mapas()[i].chave;
    if (voltar_.clicou(e)) app.ir_menu();
    if (ir_.clicou(e)) conectar();
}

void CenaBatalha::conectar() {
    int porta;
    try {
        porta = std::stoi(porta_.valor);
    } catch (const std::exception&) {
        erro_ = "Porta inválida.";
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
            return;
        }
        host = "127.0.0.1";
    }
    auto con = std::make_shared<Conexao>();
    try {
        con->conectar(host, porta, herois_.escolhido, mapa_);
    } catch (const std::exception& ex) {
        erro_ = "Falha ao conectar em " + host + ":" + std::to_string(porta) + " (" + ex.what() + ")";
        if (servidor) servidor->parar();
        return;
    }
    app.trocar(std::make_unique<CenaLobby>(app, con, servidor, porta));
}

void CenaBatalha::desenhar() {
    ui::fundo_gradiente(rgb(50, 100, 170), rgb(30, 60, 110));
    if (hospedar_) {
        ui::texto("Hospedar batalha: escolha o mapa", 60, 50, 26);
        desenhar_mapas(mapa_, 160, false, rgb(30, 30, 50));
        porta_.desenhar();
    } else {
        ui::texto("Entrar em uma batalha", 60, 40, 26);
        ip_.desenhar();
        porta_.desenhar();
        ui::texto("O mapa é escolhido por quem hospeda.", 60, 190, 16);
    }
    ui::texto("Escolha o herói", 60, 296, 20);
    info_heroi(herois_.desenhar(), 740, 410);
    if (!erro_.empty()) ui::texto(erro_, ui::LARGURA / 2.0f, 600, 18, rgb(255, 120, 100), 2, Ancora::CENTER);
    voltar_.desenhar();
    ir_.desenhar();
}

// ====================================================================== SALA DE ESPERA
CenaLobby::CenaLobby(App& a, std::shared_ptr<Conexao> con, std::shared_ptr<Servidor> servidor, int porta)
    : Cena(a), con_(std::move(con)), servidor_(std::move(servidor)), porta_(porta) {
    if (servidor_) ips_ = rede::ips_locais();
    cancelar_ = {{ui::LARGURA / 2.0f - 120, 600, 240, 60}, "Cancelar", rgb(180, 70, 50), 24};
}

void CenaLobby::evento(const ui::Evento& e) {
    if (cancelar_.clicou(e)) app.ir_menu();  // sair() fecha a conexao
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
                std::map<int, std::string>{{1, msg.herois[1]}, {2, msg.herois[2]}}, servidor_, std::move(pendentes));
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
    titulo(90);
    const std::string pontos(static_cast<size_t>(static_cast<int>(t_ * 2) % 4), '.');
    ui::texto(status_ + pontos, ui::LARGURA / 2.0f, 330, 28, ui::BRANCO, 2, Ancora::CENTER);
    if (servidor_) {
        ui::texto("Seu oponente deve usar \"Batalha: Entrar\" com o IP:", ui::LARGURA / 2.0f, 400, 20, ui::BRANCO, 2,
                  Ancora::CENTER);
        std::string ips;
        for (auto& ip : ips_) ips += (ips.empty() ? "" : ", ") + ip;
        if (ips.empty()) ips = "127.0.0.1";
        ui::texto(ips + "   porta " + std::to_string(porta_), ui::LARGURA / 2.0f, 440, 28, ui::AMARELO, 2,
                  Ancora::CENTER);
        ui::texto("(no mesmo computador, use 127.0.0.1)", ui::LARGURA / 2.0f, 480, 16, ui::BRANCO, 2, Ancora::CENTER);
    }
    cancelar_.desenhar();
}

void CenaLobby::sair() {
    if (!iniciou_) encerrar();
}

}  // namespace bl
