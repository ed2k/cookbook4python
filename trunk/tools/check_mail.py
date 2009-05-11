#!/usr/bin/env python
"""This is an mbox filter. It scans through an entire mbox style mailbox
and writes the messages to a new file. Each message is passed
through a filter function which may modify the document or ignore it.

The passthrough_filter() example below simply prints the 'from' email
address and returns the document unchanged. After running this script
the input mailbox and output mailbox should be identical.
"""

import sys, os, string, re

LF = '\x0a'

def main ():
    mailboxname_in = sys.argv[1]


    # Open the mailbox.
    mbstr = file(mailboxname_in,'r').read();
    #mbstr = mbstr.replace('\r\n','\n');
    mails = mbstr.split("\nFrom -");


    print len(mails)
    
    

if __name__ == '__main__':
    main ()

