// Caneta de desenho por primitivas, porta da biblioteca "sprites2.js" do projeto de design.
//
// Todo sprite e desenhado numa caixa 128x128 (dirigiveis: 220x120) com circulos, elipses,
// retangulos arredondados, poligonos, linhas, setores, aneis e texto. O contorno em TINTA e a
// mesma forma desenhada maior por baixo. Grupos aplicam mover/girar/escalar e podem ter uma
// animacao em loop (gira, balanca, flutua...).
//
// Para caber no cache de RenderTexture, a caneta desenha por camadas: as partes paradas de um
// sprite (segmentos entre grupos animados) viram texturas, e so os grupos animados sao
// desenhados a cada quadro, na mesma ordem.
#pragma once

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <vector>

#include "raylib.h"

namespace bl::spr {

constexpr Color TINTA{22, 20, 26, 255};
constexpr Color NENHUMA{0, 0, 0, 0};  // "fill: none"
constexpr float B = 5, b3 = 3;         // bordas padrao do design

inline Color H(uint32_t rgb, int a = 255) {
    return {static_cast<unsigned char>(rgb >> 16), static_cast<unsigned char>(rgb >> 8), static_cast<unsigned char>(rgb),
            static_cast<unsigned char>(a)};
}
Color esc(Color c, float k = 0.6f);   // escurece multiplicando os canais
Color clar(Color c, float k = 0.35f);  // clareia em direcao ao branco
Color al(Color c, float a);           // troca o alfa (0..1)

float onda(float x);   // 0 -> 1 -> 0 a cada periodo (triangulo)
float recuo(float x);  // sobe em 10% do periodo e volta devagar

// Animacao em loop de um grupo (os mesmos tipos do design).
struct Anim {
    enum Tipo : uint8_t { NENHUMA, GIRA, BALANCA, FLUTUA, RECUO, VOA, PULSA, CRESCE, PISCA, SOME, BRACO } t = NENHUMA;
    float dur = 1, amp = 0, dx = 0, dy = 0, s0 = 1, s1 = 1, a0 = 1, a1 = 1, atraso = 0;
    int dir = 1;

    static Anim gira(float dur, int dir = 1) { Anim a; a.t = GIRA; a.dur = dur; a.dir = dir; return a; }
    static Anim balanca(float amp, float dur) { Anim a; a.t = BALANCA; a.amp = amp; a.dur = dur; return a; }
    static Anim flutua(float dy, float dur, float dx = 0, float atraso = 0) {
        Anim a; a.t = FLUTUA; a.dy = dy; a.dx = dx; a.dur = dur; a.atraso = atraso; return a;
    }
    static Anim recuo(float dy, float dur) { Anim a; a.t = RECUO; a.dy = dy; a.dur = dur; return a; }
    static Anim voa(float dx, float dy, float dur) { Anim a; a.t = VOA; a.dx = dx; a.dy = dy; a.dur = dur; return a; }
    static Anim pulsa(float s0, float s1, float dur, float atraso = 0) {
        Anim a; a.t = PULSA; a.s0 = s0; a.s1 = s1; a.dur = dur; a.atraso = atraso; return a;
    }
    static Anim cresce(float s0, float s1, float dur) { Anim a; a.t = CRESCE; a.s0 = s0; a.s1 = s1; a.dur = dur; return a; }
    static Anim pisca(float a0, float a1, float dur) { Anim a; a.t = PISCA; a.a0 = a0; a.a1 = a1; a.dur = dur; return a; }
    static Anim some(float dur) { Anim a; a.t = SOME; a.dur = dur; return a; }
    // braco que ataca: no jogo segue a pose do clipe de disparo; parado, balanca como no design
    static Anim braco(float amp, float dur) { Anim a; a.t = BRACO; a.amp = amp; a.dur = dur; return a; }
};

// Transformacao de um grupo: mover, girar e escalar (nessa ordem, como no SVG).
struct G {
    float x = 0, y = 0, rot = 0, sx = 1, sy = 1;
    Anim a;
    G& em(float px, float py) { x = px; y = py; return *this; }
    G& gira(float r) { rot = r; return *this; }
    G& esc(float s) { sx = sy = s; return *this; }
    G& esc(float ex, float ey) { sx = ex; sy = ey; return *this; }
    G& anim(Anim an) { a = an; return *this; }
};
inline G g_em(float x, float y) { return G().em(x, y); }

// Pose vinda do animador (clipes de disparo e habilidade). Tudo zero = parado.
struct Pose {
    bool ativa = false;   // algum clipe tocando
    float recuo = 0;      // 0..1 do recuo dos canos (maquinas)
    float giro = 0;       // graus extras no braco de ataque
    float estica = 0;     // avanco do braco (unidades da caixa 128)
    float flash = 0;      // 0..1 clarao na ponta do item
    float ergue = 0;      // 0..1 braco erguido (habilidade)
    // mira no mapa (vista 3/4): o braco de ataque aponta para o alvo e a torreta das maquinas gira
    bool mirando = false;
    float mira = 0;       // graus no braco de ataque, somados ao giro do clipe
    float torreta = 0;    // graus da torreta/canos (maquinas)
};

class Caneta {
public:
    enum class Modo { TUDO, CONTAR, ESTATICO, VIVO };

    // ---- configuracao do passe
    Modo modo = Modo::TUDO;
    int alvo = 0;          // segmento (ESTATICO) ou grupo animado (VIVO)
    float t = 0;           // tempo das animacoes em loop
    bool em_jogo = false;  // no mapa: recuo dos canos so quando atira
    Pose pose;

    // resultado do passe CONTAR
    int grupos_anim() const { return cont_anim_; }
    const std::vector<int>& prim_por_segmento() const { return por_seg_; }
    void reiniciar(float alfa = 1);  // alfa: multiplica todas as cores do passe

    // ---- primitivas (coordenadas da caixa do sprite)
    void c(float x, float y, float r, Color fill, float b = 0);
    void e(float x, float y, float rx, float ry, Color fill, float b = 0, float rot = 0);
    void rr(float x, float y, float w, float h, float raio, Color fill, float b = 0, float rot = 0);
    void poly(std::initializer_list<Vector2> pts, Color fill, float b = 0);
    void poly(const std::vector<Vector2>& pts, Color fill, float b = 0);
    void star(float x, float y, float ro, float ri, int n, Color fill, float b = 0, float rot = -90);
    void ln(std::initializer_list<Vector2> pts, float w, Color cor, float b = 0);
    void ln(const std::vector<Vector2>& pts, float w, Color cor, float b = 0);
    void sec(float x, float y, float r, float a0, float a1, Color fill, float b = 0);
    void arc(float x, float y, float r, float a0, float a1, float w, Color cor, float b = 0);
    void ring(float x, float y, float ri, float ro, Color cor);
    void ringE(float x, float y, float rx, float ry, float w, Color cor);
    void txt(float x, float y, const char* s, float tam, Color cor, float b = 0);
    void g(const G& gr, const std::function<void()>& fn);

private:
    bool desenha();  // a primitiva atual entra neste passe?
    Color cor(Color c) const;
    void aplicar_anim(const Anim& a);

    int cont_anim_ = 0;   // grupos animados de primeiro nivel ja vistos
    int prof_anim_ = 0;   // profundidade dentro de um grupo animado
    int grupo_atual_ = -1;
    float alfa_ = 1;
    std::vector<int> por_seg_;
};

}  // namespace bl::spr
