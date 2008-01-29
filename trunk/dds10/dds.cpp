
/* DDS 1.0.0   A bridge double dummy solver.				      */
/* Copyright (C) 2006  Bo Haglund                                             */
/* Cleanups and porting to Linux and MacOSX (C) 2006 by Alex Martelli         */
/*								              */
/* This program is free software; you can redistribute it and/or              */
/* modify it under the terms of the GNU General Public License                */
/* as published by the Free Software Foundation; either version 2             */
/* of the License, or (at your option) any later version.                     */ 
/*								              */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              */
/* GNU General Public License for more details.                               */
/*								              */
/* You should have received a copy of the GNU General Public License          */
/* along with this program; if not, write to the Free Software                */
/* Foundation, Inc, 51 Franklin Street, 5th Floor, Boston MA 02110-1301, USA. */


#include "dll.h"

int handStore[4][4];
int nodeTypeStore[4];
int lho[4], rho[4], partner[4];
int trumpContract;
int trump;
unsigned short int bitMapRank[15];
int nodes;
int no[50];
int iniDepth;
int handToPlay;
int payOff, val;
int sct[4][4], nullsct[4][4];
struct pos iniPosition, position;
struct pos lookAheadPos; /* Is initialized for starting
   alpha-beta search */
struct moveType forbiddenMoves[14];
struct moveType initialMoves[4];
struct moveType cd;
struct movePlyType movePly[50];
int tricksTarget;
struct gameInfo game;
int estTricks[4];
FILE *fp2, *fp7, *fp11;
struct moveType * bestMove;
int hiwinSetSize=0, hinodeSetSize=0;
int MaxnodeSetSize=0;
int MaxwinSetSize=0;
int nodeSetSizeLimit=0;
int winSetSizeLimit=0;
int ttCollect=FALSE;
int suppressTTlog=FALSE;
char cardRank[15], cardSuit[5], cardHand[4];


#if defined(_WIN32)
extern "C" BOOL WINAPI DllMain(HINSTANCE hInst,
				DWORD reason,
				LPVOID reserved) {

  if (reason==DLL_PROCESS_ATTACH)
    InitStart();
  else if (reason==DLL_PROCESS_DETACH) {
      if (bestMove)
        free(bestMove);
      if (heurTable)
        free(heurTable);
      if (nodeCards)
        free(nodeCards);
      if (winCards)
        free(winCards);
      if (ttStore)
        free(ttStore);
      if (rel)
        free(rel);
  }
  return 1;
}
#endif

  int STDCALL SolveBoard(struct deal dl, int target,
    int solutions, int mode, struct futureTricks *futp) {

  int k, l, n, cardCount, found, totalTricks, tricks, last, checkRes;
  int g, upperbound, lowerbound, first, i, j, forb, ind, flag, noMoves;
  int noStartMoves;
  int handRelFirst;
  int latestTrickSuit[4];
  int latestTrickRank[4];
  int maxHand=0, maxSuit=0, maxRank;
  struct movePlyType temp;
  struct moveType mv;
  
  InitStart();

  for (k=0; k<=13; k++) {
    forbiddenMoves[k].rank=0;
    forbiddenMoves[k].suit=0;
  }

  if (target<-1)
    return -5;
  if (target>13)
    return -7;
  if (solutions<1)
    return -8;
  if (solutions>3)
    return -9;
  
  if (target==-1)
    tricksTarget=99;
  else
    tricksTarget=target;

  for (i=0; i<=3; i++)
    for (j=0; j<=3; j++)
      game.suit[i][j]=dl.remainCards[i][j]>>2;

  cardCount=0;
  for (k=0; k<=3; k++)
    for (l=0; l<=3; l++)
      cardCount+=CountOnes(game.suit[k][l]);

  if (dl.currentTrickRank[2]) {
    handToPlay=handStore[dl.first][3];
    handRelFirst=3;
    noStartMoves=3;
    if (cardCount<=4) {
      for (k=0; k<=3; k++) {
        if (game.suit[handToPlay][k]!=0) {
          latestTrickSuit[handToPlay]=k;
          latestTrickRank[handToPlay]=
            InvBitMapRank(game.suit[handToPlay][k]);
          break;
        }
      }
      latestTrickSuit[handStore[dl.first][2]]=dl.currentTrickSuit[2];
      latestTrickRank[handStore[dl.first][2]]=dl.currentTrickRank[2];
      latestTrickSuit[handStore[dl.first][1]]=dl.currentTrickSuit[1];
      latestTrickRank[handStore[dl.first][1]]=dl.currentTrickRank[1];
      latestTrickSuit[dl.first]=dl.currentTrickSuit[0];
      latestTrickRank[dl.first]=dl.currentTrickRank[0];
    }
  }
  else if (dl.currentTrickRank[1]) {
    handToPlay=handStore[dl.first][2];
    handRelFirst=2;
    noStartMoves=2;
    if (cardCount<=4) {
      for (k=0; k<=3; k++) {
        if (game.suit[handToPlay][k]!=0) {
          latestTrickSuit[handToPlay]=k;
          latestTrickRank[handToPlay]=
            InvBitMapRank(game.suit[handToPlay][k]);
          break;
        }
      }
      for (k=0; k<=3; k++) {
        if (game.suit[handStore[dl.first][3]][k]!=0) {
          latestTrickSuit[handStore[dl.first][3]]=k;
          latestTrickRank[handStore[dl.first][3]]=
            InvBitMapRank(game.suit[handStore[dl.first][3]][k]);
          break;
        }
      }
      latestTrickSuit[handStore[dl.first][1]]=dl.currentTrickSuit[1];
      latestTrickRank[handStore[dl.first][1]]=dl.currentTrickRank[1];
      latestTrickSuit[dl.first]=dl.currentTrickSuit[0];
      latestTrickRank[dl.first]=dl.currentTrickRank[0];
    }
  }
  else if (dl.currentTrickRank[0]) {
    handToPlay=handStore[dl.first][1];
    handRelFirst=1;
    noStartMoves=1;
    if (cardCount<=4) {
      for (k=0; k<=3; k++) {
        if (game.suit[handToPlay][k]!=0) {
          latestTrickSuit[handToPlay]=k;
          latestTrickRank[handToPlay]=
            InvBitMapRank(game.suit[handToPlay][k]);
          break;
        }
      }
      for (k=0; k<=3; k++) {
        if (game.suit[handStore[dl.first][3]][k]!=0) {
          latestTrickSuit[handStore[dl.first][3]]=k;
          latestTrickRank[handStore[dl.first][3]]=
            InvBitMapRank(game.suit[handStore[dl.first][3]][k]);
          break;
        }
      }
      for (k=0; k<=3; k++) {
        if (game.suit[handStore[dl.first][2]][k]!=0) {
          latestTrickSuit[handStore[dl.first][2]]=k;
          latestTrickRank[handStore[dl.first][2]]=
            InvBitMapRank(game.suit[handStore[dl.first][2]][k]);
          break;
        }
      }
      latestTrickSuit[dl.first]=dl.currentTrickSuit[0];
      latestTrickRank[dl.first]=dl.currentTrickRank[0];
    }
  }
  else {
    handToPlay=dl.first;
    handRelFirst=0;
    noStartMoves=0;
    if (cardCount<=4) {
      for (k=0; k<=3; k++) {
        if (game.suit[handToPlay][k]!=0) {
          latestTrickSuit[handToPlay]=k;
          latestTrickRank[handToPlay]=
            InvBitMapRank(game.suit[handToPlay][k]);
          break;
        }
      }
      for (k=0; k<=3; k++) {
        if (game.suit[handStore[dl.first][3]][k]!=0) {
          latestTrickSuit[handStore[dl.first][3]]=k;
          latestTrickRank[handStore[dl.first][3]]=
            InvBitMapRank(game.suit[handStore[dl.first][3]][k]);
          break;
        }
      }
      for (k=0; k<=3; k++) {
        if (game.suit[handStore[dl.first][2]][k]!=0) {
          latestTrickSuit[handStore[dl.first][2]]=k;
          latestTrickRank[handStore[dl.first][2]]=
            InvBitMapRank(game.suit[handStore[dl.first][2]][k]);
          break;
        }
      }
      for (k=0; k<=3; k++) {
        if (game.suit[handStore[dl.first][1]][k]!=0) {
          latestTrickSuit[handStore[dl.first][1]]=k;
          latestTrickRank[handStore[dl.first][1]]=
            InvBitMapRank(game.suit[handStore[dl.first][1]][k]);
          break;
        }
      }
    }
  }

  game.contract=100+10*(dl.trump+1);
  game.first=dl.first;
  first=dl.first;
  game.noOfCards=cardCount;
  if (dl.currentTrickRank[0]!=0) {
    game.leadHand=dl.first;
    game.leadSuit=dl.currentTrickSuit[0];
    game.leadRank=dl.currentTrickRank[0];
  }
  else {
    game.leadHand=0;
    game.leadSuit=0;
    game.leadRank=0;
  }

  for (k=0; k<=2; k++) {
    initialMoves[k].suit=255;
    initialMoves[k].rank=255;
  }

  for (k=0; k<noStartMoves; k++) {
    initialMoves[noStartMoves-1-k].suit=dl.currentTrickSuit[k];
    initialMoves[noStartMoves-1-k].rank=dl.currentTrickRank[k];
  }

  if (cardCount % 4)
    totalTricks=(cardCount-4)/4+2;
  else
    totalTricks=(cardCount-4)/4+1;
  checkRes=CheckDeal(&cd);
  if (game.noOfCards<=0)
    return -2;
  if (game.noOfCards>52)
    return -10;
  if (totalTricks<target)
    return -3;
  if (checkRes)
    return -4;

  if (cardCount<=4) {
    maxRank=0;
    /* Highest trump? */
    if (dl.trump!=4) {
      for (k=0; k<=3; k++) {
        if ((latestTrickSuit[k]==dl.trump)&&
          (latestTrickRank[k]>maxRank)) {
          maxRank=latestTrickRank[k];
          maxSuit=dl.trump;
          maxHand=k;
        }
      }
    }
    /* Highest card in leading suit */
    if (maxRank==0) {
      for (k=0; k<=3; k++) {
        if (k==dl.first) {
          maxSuit=latestTrickSuit[dl.first];
          maxHand=dl.first;
          maxRank=latestTrickRank[dl.first];
          break;
        }
      }
      for (k=0; k<=3; k++) {
        if ((k!=dl.first)&&(latestTrickSuit[k]==maxSuit)&&
          (latestTrickRank[k]>maxRank)) {
          maxHand=k;
          maxRank=latestTrickRank[k];
        }
      }
    }
    futp->nodes=0;
    futp->cards=1;
    futp->suit[0]=latestTrickSuit[handToPlay];
    futp->rank[0]=latestTrickRank[handToPlay];
    futp->equals[0]=0;
    if ((target==0)&&(solutions<3))
      futp->score[0]=0;
    else if ((handToPlay==maxHand)||
	(partner[handToPlay]==maxHand))
      futp->score[0]=1;
    else
      futp->score[0]=0;
    return 1;
  }

  InitGame(0, FALSE, first, handRelFirst);

  nodes=0;
  iniDepth=cardCount-4;
  hiwinSetSize=0;
  hinodeSetSize=0;

  if (mode==0) {
    MoveGen(&lookAheadPos, iniDepth);
    if (movePly[iniDepth].last==0) {
	futp->nodes=0;
	futp->cards=1;
	futp->suit[0]=movePly[iniDepth].move[0].suit;
	futp->rank[0]=movePly[iniDepth].move[0].rank;
	futp->equals[0]=
	  movePly[iniDepth].move[0].sequence<<2;
	futp->score[0]=-2;
	return 1;
    }
  }
  if ((target==0)&&(solutions<3)) {
    MoveGen(&lookAheadPos/*&iniPosition*/, iniDepth);
    futp->nodes=0;
    for (k=0; k<=movePly[iniDepth].last; k++) {
	futp->suit[k]=movePly[iniDepth].move[k].suit;
	futp->rank[k]=movePly[iniDepth].move[k].rank;
	futp->equals[k]=
	  movePly[iniDepth].move[k].sequence<<2;
	futp->score[k]=0;
    }
    if (solutions==1)
	futp->cards=1;
    else
	futp->cards=movePly[iniDepth].last+1;
    return 1;
  }

  if ((target!=-1)&&(solutions!=3)) {
    val=ABsearch(&lookAheadPos, tricksTarget, iniDepth);
    temp=movePly[iniDepth];
    last=movePly[iniDepth].last;
    noMoves=last+1;
    hiwinSetSize=winSetSize;
    hinodeSetSize=nodeSetSize;
    if (nodeSetSize>MaxnodeSetSize)
      MaxnodeSetSize=nodeSetSize;
    if (winSetSize>MaxwinSetSize)
      MaxwinSetSize=winSetSize;
    if (val==1)
      payOff=tricksTarget;
    else
      payOff=0;
    futp->cards=1;
    ind=2;

    if (payOff<=0) {
      futp->suit[0]=movePly[game.noOfCards-4].move[0].suit;
      futp->rank[0]=movePly[game.noOfCards-4].move[0].rank;
	futp->equals[0]=(movePly[game.noOfCards-4].move[0].sequence)<<2;
	if (tricksTarget>1)
        futp->score[0]=-1;
	else
	  futp->score[0]=0;
    }
    else {
      futp->suit[0]=bestMove[game.noOfCards-4].suit;
      futp->rank[0]=bestMove[game.noOfCards-4].rank;
	futp->equals[0]=(bestMove[game.noOfCards-4].sequence)<<2;
      futp->score[0]=payOff;
    }
  }
  else {
    g=estTricks[handToPlay];
    upperbound=13;
    lowerbound=0;
    do {
      if (g==lowerbound)
        tricks=g+1;
      else
        tricks=g;
      val=ABsearch(&lookAheadPos, tricks, iniDepth);
      if (val==TRUE)
        mv=bestMove[game.noOfCards-4];
      hiwinSetSize=Max(hiwinSetSize, winSetSize);
      hinodeSetSize=Max(hinodeSetSize, nodeSetSize);
      if (nodeSetSize>MaxnodeSetSize)
        MaxnodeSetSize=nodeSetSize;
      if (winSetSize>MaxwinSetSize)
        MaxwinSetSize=winSetSize;
      if (val==FALSE) {
	  upperbound=tricks-1;
	  g=upperbound;
	}	
	else {
	  lowerbound=tricks;
	  g=lowerbound;
	}
      InitSearch(&iniPosition, game.noOfCards-4,
        initialMoves, first, FALSE);
    }
    while (lowerbound<upperbound);
    payOff=g;
    temp=movePly[iniDepth];
    last=movePly[iniDepth].last;
    noMoves=last+1;
    ind=2;
    bestMove[game.noOfCards-4]=mv;
    futp->cards=1;
    if (payOff<=0) {
      futp->score[0]=0;
      futp->suit[0]=movePly[game.noOfCards-4].move[0].suit;
      futp->rank[0]=movePly[game.noOfCards-4].move[0].rank;
	futp->equals[0]=(movePly[game.noOfCards-4].move[0].sequence)<<2;
    }
    else {
      futp->score[0]=payOff;
      futp->suit[0]=bestMove[game.noOfCards-4].suit;
      futp->rank[0]=bestMove[game.noOfCards-4].rank;
	futp->equals[0]=(bestMove[game.noOfCards-4].sequence)<<2;
    }

    tricksTarget=payOff;
  }

  if ((solutions==2)&&(payOff>0)) {
    forb=1;
    ind=forb;
    while ((payOff==tricksTarget)&&(ind<(temp.last+1))) {
      forbiddenMoves[forb].suit=bestMove[game.noOfCards-4].suit;
      forbiddenMoves[forb].rank=bestMove[game.noOfCards-4].rank;
      forb++; ind++;
      /* All moves before bestMove in the move list shall be
      moved to the forbidden moves list, since none of them reached
      the target */
      for (k=0; k<=12; k++)
        if ((bestMove[iniDepth].suit==movePly[iniDepth].move[k].suit)
          &&(bestMove[iniDepth].rank==movePly[iniDepth].move[k].rank))
          break;
      for (i=0; i<k; i++) {  /* All moves until best move */
        flag=FALSE;
        for (j=0; j<forb; j++) {
          if ((movePly[iniDepth].move[i].suit==forbiddenMoves[j].suit)
            &&(movePly[iniDepth].move[i].rank==forbiddenMoves[j].rank)) {
            /* If the move is already in the forbidden list */
            flag=TRUE;
            break;
          }
        }
        if (!flag) {
          forbiddenMoves[forb]=movePly[iniDepth].move[i];
          forb++;
        }
      }
      InitSearch(&iniPosition, game.noOfCards-4,
        initialMoves, first, FALSE);
      val=ABsearch(&lookAheadPos, tricksTarget, iniDepth);
      hiwinSetSize=winSetSize;
      hinodeSetSize=nodeSetSize;
      if (nodeSetSize>MaxnodeSetSize)
        MaxnodeSetSize=nodeSetSize;
      if (winSetSize>MaxwinSetSize)
        MaxwinSetSize=winSetSize;
      if (val==TRUE) {
        payOff=tricksTarget;
        futp->cards=ind;
        futp->suit[ind-1]=bestMove[game.noOfCards-4].suit;
        futp->rank[ind-1]=bestMove[game.noOfCards-4].rank;
	  futp->equals[ind-1]=(bestMove[game.noOfCards-4].sequence)<<2;
        futp->score[ind-1]=payOff;
      }
      else
        payOff=0;
    }
  }
  else if ((solutions==2)&&(payOff==0)&&
	((target==-1)||(tricksTarget==1))) {
    futp->cards=noMoves;
    /* Find the cards that were in the initial move list
    but have not been listed in the current result */
    n=0;
    for (i=0; i<noMoves; i++) {
      found=FALSE;
      if ((temp.move[i].suit==futp->suit[0])&&
        (temp.move[i].rank==futp->rank[0])) {
          found=TRUE;
      }
      if (!found) {
        futp->suit[1+n]=temp.move[i].suit;
        futp->rank[1+n]=temp.move[i].rank;
	  futp->equals[1+n]=(temp.move[i].sequence)<<2;
        futp->score[1+n]=0;
        n++;
      }
    }
  }

  if ((solutions==3)&&(payOff>0)) {
    forb=1;
    ind=forb;
    for (i=0; i<last; i++) {
      forbiddenMoves[forb].suit=bestMove[game.noOfCards-4].suit;
      forbiddenMoves[forb].rank=bestMove[game.noOfCards-4].rank;
      forb++; ind++;

      g=payOff;
      upperbound=payOff;
      lowerbound=0;
      InitSearch(&iniPosition, game.noOfCards-4,
        initialMoves, first, FALSE);
      do {
        if (g==lowerbound)
          tricks=g+1;
        else
          tricks=g;
        val=ABsearch(&lookAheadPos, tricks, iniDepth);
        if (val==TRUE)
          mv=bestMove[game.noOfCards-4];
        hiwinSetSize=Max(hiwinSetSize, winSetSize);
        hinodeSetSize=Max(hinodeSetSize, nodeSetSize);
        if (nodeSetSize>MaxnodeSetSize)
          MaxnodeSetSize=nodeSetSize;
        if (winSetSize>MaxwinSetSize)
          MaxwinSetSize=winSetSize;
        if (val==FALSE) {
	    upperbound=tricks-1;
	    g=upperbound;
	  }	
	  else {
	    lowerbound=tricks;
	    g=lowerbound;
	  }
        InitSearch(&iniPosition, game.noOfCards-4,
          initialMoves, first, FALSE);
      }
      while (lowerbound<upperbound);
      payOff=g;
      if (payOff==0) {
        last=movePly[iniDepth].last;
        futp->cards=last;
        for (j=0; j<last; j++) {
          futp->suit[ind-1+j]=movePly[game.noOfCards-4].move[ind-1+j].suit;
          futp->rank[ind-1+j]=movePly[game.noOfCards-4].move[ind-1+j].rank;
	    futp->equals[ind-1+j]=(movePly[game.noOfCards-4].move[ind-1+j].sequence)<<2;
          futp->score[ind-1+j]=payOff;
        }
        break;
      }
      else {
        bestMove[game.noOfCards-4]=mv;

        futp->cards=ind;
        futp->suit[ind-1]=bestMove[game.noOfCards-4].suit;
        futp->rank[ind-1]=bestMove[game.noOfCards-4].rank;
	  futp->equals[ind-1]=(bestMove[game.noOfCards-4].sequence)<<2;
        futp->score[ind-1]=payOff;
      }   
    }
  }
  else if ((solutions==3)&&(payOff==0)) {
    futp->cards=noMoves;
    /* Find the cards that were in the initial move list
    but have not been listed in the current result */
    n=0;
    for (i=0; i<noMoves; i++) {
      found=FALSE;
      if ((temp.move[i].suit==futp->suit[0])&&
        (temp.move[i].rank==futp->rank[0])) {
          found=TRUE;
      }
      if (!found) {
        futp->suit[1+n]=temp.move[i].suit;
        futp->rank[1+n]=temp.move[i].rank;
	  futp->equals[1+n]=(temp.move[i].sequence)<<2;
        futp->score[1+n]=0;
        n++;
      }
    }
  }

  for (k=0; k<=13; k++) {
    forbiddenMoves[k].suit=0;
    forbiddenMoves[k].rank=0;
  }

  futp->nodes=nodes;
  return 1;
}

