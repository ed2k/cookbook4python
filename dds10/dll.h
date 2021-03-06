/* portability-macros header prefix */

/* MSC prefers a different spelling of "long long" */
#if defined(_MSC_VER)
/*#    define LONGLONG __int64*/
#else
#    define LONGLONG long long
#endif

/* Windows requires a __declspec(dllexport) tag, etc */
#if defined(_WIN32)
#    define DLLEXPORT __declspec(dllexport)
#    define STDCALL __stdcall
/*#    define INT8 __int8*/
#else
#    define DLLEXPORT
#    define STDCALL
#    define INT8 char
#endif

#ifdef __cplusplus
#    define EXTERN_C extern "C"
#else
#    define EXTERN_C
#endif

#if defined(_WIN32)
#    include <windows.h>
#    include <process.h>
#endif

/* end of portability-macros section */

/*#define BENCH*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*#define STAT*/	/* Define STAT to generate a statistics log, stat.txt */
/*#define TTDEBUG*/     /* Define TTDEBUG to generate transposition table debug information */
/*#define CANCEL*/    /* Define CANCEL to get support for cancelling ongoing search */

#ifdef  TTDEBUG
#define SEARCHSIZE  20000
#else
#define SEARCHSIZE  1
#endif

#define CANCELCHECK  200000

#if defined(INFINITY)
#    undef INFINITY
#endif
#define INFINITY    32000

#define MAXNODE     1
#define MINNODE     0

#define TRUE        1
#define FALSE       0

#define MOVESVALID  1
#define MOVESLOCKED 2

#define NSIZE	100000
#define WSIZE   100000
#define LSIZE   20000
#define NINIT	250000/*400000*/
#define WINIT	700000/*1000000*/
#define LINIT	50000

#define Max(x, y) (((x) >= (y)) ? (x) : (y))
#define Min(x, y) (((x) <= (y)) ? (x) : (y))

struct gameInfo  {          /* All info of a particular deal */
  int vulnerable;
  int declarer;
  int contract;
  int leadHand;
  int leadSuit;
  int leadRank;
  int first;
  int noOfCards;
  unsigned short int suit[4][4];
    /* 1st index is hand id, 2nd index is suit id */
};

struct dealType {
  unsigned short int deal[4][4];
};  

struct moveType {
  unsigned char suit;
  unsigned char rank;
  unsigned short int sequence;          /* Whether or not this move is
                                        the first in a sequence */
  short int weight;                     /* Weight used at sorting */
};

struct movePlyType {
  struct moveType move[14];             
  int current;
  int last;
};

struct highCardType {
  int rank;
  int hand;
};

struct futureTricks {
  int nodes;
  #ifdef BENCH
  int totalNodes;
  #endif
  int cards;
  int suit[13];
  int rank[13];
  int equals[13];
  int score[13];
};

struct deal {
  int trump;
  int first;
  int currentTrickSuit[3];
  int currentTrickRank[3];
  unsigned int remainCards[4][4];
};

struct makeType {
  unsigned short int winRanks[4];
};

struct pos {
  unsigned short int rankInSuit[4][4];   /* 1st index is hand, 2nd index is
                                        suit id */
  int orderSet[4];
  int winOrderSet[4];
  int winMask[4];
  int leastWin[4];
  unsigned short int removedRanks[4];    /* Ranks removed from board,
                                        index is suit */
  unsigned short int winRanks[50][4];  /* Cards that win by rank,
                                       indices are depth and suit */
  unsigned char length[4][4];
  char ubound;
  char lbound;
  char bestMoveSuit;
  char bestMoveRank;
  int first[50];                 /* Hand that leads the trick for each ply*/
  int high[50];                  /* Hand that is presently winning the trick */
  struct moveType move[50];      /* Presently winning move */              
  int handRelFirst;              /* The current hand, relative first hand */
  int tricksMAX;                 /* Aggregated tricks won by MAX */
  struct highCardType winner[4]; /* Winning rank of the trick,
                                    index is suit id. */
  struct highCardType secondBest[4]; /* Second best rank, index is suit id. */
};

struct posSearchType {
  struct winCardType * posSearchPoint; 
  LONGLONG suitLengths;
  struct posSearchType * left;
  struct posSearchType * right;
};


struct nodeCardsType {
  char ubound;	/* ubound and
			lbound for the N-S side */
  char lbound;
  char bestMoveSuit;
  char bestMoveRank;
  char leastWin[4];
};

struct winCardType {
  int orderSet;
  int winMask;
  struct nodeCardsType * first;
  struct winCardType * prevWin;
  struct winCardType * nextWin;
  struct winCardType * next;
}; 


struct evalType {
  int tricks;
  unsigned short int winRanks[4];
};

struct relRanksType {
  int aggrRanks[4];
  int winMask[4];
};


struct ttStoreType {
  struct nodeCardsType * cardsP;
  char tricksLeft;
  char target;
  char ubound;
  char lbound;
  unsigned char first;
  unsigned short int suit[4][4];
};


extern struct gameInfo game;
extern struct gameInfo * gameStore;
extern struct ttStoreType * ttStore;
extern struct nodeCardsType * nodeCards;
extern struct winCardType * winCards;
extern struct pos position, iniPosition, lookAheadPos;
extern struct moveType move[13];
extern struct movePlyType movePly[50];
extern struct posSearchType * posSearch;
extern struct searchType searchData;
extern struct moveType forbiddenMoves[14];  /* Initial depth moves that will be 
					       excluded from the search */
