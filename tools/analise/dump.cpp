#include "jogo/defs.hpp"
#include "jogo/stats.hpp"
#include "jogo/mapas.hpp"
#include "jogo/rodadas.hpp"
#include "jogo/sim.hpp"
#include <cstdio>
#include <cmath>
using namespace bl;
static std::string dt(DType d){ if(d==DT_NORMAL) return "normal"; std::string s; if(d&DT_AFIADO)s+="afiado"; if(d&DT_EXPLOSAO)s+=(s.empty()?"":"+")+std::string("explosão"); if(d&DT_GELO)s+=(s.empty()?"":"+")+std::string("gelo"); if(d&DT_ENERGIA)s+=(s.empty()?"":"+")+std::string("energia"); return s.empty()?"-":s;}
static const char* tn(TipoAtaque t){switch(t){case TipoAtaque::PROJETIL:return "projétil";case TipoAtaque::RADIAL:return "radial";case TipoAtaque::AURA:return "aura";case TipoAtaque::HITSCAN:return "hitscan";case TipoAtaque::CADEIA:return "cadeia";case TipoAtaque::MORTEIRO:return "morteiro";case TipoAtaque::PILHA:return "pilha";case TipoAtaque::QUEDA:return "queda";case TipoAtaque::RENDA:return "renda";case TipoAtaque::BUFF:return "buff";case TipoAtaque::INVOCAR:return "invocar";}return "?";}
static std::string fmt(double v){char b[32]; if(std::fabs(v-std::round(v))<1e-9) snprintf(b,32,"%.0f",v); else snprintf(b,32,"%.3g",v); return b;}
// potencial de dano/s (todos os alvos) e dano/s em 1 alvo, e em MOAB
struct Pot{double area=0, um=0, moab=0, renda=0;};
static Pot pot(const Stats& st){Pot p; for(auto&a:st.ataques){ double c=std::max(0.02,a.cad);
  switch(a.tipo){
  case TipoAtaque::RENDA: p.renda+=a.valor; break;
  case TipoAtaque::BUFF: case TipoAtaque::INVOCAR: break;
  case TipoAtaque::PILHA: p.area+=a.pilha_pierce*a.dano/c; p.um+=a.dano/c; p.moab+=(a.dano>0?a.dano+a.moab:0)/c; break;
  case TipoAtaque::AURA: p.area+=a.pierce*a.dano/c; p.um+=a.dano/c; p.moab+=(a.dano>0?a.dano+a.moab:0)/c; break;
  case TipoAtaque::CADEIA: p.area+=(a.saltos+1)*a.dano/c; p.um+=a.dano/c; p.moab+=(a.dano+a.moab)/c; break;
  case TipoAtaque::QUEDA: case TipoAtaque::MORTEIRO: {double n=std::max(1.0,a.n); double sd=a.sdano?a.sdano:1, sp=a.spierce?a.spierce:20; p.area+=n*(sp*sd + (a.tipo==TipoAtaque::MORTEIRO?0:0))/c; p.um+=n*sd/c; p.moab+=n*(sd+a.moab)/c; break;}
  default: {double n=std::max(1.0,a.n); double pr=(a.tipo==TipoAtaque::HITSCAN&&!a.linha)?1+a.quica:a.pierce;
    p.area+=n*(pr*a.dano + a.spierce*a.sdano)/c; double hits=(a.tipo==TipoAtaque::RADIAL||a.spread>=360)?1:(a.spread>0&&a.spread<=12?n:1);
    p.um+=hits*(a.dano+a.sdano)/c; p.moab+=hits*(a.dano>0?a.dano+a.moab:0)/c + hits*(a.sdano)/c;}
  }} return p;}
