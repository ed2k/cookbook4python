#!/usr/bin/python

print "Content-type: text/plain\n\n"
import sys
import os
import cgi
import cgitb; cgitb.enable()

form = cgi.FieldStorage()
arg = form.getvalue('arg','')
#print 'arg:',arg
#print os.popen('ls -l').read()
print os.popen('./dds '+arg).read()
