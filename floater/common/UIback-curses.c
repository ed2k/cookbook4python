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

/* UIBack-curses.c -- UI backend using curses */

#include "floater.h"
#include "UIback.h"

#include <stdio.h>


static void curses_setstatusline(const char *s) {
  TclSet("statusline", (s));
}

static void curses_setinfoline(const char *s) {
  TclSet("infoline", (s));
}

static void curses_beep() 
{
  flash();
  beep();
}


static void curses_clearmatrix(int dodelay)
{
  textrefresh();
  if(dodelay)
    TclDo("delayedclearmatrix");
  else
    TclDo("erasebidplay all");
}

static void curses_redrawmatrixcards() 
{
}


static void curses_force_update() 
{
  textrefresh();
}

static void curses_startMyTurnTimer() 
{
}

static void curses_stopMyTurnTimer() 
{
}


static void curses_removecardfromhand(const char *play)
{
  TclDo2("removecardfromhand ", (play));
}

static void curses_togglepassedcard(const char *play)
{
  TclDo3("catch {togglepassedcard ", (play), "}");
}


static void curses_newauction(void) 
{
  TclDo("showauction 1");
}



static void curses_fulldeal(const char *selfs,
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
  char command[1000];


  sprintf(command,
	  "fulldeal"
	  " {%s} {%s} {%s} {%s}"
	  " {%s} {%s} {%s} {%s}"
	  " {%s} {%s} {%s} {%s}"
	  " {%s} {%s} {%s} {%s}",
	  selfs, selfh, selfd, selfc,
	  lhos, lhoh, lhod, lhoc,
	  ps, ph, pd, pc,
	  rhos, rhoh, rhod, rhoc);

  TclDo(command);
}


static void curses_inserttalk(const char *s) 
{
  /* is this doable? */
}


static void curses_insertcmd(const char *s) 
{
  /* is this doable? */
}


static void curses_updatestatus(struct ui_status *status)
{
  update_statusbar();
  draw_on_matrix_screen();
  TclDo3("connstat {", status->connstat, "}");
  TclDo3("displayhandname {", status->displayhandname, "}");
  TclDo3("statushandvul {", status->statushandvul, "}");
  if (! status->displayinghearts) {
    TclDo3("statushanddlr {", status->statushanddlr, "}");
    TclDo3("statuscontract {", status->statuscontract, "}");
  }
  TclDo3("statustolead {", status->statustolead, "}");
  TclDo3("displaytrickswon {", status->displaytrickswon, "}");
  TclDo3("statusclaim {", status->statusclaim, "}");
  TclDo3("statusresult {", status->statusresult, "}");
  TclDo3("statusscore {", status->statusscore, "}");
  move_to_curyx();
}

static void curses_patientcursor() 
{
}

static void curses_normalcursor() 
{
}


static void curses_hoststatus(int tablehostmode,
			      int scoremethod,
			      int iscompetitive)
{
}


static void curses_commstate(int state)
{
}

static void curses_showbid(int col, int row,
			   const char *bidlevel, const char *biddenom)
{
  char cmd[1000];

  sprintf(cmd, "drawbid %d %d %s %s", col, row, bidlevel, biddenom);
  TclDo(cmd);
}

static void curses_showplayerbid(int column,
				 const char *bidlevel,
				 const char *biddenom)
{
  char cmd[100];

  sprintf(cmd,
	  "showbid %s %s %s",
	  bcol_to_string(column),
	  bidlevel, biddenom);
  
  TclDo(cmd);
}





static struct ui_backend curses_backend = {
  curses_setstatusline,
  curses_setinfoline,
  curses_beep,
  curses_clearmatrix,
  curses_redrawmatrixcards,
  curses_force_update,
  curses_startMyTurnTimer,
  curses_stopMyTurnTimer,
  curses_removecardfromhand,
  curses_togglepassedcard,
  curses_newauction,
  curses_fulldeal,
  curses_inserttalk,
  curses_insertcmd,
  curses_updatestatus,
  curses_patientcursor,
  curses_normalcursor,
  curses_hoststatus,
  curses_commstate,
  curses_showbid,
  curses_showplayerbid
};


void UIuse_curses() 
{
  UIbackend = &curses_backend;
}
