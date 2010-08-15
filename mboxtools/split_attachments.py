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

def main ():
    mailboxname_in = sys.argv[1]
    attach_dir = '/p/g/'


    # Open the mailbox.
    mbstr = file(mailboxname_in,'r').read();
    mbstr = mbstr.replace('\r\n','\n');
    mails = mbstr.split("\nFrom -");


    for idx, mail in enumerate(mails):
        #take out the first line
        i = mail.find("\n");
        mails[idx] =email.message_from_string( mail[i+1:]);
    print 'read in', len(mails)
    
    for idx, msg in enumerate(mails):
        parts = msg.get_payload();
        #print "-", idx, msg.is_multipart(), len(parts), msg['Date']
        if not msg.is_multipart():
            continue
        # clea up multipart
        if (parts[0].get_content_maintype() == 'text' and len(parts) == 1) or (not parts[0].is_multipart()):      
            m = MIMEText(parts[0].get_payload())
            m['From'] = msg['From']
            m['To'] = msg['To']
            m['Date'] = msg['Date']
            m['Subject'] = msg['Subject']

            mails[idx] = m;
            continue
        if len(parts) > 1:
            # split attachments
            for jdx, subpart in enumerate(parts[1:]):
                filename = subpart.get_filename()
                if (not filename) or (filename == 'Untitled Attachment'):
                    if subpart.get_content_type()!= 'message/rfc822':
                        #text/x-moz-deleted this type is OK to ommit
                        print 'TODO3 unknown type', subpart.get_content_type()
                        continue
                    #add new mail, has to be one part in payload 
                    m = subpart.get_payload();
                    if len(m) != 1:
                        print "error"
                        continue
                    mails.append(m[0])
                    continue
                #print 'TODO att',jdx, subpart.get_filename()
                filename = attach_dir + filename
                index=0
                while os.path.exists(filename):
                    index += 1
                    filename = filename +'.'+ str(index)
                fp = open (filename, "w")
		fp.write(subpart.get_payload(decode=1))
		fp.close()

        #keep the first text part 
        subparts =  parts[0].get_payload()
        if len(subparts)!=2 or subparts[0].get_content_type()!='text/plain' or subparts[1].get_content_type()!='text/html':
            print 'TODO1', idx, len(subparts), parts[0].is_multipart()
           # print 'test multipart',parts[0].keys, len(parts)
           # print msg.keys()
            continue
        #This message is in MIME format. Since your mail reader does not understand
        #print 'special case due to outlook server export to thunderbird'
        m = MIMEText(subparts[0].get_payload())

        m['From'] = msg['From']
        m['To'] = msg['To']
        m['Date'] = msg['Date']
        m['Subject'] = msg['Subject']
        
        mails[idx] = m;
        #for jdx,part in enumerate(msg.walk()):
            #    print '- - ',jdx, part.get_content_maintype(), part

    fout = file('test', 'w')
    for idx, msg in enumerate(mails):
        #parts = msg.get_payload();
        #print "-", idx, msg.is_multipart(), len(parts), msg['Subject']
        fout.write ('From -\n')
        str = msg.as_string()
        fout.write(str)
        if str[len(str)-1] != '\n':
            fout.write('\n')
    fout.close()
    print len(mails) 
 
    

if __name__ == '__main__':
    main ()

