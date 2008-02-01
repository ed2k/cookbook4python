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
#include "tickler.h"
#include "floatcmd.h"

extern char *helpdescriptions;
extern char *helpcommands;

/* find the text in helpdescriptions that starts with a tab character followed
   by the given string, and ends with a tab character */
static void lookuphelp(char *s)
{
  char *t = helpdescriptions;
  int len = strlen(s);

  for (;;) {
    while (*t != '\0' && *t != '\t') t++;
    if (*t == '\0') {
      helpmsg3("No help available for `", s, "'");
      return;
    }
    t++; /* advance over tab */
    if (strncasecmp(s, t, len) == 0) {
      /* found it */
      s = t = TEMPCOPY(t);
      while (*t != '\n') t++;
      *t = '\0';
      helpmsg3("`", s, "'");
      s = t = t + 1;
      while (*t != '\0') {
	if (*t == '\t') {
	  *t = '\0';
	  goto done;
	}
	if (*t == '\n') {
	  *t = ' ';
	}
	t++;
      }
    done:
      helpmsg(s);
      return;
    }
  }
}

static void dumphelpable(void)
{
  helpmsg("What follows is a list of commands for which hand-written descriptions are available.  For example, for help on the claim command, do `help claim.' See the Floater WWW page for more information.");
  talkmsg(helpcommands);
}

/* a request for help on s */
void help(char *s)
{
  if (*s == '\0' || strcaseeq(s, "help")) {
    /* no arguments: just print out all commands for which we have help */
    dumphelpable();
    return;
  }

  if ((s = helpstring_to_command(s)) == NULL) return;

  lookuphelp(s);
}
