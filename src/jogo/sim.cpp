#include "jogo/sim.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <set>
#include <sstream>
#include <stdexcept>

#include "jogo/rodadas.hpp"

namespace bl {

// Super Ceramica do BTD6 (Blooncyclopedia, "Ceramic Bloon (BTD6)", secao Super Ceramic Bloons)
constexpr double SUPER_CERAMICA = 60, SUPER_CERAMICA_FORT = 120, SUPER_CERAMICA_DINHEIRO = 87;

const char* const MODOS_ALVO[4] = {"primeiro", "ultimo", "perto", "forte"};

namespace {

constexpr double PI = 3.14159265358979323846;

double graus(double rad) { return rad * 180.0 / PI; }
double radianos(double g) { return g * PI / 180.0; }
double quad(double v) { return v * v; }

// Divisao inteira arredondando para baixo (como o // do Python), tambem para negativos.
long long fdiv(long long a, long long b) {
    long long q = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0))) --q;
    return q;
}

std::int64_t chave_celula(long long cx, long long cy) {
    return (static_cast<std::int64_t>(cx) << 32) ^ static_cast<std::uint32_t>(cy);
}

// Dano continuo (cola corrosiva, fogo)
const Ataque& ataque_dot() {
    static const Ataque at = novo_ataque({{"dano", 1}, {"dtype", "normal"}});
    return at;
}

}  // namespace

