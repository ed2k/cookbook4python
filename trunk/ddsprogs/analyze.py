#!/usr/local/bin/python

import sys
import string
import cPickle
import drescomm

strats = (
    (8, 8, 15), (8, 9, 16), (8, 9, 17), (8, 10, 16), (8, 10, 17),
                (8, 11, 16), (8, 11, 17),
    (9, 9, 15), (9, 10, 16), (9, 10, 17), (9, 11, 16), (9, 11, 17),
    (10, 10, 15), (10, 11, 16), (10, 11, 17), (11, 11, 15),
    )

dres = {}

def n2s(n):
    ''' name to strategy '''
    return tuple(string.uppercase.find(c.upper())+1 for c in n)

def s2n(s):
    ''' strategy to name '''
    return ''.join(string.uppercase[i-1] for i in s)

_memo_score = {}
def score(tricks_bid, tricks_made, vul):
    k = tricks_bid, tricks_made, vul
    try: return _memo_score[k]
    except KeyError: pass
    if tricks_bid > tricks_made:
        s = -50 * (tricks_bid - tricks_made)
        if vul: s *= 2
    elif tricks_bid >= 9:
        s = 400 + 30 * (tricks_made - 9)
        if vul: s += 200
    else:
        s = 90 + 30 * (tricks_made - 7)
    _memo_score[k] = s
    return s

_memo_score_hps = {}
def score_hps(n, s, tricks_bid, vul):
    k = n, s, tricks_bid, vul
    try: return _memo_score_hps[k]
    except KeyError: pass
    total = 0
    tm_dict = dres[n, s]
    for tricks_made in tm_dict:
        sc = score(tricks_bid, tricks_made, vul)
        total += sc * tm_dict[tricks_made]
    result = _memo_score_hps[k] = total, sum(tm_dict.itervalues())
    return result

_memo_boards_rm_hps = {}
def boards_rm_hps(n, s, tricks_bid):
    k = n, s, tricks_bid
    try: return _memo_boards_rm_hps[k]
    except KeyError: pass
    ties = 0
    tm_dict = dres[n, s]
    if tricks_bid == 7:
        ties = sum(tm_dict[tm] for tm in tm_dict if tm<9)
    elif tricks_bid == 8:
        ties = tm_dict.get(8, 0)
    else:
        ties = sum(tm_dict[tm] for tm in tm_dict if tm>=9)
    result = _memo_boards_rm_hps[k] = ties, sum(tm_dict.itervalues())
    return result

def tricks_bid_strat(n, s, strat):
    L, H, T = strat
    if s >= H:
        tricks_bid = 9
    elif s < L:
        tricks_bid = 7
    elif n < T:
        tricks_bid = 8
    else:
        tricks_bid = 9
    return tricks_bid

def score_hps_strat(n, s, strat, vul):
    return score_hps(n, s, tricks_bid_strat(n, s, strat), vul)

def boards_rm_hps_strat(n, s, strat):
    return boards_rm_hps(n, s, tricks_bid_strat(n, s, strat))

def score_strat(strat, vul, zero=0):
    grand_total, number_deals = zero, zero
    for n in range(15, 18):
        for s in range(8, 11):
            total, deals = score_hps_strat(n, s, strat, vul)
            grand_total += total
            number_deals += deals
    return grand_total/number_deals

def boards_rm_strat(strat, oof=1000):
    total_ties, number_deals = 0, 0
    for n in range(15, 18):
        for s in range(8, 11):
            ties, deals = boards_rm_hps_strat(n, s, strat)
            total_ties += ties
            number_deals += deals
    return (total_ties*oof)//number_deals

def doscoresdet():
    print 'TP expectations for all strategies'

    print '%3.3s' % ' ',
    for v in ('NV', 'V'):
        print '%6.6s' % v,
    print

    for st in strats:
        print s2n(st),
        for v in ('NV', 'V'):
            print '%6.2f' % score_strat(st, v=='V', 0.0),
        print
    print


def doscores():
    print 'TP expectations for all strategies'

    print '%2.2s' % ' ',
    for st in strats:
        print s2n(st),
    print

    for v in ('NV', 'V'):
        print '%2.2s' % v,
        for st in strats:
            print score_strat(st, v=='V'),
        print
    print

