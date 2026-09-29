#include "cliente/som.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <random>
#include <vector>

#include "raylib.h"

namespace bl::som {

namespace {

constexpr int TAXA = 22050;
constexpr double PI_D = 3.14159265358979323846;

std::map<std::string, Sound> sons;
std::map<std::string, double> ultimo;
bool ativo = false;

// fn(t, f): t = segundos, f = fracao do som (0..1) -> amostra em [-1, 1]
void gerar(const std::string& nome, double dur, const std::function<double(double, double)>& fn) {
    const int n = static_cast<int>(TAXA * dur);
    std::vector<short> buf(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        double v = std::max(-1.0, std::min(1.0, fn(static_cast<double>(i) / TAXA, static_cast<double>(i) / n)));
        buf[static_cast<size_t>(i)] = static_cast<short>(v * 12000);
    }
    Wave w{static_cast<unsigned>(n), TAXA, 16, 1, buf.data()};
    sons[nome] = LoadSoundFromWave(w);  // copia os dados
}

double seno(double freq, double t) { return std::sin(2 * PI_D * freq * t); }

}  // namespace

void iniciar() {
    InitAudioDevice();
    if (!IsAudioDeviceReady()) return;  // sem placa de som: o jogo roda mudo
    ativo = true;
    std::mt19937 rng(3);
    auto ruido = [&] { return std::uniform_real_distribution<double>(-1, 1)(rng); };
    gerar("pop", 0.07, [&](double t, double f) {
        return ruido() * std::pow(1 - f, 3) + 0.6 * seno(900, t) * std::pow(1 - f, 4);
    });
    gerar("colocar", 0.18, [](double t, double f) { return seno(300 + 500 * f, t) * (1 - f); });
    gerar("upgrade", 0.35, [](double t, double f) {
        return (seno(520, t) + seno(780, t) + seno(1040 * (1 + f), t)) / 3 * (1 - f);
    });
    gerar("venda", 0.25, [](double t, double f) { return seno(900 - 500 * f, t) * (1 - f); });
    gerar("erro", 0.2, [](double t, double f) { return (seno(140, t) > 0 ? 1 : -1) * 0.5 * (1 - f); });
    gerar("explosao", 0.3, [&](double, double f) { return ruido() * std::pow(1 - f, 2); });
    gerar("rodada", 0.5, [](double t, double f) { return seno(f < 0.5 ? 440 : 660, t) * (1 - f) * 0.8; });
    gerar("vazou", 0.3, [](double t, double f) { return seno(200 - 120 * f, t) * (1 - f); });
    gerar("habilidade", 0.5, [](double t, double f) { return seno(300 + 900 * f, t) * (1 - f) * 0.8; });
    SetSoundVolume(sons["pop"], 0.35f);
    SetSoundVolume(sons["explosao"], 0.4f);
}

void liberar() {
    if (!ativo) return;
    for (auto& [n, s] : sons) UnloadSound(s);
    sons.clear();
    CloseAudioDevice();
    ativo = false;
}

void tocar(const std::string& nome, int intervalo_ms) {
    if (!ativo) return;
    auto it = sons.find(nome);
    if (it == sons.end()) return;
    const double agora = GetTime() * 1000;
    auto u = ultimo.find(nome);
    if (intervalo_ms && u != ultimo.end() && agora - u->second < intervalo_ms) return;
    ultimo[nome] = agora;
    PlaySound(it->second);
}

}  // namespace bl::som
