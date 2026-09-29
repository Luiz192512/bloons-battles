// Sockets TCP com a mesma interface no Windows (Winsock2) e no Linux (POSIX).
//
// O cabecalho nao inclui <winsock2.h> de proposito: ele conflita com a raylib
// (Rectangle, DrawText, CloseWindow...). So socket.cpp conhece a API do sistema.
#pragma once

#include <atomic>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace bl::rede {

struct ErroRede : std::runtime_error {
    using std::runtime_error::runtime_error;
};

class Socket {
public:
    Socket() = default;
    ~Socket();
    Socket(Socket&& o) noexcept;
    Socket& operator=(Socket&& o) noexcept;
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    // Servidor: bind + listen. porta 0 escolhe uma porta livre.
    static Socket ouvir(const std::string& host, int porta);
    // Cliente: connect com limite de tempo.
    static Socket conectar(const std::string& host, int porta, double timeout_s);

    // Bloqueia ate chegar uma conexao. Devolve socket invalido se o servidor foi fechado.
    Socket aceitar(std::string* endereco = nullptr);

    bool enviar_tudo(const std::string& dados);
    // > 0: bytes lidos; 0: conexao fechada; -1: erro; -2: tempo esgotado
    int receber(char* buf, int n);

    void timeout_recepcao(double segundos);
    void sem_atraso();  // TCP_NODELAY: manda as mensagens curtas na hora
    int porta_local() const;
    void desligar();    // shutdown: acorda threads presas em recv/accept
    void fechar();
    bool valido() const { return fd_.load() != INVALIDO; }

private:
    static constexpr std::intptr_t INVALIDO = -1;
    explicit Socket(std::intptr_t fd) : fd_(fd) {}
    std::atomic<std::intptr_t> fd_{INVALIDO};
};

// IPs desta maquina na rede local (para mostrar a quem vai se conectar).
std::vector<std::string> ips_locais();

}  // namespace bl::rede
