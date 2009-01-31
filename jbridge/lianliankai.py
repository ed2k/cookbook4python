import os
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
   
    movementQueue = [[originRow,originCol,[0,0],0]]
    while len(movementQueue) > 0:                 
        row, col, direction, nTurns = movementQueue.pop(0)      

        for d in DeltaMove:
            dx,dy = d
            t = nTurns
            if direction != d: t += 1
            if t > 2: continue
            if [row+dy,col+dx] == [originRow,originCol]: continue
            v = marker[row+dy+2][col+dx+2] 
            if v == marker[originRow+2][originCol+2] :
                return [row+dy,col+dx]
            if v != 0: continue

            movementQueue.append([row+dy,col+dx,d, t])
            marker[row+dy+2][col+dx+2] = -1
        
a = [[1,1,3,3,2,1],
     [1,0,1,0,0,6],
     [1,2,0,0,5,6]]

for r in a: print r
print find(a,2,1)


