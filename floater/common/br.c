/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Floater Bridge Network.
 *
 * The Initial Developer of the Original Code is
 * Geoff Pike <pike@EECS.Berkeley.EDU>.
 * Portions created by the Initial Developer are Copyright (C) 1996-2003
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Syela
 *   Lex Spoon <lex@cc.gatech.edu>
 *
 * Alternatively, the contents of this file may be used under the
 * terms of either the GNU General Public License Version 2 or later
 * (the "GPL"), in which case the provisions of the GPL are applicable
 * instead of those above. If you wish to allow use of your version of
 * this file only under the terms of the GPL, and not to allow others
 * to use your version of this file under the terms of the MPL,
 * indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the GPL. If
 * you do not delete the provisions above, a recipient may use your
 * version of this file under the terms of either the MPL or the GPL.
 * ***** END LICENSE BLOCK ***** */

/* br.c -- game state and game rules for both hearts and bridge
 *
 * $Id
 */
#include <stdio.h>    
#include "floater.h"
#include "floatcmd.h"
#include "tickler.h"
#include "comm.h"
#include "br.h"
#include "deal.h"
#include "UI.h"
#include "calendar.h"
#include "heartsrules.h"

fl_bool duplicate = TRUE;
fl_bool competitive = TRUE; /* whether to report results by email */
fl_bool new_competitive = TRUE; /* what competitive will be for the next hand */

char globaldate[100];
char globalseed[100];

char playerport[4][MAXPORTLEN+1];
char playeraddr[4][MAXADDRLEN+1];

char *northname = NULL, *southname = NULL, *eastname = NULL, *westname = NULL;
semistatic char *savednorthname = NULL, *savedsouthname = NULL,
  *savedeastname = NULL, *savedwestname = NULL; /* players at end of auction */
static char *handIDsavednames = NULL; /* the value of curhandID when savednorthname etc. were set */
char *bridgestatus = NULL;
char *bridgeinfo = NULL;
int myseat = Noseat;
int rhoseat = Noseat;
fl_bool seated = FALSE;
int scoremethod; /* index into scoringmethods[] */
int newscoremethod = -1; /* if >=0, scoring method for the next hand */
int cardsperhand;  /* 13 for bridge, 17 for 3-handed hearts */
static int numplayers;
static fl_bool gameover = FALSE; /* used by hearts and rubber */

/* hearts stuff */
fl_bool playinghearts = FALSE;
static fl_bool passdone = FALSE; /* have I have passed yet on the current hand? */
static char strscore[4][1000];
static int heartsscore[4];
static char *notacard = "c1";
#define mypassdone(s) (!(strneq(notacard, &((s)[myseat * 6]), 2) ||	\
			 strneq(notacard, &((s)[myseat * 6 + 2]), 2) ||	\
			 strneq(notacard, &((s)[myseat * 6 + 4]), 2)))
static char passset[10];
static void heartspass(char *p);

/* rubber bridge stuff */
static fl_bool nsvul, ewvul;
static int nstotal, ewtotal, nsbelowline, ewbelowline;

#define HEARTS_PLAY_TO 1000

/* used to generate a unique ID for each hand at a given table */
int handcounter = 0;
char *curhandname = NULL; /* the name of the current hand */
char *curhandID = NULL; /* the unique ID of the current hand */
char *curastate = NULL; /* auction state */
char *curpstate = NULL; /* play state */
char *curpassstate = NULL; /* hearts pass state */
char *curprettyauction = NULL; /* auction in "pretty" form */
int curdealer = -1; /* seat of current dealer */
int curdeclarer = -1; /* seat of current declarer */
char curcontract[20];
int curtrump = -1;
int curdeck[52];
char initialcards[4][35]; /* at most 4 players, at most 17 cards per player */
char initialcards_beforepass[4][35];
int currestricks, currespoints;

fl_bool claiming = FALSE; /* if declarer is trying to claim right now */
int curclaimtricks; /* how many tricks (of 13) he's claiming */
char *claimaccept; /* the defender who has accepted the claim, if any */

static fl_bool handplayedout; /* used to turn off randomplay between hands */

/* some forward decls */
static int minseen(int i);
static void setseen(int scoring, char *set, int num);
static char *prettypartialauction(char *auction);
void playergone(int which);
fl_bool cardmember(char *, char *);
fl_bool cardmembern(char *, char *, int);
static char *makegstate();
static char *inorderprintcardset(char *w);

/* should be 2: IMPs and matchpoints */
#define NUMSEENS NUMPAIRGAMES



/******************************************************************************
Dealing with seats, columns, etc.
******************************************************************************/
/* convert a column to a Tcl/Tk friendly string */
const char *bcol_to_string(int bcol)
{
  switch(bcol){
  case 0:
    return "lho";
    break;
  case 1:
    return "pard";
    break;
  case 2:
    return "rho";
    break;
  case 3:
    return "self";
    break;
  default:
    fatal("internal error: invalid column number", 1);
    return NULL;
  }
}


/******************************************************************************
How to dealing, score, etc., for the different forms of the game
******************************************************************************/

static void rndseed(void);
static void dupseed(void);

void impstartup(void);
void mpstartup(void);

void yukstartup(void);
void rubberstartup(void);

/* methods with worldwide identical random hands should be first */
scoring scoringmethods[numscoringmethods] =
{{"IMP", 4, impstartup, dupseed},
   {"MP", 4, mpstartup, dupseed},
   {"HEARTS", 3, yukstartup, rndseed},
   {"RUBBER", 4, rubberstartup, rndseed}
#if 0
 ,
   {"YUK4", 4, yukstartup, rndseed},
#endif
};

fl_bool isheartsstring(char *s)
{
  return strneq(s, "HEART", 5);
}

fl_bool ishearts(int n)
{
  return isheartsstring(scoringmethods[n].scoringname);
}

fl_bool isrubberstring(char *s)
{
  return streq(s, "RUBBER"); 
}

fl_bool isrubber(int n)
{
  return isrubberstring(scoringmethods[n].scoringname);
}

/* If successful, returns name of new method.  Otherwise NULL. */
/* New scoring method will begin on the next hand. */
char *setscoringmethod(char *method)
{
  int i, l;
  char s[100];

  /* try for exact match */
  for (i=0; i<numscoringmethods; i++)
    if (streq(method, scoringmethods[i].scoringname)) {
      newscoremethod = i;
      return method;
    }

  /* try for partial match */
  l = strlen(method);
  for (i = 0; i < numscoringmethods; i++)
    if (strneq(method, scoringmethods[i].scoringname, l)) {
      newscoremethod = i;
      return scoringmethods[i].scoringname;
    }

  strcpy(s, "Valid scoring methods are:");
  for (i = 0; i < numscoringmethods; i++)
    sprintf(s + strlen(s), " %s", scoringmethods[i].scoringname);
  errormsg(s);
  return NULL;
}

/* for hearts: numplayers then scores for each player */
/* for rubber: nsvul, ewvul, nstotal, ewtotal, nsbelowline, ewbelowline */
static char *makegstate(void)
{
  static char scores[100];
  int i;

  if (playinghearts) {
    sprintf(scores, "%d", scoringmethods[scoremethod].numplayers);
    for (i = 0; i <  scoringmethods[scoremethod].numplayers; i++)
      sprintf(scores + strlen(scores), " %d", heartsscore[i]);
  } else {
    /* assume rubber bridge */
    sprintf(scores, "%c%c%d %d %d %d",
	    nsvul ? 'V' : 'N',
	    ewvul ? 'V' : 'N',
	    nstotal, ewtotal, nsbelowline, ewbelowline);
  }

  return scores;
}

void parseheartsgstate(char *gstate, int *nscore, int *escore, int *sscore)
{
  assert(strexists(gstate));
  assert(gstate[0] == '3');
  assert(sscanf(gstate + 2, "%d %d %d", nscore, escore, sscore) == 3);
  if (*nscore == 0 && *sscore == 0 && *escore == 0)
    /* assume, unsoundly, we're starting a new game */
    strscore[0][0] = strscore[1][0] = 
      strscore[2][0] = strscore[3][0] = '\0';
}

void parserubbergstate(char *gstate,
		       fl_bool *nsvul, fl_bool *ewvul,
		       int *nstotal, int *ewtotal,
		       int *nsbelowline, int *ewbelowline)
{
  assert((gstate[0] == 'V' || gstate[0] == 'N') &&
	 (gstate[1] == 'V' || gstate[1] == 'N'));
  *nsvul = (gstate[0] == 'V');
  *ewvul = (gstate[1] == 'V');
  assert(sscanf(gstate + 2, "%d %d %d %d", 
		nstotal, ewtotal, nsbelowline, ewbelowline) == 4);
}

static void dupseed(void)
{
  int dealindex, handno;
  char *handID, *handname;

  handno = minseen(scoremethod);

  dealindex = handno * NUMPAIRGAMES + scoremethod;

  while ((++handcounter % 16) != (handno % 16));
  handID = itoa(handcounter);
  handname = TEMPCAT4(globaldate, scoringmethods[scoremethod].scoringname,
		      itoa(handno), NONCOMPETITIVE() ? "-" : "");
  selfmessage(makemsg8(NEW_HAND, handID, handname,
		       makeseed(globalseed, dealindex), "", "", "",
		       itoa(scoremethod), ""));
}

#define isheartsscorezero()				\
    (heartsscore[0] == 0 && heartsscore[1] == 0		\
     && heartsscore[2] == 0 && heartsscore[3] == 0)
			     
static void rndseed(void)
{
  char *handID, *handname;

  if (gameover) (scoringmethods[scoremethod].startup)();

  handcounter++;
  /* In the case of a new game of hearts, we want to start out with
    "pass left."  The direction of passing is determined by who deals.
     Assume, unsoundly, we're starting a new game if scores are all 0. */
  if (ishearts(scoremethod) && isheartsscorezero()) {
    while (DEALNO_TO_DEALERx(handcounter, 3) != 0) handcounter++;
  } 
  handID = itoa(handcounter);

  handname = TEMPCOPY(scoringmethods[scoremethod].scoringname);
  selfmessage(makemsg8(NEW_HAND, handID, handname,
		       makerndseed(), "", "", makegstate(),
		       itoa(scoremethod),
		       STRDUP("c1c1c1c1c1c1c1c1c1"))); /* why STRDUP? */
}

void impstartup(void) { duplicate = TRUE; gameover = FALSE; }

void mpstartup(void) { duplicate = TRUE; gameover = FALSE; }

void yukstartup(void)
{
  int i;

  gameover = duplicate = FALSE;
  for (i = 0; i < 4; i++) {
    heartsscore[i] = 0;
    strscore[i][0] = '\0';
  }
}

void rubberstartup(void)
{
  gameover = duplicate = FALSE;
  nstotal = ewtotal = nsbelowline = ewbelowline = 0;
  nsvul = ewvul = FALSE;
}

/****************************************************************************
Administrivia that is the same across all forms of scoring
****************************************************************************/

static void setcurdealer(void)
{
  curdealer = DEALNO_TO_DEALERx(atoi(curhandID), numplayers);
}

/* E.g., "27Nov98IMP12" => "12" */
static char *handname_to_handnum(char *s)
{
  char *name, *t;
  
  name = TEMPCOPY(s);
  if ((t = strchr(name, '-')) != NULL) *t = '\0'; /* truncate at '-' if any */
  for (t = name + strlen(name) - 1; isdigit(*t) && t >= name; t--);
  if (!isdigit(*t)) t++;
  return TEMPCOPY(t);
}

/* strip off the form of scoring and hand number */
/* e.g. "8Oct95IMP1" => "8Oct95" */
/* form of scoring is expected to be letters, followed optionally by digits */
char *handname_to_weekname(char *handname)
{
  int i;
  char *weekname;

  assert(strexists(handname));
  for (i=0; handname[i] != '\0'; i++);
  --i;
  if (handname[i] == '-') --i;
  while (isdigit(handname[i])) --i;
  while (isalpha(handname[i])) --i;
  weekname = TEMPCOPY(handname);
  weekname[i + 1] = '\0'; /* truncate */
  return weekname;
}

/* strip off the hand number only */
/* e.g. "8Oct95IMP1" => "8Oct95IMP" */
/* form of scoring is expected to be letters, followed optionally by digits */
char *handname_to_setname(char *handname)
{
  int i;
  char *setname;

  assert(strexists(handname));
  for (i=0; handname[i] != '\0'; i++);
  --i;
  if (handname[i] == '-') --i;
  while (isdigit(handname[i])) --i;
  setname = TEMPCOPY(handname);
  setname[i + 1] = '\0'; /* truncate */
  return setname;
}

/* read trailing digits to get the hand number */
/* e.g. "8Oct95IMP17" => 17 */
/* form of scoring is expected to end in a letter */
int handnum(char *handname)
{
  return atoi(handname_to_handnum(handname));
}

/* make (permanent) note of the fact that we've seen this hand */
static void seenhand(char *name, char *scoring)
{
  char *num, *setname;

  if (atoi(scoring) >= NUMSEENS) return;
  if (strlen(name) == 0) return;

  /* break name into root and suffix (where suffix = digits at the end) */
  setname = handname_to_setname(name);
  num = handname_to_handnum(name);

  TclDo6("seenhand ", setname, " ", num, " ", scoring);
  setseen(atoi(scoring), setname, atoi(num));
}

#define want_to_send(playername, set, num) \
  TclDo6("want_to_send {", (playername), "} ", (set), " ", (num))

/* Record "seen" information that we should tell at the end of the hand.
   If I am tablehost, try to keep track for everyone.  Otherwise, keep track
   just of my own business. */
static void must_tell(char *name, char *scoring)
{
  char *num, *setname;

  if (atoi(scoring) >= NUMSEENS) return;
  if (strlen(name) == 0) return;

  /* break name into root and suffix (where suffix = digits at the end) */
  setname = handname_to_setname(name);
  num = handname_to_handnum(name);
  
  if (tablehostmode) {
    int n;
    char **z = getwho(&n);
    while (n-- > 0) want_to_send(outputname(z[n]), setname, num);
  } else want_to_send(myoutputname, setname, num);
}

