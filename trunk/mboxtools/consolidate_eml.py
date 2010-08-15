#!/usr/bin/env python
"""consolidate *.eml in one directory
"""

import email
from email.MIMEText import MIMEText
import sys, os, string, re

LF = '\x0a'

def main ():
    dir = sys.argv[1]
    mails = []
    for filename in os.listdir(dir):
        if filename[len(filename)-4:] != '.eml': continue
        path = os.path.join(dir, filename)
        if not os.path.isfile(path):            continue
        mails.append(email.message_from_file(file(path)));


    print 'read in', len(mails)


    # write to new mbox file
    fout = file('test', 'w')
    for idx, msg in enumerate(mails):
        fout.write ('From -\n')
        str = msg.as_string()
        fout.write(str)
        if str[len(str)-1] != '\n':
            fout.write('\n')
    fout.close()
    print len(mails) 
 
    

if __name__ == '__main__':
    main ()

