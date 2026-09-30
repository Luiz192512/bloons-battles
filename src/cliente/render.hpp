// Desenho de uma pista (mapa, torres, bloons, projeteis e efeitos).
#pragma once

#include <random>
#include <string>
#include <vector>

#include "cliente/anim.hpp"
#include "jogo/sim.hpp"
#include "raylib.h"

namespace bl {

struct Efeito {
    std::string tipo;
    Evento dados;
    double t = 0, dur;
};

class RenderPista {
public:
    RenderPista(Pista& pista, std::string chave_mapa, bool sons = true);

    // Transforma os eventos da simulacao em efeitos visuais, sons e avisos.
    void consumir_eventos();
    void atualizar(double dt);
    void desenhar(int selecionada = 0);
    void desenhar_mini(Rectangle r);

    std::vector<Evento> avisos;  // fim_rodada, eco, envio
    // Animacoes de disparo/habilidade: so leem a pista (nao mexem na simulacao).
    anim::Animador& animador() { return animador_; }

private:
    void desenhar_torre(const Torre& t, bool sel);
    void desenhar_habilidades_em_uso();
    void desenhar_bloons();
    void desenhar_efeitos();

    Pista& pista_;
    std::string chave_mapa_;
    bool sons_;
    std::vector<Efeito> efeitos_;
    bool flash_ = false;
    Cor flash_cor_{};
    double flash_t_ = 0;
    std::mt19937 rng_{1};
    anim::Animador animador_;
};

}  // namespace bl