/* we've played out the whole hand---this fn computes the result */
/* contract is assumed to be a digit, a letter, and an optional x or xx */
/* results (for declarer) are put in resulttricks and resultpoints */
/* resulttricks = -N means down N; +N means making N-odd */
/* resultpoints is just the points, e.g. 140 for 2S making 3 */
void computeresult(char *contract, char *play, int declarer, fl_bool vul,
		   int *resulttricks, int *resultpoints, int *legs)
{
  int decltricks, deftricks, tricksneeded, score;
  fl_bool doubled, redoubled; /* if one is true, other is false */
  fl_bool rubber = (legs != NULL); /* is it rubber bridge? */

#if DBG
  if (hasWindows)
    printf("compute result(%s)", contract);
#endif
  if (strlen(play) == 0) {
    /* passed out */
    *resulttricks = *resultpoints = 0;
    return;
  }

#if DBG
  if (playinghearts) puts("computeresult() while playing hearts");
#endif

  computetrickswon(play, 1000, char_to_strain(contract[1]),
		   declarer, &decltricks, &deftricks);
  tricksneeded = contract[0] - '1' + 7;
  *resulttricks = (decltricks >= tricksneeded) ?
    (decltricks - 6) :
      (decltricks - tricksneeded);
  doubled = (strlen(contract) == 3);
  redoubled = doubled ? FALSE : (strlen(contract) == 4);
  if (*resulttricks > 0) {
    /* made the contract */
    int trickscore, firsttrickscore;
    
    switch (tolower(contract[1])) {
    case 'n':
      firsttrickscore = 40;
      trickscore = 30;
      break;
    case 'h':
    case 's':
      firsttrickscore = trickscore = 30;
      break;
    case 'c':
    case 'd':
      trickscore = firsttrickscore = 20;
      break;
    default:
      assert(0);
    }
    /* trick score */
    score = firsttrickscore + (tricksneeded - 7) * trickscore;
    if (doubled) score *= 2;
    else if (redoubled) score *= 4;
    if (rubber) *legs += score;
    else {
      /* game or partscore bonus */
      if (score >= 100) score += vul ? 500 : 300;
      else score += 50;
    }
    /* overtricks */
    if (doubled || redoubled)
      score += (decltricks - tricksneeded) * (vul ? 200 : 100) * (redoubled ? 2 : 1);
    else score += (decltricks - tricksneeded) * trickscore;
    /* the insult */
    if (doubled) score += 50;
    else if (redoubled) score += 100;
    /* slam bonus */
    if (tricksneeded == 12) score += vul ? 750 : 500;
    else if (tricksneeded == 13) score += vul ? 1500 : 1000;
  } else {
    /* went down */
    int nv[] = {-100, -300, -500, -800, -1100, -1400, -1700,
		  -2000, -2300, -2600, -2900, -3200, -3500};
    int v[] = {-200, -500, -800, -1100, -1400, -1700, -2000,
		  -2300, -2600, -2900, -3200, -3500, -3800};

    if (doubled || redoubled) {
      score = (vul ? v[-1 - *resulttricks] : nv[-1 - *resulttricks]);
      if (redoubled) score *= 2;
    } else score = *resulttricks * (vul ? 100 : 50);
  }
  *resultpoints = score;
}

/****************************************************************************/

static void computeheartsresult(char *play)
{
  int score[4] = {0, 0, 0, 0};
  int scoremult[4] = {1, 1, 1, 1};
  fl_bool mighthaveshot[4] = {FALSE, FALSE, FALSE, FALSE};
  fl_bool mightbeshoot = TRUE;
  int didshoot = -1; /* contains the seat who shot, or -1 if no one shot */
  int np = scoringmethods[scoremethod].numplayers;
  char trick[9];
  int len = strlen(play);
  int i, j;
  int lastwinner = -1; /* initially, we don't know */
  int winner; /* seat of winner of a given trick */
  char msg[BUFSIZ];
  char interestingcards[105] = {'\0'};
  int bestscore, numwinners;

  assert(playinghearts);

  for (i = 0; i < len / (2 * np); i++) {
    strncpy(trick, play + 2 * np * i, 2 * np);
    trick[2 * np] = '\0';
    winner = computeleader(trick, np, Notrump, lastwinner, TRUE);
#if DEBUGHEARTS
    status4("Winner of trick ", trick, " is ", itoa(winner));
#endif
    for (j = 0; j < np; j++) {
      int tmp;
      fl_bool interesting;

#if DEBUGHEARTS
      if (hasWindows) {
	fprintf(stderr, "card_to_addscore(%c %c)\n", trick[j*2], trick[j*2+1]);
      }
#endif
      tmp = card_to_addscore(trick[j*2], trick[j*2+1]);
      score[winner] += tmp;
      if (tmp > 0) mighthaveshot[winner] = TRUE;
      interesting = (tmp != 0);
      scoremult[winner] *= (tmp = card_to_multscore(trick[j*2], trick[j*2+1]));
      if (seated && (winner == myseat) && (interesting || (tmp != 1)))
	sprintf(interestingcards + strlen(interestingcards), "%c%c",
		trick[j*2], trick[j*2+1]);
    }
    lastwinner = winner;
  }

  /* Was there a moon shot? */
  for (i = 0; i < np; i++) {
    if (didshoot != -1 && mighthaveshot[i]) {
      didshoot = -1;
      mightbeshoot = FALSE;
    } else if (mightbeshoot && mighthaveshot[i]) {
      didshoot = i;
    }
  }

  if (seated)
    if (interestingcards[0] != '\0')
      status3("You took ", inorderprintcardset(interestingcards), ".");
    else
      status("You took none of the interesting cards.");

  gameover = FALSE;

  for (i = 0; i < np; i++) {
    if (didshoot == i) {
      score[i] = 0;
      if (seated && (myseat == i)) status("You shot the moon!");
      else {
	sprintf(msg, "%c shot the moon!", seat_to_char(i));
	status(msg);
      }
    }
    if (didshoot != i && didshoot != -1) 
      score[i] = SHOOT_THE_MOON_AMT;
		
    if (didshoot == -1) score[i] *= scoremult[i];
    heartsscore[i] += score[i];

    if (heartsscore[i] > HEARTS_PLAY_TO) gameover = TRUE;
  }

  /* Compute best score and number of players with that score. */
  if (gameover) {
    bestscore = (HEARTS_PLAY_TO + SHOOT_THE_MOON_AMT) * 3;
    for (numwinners = i = 0; i < np; i++) {
      if (heartsscore[i] < bestscore) {
	bestscore = heartsscore[i];
	numwinners = 1;
      } else if (heartsscore[i] == bestscore) numwinners++;
    }
  }    

  /* check for special case of moon shoot that could cause shooter to lose */
  if (gameover && didshoot != -1 && heartsscore[didshoot] != bestscore) {
    for (i = 0; i < np; i++) {
      heartsscore[i] -= SHOOT_THE_MOON_AMT;
      score[i] -= SHOOT_THE_MOON_AMT;
    }
    for (gameover = FALSE, i = 0; !gameover && (i < np); i++) 
      if (heartsscore[i] > HEARTS_PLAY_TO) gameover = TRUE;
  }

  /* Display scores. */
  for (i = 0; i < np; i++) {
    sprintf(strscore[i] + strlen(strscore[i]), "%5d", score[i]);
    sprintf(msg, "%c: %s | %d total",
	    seat_to_char(i), strscore[i], heartsscore[i]);
    status(msg);
  }
  UIheartshandover(heartsscore[North], heartsscore[East], heartsscore[South]);

  if (gameover) {
    for (msg[0] = '\0', i = 0; i < np; i++) {
      if (heartsscore[i] == bestscore) {
	if (numwinners == 1)
	  sprintf(msg, "The winner is %c with a score of %d.", 
		  seat_to_char(i), bestscore);
	else
	  sprintf(msg + strlen(msg), " and %c", seat_to_char(i));
      }
    }
    if (numwinners == 1) status(msg);
    else status4(msg + 5, " tied with a score of ", itoa(bestscore), "!");

    for (i = 0; i < np; i++) heartsscore[i] = 0;
  }

}

/*****************************************************************************/

/* Returns the name of the vulnerability for the current hand (e.g. "Both"). */
char *vulname(void)
{
  int vul = isrubber(scoremethod) ?
    (ewvul ? (nsvul ? Both : EW) : (nsvul ? NS : Nonevul)) :
    ONEB(atoi(curhandID) - 1);

  return vulnames[vul];
}


/****************************************************************************
Code for handling the `previous' command (shows the previous deal)
****************************************************************************/

static char *previous1 = NULL;
static char *previous2 = NULL;
static char *previous3 = NULL;
static char *previous4 = NULL;
static char *previous5 = NULL;

/* Set up the strings that will be output when the user asks to see
   the previous hand. */
static void setprevioushand()
{
  char s[1000];

  reset(previous1);
  reset(previous2);
  reset(previous3);
  reset(previous4);
  reset(previous5);
  if (playinghearts) {
    if (notpassing(curdealer)) {
      sprintf(s, "N: %s", prettyprintcardset(initialcards[North]));
      previous1 = STRDUP(s);
      sprintf(s, "E: %s", prettyprintcardset(initialcards[East]));
      previous2 = STRDUP(s);
      sprintf(s, "S: %s", prettyprintcardset(initialcards[South]));
      previous3 = STRDUP(s);
    } else {
      sprintf(s, "N: %s => %s",
	      prettyprintcardset(initialcards_beforepass[North]),
	      prettyprintcardset(initialcards[North]));
      previous1 = STRDUP(s);
      sprintf(s, "E: %s => %s",
	      prettyprintcardset(initialcards_beforepass[East]),
	      prettyprintcardset(initialcards[East]));
      previous2 = STRDUP(s);
      sprintf(s, "S: %s => %s",
	      prettyprintcardset(initialcards_beforepass[South]),
	      prettyprintcardset(initialcards[South]));
      previous3 = STRDUP(s);
    }
  } else {
    int top = North, bot = South, left = West, right = East;

    if (seated)
      if (myseat == North) {
	top = South;
	bot = North;
	left = East;
	right = West;
      } else if (myseat == West) {
	top = East;
	bot = West;
	left = North;
	right = South;
      } else if (myseat == East) {
	top = West;
	bot = East;
	left = South;
	right = North;
      }

    sprintf(s, "%s: %s dealt, %s",
	    curhandname,
	    seattostring[curdealer],
	    vulname());
    previous1 = STRDUP(s);
    sprintf(s, "               %s", prettyprintcardset(initialcards[top]));
    previous2 = STRDUP(s);
    sprintf(s, "%s      %s", prettyprintcardset(initialcards[left]),
	    prettyprintcardset(initialcards[right]));
    previous3 = STRDUP(s);
    if (bot != South)
      sprintf(s, "               %s (%s)",
	      prettyprintcardset(initialcards[bot]), seattostring[bot]);
    else
      sprintf(s, "               %s", prettyprintcardset(initialcards[bot]));
    previous4 = STRDUP(s);
    resultdesc(curhandname,
	       ((curdeclarer == North || curdeclarer == South) ?
		currespoints : -currespoints),
	       curastate,
	       curpstate,
	       northname == NULL ? "" : northname,
	       southname == NULL ? "" : southname,
	       eastname == NULL ? "" : eastname,
	       westname == NULL ? "" : westname,
	       NULL,
	       &previous5, NULL, &curdealer);
  }
  if (hasWindows) TclDo("menus_enable_previous");
}

/* Invoked when the user asks to see the previous hand. */
void showprevioushand(void)
{
  if (strexists(previous1)) {
    talkmsg2("Previous: ", previous1);
    talkmsg2("Previous: ", previous2);
    talkmsg2("Previous: ", previous3);
    if (strexists(previous4)) 
      talkmsg2("Previous: ", previous4);
    if (strexists(previous5)) 
      talkmsg2("Previous: ", previous5);
  } else errormsg("There is no previous hand to view");
}

/****************************************************************************/

/* Display the auction of the current hand */
void reviewauction(void)
{

  if (!playinghearts && strexists(curprettyauction))
    talkmsg2("Review: ", curprettyauction);
  else if (!playinghearts && strexists(curastate))
    talkmsg2("Review: ", prettypartialauction(curastate));
  else errormsg("There is no auction to review");
}

/* show the last full trick from the current hand */
/* the four cards are displayed in the order they were played */
void showlasttrick(void)
{
  int i, r, np = scoringmethods[scoremethod].numplayers;

  if (strexists(curpstate) && (r = strlen(curpstate)) >= 2 * np) {
    char s[100] = {'\0'};

    r = (r / (2 * np)) * 2 * np;
    for (i = r - (2 * np); i < r; i += 2)
      sprintf(s + strlen(s), "%c%c ",
	      toupper(curpstate[i]), toupper(curpstate[i + 1]));

    TclDo3("set_previous_trick {[ Previous trick: ", s, "]}");
  } else errormsg("There is no previous trick");
}

char *display_of_previous_trick(void)
{
  char *s = TclGet("previous_trick");

  return ((strexists(s)) ? s : NULL);
}

/* Modify argument s.  E.g. "12 34 56 78 " => "78 12 34 56 " */
static void rotate12x3(char *s)
{
  char a[10], b[4];

  nstrncpy(a, s, 9);
  nstrncpy(b, s + 9, 3);
  strcpy(s, b);
  strcpy(s + 3, a);
}

/* Compute what the last trick will be, and display it. */
void display_final_trick(char *handID, char *played)
{
  int deck[52], i, j;
  char s[13];

  assert(strlen(played) > 48 * 2); /* at least 49 played */
  setseed(TclDo3("gset handseed(", handID, ")"));
  deal(deck, 0, 0, 0, 0, 52);

  /* delete first 48 cards that have been played from deck */
  for (i = 0; i < 48; i++) {
    int p = 13 * char_to_strain(played[i * 2]) +
      inverse_card_to_char(played[i * 2 + 1]);

    for (j = 0; j < 52; j++) if (deck[j] == p) break;
    assert(j < 52);
    assert(deck[j] >= 0);
    deck[j] = -1;
  }

  /* put remaining 4 cards into a string, s */
  for (j = i = 0; i < 52; i++) if (deck[i] >= 0) {
    s[j++] = tolower(suit_to_char(whatsuit(deck[i])));
    s[j++] = tolower(card_to_char[whatcard(deck[i])]);
    s[j++] = ' ';
  }
  s[j] = '\0';
  assert(strlen(s) == 12);
  
  /* rotate until 49th card played is in first position in s */
  j = 0;
  while (j < 4) 
    if (strneq(s, played + 48 * 2, 2)) {
      TclDo3("set_previous_trick {[ Final trick: ", upcase(s), "]} 0");
      return;
    } else {
      rotate12x3(s);
      j++;
    }
  assert(0);
}

