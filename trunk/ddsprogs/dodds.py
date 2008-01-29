#!/usr/local/bin/python

import dds
import os
import time
timer = time.time

def dod(ds):
    ''' compute double-dummy tricks (N declarer, NT; call dd.solve with
        leader code, 0=N to 3=W, and trump code, 0=S to 3=C, for other).

        Args:
            ds: a string describing the whole deal, such as
  'n AQ3.T653.KQ4.AJ5 e KJT4.AKQ8.JT86.8 s 862.974.A73.KQ72 w 975.J2.952.T9643'
        Returns:
            tricks, time
                where tricks, an int, is the number of NS tricks,
                      time, a float, is the elapsed time (in seconds and
                          fraction) expended for the analysis
    '''
    start = timer()
    deal = [h.split('.') for h in ds.split()[1::2]]
    dd = dds.deal(deal)
    s = dd.solve()
    print s
    return 13-s[-1][0], timer()-start

def shodres(n, s, dr):
    """ display summary of a dict of results with certain HCP values """
    h = sum(dr.itervalues())
    print '%2.2d-%2.2d (%d):' % (n, s, h),
    k = sorted(dr)
    for t in k:
        print '%d:%d' % (t, dr[t]),
    print

def report(i, timetot, dres):
    print '%d hands in %.2f (%.2f)' % (
            i+1, timetot, (timetot)/(i+1) )
    for n in range(15,18):
        for s in range(8, 11):
            shodres(n, s, dres[n,s])
    print

def main():
    logfile = open('ddslog2.txt', 'a+')
    dres = {}
    for n in range(15,18):
        for s in range(8, 11):
            dres[n,s]={}
    timetot = 0.0
    i = 0
    logfile.seek(0, 0)
    for line in logfile:
        n, s, tricks, time, _ = line.split(None, 4)
        timetot += float(time)
        n = int(n)
        s = int(s)
        tricks = int(tricks)
        dres[n,s][tricks] = 1+dres[n,s].get(tricks,0)
        i += 1
    report(i, timetot, dres)
    delta = 30.0
    timerepo = timetot + delta
    while i < 100*1000:
        handsfile = os.popen('./dealer ntraise.dds')
        for ds in handsfile:
            try:
                hcps, ds = ds.split('|')
            except ValueError:
                break
            i += 1
            n, s = [int(h) for h in hcps.split()]
            tricks, timelap = dod(ds.strip())
            logfile.write('%d %d %d %.2f %s\n' % ( n, s, tricks, timelap,
                    ' '.join(ds.split()[1::2])))
            dres[n,s][tricks] = 1+dres[n,s].get(tricks,0)
            timetot += timelap
            if timetot >= timerepo:
                timerepo += delta
                report(i, timetot, dres)

main()
