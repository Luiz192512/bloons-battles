#include "ipc/placar.hpp"

#include <cstring>
#include <type_traits>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

namespace bl::ipc {

static_assert(std::is_trivially_copyable<Placar>::value, "o placar precisa ser copiavel byte a byte");

std::string nome_placar(int porta) { return "bloons_sala_" + std::to_string(porta); }

int pid_atual() {
#ifdef _WIN32
    return _getpid();
#else
    return static_cast<int>(getpid());
#endif
}

void copiar_texto(char* destino, std::size_t tam, const std::string& origem) {
    std::size_t n = origem.size() < tam - 1 ? origem.size() : tam - 1;
    std::memcpy(destino, origem.data(), n);
    destino[n] = '\0';
}

std::unique_ptr<PlacarCompartilhado> PlacarCompartilhado::criar(int porta) {
    std::unique_ptr<PlacarCompartilhado> p(
        new PlacarCompartilhado(MemoriaCompartilhada::criar(nome_placar(porta), sizeof(Placar))));
    p->atualizar([&](Placar& pl) {
        pl.magico = PLACAR_MAGICO;
        pl.versao = PLACAR_VERSAO;
        pl.pid_servidor = pid_atual();
        pl.porta = porta;
        pl.estado = AGUARDANDO;
        pl.vencedor = -1;
    });
    return p;
}

std::unique_ptr<PlacarCompartilhado> PlacarCompartilhado::abrir(int porta) {
    auto mem = MemoriaCompartilhada::abrir(nome_placar(porta), sizeof(Placar));
    if (!mem) return nullptr;
    std::unique_ptr<PlacarCompartilhado> p(new PlacarCompartilhado(std::move(mem)));
    Placar pl = p->ler();
    if (pl.magico != PLACAR_MAGICO || pl.versao != PLACAR_VERSAO) return nullptr;  // outro programa com o mesmo nome
    return p;
}

void PlacarCompartilhado::atualizar(const std::function<void(Placar&)>& f) {
    MemoriaCompartilhada::Trava trava(*mem_);
    f(*dados());
    dados()->escritas += 1;
}

Placar PlacarCompartilhado::ler() {
    MemoriaCompartilhada::Trava trava(*mem_);
    Placar copia;
    std::memcpy(&copia, dados(), sizeof copia);
    return copia;
}

}  // namespace bl::ipc
