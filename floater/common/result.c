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
#include "br.h"
#include "comm.h"
#include "score.h"

#ifdef RESULTSERVER
fl_bool startingup = TRUE;
#endif

#define SHORT_IMPS_FORMAT "%.1f"
#define IMPS_FORMAT "%.1f IMPs"
#define MPS_FORMAT "%.1f%%"

/* e.g. "8Oct95IMP17" => 1 (because IMP is scoringmethod 1) */
/* form of scoring is expected to end in a letter */
static int handscoring(char *handname)
{
  int i;
  char *scoringname;

  while (isdigit(*handname)) handname++;
  while (isalpha(*handname)) handname++;
  while (isdigit(*handname)) handname++;
  scoringname = TEMPCOPY(handname);
  for (i = 0; !isdigit(scoringname[i]); i++);
  scoringname[i] = '\0'; /* truncate at first digit */
  for (i = 0; i < numscoringmethods; i++)
    if (streq(scoringmethods[i].scoringname, scoringname)) return i;

  return -1;
}


typedef struct resultlist {
  char *handname;
  int NSpoints;
  char *auction;
  char *play;
  char *scoring;
  char *north;
  char *south;
  char *east;
  char *west;
  char *msg;
  float score; /* % of MPs or number of IMPs */
  char *scorestring; /* stringified version of score (e.g. "56.2%") */
  struct resultlist *next;
} resultlist;

typedef struct resultlistlist {
  char *handname;
  resultlist *r;
  struct resultlistlist *next;
} resultlistlist;

static resultlistlist *rll = NULL;

/* return the length of a resultlist */
static int resultlistlength(resultlist *l)
{
  int i = 0;

  while (l != NULL) {
    l = l->next;
    i++;
  }
  return i;
}

/*
  Return a pointer to the head of the result list for the named hand
  by searching the global resultlistlist.  If no results exist for
  this hand and can_create, create an entry in the global resultlistlist.
*/
static resultlist **findresultlist2(char *handname, char can_create)
{
  resultlistlist *s;

  for (s = rll; s != NULL; s = s->next)
    if (streq(s->handname, handname)) return &(s->r);

  if (!can_create) return NULL;
  s = alloc(resultlistlist);
  s->handname = STRDUP(handname);
  s->r = NULL;
  s->next = rll;
  rll = s;
  return &(s->r);
}
#define findresultlist(h) findresultlist2((h), TRUE)
#define findexistingresultlist(h) findresultlist2((h), FALSE)

/*
  Score the list of results, at either IMPs or MPs---set (overwrite) the
  score and scorestring fields of each element of r.
*/
static void score(resultlist *r, int scoring)
{
  int size, i, *scores;
  float *fscores;
  resultlist *t;
  char s[100];

  if (playinghearts) return;  /* xxx */

  /* copy scores in r into an array of ints of the appropriate size */
  for (size=0, t=r; t != NULL; t=t->next) size++;
  assert(size > 0);
  scores = (int *) salloc(sizeof(int) * size);
  for (i=0, t=r; i<size; t=t->next) scores[i++] = t->NSpoints;
  
  fscores = (float *) salloc(sizeof(float) * size); /* destination array */
  if (scoring == 0) IMPscore(scores, size, fscores);
  else if (scoring == 1) MPscore(scores, size, fscores);
  else if (scoring == 2) /* YUK3score(scores, size, fscores) */;
  else assert(0);

  /* copy result back into resultlist */
  for (i=0; i<size; i++, r=r->next) {
    r->score = fscores[i];
    if (streq(scoringmethods[scoring].scoringname, "IMP"))
      sprintf(s, IMPS_FORMAT, r->score);
    else
      sprintf(s, MPS_FORMAT, r->score);
    reset(r->scorestring);
    r->scorestring = STRDUP(s);
  }
  free(scores);
  free(fscores);
}

