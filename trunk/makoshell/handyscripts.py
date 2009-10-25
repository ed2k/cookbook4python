
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
def xmlBeautifier(data):
    #data = open(sys.argv[1],'r').read()

    fields = re.split('(<.*?>)',data)
    level = 0
    space = 1
    rText = []
    for f in fields:
       if f.strip() == '': pass
       elif f[:4] == '<!--' and f[-3:] == '-->':
           rText.append( ' '*(level*space) + f) 
       elif f[:2]=='<?': 
           rText.append( ' '*(level*space) + f) 
       elif f[0]=='<' and f[1] != '/':
           rText.append( ' '*(level*space) + f)
           level = level + 1
           if f[-2:] == '/>':
               level = level - 1
       elif f[:2]=='</':
           level = level - 1
           rText.append( ' '*(level*space) + f)
       else:
           f = f.lstrip()
           f = f.rstrip()
           rText.append( ' '*(level*space) + f)

    return '\n'.join(rText)


    # xml simple beautifier
def htmlBeautifier(data):
    #data = open(sys.argv[1],'r').read()
    fields = re.split('(<.*?>)',data)
    level = 0
    space = 1
    rText = []
    for f in fields:
       if f.strip() == '': pass
       elif f[:5] == '<link' or f[:4] == '<img' :
           rText.append( ' '*(level*space) + f) 
       elif f[:4] == '<!--' and f[-3:] == '-->':
           rText.append( ' '*(level*space) + f) 
       elif f[:2]=='<?' or f[:6] == '<input': 
           rText.append( ' '*(level*space) + f) 
       elif f[0]=='<' and f[1] != '/':
           rText.append( ' '*(level*space) + f)
           level = level + 1
           if f[-2:] == '/>':
               level = level - 1
       elif f[:2]=='</':
           level = level - 1
           rText.append( ' '*(level*space) + f)
       else:
           f = f.lstrip()
           f = f.rstrip()
           rText.append( ' '*(level*space) + f)

    return '\n'.join(rText)


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
            lines.append(' '*5+', '.join(line)+',')
            line = []
        line.append('0x'+b)
        i += 1
    if i > 1:
        lines.append(' '*5+', '.join(line))
    return '\n'.join(lines)

def parseOWAPage(data):
    #owa8.1.375.2 fold link := class="fld["| bld"| sl"]
    #href="?ae=Folder&t=IPF.Note&id=[^"]+"                  
    #message link := name="chkmsg" value="[^"]+"  
    r = []
    h = 'class="fld'
    fields = re.findall(h+'.+?id=[^"]+"',data,re.DOTALL)
    for f in fields:
        m = re.match('(.*?)".+?href="\?ae=Folder&t=IPF.Note&id=(.+)',f[len(h):-1],re.DOTALL)
        #if m.group(1) == ' sl':
        r.append(m.group(1)+' '+m.group(2))    
    h = 'name="chkmsg" value="'
    fields = re.findall(h+'[^"]+"',data)
    for f in fields:
        r.append(f[len(h):-1])
    return '\n'.join(r)
selectAndReplace(win,parseOWAPage)
