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

/* $Id: tickler.h,v 1.13 2005/08/20 20:30:33 lexspoon Exp $
 *
 * Interfacing with Tcl/Tk
 */


#ifndef __TICKLER_H__
#define __TICKLER_H__

#include "calendar.h"

#ifndef TCL_8_OR_HIGHER
#ifdef TCL_PARSE_PART1
#define TCL_8_OR_HIGHER 1
#else
#define TCL_8_OR_HIGHER 0
#endif /* TCL_PARSE_PART1 */
#endif /* TCL_8_OR_HIGHER */

#if TCL_8_OR_HIGHER
#define SET_INTERP_RESULT(s) Tcl_SetResult(interp, (s), TCL_STATIC)
#define GET_INTERP_RESULT() Tcl_GetStringResult(interp)
#else
#define SET_INTERP_RESULT(s) (interp->result = (s))
#define GET_INTERP_RESULT() (interp->result)
#endif /* TCL_8_OR_HIGHER */

void check_library_paths(void);

void setupbindings(void);

/* returns whether the character appears in the string */
#define isin(c, s) (strchr((s), (c)) != NULL)

#define iscmd(c) ((c) != '\0' && !isspace(c) && (isalpha(c) || isdigit(c) || \
	isin(c, "${}[]@#%^&*()`';:,.<>/?-_=+|~")))


void source(const char *s);
char *TclDoIgnoreErrors(const char *s);
char *TclDo(const char *s);
char *TclDo2(const char *s, const char *s2);
char *TclDo3(const char *s, const char *s2, const char *s3);
char *TclDo4(const char *, const char *, const char *, const char *);
char *TclDo5(const char *, const char *, const char *, const char *,
	     const char *);
char *TclDo6(const char *, const char *, const char *, const char *,
	     const char *, const char *);
char *TclDo7(const char *, const char *, const char *, const char *,
	     const char *, const char *, const char *);
char *TclDo8(const char *, const char *, const char *, const char *,
	     const char *, const char *, const char *, const char *);
char *TclDo9(const char *, const char *, const char *, const char *,
	     const char *, const char *, const char *, const char *,
             const char *);
char *TclDo10(const char *, const char *, const char *, const char *,
	      const char *, const char *, const char *, const char *,
              const char *, const char *);
char *TclDo11(const char *, const char *, const char *, const char *,
	      const char *, const char *, const char *, const char *,
              const char *, const char *, const char *);

#ifndef DBGprintf

#ifdef NO_X11
#define DBGprintf 0
#else /* have X11 */
#define DBGprintf DBG
#endif

#endif /* DBGprintf */

/* debugging versions of TclDo() */
#if DBGprintf
#define DTCLDO 1
const char *dTclDo(const char *s);
const char *dTclDo2(const char *s, const char *s2);
const char *dTclDo3(const char *s, const char *s2, const char *s3);
const char *dTclDo4(const char *, const char *, const char *, const char *);
const char *dTclDo5(const char *, const char *, const char *, const char *,
		    const char *);
const char *dTclDo6(const char *, const char *, const char *, const char *,
		    const char *, const char *);
const char *dTclDo7(const char *, const char *, const char *, const char *,
		    const char *, const char *, const char *);
#else
#define DTCLDO 0
#define dTclDo TclDo
#define dTclDo2 TclDo2
#define dTclDo3 TclDo3
#define dTclDo4 TclDo4
#define dTclDo5 TclDo5
#define dTclDo6 TclDo6
#define dTclDo7 TclDo7
#endif

/* s is the name of a Tcl variable; return whether it is true or false */
#define TclBool(s) atoi(TclDo2("gset ", (s)))
#define TclInt(s) TclBool(s)

#define TclSet(var, value) Tcl_SetVar(interp, (var), (value), TCL_GLOBAL_ONLY)
#define TclGet(var) Tcl_GetVar(interp, (var), TCL_GLOBAL_ONLY)

int floaterentrypoint(ClientData clientData, Tcl_Interp *interp,
		 int argc, const char *argv[]);
int FloaterClose(ClientData clientData, Tcl_Interp *interp,
		 int argc, const char *argv[]);
int floatertimeout(ClientData clientData, Tcl_Interp *interp,
		   int argc, const char *argv[]);
int cmd(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]);
int receivemessage(ClientData clientData, Tcl_Interp *interp,
		   int argc, const char *argv[]);
int talk(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]);

