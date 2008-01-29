
def shodres(n, s, dr):
    """ display summary of a dict of results with certain HCP values """
    h = sum(dr.itervalues())
    print '%2.2d-%2.2d (%d):' % (n, s, h),
    k = sorted(dr)
    for t in k:
        print '%d:%d' % (t, dr[t]),
    print

def report(i, timetot, dres):
    if i <= 0:
        print 'still empty (%d hands)' % i
        return
    print '%d hands in %.2f (%.2f)' % (
            i, timetot, timetot/i )
    for n in range(15,18):
        for s in range(8, 11):
            shodres(n, s, dres[n,s])
    print


