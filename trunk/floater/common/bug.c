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
#include "floater.h"
#include "br.h"
#include "comm.h"
#include "UI.h"

#define FLOATER_EXECUTIONLOG 15
#define EXECUTIONLOG_MAXLEN 200

static int exlogged = 0;
static char *exlog[FLOATER_EXECUTIONLOG];

void executionlog(char *label, char *event)
{
  int i;

  if (exlogged++ == 0)
    for (i = 0; i < FLOATER_EXECUTIONLOG - 1; i++) exlog[i] = NULL;

  if (exlog[0] != NULL) reset(exlog[0]);
  for (i = 0; i < FLOATER_EXECUTIONLOG - 1; i++) exlog[i] = exlog[i + 1];
  exlog[i] = STRCAT(label, event);
}

void dumpexecutionlog(char *s)
{
  int i;
  
  for (i = 0; i < FLOATER_EXECUTIONLOG; i++)
    if (strexists(exlog[i])) {
      if (strlen(exlog[i]) > EXECUTIONLOG_MAXLEN)
	exlog[i][EXECUTIONLOG_MAXLEN] = '\0';
      sprintf(s + strlen(s), "log%d: %s\n",
	      exlogged + i - (FLOATER_EXECUTIONLOG - 1),
	      exlog[i]);
    }    
}