void talkm(const char *s);
void talkm2(const char *s, const char *s2);
void talkm3(const char *s, const char *s2, const char *s3);
void talkm4(const char *s, const char *s2, const char *s3, const char *s4);
void talkm5(const char *, const char *, const char *, const char *,
	    const char *);
void talkm6(const char *, const char *, const char *, const char *,
	    const char *, const char *);
void talkm7(const char *, const char *, const char *, const char *,
	    const char *, const char *, const char *);
void talkm8(const char *, const char *, const char *, const char *,
	    const char *, const char *, const char *, const char *);
void talkm9(const char *, const char *, const char *, const char *,
	    const char *, const char *, const char *, const char *,
	    const char *);
void talkm10(const char *, const char *, const char *, const char *,
 	     const char *, const char *, const char *, const char *,
	     const char *, const char *);

char *tclclean(const char *);
char *braceclean(const char *);
char *destructivebracketclean(char *s);
#define fileclean(s) \
    destructiveBackslashToSlash(braceclean(destructivebracketclean(s)))

/* check for nasties embedded in text from outside */
const char *safety_check(const char *s);

void fatal(const char *msg, int exitcode);
void error(void);
void inittickler(void);

#define clearcc() TclDo("uplevel #0 {set cclines(NS) [set cclines(EW) 0]}")

#if STRICT_FILESYSTEM_ACCESS
#define validfile(filename) atob(TclDo3("validfile {", (filename), "}"))
#else
#define validfile(filename) TRUE
#endif

/* returns whether it succeeded; sets Tcl variable var to the result of open */
#define TclOpen(var, filename, mode)					     \
(validfile(filename) ?							     \
  !atob(TclDo6("catch {open {", (filename), "} ", (mode), "} ", (var))) : \
  (TclDo3("set ", (var), " {File must lie in Floater's subdirectory}"), FALSE))

#ifdef LOGINSERVER

#define floatertime() localtime_as_string()

#define rtalkmsg(s) (talkm(s))
#define rtalkmsg2(s, s2) (talkm2((s), (s2)))
#define rtalkmsg3(s, s2, s3) (talkm3((s), (s2), (s3)))
#define rtalkmsg4(s, s2, s3, s4) (talkm4((s), (s2), (s3), (s4)))
#define rtalkmsg5(s, s2, s3, s4, s5) (talkm5((s), (s2), (s3), (s4), (s5)))
#define rtalkmsg6(s, s2, s3, s4, s5, s6) (talkm6((s), (s2), (s3), (s4), (s5), (s6)))
#define rtalkmsg7(s, s2, s3, s4, s5, s6, s7) (talkm7((s), (s2), (s3), (s4), (s5), (s6), (s7)))
#define rtalkmsg8(s, s2, s3, s4, s5, s6, s7, s8) (talkm8((s), (s2), (s3), (s4), (s5), (s6), (s7), (s8)))

#define talkmsg(s) (talkm2(floatertime(), (s)))
#define talkmsg2(s, s2) (talkm3(floatertime(), (s), (s2)))
#define talkmsg3(s, s2, s3) (talkm4(floatertime(), (s), (s2), (s3)))
#define talkmsg4(s, s2, s3, s4) (talkm5(floatertime(), (s), (s2), (s3), (s4)))
#define talkmsg5(s, s2, s3, s4, s5) (talkm6(floatertime(), (s), (s2), (s3), (s4), (s5)))
#define talkmsg6(s, s2, s3, s4, s5, s6) (talkm7(floatertime(), (s), (s2), (s3), (s4), (s5), (s6)))
#define talkmsg7(s, s2, s3, s4, s5, s6, s7) (talkm8(floatertime(), (s), (s2), (s3), (s4), (s5), (s6), (s7)))
#define talkmsg8(s, s2, s3, s4, s5, s6, s7, s8) (talkm9(floatertime(), (s), (s2), (s3), (s4), (s5), (s6), (s7), (s8)))

#else /* not LOGINSERVER */

