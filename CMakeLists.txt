cmake_minimum_required(VERSION 3.16)
project(BloonsBattles LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(BLOONS_CLIENTE "Compilar o jogo com interface (precisa da raylib)" ON)
option(BLOONS_TESTES "Compilar os testes automaticos" ON)

# Runtime do C/C++ embutido no .exe no Visual Studio (/MT): o jogo abre em qualquer PC,
# sem precisar instalar o "Visual C++ Redistributable".
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)  # a raylib segue a mesma regra

find_package(Threads REQUIRED)

# ------------------------------------------------------------------ nucleo
# Simulacao, protocolo, sockets e servidor: nao dependem de interface grafica.
add_library(bloons_nucleo STATIC
  src/jogo/dados.cpp
  src/jogo/defs.cpp
  src/jogo/mapas.cpp
  src/jogo/rodadas.cpp
  src/jogo/stats.cpp
  src/jogo/sim.cpp
  src/comum/protocolo.cpp
  src/rede/socket.cpp
  src/servidor/sala.cpp
  src/servidor/servidor.cpp
  src/ipc/memoria.cpp
  src/ipc/placar.cpp
)
target_include_directories(bloons_nucleo PUBLIC src third_party)
target_link_libraries(bloons_nucleo PUBLIC Threads::Threads)
if(WIN32)
  target_link_libraries(bloons_nucleo PUBLIC ws2_32)
else()
  # shm_open/sem_open: ficam na librt em glibc antiga (nas novas ja estao na libc)
  find_library(LIB_RT rt)
  if(LIB_RT)
    target_link_libraries(bloons_nucleo PUBLIC ${LIB_RT})
  endif()
endif()
if(MSVC)
  target_compile_options(bloons_nucleo PUBLIC /utf-8 /W3)
else()
  target_compile_options(bloons_nucleo PRIVATE -Wall -Wextra -Wno-missing-field-initializers)
endif()

# Executavel estatico no Windows com MinGW: um .exe so, sem DLLs do MinGW
if(MINGW)
  set(LINK_ESTATICO -static -static-libgcc -static-libstdc++)
endif()

add_executable(bloons_servidor src/servidor/main_servidor.cpp)
target_link_libraries(bloons_servidor PRIVATE bloons_nucleo ${LINK_ESTATICO})

# Monitor: processo separado que le o placar da sala em memoria compartilhada
add_executable(bloons_monitor src/ipc/main_monitor.cpp)
target_link_libraries(bloons_monitor PRIVATE bloons_nucleo ${LINK_ESTATICO})

# ------------------------------------------------------------------ testes
if(BLOONS_TESTES)
  enable_testing()
  add_executable(bloons_testes tests/testes.cpp)
  target_link_libraries(bloons_testes PRIVATE bloons_nucleo ${LINK_ESTATICO})
  add_dependencies(bloons_testes bloons_monitor)  # um teste abre o monitor como outro processo
  add_test(NAME bloons_testes COMMAND bloons_testes)
endif()

# ------------------------------------------------------------------ jogo (raylib)
if(BLOONS_CLIENTE)
  set(RAYLIB_VERSAO 5.5)
  find_package(raylib QUIET)  # raylib ja instalada? senao baixa o codigo e compila junto
  if(NOT TARGET raylib)
    include(FetchContent)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
    set(SUPPORT_MODULE_RMODELS OFF CACHE BOOL "" FORCE)
    # o F12 da raylib multiplica pela escala de DPI e sai com faixa preta: o jogo faz a propria captura
    set(SUPPORT_SCREEN_CAPTURE OFF CACHE BOOL "" FORCE)
    set(SUPPORT_GIF_RECORDING OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(raylib
      GIT_REPOSITORY https://github.com/raysan5/raylib.git
      GIT_TAG ${RAYLIB_VERSAO}
      GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(raylib)
  endif()

  # Fontes embutidas no executavel (nao precisa levar a pasta assets junto)
  set(FONTES_GERADAS ${CMAKE_CURRENT_BINARY_DIR}/gerado/fontes.cpp)
  add_custom_command(
    OUTPUT ${FONTES_GERADAS}
    COMMAND ${CMAKE_COMMAND} -DSAIDA=${FONTES_GERADAS} -DPASTA=${CMAKE_CURRENT_SOURCE_DIR}/assets/fontes
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embutir.cmake
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embutir.cmake
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/fontes/LilitaOne-Regular.ttf
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/fontes/LuckiestGuy-Regular.ttf
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/fontes/Nunito.ttf
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/fontes/DejaVuSansMono.ttf
    COMMENT "Embutindo fontes")

  add_executable(BloonsBattles WIN32
    src/cliente/main.cpp
    src/cliente/app.cpp
    src/cliente/ui.cpp
    src/cliente/arte.cpp
    src/cliente/caneta.cpp
    src/cliente/sprites.cpp
    src/cliente/anim.cpp
    src/cliente/vitrine.cpp
    src/cliente/render.cpp
    src/cliente/som.cpp
    src/cliente/conexao.cpp
    src/cliente/controle.cpp
    src/cliente/cena_jogo.cpp
    src/cliente/cenas_menu.cpp
    ${FONTES_GERADAS}
  )
  target_link_libraries(BloonsBattles PRIVATE bloons_nucleo raylib ${LINK_ESTATICO})
  if(MSVC)
    # janela sem console, mas com main() comum
    set_target_properties(BloonsBattles PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
  else()
    target_compile_options(BloonsBattles PRIVATE -Wall)
  endif()
endif()
