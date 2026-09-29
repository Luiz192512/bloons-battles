#include "jogo/rodadas.hpp"

#include <algorithm>

namespace bl {

std::vector<Grupo> grupos_da_rodada(int r) {
    auto it = RODADAS.find(r);
    if (it != RODADAS.end()) return it->second;
    // rodadas livres/intermediarias acima de 80: mistura crescente
    int k = r - 80;
    return {
        {"ceramica", 80 + 6 * k, 0.08, 0.0, false, true, k > 3},
        {"moab", 6 + k, 0.5, 4.0, false, false, true},
        {"bfb", 2 + k / 3, 1.5, 8.0, false, false, k > 6},
        {"zomg", k / 6, 2.0, 12.0},
        {"ddt", k / 4, 0.8, 14.0},
    };
}

std::vector<std::pair<double, Grupo>> agenda_da_rodada(int r) {
    std::vector<std::pair<double, Grupo>> eventos;
    for (const Grupo& g : grupos_da_rodada(r))
        for (int i = 0; i < g.qtd; ++i) eventos.push_back({g.inicio + i * g.espaco, g});
    std::stable_sort(eventos.begin(), eventos.end(),
                     [](const auto& a, const auto& b) { return a.first < b.first; });
    return eventos;
}

double duracao_rodada(int r) {
    auto ag = agenda_da_rodada(r);
    return ag.empty() ? 0.0 : ag.back().first;
}

}  // namespace bl
