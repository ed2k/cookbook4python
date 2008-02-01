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
#include "floatcmd.h"
#include "UI.h"
#include "comm.h"
#include "commandhash.h"
#include "br.h"
#include "deal.h"
#include "message.h"
#include "bug.h"

#define HAVE_SAVE_DIALOG (hasWindows && atob(TclDo("uplevel #0 {expr $tk_version >= 4.2}")))
#define GET_SAVE_FILE() TclDo("tk_getSaveFile")
#define HAVE_OPENFILE_DIALOG HAVE_SAVE_DIALOG
#define GET_OPENFILE() TclDo("tk_getOpenFile")

static int dobug(char *s);
static int dohelp(char *s);
static int docopyright(char *s);
static int dowarranty(char *s);
static int doreadme(char *s);
static int doconfusing(char *s);
static int doautodeal_now(char *s);
static int doautodeal(char *s);
static int dologin(char *s);
static int donewuser(char *s);
static int doopp(char *s);
static int dolho(char *s);
static int dorho(char *s);
static int doexplain(char *s);
static int doalert(char *s);
static int doredalert(char *s);
static int dokib1(char *s);
static int dospec(char *s);
static int doclub(char *s);
static int dodiamond(char *s);
static int doheart(char *s);
static int dospade(char *s);
static int dos(char *s);
static int dofollow(char *s);
static int dofollowsmall(char *s);
static int dohideAuction(char *s);
static int dobind(char *s);
static int domake(char *s);
static int dodown(char *s);
static int doclaim(char *s);
static int doaccept(char *s);
static int doreject(char *s);
static int doretract(char *s);
static int dodumpres(char *s);
static int dodump(char *s);
static int dofind(char *s);
static int dobeep(char *s);
static int donote(char *s);
static int donothing(char *s);
static int dodeal(char *s);
static int dotcl(char *s);
static int doscore(char *s);
static int dohost(char *s);
static int doihost(char *s);
static int doiupdatelocation(char *s);
static int doclose(char *s);
static int doquit(char *s);
static int dojoin(char *s);
static int dofont(char *s);
static int dotalkfont(char *s);
static int dotalkfontsize(char *s);
static int doijoin(char *s);
static int doserver(char *s);
static int dochildren(char *s);
static int doparent(char *s);
static int doip(char *s);
static int dopunt(char *s);
static int dounseat(char *s);
static int dochangepw(char *s);
static int doemailchange(char *s);
static int dowho(char *s);
static int dobid(char *s);
static int doplay(char *s);
static int dodisc(char *s);
static int dotables(char *s);
static int dostand(char *s);
static int dosouth(char *s);
static int donorth(char *s);
static int doeast(char *s);
static int dowest(char *s);
static int docomp(char *s);
static int dononcomp(char *s);
static int docards(char *s);
static int doprevious(char *s);
static int doreview(char *s);
static int doabout(char *s);
static int docompact(char *s);
static int doseparateTalk(char *s);
static int dogeometry(char *s);
static int doinullcommand(char *s);
static int doshowEntryLines(char *s);
static int dodeiconifyIfBeeped(char *s);
static int dobeepAtMyTurn(char *s);
#if !GUI_ONLY && !SILENT
static int dobottom(char *s);
static int doscroll(char *s);
#endif
static int dolast(char *s);
static int doexecute(char *s);
static int docc(char *s);
static int doccdump(char *s);
static int doccload(char *s);
static int doccsave(char *s);
/* static int doccgrep(char *s); */

#if RANDOMPLAY
static int dorand(char *s);
#endif

static int docomputer(char *s);

#ifdef LOGINSERVER
static int dologinserver(char *s);
#endif

#ifdef RESULTSERVER
static int doresultserver(char *s);
#endif

#ifdef PSEUDOMAILER
static int dopseudomailer(char *s);
#endif

/* this file implements most of the floater commands (e.g. login, bid, play) */

/*
   Functions starting with "do" as the first two letters of their name
   are called by docmd() with a string of arguments as its parameter.
   Frequently do....() just parses the argument string and calls
   another function that actually does the work.
*/

#define noargs(s, cmd) (argify(s), \
		   ((argc == 0) ? \
		    TRUE : \
		    (errormsg3("`", cmd, "' doesn't take arguments"), \
		     FALSE)))


#define eparsebool(s, b) \
     (parsebool((s), (b)) ? TRUE : \
      (errormsg3("Unable to parse `", s, "' as `yes' or `no'"), FALSE))

#define checktablehost() (tablehostmode ? TRUE : \
	  (errormsg("You must be the host of the table to do that"), FALSE))

#if RANDOMPLAY
fl_bool randomplay = FALSE;
#endif

fl_bool computerplay = FALSE;