struct relRanksType * rel;
struct ttStoreType * ttStore;
struct nodeCardsType * nodeCards;
struct winCardType * winCards;
struct heurTableType * heurTable;
int playedTricks;
unsigned short int iniRemovedRanks[4];
int nodeSetSize=0;
int winSetSize=0;
int addNodeCount;
int lastTTstore=0;

int _initialized=0;

void InitStart(void) {
  int h, s;

  if (_initialized)
      return;
  _initialized = 1;

  for (s=0; s<=3; s++)
    for (h=0; h<=3; h++)
	  nullsct[h][s]=0;

  nodeSetSizeLimit=2000000;
  winSetSizeLimit=4000000;

  bitMapRank[14]=0x1000;
  bitMapRank[13]=0x0800;
  bitMapRank[12]=0x0400;
  bitMapRank[11]=0x0200;
  bitMapRank[10]=0x0100;
  bitMapRank[9]=0x0080;
  bitMapRank[8]=0x0040;
  bitMapRank[7]=0x0020;
  bitMapRank[6]=0x0010;
  bitMapRank[5]=0x0008;
  bitMapRank[4]=0x0004;
  bitMapRank[3]=0x0002;
  bitMapRank[2]=0x0001;
  bitMapRank[1]=0;
  bitMapRank[0]=0;

  bestMove = (struct moveType *)calloc(50, sizeof(moveType));
 
  handStore[0][0]=0;
  handStore[0][1]=1;
  handStore[0][2]=2;
  handStore[0][3]=3;
  handStore[1][0]=1;
  handStore[1][1]=2;
  handStore[1][2]=3;
  handStore[1][3]=0;
  handStore[2][0]=2;
  handStore[2][1]=3;
  handStore[2][2]=0;
  handStore[2][3]=1;
  handStore[3][0]=3;
  handStore[3][1]=0;
  handStore[3][2]=1;
  handStore[3][3]=2;

  lho[0]=1; lho[1]=2; lho[2]=3; lho[3]=0;
  rho[0]=3; rho[1]=0; rho[2]=1; rho[3]=2;
  partner[0]=2; partner[1]=3; partner[2]=0; partner[3]=1;

  cardRank[2]='2'; cardRank[3]='3'; cardRank[4]='4'; cardRank[5]='5';
  cardRank[6]='6'; cardRank[7]='7'; cardRank[8]='8'; cardRank[9]='9';
  cardRank[10]='T'; cardRank[11]='J'; cardRank[12]='Q'; cardRank[13]='K';
  cardRank[14]='A';

  cardSuit[0]='S'; cardSuit[1]='H'; cardSuit[2]='D'; cardSuit[3]='C';
  cardSuit[4]='N';

  cardHand[0]='N'; cardHand[1]='E'; cardHand[2]='S'; cardHand[3]='W';

  heurTable = (struct heurTableType *)malloc(sizeof(heurTableType));

  IniHeurTable();

  nodeCards = (struct nodeCardsType *)calloc(nodeSetSizeLimit+1, sizeof(nodeCardsType));

  winCards = (struct winCardType *)calloc(winSetSizeLimit+1, sizeof(winCardType));

  ttStore = (struct ttStoreType *)calloc(SEARCHSIZE, sizeof(ttStoreType));

  rel = (relRanksType *)calloc(16385, sizeof(relRanksType));
 
  return;
}


void InitGame(int gameNo, int moveTreeFlag, int first, int handRelFirst) {

  int k, s, h, r, cardFound, currHand=0, order, m, temp1, temp2;
  long a;
  unsigned short int ind;
  unsigned short int rankInSuit[4][4];
  int points[4], tricks;
  int addNS, addEW, addMAX, trumpNS, trumpEW;
  struct gameInfo gm;

  #ifdef STAT
    fp2=fopen("stat.txt","w");
  #endif

  #ifdef TTDEBUG
  if (!suppressTTlog) {
    fp7=fopen("storett.txt","w");
    fp11=fopen("rectt.txt", "w");
    fclose(fp11);
    ttCollect=TRUE;
  }
  #endif	
  
  for (k=0; k<=3; k++)
    for (m=0; m<=3; m++)
      iniPosition.rankInSuit[k][m]=game.suit[k][m];

  iniPosition.first[game.noOfCards-4]=first;
  iniPosition.handRelFirst=handRelFirst;
  lookAheadPos=iniPosition;
  

  for (ind=0; ind<=16383; ind++) {
    for (s=0; s<=3; s++) {
      for (h=0; h<=3; h++) {
        rankInSuit[h][s]=0;
        rel[ind].relRanks[h][s]=0;
      }
      
      for (r=2; r<=14; r++) {
        for (h=0; h<=3; h++) {
          if ((ind & game.suit[h][s] & bitMapRank[r])!=0) {
            rankInSuit[h][s]=rankInSuit[h][s] | bitMapRank[r];
            break;
          }    
        }
      }

      k=14;
      order=1;
      while (k>=2) {
        cardFound=FALSE;
        for (h=0; h<=3; h++) {
          if ((rankInSuit[h][s] & bitMapRank[k])!=0) {
            currHand=h;
            cardFound=TRUE;
            break;
          }
        }
        if (cardFound==FALSE) {
          k--;
          continue;
        }

        rel[ind].relRanks[currHand][s]=rel[ind].relRanks[currHand][s] |
          bitMapRank[15-order];
        order++;
        k--;
      }
    }
  }
  
  for (h=0; h<=3; h++)
    for (a=0; a<=13; a++) {
      posSearch.posSearchPoint[h][a]=NULL;
      posSearch.first[h][a]=NULL;
      posSearch.last[h][a]=NULL;
    }

  nodeSetSize=0;
  winSetSize=0;
  addNodeCount=0;

  temp1=game.contract/100;
  temp2=game.contract-100*temp1;
  trump=temp2/10-1;
  if ((trump<4)&&(trump>=0))
    trumpContract=TRUE;
  else
    trumpContract=FALSE;
  
  /*start: tricksest */

  gm=game;
  if (game.leadRank!=0)
    gm.suit[gm.leadHand][gm.leadSuit]=
      gm.suit[gm.leadHand][gm.leadSuit] | bitMapRank[gm.leadRank];

  for (h=0; h<=3; h++)
    points[h]=0;

  for (h=0; h<=3; h++)
    for (s=0; s<=3; s++) {
      if ((gm.suit[h][s] & bitMapRank[14])!=0)
        points[h]=points[h]+4;
      if ((gm.suit[h][s] & bitMapRank[13])!=0)
        points[h]=points[h]+3;
      if ((gm.suit[h][s] & bitMapRank[12])!=0)
        points[h]=points[h]+2;
      if ((gm.suit[h][s] & bitMapRank[11])!=0)
        points[h]=points[h]+1;
      if (trumpContract) {
        if ((CountOnes(gm.suit[h][s])<=2) &&
          (gm.suit[h][s]<bitMapRank[14]) &&
          (gm.suit[h][s]>bitMapRank[10]))
          points[h]--;
        if (CountOnes(gm.suit[h][s])==0)
          points[h]=points[h]+3;
        else if (CountOnes(gm.suit[h][s])==1)
          points[h]=points[h]+2;
        else if (CountOnes(gm.suit[h][s])==2)
          points[h]=points[h]+1;
      }
    }
  if (trumpContract) {
    trumpNS=CountOnes(gm.suit[0][trump])+CountOnes(gm.suit[2][trump]);
    trumpEW=CountOnes(gm.suit[1][trump])+CountOnes(gm.suit[3][trump]);
  }
  else {
    trumpNS=0;
    trumpEW=0;
  }

  addNS=points[0]+points[2];
  addEW=points[1]+points[3];
  addNS=addNS+trumpNS-trumpEW;
  addEW=addEW+trumpEW-trumpNS;
  addMAX=Max(addNS, addEW);
  if (addMAX>=37)
    tricks=13;
  else if (addMAX>=33)
    tricks=12;
  else if ((addMAX>=29) && trumpContract)
    tricks=11;
  else if ((addMAX>=31) && (!trumpContract))
    tricks=11;
  else if ((addMAX>=26) && trumpContract)
    tricks=10;
  else if ((addMAX>=29) && (!trumpContract))
    tricks=10;
  else if ((addMAX>=25) && trumpContract)
    tricks=9;
  else if ((addMAX>=26) && (!trumpContract))
    tricks=9;
  else if (addMAX>=24)
    tricks=8;
  else if (addMAX>=22)
    tricks=7;
  else
    tricks=6;

  if (addNS>addEW) {
    estTricks[0]=tricks;
    estTricks[2]=tricks;
    estTricks[1]=13-tricks;
    estTricks[3]=13-tricks;
  }
  else {
    estTricks[1]=tricks;
    estTricks[3]=tricks;
    estTricks[0]=13-tricks;
    estTricks[2]=13-tricks;
  }

  /*end: tricksest */

  #ifdef STAT
  fprintf(fp2, "Estimated tricks for hand to play:\n");	
  fprintf(fp2, "hand=%d  est tricks=%d\n", 
	  handToPlay, estTricks[handToPlay]);
  #endif

  InitSearch(&lookAheadPos, game.noOfCards-4, initialMoves, first,
    moveTreeFlag);
  return;
}


