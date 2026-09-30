// Servidor do modo Batalha (lockstep).
//
// O servidor nao simula o jogo: ele ORDENA os comandos. A cada tick (15 por
// segundo) ele esvazia a fila compartilhada e transmite "0K<tick>|cmd|cmd".
// Os dois clientes aplicam os mesmos comandos, na mesma ordem, no mesmo tick,
// e como a simulacao e deterministica os estados ficam identicos.
//
// Threads:
//   aceitar  - espera conexoes (accept) e cria uma thread por cliente;
//   cliente  - le as linhas do socket e trata cada mensagem (produtora da fila);
//   relogio  - fecha um tick a cada 1/15 s e transmite (consumidora da fila).
#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "comum/protocolo.hpp"
#include "ipc/placar.hpp"
#include "rede/socket.hpp"
#include "servidor/sala.hpp"

namespace bl {

class Servidor {
public:
    static constexpr int TICKS_POR_SEGUNDO = 15;

    // Abre o socket (bind + listen). porta 0 escolhe uma livre. Lanca rede::ErroRede.
    explicit Servidor(const std::string& host = proto::HOST_PADRAO, int porta = proto::PORTA_PADRAO);
    ~Servidor();

    void iniciar();
    void parar();
    int porta() const { return porta_; }

    // Chamado a cada evento importante (opcional).
    std::function<void(const std::string&)> log;

    Sala sala;

private:
    struct Conexao {
        rede::Socket sock;
        std::mutex envio;  // um envio por vez por socket
    };
    using ConexaoP = std::shared_ptr<Conexao>;

    void aceitar();
    void relogio();
    void atender(ConexaoP con, std::string endereco);
    void tratar(int numero, const proto::Mensagem& msg);
    void enviar(int numero, const std::string& linha);
    void transmitir(const std::string& linha);
    void registrar(const std::string& texto);

    // Placar em memoria compartilhada (outros processos da maquina leem). Nunca e travado junto
    // com os mutexes da Sala: primeiro copia da Sala, depois trava o placar.
    void publicar(const std::function<void(ipc::Placar&)>& f);

    rede::Socket sock_;
    std::unique_ptr<ipc::PlacarCompartilhado> placar_;
    int porta_;
    std::atomic<bool> rodando_{false};

    std::mutex lock_clientes_;  // protege clientes_ e todas_
    std::map<int, ConexaoP> clientes_;
    std::vector<ConexaoP> todas_;

    std::mutex lock_threads_;   // protege a lista de threads
    std::thread t_aceitar_;
    std::thread t_relogio_;
    std::vector<std::thread> t_clientes_;
};

}  // namespace bl
