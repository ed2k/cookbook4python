#!/usr/bin/env python

import math, optparse, random, socket, sys, time
import dpkt
import struct


def mac2str(mac):
    return "".join(map(lambda x: chr(int(x,16)), mac.split(":")))
def str2mac(s):
    return ("%02x:"*6)[:-1] % tuple(map(ord, s))
def d2ip(s):
    return ("%d."*4)[:-1] % tuple(map(ord, s))

class Ping(object):
    def __init__(self):
        usage = '%prog [OPTIONS] <host>'
        self.op = optparse.OptionParser(usage=usage)
        self.op.add_option('-c', dest='count', type='int', default=sys.maxint,
                           help='Total number of queries to send')
        self.op.add_option('-i', dest='wait', type='float', default=1,
                           help='Specify packet interval timeout in seconds')

    def gen_ping(self, opts):
        pass
    def open_sock(self, opts):
        pass
    def print_header(self, opts):
        pass
    def print_reply(self, opts, buf):
        pass
    
    def main(self, argv=None):
        if not argv:
            argv = sys.argv[1:]
        opts, args = self.op.parse_args(argv)
    
        if not args:
            self.op.error('missing host')
        elif len(args) > 1:
            self.op.error('only one host may be specified')

        host = args[0]
        #opts.ip = socket.gethostbyname(host)
        sock = self.open_sock(opts)

        sent = rcvd = rtt_max = rtt_sum = rtt_sumsq = 0
        rtt_min = 0xffff
        try:
            self.print_header(opts)
            for ping in self.gen_ping(opts):
                try:
                    start = time.time()
                    #sock.send(ping)

                    buf = sock.recv(0xffff)
                    #print [ord(x) for x in buf]

                    rtt = time.time() - start
                    
                    if rtt < rtt_min: rtt_min = rtt
                    if rtt > rtt_max: rtt_max = rtt
                    rtt_sum += rtt
                    rtt_sumsq += rtt * rtt

                    self.print_reply(opts, buf, rtt)
                    rcvd += 1
                except socket.timeout:
                    pass
                sent += 1
                #time.sleep(opts.wait)
        except KeyboardInterrupt:
            pass

        print '\n--- the end ---'

class ICMPPing(Ping):
    def __init__(self):
        Ping.__init__(self)
        self.op.add_option('-p', dest='payload', type='string',
                           default='hello world!',
                           help='Echo payload string')

    def open_sock(self, opts):
        sock = socket.socket(socket.AF_INET, socket.SOCK_RAW, 6)
        #sock.connect((opts.ip, 1))
        sock.settimeout(opts.wait)
        return sock
    
    def gen_ping(self, opts):
        for i in xrange(opts.count):
            icmp = dpkt.icmp.ICMP(
                type=8, data=dpkt.icmp.ICMP.Echo(id=random.randint(0, 0xffff),
                                                 seq=i, data=opts.payload))
            yield str(icmp)

##    def print_header(self, opts):
##        print 'PING %s: %d data bytes' % (opts.ip, len(opts.payload))
        
    def print_reply(self, opts, buf, rtt):
        #ip = dpkt.ip.IP(buf)
        ver_ihl,tos,iplen, id,flags_offset,ttl,proto,checksum,src,dst = struct.unpack('>ssH2s2ssB2s4s4s',buf[:20])
        sport,dport,seq = struct.unpack('>HH4s',buf[20:28])
        print d2ip(src),d2ip(dst),iplen,sport,dport,[ord(x) for x in seq]

if __name__ == '__main__':
    p = ICMPPing()
    p.main()