void InitSearch(struct pos * posPoint, int depth,
  struct moveType startMoves[], int first, int mtd)  {

  int s, d, h, max, hmax=0, handRelFirst, a;
  int k, noOfStartMoves;        /* No of starting moves */
  int hand[3], suit[3], rank[3];
  struct moveType move;
  unsigned short int startMovesBitMap[4][4]; /* Indices are hand and suit */

  for (h=0; h<=3; h++)
    for (s=0; s<=3; s++)
      startMovesBitMap[h][s]=0;

  for (k=0; k<=2; k++) {
    hand[k]=handStore[first][k];
    suit[k]=startMoves[k].suit;
    rank[k]=startMoves[k].rank;
    if (rank[k]>=2)
      startMovesBitMap[hand[k]][suit[k]]=startMovesBitMap[hand[k]][suit[k]] |
        bitMapRank[rank[k]];
  }

  for (d=0; d<=49; d++) {
    bestMove[d].suit=0;
    bestMove[d].rank=0;
    bestMove[d].weight=0;
    bestMove[d].sequence=0;
  }

  handRelFirst=posPoint->handRelFirst;
  if ((handStore[first][handRelFirst]==0)||
    (handStore[first][handRelFirst]==2)) {
    nodeTypeStore[0]=MAXNODE;
    nodeTypeStore[1]=MINNODE;
    nodeTypeStore[2]=MAXNODE;
    nodeTypeStore[3]=MINNODE;
  }
  else {
    nodeTypeStore[0]=MINNODE;
    nodeTypeStore[1]=MAXNODE;
    nodeTypeStore[2]=MINNODE;
    nodeTypeStore[3]=MAXNODE;
  }

  k=0;
  while ((startMoves[k].suit<=3)&&(startMoves[k].suit!=255)&&
    (startMoves[k].rank<=14)&&(startMoves[k].rank>=2)) {
    k++;
    if (k>3)
      break;
  }
  noOfStartMoves=k;

  posPoint->first[depth]=first;
  posPoint->handRelFirst=k;
  posPoint->tricksMAX=0;

  if (k>0) {
    posPoint->move[depth+k]=startMoves[k-1];
    move=startMoves[k-1];
  }

  posPoint->high[depth+k]=first;

  while (k>0) {
    movePly[depth+k].current=0;
    movePly[depth+k].last=0;
    movePly[depth+k].move[0].suit=startMoves[k-1].suit;
    movePly[depth+k].move[0].rank=startMoves[k-1].rank;
    if (k<noOfStartMoves) {     /* If there is more than one start move */
      if (WinningMove(&startMoves[k-1], &move)) {
        posPoint->move[depth+k].suit=startMoves[k-1].suit;
        posPoint->move[depth+k].rank=startMoves[k-1].rank;
        posPoint->high[depth+k]=handStore[first][noOfStartMoves-k];
        move=posPoint->move[depth+k];
      }
      else {
        posPoint->move[depth+k]=posPoint->move[depth+k+1];
        posPoint->high[depth+k]=posPoint->high[depth+k+1];
      }
    }
    k--;
  }

  for (s=0; s<=3; s++)
    posPoint->removedRanks[s]=0;

  for (s=0; s<=3; s++)       /* Suit */
    for (h=0; h<=3; h++)     /* Hand */
      posPoint->removedRanks[s]=posPoint->removedRanks[s] |
        posPoint->rankInSuit[h][s];
  for (s=0; s<=3; s++)
    posPoint->removedRanks[s]=~(posPoint->removedRanks[s]);

  for (s=0; s<=3; s++)       /* Suit */
    for (h=0; h<=3; h++)     /* Hand */
      posPoint->removedRanks[s]=posPoint->removedRanks[s] &
        (~startMovesBitMap[h][s]);
        
  for (s=0; s<=3; s++)
    iniRemovedRanks[s]=posPoint->removedRanks[s];


  for (d=0; d<=49; d++) {
    for (s=0; s<=3; s++)
      posPoint->winRanks[d][s]=0;
  }


  /* Initialize winning rank */
  for (s=0; s<=3; s++) {
    posPoint->winner[s].rank=0;
    posPoint->winner[s].hand=0;
  }

  if (noOfStartMoves>=1) {
    for (k=noOfStartMoves; k>=0; k--) {
      s=movePly[depth+k].move[0].suit;
      if (movePly[depth+k].move[0].rank>posPoint->winner[s].rank)
        posPoint->winner[s].rank=movePly[depth+k].move[0].rank;
    }
  }

  for (s=0; s<=3; s++) {
    k=posPoint->winner[s].rank;
    max=k;
    while (k<=14) {
      for (h=0; h<=3; h++) {
        if ((posPoint->rankInSuit[h][s] & bitMapRank[k]) != 0) {
          max=k;
          hmax=h;
          break;
        }
      }
      k++;
    }
    posPoint->winner[s].rank=max;
    posPoint->winner[s].hand=hmax;
  }

  /* Initialize second best rank */
  for (s=0; s<=3; s++) {
    posPoint->secondBest[s].rank=0;
    posPoint->secondBest[s].hand=0;
  }

  if (noOfStartMoves>=1) {
    for (k=noOfStartMoves; k>=0; k--) {
      s=movePly[depth+k].move[0].suit;
      if ((movePly[depth+k].move[0].rank>posPoint->secondBest[s].rank)&&
         (movePly[depth+k].move[0].rank<posPoint->winner[s].rank))
        posPoint->secondBest[s].rank=movePly[depth+k].move[0].rank;
    }
  }

  for (s=0; s<=3; s++) {
    k=posPoint->secondBest[s].rank;
    max=k;
    while (k<=14) {
      for (h=0; h<=3; h++) {
        if (((posPoint->rankInSuit[h][s] & bitMapRank[k]) != 0)&&
          (k<posPoint->winner[s].rank)) {
          max=k;
          hmax=h;
          break;
        }
      }
      k++;
    }
    posPoint->secondBest[s].rank=max;
    posPoint->secondBest[s].hand=hmax;
  }

  for (s=0; s<=3; s++)
    for (h=0; h<=3; h++)
      posPoint->length[h][s]=CountOnes(posPoint->rankInSuit[h][s]);

  for (d=0; d<=49; d++) {
    score1Counts[d]=0;
    score0Counts[d]=0;
    c1[d]=0;  c2[d]=0;  c3[d]=0;  c4[d]=0;  c5[d]=0;  c6[d]=0; c7[d]=0;
    c8[d]=0;
    no[d]=0;
  }  

  if (!mtd) {
    for (h=0; h<=3; h++)
      for (a=0; a<=13; a++) {
        posSearch.posSearchPoint[h][a]=NULL;
        posSearch.first[h][a]=NULL;
        posSearch.last[h][a]=NULL;
      }
    nodeSetSize=0;
    winSetSize=0;
    addNodeCount=0;
  }
  
  #ifdef TTDEBUG
  if (!suppressTTlog) 
    lastTTstore=0;
  #endif
  recInd=0;

  return;
}

unsigned short int CountOnes(unsigned short int b) {
  unsigned short int numb;

  for (numb=0; b!=0; numb++, b&=(b-1));
  return numb;
}

int mexists, ready, hfirst;
int mcurrent, qtricks, out, sout, hout;
int res;
char cind;
int minimum, sopFound, nodeFound;
int score1Counts[50], score0Counts[50];
int sumScore1Counts, sumScore0Counts;
int c1[50], c2[50], c3[50], c4[50], c5[50], c6[50], c7[50], c8[50], c9[50];
int sumc1, sumc2, sumc3, sumc4, sumc5, sumc6, sumc7, sumc8, sumc9;
int scoreFlag;
int tricks;
int hh, ss, rr, mm, dd, fh;
int mcount, hand, suit, rank, order;
int k, cardFound, currHand, a;
struct evalType evalData;
struct quickType quick;
struct posSearchType posSearch;
struct winCardType * np;
struct nodeCardsType * sopP;
struct cardType card;
struct nodeCardsType  * tempP;
unsigned short int aggr[4];
unsigned short int tricksLeft;

