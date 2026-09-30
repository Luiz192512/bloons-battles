// Tela de jogo: mapa, loja de torres, painel de upgrades, habilidades e modo Batalha.
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "cliente/app.hpp"
#include "cliente/render.hpp"

namespace bl {

class CenaJogo : public Cena {
public:
    CenaJogo(App& app, std::unique_ptr<Controlador> controle);
    void evento(const ui::Evento& e) override;
    void atualizar(double dt) override;
    void desenhar() override;
    void sair() override;
    void selecionar(int torre_id) { selecionada_ = torre_id; }  // usado pela demo
    void abrir_pausa() { pausar(true); }

private:
    using Acao = std::function<void()>;
    struct Alvo {
        Rectangle r;
        Acao acao;
    };
    struct Dica {
        Vector2 pos;
        std::string titulo, desc;
        double preco = -1;      // upgrade: pilula de preco + atalho "para comprar"
        bool pode = true;
        std::string tecla;
        Rectangle ancora{};     // se tiver largura, a dica aparece ao lado deste retangulo
    };
    struct AvisoOp {
        bool recebendo;
        std::string txt;
        double t;
    };
    struct Hab {
        TorreP t;
        int idx;
        const J* h;
    };

    Pista& pista() { return ctl_->pista(); }
    void aviso(const std::string& txt, bool erro = true);
    bool comando(const std::string& cmd);
    std::vector<Hab> hab_lista();
    void tecla(int k);
    void escolher(const std::string& chave);
    void clique(Vector2 pos);
    void upar(int p);
    void vender();
    void pausar(bool v);
    void alternar_auto();
    void reiniciar();
    void desistir();
    void enviar(const Envio& env);

    // desenho
    void circulo_alcance(float x, float y, double r, bool valido);
    void previa(Vector2 mouse);
    void hud_topo();
    void painel_lateral(Vector2 mouse);
    void dica(const Dica& d, float largura = 250);
    void painel_upgrade(const TorreP& t, Vector2 mouse);
    void linha_upgrade(const TorreP& t, int pth, float x, float y, float w, Vector2 mouse);
    void painel_heroi(const Torre& t, float x, float y, float w);
    void cabecalho_upgrade(const Torre& t, float x, float y, float w);
    void rodape_upgrade(const Torre& t, float x, float y, float w, Vector2 mouse);
    void habilidades(Vector2 mouse);
    void painel_envios(Vector2 mouse);
    void mini_oponente();
    void oponente_grande();
    void painel_log();
    void mensagens();
    Rectangle sobreposicao(const std::string& titulo, Color cor);
    void tela_pausa();
    void tela_fim();

    std::unique_ptr<Controlador> ctl_;
    std::string chave_mapa_;
    RenderPista render_;
    std::unique_ptr<RenderPista> render_op_;
    int selecionada_ = 0;
    std::string colocando_;
    std::pair<std::string, double> msg_{"", 0}, banner_{"", 0};
    bool menu_pausa_ = false;
    bool mostrar_log_;
    bool mostrar_op_ = true;
    bool op_grande_ = false;
    std::vector<Dica> dicas_;
    std::string heroi_;
    std::vector<std::pair<std::string, Rectangle>> cards_;
    std::vector<Alvo> botoes_up_, botoes_hab_, botoes_envio_;
    std::vector<std::pair<ui::Botao, Acao>> botoes_menu_;
    Rectangle botao_play_;
    Rectangle rect_mini_;
    std::vector<AvisoOp> avisos_op_;
    // depuracao das animacoes (F9 disparo, Shift+F9 habilidade na torre selecionada)
    int depura_ = 0;
    double depura_t_ = 0;
    std::pair<double, double> eco_texto_{0, 0};
};

}  // namespace bl
