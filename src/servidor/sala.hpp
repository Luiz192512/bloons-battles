// Memoria compartilhada do servidor.
//
// Varias threads acessam este objeto ao mesmo tempo:
//   - uma thread por cliente (entra na sala, enfileira comandos, envia hashes);
//   - a thread do relogio (esvazia a fila a cada tick e transmite).
//
// Cada regiao tem seu proprio mutex. Regra anti deadlock: nunca segurar dois
// mutexes ao mesmo tempo; quando precisar de mais de um, adquira sempre na ordem
// lock_sala -> lock_fila -> lock_hash.
#pragma once

#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace bl {

class Sala {
public:
    enum class Estado { AGUARDANDO, EM_JOGO, FIM };

    struct InfoJogador {
        int numero;
        std::string heroi;
        std::string mapa;
    };

    struct Inicio {
        std::string mapa, heroi1, heroi2;
    };

    // ---------- sala
    // Numero do novo jogador, ou nada se a sala estiver cheia.
    // Secao critica: dois clientes conectando ao mesmo tempo nao podem receber o mesmo numero.
    std::optional<int> reservar_vaga(const std::string& heroi, const std::string& mapa);
    // Muda para EM_JOGO uma unica vez. Devolve (mapa, heroi1, heroi2) a quem iniciou.
    std::optional<Inicio> iniciar();
    bool em_jogo();
    Estado estado();
    // Marca FIM. Devolve true so para a primeira thread que encerrar.
    bool encerrar();
    // Remove o jogador. Devolve true se a partida estava em andamento.
    bool remover(int numero);
    static int oponente(int numero) { return numero == 1 ? 2 : 1; }

    // ---------- fila de comandos (produtor/consumidor)
    void enfileirar(int jogador, const std::string& cmd);
    // Troca a fila por uma vazia e avanca o tick, atomicamente.
    std::pair<long, std::vector<std::string>> fechar_tick();

    // ---------- deteccao de dessincronia
    // Guarda o hash. Quando os dois chegam, devolve true se forem iguais.
    std::optional<bool> registrar_hash(int jogador, long tick, std::uint32_t valor);

private:
    Estado estado_ = Estado::AGUARDANDO;
    std::map<int, InfoJogador> jogadores_;
    std::mutex lock_sala_;  // jogadores e estado

    long tick_ = 0;
    std::vector<std::string> fila_;  // comandos aguardando o proximo tick
    std::mutex lock_fila_;

    std::map<long, std::map<int, std::uint32_t>> hashes_;  // tick -> jogador -> hash
    std::mutex lock_hash_;
};

}  // namespace bl
