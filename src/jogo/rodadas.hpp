// Rodadas (inspiradas no modo classico) e envios de bloons do modo Batalha.
#pragma once

#include <utility>
#include <vector>

#include "jogo/defs.hpp"

namespace bl {

std::vector<Grupo> grupos_da_rodada(int r);

// Lista (tempo_s, grupo) de cada bloon da rodada, ordenada por tempo.
std::vector<std::pair<double, Grupo>> agenda_da_rodada(int r);

double duracao_rodada(int r);

}  // namespace bl
