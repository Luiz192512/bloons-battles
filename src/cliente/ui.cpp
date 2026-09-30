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

int contorno_auto(int tam) { return std::max(2, static_cast<int>(std::lround(tam * 1.32f / 10))); }

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

namespace {
std::string captura_pendente;
}  // namespace

void capturar(const std::string& arquivo) { captura_pendente = arquivo; }

void terminar_quadro() {
    EndMode2D();
    EndScissorMode();
    if (!captura_pendente.empty()) {
        rlDrawRenderBatchActive();
        const int w = GetRenderWidth(), h = GetRenderHeight();
        unsigned char* px = rlReadScreenPixels(w, h);
        Image img{px, w, h, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
        ExportImage(img, captura_pendente.c_str());
        RL_FREE(px);
        captura_pendente.clear();
    }
    EndDrawing();
}

void restaurar_tela() {
    rlLoadIdentity();
    rlMultMatrixf(MatrixToFloat(GetCameraMatrix2D(camera)));
    rlEnableScissorTest();
    rlScissor(static_cast<int>(area.x), static_cast<int>(GetScreenHeight() - area.y - area.height),
              static_cast<int>(area.width), static_cast<int>(area.height));
}

void recortar(Rectangle r) {
    rlDrawRenderBatchActive();
    const float esc = camera.zoom;
    const float x = area.x + r.x * esc, y = area.y + r.y * esc;
    rlEnableScissorTest();
    rlScissor(static_cast<int>(x), static_cast<int>(GetScreenHeight() - y - r.height * esc), static_cast<int>(r.width * esc),
              static_cast<int>(r.height * esc));
}

void fim_recorte() {
    rlDrawRenderBatchActive();
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
    if (contorno > 0) contorno = std::max(contorno, contorno_auto(tam));
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
        // contorno em anel: amostras no raio cheio e na metade (fica redondo sem desenhar (2c+1)^2 vezes)
        const int n = std::max(8, contorno * 4);
        for (float raio : {static_cast<float>(contorno), contorno * 0.5f}) {
            for (int i = 0; i < n; ++i) {
                const float a = 6.2831853f * i / n;
                DrawTextEx(f, txt.c_str(), {rx + contorno + std::cos(a) * raio, ry + contorno + std::sin(a) * raio}, s, 0,
                           cor_contorno);
            }
            if (contorno < 3) break;
        }
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
    if (sombra) ret(mover(r, 0, 6), rgb(22, 20, 26, 115), raio);
    ret(r, borda, raio);
    Rectangle interno = inflar(r, -espessura * 2, -espessura * 2);
    // bisel escuro por dentro da borda e brilho no topo
    ret(interno, escurecer(cor, 0.55f), std::max(2.0f, raio - espessura));
    Rectangle face = inflar(interno, -6, -6);
    ret(face, cor, std::max(2.0f, raio - espessura - 3));
    ret({face.x + 4, face.y + 2, face.width - 8, 6}, rgb(255, 255, 255, 36), 3);
    return face;
}

Rectangle painel_madeira(Rectangle r, float raio) {
    ret(r, MADEIRA, raio);
    // veio da madeira: uma linha a cada 24 px
    for (float k = r.x + 22; k < r.x + r.width; k += 24) DrawRectangleRec({k, r.y, 2, r.height}, VEIO);
    DrawRectangleRec({r.x, r.y, 4, r.height}, TINTA);
    DrawRectangleRec({r.x + 4, r.y, 3, r.height}, rgb(255, 255, 255, 30));
    return inflar(r, -8, -8);
}

Rectangle painel_menu(Rectangle r, float raio) {
    ret(mover(r, 0, 8), rgb(22, 20, 26, 115), raio);
    ret(r, TINTA, raio);
    Rectangle m = inflar(r, -8, -8);
    ret(m, MADEIRA_ESCURA, raio - 4);
    Rectangle f = inflar(m, -8, -8);
    ret(f, MADEIRA, raio - 8);
    for (float k = f.y + 20; k < f.y + f.height - 4; k += 24) DrawRectangleRec({f.x + 6, k, f.width - 12, 2}, rgb(122, 78, 38, 110));
    ret({f.x + 6, f.y + 2, f.width - 12, 6}, rgb(255, 255, 255, 30), 3);
    return f;
}

Rectangle placa(Rectangle r, Color cor, float raio) {
    ret(mover(r, 0, SOMBRA_Y), rgb(22, 20, 26, 115), raio);
    ret(r, TINTA, raio);
    Rectangle f = inflar(r, -6, -6);
    ret(f, cor, std::max(2.0f, raio - 3));
    ret({f.x + 3, f.y, f.width - 6, 2}, rgb(255, 255, 255, 46), 1);
    ret({f.x, f.y + f.height - 4, f.width, 4}, rgb(0, 0, 0, 60), std::max(1.0f, raio - 4));
    return f;
}

Rectangle tecla(const std::string& k, float x, float y, int tam) {
    const Vector2 m = medir(k, tam);
    const float w = std::max(18.0f, m.x + 8), h = 18;
    const Rectangle r{x, y, w, h};
    ret(r, TINTA, R_TECLA);
    ret({x + 2, y + h - 3, w - 4, 2}, rgb(58, 54, 64), 1);
    texto(k, x + w / 2, y + h / 2 - 1, tam, BRANCO, 0, Ancora::CENTER);
    return r;
}

Rectangle tecla_centro(const std::string& k, float cx, float cy, int tam) {
    const float w = std::max(18.0f, medir(k, tam).x + 8);
    return tecla(k, cx - w / 2, cy - 9, tam);
}

Rectangle pilula(const std::string& txt, float cx, float cy, Color fundo, Color cor, int tam) {
    const Vector2 m = medir(txt, tam);
    const float h = std::round(m.y * 0.95f) + 2, w = m.x + 14;
    const Rectangle r{cx - w / 2, cy - h / 2, w, h};
    ret(r, fundo, h / 2);
    texto(txt, cx, cy, tam, cor, 0, Ancora::CENTER);
    return r;
}

Rectangle pilula_preco(double valor, float cx, float cy, bool pode, int tam) {
    return pilula("$" + formatar(valor), cx, cy, pode ? TINTA : VERMELHO_ESCURO, pode ? DINHEIRO : PRECO_RUIM, tam);
}

void pips(float x, float y, int tier, int max) {
    for (int i = 0; i < max; ++i) {
        const Rectangle r{x + i * 23.0f, y, 19, 12};
        ret(r, TINTA, 4);
        const Color c = i < tier ? (i == 4 ? AMARELO : rgb(155, 230, 110)) : PAINEL_VERDE_ESC;
        const Rectangle f = inflar(r, -4, -4);
        ret(f, c, 2);
        if (i < tier) ret({f.x, f.y, f.width, 2}, rgb(255, 255, 255, 115), 1);
    }
}

void barra(Rectangle r, float frac, Color cor, Color fundo) {
    ret(inflar(r, 5, 5), TINTA, (r.height + 5) / 2);
    ret(r, fundo, r.height / 2);
    if (frac > 0) {
        Rectangle c = r;
        c.width = std::max(r.height, r.width * std::min(1.0f, frac));
        ret(c, cor, r.height / 2);
        ret({c.x + 2, c.y + 1, c.width - 4, std::max(1.0f, r.height / 3)}, rgb(255, 255, 255, 90), 2);
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
Color labio(Color c) {
    auto igual = [&](Color a) { return a.r == c.r && a.g == c.g && a.b == c.b; };
    if (igual(VERDE)) return VERDE_ESCURO;
    if (igual(AZUL)) return AZUL_ESCURO;
    if (igual(VERMELHO)) return VERMELHO_ESCURO;
    if (igual(AMARELO)) return OURO_ESCURO;
    return escurecer(c, 0.6f);
}

void Botao::desenhar() const {
    const bool sobre = habilitado && dentro(rect, mouse());
    const bool apertado = sobre && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    const float dy = apertado ? 3.0f : sobre ? -2.0f : 0.0f;
    const float sombra = apertado ? 1.0f : sobre ? 7.0f : 5.0f;
    Color face = habilitado ? cor : CINZA;
    if (sobre && !apertado) face = clarear(face, 0.15f);
    const Color lab = habilitado ? labio(cor) : CINZA_ESCURO;
    const Rectangle r = mover(rect, 0, dy);
    const float raio = std::min(R_BOTAO + 2, r.height / 2);
    ret(mover(r, 0, sombra), rgb(22, 20, 26, 140), raio);
    if (selecionado) ret(inflar(r, 10, 10), AMARELO, raio + 4);
    ret(r, TINTA, raio);
    Rectangle f = inflar(r, -8, -8);
    ret(f, lab, raio - 4);
    ret({f.x, f.y, f.width, f.height - (apertado ? 2 : LABIO)}, face, raio - 4);
    if (habilitado) ret({f.x + 4, f.y + 2, f.width - 8, 3}, rgb(255, 255, 255, 77), 2);
    const Color txt = habilitado ? BRANCO : rgb(216, 216, 222);
    const float cy = r.y + r.height / 2 - (apertado ? 1 : 2);
    if (atalho.empty()) {
        texto(rotulo, r.x + r.width / 2, cy, tam, txt, 2, Ancora::CENTER);
    } else {
        const float wt = medir(rotulo, tam).x, wk = std::max(18.0f, medir(atalho, 9).x + 8);
        const float x0 = r.x + r.width / 2 - (wt + 10 + wk) / 2;
        texto(rotulo, x0, cy, tam, txt, 2, Ancora::MIDLEFT);
        tecla(atalho, x0 + wt + 14, cy - 9);
    }
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
    if (!rotulo.empty()) texto(rotulo, rect.x + 2, rect.y - 14, 13, BRANCO, 0, Ancora::MIDLEFT, Peso::TEXTO);
    if (ativo || erro) ret(inflar(rect, 8, 8), erro ? VERMELHO : AMARELO, 16);
    ret(rect, TINTA, 12);
    ret(inflar(rect, -6, -6), ativo ? PERGAMINHO : rgb(230, 211, 160), 9);
    const Rectangle t = texto(valor, rect.x + 14, rect.y + rect.height / 2, 20, TINTA, 0, Ancora::MIDLEFT);
    if (ativo && static_cast<int>(tempo() * 2) % 2)
        DrawRectangleRec({t.x + t.width + 3, rect.y + 13, 3, rect.height - 26}, TINTA);
}

float placa_erro(const std::string& msg, float x, float y, float largura) {
    const auto linhas = quebrar(msg, 14, largura - 60, Peso::TEXTO);
    const float h = 20 + 18.0f * std::max<size_t>(1, linhas.size());
    const Rectangle r{x, y, largura, h};
    ret(r, TINTA, 12);
    ret(inflar(r, -6, -6), VERMELHO_ESCURO, 9);
    texto("!", r.x + 22, r.y + h / 2, 18, AMARELO, 3, Ancora::CENTER);
    for (size_t i = 0; i < linhas.size(); ++i)
        texto(linhas[i], r.x + 40, r.y + 10 + i * 18.0f, 14, BRANCO, 0, Ancora::TOPLEFT, Peso::TEXTO);
    return h;
}

}  // namespace bl::ui
