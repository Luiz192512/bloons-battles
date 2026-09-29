// Arte do jogo desenhada por codigo (sem imagens externas).
//
// Macacos, bloons, dirigiveis e o fundo de cada mapa sao desenhados uma vez numa
// RenderTexture (em resolucao dobrada, para ficarem lisos) e guardados em cache.
// Projeteis, pilhas e efeitos sao desenhados direto na tela a cada quadro.
#pragma once

#include <string>

#include "jogo/defs.hpp"
#include "jogo/sim.hpp"
#include "raylib.h"

namespace bl::arte {

inline Color cor(Cor c, unsigned char a = 255) { return Color{c.r, c.g, c.b, a}; }

// Macaco/torre olhando para cima, centrado em (x, y). rotacao em graus, sentido horario.
void torre(const std::string& chave, float x, float y, int tam, int tier = 0, float rotacao = 0,
           unsigned char alfa = 255);
void bloon(const TipoBloon& tipo, bool camo, bool regen, bool fort, int dano, float x, float y, float escala = 1);
// Dirigivel apontando para +x; ang em graus.
void dirigivel(const TipoBloon& tipo, bool fort, int dano, float x, float y, float ang, float escala = 1);
// Tamanho (largura, altura) do desenho de um dirigivel sem rotacao.
Vector2 tamanho_dirigivel(const TipoBloon& tipo);

void fundo_mapa(const std::string& chave, Rectangle destino);

void icone_coracao(float x, float y, int tam);
void icone_moeda(float x, float y, int tam);
void icone_eco(float x, float y, int tam);

void projetil(const Projetil& p, float ox = 0, float oy = 0, float escala = 1);
void pilha(const Pilha& s, float ox = 0, float oy = 0, float escala = 1);

void liberar();

}  // namespace bl::arte
