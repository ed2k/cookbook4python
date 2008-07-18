#!/usr/bin/env python

# pdfparser.py - 0.1
# Yusuke Shinyama, Dec 24 2004


import sys, os, re, StringIO
stderr = sys.stderr

# TODO:
#   LZW decompression
#   CCITT decompression
#   DCT decompression
#   Decryption with RC4
#   %PDF-1.5 support


BUFLEN = 4096


def unpack_stream(dic, data):
  print dic
  if '/Filter' not in dic:
    return data
  filters = dic['/Filter']
  if not isinstance(filters, list):
    filters = [ filters ]
  for f in filters:
    # take out / and Decode, format /xxxDecode
    f = f[1:-6]
    if f == 'ASCIIHex':
      # TODO
      return '(Unsupported Stream: ASCIIHexDecode)'
    elif f == 'ASCII85':
      # TODO
      return '(Unsupported Stream: ASCII85Decode)'
    elif f == 'CCITTFax':
      # TODO
     return '(Unsupported Stream: CCITTFaxDecode)'
    elif f == 'DCT':
      # TODO
      return '(Unsupported Stream: DCTDecode)'
    elif f == 'Flate':
      import zlib
      # get errors if the document is encrypted.
      
      try:
         data = zlib.decompress(data)
      except:
         data = 'zlib decompress failed'
    else:
      print >>stderr, 'Unknown filter: %s' % f
  return data


# resolve
def resolve(x):
  if isinstance(x, PDFObjRef):
    return x.resolve()
  else:
    return x


##  PDFObjRef
##
class PDFObjRef:
  
  def __init__(self, objid, genno, parser, pos):
    self.objid = objid
    self.genno = genno
    self.parser = parser
    self.pos = pos
    self.value = None
    return

  def __repr__(self):
    return '<PDFObjRef:%d (pos=%s)>' % (self.objid, self.pos)

  def resolve(self):
    if not self.value:
      assert self.pos != None
      pos0 = self.parser.curpos
      self.parser.seek(self.pos)
      self.parser.parse1(True)
      assert self.value != None, self
      self.parser.seek(pos0)
      #print >>stderr, 'resolve:', self, '->', self.value
    return self.value
  

##  PDFStream
##
class PDFStream:
  
  def __init__(self, dic, data):
    self.dic = dic
    self.data = data
    self.unpacked = None
    self.parsed = None
    return
  
  def __repr__(self):
    return '<PDFStream: %r>' % (self.dic)

  def get_data(self):
    if self.unpacked == None:
      self.unpacked = unpack_stream(self.dic, self.data)
    return self.unpacked

  def parse(self, xref):
    if self.parsed == None:
      parser = PDFParser(StringIO.StringIO(self.get_data()))
      self.parsed = parser.parse1()
    return self.parsed
  

