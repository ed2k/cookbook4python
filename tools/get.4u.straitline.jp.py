import urllib
import os
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
    print fname
    if os.path.exists(fname):
        return
    file(fname,'w').write(urllib.urlopen('http://'+url).read())

url = url_base+'/?page='+str(start_id)
print 'open',url

wget(url)