static void updaterubberscore(void)
{
  fl_bool curvul, nsdecl;
  int *legs, abs_currespoints;
  char s[100], *t = s;

  if ((nsdecl = (curdeclarer == North || curdeclarer == South)))
    legs = &nsbelowline;
  else legs = &ewbelowline;

  curvul = (nsdecl && nsvul) || (!nsdecl && ewvul);

  computeresult(curcontract, curpstate, curdeclarer, curvul,
		&currestricks, &currespoints, legs);
  resultdesc(curhandname,
	     currespoints,
	     curastate,
	     curpstate,
	     northname == NULL ? "" : northname,
	     southname == NULL ? "" : southname,
	     eastname == NULL ? "" : eastname,
	     westname == NULL ? "" : westname,
	     NULL,
	     &t, NULL, &curdealer);

  status(t);

  if (*legs >= 100) {
    /* game was made */
    if (curvul) {
      status2("Game and rubber to ", nsdecl ? "NS" : "EW");
      if (nsdecl) nstotal += ewvul ? 500 : 700;
      else ewtotal += nsvul ? 500 : 700;
      gameover = TRUE;
    }      
    else {
      status2("Game to ", nsdecl ? "NS" : "EW");
      if (nsdecl) nsvul = TRUE; else ewvul = TRUE;
    }
    nsbelowline = ewbelowline = 0;
  }

  abs_currespoints = (currespoints >= 0) ? currespoints : -currespoints;
  if ((nsdecl && currespoints > 0) || (!nsdecl && currespoints < 0))
    nstotal += abs_currespoints;
  else
    ewtotal += abs_currespoints;

  if (gameover) {
    status2("NS total for the rubber: ", itoa(nstotal));
    status2("EW total for the rubber: ", itoa(ewtotal));
  }
}

static void broadcastresult(void)
{
  if (!duplicate) return;

  if (!competitive)
    commseens(20);

  commresult(competitive,
	     makemsg9(RESULT,
		      curhandname,
		      itoa((curdeclarer == North || curdeclarer == South) ?
			   currespoints : -currespoints),
		      TclDo3("gset handastate(", curhandID, ")"),
		      TclDo3("gset handpstate(", curhandID, ")"),
		      TclDo3("gset handscoring(", curhandID, ")"),
		      savednorthname == NULL ? "" : savednorthname,
		      savedsouthname == NULL ? "" : savedsouthname,
		      savedeastname == NULL ? "" : savedeastname,
		      savedwestname == NULL ? "" : savedwestname),
	     curhandname);
}

/* called once per hand, when the auction and play (if any) are over */ 
static void endofhand(void)
{
  fl_bool curvul;
  int vul;
  
  if (playinghearts) computeheartsresult(curpstate);
  else if (isrubber(scoremethod)) updaterubberscore();
  else if (DUPLICATE(scoremethod)) {
    vul = ONEB(atoi(curhandID) - 1);
    curvul = (vul == Both ||
	      ((curdeclarer == North || curdeclarer == South) && vul == NS) || 
	      ((curdeclarer == East || curdeclarer == West) && vul == EW));
    computeresult(curcontract, curpstate, curdeclarer, curvul,
		  &currestricks, &currespoints, NULL);
    broadcastresult();
  } else assert(0);

  if (hasWindows) TclDo("menus_noclaim");

  setprevioushand();
  TclDo("autonewdeal");
}

/* put player's 13 (or 17) cards into dest, e.g. "sasksq..." */
static void decktohand(int *deck, int player, char *dest)
{
  int i, icard;
  char suits[] = "cdhs";

  for (i = 0; i < cardsperhand; i++) {
    icard = deck[i + cardsperhand * player];
    *dest++ = suits[whatsuit(icard)];
    *dest++ = tolower(card_to_char[whatcard(icard)]);
  }
  *dest = '\0';
}

int char2int_card(char c){
    if((c - '2') < 10) return (int)(c-'2');
    if( c == 'A')return 12;
    if (c=='K') return 11;
    if (c=='Q') return 10;
    if (c=='J') return 9;
    if (c=='T') return 8;
}
/*
0 AT32.6.872.98765 
 */
void setup_bridge_deck(char *seed){
    int seat = *seed - '0';
    int suit = 3;
    int pos = 0;
    seed += 2;
    while (*seed != 0) {
        if (*seed == '|'){
            seat = *(seed+1) - '0';
            suit = 3;
            pos = 0;
            seed += 3;
        } else if (*seed == '.') {
            suit -= 1;
            seed ++;
        }        
        curdeck[seat*13+pos] = suit*13 + char2int_card(*seed) ;
        seed++;
        pos ++;
    }
    int i;
    for(i=0;i<52;i++)printf("%d ",(curdeck[i] %13)+2);
    printf("new deal\n");
}
/*
seed is somethin like 9\15\aiJfmB01x7CKykEtLawMswKYg4lqEHv1jm8Id5vLWxzFOoNZbvDQQG_
 */
static void setupdeck(char *seed)
{
  int i;
  int decksize = scoringmethods[scoremethod].numplayers * cardsperhand;


  printf("setupdeck seed=%s\n", seed);

  if (*(seed+1) == ' ') setup_bridge_deck(seed); //-yisu hajack the program
  // don't understand the implementation that generate the deck from seed
  else {
  setseed(seed);
  deal(curdeck, 0, 0, 0, 0, decksize);

  sortdeal(curdeck, decksize, cardsperhand);
  }
  for (i = 0; i < scoringmethods[scoremethod].numplayers; i++) {
    decktohand(curdeck, i, initialcards[i]);
    if (playinghearts) strcpy(initialcards_beforepass[i], initialcards[i]);
  }
  if (playinghearts) initialcards[West][0] = '\0';
  //code to hack the generated deal,
  FILE *f = fopen("curdeck.tmp","w");
  for(i=0;i<52;i++)fprintf(f,"%d ",curdeck[i]);
  fprintf(f,"\n");
  fclose(f);
  //-yisu
}

/* returns whether m represents new information */
fl_bool filehandinfo(message *m)
{
  extern fl_bool myturntoact; /* for the UI */
  fl_bool really_new;

  switch (m->kind) {

  case NEW_HAND: printf("filehandinfo NEW_HAND\n");
    if ((really_new = (curhandID == NULL || !streq(curhandID, m->args[0])))) {
      if (hasWindows) TclDo("menus_newhand");
      TclDo("reset_previous_trick");
      must_tell(m->args[1], m->args[6]);
      /* nothing to remember -yisu seenhand(m->args[1], m->args[6]); */
      reset(curhandID);
      reset(curhandname);
      reset(curprettyauction);
      curhandID = STRDUP(m->args[0]);
      curhandname = STRDUP(m->args[1]);
      assert(curhandname[0] != '\0');
      competitive = (curhandname[strlen(curhandname) - 1] != '-');
      handplayedout = FALSE;

      /* the following is fairly hacked xxx */
      if (ishearts(scoremethod = atoi(m->args[6]))) {
	playinghearts = TRUE;
	numplayers = 3;
	cardsperhand = 17;
	parseheartsgstate(m->args[5],
			  heartsscore, heartsscore+1, heartsscore+2);
	reset(curpassstate);
	curpassstate = STRDUP(m->args[7]);
	passset[0] = '\0';
	passdone = FALSE;
      } else {
	playinghearts = FALSE;
	numplayers = 4;
	cardsperhand = 13;
	if (isrubberstring(m->args[6]))
	  parserubbergstate(m->args[5], &nsvul, &ewvul,
			    &nstotal, &ewtotal, &nsbelowline, &ewbelowline);
      }
		
    }
    curtrump = -1;
    claiming = FALSE;
    curcontract[0] = '\0';
    setcurdealer();
    if (really_new) {
      fl_bool in_progress = (strexists(m->args[3]) || strexists(m->args[4])
			  || strexists(m->args[7]));
      status5(in_progress ? "Playing " : "New hand is ", m->args[1],
	      "; ", seattostring[curdealer], " dealt");
    }
    TclDo5("gset handname(", m->args[0], ") {", m->args[1], "}");
    TclDo5("gset handseed(", m->args[0], ") {", m->args[2], "}");
    dTclDo3("gset handpstate(", m->args[0], ") {}");
    dTclDo3("gset handastate(", m->args[0], ") {}");
    TclDo5("gset handgstate(", m->args[0], ") {", m->args[5], "}");
    TclDo5("gset handscoring(", m->args[0], ") {", m->args[6], "}");
    TclDo5("gset handpassstate(", m->args[0], ") {", m->args[7], "}");


    setupdeck(m->args[2]);
    really_new |= filehandinfo(makemsg2(AUCTION_STATUS,
					m->args[0], m->args[3]));
    really_new |= filehandinfo(makemsg2(PLAY_STATUS, m->args[0], m->args[4]));
    if (ishearts(atoi(m->args[6])))
      really_new |= filehandinfo(makemsg2(PASS_STATUS,
					  m->args[0], m->args[7]));
    return really_new;

  case AUCTION_STATUS:
#if DBG
    if (!atob(TclDo3("global handastate; info exists handastate(", m->args[0], ")")))
      if (hasWindows)
	printf("%s: NO HANDASTATE! (%s, %s)\n", myoutputname,
	       m->args[0], m->args[1]);
#endif
    if (strlen(m->args[1]) != 0 &&
	atob(TclDo3("global handastate; info exists handastate(", m->args[0], ")")) &&
	(strlen(TclDo3("gset handastate(", m->args[0], ")")) >=
	 strlen(m->args[1])))
      return FALSE;
    /* info about other than the current hand? */
    if (curhandID == NULL || !streq(curhandID, m->args[0])) return FALSE;
    dTclDo5("gset handastate(", m->args[0], ") {", m->args[1], "}");
    reset(curastate);
    curastate = STRDUP(m->args[1]);
    if (auctionisover(curastate)) {
      if (handIDsavednames == NULL || !streq(handIDsavednames, curhandID)) {
	reset(handIDsavednames);
	reset(savednorthname);
	reset(savedsouthname);
	reset(savedeastname);
	reset(savedwestname);
	handIDsavednames = STRDUP(curhandID);
	if (strexists(northname)) savednorthname = STRDUP(northname);
	if (strexists(southname)) savedsouthname = STRDUP(southname);
	if (strexists(eastname)) savedeastname = STRDUP(eastname);
	if (strexists(westname)) savedwestname = STRDUP(westname);
      }
      computecontract(curastate, curdealer,
		      curcontract, &curdeclarer, &curtrump);
      if (hasWindows) {
	TclSet("contract_tricks",
	       isdigit(curcontract[0]) ? itoa(curcontract[0] - '0' + 6) : "0");
	TclDo2("menus_declaring ", itoa(declaring()));
      }
      reset(curprettyauction);
      if (!playinghearts) curprettyauction = STRDUP(prettyauction(curastate));
      if (passedout(curastate)) {
	reset(curpstate);
	curpstate = STRDUP("");
	status("Passed out!");
	endofhand();
      }
    }
    myturntoact = seated && (silentmyturntobid() || silentmyturntoplay());
    return TRUE;

  case PASS_STATUS:
    if (strlen(m->args[1]) != 0 &&
	atob(TclDo3("global handpassstate; info exists handpassstate(", m->args[0], ")")) &&
	!isnewinfo(TclDo3("gset handpassstate(", m->args[0], ")"), m->args[1]))
      return FALSE;
	
    if (curhandID == NULL || !streq(curhandID, m->args[0])) return FALSE;
    dTclDo5("gset handpassstate(", m->args[0], ") {", m->args[1], "}");
    updatepassstate(curpassstate, m->args[1]);
    if (seated) {
      passdone = mypassdone(curpassstate);
#if HEARTSDEBUG
      status(passdone ? "You have passed" : "You have not passed");
      status2("curpassstate = ", curpassstate);
#endif
    }
    if (passisover(curpassstate, curdealer)) {
      adjusthands(curpassstate, curdealer, curdeck);
    } else {
      reportpassstatus(curpassstate);
    }
    myturntoact = seated && silentmyturntoplay();
    return TRUE;

  case PLAY_STATUS:
#if DBG
    if (hasWindows) {
      printf("%s: playstat %s (old=%s)\n", myoutputname, m->args[1],
	     TclDo3("gset handpstate(", m->args[0], ")"));
      if (!atob(TclDo3("global handpstate; info exists handpstate(", m->args[0], ")")))
	printf("%s: NO HANDPSTATE! (%s, %s)\n", myoutputname,
	       m->args[0], m->args[1]);
      else if (!isin('!', m->args[1]) &&
	       (strlen(TclDo3("gset handpstate(", m->args[0], ")")) >=
		strlen(m->args[1])))
	printf("%s: got %s, but already knew %s\n", myoutputname,
	       m->args[1], TclDo3("gset handpstate(", m->args[0], ")"));
    }
#endif
    if (strlen(m->args[1]) != 0 &&
	atob(TclDo3("global handpstate; info exists handpstate(", m->args[0], ")")) &&
	!isin('!', m->args[1]) &&
	(strlen(TclDo3("gset handpstate(", m->args[0], ")")) >=
	 strlen(m->args[1])))
      return FALSE;
    TclDo5("gset handpstate(", m->args[0], ") {", m->args[1], "}");
    /* return FALSE if info is about other than the current hand */
    if (curhandID == NULL || !streq(curhandID, m->args[0])) return FALSE;

    reset(curpstate);
    curpstate = STRDUP(m->args[1]);
    if (playisover(curastate, curpstate)) endofhand();
    if (numcards_(curpstate) % 4 == 0) {
      if (numcards_(curpstate) < 52) TclDo("reset_previous_trick");
      if (hasWindows && declaring()) {
	int decltricks, deftricks;

	computetrickswon(curpstate, 1000, char_to_strain(curcontract[1]),
			 curdeclarer, &decltricks, &deftricks);
	TclDo4("update_claimmenu ", itoa(decltricks), " ", itoa(deftricks));
      }
    }
    myturntoact = seated && (silentmyturntobid() || silentmyturntoplay());
    return TRUE;
  default:
    assert(0);
  }
}

