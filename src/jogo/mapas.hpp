// Mapas: trilhas (polilinhas), agua e obstaculos. Coordenadas em 1040x720.
#pragma once

#include <string>
#include <vector>

#include "jogo/defs.hpp"

namespace bl {

constexpr int LARGURA_MAPA = 1040;
constexpr int ALTURA_MAPA = 720;
constexpr double LARGURA_TRILHA = 46;

struct Ponto {
    double x, y;
};

struct Posicao {
    double x, y, ang;  // angulo em graus
};

struct Amostra {
    double d, x, y;
};

// Polilinha com consulta de posicao por distancia percorrida.
class Caminho {
public:
    explicit Caminho(std::vector<Ponto> pontos);

    Posicao posicao(double d) const;
    double distancia_ponto(double x, double y) const;
    std::vector<double> distancias_no_raio(double x, double y, double r) const;

    std::vector<Ponto> pontos;
    std::vector<double> acum;
    double comprimento = 0;
    std::vector<Amostra> amostras;  // a cada 8 px, para busca rapida de pontos proximos
};

class Mapa {
public:
    explicit Mapa(const DefMapa& def);

    bool eh_agua(double x, double y) const;
    bool na_trilha(double x, double y, double raio) const;
    bool bloqueado(double x, double y, double raio) const;

    const DefMapa& def;
    std::vector<Caminho> caminhos;
};

// Mapas prontos (um por chave), criados na primeira consulta.
const Mapa& mapa(const std::string& chave);  // lanca std::out_of_range

}  // namespace bl
