import json,urllib.request,urllib.parse,sys,time
A="https://www.bloonswiki.com/api.php"
def get(p):
    p["format"]="json"
    u=A+"?"+urllib.parse.urlencode(p)
    for i in range(4):
        try: return json.load(urllib.request.urlopen(urllib.request.Request(u,headers={"User-Agent":"analise-btdb2/1.0"}),timeout=60))
        except Exception as e: time.sleep(2**i); err=e
    raise err
def embedded(tpl):
    out=[];c={}
    while True:
        r=get({"action":"query","list":"embeddedin","eititle":tpl,"eilimit":"500","einamespace":"0",**c})
        out+= [x["title"] for x in r["query"]["embeddedin"]]
        if "continue" in r: c={"eicontinue":r["continue"]["eicontinue"]}
        else: return out
def contents(titles):
    res={}
    for i in range(0,len(titles),50):
        r=get({"action":"query","prop":"revisions","rvprop":"content","rvslots":"main","titles":"|".join(titles[i:i+50])})
        for pg in r["query"]["pages"].values():
            if "revisions" in pg: res[pg["title"]]=pg["revisions"][0]["slots"]["main"]["*"]
    return res
if __name__=="__main__":
    allp={}
    for tpl in sys.argv[1:]:
        t=embedded("Template:"+tpl); print(tpl,len(t))
        allp[tpl]=contents(t)
    json.dump(allp,open("pages.json","w"),ensure_ascii=False)
