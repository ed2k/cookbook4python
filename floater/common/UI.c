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

/* UI.c */



#include "floater.h"
#include "tickler.h"
#include "comm.h"
#include "deal.h"
#include "br.h"
#include "UI.h"
#include "UIback.h"

fl_bool myturntoact = FALSE;

char *vulnames[4] = {"EW vul.", "None vul.", "Both vul.", "NS vul."};
char *passnames[3] = {"Pass Left", "Pass Right", "No Pass"};

extern int state;
extern fl_bool tablehostmode;
extern char *northname, *southname, *eastname, *westname;
extern char *curhandID;

static void startshowtricktimer()
{
  TclDo("startshowtricktimer");
}

static int needAuctionUpdate()
{
  return (atob(TclGet("needAuctionUpdate")) ?
	  (TclSet("needAuctionUpdate", "0"), TRUE) : FALSE);
  
}


static void UIclearmatrix() {
  UIbackend->clearmatrix(FALSE);
}

static void UIdelayedclearmatix() {
  UIbackend->clearmatrix(TRUE);
}

static void UIredrawmatrixcards() {
  UIbackend->redrawmatrixcards();
}

static void force_update() {
  UIbackend->force_update();
}

static void startMyTurnTimer() {
  UIbackend->startMyTurnTimer();
}

static void stopMyTurnTimer() {
  UIbackend->stopMyTurnTimer();
}

static void removecardfromhand(const char *play) {
  UIbackend->removecardfromhand(play);
}

static void togglepassedcard(const char *play) {
  UIbackend->togglepassedcard(play);
}

/* clear the frames/labels showing the auction */
static void newauction() {
  UIbackend->newauction();
}

/* change the displayed cards for each hanldeal */
static void fulldeal(const char *selfs,
		     const char *selfh,
		     const char *selfd,
		     const char *selfc,
		     
		     const char *lhos,
		     const char *lhoh,
		     const char *lhod,
		     const char *lhoc,
			
		     const char *ps,
		     const char *ph,
		     const char *pd,
		     const char *pc,
			
		     const char *rhos,
		     const char *rhoh,
		     const char *rhod,
		     const char *rhoc)
{
  UIbackend->fulldeal(selfs, selfh, selfd, selfc,
		      lhos, lhoh, lhod, lhoc,
		      ps, ph, pd, pc,
		      rhos, rhoh, rhod, rhoc);
}

/* handtosuits - extract the four suits from a hand.
   The outputs s, h, d, and c should point to sufficient
   memory to hold at least 13 rank designations plus
   a terminating '\0' (ie, at least 14 bytes)
*/
static void handtosuits(const char *hand,
			char *s, char *h, char *d, char *c)
{
  char *suits[4];
  const char *cp;
  int suitno;
  
  suits[0] = s;
  suits[1] = h;
  suits[2] = d;
  suits[3] = c;
  

  cp = hand;
  for(suitno = 0; suitno < 4; suitno++) {
    char *sp;
    sp = suits[suitno];
    
    
    /* scan for beginning of suit */
    while(*cp != '{')  cp++;
    cp++;

    /* extract the suit */
    while(*cp != '}') {
      *sp = *cp;
      sp++;
      cp++;
    }
    *sp = '\0';

    /* skip the '}' after the suit */
    cp++;
  }
}

			
/* same as fulldeal, but with 4 arguments instead of 16 */
static void fulldeal4(const char *self,
		      const char *lho,
		      const char *p,
		      const char *rho)
{
  char selfs[100], selfh[100], selfd[100], selfc[100];
  char lhos[100], lhoh[100], lhod[100], lhoc[100];
  char ps[100], ph[100], pd[100], pc[100];
  char rhos[100], rhoh[100], rhod[100], rhoc[100];

  handtosuits(self, selfs, selfh, selfd, selfc);
  handtosuits(lho, lhos, lhoh, lhod, lhoc);
  handtosuits(p, ps, ph, pd, pc);
  handtosuits(rho, rhos, rhoh, rhod, rhoc);


  fulldeal(selfs, selfh, selfd, selfc,
	   lhos, lhoh, lhod, lhoc,
	   ps, ph, pd, pc,
	   rhos, rhoh, rhod, rhoc);
}


static void nodeal() 
{
  fulldeal("", "", "", "",
	   "", "", "", "",
	   "", "", "", "",
	   "", "", "", "");
}



static void UIauction(fl_bool display,
		      fl_bool showbuttonbar,
		      fl_bool hideMatrix);

static void noauction() 
{
  UIauction(FALSE, FALSE, FALSE);
}



/* Hide the auction after this many cards have been played. */
#define HIDE_AUCTION 4


/* If negative, hide auction after HIDE_AUCTION cards; otherwise hide
   auction this many seconds after the final pass. */
float auction_hide_time; /* initial value taken from Tcl var of same name */

static fl_bool auction_hide_time_inited = FALSE;
#define init_auction_hide_time()				\
    ((auction_hide_time = atof(TclGet("auction_hide_time"))),	\
     (auction_hide_time_inited = TRUE))



void UIwrongpassword(void)
{
  status("Wrong password");
}

void UInameinuse(void)
{
  status("That name is not available; you are not logged in; please try again");
}

void UIinsertTalk(const char *s)
{
  UIbackend->insertTalk(s);
}

void UIinsertCmd(const char *s)
{
  UIbackend->insertCmd(s);
}


/*****************************************************************************
Drawing the auction, the hands, and on and on
*****************************************************************************/

/* a `column', e.g. declarercolumn, is an integer from 0 to 3:
   3 means me, 0 means lho, 1 means partner, 2 means rho */

/* a `seat' is an integer meaning North, South, East, or West
   (see deal.h -- North is 0, East is 1, ... ) */

fl_bool spec = FALSE;
static int disnumplayers;
semistatic fl_bool kibitzing1 = FALSE; /* are we kibitzing one person only? */
semistatic int kibitzingseat = Noseat; /* the seat we are standing behind;
					* if kibitzing1 is TRUE, then we
					* see their cards */
