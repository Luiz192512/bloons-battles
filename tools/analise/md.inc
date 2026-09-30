static std::string hab_txt(const J& h){ std::string t=h["tipo"]; std::string n=h["nome"]; std::string r=fmt(h["recarga"].get<double>());
 auto V=[&](const char*k){return h.contains(k)?fmt(h[k].is_boolean()?(h[k].get<bool>()?1.0:0.0):h[k].get<double>()):std::string("?");};
 std::string d;
 if(t=="turbo") d="a própria torre ataca "+fmt(1/h["valor"].get<double>())+"× mais rápido por "+V("dur")+"s";
 else if(t=="turbo_area") d=std::string(h.value("global_",false)?"todas as torres":"torres a até alcance+60px")+(h.contains("filtro")?" do tipo "+h["filtro"].get<std::string>():"")+" atacam "+fmt(1/h["valor"].get<double>())+"× mais rápido por "+V("dur")+"s";
 else if(t=="dano_global") d=V("valor")+" de dano [normal] em todo bloon na tela"+(h.contains("atordoa")?" + atordoa "+V("atordoa")+"s":"")+(h.contains("queima")?" + fogo":"");
 else if(t=="dano_forte") d=V("valor")+" de dano em "+V("n")+" alvo(s) mais forte(s)"+(h.value("moab_so",false)?" (só MOAB)":"")+(h.contains("splash")?" + explosão r"+V("splash")+" d"+V("sdano"):"")+(h.contains("atordoa")?" + atordoa "+V("atordoa")+"s":"")+(h.contains("congela")?" + congela "+V("congela")+"s":"");
 else if(t=="lentidao") d="todos os bloons a ×"+V("valor")+" de velocidade por "+V("dur")+"s"+(h.contains("dano")?" + "+V("dano")+" de dano em todos":"");
 else if(t=="congelar_global") d="congela todos por "+V("dur")+"s"+(h.value("moab",false)?" (inclui MOAB, metade do tempo)":" (não pega MOAB/branco/zebra)");
 else if(t=="dinheiro") d="+$"+V("valor");
 else if(t=="roubo") d="+$"+V("valor")+" e tira o mesmo do oponente";
 else if(t=="invocar") d="invoca "+h.value("base",std::string("sentinela"))+" por "+V("dur")+"s";
 else if(t=="reverso") d="recua todos os bloons "+V("valor")+"px (MOAB metade)";
 else if(t=="spikes_global") d="pilhas de "+V("valor")+" espinhos (dano "+V("dano")+") a cada 140px de toda trilha, 20s";
 else if(t=="spikes_local") d="pilha de "+V("valor")+" pierce (dano "+V("dano")+") no ponto mais avançado da trilha no alcance, "+V("dur")+"s";
 return "**Hab. "+n+"** ("+r+"s): "+d; }
