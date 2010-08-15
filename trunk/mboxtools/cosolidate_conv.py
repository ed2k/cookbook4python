#!/usr/bin/env python
"""consolidate *.txt under directories
 they gaim generated conversation history
"""

import email
from email.MIMEText import MIMEText
import sys, os, string, re

LF = '\x0a'

def main ():
    fout = file('test', 'w')
    for d in os.listdir('.'):
        path = os.path.join('.',d )
        if  os.path.isfile(d): continue
        print d
        for filename in os.listdir(d):
            path = os.path.join(d, filename)
            if not os.path.isfile(path):            continue
            print filename
            fout.write ('From -\n')
            f = file(path, 'r')
            fout.write('From: '+d +'\n');
            fout.write('Subject: IM '+filename[0:len(filename)-4]+'\n\n');
            fout.write(f.read());
            fout.write('\n')
            f.close();

    fout.close()



if __name__ == '__main__':
    main ()


