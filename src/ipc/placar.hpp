// Placar da sala em memoria compartilhada entre processos.
//
// Nome da regiao: "bloons_sala_<porta>". Quem escreve e quem le:
//   - servidor (processo que hospeda): estado da sala, mapa, herois, tick, total de comandos;
//   - cada cliente na mesma maquina: a propria linha (vidas, dinheiro, eco, rodada...);
//   - bloons_monitor (processo separado, no console): so le e mostra.
//
// Toda leitura e escrita acontece com o mutex com nome da regiao. Nenhum processo segura esse
// mutex junto com os mutexes internos da Sala (primeiro copia os dados da Sala, solta, e so
// depois trava o placar), entao nao ha como formar deadlock entre processos.
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "ipc/memoria.hpp"

namespace bl::ipc {

constexpr std::uint32_t PLACAR_MAGICO = 0x424C4F4E;  // "BLON"
constexpr std::uint32_t PLACAR_VERSAO = 1;

enum EstadoPlacar : std::int32_t { SEM_SALA = 0, AGUARDANDO = 1, EM_JOGO = 2, FIM = 3 };

// So tipos de tamanho fixo: os bytes precisam ter o mesmo significado em todos os processos.
struct PlacarJogador {
    std::int32_t conectado;
    char heroi[24];
    std::int32_t vidas;
    std::int64_t dinheiro;
    std::int32_t eco;
    std::int32_t rodada;
    std::int32_t pops;
    std::int32_t torres;
    std::int32_t bloons;
    std::uint32_t hash;
    std::int64_t tick;
    std::int32_t pid;  // processo que escreveu esta linha
};

struct Placar {
    std::uint32_t magico;
    std::uint32_t versao;
    std::int32_t pid_servidor;
    std::int32_t porta;
    std::int32_t estado;
    char mapa[24];
    std::int64_t tick;
    std::int64_t comandos;  // comandos de jogo ja repassados nos ticks
    std::int32_t vencedor;
    std::uint32_t escritas;  // contador de atualizacoes (qualquer processo)
    PlacarJogador jogador[3];  // indices 1 e 2
};

std::string nome_placar(int porta);
int pid_atual();
void copiar_texto(char* destino, std::size_t tam, const std::string& origem);

class PlacarCompartilhado {
public:
    // Servidor: cria a regiao e preenche o cabecalho.
    static std::unique_ptr<PlacarCompartilhado> criar(int porta);
    // Clientes e monitor: abrem a regiao criada pelo servidor (nullptr se nao existir nesta maquina).
    static std::unique_ptr<PlacarCompartilhado> abrir(int porta);

    // Altera o placar dentro da secao critica (mutex entre processos).
    void atualizar(const std::function<void(Placar&)>& f);
    // Copia o placar inteiro dentro da secao critica.
    Placar ler();

private:
    explicit PlacarCompartilhado(std::unique_ptr<MemoriaCompartilhada> m) : mem_(std::move(m)) {}
    Placar* dados() { return static_cast<Placar*>(mem_->dados()); }
    std::unique_ptr<MemoriaCompartilhada> mem_;
};

}  // namespace bl::ipc