/* Search the list r and return whether this identical result appears. */
static char oldresult(resultlist *r, char *handname,
		      int NSpoints, char *auction, char *play,
		      char *scoring, char *north, char *south,
		      char *east, char *west)
{
  while (r != NULL)
    if (r->NSpoints == NSpoints &&
	streq(r->north, north) &&
	streq(r->south, south) &&
	streq(r->east, east) &&
	streq(r->west, west) &&
	streq(r->auction, auction) &&
	streq(r->play, play) &&
	streq(r->scoring, scoring) &&
	streq(r->handname, handname)) return TRUE;
    else
      r = r->next;

  return FALSE;
}

/*
  If we've seen this exact result before, do nothing; otherwise insert it
  into the given resultlist.
*/
static void add_result_to_resultlist(resultlist **r, char *handname,
				     int NSpoints, char *auction, char *play,
				     char *scoring, char *north, char *south,
				     char *east, char *west, char *msg)
{
  resultlist *new, **origr;

  origr = r;

  if (oldresult(*r, handname, NSpoints, auction, play, scoring,
		north, south, east, west)) return;
  /* if we get here, we have not seen the result before */
  while (*r != NULL && (*r)->NSpoints > NSpoints) r = &((*r)->next);
  new = alloc(resultlist);
#ifdef RESULTSERVER
  new->msg = STRDUP(msg);
#endif

  new->handname = STRDUP(handname);
  new->NSpoints = NSpoints;
  new->auction = STRDUP(auction);
  new->play = STRDUP(play);
  new->scoring = STRDUP(scoring);
  new->north = STRDUP(north);
  new->south = STRDUP(south);
  new->east = STRDUP(east);
  new->west = STRDUP(west);

#ifndef RESULTSERVER
#ifdef RES2HTML
  TclDo5("gotresult {", handname_to_setname(handname), "} {", handname, "}");
  {
    char s[300];
    sprintf(s, "{%s} {%s} {%s} {%s}", outputname(east), outputname(west),
	    outputname(north), outputname(south));
    TclDo2("gotnames ", s);
  }
#endif
#endif
  new->scorestring = NULL;
  new->next = *r;
  *r = new;
}

void scoreall(void)
{
  resultlistlist *a;

  for (a = rll; a != NULL; a = a->next) score(a->r, handscoring(a->handname));
}

/* print all the results we know about */
void dumpresults(void)
{
  resultlistlist *a;
  resultlist *b;
  char *s, *t;

  for (a = rll; a != NULL; a = a->next) {
    resultmsg3("-+> ", a->handname, " <+-");
    for (b = a->r; b != NULL; b = b->next) {
      resultdesc(a->handname, b->NSpoints, b->auction, b->play,
		 b->north, b->south, b->east, b->west, b->scorestring, &s, &t,
		 NULL);
      resultmsg(s);
      resultmsg(t);
      free(s);
      free(t);
    }
  }
}

/* Did name ever play this hand as east or west? */
static fl_bool eastwestsearch(char *name, resultlist *l)
{
  while (l != NULL)
    if (streq(outputname(l->east), name) || streq(outputname(l->west), name))
      return TRUE;
    else
      l = l->next;

  return FALSE;
}

/* change "31.0%" to "69.0%" or change "3.4 IMPs" to "-3.4 IMPs" */
static char *negate_string(char *s)
{
  char t[100];
  float f;

  sscanf(s, "%f", &f);
  if (isin('%', s)) 
    sprintf(t, MPS_FORMAT, 100.0 - f);
  else
    sprintf(t, IMPS_FORMAT, (f == 0.0 ? 0.0 : -f));
  return TEMPCOPY(t);
}

#define reverse_init() TclDo("reverse_init")
#define reverse_print(l) TclDo3("reverse_print {", (l), "}")
#define reverse_done() TclDo("reverse_done")

