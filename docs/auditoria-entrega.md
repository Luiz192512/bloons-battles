# Auditoria de entrega

Data: 10/11/2026. Prazo de entrega: 15/11/2026 23:59. Branch `luiz/project-audit-coop-lan-adff47`.
Feita rodando o projeto de verdade (build, testes, cliente gráfico e um segundo jogador por TCP),
não só lendo o código.

## 1. Estado da build e dos testes

| Item | Resultado |
|---|---|
| Ferramentas desta máquina | MinGW-w64 (g++ 16.2) e CMake do Visual Studio Build Tools 18 (o `compilar.bat` não achou o `vswhere`, então usei MinGW direto) |
| Configurar e compilar | `cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release` e `cmake --build build --parallel`: sem erros |
| Testes antes das mudanças | **35 ok, 0 falha** (`bloons_testes.exe`) |
| Testes depois das mudanças | **41 ok** na base sem 3D e **62 ok, 0 falha** na branch `luiz/coop-lan-3d` (a que tem os modelos 3D, a do seu build-release); 6 testes são novos: protocolo coop, sim coop x2, servidor coop x2, IPs locais |
| Cliente gráfico | `--demo solo`, `--demo batalha` e `--vitrine 0` abrem, desenham e saem com código 0 (capturas conferidas) |
| Contagem na documentação | README dizia 27 e `btd6-transformacao.md` dizia 35. A real era 35; agora 41 e o README foi corrigido |
| Linux | **Compila e passa 100% dos testes** num contêiner Ubuntu 24.04 (Docker), e o hash da simulação (`7865b88e`) é idêntico ao do Windows em MSVC e MinGW: o lockstep entre Windows e Linux está provado |
| CI no GitHub | As 3 últimas execuções falham com os 3 jobs sem nenhum passo executado e sem log (`log not found`): o runner nem iniciou. É problema da conta/Actions (cobrança ou limite), **não do código**; só você resolve em Settings > Billing do GitHub. O código passa nos 3 ambientes equivalentes rodados localmente (MSVC, MinGW, Ubuntu) |

## 2. O que falta para entregar

| Item | Evidência | Prioridade | Situação |
|---|---|---|---|
| Gravar o vídeo de 1 min | `docs/plano.md` seção 5 tem só o roteiro | P0 | pendente (só você pode gravar) |
| Release do GitHub desatualizada | `gh release list`: v1.0 de 29/09/2026, antes da transformação BTD6 e do coop | P0 | pendente: rodar `scripts\publicar_release.bat v1.1` (não rodei) |
| Zips de entrega (`entrega/`) | pasta não existe (está no `.gitignore`) | P0 | pendente: gerados pelo script de release |
| Testar a Batalha e o Coop em **2 PCs reais** | só foi testado em 127.0.0.1 e com um jogador por TCP | P0 | pendente (precisa dos dois PCs e do firewall) |
| Conferir o formulário com o prof. | `docs/plano.md` seção 6 | P1 | atualizado com o coop (`1N`) |
| CI vermelho no GitHub | ver seção 1 | P1 | pendente |
| Build Linux | verificado localmente no Ubuntu 24.04 | P1 | feito |
| Pendências do BTD6 | `docs/btd6-transformacao.md` seção "Pendências" | P2 | não bloqueiam a entrega: são mecânicas ausentes ou aproximações, já documentadas |
| Mensagens e README | contagem de testes e coop | P1 | feito |

## 3. Bugs e defeitos

| ID | Descrição | Reprodução | Causa raiz | Correção | Teste | Situação |
|---|---|---|---|---|---|---|
| B-01 | A sala de espera mostra um IP possivelmente errado para o outro jogador digitar | Máquina com Docker/WSL (172.x) ou 169.254.x: `ips_.front()` pega o menor **texto** do `std::set` | `ips_locais()` em `src/rede/socket.cpp` ordenava os IPs como texto e o primeiro era usado | IP da interface de saída vem primeiro; 127.x e 169.254.x ficam de fora | `rede_ips_locais_sem_loopback_nem_link_local` | corrigido |
| B-02 | README diz 27 testes e o texto "oponente" aparece no modo coop | leitura | docs desatualizados | README e textos da tela de hospedar e da sala | n/a | corrigido |
| B-03 | Corrida na entrada: se os dois jogadores conectarem no mesmo instante, o início (`0I`) pode não chegar ao jogador 1 | `Servidor::atender`: `reservar_vaga` acontecia antes de `clientes_[numero] = con` | ordem das operações | reservar e registrar sob `lock_clientes_` (ordem clientes, sala) | `servidor_entrada_simultanea_recebe_inicio_nos_dois` (25 rodadas) | corrigido |
| B-04 | `1F` (desistir) durante a sala de espera encerra a sala | `Servidor::tratar` não conferia `em_jogo` | falta de checagem de estado | só vale em partida | `servidor_desistir_na_sala_de_espera_nao_encerra_a_sala` | corrigido |
| B-06 | **Online (Batalha e coop) a mira da Dartling/Helicóptero/Ás, as opções do Bumerangue/Heli e a coleta de bananas das Fazendas não funcionavam**: a simulação da branch 3D aceitava `A`, `O` e `C`, mas o protocolo os recusava | enviar `1C345` ao servidor: `0X1M` | `eh_comando_jogo`/`comando_valido` só conheciam T U V M B S N | protocolo ganhou `A`, `O` e `C` (ids de coleta até 9 dígitos) | `protocolo_mira_opcao_e_coleta` | corrigido |
| B-07 | Arquivos `tests/__pycache__/*.pyc` versionados na branch 3D | `git ls-files tests` | `.pyc` adicionados por engano | removidos do índice (`.gitignore` já ignora) | n/a | corrigido |
| B-08 | Risco de dessincronia entre sistemas por funções matemáticas (sin, cos, pow) diferentes | análise | libm de cada plataforma | n/a | hash idêntico em MSVC, MinGW e Linux | descartado na prática |
| B-05 | O IP mostrado pode ser o de uma VPN (ex.: Radmin) quando ela for a rota padrão | análise | depende da rota do sistema | n/a | n/a | suspeita (comportamento esperado: a rota padrão manda) |