extern struct moveType initialMoves[4];
extern struct moveType highMove;
extern struct moveType * bestMove;
extern struct relRanksType * rel;
extern struct winCardType **pw;
extern struct nodeCardsType **pn;
extern struct posSearchType **pl;


extern int handStore[4][4];             /* All hand identities are given as
                                        0=NORTH, 1=EAST, 2=SOUTH, 3=WEST.
                                        Player is the hand whose move is to be
                                        optimized.
                                        1st index is first hand, 2nd index
                                        is hand relative first. The value is
                                        the absolute value, NORTH, EAST,
                                        SOUTH, WEST of the hand pointed to
                                        by handRelFirst */
extern unsigned short int bitMapRank[16];
extern unsigned short int iniRemovedRanks[4];
extern unsigned short int relRankInSuit[4][4];
extern int sum;
extern int score1Counts[50], score0Counts[50];
extern int c1[50], c2[50], c3[50], c4[50], c5[50], c6[50], c7[50],
  c8[50], c9[50];
extern int nodeTypeStore[4];            /* Look-up table for determining if
                                        node is MAXNODE or MINNODE */
extern int lho[4], rho[4], partner[4];                                        
extern int trumpContract;
extern int trump;
extern int nodes;                       /* Number of nodes searched */
extern int no[50];                      /* Number of nodes searched on each
                                        depth level */
extern int payOff;
extern int iniDepth;
extern int treeDepth;
extern int tricksTarget;                /* No of tricks for MAX in order to
                                        meet the game goal, e.g. to make the
                                        contract */
extern int tricksTargetOpp;             /* Target no of tricks for MAX
                                        opponent */
extern int targetNS;
extern int targetEW;
extern int handToPlay;
extern int nodeSetSize;
extern int winSetSize;
extern int lenSetSize;
extern int lastTTstore;
extern int searchTraceFlag;
extern int countMax;
extern int depthCount;
extern int highHand;
extern int nodeSetSizeLimit;
extern int winSetSizeLimit;
extern int lenSetSizeLimit;
extern int estTricks[4];
extern int recInd; 
extern int suppressTTlog;
extern unsigned char suitChar[4];
extern unsigned char rankChar[15];
extern unsigned char handChar[4];
extern int cancelOrdered;
extern int cancelStarted;
extern int threshold;
extern unsigned char cardRank[15], cardSuit[5], cardHand[4];

extern FILE * fp2, *fp7, *fp11;
  /* Pointers to logs */

EXTERN_C DLLEXPORT int STDCALL SolveBoard(struct deal dl, 
  int target, int solutions, int mode, struct futureTricks *futp);

void InitStart(void);
void InitGame(int gameNo, int moveTreeFlag, int first, int handRelFirst);
void InitSearch(struct pos * posPoint, int depth,
  struct moveType startMoves[], int first, int mtd);
int ABsearch(struct pos * posPoint, int target, int depth);
struct makeType Make(struct pos * posPoint, int depth);
int MoveGen(struct pos * posPoint, int depth);
void InsertSort(int n, int depth);
void UpdateWinner(struct pos * posPoint, int suit);
void UpdateSecondBest(struct pos * posPoint, int suit);
int WinningMove(struct moveType * mvp1, struct moveType * mvp2);
unsigned short int CountOnes(unsigned short int b);
int AdjustMoveList(void);
int QuickTricks(struct pos * posPoint, int hand, 
	int depth, int target, int *result);
int LaterTricksMIN(struct pos *posPoint, int hand, int depth, int target); 
int LaterTricksMAX(struct pos *posPoint, int hand, int depth, int target);
struct nodeCardsType * CheckSOP(struct pos * posPoint, struct nodeCardsType
  * nodep, int target, int tricks, int * result, int *value);
struct nodeCardsType * UpdateSOP(struct pos * posPoint, struct nodeCardsType
  * nodep);  
struct nodeCardsType * FindSOP(struct pos * posPoint,
  struct winCardType * nodeP, int firstHand, 
	int target, int tricks, int * valp);  
struct cardType NextCard(struct cardType card);
struct nodeCardsType * BuildPath(struct pos * posPoint, int firstHand,
  int tricks, struct posSearchType *nodep, int * result);
void BuildSOP(struct pos * posPoint, int tricks, int firstHand, int target,
  int depth, int scoreFlag, int score);
struct posSearchType * SearchLenAndInsert(struct posSearchType
	* rootp, LONGLONG key, int insertNode, int *result);  
void Undo(struct pos * posPoint, int depth);
int CheckDeal(struct moveType * cardp);
void WinAdapt(struct pos * posPoint, int depth, struct nodeCardsType * cp,
   unsigned short int aggr[]);
int InvBitMapRank(unsigned short bitMap);
int InvWinMask(int mask);
void ReceiveTTstore(struct pos *posPoint, struct nodeCardsType * cardsP, int target, int depth);
int DismissX(struct pos *posPoint, int depth); 
int DumpInput(int errCode, struct deal dl, int target, int solutions, int mode); 
void Wipe(void);
void AddNodeSet(void);
void AddLenSet(void);
void AddWinSet(void);
