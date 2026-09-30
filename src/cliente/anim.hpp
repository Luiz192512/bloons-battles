// Animacoes de disparo e de habilidade dos macacos (so no cliente).
//
// Cada animacao e um clipe: trilhas de keyframes por canal (recuo, giro do braco, escala...)
// com curva de suavizacao em cada trecho. Os clipes sao dados em tabela, nao codigo espalhado
// no render, e podem ser vistos em loop na vitrine (--vitrine, pagina Animacoes).
//
// O Animador so LE a simulacao: descobre que uma torre atirou quando a recarga de um ataque
// volta a subir, e que usou a habilidade quando hab_rec reinicia. Nada aqui escreve na Pista,
// entao o modo Batalha continua deterministico.
#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include "cliente/caneta.hpp"
#include "jogo/sim.hpp"

namespace bl::anim {

enum class Curva { LINEAR, ENTRA_QUAD, SAI_QUAD, SAI_CUBICA, ENTRA_SAI_CUBICA, SAI_VOLTA, SAI_ELASTICA };
float suavizar(Curva c, float x);
const char* nome_curva(Curva c);

enum Canal {
    RECUO,        // canos das maquinas (0..1)
    GIRO,         // graus no braco de ataque
    ESTICA,       // avanco do braco (unidades da caixa 128)
    FLASH,        // clarao na ponta do item (0..1)
    ESCALA,       // escala do corpo inteiro
    SALTO,        // pulo do corpo (px na tela, negativo = para cima)
    CORPO_RECUO,  // o corpo todo recua na direcao contraria ao tiro (px)
    BRILHO,       // brilho em volta da torre (0..1)
    ONDA,         // onda de choque: raio relativo (0..1)
    ONDA_ALFA,    // opacidade da onda (0..1)
    N_CANAIS
};
const char* nome_canal(Canal c);

struct Chave {
    float t, v;
    Curva curva = Curva::LINEAR;  // curva do trecho que TERMINA nesta chave
};

struct Trilha {
    std::vector<Chave> chaves;
    float valor(float t, float padrao) const;
};

struct Clipe {
    std::string nome;
    float dur = 0.3f;
    std::array<Trilha, N_CANAIS> trilhas;
    std::string efeito;  // so nos clipes de habilidade (cor da onda)
    Clipe& chave(Canal c, float t, float v, Curva cv = Curva::LINEAR) {
        trilhas[c].chaves.push_back({t, v, cv});
        return *this;
    }
};

float padrao_canal(Canal c);

// Valores de um instante: pose para a caneta + efeitos em volta da torre.
struct Quadro {
    spr::Pose pose;
    float escala = 1, salto = 0, corpo_recuo = 0, brilho = 0, onda = 0, onda_alfa = 0;
    Color cor = WHITE;  // cor da habilidade (brilho e onda)
    bool habilidade = false;
    // mira na vista 3/4: espelho (com a animacao de virada) e inclinacao do corpo
    float sx = 1, inclina = 0;
};
Quadro avaliar(const Clipe& c, float t);

// Como cada torre mira no mapa sem deitar o sprite 3/4.
enum class TipoMira {
    MACACO,   // vira de lado, braco aponta para o alvo, corpo inclina de leve
    TORRETA,  // maquina parada; so a torreta/canos giram (bomba, sentinela, churchill)
    ESPELHA,  // navios e aeronaves: so viram para o lado do alvo ou do movimento
    FIXA,     // construcoes e tubos simetricos: nao mudam
};
TipoMira tipo_mira(const std::string& chave);

// Estado da mira de uma torre (so no cliente). Le o angulo de mira da simulacao (Torre::ang).
class Mira {
public:
    static constexpr float DUR_VIRADA = 0.16f;  // s
    static constexpr float BRACO_MIN = -50, BRACO_MAX = 75, INCLINA_MAX = 7;
    void atualizar(const std::string& chave, double ang_graus, double agora);
    void aplicar(Quadro& q, double agora) const;

private:
    TipoMira tipo_ = TipoMira::MACACO;
    bool iniciada_ = false;
    int lado_ = 1, lado_antes_ = 1;
    double t_virada_ = -100, ultimo_ = 0;
    float braco_ = 0, inclina_ = 0, torreta_ = 0;
};

// Biblioteca de clipes
const Clipe& clipe_disparo(const std::string& chave_torre);
const Clipe& clipe_habilidade(const std::string& efeito);
const std::vector<const Clipe*>& todos_os_clipes();

class Animador {
public:
    // Chamar uma vez por quadro, depois de a simulacao avancar.
    void observar(const Pista& pista, double agora);
    Quadro quadro(int torre_id, double agora) const;
    // Depuracao (F9 na partida / vitrine): toca um clipe numa torre sem mexer na simulacao.
    void tocar_disparo(int torre_id, const std::string& chave, double agora);
    void tocar_habilidade(int torre_id, const std::string& efeito, double agora);

private:
    struct Estado {
        Mira mira;
        std::vector<double> rec, hab;
        const Clipe* disparo = nullptr;
        double t_disparo = -1;
        const Clipe* hab_clipe = nullptr;
        double t_hab = -1;
        Color cor = WHITE;
    };
    std::unordered_map<int, Estado> est_;
};

}  // namespace bl::anim
