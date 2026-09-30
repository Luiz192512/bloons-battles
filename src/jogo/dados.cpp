// Dados do jogo: bloons, 22 torres (3 caminhos x 5 upgrades), 18 herois, mapas e rodadas.
//
// Valores do Bloons TD 6 (dificuldade Media), tirados da Blooncyclopedia e da Bloons Wiki
// (fontes em docs/btd6-transformacao.md) e adaptados para esta simulacao. Alcances do original
// multiplicados por 4 (px de um mapa 1040x720).
//
// Cada upgrade tem um objeto de efeitos aplicado por stats.cpp (aplicar):
//   aditivos:        dano, pierce, n, splash, sdano, spierce, quica, moab, cer, fort,
//                    dist, raio_proj, valor, pilha_pierce, saltos, empurra, alcance
//   multiplicativos: cad, vel, pilha_vida, alcance_x, valor_x, impreciso
//   definicao:       dtype, sdtype, camo, busca, global_, visual, lento, congela, cola,
//                    queima, atordoa, fragiliza, retira_camo, retira_regen, ouro, spread,
//                    frag, fusivel, boom, armadilha, prende_moab, ouro_chumbo
//   estruturais:     a (indice do ataque alvo, "todos", ou um tipo/visual como "uva" ou "renda"),
//                    novo (ataque novo), subst (substitui o ataque), hab (habilidade),
//                    buffs (para torres buff). Um upgrade pode ter uma lista de efeitos
//                    (J::array) para mirar ataques diferentes.

#include "jogo/defs.hpp"

