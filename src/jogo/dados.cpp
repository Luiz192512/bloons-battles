// Dados do jogo: bloons, 22 torres (3 caminhos x 5 upgrades), 18 herois, mapas e rodadas.
//
// Valores inspirados no Bloons TD Battles 2 (dificuldade Media), adaptados para esta
// simulacao. Alcances do original multiplicados por ~4 (px de um mapa 1040x720).
//
// Cada upgrade tem um objeto de efeitos aplicado por stats.cpp (aplicar):
//   aditivos:        dano, pierce, n, splash, sdano, spierce, quica, moab, cer, fort,
//                    dist, raio_proj, valor, pilha_pierce, saltos, empurra, alcance
//   multiplicativos: cad, vel, pilha_vida, alcance_x, valor_x, impreciso
//   definicao:       dtype, sdtype, camo, busca, global_, visual, lento, congela, cola,
//                    queima, atordoa, fragiliza, retira_camo, retira_regen, ouro, spread,
//                    frag, fusivel, boom
//   estruturais:     a (indice do ataque alvo, ou "todos"), novo (ataque novo),
//                    subst (substitui o ataque), hab (habilidade), buffs (para torres buff)

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
            U("Tiros Super Afiados", 220, "Dardos estouram +2 bloons.", {{"pierce", 2}}),
            U("Espinhopulta", 300, "Arremessa bolas de espinhos com grande perfuração.", {{"pierce", 18}, {"cad", 1.3}, {"raio_proj", 6}, {"vel", 0.7}, {"visual", "bola_espinho"}}),
            U("Juggernaut", 1800, "Bola gigante que estoura chumbo e cerâmica.", {{"dano", 1}, {"pierce", 50}, {"dtype", "normal"}, {"cer", 4}, {"raio_proj", 6}, {"visual", "juggernaut"}}),
            U("Ultra-Juggernaut", 15000, "Se parte em 6 mini juggernauts.", {{"dano", 3}, {"pierce", 100}, {"cer", 8}, {"raio_proj", 4}, {"frag", {{"n", 6}, {"dano", 2}, {"pierce", 30}, {"dtype", "normal"}, {"visual", "bola_espinho"}}}}),
        },
        {
            U("Tiros Rápidos", 100, "Atira mais rápido.", {{"cad", 0.85}}),
            U("Tiros Muito Rápidos", 190, "Atira ainda mais rápido.", {{"cad", 0.78}}),
            U("Tiro Triplo", 400, "Atira 3 dardos por vez.", {{"n", 2}, {"spread", 30}}),
            U("Fã-Clube Super Macaco", 8000, "Habilidade: dardos próximos viram Super Macacos.", {{"hab", H("Fã-Clube Super Macaco", "turbo_area", 50, {{"dur", 15}, {"valor", 0.08}, {"filtro", "dardo"}})}}),
            U("Fã-Clube Macaco Plasma", 45000, "Habilidade: vira Macacos Plasma.", {{"dano", 2}, {"pierce", 3}, {"dtype", "normal"}, {"hab", H("Fã-Clube Macaco Plasma", "turbo_area", 45, {{"dur", 15}, {"valor", 0.04}, {"filtro", "dardo"}})}}),
        },
        {
            U("Dardos de Longo Alcance", 90, "Mais alcance.", {{"alcance", 32}, {"dist", 60}}),
            U("Visão Aprimorada", 200, "Mais alcance e detecta camo.", {{"alcance", 16}, {"camo", true}}),
            U("Besta", 575, "Flechas mais fortes.", {{"alcance", 16}, {"dano", 2}, {"pierce", 1}, {"visual", "flecha"}}),
            U("Atirador Afiado", 2000, "Tiros críticos.", {{"cad", 0.6}, {"dano", 3}}),
            U("Mestre da Besta", 25000, "Rajadas de flechas que ricocheteiam.", {{"cad", 0.3}, {"dano", 5}, {"pierce", 4}, {"quica", 2}, {"alcance", 40}}),
        },
    };
    t.desc = "Atira dardos. Barato e versátil.";
    t.cor = {150, 95, 45};
    return t;
}

static DefTorre t_bumerangue() {
    DefTorre t("bumerangue", "Macaco Bumerangue", 325, "w", 172);
    t.ataques = {A("projetil", {{"cad", 1.2}, {"dano", 1}, {"pierce", 4}, {"vel", 520}, {"dist", 240}, {"boom", true}, {"raio_proj", 8}, {"visual", "bumerangue"}})};
    t.caminhos = {
        {
            U("Bumerangues Melhorados", 200, "", {{"pierce", 4}}),
            U("Glaives", 280, "Lâminas afiadas.", {{"pierce", 6}, {"visual", "glaive"}}),
            U("Ricochete de Glaive", 1300, "Glaives ricocheteiam entre bloons.", {{"pierce", 30}, {"quica", 6}}),
            U("M.O.A.R. Glaives", 3000, "Glaives que destroem M.O.A.B.s.", {{"pierce", 40}, {"dano", 1}, {"moab", 3}}),
            U("Senhor das Glaives", 32500, "Glaives orbitam o macaco.", {{"dano", 5}, {"moab", 10}, {"novo", A("aura", {{"cad", 0.1}, {"dano", 4}, {"pierce", 100}, {"dtype", "normal"}, {"raio_aura", 90}, {"visual", "orbita_glaive"}})}}),
        },
        {
            U("Arremesso Rápido", 175, "", {{"cad", 0.75}}),
            U("Bumerangues Velozes", 250, "", {{"vel", 1.3}, {"cad", 0.9}}),
            U("Bumerangue Biônico", 1600, "Braço biônico.", {{"cad", 0.4}, {"moab", 2}}),
            U("Turbo Carga", 4000, "Habilidade: velocidade extrema.", {{"cad", 0.8}, {"hab", H("Turbo Carga", "turbo", 45, {{"dur", 10}, {"valor", 0.25}})}}),
            U("Carga Permanente", 35000, "Turbo permanente.", {{"cad", 0.35}, {"dano", 5}}),
        },
        {
            U("Bumerangues de Longo Alcance", 100, "", {{"alcance", 24}, {"dist", 40}}),
            U("Bumerangues Incandescentes", 300, "Estouram chumbo.", {{"dano", 1}, {"dtype", "normal"}}),
            U("Bumerangue Kylie", 1300, "Segue a trilha.", {{"pierce", 20}, {"dist", 220}, {"visual", "kylie"}}),
            U("Prensa de M.O.A.B.", 2200, "Empurra dirigíveis.", {{"moab", 4}, {"empurra", 40}}),
            U("Dominação M.O.A.B.", 50000, "Esmaga dirigíveis.", {{"dano", 20}, {"moab", 30}, {"empurra", 80}, {"pierce", 30}}),
        },
    };
    t.desc = "Bumerangues vão e voltam.";
    t.cor = {170, 110, 50};
    return t;
}

static DefTorre t_bomba() {
    DefTorre t("bomba", "Canhão Bomba", 525, "e", 160);
    t.ataques = {A("projetil", {{"cad", 1.5}, {"dano", 1}, {"pierce", 1}, {"vel", 600}, {"dist", 260}, {"visual", "bomba"}, {"raio_proj", 7}, {"splash", 45}, {"sdano", 1}, {"spierce", 14}, {"sdtype", "explosao"}})};
    t.caminhos = {
        {
            U("Bombas Maiores", 350, "", {{"splash", 12}, {"spierce", 10}}),
            U("Bombas Pesadas", 650, "", {{"sdano", 1}, {"spierce", 8}}),
            U("Bombas Muito Grandes", 1100, "", {{"splash", 20}, {"spierce", 20}, {"sdano", 1}}),
            U("Impacto Bloon", 3600, "Atordoa bloons.", {{"atordoa", 1.0}, {"sdano", 1}}),
            U("Esmaga Bloon", 55000, "Explosão devastadora.", {{"sdano", 8}, {"splash", 30}, {"spierce", 200}, {"atordoa", 2.0}, {"cer", 10}, {"moab", 10}}),
        },
        {
            U("Recarga Rápida", 250, "", {{"cad", 0.75}}),
            U("Lança-Mísseis", 400, "Mísseis rápidos.", {{"cad", 0.85}, {"vel", 1.6}, {"alcance", 16}, {"visual", "missil"}}),
            U("Destruidor de M.O.A.B.", 1100, "", {{"moab", 15}}),
            U("Assassino de M.O.A.B.", 3200, "Habilidade: míssil anti dirigível.", {{"moab", 15}, {"hab", H("Míssil Assassino", "dano_forte", 30, {{"valor", 750}, {"n", 1}, {"moab_so", true}})}}),
            U("Eliminador de M.O.A.B.", 25000, "", {{"moab", 100}, {"hab", H("Míssil Eliminador", "dano_forte", 10, {{"valor", 4500}, {"n", 1}, {"moab_so", true}})}}),
        },
        {
            U("Alcance Extra", 200, "", {{"alcance", 28}, {"dist", 40}}),
            U("Bombas de Fragmentação", 300, "Soltam fragmentos.", {{"frag", {{"n", 8}, {"dano", 1}, {"pierce", 1}, {"dtype", "afiado"}, {"visual", "fragmento"}}}}),
            U("Bombas de Cacho", 800, "Soltam mini bombas.", {{"frag", {{"n", 8}, {"dano", 1}, {"pierce", 1}, {"splash", 30}, {"sdano", 1}, {"spierce", 8}, {"visual", "bomba"}}}}),
            U("Cacho Recursivo", 2800, "", {{"sdano", 1}, {"frag", {{"n", 10}, {"dano", 1}, {"pierce", 1}, {"splash", 34}, {"sdano", 2}, {"spierce", 10}, {"visual", "bomba"}}}}),
            U("Blitz de Bombas", 23000, "Habilidade: bombardeio geral.", {{"sdano", 2}, {"hab", H("Blitz de Bombas", "dano_global", 60, {{"valor", 1000}})}}),
        },
    };
    t.desc = "Bombas explodem em área.";
    t.cor = {60, 60, 70};
    return t;
}

static DefTorre t_tachinha() {
    DefTorre t("tachinha", "Atirador de Tachinhas", 280, "r", 92);
    t.ataques = {A("radial", {{"cad", 1.4}, {"dano", 1}, {"pierce", 1}, {"n", 8}, {"vel", 520}, {"dist", 120}, {"visual", "tachinha"}})};
    t.caminhos = {
        {
            U("Disparo Rápido", 150, "", {{"cad", 0.75}}),
            U("Disparo Mais Rápido", 300, "", {{"cad", 0.66}}),
            U("Tiros Quentes", 600, "Tachinhas de fogo estouram chumbo.", {{"dtype", "normal"}, {"dano", 1}, {"visual", "fogo"}}),
            U("Anel de Fogo", 3500, "Anel de chamas em volta.", {{"subst", A("aura", {{"cad", 0.3}, {"dano", 1}, {"pierce", 60}, {"dtype", "energia"}, {"raio_aura", 110}, {"visual", "anel_fogo"}})}}),
            U("Anel Infernal", 45500, "Meteoros e anel mais forte.", {{"dano", 3}, {"pierce", 80}, {"novo", A("projetil", {{"cad", 4.0}, {"dano", 700}, {"pierce", 1}, {"vel", 1500}, {"dist", 900}, {"dtype", "normal"}, {"splash", 60}, {"sdano", 30}, {"spierce", 20}, {"global_", true}, {"visual", "meteoro"}, {"raio_proj", 16}, {"alvo", "forte"}})}}),
        },
        {
            U("Tachinhas de Longo Alcance", 100, "", {{"alcance", 16}, {"dist", 30}}),
            U("Tachinhas de Super Alcance", 225, "", {{"alcance", 16}, {"dist", 30}}),
            U("Atirador de Lâminas", 550, "", {{"pierce", 3}, {"dano", 1}, {"visual", "lamina"}}),
            U("Turbilhão de Lâminas", 2700, "Habilidade: redemoinho de lâminas.", {{"hab", H("Turbilhão", "turbo", 20, {{"dur", 3}, {"valor", 0.05}})}}),
            U("Super Turbilhão", 15000, "", {{"pierce", 10}, {"dano", 2}, {"hab", H("Super Turbilhão", "turbo", 20, {{"dur", 9}, {"valor", 0.04}})}}),
        },
        {
            U("Mais Tachinhas", 100, "", {{"n", 2}}),
            U("Ainda Mais Tachinhas", 300, "", {{"n", 2}}),
            U("Pulverizador de Tachinhas", 600, "", {{"n", 4}, {"cad", 0.6}}),
            U("Sobrecarga", 3200, "", {{"cad", 0.33}}),
            U("Zona das Tachinhas", 24000, "", {{"n", 20}, {"pierce", 4}, {"cad", 0.5}, {"dano", 1}, {"alcance", 30}}),
        },
    };
    t.desc = "Dispara tachinhas em 8 direções.";
    t.cor = {170, 170, 180};
    return t;
}

