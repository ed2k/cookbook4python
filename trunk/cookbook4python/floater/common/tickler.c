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
 *     Lex Spoon <lex@cc.gatech.edu>
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

/* tickler.c - interface to Tcl from C */


#include "floater.h"
#include "floatcmd.h"
#include "UI.h"
#include "tickler.h"
#include "commandhash.h"
#include "br.h"
#include "comm.h"

fl_bool debugprint = FALSE;

static char *talkmO, *talkmC;

semistatic const char *fatal_error_message = NULL;

#if !TCL_IN_C
static fl_bool check_library_path(char *env_var, char *compiled_in, char *guess, char *file)
{
  char *tcl_dir;
  FILE *f;
  char testfilename[500];
  char temp1[500];
  char *temp2;
  static int count = 0;
  fl_bool ret;

  if (++count > 2) return FALSE;

  if ((tcl_dir = getenv(env_var)) == NULL)
    tcl_dir = compiled_in;
  
  sprintf(testfilename, "%s/%s", tcl_dir, file);
  if ((f = fopen(testfilename, "r")) == NULL) {
    fprintf(stderr, "WARNING! Unable to open %s---\n\t"
	    "does environment variable %s need to be set?\n",
	    testfilename, env_var);
    if (count == 1) {
#ifndef SRC_DIR
#define SRC_DIR "."
#endif
      sprintf(temp1, "%s/%s", SRC_DIR, guess);
      fprintf(stderr, "WARNING! Guessing to set %s to %s\n",
	      env_var, temp1);
      temp2 = salloc(sizeof(char) * (strlen(env_var) + strlen(temp1) + 5));
      sprintf(temp2, "%s=%s", env_var, temp1);
      putenv(temp2);
      ret = check_library_path(env_var, compiled_in, guess, file);
    } else ret = FALSE;
    --count;
    return ret;
  } else fclose(f);
  --count;
  return TRUE;
}

#define check_floater_library_path() \
    check_library_path("FLOATER_LIBRARY", FLOATER_LIBRARY, \
		       "tclcode", "floater.tcl")

void check_library_paths(void)
{
  if (!check_floater_library_path()) exit(-7);
}
#else /* TCL_IN_C */
void check_library_paths(void) {}
#endif

void inittickler(void)
{
  talkmO = STRDUP("talkmsg {");
  talkmC = STRDUP("}");
}

/* check for nasties embedded in text from outside */
const char *safety_check(const char *s)
{
  if (isin('[', s) || isin(']', s))
    fatal(TEMPCAT("Sabotage attempt from another player?!\n", s), -3);
  return s;
}

/* fatal error */
void fatal(const char *msg, int exitcode)
{
  static int fatalerrs = 0;

  if (!hasWindows) endcurses();
  fprintf(stderr, "fatal error: %s\n", msg);
  fatal_error_message = msg;
#ifdef _WIN32
  {
    void WishPanic _ANSI_ARGS_(TCL_VARARGS(char *,format));
    WishPanic(msg);
  }
#endif
  floater_abort;
/*
  if (fatalerrs++ < 10) TclDo2("exit ", itoa(exitcode));
  exit(exitcode);
*/
}

/* error from tcl --- can we ignore it? */
static fl_bool ignorable_error(void)
{
  const char *msg;

  msg = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
  if (msg == NULL) msg = STRDUP(GET_INTERP_RESULT());
  return strneq(msg, "selection isn't in", strlen("selection isn't in"));
}

/* fatal error from tcl --- print error message and exit */
void error(void)
{
  const char *msg;

  if (!hasWindows) endcurses();
  msg = Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
  if (msg == NULL) msg = STRDUP(GET_INTERP_RESULT());
  fprintf(stderr, "%s\n", msg);
  fatal(msg, 1);
}

