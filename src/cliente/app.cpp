#include "cliente/app.hpp"

#include <cmath>

#include "cliente/arte.hpp"
#include "cliente/cena_jogo.hpp"
#include "cliente/cenas_menu.hpp"
#include "cliente/som.hpp"
#include "rlgl.h"

namespace bl {

// ---------------------------------------------------------------- fundo dos menus
FundoBloons::FundoBloons() {
    const char* tipos[] = {"vermelho", "azul", "verde", "amarelo", "rosa", "preto",
                           "branco", "zebra", "arco_iris", "ceramica", "roxo", "chumbo"};
    std::uniform_real_distribution<float> ux(0, ui::LARGURA), uy(0, ui::ALTURA), uv(30, 80), uf(0, 6);
    for (int i = 0; i < 26; ++i)
        bloons_.push_back({ux(rng_), uy(rng_), &tipo_bloon(tipos[rng_() % 12]), uv(rng_), uf(rng_)});
}

void FundoBloons::desenhar(double dt) {
    ui::fundo_gradiente();
    std::uniform_real_distribution<float> ux(0, ui::LARGURA);
    for (auto& b : bloons_) {
        b.y -= b.vel * static_cast<float>(dt);
        b.fase += static_cast<float>(dt);
        if (b.y < -40) {
            b.y = ui::ALTURA + 40;
            b.x = ux(rng_);
        }
        const float x = b.x + std::sin(b.fase) * 12;
        const float r = static_cast<float>(b.tipo->raio);
        DrawLineEx({x, b.y + r + 3}, {x + std::sin(b.fase * 2) * 4, b.y + r + 26}, 1, ui::rgb(80, 80, 80));
        arte::bloon(*b.tipo, false, false, false, 0, x, b.y);
    }
    // grama no rodape
    DrawRectangle(0, ui::ALTURA - 60, ui::LARGURA, 60, ui::rgb(98, 170, 58));
    DrawRectangle(0, ui::ALTURA - 60, ui::LARGURA, 6, ui::rgb(70, 140, 40));
}

// ---------------------------------------------------------------- app
App::App() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(ui::LARGURA, ui::ALTURA, "Bloons TD Battles - SO 2026");
    SetExitKey(KEY_NULL);  // Esc e do jogo (pausa), nao fecha a janela
    SetTargetFPS(60);
    rlDisableBackfaceCulling();  // os poligonos da arte nao tem ordem de vertices fixa
    ui::iniciar_fontes();
    som::iniciar();
    cena_ = std::make_unique<CenaMenu>(*this);
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
        ui::terminar_quadro();

        if (proxima_) {
            std::unique_ptr<Cena> antiga = std::move(cena_);
            cena_ = std::move(proxima_);
            antiga->sair();
        }
    }
}

}  // namespace bl