/* set up bindings used by the "command" tcl command --- see commandhash.c */
void setupbindings(void)
{
  char strains[] = "CDHSN";
  char suits[] = "CDHS";
  char cards[] = "23456789TJQKA";
  int i, j;
  char s[20], t[20];

/*  cmdbind("bind", dobind);*/

#if DBG
  cmdbind("dr", dodumpres);
  cmdbind("dumpresults", dodumpres);
  cmdbind("dump", dodump);
/*  cmdbind("tcl", dotcl);*/ /* handled specially to avoid ; problems */
#endif
#ifndef SERVER
  cmdbind("stand", dostand);
  cmdbind("kib", dokib1);
  cmdbind("kibitz", dokib1);
  cmdbind("kibbitz", dokib1);
  cmdbind("bid", dobid);
  cmdbind("play", doplay);
  cmdbind("spec", dospec);
#if RANDOMPLAY
  cmdbind("randomplay", dorand);
#endif
  cmdbind("computerplay", docomputer);
  /*  cmdbind("help", dohelp);*/ /* handled specially to avoid ; problems */
  cmdbind("copyright", docopyright);
  cmdbind("warranty", dowarranty);
  cmdbind("about", doabout);
  cmdbind("confusing", doconfusing);
  cmdbind("find", dofind);
  cmdbind("whois", dofind);
  cmdbind("beep", dobeep);
  cmdbind("note", donote);
  cmdbind("bug", dobug);
  cmdbind("deal", dodeal);
  cmdbind("cards", docards);
  cmdbind("previous", doprevious);
  cmdbind("review", doreview);
  cmdbind("last", dolast);
  cmdbind("north", donorth);
  cmdbind("south", dosouth);
  cmdbind("east", doeast);
  cmdbind("west", dowest);
  cmdbind("follow", dofollow);
  cmdbind(".", dofollowsmall);
  cmdbind("c", doclub);
  cmdbind("d", dodiamond);
  cmdbind("h", doheart);
  cmdbind("s", dos); /* special case: dosouth or dospade */
  cmdbind("n", donorth);
  cmdbind("e", doeast);
  cmdbind("w", dowest);
  cmdbind("say", dosay);
  cmdbind("opp", doopp);
  cmdbind("lho", dolho);
  cmdbind("rho", dorho);
#if !GUI_ONLY && !SILENT
  if (!hasWindows) {
    cmdbind("bottom", dobottom);
    cmdbind("scroll", doscroll);
  }
#endif
  if (hasWindows) {
    cmdbind("font", dofont);
    cmdbind("talkfont", dotalkfont);
    cmdbind("talkfontsize", dotalkfontsize);
    cmdbind("compact", docompact);
    cmdbind("showEntryLines", doshowEntryLines);
    cmdbind("deiconifyIfBeeped", dodeiconifyIfBeeped);
    cmdbind("geometry", dogeometry);
    cmdbind("hideAuction", dohideAuction);
    cmdbind("separateTalk", doseparateTalk);
    cmdbind("inullcommand", doinullcommand);
  }
  cmdbind("beepAtMyTurn", dobeepAtMyTurn);
  cmdbind("explain", doexplain);
  cmdbind("alert", doalert);
  cmdbind("redalert", doredalert);
  cmdbind("score", doscore);
  cmdbind("who", dowho);
  cmdbind("join", dojoin);
  cmdbind("ijoin", doijoin);
  cmdbind("autodeal", doautodeal);
  cmdbind("autodeal_now", doautodeal_now);
  cmdbind("login", dologin);
  cmdbind("newuser", donewuser);
  cmdbind("password", dochangepw);
  cmdbind("emailchange", doemailchange);
  cmdbind("host", dohost);
  cmdbind("ihost", doihost);
  cmdbind("iupdatelocation", doiupdatelocation);
  cmdbind("close", doclose);
  cmdbind("server", doserver);
  cmdbind("claim", doclaim);
  cmdbind("make", domake);
  cmdbind("down", dodown);
  cmdbind("accept", doaccept);
  cmdbind("reject", doreject);
  cmdbind("retract", doretract);
  cmdbind("cc", (hasWindows) ? doccdump : docc);
  cmdbind("ccdump", doccdump);
  cmdbind("ccload", doccload);
  cmdbind("ccsave", doccsave);
  /*  cmdbind("ccgrep", doccgrep); */
  cmdbind("execute", doexecute);
#else
  cmdbind("iupdatelocation", donothing);
#endif /* ifndef SERVER */
#ifdef LOGINSERVER
  cmdbind("loginserver", dologinserver);
#endif
#ifdef RESULTSERVER
  cmdbind("resultserver", doresultserver);
#endif
#ifdef PSEUDOMAILER
  cmdbind("pseudomailer", dopseudomailer);
#endif
  cmdbind("children", dochildren);
  cmdbind("parent", doparent);
  cmdbind("punt", dopunt);
  cmdbind("kick", dopunt);
  cmdbind("boot", dopunt);
  cmdbind("unseat", dounseat);
  cmdbind("ip", doip);
  cmdbind("tables", dotables);
  cmdbind("quit", doquit);
  cmdbind("disconnect", dodisc);
  cmdbind("competitive", docomp);
  cmdbind("noncompetitive", dononcomp);
  /*  hashdump();*/

  for (i=1; i<=7; i++)
    for (j=0; j<strlen(strains); j++) {
      sprintf(t, "%c%c", i + '0', strains[j]);
      sprintf(s, "bid %s", t);
      stringbind(t, s);
      if (strains[j] == 'N') {
	strcat(t, "T");
	stringbind(t, s);
      }
    }

  stringbind("p", "bid p");
  stringbind("pa", "bid p");
  stringbind("pas", "bid p");
  stringbind("pass", "bid p");
  stringbind("xx", "bid xx");
  stringbind("redouble", "bid xx");
  stringbind("x", "bid x");
  stringbind("double", "bid x");

  /* bindings for SA, SK, SQ, ..., C4, C3, C2 */
  for (i=0; i<strlen(suits); i++)
    for (j=0; j<strlen(cards); j++) {
      sprintf(t, "%c%c", suits[i], cards[j]);
      sprintf(s, "play %s", t);
      stringbind(t, s);
    }

  /* bindings for A, K, Q, ..., 4, 3, 2 */
  for (j=0; j<strlen(cards); j++) {
    sprintf(t, "%c", cards[j]);
    sprintf(s, "follow %s", t);
    stringbind(t, s);
  }
}  


#define skipwhite(s) while (isspace(*s)) s++;

#define MAX_ARGS 10
#define MAX_ARG_LENGTH 100

static int argc;
static char arg[MAX_ARGS][MAX_ARG_LENGTH];

/*
   argify(s) takes s and breaks it up at whitespace boundaries.
   Results are stored in (globals) argc and arg.  s is not modified.
   (Normally s doesn't contain the name of the function being called,
   just its args.)
*/
static void argify(char *s)
{
  int i;

  argc = 0;
  skipwhite(s);
  while (*s != '\0') {
    /* copy an arg */
    for (i = 0; i < MAX_ARG_LENGTH && *s != '\0' && !isspace(*s); i++, s++)
      arg[argc][i] = *s;

    /* put '\0' at end */
    if (i == MAX_ARG_LENGTH) i--;
    arg[argc][i] = '\0';

    argc++;
    skipwhite(s);
  }    
}

static int docc(char *s)
{
#if !GUI_ONLY && !SILENT
  s = upcase(trim(s));
  argify(s);
  if (argc == 0)
    s = (seated && (myseat == East || myseat == West)) ? "EW" : "NS";
  else if (argc == 1) {
    if (streq(s, "N") || streq(s, "S") || streq(s, "NS") || streq(s, "SN"))
      s = "NS";
    else if (streq(s, "E") || streq(s, "W") || streq(s, "EW") || streq(s, "WE")) s = "EW";
    else goto bogus;
  } else goto bogus;
    
  beginedit(s, !(seated && isin(seattochar[myseat], s)));
  return TCL_OK;

bogus:
  errormsg("Invalid argument to `cc'---the one optional argument may be EW or NS");
  return TCL_OK;
#endif
}

#define isrange(s) TclDo2("regexp {^[0-9]*(-[0-9]*)?$} ", tclclean(s))

static int doccdump(char *s)
{
  char *range = "";

  s = upcase(trim(s));
  argify(s);
  if (argc == 0)
    s = (seated && (myseat == East || myseat == West)) ? "EW" : "NS";
  else if (argc == 1 || argc == 2) {
    if (argc == 2) {
      if (!isrange(range = arg[0])) goto bogus;
      s = arg[1];
    }
    if (streq(s, "N") || streq(s, "S") || streq(s, "NS") || streq(s, "SN"))
      s = "NS";
    else if (streq(s, "E") || streq(s, "W") || streq(s, "EW") || streq(s, "WE")) s = "EW";
    else if (argc == 1 && isrange(range = arg[0]))
      s = (seated && (myseat == East || myseat == West)) ? "EW" : "NS";
    else goto bogus;
  } else goto bogus;

  if (atob(TclDo3("if [catch {gset cclines(", s, ")} temp] {expr 1} {expr $temp < 1}")))
    status3("CC for ", s, " is empty.");
  else {
    if (strexists(range))
      status7("CC for ", s, " (", maybe_plural(TclDo("set temp"), "line"),
	      "), ", range, ":");
    else
      status5("CC for ", s, " (", maybe_plural(TclDo("set temp"), "line"),
	      "):");
    TclDo5("catch {ccdump ", s, " ", range, "}");
  }
  return TCL_OK;

bogus:
  errormsg("Invalid argument to `ccdump'---the optional arguments are a range and then EW or NS");
  return TCL_OK;

}