static std::string ataque(const Ataque& a){ std::string s=tn(a.tipo);
  if(a.tipo==TipoAtaque::RENDA) return s+" $"+fmt(a.valor)+"/rodada";
  if(a.tipo==TipoAtaque::BUFF){std::string b="buff:"; auto&f=a.buffs; if(f.cad!=1)b+=" cad×"+fmt(f.cad); if(f.alcance_pct)b+=" alc+"+fmt(f.alcance_pct*100)+"%"; if(f.pierce)b+=" pierce+"+fmt(f.pierce); if(f.dano)b+=" dano+"+fmt(f.dano); if(f.moab)b+=" moab+"+fmt(f.moab); if(f.ouro)b+=" ouro+"+fmt(f.ouro); if(f.camo)b+=" camo"; if(f.dtype_normal)b+=" dtype normal"; return b;}
  if(a.tipo==TipoAtaque::INVOCAR) return s+" "+a.base+" a cada "+fmt(a.cad)+"s (dura "+fmt(a.dur)+"s, +"+fmt(a.nivel_inv)+" nív.)";
  s+=" cad "+fmt(a.cad)+"s";
  if(a.dano) s+=" dano "+fmt(a.dano);
  if(a.tipo==TipoAtaque::PILHA) s+=" pierce "+fmt(a.pilha_pierce)+" vida "+fmt(a.pilha_vida)+"s";
  else if(a.tipo!=TipoAtaque::MORTEIRO&&a.tipo!=TipoAtaque::QUEDA) s+=" pierce "+fmt(a.pierce);
  if(a.n>1) s+=" n "+fmt(a.n); if(a.spread) s+=" leque "+fmt(a.spread)+"°";
  if(a.dano||a.tipo==TipoAtaque::PILHA) s+=" ["+dt(a.dtype)+"]";
  if(a.splash) s+=" | área r"+fmt(a.splash)+" d"+fmt(a.sdano)+" p"+fmt(a.spierce)+" ["+dt(a.sdtype?a.sdtype:DT_EXPLOSAO)+"]";
  if(a.raio_aura) s+=" raio "+fmt(a.raio_aura);
  if(a.saltos) s+=" saltos "+fmt(a.saltos);
  if(a.quica) s+=" ricochete "+fmt(a.quica);
  if(a.moab) s+=" +"+fmt(a.moab)+" MOAB"; if(a.cer) s+=" +"+fmt(a.cer)+" cerâm."; if(a.fort) s+=" +"+fmt(a.fort)+" fort.";
  if(a.congela) s+=" congela "+fmt(a.congela)+"s"; if(a.atordoa) s+=" atordoa "+fmt(a.atordoa)+"s";
  if(a.tem_lento) s+=" lento ×"+fmt(a.lento_f)+" "+fmt(a.lento_t)+"s"; if(a.tem_cola) s+=" cola ×"+fmt(a.cola_f)+" "+fmt(a.cola_t)+"s"+(a.cola_dps?" "+fmt(a.cola_dps)+"dps":"");
  if(a.tem_queima) s+=" fogo "+fmt(a.queima_dps)+"dps "+fmt(a.queima_t)+"s"; if(a.empurra) s+=" empurra "+fmt(a.empurra)+"px"; if(a.fragiliza) s+=" fragiliza +"+fmt(a.fragiliza);
  if(a.frag) s+=" | "+std::to_string(a.frag_n)+" frag(d"+fmt(a.frag->dano)+" p"+fmt(a.frag->pierce)+(a.frag->splash?" área":"")+")";
  if(a.busca) s+=" teleguiado"; if(a.global_) s+=" global"; if(a.boom) s+=" volta"; if(a.so_moab) s+=" só MOAB"; if(a.alvo_forte) s+=" alvo forte";
  if(a.retira_camo) s+=" tira camo"; if(a.retira_regen) s+=" tira regen"; if(a.linha) s+=" linha";
  if(a.moab_lento) s+=" (MOAB lento)"; if(a.moab_congela) s+=" (MOAB congela)"; if(a.moab_cola) s+=" (MOAB cola)"; if(a.moab_atordoa) s+=" (MOAB atordoa)";
  if(a.impreciso) s+=" imprecisão "+fmt(a.impreciso); if(a.fusivel) s+=" pavio "+fmt(a.fusivel)+"s";
  return s;}