semistatic char displayhandID[100] = {'\0'}; /* the hand we're watching */
semistatic char displayhandname[100] = {'\0'}; /* the hand we're watching */
semistatic char statushandvul[100] = {'\0'}; /* the vulnerability */
semistatic char statusscore[100] = {'\0'}; /* the state of the score */
semistatic char statushanddlr[100] = {'\0'}; /* the dealer */

semistatic char displayhandastate[1000]; /* state of the auction */
semistatic char displayhandpstate[200]; /* state of the play */
semistatic char displayhandpassstate[100]; /* state of the pass at hearts */
semistatic int displaypov = -1; /* point of view---what seat is drawn at the bottom */

semistatic fl_bool displaydoubledummy = FALSE;
semistatic int displayhandscoring;

/* the hand we're displaying---if not NULL at least one hand is being shown */
semistatic int *displaydeck = NULL;

static fl_bool displayinghearts = FALSE;
static fl_bool displayingrubber = FALSE;
semistatic int dealercolumn = -1; /* in the display of the auction */
semistatic int displaycalls;
semistatic int displayplays;
semistatic fl_bool displayseated = FALSE;
semistatic fl_bool displayshowbuttonbar = FALSE;
semistatic char statustolead[100] = {'\0'};
semistatic char statuscontract[100] = {'\0'};
semistatic char statusresult[100] = {'\0'};
semistatic char statusclaim[100] = {'\0'};
semistatic char displaycontract[100] = {'\0'};
semistatic char displaytrickswon[100] = {'\0'};
semistatic int leadercolumn = -1; /* who led this trick (by column) */
semistatic int declarercolumn = -1; /* who is declarer (by column) */
semistatic int displaytrump = -1;  /* which suit is trump */
semistatic fl_bool displayhandisover = FALSE;

/* rubber bridge stuff */
static fl_bool disnsvul, disewvul;
static int disnstotal, disewtotal, disnsbelowline, disewbelowline;

/* hearts stuff */
static int nscore, escore, sscore;

/* are we keeping up to date with what's going on at the table? */
fl_bool caughtup = TRUE;

#define southcards(b) ((b) ? handto4str(displaydeck, South) : "{} {} {} {} ")
#define northcards(b) ((b) ? handto4str(displaydeck, North) : "{} {} {} {} ")
#define eastcards(b) ((b) ? handto4str(displaydeck, East) : "{} {} {} {} ")
#define westcards(b) ((b) ? (disnumplayers == 3 ? "{} {} {} {2} " : \
			     handto4str(displaydeck, West)) : "{} {} {} {} ")


void setinfoline(const char *s)
{
  UIbackend->setinfoline(s);
}

void setstatusline(const char *s)
{
  UIbackend->setstatusline(s);
}



/* attempt to kibitz one player -- Syela 040223 */
void UIkib1(int kibseat)
{
  char *astate, *pstate; /* lurking in the ether until TCL summons them out */

  
  if (seated) {
    errormsg("Players can't kibitz other players");
    return;
  }
  if (state == LIMBO && !tablehostmode) { /* paranoid doublecheck */
    errormsg("You can't kibitz a seat when you aren't at a table");
    return;
  }

  if(strlen(displayhandID) > 0) {
       /* retrieve the values from TCL, just in case the globals are
	* out of date.  It is unclear when, exactly, the globals
	* get updated */
       astate = TclDo3("gset handastate(", displayhandID, ")");
  
       pstate = TclDo3("gset handpstate(", displayhandID, ")");
  }
  else {
       /* no hand is being played */
       astate = pstate = "";
  }
  
 
  if (kibseat == Noseat) {
    if (kibitzing1)
      status2("You are no longer kibitzing ", seattostring[kibitzingseat]);
    if (curhandID == NULL || playisover(astate,pstate)) kibitzingseat = kibseat; 
    /* else lock them to stop hopping */
    kibitzing1 = FALSE;
    displaypov = -1;
    UIsit(myname, '0', Noseat); /* need black magic to set everything that needs set */
  } else if (kibitzing1 && kibseat == kibitzingseat) {
    status2("You are already kibitzing ", seattostring[kibitzingseat]);
    
  } else if (kibitzingseat != Noseat && curhandID != NULL &&
             kibitzingseat != kibseat && !playisover(astate,pstate)) {
    errormsg("You can't kibitz another seat this hand (try spec)");
  } else {
    status2("You are now kibitzing ", seattostring[kibseat]);
    kibitzingseat = kibseat;
    kibitzing1 = TRUE;
    displaypov = -2; /* otherwise kibbing south didn't always show south cards */
    UIsit(myname, '0', Noseat); /* there oughtta be a more elegant way -- Syela */
  }
}

/* enter "spectator" mode */
void UIspec(void)
{
  if (seated && declarercolumn != 1) {
    errormsg("Players can't enter spectator mode (except dummy)");
#if DBGu
    errormsg2("declarercolumn = ", itoa(declarercolumn));
#endif
    return;
  }
  spec = TRUE;
}

/* what's on the status line? */
static char *statusline(void)
{
  return (char *) TclGet("statusline");
}

void UItablehost(void)
{
  extern int newscoremethod; /* from br.c */

  UIbackend->hoststatus(tablehostmode,
			newscoremethod,
			new_competitive);
}

void UIcommstate(int state)
{
  UIbackend->commstate(state);
  if (tablehostmode)
    UItablehost();
}

/****************************************************************************
UItogglepasscard(), UIpassset(), and UIheartshandover() are for hearts.
****************************************************************************/

/* Toggle screen highlight that indicates whether a card is in the pass set. */
void UItogglepassedcard(char *p)
{
  char t[4];
  t[0] = p[0];
  t[1] = ' ';
  t[2] = p[1];
  t[3] = '\0';
  togglepassedcard(t);
}

