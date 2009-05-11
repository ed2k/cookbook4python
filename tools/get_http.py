#grep viewthread tmp | grep span |grep extra  | cut -f7 -d'>' | cut -f1 -d'<' 
import urllib

for pagenum in xrange(1,90):
    url = "http://www.topk.cn/bbs/forumdisplay.php?fid=44&page="+ str(pagenum)
    page = urllib.urlopen(url)
    savefile = 'latest_english_'+ str(pagenum)
    print url,savefile
    file(savefile,'w').write(page.read())

