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
def fix_email(line):
    ''' fix '[' to '<' so that both gmail and thunderbird can read'''
    from string import maketrans
    return line.translate(maketrans('[]','<>'))
def fix_header2(msg):
    headers = msg.splitlines()
    # first line is from
    mfirst = 'From - '
    mfrom = 'From: '+ fix_email(headers[0].lstrip())
    mto = ''
    mdate = ''
    msubject = ''
    r = []
    for i,line in enumerate(headers[1:]):
        f = line.split()
        if len(f) > 0 :
          if f[0] in ['To:']: mto = fix_email(' '.join(f))
          elif f[0] in ['Subject:']: msubject = ' '.join(f)
          elif f[0] in ['From:']: mfrom = fix_email(' '.join(f))
          elif f[0] in ['Sent:','Date:']:
            mdate = 'Date: '+line[5:].lstrip()
            mfirst += line[5:].lstrip()
          else: r.append(line)
        else: r.append(line)
    return '\n'.join([mfirst,mdate,mfrom,mto,msubject]+r)

import time
def conv_date_format(str,format='ctime'):
    if format == 'ctime':
        return time.strftime('%a, %d %b %Y %H:%M:%S',time.gmtime(str))       
    f = str.split('.')
    if len(f) == 1:
        t = str.split()    
        if len(t)==2:
            d = time.strptime(str,'%B %Y')
            return time.strftime('%b %Y',d)            
        else:
            d = time.strptime(str,'%d %B %Y')
            return time.strftime('%d %b %Y',d)                
        
    fstr = f[1]
    if len(f) == 3:
        t = f[1].split('-')
        fstr += '.'+t[0]
    d = time.strptime(fstr,'%Y-%m-%d.%H%M%S')
    return time.strftime('%a, %d %b %Y %H:%M:%S',d)            

def conv_imdir2mbox(rootdir):
    mails = []
    for dirpath in os.listdir(rootdir):
        print dirpath 
        path1 = os.path.join(rootdir,dirpath)       
        for d in os.listdir(path1):  
            path2 = os.path.join(path1,d)
            for f in os.listdir(path2):          
                date = conv_date_format(f)
                print dirpath,d,date            
                mfirst = 'From - '
                mfrom = 'From: '+ d
                mto = 'To: '+dirpath
                mdate = 'Date: '+date
                msubject = 'Subject: IM '+f
                contents = file(os.path.join(path2,f)).read()
                mails.append('\n'.join([mfirst,mdate,mfrom,mto,msubject])+'\n\n'+contents)
    return mails

def conv_datefrom2mbox(rootdir):
    mails = []
    for dirpath in os.listdir(rootdir):
        print dirpath 
        path1 = os.path.join(rootdir,dirpath)       
        for d in os.listdir(path1):        
            date = conv_date_format(dirpath)
            print dirpath,d,date            
            mfirst = 'From - '
            mfrom = 'From: '+ d
            mto = 'To: sunyin'
            mdate = 'Date: '+date
            msubject = 'Subject: IM '+d+ ' ' + dirpath
            contents = file(os.path.join(path1,d)).read()
            mails.append('\n'.join([mfirst,mdate,mfrom,mto,msubject])+'\n\n'+contents)
    return mails
def xmlformatter(data):
    r = []
    fields = re.split('(<.*?>)',data)
    spaces = 4
    level = 0
    for f in fields:
        if string.strip(f)=='': continue
        if f[0:2] == '<?':
            r.append( ' '*(level*spaces) + f       )
        elif f[0]=='<' and f[1] != '/':
            r.append( ' '*(level*spaces) + f)
            level = level + 1
            if f[-2] == '/':
            	level = level - 1        	
        elif f[:2]=='</':
            level = level - 1
            r.append( ' '*(level*spaces) + f)
        else:
            r.append( ' '*(level*spaces) + f   )
    return '\n'.join(r)
def conv_xml2mbox(rootdir):
    mails = []
    for dirpath in os.listdir(rootdir):
        path1 = os.path.join(rootdir,dirpath)       

        date = conv_date_format(os.path.getmtime(path1),format='ctime')
        print dirpath,date            
        mfirst = 'From - '
        mfrom = 'From: '+ dirpath[:-4]
        mto = 'To: sunyin'
        mdate = 'Date: '+date
        msubject = 'Subject: IM '+ dirpath
        contents = xmlformatter(file(path1).read())
        mails.append('\n'.join([mfirst,mdate,mfrom,mto,msubject])+'\n\n'+contents)
    return mails


def main ():
    mailboxname_in = sys.argv[1]
    if os.path.isdir(mailboxname_in):
        # if dirpath name is date, use date/from format
        # else use to/from/date.txt        
        d1 = os.listdir(mailboxname_in)[0]
        p1 = os.path.join(mailboxname_in,d1)
        if os.path.isdir(p1):
            p2 = os.path.join(p1,os.listdir(p1)[0])
            if os.path.isdir(p2):
                mails = conv_imdir2mbox(mailboxname_in)
            else:
                mails = conv_datefrom2mbox(mailboxname_in)
        else:
            mails = conv_xml2mbox(mailboxname_in)
    else:
        # Open the mailbox.
        mbstr = file(mailboxname_in,'r').read();
        #   mbstr = mbstr.replace('\r\n','\n');
        # can not use "^From: " as it might be "^From:\t"
        mails = mbstr.split("\nFrom:");
        print 'read in', len(mails)

        # take out From: to be consistent
        if len(mails) > 0 : mails[0] = mails[0][6:]
        # assume no multipart, otherwise run split_attachements first
        # try to utilize email library to extract header
        #for idx, mail in enumerate(mails):
        #    mails[idx] =email.message_from_string( 'From: '+mail);
        ##        if mails[idx].is_multipart():

    # write to new mbox file
    fout = file('test', 'w')
    for idx, msg in enumerate(mails):
        str = fix_header2(msg)
        fout.write(str)
        if str[len(str)-1] != '\n':
            fout.write('\n')
    fout.close()

    return


# compare date for list of emails
def cmp_date(a,b):
    return

if __name__ == '__main__':
    main ()

