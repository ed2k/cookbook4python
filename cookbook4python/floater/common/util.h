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

/* util.h - various utilities that have no better home */

#ifndef __UTIL_H__
#define __UTIL_H__

/* These all-uppercase string routines act like their same-named
 * lower-case functions except that they mark the result as garbage.
 * See alloc.h for information about garbage memory. */

char *floaterstrcat(char *, const char *);
char *tempcat(char *, char *);

#define STRDUP floaterstrdup
#define TEMPCOPY(s) markgarbage(STRDUP(s))

   
#define STRNDUP(s, N) (nstrncpy(salloc(((N) + 1) * sizeof(char)), (s), (N)))

#define STRCPY(dest, src) { assert(strlen(src) < sizeof(dest)); \
			      strcpy(dest, src); }
#define STRCAT(a, b) floaterstrcat(STRNDUP((a), (strlen(a) + strlen(b))), (b))
#define STRCAT4(a, b, c, d) floaterstrcat(floaterstrcat(floaterstrcat(STRNDUP((a), (strlen(a) + strlen(b) + strlen(c) + strlen(d))), (b)), (c)), (d))
#define TEMPCAT(a, b) markgarbage(floaterstrcat(STRNDUP((a), (strlen(a) + strlen(b))), (b)))
#define TEMPCAT3(a, b, c) markgarbage(floaterstrcat(floaterstrcat(STRNDUP((a), (strlen(a) + strlen(b) + strlen(c))), (b)), (c)))
#define TEMPCAT4(a, b, c, d) markgarbage(STRCAT4(a, b, c, d))

/* copy at most n characters; always null terminate (so s1 may get
   n characters and then a null) */
char *nstrncpy(char *s1, char *s2, int n);

char *trim(const char *s);
char *gstrcpy(char *s1, char *s2);
int lastnum(char *s);
int lastnumslash(char *s);
int maxnumslash(char *s);
fl_bool parsebool(char *s, fl_bool *b);
int parseint(char **s);
char *destructiveBackslashToSlash(char *s);
#define itoa floater_itoa
#define dtoa floater_dtoa
char *itoa(int n);
char *dtoa(double f);
char *upcase(char *name);
char *downcase(char *name);
char *strncpy2(char *dest, char *src, int n);
char *floaterstrdup(const char *s);
fl_bool strcaseeq(char *s, char *t); /* string compare ignoring case */
char *maybe_plural(const char *num, const char *thing);
char *nextword(char **s);
/* int strncasecmp(char *s1, char *s2, int n); */

#define strncpy3(dest, src) strncpy2((dest), (src), sizeof(dest) - 1)

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max3(a, b, c) max(max(a, b), c)
#define max4(a, b, c, d) max(max(a, b), max(c, d))

#define reset(x) ((x) == NULL ? NULL : (free(x), ((x) = NULL)))

/* do nothing for x milliseconds */
#define sleep(x) TclDo2("after ", itoa(x))

/* returns whether the given string is a numeral */
#define isnumeral(s) atob(TclDo3("regexp {^[ ]*(-)?[0-9]+[ ]*$} {", (s), "}"))
#define isnumeralf(s) atob(TclDo3("regexp {^[ ]*(-)?[0-9]+([.][0-9]*)?[ ]*$} {", (s), "}"))

#define charindex(c, s) (strchr(s, c) - s)
#define char_to_rank(c) charindex(toupper(c), card_to_char)

#define strexists(s) ((s) != NULL && (*(s)) != '\0')

#define safestreq(a, b) ((a) != NULL && (b) != NULL && streq((a), (b)))

char *bracetospace(char *s);

#endif
