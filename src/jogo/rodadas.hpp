// Rodadas (inspiradas no modo classico) e envios de bloons do modo Batalha.
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "jogo/defs.hpp"

namespace bl {

std::vector<Grupo> grupos_da_rodada(int r);

// Lista (tempo_s, grupo) de cada bloon da rodada, ordenada por tempo.
std::vector<std::pair<double, Grupo>> agenda_da_rodada(int r);

double duracao_rodada(int r);

// Regras do BTD6 que mudam com a rodada (modo solo)
double mult_renda_da_rodada(int r);  // dinheiro por estouro
double mult_vida_moab(int r);        // vida dos dirigiveis a partir da R81
double mult_velocidade(int r);       // velocidade de todos os bloons a partir da R81
double xp_da_rodada(int r);          // XP do heroi ao fim da rodada
double mult_xp_mapa(const std::string& dificuldade_do_mapa);

}  // namespace bl
