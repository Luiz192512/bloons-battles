// Efeitos sonoros sintetizados na hora (sem arquivos de audio).
#pragma once

#include <string>

namespace bl::som {

void iniciar();
void liberar();
// intervalo_ms: ignora se o mesmo som tocou ha menos tempo que isso
void tocar(const std::string& nome, int intervalo_ms = 0);
// Volume geral, de 0 a 100. Guardado em bloons_config.txt (na pasta de onde o jogo foi aberto).
int volume();
void definir_volume(int v);

}  // namespace bl::som
