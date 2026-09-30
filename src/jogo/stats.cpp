#include "jogo/stats.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <stdexcept>

namespace bl {

namespace {

// Efeitos que somam, multiplicam, ou valem para a torre inteira (e nao para um ataque)
const std::set<std::string> SOMA = {"dano", "pierce", "n", "splash", "sdano", "spierce", "quica", "moab", "cer",
                                    "fort", "dist", "raio_proj", "valor", "pilha_pierce", "saltos", "empurra",
                                    "fragiliza", "nivel_inv", "raio_aura"};
const std::set<std::string> MULT = {"cad", "vel", "pilha_vida", "impreciso"};
const std::set<std::string> NIVEL_TORRE = {"alcance", "alcance_x", "camo", "ouro", "ouro_chumbo", "desconto", "venda", "hab",
                                           "persegue"};

const std::map<std::string, double Ataque::*> NUMEROS = {
    {"cad", &Ataque::cad}, {"dano", &Ataque::dano}, {"pierce", &Ataque::pierce}, {"vel", &Ataque::vel},
    {"dist", &Ataque::dist}, {"n", &Ataque::n}, {"spread", &Ataque::spread}, {"raio_proj", &Ataque::raio_proj},
    {"splash", &Ataque::splash}, {"sdano", &Ataque::sdano}, {"spierce", &Ataque::spierce},
    {"moab", &Ataque::moab}, {"cer", &Ataque::cer}, {"fort", &Ataque::fort}, {"congela", &Ataque::congela},
    {"atordoa", &Ataque::atordoa}, {"empurra", &Ataque::empurra}, {"fragiliza", &Ataque::fragiliza},
    {"quica", &Ataque::quica}, {"raio_aura", &Ataque::raio_aura}, {"pilha_pierce", &Ataque::pilha_pierce},
    {"pilha_vida", &Ataque::pilha_vida}, {"saltos", &Ataque::saltos}, {"valor", &Ataque::valor},
    {"fusivel", &Ataque::fusivel}, {"impreciso", &Ataque::impreciso}, {"dur", &Ataque::dur},
    {"nivel_inv", &Ataque::nivel_inv}, {"crit_cada", &Ataque::crit_cada}, {"crit_max", &Ataque::crit_max},
    {"crit_dano", &Ataque::crit_dano}, {"crit_mais", &Ataque::crit_mais}, {"pocao_max", &Ataque::pocao_max},
    {"pocao_bloq", &Ataque::pocao_bloq},
};

const std::map<std::string, bool Ataque::*> LOGICOS = {
    {"busca", &Ataque::busca}, {"boom", &Ataque::boom}, {"retira_camo", &Ataque::retira_camo},
    {"retira_regen", &Ataque::retira_regen}, {"global_", &Ataque::global_}, {"so_moab", &Ataque::so_moab},
    {"moab_lento", &Ataque::moab_lento}, {"moab_congela", &Ataque::moab_congela},
    {"moab_cola", &Ataque::moab_cola}, {"moab_atordoa", &Ataque::moab_atordoa},
    {"na_trilha", &Ataque::na_trilha}, {"linha", &Ataque::linha},
    {"armadilha", &Ataque::armadilha}, {"prende_moab", &Ataque::prende_moab}, {"pocao", &Ataque::pocao},
};

TipoAtaque tipo_de(const std::string& s) {
    static const std::map<std::string, TipoAtaque> M = {
        {"projetil", TipoAtaque::PROJETIL}, {"radial", TipoAtaque::RADIAL}, {"aura", TipoAtaque::AURA},
        {"hitscan", TipoAtaque::HITSCAN},   {"cadeia", TipoAtaque::CADEIA}, {"morteiro", TipoAtaque::MORTEIRO},
        {"pilha", TipoAtaque::PILHA},       {"queda", TipoAtaque::QUEDA},   {"renda", TipoAtaque::RENDA},
        {"buff", TipoAtaque::BUFF},         {"invocar", TipoAtaque::INVOCAR},
    };
    return M.at(s);
}

// Um filtro "a" em texto escolhe os ataques pelo tipo ("projetil") ou pelo visual ("uva")
bool casa(const Ataque& at, const std::string& filtro) {
    static const std::map<TipoAtaque, std::string> NOMES = {
        {TipoAtaque::PROJETIL, "projetil"}, {TipoAtaque::RADIAL, "radial"}, {TipoAtaque::AURA, "aura"},
        {TipoAtaque::HITSCAN, "hitscan"},   {TipoAtaque::CADEIA, "cadeia"}, {TipoAtaque::MORTEIRO, "morteiro"},
        {TipoAtaque::PILHA, "pilha"},       {TipoAtaque::QUEDA, "queda"},   {TipoAtaque::RENDA, "renda"},
        {TipoAtaque::BUFF, "buff"},         {TipoAtaque::INVOCAR, "invocar"},
    };
    return filtro == "todos" || at.visual == filtro || NOMES.at(at.tipo) == filtro;
}

double num(const J& v) { return v.is_boolean() ? (v.get<bool>() ? 1.0 : 0.0) : v.get<double>(); }

// Define um campo do ataque (modo '=' substitui, '+' soma, '*' multiplica).
void campo(Ataque& at, const std::string& k, const J& v, char modo) {
    auto itn = NUMEROS.find(k);
    if (itn != NUMEROS.end()) {
        double& alvo = at.*(itn->second);
        if (modo == '+') alvo += num(v);
        else if (modo == '*') alvo *= num(v);
        else alvo = num(v);
        return;
    }
    auto itl = LOGICOS.find(k);
    if (itl != LOGICOS.end()) {
        at.*(itl->second) = v.is_boolean() ? v.get<bool>() : num(v) != 0;
        return;
    }
    if (k == "tipo") {
        at.tipo = tipo_de(v.get<std::string>());
    } else if (k == "dtype") {
        at.dtype = dtype_de(v.get<std::string>());
    } else if (k == "sdtype") {
        at.sdtype = v.is_null() ? 0 : dtype_de(v.get<std::string>());
    } else if (k == "visual") {
        at.visual = v.get<std::string>();
    } else if (k == "alvo") {
        at.alvo_forte = v.is_string() && v.get<std::string>() == "forte";
    } else if (k == "base") {
        at.base = v.is_null() ? "" : v.get<std::string>();
    } else if (k == "lento") {
        at.tem_lento = !v.is_null();
        if (at.tem_lento) at.lento_f = num(v[0]), at.lento_t = num(v[1]);
    } else if (k == "cola") {
        at.tem_cola = !v.is_null();
        if (at.tem_cola) at.cola_f = num(v[0]), at.cola_t = num(v[1]), at.cola_dps = num(v[2]);
    } else if (k == "queima") {
        at.tem_queima = !v.is_null();
        if (at.tem_queima) at.queima_dps = num(v[0]), at.queima_t = num(v[1]);
    } else if (k == "frag") {
        if (v.is_null()) {
            at.frag.reset();
            at.frag_n = 0;
        } else {
            J d = {{"tipo", "projetil"}, {"vel", 520}, {"dist", 110}};
            for (auto& [fk, fv] : v.items()) d[fk] = fv;
            at.frag = std::make_shared<const Ataque>(novo_ataque(d));
            at.frag_n = static_cast<int>(v.value("n", 6.0));
        }
    } else if (k == "buffs") {
        at.buffs = Buffs{};
        if (!v.is_null()) at.buffs.mesclar(v);
    } else {
        throw std::invalid_argument("efeito desconhecido: " + k);
    }
}

}  // namespace

void Buffs::mesclar(const Buffs& o) {
    if (o.vazio) return;
    cad *= o.cad;
    alcance_pct += o.alcance_pct;
    alcance += o.alcance;
    pierce += o.pierce;
    pierce_pct += o.pierce_pct;
    vel_pct += o.vel_pct;
    dano += o.dano;
    moab += o.moab;
    cer += o.cer;
    fort += o.fort;
    ouro += o.ouro;
    camo = camo || o.camo;
    dtype_normal = dtype_normal || o.dtype_normal;
    chumbo = chumbo || o.chumbo;
    vazio = false;
}

void Buffs::melhor(const Buffs& o) {
    if (o.vazio) return;
    if (vazio) {
        *this = o;
        return;
    }
    cad = std::min(cad, o.cad);
    for (auto campo : {&Buffs::alcance_pct, &Buffs::alcance, &Buffs::pierce, &Buffs::pierce_pct, &Buffs::vel_pct,
                       &Buffs::dano, &Buffs::moab, &Buffs::cer, &Buffs::fort, &Buffs::ouro})
        this->*campo = std::max(this->*campo, o.*campo);
    camo = camo || o.camo;
    dtype_normal = dtype_normal || o.dtype_normal;
    chumbo = chumbo || o.chumbo;
}

void Buffs::mesclar(const J& novos) {
    for (auto& [k, v] : novos.items()) {
        if (k == "escopo") escopo = v.get<std::string>();
        else if (k == "global_") global_ = v.get<bool>();
        else if (k == "sem_si") sem_si = v.get<bool>();
        else if (k == "acumula") acumula = v.get<int>();
        else {
            vazio = false;
            if (k == "cad") cad *= num(v);
            else if (k == "alcance_pct") alcance_pct += num(v);
            else if (k == "alcance") alcance += num(v);
            else if (k == "pierce") pierce += num(v);
            else if (k == "pierce_pct") pierce_pct += num(v);
            else if (k == "vel_pct") vel_pct += num(v);
            else if (k == "dano") dano += num(v);
            else if (k == "moab") moab += num(v);
            else if (k == "cer") cer += num(v);
            else if (k == "fort") fort += num(v);
            else if (k == "ouro") ouro += num(v);
            else if (k == "camo") camo = v.get<bool>();
            else if (k == "dtype_normal") dtype_normal = v.get<bool>();
            else if (k == "chumbo") chumbo = v.get<bool>();
            else throw std::invalid_argument("buff desconhecido: " + k);
        }
    }
}

Ataque novo_ataque(const J& d) {
    Ataque at;
    for (auto& [k, v] : d.items()) campo(at, k, v, '=');
    return at;
}

void aplicar(Stats& st, const J& ef) {
    // uma lista de efeitos e aplicada em ordem (permite mirar ataques diferentes no mesmo upgrade)
    if (ef.is_array()) {
        for (const J& e : ef) aplicar(st, e);
        return;
    }
    // indices dos ataques afetados; um filtro em texto inclui os criados durante este mesmo efeito
    std::string filtro;
    std::vector<size_t> alvos;
    auto a = ef.find("a");
    if (a != ef.end() && a->is_string()) {
        filtro = a->get<std::string>();
    } else {
        size_t idx = a == ef.end() ? 0 : a->get<size_t>();
        if (idx < st.ataques.size()) alvos.push_back(idx);
    }

    for (auto& [k, v] : ef.items()) {
        if (k == "a") continue;
        if (NIVEL_TORRE.count(k)) {
            if (k == "alcance") st.alcance += num(v);
            else if (k == "alcance_x") st.alcance *= num(v);
            else if (k == "ouro") st.ouro += num(v);
            else if (k == "ouro_chumbo") st.ouro_chumbo += num(v);
            else if (k == "desconto") st.desconto = std::max(st.desconto, num(v));
            else if (k == "hab") st.habs.push_back(v);
            else if (k == "camo") st.camo = v.get<bool>();
            else if (k == "venda") st.venda = num(v);
            else if (k == "persegue") st.persegue = v.get<bool>();
            continue;
        }
        if (k == "novo") {
            st.ataques.push_back(novo_ataque(v));
            continue;
        }
        if (k == "subst") {
            st.ataques[0] = novo_ataque(v);
            continue;
        }
        if (!filtro.empty()) {
            alvos.clear();
            for (size_t i = 0; i < st.ataques.size(); ++i)
                if (casa(st.ataques[i], filtro)) alvos.push_back(i);
        }
        for (size_t i : alvos) {
            Ataque& at = st.ataques[i];
            if (k == "buffs") at.buffs.mesclar(v);
            else if (k == "valor_x") at.valor *= num(v);
            else if (SOMA.count(k)) campo(at, k, v, '+');
            else if (MULT.count(k)) campo(at, k, v, '*');
            else campo(at, k, v, '=');
        }
    }
}

Stats calcular(const std::string& chave, std::array<int, 3> caminhos, int nivel) {
    const DefTorre& dfn = definicao(chave);
    Stats st;
    st.alcance = dfn.alcance;
    st.camo = dfn.camo;
    for (const J& a : dfn.ataques) st.ataques.push_back(novo_ataque(a));
    if (!dfn.caminhos.empty())
        for (int p = 0; p < 3; ++p)
            for (int i = 0; i < caminhos[p]; ++i) aplicar(st, dfn.caminhos[p][i].ef);
    if (dfn.heroi) {
        for (int n = 2; n <= nivel; ++n) {
            auto it = dfn.niveis.find(n);
            if (it != dfn.niveis.end() && !it->second.empty()) aplicar(st, it->second);
        }
        if (nivel >= 3 && !dfn.hab3.is_null()) st.habs.push_back(dfn.hab3);
        if (nivel >= 10 && !dfn.hab10.is_null()) st.habs.push_back(dfn.hab10);
    }
    return st;
}

bool pode_upar(std::array<int, 3> caminhos, int p) {
    caminhos[p] += 1;
    if (caminhos[p] > 5) return false;
    int abertos = 0, acima_de_2 = 0;
    for (int x : caminhos) {
        abertos += x > 0;
        acima_de_2 += x > 2;
    }
    return abertos <= 2 && acima_de_2 <= 1;
}

int arredondar_preco(double valor) {
    // round() do Python arredonda empates para o par; aqui tambem
    return std::max(5, static_cast<int>(std::nearbyint(valor / 5.0)) * 5);
}

}  // namespace bl
