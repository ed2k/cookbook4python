data='''RPFDR         9000/CXC 146 05        R1A03         10  31
RPIFDR        9000/CXC 146 03        R1A08          8  30
INETR         9000/CXC 146 125       R2A02        369  29
GPEX2R        9000/CXC 146 179       P1C02        372  16
PEDRO7R       90BB/CXC 146 180       R1A02        368   4   4
PEDRO8R       9000/CXC 146 134       R1B01        367   3   3
PEDRO3R       9000/CXC 146 130       R1B01        366   2   2
PEDRO2R       900F/CXC 146 129       R1B01        365   1   1
PEDRO1R       9000/CXC 146 128       R1B01        364   0   0 '''   

def exrui(rp,suname=None,suid=None):
  if suname: return 'exrui:rp='+rp+',suname='+suname+';'
  return 'exrui:rp='+rp+',suid="'+suid+'";'

def parse_data(s):
  m = {}
  for line in s.splitlines():
    f = line.split()
    suname = f[0][:-1].upper()
    suid=' '.join(f[1:5])
    m[suname] = suid        
  return m
def exemi(sumaps, emlist, startidx,rp, eqmidx):
  idx = startidx
  r = []
  for em in emlist.split():
    emname = em.upper()
    line = 'exemi:rp=%s,em=%d,eqm=%s-%s,suid="%s";!%s!' % \
      (rp,idx,emname,eqmidx,sumaps[emname],emname)
    idx += 1
    r.append(line)
  return '\n'.join(r)
  
d = parse_data(data)
print exemi(d, 'pedro1 pedro2 pedro3 pedro7 pedro8',1,'6','8')