#define talkmsg(s) (talkm(s))
#define talkmsg2(s, s2) (talkm2((s), (s2)))
#define talkmsg3(s, s2, s3) (talkm3((s), (s2), (s3)))
#define talkmsg4(s, s2, s3, s4) (talkm4((s), (s2), (s3), (s4)))
#define talkmsg5(s, s2, s3, s4, s5) (talkm5((s), (s2), (s3), (s4), (s5)))
#define talkmsg6(s, s2, s3, s4, s5, s6) (talkm6((s), (s2), (s3), (s4), (s5), (s6)))
#define talkmsg7(s, s2, s3, s4, s5, s6, s7) (talkm7((s), (s2), (s3), (s4), (s5), (s6), (s7)))
#define talkmsg8(s, s2, s3, s4, s5, s6, s7, s8) (talkm8((s), (s2), (s3), (s4), (s5), (s6), (s7), (s8)))
#endif /* LOGINSERVER */

#define m_turn_off_scrolllock() \
	(atob(TclGet("showerrors")) ? turn_off_scrolllock() : NULL)

#define ERRORTEXT "Error: "
#define errormsg(s) \
	(m_turn_off_scrolllock(), talkmsg2(ERRORTEXT, s))
#define errormsg2(s, s2) \
	(m_turn_off_scrolllock(), talkmsg3(ERRORTEXT, (s), (s2)))
#define errormsg3(s, s2, s3) \
	(m_turn_off_scrolllock(), talkmsg4(ERRORTEXT, (s), (s2), (s3)))
#define errormsg4(s, s2, s3, s4) \
	(m_turn_off_scrolllock(), talkmsg5(ERRORTEXT, (s), (s2), (s3), (s4)))


#define RESULTTEXT "Result: "
#define resultmsg_(s) TEMPCAT(RESULTTEXT, (s))
#define resultmsg(s) (talkmsg2(RESULTTEXT, (s)))
#define resultmsg2(s, s2) (talkmsg3(RESULTTEXT, (s), (s2)))
#define resultmsg3(s, s2, s3) (talkmsg4(RESULTTEXT, (s), (s2), (s3)))
#define resultmsg4(s, s2, s3, s4) (talkmsg5(RESULTTEXT, (s), (s2), (s3), (s4)))

#define WARNINGTEXT "Warning: "
#define warningmsg(s) (talkmsg2(WARNINGTEXT, s))
#define warningmsg2(s, s2) (talkmsg3(WARNINGTEXT, (s), (s2)))
#define warningmsg3(s, s2, s3) (talkmsg4(WARNINGTEXT, (s), (s2), (s3)))
#define warningmsg4(s, s2, s3, s4) (talkmsg5(WARNINGTEXT, (s), (s2), (s3), (s4)))

#define FOUNDTEXT "Find: "
#define foundmsg(s) (talkmsg2(FOUNDTEXT, s))
#define foundmsg2(s, s2) (talkmsg3(FOUNDTEXT, (s), (s2)))
#define foundmsg3(s, s2, s3) (talkmsg4(FOUNDTEXT, (s), (s2), (s3)))
#define foundmsg4(s, s2, s3, s4) (talkmsg5(FOUNDTEXT, (s), (s2), (s3), (s4)))

#define STATUSTEXT "` "
#define status(s) (talkmsg2(STATUSTEXT, s))
#define status2(s, s2) (talkmsg3(STATUSTEXT, (s), (s2)))
#define status3(s, s2, s3) (talkmsg4(STATUSTEXT, (s), (s2), (s3)))
#define status4(s, s2, s3, s4) (talkmsg5(STATUSTEXT, (s), (s2), (s3), (s4)))
#define status5(s, s2, s3, s4, s5) (talkmsg6(STATUSTEXT, (s), (s2), (s3), (s4), (s5)))
#define status6(s, s2, s3, s4, s5, s6) (talkmsg7(STATUSTEXT, (s), (s2), (s3), (s4), (s5), (s6)))
#define status7(s, s2, s3, s4, s5, s6, s7) (talkmsg8(STATUSTEXT, (s), (s2), (s3), (s4), (s5), (s6), (s7)))

#define HELPTEXT "Help: "
#define helpmsg(s) (talkmsg2(HELPTEXT, s))
#define helpmsg2(s, s2) (talkmsg3(HELPTEXT, (s), (s2)))
#define helpmsg3(s, s2, s3) (talkmsg4(HELPTEXT, (s), (s2), (s3)))
#define helpmsg4(s, s2, s3, s4) (talkmsg5(HELPTEXT, (s), (s2), (s3), (s4)))
#define helpmsg5(s, s2, s3, s4, s5) (talkmsg6(HELPTEXT, (s), (s2), (s3), (s4), (s5)))



extern char *handto4str(int *deck, int player);


#endif /* __TICKLER_H__ */
