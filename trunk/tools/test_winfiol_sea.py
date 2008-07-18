import socket,sys,os


def bin2str(bytes):
   return [ord(b) for b in bytes]
def str2bin(s):
   return "".join([chr(x) for x in s])


def process(param):    
    sock,w = param 
    buf = sock.read(0xffff)
    print [buf]
    msg = buf[:2]
    msglen = ord(buf[3])
    data = buf[3:3+msglen]
    if msg == '\0xff\0x0' and msglen == 8:
        sock.send('\0x0\0x0\0x01\0x01')

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
   s = []
   for x in 'ff 00 17 01 7d b8 68 02 15 27 11 d3 a2 cc 08 00 20 96 15 3c 41 54 2d 31 31 00'.split():
	   if x=='00': s.append('0')
	   else: s.append(x)
   s = ''.join([chr(eval('0x'+x)) for x in s])
   print [s]
   sock.send(s)
   one_socket((sock,None))

sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
#sock.bind(('qlmsc9',32768))

start_client(sock,('qlmsc9',32768))