#if !TCL_IN_C
/* Source a Tcl file.  If TCL_IN_C is true then floatcl.c defines source(). */
void source(const char *s)
{
  static char *tcl_dir = NULL;
  int result_code;
  
  if (tcl_dir == NULL)
    if ((tcl_dir = getenv("FLOATER_LIBRARY")) == NULL)
      tcl_dir = FLOATER_LIBRARY;
  
  result_code = Tcl_VarEval(interp, "source ", tcl_dir, "/", s, ".tcl", NULL);
  

  /* note sure why TCL_RETURN is ever returned, but sometimes it is; it is
     not an error. */
  if (result_code != TCL_OK && result_code != TCL_RETURN){
       printf("result_code is %d\n", result_code);
       
       error();
  }
  
}    
#endif

/* Do a Tcl command */
char *TclDoIgnoreErrors(const char *s)
{
  char *s1;
     
  s1 = STRDUP(s);
  Tcl_Eval(interp, s1);
  free((void *)s1);
  return TEMPCOPY(GET_INTERP_RESULT());
}    


/* Do a Tcl command */
char *TclDo(const char *s)
{
  char *result;

  s = STRDUP(s);
  
  result =  TclDo7(s, NULL, NULL, NULL, NULL, NULL, NULL);

  free((void *)s);
  return result;
}    

/* Do a Tcl command */
char *TclDo2(const char *s, const char *s2)
{
  return TclDo7(s, s2, NULL, NULL, NULL, NULL, NULL);
}    

/* Do a Tcl command */
char *TclDo3(const char *s, const char *s2, const char *s3)
{
  return TclDo7(s, s2, s3, NULL, NULL, NULL, NULL);
}    

/* Do a Tcl command */
char *TclDo4(const char *s, const char *s2, const char *s3, const char *s4)
{
  return TclDo7(s, s2, s3, s4, NULL, NULL, NULL);
}    

/* Do a Tcl command */
char *TclDo5(const char *s, const char *s2, const char *s3, const char *s4, const char *s5)
{
  return TclDo7(s, s2, s3, s4, s5, NULL, NULL);
}    

/* Do a Tcl command */
char *TclDo6(const char *s, const char *s2, const char *s3, const char *s4, const char *s5, const char *s6)
{
  return TclDo7(s, s2, s3, s4, s5, s6, NULL);
}    

/* Do a Tcl command */
char *TclDo7(const char *s, const char *s2, const char *s3, const char *s4,
	     const char *s5, const char *s6, const char *s7)
{
  return TclDo10(s, s2, s3, s4, s5, s6, s7, NULL, NULL, NULL);
}    

/* Do a Tcl command */
char *TclDo8(const char *s, const char *s2, const char *s3, const char *s4,
	     const char *s5, const char *s6, const char *s7, const char *s8)
{
  return TclDo10(s, s2, s3, s4, s5, s6, s7, s8, NULL, NULL);
}

/* Do a Tcl command */
char *TclDo9(const char *s, const char *s2, const char *s3, const char *s4,
	     const char *s5, const char *s6, const char *s7, const char *s8,
             const char *s9)
{
  return TclDo10(s, s2, s3, s4, s5, s6, s7, s8, s9, NULL);
}    

char *TclDo10(const char *s, const char *s2, const char *s3, const char *s4,
	      const char *s5, const char *s6, const char *s7, const char *s8,
              const char *s9, const char *s10)
{
  return TclDo11(s, s2, s3, s4, s5, s6, s7, s8, s9, s10, NULL);
}


/* Do a Tcl command */
char *TclDo11(const char *s, const char *s2, const char *s3, const char *s4,
	      const char *s5, const char *s6, const char *s7, const char *s8,
              const char *s9, const char *s10, const char *s11)
{
  static int retcode;

  retcode = Tcl_VarEval(interp, s, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, NULL);
  
  /* I'm note sure why TCL_RETURN is ever returned, but sometimes it
     is; it is not an error. -lex */
  if(retcode != TCL_OK  && retcode != TCL_RETURN && !ignorable_error()) {
    error();
  }

  return TEMPCOPY(GET_INTERP_RESULT());
}    


