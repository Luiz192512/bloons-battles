#include "cliente/anim.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

#include "cliente/sprites.hpp"

namespace bl::anim {

namespace {
constexpr float PI_F = 3.14159265358979f;
}

// ---------------------------------------------------------------- curvas
float suavizar(Curva c, float x) {
    x = std::clamp(x, 0.0f, 1.0f);
    switch (c) {
        case Curva::LINEAR: return x;
        case Curva::ENTRA_QUAD: return x * x;
        case Curva::SAI_QUAD: return 1 - (1 - x) * (1 - x);
        case Curva::SAI_CUBICA: return 1 - (1 - x) * (1 - x) * (1 - x);
        case Curva::ENTRA_SAI_CUBICA: return x < 0.5f ? 4 * x * x * x : 1 - std::pow(-2 * x + 2, 3.0f) / 2;
        case Curva::SAI_VOLTA: {
            const float c1 = 1.70158f, c3 = c1 + 1;
            return 1 + c3 * std::pow(x - 1, 3.0f) + c1 * std::pow(x - 1, 2.0f);
        }
        case Curva::SAI_ELASTICA: {
            if (x <= 0 || x >= 1) return x;
            const float c4 = 2 * PI_F / 3;
            return std::pow(2.0f, -10 * x) * std::sin((x * 10 - 0.75f) * c4) + 1;
        }
    }
    return x;
}

const char* nome_curva(Curva c) {
    switch (c) {
        case Curva::LINEAR: return "linear";
        case Curva::ENTRA_QUAD: return "entra quad";
        case Curva::SAI_QUAD: return "sai quad";
        case Curva::SAI_CUBICA: return "sai cubica";
        case Curva::ENTRA_SAI_CUBICA: return "entra/sai cubica";
        case Curva::SAI_VOLTA: return "sai com volta";
        case Curva::SAI_ELASTICA: return "sai elastica";
    }
    return "?";
}

const char* nome_canal(Canal c) {
    static const char* nomes[N_CANAIS] = {"recuo",  "giro",  "estica", "flash", "escala",
                                          "salto",  "corpo", "brilho", "onda",  "onda alfa"};
    return nomes[c];
}

float padrao_canal(Canal c) { return c == ESCALA ? 1.0f : 0.0f; }

float Trilha::valor(float t, float padrao) const {
    if (chaves.empty()) return padrao;
    if (t <= chaves.front().t) return chaves.front().v;
    for (size_t i = 1; i < chaves.size(); ++i) {
        const Chave& a = chaves[i - 1];
        const Chave& b = chaves[i];
        if (t <= b.t) {
            const float u = b.t > a.t ? (t - a.t) / (b.t - a.t) : 1;
            return a.v + (b.v - a.v) * suavizar(b.curva, u);
        }
    }
    return chaves.back().v;
}

Quadro avaliar(const Clipe& c, float t) {
    auto v = [&](Canal k) { return c.trilhas[k].valor(t, padrao_canal(k)); };
    Quadro q;
    q.pose.ativa = true;
    q.pose.recuo = v(RECUO);
    q.pose.giro = v(GIRO);
    q.pose.estica = v(ESTICA);
    q.pose.flash = v(FLASH);
    q.escala = v(ESCALA);
    q.salto = v(SALTO);
    q.corpo_recuo = v(CORPO_RECUO);
    q.brilho = v(BRILHO);
    q.onda = v(ONDA);
    q.onda_alfa = v(ONDA_ALFA);
    return q;
}

// ---------------------------------------------------------------- biblioteca
namespace {

using C = Curva;

// Disparos: curtos (0,2 a 0,45 s), com antecipacao, golpe rapido e volta com "overshoot".
Clipe arremesso() {
    Clipe c{"disparo: arremesso", 0.28f, {}};
    c.chave(GIRO, 0, 0).chave(GIRO, 0.06f, 26, C::SAI_QUAD).chave(GIRO, 0.12f, -34, C::SAI_QUAD).chave(GIRO, 0.28f, 0, C::SAI_VOLTA);
    c.chave(ESTICA, 0, 0).chave(ESTICA, 0.06f, -4, C::SAI_QUAD).chave(ESTICA, 0.12f, 9, C::SAI_QUAD).chave(ESTICA, 0.28f, 0, C::SAI_VOLTA);
    c.chave(CORPO_RECUO, 0, 0).chave(CORPO_RECUO, 0.12f, -1.5f, C::SAI_QUAD).chave(CORPO_RECUO, 0.28f, 0, C::SAI_CUBICA);
    return c;
}

Clipe arco() {
    Clipe c{"disparo: arco", 0.32f, {}};
    c.chave(ESTICA, 0, 0).chave(ESTICA, 0.12f, -9, C::SAI_QUAD).chave(ESTICA, 0.15f, 4, C::SAI_QUAD).chave(ESTICA, 0.32f, 0, C::SAI_VOLTA);
    c.chave(GIRO, 0, 0).chave(GIRO, 0.12f, 6, C::SAI_QUAD).chave(GIRO, 0.32f, 0, C::SAI_VOLTA);
    c.chave(FLASH, 0.12f, 0).chave(FLASH, 0.15f, 0.5f).chave(FLASH, 0.26f, 0, C::SAI_QUAD);
    return c;
}

Clipe tiro() {
    Clipe c{"disparo: tiro", 0.22f, {}};
    c.chave(ESTICA, 0, 0).chave(ESTICA, 0.03f, -8, C::SAI_QUAD).chave(ESTICA, 0.22f, 0, C::SAI_VOLTA);
    c.chave(GIRO, 0, 0).chave(GIRO, 0.03f, -5, C::SAI_QUAD).chave(GIRO, 0.22f, 0, C::SAI_CUBICA);
    c.chave(FLASH, 0, 1).chave(FLASH, 0.11f, 0, C::SAI_QUAD);
    c.chave(CORPO_RECUO, 0, 0).chave(CORPO_RECUO, 0.03f, 2.5f, C::SAI_QUAD).chave(CORPO_RECUO, 0.22f, 0, C::SAI_CUBICA);
    return c;
}

Clipe magia() {
    Clipe c{"disparo: magia", 0.34f, {}};
    c.chave(GIRO, 0, 0).chave(GIRO, 0.1f, -24, C::ENTRA_SAI_CUBICA).chave(GIRO, 0.34f, 0, C::SAI_VOLTA);
    c.chave(ESTICA, 0, 0).chave(ESTICA, 0.1f, 6, C::SAI_QUAD).chave(ESTICA, 0.34f, 0, C::SAI_VOLTA);
    c.chave(FLASH, 0, 0).chave(FLASH, 0.08f, 0.85f, C::SAI_QUAD).chave(FLASH, 0.26f, 0, C::ENTRA_QUAD);
    c.chave(ESCALA, 0, 1).chave(ESCALA, 0.1f, 1.04f, C::SAI_QUAD).chave(ESCALA, 0.34f, 1, C::SAI_CUBICA);
    return c;
}

Clipe canhao() {
    Clipe c{"disparo: canhao", 0.45f, {}};
    c.chave(RECUO, 0, 0).chave(RECUO, 0.04f, 1, C::SAI_QUAD).chave(RECUO, 0.45f, 0, C::SAI_CUBICA);
    c.chave(ESCALA, 0, 1).chave(ESCALA, 0.04f, 0.93f, C::SAI_QUAD).chave(ESCALA, 0.3f, 1, C::SAI_VOLTA);
    c.chave(CORPO_RECUO, 0, 0).chave(CORPO_RECUO, 0.04f, 3, C::SAI_QUAD).chave(CORPO_RECUO, 0.4f, 0, C::SAI_CUBICA);
    return c;
}

Clipe pulso() {
    Clipe c{"disparo: pulso", 0.3f, {}};
    c.chave(ESCALA, 0, 1).chave(ESCALA, 0.05f, 1.1f, C::SAI_QUAD).chave(ESCALA, 0.3f, 1, C::SAI_VOLTA);
    c.chave(RECUO, 0, 0).chave(RECUO, 0.05f, 1, C::SAI_QUAD).chave(RECUO, 0.3f, 0, C::SAI_CUBICA);
    return c;
}

// Habilidades: antecipacao (agacha), pico (cresce, braco erguido, clarao), onda de choque e volta elastica.
Clipe habilidade(const char* nome, float dur, float escala_pico, float raio_onda_fim) {
    Clipe c{nome, dur, {}};
    const float pico = dur * 0.25f;
    c.chave(ESCALA, 0, 1).chave(ESCALA, dur * 0.1f, 0.88f, C::SAI_QUAD).chave(ESCALA, pico, escala_pico, C::SAI_VOLTA)
        .chave(ESCALA, dur * 0.8f, 1, C::SAI_ELASTICA);
    c.chave(SALTO, 0, 0).chave(SALTO, dur * 0.1f, 2, C::SAI_QUAD).chave(SALTO, pico, -10, C::SAI_QUAD).chave(SALTO, dur * 0.55f, 0, C::ENTRA_QUAD);
    c.chave(GIRO, 0, 0).chave(GIRO, pico, -70, C::SAI_VOLTA).chave(GIRO, dur * 0.7f, -70).chave(GIRO, dur, 0, C::ENTRA_SAI_CUBICA);
    c.chave(ESTICA, 0, 0).chave(ESTICA, pico, 8, C::SAI_VOLTA).chave(ESTICA, dur, 0, C::ENTRA_SAI_CUBICA);
    c.chave(FLASH, 0, 0).chave(FLASH, pico, 1, C::SAI_QUAD).chave(FLASH, dur * 0.6f, 0, C::ENTRA_QUAD);
    c.chave(RECUO, 0, 0).chave(RECUO, pico, 1, C::SAI_QUAD).chave(RECUO, dur * 0.7f, 0, C::SAI_CUBICA);
    c.chave(BRILHO, 0, 0).chave(BRILHO, pico, 1, C::SAI_QUAD).chave(BRILHO, dur, 0, C::ENTRA_QUAD);
    c.chave(ONDA, 0, 0).chave(ONDA, pico, 0.15f).chave(ONDA, dur, raio_onda_fim, C::SAI_CUBICA);
    c.chave(ONDA_ALFA, 0, 0).chave(ONDA_ALFA, pico, 1, C::SAI_QUAD).chave(ONDA_ALFA, dur, 0, C::ENTRA_QUAD);
    return c;
}

struct Biblioteca {
    Clipe arremesso = anim::arremesso(), arco = anim::arco(), tiro = anim::tiro(), magia = anim::magia(),
          canhao = anim::canhao(), pulso = anim::pulso();
    std::map<std::string, Clipe> habs;
    std::vector<const Clipe*> todos;
    Biblioteca() {
        habs["turbo"] = habilidade("habilidade: turbo", 0.7f, 1.15f, 0.7f);
        habs["turbo_area"] = habilidade("habilidade: turbo em area", 0.9f, 1.2f, 1.0f);
        habs["dano_global"] = habilidade("habilidade: dano global", 1.0f, 1.25f, 1.0f);
        habs["dano_forte"] = habilidade("habilidade: dano forte", 0.8f, 1.2f, 0.6f);
        habs["congelar_global"] = habilidade("habilidade: congela tudo", 1.1f, 1.18f, 1.0f);
        habs["lentidao"] = habilidade("habilidade: lentidao", 1.0f, 1.15f, 1.0f);
        habs["dinheiro"] = habilidade("habilidade: dinheiro", 0.8f, 1.2f, 0.5f);
        habs["spikes_local"] = habilidade("habilidade: espinhos", 0.8f, 1.15f, 0.6f);
        habs["spikes_global"] = habilidade("habilidade: espinhos global", 0.9f, 1.18f, 1.0f);
        habs["invocar"] = habilidade("habilidade: invocar", 0.9f, 1.2f, 0.5f);
        habs["reverso"] = habilidade("habilidade: reverso", 1.1f, 1.25f, 1.0f);
        habs["roubo"] = habilidade("habilidade: roubo", 0.8f, 1.15f, 0.6f);
        for (auto& [k, c] : habs) c.efeito = k;
        for (const Clipe* c : {&arremesso, &arco, &tiro, &magia, &canhao, &pulso}) todos.push_back(c);
        for (auto& [k, c] : habs) todos.push_back(&c);
    }
};

const Biblioteca& bib() {
    static const Biblioteca b;
    return b;
}

}  // namespace

const Clipe& clipe_disparo(const std::string& k) {
    static const std::set<std::string> arremesso{"dardo", "bumerangue", "ninja", "sauda", "pat", "alquimista", "cola", "brickell"};
    static const std::set<std::string> arco{"quincy"};
    static const std::set<std::string> tiro{"sniper", "dartling", "engenheiro", "striker", "rosalia", "jericho"};
    static const std::set<std::string> canhao{"bomba", "sentinela", "churchill", "morteiro", "submarino", "bucaneiro"};
    static const std::set<std::string> pulso{"tachinha", "espinhos", "heli", "as", "fenix", "vila", "fazenda"};
    const Biblioteca& b = bib();
    if (arremesso.count(k)) return b.arremesso;
    if (arco.count(k)) return b.arco;
    if (tiro.count(k)) return b.tiro;
    if (canhao.count(k)) return b.canhao;
    if (pulso.count(k)) return b.pulso;
    return b.magia;  // magos, druidas, gelo, super e a maioria dos herois
}

const Clipe& clipe_habilidade(const std::string& efeito) {
    auto it = bib().habs.find(efeito);
    return it != bib().habs.end() ? it->second : bib().habs.at("invocar");
}

const std::vector<const Clipe*>& todos_os_clipes() { return bib().todos; }

// ---------------------------------------------------------------- mira 3/4
TipoMira tipo_mira(const std::string& k) {
    static const std::set<std::string> fixa{"tachinha", "vila", "fazenda", "espinhos", "morteiro"};
    static const std::set<std::string> torreta{"bomba", "sentinela", "churchill"};
    static const std::set<std::string> espelha{"submarino", "bucaneiro", "as", "heli", "fenix"};
    if (fixa.count(k)) return TipoMira::FIXA;
    if (torreta.count(k)) return TipoMira::TORRETA;
    if (espelha.count(k)) return TipoMira::ESPELHA;
    return TipoMira::MACACO;
}

namespace {
float graus(float rad) { return rad * 180 / PI_F; }
// aproxima 'atual' de 'alvo' (angulos em graus, pelo caminho mais curto)
float aproximar_angulo(float atual, float alvo, float k) {
    float d = std::fmod(alvo - atual + 540.0f, 360.0f) - 180.0f;
    return atual + d * k;
}
}  // namespace

void Mira::atualizar(const std::string& chave, double ang_graus, double agora) {
    tipo_ = tipo_mira(chave);
    const float a = static_cast<float>(ang_graus) * PI_F / 180;
    const float dx = std::cos(a), dy = std::sin(a);
    // histerese: so troca de lado quando o alvo passa ~10 graus do eixo vertical
    int lado = lado_;
    if (dx > 0.17f) lado = 1;
    else if (dx < -0.17f) lado = -1;
    const float braco_alvo = std::clamp(graus(std::atan2(lado * dx, -dy)) - 18, BRACO_MIN, BRACO_MAX);
    const float inclina_alvo = INCLINA_MAX * std::clamp(dx, -1.0f, 1.0f);
    const float torreta_alvo = graus(std::atan2(dx, -dy));
    if (!iniciada_) {
        iniciada_ = true;
        lado_ = lado_antes_ = lado;
        braco_ = braco_alvo, inclina_ = inclina_alvo, torreta_ = torreta_alvo;
        ultimo_ = agora;
        return;
    }
    if (lado != lado_) {
        lado_antes_ = lado_;
        lado_ = lado;
        t_virada_ = agora;
    }
    const float dt = static_cast<float>(std::clamp(agora - ultimo_, 0.0, 0.1));
    ultimo_ = agora;
    const float k = 1 - std::exp(-dt * 12);
    braco_ += (braco_alvo - braco_) * k;
    inclina_ += (inclina_alvo - inclina_) * (1 - std::exp(-dt * 6));
    torreta_ = aproximar_angulo(torreta_, torreta_alvo, 1 - std::exp(-dt * 10));
}

void Mira::aplicar(Quadro& q, double agora) const {
    if (!iniciada_ || tipo_ == TipoMira::FIXA) return;
    if (tipo_ == TipoMira::TORRETA) {
        q.pose.torreta = torreta_;
        return;
    }
    // virada: a escala X encolhe ate quase sumir e abre do outro lado com uma pequena volta
    const float u = static_cast<float>((agora - t_virada_) / DUR_VIRADA);
    if (u >= 1) q.sx = static_cast<float>(lado_);
    else if (u < 0.5f) q.sx = lado_antes_ * (1 - 1.7f * u);
    else q.sx = lado_ * (0.15f + 0.85f * suavizar(Curva::SAI_VOLTA, (u - 0.5f) * 2));
    if (tipo_ == TipoMira::MACACO) {
        q.pose.mirando = true;
        q.pose.mira = braco_;
        q.inclina = inclina_;
    }
}

// ---------------------------------------------------------------- animador
void Animador::observar(const Pista& pista, double agora) {
    std::set<int> vivas;
    for (const auto& [id, tp] : pista.torres) {
        const Torre& t = *tp;
        vivas.insert(id);
        auto [it, nova] = est_.try_emplace(id);
        Estado& e = it->second;
        if (!nova && e.rec.size() == t.recargas.size()) {
            for (size_t i = 0; i < t.recargas.size(); ++i) {
                if (t.recargas[i] <= e.rec[i] + 1e-9) continue;
                // atirou: recomeca o clipe, a nao ser que ele esteja no comeco (tiro muito rapido)
                const Clipe& c = clipe_disparo(t.chave);
                if (!e.disparo || agora - e.t_disparo > c.dur * 0.45) {
                    e.disparo = &c;
                    e.t_disparo = agora;
                }
                break;
            }
        }
        if (!nova && e.hab.size() == t.hab_rec.size()) {
            for (size_t i = 0; i < t.hab_rec.size(); ++i) {
                if (t.hab_rec[i] <= e.hab[i] + 0.5) continue;
                const std::string efeito = t.st.habs[i].value("tipo", std::string("invocar"));
                e.hab_clipe = &clipe_habilidade(efeito);
                e.t_hab = agora;
                e.cor = spr::cor_efeito(efeito);
            }
        }
        e.rec = t.recargas;
        e.hab = t.hab_rec;
        e.mira.atualizar(t.chave, t.ang, agora);
    }
    for (auto it = est_.begin(); it != est_.end();) it = vivas.count(it->first) ? std::next(it) : est_.erase(it);
}

Quadro Animador::quadro(int id, double agora) const {
    auto it = est_.find(id);
    if (it == est_.end()) return {};
    const Estado& e = it->second;
    Quadro q;
    // a habilidade tem prioridade sobre o disparo
    if (e.hab_clipe && agora - e.t_hab < e.hab_clipe->dur) {
        q = avaliar(*e.hab_clipe, static_cast<float>(agora - e.t_hab));
        q.cor = e.cor;
        q.habilidade = true;
    } else if (e.disparo && agora - e.t_disparo < e.disparo->dur) {
        q = avaliar(*e.disparo, static_cast<float>(agora - e.t_disparo));
    }
    e.mira.aplicar(q, agora);
    return q;
}

void Animador::tocar_disparo(int id, const std::string& chave, double agora) {
    Estado& e = est_[id];
    e.disparo = &clipe_disparo(chave);
    e.t_disparo = agora;
}

void Animador::tocar_habilidade(int id, const std::string& efeito, double agora) {
    Estado& e = est_[id];
    e.hab_clipe = &clipe_habilidade(efeito);
    e.t_hab = agora;
    e.cor = spr::cor_efeito(efeito);
}

}  // namespace bl::anim