static int doccload(char *s)
{
  if (!seated) {
    errormsg("You must be seated to do `ccload'");
    return TCL_OK;
  }

  s = trim(s);
  argify(s);
  if (argc == 0 && HAVE_OPENFILE_DIALOG) {
    s = GET_OPENFILE();
    if (s[0] == '\0') return TCL_OK;
    s = TEMPCOPY(s);
  } else if (argc != 1) {
    errormsg("`ccload' expects one argument (the name of a text file)");
    return TCL_OK;
  }

  s = fileclean(s);
  if (TclOpen("ccloadfile", s, "r")) {
    char *t;

    TclDo2("beginnewcc ",
	   t = ((myseat == East || myseat == West) ? "EW" : "NS"));
    TclDo("fetchnewcc $ccloadfile;"
	  "set s [endnewcc]; close $ccloadfile;"
	  "if [string compare $s \"\"] {talkmsg $s}");
    selfmessage(makemsg2(CC_TO_HOST, t, TclDo2("ccstr ", t)));
    status3("You have loaded ", s, " as your convention card.");
  } else errormsg(TclDo("set ccloadfile"));
  return TCL_OK;
}

static int doccsave(char *s)
{
  const char *t;
  
  if (!seated) {
    errormsg("You must be seated to do `ccsave'");
    return TCL_OK;
  }

  s = trim(s);
  argify(s);
  if (argc == 0 && HAVE_SAVE_DIALOG) {
    s = GET_SAVE_FILE();
    if (s[0] == '\0') return TCL_OK;
    s = TEMPCOPY(s);
  } else if (argc != 1) {
    errormsg("`ccsave' expects one argument (the name of a text file)");
    return TCL_OK;
  }

  t = (myseat == East || myseat == West) ? "EW" : "NS";

  if (atob(TclDo3("if [catch {gset cclines(", t, ")} temp] {return 1} {expr $temp < 1}"))) {
    status3("CC for ", t, " is empty.");
    return TCL_OK;
  }
  
  s = fileclean(s);
  if (TclOpen("ccsavefile", s, "w")) {
    TclDo2("ccsave $ccsavefile ", t);
    t = TclDo3("gset cclines(", t, ")");
    status3("Convention card saved (", maybe_plural(t, "line"), ")");
  } else errormsg(TclDo("set ccsavefile"));
  
  return TCL_OK;
}


static int doexplain(char *s)
{
  if (seated) {
    while (isspace(*s)) s++;
    talkmsg3(myoutputname, "! ", s);
    broadcast(makemsg3(SAY_EXCEPT_PARD, myoutputname,
		       braceclean(s), itoa(myseat)));
  }
  else errormsg("You must be seated to use `explain' or alerting commands");
  return TCL_OK;
}

static int doalert(char *s)
{
  char *t;
  int ret;

  t = salloc(sizeof(char) * (30 + strlen(s)));
  sprintf(t, "explain Alert! %s", s);
  ret = docmd(t);
  free(t);
  return ret;
}

static int doautodeal(char *s)
{
  int i;

  argify(s);
  if (argc > 1) {
    errormsg("`autodeal' expects 0 or 1 arguments");
    return TCL_OK;
  } else if (argc == 0) {
    /* no arguments: toggle */
    i = atoi(TclGet("autonewdeal_seconds"));
    if (i < 0) i = atoi(TclGet("autonewdeal_default"));
    else i = -1;
  } else {
    /* one argument */
    i = atoi(arg[0]);
    TclSet("autonewdeal_default", itoa(i));
  }

  TclSet("autonewdeal_seconds", itoa(i));
  if (i < 0) status("Autodeal disabled");
  else status3("Autodeal enabled (", itoa(i), " second delay)");
  return TCL_OK;
}

static int doautodeal_now(char *s)
{
  if (noargs(s, "INTERNAL -- autodeal_now")) {
    TclSet("showerrors", "0");
    dodeal("");
    TclSet("showerrors", "1");
  }
  return TCL_OK;
}

static int doredalert(char *s)
{
  char *t;
  int ret;

  t = salloc(sizeof(char) * (30 + strlen(s)));
  sprintf(t, "explain Red Alert! %s", s);
  ret = docmd(t);
  free(t);
  return ret;
}

static int donote(char *s)
{
  if (checktablehost()) describetable(trim(s));
  return TCL_OK;
}

static int donothing(char *s)
{
  return TCL_OK;
}

static int dobeep(char *s)
{
  commbeep(s);
  return TCL_OK;
}

static int dofind(char *s)
{
  s = destructivebracketclean(trim(tclclean(s)));
  getfind(s);
  return TCL_OK;
}

static int dohelp(char *s)
{
  help(trim(s));
  return TCL_OK;
}

int dosay(char *s)
{
  extern fl_bool spec; /* from UI.c */

  while (isspace(*s)) s++;
  if (*s != '\0') {
    talkmsg3(myoutputname, (spec ? "% " : ": "), s);
    broadcast(makemsg2(spec ? SAY_TO_SPEC : SAY, myoutputname, braceclean(s)));
  }
  return TCL_OK;
}

static int dolho(char *s)
{
  if (seated) {
    if (playinghearts) errormsg("Sorry, `lho' is disabled when playing hearts.");
    else {
      while (isspace(*s)) s++;
      if (*s != '\0') {
	talkmsg3(myoutputname, "<- ", s);
	broadcast(makemsg3(SAY_TO_OPP, myoutputname, braceclean(s),
			   itoa(leftyseat(myseat))));
      }
    }
  } else errormsg("You must be seated to use `lho'");
  return TCL_OK;
}

static int dorho(char *s)
{
  if (seated) {
    if (playinghearts) errormsg("Sorry, `rho' is disabled when playing hearts.");
    else {
      while (isspace(*s)) s++;
      if (*s != '\0') {
	talkmsg3(myoutputname, "-> ", s);
	broadcast(makemsg3(SAY_TO_OPP, myoutputname, braceclean(s),
			   itoa(rightyseat(myseat))));
      }
    }
  } else errormsg("You must be seated to use `rho'");
  return TCL_OK;
}

static int doopp(char *s)
{
  if (seated) {
    if (playinghearts) errormsg("Sorry, `opp' is disabled when playing hearts.");
    else {
      while (isspace(*s)) s++;
      if (*s != '\0') {
	talkmsg3(myoutputname, "= ", s);
	broadcast(makemsg4(SAY_TO_OPP, myoutputname, braceclean(s),
			   itoa(leftyseat(myseat)), itoa(rightyseat(myseat))));
      }
    }
  } else errormsg("You must be seated to use `opp'");
  return TCL_OK;
}

static int dobid(char *s)
{
  argify(s);
  if (argc != 1) {
    errormsg("`bid' command requires one argument");
    return TCL_OK;
  }

  if (!myturntobid()) {
    errormsg("It isn't your turn to bid");
    return TCL_OK;
  }

  executebid(arg[0]);
  return TCL_OK;
}

static int doretract(char *s)
{
  if (noargs(s, "retract")) retractclaim();
  return TCL_OK;
}

static int doaccept(char *s)
{
  if (noargs(s, "accept")) acceptclaim();
  return TCL_OK;
}

static int doreject(char *s)
{
  if (noargs(s, "reject")) rejectclaim();
  return TCL_OK;
}

static int domake(char *s)
{
  argify(s);
  if (argc == 0) make("");
  else if (argc == 1) make(arg[0]);
  else errormsg("Too many arguments to `make'");
  return TCL_OK;
}

static int dodown(char *s)
{
  argify(s);
  if (argc == 1) down(arg[0]);
  else errormsg("`down' command requires one argument");
  return TCL_OK;
}

static int doclaim(char *s)
{
  argify(s);
  if (argc == 0) claim("");
  else if (argc == 1) claim(arg[0]);
  else errormsg("Too many arguments to `claim'");
  return TCL_OK;
}

