#include "servidor/sala.hpp"

#include "comum/protocolo.hpp"

namespace bl {

std::optional<int> Sala::reservar_vaga(const std::string& heroi, const std::string& mapa) {
    std::lock_guard<std::mutex> trava(lock_sala_);
    if (estado_ != Estado::AGUARDANDO) return std::nullopt;
    for (int numero = 1; numero <= proto::MAX_JOGADORES; ++numero) {
        if (!jogadores_.count(numero)) {
            jogadores_[numero] = InfoJogador{numero, heroi, mapa};
            return numero;
        }
    }
    return std::nullopt;
}

std::optional<Sala::Inicio> Sala::iniciar() {
    std::lock_guard<std::mutex> trava(lock_sala_);
    if (estado_ == Estado::AGUARDANDO && static_cast<int>(jogadores_.size()) == proto::MAX_JOGADORES) {
        estado_ = Estado::EM_JOGO;
        return Inicio{jogadores_[1].mapa, jogadores_[1].heroi, jogadores_[2].heroi};
    }
    return std::nullopt;
}

bool Sala::em_jogo() {
    std::lock_guard<std::mutex> trava(lock_sala_);
    return estado_ == Estado::EM_JOGO;
}

Sala::Estado Sala::estado() {
    std::lock_guard<std::mutex> trava(lock_sala_);
    return estado_;
}

bool Sala::encerrar() {
    std::lock_guard<std::mutex> trava(lock_sala_);
    if (estado_ == Estado::FIM) return false;
    estado_ = Estado::FIM;
    return true;
}

bool Sala::remover(int numero) {
    std::lock_guard<std::mutex> trava(lock_sala_);
    jogadores_.erase(numero);
    return estado_ == Estado::EM_JOGO;
}

void Sala::enfileirar(int jogador, const std::string& cmd) {
    std::lock_guard<std::mutex> trava(lock_fila_);
    fila_.push_back(std::to_string(jogador) + cmd);
}

std::pair<long, std::vector<std::string>> Sala::fechar_tick() {
    std::lock_guard<std::mutex> trava(lock_fila_);
    std::vector<std::string> comandos;
    comandos.swap(fila_);
    tick_ += 1;
    return {tick_, std::move(comandos)};
}

std::optional<bool> Sala::registrar_hash(int jogador, long tick, std::uint32_t valor) {
    std::lock_guard<std::mutex> trava(lock_hash_);
    auto& por_jogador = hashes_[tick];
    por_jogador[jogador] = valor;
    if (por_jogador.size() < 2) return std::nullopt;
    bool iguais = true;
    for (auto& [j, v] : por_jogador) iguais = iguais && v == por_jogador.begin()->second;
    hashes_.erase(tick);
    // limpa ticks antigos que um jogador nunca respondeu
    hashes_.erase(hashes_.begin(), hashes_.lower_bound(tick - 3000));
    return iguais;
}

}  // namespace bl
