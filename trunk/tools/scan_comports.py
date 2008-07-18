#!/usr/bin/env python
"""Scan for serial ports.
Part of pySerial (http://pyserial.sf.net)  (C)2002-2003 <cliechti@gmx.net>

The scan function of this module tries to open each port number
from 0 to 255 and it builds a list of those ports where this was
successful.
"""

import serial,time
world_stop = False

class SonyEricssonAT:
    def __init__(this,ser):
        this.serialPort = ser
        #ser.timeout = 1
    def atcmd(this, cmd):
        this.serialPort.write(cmd+'\r')
        #print '>',cmd
        line = ''
        result = []
        cnt = 0
        while (line != 'OK' and line != 'ERROR') and (cnt < 30):
            line = this.serialPort.readline()
            cnt+=1
            #print cnt, line
            line = line.strip()
            if line == '': continue
            #print '<',line
            result.append(line)

        return result
            
def check(serialPort):
    s = SonyEricssonAT(serialPort)
    id = serialPort.portstr[-2:]
    #print s.atcmd('AT+COPS=?')
    while not world_stop:
      r = s.atcmd('AT')
      if r[0] == 'RING': print time.ctime(),id, s.atcmd('ATA')
      else: print serialPort.portstr[-2:],r
      time.sleep(1)
    
def scan(one_start_range):
    """scan for available ports. return a list of tuples (num, name)"""
    for i in one_start_range:
        s = None
        try:
            s = serial.Serial(i-1,timeout=1,writeTimeout=1)
            s.timeout = 1
            if s.isOpen():
                print i, s.portstr
                check(s)
        except KeyboardInterrupt:
            print 'ctrl-c stop'
            world_stop = True
        except: pass
        finally:
            if s:s.close()
    return 

if __name__=='__main__':
    import thread    
    #for i in [10,14,18,22,26,30]:
    #   thread.start_new_thread(scan,([i],))
    scan(range(255))