/* Update the passset on the status line. */
void UIpassset(char *t)
{
  char *s = upcase(t);

  switch (strlen(s)) {
  case 6:
    sprintf(statustolead, "Pass set: %c%c %c%c %c%c",
	    s[0], s[1], s[2], s[3], s[4], s[5]);
    break;
  case 4:
    sprintf(statustolead, "Pass set: %c%c %c%c",
	    s[0], s[1], s[2], s[3]);
    break;
  case 2:
    sprintf(statustolead, "Pass set: %c%c",
	    s[0], s[1]);
    break;
  case 0:
    sprintf(statustolead, "");
    break;
  default:
    assert(0);
  }
}

/* Remove the pass direction for the status line and update the scores. */
void UIheartshandover(int ns, int es, int ss)
{
  strcpy(statushandvul, "");
  sprintf(statusscore, "N: %d; E: %d; S: %d",
	  nscore = ns, escore = es, sscore = ss);
}
/****************************************************************************/

/* update the UI's status readouts to reflect the current
 * network status, hand status, etc. */
void UIupdatestatus(void)
{
  extern char *bridgestatus; /* from br.c */
  char connstat[200] = { '\0' };
  struct ui_status status = {
    displayinghearts,
    connstat,
    (const char *) STRDUP(displayhandname),
    (const char *) STRDUP(statushandvul),
    (const char *) STRDUP(statushanddlr),
    (const char *) STRDUP(statuscontract),
    (const char *) STRDUP(statustolead),
    (const char *) STRDUP(displaytrickswon),
    (const char *) STRDUP(statusclaim),
    (const char *) STRDUP(statusresult),
    (const char *) STRDUP(statusscore)
  };
  
  
  switch (state) {
  case LIMBO:
    strcpy(connstat, tablehostmode ? "Hosting" : "Not at a table");
    break;
  case DISCONNECTING:
    strcpy(connstat, "Disconnecting...");
    break;
  case CONNECTED:
    if (streq(statusline(), "") || bridgestatus == NULL ||
	strlen(bridgestatus) == 0) {
#ifdef LOGINSERVER
      strcpy(connstat, "Login server");
#endif
#ifdef RESULTSERVER
      strcpy(connstat, "Result server");
#endif
#ifdef PSEUDOMAILER
      strcpy(connstat, "Pseudomailer");
#endif
#ifndef SERVER
      strcpy(connstat, tablehostmode ? "Hosting" : "Connected");
#endif
    }
  }

#if !SILENT
  UIbackend->updatestatus(&status);
#endif /* !SILENT */
}

void snafflesuit(int *hand, int suit, char *dest)
{
  int i;

  i = 0;
  while (i < cardsperhand && whatsuit(hand[i]) > suit) i++;
  while (i < cardsperhand && whatsuit(hand[i]) == suit) {
/*
   extern fl_bool cardmember(char *card, char *cards);
   char card[3];
    sprintf(card, "%c%c", tolower(suit_to_char(suit)),
	    tolower(card_to_char[whatcard(hand[i])])); 
    if (!cardmember(card, displayhandpstate)) 
*/
    *dest++ = card_to_char[whatcard(hand[i])];
    i++;
  }
  *dest = '\0';
}


void UIpatientcursor() 
{
  UIbackend->patientcursor();
}

void UInormalcursor() 
{
  UIbackend->normalcursor();
}



/* Updates (global) displayhandpassstate and returns whether redisplay
   is necessary (i.e., whether the hands have been changed by the pass). */
static fl_bool UIupdatepass(char *passstate, int pov)
{
  fl_bool change = !streq(passstate, displayhandpassstate);

  if (displaydeck == NULL || !change) return FALSE;

#if DEBUGHEARTS
  DEBUG(status5("UIupdatepass ", passstate, " (change=", itoa(change), ")"));
#endif

  if (change) STRCPY(displayhandpassstate, passstate);
	
  if (passisover(displayhandpassstate, hcol_to_seat(dealercolumn, pov))) {
    adjusthands(passstate, hcol_to_seat(dealercolumn, pov), displaydeck);
    return TRUE;
  }

  return FALSE;

  /*
  updatepassstate(displayhandpassstate, passstate);
  reportpassstatus(displayhandpassstate);
  */
}

/* dd is whether to show all four hands */
/* pass is the hearts pass, if applicable */
static void UIshowdeal(int pov, fl_bool dd, int dummycol,
		       char *pass)
{
  fl_bool ed, wd, sd, nd;
  char *played;   /* cards in played are not shown */

  

  if(displayhandisover)
       played = NULL;
  else
       played = displayhandpstate;

  

  UIpatientcursor();
  if (displayinghearts || seated || kibitzing1 || dd || dummycol >= 0)
    if (displaydeck == NULL && strlen(displayhandID) > 0) {
      displaydeck = (int *) salloc(sizeof(int) * 52);
#if DBGu
      status2("Display handID=", displayhandID);
      status2("Display seed=", TclDo3("gset handseed(", displayhandID, ")"));
#endif
      //setseed(TclDo3("gset handseed(", displayhandID, ")"));
      char *seed = TclDo3("gset handseed(", displayhandID, ")");
      printf("uishowdeal %s\n",seed);
      //deal(displaydeck, 0, 0, 0, 0, disnumplayers * cardsperhand);
      if (displayinghearts) UIupdatepass(pass, pov);      
      //sortdeal(displaydeck, disnumplayers * cardsperhand, cardsperhand);
      setup_bridge_deck(seed, displaydeck);
    }

  nd = sd = ed = wd = dd;
  if (!dd && (dummycol >= 0) && !displayinghearts)
    switch (bcol_to_seat(dummycol, pov)) {
    case South: sd = TRUE; break;
    case North: nd = TRUE; break;
    case West:  wd = TRUE; break;
    case East:  ed = TRUE; break;
    default: assert(0);
    }

  if (displaydeck != NULL && (!displayinghearts || seated || dd || kibitzing1))
    switch (pov) {
    case South:
      if (displayinghearts && seated && myseat == West) 
	fulldeal4(southcards(sd),
		  westcards(TRUE), northcards(nd), eastcards(ed));
      else 
	fulldeal4(southcards(sd || seated || kibitzing1),
		  westcards(wd), northcards(nd), eastcards(ed));
      break;
    case North:
      fulldeal4(northcards(nd || seated || kibitzing1), eastcards(ed),
		southcards(sd), westcards(wd));
      break;
    case East:
      fulldeal4(eastcards(ed || seated || kibitzing1), southcards(sd),
		westcards(wd), northcards(nd));
      break;
    case West:
      fulldeal4(westcards(wd || seated || kibitzing1), northcards(nd),
		eastcards(ed), southcards(sd));
      break;
    default:
      assert(0);
    } else {
      nodeal();
      goto done;
    }

  if (played != NULL) {
    int i;
    char s[4];

    for (i = strlen(played) - 2; i >= 0; i -= 2) {
      sprintf(s, "%c %c", played[i], played[i+1]);
      removecardfromhand(s);
    }
    UIredrawmatrixcards();
  }
done:
  if (dd) force_update();
  UInormalcursor();
}

