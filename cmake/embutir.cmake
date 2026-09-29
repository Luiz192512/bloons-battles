# Gera um .cpp com as fontes (.ttf) como vetores de bytes.
# Uso: cmake -DSAIDA=<arquivo.cpp> -DPASTA=<pasta das fontes> -P embutir.cmake

set(ARQUIVOS LilitaOne-Regular.ttf LuckiestGuy-Regular.ttf Nunito.ttf DejaVuSansMono.ttf)
set(NOMES fonte_titulo fonte_logo fonte_texto fonte_mono)

set(CODIGO "// Gerado por cmake/embutir.cmake. Nao editar.\n#include <cstddef>\n\nnamespace bl::recursos {\n")
list(LENGTH ARQUIVOS N)
math(EXPR ULTIMO "${N} - 1")
foreach(I RANGE ${ULTIMO})
  list(GET ARQUIVOS ${I} ARQ)
  list(GET NOMES ${I} NOME)
  file(READ "${PASTA}/${ARQ}" HEX HEX)
  string(LENGTH "${HEX}" TAM_HEX)
  math(EXPR TAM "${TAM_HEX} / 2")
  string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," BYTES "${HEX}")
  string(APPEND CODIGO "extern const unsigned char ${NOME}[] = {${BYTES}};\n")
  string(APPEND CODIGO "extern const std::size_t ${NOME}_tam = ${TAM};\n")
endforeach()
string(APPEND CODIGO "}  // namespace bl::recursos\n")
file(WRITE "${SAIDA}" "${CODIGO}")
