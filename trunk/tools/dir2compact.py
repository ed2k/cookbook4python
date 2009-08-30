import sys, os

class Node:
  def __init__(self,name):
    self.child = {}
    self.size = 0
    self.parent = None
    self.level = 0
    self.name = name


def print_node(n):
  print n.level,n.name,n.size
  if len(n.child) > 0:
     for k,v in n.child.items():
        print_node(v)
        
def walk_dir ():
    root_dir = sys.argv[1]
    lines = []
    if root_dir[-1] == '\\': root_dir = root_dir[:-1]
    if root_dir[-1] == '/': root_dir = root_dir[:-1]
    head, tail = os.path.split(root_dir)
    for fullp,dirnames,fnames in os.walk(root_dir,topdown=False):
      head,tail = os.path.split(fullp)
      sz = 0
      for f in fnames:
          p = os.path.join(fullp,f)
          if os.path.isfile(p): 
            sz += os.path.getsize(p)
      lines.append( (fullp,sz))
    sum_dir_size(lines)
    return

        
def sum_dir_size (lines):
    nodes =  []
    root_node = Node('/')
    prev = ""
    for fullp,sz in lines:
      f = line.split()
      sz = int(f[-1])
      names = fullp.split('/')
      pt = root_node 
      for n in names[1:]:
        if n not in pt.child.keys():
           pt.child[n]=Node(n)
           pt.child[n].parent = pt
           pt.child[n].level = pt.level+1
        pt.child[n].size += sz
        pt = pt.child[n]
    print_node(root_node)
    return


if __name__ == '__main__':
    walk_dir()
