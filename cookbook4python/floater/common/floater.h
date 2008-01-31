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
#ifndef __FLOATER_H__
#define __FLOATER_H__

#include "config.h"

/* 8y = 1.3b1; make first char 8 as long as interoperability is maintained */
/* 8z = 1.3b2; make first char 8 as long as interoperability is maintained */
/* XXX should be FLOATER_PROTOCOL_VERSION ....  or should it?  What is this used for? */
#define FLOATER_VERSION "8z"

/* semistatic is used for variables that should be considered static,
   but really aren't because they need to be accessed from bug.c.  They
   should never be accessed from anywhere else. */
#define semistatic

#define FLOATER_MAXNAME 30
#define MAX_OUTPUTNAME_LENGTH 15
#define MAXADDRLEN 80
#define MAXPORTLEN 10

/* to make Solaris happy, string.h must be before curses.h */
#include <string.h>
extern char *index(const char *, int);

#ifdef VOID
#undef VOID
#endif

#ifndef GUI_ONLY
#define GUI_ONLY 0
#endif

/* set RESULTMERCHANT to 1 iff (RESULTSERVER or RES2HTML) */
#ifdef RESULTSERVER
#define RESULTMERCHANT 1
#else
#ifdef RES2HTML
#define RESULTMERCHANT 1
#else
#define RESULTMERCHANT 0
#endif
#endif

/* if LOGINSERVER or RESULTMERCHANT or PSEUDOMAILER, define SERVER to be 1;
   otherwise leave SERVER undef. */
#ifdef LOGINSERVER
#define SERVER 1
#else
#if RESULTMERCHANT
#define SERVER 1
#else
#ifdef PSEUDOMAILER
#define SERVER 1
#endif
#endif
#endif

#ifdef SERVER
#define SILENT SERVER
#define NO_X11 SERVER
#else
#define SILENT 0
#endif

#if !GUI_ONLY && !SILENT
#include <curses.h>
#endif

/* In the kibitzer tree, do the four players connect in a ring? */
#define LHO_RHO_CONNECTIONS 0

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#ifdef VOID
#undef VOID
#endif
#include <tcl.h>
#include <string.h>
#include <ctype.h>

typedef int fl_bool;
#ifndef FALSE
#define FALSE ((fl_bool) 0)
#define TRUE ((fl_bool) 1)
#endif

extern fl_bool using_curses;
#define begincurses() (using_curses ? FALSE : (initscr(), using_curses = TRUE))
#define endcurses() (using_curses ? (endwin(), using_curses = FALSE) : TRUE)

#include "debug.h"
#include "rand.h"
#include "stringlist.h"
#include "alloc.h"
#include "bits.h"
#include "util.h"
#include "result.h"
#include "help.h"
#include "tickler.h"

extern Tcl_Interp *interp;
extern char myname[]; /* in comm.c */
extern char myoutputname[]; /* in comm.c */
extern char *platform; /* in main.c */

#define floaterclock() (atoi(TclGet("floaterclock")))

/* ascii to fl_boolean */
#define atob atoi

#ifndef streq
#define streq(a, b) (strcmp((a), (b)) == 0)
#define strneq(a, b, n) (strncmp((a), (b), (n)) == 0)
#define strncaseeq(a, b, n) (strncasecmp((a), (b), (n)) == 0)
#endif

#define DISALLOW_REENTRY() TclDo("defer 1")
#define ALLOW_REENTRY() TclDo("defer -1")

#ifdef NO_X11
#define NO_TK 1
#else
#define NO_TK 0
#include <tk.h>
#endif

#define STRICT_FILESYSTEM_ACCESS 1

extern fl_bool usingTk;

#if SILENT||NO_TK
#define hasWindows (0)
#else
#if GUI_ONLY
#define hasWindows (1)
#else
#define hasWindows (usingTk)
#endif
#endif

#include "text.h"

#endif
