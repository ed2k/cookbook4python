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
/* 
 *	main.c
 */

#include <stdio.h>
#include "floater.h"
#include "UI.h"
#include "debug.h"
#include "tickler.h"
#include "commandhash.h"
#include "comm.h"

/* the zero-th argument of the program, which is typically the name of
 * the executable.  This value is simulated on Windows. */
char *argv0;

/*
 * The following variable is a special hack that is needed in order for
 * Sun shared libraries to be used for Tcl.
 */

#if LOGGING||MAKING_TUTORIAL
FILE *logfile;
#endif

#if 0
extern int matherr();
int *tclDummyMathPtr = (int *) matherr;
#endif

Tcl_Interp *interp;		/* Interpreter for this application. */

/* default is to use GUI, if available */
fl_bool usingTk = TRUE;
fl_bool using_curses = FALSE;

fl_bool geometry_specified = FALSE;
fl_bool initializing = TRUE;

char *platform = NULL; /* String describing machine, OS, etc. */

#ifdef MS_WINDOWS
#if 0
extern void create_shortcut(char *destfile, char *linkfile,
			    char *icon, char *desc);
#define do_shortcut() create_shortcut(getenv("short_dest"),	\
				      getenv("short_link"),	\
				      getenv("short_icon"),	\
				      getenv("short_desc"))

#endif
#endif /* MS_WINDOWS */

#ifdef MS_WINDOWS
/* set floaterdir based on argv0, so that Floater saves data in its
 * installation directory */
static void set_floaterdir_from_argv0(void) 
{
  char *dir;   /* build up the directory string here */
     

  dir = STRDUP(argv0);
  
  /* remove the executable from the pathname */
  {
    char *fs_pos, *bs_pos, *kill_pos;
    
    fs_pos = strrchr(dir, '/');
    bs_pos = strrchr(dir, '\\');
    if(fs_pos == NULL)
      kill_pos = bs_pos;
    else if(bs_pos == NULL)
      kill_pos = fs_pos;
    else if((fs_pos - dir) < (bs_pos - dir))
      kill_pos = bs_pos;
    else
      kill_pos = fs_pos;

    if(kill_pos != NULL)
      *kill_pos = '\0';
  }

  dir = STRCAT(dir, "\\User_Data");

  TclSet("floaterdir", dir);
}
#endif