static int doplay(char *s)
{
  argify(s);
  if (argc != 1) {
    errormsg("`play' command requires one argument");
    return TCL_OK;
  }

  if (!myturntoplay()) {
    errormsg("It isn't your turn to play");
    return TCL_OK;
  }

  executeplay(arg[0]);
  return TCL_OK;
}

static int dobind(char *s)
{
  char *t;
  
  for (t = s; iscmd(*t); t++);
  if (isspace(*t) || *t == '\0') {
    if (*t != '\0') {
      *t = '\0';
      do { ++t; } while (isspace(*t));
    }
    if ((strncasecmp(s, "bind", 4) != 0) &&
	(strncasecmp(s, "keybind", 7) != 0)) {
      stringbind(s, t);
#if DBG
      if (hasWindows)
	printf("bound %s to %s\n", s, t);
#endif
    } else talkmsg3("ERROR: Can't bind `", s,
		    "' because it starts with special binding keyword");
  } else talkmsg("ERROR: Keyword to bind followed by cruft");
  return TCL_OK;
}

static void dologin2(char *n, char *p, fl_bool *new, char **name)
{
  if (!hasWindows) {
    char *s, *t;
  
    status("New users should use `newuser.'");
    status("Use a blank name and password to abort.");
    text2fieldinput("   Name ", n, "  'Word ", p, &s, &t);
    *name = markgarbage(salloc(sizeof(char) * (strlen(s) + strlen(t) + 5)));
    if (strcaseeq(s, "newuser")) {
      donewuser("");
      strcpy(*name, "\\");
    } else {
      sprintf(*name, "%s\\%s", s, t);
      *new = FALSE;
    }
  } else {
    char *t;

    TclDo3("gset loginname {", destructivebracketclean(n), "}");
    TclDo3("gset loginpassword {", destructivebracketclean(p), "}");
    t = TclDo("Floater_login");
    *new = (*t == 'N');
    *name = TEMPCOPY(t + 1);
  }
}

static fl_bool validemail(char *s)
{
  if (isin('@', s)) return TRUE;
  errormsg("Please try again with a full email address, such as qx@slat.grat.com");
  return FALSE;
}

static int donewuser(char *s)
{
  if (noargs(s, "newuser")) {
    if (!hasWindows) {
      char *a, *b;

      status("Enter your desired name and your email address.");
      status("Use a blank name to abort.");
      text2fieldinput("   Name ", "", "  Email ", "", &a, &b);
      if (strlen(trim(a)) > 0 && validemail(b)) login(a, b, TRUE);
    } else {
      char *name, *password;
      fl_bool new;

      TclDo("gset newbie 1");
      dologin2("", "", &new, &name);
      password = strchr(name, '\\');
      *password++ = '\0';
    
      if (!new || validemail(password)) login(name, password, new);
    }
  }
  return TCL_OK;
}

static int dologin(char *s)
{
  char *name, *password, *t;
  fl_bool new;

  argify(s);
  if (argc == 0) {
    dologin2("", "", &new, &name);
    password = strchr(name, '\\');
    *password = '\0';
    password++;
  } else if (argc == 1) {
    dologin2(tclclean(arg[0]), "", &new, &name);
    password = strchr(name, '\\');
    *password = '\0';
    password++;
  } else {
    /* assume last arg is password; rest are name (name may have spaces) */
    /* e.g. "Joe Blow pencil" => name is "Joe Blow", password is "pencil" */

    new = FALSE;

    /* just in case s is not modifiable, copy it */
    s = TEMPCOPY(s);

    /* place a `\0' just before start of last argument in s */
    for (t = s + strlen(s) - 1; !isspace(*t) && t > s; t--);
    while (isspace(*t)) t--;
    t[1] = '\0';
    password = t + 2;
    name = s;
    /* status4("name=", name, "; password=", password); */
  }

  if ((strlen(name) > 0) && (!new || validemail(password)))
    login(name, password, new);
  
  return TCL_OK;
}  

static int dochangepw(char *s)
{
  if (noargs(s, "password")) {
    if (!hasWindows) {
      extern fl_bool loggedin;
      char *a, *b;

      if (!loggedin) {
	errormsg("You must be logged in to change your password");
	return TCL_OK;
      }
      status2("Enter current (\"old\") and new passwords for ", myoutputname);
      text2fieldinput("Current ", "", "    New ", "", &a, &b);
      changepw(myoutputname, a, b);
    } else {
      char *name, *oldpassword, *newpassword;

      TclDo3("gset changepwname {", myoutputname, "}");
      TclDo("gset oldpassword {}");
      name = TclDo("Floater_changepw");
      if (strlen(name) > 2) {
	oldpassword = strchr(name, '\\');
	*oldpassword++ = '\0';
	newpassword = strchr(oldpassword, '\\');
	*newpassword++ = '\0';
    
	changepw(name, oldpassword, newpassword);
      }
    }
  }
  return TCL_OK;
}  

static int doemailchange(char *s)
{
  extern fl_bool loggedin;

  argify(s);
  if (!loggedin)
    errormsg("You must be logged in to change your email address");
  else if (argc != 1)
    errormsg("`emailchange' expects one argument (your new email address)");
  else {
    s = trim(s);
    if (validemail(s)) changeemail(myoutputname, s);
  }
  return TCL_OK;
}  

static int dotalkfont(char *s)
{
  s = destructivebracketclean(trim(bracetospace(s)));
  TclDo3("updatetalkfontfam {", s, "}");
  return TCL_OK;
}

static int dotalkfontsize(char *s)
{
  s = destructivebracketclean(trim(bracetospace(s)));
  TclDo3("updatetalkfontsize {", s, "}");
  return TCL_OK;
}

static int dofont(char *s)
{
  fl_bool bogus = TRUE;
  char *t[3] = {"small", "medium", "large"};
  int i;

  s = trim(s);
  argify(s);
  if (argc == 1) {
    for (i = 0; i < 3; i++)
      if (strcaseeq(t[i], s) || (s[1] == '\0' && tolower(s[0]) == t[i][0])) {
	char z[200];
	bogus = FALSE;

	sprintf(z, "uplevel #0 {setfontsize %c; updateallfonts}", t[i][0]);
	TclDo(z);
	sprintf(z, "gset radiofont %c%s", toupper(t[i][0]), &(t[i][1]));
	TclDo(z);
	break;
      }
  }
  if (bogus) errormsg("`font' expects one argument: small, medium, or large");
  return TCL_OK;
}

static int dodump(char *s)
{
  if (noargs(s, "dump")) hashdump();
  return TCL_OK;
}

static int docards(char *s)
{
  if (noargs(s, "cards"))
    if (seated)
      showinitialcards(-1);
    else
      errormsg("You must be seated to view your initial holding.");
  return TCL_OK;
}

static int doprevious(char *s)
{
  if (noargs(s, "previous")) showprevioushand();
  return TCL_OK;
}

static int doreview(char *s)
{
  if (noargs(s, "review")) reviewauction();
  return TCL_OK;
}

static int dolast(char *s)
{
  if (noargs(s, "last")) showlasttrick();
  return TCL_OK;
}

static int dodumpres(char *s)
{
  if (noargs(s, "dumpresults")) dumpresults();
  return TCL_OK;
}