#define col_to_name(col, pov) outputname(col_to_fullname((col), (pov)))
static char *col_to_fullname(int column, int pov)
{
  switch (col_to_seat(column, pov, displayinghearts)) {
  case South: return strexists(southname) ? southname : "(South)";
  case North: return strexists(northname) ? northname : "(North)";
  case East: return strexists(eastname) ? eastname : "(East)";
  case West: return strexists(westname) ? westname : "(West)";
  default: assert(0);
  }
}

#define seat_to_name(seat) outputname(seat_to_fullname(seat))
static char *seat_to_fullname(int seat)
{
  switch (seat) {
  case South: return strexists(southname) ? southname : "(South)";
  case North: return strexists(northname) ? northname : "(North)";
  case East: return strexists(eastname) ? eastname : "(East)";
  case West: return strexists(westname) ? westname : "(West)";
  default: assert(0);
  }
}

/*BUG: assumes current northname is the north player for hand being displayed*/
static void UIshowplayernames(int pov)
{
#if DBGu
  DEBUG(status2("UIshowplayernames ", itoa(pov)));
#endif
  switch(pov) {
  case South:
  default:
    TclDo3("setname self S {", seat_to_name(South), "}");
    TclDo3("setname lho W {", seat_to_name(West), "}");
    TclDo3("setname pard N {", seat_to_name(North), "}");
    TclDo3("setname rho E {", seat_to_name(East), "}");
    break;
  case North:
    TclDo3("setname pard S {", seat_to_name(South), "}");
    TclDo3("setname rho W {", seat_to_name(West), "}");
    TclDo3("setname self N {", seat_to_name(North), "}");
    TclDo3("setname lho E {", seat_to_name(East), "}");
    break;
  case East:
    TclDo3("setname lho S {", seat_to_name(South), "}");
    TclDo3("setname pard W {", seat_to_name(West), "}");
    TclDo3("setname rho N {", seat_to_name(North), "}");
    TclDo3("setname self E {", seat_to_name(East), "}");
    break;
  case West:
    TclDo3("setname rho S {", seat_to_name(South), "}");
    TclDo3("setname self W {", seat_to_name(West), "}");
    TclDo3("setname lho N {", seat_to_name(North), "}");
    TclDo3("setname pard E {", seat_to_name(East), "}");
    break;
  }
}

static void UIupdatepov(int pov, fl_bool dd, int dummycol)
{
  if (displayinghearts) {
    if (pov == West) pov = South;
#if DEBUGHEARTS
    status7("UIupdatepov(pov=", itoa(pov),
	    ", dd=", itoa(dd), ", dummycol=", itoa(dummycol), ")");
#endif
  }

  displaydoubledummy = dd;
  displaypov = pov;

  UIshowdeal(pov, dd, dummycol, displayhandpassstate);
  UIshowplayernames(pov);
}


/*  not used, but should be documented somewhere before this is deleted
void checkbid(char *bid)
{
  int i = 0;
  
  while (isspace(bid[i])) i++;
  assert(isin(bid[i], "12345670?"));
  i++;
  while (isspace(bid[i])) i++;
  assert(isin(tolower(bid[i]), "xprcdhsn?"));
}
*/

static void showmatrixplay(int where, char *play, int pov)
{
/*  if (!isin('?', play)) removecardfromhand(play); */
    
  {
    int i = 0;
    
    while (isspace(play[i])) i++;
    assert(isin(play[i], "cdhsCDHS?"));
    i++;
    while (isspace(play[i])) i++;
    assert(isin(play[i], "AKQJTakqjt23456789?"));
  }

  if (displayinghearts) switch (where) {
    /* West doesn't exist in hearts, so North's RHO is South
       (and South's LHO is North)  */
  case 0:
    if (pov == South) TclDo2("showplay pard ", play);
    else TclDo2("showplay lho ", play);
    break;
  case 1:
    if (pov == North) TclDo2("showplay pard ", play);
    else TclDo2("showplay rho ", play);
    break;
  case 2:
    TclDo2("showplay self ", play);
    break;
  case 3:
    /* BUG here? */
    break;
  default:
    assert(0);
  } else switch (where) {
  case 0:
    TclDo2("showplay lho ", play);
    break;
  case 1:
    TclDo2("showplay pard ", play);
    break;
  case 2:
    TclDo2("showplay rho ", play);
    break;
  case 3:
    TclDo2("showplay self ", play);
    break;
  default:
    assert(0);
  }
}

