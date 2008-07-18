import socket,sys
import struct

def mac2str(mac):
    return "".join(map(lambda x: chr(int(x,16)), mac.split(":")))
def str2mac(s):
    return ("%02x:"*6)[:-1] % tuple(map(ord, s))
def d2ip(s):
    return ("%d."*4)[:-1] % tuple(map(ord, s))

# http://www.networksorcery.com/enp/protocol/ip.htm
# tcp 6, sctp 132, udp 17
proto = int(sys.argv[1])
sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, proto)

while 1:
    try:
        buf = sock.recv(0xffff)
        ver_ihl,tos,iplen, id,flags_offset,ttl,proto,checksum,src,dst = struct.unpack('>ssH2s2ssB2s4s4s',buf[:20])
        sport,dport,seq = struct.unpack('>HH4s',buf[20:28])
        print proto,d2ip(src),d2ip(dst),iplen,sport,dport,[ord(x) for x in seq]
    except socket.timeout:
        pass