/* try to kib1 -- kluged by Syela 040223*/
static int dokib1(char *s)
{
  extern char *curhandID;
  extern fl_bool kibitzing1; /* from UI.c */
  extern int kibitzingseat; /* from UI.c */
  fl_bool waskibbing = kibitzing1;

/* to prevent hopping, once someone kib1s, they can't change their kibseat
until end-of-hand (when it is reset to Noseat if they've sat or kibbed 0), 
and they can't sit down mid-hand except in the seat they were kibbing */

  argify(s);
  if (!argc) {
    if (kibitzing1) UIkib1(Noseat);
    else dostand(s); /* because "kib <noarg>" was geoff's syntax for "stand") */
  } else if (argc == 1) { 
    if (streq(arg[0], "N") || streq(arg[0],"n")) UIkib1(North);
    else if (streq(arg[0], "E") || streq(arg[0],"e")) UIkib1(East);
    else if (streq(arg[0], "S") || streq(arg[0],"s")) UIkib1(South);
    else if (streq(arg[0], "W") || streq(arg[0],"w")) UIkib1(West);
    else if (streq(arg[0], "0")) UIkib1(Noseat);
    else { /* not hardcoding seat order, so namematch() seems unsuitable */
      extern char *northname, *southname, *eastname, *westname;
      size_t l;  /* sticking vars in this block in case someone rewrites it */
      int pmatch[4];
      l = strlen(arg[0]);
      pmatch[North] = (northname != NULL && !strncasecmp(northname, arg[0], l));
      pmatch[East] = (eastname != NULL && !strncasecmp(eastname, arg[0], l));
      pmatch[South] = (southname != NULL && !strncasecmp(southname, arg[0], l));
      pmatch[West] = (westname != NULL && !strncasecmp(westname, arg[0], l));
      if (pmatch[0] && !(pmatch[1] || pmatch[2] || pmatch[3])) UIkib1(0);
      else if (pmatch[1] && !(pmatch[0] || pmatch[2] || pmatch[3])) UIkib1(1);
      else if (pmatch[2] && !(pmatch[0] || pmatch[1] || pmatch[3])) UIkib1(2);
      else if (pmatch[3] && !(pmatch[0] || pmatch[1] || pmatch[2])) UIkib1(3);
      else errormsg("Kibitz whom?  (w/n/e/s/0 or playername)");
    }
  } else errormsg("Kibitz takes only one argument (w/n/e/s/0 or playername)");

  if (kibitzing1)
    broadcast(makemsg3(KIB1, myname, seattostring[kibitzingseat], curhandID));

  if (!kibitzing1 && waskibbing)
    broadcast(makemsg3(KIB1, myname, "no one", curhandID));

  return TCL_OK;
}

/* try to enter spectator mode */
static int dospec(char *s)
{
  extern char *curhandID;
  extern fl_bool spec; /* from UI.c */

  if (noargs(s, "spec")) {
    if (spec)
      errormsg("You are already a spectator");
    else {
      UIspec();

      /* if it worked, announce to the world we are in spec */
      if (spec) {
	make_spec(myname, curhandID);
	broadcast(makemsg2(SPEC, myname, curhandID));
      }
    }
  }
  return TCL_OK;
}

#if RANDOMPLAY
static int dorand(char *s)
{
  if (noargs(s, "randomplay")) randomplay = !randomplay;
  status(randomplay ? "Randomplay is on" : "Randomplay is off");
  return TCL_OK;
}
#endif

static int docomputer(char *s)
{
  if (noargs(s, "computerplay")) computerplay = !computerplay;
  status(computerplay ? "Computerplay is on" : "Computerplay is off");
  return TCL_OK;
}

static int dochildren(char *s)
{
  if (noargs(s, "children")) showchildren();
  return TCL_OK;
}

static int doparent(char *s)
{
  if (noargs(s, "parent")) showparent();
  return TCL_OK;
}

static int doip(char *s)
{
  if (noargs(s, "ip")) showip();
  return TCL_OK;
}

static int dopunt(char *s)
{
  argify(s);
  if (argc != 1) {
    errormsg("punt takes one arg ('punt Bozo')");
    return TCL_OK;
  }
  punt(arg[0]);
  return TCL_OK;
}

static int dounseat(char *s)
{
  argify(s);
  if (argc != 1) {
    errormsg("unseat takes one arg ('unseat Bozo')");
    return TCL_OK;
  }
  unseat(arg[0]);
  return TCL_OK;
}

static int doclose(char *s)
{
  if (checktablehost()) dodisc("");
  else errormsg("You may use `disconnect' to disconnect yourself from the table");
  return TCL_OK;
}

static int doquit(char *s)
{
  if (noargs(s, "quit")) {
    if (hasWindows) TclDo("after 1000 {catch {wm iconify .}}");
    UIpatientcursor();
    status("Quitting...");
    sever_all_communication();
#ifndef SERVER
    commseens(0);
#endif
    if (!hasWindows) endcurses();
    TclDo("after 500; exit");
    exit(0);
  }
  return TCL_OK;
}

static int dodisc(char *s)
{
  if (noargs(s, "disconnect")) {
    sever_all_communication();
    status("You are disconnected");
  }
  return TCL_OK;
}

static int doexecute(char *s)
{
  s = trim(s);
  argify(s);
  if (argc == 0 && HAVE_OPENFILE_DIALOG) {
    s = GET_OPENFILE();
    if (s[0] == '\0') return TCL_OK;
    s = TEMPCOPY(s);
  } else if (argc != 1) {
    errormsg("`execute' expects one argument (the name of a file)");
    return TCL_OK;
  }

  s = fileclean(s);
  if (TclOpen("Floater_execute_file", s, "r"))
    TclDo("Floater_execute $Floater_execute_file");
  else errormsg(TclDo("set Floater_execute_file"));
  return TCL_OK;
}

/* parse s as xxx.yyy.zzz.www:prt#, where xxx, yyy, zzz, www, prt# are ints */
/* (there may be white space at the beginning or end, but not in the middle) */
/* alternatively, if s is just :prt#, the IP address is assumed to be local */
/* put results in addr and port */
/* return 1 on success, 0 on failure */
int parseIPaddr(char *s, char *addr, char *port)
{
  char *t;

  if (strlen(s) > MAXADDRLEN) return 0;

  t = strchr(s, ':');
  if (t == NULL) return 0;
  *t++ = '\0';
  /* parse port */
  while (isdigit(*t)) *port++ = *t++;
  while (isspace(*t)) t++; /* skip space at the end */
  if (*t == '\0') *port++ = *t++; else return 0;
  
  /* parse IP address */
  while (isspace(*s)) s++; /* skip space at the beginning */
  if (*s == '\0') {
    /* nothing before colon: assume localhost */
    strcpy(addr, localIPaddr);
    return 1;
  }
  while (isdigit(*s)) *addr++ = *s++;
  if (*s == '.') *addr++ = *s++; else return 0;
  while (isdigit(*s)) *addr++ = *s++;
  if (*s == '.') *addr++ = *s++; else return 0;
  while (isdigit(*s)) *addr++ = *s++;
  if (*s == '.') *addr++ = *s++; else return 0;
  while (isdigit(*s)) *addr++ = *s++;
  if (*s == '\0') *addr++ = *s++; else return 0;

  return 1;
}

