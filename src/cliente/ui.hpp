// Componentes de interface no estilo Bloons: texto com contorno, botoes e paineis.
#pragma once

#include <string>
#include <vector>

#include "raylib.h"

namespace bl::ui {

constexpr int LARGURA = 1280, ALTURA = 720;

// ---- tinta e madeira (design system da auditoria)
constexpr Color TINTA{22, 20, 26, 255};  // contorno unico de texto, sprites e componentes
constexpr Color PRETO = TINTA;           // alias para o codigo antigo
constexpr Color MADEIRA_ESCURA{74, 44, 20, 255};
constexpr Color MADEIRA{138, 90, 46, 255};
constexpr Color VEIO{122, 78, 38, 255};
constexpr Color MARROM = MADEIRA, MARROM_ESCURO = MADEIRA_ESCURA;
constexpr Color BEGE{243, 226, 184, 255};
constexpr Color BEGE_APAGADO{201, 182, 140, 255};
constexpr Color PERGAMINHO{251, 241, 214, 255};
// ---- acoes
constexpr Color VERDE{92, 196, 60, 255};
constexpr Color VERDE_ESCURO{43, 122, 30, 255};
constexpr Color PAINEL_VERDE{88, 168, 58, 255};
constexpr Color PAINEL_VERDE_ESC{47, 107, 31, 255};
constexpr Color VERMELHO{226, 59, 46, 255};
constexpr Color VERMELHO_ESCURO{142, 30, 22, 255};
constexpr Color AZUL{58, 150, 230, 255};
constexpr Color AZUL_ESCURO{29, 90, 156, 255};
constexpr Color AMARELO{255, 214, 50, 255};
constexpr Color OURO_ESCURO{201, 138, 14, 255};
// ---- recursos
constexpr Color DINHEIRO{255, 224, 70, 255};
constexpr Color ECO{124, 240, 90, 255};
constexpr Color ECO_ESCURO{30, 122, 52, 255};
constexpr Color VIDA{255, 77, 94, 255};
constexpr Color PRECO_RUIM{255, 194, 186, 255};
constexpr Color BRANCO{255, 255, 255, 255};
constexpr Color CINZA{140, 140, 150, 255};
constexpr Color CINZA_ESCURO{85, 85, 94, 255};
constexpr Color TEXTO_ESCURO{58, 42, 26, 255};  // texto corrido sobre bege
// ---- medidas
constexpr float R_TECLA = 5, R_CARD = 10, R_BOTAO = 14, R_PAINEL = 18;
constexpr float BORDA_FINA = 2, BORDA = 3, BORDA_GROSSA = 4;
constexpr float LABIO = 5, SOMBRA_Y = 4;
constexpr int E1 = 4, E2 = 8, E3 = 16, E4 = 24, E5 = 32, E6 = 48;
// contorno proporcional ao tamanho da fonte (~10% da altura, minimo 2)
int contorno_auto(int tam);

enum class Estado { NORMAL, HOVER, PRESSIONADO, DESABILITADO, SEM_DINHEIRO, SELECIONADO };

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
// Salva o quadro atual em PNG no fim do quadro (tamanho real da janela, sem faixa preta em telas com
// escala de DPI, que era o problema do F12 padrao da raylib).
void capturar(const std::string& arquivo);
// Refaz a camera da tela depois de desenhar numa RenderTexture no meio do quadro.
void restaurar_tela();
// Recorte (scissor) em coordenadas da tela virtual; funciona com a janela redimensionada.
void recortar(Rectangle r);
void fim_recorte();
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

Rectangle painel(Rectangle r, Color cor = PAINEL_VERDE, Color borda = TINTA, float raio = R_PAINEL,
                 float espessura = BORDA, bool sombra = true);
Rectangle painel_madeira(Rectangle r, float raio = 10);
// Painel de madeira com moldura (menus): borda TINTA 4, bisel escuro e veios.
Rectangle painel_menu(Rectangle r, float raio = 20);
// Placa do HUD: sombra, borda TINTA, face escura e brilho no topo. Devolve a area interna.
Rectangle placa(Rectangle r, Color cor = MADEIRA_ESCURA, float raio = 12);
// Keycap de atalho com (x, y) = canto superior esquerdo. Devolve o retangulo desenhado.
Rectangle tecla(const std::string& k, float x, float y, int tam = 9);
Rectangle tecla_centro(const std::string& k, float cx, float cy, int tam = 9);
// Pilula com o preco ($) centrada em (cx, cy): escura quando da para comprar, vermelha quando nao.
Rectangle pilula_preco(double valor, float cx, float cy, bool pode, int tam = 13);
Rectangle pilula(const std::string& txt, float cx, float cy, Color fundo, Color cor, int tam = 13);
// Pips de tier com contorno; o 5o (tier 5) e dourado.
void pips(float x, float y, int tier, int max = 5);
void barra(Rectangle r, float frac, Color cor = VERDE, Color fundo = rgb(28, 27, 32));
void fundo_gradiente(Color cima = rgb(90, 180, 240), Color baixo = rgb(170, 225, 255));
std::string formatar(double n);  // 12345 -> "12.345"

// ---------------------------------------------------------------- widgets
// Botao "de brinquedo": normal, hover (sobe 2 px), pressionado (afunda 3 px e perde a sombra)
// e desabilitado (cinza sem brilho). Verde = confirmar, azul = neutro/voltar, vermelho = sair/vender,
// amarelo = destaque.
struct Botao {
    Rectangle rect{};
    std::string rotulo;
    Color cor = VERDE;
    int tam = 24;
    bool habilitado = true;
    std::string atalho;        // keycap ao lado do rotulo (ex.: "Enter", "Esc")
    bool selecionado = false;  // anel amarelo (item escolhido)

    void desenhar() const;
    bool clicou(const Evento& e) const;
};
Color labio(Color cor);  // cor do "labio" (parte de baixo) de um botao

// Campo de texto: foco = pergaminho com anel amarelo; erro = anel vermelho.
struct CampoTexto {
    Rectangle rect{};
    std::string valor;
    std::string rotulo;
    bool ativo = false;
    bool erro = false;
    size_t max_len = 40;

    void evento(const Evento& e);
    void desenhar() const;
};
// Aviso de erro em placa vermelha com "!" (devolve a altura usada).
float placa_erro(const std::string& msg, float x, float y, float largura);

}  // namespace bl::ui
