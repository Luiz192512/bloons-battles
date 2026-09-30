// Sprites do jogo, portados de "Sprites.dc.html" (biblioteca sprites2.js do projeto de design).
//
// Cada funcao desenha com a Caneta numa caixa fixa: torres, herois, bloons, efeitos e
// habilidades em 128x128; dirigiveis em 220x120; projeteis centrados em (0, 0) com raio ~28;
// icones do HUD em 32x32. Quem chama cuida de posicionar/escalar (arte.cpp).
#pragma once

#include <string>

#include "cliente/caneta.hpp"

namespace bl::spr {

enum class Vista { CIMA, FRENTE };

// Torre ou heroi. cam = caminho do visual (-1 = base), tier = 0, 3 ou 5; nivel so para herois.
void torre(Caneta& p, const std::string& chave, Vista v, int cam, int tier, int nivel = 1);
bool eh_maquina(const std::string& chave);
bool voa(const std::string& chave);  // sombra deslocada (as, heli)

// dano: rachaduras da ceramica (0..2)
void bloon(Caneta& p, const std::string& tipo, bool camo, bool regen, bool fort, bool flutuar, int dano = 0);
void dirigivel(Caneta& p, const std::string& tipo, int dano, bool fort, bool flutuar);
Vector2 raios_dirigivel(const std::string& tipo);  // rx, ry do corpo na caixa 220x120

bool tem_projetil(const std::string& visual);
void projetil(Caneta& p, const std::string& visual);

void efeito(Caneta& p, const std::string& chave);  // estouro, explosao, dinheiro, nivel, impacto...
// Marcas de estado por cima de um bloon (caixa 128, bloon no centro com raio ~34).
void estado_congelado(Caneta& p);
void estado_colado(Caneta& p);
void estado_queimando(Caneta& p);
void estado_atordoado(Caneta& p);

// Icone de habilidade: moldura dourada, retrato de quem usa e selo do efeito.
void habilidade(Caneta& p, const std::string& dono, bool heroi, int cam, int tier, int nivel, const std::string& efeito);
Color cor_efeito(const std::string& efeito);

void icone(Caneta& p, const std::string& nome);  // coracao, moeda, eco, rodada, oponente, cadeado, estrela

}  // namespace bl::spr