/* first arg is whether to show the auction at all;
   second is whether to include the buttonbar;
   third is whether it is necessary to display the playing-card area
*/
/* XXX  uggh, all this matrix and auction showing and hiding should be moved to UIback-tk.c */
static void UIauction(fl_bool display, fl_bool showbuttonbar, fl_bool hideMatrix)
{
  static fl_bool displayed = FALSE;
  fl_bool matrixHidden = hasWindows && !atob(TclDo("global matrix_showing; return $matrix_showing([matrix_widget])"));
  
#if DBGu
  if (hasWindows) {
    char s[1000];

    sprintf(s, "UIauction(%d => %d, %d => %d, %d => %d)",
	    (int)display,
	    (int)(display &&
		  !atob(TclDo3("uplevel #0 {info exists hide_auction(",
			       displayhandID, ")}"))),
	    (int)showbuttonbar,
	    (int)(showbuttonbar && atob(TclGet("bidButtons_"))),
	    (int)hideMatrix,
	    (int)(hideMatrix && atob(TclGet("hideMatrix_"))));
    status(s);
  }
#endif

  if (hasWindows) {
    display = display && !atob(TclDo3("uplevel #0 {info exists hide_auction(",
				      displayhandID, ")}"));
/*    showbuttonbar = showbuttonbar && atob(TclGet("bidButtons_")); */
    hideMatrix = hideMatrix && atob(TclGet("hideMatrix_"));
  }

  if (display == displayed && showbuttonbar == displayshowbuttonbar
      && hideMatrix == matrixHidden) return;

  UIpatientcursor();

  if (hasWindows || (displayed != display))
    if ((displayed = display)) 
      TclDo("showauction 1");
    else
      TclDo("showauction 0");

  if (hasWindows) {
    if (showbuttonbar != displayshowbuttonbar)
      if ((displayshowbuttonbar = showbuttonbar))
        TclDo("showButtonBar 1");
      else
	TclDo("showButtonBar 0");

    if (hideMatrix != matrixHidden)
      if ((matrixHidden = hideMatrix))
	TclDo("showMatrix 0");
      else
	TclDo("showMatrix 1");

    UInormalcursor();
  }
}


static void UIshowbid(int col, int row,
		      const char *bidlevel, const char *biddenom)
{
  UIbackend->showbid(col, row, bidlevel, biddenom);
}

static void UIshowplayerbid(int column,
			    const char *bidlevel, const char *biddenom)
{
  UIbackend->showplayerbid(column, bidlevel, biddenom);
}


static void UIauctionstartup(int pov)
{
  int i;

  if (dealercolumn < 0) {
    dealercolumn = bseat_to_col(DEALNO_TO_DEALER(atoi(displayhandID)), pov);
#if DBGu
    DEBUG(status4("pov=", itoa(pov), " dealercol=", itoa(dealercolumn)));
#endif
  }

  if (displaycalls == 0) {
    newauction();
    UIclearmatrix();
    for (i=0; i<dealercolumn; i++)
      UIshowbid(i, 0, "-", "-");
  }
}

/* draw a question mark to show whose call it is */
static void UIdisplaywhosecall(int pov)
{
  int line, column;

  if (auctionisover(displayhandastate)) {
    sleep(125);
    return;
  }
  UIauctionstartup(pov);
  line = (displaycalls + dealercolumn) / 4;
  column = (displaycalls + dealercolumn) % 4;
  UIshowbid(column, line, "?", "?");
  UIshowplayerbid(column, "?", "?");
}

/* the global displaycalls is the call to display */
static void UIdisplaycall(char *auction, int pov)
{
  int call = displaycalls;
  char cmd[100];
  int line, column;
  char bidlevel[5], biddenom[5];
  

#if DBGu
  {
    char foo[1000];

    sprintf(foo, "%c%c; dealercol=%d, pov=%d", auction[call*2],
	    auction[call*2+1], dealercolumn, pov);
    DEBUG(status2("Display call ", foo));
  }
#endif

  UIauctionstartup(pov);

  line = (call + dealercolumn) / 4;
  column = (call + dealercolumn) % 4;

  switch (auction[call * 2 + 1]) {
  case 'r':
    strcpy(bidlevel, "0");
    strcpy(biddenom, "xx");
    break;
  case 'p':
  case 'x':
    strcpy(bidlevel, "0");
    sprintf(biddenom, "%c", auction[call * 2 + 1]);
    break;
  default:
    assert(isdigit(auction[call * 2]));
    sprintf(bidlevel, "%c", auction[call * 2]);
    sprintf(biddenom, "%c", auction[call * 2 + 1]);
    break;
  }
    
    
  UIshowbid(column, line, bidlevel, biddenom);
  UIshowplayerbid(column, bidlevel, biddenom);

  displaycalls++;
}


/* Draw a question mark to show whose play it is.  If we're between tricks,
   update status bar to show who's lead it is. */
static void UIdisplaywhoseplay(char *auction, char *pstate, char *passstate,
			       int pov)
{
  int column, played;

  if (playisover(auction, pstate) ||
      displayhandisover ||
      (displayinghearts &&
       !passisover(passstate, hcol_to_seat(dealercolumn, pov)))) {
    statustolead[0] = '\0';
    return;
  }
  if (((played = numcards(pstate)) % disnumplayers) == 0) {
    sprintf(statustolead, "%s's lead", col_to_name(leadercolumn, pov));
    startshowtricktimer();
    if (played > 0) return;
  } else statustolead[0] = '\0';
  column = (displayplays + leadercolumn) % 4; /* ??? disnumplayers? */
  if (displayinghearts) {
    int np = disnumplayers;
    column = hseat_to_col(((displayplays % np) + hcol_to_seat(leadercolumn, pov)) % np, pov);
  }
  showmatrixplay(column, "? ?", pov);   /* XXX should be factored out */
}

/* Display dummy, except if I am dummy (or I am kibitzing dummy) then
   display declarer's hand. */
static void UIdisplaydummy(int pov)
{
  int dummycol = ((declarercolumn == 1 && (seated || kibitzing1)) ? 1 :
                     (declarercolumn ^ 2));

  if (displaydoubledummy) return;
  UIshowdeal(pov, FALSE, dummycol, NULL);
  UIshowplayernames(pov);
}

