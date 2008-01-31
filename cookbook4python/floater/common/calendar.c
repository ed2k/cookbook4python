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
#include "calendar.h"
#include <time.h>
#include <sys/types.h>
#ifdef TIME_H
#include <time.h>
#else
#include <sys/time.h>
#endif
#include "deal.h"
#include "br.h"

static void setpreviousglobaldate(void);
char previousglobaldate[100];

static char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
			     "Aug", "Sep", "Oct", "Nov", "Dec"};

/* Returns whether we are in a new week.  If so, modifies globaldate
   and globalseed and clears seen as a side effect. NB: a new week is
   defined to start 8AM GMT on Sunday. */
fl_bool checknewweek(void)
{
  struct tm *t;
  time_t tloc;
  char s[100];

  /* get Greenwich Mean Time */
  time(&tloc);
  t = gmtime(&tloc);

  /* turn the clock back to Sunday */
  if (t->tm_wday > 0 || (t->tm_wday == 0 && t->tm_hour < 8)) {
    tloc -= 24 * 60 * 60 * ((t->tm_wday == 0) ? 7 : t->tm_wday);
    t = gmtime(&tloc);
  }

  sprintf(s, "%d%s%02d", t->tm_mday, months[t->tm_mon], t->tm_year % 100);
  if (streq(s, globaldate)) return FALSE;

  setglobaldate(s);
  setpreviousglobaldate();
  TclSet("globaldate", s);
  TclSet("previousglobaldate", previousglobaldate);

#ifdef LOGINSERVER
  /* after 30 min, copy results and password file to rawresults directory */
  TclDo3("after [expr 1800 * 1000] { dumppw; copyresultfiles ",
	 previousglobaldate, "}");
#endif

#ifdef LOGINSERVER
  /* after 1 hour, discard info from the previous week (e.g. who's
     seen what hands) */
  TclDo3("after [expr 3600 * 1000] {discard_data_except_from ",
	 globaldate, "}");
#else
  /* Note that the s represents the current week, but *t
     represents the previous week (because of the call to
     setpreviousglobaldate()).  Unintended, but OK. */
  reseed(s, t->tm_mday * 10000 + t->tm_mon * 100 + t->tm_year);
  setglobalseed(getseed());
  resetseen();
#endif
  return TRUE;
}


/* given a date, give back a date string for one week prior */
static char *calculatepreviousdate(struct tm *t)
{
  time_t tloc;
  char datestring[100];

  tloc = mktime(t);
  
  /* go back a week */
  tloc -= 24 * 60 * 60 * 7; 

  /* turn the clock back to Sunday */
  if (t->tm_wday > 0 || (t->tm_wday == 0 && t->tm_hour < 8))
    tloc -= 24 * 60 * 60 * ((t->tm_wday == 0) ? 7 : t->tm_wday);
  t = gmtime(&tloc);


  /* compute the string */
  sprintf(datestring,
	  "%d%s%02d",
	  t->tm_mday, months[t->tm_mon], t->tm_year % 100);
  return STRDUP(datestring);
}


static void setpreviousglobaldate(void)
{
  struct tm *t;
  time_t tloc;

  time(&tloc);
  t = gmtime(&tloc);


  sprintf(previousglobaldate,
	  calculatepreviousdate(t));
}


#ifdef RES2HTML
/* Return the global date of i weeks ago. */
char *pastglobaldate(int i)
{
  struct tm *t;
  time_t tloc;
  static char s[100];

  /* get Greenwich Mean Time */
  time(&tloc);
  t = gmtime(&tloc);
  tloc -= 24 * 60 * 60 * 7 * i; /* go back i weeks */

  /* turn the clock back to Sunday */
  if (t->tm_wday > 0 || (t->tm_wday == 0 && t->tm_hour < 8))
    tloc -= 24 * 60 * 60 * ((t->tm_wday == 0) ? 7 : t->tm_wday);
  t = gmtime(&tloc);

  sprintf(s, "%d%s%02d", t->tm_mday, months[t->tm_mon], t->tm_year % 100);
  return s;
}
#endif

/* Returns, in a static character string, the GM date and time */
/* Currently used only for generating random number seeds */
char *date_and_time(void)
{
  struct tm *t;
  time_t tloc;
  static char s[100];

  /* get Greenwich Mean Time */
  time(&tloc);
  t = gmtime(&tloc);

  sprintf(s, "%d%s%d %2d:%2d:%2d", t->tm_mday, months[t->tm_mon], t->tm_year,
	  t->tm_hour, t->tm_min, t->tm_sec);
  return s;
}

#ifdef SERVER
char *localtime_as_string(void)
{
  time_t tloc;
  time(&tloc);
  return asctime(localtime(&tloc));
}
#endif
