// Aplicativo: janela, laco principal e troca de cenas.
#pragma once

#include <memory>
#include <random>

#include "cliente/controle.hpp"
#include "cliente/ui.hpp"

namespace bl {

class App;

class Cena {
public:
    explicit Cena(App& app) : app(app) {}
    virtual ~Cena() = default;
    virtual void evento(const ui::Evento& e) = 0;
    virtual void atualizar(double dt) = 0;
    virtual void desenhar() = 0;
    virtual void sair() {}

protected:
    App& app;
};

// Bloons subindo pelo ceu no fundo dos menus.
class FundoBloons {
public:
    FundoBloons();
    void desenhar(double dt);

private:
    struct B {
        float x, y;
        const TipoBloon* tipo;
        float vel, fase;
    };
    std::vector<B> bloons_;
    std::mt19937 rng_{5};
};

class App {
public:
    App();
    ~App();
    void rodar();

    // As trocas acontecem no fim do quadro: a cena atual pode pedir a troca de dentro dela mesma.
    void trocar(std::unique_ptr<Cena> cena);
    void ir_menu();
    void iniciar_jogo(std::unique_ptr<Controlador> controle);
    void sair() { rodando_ = false; }

    FundoBloons fundo;
    std::mt19937 rng{std::random_device{}()};

private:
    std::unique_ptr<Cena> cena_, proxima_;
    bool rodando_ = true;
    bool mostrar_fps_ = false;
};

}  // namespace bl
