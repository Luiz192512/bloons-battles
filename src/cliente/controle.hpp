// Controladores: ligam a tela de jogo a uma partida solo (local) ou batalha (rede).
#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cliente/conexao.hpp"
#include "ipc/placar.hpp"
#include "jogo/sim.hpp"
#include "servidor/servidor.hpp"

namespace bl {

std::string mensagem_erro(char erro);

class Controlador {
public:
    virtual ~Controlador() = default;
    virtual bool online() const = 0;
    virtual char enviar(const std::string& cmd) = 0;  // OK ou codigo de erro
    virtual void botao_play() {}
    virtual void atualizar(double dt) = 0;
    virtual bool terminou() const = 0;
    virtual int vencedor() const = 0;
    virtual std::vector<std::string> log() { return {}; }
    virtual void desistir() {}
    virtual void fechar() {}
    virtual long tick_rede() const { return 0; }
    virtual bool dessincronizado() const { return false; }

    Pista& pista() { return partida->pista(meu); }
    Pista& oponente() { return partida->pista(meu == 1 ? 2 : 1); }

    std::unique_ptr<Partida> partida;
    int meu = 1;
    int velocidade = 1;
    bool pausado = false;
    std::string status;
};

class ControladorSolo : public Controlador {
public:
    ControladorSolo(const std::string& mapa, const std::string& dificuldade, const std::string& heroi, int seed,
                    const std::string& restricao = "");
    bool online() const override { return false; }
    std::unique_ptr<ControladorSolo> reiniciar() const;
    char enviar(const std::string& cmd) override { return partida->aplicar(1, cmd); }
    void botao_play() override;
    void atualizar(double dt) override;
    bool terminou() const override { return partida->fim; }
    int vencedor() const override { return partida->vencedor; }

private:
    std::string mapa_, dificuldade_, heroi_, restricao_;
    int seed_;
    double acc_ = 0;
};

// Lockstep: os comandos so sao aplicados quando voltam do servidor dentro de um tick.
class ControladorBatalha : public Controlador {
public:
    static constexpr int PASSOS_POR_TICK = 2;  // servidor a 15 ticks/s x 2 = 30 passos/s
    static constexpr int HASH_A_CADA = 45;     // ticks (3 s)

    ControladorBatalha(std::shared_ptr<Conexao> conexao, int numero, long long seed, const std::string& mapa,
                       const std::map<int, std::string>& herois, std::shared_ptr<Servidor> servidor,
                       std::vector<proto::Mensagem> pendentes, int porta = proto::PORTA_PADRAO);
    bool online() const override { return true; }
    // Validacao local antecipada, so para dar retorno imediato ao jogador.
    char checar(const std::string& cmd);
    char enviar(const std::string& cmd) override;
    void atualizar(double dt) override;
    bool terminou() const override { return partida->fim || fim_remoto >= 0; }
    int vencedor() const override { return partida->fim ? partida->vencedor : fim_remoto; }
    std::vector<std::string> log() override { return conexao_->log(); }
    void desistir() override;
    void fechar() override;
    long tick_rede() const override { return tick_; }
    bool dessincronizado() const override { return dessinc_; }

private:
    std::shared_ptr<Conexao> conexao_;
    std::shared_ptr<Servidor> servidor_;
    std::vector<proto::Mensagem> pendentes_;  // mensagens que chegaram junto com o inicio
    // Linha deste jogador no placar em memoria compartilhada (so existe se o servidor da sala
    // roda nesta mesma maquina).
    void publicar_placar();
    std::unique_ptr<ipc::PlacarCompartilhado> placar_;
    bool dessinc_ = false;
    int fim_remoto = -1;
    long tick_ = 0;
};

}  // namespace bl
