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

/* find a bunch of digits followed by '\\'; return the integer, advance *s */
int parseint(char **s)
{
  int r = 0;

  while (isdigit(**s)) { r = r * 10 + (**s - '0'); ++*s; }
  assert(**s == '\\');
  ++*s;
  return r;
}

/* If s is "yes" or "no" or a prefix of same, or "0" or "1", put its
   fl_boolean value in *b and return TRUE.  Otherwise, i.e., on failure,
   return FALSE. */
fl_bool parsebool(char *s, fl_bool *b)
{
  s = trim(upcase(s));
  if (strlen(s) == 0) return FALSE;
  if (strneq(s, "YES", strlen(s))) return (*b = TRUE);
  if (strneq(s, "NO", strlen(s))) return ((*b = FALSE), TRUE);
  if (streq(s, "1")) return (*b = TRUE);
  if (streq(s, "0")) return ((*b = FALSE), TRUE);
  return FALSE;
}

/* (non-destructively) remove any leading or trailing whitespace */
/* return value is marked as garbage */
char *trim(const char *s)
{
  return TclDo3("string trim {", braceclean(s), "}");
}

/* copy at most n characters; always null terminate (so s1 may get
   n characters and then a null) */
char *nstrncpy(char *s1, char *s2, int n)
{
  int i;
  char *r;

  for (i=0, r=s1; i<n; i++) if ((*r++ = *s2++) == '\0') return s1;
  *r = '\0';
  return s1;
}

char *tempcat(char *s1, char *s2)
{
  return TEMPCAT(s1, s2);
}

/* because some systems supposedly don't have strcat() return s1 */
char *floaterstrcat(char *s1, const char *s2)
{
  strcat(s1, s2);
  return s1;
}

/* Allocate enough storage for a copy of s, make the copy into that
   space, and return it. */
char *floaterstrdup(const char *s)
{
  return strcpy(salloc((strlen(s) + 1) * sizeof(char)), s);
}

/* Returns whether s and t are the same, ignoring case differences. */
fl_bool strcaseeq(char *s, char *t)
{
  int i = 0;

  while (tolower(s[i]) == tolower(t[i]))
    if (s[i] == '\0') return TRUE;
    else i++;
  return FALSE;
}

/* If name is length 0 return name; otherwise copy name and change
   lower case to upper and mark return value as temp. */
char *upcase(char *name)
{
  char *s, *t;

  if (name[0] == '\0') return name;

  for (s = t = TEMPCOPY(name); *s != '\0'; s++)
    if (isalpha(*s)) *s = toupper(*s);
  return t;
}

/* analogous to upcase() */
char *downcase(char *name)
{
  char *s, *t;

  if (name[0] == '\0') return name;

  for (s = t = TEMPCOPY(name); *s != '\0'; s++)
    if (isalpha(*s)) *s = tolower(*s);
  return t;
}

/* strncpy(), but always put a null character at the end of dest */
char *strncpy2(char *dest, char *src, int n)
{
  strncpy(dest, src, n);
  dest[n] = '\0';
  return dest;
}

/* the last thing in s is a number---return it */
/* if s is the empty string, return 0 */
int lastnum(char *s)
{
  char *t;

  if (strlen(s) == 0) return 0;
  for (t = s + strlen(s) - 1; isdigit(*t) && t >= s; t--);
  if (!isdigit(*t)) t++;
  assert(isdigit(*t));
  return atoi(t);
}
/* the last thing in s is a number followed by a backslash---return the num */
/* EXCEPT if s is the empty string, return 0 */
int lastnumslash(char *s)
{
  char *t;

  if (strlen(s) == 0) return 0;
  for (t = s + strlen(s) - 2; isdigit(*t) && t > s; t--);
  if (!isdigit(*t)) t++;
  assert(isdigit(*t));
  return atoi(t);
}

/* s should be a list of backslash separated integers.  Return the
   maximum of the list.  If the list is empty, return 0. */
int maxnumslash(char *s)
{
  int n, m;
  fl_bool inited = FALSE;

  while (isdigit(*s)) {
    n = parseint(&s);
    if (inited) {
      if (n > m) m = n;
    } else {
      m = n;
      inited = TRUE;
    }
  }
  return (inited ? m : 0);
}

/* reverse of atoi(); marks return value as temporary */
char *itoa(int n)
{
  char t[100];

  sprintf(t, "%d", n);
  return TEMPCOPY(t);
}

/* itoa() for doubles. */
char *dtoa(double f)
{
  char t[100];

  sprintf(t, "%g", f);
  return TEMPCOPY(t);
}

char *maybe_plural(const char *num, const char *thing)
{
  if (streq(num, "1") || streq(num, "one") || streq(num, "One"))
    return TEMPCAT3(num, " ", thing);
  else
    return TEMPCAT4(num, " ", thing, "s");
}

/* Returns the first word in *s, destructively modifying s in the process.
   Return value is always *s; the string's first space is set to '\0';
   *s points to the character after the first space. */
char *nextword(char **s)
{
  char *ret, *t;

  ret = t = *s;
  if (ret == NULL) return NULL;
  while ((*t != '\0') && !isspace(*t)) t++;
  if (*t != '\0') *t++ = '\0';
  *s = t;
  return ret;
}

/* Replace all backslashes in s with slashes. */
char *destructiveBackslashToSlash(char *s)
{
  char *r = s;

  while (*r != '\0') if (*r == '\\') *r++ = '/'; else r++;
  return s;
}


/* Replace '{' and '}' with ' '. */
char *bracetospace(char *s)
{
  char *r = s;
  while (*r != '\0') if (*r == '{' || *r == '}') *r++ = ' '; else r++;
  return s;
}

#ifdef NEED_STRNCASECMP
int strncasecmp(char *s1, char *s2, int n)
{
  unsigned char c1, c2;
  int t;

  while (n-- > 0) {
    c1 = (unsigned char)toupper(*s1);
    c2 = (unsigned char)toupper(*s2);
    if ((t = (int)c1 - (int)c2) != 0) return t;
    s1++;
    s2++;
  }
  return 0;
}
#endif /* NEED_STRNCASECMP */

#if 0
/* SRC is an array of N characters.  Write the chars to a file called DEST. */
void putfile(char *src, int N, char *dest)
{
  FILE *f;
  int i;
  if ((f = fopen(dest, "w")) == NULL)
    fatal(TEMPCAT3("Unable to open `", dest, "' for writing"), 1);
  for (i = 0; i < N; i++)
    fputc(src[i], f);
  if (fclose(f) != 0)
    fatal(TEMPCAT3("Unable to close `", dest, "'"), 1);
}

/* Convert the contents of the named file to C source code for an array
   containing the same characters.  Writes to stdout. */
void getfile(char *filename)
{
  FILE *f;
  int count = 0, ch;
  if ((f = fopen(filename, "r")) == NULL)
    fatal(TEMPCAT3("Unable to open `", filename, "' for reading"), 1);
  puts("char s[] = {");
  if ((ch = getc(f)) != EOF) {
    printf("%x", ch);
    count++;
    while ((ch = getc(f)) != EOF) {
      printf(", %x", ch);
      if (count++ % 10 == 0)
	putchar('\n');
    }
  }
  printf(";\nsize_t N = %d;\n", count);
  if (fclose(f) != 0)
    fatal(TEMPCAT3("Unable to close `", filename, "'"), 1);
}
#endif /* 0 */