// ================================================================ Rng
std::uint64_t Rng::proximo() {
    std::uint64_t z = (estado_ += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

double Rng::random() { return static_cast<double>(proximo() >> 11) * (1.0 / 9007199254740992.0); }
double Rng::uniform(double a, double b) { return a + (b - a) * random(); }
int Rng::randrange(int n) { return static_cast<int>(random() * n); }

std::uint32_t crc32(const std::string& dados, std::uint32_t valor) {
    static std::uint32_t tabela[256];
    static bool pronta = [] {
        for (std::uint32_t i = 0; i < 256; ++i) {
            std::uint32_t c = i;
            for (int k = 0; k < 8; ++k) c = c & 1 ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            tabela[i] = c;
        }
        return true;
    }();
    (void)pronta;
    std::uint32_t c = ~valor;
    for (unsigned char ch : dados) c = tabela[(c ^ ch) & 0xFF] ^ (c >> 8);
    return ~c;
}

// ================================================================ Torre
Torre::Torre(int id_, const std::string& chave_, int dono_, double x_, double y_, double custo, double temporaria_)
    : id(id_), chave(chave_), dfn(&definicao(chave_)), dono(dono_), x(x_), y(y_), cx(x_), cy(y_),
      nivel(dfn->heroi ? 1 : 0), investido(custo), temporaria(temporaria_) {
    recalcular();
}

void Torre::recalcular(int extra_nivel_inv) {
    Stats novo = calcular(chave, caminhos, nivel);
    if (extra_nivel_inv) {
        for (Ataque& at : novo.ataques) {
            at.dano += extra_nivel_inv;
            at.pierce += 2 * extra_nivel_inv;
            at.cad *= std::pow(0.8, extra_nivel_inv);
        }
    }
    st = std::move(novo);
    ats.clear();
    for (const Ataque& at : st.ataques) ats.push_back(std::make_shared<const Ataque>(at));
    recargas.resize(st.ataques.size(), 0.0);
    crit_conta.resize(st.ataques.size(), 0);
    for (size_t i = 0; i < crit_conta.size(); ++i)  // upgrade que encurta o intervalo do critico
        if (crit_conta[i] > std::max(st.ataques[i].crit_cada, st.ataques[i].crit_max)) crit_conta[i] = 0;
    for (size_t i = hab_rec.size(); i < st.habs.size(); ++i)
        hab_rec.push_back(st.habs[i]["recarga"].get<double>() * 0.5);
    hab_rec.resize(st.habs.size());
}

// ================================================================ Projetil
Projetil::Projetil(int id_, double x_, double y_, double ang_, AtaqueP at_, TorreP torre_, bool frag_filho_)
    : id(id_), x(x_), y(y_), vel(at_->vel), at(std::move(at_)), torre(std::move(torre_)), ox(x_), oy(y_),
      ang(ang_), frag_filho(frag_filho_) {
    double rad = radianos(ang_);
    vx = std::cos(rad) * vel;
    vy = std::sin(rad) * vel;
    pierce = at->pierce;
    dist = at->dist;
    raio = at->raio_proj;
    fusivel = at->fusivel;
    quicos = at->quica;
}

// ================================================================ Pista
Pista::Pista(int dono_, const Mapa& mapa_, int seed, int vidas_, int dinheiro_, double mult_custo_)
    : dono(dono_), mapa(mapa_), rng(static_cast<std::uint64_t>(seed) * 7919u + dono_), vidas(vidas_),
      dinheiro(dinheiro_), mult_custo(mult_custo_) {}

void Pista::evento(Evento e) {
    if (eventos.size() < MAX_EVENTOS) eventos.push_back(std::move(e));
}

int Pista::custo(int base) const { return arredondar_preco(base * mult_custo); }

int Pista::custo(int base, double x, double y) const {
    double desc = 0.0;
    for (auto& [id, t] : torres)
        if (t->st.desconto && quad(t->x - x) + quad(t->y - y) <= quad(t->alcance()))
            desc = std::max(desc, t->st.desconto);
    return arredondar_preco(base * mult_custo * (1.0 - desc));
}

std::optional<int> Pista::custo_upgrade(const Torre& t, int p) const {
    const DefTorre& dfn = *t.dfn;
    if (dfn.heroi || dfn.caminhos.empty() || !pode_upar(t.caminhos, p)) return std::nullopt;
    return custo(dfn.caminhos[p][t.caminhos[p]].custo, t.x, t.y);
}

int Pista::valor_venda(const Torre& t) const { return static_cast<int>(t.investido * t.st.venda); }

TorreP Pista::torre(int id) const {
    auto it = torres.find(id);
    return it == torres.end() ? nullptr : it->second;
}

// ---------------------------------------------------------------- bloons
BloonP Pista::criar_bloon(const std::string& nome, double d, int cam, bool camo, bool regen, bool fort,
                          const TipoBloon* orig) {
    if (cam < 0) {
        cam = proximo_cam % static_cast<int>(mapa.caminhos.size());
        proximo_cam += 1;
    }
    const TipoBloon& tipo = tipo_bloon(nome);
    auto b = std::make_shared<Bloon>();
    b->id = nid();
    b->tipo = &tipo;
    b->d = d;
    b->cam = cam;
    b->fort = fort && tipo.vida_fortificado;
    b->vida = (b->fort ? tipo.vida_fortificado : tipo.vida) * (tipo.moab ? mult_vida : 1.0);
    if (freeplay && tipo.nome == "ceramica") b->vida = b->fort ? SUPER_CERAMICA_FORT : SUPER_CERAMICA;
    b->vida_max = b->vida;
    b->camo = camo || tipo.camo_nativo;
    b->regen = regen;
    b->regen_orig = orig ? orig : &tipo;
    posicionar(*b);
    bloons.push_back(b);
    return b;
}

void Pista::posicionar(Bloon& b) {
    Posicao p = mapa.caminhos[b.cam].posicao(b.d);
    b.x = p.x;
    b.y = p.y;
    b.ang = p.ang;
}

void Pista::agendar(const std::string& nome, double atraso, bool camo, bool regen, bool fort) {
    fila.push_back({tempo + atraso, nome, camo, regen, fort});
}

int Pista::rbe_tipo(const TipoBloon& tp, bool fort) const {
    if (!freeplay) return rbe(tp.id, fort);
    fort = fort && tp.vida_fortificado;
    int r = static_cast<int>(tp.nome == "ceramica" ? (fort ? SUPER_CERAMICA_FORT : SUPER_CERAMICA)
                                                   : (fort ? tp.vida_fortificado : tp.vida));
    const size_t n = tp.moab ? tp.filhos.size() : std::min<size_t>(1, tp.filhos.size());
    for (size_t i = 0; i < n; ++i) r += rbe_tipo(tipo_bloon(tp.filhos[i]), fort);
    return r;
}

int Pista::rbe_restante(const Bloon& b) const {
    int filhos = 0;
    if (freeplay) {
        const size_t n = b.tipo->moab ? b.tipo->filhos.size() : std::min<size_t>(1, b.tipo->filhos.size());
        for (size_t i = 0; i < n; ++i) filhos += rbe_tipo(tipo_bloon(b.tipo->filhos[i]), b.fort);
    } else {
        for (int f : b.tipo->filhos_id) filhos += rbe(f, b.fort);
    }
    return std::max(1, static_cast<int>(b.vida)) + filhos;
}

bool Pista::aplicar_dano(Bloon& b, double dano, const Ataque& at, Torre* torre, Projetil* proj, DType dtype) {
    if (!b.vivo) return false;
    if (!dtype) dtype = at.dtype;
    if (torre && torre->buff.dtype_normal) dtype = DT_NORMAL;
    const TipoBloon& tp = *b.tipo;
    if (torre && torre->buff.chumbo && tp.nome == "chumbo") dtype = DT_NORMAL;  // Acidic Mixture Dip
    // remover camo e regen nao depende do tipo de dano (Signal Flare, Shimmer e a espuma pegam o DDT)
    if (at.retira_camo) b.camo = false;
    if (at.retira_regen) b.regen = false;
    if (dtype != DT_NORMAL && (tp.imune & dtype)) {
        evento({"bloqueio", b.x, b.y});
        return true;
    }
    if (b.cong_t > 0 && dtype == DT_AFIADO) return true;
    if (at.critico) evento({"crit", b.x, b.y});
    if (at.encolhe) {
        // vira um bloon vermelho comum, sem filhos nem propriedades; o B.A.D. e imune
        if (tp.nome == "bad" || tp.nome == "vermelho") return tp.nome != "bad";
        b.tipo = &tipo_bloon("vermelho");
        b.regen_orig = b.tipo;
        b.vida = b.vida_max = 1;
        b.fort = b.regen = false;
        evento({"flash", b.x, b.y});
        return true;
    }
    if (at.fragiliza) b.frag = std::max(b.frag, at.fragiliza);
    // efeitos
    if (at.congela && (tp.congela || (tp.moab && at.moab_congela)))
        b.cong_t = std::max(b.cong_t, at.congela * (tp.moab ? 0.5 : 1.0));
    if (at.tem_lento && (!tp.moab || at.moab_lento)) {
        double f = std::min(b.lento_t > 0 ? b.lento_f : 1.0, at.lento_f);
        b.lento_t = std::max(b.lento_t, at.lento_t);
        b.lento_f = f;
    }
    if (at.tem_cola && (!tp.moab || at.moab_cola)) {
        b.cola_f = at.cola_f;
        b.cola_t = at.cola_t;
        b.cola_dps = std::max(b.cola_dps, at.cola_dps);
    }
    if (at.tem_queima) {
        b.queima_dps = std::max(b.queima_dps, at.queima_dps);
        b.queima_t = std::max(b.queima_t, at.queima_t);
    }
    if (at.atordoa && (!tp.moab || at.moab_atordoa))
        b.atord_t = std::max(b.atord_t, at.atordoa * (tp.moab ? 0.4 : 1.0));
    if (at.empurra) b.d = std::max(0.0, b.d - at.empurra * (tp.moab ? 0.35 : 1.0));
    if (dano <= 0) return true;
    double extra = b.frag;
    if (tp.moab) extra += at.moab + (torre ? torre->buff.moab : 0);
    if (tp.nome == "ceramica") extra += at.cer;
    if (b.fort) extra += at.fort;
    b.vida -= dano + extra;
    b.regen_t = 0.0;
    if (b.vida <= 0) estourar(b, -b.vida, dtype, torre, proj);
    return true;
}

void Pista::estourar(Bloon& b, double excesso, DType dtype, Torre* torre, Projetil* proj, int profundidade) {
    b.vivo = false;
    double ouro = 0.0;
    if (torre) {
        torre->pops += 1;
        ouro = torre->st.ouro + torre->buff.ouro;
        if (b.tipo->nome == "chumbo") ouro += torre->st.ouro_chumbo;
    }
    const double base = freeplay && b.tipo->nome == "ceramica" ? SUPER_CERAMICA_DINHEIRO : 1.0;
    receber((base + ouro) * mult_renda);
    pops_total += 1;
    // cemiterio do Necromante: 500 bloons, ou 3.000 no Principe das Trevas (caminho 3, tier 5)
    for (auto& n : necromantes)
        if (quad(b.x - n->x) + quad(b.y - n->y) <= quad(n->alcance()))
            n->cemiterio = std::min(n->caminhos[2] >= 5 ? 3000.0 : 500.0, n->cemiterio + 1);
    if (xp_por_estouro) xp(1.0);
    evento({"pop", b.x, b.y, 0, 0, 0, b.tipo->nome});
    const TipoBloon& tp = *b.tipo;
    const int n = freeplay && !tp.moab ? std::min(1, static_cast<int>(tp.filhos.size())) : static_cast<int>(tp.filhos.size());
    if (!n) return;
    const double passo = tp.moab ? 22.0 : 7.0;
    const bool filho_camo = b.camo || tp.nome == "ddt";
    const bool filho_regen = b.regen || tp.nome == "ddt";
    for (int i = 0; i < n; ++i) {
        double d = std::max(0.0, b.d + (i - (n - 1) / 2.0) * passo);
        BloonP c = criar_bloon(tp.filhos[i], d, b.cam, filho_camo, filho_regen, b.fort,
                               b.regen ? b.regen_orig : nullptr);
        c->lento_f = b.lento_f;
        c->lento_t = b.lento_t;
        c->queima_dps = b.queima_dps;
        c->queima_t = b.queima_t;
        if (proj) proj->atingidos.insert(c->id);
        if (excesso > 0 && !c->tipo->moab && profundidade < 12) {
            if (dtype == DT_NORMAL || !(c->tipo->imune & dtype)) {
                c->vida -= excesso;
                if (c->vida <= 0) estourar(*c, -c->vida, dtype, torre, proj, profundidade + 1);
            }
        }
    }
}

void Pista::xp(double v) {
    if (!tem_heroi) return;
    for (auto& [id, t] : torres) {
        if (t->dfn->heroi && t->nivel < 20) {
            t->xp += v;
            while (t->nivel < 20 && t->xp >= XP_NIVEL[t->nivel + 1] * t->dfn->xp_escala) {
                t->nivel += 1;
                t->recalcular();
                evento({"nivel", t->x, t->y, static_cast<double>(t->nivel)});
            }
        }
    }
}

// ---------------------------------------------------------------- comandos
char Pista::colocar_torre(const std::string& chave, double x, double y) {
    const bool heroi = achar_heroi(chave) != nullptr;
    if (!achar_torre(chave) && !heroi) return ERRO_INVALIDO;
    if (heroi && (tem_heroi || chave != heroi_escolhido)) return ERRO_HEROI;
    const DefTorre& dfn = definicao(chave);
    if (!posicao_valida(dfn, x, y)) return ERRO_POSICAO;
    int c = custo(dfn.custo, x, y);
    if (dinheiro < c) return ERRO_DINHEIRO;
    dinheiro -= c;
    auto t = std::make_shared<Torre>(prox_torre, chave, dono, x, y, c);
    prox_torre += 1;
    preparar_trilha(*t);
    torres[t->id] = t;
    if (heroi) tem_heroi = true;
    buff_t = 0.0;
    evento({"colocar", x, y});
    return OK;
}

bool Pista::posicao_valida(const DefTorre& dfn, double x, double y) const {
    const double r = dfn.raio;
    if (!(r * 0.5 <= x && x <= LARGURA_MAPA - r * 0.5 && r * 0.5 <= y && y <= ALTURA_MAPA - r * 0.5)) return false;
    const bool voa = dfn.mov != Mov::FIXO;
    if (!voa) {
        if (mapa.na_trilha(x, y, r * 0.8)) return false;
        if (mapa.bloqueado(x, y, r)) return false;
        if (dfn.agua != mapa.eh_agua(x, y)) return false;
    }
    for (auto& [id, t] : torres) {
        if (t->temporaria) continue;
        if (quad(t->cx - x) + quad(t->cy - y) < quad(t->dfn->raio + r) * 0.8) return false;
    }
    return true;
}

void Pista::preparar_trilha(Torre& t) {
    t.pontos_trilha.clear();
    for (size_t ci = 0; ci < mapa.caminhos.size(); ++ci)
        for (double d : mapa.caminhos[ci].distancias_no_raio(t.x, t.y, std::max(40.0, t.alcance())))
            t.pontos_trilha.push_back({static_cast<int>(ci), d});
}

char Pista::upar(int tid, int p) {
    TorreP t = torre(tid);
    if (!t || t->temporaria || p < 0 || p > 2) return ERRO_INVALIDO;
    auto c = custo_upgrade(*t, p);
    if (!c || !requisito_upgrade(*t, p).empty()) return ERRO_BLOQUEADO;
    if (dinheiro < *c) return ERRO_DINHEIRO;
    dinheiro -= *c;
    t->investido += *c;
    if (!aviso_sacrificio(*t, p).empty()) sacrificar(*t, p);
    t->caminhos[p] += 1;
    t->recalcular();
    preparar_trilha(*t);
    buff_t = 0.0;
    if (t->chave == "fazenda" && p == 2 && t->caminhos[2] == 5) vidas += 15;  // Wall Street
    evento({"upgrade", t->x, t->y});
    return OK;
}

char Pista::vender(int tid) {
    TorreP t = torre(tid);
    if (!t || t->temporaria) return ERRO_INVALIDO;
    if (sem_venda) return ERRO_BLOQUEADO;  // CHIMPS
    dinheiro += valor_venda(*t);
    torres.erase(tid);
    if (t->dfn->heroi) tem_heroi = false;
    buff_t = 0.0;
    evento({"venda", t->x, t->y});
    return OK;
}

char Pista::mudar_modo(int tid, int m) {
    TorreP t = torre(tid);
    if (!t || m < 0 || m > 4) return ERRO_INVALIDO;
    // modo 4 (Elite) so existe no Atirador de Elite (Sniper, caminho 2, tier 5)
    if (m == 4 && !(t->chave == "sniper" && t->caminhos[1] >= 5)) return ERRO_BLOQUEADO;
    t->modo = m;
    return OK;
}

char Pista::mirar(int tid, double x, double y) {
    TorreP t = torre(tid);
    if (!t) return ERRO_INVALIDO;
    // o As so escolhe o centro da rota depois do upgrade Rota Centralizada (caminho 3, tier 2)
    const bool as_centrado = t->chave == "as" && t->caminhos[2] >= 2;
    if (!(t->chave == "dartling" || t->chave == "morteiro" || t->chave == "heli" || as_centrado)) return ERRO_INVALIDO;
    if (!(x >= 0 && x <= LARGURA_MAPA && y >= 0 && y <= ALTURA_MAPA)) return ERRO_INVALIDO;
    if (as_centrado) t->rota = 3;
    if (t->chave == "heli" && t->rota == 0) t->rota = 1;
    t->mx = x;
    t->my = y;
    t->tem_mira = true;
    return OK;
}

char Pista::opcao(int tid, int valor) {
    TorreP t = torre(tid);
    if (!t || valor < 0) return ERRO_INVALIDO;
    if (t->chave == "bumerangue" && valor <= 1) {
        t->mao = valor;
        return OK;
    }
    if (t->chave == "heli" && valor <= 2) {
        // sem ponto escolhido, fica onde esta
        if (valor && !t->tem_mira) t->mx = t->x, t->my = t->y, t->tem_mira = true;
        t->rota = valor;
        t->patrulha_volta = false;
        return OK;
    }
    if (t->chave == "fazenda" && valor == 0 && teto_banco(*t) > 0) {
        // saque do banco: tudo o que esta guardado vai para o caixa
        if (t->banco < 1) return ERRO_BLOQUEADO;
        const double v = std::floor(t->banco);
        t->banco = 0;
        receber(v);
        evento({"dinheiro", t->x, t->y, v});
        return OK;
    }
    if (t->chave == "as" && valor <= 3) {
        if (valor == 3 && t->caminhos[2] < 2) return ERRO_BLOQUEADO;
        if (valor == 3 && !t->tem_mira) t->mx = t->cx, t->my = t->cy, t->tem_mira = true;
        t->rota = valor;
        return OK;
    }
    return ERRO_INVALIDO;
}

double Pista::teto_banco(const Torre& t) const {
    if (t.chave != "fazenda" || t.caminhos[1] < 3) return 0;
    return t.caminhos[1] >= 5 ? 30000 : t.caminhos[1] == 4 ? 18000 : 14000;
}

void Pista::soltar(double x, double y, double valor, double vida, const std::string& visual) {
    x = std::clamp(x, 20.0, static_cast<double>(LARGURA_MAPA) - 20.0);
    y = std::clamp(y, 20.0, static_cast<double>(ALTURA_MAPA) - 20.0);
    coletaveis.push_back({nid(), x, y, valor, vida, visual});
}

char Pista::coletar(int cid) {
    for (size_t i = 0; i < coletaveis.size(); ++i) {
        if (coletaveis[i].id != cid) continue;
        const Coletavel c = coletaveis[i];
        coletaveis.erase(coletaveis.begin() + static_cast<long>(i));
        const double antes = dinheiro;
        receber(c.valor);
        evento({"dinheiro", c.x, c.y, std::floor(dinheiro - antes)});
        return OK;
    }
    return ERRO_INVALIDO;
}

std::string Pista::requisito_upgrade(const Torre& t, int p) const {
    // Macacopolis (Vila, caminho 3, tier 5) precisa de uma Fazenda de Bananas no alcance para sacrificar
    if (t.chave == "vila" && p == 2 && t.caminhos[2] == 4) {
        const double r2 = quad(t.alcance());
        for (auto& [id, f] : torres)
            if (f->chave == "fazenda" && quad(f->x - t.x) + quad(f->y - t.y) <= r2) return "";
        return "Requer Fazenda de Bananas no alcance";
    }
    return "";
}

std::string Pista::aviso_sacrificio(const Torre& t, int p) const {
    if (t.chave == "super" && p == 0 && t.caminhos[0] == 3)
        return "O Templo exige sacrifício. Todas as torres no alcance serão destruídas e fortalecem o Templo.";
    if (t.chave == "super" && p == 0 && t.caminhos[0] == 4)
        return "Invocar o Verdadeiro Deus Sol? As torres no alcance serão sacrificadas de novo.";
    if (t.chave == "vila" && p == 2 && t.caminhos[2] == 4)
        return "As Fazendas de Bananas no alcance serão sacrificadas e viram renda da Macacópolis.";
    return "";
}

// Destroi as torres vizinhas e guarda o bonus. Aproximacao do BTD6: la o Templo tem faixas de valor por
// categoria (ate 3 categorias); aqui cada categoria da um tipo de bonus, proporcional ao valor sacrificado.
void Pista::sacrificar(Torre& t, int p) {
    (void)p;
    const bool so_fazendas = t.chave == "vila";
    const double r2 = quad(t.alcance());
    std::map<std::string, double> total;
    std::vector<int> ids;
    for (auto& [id, f] : torres) {
        if (f.get() == &t || f->dfn->heroi || f->temporaria) continue;
        if (so_fazendas && f->chave != "fazenda") continue;
        if (quad(f->x - t.x) + quad(f->y - t.y) > r2) continue;
        total[f->dfn->categoria] += f->investido;
        ids.push_back(id);
    }
    for (int id : ids) {
        evento({"venda", torres[id]->x, torres[id]->y});
        torres.erase(id);
    }
    if (so_fazendas) {
        double soma = 0;
        for (auto& [cat, v] : total) soma += v;
        t.renda_sacrificio += soma * 0.15;  // por rodada
        return;
    }
    Buffs& b = t.sacrificio;
    b.vazio = false;
    b.dano += std::min(5.0, std::floor(total["primaria"] / 5000.0));
    b.pierce += std::min(20.0, std::floor(total["militar"] / 2500.0));
    b.vel_pct += std::min(0.5, total["magica"] / 50000.0);
    b.alcance_pct += std::min(0.3, total["suporte"] / 50000.0);
}

char Pista::usar_habilidade(int tid, int idx) {
    TorreP t = torre(tid);
    if (!t || idx < 0 || idx >= static_cast<int>(t->st.habs.size()) || t->hab_rec[idx] > 0) return ERRO_INVALIDO;
    J h = t->st.habs[idx];
    t->hab_rec[idx] = h["recarga"].get<double>();
    executar_habilidade(t, h);
    evento({"habilidade", t->x, t->y, 0, 0, 0, h["nome"].get<std::string>()});
    return OK;
}

// ---------------------------------------------------------------- habilidades
void Pista::executar_habilidade(const TorreP& tp, const J& h) {
    Torre& t = *tp;
    const std::string tipo = h["tipo"].get<std::string>();
    const double congela = h.value("congela", 0.0);
    J def = {{"dtype", "normal"}, {"atordoa", h.value("atordoa", 0.0)}, {"congela", congela},
             {"moab_atordoa", true}, {"moab_congela", congela != 0}};
    if (h.contains("queima")) def["queima"] = h["queima"];
    def["moab"] = h.value("moab_mais", 0.0);  // dano extra em dirigiveis e ceramicas (Storm of Arrows, Firestorm)
    def["cer"] = h.value("cer_mais", 0.0);
    const Ataque at = novo_ataque(def);

    if (tipo == "turbo") {
        t.turbo = h["valor"].get<double>();
        t.turbo_t = h["dur"].get<double>();
        if (h.contains("disfarce")) {
            t.disfarce = h["disfarce"].get<std::string>();
            t.disfarce_t = t.turbo_t;
        }
    } else if (tipo == "espiral") {
        // laminas do ataque principal, sem limite de alcance, saindo em bracos que giram
        if (!t.ats.empty()) {
            auto a = std::make_shared<Ataque>(*t.ats[0]);
            a->tipo = TipoAtaque::PROJETIL;
            a->dist = 2200;
            a->vel = 620;
            a->busca = false;
            a->pierce = h.value("pierce", 12.0);
            a->visual = "lamina";
            a->critico = false;
            t.espiral_at = a;
            t.espiral_t = h["dur"].get<double>();
            t.espiral_bracos = static_cast<int>(h.value("n", 2.0));
            t.espiral_prox = 0.0;
        }
    } else if (tipo == "turbo_area") {
        std::set<std::string> filtro;
        std::stringstream ss(h.value("filtro", std::string()));
        for (std::string item; std::getline(ss, item, ',');)
            if (!item.empty()) filtro.insert(item);
        // "n": so as n torres mais proximas (Biohack); "buffs": buff temporario alem do turbo (Rallying Roar)
        std::vector<Torre*> alvos;
        for (auto& [id, o] : torres) {
            if (!filtro.empty() && !filtro.count(o->chave)) continue;
            if (h.value("sem_si", false) && o.get() == &t) continue;
            if (h.value("global_", false) || quad(o->x - t.x) + quad(o->y - t.y) <= quad(t.alcance() + 60))
                alvos.push_back(o.get());
        }
        if (h.contains("n")) {
            std::stable_sort(alvos.begin(), alvos.end(), [&](Torre* a, Torre* b) {
                return quad(a->x - t.x) + quad(a->y - t.y) < quad(b->x - t.x) + quad(b->y - t.y);
            });
            alvos.resize(std::min(alvos.size(), static_cast<size_t>(h["n"].get<int>())));
        }
        for (Torre* o : alvos) {
            if (h.contains("disfarce")) {
                o->disfarce = h["disfarce"].get<std::string>();
                o->disfarce_t = h["dur"].get<double>();
            }
            if (h.contains("valor")) {
                o->turbo = std::min(o->turbo, h["valor"].get<double>());
                o->turbo_t = std::max(o->turbo_t, h["dur"].get<double>());
            }
            if (h.contains("buffs")) {
                Torre::Pocao& p = o->pocoes["hab:" + h["nome"].get<std::string>()];
                p.b = Buffs{};
                p.b.mesclar(h["buffs"]);
                p.t = h["dur"].get<double>();
                p.tiros = 1e18;
                buff_t = 0;
            }
        }
    } else if (tipo == "dano_global") {
        const size_t n = bloons.size();
        for (size_t i = 0; i < n; ++i) aplicar_dano(*bloons[i], h["valor"].get<double>(), at, &t);
        evento({"flash", 0, 0, 0, 0, 0, "", {255, 255, 255}});
    } else if (tipo == "dano_forte") {
        std::vector<BloonP> alvos;
        for (auto& b : bloons)
            if (b->vivo && (b->tipo->moab || !h.value("moab_so", false))) alvos.push_back(b);
        std::stable_sort(alvos.begin(), alvos.end(), [](const BloonP& a, const BloonP& b) {
            if (a->tipo->rank != b->tipo->rank) return a->tipo->rank > b->tipo->rank;
            return a->d > b->d;
        });
        const size_t n = std::min(alvos.size(), static_cast<size_t>(h.value("n", 1)));
        for (size_t i = 0; i < n; ++i) {
            Bloon& b = *alvos[i];
            evento({"raio", t.x, t.y, 0, b.x, b.y});
            double bx = b.x, by = b.y;
            // "pct": parte da vida maxima do alvo (MOAB Hex da Ezili tira 4% por segundo por 25 s)
            aplicar_dano(b, h["valor"].get<double>() + b.vida_max * h.value("pct", 0.0), at, &t);
            if (h.value("splash", 0.0))
                explosao(bx, by, h["splash"].get<double>(), h.value("sdano", 1.0), 999, at, &t, DT_NORMAL);
        }
    } else if (tipo == "recarregar") {
        // Artillery Command: zera a recarga das habilidades das torres do filtro
        std::stringstream ss(h.value("filtro", std::string()));
        std::set<std::string> filtro;
        for (std::string item; std::getline(ss, item, ',');) filtro.insert(item);
        for (auto& [id, o] : torres)
            if (filtro.count(o->chave))
                for (double& r : o->hab_rec) r = 0;
    } else if (tipo == "sem_regen") {
        // Heartstopper: os bloons na tela perdem a regeneracao
        for (auto& b : bloons) b->regen = false;
        evento({"flash", 0, 0, 0, 0, 0, "", {150, 40, 90}});
    } else if (tipo == "lentidao") {
        lentidao_global_f = h["valor"].get<double>();
        lentidao_global_t = h["dur"].get<double>();
        if (h.value("dano", 0.0)) {
            const size_t n = bloons.size();
            for (size_t i = 0; i < n; ++i) aplicar_dano(*bloons[i], h["dano"].get<double>(), at, &t);
        }
        evento({"flash", 0, 0, 0, 0, 0, "", {150, 220, 90}});
    } else if (tipo == "congelar_global") {
        for (auto& b : bloons)
            if (b->tipo->congela || h.value("moab", false))
                b->cong_t = std::max(b->cong_t, h["dur"].get<double>() * (b->tipo->moab ? 0.5 : 1.0));
        evento({"flash", 0, 0, 0, 0, 0, "", {180, 230, 255}});
    } else if (tipo == "dinheiro") {
        // CHIMPS nao deixa gerar dinheiro por habilidade; Half Cash e Deflation multiplicam
        if (h.value("caixa", false)) {
            // caixa de suprimentos: cai ao lado da torre e paga quando o jogador coleta
            if (!so_estouro_e_rodada)
                soltar(t.x + rng.uniform(-60, 60), t.y + rng.uniform(30, 60), h["valor"].get<double>(), 60.0, "caixa");
        } else {
            const double v = so_estouro_e_rodada ? 0.0 : h["valor"].get<double>() * mult_dinheiro;
            dinheiro += v;
            evento({"dinheiro", t.x, t.y, v});
        }
    } else if (tipo == "emprestimo") {
        const double v = so_estouro_e_rodada ? 0.0 : h["valor"].get<double>() * mult_dinheiro;
        dinheiro += v;
        divida += v;
        evento({"dinheiro", t.x, t.y, v});
    } else if (tipo == "roubo") {
        dinheiro += h["valor"].get<double>();
        if (oponente) oponente->dinheiro = std::max(0.0, oponente->dinheiro - h["valor"].get<double>());
        evento({"dinheiro", t.x, t.y, h["valor"].get<double>()});
    } else if (tipo == "invocar") {
        invocar(t, h.value("base", std::string("sentinela")), h.value("dur", 15.0),
                h.contains("nivel") ? h["nivel"] : J());
    } else if (tipo == "reverso") {
        for (auto& b : bloons) b->d = std::max(0.0, b->d - h["valor"].get<double>() * (b->tipo->moab ? 0.5 : 1.0));
        evento({"flash", 0, 0, 0, 0, 0, "", {40, 40, 60}});
    } else if (tipo == "spikes_global" || tipo == "spikes_local") {
        auto at_p = std::make_shared<const Ataque>(novo_ataque({{"dano", h.value("dano", 1.0)}, {"dtype", "normal"}}));
        if (tipo == "spikes_global") {
            for (const Caminho& cam : mapa.caminhos) {
                for (double d = 60.0; d < cam.comprimento; d += 140.0) {
                    Posicao p = cam.posicao(d);
                    pilhas.push_back(std::unique_ptr<Pilha>(
                        new Pilha{p.x, p.y, h["valor"].get<double>(), at_p, tp, 20.0, {}, true, "espinhos"}));
                }
            }
        } else {
            auto [ci, d] = ponto_trilha_mais_avancado(t);
            Posicao p = mapa.caminhos[ci].posicao(d);
            pilhas.push_back(std::unique_ptr<Pilha>(
                new Pilha{p.x, p.y, h["valor"].get<double>(), at_p, tp, h.value("dur", 10.0), {}, true, "espinheiro"}));
        }
    }
}

std::pair<int, double> Pista::ponto_trilha_mais_avancado(const Torre& t) const {
    if (!t.pontos_trilha.empty()) {
        auto melhor = t.pontos_trilha.front();
        for (auto& p : t.pontos_trilha)
            if (p.second > melhor.second) melhor = p;
        return melhor;
    }
    const Caminho& cam = mapa.caminhos[0];
    const Amostra* melhor = &cam.amostras.front();
    double md = quad(melhor->x - t.x) + quad(melhor->y - t.y);
    for (const Amostra& a : cam.amostras) {
        double d = quad(a.x - t.x) + quad(a.y - t.y);
        if (d < md) md = d, melhor = &a;
    }
    return {0, melhor->d};
}

void Pista::invocar(const Torre& t, const std::string& base, double dur, const J& nivel) {
    double ang = rng.uniform(0, 2 * PI);
    double dist = 40 + rng.uniform(0, 30);
    double x = std::min(std::max(t.x + std::cos(ang) * dist, 20.0), LARGURA_MAPA - 20.0);
    double y = std::min(std::max(t.y + std::sin(ang) * dist, 20.0), ALTURA_MAPA - 20.0);
    auto s = std::make_shared<Torre>(prox_torre, base, dono, x, y, 0, dur);
    prox_torre += 1;
    if (nivel.is_array()) {
        for (int i = 0; i < 3; ++i) s->caminhos[i] = nivel[i].get<int>();
        s->recalcular();
    }
    if (base == "sentinela") {
        int extra = 0;
        for (const Ataque& at : t.st.ataques)
            if (at.tipo == TipoAtaque::INVOCAR) extra = static_cast<int>(at.nivel_inv);
        if (extra) s->recalcular(extra);
    }
    preparar_trilha(*s);
    torres[s->id] = s;
    evento({"invocar", x, y});
}

// Defesa Comanche: quando um bloon passa de 25% da trilha, 3 mini-helicopteros escoltam o Heli por
// 20 s (e so voltam 45 s depois). No Comandante Comanche os 3 sao permanentes.
void Pista::comanches(Torre& t) {
    if (t.comanche_t > 0) t.comanche_t -= DT;
    int vivos = 0;
    for (auto& [id, o] : torres) vivos += o->mae == t.id;
    const bool fixos = t.caminhos[2] >= 5;
    if (vivos >= 3) return;
    if (!fixos) {
        if (vivos > 0 || t.comanche_t > 0) return;
        bool avancou = false;
        for (auto& b : bloons)
            if (b->vivo && b->d >= mapa.caminhos[b->cam].comprimento * 0.25) {
                avancou = true;
                break;
            }
        if (!avancou) return;
        t.comanche_t = 45.0;
    }
    for (int k = vivos; k < 3; ++k) {
        const double ang = 2 * PI * k / 3;
        auto s = std::make_shared<Torre>(prox_torre, "heli", dono, t.x + std::cos(ang) * 46, t.y + std::sin(ang) * 46, 0,
                                         fixos ? 1e9 : 20.0);
        prox_torre += 1;
        s->mae = t.id;
        s->caminhos = {0, 0, 2};
        s->recalcular();
        preparar_trilha(*s);
        torres[s->id] = s;
        evento({"invocar", s->x, s->y});
    }
}

// ---------------------------------------------------------------- passo
void Pista::passo() {
    tempo += DT;
    spawns();
    grade();
    buff_t -= DT;
    if (buff_t <= 0) {
        recalcular_buffs();
        buff_t = 0.5;
    }
    if (lentidao_global_t > 0) lentidao_global_t -= DT;
    std::vector<TorreP> lista;
    for (auto& [id, t] : torres) lista.push_back(t);
    for (auto& t : lista) passo_torre(t);
    passo_projeteis();
    passo_pilhas();
    for (Coletavel& c : coletaveis) c.vida -= DT;
    coletaveis.erase(std::remove_if(coletaveis.begin(), coletaveis.end(), [](const Coletavel& c) { return c.vida <= 0; }),
                     coletaveis.end());
    mover_bloons();
    bloons.erase(std::remove_if(bloons.begin(), bloons.end(), [](const BloonP& b) { return !b->vivo; }),
                 bloons.end());
}

void Pista::spawns() {
    if (fila.empty()) return;
    std::vector<Agendado> restantes;
    for (Agendado& item : fila) {
        if (item.t <= tempo) criar_bloon(item.nome, 0.0, -1, item.camo, item.regen, item.fort);
        else restantes.push_back(std::move(item));
    }
    fila = std::move(restantes);
}

void Pista::grade() {
    grade_.clear();
    for (auto& b : bloons)
        if (b->vivo)
            grade_[chave_celula(fdiv(static_cast<long long>(b->x), CELULA), fdiv(static_cast<long long>(b->y), CELULA))]
                .push_back(b.get());
}

std::vector<Bloon*> Pista::vizinhos(double x, double y, double r) const {
    std::vector<Bloon*> out;
    long long c0 = fdiv(static_cast<long long>(x - r), CELULA), c1 = fdiv(static_cast<long long>(x + r), CELULA);
    long long l0 = fdiv(static_cast<long long>(y - r), CELULA), l1 = fdiv(static_cast<long long>(y + r), CELULA);
    for (long long cx = c0; cx <= c1; ++cx) {
        for (long long cy = l0; cy <= l1; ++cy) {
            auto it = grade_.find(chave_celula(cx, cy));
            if (it != grade_.end()) out.insert(out.end(), it->second.begin(), it->second.end());
        }
    }
    return out;
}

namespace {
// O buff vale para esta torre? O escopo lista chaves de torre, categorias ou "agua", separados por "|".
bool no_escopo(const Torre& t, const std::string& escopo) {
    if (escopo.empty()) return true;
    size_t i = 0;
    while (i <= escopo.size()) {
        size_t j = escopo.find('|', i);
        if (j == std::string::npos) j = escopo.size();
        const std::string tok = escopo.substr(i, j - i);
        if (tok == t.chave) return true;
        if (!t.dfn->heroi && (tok == t.dfn->categoria || (tok == "agua" && t.dfn->agua))) return true;
        i = j + 1;
    }
    return false;
}

bool ataca(const Torre& t) {
    for (const AtaqueP& a : t.ats)
        if (a->tipo != TipoAtaque::BUFF && a->tipo != TipoAtaque::RENDA) return true;
    return false;
}
}  // namespace

void Pista::recalcular_buffs() {
    necromantes.clear();
    for (auto& [id, t] : torres) {
        t->necromante = false;
        for (const Ataque& at : t->st.ataques)
            if (at.tipo == TipoAtaque::PILHA && at.visual == "zumbi") t->necromante = true;
        if (t->necromante) necromantes.push_back(t);
    }
    // por torre alvo: fonte (chave da torre + indice do ataque) -> buffs recebidos dessa fonte.
    // Fontes iguais nao acumulam no BTD6 (duas Vilas nao dao 2x Jungle Drums): fica o melhor de cada
    // campo. Excecoes com "acumula" > 1: Shinobi Tactics (20) e Poplust (5).
    std::map<int, std::map<std::string, std::vector<const Buffs*>>> por_alvo;
    for (auto& [fid, f] : torres) {
        const double r2 = quad(f->alcance());
        for (size_t i = 0; i < f->st.ataques.size(); ++i) {
            const Ataque& at = f->st.ataques[i];
            const Buffs& b = at.buffs;
            if (b.vazio || at.pocao) continue;
            const std::string fonte = f->chave + "#" + std::to_string(i);
            for (auto& [tid, t] : torres) {
                if (t == f && (at.tipo == TipoAtaque::BUFF || b.sem_si)) continue;
                if (!b.global_ && quad(t->x - f->x) + quad(t->y - f->y) > r2) continue;
                if (!no_escopo(*t, b.escopo)) continue;
                por_alvo[tid][fonte].push_back(&b);
            }
        }
    }
    for (auto& [id, t] : torres) {
        t->buff = Buffs{};
        auto it = por_alvo.find(id);
        if (it != por_alvo.end()) {
            for (auto& [fonte, lista] : it->second) {
                const size_t max = static_cast<size_t>(std::max(1, lista.front()->acumula));
                if (max > 1) {
                    for (size_t k = 0; k < lista.size() && k < max; ++k) t->buff.mesclar(*lista[k]);
                } else {
                    Buffs m;
                    for (const Buffs* b : lista) m.melhor(*b);
                    t->buff.mesclar(m);
                }
            }
        }
        for (auto& [tipo, p] : t->pocoes) t->buff.mesclar(p.b);
        if (!t->sacrificio.vazio) t->buff.mesclar(t->sacrificio);
    }
}

// Berserker Brew vai na torre mais proxima; o AMD vai numa torre sorteada. As duas preferem torres que
// ainda nao tem aquela pocao.
bool Pista::jogar_pocao(const TorreP& fp, const Ataque& at) {
    const Torre& f = *fp;
    const std::string& tipo = at.visual;
    const double r2 = quad(f.alcance());
    std::vector<Torre*> livres, todas;
    for (auto& [id, t] : torres) {
        if (t.get() == &f || !ataca(*t)) continue;
        if (quad(t->x - f.x) + quad(t->y - f.y) > r2) continue;
        auto bl = t->pocao_bloq.find(tipo);
        if (bl != t->pocao_bloq.end() && bl->second > 0) continue;
        auto p = t->pocoes.find(tipo);
        if (p == t->pocoes.end()) livres.push_back(t.get());
        else if (at.pocao_max <= 0 || p->second.tiros < at.pocao_max) todas.push_back(t.get());
    }
    std::vector<Torre*>& cand = livres.empty() ? todas : livres;
    if (cand.empty()) return false;
    Torre* alvo_ = nullptr;
    if (at.pocao_max > 0) {
        alvo_ = cand[static_cast<size_t>(rng.randrange(static_cast<int>(cand.size())))];
    } else {
        for (Torre* t : cand)
            if (!alvo_ || quad(t->x - f.x) + quad(t->y - f.y) < quad(alvo_->x - f.x) + quad(alvo_->y - f.y)) alvo_ = t;
    }
    constexpr double SEMPRE = 1e18;
    Torre::Pocao& p = alvo_->pocoes[tipo];
    const double tiros = at.valor > 0 ? at.valor : SEMPRE;
    if (at.pocao_max > 0 && p.tiros > 0 && p.tiros < SEMPRE) p.tiros = std::min(at.pocao_max, p.tiros + tiros);
    else p.tiros = tiros;
    p.t = at.dur > 0 ? at.dur : SEMPRE;
    p.b = at.buffs;
    alvo_->pocao_bloq[tipo] = at.pocao_bloq;
    buff_t = 0;
    evento({"tiro", f.x, f.y, 0, 0, 0, f.chave});
    return true;
}

// ---------------------------------------------------------------- torres
Bloon* Pista::alvo(const Torre& t, const Ataque& at, double alcance) const {
    const bool camo = t.detecta_camo();
    const bool glob = at.global_ || alcance >= 5000;
    const double r2 = alcance * alcance;
    const int modo = at.alvo_forte ? 3 : t.modo;
    Bloon* melhor = nullptr;
    double chave_melhor = 0;
    for (auto& bp : bloons) {
        Bloon* b = bp.get();
        if (!b->vivo || (b->camo && !camo) || (at.so_moab && !b->tipo->moab)) continue;
        double dx = b->x - t.x, dy = b->y - t.y;
        double dist2 = dx * dx + dy * dy;
        if (!glob && dist2 > r2) continue;
        double k;
        if (modo == 0) k = b->d;
        else if (modo == 1) k = -b->d;
        else if (modo == 2) k = -dist2;
        else if (modo == 4) {
            // Elite: o mais forte, mas quem ja passou de 75% da trilha vem antes de todos
            const bool urgente = b->d > mapa.caminhos[b->cam].comprimento * 0.75;
            k = (urgente ? 1e12 + b->d * 1e3 : 0.0) + b->tipo->rank * 100000.0 + b->d;
        } else k = b->tipo->rank * 100000.0 + b->d;
        if (!melhor || k > chave_melhor) chave_melhor = k, melhor = b;
    }
    return melhor;
}

Posicao Pista::mira_antecipada(const Torre& t, const Ataque& at, const Bloon& b) const {
    Posicao m{b.x, b.y, b.ang};
    if (at.vel <= 0 || b.cong_t > 0 || b.atord_t > 0) return m;
    // mesma conta de mover_bloons, em px/s
    double v = VELOCIDADE_BASE * mult_vel * b.tipo->velocidade * (lentidao_global_t > 0 ? lentidao_global_f : 1.0);
    if (b.lento_t > 0) v *= b.lento_f;
    if (b.cola_t > 0) v *= b.cola_f;
    const Caminho& cam = mapa.caminhos[b.cam];
    // o tempo de voo depende do ponto mirado, que depende do tempo de voo: 3 voltas bastam
    for (int k = 0; k < 3; ++k) {
        const double tempo = std::hypot(m.x - t.x, m.y - t.y) / at.vel;
        m = cam.posicao(std::min(b.d + v * tempo, cam.comprimento));
    }
    return m;
}

void Pista::mover_torre(Torre& t) {
    if (t.dfn->mov == Mov::ORBITA) {
        t.orbita += DT * 1.3;
        const double raio = 110.0, o = t.orbita;
        const double ax = t.x, ay = t.y;
        if (t.rota == 1) {  // infinito: um 8 deitado
            t.x = t.cx + std::cos(o) * raio * 1.7;
            t.y = t.cy + std::sin(2 * o) * raio * 0.55;
        } else if (t.rota == 2) {  // oito: um 8 em pe
            t.x = t.cx + std::sin(2 * o) * raio * 0.55;
            t.y = t.cy + std::cos(o) * raio * 1.3;
        } else {  // circulo, em volta da pista ou do ponto escolhido
            const bool centrado = t.rota == 3 && t.tem_mira;
            t.x = (centrado ? t.mx : t.cx) + std::cos(o) * raio;
            t.y = (centrado ? t.my : t.cy) + std::sin(o) * raio * 0.75;
        }
        // o nariz aponta para onde o aviao anda
        if (t.x != ax || t.y != ay) t.ang = graus(std::atan2(t.y - ay, t.x - ax));
    } else if (t.dfn->mov == Mov::HELI) {
        const double limite = t.st.persegue ? 9999 : 260;
        Bloon* melhor = nullptr;
        for (auto& bp : bloons) {
            Bloon* b = bp.get();
            if (b->vivo && (!b->camo || t.detecta_camo()))
                if (quad(b->x - t.cx) + quad(b->y - t.cy) <= limite * limite)
                    if (!melhor || b->d > melhor->d) melhor = b;
        }
        double ax = melhor ? melhor->x : t.cx, ay = melhor ? melhor->y : t.cy;
        double chegada = 30;
        if (t.rota && t.tem_mira) {
            // modos do jogador: ir ate o ponto mirado, ou patrulhar entre ele e o heliponto
            chegada = 4;
            ax = t.rota == 2 && t.patrulha_volta ? t.cx : t.mx;
            ay = t.rota == 2 && t.patrulha_volta ? t.cy : t.my;
            if (t.rota == 2 && std::hypot(ax - t.x, ay - t.y) <= 12) t.patrulha_volta = !t.patrulha_volta;
        }
        double dx = ax - t.x, dy = ay - t.y;
        double dist = std::hypot(dx, dy);
        if (dist > chegada) {
            double v = 260.0 * DT;
            t.x += dx / dist * std::min(v, dist);
            t.y += dy / dist * std::min(v, dist);
        }
    }
}

void Pista::passo_torre(const TorreP& tp) {
    Torre& t = *tp;
    if (t.temporaria) {
        t.temporaria -= DT;
        // a escolta some junto com o Heli que a chamou
        if (t.temporaria <= 0 || (t.mae && !torres.count(t.mae))) {
            torres.erase(t.id);
            return;
        }
    }
    if (t.chave == "heli" && t.caminhos[2] >= 4 && !t.temporaria) comanches(t);
    if (t.dfn->mov != Mov::FIXO) mover_torre(t);
    if (t.turbo_t > 0) {
        t.turbo_t -= DT;
        if (t.turbo_t <= 0) t.turbo = 1.0;
    }
    if (t.disfarce_t > 0) {
        t.disfarce_t -= DT;
        if (t.disfarce_t <= 0) t.disfarce.clear();
    }
    if (t.espiral_t > 0 && t.espiral_at) {
        t.espiral_t -= DT;
        t.espiral_prox -= DT;
        if (t.espiral_prox <= 0) {
            t.espiral_prox = 0.066;
            for (int k = 0; k < t.espiral_bracos; ++k)
                disparar(tp, t.espiral_at, t.espiral_ang + k * 360.0 / t.espiral_bracos);
            t.espiral_ang += 17.0;
        }
    }
    for (double& r : t.hab_rec)
        if (r > 0) r -= DT;
    for (auto& [tipo, s] : t.pocao_bloq) s -= DT;
    for (auto it = t.pocoes.begin(); it != t.pocoes.end();) {
        it->second.t -= DT;
        if (it->second.t <= 0 || it->second.tiros <= 0) {
            it = t.pocoes.erase(it);
            buff_t = 0;  // recalcula os buffs no proximo passo
        } else {
            ++it;
        }
    }
    const double mult_cad = t.turbo * std::max(0.1, t.buff.mult_cad());
    const double alcance = t.alcance();
    const std::vector<AtaqueP> ats = t.ats;  // o heroi pode subir de nivel no meio do laco
    const std::vector<double> antes = t.recargas;
    for (size_t i = 0; i < ats.size(); ++i) {
        const AtaqueP& atp = ats[i];
        const Ataque& at = *atp;
        const TipoAtaque tipo = at.tipo;
        if (tipo == TipoAtaque::BUFF && at.pocao) {
            t.recargas[i] -= DT;
            if (t.recargas[i] <= 0 && jogar_pocao(tp, at)) t.recargas[i] = std::max(0.02, at.cad * mult_cad);
            continue;
        }
        if (tipo == TipoAtaque::BUFF || tipo == TipoAtaque::RENDA) continue;
        t.recargas[i] -= DT;
        if (t.recargas[i] > 0) continue;
        const double cad = std::max(0.02, at.cad * mult_cad);
        if (tipo == TipoAtaque::INVOCAR) {
            t.recargas[i] = cad;
            invocar(t, at.base.empty() ? "sentinela" : at.base, at.dur ? at.dur : 20.0, J());
            continue;
        }
        if (tipo == TipoAtaque::PILHA) {
            if (!t.pontos_trilha.empty() && bloons_ou_fila()) {
                int meus = 0;
                for (auto& p : pilhas) meus += p->torre.get() == &t;
                if (meus < MAX_PILHAS_POR_TORRE) {
                    auto [ci, d] = t.pontos_trilha[rng.randrange(static_cast<int>(t.pontos_trilha.size()))];
                    Posicao pos = mapa.caminhos[ci].posicao(d);
                    double x = pos.x + rng.uniform(-10, 10);
                    double y = pos.y + rng.uniform(-10, 10);
                    double pierce = at.pilha_pierce + t.buff.pierce;
                    if (at.visual == "zumbi") {
                        // o zumbi sai do cemiterio: sem bloons guardados, nao ha o que reviver
                        if (t.cemiterio < 1) continue;
                        pierce = std::min(pierce, std::floor(t.cemiterio));
                        t.cemiterio -= pierce;
                    }
                    pilhas.push_back(
                        std::unique_ptr<Pilha>(new Pilha{x, y, pierce, atp, tp, at.pilha_vida, {}, true, at.visual}));
                    t.recargas[i] = cad;
                }
            }
            continue;
        }
        if (tipo == TipoAtaque::QUEDA) {
            if (!bloons.empty()) {
                double x = t.x, y = t.y;
                if (at.na_trilha) {
                    Bloon* a = alvo(t, at, 9999);
                    if (a) x = a->x, y = a->y;
                }
                auto p = std::make_unique<Projetil>(nid(), x, y, 0, atp, tp);
                p->vx = p->vy = 0.0;
                p->fusivel = at.fusivel ? at.fusivel : 0.5;
                projeteis.push_back(std::move(p));
                t.recargas[i] = cad;
            }
            continue;
        }
        if (tipo == TipoAtaque::AURA) {
            aura(tp, atp, at.raio_aura ? at.raio_aura : alcance);
            t.recargas[i] = cad;
            continue;
        }
        if (tipo == TipoAtaque::RADIAL) {
            const bool glob = at.global_ || t.dfn->mov == Mov::ORBITA;
            const bool ok = glob ? !bloons.empty() : alvo(t, at, alcance) != nullptr;
            if (ok) {
                const int n = static_cast<int>(at.n);
                const double base = t.dfn->mov == Mov::ORBITA ? t.ang : 0.0;
                Bloon* guia = at.busca ? alvo(t, at, 9999) : nullptr;  // teleguiados partem atras do alvo
                for (int k = 0; k < n; ++k) disparar(tp, atp, base + k * 360.0 / n, guia);
                t.recargas[i] = cad;
                evento({"tiro", t.x, t.y, 0, 0, 0, t.chave});
            }
            continue;
        }
        // torre com ponto escolhido pelo jogador: a Dartling atira na direcao do cursor e o Morteiro
        // bombardeia o ponto fixo, haja ou nao bloon ali
        const bool no_ponto =
            t.tem_mira &&
            ((t.chave == "dartling" && (tipo == TipoAtaque::PROJETIL || tipo == TipoAtaque::HITSCAN) && !at.busca) ||
             (t.chave == "morteiro" && tipo == TipoAtaque::MORTEIRO));
        Bloon* a = no_ponto ? nullptr : alvo(t, at, alcance);
        if (no_ponto ? bloons.empty() : !a) continue;
        t.recargas[i] = cad;
        const AtaqueP atc = critico(t, i, atp);
        // projetil reto mira onde o bloon vai estar; teleguiado, bumerangue e o resto miram o bloon
        const bool antecipa = !no_ponto && tipo == TipoAtaque::PROJETIL && !at.busca && !at.boom;
        const Posicao mira = no_ponto   ? Posicao{t.mx, t.my, 0}
                             : antecipa ? mira_antecipada(t, *atc, *a)
                                        : Posicao{a->x, a->y, 0};
        const double ang = graus(std::atan2(mira.y - t.y, mira.x - t.x));
        if (i == 0 && t.dfn->mov == Mov::FIXO) t.ang = ang;
        if (no_ponto && tipo == TipoAtaque::HITSCAN) {
            raio_em_linha(t, *ataque_efetivo(t, atc), t.mx, t.my);
        } else if (tipo == TipoAtaque::HITSCAN) {
            hitscan(tp, atc, *a);
        } else if (tipo == TipoAtaque::CADEIA) {
            cadeia(tp, atc, *a);
        } else if (tipo == TipoAtaque::MORTEIRO) {
            for (int k = 0; k < static_cast<int>(at.n); ++k) {
                const double im = at.impreciso;
                double x = mira.x + rng.uniform(-im, im);
                double y = mira.y + rng.uniform(-im, im);
                auto p = std::make_unique<Projetil>(nid(), x, y, 0, atc, tp);
                p->vx = p->vy = 0.0;
                p->fusivel = 0.7;
                projeteis.push_back(std::move(p));
                evento({"morteiro", t.x, t.y, 0, x, y});
            }
        } else {
            const int n = static_cast<int>(at.n);
            const double spread = at.spread;
            if (n <= 1) {
                // um projetil so com espalhamento (Dartling): desvio sorteado dentro do leque
                disparar(tp, atc, spread > 0 ? ang + rng.uniform(-spread / 2, spread / 2) : ang, a);
            } else if (spread >= 360) {
                for (int k = 0; k < n; ++k) disparar(tp, atc, ang + k * 360.0 / n, a);
            } else {
                const double passo = spread / (n - 1);
                for (int k = 0; k < n; ++k) disparar(tp, atc, ang - spread / 2 + k * passo, a);
            }
            evento({"tiro", t.x, t.y, 0, 0, 0, t.chave});
        }
    }
    // cada ataque disparado gasta um tiro das pocoes recebidas (Berserker Brew dura 25 tiros)
    if (!t.pocoes.empty())
        for (size_t i = 0; i < ats.size() && i < antes.size() && i < t.recargas.size(); ++i) {
            const TipoAtaque tipo = ats[i]->tipo;
            if (tipo == TipoAtaque::BUFF || tipo == TipoAtaque::RENDA || tipo == TipoAtaque::INVOCAR) continue;
            if (t.recargas[i] > antes[i] - DT + 1e-9)
                for (auto& [nome, p] : t.pocoes) p.tiros -= 1;
        }
}

// Conta os tiros de um ataque com critico; no tiro critico devolve uma copia com o dano do critico.
AtaqueP Pista::critico(Torre& t, size_t i, const AtaqueP& at) {
    if (at->crit_cada <= 0 || i >= t.crit_conta.size()) return at;
    int& falta = t.crit_conta[i];
    auto sortear = [&] {
        const int a = static_cast<int>(at->crit_cada), b = static_cast<int>(std::max(at->crit_cada, at->crit_max));
        return b > a ? a + rng.randrange(b - a + 1) : a;
    };
    if (falta <= 0) falta = sortear();
    if (--falta > 0) return at;
    falta = sortear();
    auto c = std::make_shared<Ataque>(*at);
    c->dano = at->crit_dano > 0 ? at->crit_dano : at->dano + at->crit_mais;
    c->critico = true;
    return c;
}

AtaqueP Pista::ataque_efetivo(const Torre& t, const AtaqueP& at) const {
    const Buffs& b = t.buff;
    if (b.vazio || (!b.dano && !b.pierce && !b.pierce_pct && !b.cer && !b.fort && !b.dtype_normal)) return at;
    auto ef = std::make_shared<Ataque>(*at);
    ef->dano = at->dano + (at->dano > 0 ? b.dano : 0);
    ef->pierce = (at->pierce + b.pierce) * (1.0 + b.pierce_pct);
    if (at->dano > 0) ef->cer = at->cer + b.cer, ef->fort = at->fort + b.fort;
    if (at->sdano) ef->sdano = at->sdano + b.dano;
    if (b.dtype_normal) {
        ef->dtype = DT_NORMAL;
        ef->sdtype = DT_NORMAL;
    }
    return ef;
}

void Pista::disparar(const TorreP& t, const AtaqueP& at0, double ang, Bloon* a) {
    AtaqueP at = ataque_efetivo(*t, at0);
    auto p = std::make_unique<Projetil>(nid(), t->x, t->y, ang, at, t);
    if (at->busca && a) p->alvo = a->shared_from_this();
    projeteis.push_back(std::move(p));
}

void Pista::aura(const TorreP& tp, const AtaqueP& at0, double raio) {
    Torre& t = *tp;
    AtaqueP atp = ataque_efetivo(t, at0);
    const Ataque& at = *atp;
    double pierce = at.pierce;
    const double r2 = raio * raio;
    bool acertou = false;
    const bool camo = t.detecta_camo() || at.retira_camo;
    for (Bloon* b : vizinhos(t.x, t.y, raio)) {
        if (pierce <= 0) break;
        if (!b->vivo || (b->camo && !camo)) continue;
        if (quad(b->x - t.x) + quad(b->y - t.y) <= r2) {
            if (aplicar_dano(*b, at.dano, at, &t)) {
                pierce -= 1;
                acertou = true;
                if (at.frag && b->cong_t > 0 && !b->vivo) fragmentar(b->x, b->y, at, tp);
            }
        }
    }
    if (acertou && at.visual != "nenhum") evento({"aura", t.x, t.y, raio, 0, 0, at.visual});
}

// raio que sai da torre na direcao de (ax, ay) e atravessa o mapa, ferindo o que estiver na linha
void Pista::raio_em_linha(Torre& t, const Ataque& at, double ax, double ay) {
    double dx = ax - t.x, dy = ay - t.y;
    double L = std::hypot(dx, dy);
    if (L == 0) L = 1.0, dx = 1.0;
    const double ux = dx / L, uy = dy / L;
    evento({"raio", t.x, t.y, 0, t.x + ux * 1600, t.y + uy * 1600, at.visual});
    std::vector<std::pair<double, BloonP>> atingidos;
    for (auto& b : bloons) {
        if (!b->vivo) continue;
        double px = b->x - t.x, py = b->y - t.y;
        double proj = px * ux + py * uy;
        if (proj < 0) continue;
        if (std::abs(px * uy - py * ux) <= 18 + b->tipo->raio * 0.5) atingidos.push_back({proj, b});
    }
    std::stable_sort(atingidos.begin(), atingidos.end(),
                     [](const auto& a, const auto& b) { return a.first < b.first; });
    const size_t n = std::min(atingidos.size(), static_cast<size_t>(std::max(0.0, at.pierce)));
    for (size_t i = 0; i < n; ++i) aplicar_dano(*atingidos[i].second, at.dano, at, &t);
}

void Pista::hitscan(const TorreP& tp, const AtaqueP& at0, Bloon& alvo_) {
    Torre& t = *tp;
    AtaqueP atp = ataque_efetivo(t, at0);
    const Ataque& at = *atp;
    if (at.linha) {
        raio_em_linha(t, at, alvo_.x, alvo_.y);
        return;
    }
    evento({"raio", t.x, t.y, 0, alvo_.x, alvo_.y, at.visual});
    const double x = alvo_.x, y = alvo_.y;
    aplicar_dano(alvo_, at.dano, at, &t);
    if (at.splash) explosao(x, y, at.splash, at.sdano, at.spierce, at, &t, at.sdtype);
    if (at.frag) fragmentar(x, y, at, tp);
    std::unordered_set<int> feitos = {alvo_.id};
    double ux = x, uy = y;
    for (int k = 0; k < static_cast<int>(at.quica); ++k) {
        Bloon* prox = mais_proximo(ux, uy, 160, feitos, t.detecta_camo());
        if (!prox) break;
        feitos.insert(prox->id);
        evento({"raio", ux, uy, 0, prox->x, prox->y, at.visual});
        ux = prox->x;
        uy = prox->y;
        aplicar_dano(*prox, at.dano, at, &t);
    }
}

void Pista::cadeia(const TorreP& tp, const AtaqueP& at0, Bloon& alvo_) {
    Torre& t = *tp;
    AtaqueP atp = ataque_efetivo(t, at0);
    const Ataque& at = *atp;
    std::unordered_set<int> feitos = {alvo_.id};
    double ux = t.x, uy = t.y;
    Bloon* atual = &alvo_;
    for (int k = 0; k < static_cast<int>(at.saltos) + 1; ++k) {
        evento({"raio", ux, uy, 0, atual->x, atual->y, "relampago"});
        ux = atual->x;
        uy = atual->y;
        aplicar_dano(*atual, at.dano, at, &t);
        Bloon* prox = mais_proximo(ux, uy, 140, feitos, true);
        if (!prox) break;
        feitos.insert(prox->id);
        atual = prox;
    }
}

Bloon* Pista::mais_proximo(double x, double y, double r, const std::unordered_set<int>& excluir, bool camo) {
    Bloon* melhor = nullptr;
    double md = r * r;
    for (Bloon* b : vizinhos(x, y, r)) {
        if (!b->vivo || excluir.count(b->id) || (b->camo && !camo)) continue;
        double d2 = quad(b->x - x) + quad(b->y - y);
        if (d2 < md) melhor = b, md = d2;
    }
    return melhor;
}

void Pista::explosao(double x, double y, double raio, double dano, double pierce, const Ataque& at, Torre* torre,
                     DType dtype) {
    evento({"explosao", x, y, raio, 0, 0, at.visual});
    if (!dtype) dtype = DT_EXPLOSAO;
    if (torre && torre->buff.dtype_normal) dtype = DT_NORMAL;
    int n = static_cast<int>(pierce);
    for (Bloon* b : vizinhos(x, y, raio + 40)) {
        if (n <= 0) break;
        if (!b->vivo) continue;
        const double rr = raio + b->tipo->raio * 0.6;
        if (quad(b->x - x) + quad(b->y - y) <= rr * rr)
            if (aplicar_dano(*b, dano, at, torre, nullptr, dtype)) n -= 1;
    }
}

void Pista::fragmentar(double x, double y, const Ataque& origem, const TorreP& torre) {
    const int n = origem.frag_n;
    for (int k = 0; k < n; ++k)
        projeteis.push_back(std::make_unique<Projetil>(nid(), x, y, k * 360.0 / n, origem.frag, torre, true));
}

// ---------------------------------------------------------------- projeteis
void Pista::passo_projeteis() {
    std::vector<std::unique_ptr<Projetil>> vivos;
    // fragmentos criados durante o laco entram no fim da lista e andam neste mesmo passo
    for (size_t i = 0; i < projeteis.size(); ++i) {
        Projetil& p = *projeteis[i];
        if (!p.vivo) continue;
        const Ataque& at = *p.at;
        if (p.fusivel > 0) {
            if (p.vx == 0 && p.vy == 0) {
                p.fusivel -= DT;
                if (p.fusivel <= 0) {
                    explosao(p.x, p.y, at.splash ? at.splash : 40, at.sdano ? at.sdano : 1,
                             at.spierce ? at.spierce : 20, at, p.torre.get(), at.sdtype);
                    if (at.tem_queima) {
                        auto fogo = std::make_shared<const Ataque>(
                            novo_ataque({{"dano", at.queima_dps}, {"dtype", "normal"}}));
                        pilhas.push_back(std::unique_ptr<Pilha>(
                            new Pilha{p.x, p.y, 30, fogo, p.torre, at.queima_t, {}, true, "chamas"}));
                    }
                    if (at.frag) fragmentar(p.x, p.y, at, p.torre);
                    p.vivo = false;
                    continue;
                }
                vivos.push_back(std::move(projeteis[i]));
                continue;
            }
        }
        // teleguiado
        if (p.alvo) {
            if (!p.alvo->vivo) {
                Bloon* novo = mais_proximo(p.x, p.y, 300, p.atingidos, true);
                p.alvo = novo ? novo->shared_from_this() : nullptr;
            }
            if (p.alvo) {
                double dx = p.alvo->x - p.x, dy = p.alvo->y - p.y;
                double L = std::hypot(dx, dy);
                if (L == 0) L = 1.0;
                const double k = 0.25;
                p.vx = p.vx * (1 - k) + dx / L * p.vel * k;
                p.vy = p.vy * (1 - k) + dy / L * p.vel * k;
                double n = std::hypot(p.vx, p.vy);
                if (n == 0) n = 1.0;
                p.vx = p.vx / n * p.vel;
                p.vy = p.vy / n * p.vel;
            }
        }
        if (at.boom) {
            mover_bumerangue(p);
        } else {
            p.x += p.vx * DT;
            p.y += p.vy * DT;
            p.dist -= p.vel * DT;
        }
        p.ang = graus(std::atan2(p.vy, p.vx));
        if (p.dist <= 0 || !(-60 < p.x && p.x < LARGURA_MAPA + 60 && -60 < p.y && p.y < ALTURA_MAPA + 60)) {
            fim_projetil(p);
            continue;
        }
        colidir(p);
        if (p.vivo) vivos.push_back(std::move(projeteis[i]));
    }
    projeteis = std::move(vivos);
}

void Pista::mover_bumerangue(Projetil& p) {
    if (!p.voltando) {
        p.x += p.vx * DT;
        p.y += p.vy * DT;
        p.dist -= p.vel * DT;
        if (p.dist <= p.at->dist * 0.5) {
            p.voltando = true;
        } else {
            // curva suave para o lado da mao que arremessou
            double ang = std::atan2(p.vy, p.vx) + (p.torre->mao ? -2.2 : 2.2) * DT;
            p.vx = std::cos(ang) * p.vel;
            p.vy = std::sin(ang) * p.vel;
        }
    } else {
        double dx = p.torre->x - p.x, dy = p.torre->y - p.y;
        double L = std::hypot(dx, dy);
        if (L == 0) L = 1.0;
        const double k = 0.18;
        p.vx = p.vx * (1 - k) + dx / L * p.vel * k;
        p.vy = p.vy * (1 - k) + dy / L * p.vel * k;
        double n = std::hypot(p.vx, p.vy);
        if (n == 0) n = 1.0;
        p.vx = p.vx / n * p.vel;
        p.vy = p.vy / n * p.vel;
        p.x += p.vx * DT;
        p.y += p.vy * DT;
        p.dist -= p.vel * DT * 0.5;
        if (L < 20) p.dist = 0;
    }
}

void Pista::fim_projetil(Projetil& p) {
    p.vivo = false;
    const Ataque& at = *p.at;
    if (at.frag && !p.frag_filho && (at.visual == "juggernaut" || at.visual == "bola_espinho"))
        fragmentar(p.x, p.y, at, p.torre);
}

void Pista::colidir(Projetil& p) {
    const Ataque& at = *p.at;
    const double r = p.raio;
    for (Bloon* b : vizinhos(p.x, p.y, r + 90)) {
        if (!b->vivo || p.atingidos.count(b->id)) continue;
        const double rr = r + b->tipo->raio;
        if (quad(b->x - p.x) + quad(b->y - p.y) > rr * rr) continue;
        p.atingidos.insert(b->id);
        const double bx = b->x, by = b->y;
        const bool consumiu = aplicar_dano(*b, at.dano, at, p.torre.get(), &p);
        if (at.splash) {
            explosao(bx, by, at.splash, at.sdano, at.spierce, at, p.torre.get(), at.sdtype);
            if (at.frag && !p.frag_filho) fragmentar(bx, by, at, p.torre);
        } else if (at.frag && !p.frag_filho && !b->vivo && at.visual != "juggernaut" &&
                   at.visual != "bola_espinho") {
            fragmentar(bx, by, at, p.torre);
        }
        if (consumiu) p.pierce -= 1;
        if (p.quicos > 0 && p.pierce > 0) {
            Bloon* prox = mais_proximo(bx, by, 200, p.atingidos, p.torre->detecta_camo());
            if (prox) {
                p.quicos -= 1;
                double dx = prox->x - bx, dy = prox->y - by;
                double L = std::hypot(dx, dy);
                if (L == 0) L = 1.0;
                p.vx = dx / L * p.vel;
                p.vy = dy / L * p.vel;
                p.dist = std::max(p.dist, 220.0);
            }
        }
        if (p.pierce <= 0) {
            p.vivo = false;
            return;
        }
    }
}

// ---------------------------------------------------------------- pilhas
void Pista::passo_pilhas() {
    if (pilhas.empty()) return;
    std::vector<std::unique_ptr<Pilha>> vivas;
    for (auto& sp : pilhas) {
        Pilha& s = *sp;
        s.vida -= DT;
        if (s.vida <= 0 || s.pierce <= 0) {
            const Ataque& at = *s.at;
            if (at.splash && at.tipo == TipoAtaque::PILHA)
                explosao(s.x, s.y, at.splash, at.sdano, at.spierce, at, s.torre.get(), DT_NORMAL);
            continue;
        }
        if (s.at->armadilha) {
            // a armadilha engole bloons inteiros ate encher a capacidade em RBE
            const bool camo = s.torre && s.torre->detecta_camo();
            for (Bloon* b : vizinhos(s.x, s.y, 30)) {
                if (!b->vivo || (b->camo && !camo)) continue;
                if (b->tipo->moab && (!s.at->prende_moab || b->tipo->nome == "bad")) continue;
                if (quad(b->x - s.x) + quad(b->y - s.y) > quad(14 + b->tipo->raio)) continue;
                const int r = rbe_restante(*b);
                if (r > s.pierce) continue;
                b->vivo = false;
                s.pierce -= r;
                pops_total += 1;
                receber(r * s.at->valor * mult_renda);
                evento({"pop", b->x, b->y, 0, 0, 0, b->tipo->nome});
            }
            vivas.push_back(std::move(sp));
            continue;
        }
        for (Bloon* b : vizinhos(s.x, s.y, 30)) {
            if (s.pierce <= 0) break;
            if (!b->vivo || s.atingidos.count(b->id)) continue;
            const double rr = 14 + b->tipo->raio;
            if (quad(b->x - s.x) + quad(b->y - s.y) <= rr * rr) {
                s.atingidos.insert(b->id);
                if (aplicar_dano(*b, s.at->dano, *s.at, s.torre.get())) s.pierce -= 1;
            }
        }
        vivas.push_back(std::move(sp));
    }
    pilhas = std::move(vivas);
}

// ---------------------------------------------------------------- movimento dos bloons
void Pista::mover_bloons() {
    if (buraco_t > 0) buraco_t -= DT;
    if (buraco_rec > 0) buraco_rec -= DT;
    const double base = VELOCIDADE_BASE * DT * mult_vel;
    const double glob = lentidao_global_t > 0 ? lentidao_global_f : 1.0;
    // bloons filhos criados por dano continuo entram no fim da lista e andam neste passo
    for (size_t i = 0; i < bloons.size(); ++i) {
        Bloon& b = *bloons[i];
        if (!b.vivo) continue;
        // dano continuo (cola corrosiva, fogo)
        if (b.queima_t > 0 || (b.cola_t > 0 && b.cola_dps)) {
            b.dot += (b.queima_t > 0 ? b.queima_dps : 0) * DT;
            if (b.cola_t > 0) b.dot += b.cola_dps * DT;
            if (b.dot >= 1.0) {
                int dano = static_cast<int>(b.dot);
                b.dot -= dano;
                aplicar_dano(b, dano, ataque_dot(), nullptr);
                if (!b.vivo) continue;
            }
        }
        if (b.queima_t > 0) b.queima_t -= DT;
        if (b.cong_t > 0) {
            b.cong_t -= DT;
            continue;
        }
        if (b.atord_t > 0) {
            b.atord_t -= DT;
            continue;
        }
        double v = base * b.tipo->velocidade * glob;
        if (b.lento_t > 0) {
            b.lento_t -= DT;
            v *= b.lento_f;
        }
        if (b.cola_t > 0) {
            b.cola_t -= DT;
            v *= b.cola_f;
        }
        b.d += v;
        const Caminho& cam = mapa.caminhos[b.cam];
        if (b.d >= cam.comprimento) {
            b.vivo = false;
            // Lenda da Noite: o primeiro bloon que ia vazar abre o buraco negro, que engole tudo por 8 s
            if (buraco_t <= 0 && buraco_rec <= 0) {
                for (auto& [id, t] : torres)
                    if (t->st.buraco_negro) {
                        buraco_t = 8.0;
                        buraco_rec = 120.0;
                        evento({"habilidade", t->x, t->y, 0, 0, 0, "Buraco Negro"});
                        break;
                    }
            }
            if (buraco_t > 0) {
                evento({"flash", b.x, b.y});
                continue;
            }
            int perda = rbe_restante(b);
            vidas -= perda;
            vazou += perda;
            evento({"vazou", b.x, b.y, static_cast<double>(perda)});
            continue;
        }
        Posicao p = cam.posicao(b.d);
        b.x = p.x;
        b.y = p.y;
        b.ang = p.ang;
        if (b.regen) {
            b.regen_t += DT;
            if (b.regen_t >= 3.0) {
                b.regen_t = 0.0;
                regenerar(b);
            }
        }
    }
}

void Pista::regenerar(Bloon& b) {
    auto it = regen_proximo().find(b.tipo->nome);
    if (it == regen_proximo().end()) return;
    const TipoBloon* prox = &tipo_bloon(it->second);
    const TipoBloon* orig = b.regen_orig;
    if (prox->rank > orig->rank) return;
    if (b.tipo->nome == "rosa" && (orig->nome == "branco" || orig->nome == "roxo")) prox = orig;
    b.tipo = prox;
    b.vida = b.fort && b.tipo->vida_fortificado ? b.tipo->vida_fortificado : b.tipo->vida;
    evento({"regen", b.x, b.y});
}

// ---------------------------------------------------------------- rodada
void Pista::receber(double v) {
    v *= mult_dinheiro;
    if (divida > 0) {
        const double pago = std::min(divida, v * 0.5);
        divida -= pago;
        v -= pago;
    }
    dinheiro += v;
}

void Pista::pagar_renda() {
    if (so_estouro_e_rodada) return;  // CHIMPS: fazendas, bancos e renda de heroi nao pagam
    for (auto& [id, t] : torres) {
        if (t->renda_sacrificio > 0) {
            receber(t->renda_sacrificio);
            evento({"dinheiro", t->x, t->y, std::floor(t->renda_sacrificio)});
        }
        for (const Ataque& at : t->st.ataques) {
            if (at.tipo == TipoAtaque::RENDA && at.valor) {
                // Fazenda de bananas: a renda cai no chao em 4 cachos e o jogador coleta. Banco (caminho 2,
                // tier 3) e Mercado (caminho 3, tier 3) depositam direto, como as outras rendas.
                const bool cai = t->chave == "fazenda" && t->caminhos[1] < 3 && t->caminhos[2] < 3;
                if (const double teto = teto_banco(*t); teto > 0) {
                    // Banco: a renda da rodada entra no saldo, que rende 20% de juros. Cheio, ele se
                    // saca sozinho (no jogo real o dinheiro a mais se perderia).
                    t->banco = (t->banco + at.valor) * 1.20;
                    if (t->banco >= teto) {
                        receber(teto);
                        evento({"dinheiro", t->x, t->y, teto});
                        t->banco = 0;
                    }
                    continue;
                }
                if (!cai) {
                    receber(at.valor);
                    evento({"dinheiro", t->x, t->y, std::floor(at.valor)});
                    continue;
                }
                const double vida = t->caminhos[1] >= 1 ? 30.0 : 15.0;  // Bananas Duradouras
                for (int k = 0; k < 4; ++k) {
                    const double a = rng.uniform(0, 2 * PI), r = rng.uniform(38, 70);
                    soltar(t->x + std::cos(a) * r, t->y + std::sin(a) * r, at.valor / 4, vida, "banana");
                }
            }
        }
    }
}

std::uint32_t Pista::hash() const {
    double soma_d = 0;
    for (auto& b : bloons) soma_d += b->d;
    char buf[256];
    std::snprintf(buf, sizeof buf, "(%lld, %d, %zu, %zu, %lld, %d, %zu, %zu)", static_cast<long long>(dinheiro), vidas,
                  bloons.size(), torres.size(), static_cast<long long>(soma_d), pops_total, projeteis.size(),
                  coletaveis.size());
    return crc32(buf);
}

// ================================================================ Partida
Partida::Partida(const std::string& modo_, const std::string& chave_mapa, int seed_, const std::string& dif,
                 const std::map<int, std::string>& herois)
    : modo(modo_), mapa(bl::mapa(chave_mapa)), seed(seed_) {
    if (modo == "solo") {
        const Dificuldade* d = achar_dificuldade(dif);
        if (!d) throw std::out_of_range("dificuldade desconhecida: " + dif);
        dificuldade = dif;
        ultima_rodada = d->ultima_rodada;
        sandbox = d->sandbox;
        rodada = d->primeira_rodada - 1;
        pistas[1] = std::make_unique<Pista>(1, mapa, seed, d->vidas, d->dinheiro_inicial, d->mult_custo);
        Pista& p = *pistas[1];
        p.xp_por_estouro = false;
        p.mult_dinheiro = d->mult_dinheiro;
        p.sem_venda = d->sem_venda;
        p.so_estouro_e_rodada = d->so_estouro_e_rodada;
        p.mult_vel_dificuldade = d->mult_vel;
        p.mult_vel = d->mult_vel;
    } else {
        dificuldade = "medio";
        ultima_rodada = 1000000000;
        pistas[1] = std::make_unique<Pista>(1, mapa, seed, VIDAS_BATALHA, DINHEIRO_INICIAL);
        pistas[2] = std::make_unique<Pista>(2, mapa, seed, VIDAS_BATALHA, DINHEIRO_INICIAL);
        pistas[1]->oponente = pistas[2].get();
        pistas[2]->oponente = pistas[1].get();
    }
    for (auto& [j, h] : herois)
        if (pistas.count(j) && achar_heroi(h)) pistas[j]->heroi_escolhido = h;
}

char Partida::aplicar(int jogador, const std::string& cmd) {
    auto it = pistas.find(jogador);
    if (it == pistas.end() || fim || cmd.empty()) return ERRO_INVALIDO;
    Pista& pista = *it->second;
    const char c = cmd[0];
    const std::string corpo = cmd.substr(1);
    char erro = ERRO_INVALIDO;
    try {
        auto dividir = [&](char sep) {
            size_t k = corpo.find(sep);
            if (k == std::string::npos) throw std::invalid_argument("comando");
            return std::make_pair(corpo.substr(0, k), corpo.substr(k + 1));
        };
        if (c == 'T') {
            auto [chave, xy] = dividir('@');
            size_t k = xy.find(',');
            if (k == std::string::npos) throw std::invalid_argument("comando");
            erro = pista.colocar_torre(chave, std::stoi(xy.substr(0, k)), std::stoi(xy.substr(k + 1)));
        } else if (c == 'U') {
            auto [tid, p] = dividir(':');
            erro = pista.upar(std::stoi(tid), std::stoi(p));
        } else if (c == 'V') {
            erro = pista.vender(std::stoi(corpo));
        } else if (c == 'M') {
            auto [tid, m] = dividir(':');
            erro = pista.mudar_modo(std::stoi(tid), std::stoi(m));
        } else if (c == 'A') {
            auto [tid, xy] = dividir('@');
            size_t k = xy.find(',');
            if (k == std::string::npos) throw std::invalid_argument("comando");
            erro = pista.mirar(std::stoi(tid), std::stoi(xy.substr(0, k)), std::stoi(xy.substr(k + 1)));
        } else if (c == 'O') {
            auto [tid, v] = dividir(':');
            erro = pista.opcao(std::stoi(tid), std::stoi(v));
        } else if (c == 'C') {
            erro = pista.coletar(std::stoi(corpo));
        } else if (c == 'B') {
            auto [tid, i] = dividir(':');
            erro = pista.usar_habilidade(std::stoi(tid), std::stoi(i));
        } else if (c == 'X') {
            erro = comando_sandbox(corpo);
        } else if (c == 'S') {
            erro = enviar(jogador, corpo);
        } else if (c == 'N') {
            erro = iniciar_rodada();
        }
    } catch (const std::exception&) {
        erro = ERRO_INVALIDO;
    }
    if (erro) ultimos_erros[jogador] = erro;
    return erro;
}

char Partida::enviar(int jogador, const std::string& chave) {
    if (modo != "batalha") return ERRO_INVALIDO;
    const Envio* env = achar_envio(chave);
    if (!env) return ERRO_INVALIDO;
    if (rodada < env->rodada_min) return ERRO_BLOQUEADO;
    auto it = envio_rec.find({jogador, chave});
    if (it != envio_rec.end() && it->second > tempo) return ERRO_BLOQUEADO;
    Pista& pista = *pistas[jogador];
    if (pista.dinheiro < env->custo) return ERRO_DINHEIRO;
    pista.dinheiro -= env->custo;
    pista.eco = std::max(0.0, pista.eco + env->eco);
    envio_rec[{jogador, chave}] = tempo + 0.6;
    Pista* alvo = pista.oponente;
    for (int k = 0; k < env->qtd; ++k) alvo->agendar(env->tipo, 0.3 + k * env->espaco, env->camo, env->regen, env->fort);
    pista.evento({"envio", 0, 0, 0, 0, 0, env->nome});
    return OK;
}

char Partida::comando_sandbox(const std::string& corpo) {
    if (!sandbox || corpo.empty()) return ERRO_INVALIDO;
    Pista& p = *pistas[1];
    std::vector<std::string> partes;
    for (size_t ini = 0;;) {
        const size_t k = corpo.find(':', ini);
        partes.push_back(corpo.substr(ini, k == std::string::npos ? k : k - ini));
        if (k == std::string::npos) break;
        ini = k + 1;
    }
    const std::string& op = partes[0];
    if (op == "b" && partes.size() >= 3) {
        const TipoBloon* tp = achar_bloon(partes[1]);
        const int qtd = std::stoi(partes[2]);
        if (!tp || qtd < 1 || qtd > 200) return ERRO_INVALIDO;
        const std::string mods = partes.size() > 3 ? partes[3] : "";
        const bool camo = mods.find('c') != std::string::npos, regen = mods.find('r') != std::string::npos,
                   fort = mods.find('f') != std::string::npos;
        // dirigiveis saem mais espacados para nao empilhar na entrada
        const double espaco = tp->moab ? 0.8 : 0.15;
        for (int k = 0; k < qtd; ++k) p.agendar(tp->nome, 0.1 + k * espaco, camo, regen, fort);
        return OK;
    }
    if (op == "r" && partes.size() == 2) {
        const int r = std::stoi(partes[1]);
        if (r < 1 || r > 1000) return ERRO_INVALIDO;
        rodada = r;
        p.mult_renda = mult_renda_da_rodada(rodada);
        p.mult_vida = mult_vida_moab(rodada);
        p.mult_vel = mult_velocidade(rodada) * p.mult_vel_dificuldade;
        p.freeplay = rodada > 80;
        for (auto& [t, g] : agenda_da_rodada(rodada)) p.agendar(g.tipo, t, g.camo, g.regen, g.fort);
        return OK;
    }
    if (op == "l") {
        p.fila.clear();
        for (auto& b : p.bloons) b->vivo = false;
        return OK;
    }
    if (op == "t") {
        p.projeteis.clear();
        p.pilhas.clear();
        p.torres.clear();
        p.tem_heroi = false;
        p.buff_t = 0.0;
        return OK;
    }
    if (op == "h") {
        for (auto& [id, t] : p.torres)
            for (double& r : t->hab_rec) r = 0.0;
        return OK;
    }
    return ERRO_INVALIDO;
}

bool Partida::continuar_freeplay() {
    if (modo != "solo" || !fim || vencedor != 1) return false;
    fim = false;
    vencedor = 0;
    em_freeplay = true;
    ultima_rodada = 1000000000;
    return true;
}

char Partida::iniciar_rodada() {
    if (modo != "solo" || em_rodada || fim) return ERRO_INVALIDO;
    rodada += 1;
    em_rodada = true;
    Pista& p = *pistas[1];
    p.mult_renda = mult_renda_da_rodada(rodada);
    p.mult_vida = mult_vida_moab(rodada);
    p.mult_vel = mult_velocidade(rodada) * p.mult_vel_dificuldade;
    p.freeplay = rodada > 80;
    for (auto& [t, g] : agenda_da_rodada(rodada)) p.agendar(g.tipo, t, g.camo, g.regen, g.fort);
    return OK;
}

void Partida::passo() {
    if (fim) return;
    tick += 1;
    tempo += DT;
    for (auto& [j, p] : pistas) p->passo();
    if (modo == "solo") passo_solo();
    else passo_batalha();
}

void Partida::passo_solo() {
    Pista& p = *pistas[1];
    if (sandbox) {
        // nada acaba no Sandbox: repoe o que foi gasto ou perdido a cada passo
        p.vidas = 999999;
        p.dinheiro = 9999999;
    }
    if (p.vidas <= 0) {
        fim = true;
        vencedor = 0;
        return;
    }
    if (em_rodada && p.fila.empty() && p.bloons.empty()) {
        em_rodada = false;
        p.receber(100 + rodada);
        p.pagar_renda();
        // no freeplay (depois de vencer) a XP cai 70% ate a R100 e 90% depois
        const double corte = !em_freeplay ? 1.0 : rodada <= 100 ? 0.3 : 0.1;
        p.xp(xp_da_rodada(rodada) * mult_xp_mapa(mapa.def.dificuldade) * corte);
        p.evento({"fim_rodada", 0, 0, static_cast<double>(rodada)});
        if (rodada >= ultima_rodada) {
            fim = true;
            vencedor = 1;
            return;
        }
        if (automatico) iniciar_rodada();
    }
}

void Partida::passo_batalha() {
    if (tempo >= prox_eco_t) {
        prox_eco_t += ECO_INTERVALO;
        for (auto& [j, p] : pistas) {
            p->receber(p->eco);
            p->evento({"eco", 0, 0, std::floor(p->eco)});
        }
    }
    if (tempo >= prox_rodada_t) {
        rodada += 1;
        auto ag = agenda_da_rodada(rodada);
        for (auto& [j, p] : pistas) {
            for (auto& [t, g] : ag) p->agendar(g.tipo, t, g.camo, g.regen, g.fort);
            p->pagar_renda();
            p->xp(10 + rodada);
            p->evento({"fim_rodada", 0, 0, static_cast<double>(rodada)});
        }
        prox_rodada_t = tempo + duracao_rodada(rodada) + PAUSA_ENTRE_RODADAS;
    }
    const int v1 = pistas[1]->vidas, v2 = pistas[2]->vidas;
    if (v1 <= 0 || v2 <= 0) {
        fim = true;
        if (v1 <= 0 && v2 <= 0) vencedor = v1 > v2 ? 1 : v2 > v1 ? 2 : 0;
        else vencedor = v1 <= 0 ? 2 : 1;
    }
}

double Partida::tempo_para_rodada() const {
    return modo != "batalha" ? 0.0 : std::max(0.0, prox_rodada_t - tempo);
}

double Partida::tempo_para_eco() const { return modo != "batalha" ? 0.0 : std::max(0.0, prox_eco_t - tempo); }

double Partida::recarga_envio(int jogador, const std::string& chave) const {
    auto it = envio_rec.find({jogador, chave});
    return it == envio_rec.end() ? 0.0 : std::max(0.0, it->second - tempo);
}

std::uint32_t Partida::hash() const {
    std::uint32_t h = static_cast<std::uint32_t>(tick);
    for (auto& [j, p] : pistas) h = crc32(std::to_string(p->hash()), h);
    return h;
}

}  // namespace bl
