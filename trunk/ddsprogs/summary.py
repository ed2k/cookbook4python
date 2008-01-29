#!/usr/local/bin/python

import sys
import cPickle
import drescomm

def main():
    dres = {}
    for n in range(15,18):
        for s in range(8, 11):
            dres[n,s]={}
    timetot = 0.0
    i = 0
    for fn in sys.argv[1:]:
        logfile = open(fn)
        print fn
        for line in logfile:
            try: n, s, tricks, time, _ = line.split(None, 4)
            except ValueError: continue
            timetot += float(time)
            n = int(n)
            s = int(s)
            tricks = int(tricks)
            dres[n,s][tricks] = 1+dres[n,s].get(tricks,0)
            i += 1
        drescomm.report(i, timetot, dres)

    f = open('pick_dds.pick', 'wb')
    cPickle.dump(i, f, 2)
    cPickle.dump(timetot, f, 2)
    cPickle.dump(dres, f, 2)
    f.close()

main()
