from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import SocketServer
import SimpleHTTPServer
import urllib

import datetime
import hashlib
import logging
import pickle
import re
import time
import urllib
import wsgiref.handlers

import transform_content


################################################################################
logging.basicConfig(level=logging.DEBUG)
DEBUG = False
EXPIRATION_DELTA_SECONDS = 3600
EXPIRATION_RECENT_URLS_SECONDS = 90

## DEBUG = True
## EXPIRATION_DELTA_SECONDS = 10
## EXPIRATION_RECENT_URLS_SECONDS = 1

HTTP_PREFIX = "http://"
HTTPS_PREFIX = "http://"

IGNORE_HEADERS = frozenset([
  'set-cookie',
  'expires',
  'cache-control',

  # Ignore hop-by-hop headers
  'connection',
  'keep-alive',
  'proxy-authenticate',
  'proxy-authorization',
  'te',
  'trailers',
  'transfer-encoding',
  'upgrade',
])

TRANSFORMED_CONTENT_TYPES = frozenset([
  "text/html",
  "text/css",
])

MIRROR_HOSTS = frozenset([
  'mirrorr.com',
  'mirrorrr.com',
  'www.mirrorr.com',
  'www.mirrorrr.com',
  'www1.mirrorrr.com',
  'www2.mirrorrr.com',
  'www3.mirrorrr.com',
])

MAX_CONTENT_SIZE = 10 ** 6

MAX_URL_DISPLAY_LENGTH = 50

################################################################################

def get_url_key_name(url):
  url_hash = hashlib.sha256()
  url_hash.update(url)
  return "hash_" + url_hash.hexdigest()

################################################################################

class MirroredContent(object):
  def __init__(self, original_address, translated_address,
               status, headers, data, base_url):
    self.original_address = original_address
    self.translated_address = translated_address
    self.status = status
    self.headers = headers
    self.data = data
    self.base_url = base_url

  @staticmethod
  def get_by_key_name(key_name):
    return memcache.get(key_name)

  @staticmethod
  def fetch_and_store(key_name, base_url, translated_address, mirrored_url):
    """Fetch and cache a page.
    
    Args:
      key_name: Hash to use to store the cached page.
      base_url: The hostname of the page that's being mirrored.
      translated_address: The URL of the mirrored page on this site.
      mirrored_url: The URL of the original page. Hostname should match
        the base_url.
    
    Returns:
      A new MirroredContent object, if the page was successfully retrieved.
      None if any errors occurred or the content could not be retrieved.
    """
    # Check for the X-Mirrorrr header to ignore potential loops.
    if base_url in MIRROR_HOSTS:
      logging.warning('Encountered recursive request for "%s"; ignoring',
                      mirrored_url)
      return None

    logging.debug("Fetching '%s'", mirrored_url)
    try:
      response = urllib.urlopen(mirrored_url).read()
    except :
      logging.exception("Could not fetch URL")
      return None

    adjusted_headers = {}
    #for key, value in response.headers.iteritems():
    #  adjusted_key = key.lower()
    #  if adjusted_key not in IGNORE_HEADERS:
    #    adjusted_headers[adjusted_key] = value

    #content = response.content
    #page_content_type = adjusted_headers.get("content-type", "")
    #for content_type in TRANSFORMED_CONTENT_TYPES:
      # Startswith() because there could be a 'charset=UTF-8' in the header.
    #  if page_content_type.startswith(content_type):
    #    content = transform_content.TransformContent(base_url, mirrored_url,
    #                                                 content)
    #    break

    print(base_url, mirrored_url, response[:200])
    content = transform_content.TransformContent(base_url, mirrored_url,
                                                     (response))
     
    # If the transformed content is over 1MB, truncate it (yikes!)
    if len(content) > MAX_CONTENT_SIZE:
      logging.warning('Content is over 1MB; truncating')
      content = content[:MAX_CONTENT_SIZE]

    new_content = MirroredContent(
      base_url=base_url,
      original_address=mirrored_url,
      translated_address=translated_address,
      status = '',#status=response.status_code,
      headers=adjusted_headers,
      data=content)
      
    return new_content

################################################################################

class Request(object):
  def __init__(self, obj):  
    h,p = obj.client_address
    if p == 80: url = h + '/' + obj.path
    else: url = h + ':'+str(p)+'/' + obj.path   
    print ('url',url)
    self.url = url
    idx = h.find(':')
    self.schema = h[:idx]
  
class BaseHandler(object):
  def __init__(self, obj):
     if (obj.path.startswith("/?url=")):
        obj.path = "/"+obj.path[6:]
     self.obj = obj
     self.request = Request(obj)
  def get_relative_url(self):
    return self.obj.path


class HomeHandler(BaseHandler):
  def get(self, url):
    secure_url = "https://test8898.appspot.com"
    context = {
      "latest_urls": None,
      "secure_url": secure_url,
    }
    return str(file("main.html").read())

ad_strings = ['content.yieldmanager.edgesuite.net','ad.doubleclick.net', 
  'adserver.wenxuecity.com']
def ad_filter(address):
  for s in ad_strings:
    if address[:len(s)] == s: return True
  if address.find('.swf') > 0: return True
  return False 