/* generate some text that is included in all bug report emails */
static char *dumpvars(void)
{
  char *s = markgarbage(salloc(10000));

#define CKNULL(w) ((w) == NULL ? "NULL" : (w))
#define NULL_OR_NOT(w) ((w) == NULL ? "NULL" : "not NULL")

  extern char *fatal_error_message;

  /* from br.c */
  extern int handcounter;
  extern char *curhandname;
  extern char *curhandID ;
  extern char *curastate ;
  extern char *curpstate ;
  extern char *curprettyauction ;
  extern int curdealer ;
  extern int curdeclarer ;
  extern char curcontract[10];
  extern int curtrump ;
  extern int curdeck[];
  extern char initialcards[4][35];
  extern int currestricks, currespoints;
  extern fl_bool claiming;
  extern int curclaimtricks;
  extern char *claimaccept;
  extern char *northname, *southname, *eastname, *westname;

  /* from comm.c */
  extern char *localIPaddr;
  extern char *localport, *mynote, *mytabledesc;
  extern char tablehostname[];
  extern char tablehostaddr[];
  extern char tablehostport[];
  extern char tablerootname[];
  extern char tablerootaddr[];
  extern char tablerootport[];
  extern fl_bool triedtologin;
  extern fl_bool loggedin;
  extern fl_bool tablerootmode;
  extern int state;

  /* from UI.c */
  extern fl_bool kibitzing1 ;
  extern int kibitzingseat;
  extern char displayhandID[];
  extern char displayhandname[];
  extern char statushandvul[];
  extern char statushanddlr[];
  extern char displayhandastate[];
  extern char displayhandpstate[];
  extern int displaypov;
  extern int dealercolumn;
  extern int displaycalls;
  extern int displayplays;
  extern fl_bool displayseated ;
  extern fl_bool displayshowbuttonbar ;
  extern char statustolead[];
  extern char statuscontract[];
  extern char statusresult[];
  extern char statusclaim[];
  extern char displaycontract[];
  extern char displaytrickswon[];
  extern int leadercolumn;
  extern int declarercolumn;
  extern int displaytrump;
  extern fl_bool displayhandisover;
  extern fl_bool caughtup;

  dumpexecutionlog(s);
  sprintf(s + strlen(s), "fatal_error_message=%s\n"
	  "version=%s\n"
	  "uname=%s\n"
	  "platform=%s\n"
	  "tclversion=%s\n"
	  "comp=%d\n"                  
	  "new_comp=%d\n"		  
	  "seated=%d\n"			  
	  "myseat=%d\n"			  
	  "myname=%s\n"			  
	  "northname=%s\n"		  
	  "southname=%s\n"		  
	  "eastname=%s\n"		  
	  "westname=%s\n"		  
	  "handcounter=%d\n"		  
	  "curhandname=%s\n"		  
	  "curhandID=%s\n"		  
	  "curastate=%s\n"		  
	  "curpstate=%s\n"		  
	  "curprettyauction=%s\n"	  
	  "curdealer=%d\n"		  
	  "curdeclarer=%d\n"		  
	  "curcontract=%s\n"		  
	  "curtrump=%d\n"		  
	  "curdeck=%s\n"		  
	  "initialcards[North]=%s\n"	  
	  "initialcards[South]=%s\n"	  
	  "initialcards[East]=%s\n"	  
	  "initialcards[West]=%s\n"	  
	  "currestricks=%d\n"		  
	  "currespoints=%d\n"		  
	  "claiming=%d\n"		  
	  "curclaimtricks=%d\n"		  
	  "claimaccept=%s\n"		  
	  "tablehostmode=%d\n"		  
	  "localIPaddr=%s\n"		  
	  "localport=%s\n"		  
	  "mynote=%s\n"			  
	  "mytabledesc=%s\n"		  
	  "tablehostname=%s\n"		  
	  "tablehostaddr=%s\n"		  
	  "tablehostport=%s\n"		  
	  "tablerootname=%s\n",
	  CKNULL(fatal_error_message),
	  TclGet("floater_version"),
	  TclDo("catch {exec uname -a} uname; set uname"),
	  TclGet("platform"),
	  (hasWindows ? TclDo("set s [info tclversion]/[global tk_version; set tk_version]") : TclDo("info tclversion")),
	  competitive,
	  new_competitive,
	  seated,
	  myseat,
	  CKNULL(myname),
	  CKNULL(northname),
	  CKNULL(southname),
	  CKNULL(eastname),
	  CKNULL(westname),
	  handcounter,
	  CKNULL(curhandname),
	  CKNULL(curhandID),
	  CKNULL(curastate),
	  CKNULL(curpstate),
	  CKNULL(curprettyauction),
	  curdealer,
	  curdeclarer,
	  CKNULL(curcontract),
	  curtrump,
	  NULL_OR_NOT(curdeck),
	  CKNULL(initialcards[North]),
	  CKNULL(initialcards[South]),
	  CKNULL(initialcards[East]),
	  CKNULL(initialcards[West]),
	  currestricks,
	  currespoints,
	  claiming,
	  curclaimtricks,
	  CKNULL(claimaccept),
	  tablehostmode,
	  CKNULL(localIPaddr),
	  CKNULL(localport),
	  CKNULL(mynote),
	  CKNULL(mytabledesc),
	  CKNULL(tablehostname),
	  CKNULL(tablehostaddr),
	  CKNULL(tablehostport),
	  CKNULL(tablerootname));

  sprintf(s + strlen(s), 
	  "tablerootaddr=%s\n"		  
	  "tablerootport=%s\n"		  
	  "loginservername=%s\n"	  
	  "loginserveraddr=%s\n"	  
	  "loginserverport=%s\n",
	  CKNULL(tablerootaddr),
	  CKNULL(tablerootport),
	  CKNULL(loginservername()),
	  CKNULL(loginserveraddr()),
	  CKNULL(loginserverport()));

  sprintf(s + strlen(s),
	  "triedtologin=%d\n"		  
	  "loggedin=%d\n"		  
	  "state=%d\n",
	  triedtologin,
	  loggedin,
	  state);

  sprintf(s + strlen(s),
	  "tablerootmode=%d\n",
	  tablerootmode);
  
  sprintf(s + strlen(s),
	  "kibitzing1=%d\n"		  
	  "kibitzingseat=%d\n"		  
	  "displayhandID=%s\n",
	  kibitzing1,
	  kibitzingseat,
	  CKNULL(displayhandID));

  sprintf(s + strlen(s),
	  "displayhandname=%s\n"	  
	  "statushandvul=%s\n"		  
	  "statushanddlr=%s\n"		  
	  "displayhandastate=%s\n"	  
	  "displayhandpstate=%s\n"	  
	  "displaypov=%d\n"		  
	  "dealercolumn=%d\n"		  
	  "displaycalls=%d\n"		  
	  "displayplays=%d\n"		  
	  "displayseated=%d\n"		  
	  "displayshowbuttonbar=%d\n",
	  CKNULL(displayhandname),
	  CKNULL(statushandvul),
	  CKNULL(statushanddlr),
	  CKNULL(displayhandastate),
	  CKNULL(displayhandpstate),
	  displaypov,
	  dealercolumn,
	  displaycalls,
	  displayplays,
	  displayseated,
	  displayshowbuttonbar);

  sprintf(s + strlen(s),
	  "statustolead=%s\n"		  
	  "statuscontract=%s\n"		  
	  "statusresult=%s\n"		  
	  "statusclaim=%s\n"		  
	  "displaycontract=%s\n"	  
	  "displaytrickswon=%s\n"	  
	  "leadercolumn=%d\n"		  
	  "declarercolumn=%d\n"		  
	  "displaytrump=%d\n"		  
	  "displayhandisover=%d\n"	  
	  "caughtup=%d\n",	  	  
	  CKNULL(statustolead),
	  CKNULL(statuscontract),
	  CKNULL(statusresult),
	  CKNULL(statusclaim),
	  CKNULL(displaycontract),
	  CKNULL(displaytrickswon),
	  leadercolumn,
	  declarercolumn,
	  displaytrump,
	  displayhandisover,
	  caughtup);

  return s;
}

/* Email a bug report. */
void mail_bug(char *s, char *filename, int line)
{
  char *t;
  static int inside = 0; /* prevent reentry */

  if (inside != 0) return;

  inside++;
  t = salloc(sizeof(char) * (strlen(s) + 20));
  sprintf(t, s, filename, line);
  fputs(t, stderr);
  fputs("\nEmailing bug report... ", stderr);
  TclDo3("mail_bug {", tclclean(t), "}");
  TclDo3("mail_bug {", destructivebracketclean(braceclean(dumpvars())), "}");
  fputs("done\n", stderr);
  free(t);
#ifdef LOGINSERVER
  TclDo("catch flush_permseenfiles");
#endif
  inside--;
}

/* Handles user request to email a bug report. */
void userbug(char *t)
{
  static int inside = 0; /* prevent reentry */

  if (inside != 0) return;

  inside++;
  status("Emailing bug report...");
/*  TclDo("update");  */   /* XXX  is this really needed??  explain if so  */
  TclDo3("mail_bug {", tclclean(t), "}");
  TclDo3("mail_bug {", destructivebracketclean(braceclean(dumpvars())), "}");
  status("Done reporting bug");
  inside--;
}