/* the global displayplays is the play to display */
static void UIdisplayplay(char *pstate, int pov)
{
  int play = displayplays;
  int column;
  char card[100];

#if DBGu
  {
    char foo[10];

    sprintf(foo, "%c%c", pstate[play*2], pstate[play*2+1]);
    DEBUG(status2("Display play ", foo));
  }
#endif

  if (!displayinghearts && displayplays == 0) UIdisplaydummy(pov);
  /* if not a claim, it's a card, so let's display it */
  if (pstate[play*2] != '!') {
    column = (play + leadercolumn) % 4;
    if (displayinghearts) {
      int np = disnumplayers;
      column = hseat_to_col(((displayplays % np) +
			     hcol_to_seat(leadercolumn, pov)) % np, pov);
    }
    sprintf(card, "%c %c", pstate[play*2], pstate[play*2+1]);
    showmatrixplay(column, card, pov);
    removecardfromhand(card);
  }
  displayplays++;
}


static void UIsummarizeauction(char *auction, int pov)
{
  if (declarercolumn >= 0 && !displayinghearts)
    sprintf(statuscontract, "%s by %s", displaycontract,
	    col_to_name(declarercolumn, pov));
  else /* passed out */
    sprintf(statuscontract, "%s", displaycontract);
}

static void UIupdateauction(char *auction, char *pstate, int pov, fl_bool seated)
{
  int calls;
  fl_bool change = !streq(auction, displayhandastate);

  if (change) STRCPY(displayhandastate, auction);

  if (auctionisover(auction)) {
    UIsummarizeauction(auction, pov);
    UIauction((numcards(pstate) < HIDE_AUCTION),
	      FALSE,
	      FALSE);
    
    if (!change) return;
    calls = numcalls(auction);
    while (displaycalls < calls) UIdisplaycall(auction, pov);
    if (hasWindows) {
      if (!auction_hide_time_inited) init_auction_hide_time();
      if (auction_hide_time >= 0) {
	char s[1000];
	sprintf(s,
		"after [expr round(1000.0 * %f)] {"
		"  gset hide_auction(%s) 1;"
		"  gset needAuctionUpdate 1;"
		"}",
		auction_hide_time, displayhandID);
	TclDo(s);
      }
    }
  } else {
    UIauction(TRUE, seated, TRUE);
    calls = numcalls(auction);
    while (displaycalls < calls) UIdisplaycall(auction, pov);
    UIdisplaywhosecall(pov);
  }

}

static void UIupdatetrickswon(char *pstate, int pov)
{
  int decl, def, we, they;

  computetrickswon(pstate, 1000, displaytrump, declarercolumn,
		   &decl, &def);
  if (declarercolumn == 1 || declarercolumn == 3) {
    we = decl;
    they = def;
  } else {
    we = def;
    they = decl;
  }
  if (seated) sprintf(displaytrickswon, "Tricks won: %d; lost: %d", we, they);
  else sprintf(displaytrickswon, "Tricks won (%s): %d; (%s): %d",
	       ((pov == South || pov == North) ? "NS" : "EW"), we,
	       ((pov == South || pov == North) ? "EW" : "NS"), they);
}

/* update the status of the play; also, when 1st card is played, show dummy */
/*  (or before 1st card is played, show dummy and declarer their two hands) */
static void UIupdateplay(char *auction, char *pstate, char *passstate, int pov)
{
  int plays;

  /* to prevent the display of dummy from happening multiple times */
  static char *lastdisplayhandID = NULL;
  static int lastpov = -99;

  if(displayhandisover) {
    /* don't display any plays, between hands */
    UIclearmatrix();
    return;
  }

  
  /* note the addition of !passisover to the if statement below.  hearts*/
  STRCPY(displayhandpstate, pstate);
  if (strlen(pstate) == 0) 
    if (!auctionisover(displayhandastate) ||
	(displayinghearts &&
	 !passisover(displayhandpassstate, hcol_to_seat(dealercolumn, pov))))
      return;
    else if (!safestreq(lastdisplayhandID, displayhandID) || lastpov != pov) {
      reset(lastdisplayhandID);
      lastdisplayhandID = STRDUP(displayhandID);
      lastpov = pov;
      UIclearmatrix();
      DEBUG(status2("Time to display dummy; declarercolumn=",
		    itoa(declarercolumn))); 
      if (declarercolumn == 1 || declarercolumn == 3) UIdisplaydummy(pov);
    }
  
  plays = numcards(pstate);
  while (displayplays < plays) {
    /* assert(displayplays < plays); */
    if (displayplays % disnumplayers == 0) {
      if (displayinghearts) {
	int leaderseat = computeleader(pstate, displayplays,
				       displaytrump, -1, TRUE);
	leadercolumn = hseat_to_col(leaderseat, pov);
#if DEBUGHEARTS
	DEBUG(status6("displayplays=", itoa(displayplays),
		      " leaderseat=", itoa(leaderseat),
		      " leadercolumn=", itoa(leadercolumn)));
#endif
	
      } else leadercolumn = computeleader(pstate, displayplays,
					  displaytrump, declarercolumn,
					  FALSE);
      UIclearmatrix();
    }
    assert(displayplays < plays);
    UIdisplayplay(pstate, pov);
    if (displayplays == 49 && !displayinghearts && !isin('!', pstate))
      display_final_trick(displayhandID, pstate);
  }
  if (displayplays % disnumplayers == 0) {
    if (displayinghearts) {
      int leaderseat = computeleader(pstate, displayplays,
				     displaytrump, -1, TRUE);
      leadercolumn = hseat_to_col(leaderseat, pov);
    } else 
      leadercolumn = computeleader(pstate, displayplays,
				    displaytrump, declarercolumn, FALSE);
  }
  if (!displayinghearts) UIupdatetrickswon(pstate, pov);
  UIdisplaywhoseplay(auction, pstate, passstate, pov);
}

