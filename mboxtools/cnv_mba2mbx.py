#!/usr/bin/env python
""" convert mail navigator to unix mailbox
This is an mbox filter. It scans through an entire mbox style mailbox
and writes the messages to a new file. Each message is passed
through a filter function which may modify the document or ignore it.

The passthrough_filter() example below simply prints the 'from' email
address and returns the document unchanged. After running this script
the input mailbox and output mailbox should be identical.
"""

import email
from email.MIMEText import MIMEText
import sys, os, string, re

LF = '\x0a'

def emails2mbox_file(mails, filename):
    # write to new mbox file
    fout = file(filename, 'w')
    for msg in mails:
        fout.write ('From -\n')
        str = msg.as_string()
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

def main ():
    mailboxname_in = sys.argv[1]


    # Open the mailbox.
    mbstr = file(mailboxname_in,'r').read();
 #   mbstr = mbstr.replace('\r\n','\n');
    mails = mbstr.split("\nFrom: ");
    print 'read in', len(mails)

    # take out From: to be consistent
    if len(mails) > 0 : mails[0] = mails[0][6:]
# assume no multipart, otherwise run split_attachements first
    for idx, mail in enumerate(mails):
        mails[idx] =email.message_from_string( 'From: '+mail);
##        if mails[idx].is_multipart():


    # write to new mbox file
    fout = file('test', 'w')
    for idx, msg in enumerate(mails):
        fout.write ('From -\n')
        str = msg.as_string()
        fout.write(str)
        if str[len(str)-1] != '\n':
            fout.write('\n')
    fout.close()

    return
    # common headers, From, To, Sent, Date, Cc, Subject
    # X-Mozilla-Status, X-Mozilla-Status2, Importance, Received
    # X-Mailer
    m1 = []
    m2 = []
    for idx, msg in enumerate(mails):
        if len(msg.keys()) >= 3 :
            m1.append(msg)
            continue 

        m = fix_header(msg.get_payload())

        if len(m.keys()) < 3:
            #print m.keys() , msg.keys()
            #filter out not-fixed
            m2.append(m);
        else: m1.append(m)    
        

    emails2mbox_file(m1,'t1')
    emails2mbox_file(m2,'t2')
#   print len(mails) 

# compare date for list of emails
def cmp_date(a,b):
    return

if __name__ == '__main__':
    main ()

