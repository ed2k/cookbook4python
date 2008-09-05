from socket import *
import string
import time,sys
import udpGenServer 

import sys, time, thread


if __name__ == '__main__':
    # REMEMBER TO SPECIFY THE PROPER DETINATION HOST HERE...
    # 'host' should be the address that the server half of this is running on
    host = sys.argv[1]
    UDPSock = socket(AF_INET,SOCK_DGRAM)
    datasize = int(sys.argv[2])
    pktsPerSecs = int(sys.argv[3])
    data = "X" * int(datasize)
    print "udp_stress_server.py running on %s port %s" % (host, udpGenServer.port)
    print "\n\nEnter number of bytes to send and the number of times to send them:\n(for instance '100 10' to send 10 bursts of 100 bytes each)";    
    thread.start_new_thread(udpGenServer.rate_print,())
    initTime = time.time()
    while True:
        sz = UDPSock.sendto(data,(host,udpGenServer.port))
        udpGenServer.totalrcvs += 1
        udpGenServer.totalbytes += sz
        if sz != datasize:
            print "E",sz
        dt = time.time() - initTime
        tLeft = udpGenServer.totalrcvs - dt*pktsPerSecs
        #print tLeft
        if tLeft > 0.001: time.sleep(tLeft/pktsPerSecs)
    print
    print "Done.", totalbytes, totalrcvs
    UDPSock.close()
