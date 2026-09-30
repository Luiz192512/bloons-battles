import sys,re
s=open("torres.txt").read()
for n in sys.argv[2:]:
    i=s.find("##### "+n+" |"); j=s.find("\n##### ",i+5); b=s[i:j]
    for l in b.split("\n")[2:]:
        l=re.sub(r"It has the following interactions with crosspaths:.*","",l)
        print(l[:int(sys.argv[1])])
