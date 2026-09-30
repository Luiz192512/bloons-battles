// Atributos de uma torre a partir da definicao + upgrades + nivel de heroi.
#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "jogo/defs.hpp"

namespace bl {

enum class TipoAtaque { PROJETIL, RADIAL, AURA, HITSCAN, CADEIA, MORTEIRO, PILHA, QUEDA, RENDA, BUFF, INVOCAR };

// Buffs que torres de suporte dao as vizinhas. "cad" multiplica; o resto soma (ou liga).
struct Buffs {
    double cad = 1.0;
    double alcance_pct = 0, pierce = 0, dano = 0, moab = 0, ouro = 0;
    bool camo = false, dtype_normal = false;
    bool vazio = true;

    void mesclar(const Buffs& o);
    void mesclar(const J& novos);
};

struct Ataque {
    TipoAtaque tipo = TipoAtaque::PROJETIL;
    double cad = 1.0, dano = 1, pierce = 1;
    DType dtype = DT_AFIADO;
    double vel = 600.0, dist = 250.0, n = 1, spread = 0.0;
    bool busca = false, boom = false;
    double raio_proj = 5.0, splash = 0.0, sdano = 0, spierce = 0;
    DType sdtype = 0;  // 0 = explosao
    double moab = 0, cer = 0, fort = 0;
    bool tem_lento = false;
    double lento_f = 1, lento_t = 0;
    double congela = 0.0;
    bool tem_cola = false;
    double cola_f = 1, cola_t = 0, cola_dps = 0;
    bool tem_queima = false;
    double queima_dps = 0, queima_t = 0;
    double atordoa = 0.0, empurra = 0.0, fragiliza = 0;
    bool retira_camo = false, retira_regen = false;
    bool armadilha = false, prende_moab = false;  // Bloon Trap: pierce = capacidade em RBE, valor = $ por RBE
    double quica = 0;
    std::shared_ptr<const Ataque> frag;  // ataque dos fragmentos
    int frag_n = 0;
    bool global_ = false;
    std::string visual = "dardo";
    bool alvo_forte = false;  // alvo="forte"
    bool so_moab = false, moab_lento = false, moab_congela = false, moab_cola = false, moab_atordoa = false;
    double raio_aura = 0.0, pilha_pierce = 0, pilha_vida = 10.0, saltos = 0, valor = 0.0;
    Buffs buffs;
    double fusivel = 0.0;
    bool na_trilha = false;
    double impreciso = 0.0;
    bool linha = false;
    double dur = 0.0;
    std::string base;
    double nivel_inv = 0;
    // critico: a cada crit_cada tiros (sorteado ate crit_max, se maior), o tiro da crit_dano no lugar do
    // dano normal, ou soma crit_mais (Sharp Shooter, Crossbow Master, Robo Monkey)
    double crit_cada = 0, crit_max = 0, crit_dano = 0, crit_mais = 0;
};

// Cria um ataque a partir dos valores padrao + os campos do objeto.
Ataque novo_ataque(const J& d);

// Atributos efetivos (antes de buffs temporarios) de uma torre.
struct Stats {
    double alcance = 0;
    bool camo = false;
    double ouro = 0.0;
    double ouro_chumbo = 0.0;  // $ extra por chumbo estourado (Lead to Gold)
    double desconto = 0.0;
    double venda = 0.7;
    bool persegue = false;
    std::vector<J> habs;
    std::vector<Ataque> ataques;
};

void aplicar(Stats& st, const J& ef);
Stats calcular(const std::string& chave, std::array<int, 3> caminhos = {0, 0, 0}, int nivel = 0);
bool pode_upar(std::array<int, 3> caminhos, int p);
int arredondar_preco(double valor);

}  // namespace bl
