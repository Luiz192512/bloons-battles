#include "cliente/app.hpp"

#include <cmath>
#include <cstdio>
#include <ctime>

#include "cliente/arte.hpp"
#include "cliente/cena_jogo.hpp"
#include "cliente/cenas_menu.hpp"
#include "cliente/som.hpp"
#include "cliente/vitrine.hpp"
#include "rlgl.h"

namespace bl {

// ---------------------------------------------------------------- fundo dos menus
FundoBloons::FundoBloons() {
    const char* tipos[] = {"vermelho", "azul", "verde", "amarelo", "rosa", "preto",
                           "branco", "zebra", "arco_iris", "ceramica", "roxo", "chumbo"};
    std::uniform_real_distribution<float> ux(0, ui::LARGURA), uy(0, ui::ALTURA), uv(26, 60), uf(0, 6), ut(56, 96);
    for (int i = 0; i < 16; ++i) bloons_.push_back({ux(rng_), uy(rng_), tipos[rng_() % 12], uv(rng_), uf(rng_), ut(rng_)});
}

void FundoBloons::desenhar(double dt) {
    ui::fundo_gradiente(ui::rgb(91, 180, 240), ui::rgb(174, 224, 255));
    std::uniform_real_distribution<float> ux(0, ui::LARGURA);
    const double t = ui::tempo();
    // um M.O.A.B. passando la no alto
    const float mx = static_cast<float>(std::fmod(t * 22, ui::LARGURA + 400.0)) - 200;
    arte::dirigivel_vivo("moab", 0, false, mx, 70, 150, t);
    for (auto& b : bloons_) {
        b.y -= b.vel * static_cast<float>(dt);
        b.fase += static_cast<float>(dt);
        if (b.y < -80) {
            b.y = ui::ALTURA + 80;
            b.x = ux(rng_);
        }
        arte::bloon_vivo(b.tipo, false, false, false, b.x + std::sin(b.fase) * 12, b.y, b.tam, t + b.fase);
    }
    // grama no rodape, com contorno de tinta
    DrawRectangle(0, ui::ALTURA - 84, ui::LARGURA, 84, ui::VERDE);
    DrawRectangle(0, ui::ALTURA - 84, ui::LARGURA, 4, ui::TINTA);
    DrawRectangle(0, ui::ALTURA - 80, ui::LARGURA, 6, ui::rgb(155, 230, 110));
    DrawRectangle(0, ui::ALTURA - 18, ui::LARGURA, 18, ui::VERDE_ESCURO);
}

// ---------------------------------------------------------------- app
App::App(const std::vector<std::string>& args) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(ui::LARGURA, ui::ALTURA, "Bloons TD Battles - SO 2026");
    SetExitKey(KEY_NULL);  // Esc e do jogo (pausa), nao fecha a janela
    SetTargetFPS(60);
    rlDisableBackfaceCulling();  // os poligonos da arte nao tem ordem de vertices fixa
    ui::iniciar_fontes();
    som::iniciar();
    cena_ = std::make_unique<CenaMenu>(*this);
    for (size_t i = 0; i < args.size(); ++i) {
        auto num = [&](size_t k, double padrao) {
            try {
                return k < args.size() ? std::stod(args[k]) : padrao;
            } catch (const std::exception&) {
                return padrao;
            }
        };
        if (args[i] == "--vitrine") {
            cena_ = criar_vitrine(*this, static_cast<int>(num(i + 1, 0)));
        } else if (args[i] == "--tela" && i + 1 < args.size()) {
            // telas de menu para conferir o visual: solo, hospedar, entrar, entrar-erro, lobby
            const std::string t = args[i + 1];
            if (t == "solo") cena_ = std::make_unique<CenaSolo>(*this);
            else if (t == "hospedar" || t == "lobby") cena_ = std::make_unique<CenaBatalha>(*this, true);
            else if (t.rfind("entrar", 0) == 0) cena_ = std::make_unique<CenaBatalha>(*this, false);
            if (auto* b = dynamic_cast<CenaBatalha*>(cena_.get())) {
                if (t == "entrar-erro") {
                    b->ip_teste("127.0.0.1", "1");
                    b->conectar();
                }
                if (t == "lobby") b->conectar();
            }
            if (proxima_) cena_ = std::move(proxima_);
        } else if (args[i] == "--fps") {
            mostrar_fps_ = true;
        } else if (args[i] == "--pausa") {
            if (auto* j = dynamic_cast<CenaJogo*>(cena_.get())) j->abrir_pausa();
        } else if (args[i] == "--demo") {
            const bool batalha = i + 1 < args.size() && args[i + 1] == "batalha";
            cena_ = criar_demo(*this, batalha);
        } else if (args[i] == "--captura" && i + 1 < args.size()) {
            captura_ = args[i + 1];
            captura_t_ = num(i + 2, 3);
            captura_n_ = static_cast<int>(num(i + 3, 1));
            captura_int_ = num(i + 4, 0.1);
        }
    }
}

