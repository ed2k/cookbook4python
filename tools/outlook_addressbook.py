# the problem is the global address book has 189K entrie and it is changing all 
# the time, so the local copy is always not complete synced.
# problem 1. how to detect its change
# problem 2. how to iterate all the info
# possible solution use id -> the place in addresssEntries.Item(n)
import win32com.client
import pickle, sys, zipfile

def appendfile(obj):
  xls = []
  for n in obj.keys(): 
    a= obj[n]
    manager = ''
    address = ''
    try: manager=a['manager']
    except: pass
    try: address = a['address']
    except: pass
    t = [str(a['id']), n, manager,address]
    xls.append('<>'.join(t))

  file("outlook.gal",'a').write('\n'.join(xls))

def savefile():
  xls = []
  for n in addobj.keys(): 
    a= addobj[n]
    manager = ''
    address = ''
    try: manager=a['manager']
    except: pass
    try: address = a['address']
    except: pass
    t = [str(a['id']), n, manager,address]
    xls.append('<>'.join(t))

  file("outlook.gal",'wb').write('\n'.join(xls))

def dumpobj():
  cache = open('outlook_gal.pkl','wb')
  pickle.dump(addobj, cache)
  cache.close()

def loadobj():
  cache = open('outlook_gal.pkl','r')
  addobj = pickle.load(cache)
  cache.close()

#an employee, boss map
addobj = {}

def readfile():
 for line in open("outlook.gal",'r'):
  line = line.strip()
  if line == '': continue
  if line[0] == '#': continue
  f = line.split('<>')
  if len(f) == 1: continue
  id = int(f[0])
  name = f[1]
  # don't check duplication to improve performance for 190K lines
  #if name not in keys:
  addobj[name] = {'id':id}
  #else: print 'found duplicate',len(addobj), name 
  if len(f)> 2 and (f[2].strip() !=''): 
    addobj[name]['manager'] = f[2]
  if len(f) > 3 and (f[3].strip() != ''):
    addobj[name]['address'] = f[3]
 return addobj

def get_from_outlook():
    addobj = readfile()
    print 'read',len(addobj)
    
    obj = win32com.client.Dispatch("Outlook.Application")
    
    ns = obj.GetNamespace("MAPI")
    ga = ns.AddressLists("Global Address List")
    aes = ga.AddressEntries
    print 'total address', len(aes)
    ids = [addobj[x]['id']  for x in addobj.keys()]
    ids.sort()
    # ae = aes(lastone+" ")
    # Name Address Manager
    # or use can use aes.Item(n) to access each entry
    
    # if ae.Members not None it is a distribution list
    # len(.Members) -> 1..len .Members.Item(i)
    print 'lastone', ids[-1]
    if ids[-1] == -1 : sys.exit()
    cnt = 0
    newobj = {}
    for idx in xrange(ids[-1]+1,len(aes)+1):
      ae = aes.Item(idx)
      name = ae.Name
      name = name.encode('ascii', 'ignore')
      if name not in addobj.keys():
        #print name
        addobj[name] = {}
        cnt += 1
      else: 
        if addobj[name]['id'] != idx:
          print 'found one mismatch deal with it later', name
          addobj[name]['id'] = idx
          newobj[name] = addobj[name]
        continue; #--------------------------------->
      addobj[name]['id'] = idx
    
      addobj[name]['address'] = ae.Address.encode('ascii', 'ignore')
      manager = 'None'
      try: 
        manager = ae.Manager.Name
      except :
        manager = 'Exception'
      manager = manager.encode('ascii', 'ignore')
      addobj[name]['manager'] = manager
      newobj[name] = addobj[name]
    
      if cnt> 1000: 
        cnt = 0
        print 'appendaddobj', len(addobj)
        appendfile(newobj)
        newobj = {}
    
    
    appendfile(newobj)

def get_manager(db, user):
    ''' get a user's manger, if none return null'''
    if user not in db: return None
    if 'manager' not in db[user]: return None
        
    m = db[user]['manager']
    if m == 'Exception': return None
    return m

        
def get_managers(addobj, user_list):
    ''' from a list of people, get their managers '''
    managers = {}
    cnt = 0
    for n in user_list:
        try: m = addobj[n]['manager']
        except: 
            #print n,'not found'
            continue
        if m == 'Exception': continue
        if m not in managers :
            managers[m] = []
        managers[m].append(n) 
        cnt += 1        
    print 'total with managers', cnt
    print 'managers', len(managers)
    return managers


addobj = readfile()
# a boss -> employee map
boss_empl = get_managers(addobj,addobj)