/* clean out potentially special characters to prevent trouble with Tcl */
char *tclclean(const char *s)
{
  char *t, *n, dirty;
  
  for (dirty=FALSE, t=n=TEMPCOPY(s); *t != '\0'; t++)
    if (!isalpha(*t) && !isdigit(*t) && !isspace(*t) &&
	!isin(*t, "$[]!@#%^&*()`';:,.<>/?-_=+|~\"")) {
      *t = '.';
      dirty = TRUE;
    }

#if DBG
  if (hasWindows && dirty) printf("dirty! %s => %s\n", s, n);
#endif

  return n;
}
    
/* replace { with \{ and } with \} */
char *braceclean(const char *s)
{
  char *t, *n;
  
  n = t = markgarbage(salloc(strlen(s) * 2 * sizeof(char) + 1));
  while ((*t = *s++) != '\0')
    if (*t == '{' || *t == '}') {
      t[1] = t[0];
      t[0] = '\\';
      t += 2;
    } else t++;

  return n;
}

/* destuctively replace [ or ] with . */
char *destructivebracketclean(char *s)
{
  int i;

  for (i = 0; s[i] != '\0'; i++) if (s[i] == '[' || s[i] == ']') s[i] = '.';
  return s;
}
    
/* put a message in the talk window */
void talkm(const char *s)
{
#if DBG
  if (debugprint && (hasWindows || SILENT)) puts(s);
#endif
  TclDo3(talkmO, tclclean(s), talkmC);
}

/* put a message in the talk window */
void talkm2(const char *s, const char *s2)
{
#if DBG
  if (debugprint && (hasWindows || SILENT)) printf("%s%s\n", s, s2);
#endif
    
  TclDo4(talkmO, tclclean(s), tclclean(s2), talkmC);
}

/* put a message in the talk window */
void talkm3(const char *s, const char *s2, const char *s3)
{
#if DBG
  if (debugprint && (hasWindows || SILENT)) printf("%s%s%s\n", s, s2, s3);
#endif
  TclDo5(talkmO, tclclean(s), tclclean(s2), tclclean(s3), talkmC);
}

/* put a message in the talk window */
void talkm4(const char *s, const char *s2, const char *s3, const char *s4)
{
#if DBG
  if (debugprint && (hasWindows || SILENT))
    printf("%s%s%s%s\n", s, s2, s3, s4);
#endif
  TclDo6(talkmO, tclclean(s), tclclean(s2), tclclean(s3), tclclean(s4), talkmC);
}

/* put a message in the talk window */
void talkm5(const char *s, const char *s2, const char *s3, const char *s4, const char *s5)
{
#if DBG
  if (debugprint && (hasWindows || SILENT))
    printf("%s%s%s%s\n", s, s2, s3, s4);
#endif
  TclDo7(talkmO, tclclean(s), tclclean(s2), tclclean(s3), tclclean(s4), tclclean(s5), talkmC);
}

/* put a message in the talk window */
void talkm6(const char *s, const char *s2, const char *s3, const char *s4, const char *s5, const char *s6)
{
#if DBG
  if (debugprint && (hasWindows || SILENT))
    printf("%s%s%s%s\n", s, s2, s3, s4);
#endif
  TclDo8(talkmO, tclclean(s), tclclean(s2), tclclean(s3), tclclean(s4), tclclean(s5), tclclean(s6), talkmC);
}



/* put a message in the talk window */
void talkm7(const char *s, const char *s2, const char *s3, const char *s4, const char *s5, const char *s6, const char *s7)
{
#if DBG
  if (debugprint && (hasWindows || SILENT))
    printf("%s%s%s%s\n", s, s2, s3, s4);
#endif
  TclDo9(talkmO, tclclean(s), tclclean(s2), tclclean(s3), tclclean(s4), tclclean(s5), tclclean(s6), tclclean(s7), talkmC);
}

/* put a message in the talk window */
void talkm8(const char *s, const char *s2, const char *s3, const char *s4, const char *s5, const char *s6, const char *s7, const char *s8)
{
#if DBG
  if (debugprint && (hasWindows || SILENT))
    printf("%s%s%s%s\n", s, s2, s3, s4);
#endif
  TclDo10(talkmO, tclclean(s), tclclean(s2), tclclean(s3), tclclean(s4), tclclean(s5), tclclean(s6), tclclean(s7), tclclean(s8), talkmC);
}


