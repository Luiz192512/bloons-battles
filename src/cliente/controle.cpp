#include "cliente/controle.hpp"

namespace bl {

namespace P = proto;

std::string mensagem_erro(char erro) {
    switch (erro) {
        case ERRO_DINHEIRO: return "Dinheiro insuficiente!";
        case ERRO_POSICAO: return "Não pode colocar aqui!";
        case ERRO_INVALIDO: return "Ação inválida.";
        case ERRO_HEROI: return "Você só pode ter 1 herói.";
        case ERRO_BLOQUEADO: return "Bloqueado.";
        default: return "Não foi possível.";
    }
}

// ---------------------------------------------------------------- solo
ControladorSolo::ControladorSolo(const std::string& mapa, const std::string& dificuldade, const std::string& heroi,
                                 int seed, const std::string& restricao)
    : mapa_(mapa), dificuldade_(dificuldade), heroi_(heroi), restricao_(restricao), seed_(seed) {
    partida = std::make_unique<Partida>("solo", mapa, seed, dificuldade, std::map<int, std::string>{{1, heroi}});
    partida->pista(1).restricao = restricao;
}

std::unique_ptr<ControladorSolo> ControladorSolo::reiniciar() const {
    return std::make_unique<ControladorSolo>(mapa_, dificuldade_, heroi_, seed_, restricao_);
}

void ControladorSolo::botao_play() {
    if (!partida->em_rodada) partida->aplicar(1, "N");
    else velocidade = velocidade == 1 ? 3 : 1;
}

void ControladorSolo::atualizar(double dt) {
    if (pausado || partida->fim) return;
    acc_ += std::min(dt, 0.1) * velocidade;
    int n = 0;
    while (acc_ >= DT && n < 12) {
        partida->passo();
        acc_ -= DT;
        ++n;
    }
}

// ---------------------------------------------------------------- batalha
ControladorBatalha::ControladorBatalha(std::shared_ptr<Conexao> conexao, int numero, long long seed,
                                       const std::string& mapa, const std::map<int, std::string>& herois,
                                       std::shared_ptr<Servidor> servidor, std::vector<P::Mensagem> pendentes,
                                       int porta)
    : conexao_(std::move(conexao)), servidor_(std::move(servidor)), pendentes_(std::move(pendentes)) {
    meu = numero;
    partida = std::make_unique<Partida>("batalha", mapa, static_cast<int>(seed), "medio", herois);
    if (meu == 1 || meu == 2) placar_ = ipc::PlacarCompartilhado::abrir(porta);
}

void ControladorBatalha::publicar_placar() {
    if (!placar_) return;
    const Pista& p = pista();
    // copia os numeros antes de travar: a secao critica so copia bytes
    ipc::PlacarJogador linha{};
    linha.conectado = 1;
    ipc::copiar_texto(linha.heroi, sizeof linha.heroi, p.heroi_escolhido);
    linha.vidas = p.vidas;
    linha.dinheiro = static_cast<std::int64_t>(p.dinheiro);
    linha.eco = static_cast<std::int32_t>(p.eco);
    linha.rodada = partida->rodada;
    linha.pops = p.pops_total;
    linha.torres = static_cast<std::int32_t>(p.torres.size());
    linha.bloons = static_cast<std::int32_t>(p.bloons.size());
    linha.hash = p.hash();
    linha.tick = tick_;
    linha.pid = ipc::pid_atual();
    const int j = meu;
    placar_->atualizar([&](ipc::Placar& pl) { pl.jogador[j] = linha; });
}

char ControladorBatalha::checar(const std::string& cmd) {
    Pista& p = pista();
    const char c = cmd[0];
    const std::string corpo = cmd.substr(1);
    try {
        if (c == 'T') {
            size_t a = corpo.find('@'), v = corpo.find(',');
            if (a == std::string::npos || v == std::string::npos) return ERRO_INVALIDO;
            const std::string chave = corpo.substr(0, a);
            int x = std::stoi(corpo.substr(a + 1, v - a - 1)), y = std::stoi(corpo.substr(v + 1));
            const DefTorre* dfn = achar_definicao(chave);
            if (!dfn) return ERRO_INVALIDO;
            if (dfn->heroi && (p.tem_heroi || chave != p.heroi_escolhido)) return ERRO_HEROI;
            if (!p.posicao_valida(*dfn, x, y)) return ERRO_POSICAO;
            if (p.dinheiro < p.custo(dfn->custo, x, y)) return ERRO_DINHEIRO;
        } else if (c == 'U') {
            size_t k = corpo.find(':');
            if (k == std::string::npos) return ERRO_INVALIDO;
            TorreP t = p.torre(std::stoi(corpo.substr(0, k)));
            if (!t) return ERRO_INVALIDO;
            auto custo = p.custo_upgrade(*t, std::stoi(corpo.substr(k + 1)));
            if (!custo) return ERRO_BLOQUEADO;
            if (p.dinheiro < *custo) return ERRO_DINHEIRO;
        } else if (c == 'S') {
            const Envio* env = achar_envio(corpo);
            if (!env) return ERRO_INVALIDO;
            if (partida->rodada < env->rodada_min) return ERRO_BLOQUEADO;
            if (p.dinheiro < env->custo) return ERRO_DINHEIRO;
        }
    } catch (const std::exception&) {
        return ERRO_INVALIDO;
    }
    return OK;
}

char ControladorBatalha::enviar(const std::string& cmd) {
    if (terminou()) return ERRO_INVALIDO;
    if (char erro = checar(cmd)) return erro;
    conexao_->comando(cmd);
    return OK;
}

void ControladorBatalha::atualizar(double) {
    std::vector<P::Mensagem> msgs;
    msgs.swap(pendentes_);
    for (auto& m : conexao_->pegar_mensagens()) msgs.push_back(std::move(m));
    for (const P::Mensagem& msg : msgs) {
        if (msg.comando == P::TICK) {
            tick_ = msg.tick;
            if (partida->fim) continue;
            for (auto& [jogador, cmd] : msg.comandos) partida->aplicar(jogador, cmd);
            for (int i = 0; i < PASSOS_POR_TICK; ++i) partida->passo();
            publicar_placar();
            if (tick_ % HASH_A_CADA == 0) conexao_->enviar(P::hash_estado(meu, tick_, partida->hash()));
        } else if (msg.comando == P::DESSINC) {
            dessinc_ = true;
            status = "Dessincronia detectada no tick " + std::to_string(msg.tick) + "!";
        } else if (msg.comando == P::FIM_JOGO) {
            fim_remoto = msg.vencedor;
        }
    }
    if (!conexao_->ativo() && !terminou()) {
        fim_remoto = 0;
        status = "Conexão perdida.";
    }
}

void ControladorBatalha::desistir() { conexao_->enviar(P::desistir(meu)); }

void ControladorBatalha::fechar() {
    conexao_->fechar();
    if (servidor_) servidor_->parar();
}

}  // namespace bl
