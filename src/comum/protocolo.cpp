#include "comum/protocolo.hpp"

#include <cctype>

namespace bl::proto {

namespace {

bool digitos(const std::string& s, size_t min = 1, size_t max = 100) {
    if (s.size() < min || s.size() > max) return false;
    for (char c : s)
        if (c < '0' || c > '9') return false;
    return true;
}

bool nome_valido(const std::string& s) {
    if (s.empty() || s.size() > 20) return false;
    for (char c : s)
        if (!((c >= 'a' && c <= 'z') || c == '_')) return false;
    return true;
}

std::vector<std::string> dividir(const std::string& s, char sep) {
    std::vector<std::string> partes;
    size_t ini = 0;
    for (;;) {
        size_t k = s.find(sep, ini);
        partes.push_back(s.substr(ini, k == std::string::npos ? std::string::npos : k - ini));
        if (k == std::string::npos) return partes;
        ini = k + 1;
    }
}

// "<a><sep><b>" com a e b validados por funcoes
template <class FA, class FB>
bool par(const std::string& s, char sep, FA fa, FB fb) {
    size_t k = s.find(sep);
    return k != std::string::npos && fa(s.substr(0, k)) && fb(s.substr(k + 1));
}

long long numero(const std::string& s, const std::string& linha) {
    try {
        return std::stoll(s);
    } catch (const std::exception&) {
        throw MensagemInvalida(linha);
    }
}

std::string linha(int origem, const std::string& corpo) { return std::to_string(origem) + corpo + FIM_MSG; }

}  // namespace

bool eh_comando_jogo(char c) {
    return c == TORRE || c == UPGRADE || c == VENDA || c == MODO || c == HABILIDADE || c == ENVIO;
}

bool comando_valido(const std::string& cmd) {
    if (cmd.empty()) return false;
    const std::string r = cmd.substr(1);
    auto d15 = [](const std::string& s) { return digitos(s, 1, 5); };
    switch (cmd[0]) {
        case TORRE:
            return par(r, '@', nome_valido, [](const std::string& xy) {
                return par(xy, ',', [](const std::string& s) { return digitos(s, 1, 4); },
                           [](const std::string& s) { return digitos(s, 1, 4); });
            });
        case UPGRADE:
            return par(r, ':', d15, [](const std::string& s) { return s.size() == 1 && s[0] >= '0' && s[0] <= '2'; });
        case VENDA:
            return d15(r);
        case MODO:
            return par(r, ':', d15, [](const std::string& s) { return s.size() == 1 && s[0] >= '0' && s[0] <= '3'; });
        case HABILIDADE:
            return par(r, ':', d15, [](const std::string& s) { return digitos(s, 1, 1); });
        case ENVIO: {
            if (r.empty() || r.size() > 6) return false;
            for (char c : r)
                if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) return false;
            return true;
        }
        default:
            return false;
    }
}

// ---------- montagem
std::string entrar(const std::string& heroi, const std::string& mapa, int jogador) {
    return linha(jogador, std::string(1, ENTRAR) + heroi + "," + mapa);
}
std::string boas_vindas(int jogador) { return linha(SERVIDOR, std::string(1, ENTRAR) + std::to_string(jogador)); }
std::string inicio(long long seed, const std::string& mapa, const std::string& h1, const std::string& h2) {
    return linha(SERVIDOR, std::string(1, INICIO) + std::to_string(seed) + "," + mapa + "," + h1 + "," + h2);
}
std::string tick(long numero, const std::vector<std::string>& comandos) {
    std::string corpo = std::string(1, TICK) + std::to_string(numero);
    for (auto& c : comandos) corpo += SEP_TICK + c;
    return linha(SERVIDOR, corpo);
}
std::string comando(int jogador, const std::string& cmd) { return linha(jogador, cmd); }
std::string torre(int jogador, const std::string& chave, int x, int y) {
    return comando(jogador, std::string(1, TORRE) + chave + "@" + std::to_string(x) + "," + std::to_string(y));
}
std::string upgrade(int jogador, int id_torre, int caminho) {
    return comando(jogador, std::string(1, UPGRADE) + std::to_string(id_torre) + ":" + std::to_string(caminho));
}
std::string venda(int jogador, int id_torre) { return comando(jogador, std::string(1, VENDA) + std::to_string(id_torre)); }
std::string modo(int jogador, int id_torre, int m) {
    return comando(jogador, std::string(1, MODO) + std::to_string(id_torre) + ":" + std::to_string(m));
}
std::string habilidade(int jogador, int id_torre, int idx) {
    return comando(jogador, std::string(1, HABILIDADE) + std::to_string(id_torre) + ":" + std::to_string(idx));
}
std::string envio(int jogador, const std::string& chave) { return comando(jogador, std::string(1, ENVIO) + chave); }
std::string hash_estado(int jogador, long numero_tick, std::uint32_t valor) {
    return linha(jogador, std::string(1, HASH) + std::to_string(numero_tick) + "," + std::to_string(valor));
}
std::string dessinc(long numero_tick) { return linha(SERVIDOR, std::string(1, DESSINC) + std::to_string(numero_tick)); }
std::string desistir(int jogador) { return linha(jogador, std::string(1, FIM_JOGO)); }
std::string fim_jogo(int vencedor) { return linha(SERVIDOR, std::string(1, FIM_JOGO) + std::to_string(vencedor)); }
std::string erro(int jogador, char codigo) {
    return linha(SERVIDOR, std::string(1, ERRO) + std::to_string(jogador) + codigo);
}
std::string ping(int origem) { return linha(origem, std::string(1, PING)); }