static int beepatmyturn() {
  return atob(TclGet("beepAtMyTurn_"));
}



/* the workhorse */
void UIupdate(void)
{
  int pov, dummycolumn = -1;
  char *astate, *pstate, *passstate;
  fl_bool povchange, seatedchange;
  fl_bool updateall = FALSE;
  extern fl_bool claiming;
  extern int curclaimtricks;
  static fl_bool expectingNULL = TRUE; /* are we expecting curhandID is NULL? */
  static fl_bool expecting_myturntoact = FALSE;

  DISALLOW_REENTRY();
  draw_on_matrix_screen();

  /* beep if it is now my turn to act, but it wasn't last time through here */
  /* XXX this can't be the right location for this code -lex */
  if (myturntoact && !expecting_myturntoact) {
    expecting_myturntoact = TRUE;
    if (beepatmyturn()) UIbeep();
    startMyTurnTimer();
  } else if (!myturntoact && expecting_myturntoact) {
    expecting_myturntoact = FALSE;
    stopMyTurnTimer();
  }

  if (curhandID == NULL) {
    /* clear _everything_ */
    spec = FALSE;
    if (!kibitzing1) kibitzingseat = Noseat;
    if (!expectingNULL) {
      expectingNULL = TRUE;
      nodeal();
      noauction();
      UIclearmatrix();
      statustolead[0] = statusclaim[0] = statusresult[0] = statuscontract[0] =
	displaycontract[0] = displaytrickswon[0] = statushandvul[0] =
	statushanddlr[0] = displayhandID[0] = displayhandname[0] =
	statusscore[0] = '\0';
    }
    goto done;
  }
  if (seated) pov = myseat; else pov = South;
  if (kibitzing1) pov = kibitzingseat;
  if (displayinghearts && pov == West) pov = South;
  if (strlen(displayhandID) == 0 ||
      (curhandID != NULL && !streq(curhandID, displayhandID) && caughtup)) {
/*    newauction();*/
    spec = FALSE;
    if (!kibitzing1) kibitzingseat = Noseat;
    displayhandisover = FALSE;
    if (curhandID == NULL || strlen(curhandID) == 0) goto done;
    strcpy(displayhandID, curhandID);
    strcpy(displayhandname, TclDo3("gset handname(", curhandID, ")"));
    status2("Displaying hand ", displayhandname);
    UIclearmatrix();
    reset(displaydeck);
    displayhandscoring = atoi(TclDo3("gset handscoring(", curhandID, ")"));
    displayinghearts = ishearts(displayhandscoring);
    displayingrubber = isrubber(displayhandscoring);
    disnumplayers = displayinghearts ? 3 : 4; /* assumes 3-handed hearts */
    if (displayinghearts)
      parseheartsgstate(TclDo3("gset handgstate(", curhandID, ")"),
			&nscore, &escore, &sscore);
    if (displayingrubber)
      parserubbergstate(TclDo3("gset handgstate(", curhandID, ")"),
			&disnsvul, &disewvul,
			&disnstotal, &disewtotal,
			&disnsbelowline, &disewbelowline);
    if (displayingrubber) {
      int vul = disewvul ? (disnsvul ? Both : EW) : (disnsvul ? NS : Nonevul);

      strcpy(statushandvul, vulnames[vul]);
      sprintf(statusscore, "NS: %d/%d; EW: %d/%d",
	      disnstotal - disnsbelowline, disnsbelowline,
	      disewtotal - disewbelowline, disewbelowline);
      sprintf(statushanddlr, "%c dealt",
	      seat_to_char(DEALNO_TO_DEALER(atoi(displayhandID))));
    } else if (displayinghearts) {
      sprintf(statusscore, "N: %d; E: %d; S: %d",
	      nscore, escore, sscore);
      strcpy(statushandvul,
	     passnames[DEALNO_TO_DEALER(atoi(displayhandID))]);
    } else {
      /* duplicate bridge */
      statusscore[0] = '\0';
      strcpy(statushandvul, vulnames[ONEB(atoi(curhandID) - 1)]);
      sprintf(statushanddlr, "%c dealt",
	      seat_to_char(DEALNO_TO_DEALER(atoi(displayhandID))));
    }

    displayhandastate[0] = displayhandpstate[0] =
      displayhandpassstate[0]= '\0';
    statustolead[0] = statusclaim[0] = statusresult[0] = statuscontract[0] =
      displaycontract[0] = displaytrickswon[0] = '\0';
    dealercolumn = declarercolumn = -1;
    displayplays = 0;
    displaycalls = 0;
    displaypov = -1;
    displaytrump = -1;
    updateall = TRUE;
  }
  if (caughtup) {
    fl_bool oppclaim; /* whether claim in progress should make all 4 hands vis. */

    if (dealercolumn < 0) {
      dealercolumn = seat_to_col(DEALNO_TO_DEALER(atoi(displayhandID)), pov,
				 displayinghearts);
#if DBGu
      DEBUG(status4("pov=", itoa(pov), " dealercol=", itoa(dealercolumn)));
#endif
    }
    if (claiming) 
      sprintf(statusclaim, "Claiming %d tricks total", curclaimtricks);
    else statusclaim[0] = '\0';
    povchange = (pov != displaypov);
    if (povchange) displaycalls = 0; /* need to redisplay whole auction */
    seatedchange = (displayseated != seated);
    if (auctionisover(displayhandastate) &&
	(displaytrump < 0 || povchange || updateall))
      computecontract(displayhandastate, dealercolumn,
		      displaycontract, &declarercolumn, &displaytrump);
    dummycolumn = (declarercolumn >= 0) ? ((declarercolumn + 2) % 4) : -1;
    oppclaim = claiming && (declarercolumn <= 0 || declarercolumn == 2);
    astate = TclDo3("gset handastate(", displayhandID, ")");
    pstate = TclDo3("gset handpstate(", displayhandID, ")");
    if (updateall || povchange || displaydoubledummy != (spec || oppclaim || playisover(astate,pstate)))
      UIupdatepov(pov, (spec || oppclaim || playisover(astate,pstate)), dummycolumn);
    passstate = TclDo3("gset handpassstate(", displayhandID, ")");
#if 0
    status2("UIupdate() with pass state=", passstate);
#endif
    if (seatedchange || povchange || updateall || needAuctionUpdate() ||
	(strlen(displayhandpstate) != strlen(pstate) &&
	 numcards(pstate) >= HIDE_AUCTION) ||
	!streq(displayhandastate, astate)) {
      if (displayinghearts) UIauction(FALSE, FALSE, FALSE);
      else {
	UIupdateauction(astate, pstate, pov, seated);
	if (auctionisover(displayhandastate) &&
	    (displaytrump < 0 || povchange || updateall))
	  computecontract(displayhandastate, dealercolumn,
			  displaycontract, &declarercolumn, &displaytrump);
	dummycolumn = (declarercolumn >= 0) ? ((declarercolumn + 2) % 4) : -1;
	if (updateall || povchange || displaydoubledummy != (spec || oppclaim))
	  UIupdatepov(pov, (spec || oppclaim || playisover(astate,pstate)), dummycolumn);
      }
    }

    if (displayinghearts && UIupdatepass(passstate, pov)) updateall = TRUE;

    if (seatedchange || povchange || updateall)
      UIupdatepov(pov, (spec || oppclaim || playisover(astate,pstate)), dummycolumn);

  /* note the addition of !passisover to the if statement below.  hearts*/
    if (seatedchange || povchange || updateall || 
	(auctionisover(astate) && 
	 (!displayinghearts ||
	  passisover(passstate, hcol_to_seat(dealercolumn, pov)))) ||
	!streq(displayhandpstate, pstate))
      UIupdateplay(astate, pstate, passstate, pov);
    displayseated = seated;
    if (playisover(astate, pstate) && !displayhandisover) {
      int restricks, respoints;
      fl_bool vulp;
      int vul, declarerseat;
      int unused = 0;

      no_spectators(displayhandID);
      if (!kibitzing1) kibitzingseat = Noseat;  /* this oughtta unlock 'em */
      displayhandisover = TRUE;
      statustolead[0] = '\0';
  
      declarerseat = col_to_seat(declarercolumn, pov, displayinghearts);
      vul = displayingrubber ?
	(disewvul ? (disnsvul ? Both : EW) : (disnsvul ? NS : Nonevul)) :
	ONEB(atoi(displayhandID) - 1);
      vulp =
	(vul == Both ||
	 ((declarerseat == North || declarerseat == South) && vul == NS) || 
	 ((declarerseat == East || declarerseat == West) && vul == EW));
      computeresult(displaycontract, pstate, declarerseat,
		    vulp, &restricks, &respoints,
		    displayingrubber ? &unused : NULL);
      if ((declarercolumn & 1) == 0) respoints = -respoints;
      if (!displayinghearts) {
	if (restricks < 0)
	  sprintf(statusresult, "Down %d, %d", -restricks, respoints);
	else if (restricks > 0)
	  sprintf(statusresult, "Made %d, %d", restricks, respoints);
	else
	  sprintf(statusresult, "Passed out");
#if DEBUGBR
	if (restricks != 0) {
	  char s[1000];
	
	  sprintf(s, "%s by %s (%s), %s", displaycontract,
		  col_to_name(declarercolumn, pov), vulp ? "v" : "nv",
		  statusresult);
	  status(s);
	}
#endif
      }

      /* display all 52 cards to everyone */
      UIclearmatrix();
      UIshowdeal(pov, TRUE, -1, displayhandpassstate);
      UIshowplayernames(pov);
      /* UIdelayedclearmatrix(); */
    }
  }
  expectingNULL = FALSE;
 done:
  if (!hasWindows)
    needrefresh = TRUE;
  ALLOW_REENTRY();
  return;
}

