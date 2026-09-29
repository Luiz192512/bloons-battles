// Menus: principal, escolha do modo solo, escolha da batalha e sala de espera.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "cliente/app.hpp"
#include "cliente/conexao.hpp"
#include "servidor/servidor.hpp"

namespace bl {

class CenaMenu : public Cena {
public:
    explicit CenaMenu(App& app);
    void evento(const ui::Evento& e) override;
    void atualizar(double dt) override { dt_ = dt; }
    void desenhar() override;

private:
    std::vector<ui::Botao> botoes_;
    double dt_ = 0.016;
};

// Grade de selecao de heroi (comum ao solo e a batalha)
class GradeHerois {
public:
    GradeHerois(float x, float y, int colunas, float tam) : x_(x), y_(y), colunas_(colunas), tam_(tam) {}
    void evento(const ui::Evento& e);
    const DefTorre& desenhar() const;
    std::string escolhido = "quincy";

private:
    Rectangle rect(size_t i) const;
    float x_, y_;
    int colunas_;
    float tam_;
};

class CenaSolo : public Cena {
public:
    explicit CenaSolo(App& app);
    void evento(const ui::Evento& e) override;
    void atualizar(double) override {}
    void desenhar() override;

private:
    Rectangle rect_mapa(size_t i) const { return {60 + i * 250.0f, 90, 230, 150}; }
    Rectangle rect_dif(size_t i) const { return {60 + i * 250.0f, 312, 230, 50}; }
    std::string mapa_ = "prado", dif_ = "medio";
    GradeHerois herois_{60, 410, 9, 62};
    ui::Botao voltar_, jogar_;
};

class CenaBatalha : public Cena {
public:
    CenaBatalha(App& app, bool hospedar);
    void evento(const ui::Evento& e) override;
    void atualizar(double) override {}
    void desenhar() override;

private:
    void conectar();
    Rectangle rect_mapa(size_t i) const { return {60 + i * 250.0f, 90, 230, 160}; }
    bool hospedar_;
    std::string mapa_ = "prado";
    GradeHerois herois_{60, 330, 9, 62};
    ui::CampoTexto ip_, porta_;
    ui::Botao voltar_, ir_;
    std::string erro_;
};

class CenaLobby : public Cena {
public:
    CenaLobby(App& app, std::shared_ptr<Conexao> con, std::shared_ptr<Servidor> servidor, int porta);
    void evento(const ui::Evento& e) override;
    void atualizar(double dt) override;
    void desenhar() override;
    void sair() override;

private:
    void encerrar();
    std::shared_ptr<Conexao> con_;
    std::shared_ptr<Servidor> servidor_;
    int porta_;
    std::vector<std::string> ips_;
    std::string status_ = "Conectando...";
    ui::Botao cancelar_;
    double t_ = 0;
    bool iniciou_ = false;
};

void titulo(float y = 110);

}  // namespace bl
