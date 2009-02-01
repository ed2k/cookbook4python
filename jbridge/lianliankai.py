import os,sys
DeltaMove = [[1,0],[0,1],[-1,0],[0,-1]]

# origina is n by m matrix
#data used in search, pos, direction, num of turns, marker board (remeber visited pos)
def find(origin,originRow, originCol):
    rlen = len(origin)
    clen = len(origin[0])

    marker = []
    marker.append([-1]*(clen+4))
    marker.append([-1]+[0]*(clen+2)+[-1])
    for r in origin: marker.append([-1,0]+r+[0,-1])
    marker.append([-1]+[0]*(clen+2)+[-1])
    marker.append([-1]*(clen+4))
    movementQueue = {}
    movementQueue[(originRow,originCol)] = ([0,0],-1)

    while len(movementQueue) > 0:
        row, col = movementQueue.keys()[0]
        direction, nTurns = movementQueue.pop((row,col))      

        for d in DeltaMove:
            dx,dy = d
            t = nTurns
            if direction != d: t += 1
            if t > 2: continue
            nr = row+dy; nc = col+dx
            if [nr,nc] == [originRow,originCol]: continue
            v = marker[nr+2][nc+2] 
            if v == marker[originRow+2][originCol+2] :
                return [nr,nc]
            if v != 0: continue
            #print [nr,nc,d, t]
            if (nr,nc) in movementQueue.keys():
                p, oldTurn = movementQueue[(nr,nc)]
                if oldTurn > t:
                    movementQueue.pop((nr,nc))
                    movementQueue[(nr,nc)] = (d,t)
                elif oldTurn == t:
                    movementQueue[(nr,nc)] = (d,t)
            else:
                movementQueue[(nr,nc)] = (d,t)
            
        
board = [[0, 0, 0, 4, 18, 37, 32, 25, 22, 36, 6, 7, 37],
[13, 23, 0, 3, 14, 1, 13, 29, 32, 16, 20, 11, 10],
[17, 1, 39, 36, 11, 22, 0, 15, 30, 40, 35, 1, 13],
[5, 10, 16, 5, 1, 20, 0, 16, 36, 17, 7, 29, 4],
[0, 25, 1, 4, 10, 32, 17, 29, 14, 27, 16, 17, 13],
[7, 41, 14, 11, 15, 4, 6, 33, 27, 29, 0, 7, 35],
[3, 40, 0, 0, 10, 6, 3, 2, 41, 37, 0, 30, 14],
[27, 29, 33, 13, 11, 37, 23, 4, 20, 11, 23, 14, 39],
[5, 10, 29, 3, 23, 7, 1, 22, 2, 35, 36, 6, 3],
[0, 4, 35, 5, 0, 14, 20, 32, 13, 27, 3, 22, 32]
]
print find(board,3,0)
#sys.exit()

def findOne(board):
    for r in xrange(len(board)):
        for c in xrange(len(board[0])):
            if board[r][c] == 0: continue
            pos = find(board,r,c)
            if pos is not None:
                return [[r,c],pos]

import socket,time

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("localhost", 4444))
s.send("calibrate\n")
while True:
    r =  s.recv(1024)
    #print r
    a = []
    for line in r.split(',')[:-1]:
        a.append([int(i) for i in line.split()])
                  
    result =  findOne(a)
    if result is not None:
        p1,p2 = result
        print a[p1[0]][p1[1]],p1,a[p2[0]][p2[1]],p2
        #s.send('mark '+' '.join(p1+p2)+'\n')
        
        for x in xrange(10):
            for y in xrange(14):
                if a[x][y] != 0:
                    a[x][y] = 1           
        a[p1[0]][p1[1]] = 88
        a[p2[0]][p2[1]] = 88    
    for r in a:
        print '%2d,'*14 % tuple(r)    
    time.sleep(2)
    s.send("update\n")

