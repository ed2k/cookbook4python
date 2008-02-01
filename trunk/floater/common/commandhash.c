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
#include "commandhash.h"
#include "tickler.h"

#define HASHSIZE 1997

#define NOT_ABBREV 0
#define ABBREV 1
#define AMBIGUOUS_ABBREV 2
typedef struct hashentry {
  char kind; /* NOT_ABBREV, ABBREV, or AMBIGUOUS_ABBREV */
  char *s;
  commandptr binding;
  struct hashentry *next;
} hashentry; 

static fl_bool inited = FALSE;
hashentry *hashtable[HASHSIZE];


int hashnumber(char *s)
{
  unsigned long h = 0, b;
  int i = 0;

  while (*s) {
    b = toupper(*s);
    h += (b << (i++ % 23));
    s++;
  }
  return h % HASHSIZE;
}

hashentry *hash(char *s)
{
  hashentry *e;

/*  printf("hash(%s)\n", s);*/

  s = upcase(s);
  for (e = hashtable[hashnumber(s)]; e != NULL; e = e->next)
    if (streq(s, e->s)) return e;

  return NULL;
}

/* s is a string containing a command name followed by optional arguments */
/* return NULL, AMBIGUOUS, or a char pointer or function pointer */
/* ignore optional args---they are handled by callee */
commandptr commandhash(char *s)
{
  int i;
  hashentry *e;
  char c;

/*  printf("commandhash(%s)\n", s);*/

  for (i=0; iscmd(s[i]); i++);
  
/*  printf("i = %d; s[i] = %d\n", i, s[i]);*/

  /* crap at end of command name? */
  if (s[i] != '\0' && !isspace(s[i])) return NULL;
  
  c = s[i];
  s[i] = '\0';
  e = hash(s);
  s[i] = c;
  if (e == NULL) return NULL;
  else if (e->kind == AMBIGUOUS_ABBREV) return (commandptr) AMBIGUOUS;
  else return e->binding;
}

/* insert a binding at the front of hash list t */
void insertbinding(char *s, commandptr binding, int t, int kind)
{
  hashentry *e;

  e = alloc(hashentry);
  e->kind = kind;
  e->s = STRDUP(upcase(s));
  e->binding = binding;
  e->next = hashtable[t];
  hashtable[t] = e;
}

/* set up binding for an abbreviation of a command name */ 
void bindabbrev(char *s, commandptr binding)
{
  hashentry *e;

  e = hash(s);
  if (e == NULL) insertbinding(s, binding, hashnumber(s), ABBREV);
  else if (e->kind == ABBREV) e->kind = AMBIGUOUS_ABBREV;
  /* if e->kind was AMBIGUOUS_ABBREV or NOT_ABBREV, do nothing */
}

/* The old way is a hack based on assuming that function pointers are always
even.  If that turns out to be false, we switch to the new way, which is
slower but always works. */ 
fl_bool using_old_isstringcommand = FALSE; /* bug fix: TRUE may not properly be converted to FALSE, so always use FALSE, which is safe */

/* binding may be a string or a command --- ISSTRINGCOMMAND can tell you */
void cmdbind(char *s, commandptr binding)
{
  int i;

  if (!inited) {
    for (i=0; i<HASHSIZE; i++) hashtable[i] = NULL;
    inited = TRUE;
  }

  if (using_old_isstringcommand &&
      NEW_ISSTRINGCOMMAND(binding) && !OLD_ISSTRINGCOMMAND(binding))
    using_old_isstringcommand = FALSE;

  /* make binding for s */
  insertbinding(s, binding, hashnumber(s), NOT_ABBREV);

  s = STRDUP(s); /* s must be modifyable for following loop to run */

  /* make binding for abbreviations of s */
  for (i = strlen(s) - 1; i > 0; i--) {
    s[i] = '\0';
    bindabbrev(s, binding);
  }

  free(s);
}

/* bind s to binding (both are strings) */
void stringbind(char *s, char *binding)
{
  char *new;
  char t[100];
  
  /* set up a copy of binding at an odd address */
  new = salloc(strlen(binding) + 2);
  new++;
  strcpy(new, binding);
  TclDo3("uplevel #0 {set {isstringcommand(", itoa((long)new), ")} 1}");
  
  cmdbind(s, (commandptr) new);
}

void entrydump(hashentry *e)
{
  printf("%s -> %s (%s)\n",
	 e->s,
	 ISSTRINGCOMMAND(e->binding) ? (char *) e->binding : "<function>",
	 (e->kind == NOT_ABBREV) ? "not abbrev" :
	 ((e->kind == ABBREV) ? "abbrev" : "ambiguous abbrev"));
}
	 

void hashdump(void)
{
  int i;
  hashentry *e;

  for (i=0; i<HASHSIZE; i++)
    for (e=hashtable[i]; e != NULL; e=e->next)
      entrydump(e);
}
      
 