// ---------- leitura
Mensagem interpretar(const std::string& bruta) {
    // tira espacos das pontas (como str.strip)
    size_t a = 0, b = bruta.size();
    while (a < b && std::isspace(static_cast<unsigned char>(bruta[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(bruta[b - 1]))) --b;
    const std::string l = bruta.substr(a, b - a);
    if (l.size() < 2 || l[0] < '0' || l[0] > '9') throw MensagemInvalida(l);

    Mensagem m;
    m.origem = l[0] - '0';
    m.comando = l[1];
    const std::string corpo = l.substr(2);
    const bool servidor = m.origem == SERVIDOR;
    const char cmd = m.comando;

    if (cmd == PING) {
        if (!corpo.empty()) throw MensagemInvalida(l);
        return m;
    }
    if (cmd == ENTRAR) {
        if (servidor) {
            if (corpo != "1" && corpo != "2") throw MensagemInvalida(l);
            m.jogador = corpo[0] - '0';
            return m;
        }
        auto partes = dividir(corpo, ',');
        if (partes.size() != 2 || !nome_valido(partes[0]) || !nome_valido(partes[1])) throw MensagemInvalida(l);
        m.heroi = partes[0];
        m.mapa = partes[1];
        return m;
    }
    if (cmd == INICIO && servidor) {
        auto partes = dividir(corpo, ',');
        if (partes.size() != 4 || !digitos(partes[0])) throw MensagemInvalida(l);
        m.seed = numero(partes[0], l);
        m.mapa = partes[1];
        m.herois[1] = partes[2];
        m.herois[2] = partes[3];
        return m;
    }
    if (cmd == TICK && servidor) {
        auto partes = dividir(corpo, SEP_TICK);
        if (!digitos(partes[0])) throw MensagemInvalida(l);
        m.tick = static_cast<long>(numero(partes[0], l));
        for (size_t i = 1; i < partes.size(); ++i) {
            const std::string& c = partes[i];
            if (c.size() < 2 || (c[0] != '1' && c[0] != '2') || !comando_valido(c.substr(1))) throw MensagemInvalida(l);
            m.comandos.push_back({c[0] - '0', c.substr(1)});
        }
        return m;
    }
    if (eh_comando_jogo(cmd) && !servidor) {
        if (!comando_valido(l.substr(1))) throw MensagemInvalida(l);
        m.cmd = l.substr(1);
        return m;
    }
    if (cmd == HASH && !servidor) {
        auto partes = dividir(corpo, ',');
        if (partes.size() != 2 || !digitos(partes[0]) || !digitos(partes[1])) throw MensagemInvalida(l);
        m.tick = static_cast<long>(numero(partes[0], l));
        m.hash = static_cast<std::uint32_t>(numero(partes[1], l));
        return m;
    }
    if (cmd == DESSINC && servidor) {
        if (!digitos(corpo)) throw MensagemInvalida(l);
        m.tick = static_cast<long>(numero(corpo, l));
        return m;
    }
    if (cmd == FIM_JOGO) {
        if (servidor) {
            if (!digitos(corpo)) throw MensagemInvalida(l);
            m.vencedor = static_cast<int>(numero(corpo, l));
            return m;
        }
        if (!corpo.empty()) throw MensagemInvalida(l);
        return m;
    }
    if (cmd == ERRO && servidor) {
        if (corpo.size() != 2 || corpo[0] < '0' || corpo[0] > '9') throw MensagemInvalida(l);
        m.jogador = corpo[0] - '0';
        m.codigo = corpo[1];
        return m;
    }
    throw MensagemInvalida(l);
}

std::vector<std::string> LeitorDeLinhas::alimentar(const char* dados, size_t n) {
    buffer_.append(dados, n);
    std::vector<std::string> linhas;
    size_t ini = 0;
    for (size_t k; (k = buffer_.find(FIM_MSG, ini)) != std::string::npos; ini = k + 1)
        if (k > ini) linhas.push_back(buffer_.substr(ini, k - ini));
    buffer_.erase(0, ini);
    if (buffer_.size() > LIMITE) buffer_.clear();  // linha absurda: descarta
    return linhas;
}

}  // namespace bl::proto
