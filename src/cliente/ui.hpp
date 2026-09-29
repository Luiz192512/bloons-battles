// Componentes de interface no estilo Bloons: texto com contorno, botoes e paineis.
#pragma once

#include <string>
#include <vector>

#include "raylib.h"

namespace bl::ui {

constexpr int LARGURA = 1280, ALTURA = 720;

constexpr Color AMARELO{255, 214, 50, 255};
constexpr Color VERDE{96, 196, 60, 255};
constexpr Color VERDE_ESCURO{40, 110, 30, 255};
constexpr Color VERMELHO{225, 60, 50, 255};
constexpr Color AZUL{60, 150, 230, 255};
constexpr Color MARROM{122, 78, 40, 255};
constexpr Color MARROM_ESCURO{72, 44, 20, 255};
constexpr Color BEGE{238, 214, 160, 255};
constexpr Color BRANCO{255, 255, 255, 255};
constexpr Color PRETO{15, 15, 20, 255};
constexpr Color CINZA{130, 130, 140, 255};
constexpr Color DINHEIRO{255, 222, 70, 255};

inline Color rgb(int r, int g, int b, int a = 255) {
    return Color{static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b),
                 static_cast<unsigned char>(a)};
}
inline Color com_alfa(Color c, int a) { return rgb(c.r, c.g, c.b, a); }
Color clarear(Color c, float f = 0.25f);
Color escurecer(Color c, float f = 0.7f);  // multiplica os canais por f

// ---------------------------------------------------------------- entrada
// Os eventos do quadro, ja em coordenadas da tela virtual 1280x720.
struct Evento {
    enum Tipo { CLIQUE, CLIQUE_DIR, TECLA, CARACTERE } tipo;
    Vector2 pos{};
    int tecla = 0;  // KEY_* da raylib
    int ch = 0;     // caractere unicode digitado
};
Vector2 mouse();  // posicao do mouse na tela virtual
bool shift();

// ---------------------------------------------------------------- tela
// A tela virtual 1280x720 e desenhada com uma camera 2D que escala para o tamanho da janela.
void comecar_quadro();
void terminar_quadro();
// Refaz a camera da tela depois de desenhar numa RenderTexture no meio do quadro.
void restaurar_tela();
double tempo();  // segundos desde o inicio

// ---------------------------------------------------------------- texto
enum class Peso { TITULO, LOGO, TEXTO, MONO };
enum class Ancora { TOPLEFT, TOPRIGHT, CENTER, MIDLEFT, MIDRIGHT, MIDTOP, MIDBOTTOM };

void iniciar_fontes();
void liberar_fontes();
Vector2 medir(const std::string& txt, int tam, Peso peso = Peso::TITULO);
Rectangle texto(const std::string& txt, float x, float y, int tam = 20, Color cor = BRANCO, int contorno = 2,
                Ancora ancora = Ancora::TOPLEFT, Peso peso = Peso::TITULO, Color cor_contorno = PRETO);
std::vector<std::string> quebrar(const std::string& txt, int tam, float largura, Peso peso = Peso::TITULO);

// ---------------------------------------------------------------- formas
bool dentro(Rectangle r, Vector2 p);
Rectangle mover(Rectangle r, float dx, float dy);
Rectangle inflar(Rectangle r, float dx, float dy);  // como o inflate do pygame (dx e dy no total)
void ret(Rectangle r, Color cor, float raio = 0);
void ret_linha(Rectangle r, Color cor, float espessura, float raio = 0);

Rectangle painel(Rectangle r, Color cor = rgb(86, 150, 50), Color borda = MARROM, float raio = 14,
                 float espessura = 5, bool sombra = true);
Rectangle painel_madeira(Rectangle r, float raio = 10);
void barra(Rectangle r, float frac, Color cor = VERDE, Color fundo = rgb(40, 40, 40));
void fundo_gradiente(Color cima = rgb(90, 180, 240), Color baixo = rgb(170, 225, 255));
std::string formatar(double n);  // 12345 -> "12.345"

// ---------------------------------------------------------------- widgets
struct Botao {
    Rectangle rect{};
    std::string rotulo;
    Color cor = VERDE;
    int tam = 24;
    bool habilitado = true;

    void desenhar() const;
    bool clicou(const Evento& e) const;
};

struct CampoTexto {
    Rectangle rect{};
    std::string valor;
    std::string rotulo;
    bool ativo = false;
    size_t max_len = 40;

    void evento(const Evento& e);
    void desenhar() const;
};

}  // namespace bl::ui