/* put a message in the talk window */
void talkm9(const char *s, const char *s2, const char *s3, const char *s4, const char *s5, const char *s6, const char *s7, const char *s8, const char *s9)
{
#if DBG
  if (debugprint && (hasWindows || SILENT))
    printf("%s%s%s%s\n", s, s2, s3, s4);
#endif
  TclDo11(talkmO, tclclean(s), tclclean(s2), tclclean(s3), tclclean(s4), tclclean(s5), tclclean(s6), tclclean(s7), tclclean(s8), tclclean(s9), talkmC);
}


/* C routine called by "command" (our homemade Tcl command) */
int cmd(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
  if (argc == 1) return TCL_OK; /* null command */

  if (argc != 2) {
    SET_INTERP_RESULT("wrong # of arguments to `command'");
    return TCL_ERROR;
  }

/*  printf("command: %s\n", argv[1]);*/

  return docmd(TEMPCOPY(argv[1]));
}


/* C routine called by "talk" (our homemade Tcl command) */
int talk(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
  if (argc != 2) {
    SET_INTERP_RESULT("wrong # of arguments to `talk'");
    return TCL_ERROR;
  }

  if (*(argv[1]) != '/') return dosay(TEMPCOPY(argv[1]));
  else return docmd(TEMPCOPY(argv[1] + 1));
}
 
/* things that periodically need to be done */
void tasks(void)
{
  periodictablereannounce();
  checktablelisttimeouts();
  try_rejoin();

#if LOGINSERVER
  if (atoi(TclGet("checknewweek"))) {
    checknewweek();
    TclSet("checknewweek", "0");
    TclDo("after 500000 { gset checkneweek 1 }");
  }
#endif

#ifndef SERVER
  UIupdate();
#if RANDOMPLAY
  {
    extern fl_bool randomplay;
    extern char myname[];
    
    TclDo("gset showerrors 0");
    if (randomplay && (myturntobid() || myturntoplay())) {
      int i = 0;
      if (myturntobid()) makerandombid();
      while (myturntoplay() && randomplay && ++i < 100) makerandomplay();
    }
    TclDo("gset showerrors 1");
  }
#endif
#if 0
  {
    extern fl_bool computerplay;
    extern char myname[];
    
    TclDo("gset showerrors 0");
    if (computerplay && (myturntobid() || myturntoplay())) {
      if (myturntobid()) makecomputerbid();
      while (myturntoplay() && computerplay) makecomputerplay();
    }
    TclDo("gset showerrors 1");
  }
#endif /* if 0 */
#endif /* ifndef SERVER */
}



/* convert a hand into something of the form "{AKQJ} {AKQJ} {AKQJ} {4} " */
char *handto4str(int *deck, int player)
{
  char s[100];
  int i;

  s[0] = '\0';
  SUITLOOP(i) {
    sprintf(s + strlen(s), "{");
    snafflesuit(deck + cardsperhand * player, i, s + strlen(s));
    sprintf(s + strlen(s), "} ");
  }
  return TEMPCOPY(s);
}



int floaterentrypoint(ClientData clientData, Tcl_Interp *interp,
		 int argc, const char *argv[])
{
  extern void tasks(void);
  int ret;

  /* "unnecessary," but retained to check "impossible" conditions */
  static int entry = 0;

  
  assert(entry == 0);

  /* recursion is not allowed between DISALLOW_REENTRY() and ALLOW_REENTRY() */
  DISALLOW_REENTRY();
  entry++;

  if (entry > 100)
    fatal("Excessive recursion caused by user's command",
	  (entry > 101) ? 102 : 101);

  UIupdatestatus();
  ret = (*((Tcl_CmdProc *) clientData))((ClientData) NULL, interp, argc, argv);
  if (entry == 1) tasks();
  UIupdatestatus();

  if (--entry == 0) destroygarbage();
  ALLOW_REENTRY();
  return ret;
}
