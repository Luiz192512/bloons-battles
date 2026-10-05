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

// Consulta dos 15 upgrades de cada torre, fora da partida
class CenaMacacos : public Cena {
public:
    explicit CenaMacacos(App& app);
    void evento(const ui::Evento& e) override;
    void atualizar(double) override {}
    void desenhar() override;

private:
    Rectangle rect_torre(size_t i) const { return {40 + (i % 4) * 66.0f, 100 + (i / 4) * 66.0f, 60, 60}; }
    Rectangle rect_up(int p, int k) const { return {348 + k * 180.0f, 226 + p * 132.0f, 172, 124}; }
    std::string torre_ = "dardo";
    ui::Botao voltar_;
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
    Rectangle rect_mapa(size_t i) const { return {40 + i * 304.5f, 124, 286.5f, 172}; }
    // 4 dificuldades + 3 modos do BTD6 (CHIMPS, Meio Dinheiro, Deflacao) numa linha so
    Rectangle rect_dif(size_t i) const {
        const float w = (1200.0f - (DIFICULDADES.size() - 1) * 8.0f) / DIFICULDADES.size();
        return {40 + i * (w + 8.0f), 368, w, 56};
    }
    // modos de restricao do BTD6, no cabecalho do passo 2 para nao apertar a linha das dificuldades
    Rectangle rect_restricao(size_t i) const { return {688 + i * 138.0f, 338, 130, 24}; }
    std::string mapa_ = "prado", dif_ = "medio", restricao_;
    GradeHerois herois_{40, 496, 9, 58};
    ui::Botao voltar_, jogar_;
};

class CenaBatalha : public Cena {
public:
    CenaBatalha(App& app, bool hospedar);
    void evento(const ui::Evento& e) override;
    void atualizar(double) override {}
    void desenhar() override;

    void conectar();
    void ip_teste(const std::string& ip, const std::string& porta) { ip_.valor = ip, porta_.valor = porta; }

private:
    Rectangle rect_mapa(size_t i) const { return {40 + i * 304.5f, 124, 286.5f, 172}; }
    bool hospedar_;
    std::string mapa_ = "prado";
    GradeHerois herois_{40, 496, 9, 58};
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

// Logo: "BLOONS TD" com a faixa vermelha "BATTLES" (escala 1 = menu principal).
void titulo(float y = 110, float escala = 1);
// Painel de passo numerado ("1 · Mapa") nas telas de escolha.
Rectangle painel_passo(Rectangle r, const std::string& rotulo);
// Card grande do heroi escolhido (retrato animado, nome, preco e habilidades).
void info_heroi_card(const DefTorre& h, Rectangle r);

}  // namespace bl