/* parses auction and sets contract, declarer, and trump */
void computecontract(char *auction, int dealer, char *contract,
		     int *declarer, int *trump)
{
  int i, j;
  fl_bool doubled, redoubled, passout;

  if (playinghearts) {
    sprintf(contract, "1N Yuk Hearts"); /* why?? */
    *declarer = -1;
    *trump = Notrump;
    return;
  }

  i = strlen(auction) - 8;
  assert(i >= 0);
  doubled = (auction[i+1] == 'x');
  redoubled = (auction[i+1] == 'r');
  passout = (auction[i+1] == 'p');
  if (passout) {
    strcpy(contract, "P.O.");
    *declarer = -1;
    *trump = Notrump;
    return;
  }
  if (redoubled)
    /* if redoubled, find the double */
    do { i -= 2; } while (auction[i+1] != 'x');
  if (doubled || redoubled)
    /* go back to find the contract that was doubled */
    do { i -= 2; } while (auction[i+1] == 'p');
  j = 0;
  contract[j++] = auction[i];
  contract[j++] = toupper(auction[i+1]);
  if (doubled || redoubled) contract[j++] = 'x';
  if (redoubled) contract[j++] = 'x';
  contract[j] = '\0';

  /* find first mention of the strain by declarer's side */
  for (j = i & 2; j < i; j += 4) if (auction[j+1] == auction[i+1]) break;
  *declarer = ((j / 2) + dealer) % 4;

  assert(contract[0] >= '1' && contract[0] <= '7');
  *trump = char_to_strain(contract[1]);
}

/* return a string representing the final contract */
/* caller should free the result */
char *contract(char *auction)
{
  char result[100];
  int decl, trump;
  
  computecontract(auction, 0, result, &decl, &trump);
  return STRDUP(result);
}

char seat_to_char(int seat)
{
  switch (seat) {
  case North: return 'N';
  case South: return 'S';
  case East: return 'E';
  case West: return 'W';
  default: return '0';
  }
}

/* return a character representing the declarer ('N', 'S', 'E', 'W', or '0') */
/* caller should free the result */
char declarer(char *auction, int dealer)
{
  char contract[100];
  int decl, trump;
  
  computecontract(auction, dealer, contract, &decl, &trump);
  return seat_to_char(decl);
}

/* convert the first bid in the given auction to text e.g. "P", "XX", "4S" */
/* caller should not modify or free the return value */
char *bid_to_text(char *auction)
{
  char result[] = {'\0', '\0', '\0', '\0'};
  static char R[] = "XX";
  
  if (auction[0] == ' ') result[0] = toupper(auction[1]);
  else {
    result[0] = toupper(auction[0]);
    result[1] = toupper(auction[1]);
  }
  return (streq(result, "R") ? R : TEMPCOPY(result));
}

/* convert auction to a "pretty" string, e.g. "1S (X) 2S 4S" */
/* marks return string as dead */
char *prettyauction(char *auction)
{
  char result[1000];
  int decl, trump, len, i;

  len = strlen(auction) - 6; /* don't count final three passes */

  /* figure out which side is declaring */
  computecontract(auction, North, result, &decl, &trump);

  /* if dealer's side is declaring, start from first bid made, otherwise 2nd */
  i = (passedout(auction) || decl == North || decl == South) ? 0 : 2;

  result[0] = '\0'; /* clear the result */
  while (i <= len && strlen(result) < 990) {
    /* print interfering call, if not a pass, in parens */
    if (i > 0 && auction[i-1] != 'p')
      sprintf(result + strlen(result),
	      ((i % 8 == 2 && i > 2) ? "; (%s)" : " (%s)"),
	      bid_to_text(auction + i - 2));
    /* print call from declaring side whether it is a pass or not */
    if (i < len) /* don't print any of the 3 passes ending the auction */
      sprintf(result + strlen(result),
	      ((i % 8 == 0 && i > 0) ? "; %s" : " %s"),
	      bid_to_text(auction + i));
    i += 4;
  }
  if (i <= len && strlen(result) >= 990)
    sprintf(result + strlen(result), "...");
  return TEMPCOPY(result + 1);
}  

/* convert auction to a "pretty" string, e.g. "1S (X) 2S 4S" */
/* unlike prettyauction(), does not assume the auction is complete */
/* marks return string as dead */
static char *prettypartialauction(char *auction)
{
  char result[1000];
  int decl, len, i;

  len = strlen(auction);

  decl = (validseat(myseat) ? myseat : South);

  /* if dealer's side is declaring, start from first bid made, otherwise 2nd */
  i = (passedout(auction) || decl == North || decl == South) ? 0 : 2;

  result[0] = '\0'; /* clear the result */
  while (i <= len && strlen(result) < 990) {
    /* print their call, if not a pass, in parens */
    if (i > 0 && auction[i-1] != 'p')
      sprintf(result + strlen(result),
	      ((i % 8 == 2 && i > 2) ? "; (%s)" : " (%s)"),
	      bid_to_text(auction + i - 2));
    /* print call from our side whether it is a pass or not */
    sprintf(result + strlen(result),
	    ((i % 8 == 0 && i > 0) ? "; %s" : " %s"),
	    bid_to_text(auction + i));
    i += 4;
  }
  if (i <= len && strlen(result) >= 990)
    sprintf(result + strlen(result), "...");
  return TEMPCOPY(result + 1);
}  

/* Output status of the (generally partial) auction in the following format:
   1C p 1D 2s; 3C 3s X p; ...
   Show our calls (or NS's if not seated) in caps, theirs in lowercase. */
static char *statusauction(void)
{
  static char *cached_question = NULL;
  static char *cached_answer = NULL;
  char *s, *auction = curastate, *t;
  int len, i, seat, calls, parity;

  assert(strexists(curhandname));
  if (!strexists(auction) || auctionisover(auction)) return "";

  if (cached_question == NULL) {
    cached_question = STRDUP("");
    cached_answer = STRDUP("");
  }

  if (streq(cached_question, auction)) return cached_answer;
  reset(cached_question);
  reset(cached_answer);
  cached_question = STRDUP(auction);
  cached_answer = s = (char *) salloc(1000);

  seat = validseat(myseat) ? myseat : South;
  parity = (seat + curdeclarer) & 1;
  len = strlen(auction);
  i = calls = 0;
  strcpy(s, "Auction so far: ");
  while (i < len) {
    if (strlen(s) > 990) return (strcat(s, "..."), s);
    t = bid_to_text(auction + i);
    strcat(s, ((((i / 2) & 1) == parity) ? downcase(t) : t));
    if (++calls % 4 == 0) strcat(s, "; "); else strcat(s, " ");
    i += 2;
  }
  strcat(s, "?");
  return s;
}

/* Compute whose lead it is.  Assume 3-handed hearts.  If
   openingleader is negative, try to compute it by looking to see who has
   the 3 of clubs.  */
/* BUG: uses (global) initialcards, which should be a parameter if looking at
   one hand while playing another is allowed. */
int computeheartsleader(char *pstate, int cardsplayed, int openingleader)
{
  int i;
  char *trick = pstate + 2 * cardsplayed - 6;

  assert((cardsplayed % scoringmethods[scoremethod].numplayers) == 0);
  if (cardsplayed == 0) {
    if (openingleader < 0) {
      /* who has the 3 of clubs? needs fix for 4 handed, relies
	 on not being expected to return a column */
      int i;
      for (i = 0; i < scoringmethods[scoremethod].numplayers; i++) {
	if (cardmember("c3", initialcards[i])) return(i);
      }
      assert(0);
    } else return openingleader;
  } else {
    int winningrank, winningsuit;
    int firstcard = char_to_rank(trick[1]);
    int secondcard = char_to_rank(trick[3]);
    int thirdcard = char_to_rank(trick[5]);
    char firstsuit = trick[0];
    char secondsuit = trick[2];
    char thirdsuit = trick[4];
    int leader;
    
    leader = computeheartsleader(pstate, cardsplayed - 3, openingleader);

    /* notrump */
    winningrank = firstcard;
    winningsuit = firstsuit;
    if (secondsuit == firstsuit)
      winningrank = max(secondcard, winningrank);
    if (thirdsuit == winningsuit)
      winningrank = max(thirdcard, winningrank);
    
#if 0
    {
      char s[100];
      sprintf(s, "Winning card for trick %d is %c%c", cardsplayed/scoringmethods[scoremethod].numplayers,
	   winningsuit, card_to_char[winningrank]);
      puts(s);
      DEBUG(status(s));
    }
#endif

    /* now we have the winning rank and suit; find who played that card */
    for (i=1; i<4; i++) /* for 4-handed use: for (i=0; i<4; i++) */
      if (winningrank == char_to_rank(pstate[2*(i+cardsplayed)-7]) &&
	  winningsuit == pstate[2*(i+cardsplayed)-8]) break;
    assert(i<4);
    return ((leader - 1 + i) % scoringmethods[scoremethod].numplayers);
  }
}


/*
compute whose lead it is:
    if declarer is a column it returns a column,
    if declarer is a seat it returns a seat
*/    
int computeleader(char *pstate, int cardsplayed, int trumpsuit, int declarer,
		  fl_bool hearts)
{
  int i;

  if (hearts) return computeheartsleader(pstate, cardsplayed, declarer);
  assert((cardsplayed % 4) == 0);
  if (cardsplayed == 0) return ((declarer + 1) % 4);
  else {
    char suits[] = "cdhs";
    int winningrank, winningsuit;
    int firstcard = char_to_rank(pstate[2*cardsplayed-7]);
    int secondcard = char_to_rank(pstate[2*cardsplayed-5]);
    int thirdcard = char_to_rank(pstate[2*cardsplayed-3]);
    int fourthcard = char_to_rank(pstate[2*cardsplayed-1]);
    char firstsuit = pstate[2*cardsplayed-8];
    char secondsuit = pstate[2*cardsplayed-6];
    char thirdsuit = pstate[2*cardsplayed-4];
    char fourthsuit = pstate[2*cardsplayed-2];
    int leader;
    
    leader = computeleader(pstate, cardsplayed - 4, trumpsuit, declarer,
			   hearts);
    winningrank = -1;
    if (trumpsuit != Notrump) {
      char trumpchar = suits[trumpsuit];

      if (firstsuit == trumpchar)
	winningrank = firstcard;
      if (secondsuit == trumpchar)
	winningrank = max(secondcard, winningrank);
      if (thirdsuit == trumpchar)
	winningrank = max(thirdcard, winningrank);
      if (fourthsuit == trumpchar)
	winningrank = max(fourthcard, winningrank);
      if (winningrank != -1) winningsuit = trumpchar;
    }
    if (winningrank == -1) {
      /* notrump contract or trump contract with no trumps played */
      winningrank = firstcard;
      winningsuit = firstsuit;
      if (secondsuit == firstsuit)
	winningrank = max(secondcard, winningrank);
      if (thirdsuit == firstsuit)
	winningrank = max(thirdcard, winningrank);
      if (fourthsuit == firstsuit)
	winningrank = max(fourthcard, winningrank);
    }
    
#if 0
    {
      char s[100];
      sprintf(s, "Winning card for trick %d is %c%c", cardsplayed/4,
	   winningsuit, card_to_char[winningrank]);
      puts(s);
      DEBUG(status(s));
    }
#endif
   /* now we have the winning rank and suit; find who played that card */
    for (i=0; i<4; i++)
      if (winningrank == char_to_rank(pstate[2*(i+cardsplayed)-7]) &&
	  winningsuit == pstate[2*(i+cardsplayed)-8]) break;
    assert(i<4);
    /* if i is 0, that means leader of this trick; 1 means next player; etc. */
    return ((leader + i) % 4);
  }
}

fl_bool myturntobid(void)
{
  if (!seated || curhandID == NULL || curastate == NULL) return FALSE;
  if (auctionisover(curastate)) {
    errormsg("The auction is over");
    return FALSE;
  }
  
  return (((curdealer + numcalls(curastate)) % 4) == myseat);
}

/* is it my turn to play a card (either from my own hand or from dummy)?  */
fl_bool myturntoplay(void)
{
  int leader, played;

  if (!seated || curastate == NULL || curhandID == NULL || curpstate == NULL)
    return FALSE;

  if (playinghearts && seated && myseat == West)
    return FALSE;

  if (playinghearts && !passisover(curpassstate, curdealer))
    return !passdone;

  if (playisover(curastate, curpstate) || handplayedout)
    return FALSE;

  if (!auctionisover(curastate))
    return FALSE;

  if (((curdeclarer ^ myseat) == 2) && !playinghearts) {
    errormsg("You are dummy");
    return FALSE;
  }

  played = numcards(curpstate);
  leader = computeleader(curpstate,
			 played -
			 (played %
			  (scoringmethods[scoremethod].numplayers)),
			 curtrump, curdeclarer, playinghearts);
  
  if (curdeclarer == myseat && !playinghearts) /* I am declarer */
    return (((played + leader) % 2) == (myseat % 2));
  else return (((played + leader) %
		scoringmethods[scoremethod].numplayers) == myseat);
}

fl_bool silentmyturntobid(void)
{
  fl_bool r;

  if (!atob(TclGet("showerrors"))) return myturntobid();
  TclSet("showerrors", "0");
  r = myturntobid();
  TclSet("showerrors", "1");
  return r;
}

fl_bool silentmyturntoplay(void)
{
  fl_bool r;

  if (!atob(TclGet("showerrors"))) return myturntoplay();
  TclSet("showerrors", "0");
  r = myturntoplay();
  TclSet("showerrors", "1");
  return r;
}

/* given that it is either my (declarer's) turn or dummy's turn,
   is it dummy's turn? */
fl_bool dummyturntoplay(void)
{
  int leader;
  int numtricksplayed, numcardsplayed;

  if (playinghearts) return(FALSE);

  numcardsplayed = numcards_(curpstate);
  assert(numcardsplayed < 52);
  numtricksplayed = numcardsplayed / 4;
  leader = computeleader(curpstate, 4 * numtricksplayed,
			 curtrump, curdeclarer, playinghearts);
  
  return (((numcardsplayed + leader) % 4) == (myseat ^ 2));
}

