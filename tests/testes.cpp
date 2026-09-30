// Testes automaticos: protocolo, dados, regras, simulacao, exclusao mutua e servidor real.
//
// Uso: bloons_testes           (roda todos)
//      bloons_testes servidor  (so os que tem "servidor" no nome)
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <functional>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "comum/protocolo.hpp"
#include "ipc/memoria.hpp"
#include "ipc/placar.hpp"
#include "jogo/defs.hpp"
#include "jogo/sim.hpp"
#include "jogo/stats.hpp"
#include "rede/socket.hpp"
#include "servidor/sala.hpp"
#include "servidor/servidor.hpp"

using namespace bl;
namespace P = bl::proto;

// ---------------------------------------------------------------- mini framework
namespace {

struct Falha {
    std::string msg;
};

struct Caso {
    const char* nome;
    std::function<void()> f;
};

std::vector<Caso>& casos() {
    static std::vector<Caso> v;
    return v;
}

struct Registro {
    Registro(const char* nome, std::function<void()> f) { casos().push_back({nome, std::move(f)}); }
};

#define TESTE(nome)                                  \
    static void nome();                              \
    static Registro registro_##nome(#nome, nome);    \
    static void nome()

#define CHECA(cond)                                                                                  \
    do {                                                                                             \
        if (!(cond)) throw Falha{std::string(__FILE__) + ":" + std::to_string(__LINE__) + ": " #cond}; \
    } while (0)

#define CHECA_IGUAL(a, b)                                                                                   \
    do {                                                                                                    \
        auto va_ = (a);                                                                                     \
        auto vb_ = (b);                                                                                     \
        if (!(va_ == vb_))                                                                                  \
            throw Falha{std::string(__FILE__) + ":" + std::to_string(__LINE__) + ": " #a " == " #b};        \
    } while (0)

template <class F>
bool lanca(F f) {
    try {
        f();
    } catch (const P::MensagemInvalida&) {
        return true;
    }
    return false;
}

}  // namespace

// ================================================================ protocolo
TESTE(protocolo_exemplos_da_documentacao) {
    CHECA_IGUAL(P::entrar("quincy", "prado"), std::string("1Jquincy,prado\n"));
    CHECA_IGUAL(P::torre(1, "dardo", 230, 250), std::string("1Tdardo@230,250\n"));
    CHECA_IGUAL(P::upgrade(1, 12, 0), std::string("1U12:0\n"));
    CHECA_IGUAL(P::venda(1, 12), std::string("1V12\n"));
    CHECA_IGUAL(P::modo(1, 12, 3), std::string("1M12:3\n"));
    CHECA_IGUAL(P::habilidade(1, 12, 0), std::string("1B12:0\n"));
    CHECA_IGUAL(P::envio(2, "r8"), std::string("2Sr8\n"));
    CHECA_IGUAL(P::inicio(8231, "prado", "quincy", "adora"), std::string("0I8231,prado,quincy,adora\n"));
    CHECA_IGUAL(P::tick(451, {"1Tdardo@230,250", "2Sr8"}), std::string("0K451|1Tdardo@230,250|2Sr8\n"));
    CHECA_IGUAL(P::tick(452, {}), std::string("0K452\n"));
    CHECA_IGUAL(P::hash_estado(1, 450, 123456), std::string("1H450,123456\n"));
    CHECA_IGUAL(P::dessinc(450), std::string("0D450\n"));
    CHECA_IGUAL(P::fim_jogo(2), std::string("0F2\n"));
    CHECA_IGUAL(P::desistir(1), std::string("1F\n"));
}

TESTE(protocolo_ida_e_volta) {
    auto m = P::interpretar(P::entrar("adora", "lago"));
    CHECA(m.comando == P::ENTRAR && m.heroi == "adora" && m.mapa == "lago");
    m = P::interpretar(P::boas_vindas(2));
    CHECA(m.comando == P::ENTRAR && m.jogador == 2);
    m = P::interpretar(P::inicio(7, "prado", "quincy", "psi"));
    CHECA(m.comando == P::INICIO && m.seed == 7 && m.mapa == "prado" && m.herois[1] == "quincy" &&
          m.herois[2] == "psi");
    m = P::interpretar(P::tick(3, {"1Tdardo@1,2", "2U5:1"}));
    CHECA(m.comando == P::TICK && m.tick == 3 && m.comandos.size() == 2);
    CHECA(m.comandos[0] == std::make_pair(1, std::string("Tdardo@1,2")));
    CHECA(m.comandos[1] == std::make_pair(2, std::string("U5:1")));
    m = P::interpretar(P::torre(1, "super", 10, 20));
    CHECA(m.comando == P::TORRE && m.cmd == "Tsuper@10,20");
    m = P::interpretar(P::envio(2, "moa"));
    CHECA(m.comando == P::ENVIO && m.cmd == "Smoa");
    m = P::interpretar(P::hash_estado(2, 90, 42));
    CHECA(m.comando == P::HASH && m.tick == 90 && m.hash == 42u);
    m = P::interpretar(P::dessinc(90));
    CHECA(m.comando == P::DESSINC && m.tick == 90);
    m = P::interpretar(P::fim_jogo(1));
    CHECA(m.comando == P::FIM_JOGO && m.vencedor == 1);
    m = P::interpretar(P::desistir(2));
    CHECA(m.comando == P::FIM_JOGO && m.vencedor == -1);
    m = P::interpretar(P::erro(1, 'M'));
    CHECA(m.comando == P::ERRO && m.jogador == 1 && m.codigo == 'M');
}

TESTE(protocolo_invalidas) {
    for (const char* l : {"", "X", "1Z", "1Tdardo@x,4", "1U5:7", "1V", "1M3:9", "0Fx", "1Pextra", "1J", "1Jso_um",
                          "0K1|9Tdardo@1,1", "0K1|1Qqq", "1Tdardo@12345,1", "0Ix,prado,a,b", "1H1"}) {
        std::string linha = l;
        if (!lanca([&] { P::interpretar(linha); })) throw Falha{"deveria ser invalida: " + linha};
    }
}

TESTE(protocolo_cliente_nao_pode_falar_como_servidor) {
    CHECA(lanca([] { P::interpretar("1K5|1Sr8"); }));
}

TESTE(protocolo_leitor_junta_pedacos_do_tcp) {
    P::LeitorDeLinhas leitor;
    CHECA(leitor.alimentar(std::string("1Tdardo@2")).empty());
    CHECA(leitor.alimentar(std::string("30,250\n2S")) == std::vector<std::string>{"1Tdardo@230,250"});
    CHECA((leitor.alimentar(std::string("r8\n1P\n")) == std::vector<std::string>{"2Sr8", "1P"}));
}

// ================================================================ dados
TESTE(dados_22_torres_com_3x5_upgrades) {
    CHECA_IGUAL(torres().size(), size_t(22));
    for (auto& t : torres()) {
        CHECA_IGUAL(t.caminhos.size(), size_t(3));
        for (auto& c : t.caminhos) CHECA_IGUAL(c.size(), size_t(5));
    }
}

TESTE(dados_herois) {
    CHECA(herois().size() >= 17);
    for (auto& h : herois()) calcular(h.chave, {0, 0, 0}, 20);
}

TESTE(dados_todas_as_combinacoes_validas_calculam) {
    const int pares[3][2] = {{0, 1}, {1, 2}, {2, 0}};
    for (auto& t : torres())
        for (int a = 0; a < 6; ++a)
            for (int b = 0; b < 3; ++b)
                for (auto& p : pares) {
                    std::array<int, 3> cam{0, 0, 0};
                    cam[p[0]] = a;
                    cam[p[1]] = b;
                    calcular(t.chave, cam);
                }
}

TESTE(dados_rbe) {
    CHECA_IGUAL(rbe(tipo_bloon("vermelho").id), 1);
    CHECA_IGUAL(rbe(tipo_bloon("rosa").id), 5);
    CHECA_IGUAL(rbe(tipo_bloon("ceramica").id), 104);
    CHECA_IGUAL(rbe(tipo_bloon("moab").id), 616);
}

TESTE(regras_caminhos_cruzados) {
    CHECA(pode_upar({4, 2, 0}, 0));
    CHECA(!pode_upar({5, 0, 0}, 0));
    CHECA(!pode_upar({2, 2, 0}, 2));  // terceiro caminho
    CHECA(!pode_upar({3, 2, 0}, 1));  // dois caminhos acima de 2
    CHECA(pode_upar({2, 1, 0}, 1));
}

// ================================================================ simulacao
namespace {
Partida solo() { return Partida("solo", "prado", 1, "medio", {{1, "quincy"}}); }
}  // namespace

TESTE(sim_colocar_upar_vender) {
    Partida p = solo();
    Pista& pi = p.pista(1);
    CHECA_IGUAL(p.aplicar(1, "Tdardo@230,250"), OK);
    CHECA_IGUAL(pi.dinheiro, 650.0 - 200);
    CHECA_IGUAL(p.aplicar(1, "Tdardo@230,250"), ERRO_POSICAO);     // em cima de outra
    CHECA_IGUAL(p.aplicar(1, "Tdardo@170,200"), ERRO_POSICAO);     // na trilha
    CHECA_IGUAL(p.aplicar(1, "Tsubmarino@600,500"), ERRO_POSICAO); // sem agua
    CHECA_IGUAL(p.aplicar(1, "Tsuper@600,500"), ERRO_DINHEIRO);
    CHECA_IGUAL(p.aplicar(1, "Tadora@600,500"), ERRO_HEROI);       // heroi errado
    CHECA_IGUAL(p.aplicar(1, "U1:0"), OK);
    CHECA_IGUAL(p.aplicar(1, "V1"), OK);
    CHECA_IGUAL(static_cast<int>(pi.dinheiro), 650 - 200 - 140 + static_cast<int>((200 + 140) * 0.7));
}

TESTE(sim_bloon_estoura_em_camadas_e_da_dinheiro) {
    Partida p = solo();
    Pista& pi = p.pista(1);
    BloonP b = pi.criar_bloon("rosa", 100);
    double dinheiro = pi.dinheiro;
    pi.aplicar_dano(*b, 1, novo_ataque({{"dano", 1}}), nullptr);
    CHECA(!b->vivo);
    std::vector<std::string> filhos;
    for (auto& x : pi.bloons)
        if (x->vivo) filhos.push_back(x->tipo->nome);
    CHECA(filhos == std::vector<std::string>{"amarelo"});
    CHECA_IGUAL(pi.dinheiro, dinheiro + 1);
}

TESTE(sim_imunidades) {
    Partida p = solo();
    Pista& pi = p.pista(1);
    BloonP chumbo = pi.criar_bloon("chumbo", 100);
    pi.aplicar_dano(*chumbo, 5, novo_ataque({{"dtype", "afiado"}}), nullptr);
    CHECA(chumbo->vivo);  // dardo nao estoura chumbo
    pi.aplicar_dano(*chumbo, 1, novo_ataque({{"dtype", "explosao"}}), nullptr);
    CHECA(!chumbo->vivo);  // bomba estoura
    BloonP preto = pi.criar_bloon("preto", 100);
    pi.aplicar_dano(*preto, 1, novo_ataque({{"dtype", "explosao"}}), nullptr);
    CHECA(preto->vivo);  // preto e imune a explosao
}

TESTE(sim_vazamento_tira_vidas_pelo_rbe) {
    Partida p = solo();
    Pista& pi = p.pista(1);
    int vidas = pi.vidas;
    pi.criar_bloon("rosa", pi.mapa.caminhos[0].comprimento - 1);
    for (int i = 0; i < 5; ++i) p.passo();
    CHECA_IGUAL(pi.vidas, vidas - 5);
}

TESTE(sim_rodada_solo_completa) {
    Partida p = solo();
    Pista& pi = p.pista(1);
    p.aplicar(1, "Tdardo@230,250");
    p.aplicar(1, "Tdardo@110,250");
    p.aplicar(1, "N");
    for (int i = 0; i < 30 * 90; ++i) {
        p.passo();
        if (!p.em_rodada) break;
    }
    CHECA(!p.em_rodada);
    CHECA_IGUAL(p.rodada, 1);
    CHECA_IGUAL(pi.vidas, 150);
}

namespace {
std::uint32_t jogar_batalha() {
    Partida p("batalha", "encruzilhada", 9, "medio", {{1, "quincy"}, {2, "obyn"}});
    for (int i = 0; i < 30 * 60; ++i) {
        if (i == 10) p.aplicar(1, "Tdardo@150,300"), p.aplicar(2, "Ttachinha@150,300");
        if (i == 60) p.aplicar(1, "Tquincy@600,250"), p.aplicar(2, "Tobyn@600,250");
        if (i == 200) p.aplicar(1, "Sr8"), p.aplicar(2, "Sb6"), p.aplicar(1, "U1:0");
        if (i % 120 == 0 && i > 300) p.aplicar(1, "Sg5"), p.aplicar(2, "Sr8");
        p.passo();
    }
    return p.hash();
}
}  // namespace

TESTE(sim_determinismo_batalha) { CHECA_IGUAL(jogar_batalha(), jogar_batalha()); }

TESTE(sim_envio_vai_para_o_oponente_e_aumenta_eco) {
    Partida p("batalha", "prado", 1);
    for (int i = 0; i < static_cast<int>(3.5 / DT); ++i) p.passo();
    double eco = p.pista(1).eco;
    CHECA_IGUAL(p.aplicar(1, "Sr8"), OK);
    CHECA(p.pista(1).eco > eco);
    CHECA(!p.pista(2).fila.empty());
    CHECA_IGUAL(p.aplicar(1, "Sbad"), ERRO_BLOQUEADO);  // ainda nao liberado
}

TESTE(sim_recarga_de_envio_e_so_leitura) {
    // a interface mostra a espera de 0,6 s entre envios iguais; ler o valor nao pode mudar a partida
    Partida p("batalha", "prado", 1);
    for (int i = 0; i < static_cast<int>(3.5 / DT); ++i) p.passo();
    CHECA_IGUAL(p.recarga_envio(1, "r8"), 0.0);
    CHECA_IGUAL(p.aplicar(1, "Sr8"), OK);
    const std::uint32_t antes = p.hash();
    const double r = p.recarga_envio(1, "r8");
    CHECA(r > 0.5 && r <= 0.6);
    CHECA_IGUAL(p.recarga_envio(2, "r8"), 0.0);  // cada jogador tem a sua
    CHECA_IGUAL(p.hash(), antes);
    CHECA_IGUAL(p.aplicar(1, "Sr8"), ERRO_BLOQUEADO);
    for (int i = 0; i < static_cast<int>(0.7 / DT); ++i) p.passo();
    CHECA_IGUAL(p.recarga_envio(1, "r8"), 0.0);
}

TESTE(sim_todas_as_torres_atacam_sem_travar) {
    // Coloca cada torre com os tres upgrades mais caros permitidos e roda bloons fortes.
    for (auto& t : torres()) {
        for (int principal = 0; principal < 3; ++principal) {
            Partida p("solo", t.agua ? "lago" : "prado", 3, "facil", {});
            Pista& pi = p.pista(1);
            pi.dinheiro = 1e7;
            const char* pos = t.agua ? "530,300" : "300,220";
            CHECA_IGUAL(p.aplicar(1, "T" + t.chave + "@" + pos), OK);
            for (int k = 0; k < 5; ++k) p.aplicar(1, "U1:" + std::to_string(principal));
            for (int k = 0; k < 2; ++k) p.aplicar(1, "U1:" + std::to_string((principal + 1) % 3));
            CHECA_IGUAL(pi.torre(1)->caminhos[principal], 5);
            for (int i = 0; i < 30 * 46; ++i) p.passo();  // espera a recarga inicial das habilidades
            for (size_t k = 0; k < pi.torre(1)->st.habs.size(); ++k)
                CHECA_IGUAL(p.aplicar(1, "B1:" + std::to_string(k)), OK);
            for (const char* b : {"ceramica", "moab", "chumbo", "zebra", "ddt"}) pi.agendar(b, 0.1);
            for (int i = 0; i < 30 * 12; ++i) p.passo();
        }
    }
}

// ================================================================ exclusao mutua
TESTE(mutex_vagas_nao_se_repetem_com_threads_concorrentes) {
    for (int rep = 0; rep < 50; ++rep) {
        Sala sala;
        std::mutex m;
        std::condition_variable cv;
        int prontas = 0;
        bool largar = false;
        std::vector<int> obtidos;
        std::mutex lock_obtidos;
        std::vector<std::thread> threads;
        for (int i = 0; i < 8; ++i) {
            threads.emplace_back([&] {
                {
                    // barreira: todas comecam juntas
                    std::unique_lock<std::mutex> l(m);
                    ++prontas;
                    cv.notify_all();
                    cv.wait(l, [&] { return largar; });
                }
                auto numero = sala.reservar_vaga("quincy", "prado");
                std::lock_guard<std::mutex> trava(lock_obtidos);
                if (numero) obtidos.push_back(*numero);
            });
        }
        {
            std::unique_lock<std::mutex> l(m);
            cv.wait(l, [&] { return prontas == 8; });
            largar = true;
            cv.notify_all();
        }
        for (auto& t : threads) t.join();
        std::sort(obtidos.begin(), obtidos.end());
        CHECA((obtidos == std::vector<int>{1, 2}));
    }
}

TESTE(mutex_fila_de_comandos_nao_perde_nem_duplica) {
    // Produtores (clientes) e consumidor (relogio) ao mesmo tempo.
    Sala sala;
    const int por_thread = 3000;
    std::vector<std::string> recebidos;
    std::atomic<bool> parar{false};
    std::thread consumidor([&] {
        while (!parar) {
            auto [t, cmds] = sala.fechar_tick();
            recebidos.insert(recebidos.end(), cmds.begin(), cmds.end());
        }
        auto [t, cmds] = sala.fechar_tick();
        recebidos.insert(recebidos.end(), cmds.begin(), cmds.end());
    });
    std::vector<std::thread> prods;
    for (int j : {1, 2})
        prods.emplace_back([&sala, j, por_thread] {
            for (int i = 0; i < por_thread; ++i) sala.enfileirar(j, "Sr" + std::to_string(i));
        });
    for (auto& t : prods) t.join();
    parar = true;
    consumidor.join();
    CHECA_IGUAL(recebidos.size(), size_t(2 * por_thread));
    CHECA_IGUAL(std::set<std::string>(recebidos.begin(), recebidos.end()).size(), size_t(2 * por_thread));
    // a ordem de cada produtor e preservada
    std::vector<int> so_1;
    for (auto& r : recebidos)
        if (r[0] == '1') so_1.push_back(std::stoi(r.substr(3)));
    for (int i = 0; i < por_thread; ++i) CHECA_IGUAL(so_1[i], i);
}

// ================================================================ servidor
namespace {

class Cliente {
public:
    explicit Cliente(int porta) : s_(rede::Socket::conectar("127.0.0.1", porta, 3)) { s_.timeout_recepcao(3); }

    void enviar(const std::string& linha) { s_.enviar_tudo(linha); }

    // Proxima linha (ignorando ticks vazios e pings, ou filtrando por comando).
    std::string proxima(char filtro = 0) {
        auto fim = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (std::chrono::steady_clock::now() < fim) {
            while (!buffer_.empty()) {
                std::string l = buffer_.front();
                buffer_.erase(buffer_.begin());
                if (l == "0P") continue;
                if (!filtro && l.rfind("0K", 0) == 0 && l.find('|') == std::string::npos) continue;
                if (!filtro || l[1] == filtro) return l;
            }
            char buf[4096];
            int n = s_.receber(buf, sizeof buf);
            if (n > 0) {
                auto linhas = leitor_.alimentar(buf, n);
                buffer_.insert(buffer_.end(), linhas.begin(), linhas.end());
            } else if (n == 0 || n == -1) {
                break;
            }
        }
        throw Falha{"tempo esgotado esperando mensagem"};
    }

    void fechar() { s_.fechar(); }

private:
    rede::Socket s_;
    P::LeitorDeLinhas leitor_;
    std::vector<std::string> buffer_;
};

}  // namespace

TESTE(servidor_partida_completa) {
    Servidor servidor("127.0.0.1", 0);
    servidor.iniciar();
    Cliente a(servidor.porta());
    a.enviar(P::entrar("quincy", "lago"));
    CHECA_IGUAL(a.proxima(), std::string("0J1"));

    Cliente b(servidor.porta());
    b.enviar(P::entrar("heroi_inexistente", "prado"));
    CHECA_IGUAL(b.proxima(), std::string("0J2"));
    std::string ib = b.proxima(), ia = a.proxima();
    CHECA_IGUAL(ia, ib);
    auto inicio = P::interpretar(ia);
    CHECA_IGUAL(inicio.mapa, std::string("lago"));  // mapa do jogador 1
    CHECA(inicio.herois[1] == "quincy" && inicio.herois[2] == "quincy");  // heroi invalido -> padrao

    // terceiro cliente: sala cheia
    Cliente c(servidor.porta());
    c.enviar(P::entrar("adora", "prado"));
    CHECA_IGUAL(c.proxima(), std::string("0X0S"));

    // comandos chegam aos DOIS clientes dentro do mesmo tick, na mesma ordem
    a.enviar(P::torre(1, "dardo", 100, 100));
    b.enviar(P::envio(2, "r8"));
    std::vector<std::pair<int, std::string>> vistos_a, vistos_b;
    while (vistos_a.size() < 2) {
        auto m = P::interpretar(a.proxima());
        vistos_a.insert(vistos_a.end(), m.comandos.begin(), m.comandos.end());
    }
    while (vistos_b.size() < 2) {
        auto m = P::interpretar(b.proxima());
        vistos_b.insert(vistos_b.end(), m.comandos.begin(), m.comandos.end());
    }
    CHECA(vistos_a == vistos_b);
    auto ordenado = vistos_a;
    std::sort(ordenado.begin(), ordenado.end());
    CHECA((ordenado == std::vector<std::pair<int, std::string>>{{1, "Tdardo@100,100"}, {2, "Sr8"}}));

    // cliente nao pode se passar pelo outro
    a.enviar(P::envio(2, "r8"));
    CHECA_IGUAL(a.proxima(P::ERRO), std::string("0X1M"));

    // hashes diferentes -> dessincronia avisada aos dois
    a.enviar(P::hash_estado(1, 45, 111));
    b.enviar(P::hash_estado(2, 45, 222));
    CHECA_IGUAL(a.proxima(P::DESSINC), std::string("0D45"));
    CHECA_IGUAL(b.proxima(P::DESSINC), std::string("0D45"));

    // desistencia: o oponente vence
    a.enviar(P::desistir(1));
    CHECA_IGUAL(b.proxima(P::FIM_JOGO), std::string("0F2"));
    a.fechar();
    b.fechar();
    c.fechar();
    servidor.parar();
}

TESTE(servidor_desconexao_da_vitoria_ao_oponente) {
    Servidor servidor("127.0.0.1", 0);
    servidor.iniciar();
    Cliente a(servidor.porta());
    a.enviar(P::entrar("quincy", "prado"));
    CHECA_IGUAL(a.proxima(), std::string("0J1"));  // garante que a e o jogador 1
    Cliente b(servidor.porta());
    b.enviar(P::entrar("quincy", "prado"));
    b.proxima(P::INICIO);
    a.fechar();
    CHECA_IGUAL(b.proxima(P::FIM_JOGO), std::string("0F2"));
    b.fechar();
    servidor.parar();
}

TESTE(servidor_parar_com_clientes_conectados_nao_trava) {
    Servidor servidor("127.0.0.1", 0);
    servidor.iniciar();
    Cliente a(servidor.porta());
    a.enviar(P::entrar("quincy", "prado"));
    CHECA_IGUAL(a.proxima(), std::string("0J1"));
    Cliente b(servidor.porta());  // conectado, mas ainda sem mensagem
    servidor.parar();             // precisa acordar as threads presas no recv/accept
}

// ================================================================ memoria compartilhada entre processos
namespace {

std::string pasta_exe = ".";  // pasta do bloons_testes (o monitor fica ao lado)

std::string nome_unico(const std::string& base) { return base + "_" + std::to_string(ipc::pid_atual()); }

// Roda um programa como outro processo e devolve o que ele escreveu no console.
std::string rodar_processo(std::string comando) {
#ifdef _WIN32
    for (char& c : comando)
        if (c == '/') c = '\\';
    FILE* f = _popen(comando.c_str(), "r");
#else
    FILE* f = popen(comando.c_str(), "r");
#endif
    if (!f) throw Falha{"nao foi possivel rodar: " + comando};
    std::string saida;
    char buf[512];
    while (std::fgets(buf, sizeof buf, f)) saida += buf;
#ifdef _WIN32
    _pclose(f);
#else
    pclose(f);
#endif
    return saida;
}

}  // namespace

TESTE(ipc_memoria_com_nome_liga_dois_mapeamentos) {
    const std::string nome = nome_unico("bloons_teste_mem");
    auto dono = ipc::MemoriaCompartilhada::criar(nome, 4096);
    auto outro = ipc::MemoriaCompartilhada::abrir(nome, 4096);
    CHECA(outro != nullptr);
    CHECA(dono->dados() != outro->dados());  // dois mapeamentos diferentes...
    std::strcpy(static_cast<char*>(dono->dados()), "bloons");
    CHECA(std::string(static_cast<char*>(outro->dados())) == "bloons");  // ...dos mesmos bytes
    CHECA(ipc::MemoriaCompartilhada::abrir(nome_unico("bloons_nao_existe"), 64) == nullptr);
}

TESTE(ipc_mutex_com_nome_nao_perde_incrementos) {
    // cada thread abre a regiao por conta propria, como faria um processo separado
    const std::string nome = nome_unico("bloons_teste_mutex");
    auto dono = ipc::MemoriaCompartilhada::criar(nome, sizeof(long long));
    const int threads = 4, por_thread = 5000;
    std::vector<std::thread> ts;
    for (int k = 0; k < threads; ++k) {
        ts.emplace_back([&] {
            auto m = ipc::MemoriaCompartilhada::abrir(nome, sizeof(long long));
            volatile long long* contador = static_cast<long long*>(m->dados());
            for (int i = 0; i < por_thread; ++i) {
                ipc::MemoriaCompartilhada::Trava trava(*m);
                long long v = *contador;  // ler, somar e gravar: sem o mutex, incrementos se perderiam
                if (i % 1000 == 0) std::this_thread::yield();
                *contador = v + 1;
            }
        });
    }
    for (auto& t : ts) t.join();
    CHECA_IGUAL(*static_cast<long long*>(dono->dados()), static_cast<long long>(threads) * por_thread);
}

TESTE(ipc_servidor_publica_placar_e_outro_processo_le) {
    Servidor servidor("127.0.0.1", 0);
    servidor.iniciar();
    Cliente a(servidor.porta());
    a.enviar(P::entrar("quincy", "lago"));
    CHECA_IGUAL(a.proxima(), std::string("0J1"));
    Cliente b(servidor.porta());
    b.enviar(P::entrar("adora", "prado"));
    b.proxima(P::INICIO);
    a.enviar(P::torre(1, "dardo", 100, 100));
    a.proxima();  // tick com o comando

    auto placar = ipc::PlacarCompartilhado::abrir(servidor.porta());
    CHECA(placar != nullptr);
    // o relogio transmite o tick e so depois publica no placar: espera ele alcancar
    ipc::Placar pl = placar->ler();
    for (int i = 0; i < 100 && pl.comandos < 1; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        pl = placar->ler();
    }
    CHECA_IGUAL(pl.estado, static_cast<std::int32_t>(ipc::EM_JOGO));
    CHECA_IGUAL(std::string(pl.mapa), std::string("lago"));
    CHECA_IGUAL(std::string(pl.jogador[1].heroi), std::string("quincy"));
    CHECA_IGUAL(std::string(pl.jogador[2].heroi), std::string("adora"));
    CHECA(pl.tick > 0 && pl.comandos >= 1);
    CHECA_IGUAL(pl.pid_servidor, static_cast<std::int32_t>(ipc::pid_atual()));

    // um processo de verdade (bloons_monitor) le a mesma memoria
#ifdef _WIN32
    const std::string exe = pasta_exe + "/bloons_monitor.exe";
#else
    const std::string exe = pasta_exe + "/bloons_monitor";
#endif
    const std::string saida = rodar_processo("\"" + exe + "\" " + std::to_string(servidor.porta()) + " --uma-vez");
    if (saida.find("EM_JOGO") == std::string::npos || saida.find("adora") == std::string::npos)
        throw Falha{"monitor nao leu o placar: " + saida};

    a.enviar(P::desistir(1));
    b.proxima(P::FIM_JOGO);
    pl = placar->ler();
    CHECA_IGUAL(pl.estado, static_cast<std::int32_t>(ipc::FIM));
    CHECA_IGUAL(pl.vencedor, 2);
    a.fechar();
    b.fechar();
    servidor.parar();
}

// ================================================================ main
int main(int argc, char** argv) {
    const std::string filtro = argc > 1 ? argv[1] : "";
    const std::string eu = argv[0];
    const size_t barra = eu.find_last_of("/\\");
    if (barra != std::string::npos) pasta_exe = eu.substr(0, barra);
    int ok = 0, falhas = 0;
    for (auto& c : casos()) {
        if (!filtro.empty() && std::string(c.nome).find(filtro) == std::string::npos) continue;
        auto ini = std::chrono::steady_clock::now();
        try {
            c.f();
            ++ok;
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - ini);
            std::printf("  ok    %-55s %5lld ms\n", c.nome, static_cast<long long>(ms.count()));
        } catch (const Falha& f) {
            ++falhas;
            std::printf("  FALHA %-55s\n        %s\n", c.nome, f.msg.c_str());
        } catch (const std::exception& e) {
            ++falhas;
            std::printf("  ERRO  %-55s\n        %s\n", c.nome, e.what());
        }
    }
    std::printf("\n%d ok, %d falha(s)\n", ok, falhas);
    return falhas ? 1 : 0;
}
