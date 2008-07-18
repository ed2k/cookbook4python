def fix_path(path):
    seps = path.split('/')
    buf = []
    if path[0] != '/':
        seps.insert(0,'')
    for f in seps[1:]:
        if f == '' or f == '.': continue
        if f == '..':
            if len(buf) == 0:
                continue
            del buf[-1]
        else:
            buf.append(f)
    return '/'+'/'.join(buf)

if __name__ == '__main__':
    print fix_path('/../a/b/./c/../d/u.zip')
            
            