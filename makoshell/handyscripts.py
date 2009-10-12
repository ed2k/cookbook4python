#---------------------------------------------------
#                   UliPad script
# Author  :limodou
# Date    :2004/07/11
# Version :1.0
# Description:
#     Add linenum to each line of selected text
#
#---------------------------------------------------

def run(win):
    linenums = win.document.getSelectionLines()
    win.document.BeginUndoAction()
    for i, linenum in enumerate(linenums):
        text = str(i+1).ljust(6) + win.document.getLineText(linenum)
        win.document.replaceLineText(linenum, text)
    win.document.EndUndoAction()

#run(win)

import sys
import re

# xml simple beautifier
def cccc():

    #data = open(sys.argv[1],'r').read()
    data = win.document.GetSelectedText()
    #linenums = win.document.getSelectionLines()
    #data = ''
    #for i, linenum in enumerate(linenums):
    #     data +=  win.document.getLineText(linenum)

    fields = re.split('(<.*?>)',data)
    level = 0
    space = 4
    rText = []
    for f in fields:
       if f.strip() == '': continue
       if f[0:2]=='<?': 
           rText.append( ' '*(level*space) + f) 
           continue
       if f[0]=='<' and f[1] != '/':
           rText.append( ' '*(level*space) + f)
           level = level + 1
           if f[-2:] == '/>':
               level = level - 1
       elif f[:2]=='</':
           level = level - 1
           rText.append( ' '*(level*space) + f)
       else:
           rText.append( ' '*(level*space) + f)

    win.document.BeginUndoAction()
    win.document.ReplaceSelection('\n'.join(rText))
    win.document.EndUndoAction()
    

# trekbuddy waypoint converter
def dddd():
    #data = open(sys.argv[1],'r').read()
    data = win.document.GetSelectedText()
    r = []
    for line in data.splitlines():
        f=line.split()
        lat = f[0][1:]+str(float(f[1])/60)[1:]
        lot = '-'+f[2][1:]+str(float(f[3])/60)[1:]
        name = ''.join(f[5:])
        r.append('<wpt lat="'+lat+'" lon="'+lot+'">')
        r.append('<name>'+name+'</name></wpt>')

    win.document.BeginUndoAction()
    win.document.ReplaceSelection('\n'.join(r))
    win.document.EndUndoAction()

def selectAndReplace(win,func):
    data = win.document.GetSelectedText()
    r = func(data)
    win.document.BeginUndoAction()
    win.document.ReplaceSelection(r)
    win.document.EndUndoAction()
    
#cccc()
# input 
#e0 10 02 01 6b 1e 28 1c 06 07 00 11  bUH.....k.(.....
#0010   86 05 01 01 01 a0 11 60 0f 80 02 07 80 a1 09 06  .......`........
#output 0xe0, 0x10, 0x02... , 0x1c 8 bytes per line
#       0x06 ... 0x01
#       0x1 ... 
def bytes16To8hex(data):
    lines = data.splitlines()
    r = lines[0].split('  ')[0].split()
    if len(lines)>1:
        for line in lines[1:]:
            r += line[7:].split('  ')[0].split()

    i = 1
    line = []
    lines = []
    for b in r:
        if i > 8: 
            i = 1
            lines.append(', '.join(line))
            line = []
        line.append('0x'+b)
        i += 1
    if i > 1:
        lines.append(', '.join(line))
    return '\n'.join(lines)

selectAndReplace(win,bytes16To8hex)