/* print all results for the given hand */
void dumpresult(char *handname)
{
  resultlistlist *a;
  resultlist *b;
  char *s, *t;
  fl_bool negate;

  for (a = rll; a != NULL; a = a->next) if streq(handname, a->handname) {
    score(a->r, handscoring(handname));
    negate = eastwestsearch(myoutputname, a->r);
#if DEBUGRES
    resultmsg(negate ? "Displaying from EW point of view"
	      : "Displaying from NS point of view");
#endif
    resultmsg3("-+> ", a->handname, " <+-");
    if (negate) reverse_init();
    for (b = a->r; b != NULL; b = b->next) {
      int points = (negate ? -(b->NSpoints) : b->NSpoints);
      char *scorestr = (negate ? negate_string(b->scorestring)
			: b->scorestring);

      resultdesc(a->handname, points, b->auction, b->play,
		 b->north, b->south, b->east, b->west, scorestr, &s, &t, NULL);
      if (negate) {
	reverse_print(resultmsg_(t));
	reverse_print(resultmsg_(s));
      } else {
	resultmsg(s);
	resultmsg(t);
      }
      free(s);
      free(t);
    }
    if (negate) reverse_done();
    return;
  }
  resultmsg2("No others' results available for ", handname);
}

/*
  If we've seen this exact result before, do nothing; otherwise insert it
  into the appropriate result list.
*/
static void addresult(char *handname, int NSpoints, char *auction, char *play,
		      char *scoring,
		      char *north, char *south, char *east, char *west,
		      char *msg)
{
  add_result_to_resultlist(findresultlist(handname),
			   handname, NSpoints, auction, play, scoring,
			   north, south, east, west, msg);
}

/* called when a result arrives */
void result(char *handname, int NSpoints, char *auction, char *play,
	    char *scoring, char *north, char *south, char *east, char *west,
	    char *msg)
{
#if DEBUGRES
  {
    char s[1000];

    sprintf(s, "%s %d %s %s|%s|%s|%s", handname, NSpoints, scoring,
	    north, south, east, west);
    DEBUG(status(s));
  }
#endif

#ifdef RESULTSERVER
  if (!startingup)
    TclDo5("saveresult ", handname_to_weekname(handname), " {", msg, "}");
#endif

  /* add to our list of results */
  addresult(handname, NSpoints, auction, play, scoring,
	    north, south, east, west, msg);
}

#ifdef RESULTSERVER
void resultrequest(char *handname, char *who)
{
  resultlistlist *a;
  resultlist *b;
  static char s[10000];
  int len, num, scoring;

  for (a = rll; a != NULL; a = a->next) if streq(handname, a->handname) {
    if ((b = a->r) == NULL) break;
    strcpy(s, b->msg);
    while ((b = b->next) != NULL) {
      len = strlen(s);
      if (len < 9800) sprintf(s + len, "\t%s", b->msg);
      else {
	sendmessage(who, makemsg4(RESULT_DUMP, handname, s, "", ""));
	strcpy(s, b->msg);
      }
    }
    sendmessage(who, makemsg4(RESULT_DUMP, handname, s, "", ""));
  }
  num = handnum(handname);
  scoring = handscoring(handname);
  sendmessage(who, makemsg4(RESULT_DUMP, handname, "done",
			    itoa(scoring), itoa(num)));
}
#endif

