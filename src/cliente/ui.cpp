#include "cliente/ui.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <sstream>

#include "raymath.h"
#include "rlgl.h"

namespace bl::recursos {
extern const unsigned char fonte_titulo[], fonte_logo[], fonte_texto[], fonte_mono[];
extern const std::size_t fonte_titulo_tam, fonte_logo_tam, fonte_texto_tam, fonte_mono_tam;
}  // namespace bl::recursos

namespace bl::ui {

Color clarear(Color c, float f) {
    return rgb(std::min(255, int(c.r + (255 - c.r) * f)), std::min(255, int(c.g + (255 - c.g) * f)),
               std::min(255, int(c.b + (255 - c.b) * f)), c.a);
}

Color escurecer(Color c, float f) { return rgb(int(c.r * f), int(c.g * f), int(c.b * f), c.a); }

// ---------------------------------------------------------------- tela
namespace {
Camera2D camera{};
Rectangle area{};  // area da janela ocupada pela tela virtual
}  // namespace

void comecar_quadro() {
    const float w = static_cast<float>(GetScreenWidth()), h = static_cast<float>(GetScreenHeight());
    const float esc = std::min(w / LARGURA, h / ALTURA);
    area = {(w - LARGURA * esc) / 2, (h - ALTURA * esc) / 2, LARGURA * esc, ALTURA * esc};
    camera = Camera2D{{area.x, area.y}, {0, 0}, 0, esc};
    SetMouseOffset(static_cast<int>(-area.x), static_cast<int>(-area.y));
    SetMouseScale(1 / esc, 1 / esc);
    BeginDrawing();
    ClearBackground(BLACK);
    BeginScissorMode(static_cast<int>(area.x), static_cast<int>(area.y), static_cast<int>(area.width),
                     static_cast<int>(area.height));
    BeginMode2D(camera);
}

void terminar_quadro() {
    EndMode2D();
    EndScissorMode();
    EndDrawing();
}

void restaurar_tela() {
    rlLoadIdentity();
    rlMultMatrixf(MatrixToFloat(GetCameraMatrix2D(camera)));
    rlEnableScissorTest();
    rlScissor(static_cast<int>(area.x), static_cast<int>(GetScreenHeight() - area.y - area.height),
              static_cast<int>(area.width), static_cast<int>(area.height));
}

Vector2 mouse() { return GetMousePosition(); }
bool shift() { return IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT); }
double tempo() { return GetTime(); }

// ---------------------------------------------------------------- texto
namespace {

// Tamanho da fonte da raylib equivalente ao tamanho usado no desenho original (pygame)
int px(int tam, Peso peso) {
    switch (peso) {
        case Peso::TITULO: return static_cast<int>(std::lround(tam * 1.32));
        case Peso::LOGO: return static_cast<int>(std::lround(tam * 1.25));
        case Peso::TEXTO: return static_cast<int>(std::lround(tam * 1.25));
        case Peso::MONO: return static_cast<int>(std::lround(tam * 1.1));
    }
    return tam;
}

std::map<std::pair<int, int>, Font> fontes;
std::vector<int> codigos;

const Font& fonte(int tam, Peso peso) {
    const int p = px(tam, peso);
    auto chave = std::make_pair(static_cast<int>(peso), p);
    auto it = fontes.find(chave);
    if (it != fontes.end()) return it->second;
    if (codigos.empty()) {
        for (int c = 32; c < 256; ++c) codigos.push_back(c);
        for (int c : {0x2026, 0x2192, 0x2190, 0x2022, 0x2013, 0x2014}) codigos.push_back(c);
    }
    const unsigned char* dados = recursos::fonte_titulo;
    std::size_t n = recursos::fonte_titulo_tam;
    if (peso == Peso::LOGO) dados = recursos::fonte_logo, n = recursos::fonte_logo_tam;
    if (peso == Peso::TEXTO) dados = recursos::fonte_texto, n = recursos::fonte_texto_tam;
    if (peso == Peso::MONO) dados = recursos::fonte_mono, n = recursos::fonte_mono_tam;
    // carregar a fonte envia uma textura: nao pode acontecer com a camera da tela ativa
    Font f = LoadFontFromMemory(".ttf", dados, static_cast<int>(n), p, codigos.data(),
                                static_cast<int>(codigos.size()));
    SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);
    return fontes[chave] = f;
}

}  // namespace

