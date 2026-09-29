#include "jogo/mapas.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <mutex>
#include <stdexcept>

namespace bl {

constexpr double PI = 3.14159265358979323846;

Caminho::Caminho(std::vector<Ponto> pts) : pontos(std::move(pts)) {
    acum.push_back(0.0);
    for (size_t i = 1; i < pontos.size(); ++i)
        acum.push_back(acum.back() + std::hypot(pontos[i].x - pontos[i - 1].x, pontos[i].y - pontos[i - 1].y));
    comprimento = acum.back();
    for (double d = 0.0; d <= comprimento; d += 8.0) {
        Posicao p = posicao(d);
        amostras.push_back({d, p.x, p.y});
    }
}

Posicao Caminho::posicao(double d) const {
    const int n = static_cast<int>(pontos.size());
    int i;
    if (d <= 0) {
        i = 0;
    } else if (d >= comprimento) {
        i = n - 2;
    } else {
        i = static_cast<int>(std::upper_bound(acum.begin(), acum.end(), d) - acum.begin()) - 1;
    }
    i = std::max(0, std::min(i, n - 2));
    const Ponto& a = pontos[i];
    const Ponto& b = pontos[i + 1];
    double seg = acum[i + 1] - acum[i];
    if (seg == 0) seg = 1.0;
    double t = (d - acum[i]) / seg;
    double ang = std::atan2(b.y - a.y, b.x - a.x) * 180.0 / PI;
    return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, ang};
}

double Caminho::distancia_ponto(double x, double y) const {
    double melhor = 1e9;
    for (size_t i = 1; i < pontos.size(); ++i) {
        double x1 = pontos[i - 1].x, y1 = pontos[i - 1].y;
        double dx = pontos[i].x - x1, dy = pontos[i].y - y1;
        double L2 = dx * dx + dy * dy;
        if (L2 == 0) L2 = 1.0;
        double t = std::max(0.0, std::min(1.0, ((x - x1) * dx + (y - y1) * dy) / L2));
        melhor = std::min(melhor, std::hypot(x - (x1 + dx * t), y - (y1 + dy * t)));
    }
    return melhor;
}

std::vector<double> Caminho::distancias_no_raio(double x, double y, double r) const {
    std::vector<double> out;
    const double r2 = r * r;
    for (const Amostra& a : amostras)
        if ((a.x - x) * (a.x - x) + (a.y - y) * (a.y - y) <= r2 && 0 < a.d && a.d < comprimento)
            out.push_back(a.d);
    return out;
}

Mapa::Mapa(const DefMapa& d) : def(d) {
    for (auto& tr : d.trilhas) {
        std::vector<Ponto> pts;
        for (auto& [x, y] : tr) pts.push_back({x, y});
        caminhos.emplace_back(std::move(pts));
    }
}

bool Mapa::eh_agua(double x, double y) const {
    for (auto& [cx, cy, r] : def.agua)
        if ((x - cx) * (x - cx) + (y - cy) * (y - cy) <= r * r) return true;
    for (auto& [rx, ry, rw, rh] : def.agua_ret)
        if (rx <= x && x <= rx + rw && ry <= y && y <= ry + rh) return true;
    return false;
}

bool Mapa::na_trilha(double x, double y, double raio) const {
    for (auto& c : caminhos)
        if (c.distancia_ponto(x, y) < LARGURA_TRILHA / 2 + raio) return true;
    return false;
}

bool Mapa::bloqueado(double x, double y, double raio) const {
    for (auto& o : def.obstaculos)
        if (std::hypot(x - o.x, y - o.y) < o.r + raio * 0.6) return true;
    return false;
}

const Mapa& mapa(const std::string& chave) {
    static std::vector<std::unique_ptr<Mapa>> prontos;
    static std::once_flag uma_vez;
    std::call_once(uma_vez, [] {
        for (auto& d : mapas()) prontos.push_back(std::make_unique<Mapa>(d));
    });
    for (auto& m : prontos)
        if (m->def.chave == chave) return *m;
    throw std::out_of_range("mapa desconhecido: " + chave);
}

}  // namespace bl