static std::string ef_txt(const J& ef){ std::vector<std::string> v; std::string alvo;
 if(ef.contains("a")) alvo= ef["a"].is_string()? " (em todos os ataques)" : " (no ataque ["+std::to_string(ef["a"].get<int>())+"])";
 for(auto&[k,x]:ef.items()){ auto N=[&]{return fmt(x.is_boolean()?(x.get<bool>()?1.0:0.0):x.get<double>());};
  if(k=="a"||k=="visual") continue;
  else if(k=="pierce") v.push_back("+"+N()+" pierce");
  else if(k=="dano") v.push_back("+"+N()+" dano");
  else if(k=="cad"){ double f=x.get<double>(); char b[64]; snprintf(b,64,"recarga ×%s (%+.0f%% ataques/s)",fmt(f).c_str(),(1/f-1)*100); v.push_back(b);} 
  else if(k=="vel") v.push_back("vel. projétil ×"+N());
  else if(k=="n") v.push_back("+"+N()+" projéteis");
  else if(k=="spread") v.push_back("leque "+N()+"°");
  else if(k=="alcance") v.push_back("+"+N()+" alcance");
  else if(k=="alcance_x") v.push_back("alcance ×"+N());
  else if(k=="dist") v.push_back("+"+N()+" distância do projétil");
  else if(k=="raio_proj") v.push_back("+"+N()+" raio do projétil");
  else if(k=="camo") v.push_back("detecta camo");
  else if(k=="dtype") v.push_back("dano vira ["+x.get<std::string>()+"]");
  else if(k=="sdtype") v.push_back("explosão vira ["+x.get<std::string>()+"]");
  else if(k=="splash") v.push_back("+"+N()+" raio da explosão");
  else if(k=="sdano") v.push_back("+"+N()+" dano da explosão");
  else if(k=="spierce") v.push_back("+"+N()+" pierce da explosão");
  else if(k=="moab") v.push_back("+"+N()+" dano em MOAB");
  else if(k=="cer") v.push_back("+"+N()+" dano em cerâmica");
  else if(k=="fort") v.push_back("+"+N()+" dano em fortificado");
  else if(k=="quica") v.push_back("+"+N()+" ricochete(s)");
  else if(k=="saltos") v.push_back("+"+N()+" saltos da corrente");
  else if(k=="empurra") v.push_back("+"+N()+"px de empurrão");
  else if(k=="fragiliza") v.push_back("+"+N()+" fragilização (dano extra por golpe que o bloon passa a sofrer)");
  else if(k=="atordoa") v.push_back("atordoa "+N()+"s");
  else if(k=="congela") v.push_back("congela "+N()+"s");
  else if(k=="lento") v.push_back("lentidão ×"+fmt(x[0].get<double>())+" por "+fmt(x[1].get<double>())+"s");
  else if(k=="cola") v.push_back("cola ×"+fmt(x[0].get<double>())+" por "+fmt(x[1].get<double>())+"s"+(x[2].get<double>()?", corrói "+fmt(x[2].get<double>())+" dano/s":""));
  else if(k=="queima") v.push_back("fogo "+fmt(x[0].get<double>())+" dano/s por "+fmt(x[1].get<double>())+"s");
  else if(k=="frag") v.push_back(fmt(x.value("n",6.0))+" fragmentos (dano "+fmt(x.value("dano",1.0))+", pierce "+fmt(x.value("pierce",1.0))+(x.contains("splash")?", explodem r"+fmt(x["splash"].get<double>()):"")+")");
  else if(k=="busca") v.push_back("teleguiado");
  else if(k=="global_") v.push_back("alcance global");
  else if(k=="retira_camo") v.push_back("remove camo");
  else if(k=="retira_regen") v.push_back("remove regen");
  else if(k=="moab_lento") v.push_back("lentidão pega MOAB");
  else if(k=="moab_congela") v.push_back("congela MOAB");
  else if(k=="moab_cola") v.push_back("cola pega MOAB");
  else if(k=="moab_atordoa") v.push_back("atordoa MOAB");
  else if(k=="ouro") v.push_back("+$"+N()+" por bloon estourado (+$2 extra em chumbo)");
  else if(k=="desconto") v.push_back(fmt(x.get<double>()*100)+"% de desconto p/ torres no alcance");
  else if(k=="venda") v.push_back("venda por "+fmt(x.get<double>()*100)+"%");
  else if(k=="persegue") v.push_back("persegue bloons no mapa todo");
  else if(k=="valor") v.push_back("+$"+N()+"/rodada");
  else if(k=="valor_x") v.push_back("renda ×"+N());
  else if(k=="nivel_inv") v.push_back("+"+N()+" nível nas torretas (cada nível: +1 dano, +2 pierce, recarga ×0,8)");
  else if(k=="impreciso") v.push_back("imprecisão ×"+N());
  else if(k=="pilha_pierce") v.push_back("+"+N()+" pierce por pilha");
  else if(k=="pilha_vida") v.push_back("duração da pilha ×"+N());
  else if(k=="buffs"){ Buffs b; b.mesclar(x); std::string s="aura de buff:"; if(b.cad!=1) s+=" recarga ×"+fmt(b.cad); if(b.alcance_pct) s+=" +"+fmt(b.alcance_pct*100)+"% alcance"; if(b.pierce) s+=" +"+fmt(b.pierce)+" pierce"; if(b.dano) s+=" +"+fmt(b.dano)+" dano"; if(b.moab) s+=" +"+fmt(b.moab)+" MOAB"; if(b.ouro) s+=" +$"+fmt(b.ouro)+"/estouro"; if(b.camo) s+=" camo"; if(b.dtype_normal) s+=" dano normal"; v.push_back(s);} 
  else if(k=="novo") v.push_back("**novo ataque**: "+ataque(novo_ataque(x)));
  else if(k=="subst") v.push_back("**substitui o ataque [0]** por: "+ataque(novo_ataque(x)));
  else if(k=="hab") v.push_back(hab_txt(x));
  else v.push_back(k+"="+x.dump()); }
 std::string s; for(auto&e:v){ if(!s.empty()) s+="; "; s+=e;} if(s.empty()) s="*(sem efeito no código)*"; return s+alvo; }