#find levels ,0 no report, level m must have some level m-1 report to
def test1():
    for empl in addobj:
        if 'level' not in addobj[empl]:
            addobj[empl]['level'] = 0
        e = empl
        m = get_manager(addobj, e)
        mchain = [e]; # to avoid circular managers
        while (m is not None) and (m in addobj) and (m not in mchain):
            if 'level' not in addobj[m]:
                addobj[m]['level'] = 0
            if addobj[m]['level'] > addobj[e]['level']:
                break
            addobj[m]['level'] = addobj[e]['level'] + 1
            if addobj[m]['level'] > 4:
                print m, addobj[m]['level']

            e = m
            m = get_manager(addobj, e)
            mchain.append(m)

def search_empl(db, keyword, lev=-1):
    if keyword.strip() == '': return []
        
    r = []
    for empl in db:
        e = empl.lower()
        if e.find(keyword.lower()) >= 0:
            if lev == -1: 
                r.append(empl)
                continue
            if level not in db[empl]: continue
            if db[empl]['level'] > lev :        
                r.append(empl)
    return r
    
def get_cheifs(db, boss_empl):    
    "find those cheifs who is manager but doesn't have boss"
    r = []
    for empl in db:
        if empl in boss_empl:
            if get_manager(db, empl) is None:
                #print empl,db[empl]['level']
                r.append(empl)
    return r

roots = get_cheifs(addobj, boss_empl)

def tempIdValue(idList, id):
    return '     { @id='+str(id)+'; @value="'+idList[id]+'"; },'
def generate_graph(m2e, roots):
    for root in roots:
        src = []
        checks = [root]
        todo = [(0,root)]
        while len(todo) > 0:
            mid,m = todo.pop()
            for u in m2e[m]:
                if u in checks:
                    continue
                uid = len(checks)
                checks.append(u)
                if u in m2e:
                    todo.append((uid,u))
                src.append('{ @source='+str(mid) +'; @destination='+ str(uid)+'; },')

        print len(checks), len(src)
        f = file(root+'.graph','w')
        f.write('''Graph
{
   @name="Org Chart";
   @description="A graph generated from a org tree.";
   @numNodes='''+str(len(checks))+''';
   @numLinks='''+str(len(src))+''';
   @numPaths=0;
   @numPathLinks=0;
   @links=[     
   '''+'\n'.join(src)[:-1] + '''
        ];
        @paths=;
        @enumerations=;
        @attributeDefinitions=[
           {
              @name=$root;
              @type=bool;
              @default=|| false ||;
              @nodeValues=[ { @id=0; @value=T; } ];
              @linkValues=;
              @pathValues=;
           },
           {
              @name=$tree_link;
              @type=bool;
              @default=|| false ||;
              @nodeValues=;
              @linkValues=[
        '''
        )
        src = ['{ @id='+str(i)+'; @value=T; },' for i in range(len(src))]
        f.write('\n'.join(src)[:-1]+'''
           ];
           @pathValues=;
        },
        {
           @name=$base_name;
           @type=string;
           @default=;
           @nodeValues=[
              '''+'\n'.join([tempIdValue(checks,i) for i in range(len(checks))])[:-1]+'''
           ];
           @linkValues=;
           @pathValues=;
        }        
   ];
   @qualifiers=[
      {
         @type=$spanning_tree;
         @name=$default_spanning_tree;
         @description=;
         @attributes=[
            { @attribute=0; @alias=$root; },
            { @attribute=1; @alias=$tree_link; }
         ];
      }
   ];
   @filters=;
   @selectors=;
   @displays=;
   @presentations=;
   @presentationMenus=;
   @displayMenus=;
   @selectorMenus=;
   @filterMenus=;
   @attributeMenus=;
}''')
#generate_graph(boss_empl, roots)                    

from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import urllib

def box(u):
    return '<a href="?o='+u+'">'+u+'</a>'
def br(): return '<br/>'

def get_search(user):  
    results = search_empl(addobj, user)
    r = []
    for u in results:
        r.append(box(u)+br())
    return get_default()+'\n'.join(r)
def get_default():  
    return '''<form action="/" method="get">
  <input type='text' name="s" size="20"/>
  <input type="submit" value="search"/>
</form>
'''

def get_org(user):  
    m = get_manager(addobj,user)
    src = []
    if m is not None:
        src.append('Report to '+box(m)+br())
    src.append('-> '+box(user)+br())    
    if user in boss_empl:
        src.append('direct report '+'\n'.join([box(u) for u in boss_empl[user]]))
    return get_default()+'\n'.join(src)



class MyHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type","text/html")
        self.end_headers()
        
        data = urllib.unquote(self.path[1:]).split('=')        
        print 'r',[data]
        if data[0] == '?s':
            r = get_search(data[1])
        elif data[0] == '?o':
            r = get_org(data[1])
        else:
            r = get_default()
        self.wfile.write(r)
try:
     server = HTTPServer(('', 80), MyHandler)
     print 'started httpserver...'
     server.serve_forever()
except:
     print '^C received, shutting down server'
     server.socket.close()
    