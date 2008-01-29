#!/usr/local/bin/python

import dds
import os

map = {'A':14,'K':13,'Q':12,'J':11,'T':10,'9':9,'8':8,'7':7,'6':6,'5':5,'4':4,'3':3,'2':2}

def dod(ds):
    deal = [h.split('.') for h in ds.split()[1::2]]
    print deal 
    dd = dds.deal(deal)
    print dd
    #0123 -> shdc which one is trump, whose turn first 0123-> NESW
    for trump in xrange(5):
        for first in xrange(4):
            s = dd.solve(trump,first)
            print trump,first,s
    test = '1 2 16 16 16 16 8 8 8 8 2 2 2 2 4 4 4 4'
    print
    test = []
    for i in xrange(4):
        for j in xrange(4):
            n = 0
            for k in xrange(len(deal[i][j])):
                n = n | (1 << map[deal[i][j][k]])
            test.append(n)
    arg = ' '.join([str(x) for x in test])
    print arg
    print os.popen('./dds 0 0 '+arg).read();
    return 13-s[-1][0]


def main():
    ds = 'n 8.T.. e T.2.. s ..T2. w ..6.T'
    ds = 'n A2... e 76... s QJ... w KT...'
    tricks = dod(ds)
    print tricks



main()