int ABsearch(struct pos * posPoint, int target, int depth) {
    /* posPoint points to the current look-ahead position,
       target is number of tricks to take for the player,
       depth is the remaining search length, must be positive,
       the value of the subtree is returned.  */

  int moveExists, value;
  struct makeType makeData;
  struct nodeCardsType * cardsP;


  struct evalType Evaluate(struct pos * posPoint);
  struct makeType Make(struct pos * posPoint, int depth, int ncount);
  void Undo(struct pos * posPoint, int depth);

  cardsP=NULL;
  hand=handStore[posPoint->first[depth]][posPoint->handRelFirst];
 
  if (posPoint->handRelFirst==0) {
    if (posPoint->tricksMAX>=target) {
      for (ss=0; ss<=3; ss++)
        posPoint->winRanks[depth][ss]=0;

        #ifdef STAT
        c1[depth]++;
        
        score1Counts[depth]++;
        if (depth==iniDepth) {
          fprintf(fp2, "score statistics:\n");
          for (dd=iniDepth; dd>=0; dd--) {
            fprintf(fp2, "d=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
            score1Counts[dd], score0Counts[dd], c1[dd], c2[dd],
              c3[dd], c4[dd]);
            fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd],
              c6[dd], c7[dd], c8[dd]);
          }
        }
        #endif
   
      return TRUE;
    }
    if (((posPoint->tricksMAX+depth/4+1)<target)&&(depth>0)) {
      for (ss=0; ss<=3; ss++)
        posPoint->winRanks[depth][ss]=0;

        #ifdef STAT
        c2[depth]++;
        score0Counts[depth]++;
        if (depth==iniDepth) {
          fprintf(fp2, "score statistics:\n");
          for (dd=iniDepth; dd>=0; dd--) {
            fprintf(fp2, "d=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
            score1Counts[dd], score0Counts[dd], c1[dd], c2[dd],
              c3[dd], c4[dd]);
            fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd],
              c6[dd], c7[dd], c8[dd]);
          }
        }
        #endif

      return FALSE;
    }
	
    if (nodeTypeStore[hand]==MAXNODE) {
      quick=QuickTricks(posPoint, hand, depth, &out, &hout, &sout);
      qtricks=quick.quickTricks;
      if ((posPoint->tricksMAX+qtricks>=target)&&
        (qtricks>0)&&(depth>0)&&(depth!=iniDepth)) {
        for (ss=0; ss<=3; ss++)
          posPoint->winRanks[depth][ss]=quick.winRanks[ss];

          #ifdef STAT
          c3[depth]++;
          score1Counts[depth]++;
          if (depth==iniDepth) {
            fprintf(fp2, "score statistics:\n");
            for (dd=iniDepth; dd>=0; dd--) {
              fprintf(fp2, "d=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
              score1Counts[dd], score0Counts[dd], c1[dd], c2[dd],
                c3[dd], c4[dd]);
              fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd],
                c6[dd], c7[dd], c8[dd]);
            }
          }
          #endif

        return TRUE;
      }
    }
    else {
      quick=QuickTricks(posPoint, hand, depth, &out, &hout, &sout);
      qtricks=quick.quickTricks;
      if (((posPoint->tricksMAX+(depth+3)/4+2-qtricks)<=target)&&
        (qtricks>0)&&(depth>0)&&(depth!=iniDepth)) {
        for (ss=0; ss<=3; ss++)
          posPoint->winRanks[depth][ss]=quick.winRanks[ss];

          #ifdef STAT
          c4[depth]++;
          score0Counts[depth]++;
          if (depth==iniDepth) {
            fprintf(fp2, "score statistics:\n");
            for (dd=iniDepth; dd>=0; dd--) {
              fprintf(fp2, "d=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
              score1Counts[dd], score0Counts[dd], c1[dd], c2[dd],
                c3[dd], c4[dd]);
              fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd],
                c6[dd], c7[dd], c8[dd]);
            }
          }
          #endif
        return FALSE;
      }
    }

    for (ss=0; ss<=3; ss++) {
      aggr[ss]=0;
      for (hh=0; hh<=3; hh++)
        aggr[ss]=aggr[ss] | posPoint->rankInSuit[hh][ss];
      for (hh=0; hh<=3; hh++)
        posPoint->relRankInSuit[hh][ss]=rel[aggr[ss]].relRanks[hh][ss];
    }
    tricks=depth>>2;
    cardsP=posSearch.first[hand][tricks];
    sopFound=FALSE;
    if (cardsP!=NULL) {
      sopP=SOPinList(posPoint, cardsP, target, &res);
      if (res) {
        sopFound=TRUE;
        cardsP=sopP;
      }
    }
    if (!sopFound) {
      np=posSearch.posSearchPoint[hand][tricks];
      if (np==NULL)
        cardsP=NULL;
      else 
        cardsP=FindSOP(posPoint, np, target);
    }
      

    if (cardsP!=NULL) {
      scoreFlag=cardsP->scoreFlag;
      if (scoreFlag==1) {
        WinAdapt(posPoint, depth, cardsP);
        if (cardsP->bestMoveRank!=0) {
          bestMove[depth].suit=cardsP->bestMoveSuit;
          bestMove[depth].rank=cardsP->bestMoveRank;
        }
          #ifdef STAT
          c5[depth]++;
          if (scoreFlag==1)
            score1Counts[depth]++;
          else
            score0Counts[depth]++;
          if (depth==iniDepth) {
            fprintf(fp2, "score statistics:\n");
            for (dd=iniDepth; dd>=0; dd--) {
              fprintf(fp2, "d=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
              score1Counts[dd], score0Counts[dd], c1[dd], c2[dd],
                c3[dd], c4[dd]);
              fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd],
                c6[dd], c7[dd], c8[dd]);
            }
          }
          #endif
        #ifdef TTDEBUG
        if (!suppressTTlog) { 
          if (lastTTstore<SEARCHSIZE) 
            ReceiveTTstore(posPoint, cardsP, target, depth);
          else 
            ttCollect=FALSE;
	  }
        #endif 

        return TRUE;
      }
      else {
        WinAdapt(posPoint, depth, cardsP);
        if (cardsP->bestMoveRank!=0) {
          bestMove[depth].suit=cardsP->bestMoveSuit;
          bestMove[depth].rank=cardsP->bestMoveRank;
        }
          #ifdef STAT
          c6[depth]++;
          if (scoreFlag==1)
            score1Counts[depth]++;
          else
            score0Counts[depth]++;
          if (depth==iniDepth) {
            fprintf(fp2, "score statistics:\n");
            for (dd=iniDepth; dd>=0; dd--) {
              fprintf(fp2, "d=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
                score1Counts[dd], score0Counts[dd], c1[dd], c2[dd], c3[dd],
                  c4[dd]);
              fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd],
                  c6[dd], c7[dd], c8[dd]);
            }
          }
          #endif

        #ifdef TTDEBUG
        if (!suppressTTlog) {
          if (lastTTstore<SEARCHSIZE) 
            ReceiveTTstore(posPoint, cardsP, target, depth);
          else 
            ttCollect=FALSE;
        }
        #endif 
        return FALSE;
      }
    }
  }

  if (depth==0) {                    /* Maximum depth? */
    evalData=Evaluate(posPoint);        /* Leaf node */
    if (evalData.tricks>=target)
      value=TRUE;
    else
      value=FALSE;
    for (ss=0; ss<=3; ss++) {
      posPoint->winRanks[depth][ss]=evalData.winRanks[ss];

        #ifdef STAT
        c7[depth]++;
        if (value==1)
          score1Counts[depth]++;
        else
          score0Counts[depth]++;
        if (depth==iniDepth) {
          fprintf(fp2, "score statistics:\n");
          for (dd=iniDepth; dd>=0; dd--) {
            fprintf(fp2, "d=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
              score1Counts[dd], score0Counts[dd], c1[dd], c2[dd], c3[dd],
              c4[dd]);
            fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd],
              c6[dd], c7[dd], c8[dd]);
          }
        }
        #endif
    }
    return value;
  }  
  else {
    moveExists=MoveGen(posPoint, depth);
    
    if ((posPoint->handRelFirst==3)&&(depth>=29)) {
      movePly[depth].current=0;
      mexists=TRUE;
      ready=FALSE;
      while (mexists) {
        makeData=Make(posPoint, depth, FALSE);
        depth--;

        for (ss=0; ss<=3; ss++) {
          aggr[ss]=0;
         for (hh=0; hh<=3; hh++)
            aggr[ss]=aggr[ss] | posPoint->rankInSuit[hh][ss];
          for (hh=0; hh<=3; hh++)
            posPoint->relRankInSuit[hh][ss]=rel[aggr[ss]].relRanks[hh][ss];
        }
        tricks=depth>>2;
        hfirst=posPoint->first[depth];
        tempP=posSearch.first[hfirst][tricks];
        sopFound=FALSE;
        if (tempP!=NULL) {
          sopP=SOPinList(posPoint, tempP, target, &res);
          if (res) {
            sopFound=TRUE;
            tempP=sopP;
          }
        }
        if (!sopFound) {
          np=posSearch.posSearchPoint[hfirst][tricks];
          if (np==NULL)
            tempP=NULL;
          else 
            tempP=FindSOP(posPoint, np, target);
        }

        if (tempP!=NULL) {
          scoreFlag=tempP->scoreFlag;
          if ((nodeTypeStore[hand]==MAXNODE)&&(scoreFlag==1)) {
            WinAdapt(posPoint, depth+1, tempP);
            for (ss=0; ss<=3; ss++)
              posPoint->winRanks[depth+1][ss]=posPoint->winRanks[depth+1][ss]
               | makeData.winRanks[ss];
            Undo(posPoint, depth+1);
            return TRUE;
          }
          else if ((nodeTypeStore[hand]==MINNODE)&&(scoreFlag==0)) {
            WinAdapt(posPoint, depth+1, tempP);
            for (ss=0; ss<=3; ss++)
              posPoint->winRanks[depth+1][ss]=posPoint->winRanks[depth+1][ss]
               | makeData.winRanks[ss];
            Undo(posPoint, depth+1);
            return FALSE;
          }
          else {
            movePly[depth+1].move[movePly[depth+1].current].weight+=100;
            ready=TRUE;
          }
        }
        depth++;
        Undo(posPoint, depth);
        if (ready)
          break;
        if (movePly[depth].current<=movePly[depth].last-1) {
          movePly[depth].current++;
          mexists=TRUE;
        }
        else
          mexists=FALSE;
      }
      if (ready)
        InsertSort(movePly[depth].last+1, depth);
    }

    movePly[depth].current=0;
    if (nodeTypeStore[hand]==MAXNODE) {
      value=FALSE;
      for (ss=0; ss<=3; ss++)
        posPoint->winRanks[depth][ss]=0;

      while (moveExists)  {
        makeData=Make(posPoint, depth, TRUE);        /* Make current move */

        value=ABsearch(posPoint, target, depth-1);          
        Undo(posPoint, depth);      /* Retract current move */
        if (value==TRUE) {
        /* A cut-off? */
	    for (ss=0; ss<=3; ss++)
            posPoint->winRanks[depth][ss]=posPoint->winRanks[depth-1][ss] |
              makeData.winRanks[ss];
	    mcurrent=movePly[depth].current;
          bestMove[depth]=movePly[depth].move[mcurrent];
          goto ABexit;
        }  
        for (ss=0; ss<=3; ss++)
          posPoint->winRanks[depth][ss]=posPoint->winRanks[depth][ss] |
           posPoint->winRanks[depth-1][ss] | makeData.winRanks[ss];

        if (movePly[depth].current<=movePly[depth].last-1) {
          movePly[depth].current++;
          moveExists=TRUE;
        }
        else
          moveExists=FALSE;
      }
    }
    else {                          /* A minnode */
      value=TRUE;
      for (ss=0; ss<=3; ss++)
        posPoint->winRanks[depth][ss]=0;
        
      while (moveExists)  {
        makeData=Make(posPoint, depth, TRUE);        /* Make current move */
        
        value=ABsearch(posPoint, target, depth-1);
        Undo(posPoint, depth);       /* Retract current move */
        if (value==FALSE) {
        /* A cut-off? */
	    for (ss=0; ss<=3; ss++)
            posPoint->winRanks[depth][ss]=posPoint->winRanks[depth-1][ss] |
              makeData.winRanks[ss];
	    mcurrent=movePly[depth].current;
          bestMove[depth]=movePly[depth].move[mcurrent];
          goto ABexit;
        }
        for (ss=0; ss<=3; ss++)
          posPoint->winRanks[depth][ss]=posPoint->winRanks[depth][ss] |
           posPoint->winRanks[depth-1][ss] | makeData.winRanks[ss];

        if (movePly[depth].current<=movePly[depth].last-1) {
          movePly[depth].current++;
          moveExists=TRUE;
        }
        else
          moveExists=FALSE;
      }
    }
  }
  ABexit:
  if (depth>=4) {
    if((posPoint->handRelFirst==0)&& (nodeSetSize<nodeSetSizeLimit)&&
      (winSetSize<winSetSizeLimit)) {
      tricks=depth>>2;
      hand=posPoint->first[depth-1];
	  if (value)
	    k=target;
	  else
		k=target-1;
      BuildSOP(posPoint, tricks, hand, target, depth,
        value, k);
    }  
  }
    #ifdef STAT
    c8[depth]++;
    if (value==1)
      score1Counts[depth]++;
    else
      score0Counts[depth]++;
    if (depth==iniDepth) {
	  if (fp2==NULL)
        exit(0);		  
      fprintf(fp2, "\n");
      fprintf(fp2, "top level cards:\n");
      for (hh=0; hh<=3; hh++) {
        fprintf(fp2, "hand=%c\n", cardHand[hh]);
        for (ss=0; ss<=3; ss++) {
          fprintf(fp2, "suit=%c", cardSuit[ss]);
          for (rr=14; rr>=2; rr--)
            if (posPoint->rankInSuit[hh][ss] & bitMapRank[rr])
              fprintf(fp2, " %c", cardRank[rr]);
          fprintf(fp2, "\n");
        }
        fprintf(fp2, "\n");
      }
      fprintf(fp2, "top level winning cards:\n");
      for (ss=0; ss<=3; ss++) {
        fprintf(fp2, "suit=%c", cardSuit[ss]);
        for (rr=14; rr>=2; rr--)
          if (posPoint->winRanks[depth][ss] & bitMapRank[rr])
            fprintf(fp2, " %c", cardRank[rr]);
        fprintf(fp2, "\n");
      }
      fprintf(fp2, "\n");
	fprintf(fp2, "\n");

      fprintf(fp2, "score statistics:\n");
      sumScore0Counts=0;
      sumScore1Counts=0;
      sumc1=0; sumc2=0; sumc3=0; sumc4=0;
      sumc5=0; sumc6=0; sumc7=0; sumc8=0; sumc9=0;
      for (dd=iniDepth; dd>=0; dd--) {
        fprintf(fp2, "depth=%d s1=%d s0=%d c1=%d c2=%d c3=%d c4=%d", dd,
          score1Counts[dd], score0Counts[dd], c1[dd], c2[dd], c3[dd], c4[dd]);
        fprintf(fp2, " c5=%d c6=%d c7=%d c8=%d\n", c5[dd], c6[dd],
          c7[dd], c8[dd]);
        sumScore0Counts=sumScore0Counts+score0Counts[dd];
        sumScore1Counts=sumScore1Counts+score1Counts[dd];
        sumc1=sumc1+c1[dd];
        sumc2=sumc2+c2[dd];
        sumc3=sumc3+c3[dd];
        sumc4=sumc4+c4[dd];
        sumc5=sumc5+c5[dd];
        sumc6=sumc6+c6[dd];
        sumc7=sumc7+c7[dd];
        sumc8=sumc8+c8[dd];
        sumc9=sumc9+c9[dd];
      } 
      fprintf(fp2, "\n");
      fprintf(fp2, "score sum statistics:\n");
	  fprintf(fp2, "\n");
      fprintf(fp2, "sumScore0Counts=%d sumScore1Counts=%d\n",
        sumScore0Counts, sumScore1Counts);
      fprintf(fp2, "nodeSetSize=%d  winSetSize=%d\n", nodeSetSize,
        winSetSize);
      fprintf(fp2, "sumc1=%d sumc2=%d sumc3=%d sumc4=%d\n",
        sumc1, sumc2, sumc3, sumc4);
      fprintf(fp2, "sumc5=%d sumc6=%d sumc7=%d sumc8=%d sumc9=%d\n",
        sumc5, sumc6, sumc7, sumc8, sumc9);
	  fprintf(fp2, "\n");	
      fprintf(fp2, "\n");
      fprintf(fp2, "No of searched nodes per depth:\n");
      for (dd=iniDepth; dd>=0; dd--)
        fprintf(fp2, "depth=%d  nodes=%d\n", dd, no[dd]);
	  fprintf(fp2, "\n");
      fprintf(fp2, "Total nodes=%d\n", nodes);
    }
    #endif
    
  return value;
}

struct makeType Make(struct pos * posPoint, int depth, int ncount)  {
  int r, s, t, u, w, firstHand;
  int suit, rank, count, mcurr, h, q, done;
  struct makeType trickCards;

  for (suit=0; suit<=3; suit++)
    trickCards.winRanks[suit]=0;

  firstHand=posPoint->first[depth];
  r=movePly[depth].current;

  if (posPoint->handRelFirst==3)  {         /* This hand is last hand */
    if (WinningMove(&movePly[depth].move[r], &(posPoint->move[depth+1]))) {
      posPoint->move[depth]=movePly[depth].move[r];
      posPoint->high[depth]=handStore[firstHand][3];
    }
    else {
      posPoint->move[depth]=posPoint->move[depth+1];
      posPoint->high[depth]=posPoint->high[depth+1];
    }

    /* Is the trick won by rank? */
    suit=posPoint->move[depth].suit;
    rank=posPoint->move[depth].rank;
    count=0;
    for (h=0; h<=3; h++) {
      mcurr=movePly[depth+h].current;
      if (movePly[depth+h].move[mcurr].suit==suit)
          count++;
    }

    if (nodeTypeStore[posPoint->high[depth]]==MAXNODE)
      posPoint->tricksMAX++;
    posPoint->first[depth-1]=posPoint->high[depth];   /* Defines who is first
    in the next move */

    t=handStore[firstHand][3];
    posPoint->handRelFirst=0;      /* Hand pointed to by posPoint->first
                                    will lead the next trick */

    done=FALSE;
    for (s=3; s>=0; s--) {
      q=handStore[firstHand][3-s];
    /* Add the moves to removed ranks */
      r=movePly[depth+s].current;
      w=movePly[depth+s].move[r].rank;
      u=movePly[depth+s].move[r].suit;
      posPoint->removedRanks[u]=posPoint->removedRanks[u] | bitMapRank[w];

      if (s==0)
        posPoint->rankInSuit[t][u]=posPoint->rankInSuit[t][u] &
          (~bitMapRank[w]);

      if (w==posPoint->winner[u].rank) 
        UpdateWinner(posPoint, u);
      else if (w==posPoint->secondBest[u].rank)
        UpdateSecondBest(posPoint, u);

    /* Determine win-ranked cards */
      if ((q==posPoint->high[depth])&&(!done)) {
        done=TRUE;
        if (count>=2) {
          trickCards.winRanks[u]=bitMapRank[w];
          /* Mark ranks as winning if they are part of a sequence */
          trickCards.winRanks[u]=trickCards.winRanks[u]
            | movePly[depth+s].move[r].sequence; 
        }
      }
    }
  }
  else if (posPoint->handRelFirst==0) {   /* Is it the 1st hand? */
    posPoint->first[depth-1]=firstHand;   /* First hand is not changed in
                                            next move */
    posPoint->high[depth]=firstHand;
    posPoint->move[depth]=movePly[depth].move[r];
    t=firstHand;
    posPoint->handRelFirst=1;
    r=movePly[depth].current;
    u=movePly[depth].move[r].suit;
    w=movePly[depth].move[r].rank;
    posPoint->rankInSuit[t][u]=posPoint->rankInSuit[t][u] &
      (~bitMapRank[w]);
  }
  else {
    if (WinningMove(&movePly[depth].move[r], &(posPoint->move[depth+1]))) {
      posPoint->move[depth]=movePly[depth].move[r];
      posPoint->high[depth]=handStore[firstHand][posPoint->handRelFirst];
    }
    else {
      posPoint->move[depth]=posPoint->move[depth+1];
      posPoint->high[depth]=posPoint->high[depth+1];
    }
    
    t=handStore[firstHand][posPoint->handRelFirst];
    posPoint->handRelFirst++;               /* Current hand is stepped */
    posPoint->first[depth-1]=firstHand;     /* First hand is not changed in
                                            next move */
    r=movePly[depth].current;
    u=movePly[depth].move[r].suit;
    w=movePly[depth].move[r].rank;
    posPoint->rankInSuit[t][u]=posPoint->rankInSuit[t][u] &
      (~bitMapRank[w]);
  }

  posPoint->length[t][u]--;

  if (ncount) {
    nodes++;
    no[depth]++;
  }  
  return trickCards;
}


void Undo(struct pos * posPoint, int depth)  {
  int r, s, t, u, w, firstHand;

  firstHand=posPoint->first[depth];

  switch (posPoint->handRelFirst) {
    case 3: case 2: case 1:
     posPoint->handRelFirst--;
     break;
    case 0:
     posPoint->handRelFirst=3;
  }

  if (posPoint->handRelFirst==0) {          /* 1st hand which won the previous
                                            trick */
    t=firstHand;
    r=movePly[depth].current;
    u=movePly[depth].move[r].suit;
    w=movePly[depth].move[r].rank;
  }
  else if (posPoint->handRelFirst==3)  {    /* Last hand */
    for (s=3; s>=0; s--) {
    /* Delete the moves from removed ranks */
      r=movePly[depth+s].current;
      w=movePly[depth+s].move[r].rank;
      u=movePly[depth+s].move[r].suit;
      posPoint->removedRanks[u]=posPoint->removedRanks[u] & (~bitMapRank[w]);

      if (w>posPoint->winner[u].rank) {
        posPoint->secondBest[u].rank=posPoint->winner[u].rank;
        posPoint->secondBest[u].hand=posPoint->winner[u].hand;
        posPoint->winner[u].rank=w;
        posPoint->winner[u].hand=handStore[firstHand][3-s];
      }
      else if (w>posPoint->secondBest[u].rank) {
        posPoint->secondBest[u].rank=w;
        posPoint->secondBest[u].hand=handStore[firstHand][3-s];
      }
    }
    t=handStore[firstHand][3];

        
    if (nodeTypeStore[posPoint->first[depth-1]]==MAXNODE)   /* First hand
                                            of next trick is winner of the
                                            current trick */
      posPoint->tricksMAX--;
  }
  else {
    t=handStore[firstHand][posPoint->handRelFirst];
    r=movePly[depth].current;
    u=movePly[depth].move[r].suit;
    w=movePly[depth].move[r].rank;
  }    

  posPoint->rankInSuit[t][u]=posPoint->rankInSuit[t][u] |
    bitMapRank[w];

  posPoint->length[t][u]++;

  return;
}



struct evalType Evaluate(struct pos * posPoint)  {
  int s, smax=0, max, k, firstHand, count;
  struct evalType eval;

  firstHand=posPoint->first[0];

  for (s=0; s<=3; s++)
    eval.winRanks[s]=0;

  /* Who wins the last trick? */
  if (trumpContract)  {            /* Highest trump card wins */
    max=0;
    count=0;
    for (s=0; s<=3; s++) {
      if (posPoint->rankInSuit[s][trump]!=0)
        count++;
      if (posPoint->rankInSuit[s][trump]>max) {
        smax=s;
        max=posPoint->rankInSuit[s][trump];
      }
    }

    if (max>0) {        /* Trumpcard wins */
      if (count>=2)
        eval.winRanks[trump]=max;

      if (nodeTypeStore[smax]==MAXNODE)
        goto maxexit;
      else
        goto minexit;
    }
  }

