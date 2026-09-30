// Monitor da sala: outro processo que le o placar em memoria compartilhada e mostra no console.
//
// Uso:
//     bloons_monitor [porta]            atualiza a cada 0,5 s ate Ctrl+C
//     bloons_monitor [porta] --uma-vez  mostra uma vez e sai (0 = leu, 1 = sala nao encontrada)
//
// Precisa rodar na mesma maquina de quem hospeda a partida.
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>

#include "comum/protocolo.hpp"
#include "ipc/placar.hpp"

namespace {

volatile std::sig_atomic_t parar = 0;
void ao_sinal(int) { parar = 1; }

const char* nome_estado(int e) {
    switch (e) {
        case bl::ipc::AGUARDANDO: return "AGUARDANDO";
        case bl::ipc::EM_JOGO: return "EM_JOGO";
        case bl::ipc::FIM: return "FIM";
        default: return "SEM_SALA";
    }
}

void mostrar(const bl::ipc::Placar& pl) {
    std::printf("sala %d | %s | mapa %s | tick %lld | comandos %lld | escritas %u | servidor pid %d",
                pl.porta, nome_estado(pl.estado), pl.mapa[0] ? pl.mapa : "-", static_cast<long long>(pl.tick),
                static_cast<long long>(pl.comandos), pl.escritas, pl.pid_servidor);
    if (pl.estado == bl::ipc::FIM) std::printf(" | vencedor J%d", pl.vencedor);
    std::printf("\n");
    for (int j = 1; j <= 2; ++j) {
        const bl::ipc::PlacarJogador& l = pl.jogador[j];
        if (!l.conectado) {
            std::printf("  J%d  (desconectado)\n", j);
        } else if (!l.pid) {
            // a linha so e escrita pelo cliente depois do inicio, e so se ele roda nesta maquina
            std::printf("  J%d  %-10s %s\n", j, l.heroi,
                        pl.estado == bl::ipc::AGUARDANDO ? "(aguardando o inicio)" : "(cliente em outra maquina)");
        } else {
            std::printf("  J%d  %-10s vidas %4d  $%-8lld eco %-5d rodada %-3d torres %-3d bloons %-4d "
                        "estouros %-6d tick %-6lld hash %08x  pid %d\n",
                        j, l.heroi, l.vidas, static_cast<long long>(l.dinheiro), l.eco, l.rodada, l.torres,
                        l.bloons, l.pops, static_cast<long long>(l.tick), l.hash, l.pid);
        }
    }
    std::fflush(stdout);
}

}  // namespace

int main(int argc, char** argv) {
    int porta = bl::proto::PORTA_PADRAO;
    bool uma_vez = false;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--uma-vez") uma_vez = true;
        else porta = std::atoi(argv[i]);
    }
    std::signal(SIGINT, ao_sinal);

    auto placar = bl::ipc::PlacarCompartilhado::abrir(porta);
    if (uma_vez) {
        if (!placar) {
            std::printf("sala %d nao encontrada nesta maquina\n", porta);
            return 1;
        }
        mostrar(placar->ler());
        return 0;
    }
    while (!parar) {
        if (!placar) {
            placar = bl::ipc::PlacarCompartilhado::abrir(porta);
            if (!placar) {
                std::printf("aguardando uma sala na porta %d...\n", porta);
                std::fflush(stdout);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
        }
        const bl::ipc::Placar pl = placar->ler();
        mostrar(pl);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}