fl_bool auctionisover(char *auction)
{
  if (playinghearts) return TRUE;

  {
    int len = strlen(auction);
    return (len >= 8 && streq(" p p p", auction + len - 6));
  }
}

fl_bool playisover(char *auction, char *play)
{
  if (playinghearts) return (numcards(play) == 51); /* fix for 4 handed */

  return ((auctionisover(auction) &&
	   (passedout(auction) || numcards_(play) == 52)));
}

static fl_bool isdoublelegal(char *auction)
{
  int len = strlen(auction);

  if (auctionisover(auction)) return FALSE;
  if (strlen(auction) < 2) return FALSE;
  if (isdigit(auction[len - 2])) return TRUE;
  if (strlen(auction) < 6) return FALSE;
  if (auction[len - 1] == 'p' && auction[len - 3] == 'p' &&
      isdigit(auction[len - 6])) return TRUE;
  return FALSE;
}
  
static fl_bool isredoublelegal(char *auction)
{
  int len = strlen(auction);

  if (auctionisover(auction)) return FALSE;
  if (strlen(auction) < 4) return FALSE;
  if (auction[len - 1] == 'x') return TRUE;
  if (strlen(auction) < 8) return FALSE;
  if (auction[len - 1] == 'p' && auction[len - 3] == 'p' &&
      auction[len - 5] == 'x') return TRUE;
  return FALSE;
}
  
static fl_bool sufficientbid(char *bid, char *auction)
{
  int i;
  char *strains = "cdhsn";

  for (i = strlen(auction) - 2; i >= 0 && !isdigit(auction[i]); i -= 2);
  if (i < 0) return TRUE; /* no prior bids */
  if (bid[0] > auction[i]) return TRUE; /* new bid is at a higher level */
  if ((bid[0] == auction[i]) &&
      (strchr(strains, bid[1]) > strchr(strains, auction[i+1]))) return TRUE;
  return FALSE;
}

/* returns whether it succeeded */
fl_bool executebid(char *b)
{
  fl_bool ok = FALSE;
  char *new;

  if (playinghearts) return(FALSE);

/*  printf("executebid(%s)\n", b);*/

  b = downcase(b);
  switch(b[0]) {
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
    ok = ((isin(b[1], "cdhsn") && b[2] == '\0') || streq(b + 1, "nt"));
    break;
  case 'p':
    ok = (streq(b, "p") || streq(b, "pa") ||
	  streq(b, "pas") || streq(b, "pass"));
    break;
  case 'd':
  case 'x':
    if (streq(b, "double") || 
	streq(b, "doubl") || 
	streq(b, "doub") || 
	streq(b, "dou") || 
	streq(b, "do") || 
	streq(b, "d") || 
	streq(b, "x")) {
      b[0] = 'x';
      ok = isdoublelegal(curastate);
      break;
    } /* else fall through */
  case 'r':
    if (streq(b, "redouble") || 
	streq(b, "redoubl") || 
	streq(b, "redoub") || 
	streq(b, "redou") || 
	streq(b, "redo") || 
	streq(b, "red") || 
	streq(b, "re") || 
	streq(b, "r") ||
	streq(b, "xx")) {
      ok = isredoublelegal(curastate);
      b[0] = 'r';
    }
    break;
  }    
  if (!ok) {
    errormsg("That isn't a legal call");
    return FALSE;
  }

  if (isdigit(b[0])) if (!sufficientbid(b, curastate)) {
    errormsg("That's an insufficient bid");
    return FALSE;
  }

  /* it's a legal call */
  new = markgarbage(salloc(strlen(curastate) + 4));
  strcpy(new, curastate);
  if (isdigit(b[0])) {
    b[2] = '\0';
    strcat(new, b);
  }
  else sprintf(new + strlen(new), " %c", b[0]);
  selfmessage(makemsg2(AUCTION_STATUS, curhandID, new));
  return TRUE;
}

fl_bool cardmember(char *card, char *cards)
{
  while (*cards != '\0')
    if (strncmp(card, cards, 2) == 0) return TRUE;
    else cards += 2;
  return FALSE;
}

fl_bool cardmembern(char *card, char *cards, int n)
{
  int i = 0;

  while (*cards != '\0' && i++ < n)
    if (strncmp(card, cards, 2) == 0) return TRUE;
    else if (i < n) cards += 2;
  return FALSE;
}

fl_bool suitmember(char suit, char *cards)
{
  while (*cards != '\0')
    if (suit == *cards) return TRUE;
    else cards += 2;
  return FALSE;
}

static void destructivelyremovefirstcard(char *s)
{
  char *t;

  t = s + 2;
  while ((*s++ = *t++) != '\0');
}

/* Viewing s1 and s2 as sets of cards (e.g. "ckcj" means club king and jack),
   return the set difference s1 minus s2.
   (Nondestructive; answer marked as dead.) */
char *cardsetdiff(char *s1, char *s2)
{
  char *r, *t;

  r = t = TEMPCOPY(s1);
  while (*t != '\0')
    if (cardmember(t, s2)) destructivelyremovefirstcard(t);
    else t += 2;
  return r;
}

char *remainingcards(int seat)
{
  return (cardsetdiff(initialcards[seat], curpstate));
}

/* Assumes w is a string of characters, 2 characters per card. */
/* Returns a "prettier" representation, suitable for output to the user. */
char *prettyprintcardset(char *w)
{
  char spades[14], hearts[14], diamonds[14], clubs[14], ret[100];
  char *s, *h, *d, *c, suit, card;

  if (!strexists(w)) return "None";
  
  assert((strlen(w) & 1) == 0);
  s = spades;
  h = hearts;
  d = diamonds;
  c = clubs;
  while (*w != '\0') {
    suit = toupper(*w);
    w++;
    card = toupper(*w);
    w++;
    switch (suit) {
    case 'S': *s++ = card; break;
    case 'H': *h++ = card; break;
    case 'D': *d++ = card; break;
    case 'C': *c++ = card; break;
    default: assert(0);
    }
  }
  *s = *h = *d = *c = '\0';
  sprintf(ret, "S %s H %s D %s C %s",
	  ((s == spades) ? "-" : spades),
	  ((h == hearts) ? "-" : hearts),
	  ((d == diamonds) ? "-" : diamonds),
	  ((c == clubs) ? "-" : clubs));
  return TEMPCOPY(ret);
}

/* Assumes w is a string of characters, 2 characters per card. */
/* Returns a "prettier" representation, suitable for output to the user. */
/* Prints the cards in the order given in w. */
static char *inorderprintcardset(char *w)
{
  char suit, card, oldsuit = '_', ret[100] = {'\0'}; 
  if (!strexists(w)) return "None";
  
  assert((strlen(w) & 1) == 0);
  while (*w != '\0') {
    suit = toupper(*w);
    w++;
    card = toupper(*w);
    w++;
    if (suit != oldsuit) {
      if (ret[0] == '\0')
	sprintf(ret + strlen(ret), "%c %c", toupper(suit), toupper(card));
      else
	sprintf(ret + strlen(ret), " %c %c", toupper(suit), toupper(card));
      oldsuit = suit;
    } else sprintf(ret + strlen(ret), "%c", toupper(card));
  }
  return TEMPCOPY(ret);
}

/* Show the cards held at the beginning of the hand for the named
seat, or for myseat (or South if not seated) if the named seat is invalid. */
void showinitialcards(int seat)
{
  char s[100];
  
  if (!validseat(seat)) seat = validseat(myseat) ? myseat : South;
  sprintf(s, "Initial cards for %c: %s", seat_to_char(seat),
	  prettyprintcardset(initialcards[seat]));
  status(s);
}

/* what suit was led to the most recent trick? */
char suitled(char *pstate)
{
  int len = strlen(pstate);

  assert((len % (2 * scoringmethods[scoremethod].numplayers)) != 0);
  return pstate[(len / (2 * scoringmethods[scoremethod].numplayers)) * 
				(2 * scoringmethods[scoremethod].numplayers)];
}

/* p should be "c2" or "c3" or ... or "sk" or "sa" */
/* returns whether it succeeded */
fl_bool executeplay(char *p)
{
  char *new, s[200], *remcards, led;
  int cardsplayed;

  while (isspace(*p)) p++;
  p = downcase(p);
  if (strlen(p) > 100) { p[90] = p[91] = p[92] = '.'; p[93] = '\0'; }
  if (strlen(p) == 1) return follow(p);
  if (strlen(p) != 2 || !isin(p[0], "cdhs") || !isin(p[1], "23456789tjqka")) {
    sprintf(s, "Illegal card play (%s): expecting `C2' or `C3' or ... or `SK' or `SA'", p);
    errormsg(s);
    return FALSE;
  }

  if (curdeclarer == myseat && dummyturntoplay()) {
    if (!cardmember(p, remcards = remainingcards(myseat ^ 2))) {
      sprintf(s, "It is dummy's turn to play, and dummy doesn't have the %s",
	      p);
      errormsg(s);
/*      printf("%s: %s (dummy has %s)\n", myoutputname, p, remcards);*/
      return FALSE;
    }
  } else {
    if (!cardmember(p, remcards = remainingcards(myseat))) {
      sprintf(s, "You can't play a card (%s) that you don't have", p);
      errormsg(s);
/*      printf("%s: %s (have %s)\n", myoutputname, p, remcards);*/
      return FALSE;
    }
  }

  cardsplayed = numcards_(curpstate);
  if (cardsplayed == 52) return FALSE; /* shouldn't happen */

  if (playinghearts) {
    if (!passisover(curpassstate, curdealer)) {
      heartspass(p);
      return TRUE;
    }

    if (MUST_LEAD_CLUB3 && cardsplayed == 0 && !streq(p, "c3")) {
      errormsg("The first lead must be the club 3");
      return FALSE;
    }
    
    if (MUST_BREAK_HEARTS && p[0] == 'h' &&
	(cardsplayed % scoringmethods[scoremethod].numplayers == 0) &&
	strchr(curpstate, 'h') == NULL &&
	(isin('c', remainingcards(myseat)) ||
	 isin('d', remainingcards(myseat)) ||
	 isin('s', remainingcards(myseat)))) {
      /* We're playing hearts, this is a heart lead, 
	 hearts haven't been played,
	 and I'm holding something other than hearts. */
      errormsg("Hearts haven't been broken, you can't lead hearts");
      return FALSE;
    }
  }

  if ((strlen(curpstate) % (2*scoringmethods[scoremethod].numplayers)) != 0 &&
      p[0] != (led = suitled(curpstate)) &&
      suitmember(led, remcards)) {
    errormsg("You must follow suit");
/*    printf("%s: %s (but %c led)\n", myoutputname, p, led);*/
    return FALSE;
  }

  new = markgarbage(salloc(strlen(curpstate) + 4));
  sprintf(new, "%s%s", curpstate, p);
  selfmessage(makemsg2(PLAY_STATUS, curhandID, new));
  if (strlen(curpstate) < 48 * 2) TclDo("reset_previous_trick");
  if (numcards_(new) == 52) handplayedout = TRUE;
  return TRUE;
}

fl_bool passisover(char *passstate, int dealer)
{
  int i = 0;

  /* passstate is initialized to everyone passing 3 c1's */
  if (!playinghearts) return TRUE;
  if (notpassing(dealer)) return TRUE;
  while (i < 3 * scoringmethods[scoremethod].numplayers) {
    if (passstate[2 * i + 1] == '1') return FALSE;
    i++;
  }
  return TRUE;
}

/* Merge new pass info into passstate. */
void updatepassstate(char *passstate, char *newpassinfo)
{
  int i;

  for (i = 0; i < 3 * scoringmethods[scoremethod].numplayers; i++) {
    if (strneq(notacard, passstate + i * 2, 2)) {
      strncpy(passstate + i * 2, newpassinfo + i * 2, 2);
    }
  }
}

fl_bool isnewinfo(char *passstate, char *newpassinfo)
{
  int i;

  for (i = 0; i < 3 * scoringmethods[scoremethod].numplayers; i++) {
    if (strneq(notacard, passstate + i * 2, 2) &&
	strncmp(notacard, newpassinfo + i * 2, 2)) {
      return TRUE;
    }
  }
  return FALSE;
}

#define HASNTPASSED 0
#define HASPASSED 1
/* should this be in UI.c?? */
void reportpassstatus(char *passstate)
{
  int i;
  char msg[BUFSIZ];
  static int lastupdate[4] = {-1,-1,-1,-1}; /* should be reset each hand(?) */

  for (i = 0; i < scoringmethods[scoremethod].numplayers; i++) {
    if (strneq(notacard, passstate + i * 6, 2)) {
      if (lastupdate[i] != HASNTPASSED) {
#if DEBUGHEARTS
	sprintf(msg, "Still waiting for %c to pass", seat_to_char(i));
	status(msg);
#endif
	lastupdate[i] = HASNTPASSED;
      }
    } else if (lastupdate[i] != HASPASSED) {
      if (!seated || myseat != i) {
	sprintf(msg, "%c has passed", seat_to_char(i));
	status(msg);
      }
      lastupdate[i] = HASPASSED;
    }			
  }
}

#if 0
int cmpcard(const void *a1, const void *a2)
{
  /* return c2 - c1, relies on them being different */
	
  char *c1 = (char *) a1;
  char *c2 = (char *) a2;
  int tmp;

  if ((tmp = c2[0] - c1[0])) return tmp;
  else {
    switch (c2[1]) {
    case 'a': return 1; break;
    case 'k': return (c1[1] == 'a' ? -1 : 1); break;
    case 'q': return (c1[1] == 'a' || c1[1] == 'k' ? -1 : 1); break;
    case 'j': 
      return (c1[1] == 'a' || c1[1] == 'k' || c1[1] == 'q' ? -1 : 1); 
      break;
    case 't':
      return (c1[1] >= '2' && c1[1] <= '9' ? 1 : -1);
      break;
    default:
      return (c1[1] >= '2' && c1[1] < c2[1] ? 1 : -1);
    }
  }
	
  return (- strncmp((char *) c1, (char *) c2, 2));
}
#endif

static char suits[] = "cdhs";
#define card_to_num(c) (13 * (charindex((c)[0], suits)) + char_to_rank((c)[1]))

/* BUG: modifies global state (initialcards, initialcards_beforepass), even
   though used from both br.c and UI.c. */
