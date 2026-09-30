// Simulacao deterministica do jogo.
//
// A mesma sequencia de comandos gera exatamente o mesmo estado em qualquer processo.
// E isso que permite o modo Batalha em lockstep: o servidor so ordena os comandos e
// cada cliente simula as duas pistas localmente.
//
// Unidades: px e segundos. Passo fixo DT = 1/30 s.
#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "jogo/defs.hpp"
#include "jogo/mapas.hpp"
#include "jogo/stats.hpp"

namespace bl {

constexpr double DT = 1.0 / 30.0;
constexpr int CELULA = 64;
constexpr int MAX_PILHAS_POR_TORRE = 30;
constexpr size_t MAX_EVENTOS = 400;
extern const char* const MODOS_ALVO[4];  // primeiro, ultimo, perto, forte

// Batalha
constexpr int VIDAS_BATALHA = 150;
constexpr int DINHEIRO_INICIAL = 650;
constexpr double ECO_INICIAL = 250.0;
constexpr double ECO_INTERVALO = 6.0;
constexpr double PAUSA_ENTRE_RODADAS = 5.0;

// Codigos de erro devolvidos pelos comandos ('\0' = ok)
constexpr char OK = '\0';
constexpr char ERRO_DINHEIRO = 'D';
constexpr char ERRO_POSICAO = 'P';
constexpr char ERRO_INVALIDO = 'M';
constexpr char ERRO_HEROI = 'H';
constexpr char ERRO_BLOQUEADO = 'B';

// Gerador pseudoaleatorio proprio (SplitMix64): mesma sequencia em qualquer compilador.
class Rng {
public:
    explicit Rng(std::uint64_t semente) : estado_(semente) {}
    std::uint64_t proximo();
    double random();                       // [0, 1)
    double uniform(double a, double b);    // [a, b)
    int randrange(int n);                  // [0, n)

private:
    std::uint64_t estado_;
};

// Algo que aconteceu na simulacao e que a tela pode querer mostrar (som, efeito, aviso).
struct Evento {
    std::string tipo;
    double x = 0, y = 0;
    double v = 0;            // valor (dinheiro, nivel, perda, raio...)
    double x2 = 0, y2 = 0;   // destino (raios, morteiro)
    std::string s;           // texto (nome, visual)
    Cor cor{};
};

struct Bloon : std::enable_shared_from_this<Bloon> {
    int id;
    const TipoBloon* tipo;
    double d;
    int cam;
    double vida, vida_max;
    bool camo, regen, fort;
    const TipoBloon* regen_orig;
    double lento_f = 1.0, lento_t = 0.0, cong_t = 0.0;
    double cola_f = 1.0, cola_t = 0.0, cola_dps = 0.0;
    double queima_dps = 0.0, queima_t = 0.0, atord_t = 0.0;
    double frag = 0;
    double regen_t = 0.0, dot = 0.0;
    double x = 0, y = 0, ang = 0;
    bool vivo = true;
};
using BloonP = std::shared_ptr<Bloon>;
using AtaqueP = std::shared_ptr<const Ataque>;

struct Torre {
    Torre(int id, const std::string& chave, int dono, double x, double y, double custo, double temporaria = 0.0);
    void recalcular(int extra_nivel_inv = 0);
    double alcance() const { return st.alcance * (1.0 + buff.alcance_pct); }
    bool detecta_camo() const { return st.camo || buff.camo; }

    int id;
    std::string chave;
    const DefTorre* dfn;
    int dono;
    double x, y, cx, cy;
    std::array<int, 3> caminhos{0, 0, 0};
    int nivel;
    double xp = 0.0;
    int pops = 0;
    double investido;
    int modo = 0;
    double ang = -90.0;
    double turbo = 1.0, turbo_t = 0.0;
    double temporaria;
    double orbita = 0.0;
    Buffs buff;
    std::vector<std::pair<int, double>> pontos_trilha;
    Stats st;
    std::vector<AtaqueP> ats;  // st.ataques congelados (projeteis guardam o ataque que os criou)
    std::vector<double> recargas;
    std::vector<double> hab_rec;
};
using TorreP = std::shared_ptr<Torre>;

struct Projetil {
    Projetil(int id, double x, double y, double ang, AtaqueP at, TorreP torre, bool frag_filho = false);
    int id;
    double x, y, vx, vy, vel;
    AtaqueP at;
    TorreP torre;
    double pierce, dist, raio;
    std::unordered_set<int> atingidos;
    BloonP alvo;
    bool voltando = false;
    double ox, oy, ang;
    bool vivo = true;
    double fusivel, quicos;
    bool frag_filho;
};

// Espinhos, estrepes, chamas ou poca de acido parados na trilha.
struct Pilha {
    double x, y, pierce;
    AtaqueP at;
    TorreP torre;
    double vida;
    std::unordered_set<int> atingidos;
    bool vivo = true;
    std::string visual;
};

struct Agendado {
    double t;
    std::string nome;
    bool camo, regen, fort;
};

// O mapa de um jogador: bloons, torres, projeteis e economia.
class Pista {
public:
    Pista(int dono, const Mapa& mapa, int seed, int vidas, int dinheiro, double mult_custo = 1.0);

    // consultas
    int custo(int base) const;
    int custo(int base, double x, double y) const;
    std::optional<int> custo_upgrade(const Torre& t, int p) const;
    int valor_venda(const Torre& t) const;
    bool posicao_valida(const DefTorre& dfn, double x, double y) const;
    TorreP torre(int id) const;
    bool bloons_ou_fila() const { return !bloons.empty() || !fila.empty(); }
    std::uint32_t hash() const;

