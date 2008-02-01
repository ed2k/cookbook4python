
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

/* UIBack-tk.c -- UI backend using Tcl/Tk */

#include "floater.h"
#include "tickler.h"
#include "UIback.h"
#include "br.h"
#include "comm.h"
#include <stdio.h>


static void tk_setstatusline(const char *s) {
  TclDo3("gui_setstatus {", s, "}");
}

static void tk_setinfoline(const char *s) {
  TclDo3("gui_setinfo {", s, "}");
}

static void tk_beep() {
  TclDo("catch {Floater_bell; Floater_deiconify}");
}


static void tk_clearmatrix(int dodelay) {
  if(dodelay)
    TclDo("after 5000 erasebidplay all");
  else
    TclDo("erasebidplay all");
}


static void tk_redrawmatrixcards() {
  TclDo("redrawmatrixcards");
}


static void tk_force_update() 
{
     /* XXX is this really needed??  explain if so.  update is
      * dangerous to call....  */
#if 0
  TclDo("update idletasks");
#endif
}


static void tk_startMyTurnTimer()
{
  TclDo("startMyTurnTimer");
}


static void tk_stopMyTurnTimer()
{
  TclDo("stopMyTurnTimer");
}


static void tk_removecardfromhand(const char *play)
{
  TclDo2("removecardfromhand ", (play));
}

static void tk_togglepassedcard(const char *play)
{
  TclDo3("catch {togglepassedcard ", (play), "}");
}

static void tk_newauction(void)
{
  TclDo("gui_newauction");
}


static void tk_fulldeal(const char *selfs,
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



static void tk_insertTalk(const char *s)
{
  TclDo3("global toplevel; $toplevel.cmd.talk insert end {", s, "}");
}

static void tk_insertCmd(const char *s)
{
  TclDo3("global toplevel; $toplevel.cmd.cmdline insert end {", s, "}");
}


static void tk_updatestatus(struct ui_status *status)
{
  char connstat[200];   /* XXX misnamed; really just a generic status line */
  
#define appendifexists(s, nl)				\
    {							\
      if (strlen(connstat) > 0 && strlen(s) > 0)	\
	strcat(connstat, nl ? "|" : "; ");				\
      strcat(connstat, s);				\
    }

  strcpy(connstat, status->connstat);
  
  appendifexists(status->displayhandname, 0);
  appendifexists(status->statushandvul, 0);
  if (! status->displayinghearts) {
    appendifexists(status->statushanddlr, 1);
    appendifexists(status->statuscontract, 0);
  }
  setstatusline(status->connstat);
  
  connstat[0] = '\0';
  appendifexists(status->statusscore, 0);
  appendifexists(status->displaytrickswon, 1);
  {
    char *temp = display_of_previous_trick();
    
    /* these nested if's try to pick the one most important thing
     * to put onto the last part of the infoline */
    if (strexists(temp)) {
      appendifexists(temp, 1);
    }
    else {
      if(strlen(status->statusresult) > 0) {
	appendifexists(status->statusresult, 1);
      }
      else {
	appendifexists(status->statusclaim, 1);
      }
    }
  }
  
  setinfoline(connstat);
#undef appendifexists
}

static void tk_patientcursor() 
{
  TclDo("patientcursor");
}

static void tk_normalcursor() 
{
  TclDo("normalcursor");
}


static void tk_hoststatus(int tablehostmode,
			  int scoremethod,
			  int iscompetitive)
{
  dTclDo5("menus_tablehost ", tablehostmode ? "1 " : "0 ",
	  ((scoremethod >= 0) ?
	   scoringmethods[scoremethod].scoringname :
	   ((scoremethod >= 0) ?
	    scoringmethods[scoremethod].scoringname :
	    "x")),
	  " ", iscompetitive ? "Competitive" : "Noncompetitive");
}


static void tk_commstate(int state)
{
  if (state == CONNECTED)
    TclDo("activate_table_menus 1");
  else
    TclDo("activate_table_menus 0");
}


static void tk_showbid(int col, int row, const char *bidlevel, const char *biddenom)
{
  char cmd[1000];
  char frame[1000];

  /* get the current location of the cell */
  sprintf(cmd, "auction_cell %d %d", col, row);
  strcpy(frame, TclDo(cmd));
  
  /* now set the bid */
   sprintf(cmd, "drawbid %s %s %s", frame, bidlevel, biddenom);
  TclDo(cmd);
}

  

static void tk_showplayerbid(int column,
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



static struct ui_backend tk_backend = {
  tk_setstatusline,
  tk_setinfoline,
  tk_beep,
  tk_clearmatrix,
  tk_redrawmatrixcards,
  tk_force_update,
  tk_startMyTurnTimer,
  tk_stopMyTurnTimer,
  tk_removecardfromhand,
  tk_togglepassedcard,
  tk_newauction,
  tk_fulldeal,
  tk_insertTalk,
  tk_insertCmd,
  tk_updatestatus,
  tk_patientcursor,
  tk_normalcursor,
  tk_hoststatus,
  tk_commstate,
  tk_showbid,
  tk_showplayerbid
};


void UIuse_tk() 
{
  UIbackend = &tk_backend;
}




