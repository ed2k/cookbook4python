import sys, os

def extmap(fname):
    a,ext = os.path.splitext(fname)

    ext = ext.upper()
    if len(ext)<2:
        return '100'
    ext = ext[1:]

    if ext in ['TXT','CSV','C','PY','LOG']: return '200'
    if ext in ['PDF','DOC','DOCX','XLS']: return '180'
    if ext in ['ISO','ERL','CAB','MDB']: return '-120'  
    if ext in ['EXE','JAR','OBJ','SYS','CLASS','DLL']: return '-200'
    if ext in ['AVI','MOV','MPEG','MKV','RMVB','RM','WMV','FLV','3GP']: return '140'
    if ext in ['MP3','JPG','IMG','PNG']: return '160'
    if ext in ['GZ','ARJ','ZIP']: return '-110'
    if ext in ['TAR','PV','vmdk']: return '-100'
    return '0'

def tr(name): return name.replace('&','&amp;')
def tag(name,value):
    return ''.join(['<',name,'>',value,'</',name,'>'])
def walk(root,dir,depth):
    path = os.path.join(root,dir)
    sub_leaves = 0
    sub_nodes = 0
    total_leaves = 0
    total_nodes = 0
    total_size = 0
    if os.path.isfile(path):
        total_leaves += 1
        print ' '*depth+'<leaf>'
        print ' '*depth+tag('label',tr(dir))
        size = os.path.getsize(path)
        total_size += size
        print ' '*depth+tag('weight',str(size))
        print ' '*depth+tag('value',extmap(dir))
        print ' '*depth+'</leaf>'
    elif os.path.isdir(path):
        total_nodes += 1
        print ' '*depth+'<branch>'
        print ' '*depth+tag('label',tr(dir))
        # list files first, dirs next      
        for p, dirpaths, fnames in os.walk(path):
          for d in fnames:
            tnodes,tleaves,sz = walk(path,d,depth+1)
            total_nodes += tnodes
            total_leaves += tleaves
            total_size += sz
          for d in dirpaths:
            tnodes,tleaves,sz = walk(path,d,depth+1)            
            total_nodes += tnodes
            total_leaves += tleaves
            total_size += sz
          sub_nodes = len(dirpaths)
          sub_leaves = len(fnames)
          break
        #print ' '*depth+tag('value',str(total_size))                
        print ' '*depth+'</branch>'
    return total_nodes,total_leaves, total_size
def check_output(f):
    depth = 0
    for line in file(f):
        line = line.strip()
        if line.find('<node') == 0:
            print depth,depth*' '+line
            depth += 1
        elif line.find('</node') == 0: 
            depth -= 1
            print depth,depth*' '+line
        elif line.find('< nsize')==0:
            print depth,depth*' '+line
        #elif line.find('<name=') == 0: print depth*' '+line
def main ():
    root_dir = sys.argv[1]
    
    if os.path.isfile(root_dir):
        check_output(root_dir)
        return
    if root_dir[-1] == '\\': root_dir = root_dir[:-1]
    if root_dir[-1] == '/': root_dir = root_dir[:-1]
    head, tail = os.path.split(root_dir)
    print '''<?xml version='1.0' encoding='ISO-8859-1'?>
<!DOCTYPE root SYSTEM "TreeMap.dtd" >
<root>
  <label>Root</label>
'''
    tn, tl, sz = walk(head,tail,0)
    print '</root>'
    #print '</end> Total number of Nodes :',tn,'Number of leaves :',tl
    return


#if __name__ == '__main__':
main()

# Squarified treemap algorithm
# rectangle, list of number sorted, big to small
# width, height 