App::~App() {
    if (cena_) cena_->sair();
    cena_.reset();
    proxima_.reset();
    arte::liberar();
    ui::liberar_fontes();
    som::liberar();
    CloseWindow();
}

void App::trocar(std::unique_ptr<Cena> cena) { proxima_ = std::move(cena); }

void App::ir_menu() { trocar(std::make_unique<CenaMenu>(*this)); }

void App::iniciar_jogo(std::unique_ptr<Controlador> controle) {
    trocar(std::make_unique<CenaJogo>(*this, std::move(controle)));
}

void App::rodar() {
    while (rodando_ && !WindowShouldClose()) {
        const double dt = GetFrameTime();
        ui::comecar_quadro();
        // eventos do quadro, na ordem: teclas, caracteres, cliques
        std::vector<ui::Evento> eventos;
        for (int k = GetKeyPressed(); k; k = GetKeyPressed()) {
            if (k == KEY_F3) mostrar_fps_ = !mostrar_fps_;
            else if (k == KEY_F12) captura_f12_ = true;
            else eventos.push_back({ui::Evento::TECLA, ui::mouse(), k, 0});
        }
        // Backspace segurado repete
        if (IsKeyPressedRepeat(KEY_BACKSPACE)) eventos.push_back({ui::Evento::TECLA, ui::mouse(), KEY_BACKSPACE, 0});
        for (int c = GetCharPressed(); c; c = GetCharPressed()) eventos.push_back({ui::Evento::CARACTERE, ui::mouse(), 0, c});
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) eventos.push_back({ui::Evento::CLIQUE, ui::mouse(), 0, 0});
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) eventos.push_back({ui::Evento::CLIQUE_DIR, ui::mouse(), 0, 0});

        for (auto& e : eventos) {
            if (proxima_) break;  // a cena ja pediu para sair
            cena_->evento(e);
        }
        if (!proxima_) cena_->atualizar(dt);
        cena_->desenhar();
        if (mostrar_fps_) ui::texto(std::to_string(GetFPS()) + " fps", 6, ui::ALTURA - 24, 14);
        relogio_ += dt;
        if (!captura_.empty() && relogio_ >= captura_t_ + capturadas_ * captura_int_) {
            std::string nome = captura_;
            if (captura_n_ > 1) {
                const size_t p = nome.rfind('.');
                char suf[16];
                std::snprintf(suf, sizeof suf, "_%02d", capturadas_);
                nome = nome.substr(0, p) + suf + (p == std::string::npos ? "" : nome.substr(p));
            }
            ui::capturar(nome);
            if (++capturadas_ >= captura_n_) rodando_ = false;
        }
        if (captura_f12_) {
            // F12: captura com data e hora no nome, na pasta de onde o jogo foi aberto
            const std::time_t agora = std::time(nullptr);
            std::tm tm{};
#ifdef _WIN32
            localtime_s(&tm, &agora);
#else
            localtime_r(&agora, &tm);
#endif
            char nome[64];
            std::strftime(nome, sizeof nome, "captura_%Y%m%d_%H%M%S.png", &tm);
            ui::capturar(nome);
            captura_f12_ = false;
        }
        ui::terminar_quadro();

        if (proxima_) {
            std::unique_ptr<Cena> antiga = std::move(cena_);
            cena_ = std::move(proxima_);
            antiga->sair();
        }
    }
}

}  // namespace bl