def no_cache_url(address):
    if address.find('flickr.com/photos') >= 0: return True
    return False

class MirrorHandler(BaseHandler):
  def get(self, base_url):
    assert base_url
    
    # Log the user-agent and referrer, to see who is linking to us.
    #logging.debug('User-Agent = "%s", Referrer = "%s"',
    #              self.request.user_agent,
    #              self.request.referer)
    logging.debug('Base_url = "%s", url = "%s"', base_url, self.request.url)

    translated_address = self.get_relative_url()[1:]  # remove leading /
    #logging.info('addr: %s',translated_address)
    if ad_filter(translated_address): return self.error(404)
    mirrored_url = HTTP_PREFIX + translated_address

    # Use sha256 hash instead of mirrored url for the key name, since key
    # names can only be 500 bytes in length; URLs may be up to 2KB.
    key_name = get_url_key_name(mirrored_url)
    logging.info("Handling request for '%s' = '%s'", mirrored_url, key_name)

    content = MirroredContent.fetch_and_store(key_name, base_url,
                                                translated_address,
                                                mirrored_url)
    if content is None:
      return self.error(404)
    return content.data
    # not working in pure python mode

    # Store the entry point down here, once we know the request is good and
    # there has been a cache miss (i.e., the page expired). If the referrer
    # wasn't local, or it was '/', then this is an entry point.
    if (cache_miss and
        'Googlebot' not in self.request.user_agent and
        'Slurp' not in self.request.user_agent and
        (not self.request.referer.startswith(self.request.host_url) or
         self.request.referer == self.request.host_url + "/")):
      # Ignore favicons as entry points; they're a common browser fetch on
      # every request for a new site that we need to special case them here.
      if not self.request.url.endswith("favicon.ico"):
        logging.info("Inserting new entry point")
        entry_point = EntryPoint(
          key_name=key_name,
          translated_address=translated_address)
        try:
          entry_point.put()
        except (db.Error, apiproxy_errors.Error):
          logging.exception("Could not insert EntryPoint")
 
    for key, value in content.headers.iteritems():
      self.response.headers[key] = value
    if not DEBUG:
      self.response.headers['cache-control'] = \
        'max-age=%d' % EXPIRATION_DELTA_SECONDS

    self.response.out.write(content.data)

class webapp_RequestHandler(BaseHTTPRequestHandler): pass

class AdminHandler(webapp_RequestHandler):
  def get(self):
    self.response.headers['content-type'] = 'text/plain'
    self.response.out.write(str(memcache.get_stats()))


class KaboomHandler(webapp_RequestHandler):
  def get(self):
    self.response.headers['content-type'] = 'text/plain'
    self.response.out.write('Flush successful: %s' % memcache.flush_all())


class CleanupHandler(webapp_RequestHandler):
  """Cleans up EntryPoint records."""

  def get(self):
    keep_cleaning = True
    try:
      content_list = EntryPoint.gql('ORDER BY last_updated').fetch(25)
      keep_cleaning = (len(content_list) > 0)
      db.delete(content_list)
      
      if content_list:
        message = "Deleted %d entities" % len(content_list)
      else:
        keep_cleaning = False
        message = "Done"
    except (db.Error, apiproxy_errors.Error), e:
      keep_cleaning = True
      message = "%s: %s" % (e.__class__, e)

    context = {  
      'keep_cleaning': keep_cleaning,
      'message': message,
    }
    self.response.out.write(template.render('cleanup.html', context))

class DBHandler(webapp_RequestHandler):
  """Class."""
  def get(self):
    # default is 0
    offset = int(self.request.get('offset'))
    d = DB1.gql('order by last_updated DESC').fetch(1,offset)[0]     

    # how get all entity attributes?
    a = getattr(d, 'content-type', None)
    if a is not None:
        a = str(a)        
        logging.info(a+'>'+d.translated_address)
        self.response.headers['content-type'] = a
        if a.find('text') > -1:
           self.response.headers['content-type'] = 'text/html'
           logging.info('create text')
           return self.response.out.write('<textarea rows=20 cols=80> %s </textarea>' % d.bin)
    a = getattr(d, 'content-length', None)
    if a is not None:
        self.response.headers['content-length'] = str(a)
    
    self.response.out.write(d.bin)


PORT = 80

class Proxy(SimpleHTTPServer.SimpleHTTPRequestHandler):
  def do_GET(self):
    print('self.path', self.path)
    pa = self.path
    obj = None
    if pa in ["/main", "/"] : obj = HomeHandler(self)
    elif pa == '/kaboom': obj = KaboomHandler(self)
    elif pa == '/admin': obj = AdminHandler(self)
    elif pa == '/db': obj = DBHandler(self)
    elif pa == '/cleanup': obj = CleanupHandler(self)
    else: obj = MirrorHandler(self)

    idx = pa[1:].find('/')
    if idx <= 0: base_url = pa[1:]
    else: base_url = pa[1:idx+1]
    print ('base url', base_url)
    self.wfile.write(obj.get(base_url)) 
    #self.copyfile(urllib.urlopen(self.path[1:]), self.wfile)

httpd = SocketServer.ForkingTCPServer(('', PORT), Proxy)
print ("serving at port", PORT)
httpd.serve_forever()