namespace bl {

namespace {

Upgrade U(const char* nome, int custo, const char* desc, J ef) {
    return Upgrade{nome, custo, desc, std::move(ef)};
}

J A(const char* tipo, J kw) {
    kw["tipo"] = tipo;
    return kw;
}

J H(const char* nome, const char* tipo, double recarga, J kw) {
    J h = {{"nome", nome}, {"tipo", tipo}, {"recarga", recarga}};
    for (auto& [k, v] : kw.items()) h[k] = v;
    return h;
}

// ================================================================ TORRES
static DefTorre t_dardo() {
    DefTorre t("dardo", "Macaco Dardo", 200, "q", 128);
    t.ataques = {A("projetil", {{"cad", 0.95}, {"dano", 1}, {"pierce", 2}, {"vel", 900}, {"dist", 220}, {"visual", "dardo"}})};
    t.caminhos = {
        {
            U("Tiros Afiados", 140, "Dardos estouram +1 bloon.", {{"pierce", 1}}),
            U("Tiros Super Afiados", 200, "Dardos estouram +2 bloons.", {{"pierce", 2}}),
            U("Espinhopulta", 320, "Bolas de espinhos: 2 de dano, pierce 18, quicam em obstáculos.", {{"dano", 1}, {"pierce", 13}, {"cad", 1.21}, {"raio_proj", 6}, {"vel", 0.5}, {"alcance", 19}, {"visual", "bola_espinho"}}),
            U("Juggernaut", 1800, "Bola gigante: pierce 60, estoura chumbo, +3 em cerâmica e +2 em fortificado.", {{"cad", 0.87}, {"pierce", 42}, {"dtype", "normal"}, {"cer", 3}, {"fort", 2}, {"raio_proj", 6}, {"vel", 2.0}, {"visual", "juggernaut"}}),
            U("Ultra-Juggernaut", 15000, "5 de dano, pierce 210; se divide em 6 juggernauts.", {{"dano", 3}, {"fort", 3}, {"cer", 5}, {"pierce", 150}, {"raio_proj", 4}, {"frag", {{"n", 6}, {"dano", 2}, {"pierce", 50}, {"dtype", "normal"}, {"cer", 3}, {"fort", 2}, {"visual", "juggernaut"}}}}),
        },
        {
            U("Tiros Rápidos", 100, "Atira mais rápido.", {{"cad", 0.85}}),
            U("Tiros Muito Rápidos", 190, "Atira ainda mais rápido.", {{"cad", 0.788}}),
            U("Tiro Triplo", 450, "Atira 3 dardos por vez, mais rápido.", {{"n", 2}, {"spread", 30}, {"cad", 0.75}}),
            U("Fã-Clube Super Macaco", 7200, "Atira 2x mais rápido. Habilidade: até 10 dardos viram Super Macacos por 15 s.", {{"cad", 0.5}, {"hab", H("Fã-Clube Super Macaco", "turbo_area", 50, {{"dur", 15}, {"valor", 0.19}, {"filtro", "dardo"}})}}),
            U("Fã-Clube Macaco Plasma", 45000, "Habilidade: até 20 dardos viram Macacos Plasma por 15 s.", {{"dano", 1}, {"pierce", 3}, {"dtype", "normal"}, {"hab", H("Fã-Clube Macaco Plasma", "turbo_area", 50, {{"dur", 15}, {"valor", 0.095}, {"filtro", "dardo"}})}}),
        },
        {
            U("Dardos de Longo Alcance", 90, "+8 de alcance.", {{"alcance", 32}, {"dist", 60}}),
            U("Visão Aprimorada", 200, "+8 de alcance e detecta camo.", {{"alcance", 32}, {"camo", true}, {"vel", 1.1667}}),
            U("Besta", 575, "Besta: 3 de dano, pierce 4, alcance 60.", {{"alcance", 48}, {"dano", 2}, {"pierce", 2}, {"vel", 1.1}, {"visual", "flecha"}}),
            U("Atirador Afiado", 2050, "6 de dano, 2x mais rápido e crítico de 50 a cada 10 tiros.", {{"cad", 0.5}, {"dano", 3}, {"crit_cada", 10}, {"crit_dano", 50}}),
            U("Mestre da Besta", 21500, "8 de dano, pierce 8, alcance 80, 2x mais rápido, estoura tudo; crítico de 80 a cada 5 tiros.", {{"cad", 0.5}, {"dano", 2}, {"pierce", 4}, {"alcance", 80}, {"dtype", "normal"}, {"crit_cada", 5}, {"crit_dano", 80}}),
        },
    };
    t.desc = "Atira dardos. Barato e versátil.";
    t.cor = {150, 95, 45};
    return t;
}

static DefTorre t_bumerangue() {
    DefTorre t("bumerangue", "Macaco Bumerangue", 315, "w", 172);
    t.ataques = {A("projetil", {{"cad", 1.2}, {"dano", 1}, {"pierce", 4}, {"vel", 520}, {"dist", 240}, {"boom", true}, {"raio_proj", 8}, {"visual", "bumerangue"}})};
    t.caminhos = {
        {
            U("Bumerangues Melhorados", 200, "Pierce 8.", {{"pierce", 4}}),
            U("Glaives", 280, "Glaives: pierce 13.", {{"pierce", 5}, {"visual", "glaive"}}),
            U("Ricochete de Glaive", 600, "Glaives em linha reta que ricocheteiam entre bloons (pierce 15).", {{"pierce", 2}, {"quica", 15}}),
            U("M.O.A.R. Glaives", 2000, "3x mais rápido, pierce 60.", {{"cad", 0.333}, {"pierce", 45}, {"quica", 45}, {"vel", 1.36}}),
            U("Senhor das Glaives", 32500, "Glaives orbitam o macaco: 4 de dano a cada 0,05 s em até 200 bloons.", {{"novo", A("aura", {{"cad", 0.05}, {"dano", 4}, {"pierce", 200}, {"cer", 4}, {"raio_aura", 120}, {"visual", "orbita_glaive"}})}}),
        },
        {
            U("Arremesso Rápido", 175, "", {{"cad", 0.75}}),
            U("Bumerangues Velozes", 250, "Arremessa ainda mais rápido.", {{"vel", 1.3}, {"cad", 0.667}}),
            U("Bumerangue Biônico", 1250, "Ataca 4x mais rápido, +1 em M.O.A.B.", {{"cad", 0.25}, {"moab", 1}}),
            U("Turbo Carga", 4200, "Habilidade: 5x mais rápido por 10 s.", {{"hab", H("Turbo Carga", "turbo", 45, {{"dur", 10}, {"valor", 0.2}})}}),
            U("Carga Permanente", 35000, "Turbo permanente, 4 de dano.", {{"cad", 0.2}, {"dano", 3}}),
        },
        {
            U("Bumerangues de Longo Alcance", 100, "+33% de alcance.", {{"alcance", 57}, {"dist", 40}}),
            U("Bumerangues Incandescentes", 300, "Estouram chumbo.", {{"dano", 1}, {"dtype", "normal"}}),
            U("Bumerangue Kylie", 1300, "Kylie em linha reta, pierce 18.", {{"pierce", 14}, {"dist", 220}, {"visual", "kylie"}}),
            U("Prensa de M.O.A.B.", 2700, "Kylie pesado empurra dirigíveis a cada 10 s.", {{"novo", A("projetil", {{"cad", 10.0}, {"dano", 1}, {"moab", 4}, {"pierce", 200}, {"vel", 520}, {"dist", 400}, {"so_moab", true}, {"empurra", 60}, {"visual", "kylie"}})}}),
            U("Dominação M.O.A.B.", 50000, "Kylie com 12 de dano, pierce 54 e 2x mais rápido.", {{"dano", 10}, {"pierce", 36}, {"cad", 0.5}}),
        },
    };
    t.desc = "Bumerangues vão e voltam.";
    t.cor = {170, 110, 50};
    return t;
}

static DefTorre t_bomba() {
    DefTorre t("bomba", "Canhão Bomba", 375, "e", 160);
    t.ataques = {A("projetil", {{"cad", 1.5}, {"dano", 1}, {"pierce", 1}, {"vel", 600}, {"dist", 260}, {"visual", "bomba"}, {"raio_proj", 7}, {"splash", 45}, {"sdano", 1}, {"spierce", 22}, {"sdtype", "explosao"}})};
    t.caminhos = {
        {
            U("Bombas Maiores", 250, "Explosão 50% maior, +6 pierce.", {{"splash", 24}, {"spierce", 6}}),
            U("Bombas Pesadas", 650, "+1 de dano e +10 pierce.", {{"sdano", 1}, {"spierce", 10}}),
            U("Bombas Muito Grandes", 1100, "4 de dano, pierce 80, empurra bloons.", {{"splash", 36}, {"spierce", 42}, {"sdano", 2}, {"empurra", 80}}),
            U("Impacto Bloon", 2800, "Atordoa bloons por 1,4 s.", {{"atordoa", 1.4}, {"alcance", 12}}),
            U("Esmaga Bloon", 55000, "24 de dano, estoura tudo e atordoa M.O.A.B.s por 2 s.", {{"sdano", 20}, {"sdtype", "normal"}, {"atordoa", 0.6}, {"moab_atordoa", true}}),
        },
        {
            U("Recarga Rápida", 250, "", {{"cad", 0.75}}),
            U("Lança-Mísseis", 400, "Mísseis rápidos.", {{"cad", 0.733}, {"vel", 1.5}, {"alcance", 16}, {"visual", "missil"}}),
            U("Destruidor de M.O.A.B.", 1000, "+15 de dano em M.O.A.B.", {{"moab", 15}, {"alcance", 20}}),
            U("Assassino de M.O.A.B.", 3450, "+30 em M.O.A.B. Habilidade: 750 no dirigível mais forte.", {{"moab", 15}, {"alcance", 20}, {"hab", H("Míssil Assassino", "dano_forte", 30, {{"valor", 750}, {"n", 1}, {"moab_so", true}})}}),
            U("Eliminador de M.O.A.B.", 26000, "+99 em M.O.A.B. Habilidade: 4.500 a cada 10 s.", {{"moab", 69}, {"hab", H("Míssil Eliminador", "dano_forte", 10, {{"valor", 4500}, {"n", 1}, {"moab_so", true}})}}),
        },
        {
            U("Alcance Extra", 200, "+12 de alcance.", {{"alcance", 48}, {"dist", 40}}),
            U("Bombas de Fragmentação", 300, "Solta 8 fragmentos.", {{"alcance", 8}, {"frag", {{"n", 8}, {"dano", 1}, {"pierce", 1}, {"dtype", "afiado"}, {"visual", "fragmento"}}}}),
            U("Bombas de Cacho", 700, "Soltam mini bombas.", {{"frag", {{"n", 8}, {"dano", 1}, {"pierce", 1}, {"splash", 30}, {"sdano", 1}, {"spierce", 8}, {"visual", "bomba"}}}}),
            U("Cacho Recursivo", 2500, "+1 de dano em todas as explosões; mini bombas recursivas.", {{"sdano", 1}, {"frag", {{"n", 8}, {"dano", 1}, {"pierce", 1}, {"splash", 60}, {"sdano", 2}, {"spierce", 8}, {"visual", "bomba"}}}}),
            U("Blitz de Bombas", 30000, "+3 de dano e ataca mais rápido.", {{"sdano", 3}, {"cad", 0.6}, {"hab", H("Blitz de Bombas", "dano_global", 60, {{"valor", 1000}})}}),
        },
    };
    t.desc = "Bombas explodem em área.";
    t.cor = {60, 60, 70};
    return t;
}

static DefTorre t_tachinha() {
    DefTorre t("tachinha", "Atirador de Tachinhas", 260, "r", 92);
    t.ataques = {A("radial", {{"cad", 1.12}, {"dano", 1}, {"pierce", 1}, {"n", 8}, {"vel", 520}, {"dist", 120}, {"visual", "tachinha"}})};
    t.caminhos = {
        {
            U("Disparo Rápido", 150, "", {{"cad", 0.75}}),
            U("Disparo Mais Rápido", 220, "Atira ainda mais rápido.", {{"cad", 0.75}}),
            U("Tiros Quentes", 600, "Tachinhas quentes: 3 de dano, estouram chumbo.", {{"dtype", "normal"}, {"dano", 2}, {"visual", "fogo"}}),
            U("Anel de Fogo", 3500, "Anel de fogo: 5 de dano em até 30 bloons em volta.", {{"subst", A("aura", {{"cad", 0.315}, {"dano", 5}, {"pierce", 30}, {"dtype", "energia"}, {"visual", "anel_fogo"}})}}),
            U("Anel Infernal", 45500, "Anel mais forte e meteoros de 700 de dano.", {{"dano", 3}, {"moab", 4}, {"pierce", 15}, {"alcance", 46}, {"cad", 0.317}, {"novo", A("projetil", {{"cad", 4.0}, {"dano", 700}, {"pierce", 1}, {"vel", 1500}, {"dist", 900}, {"dtype", "normal"}, {"splash", 60}, {"sdano", 30}, {"spierce", 20}, {"global_", true}, {"visual", "meteoro"}, {"raio_proj", 16}, {"alvo", "forte"}})}}),
        },
        {
            U("Tachinhas de Longo Alcance", 100, "+17% de alcance.", {{"alcance", 16}, {"dist", 20}, {"vel", 1.17}}),
            U("Tachinhas de Super Alcance", 225, "+17% de alcance e pierce 4.", {{"alcance", 16}, {"dist", 15}, {"pierce", 3}}),
            U("Atirador de Lâminas", 550, "Lâminas: pierce 8 e alcance 46.", {{"pierce", 4}, {"alcance", 60}, {"raio_proj", 4}, {"visual", "lamina"}}),
            U("Turbilhão de Lâminas", 2700, "Lâminas com 2 de dano. Habilidade: redemoinho de lâminas.", {{"dano", 1}, {"hab", H("Turbilhão", "turbo", 20, {{"dur", 3}, {"valor", 0.05}})}}),
            U("Super Turbilhão", 15000, "5 de dano, +5 em cerâmica, estoura tudo. Habilidade de 9 s.", {{"dano", 3}, {"cer", 5}, {"dtype", "normal"}, {"hab", H("Super Turbilhão", "turbo", 20, {{"dur", 9}, {"valor", 0.04}})}}),
        },
        {
            U("Mais Tachinhas", 150, "10 tachinhas.", {{"n", 2}}),
            U("Ainda Mais Tachinhas", 150, "12 tachinhas.", {{"n", 2}}),
            U("Pulverizador de Tachinhas", 450, "16 tachinhas, +1 pierce.", {{"n", 4}, {"pierce", 1}}),
            U("Sobrecarga", 3200, "3x mais rápido.", {{"cad", 0.333}}),
            U("Zona das Tachinhas", 20000, "32 tachinhas, mais rápido e mais alcance.", {{"n", 16}, {"cad", 0.6}, {"alcance", 28}, {"moab", 1}}),
        },
    };
    t.desc = "Dispara tachinhas em 8 direções.";
    t.cor = {170, 170, 180};
    return t;
}

static DefTorre t_gelo() {
    DefTorre t("gelo", "Macaco de Gelo", 400, "t", 80);
    t.ataques = {A("aura", {{"cad", 2.4}, {"dano", 1}, {"pierce", 40}, {"dtype", "gelo"}, {"congela", 1.5}, {"visual", "congelar"}})};
    t.caminhos = {
        {
            U("Permafrost", 150, "Bloons ficam lentos depois.", {{"lento", J::array({0.5, 2.5})}}),
            U("Estalo Frio", 350, "Congela camo e chumbo.", {{"camo", true}, {"dtype", "normal"}}),
            U("Estilhaços de Gelo", 1500, "Bloons congelados soltam 3 estilhaços ao estourar.", {{"frag", {{"n", 3}, {"dano", 2}, {"pierce", 3}, {"dtype", "afiado"}, {"visual", "fragmento_gelo"}}}}),
            U("Fragilização", 2300, "Bloons recebem +1 de dano de tudo.", {{"fragiliza", 1}}),
            U("Super Frágil", 28000, "Bloons recebem +4 de dano de tudo; ataca mais rápido.", {{"fragiliza", 3}, {"cad", 0.65}}),
        },
        {
            U("Congelamento Melhor", 200, "Ataca mais rápido e congela por 1,75 s.", {{"cad", 0.75}, {"congela", 1.75}}),
            U("Congelamento Profundo", 300, "Pierce 45 e congela por 2,2 s.", {{"pierce", 5}, {"congela", 2.2}}),
            U("Vento Ártico", 2750, "Aura que desacelera bloons em 40%.", {{"novo", A("aura", {{"cad", 0.2}, {"dano", 0}, {"pierce", 999}, {"lento", J::array({0.6, 0.3})}, {"dtype", "normal"}, {"visual", "vento"}})}}),
            U("Nevasca", 4750, "+10 de alcance. Habilidade: congela todos os bloons por 6 s.", {{"alcance", 40}, {"hab", H("Nevasca", "congelar_global", 30, {{"dur", 6}})}}),
            U("Zero Absoluto", 21000, "+10 de alcance e pierce 300.", {{"alcance", 40}, {"pierce", 255}, {"hab", H("Zero Absoluto", "congelar_global", 20, {{"dur", 10}, {"moab", true}})}}),
        },
        {
            U("Raio Maior", 150, "+7 de alcance.", {{"alcance", 28}}),
            U("Recongelar", 200, "Pode recongelar bloons já congelados.", J::object()),
            U("Canhão Criogênico", 1900, "Canhão criogênico: 2x mais rápido, alcance 46.", {{"alcance", 104}, {"subst", A("projetil", {{"cad", 1.2}, {"dano", 1}, {"pierce", 1}, {"vel", 700}, {"dist", 300}, {"dtype", "gelo"}, {"congela", 1.5}, {"splash", 80}, {"sdano", 1}, {"spierce", 40}, {"sdtype", "gelo"}, {"visual", "gelo_bola"}, {"raio_proj", 8}})}}),
            U("Pingentes", 2750, "Pingentes: mais rápido, +1 de dano, +8 em M.O.A.B.", {{"cad", 0.625}, {"sdano", 1}, {"moab", 8}}),
            U("Empalar com Pingentes", 30000, "50 de dano em M.O.A.B. e congela dirigíveis.", {{"moab", 40}, {"moab_congela", true}, {"sdtype", "normal"}}),
        },
    };
    t.desc = "Congela bloons em volta.";
    t.cor = {150, 210, 240};
    return t;
}

static DefTorre t_cola() {
    DefTorre t("cola", "Atirador de Cola", 225, "y", 184);
    t.ataques = {A("projetil", {{"cad", 1.0}, {"dano", 0}, {"pierce", 1}, {"vel", 700}, {"dist", 260}, {"cola", J::array({0.5, 11, 0})}, {"visual", "cola"}})};
    t.caminhos = {
        {
            U("Cola Encharcada", 200, "Cola passa pelas camadas.", J::object()),
            U("Cola Corrosiva", 300, "Cola corrói bloons.", {{"cola", J::array({0.5, 11, 0.5})}}),
            U("Dissolvedor de Bloons", 2000, "Corrosão a cada 0,5 s; atira 2x mais rápido.", {{"cola", J::array({0.5, 11, 2})}, {"pierce", 1}, {"cad", 0.5}}),
            U("Liquefator de Bloons", 5000, "Corrosão a cada 0,1 s.", {{"cola", J::array({0.5, 11, 10})}}),
            U("Solucionador de Bloons", 22500, "2 globos por tiro, pierce 4, 2x mais rápido.", {{"pierce", 2}, {"n", 1}, {"spread", 10}, {"cad", 0.5}}),
        },
        {
            U("Globos Maiores", 100, "+1 pierce.", {{"pierce", 1}, {"raio_proj", 1}}),
            U("Respingo de Cola", 970, "Respingo de cola em até 5 bloons.", {{"splash", 48}, {"spierce", 4}}),
            U("Mangueira de Cola", 1950, "3x mais rápido e +12 de alcance.", {{"cad", 0.34}, {"alcance", 48}}),
            U("Ataque de Cola", 4000, "Habilidade: cola todos os bloons.", {{"hab", H("Ataque de Cola", "lentidao", 40, {{"dur", 11}, {"valor", 0.5}})}}),
            U("Tempestade de Cola", 16000, "Habilidade: cola todos os bloons por 20 s.", {{"hab", H("Tempestade de Cola", "lentidao", 30, {{"dur", 20}, {"valor", 0.5}})}}),
        },
        {
            U("Cola Mais Grudenta", 280, "Cola dura 24 s.", {{"cola", J::array({0.5, 24, 0})}}),
            U("Cola Mais Forte", 400, "Bloons colados ficam 75% mais lentos.", {{"cola", J::array({0.25, 24, 0})}}),
            U("Cola de M.O.A.B.", 3600, "Cola dirigíveis.", {{"moab_cola", true}}),
            U("Cola Implacável", 4000, "Bloons colados atordoam os vizinhos ao estourar.", {{"atordoa", 1.0}}),
            U("Super Cola", 24000, "Para bloons comuns e desacelera muito os dirigíveis.", {{"pierce", 5}, {"cola", J::array({0.1, 24, 0})}, {"dano", 1}, {"moab", 30}, {"moab_cola", true}}),
        },
    };
    t.desc = "Cola desacelera bloons.";
    t.cor = {150, 200, 60};
    return t;
}

static DefTorre t_sniper() {
    DefTorre t("sniper", "Macaco Atirador", 350, "z", 9999);
    t.ataques = {A("hitscan", {{"cad", 1.59}, {"dano", 2}, {"pierce", 1}, {"dtype", "afiado"}, {"global_", true}, {"visual", "bala"}})};
    t.caminhos = {
        {
            U("Jaqueta Metálica", 350, "Estoura chumbo, 4 de dano.", {{"dtype", "normal"}, {"dano", 2}}),
            U("Calibre Grosso", 1300, "", {{"dano", 3}}),
            U("Precisão Mortal", 2200, "20 de dano, +50 em cerâmica.", {{"dano", 13}, {"cer", 50}}),
            U("Mutilar M.O.A.B.", 6300, "30 de dano e atordoa dirigíveis.", {{"atordoa", 3.0}, {"moab_atordoa", true}, {"dano", 10}}),
            U("Aleijar M.O.A.B.", 32000, "140 de dano, explosão de 28 e +5 de dano em dirigíveis atingidos.", {{"atordoa", 7.0}, {"fragiliza", 5}, {"dano", 110}, {"splash", 32}, {"sdano", 28}, {"spierce", 10}, {"sdtype", "normal"}}),
        },
        {
            U("Óculos de Visão Noturna", 250, "Detecta camo.", {{"camo", true}}),
            U("Tiro de Estilhaços", 450, "5 estilhaços por tiro.", {{"frag", {{"n", 5}, {"dano", 1}, {"pierce", 2}, {"dtype", "afiado"}, {"visual", "fragmento"}}}}),
            U("Bala Ricochete", 2100, "A bala quica 2 vezes.", {{"quica", 2}}),
            U("Lançamento de Suprimentos", 7600, "Quica 4 vezes. Habilidade: caixa de $1.100.", {{"quica", 2}, {"dtype", "normal"}, {"frag", {{"n", 5}, {"dano", 1}, {"pierce", 5}, {"dtype", "afiado"}, {"visual", "fragmento"}}}, {"hab", H("Suprimentos", "dinheiro", 60, {{"valor", 1100}})}}),
            U("Atirador de Elite", 12000, "Muito mais rápido; os outros Snipers do mapa atacam 33% mais rápido. Habilidade: caixa de $3.000.", {{"cad", 0.4}, {"buffs", {{"cad", 0.75}, {"escopo", "sniper"}, {"global_", true}, {"sem_si", true}}}, {"hab", H("Suprimentos de Elite", "dinheiro", 90, {{"valor", 3000}})}}),
        },
        {
            U("Disparo Rápido", 450, "", {{"cad", 0.7}}),
            U("Disparo Mais Rápido", 450, "", {{"cad", 0.7}}),
            U("Semiautomático", 2700, "3x mais rápido.", {{"cad", 0.333}}),
            U("Rifle Automático", 4100, "2x mais rápido, +2 em M.O.A.B.", {{"cad", 0.5}, {"moab", 2}}),
            U("Defensor de Elite", 14900, "2x mais rápido, +4 em M.O.A.B.", {{"cad", 0.5}, {"moab", 2}}),
        },
    };
    t.categoria = "militar";
    t.desc = "Alcance infinito.";
    t.cor = {80, 110, 60};
    return t;
}

static DefTorre t_submarino() {
    DefTorre t("submarino", "Submarino Macaco", 325, "x", 168);
    t.ataques = {A("projetil", {{"cad", 0.75}, {"dano", 1}, {"pierce", 2}, {"vel", 800}, {"dist", 260}, {"busca", true}, {"visual", "dardo"}})};
    t.caminhos = {
        {
            U("Alcance Maior", 130, "+10 de alcance.", {{"alcance", 40}}),
            U("Inteligência Avançada", 500, "Ataca qualquer bloon no mapa.", {{"global_", true}}),
            U("Submergir e Apoiar", 700, "Submerso: pulso que remove camo.", {{"camo", true}, {"novo", A("aura", {{"cad", 1.35}, {"dano", 0}, {"pierce", 999}, {"retira_camo", true}, {"dtype", "normal"}, {"visual", "nenhum"}})}}),
            U("Reator de Bloontônio", 2400, "Pulso radioativo de 1 de dano (pierce 50) a cada 0,28 s.", {{"novo", A("aura", {{"cad", 0.28}, {"dano", 1}, {"pierce", 50}, {"dtype", "energia"}, {"visual", "radiacao"}})}}),
            U("Energizador", 28000, "Dardos com 5 de dano.", {{"dano", 3}}),
        },
        {
            U("Dardos Farpados", 450, "+3 pierce.", {{"pierce", 3}}),
            U("Dardos Aquecidos", 300, "Dardos estouram chumbo.", {{"dtype", "normal"}}),
            U("Míssil Balístico", 1350, "Mísseis: 3 de dano, +3 em cerâmica e M.O.A.B.", {{"alcance", 32}, {"novo", A("projetil", {{"cad", 1.0}, {"dano", 3}, {"pierce", 1}, {"vel", 900}, {"dist", 2000}, {"busca", true}, {"moab", 3}, {"cer", 3}, {"splash", 40}, {"sdano", 3}, {"spierce", 40}, {"sdtype", "normal"}, {"visual", "missil"}})}}),
            U("Capacidade de Primeiro Ataque", 13000, "Habilidade: míssil de 10.000 no bloon mais forte.", {{"hab", H("Primeiro Ataque", "dano_forte", 45, {{"valor", 10000}, {"n", 1}, {"splash", 260}, {"sdano", 350}})}}),
            U("Ataque Preventivo", 29000, "Míssil de 750 em cada dirigível; habilidade mais rápida.", {{"novo", A("projetil", {{"cad", 2.0}, {"dano", 750}, {"pierce", 1}, {"vel", 1500}, {"dist", 3000}, {"busca", true}, {"global_", true}, {"dtype", "normal"}, {"alvo", "forte"}, {"so_moab", true}, {"visual", "missil"}, {"raio_proj", 10}})}, {"hab", H("Primeiro Ataque", "dano_forte", 30, {{"valor", 10000}, {"n", 1}, {"splash", 260}, {"sdano", 350}})}}),
        },
        {
            U("Canhões Gêmeos", 450, "Dardos 2x mais rápidos.", {{"cad", 0.5}}),
            U("Dardos de Explosão Aérea", 1000, "Dardos se dividem em 3 ao acertar.", {{"frag", {{"n", 3}, {"dano", 1}, {"pierce", 2}, {"dtype", "afiado"}, {"visual", "dardo"}}}}),
            U("Canhões Triplos", 1100, "Ainda mais rápido.", {{"cad", 0.67}}),
            U("Dardos Perfurantes", 2500, "2 de dano, +2 em M.O.A.B. e +1 em fortificado.", {{"dano", 1}, {"moab", 2}, {"fort", 1}}),
            U("Comandante Submarino", 25000, "Dobra o dano e dá +4 pierce.", {{"dano", 2}, {"pierce", 4}, {"moab", 2}}),
        },
    };
    t.categoria = "militar";
    t.agua = true;
    t.desc = "Só na água.";
    t.cor = {230, 200, 40};
    return t;
}

static DefTorre t_bucaneiro() {
    DefTorre t("bucaneiro", "Macaco Bucaneiro", 400, "c", 240);
    t.ataques = {A("projetil", {{"cad", 1.0}, {"dano", 1}, {"pierce", 4}, {"n", 2}, {"spread", 360}, {"vel", 700}, {"dist", 300}, {"visual", "dardo"}})};
    t.caminhos = {
        {
            U("Disparo Rápido", 275, "Ataca 33% mais rápido.", {{"a", "todos"}, {"cad", 0.75}}),
            U("Tiro Duplo", 425, "", {{"n", 2}}),
            U("Destróier", 3350, "Ataca 5x mais rápido.", {{"a", "todos"}, {"cad", 0.2}}),
            U("Porta-Aviões", 8000, "Aviões atacam em todo o mapa.", {{"novo", A("radial", {{"cad", 0.6}, {"dano", 2}, {"pierce", 5}, {"n", 4}, {"vel", 700}, {"dist", 300}, {"visual", "aviaozinho"}, {"global_", true}})}}),
            U("Nau Capitânia", 26000, "Torres na água e Ases em todo o mapa atacam 25% mais rápido.", {{"buffs", {{"cad", 0.8}, {"escopo", "agua|as"}, {"global_", true}}}}),
        },
        {
            U("Tiro de Uva", 550, "Dispara 5 uvas em leque.", {{"novo", A("projetil", {{"cad", 1.35}, {"dano", 1}, {"pierce", 1}, {"n", 5}, {"spread", 90}, {"vel", 700}, {"dist", 260}, {"visual", "uva"}})}}),
            U("Tiro Quente", 500, "Uvas de fogo estouram chumbo e queimam.", {{"a", "uva"}, {"dtype", "normal"}, {"queima", J::array({1.33, 3.1})}}),
            U("Navio Canhão", 900, "Bombas explosivas; uvas com 3 de dano.", J::array({{{"subst", A("projetil", {{"cad", 1.3}, {"dano", 1}, {"pierce", 1}, {"vel", 650}, {"dist", 320}, {"splash", 40}, {"sdano", 2}, {"spierce", 20}, {"sdtype", "explosao"}, {"frag", {{"n", 8}, {"dano", 1}, {"pierce", 1}, {"dtype", "afiado"}, {"visual", "fragmento"}}}, {"visual", "bala_canhao"}, {"raio_proj", 8}})}}, {{"a", "uva"}, {"dano", 2}}})),
            U("Macacos Piratas", 3900, "3 bombas por tiro, +5 em M.O.A.B. Habilidade: arpão derruba um dirigível.", J::array({{{"n", 2}, {"spread", 20}, {"sdano", 1}, {"moab", 5}}, {{"a", "uva"}, {"dano", 1}, {"cer", 2}}, {{"hab", H("Arpão", "dano_forte", 60, {{"valor", 4000}, {"n", 1}, {"moab_so", true}})}}})),
            U("Senhor Pirata", 29000, "35% mais rápido; uvas com 8 de dano.", J::array({{{"a", "todos"}, {"cad", 0.65}}, {{"a", "uva"}, {"dano", 4}, {"cer", 2}}, {{"moab", 3}}, {{"hab", H("Arpões do Senhor Pirata", "dano_forte", 60, {{"valor", 20000}, {"n", 3}, {"moab_so", true}})}}})),
        },
        {
            U("Longo Alcance", 200, "+11 de alcance, +2 pierce.", {{"alcance", 44}, {"pierce", 2}, {"vel", 1.25}}),
            U("Ninho do Corvo", 350, "Detecta camo.", {{"camo", true}}),
            U("Navio Mercante", 2400, "Gera $200 por rodada.", {{"novo", A("renda", {{"valor", 200}, {"visual", "moeda"}})}}),
            U("Comércio Favorecido", 5500, "Gera $500 por rodada.", {{"a", "renda"}, {"valor", 300}}),
            U("Império Comercial", 23000, "Gera $800 por rodada.", {{"a", "renda"}, {"valor", 300}}),
        },
    };
    t.categoria = "militar";
    t.agua = true;
    t.raio = 26;
    t.desc = "Só na água.";
    t.cor = {120, 80, 40};
    return t;
}

static DefTorre t_as() {
    DefTorre t("as", "Macaco Ás", 800, "v", 200);
    t.ataques = {A("radial", {{"cad", 1.68}, {"dano", 1}, {"pierce", 5}, {"n", 8}, {"vel", 700}, {"dist", 240}, {"visual", "dardo"}})};
    t.caminhos = {
        {
            U("Tiro Rápido", 450, "Ataca 33% mais rápido.", {{"a", "todos"}, {"cad", 0.75}}),
            U("Muito Mais Dardos", 550, "", {{"n", 4}}),
            U("Avião de Caça", 1000, "Mísseis anti M.O.A.B. (18 de dano).", {{"novo", A("projetil", {{"cad", 3.0}, {"dano", 18}, {"pierce", 4}, {"n", 2}, {"spread", 20}, {"vel", 900}, {"dist", 2000}, {"busca", true}, {"global_", true}, {"so_moab", true}, {"alvo", "forte"}, {"visual", "missil"}})}}),
            U("Operação: Tempestade de Dardos", 3300, "2x mais rápido, 16 dardos; mísseis com 24 de dano.", J::array({{{"a", "todos"}, {"cad", 0.5}}, {{"n", 4}}, {{"a", "missil"}, {"dano", 6}}})),
            U("Retalhador Celeste", 42500, "32 dardos com 3 de dano, 2x mais rápido.", {{"cad", 0.5}, {"n", 16}, {"dano", 2}, {"cer", 2}, {"pierce", 3}, {"dtype", "normal"}}),
        },
        {
            U("Abacaxi Explosivo", 200, "Solta abacaxis explosivos.", {{"novo", A("queda", {{"cad", 1.6}, {"splash", 130}, {"sdano", 1}, {"spierce", 20}, {"fusivel", 2.0}, {"visual", "abacaxi"}})}}),
            U("Avião Espião", 350, "Detecta camo.", {{"camo", true}}),
            U("Ás Bombardeiro", 900, "Bombardeio na trilha: bombas de 3 de dano.", {{"a", "queda"}, {"sdano", 2}, {"cad", 0.25}, {"na_trilha", true}}),
            U("Marco Zero", 16000, "Habilidade: bomba de 700 em toda a tela.", {{"hab", H("Marco Zero", "dano_global", 35, {{"valor", 700}})}}),
            U("Tsar Bomba", 26000, "Habilidade: bomba de 3.000 e atordoa 8 s.", {{"hab", H("Tsar Bomba", "dano_global", 35, {{"valor", 3000}, {"atordoa", 8}})}}),
        },
        {
            U("Dardos Mais Afiados", 500, "Pierce 8; bombas +12 pierce.", J::array({{{"pierce", 3}}, {{"a", "queda"}, {"spierce", 12}}})),
            U("Rota Centralizada", 550, "Voa em volta de um ponto escolhido.", J::object()),
            U("Mira Infalível", 2550, "Dardos teleguiados.", {{"busca", true}, {"dist", 200}}),
            U("Espectro", 23400, "Rajada de dardos e bombas teleguiados.", J::array({{{"novo", A("projetil", {{"cad", 0.12}, {"dano", 6}, {"pierce", 4}, {"vel", 900}, {"dist", 1200}, {"busca", true}, {"global_", true}, {"dtype", "normal"}, {"visual", "dardo"}})}}, {{"novo", A("projetil", {{"cad", 0.12}, {"dano", 3}, {"cer", 4}, {"pierce", 1}, {"vel", 900}, {"dist", 1200}, {"busca", true}, {"global_", true}, {"splash", 60}, {"sdano", 3}, {"spierce", 20}, {"sdtype", "normal"}, {"visual", "bomba"}})}}})),
            U("Fortaleza Voadora", 90000, "3 projéteis por disparo, 50% mais rápido, +14 em M.O.A.B.", {{"a", "projetil"}, {"n", 2}, {"spread", 30}, {"cad", 0.667}, {"moab", 14}}),
        },
    };
    t.categoria = "militar";
    t.mov = Mov::ORBITA;
    t.raio = 24;
    t.desc = "Voa em círculos atirando.";
    t.cor = {230, 200, 40};
    return t;
}

static DefTorre t_heli() {
    DefTorre t("heli", "Piloto de Helicóptero", 1500, "b", 170);
    t.ataques = {A("projetil", {{"cad", 0.57}, {"dano", 1}, {"pierce", 3}, {"n", 2}, {"spread", 10}, {"vel", 850}, {"dist", 260}, {"visual", "dardo"}})};
    t.caminhos = {
        {
            U("Dardos Quádruplos", 800, "4 dardos por vez.", {{"n", 2}, {"spread", 10}}),
            U("Perseguição", 500, "Persegue os bloons.", {{"persegue", true}}),
            U("Hélices Navalha", 1450, "Hélices estouram bloons em volta (2 de dano).", {{"novo", A("aura", {{"cad", 0.45}, {"dano", 2}, {"pierce", 10}, {"dtype", "normal"}, {"raio_aura", 130}, {"visual", "nenhum"}})}}),
            U("Apache Dardeiro", 20000, "Metralhadora e mísseis.", {{"novo", A("projetil", {{"cad", 0.05}, {"dano", 1}, {"pierce", 7}, {"vel", 1000}, {"dist", 400}, {"visual", "dardo"}})}}),
            U("Apache Prime", 45000, "Lasers: 6 de dano e pierce 23.", {{"dano", 5}, {"pierce", 20}, {"dtype", "normal"}, {"visual", "plasma"}}),
        },
        {
            U("Jatos Maiores", 300, "Voa mais rápido.", J::object()),
            U("IFR", 600, "Detecta camo.", {{"camo", true}}),
            U("Corrente Descendente", 3500, "Joga bloons de volta para a entrada.", {{"novo", A("aura", {{"cad", 0.18}, {"dano", 0}, {"pierce", 2}, {"empurra", 150}, {"raio_aura", 80}, {"visual", "vento"}})}}),
            U("Chinook de Apoio", 9500, "Dardos com 2 de dano. Habilidade: caixa de dinheiro.", {{"dano", 1}, {"raio_proj", 3}, {"hab", H("Entrega", "dinheiro", 60, {{"valor", 1000}})}}),
            U("Operações Especiais", 30000, "Habilidade: fuzileiro de elite.", {{"hab", H("Fuzileiro", "invocar", 60, {{"dur", 20}, {"base", "sniper"}, {"nivel", J::array({4, 0, 3})}})}}),
        },
        {
            U("Dardos Rápidos", 250, "+30% de alcance.", {{"alcance", 50}, {"vel", 1.3}}),
            U("Disparo Rápido", 350, "Todos os ataques 25% mais rápidos.", {{"a", "todos"}, {"cad", 0.8}}),
            U("Empurrão de M.O.A.B.", 3400, "Empurra dirigíveis e dispara mísseis.", J::array({{{"novo", A("aura", {{"cad", 0.5}, {"dano", 0}, {"pierce", 1}, {"lento", J::array({0.5, 0.5})}, {"moab_lento", true}, {"raio_aura", 90}, {"visual", "nenhum"}})}}, {{"novo", A("projetil", {{"cad", 3.0}, {"dano", 2}, {"cer", 2}, {"moab", 5}, {"pierce", 10}, {"vel", 900}, {"dist", 900}, {"busca", true}, {"visual", "missil"}})}}})),
            U("Defesa Comanche", 8500, "Dardos com 2 de dano.", {{"dano", 1}}),
            U("Comandante Comanche", 35000, "Dardos com 4 de dano.", {{"dano", 2}}),
        },
    };
    t.categoria = "militar";
    t.mov = Mov::HELI;
    t.raio = 26;
    t.desc = "Helicóptero que se move.";
    t.cor = {80, 120, 70};
    return t;
}

static DefTorre t_morteiro() {
    DefTorre t("morteiro", "Macaco Morteiro", 600, "n", 9999);
    t.ataques = {A("morteiro", {{"cad", 2.0}, {"dano", 1}, {"splash", 80}, {"sdano", 2}, {"spierce", 25}, {"sdtype", "explosao"}, {"impreciso", 72}, {"global_", true}, {"visual", "bala_canhao"}})};
    t.caminhos = {
        {
            U("Explosão Maior", 300, "Explosão 40% maior, +20 pierce.", {{"splash", 32}, {"spierce", 20}}),
            U("Destruidor de Bloons", 500, "3 de dano.", {{"sdano", 1}}),
            U("Choque de Projéteis", 825, "Explosão maior que atordoa.", {{"splash", 40}, {"atordoa", 0.5}}),
            U("A Grande", 7000, "10 de dano, pierce 85, explosão enorme.", {{"splash", 80}, {"spierce", 40}, {"sdano", 7}}),
            U("A Maior de Todas", 36000, "25 de dano, +30 em cerâmica e M.O.A.B., pierce 200.", {{"splash", 64}, {"spierce", 115}, {"sdano", 15}, {"cer", 30}, {"moab", 30}}),
        },
        {
            U("Recarga Rápida", 400, "", {{"cad", 0.75}}),
            U("Recarga Veloz", 500, "Recarrega ainda mais rápido.", {{"cad", 0.72}}),
            U("Projéteis Pesados", 900, "Estoura tudo; bônus em chumbo, M.O.A.B., fortificado e cerâmica.", {{"cad", 0.75}, {"sdtype", "normal"}, {"moab", 1}, {"fort", 1}, {"cer", 3}}),
            U("Bateria de Artilharia", 6500, "3x mais rápido. Habilidade: 4x mais rápido por 8 s.", {{"cad", 0.333}, {"hab", H("Bombardeio", "turbo", 60, {{"dur", 8}, {"valor", 0.25}})}}),
            U("Choque e Pavor", 38000, "Habilidade: atordoa tudo e causa 20 de dano por segundo por 8 s.", {{"hab", H("Choque e Pavor", "dano_global", 60, {{"valor", 160}, {"atordoa", 8}})}}),
        },
        {
            U("Precisão Aumentada", 200, "Mais preciso e detecta camo.", {{"impreciso", 0.444}, {"camo", true}}),
            U("Coisas Queimando", 400, "Deixa fogo.", {{"queima", J::array({0.8, 3.85})}}),
            U("Sinalizador", 1100, "Remove camo, inclusive de D.D.T.", {{"retira_camo", true}, {"camo", true}}),
            U("Projéteis Estilhaçantes", 9500, "Remove camo e regeneração; queima mais forte.", {{"retira_regen", true}, {"queima", J::array({4, 3.85})}}),
            U("Bloonflagração", 40000, "Fogo intenso.", {{"queima", J::array({6.67, 4.5})}}),
        },
    };
    t.categoria = "militar";
    t.desc = "Atira no local escolhido.";
    t.cor = {90, 110, 70};
    return t;
}

static DefTorre t_dartling() {
    DefTorre t("dartling", "Atirador Dartling", 850, "m", 9999);
    t.ataques = {A("projetil", {{"cad", 0.2}, {"dano", 1}, {"pierce", 1}, {"vel", 1200}, {"dist", 1400}, {"spread", 23}, {"global_", true}, {"visual", "dardo"}})};
    t.caminhos = {
        {
            U("Disparo Focado", 300, "Espalhamento 60% menor.", {{"spread", 9}}),
            U("Choque Laser", 900, "Choque: 1 de dano extra depois de 1 s.", {{"queima", J::array({1, 1})}}),
            U("Canhão Laser", 3000, "Laser: 2 de dano, pierce 6, +2 em M.O.A.B.", {{"dtype", "energia"}, {"dano", 1}, {"pierce", 5}, {"moab", 2}, {"visual", "laser"}}),
            U("Acelerador de Plasma", 11750, "Raio contínuo: pierce 50, +10 em M.O.A.B.", {{"subst", A("hitscan", {{"cad", 0.2}, {"dano", 2}, {"pierce", 50}, {"moab", 10}, {"dtype", "normal"}, {"global_", true}, {"visual", "raio_plasma"}, {"linha", true}, {"queima", J::array({1, 5})}})}}),
            U("Raio da Perdição", 75000, "Raio de 30 de dano, pierce 999.", {{"dano", 28}, {"pierce", 949}, {"queima", J::array({20, 30})}}),
        },
        {
            U("Mira Avançada", 250, "Detecta camo.", {{"camo", true}}),
            U("Giro de Cano Rápido", 950, "Gira 50% mais rápido.", {{"cad", 0.67}}),
            U("Cápsulas de Foguete Hidra", 4500, "Mísseis que explodem em qualquer bloon.", {{"splash", 32}, {"sdano", 2}, {"spierce", 6}, {"sdtype", "normal"}, {"visual", "missil"}}),
            U("Tempestade de Foguetes", 5000, "+2 pierce. Habilidade: chuva de foguetes.", {{"spierce", 2}, {"pierce", 2}, {"hab", H("Tempestade de Foguetes", "turbo", 40, {{"dur", 8}, {"valor", 0.2}})}}),
            U("M.A.D.", 65000, "Mega mísseis: +450 em M.O.A.B.", {{"cad", 3.0}, {"sdano", 1}, {"moab", 450}}),
        },
        {
            U("Giro Mais Rápido", 150, "Gira mais rápido.", J::object()),
            U("Dardos Poderosos", 1200, "+2 pierce e projéteis mais rápidos.", {{"pierce", 2}, {"vel", 1.3}}),
            U("Chumbinho", 3000, "Chumbinho: 6 projéteis de 3 de dano.", {{"n", 5}, {"dano", 2}, {"pierce", 3}, {"spread", 30}, {"dist", -1150}, {"cad", 6.0}, {"empurra", 20}}),
            U("Sistema de Negação de Área", 12000, "4 canos: 4x mais rápido.", {{"cad", 0.25}}),
            U("Zona de Exclusão Bloon", 58000, "6 canos: 12 projéteis de 6 de dano.", {{"cad", 0.667}, {"pierce", 2}, {"dano", 3}, {"n", 6}}),
        },
    };
    t.categoria = "militar";
    t.desc = "Metralhadora de dardos.";
    t.cor = {70, 90, 110};
    return t;
}

static DefTorre t_mago() {
    DefTorre t("mago", "Macaco Mago", 250, "a", 160);
    t.ataques = {A("projetil", {{"cad", 1.1}, {"dano", 1}, {"pierce", 3}, {"dtype", "energia"}, {"vel", 700}, {"dist", 240}, {"visual", "magia"}})};
    t.caminhos = {
        {
            U("Magia Guiada", 175, "Magia teleguiada.", {{"busca", true}, {"dist", 240}}),
            U("Explosão Arcana", 450, "2 de dano.", {{"dano", 1}, {"raio_proj", 2}}),
            U("Maestria Arcana", 1450, "+50% de alcance, 2x mais rápido, +4 pierce, +1 de dano.", {{"alcance", 80}, {"cad", 0.5}, {"pierce", 4}, {"dano", 1}}),
            U("Espinho Arcano", 10000, "2x mais rápido, 6 de dano, +10 em M.O.A.B., estoura chumbo.", {{"cad", 0.5}, {"dano", 3}, {"moab", 10}, {"dtype", "normal"}}),
            U("Arquimago", 32000, "Raio de 8 de dano, 2x mais rápido; ganha sopro de fogo e cintilar.", J::array({{{"dano", 2}, {"moab", 9}, {"pierce", 4}, {"cad", 0.5}}, {{"novo", A("aura", {{"cad", 2.0}, {"dano", 0}, {"pierce", 500}, {"retira_camo", true}, {"raio_aura", 300}, {"dtype", "normal"}, {"visual", "nenhum"}})}}, {{"novo", A("projetil", {{"cad", 0.135}, {"dano", 2}, {"pierce", 3}, {"vel", 500}, {"dist", 170}, {"dtype", "energia"}, {"queima", J::array({0.67, 3.1})}, {"visual", "fogo"}})}}})),
        },
        {
            U("Bola de Fogo", 300, "Lança bolas de fogo.", {{"novo", A("projetil", {{"cad", 2.2}, {"dano", 1}, {"pierce", 1}, {"vel", 600}, {"dist", 260}, {"dtype", "energia"}, {"splash", 30}, {"sdano", 1}, {"spierce", 12}, {"sdtype", "energia"}, {"visual", "fogo"}, {"raio_proj", 8}})}}),
            U("Muralha de Fogo", 800, "Muralha de fogo na trilha.", {{"novo", A("pilha", {{"cad", 6.5}, {"dano", 1}, {"pilha_pierce", 30}, {"pilha_vida", 4.5}, {"dtype", "energia"}, {"visual", "chamas"}})}}),
            U("Sopro do Dragão", 3300, "Lança-chamas: 2 de dano a cada 0,135 s.", {{"novo", A("projetil", {{"cad", 0.135}, {"dano", 2}, {"pierce", 3}, {"vel", 500}, {"dist", 170}, {"dtype", "energia"}, {"queima", J::array({0.67, 3.1})}, {"visual", "fogo"}})}}),
            U("Invocar Fênix", 6000, "Habilidade: fênix por 20 s.", {{"hab", H("Fênix", "invocar", 45, {{"dur", 20}, {"base", "fenix"}})}}),
            U("Lorde Fênix", 50000, "Fênix permanente.", {{"novo", A("projetil", {{"cad", 0.1}, {"dano", 5}, {"pierce", 8}, {"vel", 700}, {"dist", 600}, {"global_", true}, {"dtype", "energia"}, {"visual", "fogo"}})}}),
        },
        {
            U("Magia Intensa", 300, "+5 pierce e projéteis 2x mais rápidos.", {{"pierce", 5}, {"vel", 2.0}}),
            U("Sentido Macaco", 300, "+10 de alcance e detecta camo.", {{"alcance", 40}, {"camo", true}}),
            U("Cintilar", 1500, "Remove camo em volta.", {{"alcance", 40}, {"novo", A("aura", {{"cad", 2.0}, {"dano", 0}, {"pierce", 500}, {"retira_camo", true}, {"raio_aura", 300}, {"dtype", "normal"}, {"visual", "nenhum"}})}}),
            U("Necromante", 2800, "Bloons mortos voltam como aliados.", {{"novo", A("pilha", {{"cad", 2.0}, {"dano", 2}, {"pilha_pierce", 8}, {"pilha_vida", 6}, {"dtype", "normal"}, {"visual", "zumbi"}})}}),
            U("Príncipe das Trevas", 26500, "4x mais rápido e mais alcance; zumbis mais fortes.", J::array({{{"cad", 0.25}, {"alcance", 80}}, {{"a", "zumbi"}, {"dano", 3}}})),
        },
    };
    t.categoria = "magica";
    t.desc = "Magia poderosa.";
    t.cor = {120, 70, 170};
    return t;
}

static DefTorre t_super() {
    DefTorre t("super", "Super Macaco", 2500, "s", 200);
    t.ataques = {A("projetil", {{"cad", 0.045}, {"dano", 1}, {"pierce", 1}, {"vel", 1100}, {"dist", 280}, {"visual", "dardo"}})};
    t.caminhos = {
        {
            U("Rajadas Laser", 2000, "Laser: +1 pierce.", {{"dtype", "energia"}, {"pierce", 1}, {"raio_proj", 3}, {"visual", "laser"}}),
            U("Rajadas de Plasma", 2500, "Plasma: 50% mais rápido e estoura chumbo.", {{"dtype", "normal"}, {"cad", 0.667}, {"raio_proj", 1}, {"visual", "plasma"}}),
            U("Avatar do Sol", 20000, "3 raios de sol por tiro, +4 pierce.", {{"n", 2}, {"spread", 30}, {"pierce", 4}, {"vel", 2.0}, {"visual", "sol"}}),
            U("Templo do Sol", 100000, "Raio de sol único: 5 de dano, pierce 20.", {{"n", -2}, {"spread", 0}, {"dano", 4}, {"pierce", 14}, {"cad", 1.67}, {"alcance", 60}}),
            U("Verdadeiro Deus Sol", 500000, "Raio de 15 de dano.", {{"dano", 10}}),
        },
        {
            U("Super Alcance", 1500, "+10 de alcance e +1 pierce.", {{"alcance", 40}, {"pierce", 1}}),
            U("Alcance Épico", 1900, "+12 de alcance, +2 pierce.", {{"alcance", 48}, {"pierce", 2}, {"vel", 1.4}}),
            U("Robô Macaco", 7500, "Atira com os 2 braços; crítico de +9 de dano a cada 15 a 20 tiros.", {{"n", 1}, {"spread", 12}, {"pierce", 1}, {"crit_cada", 15}, {"crit_max", 20}, {"crit_mais", 9}}),
            U("Terror Tecnológico", 25000, "Habilidade: aniquilação (2.600 de dano).", {{"pierce", 2}, {"hab", H("Aniquilação", "dano_global", 45, {{"valor", 2600}})}}),
            U("O Anti-Bloon", 70000, "5 de dano, pierce 12. Habilidade de 10.400.", {{"dano", 4}, {"pierce", 5}, {"alcance", 40}, {"hab", H("Erradicação", "dano_global", 30, {{"valor", 10400}})}}),
        },
        {
            U("Repulsão", 3000, "Empurra os bloons.", {{"empurra", 20}}),
            U("Ultravisão", 1200, "Detecta camo.", {{"camo", true}, {"alcance", 12}}),
            U("Cavaleiro das Trevas", 5600, "Lâminas: pierce 4, +2 em M.O.A.B.", {{"moab", 2}, {"pierce", 3}, {"raio_proj", 2}, {"visual", "escuro"}}),
            U("Campeão das Trevas", 55555, "2x mais rápido, 2 de dano, estoura tudo.", {{"alcance", 16}, {"cad", 0.5}, {"dano", 1}, {"moab", 1}, {"dtype", "normal"}}),
            U("Lenda da Noite", 165650, "Buraco negro que evita vazamentos por 8 s.", J::object()),
        },
    };
    t.categoria = "magica";
    t.raio = 22;
    t.desc = "Muito rápido e forte.";
    t.cor = {40, 90, 200};
    return t;
}

static DefTorre t_ninja() {
    DefTorre t("ninja", "Macaco Ninja", 400, "d", 160);
    t.ataques = {A("projetil", {{"cad", 0.62}, {"dano", 1}, {"pierce", 2}, {"vel", 900}, {"dist", 240}, {"visual", "shuriken"}})};
    t.caminhos = {
        {
            U("Disciplina Ninja", 350, "Todos os ataques 43% mais rápidos.", {{"a", "todos"}, {"cad", 0.7}}),
            U("Shurikens Afiadas", 350, "Shurikens estouram 4 bloons.", {{"pierce", 2}}),
            U("Tiro Duplo", 900, "2 shurikens por vez.", {{"n", 1}, {"spread", 18}}),
            U("Bloonjitsu", 2750, "5 shurikens por vez.", {{"n", 3}, {"spread", 45}}),
            U("Grão-Mestre Ninja", 35000, "8 shurikens de 2 de dano, 2x mais rápido.", {{"n", 3}, {"spread", 72}, {"dano", 1}, {"cad", 0.5}, {"alcance", 40}}),
        },
        {
            U("Distração", 250, "Empurra bloons.", {{"empurra", 25}}),
            U("Contraespionagem", 400, "Remove camo dos bloons atingidos.", {{"a", "todos"}, {"retira_camo", true}}),
            U("Táticas Shinobi", 1200, "Ninjas no alcance (e ele) atacam 8% mais rápido e ganham +8% de pierce (acumula até 20).", {{"buffs", {{"cad", 0.92}, {"pierce_pct", 0.08}, {"escopo", "ninja"}, {"acumula", 20}}}}),
            U("Sabotagem Bloon", 5200, "Habilidade: bloons na metade da velocidade por 15 s.", {{"hab", H("Sabotagem", "lentidao", 60, {{"dur", 15}, {"valor", 0.5}})}}),
            U("Grande Sabotador", 22000, "Sabotagem de 30 s.", {{"hab", H("Grande Sabotagem", "lentidao", 60, {{"dur", 30}, {"valor", 0.5}})}}),
        },
        {
            U("Shuriken Teleguiada", 300, "Shurikens teleguiadas e +7 de alcance.", {{"alcance", 28}, {"busca", true}}),
            U("Estrepes", 450, "Estrepes na trilha.", {{"novo", A("pilha", {{"cad", 3.9}, {"dano", 1}, {"pilha_pierce", 6}, {"pilha_vida", 35}, {"visual", "estrepe"}})}}),
            U("Bomba de Luz", 2250, "Bomba de luz que atordoa; shurikens +2 pierce.", J::array({{{"pierce", 2}}, {{"novo", A("projetil", {{"cad", 2.48}, {"dano", 1}, {"pierce", 1}, {"vel", 600}, {"dist", 260}, {"splash", 150}, {"sdano", 3}, {"spierce", 30}, {"sdtype", "normal"}, {"atordoa", 1.3}, {"visual", "flash"}})}}})),
            U("Bomba Grudenta", 5000, "Bomba grudenta em dirigíveis (450 de dano).", {{"novo", A("projetil", {{"cad", 3.0}, {"dano", 450}, {"pierce", 1}, {"vel", 900}, {"dist", 500}, {"busca", true}, {"so_moab", true}, {"alvo", "forte"}, {"dtype", "normal"}, {"splash", 150}, {"sdano", 100}, {"spierce", 10}, {"sdtype", "normal"}, {"visual", "bomba"}})}}),
            U("Mestre Bombardeiro", 40000, "Bombas grudentas de 3.000, 2x mais frequentes e sem limite de alcance.", {{"a", "bomba"}, {"dano", 2550}, {"sdano", 500}, {"cad", 0.5}, {"global_", true}, {"atordoa", 1.0}, {"moab_atordoa", true}}),
        },
    };
    t.categoria = "magica";
    t.camo = true;
    t.desc = "Detecta camo.";
    t.cor = {170, 30, 30};
    return t;
}

static DefTorre t_alquimista() {
    DefTorre t("alquimista", "Alquimista", 550, "f", 180);
    t.ataques = {A("projetil", {{"cad", 2.0}, {"dano", 1}, {"pierce", 1}, {"vel", 600}, {"dist", 260}, {"dtype", "normal"}, {"splash", 30}, {"sdano", 1}, {"spierce", 15}, {"queima", J::array({0.5, 4})}, {"visual", "pocao"}, {"raio_proj", 7}})};
    t.caminhos = {
        {
            U("Poções Maiores", 250, "Poções maiores: +5 pierce, +50% de área.", {{"splash", 15}, {"spierce", 5}}),
            U("Mistura Ácida", 350, "A cada 10 s, uma torre no alcance estoura chumbo e dá +1 em cerâmica e M.O.A.B. por 10 ataques (acumula até 30).", {{"novo", A("buff", {{"pocao", true}, {"cad", 10.0}, {"valor", 10}, {"pocao_max", 30}, {"visual", "pocao_acido"}, {"buffs", {{"chumbo", true}, {"cer", 1}, {"moab", 1}}}})}}),
            U("Poção do Berserker", 1400, "A cada 8 s, a torre mais próxima ganha +1 de dano, +2 pierce, +10% de alcance e 10% de velocidade por 25 tiros ou 5 s.", {{"novo", A("buff", {{"pocao", true}, {"cad", 8.0}, {"valor", 25}, {"dur", 5}, {"pocao_bloq", 5}, {"visual", "pocao_brew"}, {"buffs", {{"dano", 1}, {"cad", 0.9}, {"pierce", 2}, {"alcance_pct", 0.1}}}})}}),
            U("Estimulante Forte", 2850, "Poção mais forte: +3 pierce, +15% de alcance e 15% de velocidade por 40 tiros ou 12 s.", {{"a", "pocao_brew"}, {"buffs", {{"cad", 0.9444}, {"pierce", 1}, {"alcance_pct", 0.05}}}, {"valor", 15}, {"dur", 12}}),
            U("Poção Permanente", 48000, "As poções novas (estimulante e ácido) ficam permanentes.", J::array({{{"a", "pocao_brew"}, {"valor_x", 0}, {"dur", 0}}, {{"a", "pocao_acido"}, {"valor_x", 0}, {"dur", 0}}})),
        },
        {
            U("Ácido Forte", 250, "Ácido mais rápido.", {{"queima", J::array({0.667, 4.5})}}),
            U("Poções Perecíveis", 475, "+4 de dano em M.O.A.B.", {{"moab", 4}}),
            U("Mistura Instável", 2800, "Poção instável que explode dirigíveis.", {{"novo", A("projetil", {{"cad", 6.0}, {"dano", 20}, {"pierce", 3}, {"vel", 700}, {"dist", 400}, {"busca", true}, {"so_moab", true}, {"alvo", "forte"}, {"dtype", "normal"}, {"visual", "pocao"}})}}),
            U("Tônico Transformador", 4200, "Habilidade: vira monstro por 20 s.", {{"hab", H("Transformação", "turbo", 60, {{"dur", 20}, {"valor", 0.2}})}}),
            U("Transformação Total", 45000, "", {{"hab", H("Transformação Total", "turbo_area", 40, {{"dur", 20}, {"valor", 0.3}})}}),
        },
        {
            U("Arremesso Rápido", 650, "Todos os ataques 25% mais rápidos.", {{"a", "todos"}, {"cad", 0.8}}),
            U("Poça de Ácido", 450, "Poças na trilha.", {{"novo", A("pilha", {{"cad", 3.0}, {"dano", 1}, {"pilha_pierce", 10}, {"pilha_vida", 8}, {"dtype", "normal"}, {"visual", "acido"}})}}),
            U("Chumbo em Ouro", 1000, "$50 extra por chumbo estourado.", {{"ouro_chumbo", 50}}),
            U("Borracha em Ouro", 2750, "Bloons dão o dobro de dinheiro ao estourar.", {{"ouro", 1.0}}),
            U("Mestre Alquimista", 40000, "Poção que encolhe bloons em vermelhos.", J::object()),
        },
    };
    t.categoria = "magica";
    t.desc = "Poções e ácido.";
    t.cor = {110, 60, 130};
    return t;
}

static DefTorre t_druida() {
    DefTorre t("druida", "Druida", 400, "g", 140);
    t.ataques = {A("projetil", {{"cad", 1.1}, {"dano", 1}, {"pierce", 1}, {"n", 5}, {"spread", 40}, {"vel", 700}, {"dist", 200}, {"visual", "espinho"}})};
    t.caminhos = {
        {
            U("Espinhos Duros", 350, "Espinhos duros: pierce 2, estouram chumbo.", {{"pierce", 1}, {"dtype", "normal"}}),
            U("Coração do Trovão", 850, "Raios em cadeia.", {{"novo", A("cadeia", {{"cad", 2.3}, {"dano", 2}, {"pierce", 1}, {"saltos", 4}, {"dtype", "energia"}, {"visual", "relampago"}})}}),
            U("Druida da Tempestade", 1700, "Tornado joga bloons para trás.", {{"novo", A("projetil", {{"cad", 2.5}, {"dano", 0}, {"pierce", 35}, {"vel", 300}, {"dist", 300}, {"busca", true}, {"empurra", 600}, {"visual", "tornado"}, {"raio_proj", 18}})}}),
            U("Bola de Relâmpago", 4500, "Detecta camo e bola de relâmpago.", {{"camo", true}, {"novo", A("cadeia", {{"cad", 0.7}, {"dano", 3}, {"pierce", 1}, {"saltos", 5}, {"dtype", "energia"}, {"visual", "relampago"}})}}),
            U("Monarca das Tempestades", 60000, "Relâmpagos de 30 e 10 de dano; supertempestade de 100.", J::array({{{"a", 1}, {"dano", 28}}, {{"a", 3}, {"dano", 7}}, {{"novo", A("projetil", {{"cad", 4.0}, {"dano", 100}, {"pierce", 50}, {"vel", 200}, {"dist", 600}, {"dtype", "normal"}, {"empurra", 200}, {"visual", "tornado"}, {"raio_proj", 24}})}}})),
        },
        {
            U("Enxame de Espinhos", 250, "8 espinhos.", {{"n", 3}}),
            U("Coração de Carvalho", 350, "Remove regeneração.", {{"a", "todos"}, {"retira_regen", true}}),
            U("Druida da Selva", 1050, "Cipós prendem bloons.", {{"novo", A("pilha", {{"cad", 3.0}, {"dano", 2}, {"pilha_pierce", 8}, {"pilha_vida", 6}, {"dtype", "normal"}, {"visual", "cipo"}})}}),
            U("Recompensa da Selva", 4900, "+10 de alcance e $120 por rodada.", {{"alcance", 40}, {"novo", A("renda", {{"valor", 120}, {"visual", "moeda"}})}}),
            U("Espírito da Floresta", 35000, "Cipós grossos na trilha.", {{"a", "cipo"}, {"dano", 6}, {"pilha_pierce", 40}, {"cad", 0.3}}),
        },
        {
            U("Alcance Druídico", 100, "+10 de alcance.", {{"alcance", 40}, {"dist", 60}}),
            U("Coração da Vingança", 300, "+10% de velocidade.", {{"cad", 0.91}}),
            U("Druida da Ira", 600, "Mais rápido enquanto estoura bloons.", {{"cad", 0.8}}),
            U("Luxúria de Estouros", 2350, "Outros Druidas no alcance: +15% de velocidade e de pierce (acumula até 5 vezes).", {{"novo", A("buff", {{"buffs", {{"vel_pct", 0.15}, {"pierce_pct", 0.15}, {"escopo", "druida"}, {"acumula", 5}}}})}}),
            U("Avatar da Ira", 45000, "+3 de dano e 2x mais rápido.", {{"alcance", 20}, {"dano", 3}, {"cad", 0.5}}),
        },
    };
    t.categoria = "magica";
    t.desc = "Poderes da natureza.";
    t.cor = {60, 130, 60};
    return t;
}

static DefTorre t_fazenda() {
    DefTorre t("fazenda", "Fazenda de Bananas", 1250, "h", 100);
    t.ataques = {A("renda", {{"valor", 80}, {"visual", "banana"}})};
    t.caminhos = {
        {
            U("Produção Aumentada", 500, "6 cachos por rodada ($120).", {{"valor", 40}}),
            U("Produção Maior", 600, "8 cachos por rodada ($160).", {{"valor", 40}}),
            U("Plantação de Bananas", 3000, "16 cachos por rodada ($320).", {{"valor", 160}}),
            U("Centro de Pesquisa de Bananas", 19000, "5 caixas de $300 ($1.500 por rodada).", {{"valor", 1180}}),
            U("Central de Bananas", 115000, "5 caixas de $1.200 ($7.000 por rodada).", {{"valor", 5500}}),
        },
        {
            U("Bananas Duradouras", 300, "Bananas duram 30 s.", J::object()),
            U("Bananas Valiosas", 800, "Bananas valem 25% mais.", {{"valor_x", 1.25}}),
            U("Banco Macaco", 3650, "Banco: guarda o dinheiro e rende 15% de juros.", {{"valor_x", 1.15}}),
            U("Empréstimo do FMI", 7200, "Habilidade: empréstimo de $9.000 (paga depois com metade da renda).", {{"hab", H("Empréstimo", "emprestimo", 85, {{"valor", 9000}})}}),
            U("Macaconomia", 100000, "Habilidade: $9.000 sem dívida.", {{"hab", H("Macaconomia", "dinheiro", 60, {{"valor", 9000}})}}),
        },
        {
            U("Coleta Fácil", 250, "Coleta mais fácil.", J::object()),
            U("Salvamento de Bananas", 400, "Vende por 80%.", {{"venda", 0.8}}),
            U("Mercado", 2700, "Dinheiro automático: $320 por rodada.", {{"valor", 240}}),
            U("Mercado Central", 15000, "$1.120 por rodada.", {{"valor", 800}}),
            U("Wall Street dos Macacos", 70000, "+$4.000 no fim de cada rodada.", {{"valor", 4000}}),
        },
    };
    t.categoria = "suporte";
    t.raio = 26;
    t.desc = "Gera dinheiro a cada rodada.";
    t.cor = {240, 210, 60};
    return t;
}

static DefTorre t_espinhos() {
    DefTorre t("espinhos", "Fábrica de Espinhos", 1000, "j", 136);
    t.ataques = {A("pilha", {{"cad", 1.75}, {"dano", 1}, {"pilha_pierce", 5}, {"pilha_vida", 50}, {"visual", "espinhos"}})};
    t.caminhos = {
        {
            U("Pilhas Maiores", 800, "Pilhas de 10 espinhos.", {{"pilha_pierce", 5}}),
            U("Espinhos Incandescentes", 600, "", {{"dtype", "normal"}}),
            U("Bolas Espinhosas", 2300, "Bolas: 2 de dano, +3 em cerâmica, pierce 11.", {{"dano", 1}, {"cer", 3}, {"fort", 1}, {"pilha_pierce", 1}, {"visual", "bola_espinho"}}),
            U("Minas Espinhosas", 9500, "Minas que explodem (10 de dano) ao acabar.", {{"pilha_pierce", 7}, {"cer", 3}, {"fort", 2}, {"splash", 120}, {"sdano", 10}, {"spierce", 30}}),
            U("Super Minas", 125000, "Super minas de 50 de dano.", {{"cad", 2.0}, {"dano", 48}, {"cer", 24}, {"fort", 12}, {"sdano", 40}}),
        },
        {
            U("Produção Rápida", 600, "Produz 25% mais rápido.", {{"cad", 0.8}}),
            U("Produção Mais Rápida", 800, "Produz 33% mais rápido.", {{"cad", 0.75}}),
            U("Triturador de M.O.A.B.", 2500, "+7 de dano em M.O.A.B.", {{"moab", 7}}),
            U("Tempestade de Espinhos", 7000, "Habilidade: espinhos em toda a trilha.", {{"hab", H("Tempestade de Espinhos", "spikes_global", 40, {{"valor", 60}, {"dano", 1}})}}),
            U("Tapete de Espinhos", 41000, "Tempestade de espinhos a cada 15 s.", {{"hab", H("Tapete de Espinhos", "spikes_global", 15, {{"valor", 60}, {"dano", 1}})}}),
        },
        {
            U("Alcance Longo", 150, "+8 de alcance; pilhas duram 100 s.", {{"alcance", 32}, {"pilha_vida", 2.0}}),
            U("Espinhos Inteligentes", 400, "4x mais rápido no começo da rodada.", J::object()),
            U("Espinhos Duradouros", 1300, "Pilhas duram 140 s.", {{"pilha_vida", 1.4}}),
            U("Espinhos Mortais", 3600, "Espinhos com 3 de dano.", {{"dano", 2}}),
            U("Perma-Espinho", 30000, "Espinhos de 10 de dano, pierce 50, duram 300 s.", {{"cad", 3.46}, {"dano", 7}, {"pilha_pierce", 45}, {"pilha_vida", 2.14}}),
        },
    };
    t.categoria = "suporte";
    t.desc = "Espinhos na trilha.";
    t.cor = {130, 130, 140};
    return t;
}

static DefTorre t_vila() {
    DefTorre t("vila", "Vila dos Macacos", 1200, "k", 160);
    t.ataques = {A("buff", {{"buffs", {{"alcance_pct", 0.1}}}})};
    t.caminhos = {
        {
            U("Raio Maior", 400, "+8 de alcance.", {{"alcance", 32}}),
            U("Tambores da Selva", 1500, "Torres próximas atacam 15% mais rápido.", {{"buffs", {{"cad", 0.85}}}}),
            U("Treinamento Primário", 800, "Torres Primárias próximas: +1 pierce e +10% de alcance.", {{"novo", A("buff", {{"visual", "buff_primaria"}, {"buffs", {{"pierce", 1}, {"alcance_pct", 0.1}, {"escopo", "primaria"}}}})}}),
            U("Mentoria Primária", 2500, "+7 de alcance; Primárias próximas ganham +5 de alcance.", J::array({{{"alcance", 28}}, {{"a", "buff_primaria"}, {"buffs", {{"alcance", 20}}}}})),
            U("Especialização Primária", 25000, "Primárias próximas: +3 pierce no total; balista gigante.", {{"a", "buff_primaria"}, {"buffs", {{"pierce", 2}}}, {"novo", A("projetil", {{"cad", 4.0}, {"dano", 10}, {"cer", 270}, {"pierce", 100}, {"vel", 1200}, {"dist", 2500}, {"busca", true}, {"global_", true}, {"dtype", "normal"}, {"alvo", "forte"}, {"visual", "balista"}, {"raio_proj", 12}})}}),
        },
        {
            U("Bloqueador de Crescimento", 250, "Remove regeneração.", {{"novo", A("aura", {{"cad", 0.5}, {"dano", 0}, {"pierce", 999}, {"retira_regen", true}, {"visual", "nenhum"}})}}),
            U("Radar", 2000, "Torres próximas detectam camo.", {{"buffs", {{"camo", true}}}}),
            U("Agência de Inteligência Macaco", 7500, "Torres próximas estouram tudo.", {{"buffs", {{"dtype_normal", true}}}}),
            U("Chamado às Armas", 20000, "Habilidade: todas as torres 50% mais rápidas por 15 s.", {{"hab", H("Chamado às Armas", "turbo_area", 45, {{"dur", 15}, {"valor", 0.667}, {"global_", true}})}}),
            U("Defesa da Pátria", 40000, "Habilidade: todas as torres 2x mais rápidas por 20 s.", {{"hab", H("Defesa da Pátria", "turbo_area", 45, {{"dur", 20}, {"valor", 0.5}, {"global_", true}})}}),
        },
        {
            U("Negócios Macacos", 500, "10% de desconto nas torres próximas.", {{"desconto", 0.1}}),
            U("Comércio Macaco", 500, "15% de desconto.", {{"desconto", 0.15}}),
            U("Cidade Macaco", 10000, "Torres próximas ganham 50% mais por estouro.", {{"buffs", {{"ouro", 0.5}}}}),
            U("Metrópole Macaco", 3000, "+10 de alcance.", {{"alcance", 40}}),
            U("Macacópolis", 5000, "Sacrifica fazendas para gerar dinheiro.", J::object()),
        },
    };
    t.categoria = "suporte";
    t.raio = 28;
    t.desc = "Melhora torres próximas.";
    t.cor = {160, 110, 60};
    return t;
}

static DefTorre t_engenheiro() {
    DefTorre t("engenheiro", "Macaco Engenheiro", 350, "l", 160);
    t.ataques = {A("projetil", {{"cad", 0.7}, {"dano", 1}, {"pierce", 3}, {"vel", 800}, {"dist", 240}, {"visual", "prego"}})};
    t.caminhos = {
        {
            U("Torreta Sentinela", 500, "Torretas a cada 10 s.", {{"novo", A("invocar", {{"cad", 10.0}, {"dur", 25}, {"base", "sentinela"}})}}),
            U("Engenharia Rápida", 400, "Torretas a cada 5 s.", {{"a", "invocar"}, {"cad", 0.5}}),
            U("Engrenagens", 575, "Engenheiro e torretas 2x mais rápidos.", J::array({{{"cad", 0.5}}, {{"a", "invocar"}, {"nivel_inv", 1}}})),
            U("Especialista em Sentinelas", 2500, "Torretas especializadas.", {{"a", "invocar"}, {"nivel_inv", 1}}),
            U("Campeão das Sentinelas", 32000, "Torretas campeãs.", {{"a", "invocar"}, {"nivel_inv", 2}}),
        },
        {
            U("Área de Serviço Maior", 250, "+50% de alcance.", {{"alcance", 80}}),
            U("Desconstrução", 350, "+1 de dano em M.O.A.B. e fortificado.", {{"moab", 1}, {"fort", 1}}),
            U("Espuma Purificadora", 900, "Espuma remove camo e regeneração.", {{"novo", A("pilha", {{"cad", 4.0}, {"dano", 1}, {"pilha_pierce", 10}, {"pilha_vida", 8}, {"retira_camo", true}, {"retira_regen", true}, {"visual", "espuma"}})}}),
            U("Overclock", 13500, "Habilidade: acelera torres próximas.", {{"hab", H("Overclock", "turbo_area", 45, {{"dur", 30}, {"valor", 0.6}})}}),
            U("Ultraimpulso", 72000, "", {{"buffs", {{"cad", 0.7}}}, {"hab", H("Ultraimpulso", "turbo_area", 30, {{"dur", 45}, {"valor", 0.4}})}}),
        },
        {
            U("Pregos Enormes", 450, "+5 pierce.", {{"pierce", 5}}),
            U("Pino", 220, "Prega bloons e atordoa por 1 s.", {{"atordoa", 1.0}}),
            U("Arma Dupla", 450, "Atira 2x mais rápido.", {{"cad", 0.5}}),
            U("Armadilha Bloon", 3600, "Armadilha que prende até 500 de RBE e dá o dobro de dinheiro.", {{"novo", A("pilha", {{"cad", 8.0}, {"dano", 0}, {"pilha_pierce", 500}, {"pilha_vida", 9999}, {"armadilha", true}, {"valor", 2}, {"visual", "armadilha"}})}}),
            U("Armadilha XXXL", 45000, "Armadilha de 10.000 de RBE que prende dirigíveis.", {{"a", "armadilha"}, {"pilha_pierce", 9500}, {"cad", 0.575}, {"prende_moab", true}, {"valor", 1}}),
        },
    };
    t.categoria = "suporte";
    t.desc = "Pregos e torretas.";
    t.cor = {230, 190, 40};
    return t;
}

// Torres auxiliares (invocadas por habilidades/upgrades, nao compraveis)
static DefTorre aux_sentinela() {
    DefTorre t("sentinela", "Torreta", 0, "", 120);
    t.ataques = {A("projetil", {{"cad", 0.95}, {"dano", 1}, {"pierce", 2}, {"vel", 800}, {"dist", 200}, {"visual", "prego"}})};
    t.raio = 12;
    t.cor = {200, 170, 40};
    return t;
}

static DefTorre aux_fenix() {
    DefTorre t("fenix", "Fênix", 0, "", 9999);
    t.ataques = {A("projetil", {{"cad", 0.1}, {"dano", 5}, {"pierce", 8}, {"vel", 700}, {"dist", 700}, {"global_", true}, {"dtype", "energia"}, {"visual", "fogo"}})};
    t.mov = Mov::ORBITA;
    t.raio = 22;
    t.cor = {250, 120, 30};
    return t;
}

// ================================================================ HEROIS
// Herois do BTD6. Niveis e habilidades da Blooncyclopedia (paginas "<Heroi> (BTD6)"), lidas pela API em
// 30/09/2026. Alcance em unidades x4 e raio de explosao x2,5 (a escala que o clone usa nas torres). Os
// numeros marcados com "aprox." nao tem fonte ou dependem de uma mecanica que o motor nao tem; a lista
// esta em docs/btd6-transformacao.md. "h3" e "h10" num nivel mudam os campos das habilidades.
static DefTorre h_quincy() {
    DefTorre t("quincy", "Quincy", 540, "u", 200);
    t.ataques = {A("projetil", {{"cad", 0.95}, {"dano", 1}, {"pierce", 3}, {"quica", 3}, {"vel", 900}, {"dist", 260}, {"visual", "flecha"}})};
    t.cor = {150, 95, 45};
    t.heroi = true;
    t.xp_escala = 1.0;
    t.titulo = "Arqueiro Orgulhoso";
    // flecha explosiva a cada 3 tiros (2 a partir do nivel 17) vira um segundo ataque com 3x a recarga;
    // raio da explosao aprox. (a wiki diz "pequena area", 10 de pierce)
    const J explosiva = A("projetil", {{"cad", 2.85}, {"dano", 1}, {"pierce", 3}, {"quica", 3}, {"vel", 900}, {"dist", 260}, {"splash", 30}, {"sdano", 1}, {"spierce", 10}, {"sdtype", "explosao"}, {"visual", "flecha"}});
    t.niveis = {
        {2, {{"pierce", 1}, {"quica", 1}}},
        {4, {{"alcance", 8}}},
        {5, {{"camo", true}}},
        {6, {{"n", 1}, {"spread", 10}}},
        {7, J::array({{{"novo", explosiva}}, {{"a", 1}, {"pierce", 1}, {"quica", 1}}})},
        {8, {{"a", "todos"}, {"moab", 2}}},
        {9, {{"a", "todos"}, {"pierce", 2}, {"quica", 2}}},
        {11, {{"a", "todos"}, {"cad", 0.6316}}},
        {12, {{"a", "todos"}, {"pierce", 1}, {"quica", 1}}},
        {13, {{"alcance", 8}, {"h3", {{"dur", 12}}}}},
        {14, {{"a", "todos"}, {"moab", 1}}},
        {15, {{"h3", {{"valor", 0.25}, {"recarga", 45}}}}},
        {16, {{"a", "todos"}, {"cad", 0.6667}}},
        {17, J::array({{{"a", "todos"}, {"dist", 65}}, {{"a", 1}, {"cad", 0.6667}}})},
        {18, {{"a", "todos"}, {"cad", 0.625}, {"h10", {{"cer_mais", 18}, {"recarga", 55}}}}},
        {19, {{"a", "todos"}, {"n", 1}, {"spread", 10}, {"pierce", 2}, {"quica", 2}}},
        {20, {{"a", "todos"}, {"cad", 0.8}, {"h10", {{"valor", 10}, {"moab_mais", 10}, {"cer_mais", 24}}}}},
    };
    t.hab3 = H("Tiro Rápido", "turbo", 60, {{"dur", 8}, {"valor", 0.333}});
    // Storm of Arrows acerta varias vezes por bloon; aqui e um golpe unico em todos os bloons (aprox.)
    t.hab10 = H("Tempestade de Flechas", "dano_global", 70, {{"valor", 6}, {"moab_mais", 6}});
    return t;
}

static DefTorre h_gwendolin() {
    DefTorre t("gwendolin", "Gwendolin", 725, "u", 152);
    t.ataques = {A("projetil", {{"cad", 0.5}, {"dano", 1}, {"pierce", 2}, {"vel", 800}, {"dist", 240}, {"dtype", "energia"}, {"visual", "fogo"}})};
    t.cor = {200, 80, 40};
    t.heroi = true;
    t.xp_escala = 1.0;
    t.titulo = "Cientista Piromaníaca";
    // Heat It Up: explosao de fogo em volta dela e +1 pierce e chumbo para as torres por perto
    const J calor = A("aura", {{"cad", 1.5}, {"dano", 3}, {"pierce", 100}, {"raio_aura", 152}, {"dtype", "energia"}, {"visual", "chamas"}, {"buffs", {{"pierce", 1}, {"chumbo", true}, {"sem_si", true}}}});
    t.niveis = {
        {2, {{"pierce", 1}}},
        {4, {{"novo", calor}}},
        {5, {{"pierce", 1}}},
        {6, {{"queima", J::array({0.667, 3.1})}}},
        {7, {{"a", "aura"}, {"raio_aura", 20}, {"h3", {{"valor", 600}}}}},
        {8, {{"n", 1}, {"spread", 8}}},
        {9, {{"dano", 1}, {"queima", J::array({1.333, 4})}}},
        {10, {{"queima", J::array({2.0, 5})}}},
        {11, J::array({{{"alcance", 12}, {"queima", J::array({2.667, 6})}}, {{"a", "aura"}, {"raio_aura", 12}}})},
        {12, J::array({{{"cad", 0.8}, {"queima", J::array({3.333, 7})}}, {{"a", "aura"}, {"dano", 7}}})},
        {13, {{"pierce", 4}, {"queima", J::array({4.0, 8})}}},
        {14, {{"queima", J::array({4.667, 9})}, {"h3", {{"dano", 2}}}}},
        {15, J::array({{{"cad", 0.75}, {"queima", J::array({5.333, 10})}}, {{"a", "aura"}, {"dano", 10}}})},
        {16, J::array({J{{"dtype", "normal"}, {"queima", J::array({6.0, 11})}, {"h10", J{{"queima", J::array({10, 10.4})}}}}, J{{"a", "aura"}, {"dtype", "normal"}}})},
        {17, J::array({{{"queima", J::array({6.667, 12})}}, {{"a", "aura"}, {"buffs", {{"dano", 1}}}}})},
        {18, J::array({{{"cad", 0.5}, {"queima", J::array({7.333, 13})}}, {{"a", "aura"}, {"dano", 10}}})},
        {19, {{"n", 1}, {"spread", 8}, {"queima", J::array({8.0, 14})}}},
        {20, {{"queima", J::array({8.667, 15})}, {"h3", {{"dano", 5}}}, {"h10", {{"valor", 10}, {"moab_mais", 40}, {"queima", J::array({20, 10.4})}}}}},
    };
    // Cocktail of Fire: parede de fogo por 12 s; o pierce total da pilha e aprox.
    t.hab3 = H("Coquetel de Fogo", "spikes_local", 30, {{"valor", 300}, {"dano", 1}, {"dur", 12}});
    t.hab10 = H("Tempestade de Fogo", "dano_global", 60, {{"valor", 5}, {"moab_mais", 15}, {"queima", J::array({4, 8})}});
    return t;
}

static DefTorre h_striker() {
    DefTorre t("striker", "Striker Jones", 700, "u", 220);
    t.ataques = {A("projetil", {{"cad", 1.2}, {"dano", 0}, {"pierce", 1}, {"vel", 650}, {"dist", 300}, {"visual", "bomba"}, {"splash", 37.5}, {"sdano", 2}, {"spierce", 10}, {"sdtype", "explosao"}, {"raio_proj", 7}})};
    t.cor = {80, 100, 60};
    t.heroi = true;
    t.xp_escala = 1.0;
    t.titulo = "Comandante de Artilharia";
    t.niveis = {
        {2, {{"splash", 10}}},
        // Bombas e Morteiros do mapa: x0,9 de recarga (x0,81 no nivel 18)
        {4, {{"novo", A("buff", {{"visual", "buff_striker"}, {"buffs", {{"cad", 0.9}, {"escopo", "bomba|morteiro"}, {"global_", true}}}})}}},
        {6, {{"spierce", 10}, {"splash", 33.75}}},
        {7, {{"sdano", 2}}},
        {8, {{"novo", A("buff", {{"visual", "buff_striker_alcance"}, {"buffs", {{"alcance_pct", 0.05}, {"pierce_pct", 0.25}}}})}}},
        {9, {{"cad", 0.8333}, {"h3", {{"valor", 12}, {"sdano", 12}, {"atordoa", 4}}}}},
        {11, {{"cad", 0.8}}},
        {12, {{"alcance", 12}, {"sdano", 2}}},
        {13, {{"cad", 0.75}}},
        {14, {{"h3", {{"splash", 101}, {"atordoa", 6}}}}},
        {15, {{"h3", {{"recarga", 11}}}}},
        {16, {{"cad", 0.75}}},
        {17, {{"alcance", 8}, {"sdano", 2}}},
        {18, {{"a", "buff_striker"}, {"buffs", {{"cad", 0.9}}}}},
        // 100% de chance de estourar o preto com explosao: aprox. com dano normal na explosao
        {19, {{"cad", 0.6667}, {"sdtype", "normal"}}},
    };
    t.hab3 = H("Projétil de Concussão", "dano_forte", 16, {{"valor", 2}, {"n", 1}, {"splash", 67.5}, {"sdano", 2}, {"atordoa", 1}});
    t.hab10 = H("Comando de Artilharia", "recarregar", 80, {{"filtro", "bomba,morteiro"}});
    return t;
}

static DefTorre h_obyn() {
    DefTorre t("obyn", "Obyn Guardião", 650, "u", 160);
    t.ataques = {A("projetil", {{"cad", 1.35}, {"dano", 2}, {"pierce", 4}, {"vel", 700}, {"dist", 260}, {"busca", true}, {"dtype", "energia"}, {"visual", "espirito"}})};
    t.cor = {40, 120, 90};
    t.heroi = true;
    t.xp_escala = 1.0;
    t.titulo = "Guardião da Floresta";
    t.niveis = {
        // Nature's Wrath: Druidas no alcance
        {2, {{"novo", A("buff", {{"visual", "buff_obyn"}, {"buffs", {{"pierce", 1}, {"escopo", "druida"}}}})}}},
        {5, J::array({{{"cad", 0.8148}}, {{"a", "buff_obyn"}, {"buffs", {{"alcance_pct", 0.4}}}}})},
        {6, {{"pierce", 5}}},
        {7, {{"h3", {{"valor", 100}}}}},
        {8, {{"a", "buff_obyn"}, {"buffs", {{"camo", true}}}}},
        {9, {{"dano", 2}}},
        // Nature's Clarity: torres Magicas no alcance
        {11, {{"novo", A("buff", {{"visual", "buff_obyn_magia"}, {"buffs", {{"alcance", 20}, {"pierce", 2}, {"escopo", "magica"}}}})}}},
        {12, {{"cad", 0.7273}}},
        {13, {{"pierce", 5}}},
        {14, {{"dano", 2}}},
        {16, {{"h3", {{"valor", 500}}}}},
        {17, {{"cad", 0.625}}},
        {19, {{"dano", 2}}},
        {20, {{"h10", {{"valor", 7500}, {"recarga", 75}}}}},
    };
    t.hab3 = H("Espinheiros", "spikes_local", 30, {{"valor", 50}, {"dano", 1}, {"dur", 120}});
    // Wall of Trees prende 2.500 de RBE; aqui e uma pilha com esse pierce (aprox.)
    t.hab10 = H("Muralha de Árvores", "spikes_local", 90, {{"valor", 2500}, {"dano", 1}, {"dur", 30}});
    return t;
}

static DefTorre h_churchill() {
    DefTorre t("churchill", "Capitão Churchill", 2000, "u", 260);
    // o projetil atravessa e explode ate 3 vezes (pierce 3); a explosao da o dano
    t.ataques = {A("projetil", {{"cad", 1.5}, {"dano", 0}, {"pierce", 3}, {"vel", 900}, {"dist", 320}, {"dtype", "normal"}, {"splash", 45}, {"sdano", 3}, {"spierce", 12}, {"sdtype", "explosao"}, {"visual", "bala_canhao"}, {"raio_proj", 8}})};
    t.cor = {70, 90, 60};
    t.heroi = true;
    t.xp_escala = 1.71;
    t.titulo = "Tanque Blindado";
    const J metralhadora = A("hitscan", {{"cad", 0.1}, {"dano", 2}, {"pierce", 1}, {"visual", "bala"}});
    t.niveis = {
        {2, {{"spierce", 3}}},
        {4, {{"alcance", 40}}},
        {5, {{"novo", metralhadora}}},
        {6, {{"camo", true}}},
        {7, J::array({{{"pierce", 1}, {"sdano", 3}}, {{"a", "hitscan"}, {"dano", 2}}})},
        {8, {{"cad", 0.8}}},
        {9, {{"pierce", 1}}},
        {11, {{"spierce", 5}}},
        {12, J::array({{{"sdano", 3}}, {{"a", "hitscan"}, {"dano", 2}}})},
        {13, {{"h3", {{"buffs", {{"moab", 21}, {"cer", 21}, {"dano", 3}, {"dtype_normal", true}}}}}, {"h10", {{"valor", 4800}}}}},
        {14, J::array({{{"sdano", 3}}, {{"a", "hitscan"}, {"dano", 2}}})},
        // +4 (metralhadora +2) em chumbo, fortificado, camo e atordoado: aqui so em fortificado (aprox.)
        {15, J::array({{{"fort", 4}}, {{"a", "hitscan"}, {"fort", 2}}})},
        {17, {{"cad", 0.75}, {"h3", {{"buffs", {{"moab", 33}, {"cer", 33}, {"dano", 6}, {"dtype_normal", true}}}}}, {"h10", {{"valor", 9600}}}}},
        {18, J::array({{{"sdano", 3}}, {{"a", "hitscan"}, {"dano", 4}, {"fort", 2}}})},
        {19, {{"pierce", 2}}},
        {20, J::array({J{{"sdano", 10}, {"h3", J{{"dur", 17.5}}}, {"h10", J{{"valor", 19200}, {"recarga", 30}}}}, J{{"a", "hitscan"}, {"dano", 4}}})},
    };
    // Armor Piercing Shells dura 9 s e ganha 0,5 s por nivel (aqui so no 20)
    t.hab3 = H("Projéteis Perfurantes", "turbo_area", 30, {{"dur", 9}, {"filtro", "churchill"}, {"buffs", {{"moab", 9}, {"cer", 9}, {"dtype_normal", true}}}});
    // MOAB Barrage: 16 tiros de 200 em ate 10 dirigiveis
    t.hab10 = H("Barragem M.O.A.B.", "dano_forte", 60, {{"valor", 3200}, {"n", 10}, {"moab_so", true}});
    return t;
}

static DefTorre h_benjamin() {
    DefTorre t("benjamin", "Benjamin", 1200, "u", 100);
    t.ataques = {A("renda", {{"valor", 90}, {"visual", "moeda"}})};
    t.cor = {60, 60, 80};
    t.heroi = true;
    t.xp_escala = 1.5;
    t.titulo = "Hacker";
    // renda por rodada: $90, $140 (2 a 7), $250 (8 a 10), $1.000 (11 a 14), $2.500 (15 e 16), $5.000 (17 a 20)
    t.niveis = {
        {2, {{"valor", 50}}},
        {8, {{"valor", 110}}},
        {11, {{"valor", 750}}},
        {13, {{"h3", {{"n", 6}, {"dur", 8}, {"buffs", {{"dano", 2}}}}}}},
        {15, {{"valor", 1500}}},
        {17, {{"valor", 2500}}},
        {19, {{"h3", {{"dur", 9}, {"buffs", {{"dano", 3}}}}}}},
    };
    t.hab3 = H("Biohack", "turbo_area", 30, {{"dur", 6}, {"n", 4}, {"sem_si", true}, {"buffs", {{"dano", 1}}}});
    // Syphon Funding rebaixa os bloons novos e dobra o dinheiro deles; aqui vira dinheiro direto (aprox.)
    t.hab10 = H("Sifão de Fundos", "dinheiro", 65, {{"valor", 1500}});
    return t;
}

static DefTorre h_ezili() {
    DefTorre t("ezili", "Ezili", 550, "u", 160);
    t.ataques = {A("projetil", {{"cad", 1.2}, {"dano", 1}, {"pierce", 1}, {"vel", 1400}, {"dist", 240}, {"busca", true}, {"dtype", "normal"}, {"queima", J::array({0.8, 2.6})}, {"splash", 20}, {"sdano", 1}, {"spierce", 5}, {"sdtype", "normal"}, {"visual", "maldicao"}})};
    t.cor = {110, 40, 90};
    t.heroi = true;
    t.xp_escala = 1.425;
    t.titulo = "Sacerdotisa Vodu";
    t.niveis = {
        {2, {{"alcance", 12}}},
        {5, {{"cad", 0.8333}}},
        {6, {{"moab", 19}}},
        {8, {{"queima", J::array({1.111, 5.5})}}},
        {9, {{"retira_camo", true}, {"retira_regen", true}}},
        {11, {{"alcance", 16}}},
        {12, {{"moab", 10}, {"h3", {{"recarga", 40}}}}},
        {13, {{"splash", 7.5}, {"spierce", 10}}},
        {14, {{"queima", J::array({2, 4.05})}, {"h3", {{"recarga", 30}}}}},
        {15, {{"queima", J::array({2, 6.05})}}},
        {17, {{"splash", 5}, {"spierce", 10}}},
        {18, {{"cad", 0.8}}},
        {19, {{"queima", J::array({6, 6.05})}}},
        {20, {{"moab", 20}, {"h10", {{"pct", 1.25}, {"recarga", 40}}}}},
    };
    t.hab3 = H("Para-Coração", "sem_regen", 45, J::object());
    // MOAB Hex: 4% da vida maxima + 1 por segundo por 25 s
    t.hab10 = H("Maldição M.O.A.B.", "dano_forte", 60, {{"valor", 25}, {"pct", 1.0}, {"n", 1}, {"moab_so", true}});
    return t;
}

static DefTorre h_pat() {
    DefTorre t("pat", "Pat Fusty", 800, "u", 108);
    t.ataques = {A("morteiro", {{"cad", 1.1}, {"dano", 3}, {"pierce", 1}, {"cer", 2}, {"dtype", "normal"}, {"splash", 20}, {"sdano", 2}, {"spierce", 10}, {"sdtype", "normal"}, {"visual", "impacto"}})};
    t.cor = {150, 110, 70};
    t.heroi = true;
    t.xp_escala = 1.425;
    t.titulo = "Macaco Gigante";
    t.niveis = {
        {2, {{"splash", 12.5}}},
        {4, {{"cad", 0.8182}}},
        {6, {{"splash", 17.5}, {"atordoa", 0.3}}},
        {7, {{"dano", 1}, {"sdano", 1}, {"spierce", 10}}},
        {8, {{"cad", 0.8333}}},
        {9, {{"alcance", 12}, {"h3", {{"buffs", {{"dano", 2}}}}}}},
        {11, {{"dano", 1}, {"sdano", 1}}},
        {12, {{"cad", 0.8667}}},
        {13, {{"atordoa", 0.5}}},
        {14, {{"h3", {{"dur", 10}, {"buffs", {{"dano", 3}}}}}}},
        {15, {{"spierce", 10}}},
        {16, {{"cer", 5}}},
        {17, {{"cad", 0.8462}}},
        {18, {{"spierce", 10}}},
        {19, {{"dano", 5}, {"sdano", 5}}},
        {20, {{"h10", {{"n", 4}}}}},
    };
    t.hab3 = H("Rugido de Incentivo", "turbo_area", 45, {{"dur", 8}, {"buffs", {{"dano", 1}}}});
    t.hab10 = H("Grande Aperto", "dano_forte", 20, {{"valor", 9999999}, {"n", 1}, {"moab_so", true}});
    return t;
}

static DefTorre h_adora() {
    DefTorre t("adora", "Adora", 1000, "u", 170);
    t.ataques = {A("projetil", {{"cad", 1.0}, {"dano", 1}, {"pierce", 5}, {"vel", 900}, {"dist", 320}, {"busca", true}, {"dtype", "energia"}, {"visual", "luz"}})};
    t.cor = {230, 200, 90};
    t.heroi = true;
    t.xp_escala = 1.71;
    t.titulo = "Sacerdotisa do Sol";
    t.niveis = {
        {2, {{"alcance", 16}}},
        {4, {{"n", 1}, {"spread", 30}}},
        {5, {{"pierce", 3}}},
        {6, {{"n", 1}, {"spread", 30}}},
        {8, {{"n", 1}, {"spread", 30}}},
        {9, {{"alcance", 20}, {"fort", 1}}},
        {11, {{"cad", 0.85}}},
        {12, {{"n", 1}, {"spread", 30}}},
        {13, {{"pierce", 3}, {"fort", 1}}},
        {14, {{"n", 1}, {"spread", 30}}},
        {15, {{"dano", 1}}},
        {16, {{"h3", {{"dur", 15}, {"buffs", {{"alcance_pct", 1.0}, {"pierce_pct", 1.0}, {"dtype_normal", true}, {"dano", 2}}}}}}},
        {17, {{"cad", 0.8235}}},
        {18, {{"n", 2}, {"spread", 60}}},
        {19, {{"alcance", 20}, {"fort", 1}}},
        {20, {{"h10", {{"dur", 15}}}}},
    };
    t.hab3 = H("Braço Longo da Luz", "turbo_area", 45, {{"dur", 10}, {"filtro", "adora"}, {"buffs", {{"alcance_pct", 1.0}, {"pierce_pct", 1.0}, {"dtype_normal", true}}}});
    // Ball of Light vira a Fenix invocada (aprox.)
    t.hab10 = H("Bola de Luz", "invocar", 60, {{"dur", 12}, {"base", "fenix"}});
    return t;
}

static DefTorre h_brickell() {
    DefTorre t("brickell", "Almirante Brickell", 900, "u", 200);
    t.ataques = {A("projetil", {{"cad", 0.8}, {"dano", 3}, {"pierce", 3}, {"vel", 1400}, {"dist", 260}, {"visual", "bala"}})};
    t.agua = true;
    t.cor = {40, 70, 130};
    t.heroi = true;
    t.xp_escala = 1.425;
    t.titulo = "Comandante Naval";
    // minas na trilha a cada 3 s (no BTD6 ficam na agua perto da trilha)
    const J minas = A("pilha", {{"cad", 3.0}, {"dano", 1}, {"pilha_pierce", 20}, {"pilha_vida", 120}, {"visual", "armadilha"}});
    t.niveis = {
        {2, J::array({{{"cad", 0.8125}}, {{"a", "pilha"}, {"cad", 0.9333}}})},
        {4, {{"a", "pilha"}, {"pilha_pierce", 8}}},
        {5, {{"h3", {{"buffs", {{"pierce", 1}, {"dtype_normal", true}}}}}}},
        {6, J::array({{{"dano", 3}}, {{"a", "pilha"}, {"dano", 1}}})},
        {7, {{"alcance", 32}, {"camo", true}}},
        {8, {{"novo", A("buff", {{"visual", "buff_brickell"}, {"buffs", {{"pierce", 1}, {"escopo", "agua"}}}})}, {"h3", {{"buffs", {{"pierce", 1}, {"dtype_normal", true}, {"camo", true}}}}}}},
        {9, {{"a", "pilha"}, {"dano", 5}}},
        {11, {{"a", "pilha"}, {"cad", 0.8929}}},
        {12, J::array({{{"dano", 6}, {"cad", 0.5385}}, {{"a", "pilha"}, {"dano", 5}}})},
        {13, {{"h10", {{"recarga", 50}}}}},
        {14, {{"h3", {{"dur", 10}}}}},
        {15, J::array({{{"dano", 6}}, {{"a", "pilha"}, {"dtype", "normal"}, {"retira_camo", true}}})},
        {16, {{"alcance", 16}}},
        {17, J::array({{{"dano", 22}}, {{"a", "pilha"}, {"dano", 10}}})},
        {18, {{"h10", {{"recarga", 40}}}}},
        {19, {{"h3", {{"global_", true}}}}},
        {20, {{"h10", {{"valor", 11000}, {"sdano", 11000}}}}},
    };
    t.hab3 = H("Táticas Navais", "turbo_area", 50, {{"dur", 8}, {"valor", 0.5}, {"filtro", "submarino,bucaneiro,brickell"}});
    t.hab10 = H("Mega Mina", "dano_forte", 60, {{"valor", 4000}, {"n", 1}, {"moab_so", true}, {"splash", 300}, {"sdano", 4000}, {"atordoa", 5}});
    return t;
}

static DefTorre h_etienne() {
    DefTorre t("etienne", "Etienne", 650, "u", 220);
    // os drones atiram de perto dos bloons; aqui os dardos saem dele, teleguiados
    t.ataques = {A("projetil", {{"cad", 0.7}, {"dano", 1}, {"pierce", 2}, {"vel", 800}, {"dist", 400}, {"busca", true}, {"visual", "drone"}})};
    t.cor = {80, 110, 150};
    t.heroi = true;
    t.xp_escala = 1.0;
    t.titulo = "Especialista em Drones";
    t.niveis = {
        {2, {{"alcance", 20}, {"novo", A("buff", {{"visual", "buff_etienne"}, {"buffs", {{"alcance_pct", 0.1}}}})}}},
        {4, {{"pierce", 1}}},
        {5, {{"cad", 0.7857}, {"camo", true}}},
        {6, {{"h3", {{"recarga", 55}}}}},
        {7, {{"n", 1}, {"spread", 20}}},
        // UAV: camo para todas as torres do mapa
        {8, {{"novo", A("buff", {{"visual", "buff_uav"}, {"buffs", {{"camo", true}, {"global_", true}}}})}}},
        {9, {{"dano", 1}}},
        {11, {{"n", 1}, {"spread", 10}}},
        {12, {{"pierce", 3}}},
        {13, {{"h10", {{"recarga", 75}}}}},
        {14, {{"dano", 1}}},
        {15, {{"h10", {{"dur", 20}}}}},
        {16, J::array({J{{"pierce", 3}, {"alcance", 80}, {"h3", J{{"recarga", 50}}}}, J{{"a", "buff_etienne"}, {"buffs", J{{"alcance_pct", 0.1}}}}})},
        {18, {{"dano", 1}}},
        {19, {{"n", 1}, {"spread", 10}}},
    };
    // Drone Swarm: 4 drones a mais por 18,5 s, aqui como turbo (aprox.)
    t.hab3 = H("Enxame de Drones", "turbo", 80, {{"dur", 18.5}, {"valor", 0.5}});
    t.hab10 = H("UCAV", "invocar", 90, {{"dur", 12}, {"base", "fenix"}});
    return t;
}

static DefTorre h_sauda() {
    DefTorre t("sauda", "Sauda", 600, "u", 92);
    // golpe vale 2x contra fortificado, ceramica e dirigivel
    t.ataques = {A("aura", {{"cad", 0.45}, {"dano", 1}, {"pierce", 3}, {"cer", 1}, {"moab", 1}, {"fort", 1}, {"visual", "espadas"}})};
    t.cor = {200, 120, 60};
    t.heroi = true;
    t.xp_escala = 1.425;
    t.titulo = "Espadachim";
    t.niveis = {
        {2, {{"pierce", 1}}},
        {4, {{"dano", 1}, {"cer", 1}, {"moab", 1}, {"fort", 1}}},
        {5, {{"cad", 0.8}}},
        {6, {{"pierce", 2}, {"alcance", 12}}},
        {8, {{"cad", 0.75}}},
        {9, {{"dano", 1}, {"cer", 1}, {"moab", 1}, {"fort", 1}, {"queima", J::array({0.5, 4.05})}}},
        {12, {{"h3", {{"valor", 40}, {"moab_mais", 140}, {"sdano", 40}}}}},
        {13, {{"dtype", "normal"}}},
        {14, {{"cad", 0.6667}}},
        {15, {{"pierce", 2}, {"alcance", 12}, {"h3", {{"valor", 80}, {"sdano", 80}}}}},
        {16, {{"h10", {{"valor", 240}}}}},
        {17, {{"dano", 1}, {"cer", 1}, {"moab", 1}, {"fort", 1}, {"queima", J::array({5, 6})}}},
        {18, {{"cad", 0.5556}}},
        {20, {{"h3", {{"valor", 350}, {"sdano", 350}}}, {"h10", {{"valor", 600}}}}},
    };
    t.hab3 = H("Espada Saltitante", "dano_forte", 30, {{"valor", 20}, {"moab_mais", 60}, {"n", 1}, {"splash", 37.5}, {"sdano", 20}});
    // Sword Charge varre a trilha (60 de dano, 2 passadas de 120 no 16, 3 de 200 no 20)
    t.hab10 = H("Investida da Espada", "dano_global", 45, {{"valor", 60}});
    return t;
}

static DefTorre h_psi() {
    DefTorre t("psi", "Psi", 1000, "u", 9999);
    // a vibracao psionica destroi o bloon sem filhos depois de um tempo; aqui vira dano alto (aprox.)
    t.ataques = {A("hitscan", {{"cad", 1.0}, {"dano", 5}, {"pierce", 1}, {"dtype", "energia"}, {"global_", true}, {"visual", "psi"}})};
    t.camo = true;
    t.cor = {160, 90, 200};
    t.heroi = true;
    t.xp_escala = 1.5;
    t.titulo = "Macaco Psíquico";
    const J mente = A("hitscan", {{"cad", 1.0}, {"dano", 5}, {"pierce", 1}, {"dtype", "energia"}, {"global_", true}, {"visual", "psi"}});
    t.niveis = {
        {2, {{"a", "todos"}, {"dano", 2}}},
        {5, {{"a", "todos"}, {"cad", 0.85}}},
        {9, {{"novo", mente}}},
        {11, {{"a", "todos"}, {"dtype", "normal"}}},
        {13, {{"a", "todos"}, {"dano", 5}}},
        {14, {{"a", "todos"}, {"moab", 200}}},
        {16, {{"a", "todos"}, {"dano", 4}, {"moab", 500}}},
        {17, {{"novo", mente}}},
        {20, {{"a", "todos"}, {"moab", 1000}}},
    };
    // Psychic Blast: atordoa uma camada por 6 s (1,5 s em dirigivel)
    t.hab3 = H("Explosão Psíquica", "dano_global", 45, {{"valor", 0}, {"atordoa", 6}});
    t.hab10 = H("Grito Psiônico", "dano_global", 60, {{"valor", 20}, {"atordoa", 2}});
    return t;
}

static DefTorre h_geraldo() {
    DefTorre t("geraldo", "Geraldo", 750, "u", 160);
    t.ataques = {A("projetil", {{"cad", 1.1}, {"dano", 1}, {"pierce", 1}, {"vel", 2000}, {"dist", 260}, {"dtype", "energia"}, {"splash", 25}, {"sdano", 1}, {"spierce", 4}, {"sdtype", "energia"}, {"visual", "relampago"}})};
    t.cor = {120, 70, 40};
    t.heroi = true;
    t.xp_escala = 1.0;
    t.titulo = "Comerciante Místico";
    t.niveis = {
        {3, {{"alcance", 20}}},
        {5, {{"cad", 0.9091}}},
        {7, {{"dano", 1}, {"sdano", 1}, {"spierce", 5}}},
        {9, {{"splash", 7.5}}},
        {11, {{"spierce", 10}}},
        {17, {{"dano", 1}, {"sdano", 1}}},
        {19, {{"cad", 0.8}, {"alcance", 12}, {"splash", 12.5}, {"spierce", 5}, {"dano", 1}, {"sdano", 1}}},
        {20, {{"dano", 5}, {"sdano", 5}}},
    };
    // o Geraldo do BTD6 nao tem habilidades, e sim uma loja; a Torreta e o Espinho de Laminas sao itens da loja (aprox.)
    t.hab3 = H("Torreta Atiradora", "invocar", 40, {{"dur", 25}, {"base", "sentinela"}});
    t.hab10 = H("Armadilha de Lâminas", "spikes_local", 50, {{"valor", 400}, {"dano", 4}, {"dur", 20}});
    return t;
}

static DefTorre h_corvus() {
    DefTorre t("corvus", "Corvus", 1325, "u", 170);
    // o espirito e uma sub-torre voadora; aqui e um projetil teleguiado. Recarga aprox.
    t.ataques = {A("projetil", {{"cad", 0.6}, {"dano", 1}, {"pierce", 4}, {"vel", 800}, {"dist", 280}, {"busca", true}, {"dtype", "energia"}, {"visual", "espirito"}})};
    t.cor = {40, 40, 80};
    t.heroi = true;
    t.xp_escala = 1.425;
    t.titulo = "Guardião das Almas";
    t.niveis = {
        {2, {{"pierce", 2}}},
        {4, {{"dano", 1}}},
        {6, {{"dano", 1}}},
        {9, {{"dano", 2}, {"moab", 1}}},
        {11, {{"h3", {{"valor", 30}, {"cer_mais", 10}, {"recarga", 36}}}}},
        {14, {{"dano", 5}, {"moab", 4}}},
        {16, {{"dano", 10}}},
        {20, {{"dano", 20}, {"moab", 5}, {"h3", {{"recarga", 31.5}}}}},
    };
    t.hab3 = H("Colheita de Almas", "dano_forte", 40, {{"valor", 20}, {"n", 20}});
    // Dark Ritual: 1 de dano a cada 0,2 s por 10 s em ate 100 bloons; aqui um golpe unico (aprox.)
    t.hab10 = H("Ritual Sombrio", "dano_global", 90, {{"valor", 50}});
    return t;
}

static DefTorre h_rosalia() {
    DefTorre t("rosalia", "Rosalia", 875, "u", 160);
    // so o laser (a granada e uma troca manual de arma no BTD6); o tiro reforcado a cada 10 vira critico
    t.ataques = {A("projetil", {{"cad", 1.0}, {"dano", 3}, {"pierce", 3}, {"vel", 1000}, {"dist", 320}, {"dtype", "energia"}, {"visual", "laser"}})};
    t.cor = {200, 90, 120};
    t.heroi = true;
    t.xp_escala = 1.425;
    t.titulo = "Engenheira com Jetpack";
    t.niveis = {
        {5, {{"dano", 2}, {"pierce", 3}, {"crit_cada", 10}, {"crit_dano", 25}}},
        {8, {{"moab", 5}}},
        {9, {{"h3", {{"sdano", 150}}}}},
        {12, {{"dano", 5}, {"moab", 5}}},
        {13, {{"pierce", 4}, {"dtype", "normal"}}},
        {14, {{"h3", {{"valor", 10}, {"sdano", 300}, {"moab_mais", 5}}}}},
        {15, {{"moab", 20}}},
        {16, {{"h3", {{"recarga", 30}}}, {"h10", {{"recarga", 60}}}}},
        {19, {{"dano", 5}, {"moab", 15}}},
        {20, {{"h3", {{"sdano", 900}}}, {"h10", {{"valor", 4000}, {"sdano", 4000}}}}},
    };
    // Scatter Missile: 20 misseis de 5 em volta do alvo; aqui a soma numa explosao
    t.hab3 = H("Míssil de Dispersão", "dano_forte", 45, {{"valor", 5}, {"n", 1}, {"splash", 45}, {"sdano", 100}, {"atordoa", 1}});
    t.hab10 = H("Carga Cinética", "dano_forte", 75, {{"valor", 1500}, {"n", 1}, {"moab_so", true}, {"splash", 125}, {"sdano", 1500}});
    return t;
}

static DefTorre h_dan() {
    DefTorre t("dan", "Dan D'Monke", 650, "u", 120);
    t.ataques = {A("projetil", {{"cad", 0.5}, {"dano", 1}, {"pierce", 4}, {"vel", 2000}, {"dist", 150}, {"raio_proj", 6}, {"visual", "espadas"}})};
    t.cor = {110, 80, 50};
    t.heroi = true;
    t.xp_escala = 1.425;
    t.titulo = "Macaco da Corte";
    t.niveis = {
        {2, {{"pierce", 2}}},
        {4, {{"dano", 1}}},
        {6, {{"dano", 1}}},
        {11, {{"alcance", 18}, {"pierce", 2}}},
        {15, {{"pierce", 2}}},
        {16, {{"dano", 1}}},
        {17, {{"h3", {{"dur", 15}}}, {"h10", {{"recarga", 45}}}}},
        {19, {{"dano", 2}}},
    };
    // Transformation: troca de forma e ganha x0,75 de recarga por 9 s (o raio e a outra forma nao entram)
    t.hab3 = H("Transformação", "turbo", 20, {{"dur", 9}, {"valor", 0.75}});
    // Rabble Rouser: x1,5 de dano e x1,2 de recarga por 10 s; o dano vira +1 (aprox.)
    t.hab10 = H("Agitador", "turbo_area", 60, {{"dur", 10}, {"valor", 1.2}, {"buffs", {{"dano", 1}}}});
    return t;
}

static DefTorre h_silas() {
    DefTorre t("silas", "Silas", 850, "u", 220);
    // o orbe congela e desacelera; o dano vem da explosao (1, depois 5, 10 e 20)
    t.ataques = {A("projetil", {{"cad", 1.4}, {"dano", 1}, {"pierce", 10}, {"vel", 700}, {"dist", 400}, {"busca", true}, {"dtype", "gelo"}, {"lento", J::array({0.5, 3})}, {"congela", 2.5}, {"visual", "gelo_bola"}})};
    t.camo = true;
    t.cor = {120, 190, 230};
    t.heroi = true;
    t.xp_escala = 1.5;
    t.titulo = "Mago do Gelo";
    // Arctic Wind: 15% de lentidao em volta dele
    const J vento = A("aura", {{"cad", 0.5}, {"dano", 0}, {"pierce", 999}, {"raio_aura", 120}, {"lento", J::array({0.85, 0.6})}, {"visual", "nenhum"}});
    t.niveis = {
        {2, {{"novo", vento}}},
        {9, {{"h3", {{"buffs", {{"dano", 15}, {"dtype_normal", true}}}}}}},
        {11, J::array({{{"moab_lento", true}}, {{"a", "aura"}, {"lento", J::array({0.8, 0.6})}, {"moab_lento", true}}})},
        {12, {{"dano", 4}}},
        {15, {{"h3", {{"dur", 12}, {"buffs", {{"dano", 25}, {"dtype_normal", true}}}}}}},
        {16, {{"dano", 5}}},
        {18, J::array({{{"dano", 10}}, {{"a", "aura"}, {"lento", J::array({0.7, 0.6})}}})},
        {20, {{"h3", {{"buffs", {{"dano", 50}, {"dtype_normal", true}}}}}}},
    };
    // Frostbite: dano extra por segundo de congelamento restante; aqui +10 de dano por 8 s (aprox.)
    t.hab3 = H("Queimadura de Gelo", "turbo_area", 30, {{"dur", 8}, {"filtro", "silas"}, {"buffs", {{"dano", 10}, {"dtype_normal", true}}}});
    t.hab10 = H("Sepultura Gelada", "congelar_global", 90, {{"dur", 6}, {"moab", true}});
    return t;
}

}  // namespace

// ================================================================ BLOONS
std::vector<TipoBloon> criar_bloons() {
    std::vector<TipoBloon> v;
    v.push_back(TipoBloon{"vermelho", "Vermelho", {225, 38, 38}, 12.0, 1.0, 1, {}, 0, false, true, false, 1, 0});
    v.push_back(TipoBloon{"azul", "Azul", {40, 140, 235}, 13.0, 1.4, 1, {"vermelho"}, 0, false, true, false, 2, 0});
    v.push_back(TipoBloon{"verde", "Verde", {60, 190, 60}, 13.5, 1.8, 1, {"azul"}, 0, false, true, false, 3, 0});
    v.push_back(TipoBloon{"amarelo", "Amarelo", {250, 220, 30}, 14.0, 3.2, 1, {"verde"}, 0, false, true, false, 4, 0});
    v.push_back(TipoBloon{"rosa", "Rosa", {250, 110, 170}, 14.5, 3.5, 1, {"amarelo"}, 0, false, true, false, 5, 0});
    v.push_back(TipoBloon{"preto", "Preto", {30, 30, 34}, 9.5, 1.8, 1, {"rosa", "rosa"}, DT_EXPLOSAO, false, true, false, 6, 0});
    v.push_back(TipoBloon{"branco", "Branco", {245, 245, 245}, 9.5, 2.0, 1, {"rosa", "rosa"}, DT_GELO, false, false, false, 6, 0});
    v.push_back(TipoBloon{"roxo", "Roxo", {150, 60, 200}, 13.0, 3.0, 1, {"rosa", "rosa"}, DT_ENERGIA, false, true, false, 6, 0});
    v.push_back(TipoBloon{"chumbo", "Chumbo", {125, 130, 140}, 14.0, 1.0, 1, {"preto", "preto"}, DT_AFIADO | DT_GELO | DT_ENERGIA, false, true, false, 7, 4});
    v.push_back(TipoBloon{"zebra", "Zebra", {230, 230, 230}, 14.0, 1.8, 1, {"preto", "branco"}, DT_EXPLOSAO | DT_GELO, false, false, false, 7, 0});
    v.push_back(TipoBloon{"arco_iris", "Arco-íris", {255, 140, 0}, 15.0, 2.2, 1, {"zebra", "zebra"}, 0, false, true, false, 8, 0});
    v.push_back(TipoBloon{"ceramica", "Cerâmica", {170, 100, 45}, 15.5, 2.5, 10, {"arco_iris", "arco_iris"}, 0, false, true, false, 9, 20});
    v.push_back(TipoBloon{"moab", "M.O.A.B.", {50, 110, 220}, 42.0, 1.0, 200, {"ceramica", "ceramica", "ceramica", "ceramica"}, 0, true, false, false, 10, 400});
    v.push_back(TipoBloon{"bfb", "B.F.B.", {200, 40, 40}, 55.0, 0.25, 700, {"moab", "moab", "moab", "moab"}, 0, true, false, false, 11, 1400});
    v.push_back(TipoBloon{"zomg", "Z.O.M.G.", {40, 120, 40}, 66.0, 0.18, 4000, {"bfb", "bfb", "bfb", "bfb"}, 0, true, false, false, 12, 8000});
    v.push_back(TipoBloon{"ddt", "D.D.T.", {45, 45, 50}, 40.0, 2.64, 400, {"ceramica", "ceramica", "ceramica", "ceramica"}, DT_AFIADO | DT_EXPLOSAO | DT_GELO | DT_ENERGIA, true, false, true, 12, 800});
    v.push_back(TipoBloon{"bad", "B.A.D.", {120, 40, 150}, 80.0, 0.18, 20000, {"zomg", "zomg", "ddt", "ddt", "ddt"}, 0, true, false, false, 13, 40000});
    return v;
}

const std::map<std::string, std::string> REGEN_PROXIMO_DADOS = {
    {"vermelho", "azul"},
    {"azul", "verde"},
    {"verde", "amarelo"},
    {"amarelo", "rosa"},
    {"rosa", "preto"},
    {"preto", "zebra"},
    {"branco", "zebra"},
    {"roxo", "zebra"},
    {"zebra", "arco_iris"},
    {"arco_iris", "ceramica"},
};

std::vector<DefTorre> criar_torres() {
    return {t_dardo(), t_bumerangue(), t_bomba(), t_tachinha(), t_gelo(), t_cola(), t_sniper(), t_submarino(), t_bucaneiro(), t_as(), t_heli(), t_morteiro(), t_dartling(), t_mago(), t_super(), t_ninja(), t_alquimista(), t_druida(), t_fazenda(), t_espinhos(), t_vila(), t_engenheiro()};
}

std::vector<DefTorre> criar_auxiliares() {
    return {aux_sentinela(), aux_fenix()};
}

std::vector<DefTorre> criar_herois() {
    return {h_quincy(), h_gwendolin(), h_striker(), h_obyn(), h_churchill(), h_benjamin(), h_ezili(), h_pat(), h_adora(), h_brickell(), h_etienne(), h_sauda(), h_psi(), h_geraldo(), h_corvus(), h_rosalia(), h_dan(), h_silas()};
}

const std::map<std::string, std::string>& regen_proximo() { return REGEN_PROXIMO_DADOS; }

// XP acumulado para chegar a cada nivel (indice = nivel), antes da escala do heroi. Soma da tabela do BTD6
// (Blooncyclopedia, "Module:BTD6 hero xp"): 180, 460, 1000, 1860, 3280, 5180, 8320, 9380, 13620, 16380,
// 14400, 16650, 14940, 16380, 17820, 19260, 20700, 16470, 17280.
const std::vector<int> XP_NIVEL = {0,     0,     180,   640,    1640,   3500,   6780,   11960,  20280,  29660, 43280,
                                   59660, 74060, 90710, 105650, 122030, 139850, 159110, 179810, 196280, 213560};

// ================================================================ MAPAS
std::vector<DefMapa> criar_mapas() {
    std::vector<DefMapa> v;
    {
        DefMapa m;
        m.chave = "prado";
        m.nome = "Prado dos Macacos";
        m.dificuldade = "Iniciante";
        m.trilhas = {{{-40, 330}, {170, 330}, {170, 120}, {420, 120}, {420, 575}, {240, 575}, {240, 440}, {640, 440}, {640, 190}, {860, 190}, {860, 610}, {560, 610}, {560, 760}}};
        m.obstaculos = {{60, 110, 34, "arvore"}, {300, 300, 30, "arvore"}, {980, 80, 36, "arvore"}, {760, 350, 26, "pedra"}, {80, 620, 40, "arvore"}, {960, 470, 30, "arvore"}, {330, 670, 24, "pedra"}};
        m.grama = {98, 170, 58};
        m.terra = {196, 160, 104};
        v.push_back(m);
    }
    {
        DefMapa m;
        m.chave = "lago";
        m.nome = "Lago Sereno";
        m.dificuldade = "Iniciante";
        m.trilhas = {{{-40, 150}, {260, 150}, {260, 60}, {800, 60}, {800, 150}, {960, 150}, {960, 620}, {700, 620}, {700, 520}, {330, 520}, {330, 640}, {80, 640}, {80, 330}, {-40, 330}}};
        m.agua = {{530, 300, 150}};
        m.obstaculos = {{150, 470, 36, "arvore"}, {1010, 40, 22, "pedra"}, {560, 680, 26, "arvore"}, {880, 400, 30, "arvore"}};
        m.grama = {92, 168, 70};
        m.terra = {196, 160, 104};
        v.push_back(m);
    }
    {
        DefMapa m;
        m.chave = "encruzilhada";
        m.nome = "Encruzilhada";
        m.dificuldade = "Intermediário";
        m.trilhas = {{{-40, 200}, {300, 200}, {300, 420}, {700, 420}, {700, 120}, {1080, 120}}, {{-40, 560}, {400, 560}, {400, 330}, {820, 330}, {820, 640}, {1080, 640}}};
        m.agua_ret = {{480, 470, 170, 110}};
        m.obstaculos = {{150, 380, 34, "arvore"}, {560, 240, 30, "pedra"}, {950, 380, 40, "arvore"}, {150, 60, 28, "arvore"}, {600, 680, 22, "pedra"}};
        m.grama = {110, 176, 64};
        m.terra = {186, 150, 96};
        v.push_back(m);
    }
    {
        DefMapa m;
        m.chave = "espiral";
        m.nome = "Espiral da Selva";
        m.dificuldade = "Avançado";
        m.trilhas = {{{520, -40}, {520, 60}, {940, 60}, {940, 660}, {100, 660}, {100, 140}, {820, 140}, {820, 560}, {220, 560}, {220, 250}, {700, 250}, {700, 450}, {380, 450}, {380, 360}}};
        m.agua = {{540, 350, 45}};
        m.obstaculos = {{40, 40, 30, "arvore"}, {1000, 700, 30, "arvore"}, {990, 300, 24, "pedra"}};
        m.grama = {84, 150, 56};
        m.terra = {170, 130, 84};
        v.push_back(m);
    }
    return v;
}

// ================================================================ RODADAS
// Rodadas 1 a 140 do BTD6 (JSON:Bloons TD 6/Rounds/DefaultRoundSet na Blooncyclopedia):
// {tipo, qtd, espaco (s), inicio (s), camo, regen, fort}
const std::map<int, std::vector<Grupo>> RODADAS = {
    {1, {{"vermelho", 20, 0.9217, 0.0, false, false, false}}},
    {2, {{"vermelho", 35, 0.5588, 0.0, false, false, false}}},
    {3, {{"vermelho", 10, 0.5667, 0.0, false, false, false}, {"azul", 5, 0.562, 5.7, false, false, false}, {"vermelho", 15, 0.5, 9.71, false, false, false}}},
    {4, {{"vermelho", 25, 0.5, 0.0, false, false, false}, {"azul", 18, 0.1471, 7.9, false, false, false}, {"vermelho", 10, 0.3111, 14.51, false, false, false}}},
    {5, {{"azul", 12, 0.4672, 0.0, false, false, false}, {"vermelho", 5, 0.571, 5.7, false, false, false}, {"azul", 15, 0.5643, 8.6, false, false, false}}},
    {6, {{"verde", 4, 0.57, 0.0, false, false, false}, {"vermelho", 15, 0.3571, 5.33, false, false, false}, {"azul", 15, 0.5643, 10.8, false, false, false}}},
    {7, {{"azul", 10, 0.571, 0.0, false, false, false}, {"verde", 5, 1.2375, 5.7, false, false, false}, {"vermelho", 20, 0.5705, 11.812, false, false, false}, {"azul", 10, 0.4434, 22.81, false, false, false}}},
    {8, {{"azul", 20, 0.5705, 0.0, false, false, false}, {"verde", 2, 0.57, 11.42, false, false, false}, {"vermelho", 10, 0.2189, 14.03, false, false, false}, {"verde", 12, 0.9636, 18.272, false, false, false}}},
    {9, {{"verde", 30, 0.6534, 0.0, false, false, false}}},
    {10, {{"azul", 60, 0.5932, 0.0, false, false, false}, {"azul", 20, 0.4737, 35.0, false, false, false}, {"azul", 22, 0.1899, 44.0, false, false, false}}},
    {11, {{"amarelo", 3, 0.5, 0.0, false, false, false}, {"verde", 12, 0.571, 4.47, false, false, false}, {"azul", 10, 0.4222, 10.87, false, false, false}, {"vermelho", 10, 0.5078, 14.59, false, false, false}}},
    {12, {{"verde", 10, 0.57, 0.0, false, false, false}, {"azul", 15, 0.4336, 5.7, false, false, false}, {"amarelo", 5, 0.7792, 14.27, false, false, false}}},
    {13, {{"azul", 50, 0.6122, 0.0, false, false, false}, {"verde", 23, 1.3636, 2.21, false, false, false}}},
    {14, {{"vermelho", 18, 0.571, 0.0, false, false, false}, {"azul", 5, 0.2792, 2.854, false, false, false}, {"verde", 5, 0.279, 5.71, false, false, false}, {"amarelo", 4, 0.3167, 8.564, false, false, false}, {"vermelho", 31, 0.571, 9.5, false, false, false}, {"azul", 10, 0.1533, 15.96, false, false, false}, {"verde", 5, 0.3468, 19.8375, false, false, false}, {"amarelo", 5, 0.3885, 23.708, false, false, false}}},
    {15, {{"vermelho", 20, 1.3158, 0.0, false, false, false}, {"azul", 15, 1.4286, 2.78, false, false, false}, {"verde", 12, 1.3636, 5.68, false, false, false}, {"amarelo", 10, 1.3333, 8.87, false, false, false}, {"rosa", 5, 0.75, 17.55, false, false, false}}},
    {16, {{"verde", 20, 0.571, 0.0, false, false, false}, {"verde", 20, 0.571, 0.2, false, false, false}, {"amarelo", 8, 0.2048, 14.59, false, false, false}}},
    {17, {{"amarelo", 12, 0.4545, 0.0, false, true, false}}},
    {18, {{"verde", 60, 0.4237, 0.0, false, false, false}, {"verde", 20, 0.0956, 25.0, false, false, false}}},
    {19, {{"verde", 10, 0.3208, 0.0, false, false, false}, {"amarelo", 5, 0.5833, 2.855, false, true, false}, {"rosa", 15, 0.544, 5.9583, false, false, false}, {"amarelo", 4, 0.7613, 13.4792, false, false, false}}},
    {20, {{"preto", 6, 1.0492, 0.0, false, false, false}}},
    {21, {{"amarelo", 40, 0.2564, 0.09, false, false, false}, {"rosa", 10, 0.571, 10.84, false, false, false}, {"rosa", 4, 0.4792, 16.68, false, false, false}}},
    {22, {{"branco", 16, 0.5333, 0.0, false, false, false}}},
    {23, {{"preto", 7, 0.4167, 0.0, false, false, false}, {"branco", 7, 0.3333, 4.82, false, false, false}}},
    {24, {{"verde", 1, 1.0, 0.0, true, false, false}, {"azul", 20, 0.3158, 3.0, false, false, false}}},
    {25, {{"amarelo", 25, 0.5833, 0.0, false, true, false}, {"roxo", 10, 0.4444, 17.14, false, false, false}}},
    {26, {{"rosa", 23, 0.3955, 0.0, false, false, false}, {"zebra", 4, 1.142, 11.0833, false, false, false}}},
    {27, {{"vermelho", 100, 0.1212, 0.02, false, false, false}, {"azul", 60, 0.1356, 11.54, false, false, false}, {"verde", 45, 0.167, 19.05, false, false, false}, {"amarelo", 45, 0.1756, 26.53, false, false, false}}},
    {28, {{"chumbo", 6, 1.0, 0.0, false, false, false}}},
    {29, {{"amarelo", 50, 0.2245, 0.0, false, false, false}, {"amarelo", 15, 0.1786, 12.75, false, true, false}}},
    {30, {{"chumbo", 9, 1.6338, 0.0, false, false, false}}},
    {31, {{"preto", 8, 0.7265, 0.0, false, false, false}, {"branco", 8, 0.7265, 0.35, false, false, false}, {"zebra", 8, 0.7265, 6.16, false, false, false}, {"zebra", 2, 1.713, 14.2, false, true, false}}},
    {32, {{"preto", 15, 0.6429, 0.0, false, false, false}, {"branco", 20, 0.5789, 10.34, false, false, false}, {"roxo", 10, 0.6667, 21.96, false, false, false}}},
    {33, {{"vermelho", 20, 0.2632, 0.0, true, false, false}, {"amarelo", 13, 1.6667, 5.34, true, false, false}}},
    {34, {{"amarelo", 160, 0.2264, 0.0, false, false, false}, {"zebra", 6, 3.6167, 8.62, false, false, false}}},
    {35, {{"branco", 25, 0.4181, 0.0, false, false, false}, {"arco_iris", 5, 0.571, 14.3542, false, false, false}, {"rosa", 35, 0.2058, 19.2125, false, false, false}, {"preto", 30, 0.2412, 26.76, false, false, false}}},
    {36, {{"rosa", 40, 0.0388, 0.0, false, false, false}, {"verde", 10, 0.1495, 4.01, true, true, false}, {"rosa", 40, 0.0345, 8.49, false, false, false}, {"verde", 10, 0.1495, 12.67, true, true, false}, {"rosa", 60, 0.0678, 16.99, false, false, false}}},
    {37, {{"preto", 25, 0.452, 0.0, false, false, false}, {"branco", 25, 0.452, 11.42, false, false, false}, {"chumbo", 15, 0.571, 22.84, false, false, false}, {"zebra", 10, 0.571, 31.405, false, false, false}, {"branco", 7, 0.2653, 41.9167, true, false, false}}},
    {38, {{"branco", 17, 0.2247, 0.0, false, false, false}, {"rosa", 42, 0.571, 2.0, false, false, false}, {"chumbo", 14, 0.6588, 4.8333, false, false, false}, {"zebra", 10, 0.6979, 12.0875, false, false, false}, {"ceramica", 2, 2.0, 27.06, false, false, false}}},
    {39, {{"preto", 10, 0.571, 0.0, false, false, false}, {"branco", 10, 0.571, 5.71, false, false, false}, {"zebra", 20, 0.571, 12.79, false, false, false}, {"arco_iris", 18, 0.571, 24.79, false, false, false}, {"arco_iris", 2, 2.0, 35.93, false, true, false}}},
    {40, {{"moab", 1, 1.0, 0.0, false, false, false}}},
    {41, {{"preto", 60, 0.3181, 0.0, false, false, false}, {"zebra", 60, 0.4637, 18.8458, false, false, false}}},
    {42, {{"arco_iris", 6, 1.0, 0.0, false, true, false}, {"arco_iris", 5, 1.25, 6.6, true, false, false}}},
    {43, {{"arco_iris", 10, 0.571, 0.0, false, false, false}, {"ceramica", 7, 0.9517, 3.5542, false, false, false}}},
    {44, {{"zebra", 10, 1.1111, 0.0, false, false, false}, {"zebra", 10, 0.7778, 9.9167, false, false, false}, {"zebra", 10, 0.4444, 16.8333, false, false, false}, {"zebra", 10, 0.2222, 20.75, false, false, false}, {"zebra", 10, 0.1111, 22.6667, false, false, false}}},
    {45, {{"arco_iris", 25, 0.571, 0.0, false, false, false}, {"roxo", 10, 0.3333, 14.35, true, false, false}, {"rosa", 180, 0.1955, 18.08, false, false, false}, {"chumbo", 4, 0.8333, 50.6, false, false, true}}},
    {46, {{"ceramica", 6, 1.4, 0.0, false, false, true}}},
    {47, {{"ceramica", 12, 0.571, 0.0, false, false, false}, {"rosa", 70, 0.263, 6.5, true, false, false}}},
    {48, {{"rosa", 40, 0.3846, 0.0, false, true, false}, {"roxo", 30, 0.5172, 15.29, true, true, false}, {"arco_iris", 40, 0.5128, 30.54, false, false, false}, {"ceramica", 3, 2.0, 51.72, false, false, true}}},
    {49, {{"verde", 343, 0.1462, 0.0, false, false, false}, {"arco_iris", 10, 0.571, 4.0, false, false, false}, {"ceramica", 18, 0.3845, 15.5, false, false, false}, {"zebra", 20, 0.1849, 29.275, false, false, false}, {"arco_iris", 10, 0.571, 40.1, false, false, false}, {"arco_iris", 10, 0.571, 41.38, false, true, false}}},
    {50, {{"moab", 1, 1.0, 0.0, false, false, false}, {"chumbo", 8, 0.571, 3.06, false, false, true}, {"vermelho", 20, 0.0708, 9.887, false, false, false}, {"ceramica", 20, 0.571, 16.559, false, false, false}, {"moab", 1, 1.0, 27.9789, false, false, false}}},
    {51, {{"ceramica", 15, 1.2857, 0.0, true, false, false}, {"arco_iris", 10, 0.571, 19.0, false, true, false}}},
    {52, {{"arco_iris", 25, 0.571, 0.0, false, false, false}, {"moab", 1, 1.0, 14.2749, false, false, false}, {"ceramica", 5, 0.5711, 14.846, false, false, false}, {"moab", 1, 1.0, 17.701, false, false, false}, {"ceramica", 5, 0.571, 18.272, false, false, false}}},
    {53, {{"rosa", 80, 0.443, 0.0, true, false, false}, {"moab", 1, 1.0, 19.61, false, false, false}, {"moab", 1, 1.0, 27.07, false, false, false}, {"moab", 1, 1.0, 31.08, false, false, false}}},
    {54, {{"ceramica", 35, 0.571, 0.0, false, false, false}, {"moab", 1, 1.0, 6.2292, false, false, false}, {"moab", 1, 1.0, 11.991, false, false, false}}},
    {55, {{"ceramica", 10, 0.1293, 0.0, false, false, false}, {"ceramica", 10, 0.1634, 7.8333, false, false, false}, {"ceramica", 10, 0.1634, 14.416, false, false, false}, {"ceramica", 15, 0.0649, 22.1666, false, false, false}, {"moab", 1, 1.0, 28.7833, false, false, false}}},
    {56, {{"arco_iris", 40, 0.3077, 0.0, true, false, false}, {"moab", 1, 1.0, 15.1791, false, false, false}}},
    {57, {{"moab", 2, 0.571, 0.0, false, false, false}, {"arco_iris", 40, 0.6476, 0.97, false, false, false}, {"moab", 2, 0.571, 12.5619, false, false, false}}},
    {58, {{"moab", 5, 10.9958, 0.0, false, false, false}, {"ceramica", 15, 1.5714, 0.0, false, false, false}, {"ceramica", 10, 2.2222, 23.53, false, false, true}}},
    {59, {{"ceramica", 20, 0.5263, 0.0, false, false, false}, {"chumbo", 50, 0.1521, 10.166, true, false, false}, {"ceramica", 10, 0.8889, 18.16, false, true, false}}},
    {60, {{"bfb", 1, 1.0, 0.0, false, false, false}}},
    {61, {{"zebra", 150, 0.1342, 0.0, false, true, false}, {"moab", 5, 3.5083, 1.9167, false, false, false}}},
    {62, {{"roxo", 250, 0.1606, 0.0, false, false, false}, {"moab", 5, 5.0, 1.0833, false, false, false}, {"moab", 2, 5.0, 32.14, false, false, true}, {"arco_iris", 15, 0.571, 40.3, true, true, false}}},
    {63, {{"chumbo", 75, 0.571, 0.0, false, false, false}, {"ceramica", 40, 0.0051, 3.8792, false, false, false}, {"ceramica", 40, 0.0051, 20.0833, false, false, false}, {"ceramica", 42, 0.0049, 36.4166, false, false, false}}},
    {64, {{"moab", 6, 0.8, 0.0, false, false, false}, {"moab", 3, 2.0, 5.53, false, false, true}}},
    {65, {{"zebra", 100, 0.3269, 0.0, false, false, false}, {"arco_iris", 70, 0.2074, 32.5958, false, false, false}, {"ceramica", 50, 0.2214, 47.3167, false, false, false}, {"moab", 3, 0.285, 58.5666, false, false, false}, {"bfb", 2, 2.0, 60.0, false, false, false}}},
    {66, {{"moab", 2, 0.5, 0.0, false, false, false}, {"moab", 2, 0.5, 7.0, false, false, false}, {"moab", 4, 0.3333, 13.9167, false, false, false}, {"moab", 3, 1.0, 20.75, false, false, true}}},
    {67, {{"moab", 4, 0.7613, 0.0, false, false, false}, {"ceramica", 13, 0.6662, 9.0958, true, true, true}, {"moab", 4, 0.7613, 24.1542, false, false, false}}},
    {68, {{"moab", 4, 0.571, 0.0, false, false, false}, {"bfb", 1, 1.0, 7.4417, false, false, false}}},
    {69, {{"chumbo", 40, 0.4246, 0.0, false, false, true}, {"preto", 40, 0.2564, 5.46, false, true, false}, {"ceramica", 50, 0.5102, 17.13, false, false, false}}},
    {70, {{"arco_iris", 200, 0.201, 0.0, false, false, false}, {"branco", 120, 0.3193, 0.0, true, true, false}, {"moab", 4, 0.3807, 40.0, false, false, false}}},
    {71, {{"ceramica", 30, 0.5707, 0.0, false, false, false}, {"moab", 10, 0.5556, 4.4167, false, false, false}}},
    {72, {{"ceramica", 38, 0.5864, 0.0, false, true, false}, {"bfb", 1, 1.0, 3.4167, false, false, false}, {"bfb", 1, 1.0, 16.1167, false, false, false}}},
    {73, {{"moab", 7, 0.3807, 0.0, false, false, false}, {"bfb", 2, 3.0, 13.4, false, false, false}, {"moab", 1, 1.0, 26.3833, false, false, false}}},
    {74, {{"ceramica", 50, 0.4082, 0.0, false, false, false}, {"ceramica", 25, 0.8333, 21.14, true, true, true}, {"bfb", 1, 1.0, 43.93, false, false, false}, {"ceramica", 60, 0.4665, 54.86, false, false, true}}},
    {75, {{"bfb", 1, 1.0, 0.0, false, false, false}, {"chumbo", 14, 0.571, 0.571, false, false, false}, {"moab", 1, 1.0, 8.72, false, false, true}, {"bfb", 3, 0.05, 9.707, false, false, false}, {"chumbo", 14, 0.571, 10.277, false, false, true}, {"moab", 2, 3.0, 18.272, false, false, true}, {"bfb", 3, 0.05, 22.4875, false, false, false}}},
    {76, {{"ceramica", 60, 0.0301, 0.0, false, true, false}}},
    {77, {{"moab", 11, 5.8921, 0.0, false, false, false}, {"bfb", 5, 1.1385, 26.6667, false, false, false}}},
    {78, {{"arco_iris", 150, 0.604, 0.0, false, false, false}, {"ceramica", 75, 0.0158, 10.0, false, false, false}, {"bfb", 1, 1.0, 44.0833, false, false, false}, {"roxo", 80, 0.1899, 64.4, false, false, false}, {"ceramica", 72, 0.017, 77.9167, true, false, false}}},
    {79, {{"arco_iris", 500, 0.1202, 0.0, false, true, false}, {"bfb", 4, 11.6667, 3.1667, false, false, false}, {"bfb", 2, 10.0, 47.22, false, false, true}}},
    {80, {{"zomg", 1, 1.0, 0.0, false, false, false}}},
    {81, {{"bfb", 9, 2.5, 0.0, false, false, false}, {"bfb", 8, 0.7143, 21.47, false, false, false}}},
    {82, {{"bfb", 10, 2.2222, 0.0, false, false, false}, {"bfb", 5, 3.5, 21.68, false, false, true}}},
    {83, {{"ceramica", 40, 1.5385, 0.0, false, false, false}, {"ceramica", 40, 1.5385, 0.1, false, true, false}, {"ceramica", 40, 1.5385, 0.2, false, false, true}, {"moab", 30, 0.3448, 24.94, false, false, false}}},
    {84, {{"moab", 50, 0.5102, 0.0, false, false, false}, {"bfb", 10, 2.2222, 5.0, false, false, false}}},
    {85, {{"zomg", 2, 10.0, 0.0, false, false, false}}},
    {86, {{"bfb", 5, 5.25, -0.15, false, false, true}}},
    {87, {{"zomg", 4, 3.3333, 0.0, false, false, false}}},
    {88, {{"bfb", 8, 0.7143, 0.0, false, false, false}, {"moab", 18, 0.2941, 5.84, false, false, false}, {"zomg", 2, 3.0, 11.55, false, false, false}}},
    {89, {{"moab", 20, 0.5263, 0.0, false, false, true}, {"bfb", 8, 1.4286, 10.74, false, false, true}}},
    {90, {{"chumbo", 50, 0.2041, 0.0, true, true, true}, {"ddt", 3, 0.75, 10.4, true, false, false}}},
    {91, {{"ceramica", 100, 0.303, 0.0, false, false, true}, {"bfb", 20, 0.5263, 7.38, false, false, false}}},
    {92, {{"moab", 50, 0.7143, 0.0, false, false, true}, {"zomg", 4, 2.0, 12.96, false, false, false}}},
    {93, {{"bfb", 10, 2.2222, 0.0, false, false, true}, {"ddt", 6, 0.4, 12.35, true, false, false}}},
    {94, {{"bfb", 25, 0.625, 0.0, false, false, false}, {"zomg", 6, 2.6, 1.61, false, false, false}}},
    {95, {{"roxo", 500, 0.0601, 0.0, true, true, false}, {"chumbo", 250, 0.1205, 0.0, true, true, true}, {"moab", 50, 0.4082, 30.81, false, false, true}, {"ddt", 30, 0.6897, 30.81, true, false, false}}},
    {96, {{"bfb", 10, 0.5556, 0.0, false, false, false}, {"moab", 20, 0.2632, 5.5, false, false, true}, {"bfb", 10, 0.5556, 10.74, false, false, false}, {"moab", 20, 0.2632, 16.04, false, false, true}, {"bfb", 10, 0.5556, 21.62, false, false, false}, {"zomg", 6, 1.0, 27.12, false, false, false}}},
    {97, {{"zomg", 2, 5.0, 0.0, false, false, true}}},
    {98, {{"bfb", 30, 1.0345, 0.0, false, false, true}, {"zomg", 8, 0.7143, 0.0, false, false, false}}},
    {99, {{"moab", 60, 0.2034, 0.0, false, false, false}, {"ddt", 9, 0.625, 6.51, true, false, true}}},
    {100, {{"bad", 1, 1.0, 0.0, false, false, false}}},
    {101, {{"roxo", 250, 0.0241, 0.0, false, false, false}, {"moab", 10, 0.2222, 5.0, false, false, true}, {"ceramica", 50, 0.0204, 7.0, false, false, true}, {"roxo", 200, 0.0101, 7.5, false, false, false}}},
    {102, {{"zomg", 1, 1.0, 0.0, false, false, false}, {"zomg", 1, 1.0, 9.5, false, false, true}, {"ddt", 18, 1.8235, 10.0, true, false, false}, {"bfb", 3, 0.25, 12.0, false, false, false}, {"zomg", 2, 0.1, 19.0, false, false, true}, {"ddt", 12, 2.4545, 19.5, true, false, true}, {"bfb", 3, 0.5, 21.5, false, false, true}}},
    {103, {{"zomg", 10, 0.7778, 0.0, false, false, true}, {"moab", 100, 0.101, 5.0, false, false, false}, {"roxo", 64, 0.0159, 6.0, false, false, false}, {"moab", 50, 0.1735, 6.5, false, false, true}, {"roxo", 66, 0.0154, 12.0, false, false, false}, {"roxo", 68, 0.0149, 18.0, false, false, false}}},
    {104, {{"moab", 150, 0.1477, 0.0, false, false, true}, {"bfb", 10, 1.6667, 0.0, false, false, false}, {"chumbo", 100, 0.0253, 0.0, false, false, true}, {"bfb", 8, 1.6786, 2.5, false, false, false}, {"roxo", 100, 0.0253, 2.5, false, false, false}, {"bfb", 7, 1.6667, 5.0, false, false, false}, {"ceramica", 100, 0.0253, 5.0, false, false, false}, {"bfb", 5, 1.6875, 7.5, false, false, true}, {"chumbo", 100, 0.0253, 7.5, false, false, true}, {"bfb", 4, 1.6667, 10.0, false, false, true}, {"roxo", 100, 0.0253, 10.0, false, false, false}, {"bfb", 5, 0.625, 12.5, false, false, true}, {"ceramica", 100, 0.0253, 12.5, false, false, false}, {"moab", 25, 0.1667, 18.0, false, false, false}}},
    {105, {{"ceramica", 100, 0.0404, 0.0, false, false, false}, {"chumbo", 25, 0.375, 0.0, false, false, true}, {"bfb", 30, 0.1034, 4.0, false, false, false}, {"ceramica", 300, 0.0067, 7.0, false, false, true}}},
    {106, {{"ddt", 21, 1.5, 0.0, true, false, false}, {"ddt", 18, 1.4706, 5.0, true, false, false}, {"ddt", 15, 1.4286, 10.0, true, false, false}, {"ddt", 12, 1.3636, 15.0, true, false, true}, {"ddt", 9, 1.25, 20.0, true, false, true}, {"ddt", 6, 1.0, 25.0, true, false, true}, {"ddt", 12, 0.0455, 33.0, true, false, false}}},
    {107, {{"ceramica", 100, 0.0909, 0.0, false, false, true}, {"roxo", 444, 0.0203, 1.0, true, false, false}, {"zomg", 10, 0.5556, 2.0, false, false, true}}},
    {108, {{"zomg", 9, 1.875, 0.0, false, false, false}, {"zomg", 10, 1.1111, 14.0, false, false, true}}},
    {109, {{"zomg", 15, 1.2857, 0.0, false, false, false}, {"roxo", 15, 1.2857, 0.0, false, false, false}, {"bfb", 15, 1.2857, 0.0, false, false, false}, {"moab", 15, 1.2857, 0.0, false, false, true}, {"bfb", 15, 1.2857, 0.0, false, false, true}, {"moab", 15, 1.2857, 0.0, false, false, true}}},
    {110, {{"bfb", 25, 1.5, 0.0, false, false, false}, {"ddt", 9, 1.5, 0.0, true, false, false}, {"ddt", 9, 1.25, 12.0, true, false, false}, {"ddt", 6, 1.6, 22.0, true, false, true}, {"ddt", 6, 1.2, 30.0, true, false, true}}},
    {111, {{"zomg", 17, 0.8125, 0.0, false, false, false}, {"zomg", 9, 0.75, 7.0, false, false, true}, {"zomg", 5, 0.5, 16.5, false, false, false}}},
    {112, {{"bfb", 27, 0.7692, 0.0, false, false, true}, {"ddt", 21, 0.65, 7.0, true, false, true}}},
    {113, {{"moab", 75, 0.2365, 0.0, false, false, true}, {"bfb", 15, 1.0, 7.0, false, false, true}, {"ceramica", 42, 0.0366, 14.0, true, false, false}, {"ceramica", 42, 0.0366, 20.0, false, false, true}}},
    {114, {{"ddt", 9, 3.75, 0.0, true, false, false}, {"moab", 36, 0.2857, 0.0, false, false, true}, {"ddt", 6, 5.5, 2.5, true, false, true}, {"moab", 24, 0.4348, 5.0, false, false, false}, {"bfb", 12, 0.9091, 10.0, false, false, false}, {"bfb", 8, 1.4286, 15.0, false, false, true}, {"zomg", 5, 1.25, 20.0, false, false, false}, {"zomg", 3, 2.5, 25.0, false, false, true}}},
    {115, {{"ddt", 9, 3.75, 0.0, true, false, false}, {"zomg", 3, 2.5, 0.0, false, false, true}, {"ddt", 6, 5.5, 2.5, true, false, true}, {"bfb", 8, 1.4286, 5.0, false, false, true}, {"zomg", 5, 1.25, 5.0, false, false, false}, {"bfb", 12, 0.9091, 10.0, false, false, false}, {"moab", 24, 0.4348, 15.0, false, false, false}, {"moab", 36, 0.2857, 20.0, false, false, true}}},
    {116, {{"zomg", 8, 0.4286, 0.0, false, false, true}, {"bfb", 18, 0.5882, 5.0, false, false, true}, {"roxo", 100, 0.101, 5.0, false, false, false}, {"roxo", 100, 0.0707, 8.0, false, false, false}, {"roxo", 200, 0.0201, 11.0, false, false, false}}},
    {117, {{"chumbo", 250, 0.0562, 0.0, false, false, false}, {"ddt", 27, 0.4231, 3.0, true, false, false}, {"ddt", 18, 0.6471, 3.0, true, false, true}}},
    {118, {{"zomg", 12, 1.4545, 0.0, false, false, false}, {"ddt", 30, 0.2414, 7.5, true, false, true}}},
    {119, {{"bad", 2, 10.0, 0.0, false, false, false}, {"bad", 1, 1.0, 25.0, false, false, false}}},
    {120, {{"zomg", 12, 4.0909, 0.0, false, false, false}, {"bfb", 12, 4.0909, 1.5, false, false, false}, {"moab", 24, 1.9565, 6.0, false, false, true}}},
    {121, {{"bfb", 14, 0.3846, 0.0, false, false, true}, {"moab", 28, 0.1852, 5.0, false, false, true}, {"zomg", 6, 1.0, 10.0, false, false, true}}},
    {122, {{"bfb", 40, 0.0051, 0.0, false, false, false}, {"bfb", 20, 0.0, 0.1, false, false, true}, {"chumbo", 75, 0.2027, 5.0, false, false, true}, {"chumbo", 75, 0.2027, 5.0, false, false, true}, {"chumbo", 75, 0.2027, 5.0, false, false, true}}},
    {123, {{"zomg", 8, 5.7143, 0.0, false, false, true}, {"moab", 200, 0.201, 0.0, false, false, false}}},
    {124, {{"bfb", 75, 0.4054, 0.0, false, false, true}}},
    {125, {{"zomg", 6, 5.0, 0.0, false, false, false}, {"zomg", 5, 5.0, 5.0, false, false, false}, {"bfb", 12, 2.2727, 5.0, false, false, false}, {"bfb", 10, 2.2722, 9.55, false, false, false}, {"zomg", 4, 5.0, 10.0, false, false, false}, {"bfb", 8, 2.2714, 14.1, false, false, false}, {"moab", 18, 1.4706, 15.0, false, false, false}, {"zomg", 3, 5.0, 15.0, false, false, false}, {"bfb", 6, 2.27, 18.65, false, false, false}, {"moab", 15, 1.47, 19.42, false, false, false}, {"zomg", 2, 5.0, 20.0, false, false, false}, {"bfb", 4, 2.2667, 23.2, false, false, false}, {"moab", 12, 1.47, 23.83, false, false, false}, {"zomg", 1, 1.0, 25.0, false, false, false}, {"bfb", 2, 2.25, 27.75, false, false, false}, {"moab", 9, 1.4688, 28.25, false, false, false}, {"moab", 6, 1.466, 32.66, false, false, false}, {"moab", 3, 1.46, 37.08, false, false, false}}},
    {126, {{"chumbo", 1, 1.0, 0.0, true, true, true}, {"ddt", 99, 0.2143, 6.0, true, false, false}}},
    {127, {{"moab", 48, 0.1489, 0.0, false, false, false}, {"bfb", 24, 0.3913, 5.0, false, false, false}}},
    {128, {{"ddt", 39, 0.4737, 0.0, true, false, true}, {"ceramica", 200, 0.0226, 9.0, true, false, true}, {"bfb", 30, 0.6207, 10.0, false, false, false}}},
    {129, {{"zomg", 7, 2.8333, 0.0, false, false, true}, {"chumbo", 77, 0.0132, 4.0, true, false, true}, {"roxo", 77, 0.0132, 6.0, true, false, false}, {"ceramica", 77, 0.0132, 8.0, true, false, false}, {"zomg", 7, 0.0, 10.0, false, false, false}, {"ddt", 9, 0.0, 17.0, true, false, false}, {"ddt", 3, 0.5, 21.0, true, false, false}, {"ddt", 3, 0.5, 24.0, true, false, false}, {"ddt", 3, 0.5, 27.0, true, false, false}}},
    {130, {{"moab", 12, 0.0, 0.0, false, false, false}, {"ddt", 27, 0.6154, 4.0, true, false, false}, {"moab", 9, 0.0, 4.0, false, false, true}, {"moab", 12, 0.0, 8.0, false, false, false}, {"moab", 9, 0.0, 12.0, false, false, true}, {"moab", 12, 0.0, 16.0, false, false, false}, {"ddt", 9, 0.0, 20.0, true, false, false}, {"moab", 48, 0.3404, 24.0, false, false, false}, {"ddt", 3, 0.0, 24.0, true, false, true}, {"ddt", 6, 0.0, 28.0, true, false, false}, {"ddt", 3, 0.0, 32.0, true, false, true}, {"ddt", 6, 0.0, 36.0, true, false, false}, {"moab", 48, 0.0, 40.0, false, false, true}}},
    {131, {{"zomg", 18, 2.2941, 0.0, false, false, true}}},
    {132, {{"zomg", 5, 0.25, 0.0, false, false, false}, {"zomg", 4, 0.0, 5.0, false, false, false}, {"zomg", 3, 0.5, 10.0, false, false, true}, {"roxo", 100, 0.0101, 15.0, true, false, false}, {"zomg", 5, 0.25, 25.0, false, false, false}, {"zomg", 4, 0.0, 30.0, false, false, false}, {"zomg", 3, 0.5, 35.0, false, false, true}, {"roxo", 100, 0.0101, 40.0, true, false, false}}},
    {133, {{"moab", 6, 0.2, 0.0, false, false, true}, {"bfb", 6, 0.2, 0.0, false, false, true}, {"zomg", 2, 1.0, 0.0, false, false, true}, {"moab", 6, 0.2, 12.0, false, false, true}, {"bfb", 6, 0.2, 12.0, false, false, true}, {"zomg", 2, 1.0, 12.0, false, false, true}, {"moab", 9, 0.125, 24.0, false, false, false}, {"bfb", 9, 0.125, 24.0, false, false, false}, {"zomg", 3, 0.5, 24.0, false, false, false}, {"moab", 9, 0.125, 36.0, false, false, false}, {"bfb", 9, 0.125, 36.0, false, false, false}, {"zomg", 3, 0.5, 36.0, false, false, false}, {"moab", 9, 0.125, 42.0, false, false, false}, {"bfb", 9, 0.125, 42.0, false, false, false}, {"zomg", 3, 0.5, 42.0, false, false, false}}},
    {134, {{"bfb", 12, 2.1818, 0.0, false, false, true}, {"bfb", 10, 2.1822, 4.36, false, false, false}, {"bfb", 8, 2.18, 8.74, false, false, false}, {"bfb", 6, 2.18, 13.1, false, false, false}, {"bfb", 4, 2.1833, 17.46, false, false, false}}},
    {135, {{"zomg", 7, 4.0, 0.0, false, false, true}, {"zomg", 7, 4.0, 0.0, false, false, true}, {"ddt", 7, 4.0, 0.0, true, false, true}, {"ddt", 7, 4.0, 0.0, true, false, true}, {"ddt", 7, 4.0, 0.0, true, false, true}}},
    {136, {{"moab", 96, 0.4737, 0.0, false, false, true}, {"bfb", 24, 1.9565, 0.0, false, false, false}}},
    {137, {{"zomg", 18, 0.0059, 0.0, false, false, false}, {"bfb", 24, 0.0043, 2.0, false, false, false}, {"moab", 48, 0.0021, 8.0, false, false, false}}},
    {138, {{"ddt", 33, 1.25, 0.0, true, false, true}, {"ddt", 27, 1.2692, 7.0, true, false, true}, {"ddt", 21, 1.3, 14.0, true, false, true}, {"ddt", 18, 1.1176, 21.0, true, false, false}, {"ddt", 15, 0.8571, 28.0, true, false, false}, {"ddt", 12, 0.4545, 35.0, true, false, false}}},
    {139, {{"moab", 109, 0.4167, 0.0, false, false, false}, {"moab", 18, 0.2353, 5.0, false, false, false}, {"moab", 18, 0.2353, 9.0, false, false, true}, {"moab", 18, 0.4706, 13.0, false, false, false}, {"moab", 18, 0.2353, 21.0, false, false, true}, {"moab", 18, 0.2353, 25.0, false, false, false}, {"moab", 18, 0.2353, 29.0, false, false, true}, {"moab", 18, 0.4706, 33.0, false, false, false}, {"moab", 18, 0.2353, 41.0, false, false, true}}},
    {140, {{"bad", 1, 1.0, 0.0, false, false, true}, {"bad", 1, 1.0, 45.0, false, false, false}}},
};

// Dificuldades do modo solo: {chave, nome, vidas, multiplicador de custo, ultima rodada}
// Dificuldades e modos do BTD6 (Blooncyclopedia: "Easy", "Medium", "Hard", "Impoppable (BTD6)", "CHIMPS",
// "Half Cash" e "Deflation (BTD6)"). O Dificil comeca na R3 e o Impossivel e o CHIMPS na R6; no Dificil os
// bloons andam 13,6% mais rapido que no Medio e 25% mais que no Facil.
const std::vector<Dificuldade> DIFICULDADES = {
    {"facil", "Fácil", 200, 0.85, 40, 1, 1.0 / 1.1},
    {"medio", "Médio", 150, 1.0, 60, 1, 1.0},
    {"dificil", "Difícil", 100, 1.08, 80, 3, 1.25 / 1.1},
    {"impossivel", "Impossível", 1, 1.2, 100, 6, 1.25 / 1.1},
    {"chimps", "CHIMPS", 1, 1.08, 100, 6, 1.25 / 1.1, 650, 1.0, true, true},
    {"metade", "Meio Dinheiro", 100, 1.08, 80, 3, 1.25 / 1.1, 325, 0.5},
    {"deflacao", "Deflação", 200, 0.85, 60, 31, 1.0 / 1.1, 20000, 0.0},
};

// Envios do modo Batalha (inspirados no Battles 2): custo, efeito na renda (eco) e desbloqueio
const std::vector<Envio> ENVIOS = {
    {"r8", "8 Vermelhos", "vermelho", 8, 0.12, 25, 1.0, 1, false, false, false},
    {"b6", "6 Azuis", "azul", 6, 0.14, 30, 1.2, 1, false, false, false},
    {"g5", "5 Verdes", "verde", 5, 0.16, 40, 1.4, 2, false, false, false},
    {"y4", "4 Amarelos", "amarelo", 4, 0.18, 45, 1.6, 4, false, false, false},
    {"p4", "4 Rosas", "rosa", 4, 0.18, 55, 1.8, 6, false, false, false},
    {"yr", "6 Amarelos Regen", "amarelo", 6, 0.2, 70, 2.2, 7, false, true, false},
    {"gc", "6 Verdes Camo", "verde", 6, 0.2, 80, 2.4, 8, true, false, false},
    {"k4", "4 Pretos", "preto", 4, 0.3, 90, 2.6, 9, false, false, false},
    {"w4", "4 Brancos", "branco", 4, 0.3, 90, 2.6, 9, false, false, false},
    {"u4", "4 Roxos", "roxo", 4, 0.3, 110, 2.8, 11, false, false, false},
    {"l3", "3 Chumbos", "chumbo", 3, 0.4, 150, 3.2, 12, false, false, false},
    {"z3", "3 Zebras", "zebra", 3, 0.4, 150, 3.2, 13, false, false, false},
    {"ra2", "2 Arco-íris", "arco_iris", 2, 0.5, 220, 3.6, 15, false, false, false},
    {"rar", "3 Arco-íris Regen", "arco_iris", 3, 0.4, 350, 4.0, 17, false, true, false},
    {"c2", "2 Cerâmicas", "ceramica", 2, 0.5, 400, 4.0, 18, false, false, false},
    {"cc", "3 Cerâmicas Camo", "ceramica", 3, 0.5, 700, 4.5, 21, true, false, false},
    {"cf", "3 Cerâmicas Fortificadas", "ceramica", 3, 0.5, 850, 4.5, 23, false, false, true},
    {"m1", "M.O.A.B.", "moab", 1, 1.0, 1500, 0.0, 25, false, false, false},
    {"mf", "M.O.A.B. Fortificado", "moab", 1, 1.0, 2400, 0.0, 28, false, false, true},
    {"bfb", "B.F.B.", "bfb", 1, 1.0, 6000, -10.0, 32, false, false, false},
    {"ddt", "D.D.T.", "ddt", 1, 1.0, 6500, -10.0, 34, false, false, false},
    {"zomg", "Z.O.M.G.", "zomg", 1, 1.0, 18000, -30.0, 38, false, false, false},
    {"bad", "B.A.D.", "bad", 1, 1.0, 60000, -80.0, 45, false, false, false},
};

}  // namespace bl
