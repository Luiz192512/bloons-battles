# Compilacao cruzada Linux -> Windows (64 bits) com MinGW-w64.
# Uso: cmake -S . -B build-win -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(PREFIXO x86_64-w64-mingw32)
# variante -posix: tem std::thread e std::mutex
find_program(CMAKE_C_COMPILER NAMES ${PREFIXO}-gcc-posix ${PREFIXO}-gcc)
find_program(CMAKE_CXX_COMPILER NAMES ${PREFIXO}-g++-posix ${PREFIXO}-g++)
find_program(CMAKE_RC_COMPILER NAMES ${PREFIXO}-windres)
set(CMAKE_FIND_ROOT_PATH /usr/${PREFIXO})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
