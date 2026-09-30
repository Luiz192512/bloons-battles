#include "jogo/defs.hpp"
#include "jogo/rodadas.hpp"
#include <cstdio>
using namespace bl;
int pops(const TipoBloon& t){int n=1; for(int f:t.filhos_id) n+=pops(bloons()[f]); return n;}
int main(){ for(auto&b:bloons()) printf("%s pops=%d\n",b.nome.c_str(),pops(b));
 double c=650, rb=0; for(int r=1;r<=100;++r){ double p=0; for(auto&g:grupos_da_rodada(r)) if(g.qtd>0){ auto&t=tipo_bloon(g.tipo); p+=g.qtd*pops(t); rb+=g.qtd*rbe(t.id,g.fort);} c+=p+100+r; if(r%10==0||r==1||r==5||r==15||r==25) printf("r%d $acum %.0f RBEacum %.0f\n",r,c,rb);} }