void adjusthands(char *passstate, int dealer, int *deck)
{
  int i, j, k;
  int np = scoringmethods[scoremethod].numplayers;
  int passi[4][3];   /* player, passcardnum to index into initial cards */
  int decksize = scoringmethods[scoremethod].numplayers * cardsperhand;

#if DEBUGHEARTS
  DEBUG(status5("adjusthands(", passstate, ", ", itoa(dealer), ")"));
#endif
  assert(passisover(passstate, dealer));

  if (notpassing(dealer)) return;

  for (i = 0; i < scoringmethods[scoremethod].numplayers; i++) 
    for (j = 0; j < 3; j++) {
      passi[i][j] = -1;
      for (k = 0; k < cardsperhand; k++) 
	if (card_to_num(passstate + j*2 + i*6) == 
	    deck[i * cardsperhand + k]) {
	  passi[i][j] = k;
	  break;
	}
    }

  for (i = 0; i < scoringmethods[scoremethod].numplayers; i++) {
    for (j = 0; j < 3; j++) {
      if (passi[i][j] != -1)
	deck[i * cardsperhand + passi[i][j]] =
	  card_to_num(passstate + ((i-dealer+np+2) % np)*6 + j*2);
    }
  }

  sortdeal(deck, decksize, cardsperhand);
  for (i = 0; i < np; i++) {
    decktohand(deck, i, initialcards[i]);
  }
  if (playinghearts) initialcards[West][0] = '\0';
}

void broadcast_curpassstate(void)
{
  broadcast(makemsg2(PASS_STATUS, curhandID, curpassstate));
}

/* This generates the PASS_STATUS message to other players announcing a pass.
   It is called only by heartspass(), below. Error checking code in here
   is leftover from when this routine was called by dohpass() with user
   input in c1, c2, and c3. */
static fl_bool executepass(char *c1, char *c2, char *c3)
{
  char s[BUFSIZ];

  if (!playinghearts) {
    errormsg("This isn't hearts; there's no passing.");
    return FALSE;
  }

  if (curpassstate == NULL) {
    errormsg("You cannot pass until the hand is dealt.");
    return FALSE;
  }

  if (passisover(curpassstate, curdealer)) {
    errormsg("Sorry, the pass is over.");
    return FALSE;
  }

  if (!c1[0] || ! c2[0] || !c3[0]){
    errormsg("You must pass 3 cards.");
    return FALSE;
  }
	
  /* check to make sure they hold the cards in question */
  if (!cardmember(c1, remainingcards(myseat)) &&
      !streq(c1, notacard)) {
    sprintf(s, "You can't pass a card (%s) that you don't have.", c1);
    errormsg(s);
    return FALSE;
  }
  if (!cardmember(c2, remainingcards(myseat)) &&
      !streq(c2, notacard)) {
    sprintf(s, "You can't pass a card (%s) that you don't have.", c2);
    errormsg(s);
    return FALSE;
  }
  if (!cardmember(c3, remainingcards(myseat)) &&
      !streq(c3, notacard)) {
    sprintf(s, "You can't pass a card (%s) that you don't have.", c3);
    errormsg(s);
    return FALSE;
  }

  if (streq(c1, c2) || streq(c1, c3) || streq(c2, c3)) {
    errormsg("You must pass 3 different cards.");
  }

  curpassstate[myseat * 6] = c1[0];
  curpassstate[myseat * 6 + 1] = c1[1];
  curpassstate[myseat * 6 + 2] = c2[0];
  curpassstate[myseat * 6 + 3] = c2[1];
  curpassstate[myseat * 6 + 4] = c3[0];
  curpassstate[myseat * 6 + 5] = c3[1];

  if (streq(c1, notacard) || streq(c2, notacard) || streq(c3, notacard)) {
    status("Pass cancelled");  /* this isn't an error of course */
  } else {
    sprintf(s, "You just passed %s, %s, and %s.", c1, c2, c3);
    status(s);
  }

  selfmessage(makemsg2(PASS_STATUS, curhandID, curpassstate));
  return TRUE;
}

#define iscompletepass(p) (strlen(p) == 6)

/* p is a single card to toggled in (removed from or added to) the
   "pass set."  When the pass set reaches the appropriate size,
   the set is passed. */
static void heartspass(char *p)
{
  assert(!passdone);
  UItogglepassedcard(p);
  if (cardmember(p, passset)) {
    strcpy(passset, cardsetdiff(passset, p));
  } else {
    strcat(passset, p);
    if (iscompletepass(passset)) {
      char c1[3], c2[3], c3[3];
      c1[0] = passset[0];
      c1[1] = passset[1];
      c1[2] = '\0';
      c2[0] = passset[2];
      c2[1] = passset[3];
      c2[2] = '\0';
      c3[0] = passset[4];
      c3[1] = passset[5];
      c3[2] = '\0';

      assert(executepass(c1, c2, c3));
      UIpassset("");
      return;
    }
  }
  UIpassset(passset);
}

/****************************************************************************/
void makerandombid(void)
{
  char *bids[] = {"pass", "x", "xx", "1c", "1d", "1h", "1s", "1n",
		    "2c", "2d", "2h", "2s", "2n",
		    "3c", "3d", "3h", "3s", "3n",
		    "pass", "x", "xx", "1c", "1d", "1h", "1s", "1n",
		    "pass", "x", "xx", "1c", "1d", "1h", "1s", "1n",
		    "pass", "x", "xx", "1c", "1d", "1h", "1s", "1n",
		    "pass", "x", "xx", "1c", "1d", "1h", "1s", "1n",
		    "2c", "2d", "2h", "2s", "2n",
		    "pass", "x", "xx", "1c", "1d", "1h", "1s", "1n",
		    "2c", "2d", "2h", "2s", "2n",
		    "pass", "x", "xx", "1c", "1d", "1h", "1s", "1n",
		    "2c", "2d", "2h", "2s", "2n",
		    "3c", "3d", "3h", "3s", "3n",
		    "pass", "pass", "pass", "pass", 
		    "pass", "pass", "pass", "pass", 
		    "pass", "pass", "pass", "pass", 
		    "pass", "pass", "pass", "pass", 
		    "pass", "pass", "pass", "pass", 
		    "pass", "pass", "pass", "pass", 
		    "pass", "pass", "pass", "pass", 
		    "pass", "pass", "pass", "pass", 
		    "pass", "pass", "pass", "pass", 
		    "pass", "pass", "pass", "pass", 
		    "x", "xx", "1c", "1d", "1h", "1s", "1n",
		    "2c", "2d", "2h", "2s", "2n",
		    "3c", "3d", "3h", "3s", "3n",
		    "4c", "4d", "4h", "4s", "4n",
		    "4c", "4d", "4h", "4s", "4n",
		    "4c", "4d", "4h", "4s", "4n",
		    "5c", "5d", "5h", "5s", "5n",
		    "6c", "6d", "6h", "6s", "6n",
		    "7c", "7d", "7h", "7s", "7n", "x", "x", "x", "x", "x"};

  if (strlen(curastate) > 8 && unifmod(10) > 5) { assert(executebid("p")); }
  else while (!executebid(bids[unifmod(sizeof(bids)/sizeof(bids[0]))]));
}

void makerandomplay(void)
{
  char *plays[] = {"sa", "ha", "da", "ca",
		     "s2", "h2", "d2", "c2",
		     "s3", "h3", "d3", "c3",
		     "s4", "h4", "d4", "c4",
		     "s5", "h5", "d5", "c5",
		     "s6", "h6", "d6", "c6",
		     "s7", "h7", "d7", "c7",
		     "s8", "h8", "d8", "c8",
		     "s9", "h9", "d9", "c9",
		     "st", "ht", "dt", "ct",
		     "sj", "hj", "dj", "cj",
		     "sq", "hq", "dq", "cq",
		     "sk", "hk", "dk", "ck"};

  if (handplayedout) return;
  while (!executeplay(plays[unifmod(sizeof(plays)/sizeof(plays[0]))]));
}

void playsmall(char suit)
{
  char cards[] = "23456789tjqka";
  char foo[3];
  int i;

  TclDo("gset showerrors 0");
  foo[0] = suit;
  foo[2] = '\0';
  for (i=0; cards[i] != '\0'; i++) {
    foo[1] = cards[i];
    if (executeplay(foo)) {
      TclDo("gset showerrors 1");
      return;
    }
  }
  foo[1] = '\0';
  TclDo("gset showerrors 1");
  errormsg3("Can't follow with a small `", foo, "'");
}

fl_bool playsmall_ifpossible(char suit)
{
  char cards[] = "23456789tjqka";
  char foo[3];
  int i;

  TclDo("gset showerrors 0");
  foo[0] = suit;
  foo[2] = '\0';
  for (i = 0; cards[i] != '\0'; i++) {
    foo[1] = cards[i];
    if (executeplay(foo)) {
      TclDo("gset showerrors 1");
      return TRUE;
    }
  }
  foo[1] = '\0';
  TclDo("gset showerrors 1");
  errormsg3("Can't follow with a small `", foo, "'");
  return FALSE;
}

/* assumes it is my turn to play */
void followsmall(void)
{
  int len = strlen(curpstate),
    pertrick = 2 * scoringmethods[scoremethod].numplayers;
  
  if ((len % pertrick) == 0) {
    /* It is my lead: error unless all my cards are in the same suit. */
    char *remcards = remainingcards(myseat);
    int numcards = numcards(remcards);

    if (remcards[0] == remcards[(numcards - 1) * 2]) playsmall(remcards[0]);
    else errormsg((playinghearts && !passisover(curpassstate, curdealer)) ?
		  "The pass isn't over" :
		  "It's your lead");
  } else {
    /* Not my lead: play small in suit led, or play my only card if
       I have just one left. */
    char *remcards = remainingcards(myseat);
    int numcards = numcards(remcards);
    char suit =
      (numcards == 1) ? remcards[0] : curpstate[(len / pertrick) * pertrick];
    playsmall(suit);
  }
}  

/* follow suit with the given card (i.e. card should be one of "a", "k", ...)*/
/* returns whether it succeeded */
fl_bool follow(char *card)
{
  int len = strlen(curpstate),
    pertrick = 2 * scoringmethods[scoremethod].numplayers;
  char s[10], thesuit, thecard;

  while (isspace(*card)) card++;
  thecard = tolower(*card);
  if ((len % pertrick) == 0) {
    /* it's my lead---error unless exactly one card of the given denom. */
    char *remcards;
    int numOK, sOK, hOK, dOK, cOK;

    if (curdeclarer == myseat) {
      /* check if it is dummy's play */
      int numcardsplayed = numcards(curpstate);
      int leader = computeleader(curpstate, numcardsplayed & ~3,
				 curtrump, curdeclarer, playinghearts);

      if (leader == (myseat ^ 2)) remcards = remainingcards(myseat ^ 2);
      else remcards = remainingcards(myseat);
    } else remcards = remainingcards(myseat);

#define SPRINTF(a, b, c) (sprintf(a, b, c), a)
    numOK = (sOK = cardmember(SPRINTF(s, "s%c", thecard), remcards) ? 1 : 0);
    numOK += (hOK = cardmember(SPRINTF(s, "h%c", thecard), remcards) ? 1 : 0);
    numOK += (dOK = cardmember(SPRINTF(s, "d%c", thecard), remcards) ? 1 : 0);
    numOK += (cOK = cardmember(SPRINTF(s, "c%c", thecard), remcards) ? 1 : 0);
#undef SPRINTF

    if (numOK == 0) {
      char e[100];

      sprintf(e, "You don't have any cards of that denomination (%c)",
	      thecard);
      errormsg(e);
      return FALSE;
    } else if (numOK > 1) {
      char e[100];

      sprintf(e, "You have %d cards of that denomination (%c)",
	      numOK, thecard);
      errormsg(e);
      return FALSE;
    } else thesuit = sOK ? 's' : (hOK ? 'h' : (dOK ? 'd' : 'c'));

  } else thesuit = curpstate[(len / pertrick) * pertrick];
    
  sprintf(s, "play %c%c", thesuit, thecard);
  docmd(s);

  return (strlen(curpstate) != len);
}

static message *handstatusmsg(void)
{
  assert(strexists(curhandID));
  return makemsg8(NEW_HAND, curhandID,
		  TclDo3("gset handname(", curhandID, ")"),
		  TclDo3("gset handseed(", curhandID, ")"),
		  TclDo3("gset handastate(", curhandID, ")"),
		  TclDo3("gset handpstate(", curhandID, ")"),
		  TclDo3("gset handgstate(", curhandID, ")"),
		  TclDo3("gset handscoring(", curhandID, ")"),
		  TclDo3("gset handpassstate(", curhandID, ")"));
}

void tellhandstatus(char *name, char *connstr)
{
  char *handname;

  if (curhandID == NULL) return;
  sendmessage(connstr, handstatusmsg());
  handname = TclDo3("gset handname(", curhandID, ")");

  if (!playinghearts) {
    if (claiming)
      sendmessage(connstr, makemsg2(CLAIM, curhandID, itoa(curclaimtricks)));

    if (duplicate){
        printf("tellhandstatus %s %s %s",name, handname, connstr);
      want_to_send(name, handname_to_setname(handname),
		   handname_to_handnum(handname));
    }
  }
}



/* transmit convention cards to newcomer */
void tellcc(char *connstr)
{
  sendmessage(connstr, makemsg2(CC, "EW", TclDo("ccstr EW")));
  sendmessage(connstr, makemsg2(CC, "NS", TclDo("ccstr NS")));
}

void tellseatstatus(char *connstr)
{
  if (northname != NULL) 
    sendmessage(connstr, makemsg4(SEATED, northname, "N",
				  playeraddr[North], playerport[North]));
  if (southname != NULL) 
    sendmessage(connstr, makemsg4(SEATED, southname, "S",
				  playeraddr[South], playerport[South]));
  if (eastname != NULL) 
    sendmessage(connstr, makemsg4(SEATED, eastname, "E",
				  playeraddr[East], playerport[East]));
  if (westname != NULL) 
    sendmessage(connstr, makemsg4(SEATED, westname, "W",
				  playeraddr[West], playerport[West]));
}