/* main entry point of Floater from Tcl/Tk */
int Floater(Tcl_Interp *i)
{
  extern void destroygarbage(void);
#ifdef MS_WINDOWS
#if 0
  fl_bool make_shortcut = strexists(getenv("short_link"));
  if (make_shortcut)
    usingTk = FALSE;
#endif
#endif

#ifdef MS_WINDOWS
  if (!strexists(getenv("TCL_LIBRARY"))) {
    int i;
    char *s = salloc(strlen(argv0) + 40);
    sprintf(s, "TCL_LIBRARY=%s", argv0);
    for (i = strlen(s); --i >= 0 && s[i] != '/'; );
    if (i >= 0)
      sprintf(s + i, "/tcl_library");
    putenv(s);
    sfree(s);
  }
  if (!strexists(getenv("TK_LIBRARY"))) {
    int i;
    char *s = salloc(strlen(argv0) + 40);
    sprintf(s, "TK_LIBRARY=%s", argv0);
    for (i = strlen(s); --i >= 0 && s[i] != '/'; );
    if (i >= 0)
      sprintf(s + i, "/tk_library");
    putenv(s);
    sfree(s);
  }
#endif

  interp = i;

#if !MACINTOSH
  if (Tcl_Init(i) == TCL_ERROR) {
    fatal(GET_INTERP_RESULT(), 2);
  }

#if NO_TK || SILENT
  usingTk = FALSE;
#else
  if (usingTk && (Tk_Init(i) == TCL_ERROR)) {
    fputs(GET_INTERP_RESULT(), stderr);
    usingTk = FALSE;
  }
#endif /* NO_TK||SILENT */

#ifdef MS_WINDOWS
#if 0
  if (make_shortcut) {
    do_shortcut();
    exit(0);
  }
#endif
#endif
#endif /* !MACINTOSH */

  if (GUI_ONLY && !usingTk) {
    fatal(GET_INTERP_RESULT(), 1);
  }

  if(hasWindows)
    UIuse_tk();
  else
    UIuse_curses();
    

#if !SILENT && !NO_TK
  Tcl_StaticPackage(i, "Tk", Tk_Init, (Tcl_PackageInitProc *) NULL);
#endif

  check_library_paths();

#ifdef MS_WINDOWS
  set_floaterdir_from_argv0();
#endif

#if 0
#ifdef MS_WINDOWS
  Tcl_CreateCommand(i, "maximize", maximize_window, (ClientData) NULL,
		    (Tcl_CmdDeleteProc *) NULL);
  TclDo("maximize .");
#endif
#endif
  Tcl_CreateCommand(i, "commandn", floaterentrypoint, (ClientData) cmd,
		    (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(i, "floaterreceiven", floaterentrypoint,
		    (ClientData) receivemessage, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(i, "FloaterClosen", floaterentrypoint,
		    (ClientData) FloaterClose, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(i, "floatertimeoutn", floaterentrypoint,
		    (ClientData) floatertimeout, (Tcl_CmdDeleteProc *) NULL);

#if 0
  Tcl_CreateCommand(i, "requestresultn", floaterentrypoint,
		    (ClientData) TCLreqresult, (Tcl_CmdDeleteProc *) NULL);
#endif

  Tcl_CreateCommand(i, "talkn", floaterentrypoint, (ClientData) talk,
		    (Tcl_CmdDeleteProc *) NULL);

#if !GUI_ONLY && !SILENT
  if (!hasWindows) {
    Tcl_CreateCommand(i, "anchor", anchor, (ClientData) 0,
		      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(i, "clearrect", clearrect, (ClientData) 0,
		      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(i, "down_and_anchor", down_and_anchor,
		      (ClientData) 0, (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(i, "right", right, (ClientData) 0,
		      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(i, "str", str, (ClientData) 0,
		      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(i, "ch", ch, (ClientData) 0,
		      (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(i, "reset_cursor_position", reset_cursor_position,
		      (ClientData) 0, (Tcl_CmdDeleteProc *) NULL);
    Tcl_CreateCommand(i, "draw_on_current_display",
		      draw_on_current_display,
		      (ClientData) 0, (Tcl_CmdDeleteProc *) NULL);
  }
#endif

  /* set up bindings used by the "command" tcl command --- see commandhash.c */
  setupbindings();

  inittickler();

#ifdef SERVER
  /* set directories where server files live */
  /* XXX this should be based on autoconf's chosen prefix */
  TclSet("server_dir", "/var/lib/floater");
#endif

  TclSet("geometry_specified", itoa(geometry_specified));

#if LOGGING||MAKING_TUTORIAL
  if (LOGGING || !hasWindows) {
    char s[80];
    
#if LOGGING
    sprintf(s, "log%s", TclDo("pid"));
#else
    strcpy(s, "tutlog.tcl");
#endif
    if ((logfile = fopen(s, "w")) == NULL) fatal("Unable to open logfile", 1);
  }      
#endif

  TclSet("floater_silent", SILENT ? "1" : "0");
#if MACOS_X
  TclSet("macintosh", "1");
#else
  TclSet("macintosh", "0");
#endif

#if MS_WINDOWS
  TclSet("mswindows", "1");
#else
  TclSet("mswindows", "0");
#endif
  

#if !GUI_ONLY && !SILENT
  if (!hasWindows) {
    begincurses();
    raw();
    noecho();
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);

    /* porting note: if halfdelay() isn't available, try:
       nodelay(stdscr, TRUE);

       (but, our autoconf script already tries to get this right) */
#if USE_NODELAY
    nodelay(stdscr, TRUE);
#else
    halfdelay(1); /* wait 0.1 second for a keypress before timing out */
#endif

    if (LINES < 24 || COLS < 80) {
      fatal("You must have at least 24 rows and 80 columns to use Floater.",
	    1);
    }
    inittextio();
    source("floatert");
    textrefresh();
  }
#endif
  if (hasWindows) {
#include "bitmaps/bitmaps.c"
#include "auction/auction_images.c"
#include "icons/icon_images.c"
    source("floater");
  } else if (SILENT) {
    source("floatert");
  }

  TclDo2("uplevel #0 incr loginserverport ", itoa((int) FLOATER_VERSION[0]));

  platform = STRDUP(TclDo("uplevel #0 {catch {unset s}; foreach i [lsort [array names tcl_platform]] {lappend s $tcl_platform($i)}; return [join $s _]}"));

#ifdef LOGINSERVER
  source("ls");
#endif
#ifdef RESULTSERVER
  source("rs");
#endif
#ifdef PSEUDOMAILER
  source("pm");
#endif
#ifdef RES2HTML
  source("rs");
  source("res2html");
#else
  commsetup(); /* get local IP address, etc. */
#endif
  destroygarbage();
#ifdef SERVER  
  if (hasWindows) TclDo("catch {pack forget .play}");
#endif
#ifdef LOGINSERVER
  TclDo("command loginserver");
#endif
#ifdef RESULTSERVER
  TclDo("command resultserver");
#endif
#ifdef PSEUDOMAILER
  TclDo("command pseudomailer");
#endif
#ifdef RES2HTML
  res2html(); /* does not return */
#endif

  initializing = FALSE;
#if SILENT
  while (TRUE) Tcl_DoOneEvent(0);
#else
  if (hasWindows) {
    extern char *helpcommands;

    TclDo3("menus_helpcommands {", helpcommands, "}");
    Tk_MainLoop();
  }
#if !GUI_ONLY
  else
    {
      int snooze = 0;

      while (TRUE) {
	int c;
	extern void tasks(void);
	
	c = getch();
	if (c == ERR) {
	  if (++snooze > 5) { tasks(); snooze = 0; }
	  if (needrefresh) textrefresh();
	} else {
	  gotch(c);
	  destroygarbage();
	}
	(void)Tk_DoOneEvent(TK_DONT_WAIT);
      }
    }
#endif
#endif
  return 0; /* never reached */
}

#ifndef NO_MAIN
int main(int argc, char **argv)
{
  int i;

  argv0 = *argv;

  /* if argv ends in "t" or "t.exe", then use the text UI */
  if ((argv[0][strlen(argv[0]) - 1] == 't')
      || ((strlen(argv[0]) >= 5) &&
	  (strcasecmp(argv[0] + (strlen(argv[0]) - 5), "t.exe") == 0)))
       usingTk = FALSE;

  for (i = 0; i < argc; i++) {
    /* printf("argv[%d] = %s\n", i, argv[i]); */
    if (streq(argv[i], "-geometry")) geometry_specified = TRUE;
  }

#if SILENT || NO_TK
  Tcl_Main(argc, argv, Floater);
#else
  Tk_Main(argc, argv, Floater);
#endif
  return 99; /* not reached unless some catastrophic error prevents startup */
}
#endif /* NO_MAIN */
