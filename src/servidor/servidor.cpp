#include "servidor/servidor.hpp"

#include <chrono>
#include <random>

#include "jogo/defs.hpp"

namespace bl {

namespace P = proto;

Servidor::Servidor(const std::string& host, int porta) : sock_(rede::Socket::ouvir(host, porta)) {
    porta_ = sock_.porta_local();
    try {
        placar_ = ipc::PlacarCompartilhado::criar(porta_);
    } catch (const std::exception&) {
        placar_.reset();  // sem memoria compartilhada o jogo funciona igual, so nao ha placar externo
    }
}

void Servidor::publicar(const std::function<void(ipc::Placar&)>& f) {
    if (placar_) placar_->atualizar(f);
}

Servidor::~Servidor() { parar(); }

void Servidor::registrar(const std::string& texto) {
    if (log) log(texto);
}

// ---------- ciclo de vida

void Servidor::iniciar() {
    rodando_ = true;
    t_aceitar_ = std::thread(&Servidor::aceitar, this);
    registrar("ouvindo na porta " + std::to_string(porta_));
}

void Servidor::parar() {
    if (!rodando_.exchange(false) && !t_aceitar_.joinable()) return;
    sock_.fechar();  // acorda a thread presa no accept
    std::vector<ConexaoP> conexoes;
    {
        std::lock_guard<std::mutex> trava(lock_clientes_);
        conexoes = todas_;
    }
    for (auto& c : conexoes) c->sock.fechar();  // acorda as threads presas no recv (no Windows so fechar acorda)
    if (t_aceitar_.joinable()) t_aceitar_.join();
    std::vector<std::thread> threads;
    std::thread relogio;
    {
        std::lock_guard<std::mutex> trava(lock_threads_);
        threads.swap(t_clientes_);
        relogio.swap(t_relogio_);
    }
    for (auto& t : threads)
        if (t.joinable()) t.join();
    if (relogio.joinable()) relogio.join();
}

// ---------- envio

void Servidor::enviar(int numero, const std::string& linha) {
    ConexaoP c;
    {
        std::lock_guard<std::mutex> trava(lock_clientes_);
        auto it = clientes_.find(numero);
        if (it == clientes_.end()) return;
        c = it->second;
    }
    std::lock_guard<std::mutex> trava(c->envio);
    c->sock.enviar_tudo(linha);
}

void Servidor::transmitir(const std::string& linha) {
    std::vector<int> destinos;
    {
        std::lock_guard<std::mutex> trava(lock_clientes_);
        for (auto& [n, c] : clientes_) destinos.push_back(n);
    }
    for (int n : destinos) enviar(n, linha);
}

// ---------- threads

void Servidor::aceitar() {
    while (rodando_) {
        std::string endereco;
        rede::Socket s = sock_.aceitar(&endereco);
        if (!s.valido()) break;
        auto con = std::make_shared<Conexao>();
        con->sock = std::move(s);
        {
            std::lock_guard<std::mutex> trava(lock_clientes_);
            todas_.push_back(con);
        }
        std::lock_guard<std::mutex> trava(lock_threads_);
        if (!rodando_) {
            con->sock.fechar();
            break;
        }
        t_clientes_.emplace_back(&Servidor::atender, this, con, endereco);
    }
}

void Servidor::relogio() {
    using relog = std::chrono::steady_clock;
    const auto intervalo = std::chrono::microseconds(1000000 / TICKS_POR_SEGUNDO);
    auto proximo = relog::now();
    while (rodando_ && sala.em_jogo()) {
        auto [numero, comandos] = sala.fechar_tick();
        // o placar e atualizado antes do envio: quem recebe o tick ja o encontra publicado
        const long long n_comandos = static_cast<long long>(comandos.size());
        publicar([&](ipc::Placar& pl) {
            pl.tick = numero;
            pl.comandos += n_comandos;
        });
        transmitir(P::tick(numero, comandos));
        proximo += intervalo;
        auto agora = relog::now();
        if (proximo > agora) std::this_thread::sleep_until(proximo);
        else proximo = agora;
    }
}

void Servidor::atender(ConexaoP con, std::string endereco) {
    con->sock.timeout_recepcao(P::HEARTBEAT_TIMEOUT);
    P::LeitorDeLinhas leitor;
    int numero = 0;
    char buf[4096];
    while (rodando_) {
        int n = con->sock.receber(buf, sizeof buf);
        if (n == -2) {
            registrar("jogador " + std::to_string(numero) + " sem resposta, desconectando");
            break;
        }
        if (n <= 0) break;  // conexao fechada ou resetada
        bool sair = false;
        for (const std::string& linha : leitor.alimentar(buf, n)) {
            P::Mensagem msg;
            try {
                msg = P::interpretar(linha);
            } catch (const P::MensagemInvalida&) {
                if (numero) enviar(numero, P::erro(numero, P::ERRO_INVALIDA));
                continue;
            }
            if (!numero) {
                if (msg.comando != P::ENTRAR) continue;
                const std::string heroi = achar_heroi(msg.heroi) ? msg.heroi : "quincy";
                const std::string mapa = achar_mapa(msg.mapa) ? msg.mapa : "prado";
                auto vaga = sala.reservar_vaga(heroi, mapa);
                if (!vaga) {
                    con->sock.enviar_tudo(P::erro(0, P::ERRO_SALA_CHEIA));
                    sair = true;
                    break;
                }
                numero = *vaga;
                {
                    std::lock_guard<std::mutex> trava(lock_clientes_);
                    clientes_[numero] = con;
                }
                publicar([&](ipc::Placar& pl) {
                    pl.jogador[numero].conectado = 1;
                    ipc::copiar_texto(pl.jogador[numero].heroi, sizeof pl.jogador[numero].heroi, heroi);
                });
                enviar(numero, P::boas_vindas(numero));
                registrar("jogador " + std::to_string(numero) + " entrou de " + endereco + " (" + heroi + ")");
                if (auto ini = sala.iniciar()) {
                    publicar([&](ipc::Placar& pl) {
                        pl.estado = ipc::EM_JOGO;
                        ipc::copiar_texto(pl.mapa, sizeof pl.mapa, ini->mapa);
                    });
                    std::random_device rd;
                    long long seed = std::uniform_int_distribution<long long>(1, 999999)(rd);
                    transmitir(P::inicio(seed, ini->mapa, ini->heroi1, ini->heroi2));
                    std::lock_guard<std::mutex> trava(lock_threads_);
                    if (rodando_) t_relogio_ = std::thread(&Servidor::relogio, this);
                    registrar("partida iniciada: mapa " + ini->mapa + ", " + ini->heroi1 + " x " + ini->heroi2);
                }
                continue;
            }
            tratar(numero, msg);
        }
        if (sair) break;
    }
    if (numero) {
        {
            std::lock_guard<std::mutex> trava(lock_clientes_);
            clientes_.erase(numero);
        }
        bool estava_em_jogo = sala.remover(numero);
        registrar("jogador " + std::to_string(numero) + " saiu");
        publicar([&](ipc::Placar& pl) { pl.jogador[numero].conectado = 0; });
        if (estava_em_jogo && sala.encerrar()) {
            publicar([&](ipc::Placar& pl) {
                pl.estado = ipc::FIM;
                pl.vencedor = Sala::oponente(numero);
            });
            transmitir(P::fim_jogo(Sala::oponente(numero)));
        }
    }
    con->sock.fechar();
}

void Servidor::tratar(int numero, const P::Mensagem& msg) {
    if (msg.origem != numero) {
        enviar(numero, P::erro(numero, P::ERRO_INVALIDA));
        return;
    }
    const char cmd = msg.comando;
    if (cmd == P::PING) {
        enviar(numero, P::ping(P::SERVIDOR));
    } else if (P::eh_comando_jogo(cmd)) {
        if (!sala.em_jogo()) {
            enviar(numero, P::erro(numero, P::ERRO_FORA_DE_HORA));
            return;
        }
        sala.enfileirar(numero, msg.cmd);
    } else if (cmd == P::HASH) {
        auto iguais = sala.registrar_hash(numero, msg.tick, msg.hash);
        if (iguais && !*iguais) {
            registrar("dessincronia no tick " + std::to_string(msg.tick));
            transmitir(P::dessinc(msg.tick));
        }
    } else if (cmd == P::FIM_JOGO) {
        if (sala.encerrar()) {
            publicar([&](ipc::Placar& pl) {
                pl.estado = ipc::FIM;
                pl.vencedor = Sala::oponente(numero);
            });
            transmitir(P::fim_jogo(Sala::oponente(numero)));
        }
    }
}

}  // namespace bl
