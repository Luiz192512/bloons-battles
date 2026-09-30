#include "jogo/sim.hpp"
#include "jogo/rodadas.hpp"
#include <cstdio>
#include <chrono>
using namespace bl;
// Solo automatico: coloca torres variadas em pontos livres e upa sempre que tem dinheiro.
int main(int argc,char**argv){
  std::string mapa=argc>1?argv[1]:"prado", dif=argc>2?argv[2]:"medio", modo=argc>3?argv[3]:"solo";
  Partida p(modo,mapa,7,dif,{{1,"quincy"},{2,"churchill"}});
  p.automatico=true; bool rico=argc>4; if(rico) p.ultima_rodada=170;
  const char* ks[]={"dardo","bumerangue","bomba","tachinha","gelo","cola","sniper","submarino","bucaneiro","as","heli","morteiro","dartling","mago","super","ninja","alquimista","druida","fazenda","espinhos","vila","engenheiro"};
  auto t0=std::chrono::steady_clock::now(); int passos=0, colocadas=0, ups=0;
  if(modo=="solo") p.aplicar(1,"N");
  int ki=0;
  while(!p.fim && passos<30*60*400){
    for(int j: {1,2}){ if(!p.pistas.count(j)) continue; Pista& pi=*p.pistas[j];
      if(rico) { pi.dinheiro=1e8; pi.vidas=1000000; }
      if(passos%15==0){
        // tenta colocar a proxima torre numa grade
        for(int tent=0;tent<400;++tent){ int x=40+ (int)(pi.rng.random()*960), y=40+(int)(pi.rng.random()*640);
          char e=p.aplicar(j,std::string("T")+ks[ki%22]+"@"+std::to_string(x)+","+std::to_string(y)); if(!e){colocadas++; ki++; break;} if(e==ERRO_DINHEIRO) break; }
        if(passos%30==0) { std::string h=j==1?"quincy":"churchill"; p.aplicar(j,"T"+h+"@"+std::to_string(100+j*50)+",60"); }
        for(auto&[id,t]:pi.torres){ for(int c: {0,2,1}) if(!p.aplicar(j,"U"+std::to_string(id)+":"+std::to_string(c))) {ups++; break;}
          for(int b=0;b<3;++b) p.aplicar(j,"B"+std::to_string(id)+":"+std::to_string(b)); }
        if(modo=="batalha" && passos%60==0) p.aplicar(j,"Sr8");
      }}
    p.passo(); passos++;
  }
  double s=std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count();
  Pista& a=*p.pistas[1];
  printf("%s %s %s: rodada %d fim %d venc %d vidas %d $%.0f torres %zu colocadas %d ups %d pops %d | %.1fs sim em %.1fs real\n",modo.c_str(),mapa.c_str(),dif.c_str(),p.rodada,p.fim,p.vencedor,a.vidas,a.dinheiro,a.torres.size(),colocadas,ups,a.pops_total,passos/30.0,s);
}
