#include "cliente/conexao.hpp"

#include <chrono>

namespace bl {

namespace P = proto;

Conexao::~Conexao() { fechar(); }

void Conexao::conectar(const std::string& host, int porta, const std::string& heroi, const std::string& mapa) {
    sock_ = rede::Socket::conectar(host, porta, 4);
    ativo_ = true;
    t_rx_ = std::thread(&Conexao::receber, this);
    t_hb_ = std::thread(&Conexao::heartbeat, this);
    enviar(P::entrar(heroi, mapa));
}

void Conexao::enviar(std::string linha) {
    if (!sock_.valido() || linha.size() < 2) return;
    // troca a origem provisoria pelo numero recebido do servidor
    if (numero_ && linha[0] != '0') linha[0] = static_cast<char>('0' + numero_);
    if (linha[1] != P::PING && linha[1] != P::HASH) {
        std::string t = linha;
        while (!t.empty() && (t.back() == '\n' || t.back() == '\r')) t.pop_back();
        registrar("> " + t);
    }
    bool ok;
    {
        std::lock_guard<std::mutex> trava(lock_envio_);
        ok = sock_.enviar_tudo(linha);
    }
    if (ok) bytes_env += static_cast<long long>(linha.size());
    else ativo_ = false;
}

void Conexao::comando(const std::string& cmd) { enviar(P::comando(numero_ ? numero_.load() : 1, cmd)); }

std::vector<P::Mensagem> Conexao::pegar_mensagens() {
    std::lock_guard<std::mutex> trava(lock_fila_);
    std::vector<P::Mensagem> msgs;
    msgs.swap(fila_);
    return msgs;
}

std::vector<std::string> Conexao::log() {
    std::lock_guard<std::mutex> trava(lock_log_);
    return log_;
}

void Conexao::fechar() {
    ativo_ = false;
    sock_.fechar();  // acorda a thread presa no recv
    acordar_hb_.notify_all();
    if (t_rx_.joinable()) t_rx_.join();
    if (t_hb_.joinable()) t_hb_.join();
}

void Conexao::registrar(std::string texto) {
    if (texto.size() > 60) texto = texto.substr(0, 57) + "...";
    std::lock_guard<std::mutex> trava(lock_log_);
    log_.push_back(std::move(texto));
    if (log_.size() > 40) log_.erase(log_.begin(), log_.end() - 40);
}

void Conexao::receber() {
    P::LeitorDeLinhas leitor;
    char buf[65536];
    while (ativo_) {
        int n = sock_.receber(buf, sizeof buf);
        if (n == -2) continue;
        if (n <= 0) break;
        bytes_rec += n;
        for (const std::string& linha : leitor.alimentar(buf, static_cast<size_t>(n))) {
            P::Mensagem msg;
            try {
                msg = P::interpretar(linha);
            } catch (const P::MensagemInvalida&) {
                continue;
            }
            if (msg.comando == P::PING) continue;
            if (msg.comando != P::TICK || !msg.comandos.empty()) registrar("< " + linha);
            if (msg.comando == P::ENTRAR) numero_ = msg.jogador;
            std::lock_guard<std::mutex> trava(lock_fila_);
            fila_.push_back(std::move(msg));
        }
    }
    ativo_ = false;
    acordar_hb_.notify_all();
}

void Conexao::heartbeat() {
    std::unique_lock<std::mutex> l(lock_espera_);
    while (ativo_) {
        l.unlock();
        enviar(P::ping(numero_ ? numero_.load() : 1));
        l.lock();
        acordar_hb_.wait_for(l, std::chrono::duration<double>(P::HEARTBEAT_INTERVALO), [&] { return !ativo_; });
    }
}

}  // namespace bl
