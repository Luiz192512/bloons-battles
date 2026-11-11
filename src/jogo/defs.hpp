// Definicoes estaticas do jogo: tipos de bloon, torres, herois, mapas, rodadas e envios.
#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

namespace bl {

// Objeto dinamico que guarda os efeitos dos upgrades (mantem a ordem de insercao).
using J = nlohmann::ordered_json;

struct Cor {
    unsigned char r = 0, g = 0, b = 0;
};

// Tipos de dano (bits, para montar a mascara de imunidades)
enum : std::uint8_t {
    DT_AFIADO = 1,    // dardos, tachinhas, laminas
    DT_EXPLOSAO = 2,  // bombas, morteiro
    DT_GELO = 4,      // congelamento
    DT_ENERGIA = 8,   // magia, fogo, plasma, laser
    DT_NORMAL = 16,   // estoura qualquer coisa
};
using DType = std::uint8_t;  // 0 = nenhum (usa o padrao)

DType dtype_de(const std::string& nome);

constexpr double VELOCIDADE_BASE = 95.0;  // px/s do bloon vermelho

struct TipoBloon {
    std::string nome;
    std::string rotulo;
    Cor cor;
    double raio;
    double velocidade;  // multiplicador da base
    int vida = 1;
    std::vector<std::string> filhos;
    int imune = 0;  // mascara DT_*
    bool moab = false;  // classe MOAB (dirigiveis)
    bool congela = true;
    bool camo_nativo = false;
    int rank = 0;
    int vida_fortificado = 0;

    // preenchidos na carga
    int id = 0;
    std::vector<int> filhos_id;
};

struct Upgrade {
    std::string nome;
    int custo;
    std::string desc;
    J ef;
};

enum class Mov { FIXO, ORBITA, HELI };

struct DefTorre {
    DefTorre() = default;
    DefTorre(std::string chave_, std::string nome_, int custo_, std::string tecla_, double alcance_)
        : chave(std::move(chave_)), nome(std::move(nome_)), custo(custo_), tecla(std::move(tecla_)),
          alcance(alcance_) {}

    std::string chave;
    std::string nome;
    int custo = 0;
    std::string tecla;
    double alcance = 0;
    std::vector<J> ataques;
    std::vector<std::vector<Upgrade>> caminhos;
    std::string categoria = "primaria";
    bool agua = false;
    Mov mov = Mov::FIXO;
    double raio = 20;
    bool camo = false;
    std::string desc;
    Cor cor{140, 90, 40};
    bool heroi = false;
    // so herois
    std::map<int, J> niveis;  // nivel -> efeitos
    J hab3;
    J hab10;
    std::string titulo;
    double xp_escala = 1.0;  // XP necessario multiplicado por heroi (BTD6)
};

struct Obstaculo {
    double x, y, r;
    std::string tipo;  // arvore | pedra
};

struct DefMapa {
    std::string chave;
    std::string nome;
    std::string dificuldade;
    std::vector<std::vector<std::pair<double, double>>> trilhas;
    std::vector<std::array<double, 3>> agua;      // circulos (x, y, r)
    std::vector<std::array<double, 4>> agua_ret;  // retangulos (x, y, w, h)
    std::vector<Obstaculo> obstaculos;
    Cor grama{98, 170, 58};
    Cor terra{196, 160, 104};
};

struct Grupo {
    std::string tipo;
    int qtd;
    double espaco;       // segundos entre bloons
    double inicio = 0;   // atraso do grupo em segundos
    bool camo = false;
    bool regen = false;
    bool fort = false;
};

struct Dificuldade {
    std::string chave;
    std::string nome;
    int vidas;
    double mult_custo;
    int ultima_rodada;
    int primeira_rodada = 1;
    double mult_vel = 1.0;          // velocidade dos bloons (Medio = 1)
    int dinheiro_inicial = 650;
    double mult_dinheiro = 1.0;     // Half Cash: 0,5; Deflation: 0 (so a venda devolve dinheiro)
    bool sem_venda = false;         // CHIMPS
    bool so_estouro_e_rodada = false;  // CHIMPS: sem fazendas, bancos, heroi de renda nem habilidade de dinheiro
    bool sandbox = false;           // dinheiro e vidas infinitos, sem fim, com os comandos X (so no solo)
};

struct Envio {
    std::string chave;
    std::string nome;
    std::string tipo;
    int qtd;
    double espaco;
    int custo;
    double eco;
    int rodada_min;
    bool camo = false;
    bool regen = false;
    bool fort = false;
};

// ---- dados (dados.cpp)
std::vector<TipoBloon> criar_bloons();
std::vector<DefTorre> criar_torres();
std::vector<DefTorre> criar_auxiliares();
std::vector<DefTorre> criar_herois();
std::vector<DefMapa> criar_mapas();
const std::map<std::string, std::string>& regen_proximo();
extern const std::vector<int> XP_NIVEL;
extern const std::map<int, std::vector<Grupo>> RODADAS;
extern const std::vector<Dificuldade> DIFICULDADES;
extern const std::vector<Envio> ENVIOS;

// ---- consultas (defs.cpp)
const std::vector<TipoBloon>& bloons();
const TipoBloon& tipo_bloon(const std::string& nome);  // lanca std::out_of_range
const TipoBloon* achar_bloon(const std::string& nome);
int rbe(int tipo_id, bool fortificado = false);

const std::vector<DefTorre>& torres();      // as 22 torres, na ordem da loja
const std::vector<DefTorre>& herois();      // os 18 herois
const DefTorre* achar_torre(const std::string& chave);   // so torres compraveis
const DefTorre* achar_heroi(const std::string& chave);
const DefTorre* achar_definicao(const std::string& chave);  // torre, heroi ou auxiliar
const DefTorre& definicao(const std::string& chave);        // lanca std::out_of_range

const std::vector<DefMapa>& mapas();
const DefMapa* achar_mapa(const std::string& chave);

const Dificuldade* achar_dificuldade(const std::string& chave);
const Envio* achar_envio(const std::string& chave);

}  // namespace bl
