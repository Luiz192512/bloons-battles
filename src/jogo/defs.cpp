#include "jogo/defs.hpp"

#include <stdexcept>

namespace bl {

DType dtype_de(const std::string& nome) {
    if (nome == "afiado") return DT_AFIADO;
    if (nome == "explosao") return DT_EXPLOSAO;
    if (nome == "gelo") return DT_GELO;
    if (nome == "energia") return DT_ENERGIA;
    if (nome == "normal") return DT_NORMAL;
    return 0;
}

namespace {

struct Catalogo {
    std::vector<TipoBloon> bloons;
    std::map<std::string, int> bloon_por_nome;
    std::vector<int> rbe, rbe_fort;
    std::vector<DefTorre> torres, herois, auxiliares;
    std::map<std::string, const DefTorre*> torre_por_chave, heroi_por_chave, def_por_chave;
    std::vector<DefMapa> mapas;

    int calc_rbe(int id, bool fort) {
        const TipoBloon& t = bloons[id];
        int vida = fort && t.vida_fortificado ? t.vida_fortificado : t.vida;
        for (int f : t.filhos_id) vida += calc_rbe(f, fort);
        return vida;
    }

    Catalogo() {
        bloons = criar_bloons();
        for (size_t i = 0; i < bloons.size(); ++i) {
            bloons[i].id = static_cast<int>(i);
            bloon_por_nome[bloons[i].nome] = static_cast<int>(i);
        }
        for (auto& b : bloons)
            for (auto& f : b.filhos) b.filhos_id.push_back(bloon_por_nome.at(f));
        for (size_t i = 0; i < bloons.size(); ++i) {
            rbe.push_back(calc_rbe(static_cast<int>(i), false));
            rbe_fort.push_back(calc_rbe(static_cast<int>(i), true));
        }
        torres = criar_torres();
        herois = criar_herois();
        auxiliares = criar_auxiliares();
        for (auto& t : torres) torre_por_chave[t.chave] = def_por_chave[t.chave] = &t;
        for (auto& t : herois) heroi_por_chave[t.chave] = def_por_chave[t.chave] = &t;
        for (auto& t : auxiliares) def_por_chave[t.chave] = &t;
        mapas = criar_mapas();
    }
};

const Catalogo& cat() {
    static const Catalogo c;  // inicializacao thread-safe (C++11)
    return c;
}

template <class M>
auto achar(const M& m, const std::string& k) -> typename M::mapped_type {
    auto it = m.find(k);
    return it == m.end() ? nullptr : it->second;
}

}  // namespace

const std::vector<TipoBloon>& bloons() { return cat().bloons; }

const TipoBloon* achar_bloon(const std::string& nome) {
    auto it = cat().bloon_por_nome.find(nome);
    return it == cat().bloon_por_nome.end() ? nullptr : &cat().bloons[it->second];
}

const TipoBloon& tipo_bloon(const std::string& nome) {
    return cat().bloons[cat().bloon_por_nome.at(nome)];
}

int rbe(int tipo_id, bool fortificado) {
    return fortificado ? cat().rbe_fort[tipo_id] : cat().rbe[tipo_id];
}

const std::vector<DefTorre>& torres() { return cat().torres; }
const std::vector<DefTorre>& herois() { return cat().herois; }
const DefTorre* achar_torre(const std::string& chave) { return achar(cat().torre_por_chave, chave); }
const DefTorre* achar_heroi(const std::string& chave) { return achar(cat().heroi_por_chave, chave); }
const DefTorre* achar_definicao(const std::string& chave) { return achar(cat().def_por_chave, chave); }

const DefTorre& definicao(const std::string& chave) {
    const DefTorre* d = achar_definicao(chave);
    if (!d) throw std::out_of_range("torre desconhecida: " + chave);
    return *d;
}

const std::vector<DefMapa>& mapas() { return cat().mapas; }

const DefMapa* achar_mapa(const std::string& chave) {
    for (auto& m : cat().mapas)
        if (m.chave == chave) return &m;
    return nullptr;
}

const Dificuldade* achar_dificuldade(const std::string& chave) {
    for (auto& d : DIFICULDADES)
        if (d.chave == chave) return &d;
    return nullptr;
}

const Envio* achar_envio(const std::string& chave) {
    for (auto& e : ENVIOS)
        if (e.chave == chave) return &e;
    return nullptr;
}

}  // namespace bl
