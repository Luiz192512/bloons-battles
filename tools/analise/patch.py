# uso: python3 patch.py arquivo_de_patches
# formato:  @torre p-t | descricao opcional
#           {{"pierce", 1}}        (ef em C++, uma linha)
#           @torre base
#           A("projetil", {...})   (novo ataque base)
import re,sys
import os
F=os.path.join(os.path.dirname(__file__), "..", "..", "src", "jogo", "dados.cpp")
src=open(F).read()
def bloco(k):
    i=src.find(f'DefTorre t("{k}", '); j=src.find("\nstatic DefTorre",i+10); return i,j
lines=[l.rstrip("\n") for l in open(sys.argv[1]) if l.strip() and not l.startswith("#")]
i=0; n=0
while i<len(lines):
    h=lines[i]; body=lines[i+1]; i+=2
    m=re.match(r"@(\w+) (base|(\d)-(\d))(?: \| (.*))?$",h)
    k=m.group(1); a,b=bloco(k); blk=src[a:b]
    if m.group(2)=="base":
        blk=re.sub(r"t\.ataques = \{A\(.*\)\};", lambda _: "t.ataques = {"+body+"};", blk,count=1)
    else:
        p,t=int(m.group(3)),int(m.group(4))
        ci=blk.find("t.caminhos = {")
        ul=[mm for mm in re.finditer(r'^( +U\("[^"]*", \d+, )"((?:[^"\\]|\\.)*)", (.*)\),$',blk[ci:],re.M)]
        mm=ul[(p-1)*5+(t-1)]
        desc=m.group(5) if m.group(5) is not None else mm.group(2)
        new=f'{mm.group(1)}"{desc}", {body}),'
        s0=ci+mm.start(); s1=ci+mm.end(); blk=blk[:s0]+new+blk[s1:]
    src=src[:a]+blk+src[b:]; n+=1
open(F,"w").write(src); print("patches",n)
