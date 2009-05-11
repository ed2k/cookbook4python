import sys, os
# a webserver take command from client through GET
#if os.name == "nt" or os.name == "dos":
#else:
#    pass

import socket, select, string
import traceback
import urllib


import msnlib, msn


import time, threading
class Timer(threading.Thread):
    def __init__(self, seconds, action):
        self.sleepTime = seconds        
        self.action = action
        threading.Thread.__init__(self)
    def run(self):
        #for i in [1, 2, 3]:
            time.sleep(self.sleepTime)
            self.action()
 		
def parse_cmd(cmd):
	if cmd[:7] == 'ed2k://':
		file('/home/a/.aMule/ED2KLinks','w').write(cmd)
	elif cmd[:7] == 'http://':
		return urllib.urlopen(cmd).read()
	return cmd
		
webcli = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
webcli.bind(('', 31234))
webcli.listen(1)

#t = Timer(53, m.PNG)
#t.start()

# loop
while 1:
	outfd = []
	infd = [webcli]
        
	try:
            timeout = 600
            fds = select.select(infd, outfd, [], timeout)
	except KeyboardInterrupt:
            quit()

	if len(fds[0] + fds[1]) == 0:
            # timeout
            pass #m.PNG()    
	for i in fds[0] + fds[1]:		# see msnlib.msnd.pollable.__doc__
		if i == sys.stdin:
			# read from stdin
			stdin_read()
		elif i == webcli:
			(webcli_conn, addr) = webcli.accept()
		        r = webcli_conn.recv(1024)
			#print r
			r = r.split()[1][1:]
			r = parse_cmd(r)
			r = 'HTTP/1.1 200 OK\r\nContent-Length: '+str(len(r))+'\r\n\r\n'+r
			webcli_conn.send(r)
			webcli_conn.close()
