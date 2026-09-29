#include "rede/socket.hpp"

#include <algorithm>
#include <cstring>
#include <set>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
using sock_t = SOCKET;
using socklen = int;
#define FECHAR_SOCK closesocket
#define SHUT_AMBOS SD_BOTH
#else
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
using sock_t = int;
using socklen = socklen_t;
#define FECHAR_SOCK ::close
#define SHUT_AMBOS SHUT_RDWR
#endif

namespace bl::rede {

namespace {

#ifdef _WIN32
// Winsock precisa ser iniciado uma vez por processo.
struct IniciarWinsock {
    IniciarWinsock() {
        WSADATA d;
        WSAStartup(MAKEWORD(2, 2), &d);
    }
    ~IniciarWinsock() { WSACleanup(); }
};
void garantir_winsock() { static IniciarWinsock w; }
#else
void garantir_winsock() {}
#endif

sock_t S(std::intptr_t fd) { return static_cast<sock_t>(fd); }

sockaddr_in endereco(const std::string& host, int porta) {
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port = htons(static_cast<unsigned short>(porta));
    if (host.empty() || host == "0.0.0.0") {
        a.sin_addr.s_addr = htonl(INADDR_ANY);
    } else if (inet_pton(AF_INET, host.c_str(), &a.sin_addr) != 1) {
        addrinfo dica{}, *res = nullptr;
        dica.ai_family = AF_INET;
        dica.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(host.c_str(), nullptr, &dica, &res) != 0 || !res)
            throw ErroRede("endereço desconhecido: " + host);
        a.sin_addr = reinterpret_cast<sockaddr_in*>(res->ai_addr)->sin_addr;
        freeaddrinfo(res);
    }
    return a;
}

void modo_bloqueante(sock_t s, bool bloqueante) {
#ifdef _WIN32
    u_long nb = bloqueante ? 0 : 1;
    ioctlsocket(s, FIONBIO, &nb);
#else
    int f = fcntl(s, F_GETFL, 0);
    fcntl(s, F_SETFL, bloqueante ? (f & ~O_NONBLOCK) : (f | O_NONBLOCK));
#endif
}

}  // namespace

Socket::~Socket() { fechar(); }

Socket::Socket(Socket&& o) noexcept : fd_(o.fd_.exchange(INVALIDO)) {}

Socket& Socket::operator=(Socket&& o) noexcept {
    if (this != &o) {
        fechar();
        fd_ = o.fd_.exchange(INVALIDO);
    }
    return *this;
}

Socket Socket::ouvir(const std::string& host, int porta) {
    garantir_winsock();
    sock_t s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (static_cast<std::intptr_t>(s) == INVALIDO) throw ErroRede("não foi possível criar o socket");
    int um = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&um), sizeof um);
    sockaddr_in a = endereco(host, porta);
    if (::bind(s, reinterpret_cast<sockaddr*>(&a), sizeof a) != 0) {
        FECHAR_SOCK(s);
        throw ErroRede("porta " + std::to_string(porta) + " ocupada ou sem permissão");
    }
    if (::listen(s, 8) != 0) {
        FECHAR_SOCK(s);
        throw ErroRede("listen falhou");
    }
    return Socket(static_cast<std::intptr_t>(s));
}

Socket Socket::conectar(const std::string& host, int porta, double timeout_s) {
    garantir_winsock();
    sockaddr_in a = endereco(host, porta);
    sock_t s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (static_cast<std::intptr_t>(s) == INVALIDO) throw ErroRede("não foi possível criar o socket");
    // connect nao bloqueante + espera limitada, para nao travar a tela se o IP estiver errado
    modo_bloqueante(s, false);
    int r = ::connect(s, reinterpret_cast<sockaddr*>(&a), sizeof a);
    bool ok = r == 0;
    if (!ok) {
#ifdef _WIN32
        bool andamento = WSAGetLastError() == WSAEWOULDBLOCK;
#else
        bool andamento = errno == EINPROGRESS;
#endif
        if (andamento) {
            fd_set esc, err;
            FD_ZERO(&esc);
            FD_ZERO(&err);
            FD_SET(s, &esc);
            FD_SET(s, &err);
            timeval tv{static_cast<long>(timeout_s), static_cast<long>((timeout_s - static_cast<long>(timeout_s)) * 1e6)};
            if (::select(static_cast<int>(s) + 1, nullptr, &esc, &err, &tv) > 0 && FD_ISSET(s, &esc)) {
                int e = 0;
                socklen len = sizeof e;
                getsockopt(s, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&e), &len);
                ok = e == 0;
            }
        }
    }
    if (!ok) {
        FECHAR_SOCK(s);
        throw ErroRede("sem resposta de " + host + ":" + std::to_string(porta));
    }
    modo_bloqueante(s, true);
    Socket sock(static_cast<std::intptr_t>(s));
    sock.sem_atraso();
    return sock;
}