  /* Who has the highest card in the suit played by 1st hand? */

  k=0;
  while (k<=3)  {           /* Find the card the 1st hand played */
    if (posPoint->rankInSuit[firstHand][k]!=0)      /* Is this the card? */
      break;
    k++;
  }    

  count=0;
  max=0; 
  for (s=0; s<=3; s++)  {
    if (posPoint->rankInSuit[s][k]!=0)
        count++;
    if (posPoint->rankInSuit[s][k]>max)  {
      smax=s;
      max=posPoint->rankInSuit[s][k];
    }
  }

  if (count>=2)
    eval.winRanks[k]=max;

  if (nodeTypeStore[smax]==MAXNODE)
    goto maxexit;
  else
    goto minexit;

  maxexit:
  eval.tricks=posPoint->tricksMAX+1;
  return eval;

  minexit:
  eval.tricks=posPoint->tricksMAX;
  return eval;
}



void UpdateWinner(struct pos * posPoint, int suit) {
  int k, h, hmax=0, flag;

  posPoint->winner[suit]=posPoint->secondBest[suit];

  k=posPoint->secondBest[suit].rank-1;
    while (k>=2) {
      flag=TRUE;
      for (h=0; h<=3; h++)
        if ((posPoint->rankInSuit[h][suit] & bitMapRank[k]) != 0) {
          hmax=h;
          flag=FALSE;
          break;
        }
      if (flag)
        k--;
      else
        break;
    }
    if (k<2) {
      posPoint->secondBest[suit].rank=0;
      posPoint->secondBest[suit].hand=0;
    }
    else {
      posPoint->secondBest[suit].rank=k;
      posPoint->secondBest[suit].hand=hmax;
    }
  return;
}


void UpdateSecondBest(struct pos * posPoint, int suit) {
  int k, h, hmax=0, flag;

    k=posPoint->secondBest[suit].rank-1;
    while (k>=2) {
      flag=TRUE;
      for (h=0; h<=3; h++)
        if ((posPoint->rankInSuit[h][suit] & bitMapRank[k]) != 0) {
          hmax=h;
          flag=FALSE;
          break;
        }
      if (flag)
        k--;
      else
        break;
    }
    if (k<2) {
      posPoint->secondBest[suit].rank=0;
      posPoint->secondBest[suit].hand=0;
    }
    else {
      posPoint->secondBest[suit].rank=k;
      posPoint->secondBest[suit].hand=hmax;
    }
  return;
}

struct quickType QuickTricks(struct pos * posPoint, 
  int hand, int depth, int * out, int *hout, int *sout) {
  struct quickType quick;
  int suit, qtricks, commPartner, commSuit, h, s, r;
  int other, currHand, trumpFlag  /* Set before trump is investigated */;
  int countLho, countRho, countPart, countOwn, lhoTrumpRanks=0, rhoTrumpRanks=0;
  
  qtricks=0;
  for (s=0; s<=3; s++)
    quick.winRanks[s]=0;

  if (trumpContract)
    trumpFlag=TRUE;
  else
    trumpFlag=FALSE;
      
  commPartner=FALSE;
  for (s=0; s<=3; s++) {
    if ((trumpContract)&&(trump!=s)) {
      if ((posPoint->rankInSuit[partner[hand]][s] &
        bitMapRank[posPoint->winner[s].rank])!=0) {
        /* Partner has winning card */
        if (posPoint->rankInSuit[hand][s]!=0) {
        /* Own hand has card in suit */
          if (((posPoint->rankInSuit[lho[hand]][s]!=0) ||
          /* LHO not void */
          (posPoint->rankInSuit[lho[hand]][trump]==0))
          /* LHO has no trump */
          && ((posPoint->rankInSuit[rho[hand]][s]!=0) ||
          /* RHO not void */
          (posPoint->rankInSuit[rho[hand]][trump]==0))) {
          /* RHO has no trump */
            commPartner=TRUE;
            commSuit=s;
            break;
          }  
        }
      }
    }
    else {
      if ((posPoint->rankInSuit[partner[hand]][s] &
        bitMapRank[posPoint->winner[s].rank])!=0) {
        /* Partner has winning card */
        if (posPoint->rankInSuit[hand][s]!=0) {
        /* Own hand has card in suit */
          commPartner=TRUE;
          commSuit=s;
          break;
        }
      }
    }
  }

  if (trumpContract) {
    lhoTrumpRanks=posPoint->length[lho[hand]][trump];
    rhoTrumpRanks=posPoint->length[rho[hand]][trump];
  }
  
  if (trumpFlag)
    suit=trump;
  else if (trumpContract && (trump==0))
    suit=1;
  else
    suit=0;

  do {
    other=FALSE;
    for (r=14; r>=2; r--) {
      for (h=0; h<=3; h++) {
        if ((h!=hand)&&((posPoint->rankInSuit[h][suit]
          & bitMapRank[r])!=0)) {
          other=TRUE;
          break;
        }
      }
      if (other)
        break;
    }
    if (!other) {
      if (trumpContract && (trump!=suit)) {
        if ((lhoTrumpRanks==0) &&
          /* LHO has no trump */
          (rhoTrumpRanks==0)) {
          /* RHO has no trump */
          qtricks=qtricks+posPoint->length[hand][suit];
          suit++;
          if (trumpContract && (suit==trump))
            suit++;
          continue;
        }
        else {
          suit++;
          if (trumpContract && (suit==trump))
            suit++;
          continue;
        }
      }
      else {
        qtricks=qtricks+posPoint->length[hand][suit];
        if (trumpFlag) {
          if (trump==0)
            suit=1;
          else
            suit=0;
          trumpFlag=FALSE;
        }
        else {
          suit++;
          if (trumpContract && (suit==trump))
            suit++;
        }
        continue;
      }
    }
    else if (commPartner) {
      currHand=partner[hand];
      other=FALSE;
      for (r=14; r>=2; r--) {
        for (h=0; h<=3; h++) {
          if ((h!=currHand)&&((posPoint->rankInSuit[h][suit]
            & bitMapRank[r])!=0)) {
            other=TRUE;
            break;
          }
        }
        if (other)
          break;
      }
      if (!other) {
        if (trumpContract && (trump!=suit)) {
          if ((lhoTrumpRanks==0) &&
            /* LHO has no trump */
          (rhoTrumpRanks==0)) {
          /* RHO has no trump */
            qtricks=qtricks+
              posPoint->length[currHand][suit];
            suit++;
            if (trumpContract && (suit==trump))
              suit++;
            continue;
          }
          else {
            suit++;
            if (trumpContract && (suit==trump))
              suit++;
            continue;
          }
        }
        else {
          qtricks=qtricks+
            posPoint->length[currHand][suit];
          if (trumpFlag) {
            if (trump==0)
              suit=1;
            else
              suit=0;
            trumpFlag=FALSE;
          }
          else {
            suit++;
            if (trumpContract && (suit==trump))
              suit++;
          }
          continue;
        }
      }
    }

    if ((posPoint->rankInSuit[hand][suit] &
      bitMapRank[posPoint->winner[suit].rank])!=0) {
      /* Winner found in own hand */
      countLho=posPoint->length[lho[hand]][suit];
      countRho=posPoint->length[rho[hand]][suit];
      countPart=posPoint->length[partner[hand]][suit];
      if ((trumpContract)&&(trump!=suit)) {
        if (((countLho!=0) ||
        /* LHO not void */
        (lhoTrumpRanks==0))
        /* LHO has no trump */
         && ((countRho!=0) ||
         /* RHO not void */
         (rhoTrumpRanks==0)))
         /* RHO has no trump */
        {
          quick.winRanks[suit]=quick.winRanks[suit]
            | bitMapRank[posPoint->winner[suit].rank];
          qtricks++;   /* A trick can be taken */
          if ((countLho<=1)&&(countRho<=1)&&(countPart<=1)&&
            (lhoTrumpRanks==0)&&
            (rhoTrumpRanks==0)) {
            qtricks=qtricks+
              posPoint->length[hand][suit]-1;
            suit++;
            if (trumpContract && (suit==trump))
              suit++;
            continue;
          }
        }
        if ((posPoint->rankInSuit[hand][suit] &
          bitMapRank[posPoint->secondBest[suit].rank])!=0) {
          /* Second best found in own hand */
          if ((lhoTrumpRanks==0)&&
             (rhoTrumpRanks==0)) {
          /* Opponents have no trump */
            quick.winRanks[suit]=quick.winRanks[suit]
              | bitMapRank[posPoint->secondBest[suit].rank];
            qtricks++;
            if ((countLho<=2)&&(countRho<=2)&&(countPart<=2)) {
              qtricks=qtricks+
                posPoint->length[hand][suit]-2;
              suit++;
              if (trumpContract && (suit==trump))
                suit++;
              continue;
            }
          }
        }
      }
      else {
        quick.winRanks[suit]=quick.winRanks[suit]
           | bitMapRank[posPoint->winner[suit].rank];
        qtricks++;
        if ((countLho<=1)&&(countRho<=1)&&(countPart<=1)) {
          qtricks=qtricks+posPoint->length[hand][suit]-1;
          if (trumpFlag) {
            if (trump==0)
              suit=1;
            else
              suit=0;
            trumpFlag=FALSE;
            lhoTrumpRanks=0;
            rhoTrumpRanks=0;
          }
          else {
            suit++;
            if (trumpContract && (suit==trump))
              suit++;
          }
          continue;
        }
        else if (trumpContract && (suit==trump)) {
          if (lhoTrumpRanks!=0)
            lhoTrumpRanks--;
          if (rhoTrumpRanks!=0)
            rhoTrumpRanks--;
          trumpFlag=FALSE;
        }
        if ((posPoint->rankInSuit[hand][suit] &
          bitMapRank[posPoint->secondBest[suit].rank])!=0) {
          /* Second best found in own hand */
          quick.winRanks[suit]=quick.winRanks[suit]
              | bitMapRank[posPoint->secondBest[suit].rank];
          qtricks++;
          if ((countLho<=2)&&(countRho<=2)&&(countPart<=2)) {
            qtricks=qtricks+posPoint->length[hand][suit]-2;
            if ((trumpContract && (suit==trump))) {
              if (trump==0)
                suit=1;
              else
                suit=0;
              trumpFlag=FALSE;
              lhoTrumpRanks=0;
              rhoTrumpRanks=0;
            }
            else {
              suit++;
              if (trumpContract && (suit==trump))
                suit++;
            }
            continue;
          }
          else if (trumpContract && (suit==trump)) {
            if (lhoTrumpRanks!=0)
              lhoTrumpRanks--;
            if (rhoTrumpRanks!=0)
              rhoTrumpRanks--;
            trumpFlag=FALSE;
          }
        }
      }
    }
    /* It was not possible to take a quick trick by own winning card in
    the suit */
    else {
    /* Partner winning card? */
      if ((posPoint->rankInSuit[partner[hand]][suit] &
        bitMapRank[posPoint->winner[suit].rank])!=0) {
        /* Winner found at partner*/
        if (commPartner) {
          countLho=posPoint->length[lho[hand]][suit];
          countRho=posPoint->length[rho[hand]][suit];
          countOwn=posPoint->length[hand][suit];
        /* There is communication with the partner */
          if ((trumpContract)&&(trump!=suit)) {
            if (((posPoint->rankInSuit[lho[hand]][suit]!=0) ||
              /* LHO not void */
            (lhoTrumpRanks==0))
              /* LHO has no trump */
             && ((posPoint->rankInSuit[rho[hand]][suit]!=0) ||
              /* RHO not void */
             (rhoTrumpRanks==0)))
              /* RHO has no trump */
              {
                quick.winRanks[suit]=quick.winRanks[suit]
                  | bitMapRank[posPoint->winner[suit].rank];
                qtricks++;   /* A trick can be taken */
                if ((countLho<=1)&&(countRho<=1)&&(countOwn<=1)&&
                  (lhoTrumpRanks==0)&&
                  (rhoTrumpRanks==0)) {
                   qtricks=qtricks+
                     posPoint->length[partner[hand]][suit]-1;
                   suit++;
                   if (trumpContract && (suit==trump))
                     suit++;
                   continue;
                }
              }
              if ((posPoint->rankInSuit[partner[hand]][suit] &
                bitMapRank[posPoint->secondBest[suit].rank])!=0) {
               /* Second best found in partners hand */
                if ((lhoTrumpRanks==0)&&
                 (rhoTrumpRanks==0)) {
                /* Opponents have no trump */
                  quick.winRanks[suit]=quick.winRanks[suit]
                    | bitMapRank[posPoint->secondBest[suit].rank];
                  qtricks++;
                  if ((countLho<=2)&&(countRho<=2)&&(countOwn<=2)) {
                    qtricks=qtricks+
                      posPoint->length[partner[hand]][suit]-2;
                    suit++;
                    if (trumpContract && (suit==trump))
                      suit++;
                    continue;
                  }
                }
              }
          }
          else {
            quick.winRanks[suit]=quick.winRanks[suit]
                 | bitMapRank[posPoint->winner[suit].rank];
            qtricks++;
            if ((countLho<=1)&&(countRho<=1)&&(countOwn<=1)) {
              qtricks=qtricks+
                posPoint->length[partner[hand]][suit]-1;
              if (trumpFlag) {
                if (trump==0)
                  suit=1;
                else
                  suit=0;
                trumpFlag=FALSE;
              }
              else {
                suit++;
                if (trumpContract && (suit==trump))
                  suit++;
              }
              continue;
            }
            if ((posPoint->rankInSuit[partner[hand]][suit] &
              bitMapRank[posPoint->secondBest[suit].rank])!=0) {
              /* Second best found in partners hand */
              quick.winRanks[suit]=quick.winRanks[suit]
                    | bitMapRank[posPoint->secondBest[suit].rank];
              qtricks++;
              if ((countLho<=2)&&(countRho<=2)&&(countOwn<=2)) {
                qtricks=qtricks+
                  posPoint->length[partner[hand]][suit]-2;
                if (trumpContract && (suit==trump)) {
                  if (trump==0)
                    suit=1;
                  else
                    suit=0;
                  trumpFlag=FALSE;
                }
                else {
                  suit++;
                  if (trumpContract && (suit==trump))
                    suit++;
                }
                continue;
              }
            }
          }
        }
      }
    }
    if (trumpFlag) {
      if (trump==0)
        suit=1;
      else
        suit=0;
      trumpFlag=FALSE;
    }
    else {
      suit++;
      if (trumpContract && (suit==trump))
        suit++;
    }
  }
  while (suit<=3);
  quick.quickTricks=qtricks;
  return quick;
}

unsigned short ris;
int q, first;
int recInd=0;

