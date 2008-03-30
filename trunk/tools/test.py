#!/usr/bin/python

print "Content-type: text/html\n\n"
import sys
#print sys.version

#for m in sys.modules:
#  print m

import cgi
import cgitb; cgitb.enable()
#cgi.print_environ()
#cgi.print_directory()
#cgi.print_environ_usage()
form = cgi.FieldStorage()
notes = form.getvalue('src','')
if notes == '': notes = open('notes.txt','r').read()
else: open('notes.txt','w').write(notes)
print '''<form action="test.py" method="post">
<textarea rows="5" cols="120" name="src">''' + notes + '''
</textarea><input type="submit" value="postit"></input></form>
'''
import os
cmd = 'which tclsh'
r = os.popen(cmd).read()
print '''<textarea rows="15" cols="120" >''' + r + '''
</textarea>
'''
