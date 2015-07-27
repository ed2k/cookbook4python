import os,sys
import struct
#import dpkt

#use dpkt instead, test outdated

def mac2str(mac):
    return "".join(map(lambda x: chr(int(x,16)), mac.split(":")))
def str2mac(s):
    return ("%02x:"*6)[:-1] % tuple(map(ord, s))
def d2ip(s):
    return ("%d."*4)[:-1] % tuple(map(ord, s))
    
from pcapfile import savefile
f = open(sys.argv[1])
c = savefile.load_savefile(f, verbose=True)

#self.LLcls = LLTypes.get(linktype, Raw)
UDP = 0x11
TCP = 0x6
GTP_U_PORT = 2152
T_PDU = 0xff

sender_cnt = 0
cnt = 0
iprange = ['\x06']
pkt_no = 0

endian = '<'

for pkt in c.packets:
    pkt_no += 1
    s = pkt.raw()
    # assume ethernet packet
    #if len(s) < 14: continue
    dst,src,q,etype = struct.unpack(endian+'6s6sHH',s[:16])
    #if etype != 0x800: continue
    #fcs = struct.unpack(endian+'ssss',s[-4:])
    #print (str2mac(dst), str2mac(src)), 
    ip = s[16:]
    ver_ihl,tos,iplen, id,flags_offset,ttl,proto,checksum,src,dst = struct.unpack('>BsH2s2ssB2s4s4s',ip[:20])
    ihl = ver_ihl & 0xf
    if ihl > 5: print pkt_no,'ip header length', ihl

    #data = ip[24:]
    if proto != TCP: continue
    print d2ip(src),d2ip(dst),iplen,('%x' % proto),

    udp = ip[20:]
    src,dst,seq,ack = struct.unpack('>HHII',udp[:12])    
    print '%x %x %x %x' % (src, dst, seq, ack)

    #gtp = udp[8:]
    #print [etype],[teid]

print cnt, sender_cnt
