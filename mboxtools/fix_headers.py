#!/usr/bin/env python
"""
This is an mbox filter. It scans through an entire mbox style mailbox
and writes the messages to a new file. Each message is passed
through a filter function that convert From to >From 

"""

import email
from email.MIMEText import MIMEText
import sys, os, string, re

LF = '\x0a'
DL = '\r\n'

def emails2mbox_file(mails, filename):
    # write to new mbox file
    fout = file(filename, 'w')
    for msg in mails:
        fout.write ('From - \n')
        str = msg
        fout.write(str)
        if str[len(str)-1] != '\n':
            fout.write('\n')
    fout.close()

def fix_header(mailstr):
    checks = mailstr.split('\n\n')
    if len(checks) < 2:
        m = mailstr.replace('=20','');
        m = re.sub(r'\n[ \t]+\n','\n\n',m)
        checks = m.split('\n\n')
        if len(checks) < 2:
            # speical fix, assume '\n> ' seperate the header
            checks = m.split('\n> ')
            header = checks[0]
            body = '\n'.join(checks[1:])
            
    # all kinds of >= 2 case, avoid using goto        
    if len(checks) >= 2:
        if re.match('\n\S+: ', '\n'+checks[1]) is None:
            header = checks[0]
            body = '\n\n'.join(checks[1:])
        else:
            header = checks[0]+'\n'+checks[1]
            body = '\n\n'.join(checks[2:])

    hs = header.splitlines();
    for idx,h in enumerate(hs):
        if re.match('^\S+:\s+', h) is None:
            hs[idx] = ' ' + h
    header = '\n'.join(hs)

    mm = email.message_from_string(header+'\n\n'+body);
    return mm

def process_one_mail(mail):
    r = mail.replace(DL+'\xa0'+DL+DL, DL*2)
    r = r.replace(DL*3, DL*2)
    idx = r.find(DL*2)
    headers = r[:idx]
    body = r[idx:]
    body = body.replace(DL+'From', DL+'>From')
    return headers+body 

def main ():
    mailboxname_in = sys.argv[1]

    # Open the mailbox.
    mbstr = file(mailboxname_in,'r').read();
    mbstr = mbstr.replace(DL*3, DL*2);
    mbstr = mbstr[len('From - '+DL):]
    mails = mbstr.split(DL+"From - "+DL);

# assume no multipart, otherwise run split_attachements first
    for idx, mail in enumerate(mails):
        #i = mail.find("\n");
        #mails[idx] =email.message_from_string( mail[i+1:]);
        mails[idx] = process_one_mail(mail) 

    print 'read in', len(mails)

    emails2mbox_file(mails,'t1')
#   print len(mails) 

# compare date for list of emails
def cmp_date(a,b):
    return

if __name__ == '__main__':
    main ()

