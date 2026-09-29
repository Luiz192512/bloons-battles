// Notacao de mensagens do jogo.
//
// Cada mensagem e uma linha ASCII terminada em '\n', no formato:
//
//     <origem><comando><argumentos>
//
// origem: '0' servidor, '1' ou '2' jogador.
//
// Cliente -> servidor
//     1Jquincy,prado    entrar com o heroi Quincy, sugerindo o mapa "prado"
//     1Tdardo@230,250   colocar Macaco Dardo em (230, 250)
//     1U12:0            upgrade da torre 12 no caminho 0 (superior)
//     1V12              vender a torre 12
//     1M12:3            modo de alvo da torre 12 (0 primeiro, 1 ultimo, 2 perto, 3 forte)
//     1B12:0            usar a habilidade 0 da torre 12
//     2Sr8              jogador 2 envia "8 Vermelhos" ao oponente
//     1H450,123456      hash do estado da simulacao no tick 450 (deteccao de dessincronia)
//     1F                desistir
//     1P                heartbeat
//
// Servidor -> clientes
//     0J1                           voce e o jogador 1
//     0I8231,prado,quincy,adora     inicio: semente, mapa, heroi do J1, heroi do J2
//     0K451|1Tdardo@230,250|2Sr8    tick 451 com os comandos a aplicar, na ordem
//     0D450                         estados divergiram no tick 450
//     0F2                           fim de jogo, jogador 2 venceu
//     0X1D                          erro do jogador 1 (codigo)
//     0P                            heartbeat
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace bl::proto {

constexpr const char* HOST_PADRAO = "127.0.0.1";
constexpr int PORTA_PADRAO = 5050;
constexpr int MAX_JOGADORES = 2;
constexpr double HEARTBEAT_INTERVALO = 2.0;  // segundos entre pings
constexpr double HEARTBEAT_TIMEOUT = 8.0;    // segundos sem mensagem = desconectado

constexpr int SERVIDOR = 0;
constexpr char FIM_MSG = '\n';
constexpr char SEP_TICK = '|';

// Comandos de controle
constexpr char ENTRAR = 'J';
constexpr char INICIO = 'I';
constexpr char TICK = 'K';
constexpr char HASH = 'H';
constexpr char DESSINC = 'D';
constexpr char FIM_JOGO = 'F';
constexpr char ERRO = 'X';
constexpr char PING = 'P';

// Comandos de jogo (repassados pelo servidor dentro do tick)
constexpr char TORRE = 'T';
constexpr char UPGRADE = 'U';
constexpr char VENDA = 'V';
constexpr char MODO = 'M';
constexpr char HABILIDADE = 'B';
constexpr char ENVIO = 'S';
bool eh_comando_jogo(char c);

// Codigos de erro de 0X<jogador><codigo>
constexpr char ERRO_SALA_CHEIA = 'S';
constexpr char ERRO_INVALIDA = 'M';
constexpr char ERRO_FORA_DE_HORA = 'O';

struct MensagemInvalida : std::runtime_error {
    explicit MensagemInvalida(const std::string& linha) : std::runtime_error("mensagem invalida: " + linha) {}
};

// Mensagem interpretada. So os campos do comando recebido sao preenchidos.
struct Mensagem {
    int origem = 0;
    char comando = 0;
    int jogador = 0;               // J (servidor), X
    std::string heroi, mapa;       // J (cliente), I
    long long seed = 0;            // I
    std::string herois[3];         // I: herois[1], herois[2]
    long tick = 0;                 // K, H, D
    std::vector<std::pair<int, std::string>> comandos;  // K
    std::string cmd;               // comando de jogo (sem a origem)
    std::uint32_t hash = 0;        // H
    int vencedor = -1;             // F (servidor)
    char codigo = 0;               // X
};

// Sintaxe de um comando de jogo (sem o digito de origem).
bool comando_valido(const std::string& cmd);

// ---------- montagem
std::string entrar(const std::string& heroi, const std::string& mapa, int jogador = 1);
std::string boas_vindas(int jogador);
std::string inicio(long long seed, const std::string& mapa, const std::string& heroi1, const std::string& heroi2);
std::string tick(long numero, const std::vector<std::string>& comandos);
std::string comando(int jogador, const std::string& cmd);
std::string torre(int jogador, const std::string& chave, int x, int y);
std::string upgrade(int jogador, int id_torre, int caminho);
std::string venda(int jogador, int id_torre);
std::string modo(int jogador, int id_torre, int m);
std::string habilidade(int jogador, int id_torre, int idx);
std::string envio(int jogador, const std::string& chave);
std::string hash_estado(int jogador, long numero_tick, std::uint32_t valor);
std::string dessinc(long numero_tick);
std::string desistir(int jogador);
std::string fim_jogo(int vencedor);
std::string erro(int jogador, char codigo);
std::string ping(int origem);

// ---------- leitura
// Converte uma linha (sem '\n') em Mensagem. Lanca MensagemInvalida.
Mensagem interpretar(const std::string& linha);

// TCP e um fluxo de bytes: junta pedacos recebidos e devolve linhas completas.
class LeitorDeLinhas {
public:
    static constexpr size_t LIMITE = 1 << 16;
    std::vector<std::string> alimentar(const char* dados, size_t n);
    std::vector<std::string> alimentar(const std::string& dados) { return alimentar(dados.data(), dados.size()); }

private:
    std::string buffer_;
};

}  // namespace bl::proto
