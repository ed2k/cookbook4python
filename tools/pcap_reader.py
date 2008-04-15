import os,sys
import struct
import dpkt

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
UDP = 0x11
GTP_U_PORT = 2152
T_PDU = 0xff

sender_cnt = 0
cnt = 0

hdr = f.read(16)
while len(hdr) == 16:
    sec,usec,caplen,olen = struct.unpack(endian+"IIII", hdr)
    s = f.read(caplen)
    # read next packet first
    hdr = f.read(16)
    # assume ethernet packet
    if len(s) < 14: continue
    dst,src,etype = struct.unpack(endian+'6s6sH',s[:14])
    if etype != 0x800: continue
    #fcs = struct.unpack(endian+'ssss',s[-4:])
    #print str2mac(dst), str2mac(src), 
    ip = s[14:-4]
    ver_ihl,tos,iplen, id,flags_offset,ttl,proto,checksum,src,dst = struct.unpack('>ssH2s2ssB2s4s4s',ip[:20])
    if src[:3] == '\x0a\xc8\x04' and src[3] in ['\x03','\x04','\x05']:
        sender_cnt += 1

    #data = ip[24:]
    if proto != UDP: continue
    #print d2ip(src),d2ip(dst),iplen,('%x' % proto)

    udp = ip[20:]
    src,dst,size,checksum = struct.unpack('>HHHH',udp[:8])    
    #print src, dst
    if dst != GTP_U_PORT: continue

    gtp = udp[8:]
    flags,etype,length,teid = struct.unpack('>ssH4s',gtp[:8])
    #print [etype],[teid]
    if etype != '\xff': continue

    ip = gtp[8:]
    ver_ihl,tos,iplen, id,flags_offset,ttl,proto,checksum,src,dst = struct.unpack('>ssH2s2ssB2s4s4s',ip[:20])
    if src[:3] == '\x0a\xc8\x04' and src[3] in ['\x03','\x04','\x05']:
        cnt += 1
    

print cnt, sender_cnt
