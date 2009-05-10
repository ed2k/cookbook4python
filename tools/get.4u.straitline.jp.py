import urllib
import os,time,random
url_base = "4u.straightline.jp"
start_id = 3000

# get files with a certain pattern from a serials of webpages


# download one file like wget
# url without http://, so that it matches file system format
def wget(url, opts=None):
    f = url.split('/')
    #fname = urllib.url2pathname(url)
    fname = os.path.join(*f)
    fname = os.path.abspath(fname.replace('?','_q_'))
    #print fname
    if os.path.exists(fname):
        return 'exist'
    file(fname,'w').write(urllib.urlopen('http://'+url).read())
    return 'saved'

def get_all_pages():
  for id in xrange(3000):
    url = url_base+'/?page='+str(id)
    if wget(url) == 'saved':
       print 'saved',url
       time.sleep(random.random()*10)
    
get_all_pages()