##  PDFParser
##
class PDFParser:

  def __init__(self, fp, debug=0):
    self.fp = fp
    self.objdic = {}
    self.debug = debug
    self.seek(0)
    return

  def __repr__(self):
    return '<PDFParser: %r (%d)>' % (self.fp, self.curpos)

  def get_object(self, objid, genno):
    if objid not in self.objdic:
      obj = PDFObjRef(objid, genno, self, None)
      self.objdic[objid] = obj
    else:
      obj = self.objdic[objid]
    return obj

  def seek(self, pos):
    self.fp.seek(pos)
    self.curpos = pos
    (self.linebuf, self.linepos) = ('', 0)
    return
  
  EOLCHAR = re.compile(r'[\012\015]')
  def nextline(self):
    while 1:
      m = self.EOLCHAR.search(self.linebuf, self.linepos)
      if not m:
        s = self.fp.read(BUFLEN)
        if not s:
          line = self.linebuf
          (self.linebuf, self.linepos) = ('', 0)
          break
        (self.linebuf,self.linepos) = (self.linebuf[self.linepos:]+s, 0)
        continue
      i = m.end(0)
      if m.group(0) == '\r':
        if i == len(self.linebuf):
          s = self.fp.read(BUFLEN)
          if not s:
            line = self.linebuf
            (self.linebuf, self.linepos) = ('', 0)
            break
          self.linebuf += s
        if self.linebuf[i] == '\n':
          i += 1
      line = self.linebuf[self.linepos:i]
      self.linepos = i
      break
    self.curpos += len(line)
    return line

  def revreadlines(self):
    self.fp.seek(0, 2)
    pos = self.fp.tell()
    buf = ''
    while 1:
      pos -= BUFLEN
      self.fp.seek(pos)
      s = self.fp.read(BUFLEN)
      if not s: break
      while 1:
        n = max(s.rfind('\r'), s.rfind('\n'))
        if n == -1:
          buf = s + buf
          break
        yield buf+s[n:]
        s = s[:n]
        buf = ''
    return

  SPECIAL = r'%\[\]()<>{}/\000\011\012\014\015\040'
  TOKEN = re.compile(r'<<|>>|[\[\]()<>{}/]|[^'+SPECIAL+r']+')
  NAME = re.compile(r'([^#'+SPECIAL+r']|#[0-9abcdefABCDEF]{2})+')
  NUMBER = re.compile(r'[+-]?[0-9][.0-9]*$')
  STRING_NORM = re.compile(r'(\\[0-9]{1,3}|\\.|[^\)])+')
  STRING_NORM_SUB = re.compile(r'\\[0-7]{1,3}|\\.')
  STRING_HEX = re.compile(r'[\s0-9a-fA-F]+')
  STRING_HEX_SUB = re.compile(r'[0-9a-fA-F]{1,2}')

  def parse1(self, obj1=False):
    basepos = self.curpos
    stack = []
    partobj = []
    partobjtype = None
    while 1:
      # do not strip line! we need to distinguish last '\n' or '\r'
      line = self.nextline()
      if not line: break
      if 2 <= self.debug:
        print >>stderr, 'line:', repr(line), self.curpos
      # do this before removing comment
      if line.strip() in ('xref', 'trailer', 'startxref', '%%EOF'): break
      linepos = 0
      # tokenize
      while 1:
        m = self.TOKEN.search(line, linepos)
        if not m: break
        t = m.group(0)
        linepos = m.end(0)
        if t == '%':
          # skip comment
          break
        elif t == '/':
          # name object
          mn = self.NAME.match(line, m.start(0)+1)
          s = '/'+mn.group(0)
          partobj.append(s)
          linepos = mn.end(0)
          if 1 <= self.debug:
            print ' name: %r' % s
        elif t == '(':
          # normal string object
          s = ''
          while 1:
            ms = self.STRING_NORM.match(line, linepos)
            if not ms: break
            s1 = ms.group(0)
            linepos = ms.end(0)
            if s1[-1] == '\\':
              s += s1[-1:]
              line = self.nextline()
              if not line: assert 0, 'end inside string'
              linepos = 0
            elif linepos == len(line):
              s += s1
              line = self.nextline()
              if not line: assert 0, 'end inside string'
              linepos = 0
            else:
              s += s1
              break
          assert line[linepos] == ')'
          linepos += 1
          def convesc(m):
            x = m.group(0)
            if x[1:].isdigit():
              return chr(int(x[1:], 8))
            else:
              return x[1]
          s = self.STRING_NORM_SUB.sub(convesc, s)
          partobj.append('$'+s)
          if 1 <= self.debug:
            print 'str: %r' % s
        elif t == '<':
          # hex string object
          ms = self.STRING_HEX.match(line, linepos)
          linepos = ms.end(0)
          assert line[linepos] == '>'
          linepos += 1
          def convhex(m1):
            return chr(int(m1.group(0), 16))
          s = self.STRING_HEX_SUB.sub(convhex, ms.group(0))
          partobj.append('$'+s)
          if 1 <= self.debug:
            print 'str: %r' % s
        elif t == '{' or t == '}':
          partobj.append(t)
        elif t == '[':
          # begin array
          if 1 <= self.debug:
            print 'start array'
          stack.append((partobj, partobjtype))
          partobj = []
          partobjtype = 'a'
        elif t == ']':
          # end array
          assert partobjtype == 'a'
          a = partobj
          (partobj, partobjtype) = stack.pop()
          partobj.append(a)
          if 1 <= self.debug:
            print 'end array: %r' % a
        elif t == '<<':
          # begin dictionary
          if 1 <= self.debug:
            print 'start dict'
          stack.append((partobj, partobjtype))
          partobj = []
          partobjtype = 'd'
        elif t == '>>':
          # end dictionary
          assert partobjtype == 'd'
          assert len(partobj) % 2 == 0
          d = dict([ (partobj[i], partobj[i+1]) for i in range(0,len(partobj),2) ])
          (partobj, partobjtype) = stack.pop()
          partobj.append(d)
          if 1 <= self.debug:
            print 'end dict: %r' % d
        elif t == 'obj':
          # begin indirect object
          if 1 <= self.debug:
            print 'start obj'
          stack.append((partobj, partobjtype))
          partobj = []
          partobjtype = 'o'
        elif t == 'endobj':
          # end indirect object
          assert partobjtype == 'o', partobjtype
          assert len(partobj) == 1, len(partobj)
          o = partobj[0]
          (partobj, partobjtype) = stack.pop()
          assert 2 <= len(partobj) and isinstance(partobj[-2], int) and isinstance(partobj[-1], int)
          genno = partobj.pop()
          objid = partobj.pop()
          if objid not in self.objdic:
            obj = PDFObjRef(objid, genno, self, basepos)
            self.objdic[objid] = obj
          else:
            obj = self.objdic[objid]
          obj.value = o
          partobj.append(obj)
          if 1 <= self.debug:
            print 'end obj: %r' % obj
          if obj1: return
        elif t == 'R':
          # reference to indirect object
          assert 2 <= len(partobj) and isinstance(partobj[-2], int) and isinstance(partobj[-1], int)
          genno = partobj.pop()
          objid = partobj.pop()
          if objid not in self.objdic:
            obj = PDFObjRef(objid, genno, self, basepos)
            self.objdic[objid] = obj
          else:
            obj = self.objdic[objid]
          partobj.append(obj)
          if 1 <= self.debug:
            print 'refer obj: %r' % obj
        elif t == 'stream':
          # stream object
          assert line.endswith('\n'), 'not \\n after stream'
          assert 1 <= len(partobj) and isinstance(partobj[-1], dict)
          dic = partobj.pop()
          objlen = resolve(dic['/Length'])
          zz = self.curpos
          self.fp.seek(self.curpos)
          data = self.fp.read(objlen)
          self.seek(self.curpos+objlen)
          while 1:
            line = self.nextline()
            if not line:
              assert 0, 'premature eof, need endstream'
            if line.strip():
              assert line.startswith('endstream'), 'need endstream'
              break
          #assert 0, repr(line)
          (partobj, partobjtype) = stack.pop()            
          print partobj
          id = partobj[-1]
          gen = partobj[-2]
          partobj.append(PDFStream(dic, self._decryptObj((id,gen),data)))
          linepos = len(line)
          if 1 <= self.debug:
            print 'stream:', dic, zz, objlen, repr(data[:10])
        elif self.NUMBER.match(t):
          # number
          if '.' in t:
            n = float(t)
          else:
            n = int(t)
          partobj.append(n)
          if 1 <= self.debug:
            print 'number: %r' % n
        else:
          # normal token
          partobj.append('@'+t)
          if 1 <= self.debug:
            print 'token: %r' % t
    # cleanup
    assert not obj1
    assert not stack
    assert partobjtype == None
    return partobj

  def find_xref(self):
    # find the first xref table
    prev = None
    for line in self.revreadlines():
      line = line.strip()
      if line == 'startxref': break
      prev = line
    else:
      assert 0, 'not startxref'
    return long(prev)

  # read xref table and trailers
  def read_xref(self, pos0):
    trailers = []
    while pos0:
      # read xref table
      self.seek(pos0)
      pos0 = 0
      line = self.nextline()
      if line.strip() != 'xref':
        # XRefStream: PDF-1.5
        if line[0].isdigit():
          assert 0, 'XRef Stream?: PDF-1.5 is not supported yet.'
        else:
          assert 0, 'not xref: %r' % line
      while 1:
        line = self.nextline()
        if not line:
          assert 0, 'premature eof'
        line = line.strip()
        f = line.split(' ')
        if len(f) != 2:
          assert line == 'trailer'
          break
        (start, n) = map(long, f)
        for objid in xrange(start, start+n):
          line = self.nextline()
          f = line.strip().split(' ')
          assert len(f) == 3
          (pos, genno, use) = f
          if use == 'n':
            self.objdic[objid] = PDFObjRef(objid, int(genno), self, long(pos))
      # read trailer
      objs = self.parse1()
      assert len(objs) == 1 and isinstance(objs[0], dict)
      trailer1 = objs[0]
      trailers.append(trailer1)
      if 1 <= self.debug:
        print 'trailer:', trailer1
      if '/Prev' in trailer1:
        # find previous xref
        pos0 = trailer1['/Prev']
        assert isinstance(pos0, int)
        if 1 <= self.debug:
          print 'prev trailer:', pos0
    #-ysn create internal object to hold trailers
    self.trailers = trailers  
    return trailers
