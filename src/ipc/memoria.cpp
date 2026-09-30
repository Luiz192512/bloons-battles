#include "ipc/memoria.hpp"

#include <cstring>
#include <stdexcept>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace bl::ipc {

namespace {

#ifdef _WIN32
// "Local\" = visivel para os processos da mesma sessao de usuario (nao precisa de administrador)
std::string nome_mapa(const std::string& n) { return "Local\\" + n; }
std::string nome_mutex(const std::string& n) { return "Local\\" + n + "_mutex"; }
#else
std::string nome_mapa(const std::string& n) { return "/" + n; }
std::string nome_mutex(const std::string& n) { return "/" + n + "_mutex"; }
#endif

}  // namespace

std::unique_ptr<MemoriaCompartilhada> MemoriaCompartilhada::criar(const std::string& nome, std::size_t tamanho) {
    std::unique_ptr<MemoriaCompartilhada> m(new MemoriaCompartilhada());
    m->nome_ = nome;
    m->tamanho_ = tamanho;
    m->dono_ = true;
#ifdef _WIN32
    HANDLE mapa = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, static_cast<DWORD>(tamanho),
                                     nome_mapa(nome).c_str());
    if (!mapa) throw std::runtime_error("CreateFileMapping falhou: " + std::to_string(GetLastError()));
    m->mapeamento_ = mapa;
    m->dados_ = MapViewOfFile(mapa, FILE_MAP_ALL_ACCESS, 0, 0, tamanho);
    if (!m->dados_) throw std::runtime_error("MapViewOfFile falhou: " + std::to_string(GetLastError()));
    HANDLE mtx = CreateMutexA(nullptr, FALSE, nome_mutex(nome).c_str());
    if (!mtx) throw std::runtime_error("CreateMutex falhou: " + std::to_string(GetLastError()));
    m->mutex_ = mtx;
#else
    // restos de uma execucao que caiu sem limpar: comeca do zero
    shm_unlink(nome_mapa(nome).c_str());
    sem_unlink(nome_mutex(nome).c_str());
    int fd = shm_open(nome_mapa(nome).c_str(), O_CREAT | O_RDWR, 0600);
    if (fd < 0) throw std::runtime_error(std::string("shm_open falhou: ") + std::strerror(errno));
    if (ftruncate(fd, static_cast<off_t>(tamanho)) != 0) {
        close(fd);
        throw std::runtime_error(std::string("ftruncate falhou: ") + std::strerror(errno));
    }
    void* p = mmap(nullptr, tamanho, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);  // o mapeamento continua valido sem o descritor
    if (p == MAP_FAILED) throw std::runtime_error(std::string("mmap falhou: ") + std::strerror(errno));
    m->dados_ = p;
    sem_t* s = sem_open(nome_mutex(nome).c_str(), O_CREAT, 0600, 1);  // valor 1 = mutex livre
    if (s == SEM_FAILED) throw std::runtime_error(std::string("sem_open falhou: ") + std::strerror(errno));
    m->mutex_ = s;
#endif
    Trava t(*m);
    std::memset(m->dados_, 0, tamanho);
    return m;
}

std::unique_ptr<MemoriaCompartilhada> MemoriaCompartilhada::abrir(const std::string& nome, std::size_t tamanho) {
    std::unique_ptr<MemoriaCompartilhada> m(new MemoriaCompartilhada());
    m->nome_ = nome;
    m->tamanho_ = tamanho;
#ifdef _WIN32
    HANDLE mapa = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, nome_mapa(nome).c_str());
    if (!mapa) return nullptr;
    m->mapeamento_ = mapa;
    m->dados_ = MapViewOfFile(mapa, FILE_MAP_ALL_ACCESS, 0, 0, tamanho);
    if (!m->dados_) return nullptr;
    HANDLE mtx = OpenMutexA(SYNCHRONIZE | MUTEX_MODIFY_STATE, FALSE, nome_mutex(nome).c_str());
    if (!mtx) return nullptr;
    m->mutex_ = mtx;
#else
    int fd = shm_open(nome_mapa(nome).c_str(), O_RDWR, 0600);
    if (fd < 0) return nullptr;
    struct stat st {};
    if (fstat(fd, &st) != 0 || static_cast<std::size_t>(st.st_size) < tamanho) {
        close(fd);
        return nullptr;
    }
    void* p = mmap(nullptr, tamanho, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (p == MAP_FAILED) return nullptr;
    m->dados_ = p;
    sem_t* s = sem_open(nome_mutex(nome).c_str(), 0);
    if (s == SEM_FAILED) return nullptr;
    m->mutex_ = s;
#endif
    return m;
}

MemoriaCompartilhada::~MemoriaCompartilhada() {
#ifdef _WIN32
    if (dados_) UnmapViewOfFile(dados_);
    if (mapeamento_) CloseHandle(static_cast<HANDLE>(mapeamento_));
    if (mutex_) CloseHandle(static_cast<HANDLE>(mutex_));
    // no Windows o nome some sozinho quando o ultimo processo fecha
#else
    if (dados_) munmap(dados_, tamanho_);
    if (mutex_) sem_close(static_cast<sem_t*>(mutex_));
    if (dono_) {
        shm_unlink(nome_mapa(nome_).c_str());
        sem_unlink(nome_mutex(nome_).c_str());
    }
#endif
}

void MemoriaCompartilhada::travar() {
#ifdef _WIN32
    // WAIT_ABANDONED: outro processo morreu segurando o mutex; a posse passa para nos
    WaitForSingleObject(static_cast<HANDLE>(mutex_), INFINITE);
#else
    while (sem_wait(static_cast<sem_t*>(mutex_)) != 0 && errno == EINTR) {
    }
#endif
}

void MemoriaCompartilhada::destravar() {
#ifdef _WIN32
    ReleaseMutex(static_cast<HANDLE>(mutex_));
#else
    sem_post(static_cast<sem_t*>(mutex_));
#endif
}

}  // namespace bl::ipc
