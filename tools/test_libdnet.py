#!/usr/bin/env python

import sys, unittest

sys.path.insert(0, './build')
import dnet
class unittest_TestCase:
	pass

        
class EthTestCase(unittest_TestCase):
    def setUp(self):
        self.dev = dnet.intf().get_dst(dnet.addr('1.2.3.4'))['name']
        self.eth = dnet.eth(self.dev)
        self.failUnless(self.eth, "couldn't open Ethernet handle")
    def tearDown(self):
        del self.eth

    def test_eth_get(self):
        mac = self.eth.get()
        self.failUnless(mac, "couldn't get Ethernet address for %s" % self.dev)

    def test_eth_misc(self):
        n = "\x00\x0d\x0e\x0a\x0d\x00"
        a = '00:0d:0e:0a:0d:00'
        self.failUnless(dnet.eth_ntoa(n) == a)
        self.failUnless(dnet.eth_aton(a) == n)
        dst = "\x00\x0d\x0e\x0a\x0d\x01"
        self.failUnless(dnet.eth_pack_hdr(n, dst, dnet.ETH_TYPE_IP) ==
                        '\x00\r\x0e\n\r\x00\x00\r\x0e\n\r\x01\x08\x00')


class IpTestCase(unittest.TestCase):
    def setUp(self):
        self.ip = dnet.ip()
        self.failUnless(self.ip, "couldn't open raw IP handle")
    def tearDown(self):
        del self.ip

    def test_ip_misc(self):
        n = '\x01\x02\x03\x04'
        a = '1.2.3.4'
        self.failUnless(dnet.ip_ntoa(n) == a)
        self.failUnless(dnet.ip_aton(a) == n)
        dst = '\x05\x06\x07\x08'
        hdr = dnet.ip_pack_hdr(0, dnet.IP_HDR_LEN, 666, 0, 255,
                               dnet.IP_PROTO_UDP, n, dst)
        assert hdr == 'E\x00\x00\x14\x02\x9a\x00\x00\xff\x11\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08'
        hdr = dnet.ip_checksum(hdr)
        assert hdr == 'E\x00\x00\x14\x02\x9a\x00\x00\xff\x11\xa9+\x01\x02\x03\x04\x05\x06\x07\x08'
    def test_ip_send(self):
        ip = '\x80\x44\x00\x50\x00\x00\x00\x00\x30\xba\xef\x54\x01\x00\x00\x3c\x3b\xb9\x9c\x46\x00\x01\xa0\x00\x00\x0a\xff\xff\x2b\x2d\x7e\xb2\x00\x05\x00\x08\x9b\xe6\x18\x9b\x00\x05\x00\x08\x9b\xe6\x18\x9c\x00\x0c\x00\x06\x00\x05\x00\x00\x80\x00\x00\x04\xc0\x00\x00\x04\xc0\x06\x00\x08\x00\x00\x00\x00'
	dnet.ip().send(ip)


if __name__ == '__main__':
    unittest.main()

