import socket
import dpkt,m3ua

def bin2str(bytes):
   return [ord(b) for b in bytes]
def str2bin(s):
   return "".join([chr(x) for x in s])

sst = m3ua.LenParam(data=str(m3ua.SST(PC=2738,SSN=142)))
sst.update()
addr = m3ua.LenParam(data='\xc2\x01')
addr.update()

sccp = m3ua.SCCP()
sccp.type = m3ua.Unitdata
sccp.data = str2bin([0,3,5,7])+str(addr)+str(addr)+str(sst)

protoData = m3ua.ProtocolData(OPC=2932,DPC=2738,SI=3,MP=1,SLS=15)

param = m3ua.Parameter(tag=m3ua.PROTOCOLDATA,data=str(protoData)+str(sccp))
param.update()

msg = m3ua.M3UA()
msg.msgclass = m3ua.TransferM
msg.msgtype = m3ua.DATA        
msg.data = str(param)
msg.update()

print dpkt.hexdump(str(msg))


    
    