def doboards_rm():
    print '#boards/1000 tied against RM, all strategies'

    print '%2.2s' % ' ',
    for st in strats:
        print s2n(st),
    print

    print '%2.2s' % '',
    for st in strats:
        print boards_rm_strat(st),
    print
    print


def onestrat(n):
    strat = n2s(n)
    print 'TP expectations for strategy %s %s:' % (n, strat)

    print '%2.2s' % ' ',
    for n in range(15, 18):
        for s in range(8, 11):
            print '%2.2d-%2.2d' % (n, s),
    print

    for v in ('NV', 'V'):
        print '%2.2s' % v,
        for n in range(15, 18):
            for s in range(8, 11):
                total, deals = score_hps_strat(n, s, strat, v=='V')
                ep = float(total)/deals
                print '%5.1f' % ep,
        print
    print

def mp_match(strat1, strat2, ities=False, oof=400):

    net, tot = 0, 0
    for n in range(15, 18):
        for s in range(8, 11):
            tm_dict = dres[n, s]
            t1 = tricks_bid_strat(n, s, strat1)            
            t2 = tricks_bid_strat(n, s, strat2)            
            if t1==t2:
                if not ities: tot += sum(tm_dict.itervalues())
                continue

            for tricks_made in tm_dict:
                sc1 = score(t1, tricks_made, None)
                sc2 = score(t2, tricks_made, None)
                net += cmp(sc1, sc2) * tm_dict[tricks_made]
                if not ities or sc1 != sc2: tot += tm_dict[tricks_made]

    result = cmp(net,0) * ( (abs(net)*oof) // tot )

    print '%+3d' % result,

def all_mp_matches():
    print 'MP matches between all strategies, scores in boards/400'

    print '%4.4s' % ' ',
    for st in strats:
        print s2n(st),
    print
    for s1 in strats:
        print '%4.4s' % s2n(s1),
        for s2 in strats:
            mp_match(s1, s2)
        print
    print

def stratcomp(n1, n2):
    strat1 = n2s(n1)
    strat2 = n2s(n2)
    print 'TP comparisons for %s vs %s:' % (n1, n2)

    for v in ('NV', 'V'):
        print 'At %2.2s' % v
        vul = v=='V'
        for n in range(15, 18):
            for s in range(8, 11):
                t1 = tricks_bid_strat(n, s, strat1)            
                t2 = tricks_bid_strat(n, s, strat2)            
                if t1==t2: continue
                print '%2.2d-%2.2d, bid tr %d vs %d:' % (n, s, t1, t2)
                print '',
                tm_dict = dres[n, s]
                res = []
                for tricks_made in sorted(tm_dict):
                    sc1 = score(t1, tricks_made, vul)
                    sc2 = score(t2, tricks_made, vul)
                    res.append((sc1-sc2, tm_dict[tricks_made]))
                deltot = 0
                nh = 0
                i = 0
                while i<len(res):
                    j = i+1
                    while j<len(res) and res[j][0]==res[i][0]: j+=1
                    sep = ' +'
                    if j>=len(res): sep = ''
                    if j==i+1:
                        print '%d*(%+d)%s'%(res[i][1], res[i][0], sep),
                        nc = res[i][1]
                    else:
                        srt = [res[k][1] for k in range(i,j)]
                        tvd = '(%s=%d)' % (
                                '+'.join(str(x) for x in srt),
                                sum(srt))
                        print '%s*(%+d)%s'%(tvd, res[i][0], sep),
                        nc = sum(srt)
                    deltot += nc * res[i][0]
                    nh += nc
                    i = j
                print
                print '  --> %+d (%+.1f on %d deals)' % (deltot,
                        float(deltot)/nh, nh)

def main():
    global dres
    f = open('pick_dds.pick', 'rb')
    i = cPickle.load(f)
    timetot = cPickle.load(f)
    dres = cPickle.load(f)

    drescomm.report(i, timetot, dres)
    print
    # doscores()
    doboards_rm()
    # onestrat('IIO')
    # onestrat('IJP')
    # stratcomp('IIO', 'IJP')
    # doscoresdet()
    all_mp_matches()

main()
