// Arte do jogo desenhada por codigo (sem imagens externas), no visual de "Sprites.dc.html".
//
// Os sprites (sprites.cpp) sao desenhados com a Caneta. As partes paradas de cada sprite vao
// para RenderTextures (em resolucao dobrada, para ficarem lisas) e ficam em cache; so os grupos
// animados (braco que ataca, orbitas, helices...) sao desenhados a cada quadro, na ordem certa.
#pragma once

#include <array>
#include <string>

#include "cliente/anim.hpp"
#include "jogo/defs.hpp"
#include "jogo/sim.hpp"
#include "raylib.h"

namespace bl::arte {

inline Color cor(Cor c, unsigned char a = 255) { return Color{c.r, c.g, c.b, a}; }

// Qual variacao do sprite mostrar: caminho com mais upgrades (tier 3 ou 5) ou nivel do heroi.
struct Visual {
    int cam = -1, tier = 0, nivel = 1;
};
Visual visual(const Torre& t);
Visual visual_caminhos(const std::array<int, 3>& caminhos);

// Torre no mapa (vista de cima), centrada em (x, y). tam = lado da caixa do sprite em px.
// rotacao em graus (0 = olhando para cima). q = quadro de animacao (disparo/habilidade).
void torre(const std::string& chave, const Visual& v, float x, float y, float tam, float rotacao,
           const anim::Quadro* q = nullptr, unsigned char alfa = 255);
// Icone da torre (vista 3/4 de frente), parado, para cards, painel e menus.
void torre_icone(const std::string& chave, const Visual& v, float x, float y, float tam, unsigned char alfa = 255);
// Torre de frente animada (menu, retrato do heroi, vitrine).
void torre_viva(const std::string& chave, const Visual& v, float x, float y, float tam, double t,
                const anim::Quadro* q = nullptr, bool cima = false);

void bloon(const TipoBloon& tipo, bool camo, bool regen, bool fort, int dano, float x, float y, float escala = 1);
// Bloon por nome, flutuando (fundo do menu, vitrine).
void bloon_vivo(const std::string& tipo, bool camo, bool regen, bool fort, float x, float y, float tam, double t);
float raio_bloon_px(const TipoBloon& tipo);
// Marcas de estado por cima do bloon: 0 congelado, 1 colado, 2 queimando, 3 atordoado.
void estado_bloon(int estado, float x, float y, float raio, double t);

// Dirigivel apontando para +x; ang em graus. dano 0..4.
void dirigivel(const TipoBloon& tipo, bool fort, int dano, float x, float y, float ang, float escala = 1);
void dirigivel_vivo(const std::string& tipo, int dano, bool fort, float x, float y, float largura, double t);
Vector2 tamanho_dirigivel(const TipoBloon& tipo);

void fundo_mapa(const std::string& chave, Rectangle destino);

// Icones do HUD (coracao, moeda, eco, rodada, oponente, cadeado, estrela); (x, y) = canto superior esquerdo.
void icone(const std::string& nome, float x, float y, float tam);
void icone_coracao(float x, float y, int tam);
void icone_moeda(float x, float y, int tam);
void icone_eco(float x, float y, int tam);

// Icone de habilidade centrado em (x, y).
void habilidade(const std::string& dono, bool heroi, int cam, int tier, int nivel, const std::string& efeito, float x,
                float y, float tam);

void projetil(const Projetil& p, float ox = 0, float oy = 0, float escala = 1);
void projetil_vivo(const std::string& visual, float x, float y, float tam, float ang, double t);
void pilha(const Pilha& s, float ox = 0, float oy = 0, float escala = 1);

// Efeito animado (estouro, explosao, dinheiro, nivel, impacto, camo_revelado) no instante t (s).
void efeito(const std::string& chave, float x, float y, float tam, double t);

void liberar();

}  // namespace bl::arte