static DefTorre t_gelo() {
    DefTorre t("gelo", "Macaco de Gelo", 500, "t", 80);
    t.ataques = {A("aura", {{"cad", 2.4}, {"dano", 1}, {"pierce", 40}, {"dtype", "gelo"}, {"congela", 1.5}, {"visual", "congelar"}})};
    t.caminhos = {
        {
            U("Permafrost", 100, "Bloons ficam lentos depois.", {{"lento", J::array({0.5, 2.5})}}),
            U("Estalo Frio", 350, "Congela camo e chumbo.", {{"camo", true}, {"dtype", "normal"}}),
            U("Estilhaços de Gelo", 1500, "Bloons congelados soltam estilhaços.", {{"frag", {{"n", 3}, {"dano", 1}, {"pierce", 2}, {"dtype", "afiado"}, {"visual", "fragmento_gelo"}}}}),
            U("Fragilização", 2200, "Bloons recebem dano extra.", {{"fragiliza", 1}}),
            U("Super Frágil", 28000, "", {{"fragiliza", 4}, {"dano", 2}, {"moab", 4}}),
        },
        {
            U("Congelamento Melhor", 225, "", {{"cad", 0.8}, {"congela", 2.0}}),
            U("Congelamento Profundo", 350, "", {{"congela", 2.5}, {"dano", 1}}),
            U("Vento Ártico", 2900, "Aura que desacelera tudo.", {{"alcance", 30}, {"novo", A("aura", {{"cad", 0.2}, {"dano", 0}, {"pierce", 999}, {"lento", J::array({0.4, 0.3})}, {"moab_lento", true}, {"visual", "vento"}})}}),
            U("Nevasca", 3000, "Habilidade: congela todos os bloons.", {{"hab", H("Nevasca", "congelar_global", 30, {{"dur", 3}})}}),
            U("Zero Absoluto", 26000, "", {{"hab", H("Zero Absoluto", "congelar_global", 20, {{"dur", 10}, {"moab", true}})}}),
        },
        {
            U("Raio Maior", 175, "", {{"alcance", 16}}),
            U("Recongelar", 225, "", {{"pierce", 20}}),
            U("Canhão Criogênico", 2000, "Dispara bolas de gelo.", {{"alcance", 60}, {"subst", A("projetil", {{"cad", 1.2}, {"dano", 1}, {"pierce", 1}, {"vel", 700}, {"dist", 300}, {"dtype", "gelo"}, {"congela", 1.5}, {"splash", 34}, {"sdano", 1}, {"spierce", 20}, {"sdtype", "gelo"}, {"visual", "gelo_bola"}, {"raio_proj", 8}})}}),
            U("Pingentes", 2000, "", {{"sdano", 1}, {"moab", 2}, {"spierce", 10}}),
            U("Empalar com Pingentes", 30000, "", {{"sdano", 30}, {"moab", 30}, {"congela", 5}, {"moab_congela", true}}),
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
            U("Dissolvedor de Bloons", 2500, "", {{"cola", J::array({0.5, 11, 2})}}),
            U("Liquefator de Bloons", 5000, "", {{"cola", J::array({0.45, 11, 10})}}),
            U("Solucionador de Bloons", 22000, "", {{"cola", J::array({0.4, 11, 50})}, {"splash", 40}, {"spierce", 6}}),
        },
        {
            U("Globos Maiores", 100, "", {{"pierce", 1}}),
            U("Respingo de Cola", 1600, "", {{"splash", 40}, {"spierce", 6}}),
            U("Mangueira de Cola", 3250, "", {{"cad", 0.3}}),
            U("Ataque de Cola", 3500, "Habilidade: cola todos os bloons.", {{"hab", H("Ataque de Cola", "lentidao", 40, {{"dur", 11}, {"valor", 0.5}})}}),
            U("Tempestade de Cola", 15000, "", {{"hab", H("Tempestade de Cola", "lentidao", 30, {{"dur", 15}, {"valor", 0.3}, {"dano", 5}})}}),
        },
        {
            U("Cola Mais Grudenta", 120, "", {{"cola", J::array({0.5, 22, 0})}}),
            U("Cola Mais Forte", 400, "", {{"cola", J::array({0.35, 22, 0})}}),
            U("Cola de M.O.A.B.", 3400, "Cola dirigíveis.", {{"moab_cola", true}}),
            U("Cola Implacável", 3000, "", {{"pierce", 3}, {"splash", 30}, {"spierce", 4}}),
            U("Super Cola", 35000, "Paralisa dirigíveis.", {{"atordoa", 2.0}, {"moab_atordoa", true}, {"cola", J::array({0.2, 30, 5})}}),
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
            U("Jaqueta Metálica", 350, "Estoura chumbo.", {{"dtype", "normal"}, {"dano", 2}}),
            U("Calibre Grosso", 1300, "", {{"dano", 3}}),
            U("Precisão Mortal", 3000, "", {{"dano", 11}, {"cer", 15}}),
            U("Mutilar M.O.A.B.", 5000, "Atordoa dirigíveis.", {{"atordoa", 3.0}, {"moab_atordoa", true}, {"dano", 12}}),
            U("Aleijar M.O.A.B.", 34000, "", {{"atordoa", 7.0}, {"fragiliza", 5}, {"dano", 20}}),
        },
        {
            U("Óculos de Visão Noturna", 300, "Detecta camo.", {{"camo", true}}),
            U("Tiro de Estilhaços", 450, "", {{"frag", {{"n", 5}, {"dano", 1}, {"pierce", 1}, {"dtype", "afiado"}, {"visual", "fragmento"}}}}),
            U("Bala Ricochete", 3200, "", {{"quica", 3}}),
            U("Lançamento de Suprimentos", 7200, "Habilidade: caixa de dinheiro.", {{"hab", H("Suprimentos", "dinheiro", 60, {{"valor", 1000}})}}),
            U("Atirador de Elite", 13000, "", {{"cad", 0.5}, {"hab", H("Suprimentos de Elite", "dinheiro", 50, {{"valor", 2000}})}}),
        },
        {
            U("Disparo Rápido", 400, "", {{"cad", 0.7}}),
            U("Disparo Mais Rápido", 400, "", {{"cad", 0.7}}),
            U("Semiautomático", 3500, "", {{"cad", 0.33}}),
            U("Rifle Automático", 4750, "", {{"cad", 0.5}, {"dano", 2}, {"moab", 3}}),
            U("Defensor de Elite", 14000, "", {{"cad", 0.5}, {"dano", 2}}),
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
            U("Alcance Maior", 130, "", {{"alcance", 40}}),
            U("Inteligência Avançada", 500, "Ataca qualquer bloon no mapa.", {{"global_", true}}),
            U("Submergir e Apoiar", 500, "Revela bloons camo em volta.", {{"camo", true}, {"novo", A("aura", {{"cad", 0.5}, {"dano", 0}, {"pierce", 999}, {"retira_camo", true}, {"visual", "nenhum"}})}}),
            U("Reator de Bloontônio", 2500, "Radiação estoura tudo em volta.", {{"novo", A("aura", {{"cad", 0.3}, {"dano", 1}, {"pierce", 100}, {"dtype", "normal"}, {"visual", "radiacao"}})}}),
            U("Energizador", 32000, "Radiação intensa.", {{"a", 1}, {"dano", 3}, {"pierce", 300}, {"cad", 0.6}}),
        },
        {
            U("Dardos Farpados", 450, "", {{"pierce", 3}}),
            U("Dardos Aquecidos", 300, "", {{"dtype", "normal"}, {"dano", 1}}),
            U("Míssil Balístico", 1300, "Mísseis de longo alcance.", {{"novo", A("projetil", {{"cad", 1.5}, {"dano", 3}, {"pierce", 1}, {"vel", 900}, {"dist", 2000}, {"busca", true}, {"global_", true}, {"moab", 5}, {"splash", 40}, {"sdano", 1}, {"spierce", 10}, {"visual", "missil"}})}}),
            U("Capacidade de Primeiro Ataque", 13000, "Habilidade: míssil nuclear.", {{"hab", H("Primeiro Ataque", "dano_forte", 60, {{"valor", 10000}, {"n", 1}, {"splash", 120}, {"sdano", 700}})}}),
            U("Ataque Preventivo", 32000, "Míssil em cada dirigível.", {{"novo", A("projetil", {{"cad", 5.0}, {"dano", 1000}, {"pierce", 1}, {"vel", 1500}, {"dist", 3000}, {"busca", true}, {"global_", true}, {"dtype", "normal"}, {"alvo", "forte"}, {"so_moab", true}, {"visual", "missil"}, {"raio_proj", 10}})}}),
        },
        {
            U("Canhões Gêmeos", 450, "", {{"cad", 0.5}}),
            U("Dardos de Explosão Aérea", 1000, "", {{"frag", {{"n", 3}, {"dano", 1}, {"pierce", 1}, {"dtype", "afiado"}, {"visual", "dardo"}}}}),
            U("Canhões Triplos", 1100, "", {{"cad", 0.66}}),
            U("Dardos Perfurantes", 3000, "", {{"dano", 2}, {"moab", 3}, {"cer", 2}}),
            U("Comandante Submarino", 25000, "", {{"dano", 8}, {"pierce", 6}, {"cad", 0.5}}),
        },
    };
    t.categoria = "militar";
    t.agua = true;
    t.desc = "Só na água.";
    t.cor = {230, 200, 40};
    return t;
}