#------------------ copy from pyPdf ------------
  def _decryptObj(self, id, obj):
      idnum,generation = id                         
      import struct, md5
      pack1 = struct.pack("<i", idnum)[:3]
      pack2 = struct.pack("<i", generation)[:2]
      key = self._decryption_key + pack1 + pack2
      assert len(key) == (len(self._decryption_key) + 5)
      md5_hash = md5.new(key).digest()
      key = md5_hash[:min(16, len(self._decryption_key) + 5)]
      retval = self._decryptObject(retval, key)

      self.cacheIndirectObject(generation, idnum, retval)
      return retval


      return utils.RC4_encrypt(key, obj._data)
                         
  def safeGetObject(self, obj):
      return resolve(obj)
  def _decrypt(self, password):
      encrypt = resolve(self.trailers[0]['/Encrypt'])
      dumpobj(encrypt)
      if encrypt['/Filter'] != '/Standard':
          raise NotImplementedError, "only Standard PDF encryption handler is available"
      if not (encrypt['/V'] in (1, 2)):
          raise NotImplementedError, "only algorithm code 1 and 2 are supported"
      user_password, key = self._authenticateUserPassword(password)
      if user_password:
          self._decryption_key = key
          return 1
      else:
          rev = self.safeGetObject(encrypt['/R'])
          if rev == 2:
              keylen = 5
          else:
              keylen = self.safeGetObject(encrypt['/Length']) / 8
          key = _alg33_1(password, rev, keylen)
          real_O = self.safeGetObject(encrypt["/O"])
          if rev == 2:
              userpass = utils.RC4_encrypt(key, real_O)
          else:
              val = real_O
              for i in range(19, -1, -1):
                  new_key = ''
                  for l in range(len(key)):
                      new_key += chr(ord(key[l]) ^ i)
                  val = utils.RC4_encrypt(new_key, val)
              userpass = val
          owner_password, key = self._authenticateUserPassword(userpass)
          if owner_password:
              self._decryption_key = key
              return 2
      return 0

  def _authenticateUserPassword(self, password):
      encrypt = resolve(self.trailers[0]['/Encrypt'])
      rev = encrypt['/R']
      owner_entry = encrypt['/O']
      p_entry = encrypt['/P']
      id_entry = self.trailers[0]['/ID']
      id1_entry = resolve(id_entry[0])
      if rev == 2:
          U, key = _alg34(password, owner_entry, p_entry, id1_entry)
      elif rev >= 3:
          U, key = _alg35(password, rev,
                  self.safeGetObject(encrypt["/Length"]) / 8, owner_entry,
                  p_entry, id1_entry,
                  self.safeGetObject(encrypt.get("/EncryptMetadata", False)))
      real_U = self.safeGetObject(encrypt['/U'])
      return U == real_U, key

  def getIsEncrypted(self):
      return self.trailer.has_key("/Encrypt")

