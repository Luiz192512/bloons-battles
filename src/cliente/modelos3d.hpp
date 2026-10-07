// Torres em 3D: carrega os .glb de assets/modelos (um por combinacao de upgrades) e desenha cada
// torre como um sprite renderizado na vista do jogo, girado para a mira. Sem a pasta de modelos
// (ex.: o executavel sozinho), tudo aqui devolve false e o jogo usa os sprites 2D de sempre.
#pragma once

#include <array>
#include <string>

#include "cliente/anim.hpp"
#include "raylib.h"

namespace bl::m3d {

bool disponivel();
// Existe modelo 3D para esta torre ou heroi (a variacao base)?
bool tem(const std::string& chave);

// Desenha a torre com o pe em (x, y). px = pixels por unidade do modelo (o macaco tem 1 de altura).
// ang = para onde ela aponta, em graus na tela (0 = direita, 90 = baixo). Devolve false se nao
// houver modelo para essa torre.
// pose = instante do clipe de disparo ou de habilidade (anim::Animador): o braco de ataque gira e
// avanca, a cabeca acompanha e o cano das maquinas recua. Sem pose (ou parada), usa o sprite guardado.
bool torre(const std::string& chave, const std::array<int, 3>& caminhos, float x, float y, float px, float ang,
           anim::TipoMira mira, unsigned char alfa = 255, const spr::Pose* pose = nullptr);

// Retrato 2D da torre ou heroi (assets/retratos/<chave>.png, feito a partir do modelo 3D): o desenho
// da loja e dos menus. Centro em (cx, cy), com 'lado' pixels. Devolve false se nao houver o arquivo.
bool retrato(const std::string& chave, float cx, float cy, float lado, unsigned char alfa = 255);

// O retrato dentro de uma caixa pequena (cartao da loja, grade de herois). Maquina e construcao
// entram inteiras; macaco de corpo inteiro e ampliado e ancorado pelo topo, para a cabeca e o
// tronco ficarem grandes, e o que passa da caixa e recortado.
bool retrato_em(const std::string& chave, Rectangle caixa, unsigned char alfa = 255);

void liberar();

}  // namespace bl::m3d
