import os,sys
import struct

#use dpkt instead, test outdated

def mac2str(mac):
    return "".join(map(lambda x: chr(int(x,16)), mac.split(":")))
def str2mac(s):
    return ("%02x:"*6)[:-1] % tuple(map(ord, s))
def d2ip(s):
    return ("%d."*4)[:-1] % tuple(map(ord, s))
    

f = open(sys.argv[1])
magic = f.read(4)
if magic == "\xa1\xb2\xc3\xd4": #big endian
    endian = ">"
elif magic == "\xd4\xc3\xb2\xa1": #little endian
    endian = "<"

hdr = f.read(20)
vermaj,vermin,tz,sig,snaplen,linktype = struct.unpack(endian+"HHIIII",hdr)
print endian,vermaj,vermin,tz,sig,snaplen,linktype
#self.LLcls = LLTypes.get(linktype, Raw)


hdr = f.read(16)
while len(hdr) == 16:
    sec,usec,caplen,olen = struct.unpack(endian+"IIII", hdr)
    s = f.read(caplen)
    # assume ethernet packet
    dst,src,elen = struct.unpack(endian+'6s6sH',s[:14])
    #fcs = struct.unpack(endian+'ssss',s[-4:])
    #print str2mac(dst), str2mac(src), 
    ip = s[14:-4]
    ver_ihl,tos,iplen, id,flags_offset,ttl,proto,checksum,src,dst = struct.unpack('>ssH2s2ssB2s4s4s',ip[:20])

    #data = ip[24:]
    if proto == 6:
        print d2ip(src),d2ip(dst),iplen,('%x' % proto)
    #print data
    hdr = f.read(16)
