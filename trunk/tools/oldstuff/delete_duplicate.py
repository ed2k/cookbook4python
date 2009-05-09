#!/usr/bin/env python
"""
under one directory find file that are duplicated
define duplicate, same contents (filename, date, create time can be diff.)
delete the one that had longer filename
""" 

import sys, os, string, re
#from path import path
import md5
import shutil

#--- Module functions

def get_hash(filename):
    """ Return md5 hash """
    f = open(filename,'rb')
    hsh = md5.new()
    while 1:
        data = f.read(2048)
        if not data: break
        hsh.update(data)
    f.close()
    return hsh.hexdigest()

curdir = sys.argv[1]

#--- Loop through files for simple size-check
sizemap = {}
for fname in os.listdir(curdir):
    f = os.path.join(curdir, fname)
    # ?ST_SIZE not defined, use 6 instead
    size = os.stat(f)[6]
    if size in sizemap:
        sizemap[size].append(f)
    else:
        sizemap[size] = [f]

#--- collect size dupes
sizedupes = {}
for size, files in sizemap.items():
    if len(files) > 1:
        sizedupes[size] = files
        
#--- Check size dupes with MD5 hash
hashmap = {}
for size, files in sizedupes.items():
    for f in files:
        md5hash = get_hash(f)
        if md5hash in hashmap:
            hashmap[md5hash].append(f)
        else:
            hashmap[md5hash] = [f]

#--- Collect real dupes
realdupes = {}
for md5hash, files in hashmap.items():
    if len(files) > 1:
        realdupes[md5hash] = files

#--- show dupes, move all to a new directory, except one 
print "Duplicates found in:", curdir
print

garbage_dir = "c:\sy\g"
for h, files in realdupes.items():
    print "#", h
    # get the minimum file name
    files.sort(key= len);
    for f in files[1:]:      
        print " mv %s to %s" % (f,garbage_dir)
        shutil.move(f,garbage_dir);
    print   
     
    