int MoveGen(struct pos * posPoint, int depth) {
  int suit, k, m, n, r, s, t, state;
  int scount[4];
  int WeightAlloc(struct pos *, struct moveType * mp, int depth,
    int notVoidInSuit);

  m=0;
  r=posPoint->handRelFirst;
  first=posPoint->first[depth];
  q=handStore[first][r];
  
  s=movePly[depth+r].current;             /* Current move of first hand */
  t=movePly[depth+r].move[s].suit;        /* Suit played by first hand */
  ris=posPoint->rankInSuit[q][t];

  if ((r!=0)&&(ris!=0)) {
  /* Not first hand and not void in suit */
    k=14;   state=MOVESVALID;
    while (k>=2) {
      if ((ris & bitMapRank[k])&&(state==MOVESVALID)) {
           /* Only first move in sequence is generated */
        movePly[depth].move[m].suit=t;
        movePly[depth].move[m].rank=k;
        movePly[depth].move[m].sequence=0;
        m++;
        state=MOVESLOCKED;
      }
      else if (state==MOVESLOCKED)
        if ((posPoint->removedRanks[t] & bitMapRank[k])==0) {
          if ((ris & bitMapRank[k])==0)
          /* If the card still exists and it is not in own hand */
            state=MOVESVALID;
          else
          /* If the card still exists and it is in own hand */
            movePly[depth].move[m-1].sequence=
              movePly[depth].move[m-1].sequence | bitMapRank[k];
        }
      k--;
    }
    if (m!=1) {
      for (k=0; k<=m-1; k++) 
        movePly[depth].move[k].weight=WeightAlloc(posPoint,
          &movePly[depth].move[k], depth, ris);
    }

    movePly[depth].last=m-1;
    if (m!=1)
      InsertSort(m, depth);
    if (depth!=iniDepth)
      return m;
    else {
      m=AdjustMoveList();
      return m;
    }
  }
  else {                  /* First hand or void in suit */
    for (suit=0; suit<=3; suit++)  {
      k=14;  state=MOVESVALID;
      while (k>=2) {
        if ((posPoint->rankInSuit[q][suit] & bitMapRank[k])&&
            (state==MOVESVALID)) {
           /* Only first move in sequence is generated */
          movePly[depth].move[m].suit=suit;
          movePly[depth].move[m].rank=k;
          movePly[depth].move[m].sequence=0;
          m++;
          state=MOVESLOCKED;
        }
        else if (state==MOVESLOCKED)
          if ((posPoint->removedRanks[suit] & bitMapRank[k])==0) {
            if ((posPoint->rankInSuit[q][suit] & bitMapRank[k])==0)
            /* If the card still exists and it is not in own hand */
              state=MOVESVALID;
            else
            /* If the card still exists and it is in own hand */
              movePly[depth].move[m-1].sequence=
                movePly[depth].move[m-1].sequence | bitMapRank[k];
          }
        k--;
      }
    }

    for (k=0; k<=m-1; k++) 
        movePly[depth].move[k].weight=WeightAlloc(posPoint,
          &movePly[depth].move[k], depth, ris);
  
    movePly[depth].last=m-1;
    InsertSort(m, depth);
    if (r==0) {
      for (n=0; n<=3; n++)
        scount[n]=0;
      for (k=0; k<=m-1; k++) {
        if (scount[movePly[depth].move[k].suit]==2) 
          continue;
        else {
          movePly[depth].move[k].weight+=500;
          scount[movePly[depth].move[k].suit]++;
        }
      }
      InsertSort(m, depth);
    }
    else {
      for (n=0; n<=3; n++)
        scount[n]=0;
      for (k=0; k<=m-1; k++) {
        if (scount[movePly[depth].move[k].suit]==1) 
          continue;
        else {
          movePly[depth].move[k].weight+=500;
          scount[movePly[depth].move[k].suit]++;
        }
      }
      InsertSort(m, depth);
    }
    if (depth!=iniDepth)
     return m;
    else {
      m=AdjustMoveList();
      return m;
    }  
  }
}


int WeightAlloc(struct pos * posPoint, struct moveType * mp, int depth,
  int notVoidInSuit) {
  int weight=0, s, k, r, suit, suitAdd=0;
  int suitWeightDelta, winnerRank, winnerHand, secondBestHand,
    secondBestRank;
  int suitBonus=0;
  int index0, index1, index2, cho1, cho2, cho3;
  unsigned short suitCount, suitCountPartner, suitCountLH, suitCountRH;
  unsigned short left, part, right, uni, temp1, temp2;
  int countLH, countRH;

  suit=mp->suit;

  if ((!notVoidInSuit)||(posPoint->handRelFirst==0)) {
    suitCount=posPoint->length[q][suit];
    suitAdd=suitCount+suitCount;
  }

  switch (posPoint->handRelFirst) {
    case 0:
      suitCountLH=posPoint->length[lho[q]][suit];
      suitCountRH=posPoint->length[rho[q]][suit];
      suitCountPartner=posPoint->length[partner[q]][suit];

      if ((trumpContract && (suit!=trump)) &&
        ((posPoint->rankInSuit[lho[q]][suit]==0) &&
        (posPoint->rankInSuit[lho[q]][trump]!=0)) ||
        ((posPoint->rankInSuit[rho[q]][suit]==0) &&
        (posPoint->rankInSuit[rho[q]][trump]!=0)))
          suitBonus=-12/*15*/;

      if (suitCountLH!=0)
        countLH=(suitCountLH<<2);
      else
        countLH=depth+4;
      if (suitCountRH!=0)
        countRH=(suitCountRH<<2);
      else
        countRH=depth+4;

      suitWeightDelta=suitBonus-((countLH+countRH)<<1);

      if (posPoint->winner[suit].hand==first) {
        /* 04-02-29: */
        if (suitCountPartner==1) {
          if (posPoint->secondBest[suit].hand==partner[first])
            weight=suitWeightDelta+35/*20*/-(mp->rank);
          else if (posPoint->winner[suit].rank==mp->rank) 
            weight=suitWeightDelta+16;
          else
            weight=suitWeightDelta+4-(mp->rank);
        }
        else if (posPoint->secondBest[suit].hand==partner[first])
          weight=suitWeightDelta+35/*20*/-(mp->rank);
        else if (posPoint->winner[suit].rank==mp->rank) 
          weight=suitWeightDelta+16;
        else
          weight=suitWeightDelta+4-(mp->rank);
      }
      else if (posPoint->winner[suit].hand==partner[first]) {
        /* If partner has winning card */
        if (posPoint->secondBest[suit].hand==first)
           weight=suitWeightDelta+35/*20*/-(mp->rank);
        else 
          weight=suitWeightDelta+20-(mp->rank);  
      }  
      else if (((posPoint->winner[suit].hand==lho[first])&&(suitCountLH==1))
        ||((posPoint->winner[suit].hand==rho[first])&&(suitCountRH==1)))
        weight=suitWeightDelta+20-(mp->rank);
      else if ((mp->sequence)&&
        (mp->rank==posPoint->secondBest[suit].rank)) 
        weight=suitWeightDelta+12-(mp->rank);
      else 
        weight=suitWeightDelta+4-(mp->rank);
      if ((bestMove[depth].suit==mp->suit)&&
        (bestMove[depth].rank==mp->rank)) 
        weight=weight+35;
      break;

    case 1:
      if (!notVoidInSuit) {
        k=posPoint->high[depth];
        if (WinningMove(mp, &(posPoint->move[depth+1]))) {
          /* Ruff */
          weight=40/*35*/-(mp->rank)+suitAdd;
        }
        else if (trumpContract && (suit==trump)) 
          weight=-(mp->rank)+suitAdd;
        else 
          weight=14-(mp->rank)+suitAdd;
      }
      else {
        winnerHand=posPoint->winner[suit].hand;
        winnerRank=posPoint->winner[suit].rank;
        secondBestHand=posPoint->secondBest[suit].hand;
        secondBestRank=posPoint->secondBest[suit].rank;
        if (posPoint->move[depth+1].rank==secondBestRank) {
          if ((mp->rank)==(posPoint->winner[suit].rank))
            weight=20;
          else
            weight=14-(mp->rank);
        }
        else {
          /* Modify winner, secondBest? */

          left=posPoint->rankInSuit[lho[first]][suit];
          part=posPoint->rankInSuit[partner[first]][suit];
          right=posPoint->rankInSuit[rho[first]][suit];

          if (left>part) {
            if (part>right) {
              winnerHand=lho[first];
              secondBestHand=partner[first];
            }
            else if (left>right) {
              winnerHand=lho[first];
              secondBestHand=rho[first];
            }
            else {
              winnerHand=rho[first];
              secondBestHand=lho[first];
            }
          }
          else if (right>part) {
            winnerHand=rho[first];
            secondBestHand=partner[first];
          }
          else if (left>right) {
            winnerHand=partner[first];
            secondBestHand=lho[first];
          }
          else {
            winnerHand=partner[first];
            secondBestHand=rho[first];
          }

          if (bitMapRank[posPoint->move[depth+1].rank]>
            posPoint->rankInSuit[secondBestHand][suit]) {
            if (WinningMove(mp, &(posPoint->move[depth+1]))) {
              weight=30-(mp->rank);
              break;
            }
            else {
              weight=14-(mp->rank);
              break;
            }
          }

          uni=left|part|right;
          for (k=14; k>=2; k--) {
            if ((uni & bitMapRank[k])!=0) {
              winnerRank=k;
              break;
            }
          }

          index0=IOTHER;
          index1=IOTHER;
          index2=IOTHER;
          if (winnerHand==lho[first]) {
		index0=IWINNER;
          }
          else if (secondBestHand==lho[first])
            index0=ISECONDBEST;

          if (winnerHand==partner[first]) {
            if (posPoint->length[partner[first]][suit]==1)
                index1=IONLYW;
            else
                index1=IWINNER;
          }
          else if (secondBestHand==partner[first]) {
            if (posPoint->length[partner[first]][suit]==1)
              index1=IONLYS;
            else
              index1=ISECONDBEST;
          }

          if (winnerHand==rho[first]) {
            if (posPoint->length[rho[first]][suit]==1)
                index2=IONLYW;
            else
                index2=IWINNER;
          }
          else if (secondBestHand==rho[first]) {
            if (posPoint->length[rho[first]][suit]==1)
              index2=IONLYS;
            else
              index2=ISECONDBEST;
          }

          cho1=heurTable->cho1[index0][index1][index2];
          cho2=heurTable->cho2[index0][index1][index2];
          cho3=heurTable->cho3[index0][index1][index2];

          if (winnerRank==mp->rank) {
            if (cho1==WINNER)
              weight=30-(mp->rank);
            else if (cho2==WINNER)
              weight=14-(mp->rank);
            else if (cho3==WINNER)
              weight=-(mp->rank);
            else
              weight=0;
          }
          else if ((mp->rank) > posPoint->move[depth+1].rank) {
            if (cho1==UNCLEAR)
              weight=30-(mp->rank);
            else if (cho2==UNCLEAR)
              weight=14-(mp->rank);
            else if (cho3==UNCLEAR)
              weight=-(mp->rank);
            else
              weight=0;
            if ((mp->sequence)!=0)
              weight=weight+4;
          }
          else {
            if (cho1==LOSER)
              weight=30-(mp->rank);
            else if (cho2==LOSER)
              weight=14-(mp->rank);
            else if (cho3==LOSER)
              weight=-(mp->rank);
            else
              weight=0;
          }
        }
      }
      break;

    case 2:
      if (!notVoidInSuit) {
        if (first==posPoint->high[depth+1]) {
          /* Partner on top */
          if (bitMapRank[posPoint->move[depth+1].rank] >
            posPoint->rankInSuit[rho[first]][posPoint->move[depth+1].suit]) {
            /* Partner beats also 2nd opponent? */
            if ((trumpContract)&&(suit==trump)) {
              s=posPoint->move[depth+1].suit;
              if (s!=suit)
                /* Ruffs partners winner */
                weight=-(mp->rank)+suitAdd;
              else
                weight=30-(mp->rank)+suitAdd;
            }
            else
              weight=30-(mp->rank)+suitAdd;
          }
          else if (trumpContract && (suit==trump)) {
            s=posPoint->move[depth+1].suit;
            if (s!=suit)
              /* Own hand is likely to win */
              weight=30-(mp->rank)+suitAdd;
            else
              weight=14-(mp->rank)+suitAdd;
          }
          else
            weight=14-(mp->rank)+suitAdd;
        }
        else {
          if (WinningMove(mp, &(posPoint->move[depth+1]))) 
          /* Own hand on top by ruffing? */
            weight=40/*30*/-(mp->rank)+suitAdd;
          else if (trumpContract && (suit==trump))
            /* Discard a trump but still losing */
            weight=-(mp->rank)+suitAdd;
          else
            weight=14-(mp->rank)+suitAdd;
        }
      }
      else if (first==posPoint->high[depth+1]) {
        /* Partner on top */
        if (bitMapRank[posPoint->move[depth+1].rank] >
          posPoint->rankInSuit[rho[first]][suit])
          /* Partner higher than RHO */
          weight=14-(mp->rank);
        else {
          if (posPoint->rankInSuit[partner[first]][suit] >
             posPoint->rankInSuit[rho[first]][suit]) {
             /* We have a higher card than RHO */
             if (bitMapRank[mp->rank] >
               posPoint->rankInSuit[rho[first]][suit])
               weight=40/*30*/-(mp->rank);
             else if ((mp->sequence) &&
               (mp->rank > posPoint->move[depth+1].rank))
               weight=20-(mp->rank);
             else
               weight=14-(mp->rank);
          }
          else {
             for (r=14; r>=2; r--) {
               if ((posPoint->rankInSuit[rho[first]][suit]
                 & bitMapRank[r])!=0)
                 break;
             }
             temp1=posPoint->rankInSuit[rho[first]][suit] & (~bitMapRank[r]);
             temp2=posPoint->rankInSuit[partner[first]][suit] &
               (~bitMapRank[mp->rank]);
             if (temp2 > temp1)
               weight=30-(mp->rank);
             else
               weight=14-(mp->rank);
          }
        }
      }
      /* 1st opponent on top */
      else if (posPoint->length[rho[first]][suit]==1) {
        if (posPoint->rankInSuit[partner[first]][suit]<
          posPoint->rankInSuit[rho[first]][suit])
          weight=14-(mp->rank);   /* RHO wins always */
        else if (WinningMove(mp, &(posPoint->move[depth+1]))) {
          if (bitMapRank[mp->rank]>posPoint->rankInSuit[rho[first]][suit])
            weight=35/*30*/-(mp->rank);
          else 
	      weight=21-(mp->rank);          
        }
        else
          weight=14/*0*/-(mp->rank);
      }
      else if (WinningMove(mp, &(posPoint->move[depth+1]))) {
        if (bitMapRank[mp->rank]>posPoint->rankInSuit[rho[first]][suit])
          weight=30-(mp->rank);
        else if (mp->sequence)
          weight=20-(mp->rank);
        else
          weight=14-(mp->rank);
      }
      else
        /* Losing card */
        weight=-(mp->rank);
      break;

    case 3:
      if (!notVoidInSuit) {
        if ((posPoint->high[depth+1])==lho[first]) {
          /* If the current winning move is given by the partner */
          if (trumpContract && (suit==trump))
            /* Ruffing partners winner? */
            weight=14-(mp->rank)+suitAdd;
          else 
            weight=30-(mp->rank)+suitAdd;
        }
        else if (WinningMove(mp, &(posPoint->move[depth+1]))) 
          /* Own hand ruffs */
          weight=30-(mp->rank)+suitAdd;
        else if (suit==trump) 
          weight=-(mp->rank);
        else 
          weight=14-(mp->rank)+suitAdd;  
      }
      else if ((posPoint->high[depth+1])==(lho[first])) {
        /* If the current winning move is given by the partner */
        if (trumpContract && (suit==trump))
        /* Ruffs partners winner */
          weight=24-(mp->rank);
        else 
          weight=30-(mp->rank);
      }
      else if (WinningMove(mp, &(posPoint->move[depth+1]))) 
        /* If present move is superior to current winning move and the
        current winning move is not given by the partner */
        weight=30-(mp->rank);
      else {
        /* If present move is not superior to current winning move and the
        current winning move is not given by the partner */
        if (trumpContract && (suit==trump))
          /* Ruffs but still loses */
          weight=-(mp->rank);
        else 
          weight=14-(mp->rank);
      }
  }
  return weight;
}