static std::string m1(double v){char b[32]; if(v<10) snprintf(b,32,"%.1f",v); else snprintf(b,32,"%.0f",v); return b;}
static void md_torres(){ for(auto&t:torres()){ Stats b=calcular(t.chave); Pot p=pot(b);
  printf("\n### %s — $%d · alcance %s · %s%s%s%s\n\n", t.nome.c_str(), t.custo, t.alcance>=5000?"global":fmt(t.alcance).c_str(), t.categoria.c_str(), t.agua?" · só água":"", t.mov==Mov::ORBITA?" · voa em órbita":t.mov==Mov::HELI?" · helicóptero móvel":"", t.camo?" · detecta camo":"");
  printf("**Base:** %s. Dano/s 1 alvo **%s**, potencial em área **%s**/s.\n\n", ataque(b.ataques[0]).c_str(), m1(p.um).c_str(), m1(p.area).c_str());
  printf("| Up | Nome | $ (acum.) | Mecânica no código | 1 alvo/s | Área/s | MOAB/s |\n|---|---|---|---|---|---|---|\n");
  for(int q=0;q<3;++q){ int ac=t.custo; for(int i=0;i<5;++i){ std::array<int,3> c{0,0,0}; c[q]=i+1; auto&u=t.caminhos[q][i]; ac+=u.custo; Pot pp=pot(calcular(t.chave,c)); std::string ex=pp.renda?(" · $"+fmt(pp.renda)+"/rod."):"";
   printf("| %d-%d | %s | %d (%d) | %s | %s | %s%s | %s |\n", q+1,i+1,u.nome.c_str(),u.custo,ac,ef_txt(u.ef).c_str(),m1(pp.um).c_str(),m1(pp.area).c_str(),ex.c_str(),m1(pp.moab).c_str()); } }
  printf("\n<!--NOTA:%s-->\n", t.chave.c_str()); } }
static void md_herois(){ printf("| Herói | $ | Alcance | Ataque nível 1 | Nv1 1alvo/área | Nv10 1alvo/área | Nv20 1alvo/área (MOAB) | Habilidades (nv3 / nv10) |\n|---|---|---|---|---|---|---|---|\n");
 for(auto&t:herois()){ Stats a=calcular(t.chave,{0,0,0},1), b=calcular(t.chave,{0,0,0},10), c=calcular(t.chave,{0,0,0},20); Pot pa=pot(a),pb=pot(b),pc=pot(c);
  std::string h3=t.hab3.is_null()?"-":hab_txt(t.hab3), h10=t.hab10.is_null()?"-":hab_txt(t.hab10);
  auto R=[&](Pot&p){ return p.renda? "$"+fmt(p.renda)+"/rod.": m1(p.um)+" / "+m1(p.area);};
  printf("| **%s** (%s) | %d | %s | %s | %s | %s | %s (%s) | %s · %s |\n", t.nome.c_str(), t.titulo.c_str(), t.custo, t.alcance>=5000?"global":fmt(t.alcance).c_str(), ataque(a.ataques[0]).c_str(), R(pa).c_str(), R(pb).c_str(), R(pc).c_str(), m1(pc.moab).c_str(), h3.c_str(), h10.c_str()); } }
