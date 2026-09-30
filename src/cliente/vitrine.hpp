// Ferramentas de desenvolvimento do visual (nao fazem parte do jogo normal):
//
//   BloonsBattles --vitrine [pagina]   galeria de sprites como em "Sprites.dc.html" e a pagina
//                                      de Animacoes, que toca cada clipe em loop e desenha as
//                                      curvas de cada canal (a ferramenta usada para ajustar os
//                                      tempos e as curvas dos disparos e habilidades).
//   BloonsBattles --demo solo|batalha  partida local ja montada (torres, upgrades, bloons), para
//                                      conferir a tela de jogo sem precisar jogar ate la.
//   --captura arquivo.png [s] [n] [intervalo]
//                                      salva capturas de tela depois de s segundos e fecha.
#pragma once

#include <memory>
#include <string>

#include "cliente/app.hpp"

namespace bl {

std::unique_ptr<Cena> criar_vitrine(App& app, int pagina);
std::unique_ptr<Cena> criar_demo(App& app, bool batalha);

}  // namespace bl
