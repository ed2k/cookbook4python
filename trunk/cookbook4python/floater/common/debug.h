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
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "floater.h"
#include <signal.h>
#include <assert.h>

#ifndef USER_TCL
#define USER_TCL 1
#endif

#ifndef CORE_DUMPS
#define CORE_DUMPS 1
#endif

#define MAKING_TUTORIAL 0
#define log(s) fputs(TEMPCAT((s), "\n"), logfile);

/* whether to create a log file of messages and other good stuff */
#define LOGGING 0

/* controls for various debugging output */
#define DBG 0
#define DBGu 0
#define DEBUGMSG 0
#define DEBUGMSG0 0
#define DEBUGCONN 0
#define DEBUGBR 0
#define DEBUGRES 0
#define DEBUGSEEN 0
#define DEBUGHEARTS 0

#define ANY_DBG (DBG||DEBUGCONN||DEBUGMSG||DEBUGBR||DEBUGRES||DEBUGSEEN||DEBUGHEARTS)

#define RANDOMPLAY 1 /* whether randomplay is an option */

extern fl_bool debugprint;

#if CORE_DUMPS
#define floater_abort abort()
#else
#define floater_abort exit(-2)
#endif

#if ANY_DBG
#ifdef NO_X11
#define DEBUG(x) (TclDo("gset debugprinting 1"), (x), \
		  (TclDo("gset debugprinting 0")))
#else /* NO_X11 */
#define DEBUG(x) ((debugprint=TRUE), TclDo("gset debugprinting 1"), (x), \
		  (TclDo("gset debugprinting 0")), (debugprint=FALSE))
#endif /* NO_X11 */

#else /* ANY_DBG */
#define DEBUG(x)
#endif /* if ANY_DBG */

#undef _assert
#undef assert
# define _assert(ex)	{if (!(ex)){endcurses(); mail_bug("Assertion failed: file \"%s\", line %d\n", __FILE__, __LINE__); floater_abort;}}
# define assert(ex)	_assert(ex)

#if LOGGING||MAKING_TUTORIAL
extern FILE *logfile;
#endif

#endif /* __DEBUG_H__ */