/* which is North, South, East, or West */
void UIplayergone(int which)
{
  if ((curhandID == NULL && caughtup) ||
      (curhandID != NULL && streq(displayhandID, curhandID)))
    UIshowplayernames(displaypov);
}

/* ch is one of N, S, E, W */
/* (ch is unused??  why not delete it?) */
void UIsit(char *name, char ch, int myseat)
{
  int newpov;

#if DBGu
  DEBUG(status5("UIsit(", name, " ", itoa(myseat), ")"));
#endif

  if (hasWindows) {
    extern fl_bool claiming;
    extern int curdeclarer;

    UIpatientcursor();
    TclDo2("activate_seated_menus ", itoa(seated));
    TclDo2("menus_declaring ", itoa(declaring()));
    if (claiming) {
      if (declaring()) TclDo("menus_declclaim");
      else if (defending()) TclDo("menus_defclaim");
    }
  }

  if (myseat == Noseat && !kibitzing1 && strlen(displayhandpstate) == 0) {
    reset(displaydeck);
    nodeal();
  }
  if (!(displaypov == myseat ||
	(myseat == Noseat && displaypov == South && !kibitzing1) ||
	(kibitzing1 && displaypov == kibitzingseat))) {
    dealercolumn = declarercolumn = -1;
    displayplays = 0;
    displaycalls = 0;
    displaypov = -1;
    displayhandpassstate[0] = '\0'; /* for hearts */
  }

  newpov = (myseat >= 0) ? myseat :
    (kibitzing1 ? kibitzingseat : South);
  
  if ((curhandID == NULL && caughtup) ||
      (curhandID != NULL && displayhandID != NULL &&
       streq(displayhandID, curhandID)))
    UIshowplayernames(newpov);
  if (!hasWindows) {
    char s[100];
    
    sprintf(s, "textseated %c %c", seated ? '1' : '0', seat_to_char(newpov));
    TclDo(s);
  }
  if (hasWindows) UInormalcursor();
}




void UIbeep(void){
  UIbackend->beep();
}