void iniciar_fontes() {
    // tamanhos mais usados, para nao travar o primeiro quadro
    for (int t : {10, 11, 12, 13, 14, 15, 16, 18, 20, 22, 24, 26, 28, 32}) fonte(t, Peso::TITULO);
    for (int t : {14}) fonte(t, Peso::TEXTO);
}

void liberar_fontes() {
    for (auto& [k, f] : fontes) UnloadFont(f);
    fontes.clear();
}

Vector2 medir(const std::string& txt, int tam, Peso peso) {
    const Font& f = fonte(tam, peso);
    return MeasureTextEx(f, txt.c_str(), static_cast<float>(f.baseSize), 0);
}

Rectangle texto(const std::string& txt, float x, float y, int tam, Color cor, int contorno, Ancora ancora,
                Peso peso, Color cor_contorno) {
    if (txt.empty()) return {x, y, 0, 0};
    const Font& f = fonte(tam, peso);
    const float s = static_cast<float>(f.baseSize);
    Vector2 m = MeasureTextEx(f, txt.c_str(), s, 0);
    const float w = m.x + contorno * 2, h = m.y + contorno * 2;
    float rx = x, ry = y;
    switch (ancora) {
        case Ancora::TOPLEFT: break;
        case Ancora::TOPRIGHT: rx = x - w; break;
        case Ancora::CENTER: rx = x - w / 2, ry = y - h / 2; break;
        case Ancora::MIDLEFT: ry = y - h / 2; break;
        case Ancora::MIDRIGHT: rx = x - w, ry = y - h / 2; break;
        case Ancora::MIDTOP: rx = x - w / 2; break;
        case Ancora::MIDBOTTOM: rx = x - w / 2, ry = y - h; break;
    }
    rx = std::round(rx);
    ry = std::round(ry);
    if (contorno > 0) {
        for (int dx = -contorno; dx <= contorno; ++dx)
            for (int dy = -contorno; dy <= contorno; ++dy)
                if (dx * dx + dy * dy <= contorno * contorno + 1)
                    DrawTextEx(f, txt.c_str(), {rx + contorno + dx, ry + contorno + dy}, s, 0, cor_contorno);
    }
    DrawTextEx(f, txt.c_str(), {rx + contorno, ry + contorno}, s, 0, cor);
    return {rx, ry, w, h};
}

std::vector<std::string> quebrar(const std::string& txt, int tam, float largura, Peso peso) {
    std::vector<std::string> linhas;
    std::istringstream in(txt);
    std::string palavra, atual;
    while (in >> palavra) {
        std::string teste = atual.empty() ? palavra : atual + " " + palavra;
        if (medir(teste, tam, peso).x <= largura) {
            atual = teste;
        } else {
            if (!atual.empty()) linhas.push_back(atual);
            atual = palavra;
        }
    }
    if (!atual.empty()) linhas.push_back(atual);
    return linhas;
}

// ---------------------------------------------------------------- formas
bool dentro(Rectangle r, Vector2 p) { return CheckCollisionPointRec(p, r); }
Rectangle mover(Rectangle r, float dx, float dy) { return {r.x + dx, r.y + dy, r.width, r.height}; }
Rectangle inflar(Rectangle r, float dx, float dy) {
    return {r.x - dx / 2, r.y - dy / 2, r.width + dx, r.height + dy};
}

namespace {
float arredondamento(Rectangle r, float raio) {
    float menor = std::min(r.width, r.height);
    return menor <= 0 ? 0 : std::min(1.0f, 2 * raio / menor);
}
}  // namespace

void ret(Rectangle r, Color cor, float raio) {
    if (raio <= 0) DrawRectangleRec(r, cor);
    else DrawRectangleRounded(r, arredondamento(r, raio), 8, cor);
}

void ret_linha(Rectangle r, Color cor, float espessura, float raio) {
    if (raio <= 0) DrawRectangleLinesEx(r, espessura, cor);
    else DrawRectangleRoundedLinesEx(inflar(r, -espessura, -espessura), arredondamento(r, raio), 8, espessura, cor);
}

