// Conexao do cliente: thread de recepcao + thread de heartbeat.
//
// A thread de rede (produtora) coloca mensagens na fila; a thread da tela
// (consumidora) retira a cada quadro. Os mutexes protegem essa fila e o log de
// mensagens, que sao memoria compartilhada entre as threads.
#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "comum/protocolo.hpp"
#include "rede/socket.hpp"

namespace bl {

class Conexao {
public:
    Conexao() = default;
    ~Conexao();

    // Lanca rede::ErroRede se nao conseguir conectar.
    void conectar(const std::string& host, int porta, const std::string& heroi, const std::string& mapa);
    bool ativo() const { return ativo_; }
    int numero() const { return numero_; }

    void enviar(std::string linha);
    void comando(const std::string& cmd);
    std::vector<proto::Mensagem> pegar_mensagens();
    std::vector<std::string> log();
    void fechar();

    std::atomic<long long> bytes_env{0}, bytes_rec{0};

private:
    void receber();
    void heartbeat();
    void registrar(std::string texto);

    rede::Socket sock_;
    std::vector<proto::Mensagem> fila_;
    std::mutex lock_fila_;
    std::vector<std::string> log_;
    std::mutex lock_log_;
    std::mutex lock_envio_;  // heartbeat e jogo podem enviar ao mesmo tempo
    std::atomic<bool> ativo_{false};
    std::atomic<int> numero_{0};
    std::thread t_rx_, t_hb_;
    std::mutex lock_espera_;
    std::condition_variable acordar_hb_;
};

}  // namespace bl