#---------------------------------- _alg -------------
from pyPdf import utils    
# ref: pdf1.8 spec section 3.5.2 algorithm 3.2
_encryption_padding = '\x28\xbf\x4e\x5e\x4e\x75\x8a\x41\x64\x00\x4e\x56' + \
        '\xff\xfa\x01\x08\x2e\x2e\x00\xb6\xd0\x68\x3e\x80\x2f\x0c' + \
        '\xa9\xfe\x64\x53\x69\x7a'

def _alg32(password, rev, keylen, owner_entry, p_entry, id1_entry, metadata_encrypt=True):
    import md5, struct
    m = md5.new()
    password = (password + _encryption_padding)[:32]
    m.update(password)
    m.update(owner_entry)
    p_entry = struct.pack('<i', p_entry)
    m.update(p_entry)
    m.update(id1_entry)
    if rev >= 3 and not metadata_encrypt:
        m.update("\xff\xff\xff\xff")
    md5_hash = m.digest()
    if rev >= 3:
        for i in range(50):
            md5_hash = md5.new(md5_hash[:keylen]).digest()
    return md5_hash[:keylen]

def _alg33(owner_pwd, user_pwd, rev, keylen):
    key = _alg33_1(owner_pwd, rev, keylen)
    user_pwd = (user_pwd + _encryption_padding)[:32]
    val = utils.RC4_encrypt(key, user_pwd)
    if rev >= 3:
        for i in range(1, 20):
            new_key = ''
            for l in range(len(key)):
                new_key += chr(ord(key[l]) ^ i)
            val = utils.RC4_encrypt(new_key, val)
    return val