fl_bool unoccupied(char seat)
{
#if DBG
  if (hasWindows)
    fprintf(stderr, "Checking if %c is occupied\n", seat);
#endif
  switch (seat) {
  case 'N':
    return (northname == NULL);
  case 'S':
    return (southname == NULL);
  case 'E':
    return (eastname == NULL);
  case 'W':
    return (westname == NULL);
  case '0':
    return TRUE;
  default:
    assert(0);
  }
}

char *name_at_seat(char seat) 
{
  switch (seat) {
  case 'N':
    return northname;
  case 'S':
    return southname;
  case 'E':
    return eastname;
  case 'W':
    return westname;
  default:
    return NULL;
  }
}


/* used when we first join a table---clears out names from previous table */
void cleartable(void)
{
  extern fl_bool kibitzing1;
  extern int kibitzingseat;
  seated = FALSE;
  kibitzing1 = FALSE;
  kibitzingseat = Noseat;
  reset(savednorthname);
  reset(savedsouthname);
  reset(savedeastname);
  reset(savedwestname);
  playergone(North);
  playergone(South);
  playergone(East);
  playergone(West);
  resetseen();
}

/* someone has taken (or left, if seat is '0') a seat */
void sit(char *name, char seat, char *addr, char *port)
{
  fl_bool tablefull, oldtablefull;
  int seatno;
  char *whoissitting = NULL;
  extern fl_bool kibitzing1; /* from UI.c */
  int myoldseat = myseat; /* if myseat changes, may need to recompute stuff */
  extern char tablehostname[];

  oldtablefull = ((northname != NULL) && (southname != NULL) &&
		  (westname != NULL) && (eastname != NULL));
  if (streq(myname, name))
    if ((seated = (seat != '0'))) {
      kibitzing1 = FALSE; /* the best place to clear this, module notwithstanding */
      whoissitting = "You are sitting ";
#ifndef SERVER
      sprintf(location, "playing at %s's table", outputname(tablehostname));
#endif
    } else {
      status("You are now a kibitzer");
#ifndef SERVER
      sprintf(location, "kibitzing at %s's table", outputname(tablehostname));
#endif
    }
  else whoissitting = TEMPCAT(outputname(name), " is sitting ");

  if (northname != NULL && streq(name, northname)) playergone(North);
  else if (southname != NULL && streq(name, southname)) playergone(South);
  else if (eastname != NULL && streq(name, eastname)) playergone(East);
  else if (westname != NULL && streq(name, westname)) playergone(West);

  switch (seat) {
  case 'N':
    seatno = North;
    if (streq(name, myname)) rhoseat = rightyseat(myseat = North);
    status2(whoissitting, "North");
    if (northname != NULL) playergone(North);
    northname = STRDUP(name);
    break;
  case 'S':
    seatno = South;
    if (streq(name, myname)) rhoseat = rightyseat(myseat = South);
    status2(whoissitting, "South");
    if (southname != NULL) playergone(South);
    southname = STRDUP(name);
    break;
  case 'E':
    seatno = East;
    if (streq(name, myname)) rhoseat = rightyseat(myseat = East);
    status2(whoissitting, "East");
    if (eastname != NULL) playergone(East);
    eastname = STRDUP(name);
    break;
  case 'W':
    seatno = West;
    if (streq(name, myname)) rhoseat = rightyseat(myseat = West);
    status2(whoissitting, "West");
    if (westname != NULL) playergone(West);
    westname = STRDUP(name);
    break;
  case '0':
    /* someone getting up from the table */
    if (streq(myname, name)) {
      myseat = rhoseat = Noseat;
      seated = FALSE;
    } else status2(outputname(name), " is now a kibitzer");
    break;
  default:
    assert(0);
  }
  tablefull = ((northname != NULL) && (southname != NULL) &&
	       (westname != NULL) && (eastname != NULL));
  if (!oldtablefull && tablefull) fulltable();
  if (oldtablefull && !tablefull) notfulltable();
  if (seat != '0') {
    STRCPY(playerport[seatno], port);
    STRCPY(playeraddr[seatno], addr);
  }
  if (myseat != myoldseat)  {
    if (strexists(curhandID)) {
      message *m = handstatusmsg();
      reset(curhandID);
      filehandinfo(m);
    }
    if (!playinghearts && curastate != NULL && auctionisover(curastate))
      computecontract(curastate, curdealer,
		      curcontract, &curdeclarer, &curtrump);
  }
  UIsit(name, seat, myseat);
}  

/* There's no bridge going on; clear/reset all relevant variables. */
void nobridge(void)
{
  TclDo("reset_previous_trick");
  if (hasWindows) TclDo("menus_nobridge");
  initialcards[0][0] = initialcards[1][0] =
    initialcards[2][0] = initialcards[3][0] = '\0';
  strscore[0][0] = strscore[1][0] =
    strscore[2][0] = strscore[3][0] = '\0';
  heartsscore[North] = heartsscore[South] =
    heartsscore[West] = heartsscore[East] = 0;
  claiming = FALSE;
  reset(curhandID);
  reset(curhandname);
  cleartable();
  clearcc();
}

void playergone(int which)
{
  fl_bool tablefull = ((northname != NULL) && (southname != NULL) &&
		    (westname != NULL) && (eastname != NULL));

  switch (which) {
  case North:
    reset(northname);
    northname = NULL;
    break;
  case South:
    reset(southname);
    southname = NULL;
    break;
  case East:
    reset(eastname);
    eastname = NULL;
    break;
  case West:
    reset(westname);
    westname = NULL;
    break;
  default:
    assert(0);
  } 

  /* we no longer know what boards the person in that seat has seen,
     as there's no one there */
  parseseen(seat_to_char(which), -1, "?");
  if (claiming && (which == curdeclarer)) claiming = FALSE;

  if (tablefull) notfulltable();
  UIplayergone(which);
}  

/* somebody left the table, so check if it was one of the players */
void persongone(char *name)
{
  if (northname != NULL && streq(name, northname)) playergone(North);
  else if (southname != NULL && streq(name, southname)) playergone(South);
  else if (eastname != NULL && streq(name, eastname)) playergone(East);
  else if (westname != NULL && streq(name, westname)) playergone(West);
}

/* break cc at tabs; add each piece as a separate line
   (undoes what ccstr in floater.tcp does) */
static void ccupdate2(char *cc)
{
  char *z;

  if (!strexists(cc)) return;
  for (z = cc; *z != '\0'; z++) if (*z == '\t') {
    *z = '\0';
    TclDo3("addnewcc {", braceclean(cc), "}");
    cc = z + 1;
  }
  TclDo3("addnewcc {", braceclean(cc), "}");
}

void ccupdate(char *who, char *cc)
{
  char *t;

  TclDo2("beginnewcc ", who);
  ccupdate2(cc);
  TclDo("endnewcc");
  t = TclDo3("gset cclines(", who, ")");
  status5("Convention card loaded for ", who,
	  " (", maybe_plural(t, "line"), ").");
}

/***************************************************************************
Handling of who has seen what hands, etc.
***************************************************************************/

/* the following assumes NUMSEENS is 2 */
#define knowseen() atob(TclDo("global Nseen0 Sseen0 Eseen0 Wseen0 Nseen1 Sseen1 Eseen1 Wseen1; expr [info exists Nseen0] && [info exists Sseen0] && [info exists Eseen0] && [info exists Wseen0] && [info exists Nseen1] && [info exists Sseen1] && [info exists Eseen1] && [info exists Wseen1]"))

/* seen[i] uses 1 based indexing: seen[i][1] to seen[i][seenlen[i]] */
/* (seen, maxseen, and seenlen use 0 based indexing, however) */

/* seen[i][j] is whether anyone seated has seen hand j under scoring i */
/* (but beware: maxseen[i] may or may not equal seenlen[i]) */
static char *seen[NUMSEENS];

/* maxseen[i] is the highest hand num anyone seated has seen under scoring i */
static int maxseen[NUMSEENS];

/* seen[i] uses 1 based indexing: seen[i][1] to seen[i][seenlen[i]] */
static int seenlen[NUMSEENS];

/* forgets any info we had about who has seen what boards */
void resetseen(void)
{
  parseseen('N', -1, "?");
  parseseen('S', -1, "?");
  parseseen('E', -1, "?");
  parseseen('W', -1, "?");
}

/* Invoked when the tablehost does "deal," this function checks
   whether we have the information to compute the lowest numbered deal
   at the current form of scoring that hasn't been seen by anyone
   seated.  If so, go ahead and deal.  If not, give an error message
   and issue a ANNOUNCE_GLOBALDATE message to request from everyone
   seated a response indicating what they've seen.
*/
void generatedeal(void)
{
  fl_bool tablefull;
  int m = (newscoremethod >= 0) ? newscoremethod : scoremethod;

  if (DUPLICATE(m) && checknewweek()) {
    selfmessage(makemsg2(ANNOUNCE_GLOBALDATE, globaldate, globalseed));
    errormsg("It's a new week... please wait a moment and try dealing again.");
    return;
  }

  tablefull = ((northname != NULL) && (southname != NULL)
	       && (westname != NULL || scoringmethods[m].numplayers < 4)
	       && (eastname != NULL));

  /* if (competitive || tablefull) switch ... */

  switch (m) {
  case 0: /* IMPs */
  case 1: /* MPs */
    if (!knowseen()) {
      selfmessage(makemsg2(ANNOUNCE_GLOBALDATE, globaldate, globalseed));
      if (tablefull)
	status("Not sure who has seen what boards... please try again later");
      else {
	status("Please wait until 4 are seated");
      }
      return;
    }
    break;
  case 2: /* YUK3 */
  case 3: /* rubber */
    break;
  default:
    assert(0);
  }

  if (newscoremethod == scoremethod) newscoremethod = -1;
  if (newscoremethod >= 0) {
    scoremethod = newscoremethod;
    (scoringmethods[scoremethod].startup)();
    newscoremethod = -1;
    cardsperhand = 52 / scoringmethods[scoremethod].numplayers;
    playinghearts = (scoremethod == 2);
    /* (broadcast this message to the table?) */
    status2("Switching scoring method to ",
	    scoringmethods[scoremethod].scoringname);
  }  


  (*(scoringmethods[scoremethod].seed))();
}

/* what's the lowest numbered hand that no one has seen? */
/* argument is 0 for IMPs or 1 for matchpoints */
/* Note that if seen[i] contains all TRUE values, we assume the next
   hand we can play is ++maxseen[i] and don't bother increasing the
   size of the seen[i] array. */
static int minseen(int i)
{
  int answer;

  for (answer = 1; answer <= seenlen[i]; answer++) if (!seen[i][answer]) {
    seen[i][answer] = TRUE;
    return answer;
  }
  return ++(maxseen[i]);
}

/* make sure we don't play hand num from set i again */
static void setseen(int i, char *set, int num)
{
#if DEBUGSEEN
  DEBUG(status6("setseen: set=", set, "(", itoa(i), ") handnumber=", itoa(num)));
#endif
  /*  TclDo6("a_h_s {", myoutputname, "} ", set, " ", itoa(num));*/
  if (strexists(globaldate) && strneq(globaldate, set, strlen(globaldate))) {
    if (num <= seenlen[i]) seen[i][num] = TRUE;
    else if (num > maxseen[i]) maxseen[i] = num;
  }
}

/* Returns whether I have seen hand num from set i. */
fl_bool haveseen(int i, int num)
{
  if (num <= seenlen[i]) return seen[i][num];
  else return (num <= maxseen[i]);
}

static void parseTCLseen2(char *seenlist, int i)
{
  while (isdigit(*seenlist)) seen[i][parseint(&seenlist)] = TRUE;
}
    
/* take the Tcl vars Nseen, Sseen, Eseen and Wseen, and parse them */
static void parseTCLseen(void)
{
  int i, j, mn, ms, me, mw;
  char *n, *s, *e, *w;
  char t[10];

  for (i = 0; i < NUMSEENS; i++) { 
    sprintf(t, "%d", i);
    n = TclDo2("gset Nseen", t);
    s = TclDo2("gset Sseen", t);
    e = TclDo2("gset Eseen", t);
    w = TclDo2("gset Wseen", t);
#if DEBUGSEEN
    {
      char a[5000];
      
      sprintf(a, "parseTCLseen (%s) %s %s %s %s", t, n, s, e, w);
      status(a);
    }
#endif
    /* change hyphens to backslashes so we can use parseint(), etc. */
    for (j=0; n[j] != '\0'; j++) if (n[j] == '-') n[j] = '\\';
    for (j=0; s[j] != '\0'; j++) if (s[j] == '-') s[j] = '\\';
    for (j=0; e[j] != '\0'; j++) if (e[j] == '-') e[j] = '\\';
    for (j=0; w[j] != '\0'; j++) if (w[j] == '-') w[j] = '\\';

    mn = maxnumslash(n);
    ms = maxnumslash(s);
    me = maxnumslash(e);
    mw = maxnumslash(w);
    maxseen[i] = max4(mn, ms, me, mw);
    reset(seen[i]);
    seen[i] = zsalloc(sizeof(char) * (1 + maxseen[i]));
    seenlen[i] = maxseen[i];
    parseTCLseen2(n, i);
    parseTCLseen2(s, i);
    parseTCLseen2(e, i);
    parseTCLseen2(w, i);
#if DEBUGSEEN
    {
      char a[5000];
      
      sprintf(a, "result of parseTCLseen (%s): max=%d", t, maxseen[i]);
      for (j = 1; j <= maxseen[i]; j++)
	if (seen[i][j]) sprintf(a + strlen(a), "; %d", j);
      status(a);
    }
#endif
  }
}


/* parseseen() makes note of who has seen what boards */
/* seat is 'N', 'S', 'E', or 'W'; seen is a list of numerals; */
/*  index is 0 for IMPs or 1 for matchpoints */
/* If seen is "?" that means we don't know what the player in seat has seen */
/* special case: index is negative and seen is "?" means forget everything
   about that seat */
