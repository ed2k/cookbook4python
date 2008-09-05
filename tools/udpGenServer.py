from socket import *
import time, thread


#if you change the port, change it in the client program as well
port = 28105

      
# notice diff, windows, linux  time.clock time.time

# total bytes recieved since last 'reset'
totalbytes = 0
# the total number of bursts that have come in
totalrcvs = 0

def rate_print():
    prevTotalBytes = totalbytes
    prevTotalPkts = totalrcvs
    prevTime = time.time()

    while True:
        time.sleep(3)
        dbytes = totalbytes - prevTotalBytes
        dpkts = totalrcvs -prevTotalPkts
        t = time.time()
        dt = t - prevTime
        rPkts = dpkts/dt
        rBytes = dbytes/dt
        if dbytes > 3:print rPkts,'pkts/s',rBytes,'Bytes/s',totalbytes,totalrcvs
        prevTotalBytes = totalbytes
        prevTotalPkts = totalrcvs
        prevTime = t


if __name__ == '__main__':
    # we want to bind on all possible IP addresses
    host = "0.0.0.0"
    buffer = 102400

    # Create socket and bind to address
    UDPSock = socket(AF_INET,SOCK_DGRAM)
    UDPSock.bind((host,port))    
    thread.start_new_thread(rate_print,())

    while 1:
	data,addr = UDPSock.recvfrom(buffer)
	
	if not data:
		print "No data, connection closed"
		break
	else:
		totalbytes += len(data)
		totalrcvs += 1


    UDPSock.close()
