#include "jogo/rodadas.hpp"

#include <algorithm>

namespace bl {

std::vector<Grupo> grupos_da_rodada(int r) {
    auto it = RODADAS.find(r);
    if (it != RODADAS.end()) return it->second;
    // freeplay depois da R140: mistura crescente (aproximacao; o BTD6 sorteia os grupos)
    int k = r - 140;
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

// Dinheiro por estouro (JSON:Bloons TD 6/Rounds/DefaultIncomeSet na Blooncyclopedia)
double mult_renda_da_rodada(int r) {
    if (r <= 50) return 1.0;
    if (r <= 60) return 0.5;
    if (r <= 85) return 0.2;
    if (r <= 100) return 0.1;
    if (r <= 120) return 0.05;
    if (r <= 140) return 0.04;
    return 0.02;
}

// Rampa de vida e velocidade do freeplay do BTD6 (Bloons Wiki, "Late Game and Freeplay (BTD6)")
double mult_vida_moab(int r) {
    if (r <= 80) return 1.0;
    if (r <= 100) return 1.0 + 0.02 * (r - 80);
    if (r <= 124) return 1.4 + 0.05 * (r - 100);
    if (r <= 150) return 2.6 + 0.15 * (r - 124);
    return 6.5 + 0.35 * (r - 150);
}

double mult_velocidade(int r) {
    if (r <= 80) return 1.0;
    if (r <= 100) return 1.0 + 0.02 * (r - 80);
    return 1.6 + 0.02 * (r - 101);
}

double duracao_rodada(int r) {
    auto ag = agenda_da_rodada(r);
    return ag.empty() ? 0.0 : ag.back().first;
}

}  // namespace bl