#ifndef LOGINSERVER
static int dojoin(char *s)
{
  char addr[MAXADDRLEN], port[MAXPORTLEN], *original;
  extern fl_bool triedtologin;
  extern char tablehostname[];

  if (!DBG && !triedtologin) {
    errormsg(hasWindows ?
	     "Please try to log in first.  Use `login.'" :
	     "Please try to log in first.  Use `login' or `newuser.'");
    return TCL_OK;
  }

  original = s = destructivebracketclean(s);
  argify(s);
  if (argc < 1) {
    errormsg("The command `join' takes one argument");
    return TCL_OK;
  }
  s = trim(s);
  if (!parseIPaddr(s, addr, port) && !tableaddr(s, addr, port)) {
/*    errormsg("Invalid argument to join (should be the name of a table or of the form 128.110.38.14:8765)");*/
    if (isin(':', s))
      errormsg3("Unable to parse `", s, "' as address:port");
    else
      errormsg3("No such table (`", s, "')");
    return TCL_OK;
  }

  if (strexists(tablehostname) && strcaseeq(outputname(tablehostname), s)) {
    errormsg3("You are already at ", s, "'s table");
  } else {
    sever_all_communication();
    TclDo3("command {ijoin ", original, "}");
  }

  return TCL_OK;
}

static int doijoin(char *s)
{
  char addr[MAXADDRLEN], port[MAXPORTLEN];

  argify(s);
  if (argc < 1) {
    errormsg("The command `ijoin' takes one argument");
    return TCL_OK;
  }
  s = trim(s);
  if (!parseIPaddr(s, addr, port) && !tableaddr(s, addr, port)) {
    if (isin(':', s))
      errormsg3("Unable to parse `", s, "' as address:port");
    else
      errormsg3("No such table (`", s, "')");
    return TCL_OK;
  }

  join(addr, port, KIBITZERs, NUMTRIES);

  return TCL_OK;
}

static int doserver(char *s)
{
  argify(s);
  if (argc == 0) {
    talkmsg3("Current server is ", loginserveraddr(), ".");
    return TCL_OK;
  }


  if (argc == 1) {
    setserver(arg[0]);
    talkmsg3("Server changed to ", loginserveraddr(), ".");
    return TCL_OK;
  }

  errormsg("The command `server' takes 0 or 1 arguments");
  
  return TCL_OK;
}
#endif

#ifndef SERVER
static int dohost(char *s)
{
  char addr[MAXADDRLEN], port[MAXPORTLEN];
  fl_bool canhost;
  extern fl_bool triedtologin, loggedin;

  if (!DBG && !triedtologin) {
    errormsg(hasWindows ?
	     "Please try to log in first.  Use `login.'" :
	     "Please try to log in first.  Use `login' or `newuser.'");
    return TCL_OK;
  }

  canhost = atob(TclGet("test_connection_succeeded"));
  if (!canhost) {
    if (!loggedin)
      errormsg("Sorry, you should log in before hosting a table.");
    else
      errormsg("When you login, a test connection from the Floater server to your computer is attempted.  It has not yet succeeded.  Because others must be able to connect to you when you host a table, you will not be allowed to host a table now.  The most likely reason the test connection failed is that there is a firewall or NAT gateway between the Floater server and your computer.  However, if you logged in within the last minute, it is possible that the test connection will succeed momentarily, at which point you will be able to host a table.");
    return TCL_OK;
  }

  destructivebracketclean(s);
  argify(s);
  if (argc != 0) {
    while (isspace(*s)) s++; /* skip space at the beginning */
    if (!parseIPaddr(s, addr, port) && !tableaddr(s, addr, port)) {
      if (isin(':', s))
	errormsg3("Unable to parse `", s, "' as address:port");
      else
	errormsg3("No such table (`", s, "')");
      return TCL_OK;
    }
  }

  if (tablehostmode) {
    errormsg("You already are hosting a table.");
    errormsg("(To host a new table, do `close; host'.)");
    return TCL_OK;
  }

  sever_all_communication();
  TclDo3("command {ihost ", s, "}");
  return TCL_OK;
}

static int doihost(char *s)
{
  char addr[MAXADDRLEN], port[MAXPORTLEN];

  argify(s);
  if (argc == 0) tablehost(NULL, NULL);
  else {
    while (isspace(*s)) s++; /* skip space at the beginning */
    if (!parseIPaddr(s, addr, port) && !tableaddr(s, addr, port)) {
      if (isin(':', s))
	errormsg3("Unable to parse `", s, "' as address:port");
      else
	errormsg3("No such table (`", s, "')");
      return TCL_OK;
    }
    tablehost(addr, port);
  }
  return TCL_OK;
}

static int doiupdatelocation(char *s)
{
  if (noargs(s, "iupdatelocation")) updatelocation();
  return TCL_OK;
}
#endif /* ifndef SERVER */

#ifdef LOGINSERVER
static int dologinserver(char *s)
{
  char addr[MAXADDRLEN], port[MAXPORTLEN];

  argify(s);
  if (argc == 0) loginserver(TclDo("gset loginserverport"));
  else if (argc == 1) {
    char *t;

    t = markgarbage(salloc(strlen(s) + 3));
    sprintf(t, ":%s", s); 
    if (!parseIPaddr(t, addr, port)) {
      errormsg("Invalid argument to loginserver (should be a port number)");
      return TCL_OK;
    }

    loginserver(port);
  }
  else errormsg("The command `loginserver' takes 0 or 1 arguments");
  return TCL_OK;
}
#endif

#ifdef PSEUDOMAILER
static int dopseudomailer(char *s)
{
  char addr[MAXADDRLEN], port[MAXPORTLEN];

  argify(s);
  if (argc == 0) pseudomailer(TclDo("gset pseudomailport"));
  else if (argc == 1) {
    char *t;

    t = markgarbage(salloc(strlen(s) + 3));
    sprintf(t, ":%s", s); 
    if (!parseIPaddr(t, addr, port)) {
      errormsg("Invalid argument to pseudomailer (should be a port number)");
      return TCL_OK;
    }

    pseudomailer(port);
  }
  else errormsg("The command `pseudomailer' takes 0 or 1 arguments");
  return TCL_OK;
}
#endif

#ifdef RESULTSERVER
static int doresultserver(char *s)
{
  char addr[MAXADDRLEN], port[MAXPORTLEN];

  argify(s);
  if (argc == 0) resultserver(TclDo("gset resultserverport"));
  else if (argc == 1) {
    char *t;

    t = markgarbage(salloc(strlen(s) + 3));
    sprintf(t, ":%s", s); 
    if (!parseIPaddr(t, addr, port)) {
      errormsg("Invalid argument to resultserver (should be a port number)");
      return TCL_OK;
    }

    resultserver(port);
  }
  else errormsg("The command `resultserver' takes 0 or 1 arguments");
  return TCL_OK;
}
#endif

/* for debugging only */
static int dotcl(char *s)
{
  status2("TCL: ", TclDoIgnoreErrors(s));
  return TCL_OK;
}

static int dowho(char *s)
{
  if (noargs(s, "who")) listwho();
  return TCL_OK;
}

static int doscore(char *s)
{
  char *n;

  if (!checktablehost()) return TCL_OK;
  argify(s);
  if (argc != 1) {
    setscoringmethod("crap"); /* this generates an appropriate error message */
    return TCL_OK;
  }
  if ((n = setscoringmethod(upcase(arg[0]))) != NULL) {
    status3("Scoring changed to ", n, " starting next hand.");
    UItablehost();
  }
  return TCL_OK;
}

static int dotables(char *s)
{
  if (noargs(s, "tables")) gettables();
  return TCL_OK;
}

static int dosit(char *s, int which)
{
  if (noargs(s, "sit")) requestseat(which);
  return TCL_OK;
}

