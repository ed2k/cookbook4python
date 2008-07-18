import socket,sys
import dpkt
import m3ua
import libsctp

sock = socket.socket(2,socket.SOCK_STREAM,132)
sock.bind(('142.133.69.26',2907))
sock.connect((sys.argv[1],2907))

msg = m3ua.M3UA()
libsctp.send(sock,str(msg))