    // comandos
    char colocar_torre(const std::string& chave, double x, double y);
    char upar(int tid, int p);
    char vender(int tid);
    char mudar_modo(int tid, int modo);
    char usar_habilidade(int tid, int idx);

    // bloons
    BloonP criar_bloon(const std::string& nome, double d = 0.0, int cam = -1, bool camo = false,
                       bool regen = false, bool fort = false, const TipoBloon* orig = nullptr);
    void agendar(const std::string& nome, double atraso, bool camo = false, bool regen = false, bool fort = false);
    bool aplicar_dano(Bloon& b, double dano, const Ataque& at, Torre* torre, Projetil* proj = nullptr,
                      DType dtype = 0);
    int rbe_restante(const Bloon& b) const;
    void xp(double v);
    void pagar_renda();
    void receber(double v);  // entrada de dinheiro; metade vai para a divida do emprestimo
    void evento(Evento e);

    void passo();

    int dono;
    const Mapa& mapa;
    Rng rng;
    int vidas;
    double dinheiro;
    double eco = ECO_INICIAL;
    double divida = 0.0;      // IMF Loan
    double mult_renda = 1.0;  // dinheiro por estouro conforme a rodada (BTD6)
    double mult_vida = 1.0;   // vida dos dirigiveis no freeplay
    double mult_vel = 1.0;    // velocidade dos bloons no freeplay
    double mult_custo;
    std::vector<BloonP> bloons;
    std::vector<std::unique_ptr<Projetil>> projeteis;
    std::vector<std::unique_ptr<Pilha>> pilhas;
    std::map<int, TorreP> torres;
    std::vector<Agendado> fila;
    double tempo = 0.0;
    int prox_id = 1;
    int prox_torre = 1;
    std::vector<Evento> eventos;
    std::string heroi_escolhido;
    bool tem_heroi = false;
    int pops_total = 0;
    int vazou = 0;
    double buff_t = 0.0;
    double lentidao_global_t = 0.0, lentidao_global_f = 1.0;
    Pista* oponente = nullptr;
    int proximo_cam = 0;

private:
    int nid() { return ++prox_id; }
    void posicionar(Bloon& b);
    void estourar(Bloon& b, double excesso, DType dtype, Torre* torre, Projetil* proj, int profundidade = 0);
    void preparar_trilha(Torre& t);
    void executar_habilidade(const TorreP& t, const J& h);
    std::pair<int, double> ponto_trilha_mais_avancado(const Torre& t) const;
    void invocar(const Torre& t, const std::string& base, double dur, const J& nivel);
    void spawns();
    void grade();
    std::vector<Bloon*> vizinhos(double x, double y, double r) const;
    void recalcular_buffs();
    Bloon* alvo(const Torre& t, const Ataque& at, double alcance) const;
    void mover_torre(Torre& t);
    void passo_torre(const TorreP& t);
    AtaqueP ataque_efetivo(const Torre& t, const AtaqueP& at) const;
    void disparar(const TorreP& t, const AtaqueP& at, double ang, Bloon* alvo = nullptr);
    void aura(const TorreP& t, const AtaqueP& at, double raio);
    void hitscan(const TorreP& t, const AtaqueP& at, Bloon& alvo);
    void cadeia(const TorreP& t, const AtaqueP& at, Bloon& alvo);
    Bloon* mais_proximo(double x, double y, double r, const std::unordered_set<int>& excluir, bool camo);
    void explosao(double x, double y, double raio, double dano, double pierce, const Ataque& at, Torre* torre,
                  DType dtype);
    void fragmentar(double x, double y, const Ataque& at, const TorreP& torre);
    void passo_projeteis();
    void mover_bumerangue(Projetil& p);
    void fim_projetil(Projetil& p);
    void colidir(Projetil& p);
    void passo_pilhas();
    void mover_bloons();
    void regenerar(Bloon& b);

    std::unordered_map<std::int64_t, std::vector<Bloon*>> grade_;
};

// Uma partida: solo (1 pista) ou batalha (2 pistas).
class Partida {
public:
    Partida(const std::string& modo, const std::string& mapa, int seed = 1, const std::string& dificuldade = "medio",
            const std::map<int, std::string>& herois = {});

    // Aplica um comando textual (sem origem): T, U, V, M, B, S, N.
    char aplicar(int jogador, const std::string& cmd);
    char enviar(int jogador, const std::string& chave);
    char iniciar_rodada();
    void passo();
    double tempo_para_rodada() const;
    double tempo_para_eco() const;
    // Segundos ate o jogador poder repetir este envio (0 = liberado). So leitura, para a interface.
    double recarga_envio(int jogador, const std::string& chave) const;
    std::uint32_t hash() const;
    Pista& pista(int j) { return *pistas.at(j); }
    const Pista& pista(int j) const { return *pistas.at(j); }

    std::string modo;
    const Mapa& mapa;
    int seed;
    long tick = 0;
    double tempo = 0.0;
    int rodada = 0;
    bool em_rodada = false;
    bool fim = false;
    int vencedor = -1;  // -1 = ninguem ainda, 0 = empate/derrota no solo
    bool automatico = false;
    std::string dificuldade;
    int ultima_rodada;
    std::map<int, std::unique_ptr<Pista>> pistas;
    std::map<int, char> ultimos_erros;

private:
    void passo_solo();
    void passo_batalha();
    double prox_rodada_t = 3.0;
    double prox_eco_t = ECO_INTERVALO;
    std::map<std::pair<int, std::string>, double> envio_rec;
};

std::uint32_t crc32(const std::string& dados, std::uint32_t valor = 0);

}  // namespace bl