static int doclub(char *s)
{
  if (noargs(s, "c (club)")) {
    if (!seated) errormsg("You can't play a club when you aren't seated");
    else if (!myturntoplay()) errormsg("It isn't your turn to play");
    else playsmall('c');
  }
  return TCL_OK;
}

static int dodiamond(char *s)
{
  if (noargs(s, "d (diamond)")) {
    if (!seated) errormsg("You can't play a diamond when you aren't seated");
    else if (!myturntoplay()) errormsg("It isn't your turn to play");
    else playsmall('d');
  }
  return TCL_OK;
}

static int doheart(char *s)
{
  if (noargs(s, "h (heart)")) {
    if (!seated) errormsg("You can't play a heart when you aren't seated");
    else if (!myturntoplay()) errormsg("It isn't your turn to play");
    else playsmall('h');
  }
  return TCL_OK;
}

static int dospade(char *s)
{
  if (noargs(s, "(s) spade")) {
    if (!seated) errormsg("You can't play a spade when you aren't seated");
    else if (!myturntoplay()) errormsg("It isn't your turn to play");
    else playsmall('s');
  }
  return TCL_OK;
}

static int dos(char *s)
{
  argify(s);
  if (argc > 0 || unoccupied('S')) return dosouth(s);
  else return dospade(s);
}


static int dofollowsmall(char *s)
{
  if (noargs(s, ".")) {
    if (!seated) errormsg("You can't follow small when you aren't seated");
    else if (!myturntoplay()) errormsg("It isn't your turn to play");
    else followsmall();
  }
  return TCL_OK;
}

static int doabout(char *s)
{
  if (noargs(s, "about")) TclDo("catch {about}");
  return TCL_OK;
}

static int dodeiconifyIfBeeped(char *s)
{
  fl_bool b;

  argify(s);
  if (argc == 1 && eparsebool(s, &b)) TclSet("deiconifyIfBeeped_", itoa(b));
  if (argc >= 2) 
    errormsg("Too many arguments to deiconifyIfBeeped.");
  else TclDo2("deiconifyIfBeeped ", (argc == 0) ? "1" : "0");

  return TCL_OK;
}

static int dobeepAtMyTurn(char *s)
{
  fl_bool b;

  argify(s);
  if (argc == 1 && eparsebool(s, &b)) TclSet("beepAtMyTurn_", itoa(b));
  if (argc >= 2) 
    errormsg("Too many arguments to beepAtMyTurn.");
  else TclDo2("beepAtMyTurn ", (argc == 0) ? "1" : "0");

  return TCL_OK;
}

static int doseparateTalk(char *s)
{
  fl_bool b;

  argify(s);
  if (argc == 1 && eparsebool(s, &b)) TclSet("separateTalk_", itoa(b));
  if (argc >= 2) 
    errormsg("Too many arguments to separateTalk.");
  else TclDo2("separateTalk ", (argc == 0) ? "1" : "0");

  return TCL_OK;
}

static int doinullcommand(char *s)
{
  s = trim(s);
  argify(s);
  if (argc == 1 && (streq(s, "c") || streq(s, "t")))
    insert_default_action(streq(s, "t"),
			  streq(s, "t") ? UIinsertTalk : UIinsertCmd);
  return TCL_OK;
}

static int dogeometry(char *s)
{
  argify(s);
  if (argc == 0) {
    status2("Geometry: ", TclDo("wm geometry ."));
  } else if (argc == 1) {
    char *t = TclDoIgnoreErrors(TEMPCAT("wm geometry . ", arg[0]));
    if (strlen(t) > 1) errormsg(t);
  } else {
    errormsg("`geometry' expects at most one argument (e.g., 604x836+110+6)");
  }

  return TCL_OK;
}

static int docompact(char *s)
{
  fl_bool b;

  argify(s);

  if(argc == 0)
    errormsg("compact needs an argument");
  else if (argc == 1) {
    if(eparsebool(s, &b))
      TclDo2("gui_compact ", itoa(b));
  }
  else
    errormsg("Too many arguments to compact.");


  return TCL_OK;
}


static int doshowEntryLines(char *s)
{
  fl_bool b;
  char *argToUse;
  
  argify(s);
  if (argc == 1) {
       argToUse = arg[0];
  }
  else {
       argToUse = "help";
  }
  

  TclDo2("showEntryLines ", argToUse);

  return TCL_OK;
}


#if !GUI_ONLY && !SILENT
static int dobottom(char *s)
{
  if (noargs(s, "bottom")) turn_off_scrolllock();
  return TCL_OK;
}

static int doscroll(char *s)
{
  argify(s);
  if (argc != 1)
    errormsg("`scroll' expects one argument");
  else if (!isnumeral(arg[0]))
    errormsg("`scroll' should be followed by the number of lines to scroll"); 
  else
    floaterscroll(atoi(arg[0]));

  return TCL_OK;
}
#endif /* !GUI_ONLY && !SILENT */

static int dofollow(char *s)
{
  argify(s);
  if (argc != 1) errormsg("`follow' expects one argument");
  else {
    if (!seated) errormsg("You can't play cards when you aren't seated");
    else if (!myturntoplay()) errormsg("It isn't your turn to play");
    else follow(s);
  }
  return TCL_OK;
}

static int dohideAuction(char *s)
{
  argify(s);
  if (argc != 1) errormsg("`hideAuction' expects one argument");
  else {
    if (isnumeralf(arg[0])) {
      extern float auction_hide_time;
      float n = -1.0;

      if (arg[0][0] != '-' && strlen(arg[0]) < 4) sscanf(arg[0], "%f", &n);

      if (n == -1.0) status("Auction will be hidden after trick one");
      else status3("Auction will be hidden after trick one or ",
		   dtoa((double)n), " seconds after the final pass");
      auction_hide_time = n;
      TclSet("auction_hide_time", dtoa((double)n));
    } else {
      errormsg("`hideAuction' expects a numeral as its argument");
    }
  }
  return TCL_OK;
}

static int dobug(char *s)
{
  UIpatientcursor();
  userbug(s);
  UInormalcursor();
  return TCL_OK;
}

static int dodeal(char *s)
{
  static int lastdeal = 0;
  if (noargs(s, "deal") && checktablehost()
      && (floaterclock() - lastdeal > 2)) {
    TclSet("autodealing", "0");
    if (competitive != new_competitive) {
      competitive = new_competitive;
      redescribetable();
    }
    generatedeal();
    lastdeal = floaterclock();
  }

  return TCL_OK;
}

static int docomp(char *s)
{
  if (noargs(s, "competitive") && checktablehost()) {
    status(competitive ? "Competitive mode is on and will remain on" :
	   "Competitive mode will start next hand");
    new_competitive = TRUE;
    UItablehost();
  }
  return TCL_OK;
}

static int dononcomp(char *s)
{
  if (noargs(s, "noncompetitive") && checktablehost()) {
    status(!competitive ? "Competitive mode is off and will remain off" :
	   "Non-competitive mode will start next hand");
    new_competitive = FALSE;
    UItablehost();
  }
  return TCL_OK;
}

static int dostand(char *s)
{
  return dosit(s, Noseat);
}

static int donorth(char *s)
{
  return dosit(s, North);
}

static int dosouth(char *s)
{
  return dosit(s, South);
}

static int doeast(char *s)
{
  return dosit(s, East);
}

static int dowest(char *s)
{
  return dosit(s, West);
}

#define display_text(s) TclDo5("display_text {", (s), "} [gset ", (s), "]")