def _alg33_1(password, rev, keylen):
    import md5
    m = md5.new()
    password = (password + _encryption_padding)[:32]
    m.update(password)
    md5_hash = m.digest()
    if rev >= 3:
        for i in range(50):
            md5_hash = md5.new(md5_hash).digest()
    key = md5_hash[:keylen]
    return key

def _alg34(password, owner_entry, p_entry, id1_entry):
    key = _alg32(password, 2, 5, owner_entry, p_entry, id1_entry)
    U = utils.RC4_encrypt(key, _encryption_padding)
    return U, key

def _alg35(password, rev, keylen, owner_entry, p_entry, id1_entry, metadata_encrypt):
    import md5
    m = md5.new()
    m.update(_encryption_padding)
    m.update(id1_entry)
    md5_hash = m.digest()
    key = _alg32(password, rev, keylen, owner_entry, p_entry, id1_entry)
    val = utils.RC4_encrypt(key, md5_hash)
    for i in range(1, 20):
        new_key = ''
        for l in range(len(key)):
            new_key += chr(ord(key[l]) ^ i)
        val = utils.RC4_encrypt(new_key, val)
    return val + ('\x00' * 16), key


def isb(i):
  return str(i)+' '*i
# dumpdic
def dumpobj(obj0, i=0, parents={}):
  if not isinstance(obj0, PDFObjRef) or obj0.objid not in parents:
    if isinstance(obj0, PDFObjRef):
      print '%d=' % obj0.objid,
      parents[obj0.objid] = 1
    obj1 = resolve(obj0)
  else:
    obj1 = obj0
  if isinstance(obj1, dict):
    print '<<',
    for (k,v) in obj1.iteritems():
      print
      print isb(i)+k+':',
      dumpobj(v, i+1, parents)
    #print
    #print isb(i)+'>>',
    print '>>',
  elif isinstance(obj1, list):
    print '[',
    for v in obj1:
      dumpobj(v, i+1, parents)
    print ']',
  elif isinstance(obj1, PDFStream):
    print 'Stream:', repr(obj1.get_data()[:20])+'...',
  elif isinstance(obj1, str):
    if obj1[0] == '/':
      print obj1,
    elif obj1[0] == '@':
      print obj1[1:],
    else:
      print repr(obj1[1:]),
  elif isinstance(obj1, PDFObjRef):
    print '=%d' % obj1.objid,
  return


# dumppdf
def dumppdf(fname, debug=0):
  fp = file(fname, 'rb')
  parser = PDFParser(fp, debug)
  trailers = parser.read_xref(parser.find_xref())
  #print trailers
  for t in trailers:
    if '/Root' in t:
      root = t['/Root']
      break
  else:
    assert 0, 'no /Root object!'
  parser._decrypt('')
  
  dumpobj(resolve(root), 0, {})
  fp.close()
  return

# main
if __name__ == '__main__':
  import sys, getopt
  def usage():
    print "usage: pdfparser.py [-d] pdffile"
    sys.exit(2)
  try:
    (opts, args) = getopt.getopt(sys.argv[1:], "d")
  except getopt.GetoptError:
    usage()
  if not args: usage()
  debug = 0
  for (k, v) in opts:
    if k == "-d": debug += 1
  dumppdf(args[0], debug)