static DefTorre t_bucaneiro() {
    DefTorre t("bucaneiro", "Macaco Bucaneiro", 500, "c", 240);
    t.ataques = {A("projetil", {{"cad", 1.0}, {"dano", 1}, {"pierce", 4}, {"n", 2}, {"spread", 360}, {"vel", 700}, {"dist", 300}, {"visual", "dardo"}})};
    t.caminhos = {
        {
            U("Disparo Rápido", 275, "", {{"cad", 0.75}}),
            U("Tiro Duplo", 450, "", {{"n", 2}}),
            U("Destróier", 2950, "", {{"cad", 0.2}}),
            U("Porta-Aviões", 6000, "Aviões atacam em todo o mapa.", {{"novo", A("radial", {{"cad", 0.6}, {"dano", 2}, {"pierce", 5}, {"n", 4}, {"vel", 700}, {"dist", 300}, {"visual", "aviaozinho"}, {"global_", true}})}}),
            U("Nau Capitânia", 40000, "", {{"a", "todos"}, {"dano", 3}, {"pierce", 5}, {"buffs", {{"cad", 0.85}, {"alcance_pct", 0.1}}}}),
        },
        {
            U("Tiro de Uva", 550, "Dispara uvas.", {{"novo", A("projetil", {{"cad", 1.0}, {"dano", 1}, {"pierce", 1}, {"n", 5}, {"spread", 40}, {"vel", 700}, {"dist", 260}, {"visual", "uva"}})}}),
            U("Tiro Quente", 500, "", {{"a", 1}, {"dtype", "normal"}, {"dano", 1}, {"visual", "uva_fogo"}}),
            U("Navio Canhão", 900, "Balas de canhão explosivas.", {{"novo", A("projetil", {{"cad", 1.3}, {"dano", 2}, {"pierce", 1}, {"vel", 650}, {"dist", 320}, {"splash", 40}, {"sdano", 2}, {"spierce", 16}, {"sdtype", "explosao"}, {"visual", "bala_canhao"}, {"raio_proj", 8}})}}),
            U("Macacos Piratas", 4500, "Habilidade: arpão derruba um dirigível.", {{"moab", 4}, {"hab", H("Arpão", "dano_forte", 60, {{"valor", 4000}, {"n", 1}, {"moab_so", true}})}}),
            U("Senhor Pirata", 21000, "", {{"moab", 10}, {"hab", H("Arpões do Senhor Pirata", "dano_forte", 60, {{"valor", 20000}, {"n", 3}, {"moab_so", true}})}}),
        },
        {
            U("Longo Alcance", 180, "", {{"alcance", 40}}),
            U("Ninho do Corvo", 400, "Detecta camo.", {{"camo", true}}),
            U("Navio Mercante", 2300, "Gera dinheiro por rodada.", {{"novo", A("renda", {{"valor", 200}, {"visual", "moeda"}})}}),
            U("Comércio Favorecido", 5500, "", {{"a", 1}, {"valor", 300}}),
            U("Império Comercial", 23000, "", {{"a", 1}, {"valor", 900}, {"buffs", {{"dano", 1}}}}),
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
            U("Tiro Rápido", 650, "", {{"cad", 0.7}}),
            U("Muito Mais Dardos", 650, "", {{"n", 4}}),
            U("Avião de Caça", 1000, "Mísseis teleguiados.", {{"novo", A("projetil", {{"cad", 1.0}, {"dano", 3}, {"pierce", 1}, {"vel", 900}, {"dist", 2000}, {"busca", true}, {"global_", true}, {"moab", 3}, {"splash", 25}, {"sdano", 1}, {"spierce", 5}, {"visual", "missil"}})}}),
            U("Operação: Tempestade de Dardos", 3000, "", {{"cad", 0.4}, {"n", 4}}),
            U("Retalhador Celeste", 24000, "", {{"n", 8}, {"pierce", 5}, {"cad", 0.5}, {"dano", 2}}),
        },
        {
            U("Abacaxi Explosivo", 200, "Solta abacaxis explosivos.", {{"novo", A("queda", {{"cad", 3.0}, {"splash", 50}, {"sdano", 1}, {"spierce", 20}, {"fusivel", 1.5}, {"visual", "abacaxi"}})}}),
            U("Avião Espião", 350, "Detecta camo.", {{"camo", true}}),
            U("Ás Bombardeiro", 900, "Bombas na trilha.", {{"novo", A("queda", {{"cad", 1.5}, {"splash", 60}, {"sdano", 2}, {"spierce", 30}, {"fusivel", 0.3}, {"visual", "bomba"}, {"na_trilha", true}})}}),
            U("Marco Zero", 18000, "Habilidade: bomba gigante.", {{"hab", H("Marco Zero", "dano_global", 45, {{"valor", 700}})}}),
            U("Tsar Bomba", 30000, "", {{"hab", H("Tsar Bomba", "dano_global", 60, {{"valor", 3000}, {"atordoa", 8}})}}),
        },
        {
            U("Dardos Mais Afiados", 500, "", {{"pierce", 3}}),
            U("Rota Centralizada", 300, "Voa em círculo menor.", J::object()),
            U("Mira Infalível", 2200, "Dardos teleguiados.", {{"busca", true}}),
            U("Espectro", 24000, "Chuva de dardos e bombas.", {{"novo", A("projetil", {{"cad", 0.05}, {"dano", 2}, {"pierce", 3}, {"vel", 900}, {"dist", 1200}, {"busca", true}, {"global_", true}, {"dtype", "normal"}, {"visual", "dardo"}})}}),
            U("Fortaleza Voadora", 90000, "", {{"a", "todos"}, {"dano", 3}, {"n", 8}, {"cad", 0.5}}),
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
    DefTorre t("heli", "Piloto de Helicóptero", 1600, "b", 170);
    t.ataques = {A("projetil", {{"cad", 0.57}, {"dano", 1}, {"pierce", 3}, {"n", 2}, {"spread", 10}, {"vel", 850}, {"dist", 260}, {"visual", "dardo"}})};
    t.caminhos = {
        {
            U("Dardos Quádruplos", 800, "", {{"n", 2}, {"spread", 20}}),
            U("Perseguição", 500, "Persegue os bloons.", {{"persegue", true}}),
            U("Hélices Navalha", 1750, "Hélices estouram bloons.", {{"novo", A("aura", {{"cad", 0.5}, {"dano", 2}, {"pierce", 20}, {"raio_aura", 55}, {"visual", "nenhum"}})}}),
            U("Apache Dardeiro", 19600, "Metralhadoras e mísseis.", {{"cad", 0.35}, {"n", 2}, {"novo", A("projetil", {{"cad", 1.0}, {"dano", 5}, {"pierce", 1}, {"vel", 900}, {"dist", 600}, {"moab", 5}, {"splash", 35}, {"sdano", 2}, {"spierce", 10}, {"visual", "missil"}})}}),
            U("Apache Prime", 45000, "", {{"a", "todos"}, {"dtype", "normal"}, {"dano", 4}, {"visual", "plasma"}}),
        },
        {
            U("Jatos Maiores", 300, "", {{"alcance", 20}}),
            U("IFR", 600, "Detecta camo.", {{"camo", true}}),
            U("Corrente Descendente", 2000, "Empurra bloons para trás.", {{"novo", A("aura", {{"cad", 1.2}, {"dano", 0}, {"pierce", 6}, {"empurra", 90}, {"raio_aura", 80}, {"visual", "vento"}})}}),
            U("Chinook de Apoio", 12000, "Habilidade: entrega dinheiro.", {{"hab", H("Entrega", "dinheiro", 60, {{"valor", 1000}})}}),
            U("Operações Especiais", 35000, "Habilidade: fuzileiro de elite.", {{"hab", H("Fuzileiro", "invocar", 60, {{"dur", 20}, {"base", "sniper"}, {"nivel", J::array({4, 0, 3})}})}}),
        },
        {
            U("Dardos Rápidos", 250, "", {{"vel", 1.3}}),
            U("Disparo Rápido", 350, "", {{"cad", 0.8}}),
            U("Empurrão de M.O.A.B.", 3000, "Desacelera dirigíveis.", {{"novo", A("aura", {{"cad", 0.5}, {"dano", 0}, {"pierce", 99}, {"lento", J::array({0.5, 0.5})}, {"moab_lento", true}, {"raio_aura", 90}, {"visual", "nenhum"}})}}),
            U("Defesa Comanche", 8500, "Mini comanches ajudam.", {{"n", 4}, {"cad", 0.7}}),
            U("Comandante Comanche", 35000, "", {{"dano", 4}, {"n", 6}, {"pierce", 4}}),
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
    DefTorre t("morteiro", "Macaco Morteiro", 750, "n", 9999);
    t.ataques = {A("morteiro", {{"cad", 2.0}, {"dano", 1}, {"splash", 40}, {"sdano", 1}, {"spierce", 40}, {"sdtype", "explosao"}, {"impreciso", 40}, {"global_", true}, {"visual", "bala_canhao"}})};
    t.caminhos = {
        {
            U("Explosão Maior", 500, "", {{"splash", 12}, {"spierce", 10}}),
            U("Destruidor de Bloons", 500, "", {{"sdano", 1}}),
            U("Choque de Projéteis", 900, "Atordoa bloons.", {{"atordoa", 0.5}, {"splash", 10}}),
            U("A Grande", 7000, "", {{"splash", 30}, {"sdano", 3}, {"spierce", 60}}),
            U("A Maior de Todas", 35000, "", {{"splash", 60}, {"sdano", 20}, {"spierce", 200}}),
        },
        {
            U("Recarga Rápida", 300, "", {{"cad", 0.75}}),
            U("Recarga Veloz", 500, "", {{"cad", 0.75}}),
            U("Projéteis Pesados", 900, "", {{"sdano", 1}, {"sdtype", "normal"}}),
            U("Bateria de Artilharia", 5500, "3 projéteis por vez.", {{"n", 2}}),
            U("Choque e Pavor", 30000, "Habilidade: atordoa tudo.", {{"hab", H("Choque e Pavor", "dano_global", 60, {{"valor", 50}, {"atordoa", 8}})}}),
        },
        {
            U("Precisão Aumentada", 200, "", {{"impreciso", 0.5}}),
            U("Coisas Queimando", 500, "Deixa fogo.", {{"queima", J::array({1, 3})}}),
            U("Sinalizador", 600, "Revela camo.", {{"retira_camo", true}, {"camo", true}}),
            U("Projéteis Estilhaçantes", 11000, "", {{"fragiliza", 2}, {"cer", 5}, {"moab", 5}}),
            U("Bloonflagração", 40000, "", {{"queima", J::array({20, 3})}, {"sdano", 5}, {"sdtype", "normal"}}),
        },
    };
    t.categoria = "militar";
    t.desc = "Atira no local escolhido.";
    t.cor = {90, 110, 70};
    return t;
}

static DefTorre t_dartling() {
    DefTorre t("dartling", "Atirador Dartling", 850, "m", 9999);
    t.ataques = {A("projetil", {{"cad", 0.2}, {"dano", 1}, {"pierce", 1}, {"vel", 1200}, {"dist", 1400}, {"spread", 20}, {"global_", true}, {"visual", "dardo"}})};
    t.caminhos = {
        {
            U("Disparo Focado", 250, "", {{"spread", 6}}),
            U("Choque Laser", 1200, "", {{"dtype", "energia"}, {"pierce", 1}, {"queima", J::array({1, 1})}, {"visual", "laser"}}),
            U("Canhão Laser", 3000, "", {{"dano", 1}, {"pierce", 3}, {"visual", "laser"}}),
            U("Acelerador de Plasma", 11000, "Raio contínuo.", {{"subst", A("hitscan", {{"cad", 0.2}, {"dano", 3}, {"pierce", 100}, {"dtype", "normal"}, {"global_", true}, {"visual", "raio_plasma"}, {"linha", true}})}}),
            U("Raio da Perdição", 90000, "", {{"dano", 27}, {"pierce", 200}, {"cad", 0.5}, {"moab", 10}}),
        },
        {
            U("Mira Avançada", 300, "", {{"busca", true}}),
            U("Giro de Cano Rápido", 950, "", {{"cad", 0.7}}),
            U("Cápsulas de Foguete Hidra", 5000, "", {{"splash", 25}, {"sdano", 1}, {"spierce", 6}, {"visual", "missil"}}),
            U("Tempestade de Foguetes", 6000, "Habilidade: chuva de foguetes.", {{"hab", H("Tempestade de Foguetes", "turbo", 40, {{"dur", 8}, {"valor", 0.2}})}}),
            U("M.A.D.", 58000, "Mísseis anti dirigível.", {{"moab", 40}, {"sdano", 6}, {"splash", 20}}),
        },
        {
            U("Giro Mais Rápido", 150, "", {{"vel", 1.2}}),
            U("Dardos Poderosos", 1200, "", {{"dano", 1}, {"pierce", 1}, {"vel", 1.5}}),
            U("Chumbinho", 3200, "", {{"n", 5}, {"spread", 30}, {"dano", 1}}),
            U("Sistema de Negação de Área", 6000, "", {{"n", 4}, {"pierce", 2}}),
            U("Zona de Exclusão Bloon", 42000, "", {{"n", 6}, {"dano", 3}, {"pierce", 3}, {"cad", 0.6}}),
        },
    };
    t.categoria = "militar";
    t.desc = "Metralhadora de dardos.";
    t.cor = {70, 90, 110};
    return t;
}

static DefTorre t_mago() {
    DefTorre t("mago", "Macaco Mago", 375, "a", 160);
    t.ataques = {A("projetil", {{"cad", 1.1}, {"dano", 1}, {"pierce", 2}, {"dtype", "energia"}, {"vel", 700}, {"dist", 240}, {"visual", "magia"}})};
    t.caminhos = {
        {
            U("Magia Guiada", 150, "Magia teleguiada.", {{"busca", true}}),
            U("Explosão Arcana", 600, "", {{"dano", 1}}),
            U("Maestria Arcana", 1300, "", {{"cad", 0.5}, {"pierce", 3}, {"alcance", 20}}),
            U("Espinho Arcano", 10900, "", {{"moab", 10}, {"dano", 2}}),
            U("Arquimago", 32000, "", {{"a", "todos"}, {"dano", 5}, {"pierce", 5}, {"cad", 0.6}}),
        },
        {
            U("Bola de Fogo", 300, "Lança bolas de fogo.", {{"novo", A("projetil", {{"cad", 2.2}, {"dano", 1}, {"pierce", 1}, {"vel", 600}, {"dist", 260}, {"dtype", "energia"}, {"splash", 30}, {"sdano", 1}, {"spierce", 12}, {"sdtype", "energia"}, {"visual", "fogo"}, {"raio_proj", 8}})}}),
            U("Muralha de Fogo", 900, "Chamas na trilha.", {{"novo", A("pilha", {{"cad", 5.5}, {"dano", 1}, {"pilha_pierce", 15}, {"pilha_vida", 5}, {"dtype", "energia"}, {"visual", "chamas"}})}}),
            U("Sopro do Dragão", 3000, "Lança chamas.", {{"novo", A("projetil", {{"cad", 0.1}, {"dano", 1}, {"pierce", 5}, {"n", 3}, {"spread", 25}, {"vel", 500}, {"dist", 170}, {"dtype", "energia"}, {"visual", "fogo"}})}}),
            U("Invocar Fênix", 4000, "Habilidade: fênix de fogo.", {{"hab", H("Fênix", "invocar", 45, {{"dur", 20}, {"base", "fenix"}})}}),
            U("Lorde Fênix", 50000, "Fênix permanente.", {{"novo", A("projetil", {{"cad", 0.1}, {"dano", 5}, {"pierce", 10}, {"n", 2}, {"spread", 30}, {"vel", 700}, {"dist", 600}, {"global_", true}, {"dtype", "normal"}, {"visual", "fogo"}})}}),
        },
        {
            U("Magia Intensa", 300, "", {{"pierce", 2}, {"vel", 1.2}}),
            U("Sentido Macaco", 300, "Detecta camo.", {{"camo", true}}),
            U("Cintilar", 1700, "Revela camo em volta.", {{"novo", A("aura", {{"cad", 1.0}, {"dano", 0}, {"pierce", 999}, {"retira_camo", true}, {"visual", "nenhum"}})}}),
            U("Necromante", 2000, "Bloons mortos voltam como aliados.", {{"novo", A("pilha", {{"cad", 2.0}, {"dano", 2}, {"pilha_pierce", 8}, {"pilha_vida", 6}, {"dtype", "normal"}, {"visual", "zumbi"}})}}),
            U("Príncipe das Trevas", 24000, "", {{"a", "todos"}, {"dano", 4}, {"pilha_pierce", 20}}),
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
            U("Rajadas Laser", 2500, "", {{"dtype", "energia"}, {"pierce", 1}, {"visual", "laser"}}),
            U("Rajadas de Plasma", 4500, "", {{"dtype", "normal"}, {"dano", 1}, {"pierce", 1}, {"visual", "plasma"}}),
            U("Avatar do Sol", 20000, "", {{"n", 2}, {"spread", 20}, {"dano", 2}, {"pierce", 2}, {"visual", "sol"}}),
            U("Templo do Sol", 100000, "", {{"dano", 8}, {"pierce", 5}, {"moab", 10}}),
            U("Verdadeiro Deus Sol", 500000, "", {{"dano", 20}, {"pierce", 10}, {"moab", 30}}),
        },
        {
            U("Super Alcance", 1000, "", {{"alcance", 40}, {"dist", 60}}),
            U("Alcance Épico", 1400, "", {{"alcance", 40}, {"dist", 60}}),
            U("Robô Macaco", 7000, "", {{"n", 1}, {"spread", 12}, {"dano", 1}}),
            U("Terror Tecnológico", 19000, "Habilidade: aniquilação.", {{"hab", H("Aniquilação", "dano_global", 45, {{"valor", 2000}})}}),
            U("O Anti-Bloon", 80000, "", {{"dano", 4}, {"hab", H("Erradicação", "dano_global", 45, {{"valor", 5000}})}}),
        },
        {
            U("Repulsão", 3000, "", {{"empurra", 20}}),
            U("Ultravisão", 1200, "", {{"camo", true}, {"alcance", 10}}),
            U("Cavaleiro das Trevas", 5500, "", {{"dtype", "normal"}, {"moab", 2}, {"visual", "escuro"}}),
            U("Campeão das Trevas", 60000, "", {{"dano", 4}, {"moab", 6}, {"pierce", 3}}),
            U("Lenda da Noite", 240000, "Habilidade: manda bloons de volta.", {{"dano", 10}, {"hab", H("Noite Eterna", "reverso", 60, {{"valor", 600}})}}),
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
    t.ataques = {A("projetil", {{"cad", 0.7}, {"dano", 1}, {"pierce", 2}, {"vel", 900}, {"dist", 240}, {"busca", true}, {"visual", "shuriken"}})};
    t.caminhos = {
        {
            U("Disciplina Ninja", 300, "", {{"alcance", 28}, {"cad", 0.8}}),
            U("Shurikens Afiadas", 350, "", {{"pierce", 2}}),
            U("Tiro Duplo", 850, "", {{"n", 1}, {"spread", 10}}),
            U("Bloonjitsu", 2750, "", {{"n", 3}, {"spread", 30}}),
            U("Grão-Mestre Ninja", 35000, "", {{"n", 3}, {"dano", 2}, {"cad", 0.5}}),
        },
        {
            U("Distração", 350, "Empurra bloons.", {{"empurra", 25}}),
            U("Contraespionagem", 500, "Remove camo.", {{"retira_camo", true}}),
            U("Táticas Shinobi", 900, "", {{"cad", 0.92}, {"buffs", {{"cad", 0.92}}}}),
            U("Sabotagem Bloon", 5200, "Habilidade: bloons lentos.", {{"hab", H("Sabotagem", "lentidao", 60, {{"dur", 15}, {"valor", 0.5}})}}),
            U("Grande Sabotador", 22000, "", {{"hab", H("Grande Sabotagem", "lentidao", 60, {{"dur", 20}, {"valor", 0.5}, {"dano", 300}})}}),
        },
        {
            U("Shuriken Teleguiada", 250, "", {{"pierce", 1}}),
            U("Estrepes", 400, "Espalha estrepes na trilha.", {{"novo", A("pilha", {{"cad", 4.0}, {"dano", 1}, {"pilha_pierce", 6}, {"pilha_vida", 10}, {"visual", "estrepe"}})}}),
            U("Bomba de Luz", 2750, "Atordoa bloons.", {{"novo", A("projetil", {{"cad", 4.0}, {"dano", 1}, {"pierce", 1}, {"vel", 600}, {"dist", 260}, {"splash", 55}, {"sdano", 1}, {"spierce", 50}, {"atordoa", 1.0}, {"visual", "flash"}})}}),
            U("Bomba Grudenta", 4500, "Bomba em dirigíveis.", {{"novo", A("projetil", {{"cad", 3.0}, {"dano", 500}, {"pierce", 1}, {"vel", 900}, {"dist", 500}, {"busca", true}, {"so_moab", true}, {"alvo", "forte"}, {"dtype", "normal"}, {"visual", "bomba"}})}}),
            U("Mestre Bombardeiro", 40000, "", {{"a", 2}, {"dano", 4500}, {"cad", 0.6}}),
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
    t.ataques = {A("projetil", {{"cad", 2.0}, {"dano", 1}, {"pierce", 1}, {"vel", 600}, {"dist", 260}, {"dtype", "normal"}, {"splash", 30}, {"sdano", 1}, {"spierce", 15}, {"visual", "pocao"}, {"raio_proj", 7}})};
    t.caminhos = {
        {
            U("Poções Maiores", 250, "", {{"splash", 10}, {"spierce", 10}}),
            U("Mistura Ácida", 350, "Torres próximas causam mais dano em dirigíveis.", {{"novo", A("buff", {{"buffs", {{"moab", 1}}}})}}),
            U("Poção do Berserker", 1250, "Torres próximas ficam mais fortes.", {{"a", 1}, {"buffs", {{"dano", 1}, {"cad", 0.9}, {"pierce", 2}, {"alcance_pct", 0.1}}}}),
            U("Estimulante Forte", 3000, "", {{"a", 1}, {"buffs", {{"cad", 0.85}, {"dano", 1}}}}),
            U("Poção Permanente", 60000, "", {{"a", 1}, {"buffs", {{"dano", 2}, {"pierce", 5}, {"cad", 0.8}}}}),
        },
        {
            U("Ácido Forte", 250, "", {{"queima", J::array({1, 4})}}),
            U("Poções Perecíveis", 475, "", {{"moab", 4}, {"cer", 2}}),
            U("Mistura Instável", 3000, "", {{"moab", 10}, {"splash", 10}}),
            U("Tônico Transformador", 4500, "Habilidade: vira monstro.", {{"hab", H("Transformação", "turbo", 60, {{"dur", 20}, {"valor", 0.2}})}}),
            U("Transformação Total", 45000, "", {{"hab", H("Transformação Total", "turbo_area", 40, {{"dur", 20}, {"valor", 0.3}})}}),
        },
        {
            U("Arremesso Rápido", 650, "", {{"cad", 0.75}}),
            U("Poça de Ácido", 450, "Poças na trilha.", {{"novo", A("pilha", {{"cad", 3.0}, {"dano", 1}, {"pilha_pierce", 10}, {"pilha_vida", 8}, {"dtype", "normal"}, {"visual", "acido"}})}}),
            U("Chumbo em Ouro", 1000, "Chumbo estourado dá dinheiro extra.", {{"ouro", 0.5}}),
            U("Borracha em Ouro", 2750, "", {{"ouro", 1.0}}),
            U("Mestre Alquimista", 40000, "Encolhe bloons.", {{"dano", 10}, {"moab", 30}, {"pierce", 5}}),
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
            U("Espinhos Duros", 250, "", {{"pierce", 1}, {"dtype", "normal"}}),
            U("Coração do Trovão", 1000, "Raios em cadeia.", {{"novo", A("cadeia", {{"cad", 2.3}, {"dano", 1}, {"pierce", 1}, {"saltos", 6}, {"dtype", "energia"}, {"visual", "relampago"}})}}),
            U("Druida da Tempestade", 1650, "Tornado empurra bloons.", {{"novo", A("projetil", {{"cad", 2.5}, {"dano", 0}, {"pierce", 30}, {"vel", 300}, {"dist", 300}, {"empurra", 120}, {"visual", "tornado"}, {"raio_proj", 18}})}}),
            U("Bola de Relâmpago", 4500, "", {{"a", 1}, {"saltos", 8}, {"dano", 2}, {"cad", 0.6}}),
            U("Supertempestade", 60000, "", {{"a", "todos"}, {"dano", 8}, {"pierce", 10}, {"moab", 10}}),
        },
        {
            U("Enxame de Espinhos", 250, "", {{"n", 3}}),
            U("Coração de Carvalho", 350, "Remove regeneração.", {{"retira_regen", true}}),
            U("Druida da Selva", 950, "Cipós prendem bloons.", {{"novo", A("pilha", {{"cad", 3.0}, {"dano", 2}, {"pilha_pierce", 8}, {"pilha_vida", 6}, {"dtype", "normal"}, {"visual", "cipo"}})}}),
            U("Recompensa da Selva", 5000, "Gera dinheiro.", {{"novo", A("renda", {{"valor", 250}, {"visual", "moeda"}})}}),
            U("Espírito da Floresta", 35000, "", {{"a", 1}, {"dano", 6}, {"pilha_pierce", 40}, {"cad", 0.3}}),
        },
        {
            U("Alcance Druídico", 100, "", {{"alcance", 40}}),
            U("Coração da Vingança", 300, "", {{"cad", 0.85}}),
            U("Druida da Ira", 600, "", {{"cad", 0.8}, {"dano", 1}}),
            U("Luxúria de Estouros", 2500, "", {{"pierce", 3}, {"buffs", {{"cad", 0.85}}}}),
            U("Avatar da Ira", 45000, "", {{"dano", 6}, {"pierce", 6}, {"n", 4}}),
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
            U("Produção Aumentada", 500, "", {{"valor", 20}}),
            U("Produção Maior", 600, "", {{"valor", 40}}),
            U("Plantação de Bananas", 3000, "", {{"valor", 140}}),
            U("Centro de Pesquisa de Bananas", 19000, "", {{"valor", 800}}),
            U("Central de Bananas", 100000, "", {{"valor", 3000}}),
        },
        {
            U("Bananas Duradouras", 300, "", {{"valor", 10}}),
            U("Bananas Valiosas", 800, "", {{"valor_x", 1.25}}),
            U("Banco Macaco", 3650, "", {{"valor", 250}}),
            U("Empréstimo do FMI", 7200, "Habilidade: empréstimo.", {{"hab", H("Empréstimo", "dinheiro", 90, {{"valor", 5000}})}}),
            U("Macaconomia", 100000, "", {{"hab", H("Macaconomia", "dinheiro", 60, {{"valor", 10000}})}}),
        },
        {
            U("Coleta Fácil", 250, "", {{"valor", 10}}),
            U("Salvamento de Bananas", 200, "Vende por 90%.", {{"venda", 0.9}}),
            U("Mercado", 2900, "", {{"valor", 200}}),
            U("Mercado Central", 15000, "", {{"valor", 600}}),
            U("Wall Street dos Macacos", 60000, "", {{"valor", 4000}}),
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
    t.ataques = {A("pilha", {{"cad", 2.2}, {"dano", 1}, {"pilha_pierce", 5}, {"pilha_vida", 40}, {"visual", "espinhos"}})};
    t.caminhos = {
        {
            U("Pilhas Maiores", 800, "", {{"pilha_pierce", 5}}),
            U("Espinhos Incandescentes", 600, "", {{"dtype", "normal"}}),
            U("Bolas Espinhosas", 2300, "", {{"dano", 1}, {"cer", 3}, {"visual", "bola_espinho"}}),
            U("Minas Espinhosas", 10000, "Explodem ao acabar.", {{"splash", 40}, {"sdano", 5}, {"spierce", 40}}),
            U("Super Minas", 150000, "", {{"splash", 80}, {"sdano", 400}, {"spierce", 200}}),
        },
        {
            U("Produção Rápida", 600, "", {{"cad", 0.75}}),
            U("Produção Mais Rápida", 800, "", {{"cad", 0.75}}),
            U("Triturador de M.O.A.B.", 2500, "", {{"moab", 2}}),
            U("Tempestade de Espinhos", 5000, "Habilidade: espinhos na trilha toda.", {{"hab", H("Tempestade de Espinhos", "spikes_global", 40, {{"valor", 60}, {"dano", 2}})}}),
            U("Tapete de Espinhos", 40000, "", {{"cad", 0.6}, {"hab", H("Tapete de Espinhos", "spikes_global", 30, {{"valor", 150}, {"dano", 4}})}}),
        },
        {
            U("Espinhos Duradouros", 150, "", {{"pilha_vida", 2}}),
            U("Espinhos Mortais", 400, "", {{"dano", 1}}),
            U("Longo Alcance", 1400, "", {{"alcance", 40}, {"pilha_pierce", 5}}),
            U("Espinhominador", 12500, "", {{"dano", 3}, {"pilha_pierce", 15}, {"pilha_vida", 2}}),
            U("Perma-Espinho", 30000, "", {{"dano", 10}, {"pilha_pierce", 40}, {"pilha_vida", 4}}),
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
            U("Raio Maior", 400, "", {{"alcance", 40}}),
            U("Tambores da Selva", 1500, "Torres atacam mais rápido.", {{"buffs", {{"cad", 0.85}}}}),
            U("Treinamento Primário", 800, "", {{"buffs", {{"pierce", 1}, {"alcance_pct", 0.1}}}}),
            U("Mentoria Primária", 2500, "", {{"buffs", {{"dano", 1}}}}),
            U("Especialização Primária", 25000, "Balista gigante.", {{"novo", A("projetil", {{"cad", 4.0}, {"dano", 1000}, {"pierce", 5}, {"vel", 1200}, {"dist", 2500}, {"busca", true}, {"global_", true}, {"dtype", "normal"}, {"alvo", "forte"}, {"visual", "balista"}, {"raio_proj", 12}})}}),
        },
        {
            U("Bloqueador de Crescimento", 250, "Remove regeneração.", {{"novo", A("aura", {{"cad", 0.5}, {"dano", 0}, {"pierce", 999}, {"retira_regen", true}, {"visual", "nenhum"}})}}),
            U("Radar", 2000, "Torres próximas detectam camo.", {{"buffs", {{"camo", true}}}}),
            U("Agência de Inteligência Macaco", 7500, "Torres estouram tudo.", {{"buffs", {{"dtype_normal", true}}}}),
            U("Chamado às Armas", 20000, "Habilidade: torres mais rápidas.", {{"hab", H("Chamado às Armas", "turbo_area", 45, {{"dur", 12}, {"valor", 0.66}})}}),
            U("Defesa da Pátria", 40000, "", {{"hab", H("Defesa da Pátria", "turbo_area", 60, {{"dur", 20}, {"valor", 0.5}, {"global_", true}})}}),
        },
        {
            U("Negócios Macacos", 500, "Desconto em torres próximas.", {{"desconto", 0.1}}),
            U("Comércio Macaco", 500, "", {{"desconto", 0.15}}),
            U("Cidade Macaco", 10000, "Mais dinheiro por estouro.", {{"buffs", {{"ouro", 0.5}}}}),
            U("Metrópole Macaco", 13000, "", {{"novo", A("renda", {{"valor", 500}, {"visual", "moeda"}})}}),
            U("Macacópolis", 75000, "", {{"a", 2}, {"valor", 5000}}),
        },
    };
    t.categoria = "suporte";
    t.raio = 28;
    t.desc = "Melhora torres próximas.";
    t.cor = {160, 110, 60};
    return t;
}

static DefTorre t_engenheiro() {
    DefTorre t("engenheiro", "Macaco Engenheiro", 400, "l", 160);
    t.ataques = {A("projetil", {{"cad", 0.7}, {"dano", 1}, {"pierce", 3}, {"vel", 800}, {"dist", 240}, {"visual", "prego"}})};
    t.caminhos = {
        {
            U("Torreta Sentinela", 500, "Cria torretas.", {{"novo", A("invocar", {{"cad", 10.0}, {"dur", 25}, {"base", "sentinela"}})}}),
            U("Engenharia Rápida", 400, "", {{"a", 1}, {"cad", 0.6}}),
            U("Engrenagens", 575, "", {{"a", 1}, {"nivel_inv", 1}}),
            U("Especialista em Sentinelas", 2500, "", {{"a", 1}, {"nivel_inv", 1}}),
            U("Campeão das Sentinelas", 32000, "", {{"a", 1}, {"nivel_inv", 2}, {"cad", 0.5}}),
        },
        {
            U("Área de Serviço Maior", 250, "", {{"alcance", 24}}),
            U("Desconstrução", 350, "", {{"moab", 1}, {"fort", 1}}),
            U("Espuma Purificadora", 800, "Espuma remove camo e regeneração.", {{"novo", A("pilha", {{"cad", 4.0}, {"dano", 1}, {"pilha_pierce", 10}, {"pilha_vida", 8}, {"retira_camo", true}, {"retira_regen", true}, {"visual", "espuma"}})}}),
            U("Overclock", 13500, "Habilidade: acelera torres.", {{"hab", H("Overclock", "turbo_area", 45, {{"dur", 30}, {"valor", 0.6}})}}),
            U("Ultraimpulso", 105000, "", {{"buffs", {{"cad", 0.7}}}, {"hab", H("Ultraimpulso", "turbo_area", 30, {{"dur", 45}, {"valor", 0.4}})}}),
        },
        {
            U("Pregos Enormes", 450, "", {{"pierce", 5}, {"dtype", "normal"}}),
            U("Pino", 450, "Pregos desaceleram.", {{"lento", J::array({0.6, 1.0})}}),
            U("Arma Dupla", 3700, "", {{"cad", 0.5}}),
            U("Armadilha Bloon", 3100, "Armadilha na trilha.", {{"novo", A("pilha", {{"cad", 8.0}, {"dano", 99}, {"pilha_pierce", 500}, {"pilha_vida", 30}, {"dtype", "normal"}, {"visual", "armadilha"}})}}),
            U("Armadilha XXXL", 54000, "", {{"a", "todos"}, {"dano", 4}, {"pilha_pierce", 2000}, {"moab", 500}}),
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
    t.ataques = {A("projetil", {{"cad", 0.1}, {"dano", 5}, {"pierce", 10}, {"n", 2}, {"spread", 30}, {"vel", 700}, {"dist", 700}, {"global_", true}, {"dtype", "normal"}, {"visual", "fogo"}})};
    t.mov = Mov::ORBITA;
    t.raio = 22;
    t.cor = {250, 120, 30};
    return t;
}

// ================================================================ HEROIS
static DefTorre h_quincy() {
    DefTorre t("quincy", "Quincy", 540, "u", 160);
    t.ataques = {A("projetil", {{"cad", 0.95}, {"dano", 1}, {"pierce", 3}, {"vel", 900}, {"dist", 260}, {"visual", "flecha"}})};
    t.cor = {150, 95, 45};
    t.heroi = true;
    t.titulo = "Arqueiro Orgulhoso";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {3, {{"quica", 1}}},
        {4, {{"pierce", 1}}},
        {5, {{"cad", 0.9}}},
        {6, {{"dano", 1}}},
        {7, {{"alcance", 10}, {"n", 1}, {"spread", 10}}},
        {8, {{"pierce", 2}}},
        {9, {{"cad", 0.9}}},
        {11, {{"dano", 1}}},
        {12, {{"pierce", 2}, {"camo", true}}},
        {13, {{"alcance", 10}}},
        {14, {{"cad", 0.85}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}, {"moab", 3}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Tiro Rápido", "turbo", 45, {{"dur", 7}, {"valor", 0.33}});
    t.hab10 = H("Tempestade de Flechas", "dano_global", 60, {{"valor", 12}});
    return t;
}

static DefTorre h_gwendolin() {
    DefTorre t("gwendolin", "Gwendolin", 725, "u", 150);
    t.ataques = {A("projetil", {{"cad", 0.9}, {"dano", 1}, {"pierce", 2}, {"vel", 800}, {"dist", 240}, {"dtype", "energia"}, {"queima", J::array({1, 2})}, {"visual", "fogo"}})};
    t.cor = {200, 80, 40};
    t.heroi = true;
    t.titulo = "Cientista Piromaníaca";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {4, {{"pierce", 1}}},
        {5, {{"cad", 0.9}}},
        {6, {{"dano", 1}, {"dtype", "normal"}}},
        {7, {{"alcance", 10}}},
        {8, {{"pierce", 2}}},
        {9, {{"cad", 0.9}}},
        {11, {{"dano", 1}, {"queima", J::array({3, 3})}}},
        {12, {{"pierce", 2}}},
        {13, {{"alcance", 10}}},
        {14, {{"cad", 0.85}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}, {"buffs", {{"dano", 1}}}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Coquetel de Fogo", "spikes_local", 20, {{"valor", 30}, {"dano", 1}, {"dur", 8}});
    t.hab10 = H("Tempestade de Fogo", "dano_global", 60, {{"valor", 40}, {"queima", J::array({5, 6})}});
    return t;
}

static DefTorre h_striker() {
    DefTorre t("striker", "Striker Jones", 750, "u", 170);
    t.ataques = {A("projetil", {{"cad", 1.3}, {"dano", 1}, {"pierce", 1}, {"vel", 650}, {"dist", 280}, {"visual", "bomba"}, {"splash", 35}, {"sdano", 1}, {"spierce", 10}, {"sdtype", "explosao"}, {"raio_proj", 7}})};
    t.cor = {80, 100, 60};
    t.heroi = true;
    t.titulo = "Comandante de Artilharia";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {4, {{"pierce", 1}, {"sdtype", "normal"}}},
        {5, {{"cad", 0.9}}},
        {6, {{"dano", 1}}},
        {7, {{"alcance", 10}}},
        {8, {{"pierce", 2}, {"splash", 10}, {"sdano", 1}}},
        {9, {{"cad", 0.9}}},
        {11, {{"dano", 1}}},
        {12, {{"pierce", 2}}},
        {13, {{"alcance", 10}}},
        {14, {{"cad", 0.85}, {"sdano", 2}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}, {"sdano", 4}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Projétil de Concussão", "dano_forte", 20, {{"valor", 40}, {"n", 1}, {"atordoa", 4}});
    t.hab10 = H("Comando de Artilharia", "turbo_area", 60, {{"dur", 10}, {"valor", 0.5}, {"filtro", "bomba,morteiro"}, {"global_", true}});
    return t;
}

static DefTorre h_obyn() {
    DefTorre t("obyn", "Obyn Guardião", 650, "u", 160);
    t.ataques = {A("projetil", {{"cad", 1.35}, {"dano", 2}, {"pierce", 4}, {"vel", 700}, {"dist", 260}, {"busca", true}, {"dtype", "energia"}, {"visual", "espirito"}})};
    t.cor = {40, 120, 90};
    t.heroi = true;
    t.titulo = "Guardião da Floresta";
    t.niveis = {
        {2, {{"alcance", 8}, {"buffs", {{"pierce", 1}}}}},
        {4, {{"pierce", 1}}},
        {5, {{"cad", 0.9}}},
        {6, {{"dano", 1}}},
        {7, {{"alcance", 10}}},
        {8, {{"pierce", 2}}},
        {9, {{"cad", 0.9}}},
        {11, {{"dano", 1}, {"buffs", {{"pierce", 2}, {"dano", 1}}}}},
        {12, {{"pierce", 2}}},
        {13, {{"alcance", 10}}},
        {14, {{"cad", 0.85}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Espinheiros", "spikes_local", 25, {{"valor", 60}, {"dano", 1}, {"dur", 12}});
    t.hab10 = H("Muralha de Árvores", "spikes_local", 50, {{"valor", 2000}, {"dano", 1}, {"dur", 15}});
    return t;
}

static DefTorre h_churchill() {
    DefTorre t("churchill", "Capitão Churchill", 2000, "u", 170);
    t.ataques = {A("projetil", {{"cad", 0.6}, {"dano", 3}, {"pierce", 1}, {"vel", 900}, {"dist", 300}, {"dtype", "normal"}, {"splash", 25}, {"sdano", 2}, {"spierce", 6}, {"sdtype", "normal"}, {"visual", "bala_canhao"}, {"raio_proj", 8}})};
    t.cor = {70, 90, 60};
    t.heroi = true;
    t.titulo = "Tanque Blindado";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {3, {{"moab", 3}}},
        {4, {{"pierce", 1}}},
        {5, {{"cad", 0.9}}},
        {6, {{"dano", 1}}},
        {7, {{"alcance", 10}}},
        {8, {{"pierce", 2}, {"camo", true}}},
        {9, {{"cad", 0.9}}},
        {11, {{"dano", 1}}},
        {12, {{"pierce", 2}}},
        {13, {{"alcance", 10}}},
        {14, {{"cad", 0.85}, {"moab", 10}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}, {"moab", 30}}},
    };
    t.hab3 = H("Projéteis Perfurantes", "turbo", 40, {{"dur", 8}, {"valor", 0.4}});
    t.hab10 = H("Barragem M.O.A.B.", "dano_forte", 60, {{"valor", 500}, {"n", 5}, {"moab_so", true}});
    return t;
}

static DefTorre h_benjamin() {
    DefTorre t("benjamin", "Benjamin", 1200, "u", 100);
    t.ataques = {A("renda", {{"valor", 60}, {"visual", "moeda"}})};
    t.cor = {60, 60, 80};
    t.heroi = true;
    t.titulo = "Hacker";
    t.niveis = {
        {2, {{"valor", 40}}},
        {3, {{"valor", 50}}},
        {4, {{"valor", 60}}},
        {5, {{"valor", 70}}},
        {6, {{"valor", 80}}},
        {7, {{"valor", 90}}},
        {8, {{"valor", 100}}},
        {9, {{"valor", 110}}},
        {10, {{"valor", 120}}},
        {11, {{"valor", 130}}},
        {12, {{"valor", 140}}},
        {13, {{"valor", 150}}},
        {14, {{"valor", 160}}},
        {15, {{"valor", 170}}},
        {16, {{"valor", 180}}},
        {17, {{"valor", 190}}},
        {18, {{"valor", 200}}},
        {19, {{"valor", 210}}},
        {20, {{"valor", 220}}},
    };
    t.hab3 = H("Sifão de Fundos", "dinheiro", 30, {{"valor", 250}});
    t.hab10 = H("Invasão Bancária", "dinheiro", 60, {{"valor", 1500}});
    return t;
}

static DefTorre h_ezili() {
    DefTorre t("ezili", "Ezili", 600, "u", 150);
    t.ataques = {A("projetil", {{"cad", 1.0}, {"dano", 1}, {"pierce", 2}, {"vel", 700}, {"dist", 240}, {"dtype", "normal"}, {"queima", J::array({1, 3})}, {"visual", "maldicao"}})};
    t.cor = {110, 40, 90};
    t.heroi = true;
    t.titulo = "Sacerdotisa Vodu";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {4, {{"pierce", 1}, {"retira_regen", true}}},
        {5, {{"cad", 0.9}}},
        {6, {{"dano", 1}}},
        {7, {{"alcance", 10}}},
        {8, {{"pierce", 2}}},
        {9, {{"cad", 0.9}}},
        {11, {{"dano", 1}}},
        {12, {{"pierce", 2}, {"queima", J::array({5, 4})}, {"moab", 5}}},
        {13, {{"alcance", 10}}},
        {14, {{"cad", 0.85}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Para-Coração", "lentidao", 30, {{"dur", 8}, {"valor", 0.7}});
    t.hab10 = H("Maldição M.O.A.B.", "dano_forte", 50, {{"valor", 1500}, {"n", 3}, {"moab_so", true}});
    return t;
}

static DefTorre h_pat() {
    DefTorre t("pat", "Pat Fusty", 800, "u", 80);
    t.ataques = {A("aura", {{"cad", 1.5}, {"dano", 2}, {"pierce", 10}, {"dtype", "normal"}, {"visual", "impacto"}})};
    t.cor = {150, 110, 70};
    t.heroi = true;
    t.titulo = "Macaco Gigante";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {4, {{"pierce", 1}}},
        {5, {{"cad", 0.9}}},
        {6, {{"dano", 1}}},
        {7, {{"alcance", 10}, {"atordoa", 0.5}}},
        {8, {{"pierce", 2}}},
        {9, {{"cad", 0.9}}},
        {11, {{"dano", 1}}},
        {12, {{"pierce", 2}}},
        {13, {{"alcance", 10}, {"moab", 10}}},
        {14, {{"cad", 0.85}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Rugido de Incentivo", "turbo_area", 45, {{"dur", 10}, {"valor", 0.7}});
    t.hab10 = H("Grande Aperto", "dano_forte", 60, {{"valor", 5000}, {"n", 1}, {"moab_so", true}});
    return t;
}

static DefTorre h_adora() {
    DefTorre t("adora", "Adora", 1000, "u", 170);
    t.ataques = {A("projetil", {{"cad", 0.8}, {"dano", 2}, {"pierce", 4}, {"vel", 900}, {"dist", 280}, {"busca", true}, {"dtype", "energia"}, {"visual", "luz"}})};
    t.cor = {230, 200, 90};
    t.heroi = true;
    t.titulo = "Sacerdotisa do Sol";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {4, {{"pierce", 1}}},
        {5, {{"cad", 0.9}, {"dtype", "normal"}}},
        {6, {{"dano", 1}}},
        {7, {{"alcance", 10}}},
        {8, {{"pierce", 2}}},
        {9, {{"cad", 0.9}}},
        {10, {{"n", 2}, {"spread", 20}}},
        {11, {{"dano", 1}}},
        {12, {{"pierce", 2}}},
        {13, {{"alcance", 10}}},
        {14, {{"cad", 0.85}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}, {"moab", 8}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Braço Longo da Luz", "turbo", 40, {{"dur", 10}, {"valor", 0.4}});
    t.hab10 = H("Bola de Luz", "invocar", 60, {{"dur", 15}, {"base", "fenix"}});
    return t;
}

static DefTorre h_brickell() {
    DefTorre t("brickell", "Almirante Brickell", 900, "u", 180);
    t.ataques = {A("projetil", {{"cad", 0.4}, {"dano", 1}, {"pierce", 3}, {"vel", 900}, {"dist", 260}, {"visual", "bala"}})};
    t.agua = true;
    t.cor = {40, 70, 130};
    t.heroi = true;
    t.titulo = "Comandante Naval";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {4, {{"pierce", 1}}},
        {5, {{"cad", 0.9}}},
        {6, {{"dano", 1}, {"dtype", "normal"}}},
        {7, {{"alcance", 10}}},
        {8, {{"pierce", 2}}},
        {9, {{"cad", 0.9}}},
        {11, {{"dano", 1}}},
        {12, {{"pierce", 2}}},
        {13, {{"alcance", 10}, {"buffs", {{"cad", 0.85}}}}},
        {14, {{"cad", 0.85}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Táticas Navais", "turbo_area", 45, {{"dur", 10}, {"valor", 0.5}, {"global_", true}});
    t.hab10 = H("Mega Mina", "spikes_local", 60, {{"valor", 40}, {"dano", 1500}, {"dur", 30}});
    return t;
}

static DefTorre h_etienne() {
    DefTorre t("etienne", "Etienne", 850, "u", 9999);
    t.ataques = {A("projetil", {{"cad", 0.5}, {"dano", 1}, {"pierce", 3}, {"vel", 800}, {"dist", 900}, {"busca", true}, {"global_", true}, {"visual", "drone"}})};
    t.camo = true;
    t.cor = {80, 110, 150};
    t.heroi = true;
    t.titulo = "Especialista em Drones";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {4, {{"pierce", 1}}},
        {5, {{"cad", 0.9}}},
        {6, {{"dano", 1}}},
        {7, {{"alcance", 10}}},
        {8, {{"pierce", 2}, {"n", 1}}},
        {9, {{"cad", 0.9}}},
        {11, {{"dano", 1}}},
        {12, {{"pierce", 2}}},
        {13, {{"alcance", 10}}},
        {14, {{"cad", 0.85}}},
        {15, {{"dano", 2}, {"n", 1}, {"moab", 6}}},
        {16, {{"pierce", 3}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Enxame de Drones", "turbo", 45, {{"dur", 15}, {"valor", 0.3}});
    t.hab10 = H("UCAV", "invocar", 60, {{"dur", 20}, {"base", "fenix"}});
    return t;
}

static DefTorre h_sauda() {
    DefTorre t("sauda", "Sauda", 600, "u", 70);
    t.ataques = {A("aura", {{"cad", 0.6}, {"dano", 2}, {"pierce", 8}, {"dtype", "normal"}, {"cer", 2}, {"visual", "espadas"}})};
    t.cor = {200, 120, 60};
    t.heroi = true;
    t.titulo = "Espadachim";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {4, {{"pierce", 1}}},
        {5, {{"cad", 0.9}}},
        {6, {{"dano", 1}}},
        {7, {{"alcance", 10}}},
        {8, {{"pierce", 2}}},
        {9, {{"cad", 0.9}, {"camo", true}}},
        {11, {{"dano", 1}}},
        {12, {{"pierce", 2}}},
        {13, {{"alcance", 10}}},
        {14, {{"cad", 0.85}, {"cer", 10}, {"moab", 10}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Espada Saltitante", "dano_forte", 20, {{"valor", 100}, {"n", 8}});
    t.hab10 = H("Investida da Espada", "dano_global", 45, {{"valor", 60}});
    return t;
}

static DefTorre h_psi() {
    DefTorre t("psi", "Psi", 1200, "u", 9999);
    t.ataques = {A("hitscan", {{"cad", 1.4}, {"dano", 3}, {"pierce", 1}, {"dtype", "normal"}, {"global_", true}, {"visual", "psi"}})};
    t.camo = true;
    t.cor = {160, 90, 200};
    t.heroi = true;
    t.titulo = "Macaco Psíquico";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {4, {{"pierce", 1}}},
        {5, {{"cad", 0.9}}},
        {6, {{"dano", 1}, {"moab", 5}}},
        {7, {{"alcance", 10}}},
        {8, {{"pierce", 2}}},
        {9, {{"cad", 0.9}}},
        {11, {{"dano", 1}}},
        {12, {{"pierce", 2}}},
        {13, {{"alcance", 10}}},
        {14, {{"cad", 0.85}, {"moab", 20}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Explosão Psíquica", "dano_forte", 25, {{"valor", 300}, {"n", 3}});
    t.hab10 = H("Grito Psiônico", "dano_global", 60, {{"valor", 150}, {"atordoa", 4}});
    return t;
}

static DefTorre h_geraldo() {
    DefTorre t("geraldo", "Geraldo", 725, "u", 150);
    t.ataques = {A("projetil", {{"cad", 0.8}, {"dano", 1}, {"pierce", 2}, {"vel", 900}, {"dist", 260}, {"visual", "bala"}})};
    t.cor = {120, 70, 40};
    t.heroi = true;
    t.titulo = "Comerciante Místico";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {4, {{"pierce", 1}}},
        {5, {{"cad", 0.9}, {"dtype", "normal"}}},
        {6, {{"dano", 1}}},
        {7, {{"alcance", 10}}},
        {8, {{"pierce", 2}}},
        {9, {{"cad", 0.9}}},
        {11, {{"dano", 1}, {"camo", true}}},
        {12, {{"pierce", 2}}},
        {13, {{"alcance", 10}}},
        {14, {{"cad", 0.85}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Torreta Atiradora", "invocar", 40, {{"dur", 25}, {"base", "sentinela"}});
    t.hab10 = H("Armadilha de Lâminas", "spikes_local", 50, {{"valor", 400}, {"dano", 4}, {"dur", 20}});
    return t;
}

static DefTorre h_corvus() {
    DefTorre t("corvus", "Corvus", 1150, "u", 170);
    t.ataques = {A("projetil", {{"cad", 0.6}, {"dano", 2}, {"pierce", 4}, {"vel", 800}, {"dist", 280}, {"busca", true}, {"dtype", "energia"}, {"visual", "espirito"}})};
    t.cor = {40, 40, 80};
    t.heroi = true;
    t.titulo = "Guardião das Almas";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {4, {{"pierce", 1}, {"camo", true}}},
        {5, {{"cad", 0.9}}},
        {6, {{"dano", 1}}},
        {7, {{"alcance", 10}}},
        {8, {{"pierce", 2}}},
        {9, {{"cad", 0.9}}},
        {11, {{"dano", 1}}},
        {12, {{"pierce", 2}, {"moab", 10}}},
        {13, {{"alcance", 10}}},
        {14, {{"cad", 0.85}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Lança Espiritual", "dano_forte", 25, {{"valor", 200}, {"n", 2}});
    t.hab10 = H("Colheita de Almas", "dano_global", 60, {{"valor", 80}});
    return t;
}

static DefTorre h_rosalia() {
    DefTorre t("rosalia", "Rosalia", 800, "u", 170);
    t.ataques = {A("projetil", {{"cad", 0.8}, {"dano", 2}, {"pierce", 3}, {"vel", 1000}, {"dist", 300}, {"dtype", "energia"}, {"visual", "laser"}})};
    t.cor = {200, 90, 120};
    t.heroi = true;
    t.titulo = "Engenheira com Jetpack";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {4, {{"pierce", 1}}},
        {5, {{"cad", 0.9}}},
        {6, {{"dano", 1}, {"dtype", "normal"}}},
        {7, {{"alcance", 10}}},
        {8, {{"pierce", 2}}},
        {9, {{"cad", 0.9}}},
        {11, {{"dano", 1}}},
        {12, {{"pierce", 2}}},
        {13, {{"alcance", 10}, {"splash", 25}, {"sdano", 2}, {"spierce", 8}}},
        {14, {{"cad", 0.85}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Propulsores", "turbo", 40, {{"dur", 10}, {"valor", 0.4}});
    t.hab10 = H("Tempestade de Foguetes", "dano_global", 60, {{"valor", 50}});
    return t;
}

static DefTorre h_jericho() {
    DefTorre t("jericho", "Jericho", 750, "u", 160);
    t.ataques = {A("projetil", {{"cad", 0.8}, {"dano", 1}, {"pierce", 2}, {"vel", 950}, {"dist", 260}, {"visual", "bala"}})};
    t.cor = {110, 80, 50};
    t.heroi = true;
    t.titulo = "Bandoleiro (exclusivo do Battles)";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {4, {{"pierce", 1}}},
        {5, {{"cad", 0.9}}},
        {6, {{"dano", 1}, {"dtype", "normal"}}},
        {7, {{"alcance", 10}}},
        {8, {{"pierce", 2}}},
        {9, {{"cad", 0.9}}},
        {11, {{"dano", 1}}},
        {12, {{"pierce", 2}, {"camo", true}}},
        {13, {{"alcance", 10}}},
        {14, {{"cad", 0.85}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Proteger a Missão", "turbo", 30, {{"dur", 8}, {"valor", 0.5}});
    t.hab10 = H("Salteador", "roubo", 60, {{"valor", 600}});
    return t;
}

static DefTorre h_silas() {
    DefTorre t("silas", "Silas", 700, "u", 140);
    t.ataques = {A("projetil", {{"cad", 0.9}, {"dano", 1}, {"pierce", 3}, {"vel", 800}, {"dist", 240}, {"dtype", "gelo"}, {"lento", J::array({0.6, 1.5})}, {"visual", "gelo_bola"}})};
    t.cor = {120, 190, 230};
    t.heroi = true;
    t.titulo = "Mago do Gelo";
    t.niveis = {
        {2, {{"alcance", 8}}},
        {4, {{"pierce", 1}}},
        {5, {{"cad", 0.9}, {"dtype", "normal"}}},
        {6, {{"dano", 1}}},
        {7, {{"alcance", 10}}},
        {8, {{"pierce", 2}}},
        {9, {{"cad", 0.9}}},
        {11, {{"dano", 1}}},
        {12, {{"pierce", 2}, {"congela", 0.8}}},
        {13, {{"alcance", 10}}},
        {14, {{"cad", 0.85}}},
        {15, {{"dano", 2}}},
        {16, {{"pierce", 3}}},
        {17, {{"cad", 0.85}}},
        {18, {{"dano", 2}}},
        {19, {{"pierce", 4}}},
        {20, {{"dano", 4}, {"cad", 0.8}}},
    };
    t.hab3 = H("Raio Congelante", "dano_forte", 25, {{"valor", 60}, {"n", 4}, {"congela", 3}});
    t.hab10 = H("Tempestade Glacial", "congelar_global", 60, {{"dur", 6}, {"moab", true}});
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
    v.push_back(TipoBloon{"chumbo", "Chumbo", {125, 130, 140}, 14.0, 1.0, 1, {"preto", "preto"}, DT_AFIADO | DT_GELO, false, true, false, 7, 4});
    v.push_back(TipoBloon{"zebra", "Zebra", {230, 230, 230}, 14.0, 1.8, 1, {"preto", "branco"}, DT_EXPLOSAO | DT_GELO, false, false, false, 7, 0});
    v.push_back(TipoBloon{"arco_iris", "Arco-íris", {255, 140, 0}, 15.0, 2.2, 1, {"zebra", "zebra"}, 0, false, true, false, 8, 0});
    v.push_back(TipoBloon{"ceramica", "Cerâmica", {170, 100, 45}, 15.5, 2.5, 10, {"arco_iris", "arco_iris"}, 0, false, true, false, 9, 20});
    v.push_back(TipoBloon{"moab", "M.O.A.B.", {50, 110, 220}, 42.0, 1.0, 200, {"ceramica", "ceramica", "ceramica", "ceramica"}, 0, true, false, false, 10, 400});
    v.push_back(TipoBloon{"bfb", "B.F.B.", {200, 40, 40}, 55.0, 0.25, 700, {"moab", "moab", "moab", "moab"}, 0, true, false, false, 11, 1400});
    v.push_back(TipoBloon{"zomg", "Z.O.M.G.", {40, 120, 40}, 66.0, 0.18, 4000, {"bfb", "bfb", "bfb", "bfb"}, 0, true, false, false, 12, 8000});
    v.push_back(TipoBloon{"ddt", "D.D.T.", {45, 45, 50}, 40.0, 2.75, 400, {"ceramica", "ceramica", "ceramica", "ceramica", "ceramica", "ceramica"}, DT_AFIADO | DT_EXPLOSAO, true, false, true, 12, 800});
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
    return {h_quincy(), h_gwendolin(), h_striker(), h_obyn(), h_churchill(), h_benjamin(), h_ezili(), h_pat(), h_adora(), h_brickell(), h_etienne(), h_sauda(), h_psi(), h_geraldo(), h_corvus(), h_rosalia(), h_jericho(), h_silas()};
}

const std::map<std::string, std::string>& regen_proximo() { return REGEN_PROXIMO_DADOS; }

// XP acumulado necessario para chegar a cada nivel (indice = nivel): 180 * (n - 1)^1.9
const std::vector<int> XP_NIVEL = {0, 0, 180, 671, 1451, 2507, 3831, 5417, 7260, 9357, 11703, 14297, 17136, 20217, 23537, 27096, 30891, 34922, 39185, 43680, 48406};

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
// Rodadas inspiradas no modo classico: {tipo, qtd, espaco (s), inicio (s), camo, regen, fort}
const std::map<int, std::vector<Grupo>> RODADAS = {
    {1, {{"vermelho", 20, 0.9, 0.0, false, false, false}}},
    {2, {{"vermelho", 35, 0.55, 0.0, false, false, false}}},
    {3, {{"vermelho", 25, 0.6, 0.0, false, false, false}, {"azul", 5, 1.0, 8.0, false, false, false}}},
    {4, {{"vermelho", 35, 0.45, 0.0, false, false, false}, {"azul", 18, 0.8, 4.0, false, false, false}}},
    {5, {{"vermelho", 5, 0.8, 0.0, false, false, false}, {"azul", 27, 0.5, 2.0, false, false, false}}},
    {6, {{"vermelho", 15, 0.6, 0.0, false, false, false}, {"azul", 15, 0.6, 3.0, false, false, false}, {"verde", 4, 1.2, 8.0, false, false, false}}},
    {7, {{"vermelho", 20, 0.5, 0.0, false, false, false}, {"azul", 25, 0.5, 4.0, false, false, false}, {"verde", 5, 0.9, 12.0, false, false, false}}},
    {8, {{"vermelho", 10, 0.5, 0.0, false, false, false}, {"azul", 20, 0.5, 2.0, false, false, false}, {"verde", 14, 0.6, 8.0, false, false, false}}},
    {9, {{"verde", 30, 0.55, 0.0, false, false, false}}},
    {10, {{"azul", 102, 0.17, 0.0, false, false, false}}},
    {11, {{"vermelho", 10, 0.5, 0.0, false, false, false}, {"azul", 10, 0.5, 2.0, false, false, false}, {"verde", 12, 0.5, 5.0, false, false, false}, {"amarelo", 2, 1.5, 9.0, false, false, false}}},
    {12, {{"azul", 15, 0.5, 0.0, false, false, false}, {"verde", 10, 0.6, 4.0, false, false, false}, {"amarelo", 5, 1.0, 8.0, false, false, false}}},
    {13, {{"azul", 50, 0.3, 0.0, false, false, false}, {"verde", 23, 0.5, 6.0, false, false, false}}},
    {14, {{"vermelho", 49, 0.2, 0.0, false, false, false}, {"azul", 15, 0.4, 5.0, false, false, false}, {"verde", 10, 0.5, 9.0, false, false, false}, {"amarelo", 9, 0.7, 13.0, false, false, false}}},
    {15, {{"vermelho", 20, 0.3, 0.0, false, false, false}, {"verde", 15, 0.5, 4.0, false, false, false}, {"amarelo", 12, 0.6, 9.0, false, false, false}, {"rosa", 5, 1.0, 14.0, false, false, false}}},
    {16, {{"verde", 20, 0.4, 0.0, false, false, false}, {"amarelo", 8, 0.8, 5.0, false, false, false}}},
    {17, {{"amarelo", 8, 0.8, 0.0, false, true, false}}},
    {18, {{"verde", 80, 0.22, 0.0, false, false, false}}},
    {19, {{"verde", 10, 0.4, 0.0, false, false, false}, {"amarelo", 4, 0.8, 3.0, false, false, false}, {"amarelo", 5, 0.8, 7.0, false, true, false}, {"rosa", 7, 0.7, 11.0, false, false, false}}},
    {20, {{"preto", 6, 1.0, 0.0, false, false, false}}},
    {21, {{"amarelo", 14, 0.5, 0.0, false, false, false}, {"rosa", 40, 0.3, 5.0, false, false, false}}},
    {22, {{"branco", 16, 0.8, 0.0, false, false, false}}},
    {23, {{"preto", 7, 0.8, 0.0, false, false, false}, {"branco", 7, 0.8, 4.0, false, false, false}}},
    {24, {{"verde", 1, 1.0, 0.0, true, false, false}, {"azul", 20, 0.4, 2.0, false, false, false}}},
    {25, {{"amarelo", 31, 0.4, 0.0, false, true, false}, {"roxo", 10, 0.8, 8.0, false, false, false}}},
    {26, {{"rosa", 23, 0.4, 0.0, false, false, false}, {"zebra", 4, 1.2, 8.0, false, false, false}}},
    {27, {{"vermelho", 100, 0.1, 0.0, false, false, false}, {"azul", 60, 0.12, 5.0, false, false, false}, {"verde", 45, 0.15, 10.0, false, false, false}, {"amarelo", 45, 0.2, 15.0, false, false, false}}},
    {28, {{"chumbo", 6, 1.2, 0.0, false, false, false}}},
    {29, {{"amarelo", 48, 0.3, 0.0, false, false, false}, {"rosa", 12, 0.6, 8.0, false, true, false}}},
    {30, {{"chumbo", 9, 1.0, 0.0, false, false, false}}},
    {31, {{"preto", 8, 0.6, 0.0, false, false, false}, {"branco", 8, 0.6, 3.0, false, false, false}, {"zebra", 4, 1.0, 7.0, false, true, false}}},
    {32, {{"preto", 25, 0.35, 0.0, false, false, false}, {"branco", 28, 0.35, 5.0, false, false, false}}},
    {33, {{"vermelho", 13, 0.3, 0.0, false, false, false}, {"amarelo", 20, 0.4, 3.0, true, false, false}}},
    {34, {{"amarelo", 140, 0.12, 0.0, false, false, false}, {"zebra", 5, 1.0, 10.0, false, false, false}}},
    {35, {{"rosa", 35, 0.25, 0.0, false, false, false}, {"preto", 30, 0.3, 5.0, false, false, false}, {"branco", 25, 0.3, 10.0, false, false, false}, {"arco_iris", 5, 1.2, 15.0, false, false, false}}},
    {36, {{"rosa", 81, 0.15, 0.0, false, false, false}}},
    {37, {{"preto", 20, 0.4, 0.0, false, false, false}, {"branco", 20, 0.4, 4.0, false, false, false}, {"zebra", 15, 0.5, 9.0, false, true, false}, {"branco", 10, 0.6, 14.0, true, false, false}}},
    {38, {{"rosa", 42, 0.2, 0.0, false, false, false}, {"branco", 17, 0.4, 4.0, false, false, false}, {"chumbo", 14, 0.6, 8.0, false, false, false}, {"zebra", 10, 0.5, 13.0, false, false, false}, {"ceramica", 4, 1.5, 17.0, false, false, false}}},
    {39, {{"preto", 10, 0.4, 0.0, false, false, false}, {"branco", 10, 0.4, 2.0, false, false, false}, {"chumbo", 20, 0.4, 6.0, false, false, false}, {"arco_iris", 18, 0.5, 11.0, false, true, false}}},
    {40, {{"moab", 1, 1.0, 0.0, false, false, false}}},
    {41, {{"preto", 60, 0.2, 0.0, false, false, false}, {"zebra", 60, 0.2, 6.0, false, false, false}}},
    {42, {{"arco_iris", 6, 0.8, 0.0, false, true, false}, {"arco_iris", 4, 0.8, 6.0, true, false, false}}},
    {43, {{"arco_iris", 10, 0.6, 0.0, false, false, false}, {"ceramica", 7, 0.9, 6.0, false, false, false}}},
    {44, {{"zebra", 50, 0.25, 0.0, false, false, false}}},
    {45, {{"rosa", 200, 0.08, 0.0, false, false, false}, {"ceramica", 8, 0.8, 8.0, false, true, false}}},
    {46, {{"preto", 10, 0.5, 0.0, true, false, false}, {"moab", 1, 1.0, 6.0, false, false, false}}},
    {47, {{"rosa", 70, 0.15, 0.0, true, false, false}, {"ceramica", 12, 0.6, 8.0, false, false, false}}},
    {48, {{"rosa", 120, 0.1, 0.0, false, true, false}, {"arco_iris", 50, 0.25, 10.0, false, false, false}}},
    {49, {{"verde", 343, 0.05, 0.0, false, false, false}, {"zebra", 20, 0.3, 10.0, false, false, false}, {"arco_iris", 30, 0.3, 14.0, false, false, false}, {"ceramica", 15, 0.6, 20.0, false, false, false}}},
    {50, {{"chumbo", 20, 0.4, 0.0, false, false, true}, {"moab", 2, 2.0, 6.0, false, false, false}}},
    {51, {{"arco_iris", 28, 0.3, 0.0, true, false, false}, {"ceramica", 10, 0.5, 8.0, false, false, false}}},
    {52, {{"ceramica", 25, 0.4, 0.0, false, true, false}, {"moab", 2, 2.0, 8.0, false, false, false}}},
    {53, {{"rosa", 80, 0.1, 0.0, true, false, false}, {"moab", 3, 1.5, 6.0, false, false, false}}},
    {54, {{"ceramica", 35, 0.35, 0.0, false, false, false}, {"moab", 2, 2.0, 10.0, false, false, false}}},
    {55, {{"ceramica", 45, 0.3, 0.0, false, true, false}}},
    {56, {{"arco_iris", 40, 0.25, 0.0, true, false, false}, {"moab", 3, 1.5, 8.0, false, false, false}}},
    {57, {{"ceramica", 40, 0.3, 0.0, false, false, false}, {"moab", 4, 1.2, 10.0, false, false, false}}},
    {58, {{"chumbo", 30, 0.3, 0.0, false, false, true}, {"ceramica", 25, 0.3, 8.0, false, false, true}}},
    {59, {{"ceramica", 50, 0.25, 0.0, false, true, false}, {"moab", 3, 1.5, 12.0, false, false, false}}},
    {60, {{"bfb", 1, 1.0, 0.0, false, false, false}}},
    {61, {{"zebra", 120, 0.08, 0.0, true, false, false}, {"moab", 5, 1.0, 8.0, false, false, false}}},
    {62, {{"ceramica", 60, 0.2, 0.0, true, false, false}, {"moab", 4, 1.2, 10.0, false, false, false}}},
    {63, {{"chumbo", 50, 0.2, 0.0, false, false, false}, {"ceramica", 75, 0.15, 6.0, false, false, false}}},
    {64, {{"moab", 9, 0.8, 0.0, false, false, false}}},
    {65, {{"zebra", 80, 0.1, 0.0, false, false, false}, {"arco_iris", 60, 0.15, 6.0, false, false, false}, {"ceramica", 40, 0.25, 12.0, false, false, false}, {"bfb", 1, 1.0, 18.0, false, false, false}}},
    {66, {{"ceramica", 50, 0.2, 0.0, false, false, true}, {"moab", 4, 1.0, 8.0, false, false, false}}},
    {67, {{"moab", 6, 0.8, 0.0, false, false, false}, {"ceramica", 40, 0.25, 4.0, true, false, false}}},
    {68, {{"moab", 4, 1.0, 0.0, false, false, true}, {"bfb", 1, 1.0, 6.0, false, false, false}}},
    {69, {{"chumbo", 60, 0.15, 0.0, false, false, true}, {"ceramica", 50, 0.2, 8.0, false, true, false}}},
    {70, {{"arco_iris", 200, 0.05, 0.0, false, false, false}, {"moab", 4, 1.0, 10.0, false, false, false}}},
    {71, {{"ceramica", 70, 0.15, 0.0, true, true, false}, {"moab", 5, 0.8, 10.0, false, false, false}}},
    {72, {{"chumbo", 50, 0.2, 0.0, false, false, false}, {"moab", 8, 0.6, 6.0, false, false, true}}},
    {73, {{"ceramica", 100, 0.1, 0.0, false, false, true}}},
    {74, {{"bfb", 3, 2.0, 0.0, false, false, false}, {"moab", 8, 0.6, 4.0, false, false, false}}},
    {75, {{"ceramica", 80, 0.12, 0.0, true, false, false}, {"bfb", 2, 2.0, 10.0, false, false, false}}},
    {76, {{"ceramica", 120, 0.08, 0.0, false, true, true}}},
    {77, {{"moab", 14, 0.5, 0.0, false, false, true}, {"bfb", 2, 2.0, 8.0, false, false, false}}},
    {78, {{"ceramica", 150, 0.07, 0.0, false, false, true}, {"bfb", 3, 1.5, 10.0, false, false, false}}},
    {79, {{"moab", 20, 0.4, 0.0, false, false, true}, {"bfb", 3, 1.5, 8.0, false, false, false}}},
    {80, {{"zomg", 1, 1.0, 0.0, false, false, false}, {"bfb", 2, 2.0, 5.0, false, false, false}}},
    {85, {{"zomg", 2, 2.0, 0.0, false, false, false}, {"ceramica", 120, 0.08, 4.0, false, false, true}}},
    {90, {{"ddt", 6, 1.0, 0.0, false, false, false}}},
    {95, {{"zomg", 4, 1.5, 0.0, false, false, true}, {"ddt", 10, 0.6, 6.0, false, false, false}}},
    {100, {{"bad", 1, 1.0, 0.0, false, false, false}}},
};

// Dificuldades do modo solo: {chave, nome, vidas, multiplicador de custo, ultima rodada}
const std::vector<Dificuldade> DIFICULDADES = {
    {"facil", "Fácil", 200, 0.85, 40},
    {"medio", "Médio", 150, 1.0, 60},
    {"dificil", "Difícil", 100, 1.08, 80},
    {"impossivel", "Impossível", 1, 1.2, 100},
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
