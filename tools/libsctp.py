import socket

#size_t unsigned int malloc.h
#bits/socket.h
PF_INET = 2
SOCK_SEQPACKET = 5
#netinet/in.h
IPPROTO_SCTP = 132
SCTP_SNDRCV = 0x100

from ctypes import *
#Lib = CDLL('./test.so')
#Lib = CDLL('libsocket.so')

# assume defined _XPG4_2 
#sys/socket.h socklen_t size_t
#sys/types.h size_t uint
# int, long, size_t, socklen are 4 bytes

class sockaddr(Structure):
    _fields_ = [('sa_family',c_ushort),
                ('sa_data',c_char*14)]
#sctp.h
# 32 bytes
# sctp_assoc_t int
class sctp_sndrcvinfo(Structure):
    _fields_ = [('stream',c_uint16),
                ('ssn',   c_uint16),
                ('flags', c_uint16),
                ('ppid',  c_uint32),
                ('context',   c_uint32),
                ('timetolive',c_uint32),
                ('tsn',   c_uint32),
                ('cumtsn',c_uint32),
                ('assoc_id',  c_int)]
# sys/types.h
# 8 bytes
class iovec(Structure):
    _fields_ = [('iov_base',c_uint),
                ('iov_len',c_uint)]
# 12 bytes
class cmsghdr(Structure):
    _fields_ = [('cmsg_len',c_uint),
                ('cmsg_level',c_int),
                ('cmsg_type',c_int)]
class cbuf(Structure):
    _fields_ = [('cmsg',cmsghdr),('sri',sctp_sndrcvinfo)]
# 28 bytes
class msghdr(Structure):
    _fields_ = [('msg_name', c_uint),
                ('msg_namelen',c_uint),
                ('msg_iov',   POINTER(iovec)),
                ('msg_ioven', c_int),
                ('msg_control',POINTER(cbuf)),
                ('msg_controllen',c_uint),
                ('msg_flags',c_int)]

def recv(sock):
   msg = create_string_buffer(1000)
   len = c_uint()
   safrom = sockaddr()
   fromlen = c_uint()
   sinfo = sctp_sndrcvinfo()
   msg_flags = c_int()
   Lib.sctp_recvmsg(c_int(sock.fileno()),
             byref(msg),byref(len),
             byref(safrom),byref(fromlen),
             byref(sinfo),byref(msg_flags))

#print msg.value
def send(sock,pstr):
   cstr = create_string_buffer(pstr)
   msg = msghdr()
   memset(pointer(msg),0,sizeof(msg))
   iov = iovec()
   memset(pointer(iov),0,sizeof(iovec))
   msg.msg_iov = pointer(iov)
   iov.iov_base = addressof(cstr)
   iov.iov_len = c_uint(len(pstr))
   msg.msg_iovlen = c_uint(1)
   c = cbuf()
   memset(pointer(c),0,sizeof(cbuf))
   msg.msg_control =  pointer(c)
   msg.msg_controllen = c_uint(sizeof(cbuf))
   c.cmsg.cmsg_len = c_uint(sizeof(cbuf))
   c.cmsg.cmsg_level = c_int(IPPROTO_SCTP)
   c.cmsg.cmsg_type = c_int(SCTP_SNDRCV)
   c.sri.ppid = c_uint32(3)
   c.sri.stream = c_uint16(20)

   d(msg)
   pd(msg.msg_iov)
   d(msg.msg_control[0].cmsg)
   d(msg.msg_control[0].sri)
   #n = Lib.send(c_int(sock.fileno()),byref(msg),c_int(0))
   #n = Lib.m3ua_send(c_int(sock.fileno()),c_char_p(pstr),c_int(len(pstr)))

def d(obj):
    for field in obj._fields_:
        print field[0],getattr(obj,field[0])
    cstr = create_string_buffer(sizeof(obj))
    memmove(addressof(cstr),addressof(obj),sizeof(obj))
    print [ord(x) for x in cstr.raw]

def pd(p):
    d(p[0])

send([],"abc")
           



