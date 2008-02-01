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

/* UIBack.h -- generic interface to UI backends. */

#ifndef __uiback_H__
#define __uiback_H__


/* various information about the current status */
struct ui_status {
int displayinghearts;
const char *connstat;
const char *displayhandname;
const char *statushandvul;
const char *statushanddlr;
const char *statuscontract;
const char *statustolead;
const char *displaytrickswon;
const char *statusclaim;
const char *statusresult;
const char *statusscore;
};



struct ui_backend 
{
void (*setstatusline)(const char *);
void (*setinfoline)(const char *);
void (*beep)(void);

void (*clearmatrix)(int dodelay);
void (*redrawmatrixcards)(void);
void (*force_update)(void);
void (*startMyTurnTimer)(void);
void (*stopMyTurnTimer)(void);
void (*removecardfromhand)(const char *play);
void (*togglepassedcard)(const char *play);

void (*newauction)(void);

void (*fulldeal)(const char *selfs,
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
		 const char *rhoc);

void (*insertTalk)(const char *s);
void (*insertCmd)(const char *s);

void (*updatestatus)(struct ui_status *status);

void (*patientcursor)(void);
void (*normalcursor)(void);

void (*hoststatus)(int tablehostmode,
		   int scoremethod,
		   int iscompetitive);

void (*commstate)(int state);

/* show a bid - if the row is 0 and the bid is "-", then
   erase the bid along with all bids underneath it. */
void (*showbid)(int col, int row, const char *bidlevel, const char *biddenom);


/* show a bid in front of a player.  The "column" is as defined in br.h */
void (*showplayerbid)(int column, const char *bidlevel, const char *biddenom);
};


extern struct ui_backend *UIbackend;


void UIuse_tk(void);
void UIuse_curses(void);

#endif /* __uiback_H__ */