int Min(int a, int b) {
  if (a<=b)
    return a;
  else
    return b;
}


int Max(int a, int b) {
  if (a>=b)
    return a;
  else
    return b;
}


/* Shell-1 */
/* K&R page 62: */
/*void shellSort(int n, int depth) {
  int gap, i, j;
  struct moveType temp;

  if (n==2) {
    if (movePly[depth].move[0].weight<movePly[depth].move[1].weight) {
      temp=movePly[depth].move[0];
      movePly[depth].move[0]=movePly[depth].move[1];
      movePly[depth].move[1]=temp;
      return;
    }
    else
      return;
  }
  for (gap=n>>1; gap>0; gap>>=1)
    for (i=gap; i<n; i++)
      for (j=i-gap; j>=0 && movePly[depth].move[j].weight<
         movePly[depth].move[j+gap].weight; j-=gap) {
        temp=movePly[depth].move[j];
        movePly[depth].move[j]=movePly[depth].move[j+gap];
        movePly[depth].move[j+gap]=temp;
      }
} */

/* Shell-2 */
/*void shellSort(int n, int depth)
{
  int i, j, increment;
  struct moveType temp;

  if (n==2) {
    if (movePly[depth].move[0].weight<movePly[depth].move[1].weight) {
      temp=movePly[depth].move[0];
      movePly[depth].move[0]=movePly[depth].move[1];
      movePly[depth].move[1]=temp;
      return;
    }
    else
      return;
  }
  increment = 3;
  while (increment > 0)
  {
    for (i=0; i < n; i++)
    {
      j = i;
      temp = movePly[depth].move[i];
      while ((j >= increment) && (movePly[depth].move[j-increment].weight < temp.weight))
      {
        movePly[depth].move[j] = movePly[depth].move[j - increment];
        j = j - increment;
      }
      movePly[depth].move[j] = temp;
    }
    if ((increment>>1) != 0)
      increment>>=1;
    else if (increment == 1)
      increment = 0;
    else
      increment = 1;
  }
} */


/* Insert-1 */
void InsertSort(int n, int depth) {
  int i, j;
  struct moveType a, temp;

  if (n==2) {
    if (movePly[depth].move[0].weight<movePly[depth].move[1].weight) {
      temp=movePly[depth].move[0];
      movePly[depth].move[0]=movePly[depth].move[1];
      movePly[depth].move[1]=temp;
      return;
    }
    else
      return;
  }

  a=movePly[depth].move[0];
  for (i=1; i<=n-1; i++) 
    if (movePly[depth].move[i].weight>a.weight) {
      temp=a;
      a=movePly[depth].move[i];
      movePly[depth].move[i]=temp;
    }
  movePly[depth].move[0]=a; 
  for (i=2; i<=n-1; i++) {  
    j=i;
    a=movePly[depth].move[i];
    while (a.weight>movePly[depth].move[j-1].weight) {
      movePly[depth].move[j]=movePly[depth].move[j-1];
      j--;
    }
    movePly[depth].move[j]=a;
  }
}  

/* Insert-2 */
/*void InsertSort(int n, int depth) {
  int i, j;
  struct moveType a;

  if (n==2) {
    if (movePly[depth].move[0].weight<movePly[depth].move[1].weight) {
      a=movePly[depth].move[0];
      movePly[depth].move[0]=movePly[depth].move[1];
      movePly[depth].move[1]=a;
      return;
    }
    else
      return;
  }
  for (j=1; j<=n-1; j++) {
    a=movePly[depth].move[j];
    i=j-1;
    while ((i>=0)&&(movePly[depth].move[i].weight<a.weight)) {
      movePly[depth].move[i+1]=movePly[depth].move[i];
      i--;
    }
    movePly[depth].move[i+1]=a;
  }
}  */


int AdjustMoveList(void) {
  int k, r, n, rank, suit;

  for (k=1; k<=13; k++) {
    suit=forbiddenMoves[k].suit;
    rank=forbiddenMoves[k].rank;
    for (r=0; r<=movePly[iniDepth].last; r++) {
      if ((suit==movePly[iniDepth].move[r].suit)&&
        (rank!=0)&&(rank==movePly[iniDepth].move[r].rank)) {
        /* For the forbidden move r: */
        for (n=r; n<=movePly[iniDepth].last; n++)
          movePly[iniDepth].move[n]=movePly[iniDepth].move[n+1];
        movePly[iniDepth].last--;
      }  
    }
  }
  return movePly[iniDepth].last+1;
}


void IniHeurTable (void) {

  int k, l, m;

  for (k=0; k<=3; k++)
    for (l=0; l<=5; l++)
      for (m=0; m<=5; m++) {
        heurTable->cho1[k][l][m]=ERR;
        heurTable->cho2[k][l][m]=ERR;
        heurTable->cho3[k][l][m]=ERR;
      }

  heurTable->cho1[0][0][0]=UNCLEAR;
  heurTable->cho2[0][0][0]=LOSER;
  heurTable->cho3[0][0][0]=NOTHING;
  heurTable->cho1[0][0][1]=UNCLEAR;
  heurTable->cho2[0][0][1]=LOSER;
  heurTable->cho3[0][0][1]=NOTHING;
  heurTable->cho1[0][0][2]=UNCLEAR;
  heurTable->cho2[0][0][2]=LOSER;
  heurTable->cho3[0][0][2]=NOTHING;
  heurTable->cho1[0][0][3]=UNCLEAR;
  heurTable->cho2[0][0][3]=LOSER;
  heurTable->cho3[0][0][3]=NOTHING;
  heurTable->cho1[0][0][4]=LOSER;
  heurTable->cho2[0][0][4]=UNCLEAR;
  heurTable->cho3[0][0][4]=NOTHING;
  heurTable->cho1[0][0][5]=LOSER;
  heurTable->cho2[0][0][5]=UNCLEAR;
  heurTable->cho3[0][0][5]=NOTHING;
  heurTable->cho1[0][1][0]=UNCLEAR;
  heurTable->cho2[0][1][0]=LOSER;
  heurTable->cho3[0][1][0]=NOTHING;
  heurTable->cho1[0][1][2]=UNCLEAR;
  heurTable->cho2[0][1][2]=LOSER;
  heurTable->cho3[0][1][2]=NOTHING;
  heurTable->cho1[0][1][5]=LOSER;
  heurTable->cho2[0][1][5]=UNCLEAR;
  heurTable->cho3[0][1][5]=NOTHING;
  heurTable->cho1[0][2][0]=LOSER;
  heurTable->cho2[0][2][0]=UNCLEAR;
  heurTable->cho3[0][2][0]=NOTHING;
  heurTable->cho1[0][2][1]=UNCLEAR;
  heurTable->cho2[0][2][1]=LOSER;
  heurTable->cho3[0][2][1]=NOTHING;
  heurTable->cho1[0][2][4]=LOSER;
  heurTable->cho2[0][2][4]=UNCLEAR;
  heurTable->cho3[0][2][4]=NOTHING;
  heurTable->cho1[0][3][0]=UNCLEAR;
  heurTable->cho2[0][3][0]=LOSER;
  heurTable->cho3[0][3][0]=NOTHING;
  heurTable->cho1[0][4][0]=LOSER;
  heurTable->cho2[0][4][0]=UNCLEAR;
  heurTable->cho3[0][4][0]=NOTHING;
  heurTable->cho1[0][4][2]=LOSER;
  heurTable->cho2[0][4][2]=UNCLEAR;
  heurTable->cho3[0][4][2]=NOTHING;
  heurTable->cho1[0][4][5]=LOSER;
  heurTable->cho2[0][4][5]=UNCLEAR;
  heurTable->cho3[0][4][5]=NOTHING;
  heurTable->cho1[0][5][0]=LOSER;
  heurTable->cho2[0][5][0]=UNCLEAR;
  heurTable->cho3[0][5][0]=NOTHING;
  heurTable->cho1[0][5][1]=LOSER;
  heurTable->cho2[0][5][1]=UNCLEAR;
  heurTable->cho3[0][5][1]=NOTHING;
  heurTable->cho1[0][5][4]=LOSER;
  heurTable->cho2[0][5][4]=UNCLEAR;
  heurTable->cho3[0][5][4]=NOTHING;
  heurTable->cho1[1][0][0]=UNCLEAR;
  heurTable->cho2[1][0][0]=LOSER;
  heurTable->cho3[1][0][0]=NOTHING;
  heurTable->cho1[1][0][2]=LOSER;
  heurTable->cho2[1][0][2]=UNCLEAR;
  heurTable->cho3[1][0][2]=NOTHING;
  heurTable->cho1[1][0][5]=LOSER;
  heurTable->cho2[1][0][5]=UNCLEAR;
  heurTable->cho3[1][0][5]=NOTHING;
  heurTable->cho1[1][2][0]=UNCLEAR;
  heurTable->cho2[1][2][0]=LOSER;
  heurTable->cho3[1][2][0]=NOTHING;
  heurTable->cho1[1][5][0]=LOSER;
  heurTable->cho2[1][5][0]=UNCLEAR;
  heurTable->cho3[1][5][0]=NOTHING;
  heurTable->cho1[2][0][0]=UNCLEAR;
  heurTable->cho2[2][0][0]=WINNER;
  heurTable->cho3[2][0][0]=LOSER;
  heurTable->cho1[2][0][1]=LOSER;
  heurTable->cho2[2][0][1]=UNCLEAR;
  heurTable->cho3[2][0][1]=WINNER;
  heurTable->cho1[2][0][4]=LOSER;
  heurTable->cho2[2][0][4]=UNCLEAR;
  heurTable->cho3[2][0][4]=WINNER;
  heurTable->cho1[2][1][0]=UNCLEAR;
  heurTable->cho2[2][1][0]=WINNER;
  heurTable->cho3[2][1][0]=LOSER;
  heurTable->cho1[2][4][0]=WINNER;
  heurTable->cho2[2][4][0]=LOSER;
  heurTable->cho3[2][4][0]=UNCLEAR;
  heurTable->cho1[3][0][0]=WINNER;
  heurTable->cho2[3][0][0]=UNCLEAR;
  heurTable->cho3[3][0][0]=LOSER;

  return;
}

int InvBitMapRank(unsigned short bitMap) {

  switch (bitMap) {
    case 0x1000: return 14;
    case 0x0800: return 13;
    case 0x0400: return 12;
    case 0x0200: return 11;
    case 0x0100: return 10;
    case 0x0080: return 9;
    case 0x0040: return 8;
    case 0x0020: return 7;
    case 0x0010: return 6;
    case 0x0008: return 5;
    case 0x0004: return 4;
    case 0x0002: return 3;
    case 0x0001: return 2;
    default: return 0;
  }
}

int listNo;
struct winCardType * nextp;
struct cardOrderType cardOrder[14][4][2];
struct orderTableType orderTable; 


int WinningMove(struct moveType * mvp1, struct moveType * mvp2) {
/* Return TRUE if move 1 wins over move 2, with the assumption that
move 2 is the presently winning card of the trick */

  if (mvp1->suit==mvp2->suit) {
    if ((mvp1->rank)>(mvp2->rank))
      return TRUE;
    else
      return FALSE;
  }    
  else if ((mvp1->suit)==trump)
    return TRUE;
  else
    return FALSE;
}


struct nodeCardsType * SOPinList(struct pos * posPoint, struct nodeCardsType
  * first, int target, int * result) {
    /* Walk through a sequence of SOP nodes to find a SOP that matches the
    current position. If match, pointer to matching node is returned and
    result is set to TRUE, otherwise pointer to last SOP node is returned
    and result set to FALSE. */

  struct nodeCardsType * cardsP, * prevCardsP=0;
  int s, h, match, diff;
 
  /*sum=0;*/
  for (cardsP=first; cardsP!=NULL; cardsP=cardsP->next) {
   /* sum++;  */
    match=TRUE;
    prevCardsP=cardsP;
    for (s=0; s<=3; s++) {
      for (h=0; h<=3; h++) {
        if (posPoint->length[h][s]!=cardsP->length[h][s]) {
          match=FALSE;
          break;
        }
      }
      if (!match)
        break;
    }
    if (!match)
      continue;
  	
    diff=cardsP->tricksMAX-posPoint->tricksMAX;
    if (((cardsP->scoreFlag==1)&&(diff>=1))||
      ((cardsP->scoreFlag==0)&&(diff<=-1)))
      continue;    
    *result=TRUE;
    return cardsP;
  }
  *result=FALSE;
  return prevCardsP;          /* No matching node was found */
}


struct nodeCardsType * FindSOP(struct pos * posPoint,
  struct winCardType * nodeP, int target) {
  struct nodeCardsType * sopP, * first;
  struct winCardType * np;
	
  np=nodeP;
  while (np!=NULL) {
    if ((posPoint->relRankInSuit[np->hand][np->suit] 
	& bitMapRank[15-np->order])!=0) {
	/* Winning card fits position */
	first=np->first;
	if (first!=NULL) {
	  sopP=SOPinList(posPoint, first, target, &res);
	  if (res)
	    return sopP;
	}
	if (np->nextWin!=NULL) 
	  np=np->nextWin;
	else {
	  if (np->next!=NULL)
	    np=np->next;
	  else {
	    np=np->prevWin;
	    if (np==NULL)
		return NULL;
	    while (np->next==NULL) {
		np=np->prevWin;
		if (np==NULL)  /* Previous node is header node? */
		  return NULL;
	    }
	    np=np->next;
	  }
      }		
    }
    else {
      if (np->next!=NULL)
        np=np->next;
      else {
	  np=np->prevWin;
	  if (np==NULL)
	    return NULL;		
        while (np->next==NULL) {
	    np=np->prevWin;
	    if (np==NULL)  /* Previous node is header node? */
		return NULL;
	  }
	  np=np->next;
      }
    }
  }	
  return NULL;  
}


struct cardType NextCard(struct cardType card) {
  struct cardType cd;
  int index;

  if (card.exists) {
    if (card.index==orderTable.last) {
      cd.hand=0;
      cd.order=1;
      cd.suit=0;
      cd.index=-1;
      cd.exists=FALSE;
      return cd;
    }
    else {
      index=card.index+1;
      cd.order=orderTable.order[index];
      cd.suit=orderTable.suit[index];
      cd.hand=orderTable.hand[index];
      cd.index=index;
      cd.exists=TRUE;
      return cd;
    }
  }
  else {
    if (orderTable.last==-1) {
      cd.hand=0;
      cd.order=1;
      cd.suit=0;
      cd.index=-1;
      cd.exists=FALSE;
      return cd;
    }
    else {
      cd.order=orderTable.order[0];
      cd.suit=orderTable.suit[0];
      cd.hand=orderTable.hand[0];
      cd.index=0;
      cd.exists=TRUE;
      return cd;
    }
  }
}    


