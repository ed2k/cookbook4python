import dpkt


#rfc3332
# class
MGMT = 0
TransferM = 1
SSNM = 2
ASPSM = 3
ASPTM = 4
RKM = 9

#type
ERR = 0
NTFY = 1
DATA = 1
# SSNM
DUNA = 1
DAVA = 2
DAUD = 3
SCON = 4
DUPU = 5
DRST = 6

#ASPSM
ASPUP = 1
ASPDN = 2
BEAT = 3
ASPUPACK = 4
ASPDNACK = 5
BEATACK = 6

#ASPTM
ASPAC = 1
ASPIA =2
ASPACACK = 3
ASPIAACK = 4



class M3UA(dpkt.Packet):
   __hdr__ = (
      ('version','B',1),
      ('reserved','B',0),
      ('msgclass','B',1),
      ('msgtype','B',1),
      ('msglen','I',8))
   def update(self):
      self.msglen = 8 + len(self.data)


INFO = 4
RoutingContext = 6
DiagInfo = 0x0007
HeartbeatData = 0x0009
TrafficModeType = 0x000b
Status = 0x000d

NetworkAppearance = 0x200
UserCause = 0x204
CongestionIndications = 0x205
PROTOCOLDATA = 0x210
class Parameter(dpkt.Packet):
  __hdr__ = (
      ('tag','H',1),
      ('length','H',0))
  def update(self):
      length = len(self.data)
      rem = length % 4
      if rem > 0 :
          rem = 4 - rem
          self.data = self.data + '\x00'*rem
      self.length = length + rem +1

class ProtocolData(dpkt.Packet):
  __hdr__ = (
      ('OPC','I',2932),
      ('DPC','I',2738),
      ('SI','B',3),
      ('NI', 'B',3),
      ('MP','B',1),
      ('SLS','B',13))

Unitdata = 0x09

class SCCP(dpkt.Packet):
   __hdr__ = (
      ('type','B',9),
      ('class','B',0) )

class SST(dpkt.Packet):
   __hdr__ = ( 
      ('type','B',3),
      ('SSN','B',142),
      ('PC','H',2738),
      ('SMI','B',0) )

class LenParam(dpkt.Packet):
   __hdr__ = [('length','B',0)]
   def update(self):
      self.length = len(self.data)

class SCCPExUDT(dpkt.Packet):
   __hdr__ = [('hopcnt','B',0xf)]

IE_ID_Cause = 4
IE_ID_CN_DomainIndicator = 3
class IE(dpkt.Packet):
   __hdr__ = [('id','H',4),('criti','B',0x40),('num','B',1),('value','B',0x42)]

class RANAP(dpkt.Packet):
   __hdr__ = [('idx','B',0),('pcode','B',9),('num','B',13),('ext','B',0),
              ('numie','B',2)]


