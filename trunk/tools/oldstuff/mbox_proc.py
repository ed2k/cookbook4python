#!/usr/bin/env python
"""This is an mbox filter. It scans through an entire mbox style mailbox
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
def same_subject(s1, s2):
    #TODO use sequence matcher to catalog similar subject
    # such as ignore tab, space etc.
    if len(s1) < len(s2): s = s1; s1 = s2
    elif len(s1) == len(s2): return s1 == s2

    prefix = s1[0:3].upper()
    return (prefix =='RE: ' or prefix=='FW: ') and s == s1[4:]
    
def main ():
    mailboxname_in = sys.argv[1]
#    attach_dir = '/p/g/'


    # Open the mailbox.
    mbstr = file(mailboxname_in,'r').read();
 #   mbstr = mbstr.replace('\r\n','\n');
    mails = mbstr.split("\nFrom -");

# assume no multipart, otherwise run split_attachements first
    for idx, mail in enumerate(mails):
        #take out the first line
        i = mail.find("\n");
        mails[idx] =email.message_from_string( mail[i+1:]);
        if mails[idx].is_multipart():
            print  "ERROR, run mail split first"
            break
    print 'read in', len(mails)

    # fix some no subject emails
    for idx, msg in enumerate(mails):
        sub = msg['Subject'];
        if sub is None: sub = ''
        if len(sub) > 5:
            sub = sub.replace('\n','')
            sub = sub.replace('\t','')
            sub = sub.replace('  ',' ')
            msg.replace_header('Subject',sub)
            continue
        
        #makeup subject
        body = msg.get_payload();
        end_pos = body.find('\.')
        if end_pos == -1:
            if len(body) > 25: sub = body[0:24]
            else: sub = body
        else: sub = body[0:end_pos]
        sub = sub.replace('\n','')
        sub = sub.replace('\t','')
        sub = sub.replace('  ',' ')
        if msg['Subject'] is None: msg['Subject'] = sub
        else: msg.replace_header('Subject',sub)

    # create same subject list
    ss = []
    for idx, msg in enumerate(mails):
        found_subject = False
        for items in ss:
            if same_subject(items[0]['Subject'], msg['Subject']):
                found_subject = True
                # don't put duplicate emails
                duplicated = False
                for jdx,item in enumerate(items):
                    body = item.get_payload();
                    b2 = msg.get_payload();
                    if len(body)>= len(b2):
                        if body.find(b2) != -1:
                            duplicated = True; break;
                    else :
                        if b2.find(body) != -1:
                            duplicated = True;
                            # replace existing one
                            items[jdx] = msg;
                            break;
                    
                if not duplicated:
                    items.append(msg)
                else: print 'found dup', msg['Subject']
                break
        if not found_subject:
            i = []; i.append(msg);
            ss.append(i)

    print 'diff subjects', len(ss)
    # write to new mbox file
    fout = file('test', 'w')
    for items in ss:
        for msg in items:
            fout.write ('From -\n')
            str = msg.as_string()
            fout.write(str)
            if str[len(str)-1] != '\n':
                fout.write('\n')
    fout.close()
#   print len(mails) 
 
    

if __name__ == '__main__':
    main ()

