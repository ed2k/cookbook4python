#!/usr/bin/env python
"""take From - seperated mails into python email format should
be close in mbox format that imapupload can easily upload to gmail
python imap_upload.py --gmail --box=<gmail label> --time-fields=date mboxfile
"""

import email
from email.MIMEText import MIMEText
import mailbox
import sys, os, string, re, time
from cnv2_mba2mbx import *


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
    #mbx = mailbox.mbox("tmp")
    # Open the mailbox.
    mbstr = file(mailboxname_in,'r').read();
    # 7 is to make the first message consisten to the rest
    mails = mbstr[7:].split("\nFrom - ");
    # assume no multipart, otherwise run split_attachements first
    for idx, mail in enumerate(mails):
        #take out the char \r, \n becomes the first char
        m = email.message_from_string( mail[1:]);
        if m.is_multipart():
            print  "ERROR, run mail split first"
            break
        if 'Date' not in m.keys():
            print idx,
            print [mail[:8]],m['subject']
            break
        try:
            t = time.strptime(m['Date'],"%m/%d/%Y %I:%M:%S %p")
        except:
            t = time.strptime(m['Date'],"%m/%d/%Y")
        m.replace_header('Date',time.strftime('%a, %d %b %Y %H:%M:%S',t))
        #mbx.add(m)
        mails[idx] = m
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
    # write to new mbox file for gmail
    fout = file('tmp', 'w')
    for idx, msg in enumerate(mails):
        for k in ['To','From','CC','BCC']:
            if not msg.has_key(k): continue
            msg.replace_header(k, fix_email(msg[k]))

        m = msg.as_string()
        #print [m[:100]]
        #i = m.find('\n\n')
        #str = fix_header2(m[:i])
        str = m
        #print [str[:100]]
        fout.write('From - \n')
        fout.write(str)
        if str[len(str)-1] != '\n':
            fout.write('\n\n')
        else: fout.write('\n')
        #fout.write(m[i+2:])
        if m[-1] != '\n': fout.write('\n\n')
        elif m[-2] != '\n': fout.write('\n')
        
    fout.close() 
    return


def create_same_subject_list(mails):
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
    
#   print len(mails) 
 
    

if __name__ == '__main__':
    main ()