/* Takes a result and outputs a two line description.  Output is
stored in *out1 and *out2, which must be freed by the caller.  Args
should be self-explanatory except for scorestring, which is a
textual description of a result (e.g. "+5 IMPs") or NULL. */
void resultdesc(char *name, int points, char *auction, char *play,
		char *north, char *south, char *east, char *west,
		char *scorestr, char **out1, char **out2, int *dealerp)
{
  char s[1000];
  int dealer;

  dealer = (dealerp == NULL) ? DEALNO_TO_DEALER4(handnum(name)) : *dealerp;
  if (passedout(auction)) {
    if (strexists(scorestr)) {
      sprintf(s, "Passed out, (%s)", scorestr);
    } else {
      sprintf(s, "Passed out");
    }
  } else if (strlen(play) >= 2 && (play)[0] != '!') {
    if (strexists(scorestr)) {
      sprintf(s, "%s by %c, %+d (%s); %s; %c%c led",
	      markgarbage(contract(auction)),
	      declarer(auction, dealer), points,
	      scorestr, prettyauction(auction),
	      toupper((play)[0]), toupper((play)[1]));
    } else {
      sprintf(s, "%s by %c, %+d; %s; %c%c led",
	      markgarbage(contract(auction)),
	      declarer(auction, dealer), points,
	      prettyauction(auction),
	      toupper((play)[0]), toupper((play)[1]));
    }
  } else {
    if (strexists(scorestr)) {
      sprintf(s, "%s by %c, %+d (%s); %s; claimed before lead",
	      markgarbage(contract(auction)), declarer(auction, dealer),
	      points, scorestr, prettyauction(auction));
    } else {
      sprintf(s, "%s by %c, %+d; %s; claimed before lead",
	      markgarbage(contract(auction)), declarer(auction, dealer),
	      points, prettyauction(auction));
    }      
  }
  *out1 = STRDUP(s);
  if (out2 != NULL) {
    sprintf(s, "  %s(N) %s(S) %s(E) %s(W)",
	    outputname(north), outputname(south),
	    outputname(east), outputname(west));
    *out2 = STRDUP(s);
  }
}

#ifdef RES2HTML
static double decode(char *scorestr)
{
  extern double strtod();

  return strtod(scorestr, NULL);
}

/* Skip any initial whitespace, but truncate at next whitespace (if any). */
/* Result should be at most 5 characters long. */
static char *shortscore(char *score)
{
  static char t[100];
  int i;

  strcpy(t, score);
  i = 0;
  while (isspace(t[i])) i++;
  assert(t[i] != '\0'); /* t should have something in it */
  for (i++; t[i] != '\0'; i++)
    if (t[i] == ' ') {
      t[i] = '\0';
      break;
    }
  return (streq(t, "100.0%") ? "100%" : t);
}

static char *recode(double f, char *samplescore)
{
  static char t[100];

  if (isin('%', samplescore))
    sprintf(t, MPS_FORMAT, f);
  else
    sprintf(t, SHORT_IMPS_FORMAT, f);
  return shortscore(t);
}

/* Find out how well name did, on average, over the given set.
   Returns a string, e.g., "51.9%" or "0.2" */
char *gettotal(char *name, char *setname)
{
  resultlistlist *a;
  resultlist *b;
  fl_bool negate, found;
  int numres = 0;
  double sumres = 0.0;
  char *samplescore = NULL;

  for (a = rll; a != NULL; a = a->next)
    if streq(setname, handname_to_setname(a->handname)) {
      /* find name's (first) result on this board, if any */
      for (negate = found = FALSE, b = a->r; b != NULL; b = b->next) {
	if (streq(outputname(b->east), name)
	    || streq(outputname(b->west), name)) {
	  negate = found = TRUE;
	  break;
	} else if (streq(outputname(b->south), name)
		   || streq(outputname(b->north), name)) {
	  found = TRUE;
	  break;
	}
      }
      if (found) {
	if (samplescore == NULL) samplescore = b->scorestring;
	numres++;
	sumres +=
	  decode(negate ? negate_string(b->scorestring) : b->scorestring);
      }  
    }

  return (numres == 0 ? "" : recode(sumres / numres, samplescore));
}

/* To speed this up, we should have a hash table from name/handname
pairs to the relevant result.  That would also help getscore(), below. */
char *getscorestr(char *name, char *setname, int handnum)
{
  resultlistlist *a;
  resultlist *b;
  char handname[100];

  sprintf(handname, "%s%d", setname, handnum);
  for (a = rll; a != NULL; a = a->next)
    if streq(handname, a->handname) {
      for (b = a->r; b != NULL; b = b->next) {
	if (streq(outputname(b->east), name)
	    || streq(outputname(b->west), name)) {
	  return shortscore(negate_string(b->scorestring));
	} else if (streq(outputname(b->south), name)
		   || streq(outputname(b->north), name)) {
	  return shortscore(b->scorestring);
	}
      }
      return "";
    }
  return "";
}