Socket Socket::aceitar(std::string* end) {
    sockaddr_in a{};
    socklen len = sizeof a;
    std::intptr_t fd = fd_.load();
    if (fd == INVALIDO) return Socket();
    sock_t c = ::accept(S(fd), reinterpret_cast<sockaddr*>(&a), &len);
    if (static_cast<std::intptr_t>(c) == INVALIDO) return Socket();
    if (end) {
        char buf[64] = {0};
        inet_ntop(AF_INET, &a.sin_addr, buf, sizeof buf);
        *end = std::string(buf) + ":" + std::to_string(ntohs(a.sin_port));
    }
    Socket s(static_cast<std::intptr_t>(c));
    s.sem_atraso();
    return s;
}

bool Socket::enviar_tudo(const std::string& dados) {
    size_t enviado = 0;
    while (enviado < dados.size()) {
        std::intptr_t fd = fd_.load();
        if (fd == INVALIDO) return false;
#ifdef _WIN32
        int n = ::send(S(fd), dados.data() + enviado, static_cast<int>(dados.size() - enviado), 0);
#else
        ssize_t n = ::send(S(fd), dados.data() + enviado, dados.size() - enviado, MSG_NOSIGNAL);
#endif
        if (n <= 0) return false;
        enviado += static_cast<size_t>(n);
    }
    return true;
}

int Socket::receber(char* buf, int n) {
    std::intptr_t fd = fd_.load();
    if (fd == INVALIDO) return -1;
    int r = static_cast<int>(::recv(S(fd), buf, n, 0));
    if (r >= 0) return r;
#ifdef _WIN32
    return WSAGetLastError() == WSAETIMEDOUT ? -2 : -1;
#else
    return (errno == EAGAIN || errno == EWOULDBLOCK) ? -2 : -1;
#endif
}

void Socket::timeout_recepcao(double segundos) {
    std::intptr_t fd = fd_.load();
    if (fd == INVALIDO) return;
#ifdef _WIN32
    DWORD ms = static_cast<DWORD>(segundos * 1000);
    setsockopt(S(fd), SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&ms), sizeof ms);
#else
    timeval tv{static_cast<long>(segundos), static_cast<long>((segundos - static_cast<long>(segundos)) * 1e6)};
    setsockopt(S(fd), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
#endif
}

void Socket::sem_atraso() {
    std::intptr_t fd = fd_.load();
    if (fd == INVALIDO) return;
    int um = 1;
    setsockopt(S(fd), IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&um), sizeof um);
}

int Socket::porta_local() const {
    sockaddr_in a{};
    socklen len = sizeof a;
    if (getsockname(S(fd_.load()), reinterpret_cast<sockaddr*>(&a), &len) != 0) return 0;
    return ntohs(a.sin_port);
}

void Socket::desligar() {
    std::intptr_t fd = fd_.load();
    if (fd != INVALIDO) ::shutdown(S(fd), SHUT_AMBOS);
}

void Socket::fechar() {
    std::intptr_t fd = fd_.exchange(INVALIDO);
    if (fd != INVALIDO) {
        ::shutdown(S(fd), SHUT_AMBOS);
        FECHAR_SOCK(S(fd));
    }
}

std::vector<std::string> ips_locais() {
    garantir_winsock();
    std::set<std::string> ips;
    char nome[256] = {0};
    if (gethostname(nome, sizeof nome) == 0) {
        addrinfo dica{}, *res = nullptr;
        dica.ai_family = AF_INET;
        if (getaddrinfo(nome, nullptr, &dica, &res) == 0) {
            for (addrinfo* p = res; p; p = p->ai_next) {
                char buf[64] = {0};
                inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in*>(p->ai_addr)->sin_addr, buf, sizeof buf);
                ips.insert(buf);
            }
            freeaddrinfo(res);
        }
    }
    // "conectar" um socket UDP nao envia nada, mas revela o IP da interface de saida
    sock_t s = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (static_cast<std::intptr_t>(s) != -1) {
        sockaddr_in a = endereco("8.8.8.8", 80);
        if (::connect(s, reinterpret_cast<sockaddr*>(&a), sizeof a) == 0) {
            sockaddr_in l{};
            socklen len = sizeof l;
            if (getsockname(s, reinterpret_cast<sockaddr*>(&l), &len) == 0) {
                char buf[64] = {0};
                inet_ntop(AF_INET, &l.sin_addr, buf, sizeof buf);
                ips.insert(buf);
            }
        }
        FECHAR_SOCK(s);
    }
    std::vector<std::string> out;
    for (auto& ip : ips)
        if (ip.rfind("127.", 0) != 0 && ip != "0.0.0.0") out.push_back(ip);
    return out;
}

}  // namespace bl::rede
