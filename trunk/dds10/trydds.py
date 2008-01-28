#!/usr/local/bin/python

import sys
import pydds
import time

# timer = time.clock
timer = time.time

def dod(ds):
    t = timer()
    d = timer()
    # deal = [h.split(' ') for h in ds.split('|')]
    deal = [h.split('.') for h in ds.split()[1::2]]
    d = timer() - d
    print 'D', 8*'.','|'.join(' '.join(s) for s in deal)
    # sys.stdout.flush()
    a = timer()
    dd = pydds.deal(deal)
    a = timer() - a
    print 'Hands have', dd.numcards, 'cards each'
    print 'D', dd
    # sys.stdout.flush()
    b = timer()
    s = dd.solve()
    b = timer() - b
    t = timer() - t
    print 'T%d' % s[-1][0],
    print '%.2f %.2f %.2f %.2f' % (d, a, b, t)
    # print '|'.join(' '.join(s) for s in deal),
    print s
    return t


# ds = 'AQ9 AQ9 8643 KQ8|T82 T6542 AKT A7|K653 K83 Q92 543|J74 J7 J75 JT962'
ds='n AQ3.T653.KQ4.AJ5 e KJT4.AKQ8.JT86.8 s 862.974.A73.KQ72 w 975.J2.952.T9643'
print ds
dod(ds)
sys.exit()

def main():
    handsfile = open('t1')
    nh = n = 10
    first = 4
    tot = 0.0
    for i, ds in enumerate(handsfile):
        if i+1 < first: continue
        print "%2.2d"%(i+1),
        tot += dod(ds.strip())
        n -= 1
        if n<=0: break
    print nh,'deals in','%.2f'%tot

main()
