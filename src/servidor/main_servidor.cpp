// Servidor avulso do modo Batalha.
//
// Uso:
//     bloons_servidor [porta]
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>
#include <thread>

#include "servidor/servidor.hpp"

namespace {
volatile std::sig_atomic_t parar = 0;
void ao_sinal(int) { parar = 1; }
}  // namespace

int main(int argc, char** argv) {
    int porta = argc > 1 ? std::atoi(argv[1]) : bl::proto::PORTA_PADRAO;
    std::signal(SIGINT, ao_sinal);
    std::signal(SIGTERM, ao_sinal);
    try {
        bl::Servidor servidor("0.0.0.0", porta);
        std::mutex lock_log;
        servidor.log = [&](const std::string& texto) {
            std::lock_guard<std::mutex> trava(lock_log);
            std::time_t agora = std::time(nullptr);
            char hora[16];
            std::strftime(hora, sizeof hora, "%H:%M:%S", std::localtime(&agora));
            std::printf("%s [servidor] %s\n", hora, texto.c_str());
            std::fflush(stdout);
        };
        servidor.iniciar();
        while (!parar) std::this_thread::sleep_for(std::chrono::milliseconds(200));
        servidor.parar();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "erro: %s\n", e.what());
        return 1;
    }
    return 0;
}
