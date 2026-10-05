// Torres em 3D: carrega os .glb de assets/modelos (um por combinacao de upgrades) e desenha cada
// torre como um sprite renderizado na vista do jogo, girado para a mira. Sem a pasta de modelos
// (ex.: o executavel sozinho), tudo aqui devolve false e o jogo usa os sprites 2D de sempre.
#pragma once

#include <array>
#include <string>

#include "cliente/anim.hpp"

namespace bl::m3d {

bool disponivel();
// Existe modelo 3D para esta torre ou heroi (a variacao base)?
bool tem(const std::string& chave);

// Desenha a torre com o pe em (x, y). px = pixels por unidade do modelo (o macaco tem 1 de altura).
// ang = para onde ela aponta, em graus na tela (0 = direita, 90 = baixo). Devolve false se nao
// houver modelo para essa torre.
bool torre(const std::string& chave, const std::array<int, 3>& caminhos, float x, float y, float px, float ang,
           anim::TipoMira mira, unsigned char alfa = 255);

void liberar();

}  // namespace bl::m3d