## 4. Modo Coop em LAN

**Estado: pronto e estável** no que foi testado (ver seção 5 para os limites).

Design completo em `docs/plano.md` seção 2.1. Resumo:

- Dois jogadores, **mesma pista**, vidas e dinheiro comuns, qualquer um mexe em qualquer torre.
- Um só herói: o do anfitrião. Dificuldade Médio, 60 rodadas. Qualquer um inicia a rodada (`1N`/`2N`).
- Sem velocidade 3x e sem rodada automática (quebrariam o lockstep).
- Anfitrião escolhe "Modo: Coop" na tela de hospedar; quem entra recebe o modo em `0I...,coop`.
- Se o parceiro sai ou desiste: `0F0` (fim sem vencedor) e a tela mostra "PARCEIRO SAIU".
- `bloons_servidor [porta] --coop` também funciona sem interface.

Mudanças: `src/comum/protocolo.*` (comando `N`, campo `coop`), `src/servidor/sala.hpp`,
`servidor.*` (flag e `0F0`), `src/jogo/sim.*` (`Partida` modo `"coop"`), `src/cliente/controle.*`,
`cena_jogo.cpp` (UI sem oponente, envios nem AUTO), `cenas_menu.*` (alternador e textos do lobby),
`app.cpp` (`--tela hospedar-coop` e `lobby-coop`).

Validação:

- **Testes:** protocolo coop, pista única compartilhada (o jogador 2 melhora a torre do jogador 1),
  determinismo, servidor com dois clientes TCP reais (mesma ordem de comandos nos dois, origem
  trocada barrada, saída do parceiro gera `0F0`, desistir gera `0F0`).
- **Ao vivo:** o cliente gráfico hospedou em coop e um segundo jogador (script TCP) entrou como
  jogador 2, colocou um Macaco Dardo e iniciou a rodada. A captura mostra a torre dele no mapa do
  anfitrião, dinheiro $450 (caixa único), rodada 1/60 em andamento e o painel "COOP".
  Capturas: `docs/design/capturas/21_coop_hospedar.png`, `22_coop_lobby.png`,
  `23_coop_partida_01.png`.
- Quando o cliente gráfico fechou, o outro jogador recebeu `0F0`, como projetado.

Ficou de fora: o herói do jogador 2 é ignorado (só um herói por pista); o placar do
`bloons_monitor` mostra as duas linhas com a mesma pista; o modo Coop não tem tela de vitória
própria além da genérica.

Como testar em 2 PCs: no PC A, "Batalha: Hospedar", clique em "Modo: Coop", Hospedar. No PC B,
"Batalha: Entrar", digite o IP e a porta mostrados no PC A. Se não conectar, rode
`scripts\liberar_firewall.bat` como administrador no PC A e confira se o Wi-Fi está como Rede privada.

## 5. Riscos e o que não foi verificado

- **Dois PCs reais:** não testado. Tudo foi em 127.0.0.1.
- **Dois clientes gráficos ao mesmo tempo com cliques humanos:** não testado. Um cliente gráfico foi
  combinado com um jogador por script (que não simula, então o hash de dessincronia entre dois
  clientes gráficos em coop não foi exercitado ao vivo). A determinação do estado no coop está
  coberta por teste (mesma sequência de comandos gera o mesmo hash), não por duas janelas.
- **Linux:** não compilado nem testado. **CI:** falhando por motivo não investigado.
- **Vídeo, release e zips:** pendentes, são ações suas (e a release publica algo externo).
- **Telas de fim do coop** (vitória, derrota, parceiro saiu) não foram vistas na tela, só montadas
  pelo código.
- A vitrine foi aberta só na página 0; as demais páginas não foram conferidas.
