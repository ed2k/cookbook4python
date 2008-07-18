import socket,sys,os
import dpkt,m3ua
import libsctp

def bin2str(bytes):
   return [ord(b) for b in bytes]
def str2bin(s):
   return "".join([chr(x) for x in s])

Tsst = m3ua.LenParam(data=str(m3ua.SST(PC=2738,SSN=151)))
Tsst.update()
Taddr = m3ua.LenParam(data='\xc2\x01')
Taddr.update()

Tsccp = m3ua.SCCP()
Tsccp.type = m3ua.Unitdata
Tsccp.data = str2bin([0,3,5,7])+str(Taddr)+str(Taddr)+str(Tsst)

TprotoData = m3ua.ProtocolData(OPC=3900,DPC=2738,SI=3,MP=1,SLS=15)

Tparam = m3ua.Parameter(tag=m3ua.PROTOCOLDATA,data=str(TprotoData)+str(Tsccp))
Tparam.update()

Tm3ua = m3ua.M3UA(msgclass=m3ua.TransferM,msgtype=m3ua.DATA)
Tm3ua.data = str(Tparam)
Tm3ua.update()


def process(param):    
    sock,w = param 
    buf = sock.read(0xffff)
    msg = m3ua.M3UA(buf)
    print msg.msgclass,msg.msgtype,msg.msglen
    if (msg.msgclass == m3ua.ASPSM) and (msg.msgtype == m3ua.ASPUP):
        msg.msgtype = m3ua.ASPUPACK
    elif (msg.msgclass == m3ua.ASPTM) and (msg.msgtype == m3ua.ASPAC):
        msg.msgtype = m3ua.ASPACACK
        w.write(str(msg))
        msg = Tm3ua
    if msg is not None: 
        #libsctp.send(sock,str(msg))
        w.write( str(msg))
        w.flush()
    
def one_socket(sock):
    try:
       while True:
          process(sock)
    except:
       print "something Wrong"



def start_server(sock):
   sock.listen(1)

   while True:
      conn,addr = sock.accept()
      one_socket(conn) 

def start_client(sock,address):
   sock.connect(address)
   print sock.fileno()
   w,r=os.popen4("pkg/sctp_solaris/examples/test")
   #libsctp.send(sock,str(m3ua.M3UA())) 
   #w.write( str(m3ua.M3UA(msgclass=m3ua.ASPSM,msgtype=m3ua.ASPUP)) )
   #w.flush()
   one_socket((sock,w))

PF_INET = 2
IPPROTO_SCTP = 132

sock = socket.socket(PF_INET,socket.SOCK_STREAM,IPPROTO_SCTP)
sock.bind(('142.133.69.26',2907))

start_client(sock,(sys.argv[1],2907))