struct nodeCardsType * BuildPath(struct pos * posPoint, int firstHand,
  int tricks, int target, int * result) {
  /* If result is TRUE, a new SOP has been created and BuildPath returns a
  pointer to it. If result is FALSE, an existing SOP is used and BuildPath
  returns a pointer to the SOP */

  int found, count;
  struct cardType card;
  struct winCardType * np, * p2, * nprev;
  struct nodeCardsType * sopP=0, * first, * p, * cp;

  nprev=NULL;
  card.hand=0;
  card.suit=0;
  card.order=1;
  card.exists=FALSE;
  card=NextCard(card); /* Find next winning card from current
                                (filtered) position */
  if (!card.exists) {     /* No winning cards needed for this SOP */
    first=posSearch.first[firstHand][tricks]; /* Pointer to first SOP */
    if (first!=NULL) {
      for (cp=first; cp!=NULL; cp=cp->next)
         sopP=cp;
      p=&nodeCards[nodeSetSize];
      nodeSetSize++;
      sopP->next=p;
      p->next=NULL;
      p->winp=NULL;
      *result=TRUE;
      return p;
    }
    else {
      p=&nodeCards[nodeSetSize];
      nodeSetSize++;
      posSearch.first[firstHand][tricks]=p;
      p->next=NULL;
	p->winp=NULL;
      *result=TRUE;
      return p;
    }  
  }

  np=posSearch.posSearchPoint[firstHand][tricks];  /* Points to 1st node in
    winning list */

  /* If winning node has a card that equals the next winning card deduced
  from the position, then there already exists a (partial) path */

  if (np==NULL) {   /* There is no winning list created yet */
   /* Create winning nodes */
    p2=&winCards[winSetSize];
    winSetSize++;
    p2->next=NULL;
    p2->nextWin=NULL;
	p2->prevWin=NULL;
    posSearch.posSearchPoint[firstHand][tricks]=p2;
    p2->hand=card.hand;
    p2->suit=card.suit;
    p2->order=card.order;
    p2->first=NULL;
    np=p2;           /* Latest winning node */
    card=NextCard(card);
    while (card.exists) {
      p2=&winCards[winSetSize];
      winSetSize++;
      np->nextWin=p2;
	  p2->prevWin=np;
      p2->next=NULL;
      p2->nextWin=NULL;
      p2->hand=card.hand;
      p2->suit=card.suit;
      p2->order=card.order;
      p2->first=NULL;
      np=p2;         /* Latest winning node */
      card=NextCard(card);
    }
    p=&nodeCards[nodeSetSize];
    nodeSetSize++;
    np->first=p;
	p->winp=np;
    p->next=NULL;
    *result=TRUE;
    return p;
  }
  else {   /* Winning list exists */
    count=0;
    while (1) {   /* Find all winning nodes that correspond to current
        position */
      found=FALSE;
      while (1) {    /* Find card amongst alternatives */
        if ((np->hand==card.hand)&&(np->suit==card.suit)&&
          (np->order==card.order)) {            /* Part of path found */
          found=TRUE;
	    nprev=np;
          break;
        }
        if (np->next!=NULL)
          np=np->next;
        else
          break;
      }
      if (found) {
        card=NextCard(card);
        if (!card.exists) {    /* No further winning cards in the position */
          if (np->first!=NULL) {
            for (cp=np->first; cp!=NULL; cp=cp->next)
               sopP=cp;
            p=&nodeCards[nodeSetSize];
            nodeSetSize++;
            sopP->next=p;
            p->next=NULL;
	      p->winp=np;
            *result=TRUE;
            return p;
          }
          else {
            p=&nodeCards[nodeSetSize];
            nodeSetSize++;
            np->first=p;
            p->next=NULL;
			p->winp=np;
            *result=TRUE;
            return p;
          }
        }
        else if (np->nextWin==NULL)   /* The next card can never be found */
          break;
        else {
          np=np->nextWin;       /* Try to find next winning card  */
          continue;
        }
      }
      else
        break;                    /* Card was not found */
    }               /* End outer while */

    /* Create additional node, coupled to existing node(s) */
    p2=&winCards[winSetSize];
    winSetSize++;
    if (found) {
      np->nextWin=p2;
	  p2->prevWin=np;
	}  
    else {
      np->next=p2;
      p2->prevWin=nprev;
    }	  
    p2->nextWin=NULL;
    p2->next=NULL;
    p2->hand=card.hand;
    p2->suit=card.suit;
    p2->order=card.order;
    p2->first=NULL;
    np=p2;          /* Latest winning node */
    card=NextCard(card);

    /* Rest of path must be created */
    while (card.exists) {
      p2=&winCards[winSetSize];
      winSetSize++;
      np->nextWin=p2;
	  p2->prevWin=np;
      p2->next=NULL;
      p2->hand=card.hand;
      p2->suit=card.suit;
      p2->order=card.order;
      p2->first=NULL;
      p2->nextWin=NULL;
      np=p2;         /* Latest winning node */
      card=NextCard(card);
    }

  /* All winning cards in SOP have been traversed and winning nodes created */
    p=&nodeCards[nodeSetSize];
    nodeSetSize++;
    np->first=p;
	p->winp=np;
    p->next=NULL;
    *result=TRUE;
    return p;
  }  
}


void BuildSOP(struct pos * posPoint, int tricks, int firstHand, int target,
  int depth, int scoreFlag, int score) {
  int ss, hh, order, res;
  unsigned short int w;
  unsigned short int temp[4][4];
  unsigned short int aggr[4];
  unsigned short int relRank[4];
  unsigned short int relAggr;
  int cind;
  struct nodeCardsType * cardsP;

  for (ss=0; ss<=3; ss++) {
    w=posPoint->winRanks[depth][ss];
    w=w & (-w);       /* Only lowest win */
    for (hh=0; hh<=3; hh++)
      temp[hh][ss]=posPoint->rankInSuit[hh][ss] & (~(w-1));

    aggr[ss]=0;
    for (hh=0; hh<=3; hh++)
      aggr[ss]=aggr[ss] | temp[hh][ss];
    for (hh=0; hh<=3; hh++)
      posPoint->relRankInSuit[hh][ss]=rel[aggr[ss]].relRanks[hh][ss];

    relRank[ss]=0;
    for (hh=0; hh<=3; hh++)
      relRank[ss]=relRank[ss] | posPoint->relRankInSuit[hh][ss];
  }
  relAggr=0;
  for (ss=0; ss<=3; ss++)
    relAggr=relAggr | relRank[ss];

  cind=-1;
  for (order=1; order<=13; order++)
    if ((relAggr & bitMapRank[15-order])!=0)
      for (ss=0; ss<=3; ss++)
        if ((relRank[ss] & bitMapRank[15-order])!=0)
          for (hh=0; hh<=3; hh++)
            if ((posPoint->relRankInSuit[hh][ss] &
              bitMapRank[15-order])!=0) {
              cind++;
              orderTable.order[cind]=order;
              orderTable.suit[cind]=ss;
              orderTable.hand[cind]=hh;
              break;
            }
  orderTable.last=cind;

  cardsP=BuildPath(posPoint, firstHand, tricks, target, &res);
  if (res) {
    cardsP->scoreFlag=scoreFlag;
    cardsP->tricksMAX=posPoint->tricksMAX;
    cardsP->bestMoveSuit=bestMove[depth].suit;
    cardsP->bestMoveRank=bestMove[depth].rank;
    memcpy(cardsP->length, posPoint->length, 
	  sizeof(posPoint->length));  
  }
  #ifdef STAT
    c9[depth]++;
  #endif  

  #ifdef TTDEBUG
  if ((res) && (ttCollect) && (!suppressTTlog)) {
    fprintf(fp7, "cardsP=%d\n", (int)cardsP);
    fprintf(fp7, "nodeSetSize=%d\n", nodeSetSize);
    fprintf(fp7, "addNodeCount=%d\n", addNodeCount);
    fprintf(fp7, "value=%d\n", cardsP->scoreFlag);
    fprintf(fp7, "target=%d\n", target);
    fprintf(fp7, "tricksMAX=%d\n", cardsP->tricksMAX);
    fprintf(fp7, "first=%c nextFirst=%c\n",
      cardHand[posPoint->first[depth]], cardHand[posPoint->first[depth-1]]);
    fprintf(fp7, "bestMove:  suit=%c rank=%c\n", cardSuit[bestMove[depth].suit],
      cardRank[bestMove[depth].rank]);
    fprintf(fp7, "\n");
    fprintf(fp7, "Last trick:\n");
    fprintf(fp7, "1st hand=%c\n", cardHand[posPoint->first[depth+3]]);
    for (k=3; k>=0; k--) {
      mcurrent=movePly[depth+k+1].current;
      fprintf(fp7, "suit=%c  rank=%c\n",
        cardSuit[movePly[depth+k+1].move[mcurrent].suit],
        cardRank[movePly[depth+k+1].move[mcurrent].rank]);
    }
    fprintf(fp7, "\n");
    for (hh=0; hh<=3; hh++) {
      fprintf(fp7, "hand=%c\n", cardHand[hh]);
      for (ss=0; ss<=3; ss++) {
        fprintf(fp7, "suit=%c", cardSuit[ss]);
        for (rr=14; rr>=2; rr--)
          if (posPoint->rankInSuit[hh][ss] & bitMapRank[rr])
            fprintf(fp7, " %c", cardRank[rr]);
        fprintf(fp7, "\n");
      }
      fprintf(fp7, "\n");
    }
    fprintf(fp7, "\n");

    for (hh=0; hh<=3; hh++) {
      fprintf(fp7, "hand=%c\n", cardHand[hh]);
      for (ss=0; ss<=3; ss++) {
        fprintf(fp7, "suit=%c", cardSuit[ss]);
        for (rr=1; rr<=13; rr++)
          if (posPoint->relRankInSuit[hh][ss] & bitMapRank[15-rr])
            fprintf(fp7, " %c", cardRank[rr]);
        fprintf(fp7, "\n");
      }
      fprintf(fp7, "\n");
    }
    fprintf(fp7, "\n");
  }
  #endif  
}


int CheckDeal(struct moveType * cardp) {
  int h, s, k, found;
  unsigned short int temp[4][4];

  for (h=0; h<=3; h++)
    for (s=0; s<=3; s++)
      temp[h][s]=game.suit[h][s];

  /* Check that all ranks appear only once within the same suit. */
  for (s=0; s<=3; s++)
    for (k=2; k<=14; k++) {
      found=FALSE;
      for (h=0; h<=3; h++) {
        if ((temp[h][s] & bitMapRank[k])!=0) {
          if (found) {
            cardp->suit=s;
            cardp->rank=k;
            return 1;
          }  
          else
            found=TRUE;
        }    
      }
    }

  return 0;
}


void WinAdapt(struct pos * posPoint, int depth, struct nodeCardsType * cp) {
  int h, s, a;
  unsigned short int w;
  struct winCardType *wp;

  memcpy(sct, nullsct, sizeof(nullsct));
  
  wp=cp->winp;
  while (wp!=NULL) {
    sct[wp->hand][wp->suit]++;
    wp=wp->prevWin;
  }
  
  for (s=0; s<=3; s++) {
    posPoint->winRanks[depth][s]=0;
    for (h=0; h<=3; h++) {
      w=posPoint->rankInSuit[h][s];
      a=posPoint->length[h][s]-sct[h][s];
      while (a>0) {
        w=w & (w-1);  /* Remove least significant 1-bit */
        a--;
      }
      posPoint->winRanks[depth][s]=posPoint->winRanks[depth][s] | w;
    }
  }
  return;
}

#ifdef TTDEBUG

void ReceiveTTstore(struct pos *posPoint, struct nodeCardsType * cardsP, 
  int target, int depth) {
/* Stores current position information and TT position value in table
  ttStore with current entry lastHastStore. Also stores corresponding
  information in log rectt.txt. */
  tricksLeft=0;
  for (hh=0; hh<=3; hh++)
    for (ss=0; ss<=3; ss++)
      tricksLeft=tricksLeft+posPoint->length[hh][ss];
  tricksLeft=tricksLeft/4;
  ttStore[lastTTstore].tricksLeft=tricksLeft;
  ttStore[lastTTstore].cardsP=cardsP;
  ttStore[lastTTstore].first=posPoint->first[depth];
  if ((handToPlay==posPoint->first[depth])||
    (handToPlay==partner[posPoint->first[depth]])) {
    ttStore[lastTTstore].target=target-posPoint->tricksMAX;
    ttStore[lastTTstore].scoreFlag=cardsP->scoreFlag;
  }
  else {
    ttStore[lastTTstore].target=tricksLeft-
      target+posPoint->tricksMAX+1;
    if (cardsP->scoreFlag==1)
      ttStore[lastTTstore].scoreFlag=0;
    else
      ttStore[lastTTstore].scoreFlag=1;
  }
  for (hh=0; hh<=3; hh++)
    for (ss=0; ss<=3; ss++)
      ttStore[lastTTstore].suit[hh][ss]=
        posPoint->rankInSuit[hh][ss];
  fp11=fopen("rectt.txt", "a");
  if (lastTTstore<SEARCHSIZE) {
    fprintf(fp11, "lastTTstore=%d\n", lastTTstore);
    fprintf(fp11, "tricksMAX=%d\n", posPoint->tricksMAX);
    fprintf(fp11, "leftTricks=%d\n",
      ttStore[lastTTstore].tricksLeft);
    fprintf(fp11, "cardsP=%d\n",
      ttStore[lastTTstore].cardsP);
    fprintf(fp11, "scoreFlag=%d\n",
      ttStore[lastTTstore].scoreFlag);
    fprintf(fp11, "first=%c\n",
      cardHand[ttStore[lastTTstore].first]);
    fprintf(fp11, "target=%d\n",
      ttStore[lastTTstore].target);
    fprintf(fp11, "\n");
    for (hh=0; hh<=3; hh++) {
      fprintf(fp11, "hand=%c\n", cardHand[hh]);
      for (ss=0; ss<=3; ss++) {
        fprintf(fp11, "suit=%c", cardSuit[ss]);
        for (rr=14; rr>=2; rr--)
          if (ttStore[lastTTstore].suit[hh][ss]
            & bitMapRank[rr])
            fprintf(fp11, " %c", cardRank[rr]);
         fprintf(fp11, "\n");
      }
      fprintf(fp11, "\n");
    }
    for (hh=0; hh<=3; hh++) {
      fprintf(fp11, "hand=%c\n", cardHand[hh]);
      for (ss=0; ss<=3; ss++) {
        fprintf(fp11, "suit=%c", cardSuit[ss]);
        for (rr=1; rr<=13; rr++)
          if (posPoint->relRankInSuit[hh][ss] & bitMapRank[15-rr])
            fprintf(fp11, " %c", cardRank[rr]);
        fprintf(fp11, "\n");
      }
      fprintf(fp11, "\n");
    }
  }
  fclose(fp11);
  lastTTstore++;
}
#endif

// to compile, g++ dds.cpp
int main(int argc, char* argv[]){
   if (argc < 19) {
      printf("dds trump first 4x4_num_sperated_by_space [suit rank]x3\n");
      return 0;
   }
   struct deal d;
   d.trump = atoi(argv[1]);
   d.first = atoi(argv[2]);
   memset(&(d.currentTrickSuit), 0, 3*sizeof(int));
   memset(&(d.currentTrickRank), 0, 3*sizeof(int));
   for(int i=0; i < (argc-3-16); i+=2) {
      int idx = (3+16)+i;
      d.currentTrickSuit[i] = atoi(argv[idx]);
      d.currentTrickRank[i] = atoi(argv[idx+1]);
   }
   // i is player loop, j is suit loop, spade is 0, shdc = 0123
   // each card take one bit from 1<<14 is A
   for(int i=0; i<4; i++)
      for(int j=0; j<4; j++) {
         int idx = 3+ 4*i + j;
         d.remainCards[i][j] = atoi(argv[idx]);
      }
   struct futureTricks futp;
   int status = SolveBoard(d, -1, 1, 0, &futp);
   if (status!=1) {
      printf("status: %d", status);
      return 0;
   }   
   int cards = futp.cards;
   printf("%d %d\n",futp.nodes, futp.cards);
   for(int i=0; i<cards; i++) {
      printf("%d ",futp.suit[i]);
      printf("%d ",futp.rank[i]);
      printf("%d ",futp.equals[i]);
      printf("%d ",futp.score[i]);
      printf("\n");
   }

   return status;
}