static int doconfusing(char *s)
{
  if (noargs(s, "confusing")) {
    display_text("Confusing");
  }
  return TCL_OK;
}

static int docopyright(char *s)
{
  if (noargs(s, "copyright")) {
    display_text("Copyright");
  }
  return TCL_OK;
}

static int dowarranty(char *s)
{
  if (noargs(s, "warranty")) {
    display_text("Copyright");
  }
  return TCL_OK;
}


/*****************************************************************************
Parsing of user commands
*****************************************************************************/

/* skip over characters at start of s that are part of the command */
static char *cmdskip(char *s)
{
  while (iscmd(*s)) s++;
  return s;
}

/* used by help() to decipher a user command */
/* if this function returns NULL, help() assumes nothing more needs doing */
/* otherwise, it takes the return value of this function and tries to look it
   up in its list of commands with handwritten descriptions */
/* [ note: code is a modified version of docmd() ] */
char *helpstring_to_command(char *s)
{
  int i;
  int (*h)();

  /* first, check for the commands that want semicolons intact */

  /* check for " */
  if (*s == '"') return "say";

  /* check for bind and keybind and tcl and help */
  if (strncasecmp(s, "bind", 4) == 0 && isspace(s[4]))
    return "bind";
  if (strncasecmp(s, "help", 4) == 0 && isspace(s[4]))
    return "help";
#if USER_TCL
  if (strncasecmp(s, "tcl", 3) == 0 && isspace(s[3]))
    return "tcl";
#endif
#if 0
  if (strncasecmp(s, "keybind", 7) == 0 && isspace(s[7]))
    return "keybind";
#endif

  /* skip whitespace and slashes */
  while (isspace(*s) || *s == '/') s++;

  /* break up string at semicolons; do pieces recursively */
  for (i=0; s[i] != '\0'; i++) if (s[i] == ';') {
    s[i] = '\0';
    help(s);
    help(s+i+1);
    return NULL;
  }

  /* at this point, s contains no semicolons */

  /* handle things like `2c!stayman' or `pass!!forcing' */
  /* `2c!stayman; foo; bar' -> `2c; alert stayman; foo; bar' */
  /* `pass!!forcing pass; foo' -> `pass; redalert forcing pass; foo' */
  /* `say hello! how are you?! good to see you' is untouched */
  for (i=0; s[i] != '\0' && iscmd(s[i]); i++); /* find ! immed after command */
  if (s[i] == '!') {
    helpmsg("The use of ! or !! is equivalent to typing `; alert' or `; redalert'");
    helpmsg("See the user manual for details.");
    return NULL;
  }

  h = commandhash(s);
  if ((char *)h == AMBIGUOUS) {
    if (strlen(s) <= 12)
      helpmsg3("Ambiguous abbreviation of command name (`", s, "')");
    else {
      s[10] = '\0';
      helpmsg3("Ambiguous abbreviation of command name (`", s, "...')");
    }    
    return NULL;
  }

  if (h == NULL) {
    if (strlen(s) <= 12)
      helpmsg3("Unrecognized command `", s, "'");
    else {
      s[10] = '\0';
      helpmsg3("Unrecognized command `", s, "...'");
    }    
    return NULL;
  }

  if (ISSTRINGCOMMAND(h)) {
    char *t;

    t = cmdskip(s);
    if (*t != '\0') {
      do {
	*t = '\0';
	t--;
      } while (isspace(*t));
      helpmsg3("Spurious arguments to `", s, "' ignored");
    }
    helpmsg5("`", s, "' is bound to `", (char *) h, "'");
    return NULL;
  }
  else return s;
}

/* parsing and dispatching of commands */
int docmd(char *s)
{
  static int curse = 0;
  int i;
  int (*h)();

  if (s[0] == '\0' || isspace(s[0]) && s[1] == '\0') return TCL_OK;

  if (curse > 100) {
    errormsg3("Recursion too deep while trying to execute `", s, "'");
    return TCL_OK;
  }

  if (curse == 0)
    executionlog("command: ", s);
  else
    executionlog("subcommand: ", s);

#if DBG
  if (hasWindows) printf("do %s\n", s);
#endif
  /* first, check for the commands that want semicolons intact */

  /* check for " */
  if (*s == '"') return dosay(s+1);

  /* check for bind and keybind and tcl and help */
  if (strncasecmp(s, "bind", 4) == 0 && isspace(s[4]))
    return dobind(s+5);
  if (strncasecmp(s, "help", 4) == 0 && (s[4] == '\0' || isspace(s[4])))
    return dohelp(s+4);
#if USER_TCL
  if (strncasecmp(s, "tcl", 3) == 0 && isspace(s[3]))
    return dotcl(s+4);
#endif
#if 0
  if (strncasecmp(s, "keybind", 7) == 0 && isspace(s[7]))
    return dokeybind(s+8);
#endif

  /* skip whitespace and slashes */
  while (isspace(*s) || *s == '/') s++;

  /* break up string at semicolons; do pieces recursively */
  for (i=0; s[i] != '\0'; i++) if (s[i] == ';') {
    int result1, result2;

    s[i] = '\0';
    curse++;
    result1 = docmd(s);
    result2 = docmd(s+i+1);
    curse--;
    return (result1 == TCL_OK && result2 == TCL_OK) ? TCL_OK : TCL_ERROR;
  }

  /* at this point, s contains no semicolons */

  /* handle things like `2c!stayman' or `pass!!forcing' */
  /* `2c!stayman; foo; bar' -> `2c; alert stayman; foo; bar' */
  /* `pass!!forcing pass; foo' -> `pass; redalert forcing pass; foo' */
  /* `say hello! how are you?! good to see you' is untouched */
  for (i=0; s[i] != '\0' && iscmd(s[i]); i++); /* find ! immed after command */
  if (s[i] == '!') {
    int result1, result2;
    char *t;

    s[i++] = '\0';
    t = markgarbage(salloc(sizeof(char) * (12 + strlen(s + i))));
    /* multiple '!'s for a redalert */
    if (s[i] == '!') {
      strcpy(t, "redalert ");
      while (s[i] == '!') i++;
    } else strcpy(t, "alert ");
    strcat(t, s + i);
    curse++;
    result1 = docmd(s);
    result2 = docmd(t);
    curse--;
    return (result1 == TCL_OK && result2 == TCL_OK) ? TCL_OK : TCL_ERROR;
  }

  if (*s == '\0') return TCL_OK; /* null command */
  
  h = commandhash(s);
  if ((char *)h == AMBIGUOUS) {
    if (strlen(s) <= 12)
      errormsg3("Ambiguous abbreviation of command name (`", s, "')");
    else {
      s[10] = '\0';
      errormsg3("Ambiguous abbreviation of command name (`", s, "...')");
    }    
    return TCL_OK;
  }

  if (h == NULL) {
    if (strlen(s) <= 12)
      errormsg3("Unrecognized command `", s, "'");
    else {
      s[10] = '\0';
      errormsg3("Unrecognized command `", s, "...'");
    }    
    return TCL_OK;
  }

  if (ISSTRINGCOMMAND(h)) {
    char *t;
    int ret;

    t = cmdskip(s);
    if (*t != '\0') {
      do {
	*t = '\0';
	t--;
      } while (isspace(*t));
      warningmsg3("Spurious arguments to `", s, "' ignored");
    }
    curse++;
    ret = docmd((char *)h);
    curse--;
    return ret;
  }
  else return (*h)(cmdskip(s)); /* normal case */
}


