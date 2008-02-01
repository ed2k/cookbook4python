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
#ifndef __BR_H__
#define __BR_H__

#include "message.h"
#include "deal.h"

extern fl_bool competitive;
extern fl_bool new_competitive;

/* how many scoring methods use (worldwide) identical random hands */
#define NUMPAIRGAMES 2
#define DUPLICATE(scoremethod) ((scoremethod) < NUMPAIRGAMES)
#define NONCOMPETITIVE() (scoremethod < NUMPAIRGAMES && !competitive)

typedef void (*startupfn)(void);
typedef void (*seedfn)(void);
typedef void (*playfn)(void);

typedef struct {
  char *scoringname;
  int numplayers;
  startupfn startup;
  seedfn seed;
  /* playfn play; */
} scoring;

/* hearts stuff */
extern fl_bool playinghearts;
extern int cardsperhand;
extern fl_bool passisover(char *, int);
extern void updatepassstate(char *, char *);
extern fl_bool isnewinfo(char *, char *);
extern void reportpassstatus(char *);
extern void adjusthands(char *, int, int *);
void broadcast_curpassstate(void);

extern int scoremethod; /* index into scoringmethods[] */
#define numscoringmethods 4
extern scoring scoringmethods[numscoringmethods];
/* #define numscoringmethods (sizeof(scoringmethods)/sizeof(scoringmethods[0])) */
fl_bool isrubber(int n);
fl_bool ishearts(int n);
fl_bool isrubberstring(char *s);
fl_bool isheartsstring(char *s);

void parseheartsgstate(char *, int *, int *, int *);
void parserubbergstate(char *, fl_bool *, fl_bool *, int *, int *, int *, int *);

#define passedout(auction) streq(auction, " p p p p")
#define numcards_(play) (isin('!', (play)) ? 52 : (strlen(play) / 2))
#define numcalls(a) (strlen(a) / 2)
#define numcards(p) \
    (isin('!', (p)) ? ((strlen(p) / 2) - 1) : (strlen(p) / 2))

extern fl_bool seated;

extern int myseat;
extern int rhoseat;

extern char playerport[4][MAXPORTLEN+1];
extern char playeraddr[4][MAXADDRLEN+1];
#define rhoaddr playeraddr[rhoseat]
#define rhoport playerport[rhoseat]
fl_bool auctionisover(char *auction);
fl_bool playisover(char *auction, char *play);
void sit(char *name, char seat, char *addr, char *port);
void cleartable(void);
char *contract(char *auction);
char declarer(char *auction, int dealer);
char seat_to_char(int seat);
fl_bool myturntobid(void);
fl_bool myturntoplay(void);
fl_bool silentmyturntobid(void);
fl_bool silentmyturntoplay(void);
void followsmall(void);
fl_bool executebid(char *b);
fl_bool executeplay(char *p);
void generatedeal(void);
char *setscoringmethod(char *method);
char *prettyauction(char *auction);
char *prettyprintcardset(char *w);
void showprevioushand(void);
void reviewauction(void);
void showlasttrick(void);
char *display_of_previous_trick(void);

void makerandombid(void);
void makerandomplay(void);

void playsmall(char suit); /* try to play my smallest card of given suit */
fl_bool follow(char *card); /* follow suit with the named card */

fl_bool filehandinfo(message *m);
void tellseatstatus(char *connstr);
void tellhandstatus(char *name, char *connstr);
void tellcc(char *connstr);
void ccupdate(char *who, char *cc);
fl_bool unoccupied(char seat);
char *name_at_seat(char seat);
void persongone(char *name);
void nobridge(void);
void parseseen(char seat, int index, char *seen);
fl_bool haveseen(int i, int num);
#define loadseen() TclDo("loadseen")

/* a `column' defines a seat, counting from the player's left */
#define BCOL_SELF 3
#define BCOL_LHO 0
#define BCOL_PARD 1
#define BCOL_RHO 2

#define HCOL_SELF 2
#define HCOL_LHO 0
#define HCOL_RHO 1

/* a `seat' is an integer reperesenting North, East, etc.
   They are defined in deal.h */
#define bcol_to_seat(column, pov) (((column) + 1 + (pov)) % 4)
#define bseat_to_col(seat, pov) (((seat) + 3 - (pov)) % 4)
#define hcol_to_seat(column, pov) (((column) + 1 + (pov)) % 3)
#define hseat_to_col(seat, pov) (((seat) + 2 - (pov)) % 3)
#define col_to_seat(column, pov, hearts) \
  ((hearts) ? hcol_to_seat((column), (pov)) : bcol_to_seat((column), (pov)))
#define seat_to_col(seat, pov, hearts) \
  ((hearts) ? hseat_to_col((seat), (pov)) : bseat_to_col((seat), (pov)))

#define validseat(seat) (((seat) <= 3) && ((seat) >= 0))

const char *bcol_to_string(int bcol);


#define declaring() (seated && (curdeclarer == myseat))
#define defending() \
	(seated && validseat(curdeclarer) && ((curdeclarer ^ myseat) & 1))

int computeleader(char *pstate, int cardsplayed, int trumpsuit, int declarer,
		  fl_bool hearts);
void computecontract(char *auction, int dealer, char *contract,
		     int *declarer, int *trump);
void computetrickswon(char *pstate, int tricks,
		      int trump, int declarer, int *decl, int *defense);
void computeresult(char *contract, char *play, int declarer, fl_bool vul,
		   int *resulttricks, int *resultpoints, int *legs);

void showinitialcards(int seat);
void down(char *s);
void make(char *s);
void claim(char *s);
void acceptclaim(void);
void retractclaim(void);
void rejectclaim(void);
void handleclaimmessage(char kind, char *handID,
			int tricks, char *from); /* called by comm.c */

void resetseen(void);

char *bridge_statusbar(char *s, int start, int end, fl_bool *center);

void parse_you_have_seen(char *set, char *msg);
int handnum(char *handname);
char *handname_to_setname(char *handname);
char *handname_to_weekname(char *handname);

#ifdef LOGINSERVER
void announce_seen(char *connstr, char *name);
#endif

#define isCompetitiveHandname(n) (!isin('-', (n)))

typedef void (*insertcharfn)(char *);

fl_bool insert_default_action(fl_bool needSlash, insertcharfn f);

#endif
