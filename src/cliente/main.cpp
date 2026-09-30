// Bloons TD Battles: ponto de entrada do jogo.
#include <string>
#include <vector>

#include "cliente/app.hpp"

int main(int argc, char** argv) {
    // opcoes de desenvolvimento do visual (ver cliente/vitrine.hpp): --vitrine, --demo, --captura
    std::vector<std::string> args(argv + 1, argv + argc);
    bl::App app(args);
    app.rodar();
    return 0;
}