Rectangle painel(Rectangle r, Color cor, Color borda, float raio, float espessura, bool sombra) {
    if (sombra) ret(mover(r, 4, 5), rgb(0, 0, 0, 90), raio);
    ret(r, borda, raio);
    Rectangle interno = inflar(r, -espessura * 2, -espessura * 2);
    ret(interno, cor, std::max(2.0f, raio - espessura));
    // brilho no topo
    Rectangle brilho{interno.x + 4, interno.y + 3, interno.width - 8, std::max(4.0f, interno.height / 6)};
    ret(brilho, rgb(255, 255, 255, 40), std::max(2.0f, raio - espessura));
    return interno;
}

Rectangle painel_madeira(Rectangle r, float raio) {
    ret(r, MARROM_ESCURO, raio);
    Rectangle interno = inflar(r, -8, -8);
    ret(interno, rgb(150, 100, 55), raio);
    for (float k = interno.y + 6; k < interno.y + interno.height; k += 22)
        DrawLineEx({interno.x + 4, k}, {interno.x + interno.width - 4, k}, 2, rgb(135, 88, 45));
    return interno;
}

void barra(Rectangle r, float frac, Color cor, Color fundo) {
    ret(inflar(r, 4, 4), PRETO, 6);
    ret(r, fundo, 5);
    if (frac > 0) {
        Rectangle c = r;
        c.width = std::max(4.0f, r.width * std::min(1.0f, frac));
        ret(c, cor, 5);
    }
}

void fundo_gradiente(Color cima, Color baixo) { DrawRectangleGradientV(0, 0, LARGURA, ALTURA, cima, baixo); }

std::string formatar(double n) {
    long long v = static_cast<long long>(n);
    bool neg = v < 0;
    std::string s = std::to_string(neg ? -v : v);
    for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3) s.insert(static_cast<size_t>(i), ".");
    return neg ? "-" + s : s;
}

// ---------------------------------------------------------------- widgets
void Botao::desenhar() const {
    const Color base = habilitado ? cor : CINZA;
    const bool sobre = habilitado && dentro(rect, mouse());
    Rectangle r = sobre ? mover(rect, 0, -2) : rect;
    ret(mover(r, 0, 4), rgb(0, 0, 0), 12);
    ret(r, rgb(std::max(0, base.r - 70), std::max(0, base.g - 70), std::max(0, base.b - 70)), 12);
    Rectangle interno = inflar(r, -6, -6);
    int k = sobre ? 25 : 0;
    ret(interno, rgb(std::min(255, base.r + k), std::min(255, base.g + k), std::min(255, base.b + k)), 10);
    ret({interno.x + 4, interno.y + 3, interno.width - 8, interno.height / 2 - 2}, rgb(255, 255, 255, 55), 8);
    texto(rotulo, r.x + r.width / 2, r.y + r.height / 2, tam, BRANCO, 2, Ancora::CENTER);
}

bool Botao::clicou(const Evento& e) const { return habilitado && e.tipo == Evento::CLIQUE && dentro(rect, e.pos); }

void CampoTexto::evento(const Evento& e) {
    if (e.tipo == Evento::CLIQUE) {
        ativo = dentro(rect, e.pos);
    } else if (e.tipo == Evento::TECLA && ativo) {
        if (e.tecla == KEY_BACKSPACE && !valor.empty()) valor.pop_back();
        else if (e.tecla == KEY_ENTER || e.tecla == KEY_TAB) ativo = false;
    } else if (e.tipo == Evento::CARACTERE && ativo && valor.size() < max_len && e.ch >= 32 && e.ch < 127) {
        valor += static_cast<char>(e.ch);
    }
}

void CampoTexto::desenhar() const {
    if (!rotulo.empty()) texto(rotulo, rect.x, rect.y - 30, 18);
    ret(rect, MARROM_ESCURO, 8);
    ret(inflar(rect, -6, -6), ativo ? BEGE : rgb(220, 200, 150), 6);
    std::string cursor = ativo && static_cast<int>(tempo() * 2) % 2 ? "|" : "";
    texto(valor + cursor, rect.x + 12, rect.y + rect.height / 2, 20, PRETO, 0, Ancora::MIDLEFT, Peso::TEXTO);
}

}  // namespace bl::ui