void parseseen(char seat, int index, char *seen)
{
  char *s;

#if DEBUGSEEN
  if (index >= 0) {
    char t[30];

    sprintf(t, "parseseen %c %d ", seat, index);
    status2(t, seen);
  }
#endif

  /* handle special case */
  if (index < 0) {
    int i;

    assert(seen[0] == '?');
    for (i = 0; i < NUMSEENS; i++) parseseen(seat, i, seen);
    return;
  }

  /* general case */
  s = salloc((strlen(seen) + 50) * sizeof(char));
  if (seen[0] == '?') {
    sprintf(s, "catch {gunset %cseen%d}", seat, index);
    maxseen[index] = 0;
    seenlen[index] = 0;
  }
  else
    sprintf(s, "gset %cseen%d {%s}", seat, index, seen);
  TclDo(s);
  free(s);
  if (knowseen()) {
    parseTCLseen();
    if (DUPLICATE(scoremethod))
      status("You may use the deal command to start a new hand, if you wish");
  }
}

/* e.g., "26Jan97IMP" => 0, because scoringmethod 0 is IMPs. */
static int scoring_of_setname(char *setname)
{
  int i;
  char *target;

  for (i = 0; i < numscoringmethods; i++) {
    target = scoringmethods[i].scoringname;
    if (streq(setname - strlen(target) + strlen(setname), target)) return i;
  }
  return -1;
}

void parse_you_have_seen(char *set, char *msg)
{
  char *bits, *scorings;
  int n, scoring;

#if DEBUGSEEN
  DEBUG(status2("parsing YOU_HAVE_SEEN for ", set));
#endif
  scoring = scoring_of_setname(set);
  scorings = itoa(scoring);
  n = strlen(msg) * 6;
  bits = markgarbage(zsalloc(n + 2));
  get6atatime(msg, n, bits);
  while (n > 0) {
    if (bits[n]) seenhand(TEMPCAT(set, itoa(n)), scorings);
    n--;
  }
}

#ifdef LOGINSERVER
static char *construct_you_have_seen(char *set, char *name)
{
  char *r, *t, *s;
  int i, n;

  assert(strexists(set));
  s = TclDo4("query_have_seen {", name, "} ", set);

  status5(name, " has seen ", (strexists(s) ? s : "nothing"), " from ", set);

  if (!strexists(s)) return NULL;
  
  /* parseint() expects a backslash after each integer */
  for (n = 0; s[n] != '\0'; n++) if (s[n] == ' ') s[n] = '\\';
  s = TEMPCAT(s, "\\");

  i = maxnumslash(s) + 1;
  /* bump up to next common multiple of 6 and 8 to make extra sure
     that put6atatime() and get6atatime() will work with no trouble */
  while ((i % 6) != 0 || (i % 8) != 0) i++;
  t = zsalloc(i);
  while (isdigit(*s)) t[parseint(&s)] = TRUE;
  r = markgarbage(zsalloc(1 + i * 6 / 8));
  put6atatime(t, i, r);
  free(t);
  return r;
}

void announce_seen(char *connstr, char *name)
{
  char *s, *set, *m;

  name = outputname(name); /* should be unnecessary, but can't hurt */

  status2("announce_seen for ", name);

  checknewweek();
  s = TclDo3("have_seen_sets {", name, "}");
  talkmsg5("have_seen_sets for ", name, ": `", s, "'");
  while (strexists(s)) {
    set = nextword(&s);
    if ((m = construct_you_have_seen(set, name)) != NULL) {
      sendmessage(connstr, makemsg2(YOU_HAVE_SEEN, set, m));
      status4("Sent YOU_HAVE_SEEN for ", set, " to ", name);
    }
  }
}
#endif

/***************************************************************************/

void computetrickswon(char *pstate, int tricks,
		      int trump, int declarer, int *decl, int *defense)
{
  int i, numtricks, whowon,
    pertrick = 2 * scoringmethods[scoremethod].numplayers;

  if (isin('!', pstate)) {
    char c;
    /* hexadecimal digit after ! tells us how many tricks declarer claimed */
    while (*pstate++ != '!');
    c = toupper(*pstate);
    if (c >= 'A' && c <= 'F') *decl = c - 'A' + 10;
    else *decl = c - '0';
    *defense = cardsperhand - *decl;
    return;
  }

  numtricks = min(strlen(pstate) / pertrick, tricks);
  for (i = *decl = *defense = 0; i < numtricks; i++) {
    whowon = computeleader(pstate, (i + 1) * scoringmethods[scoremethod].numplayers, trump, declarer, playinghearts);
    if ((whowon % 2) == (declarer % 2)) ++*decl; else ++*defense;
  }
}

void handleclaimmessage(char kind, char *handID, int tricks, char *from)
{
  switch (kind) {
  case RETRACT_CLAIM:
    {
      char s[100];
      sprintf(s, "Claim for %d tricks has been retracted", tricks);
      status(s);
    }
    if (claiming && curclaimtricks == tricks && streq(handID, curhandID)) {
      reset(claimaccept);
      claiming = FALSE;
      if (hasWindows && (defending() || declaring())) TclDo("menus_noclaim");
    }
    return;
  case REJECT_CLAIM:
    {
      char s[100];
      sprintf(s, "Claim for %d tricks has been rejected", tricks);
      status(s);
    }
    if (claiming && curclaimtricks == tricks && streq(handID, curhandID)) {
      reset(claimaccept);
      claiming = FALSE;
      if (hasWindows && (defending() || declaring())) TclDo("menus_noclaim");
    }
    return;
  case ACCEPT_CLAIM:
    if (claiming && declaring() && curclaimtricks == tricks &&
	streq(handID, curhandID))
      if (claimaccept == NULL) claimaccept = STRDUP(from);
      else if (!streq(claimaccept, from) &&
	       !playisover(curastate, curpstate)) {
	char new[150];

	sprintf(new, "%s!%1x", curpstate, tricks);
	selfmessage(makemsg2(PLAY_STATUS, curhandID, new));
	reset(claimaccept);
      }
    return;
  case CLAIM:
    {
      char s[100];
      if (defending()) {
	sprintf(s, "Declarer is claiming %d tricks total", tricks);
	status(s);
	status("Continue playing or use `accept' or `reject' at any time.");
      } else if (declaring()) {
	sprintf(s, "You are claiming %d tricks total", tricks);
	status(s);
	status("You may use `retract' to retract the claim.");
      } else {
	sprintf(s, "Declarer is claiming %d tricks total", tricks);
	status(s);
      }	
    }
    reset(claimaccept);
    claiming = TRUE;
    curclaimtricks = tricks;
    if (hasWindows)
      if (defending()) TclDo("menus_defclaim");
      else if (declaring()) TclDo("menus_declclaim");
    return;
  default:
    assert(0);
  }
}
    
void retractclaim(void)
{
  if (claiming && declaring())
    selfmessage(makemsg2(RETRACT_CLAIM, curhandID, itoa(curclaimtricks)));
}

void rejectclaim(void)
{
  if (!claiming) {
    errormsg("Declarer is not claiming; there's no claim to reject");
    return;
  }

  if (!seated || !defending()) {
    if (curdeclarer == myseat)
      errormsg("Declarer cannot reject the claim.  You may retract it using `retract.'");
    else
      errormsg("You must be one of the defenders to reject the claim.");
    return;
  }
  selfmessage(makemsg2(REJECT_CLAIM, curhandID, itoa(curclaimtricks)));
}

void acceptclaim(void)
{
  char *s;

  if (!claiming) {
    errormsg("Declarer is not claiming; there's no claim to accept");
    return;
  }

  if (!seated || !defending()) {
    errormsg("You must be one of the defenders to accept the claim");
    return;
  }

  s = itoa(curclaimtricks);
  status3("You have accepted declarer's claim for ", s, " tricks.");
  selfmessage(makemsg2(ACCEPT_CLAIM, curhandID, s));
}

static void handleclaim(int tricksclaimed)
{

  int decltricks, deftricks;

  computetrickswon(curpstate, 1000, char_to_strain(curcontract[1]),
		   curdeclarer, &decltricks, &deftricks);

  if (tricksclaimed < decltricks) {
    char t[100];
    sprintf(t, "You can't claim only %d tricks when you've already won %d",
	    tricksclaimed, decltricks);
    errormsg(t);
    return;
  }
  if (tricksclaimed > 13 - deftricks) {
    char t[100];
    if (tricksclaimed > 13)
      sprintf(t, "You can't claim %d tricks!", tricksclaimed);
    else 
      sprintf(t, "You can't claim %d tricks when you've already lost %d",
	      tricksclaimed, deftricks);
    errormsg(t);
    return;
  }
  if (claiming) retractclaim();
  selfmessage(makemsg2(CLAIM, curhandID, itoa(tricksclaimed)));
}

void down(char *s)
{
  int tricksclaimed, tricksneeded;
  int decltricks, deftricks;

  if (!seated || (curdeclarer != myseat)) {
    errormsg("You must be declarer to claim");
    return;
  }

  if (!auctionisover(curastate) || playisover(curastate, curpstate)) {
    errormsg("You can't claim unless the play is in progress");
    return;
  }
      
  computetrickswon(curpstate, 1000, char_to_strain(curcontract[1]),
		   curdeclarer, &decltricks, &deftricks);

  tricksneeded = (curcontract[0] - '1' + 7);
  {
    char *t;
    int a;
    
    t = s;
    while (isspace(*t)) t++;
    while (isdigit(*t)) t++;
    while (isspace(*t)) t++;
    if (*t != '\0') {    
      errormsg("Expecting a positive integer as argument to `down'");
      return;
    }
    a = atoi(s);
    if (a > 0) tricksclaimed = tricksneeded - a;
    else {
      errormsg("Expecting a positive integer as argument to `down'");
      return;
    }
  }
  if (tricksclaimed < decltricks) {
    char t[100];
    sprintf(t, "You can't claim only %d tricks when you've already won %d",
	    tricksclaimed, decltricks);
    errormsg(t);
    return;
  }
  handleclaim(tricksclaimed);
}
  

void claim(char *s)
{
  int tricksclaimed, tricksneeded;
  int decltricks, deftricks;

  if (!seated || (curdeclarer != myseat)) {
    errormsg("You must be declarer to claim");
    return;
  }

  if (!auctionisover(curastate) || playisover(curastate, curpstate)) {
    errormsg("You can't claim unless the play is in progress");
    return;
  }
      
  computetrickswon(curpstate, 1000, char_to_strain(curcontract[1]),
		   curdeclarer, &decltricks, &deftricks);

  tricksneeded = (curcontract[0] - '1' + 7);
  while (isspace(*s)) s++;
  if (strlen(s) == 0) tricksclaimed = 13 - deftricks; /* claim the rest */
  else {
    char *t;
    int a;
    
    while (isspace(*s)) s++;
    t = s;
    if (*t == '-' || *t == '+') t++;
    while (isdigit(*t)) t++;
    while (isspace(*t)) t++;
    if (*t != '\0') {    
      errormsg("`claim': expecting a numeral as argument (if any)");
      return;
    }
    if (*s == '+') tricksclaimed = atoi(s+1) + tricksneeded;
    else {
      a = atoi(s);
      if (a > 0) tricksclaimed = decltricks + a;
      else if (a < 0) {
	tricksclaimed = 13 - deftricks + a;
      } else if (strlen(curpstate) < 104) {
	/* "claim 0" means conceding the rest---don't ask defenders for OK */
	char new[1000];

	sprintf(new, "%s!%1x", curpstate, decltricks);
	selfmessage(makemsg2(PLAY_STATUS, curhandID, new));
	return;
      }
    }
  }
  
  handleclaim(tricksclaimed);
}

void make(char *s)
{
  int tricksclaimed, tricksneeded;
  int decltricks, deftricks;

  if (!seated || (curdeclarer != myseat)) {
    errormsg("You must be declarer to claim");
    return;
  }

  if (!auctionisover(curastate) || playisover(curastate, curpstate)) {
    errormsg("You can't claim unless the play is in progress");
    return;
  }
      
  computetrickswon(curpstate, 1000, char_to_strain(curcontract[1]),
		   curdeclarer, &decltricks, &deftricks);

  tricksneeded = (curcontract[0] - '1' + 7);
  while (isspace(*s)) s++;
  if (strlen(s) == 0) tricksclaimed = tricksneeded;
  else {
    char *t;
    int a;
    
    t = s;
    while (isspace(*t)) t++;
    if (*t == '-') t++;
    while (isdigit(*t)) t++;
    while (isspace(*t)) t++;
    if (*t != '\0') {    
      errormsg("`make' expects a numeral as argument (if any)");
      return;
    }
    a = atoi(s);
    if (a > 0) tricksclaimed = a + 6;
    else {
      errormsg("`make' expects a positive integer as argument (if any)");
      return;
    }
  }
  
  if (tricksclaimed < decltricks) {
    char t[100];
    sprintf(t, "You can't claim only %d tricks when you've already won %d",
	    tricksclaimed, decltricks);
    errormsg(t);
    return;
  }
  if (tricksclaimed < tricksneeded) {
    char t[100];
    sprintf(t, "You can't `make %d' when you're in %d; use `down' instead",
	    tricksclaimed - 6, tricksneeded - 6);
    errormsg(t);
    return;
  }
  handleclaim(tricksclaimed);
}


char *bridge_statusbar(char *s, int start, int end, fl_bool *center)
{
  char *t;

  *s = '\0';
  *center = FALSE;

  if (strexists(curhandname) && !auctionisover(curastate)) {
    int max = end - start - 4; /* - 3 might be OK, but this is safe for sure */

    t = statusauction();
    if (strlen(t) == 0) {
      *center = TRUE;
      if (seated)
	sprintf(s, "[ %s, %c dealt | %s ]", vulname(),
		seattochar[curdealer],
		prettyprintcardset(initialcards[myseat]));
      else
	sprintf(s, "[ %s, %c dealt ]", vulname(),
		seattochar[curdealer]);
    }
    else if (strlen(t) > max) {
      int second_dot = strlen(t) - max + 1;

      while (t[second_dot] != ' ') second_dot++;
      sprintf(s, "[ ..%s ]", t + second_dot + 1);
    } else {
      sprintf(s, "[ %s ]", t);
    }
  }
  return s;
}

/* Insert the default action as if typed by the user by calling the
supplied function.  Returns whether there exists a default action now. */
fl_bool insert_default_action(fl_bool needSlash, insertcharfn f)
{
  char *s;

  if (!seated) return FALSE;
  if (silentmyturntobid()) s = "/p";
  else if (silentmyturntoplay()) s = "/.";
  else return FALSE;
  
  (*f)(needSlash ? s : s + 1);
  return TRUE;
}
