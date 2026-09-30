// Memoria compartilhada entre PROCESSOS, com nome, e um mutex tambem com nome.
//
// Mesma interface no Windows e no Linux:
//   Windows: CreateFileMapping/MapViewOfFile + CreateMutex ("Local\<nome>")
//   Linux:   shm_open/mmap + sem_open (semaforo binario usado como mutex)
//
// Qualquer processo da mesma maquina que conheca o nome enxerga os mesmos bytes. O mutex
// com nome garante que so um processo por vez le ou escreve a regiao (exclusao mutua).
//
// O cabecalho nao inclui <windows.h> de proposito (conflita com a raylib).
#pragma once

#include <cstddef>
#include <memory>
#include <string>

namespace bl::ipc {

class MemoriaCompartilhada {
public:
    // Cria (ou recria) a regiao e zera o conteudo. Lanca std::runtime_error se o sistema recusar.
    static std::unique_ptr<MemoriaCompartilhada> criar(const std::string& nome, std::size_t tamanho);
    // Abre uma regiao ja criada por outro processo. nullptr se nao existir.
    static std::unique_ptr<MemoriaCompartilhada> abrir(const std::string& nome, std::size_t tamanho);
    ~MemoriaCompartilhada();
    MemoriaCompartilhada(const MemoriaCompartilhada&) = delete;
    MemoriaCompartilhada& operator=(const MemoriaCompartilhada&) = delete;

    void* dados() { return dados_; }
    std::size_t tamanho() const { return tamanho_; }
    const std::string& nome() const { return nome_; }

    // Mutex entre processos. Prefira Trava (RAII) a chamar os dois direto.
    void travar();
    void destravar();

    class Trava {
    public:
        explicit Trava(MemoriaCompartilhada& m) : m_(m) { m_.travar(); }
        ~Trava() { m_.destravar(); }
        Trava(const Trava&) = delete;
        Trava& operator=(const Trava&) = delete;

    private:
        MemoriaCompartilhada& m_;
    };

private:
    MemoriaCompartilhada() = default;

    std::string nome_;
    std::size_t tamanho_ = 0;
    void* dados_ = nullptr;
    bool dono_ = false;       // quem criou apaga o nome ao sair (Linux)
    void* mapeamento_ = nullptr;  // HANDLE do arquivo mapeado (Windows)
    void* mutex_ = nullptr;       // HANDLE do mutex (Windows) ou sem_t* (Linux)
};

}  // namespace bl::ipc
