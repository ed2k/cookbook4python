import sys, os, string, re

def tag(name,value):
    return ''.join(['<',name,'="',value,'">'])
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
        print ' '*depth+tag('name',dir)
        size = os.path.getsize(path)
        total_size += size
        print ' '*depth+tag('size',str(size))
        print ' '*depth+tag('ltype','111')
        print ' '*depth+tag('crdate','09/10/2008 4:56:38 PM')
        print ' '*depth+tag('moddate','09/10/2008 4:56:38 PM')
        print ' '*depth+tag('accdate','09/10/2008 4:56:38 PM')
        print ' '*depth+'</>'
    elif os.path.isdir(path):
        total_nodes += 1
        print ' '*depth+'<node> //',dir
        print ' '*depth+tag('nname',' '+dir)
        print ' '*depth+tag('crdate','09/10/2008 4:56:38 PM')
        print ' '*depth+tag('moddate','09/10/2008 4:56:38 PM')
        print ' '*depth+tag('accdate','09/10/2008 4:56:38 PM')  
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
        print ' '*depth+tag('nsize',str(total_size))                
        print ' '*depth+'</node> //',dir,'Num.Leaves :',sub_leaves,'Num.Subnodes :',sub_nodes
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
    tn, tl, sz = walk(head,tail,0)
 
    #print '</end> Total number of Nodes :',tn,'Number of leaves :',tl
    return


#if __name__ == '__main__':
main()

# Squarified treemap algorithm
# rectangle, list of number sorted, big to small
# width, height 