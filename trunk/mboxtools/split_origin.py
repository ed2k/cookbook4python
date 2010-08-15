#!/usr/bin/env python
"""assume no multipart, 
"""

import email
from email.MIMEText import MIMEText
import sys, os, string, re

LF = '\x0a'

def main ():
    mailboxname_in = sys.argv[1]


    # Open the mailbox.
    mbstr = file(mailboxname_in,'r').read();
    mails = mbstr.split("\nFrom -");

    for idx, mail in enumerate(mails):
        #take out the first line
        i = mail.find("\n");
        mails[idx] =email.message_from_string( mail[i+1:]);
    print 'read in', len(mails)
    
    for msg in mails:
        parts = msg.get_payload().split('-----Original Message-----\n')
        if len(parts) == 1: continue;
        #print len(parts)
        msg.set_payload(parts[0])
        for part in parts[1:]:
            m = re.sub(r'\n>[ \t]*','\n','\n'+part);
            # take out first '\n'
            m = m[1:]
            checks = m.split('\n\n')
            if len(checks) < 2:
                m = m.replace('=20','');
                m = re.sub(r'\n[ \t]+\n','\n\n',m)
                checks = m.split('\n\n')
                if len(checks) < 2:
                    print '-check1-', m[0:100]
                    mails.append(email.message_from_string(m))
                    continue

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
            if mm['Subject'] is None:
                print '-check2-',mm.as_string()[0:100]

            mails.append(mm);
            

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

