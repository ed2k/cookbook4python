#!/usr/bin/env python

import math, optparse, random, socket, sys, time
import dpkt, dnet
import struct

TMP_SCTP = '\x80\x44\x00\x50\x00\x00\x00\x00\x30\xba\xef\x54\x01\x00\x00\x3c\x3b\xb9\x9c\x46\x00\x01\xa0\x00\x00\x0a\xff\xff\x2b\x2d\x7e\xb2\x00\x05\x00\x08\x9b\xe6\x18\x9b\x00\x05\x00\x08\x9b\xe6\x18\x9c\x00\x0c\x00\x06\x00\x05\x00\x00\x80\x00\x00\x04\xc0\x00\x00\x04\xc0\x06\x00\x08\x00\x00\x00\x00'

def mac2str(mac):
    return "".join(map(lambda x: chr(int(x,16)), mac.split(":")))
def str2mac(s):
    return ("%02x:"*6)[:-1] % tuple(map(ord, s))
def d2ip(s):
    return ("%d."*4)[:-1] % tuple(map(ord, s))
# '12.12.12.12' -> '\xc\xc\xc\xc'
def ip2hex(s):
    return ''.join([chr(int(x)) for x in s.split('.')])

class Test(object):
    def __init__(self):
        usage = '%prog [OPTIONS] <host>'
        self.op = optparse.OptionParser(usage=usage)
        self.op.add_option('-c', dest='count', type='int', default=sys.maxint,
                           help='Total number of queries to send')
        self.op.add_option('-i', dest='wait', type='float', default=1,
                           help='Specify packet interval timeout in seconds')

    def gen_ping(self, opts):
        for i in xrange(opts.count):
            chunk = dpkt.sctp.Chunk()
            sctp = dpkt.sctp.SCTP(data=str(chunk))
            sctp.sport = 8110
            sctp.dport = 8198
            sctp.type = dpkt.sctp.INIT
	    
	    ip = dpkt.ip.IP(src=ip2hex('142.133.118.53'),dst=ip2hex('142.133.69.26'),p=132,data=str(sctp))
            ip.len = len(ip)
            yield str(ip)

    def print_sctp(self, buf):
        sport,dport,tag,checksum = struct.unpack('>HH4s4s',buf[:12])
        result = "%d %d" %(sport,dport)
	len_buf = len(buf)
        if len_buf > 12:
            type,flag,clen = struct.unpack('>BBH',buf[12:16])
            result = "%s %d %d" %(result,type,clen)
        if len_buf > 16:
            chunk = dpkt.sctp.Chunk(buf[16:])
            result = "%s %d %d %d" %(result,chunk.type,chunk.flags,chunk.len)
            pass
        return result
        
    def print_ip(self, buf):
        ver_ihl,tos,iplen, id,flags_offset,ttl,proto,checksum,src,dst = struct.unpack('>ssH2s2ssB2s4s4s',buf[:20])
        result = "%d %s %s %d" %(proto,d2ip(src),d2ip(dst),iplen)
      	len_buf = len(buf)
        if len_buf > 20:
            result = " ".join([result,self.print_sctp(buf[20:])])
        return result
        
    def main(self, argv=None):
        if not argv:
            argv = sys.argv[1:]
        opts, args = self.op.parse_args(argv)
    
        if not args:
            self.op.error('missing host')
        elif len(args) > 1:
            self.op.error('only one host may be specified')

        host = args[0]
        opts.ip = socket.gethostbyname(host)

        sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, 6)
        #sock.connect((opts.ip, 1))
        #sock.settimeout(opts.wait)

        for p in self.gen_ping(opts):
            try:
                #print [ord(x) for x in p]
                #sock.send(p)
                sent = dnet.ip().send(p)
	        print sent,
	        self.print_sctp(p)
                
                #buf = sock.recv(0xffff)
                #ip = dpkt.ip.IP(buf)
                #self.print_sctp(buf)

            except socket.timeout:
                pass
            time.sleep(opts.wait)


if __name__ == '__main__':
    test = Test()
    #p.main()
    import scapy
    p = scapy.sniff(iface='eth0',count=1,filter='ip and not tcp and not udp and not icmp')
    oldp = str(p[0].payload)
    print 'old p', test.print_ip(oldp)

    oldip = dpkt.ip.IP(oldp)
    #print [ord(x) for x in oldip.data]

    sctp = dpkt.sctp.SCTP(oldp[20:])
    sctp.type = dpkt.sctp.INIT_ACK
    sport = sctp.sport
    sctp.sport = sctp.dport
    sctp.dport =sport
    print 'sctp', test.print_sctp(str(sctp))
    
    ip = dpkt.ip.IP(src=oldip.dst,dst=oldip.src,p=132,data=str(sctp))
    ip.len = len(ip)
    test.print_sctp( str(ip))
    dnet.ip().send(str(ip))
    
    p = scapy.sniff(iface='eth0',count=1,filter='ip and not tcp and not udp and not icmp')
    oldp = str(p[0].payload)
    print '2nd',test.print_ip(oldp)
    