char *getscore(char *name, char *setname, int handnum)
{
  resultlistlist *a;
  resultlist *b;
  char handname[100];
  static char r[10];

  sprintf(handname, "%s%d", setname, handnum);
  for (a = rll; a != NULL; a = a->next)
    if streq(handname, a->handname) {
      for (b = a->r; b != NULL; b = b->next) {
	if (streq(outputname(b->east), name)
	    || streq(outputname(b->west), name)) {
	  sprintf(r, "%d", -b->NSpoints);
	  return r;
	} else if (streq(outputname(b->south), name)
		   || streq(outputname(b->north), name)) {
	  sprintf(r, "%d", b->NSpoints);
	  return r;
	}
      }
      return "";
    }
  return "";
}

#define result_for_name(name, r)			\
    ({							\
      char _u[1000], _t[10];				\
      sprintf(_u, "{r%s(%s)}", (name), (r)->handname);	\
      sprintf(_t, "%ld", (long)r);			\
      TclSet(_u, _t);					\
    })

#define get_result_for_name(name, handname)		\
    ({							\
      char _u[1000];					\
      sprintf(_u, "{r%s(%s)}", (name), (handname));	\
      (resultlist *) atoi(TclGet(_u));			\
    })

void index_results(void)
{
  resultlistlist *a;
  resultlist *b;

  for (a = rll; a != NULL; a = a->next)
    for (b = a->r; b != NULL; b = b->next) {
      result_for_name(outputname(b->east), b);
      result_for_name(outputname(b->west), b);
      result_for_name(outputname(b->south), b);
      result_for_name(outputname(b->north), b);
    }  
}

/* Sets *s to a desription of the contract, auction, and result.
   Sets northname, southname, eastname, and westname as you would expect.
   Caller should not free any of the returned values.  Caller may
   assume returned values are usable until the next call to this function. */
void retrieve_result(char *setname, char *name, int handnum,
		     char **s, char **northname, char **southname,
		     char **eastname, char **westname)
{
  resultlist *b;
  char handname[100], *scorestr;
  static char x[1000];
  fl_bool negate;

  sprintf(handname, "%s%d", setname, handnum);
  b = get_result_for_name(name, handname);
  assert(b != NULL);
  negate = (streq(outputname(b->east), name)
	    || streq(outputname(b->west), name));
  if (negate) sprintf(x, "%s to EW", negate_string(b->scorestring));
  else sprintf(x, "%s to NS", b->scorestring);
  resultdesc(b->handname, b->NSpoints, b->auction, b->play,
	     b->north, b->south, b->east, b->west, x, s, NULL, NULL);
  *northname = outputname(b->north);
  *southname = outputname(b->south);
  *eastname = outputname(b->east);
  *westname = outputname(b->west);
  strcpy(x, *s);
  free(*s);
  *s = x;
}

void *handdumpiter(char *setname, int handnum)
{
  char handname[200];
  resultlistlist *a;

  sprintf(handname, "%s%d", setname, handnum);

  for (a = rll; a != NULL; a = a->next)
    if streq(handname, a->handname) return (void *) a->r;

  return NULL;
}

void *handdumpnext(void *iter)
{
  return (void *) (((resultlist *)iter)->next);
}

void handdumpfetch(char **s, char **northname, char **southname,
		   char **eastname, char **westname, void *iter)
{
  resultlist *b = (resultlist *) iter;
  static char x[1000];

  resultdesc(b->handname, b->NSpoints, b->auction, b->play,
	     b->north, b->south, b->east, b->west, b->scorestring, s, NULL,
	     NULL);
  *northname = outputname(b->north);
  *southname = outputname(b->south);
  *eastname = outputname(b->east);
  *westname = outputname(b->west);
  strcpy(x, *s);
  *s = x;
}
#endif /* RES2HTML */