static std::string habs(const Stats& st){std::string s; for(auto&h:st.habs){ if(!s.empty())s+="; "; s+=h["nome"].get<std::string>()+" ("+h["tipo"].get<std::string>()+", "+fmt(h["recarga"].get<double>())+"s"; for(auto&[k,v]:h.items()) if(k!="nome"&&k!="tipo"&&k!="recarga") s+=", "+k+"="+v.dump(); s+=")";} return s;}
static void linha_stats(const Stats& st){ Pot p=pot(st);
  printf("  alcance %s%s%s%s | 1 alvo %.1f/s | área %.0f/s | MOAB %.1f/s%s\n", fmt(st.alcance).c_str(), st.camo?" camo":"", st.ouro?(" ouro+"+fmt(st.ouro)).c_str():"", st.desconto?(" desconto "+fmt(st.desconto*100)+"%").c_str():"", p.um,p.area,p.moab, p.renda?(" | renda $"+fmt(p.renda)).c_str():"");
  for(size_t i=0;i<st.ataques.size();++i) printf("   [%zu] %s\n", i, ataque(st.ataques[i]).c_str());
  if(!st.habs.empty()) printf("   hab: %s\n", habs(st).c_str());}
#include "md.inc"
int main(int argc,char**argv){ std::string modo=argc>1?argv[1]:"torres"; if(modo=="md"){md_torres();return 0;} if(modo=="mdh"){md_herois();return 0;}
 if(modo=="torres"){ for(auto& t: torres()){ printf("\n### %s (%s) $%d alcance %s cat %s%s%s\n", t.nome.c_str(), t.chave.c_str(), t.custo, fmt(t.alcance).c_str(), t.categoria.c_str(), t.agua?" água":"", t.mov!=Mov::FIXO?" voa":"");
   printf("BASE\n"); linha_stats(calcular(t.chave));
   for(int p=0;p<3;++p){ int acum=t.custo; for(int i=0;i<5;++i){ std::array<int,3> c{0,0,0}; c[p]=i+1; acum+=t.caminhos[p][i].custo; printf("P%d T%d %s $%d (acum $%d) ef=%s\n", p+1,i+1,t.caminhos[p][i].nome.c_str(), t.caminhos[p][i].custo, acum, t.caminhos[p][i].ef.dump().c_str()); linha_stats(calcular(t.chave,c)); } } } }
 if(modo=="cross"){ // variacoes com cruzamento 5-2-0 etc
   for(auto& t: torres()) for(int p=0;p<3;++p) for(int q=0;q<3;++q) if(q!=p){ std::array<int,3> c{0,0,0}; c[p]=5; c[q]=2; Stats a=calcular(t.chave,c); printf("%s %d-%d-%d: ",t.chave.c_str(),c[0],c[1],c[2]); Pot pp=pot(a); printf("1alvo %.1f area %.0f moab %.1f renda %.0f natq %zu\n",pp.um,pp.area,pp.moab,pp.renda,a.ataques.size()); for(size_t i=0;i<a.ataques.size();++i) printf("   [%zu] %s\n",i,ataque(a.ataques[i]).c_str()); } }
 if(modo=="herois"){ for(auto&t:herois()){ printf("\n### %s (%s) $%d alcance %s — %s\n",t.nome.c_str(),t.chave.c_str(),t.custo,fmt(t.alcance).c_str(),t.titulo.c_str()); for(int n: {1,5,10,15,20}){ printf("NIVEL %d\n",n); linha_stats(calcular(t.chave,{0,0,0},n)); } std::string s; for(auto&[n,ef]:t.niveis) s+=" "+std::to_string(n)+":"+ef.dump(); printf("niveis:%s\n",s.c_str()); } }
 if(modo=="bloons"){ for(auto&b:bloons()) printf("%s | vida %d (fort %d) | vel %.2fx = %.0f px/s | raio %.1f | RBE %d (fort %d) | imune %s | congela %d | moab %d | camo nativo %d | rank %d | filhos %zu\n", b.rotulo.c_str(), b.vida,b.vida_fortificado,b.velocidade,b.velocidade*VELOCIDADE_BASE,b.raio, rbe(b.id), rbe(b.id,true), dt(b.imune).c_str(), b.congela,b.moab,b.camo_nativo,b.rank,b.filhos.size()); }
 if(modo=="mapas"){ for(auto&d:mapas()){ const Mapa& m=mapa(d.chave); printf("%s (%s) dif %s | trilhas %zu |", d.nome.c_str(), d.chave.c_str(), d.dificuldade.c_str(), m.caminhos.size()); for(auto&c:m.caminhos) printf(" %.0fpx", c.comprimento); printf(" | agua circ %zu ret %zu | obst %zu\n", d.agua.size(), d.agua_ret.size(), d.obstaculos.size());
   double area_agua=0; for(auto&a:d.agua) area_agua+=M_PI*a[2]*a[2]; for(auto&a:d.agua_ret) area_agua+=a[2]*a[3]; double area_obst=0; for(auto&o:d.obstaculos) area_obst+=M_PI*o.r*o.r;
   // area construivel aproximada por amostragem
   int tot=0, livre=0, agua=0, cobertura=0; for(int y=10;y<720;y+=10) for(int x=10;x<1040;x+=10){ ++tot; bool tr=m.na_trilha(x,y,16); bool bl=m.bloqueado(x,y,20); bool ag=m.eh_agua(x,y); if(!tr&&!bl&&!ag) ++livre; if(ag&&!tr) ++agua; if(!tr&&!bl&&!ag){ double tot_d=0; for(auto&c:m.caminhos) for(double dd: c.distancias_no_raio(x,y,128)) (void)dd, tot_d+=1; if(tot_d>0) ++cobertura; } }
   printf("  area livre terra %.0f%% | agua %.0f%% | area de agua px2 %.0f | obst px2 %.0f\n", 100.0*livre/tot, 100.0*agua/tot, area_agua, area_obst);
   // melhor ponto: max comprimento de trilha dentro de r=128 e r=160
   for(double R: {92.0,128.0,160.0}){ double best=0; int bx=0,by=0; for(int y=10;y<720;y+=10) for(int x=10;x<1040;x+=10){ if(m.na_trilha(x,y,16)||m.bloqueado(x,y,20)||m.eh_agua(x,y)) continue; double L=0; for(auto&c:m.caminhos) L+=8.0*c.distancias_no_raio(x,y,R).size(); if(L>best){best=L;bx=x;by=y;} } printf("  melhor ponto r%.0f: %.0fpx de trilha em (%d,%d)\n",R,best,bx,by);} 
   for(const char* bn: {"vermelho","rosa","ceramica","moab","bfb","zomg","ddt","bad"}){ auto& b=tipo_bloon(bn); double L=0; for(auto&c:m.caminhos) L=std::max(L,c.comprimento); printf("  %s atravessa em %.1fs\n", bn, L/(b.velocidade*VELOCIDADE_BASE)); } } }
 if(modo=="rodadas"){ double acum=650; for(int r=1;r<=100;++r){ auto g=grupos_da_rodada(r); long rb=0,rbm=0; int nb=0; bool camo=false,regen=false,fort=false,moab=false; std::string desc; for(auto&x:g){ if(x.qtd<=0) continue; auto&t=tipo_bloon(x.tipo); rb+= (long)x.qtd*rbe(t.id,x.fort); nb+=x.qtd; camo|=x.camo||t.camo_nativo; regen|=x.regen; fort|=x.fort; moab|=t.moab; char b[80]; snprintf(b,80,"%s%d %s%s%s%s",desc.empty()?"":", ",x.qtd,t.rotulo.c_str(),x.camo?" camo":"",x.regen?" regen":"",x.fort?" fort":""); desc+=b;}
   double dur=duracao_rodada(r); double ganho=100+r; acum+=ganho+nb; printf("%d | %s | RBE %ld | bloons %d | dur %.1fs | RBE/s %.0f | %s%s%s%s| $fim %d\n", r, desc.c_str(), rb, nb, dur, dur>0?rb/dur:rb, camo?"C":"", regen?"R":"", fort?"F":"", moab?"M":"", (int)ganho); } }
}
