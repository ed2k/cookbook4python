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
#include <tcl.h>
#include <tk.h>
#include <stdio.h>
#include <time.h>
#include "floater.h"
#include "comm.h"
#include "br.h"
#include "deal.h"
#include "calendar.h"
#if 0
#include "sun.h"
#endif

#define TESTING 0

#define WEEKS 5

/* from result.c */
extern char *gettotal(char *name, char *setname);
extern char *getscorestr(char *name, char *setname, int handnum);
extern char *getscore(char *name, char *setname, int handnum);
extern void index_results(void);
extern void retrieve_result(char *setname, char *name, int handnum,
			    char **s, char **northname, char **southname,
			    char **eastname, char **westname);
extern void handdumpfetch(char **s, char **northname, char **southname,
			  char **eastname, char **westname, void *iter);
extern void *handdumpnext(void *iter);
extern void *handdumpiter(char *setname, int handnum);



static char *vs[4] = {"EW", "None", "Both", "NS"};
static char dlr[4] = {'N', 'E', 'S', 'W'};

FILE *f;

static char *rsdir;
static char *webdir;
static char *rswebdir;
static char *date;

static fl_bool _mono = FALSE;

#define lput(s) fputs((s), f)
#define lputc(c) fputc((c), f)
#define sput(form, s) fprintf(f, (form), (s))
#define addr(s, url) fprintf(f, "<a href=%s>%s</a>", make_url(url), (s))
#define locput(s) sput("<a name=\"%s\"></a>", (s))
#define newline() lput(_mono ? "\n" : "<br>\n")
#define newpar() lput(_mono ? "\n\n" : "<p>\n")
#define strong() lput("<strong>")
#define endstrong() lput("</strong>")
#define bold() lput("<b>")
#define endbold() lput("</b>")
#define large() lput("<H3>")
#define endlarge() lput("</H3>")
#define centered() lput("<center>")
#define endcentered() lput("</center>")
#define large_centered(s) \
    (large(), centered(), lput(s), endcentered(), endlarge())

#define mono(n)								\
     (((n) > 0) ? 							\
      (lput("<pre width="), lput(itoa(n)), lput(">"), _mono = TRUE) :	\
      (lput("<pre>"), _mono = TRUE))
#define endmono() (lput("</pre>"), _mono = FALSE)

static fl_bool _cols = FALSE;
static int _columnswidth;
#define begincolumns(n) (mono(n), _cols = TRUE, columnswidth() = 0)
#define endcolumns() (outputcolumns(), _cols = FALSE, endmono())
#define columnswidth() _columnswidth

#define hrule() lput("<hr>")

#define setloop(s) \
  for (strcpy(s, TclDo("firstset")); strexists(s); strcpy(s, TclDo("nextset")))

#define nameloop(s)				\
  for (strcpy(s, TclDo("firstname"));		\
       strexists(s);				\
       strcpy(s, TclDo("nextname")))

#define namesubloop(s, first, last)					 \
  for (strcpy(s, TclDo5("firstsubname {", (first), "} {", (last), "}")); \
       strexists(s);							 \
       strcpy(s, TclDo("nextsubname")))

#define handloop(s, hn)				\
  for (strcpy(hn, TclDo2("firsthand ", (s)));	\
       strexists(hn);				\
       strcpy(hn, TclDo("nexthand")))

#ifndef even
#define even(x) (((x) & 1) == 0)
#define odd(x) (!even(x))
#endif

#define nameput(name) addr(name, name_to_filename(name))
#define cnameput(name, cname) \
     (streq((name), (cname)) ? lput(name) : nameput(name))

/***************************************************************************/

#define recap_entry0(setname, playername, handnum, str)	\
     ({							\
       char _u[1000];					\
       if (strexists(str)) {				\
	 sprintf(_u, "{recap%s_%s(%d 0)}",		\
		 (setname), (playername), (handnum));	\
	 TclSet(_u, (str));				\
       }						\
     })

#define recap_entry1(setname, playername, handnum, str)	\
     ({							\
       char _u[1000];					\
       if (strexists(str)) {				\
	 sprintf(_u, "{recap%s_%s(%d 1)}",		\
		 (setname), (playername), (handnum));	\
	 TclSet(_u, (str));				\
       }						\
     })

#define get_recap_entry0(setname, playername, handnum)	\
     ({							\
       char _u[1000];					\
       sprintf(_u, "{recap%s_%s(%d 0)}",		\
	       (setname), (playername), (handnum));	\
       TclGet(_u);					\
     })

#define get_recap_entry1(setname, playername, handnum)	\
     ({							\
       char _u[1000];					\
       sprintf(_u, "{recap%s_%s(%d 1)}",		\
	       (setname), (playername), (handnum));	\
       TclGet(_u);					\
     })

#define set_recap_total(setname, playername, total)		\
     recap_entry0(setname, playername, -77, total)
#define get_recap_total(setname, playername)				\
     ({									\
       char _u[1000];							\
       sprintf(_u, "{recap%s_%s(-77 0)}", (setname), (playername));	\
       TclGet(_u);							\
     })

#define set_maxhandnum(setname, n)				\
     ({								\
       char _u[1000];						\
       sprintf(_u, "gset maxhandnum(%s) %d", (setname), (n));	\
       TclDo(_u);						\
     })
#define get_maxhandnum(setname)				\
     ({							\
       char _u[1000];					\
       sprintf(_u, "maxhandnum(%s)", (setname));	\
       atoi(TclGet(_u));				\
     })
       
#define set_leftname(s, n, name)			\
     ({							\
       char _u[1000];					\
       sprintf(_u, "leftname_%s(%d)", (s), (n));	\
       TclSet(_u, (name));				\
     })

#define get_leftname(s, n)				\
     ({							\
       char _u[1000];					\
       sprintf(_u, "leftname_%s(%d)", (s), (n));	\
       TclGet(_u);					\
     })

#define set_rightname(s, n, name)			\
     ({							\
       char _u[1000];					\
       sprintf(_u, "rightname_%s(%d)", (s), (n));	\
       TclSet(_u, (name));				\
     })

#define get_rightname(s, n)				\
     ({							\
       char _u[1000];					\
       sprintf(_u, "rightname_%s(%d)", (s), (n));	\
       TclGet(_u);					\
     })

#define bump_numhands(a, b)						 \
     ({									 \
       char _u[1000], *_a = (a), *_b = (b);				 \
       sprintf(_u, "if [catch {incr {nh%s_%s}}] {set {nh%s_%s} 1}",	 \
	       _a, _b, _a, _b);						 \
       TclDo(_u);							 \
     })

#define get_numhands(a, b)						  \
     ({									  \
       char _u[1000];							  \
       sprintf(_u, "if [catch {set t ${nh%s_%s}}] {set t 0}\n set t", (a), (b)); \
       atoi(TclDo(_u));							  \
     })

/***************************************************************************/

/* Horizontal space via the non-breaking space feature of HTML. */
static void hspace(int i)
{
  while (i-- > 0) lput("&nbsp;");
}

/* Convert a name (which may have spaces) into a suitable filename. */
static char *name_to_filename(char *s)
{
  static char r[200];
  char t[200];
  int i = 0;

  while ((t[i] = s[i]) != '\0') {
    if (t[i] == ' ') t[i] = '+';
    i++;
  }
  sprintf(r, "%s/I/%s.html", rswebdir, t);
  return r;
}

/***************************************************************************
  Columns
***************************************************************************/

static int colx, coly, colx0;

static void outputcolumns()
{
  int mx = atoi(TclDo("maxcol")), my = atoi(TclGet("maxrow"));
  int i, j;
  char x[100];

  for (j = 0; j <= my; j++, newline())
    for (i = 0; i <= mx; i++) {
      sprintf(x, "colget %d %d", i, j);
      lput(TclDo(x));
    }
}

static void newcol(int n)
{
  colx0 = colx = columnswidth();
  coly = 0;
  columnswidth() += n;
}

#define colput0(s) colput(s)
#define colput1(form, s) \
    ({ char _t_[1000]; sprintf(_t_, (form), (s)); colput(_t_); })

static void colput(char *s)
{
  char x[1000];

  if (s == NULL) s = "";
  while (*s != '\0') {
    sprintf(x, "set COL(%d,%d) {%c}", colx++, coly, *s++);
    TclDo(x);
  }
  colx = colx0;
  coly++;
}

static void colputchar(char c)
{
  char x[1000];

  sprintf(x, "set COL(%d,%d) %c", colx++, coly, c);
  TclDo(x);
}

static void colskip(int n)
{
  colx = colx0;
  coly += n;
}

/***************************************************************************/

/* The name of the (html) file we are currently writing. */
static char curfilename[1000];

static int countslashes(char *s)
{
  int i = 0;
  while (*s != '\0') if (*s++ == '/') i++;
  return i;
}

/* Make a url by making a relative filename from curfilename, if possible. */
static char *make_url(char *url)
{
  int i;
  static char r[1000];

  if (strneq(url, rswebdir, i = strlen(rswebdir))) {
    int n = countslashes(curfilename + i + 1);

    if (url[i] == '/') i++;
    r[0] = '\0';
    while (n-- > 0) sprintf(r + strlen(r), "../");
    strcat(r, url + i);
    return r;
  } else return url;
}

static char filename[300];
static char *recapfilename(char *s, int n)
{
  sprintf(filename, "%s/recap%s%d.html", rswebdir, s, n);
  return filename;
}

static char *trecapfilename(char *s, int n)
{
  sprintf(filename, "%s/trecap%s%d.html", rswebdir, s, n);
  return filename;
}

static char *hrfilename(char *s)
{
  sprintf(filename, "%s/hr%s.html", rswebdir, s);
  return filename;
}

static char *fullfilename(char *s)
{
  sprintf(filename, "%s/full%s.html", rswebdir, s);
  return filename;
}


#define STANDARD_HTML_START "<html><head><TITLE>%s</TITLE></head><body>"

#define STANDARD_HTML_END "<HR>This page automatically generated on %s.<hr><a href=\"http://www.floater.org\">Floater Home Page</a></body></html>"


static void htmlfile(char *s, char *name, fl_bool shortname)
{
  char filename[300];

  if (shortname)
    sprintf(filename, "%s/%s.html", rswebdir, s);
  else
    strcpy(filename, s);
  f = fopen(filename, "w");
  printf("Creating file %s (%s)\n", filename, name);
  assert(f != NULL);
  fprintf(f, STANDARD_HTML_START, name);
  strcpy(curfilename, filename);
}

static void htmlend(void)
{
  extern void destroygarbage(void);

  fprintf(f, STANDARD_HTML_END, date);
  fclose(f);
  destroygarbage();
}

/*************************************************************************/

static void gethand(char *setname, int handnum, int *deck)
{
  struct tm handtm, lastweek;
  time_t t;
  int scoremethod;
  char *year2;          /* the year for this hand, in 2 digits */
  int yearsSince1900;   /* the number of years after 1900 that year2
			 * represents */
  

  printf("gethand %s%d\n", setname, handnum);
  TclDo2("parseset ", setname);

  year2 = STRDUP(TclGet("setyear"));
  yearsSince1900 = atoi(TclDo3("expr [yearadjust ", year2, " ] - 1900"));

  /* extract the hand's date */
  handtm.tm_sec = 0;
  handtm.tm_min = 0;
  handtm.tm_hour = 0;
  handtm.tm_mday = atoi(TclGet("setday"));
  handtm.tm_mon = atoi(TclGet("setmonth"));
  handtm.tm_year = yearsSince1900;
  handtm.tm_isdst = 0;

  /* find a date one week past */
  t = mktime(&handtm) - 7*24*60*60;
  gmtime_r(&t, &lastweek);

  /* set the dealer's seed */
  reseed(STRDUP(TclGet("setdate")),
	 ((lastweek.tm_mday * 10000)
	  + (lastweek.tm_mon*100)
	  + lastweek.tm_year));
  scoremethod = atoi(TclGet("setscoremethod"));
  makeseed(getseed(), 2 * handnum + scoremethod);

  /* deal the hand */
  deal(deck, 0, 0, 0, 0, 52);
  sortdeal(deck, 52, 13);
}	 

static void handput(int *hand)
{
  int i, suit, f;

  SUITLOOP (suit) {
    i = 0;
    while (i < 13 && whatsuit(hand[i]) > suit) i++;
    f = i;
    while (i < 13 && whatsuit(hand[i]) == suit) {
      colputchar(card_to_char[whatcard(hand[i])]);
      i++;
    }
    if ((i - f) == 0) colputchar('-');
    colput0("");
  }
}

static void handrecord(char *setname, int handnum)
{
  extern void print_hand();
  int deck[52];

  gethand(setname, handnum, deck);
  /*
  print_hand(deck, handnum - 1, DEALNO_TO_DEALER4(handnum), ONEB(handnum - 1));
  */
  newcol(10);
  colput1("Deal %d", handnum);
  colput1("Dealer %c", dlr[DEALNO_TO_DEALER4(handnum)]);
  colput1("%s vul", vs[ONEB(handnum - 1)]);
  colskip(1);
  handput(deck + 13 * West);
  newcol(10);
  handput(deck + 13 * North);
  colskip(4);
  handput(deck + 13 * South);
  newcol(10);
  colskip(4);
  handput(deck + 13 * East);
}

/***************************************************************************/

static void create_main()
{
  char s[200], r[20], name[200];

  htmlfile("index", "The Floater Results Page", TRUE);
  bold();
  lput("Recent Floater results:");
  endbold();
  newpar();
  setloop(s) {
    printf("index: set %s\n", s);
#if 0
    mono(0);
    sprintf(r, "%12s ", s);
    lput(r);
    endmono();
#endif
    lput("[");
    addr("recap", recapfilename(s, 0));
    lput(" | ");
    addr("transposed recap", trecapfilename(s, 0));
    lput(" | ");
    addr("hand records", hrfilename(s));
    lput(" | ");
    addr("all results", fullfilename(s));
    sput("] for %s", s);
    newline();
  }
  newline();

  bold();
  lput("Result summaries for players: ");
  endbold();
  newpar();
  nameloop(name) {
    addr(name, name_to_filename(name));
    newline();
  }
  htmlend();
}

static int getmaxhandnum(char *set)
{
  int r = 0;
  char handname[200];

  handloop(set, handname) {
    int n = handnum(handname);
    if (n > r) r = n;
  }
  return r;
}

#define TRECAPWIDTH 8
static void trecap_top(char *pagename, char *setname, int i, int maxhandnum)
{
  int j;
  char r[100];

  large_centered(pagename);
  sput("Transposed recap %s: [ ", setname);
  j = 0;
  while (TRUE) {
    sprintf(r, "%d-%d", TRECAPWIDTH * j + 1, TRECAPWIDTH * j + TRECAPWIDTH); 
    if (i == j) lput(r);
    else addr(r, trecapfilename(setname, j));
    j++;
    if (j * TRECAPWIDTH + 1 > maxhandnum) break;
    lput (" | ");
  }
  lput(" ]");
  newpar();
  addr("Return to main results page", "index.html");
  newpar();
}

static void create_trecap()
{
  int i, maxhandnum, j;
  char name[200], s[200], q[200];

  setloop(s) {
    maxhandnum = getmaxhandnum(s);
    set_maxhandnum(s, maxhandnum);
    for (i = 0; i * TRECAPWIDTH + 1 <= maxhandnum; i++) {
      sprintf(name, "Transposed Recap %s: %d-%d", s,
	      TRECAPWIDTH * i + 1, TRECAPWIDTH * i + TRECAPWIDTH);
      htmlfile(trecapfilename(s, i), name, FALSE);
      trecap_top(name, s, i, maxhandnum);

      mono(70);
      sput("%15s total ", "");
      for (j = 1; j <= TRECAPWIDTH; j++) {
	sprintf(q, " %3d %c", j + i * TRECAPWIDTH,
		(j < TRECAPWIDTH ? ' ' : '\0'));
	lput(q);
      }
      newline();
      
      nameloop(name) {
	char *total = gettotal(name, s);

#define sep(j) (odd(j) ? ' ' : '|')
	if (strexists(total)) { /* no total means no results => skip the guy */
	  sput("%15s ", name);
	  sput("%5s|", total);
	  set_recap_total(s, name, total);
	  for (j = 1; j <= TRECAPWIDTH; j++) {
	    sprintf(q, "%5s", getscorestr(name, s, j + i * TRECAPWIDTH));
	    lput(q);
	    recap_entry0(s, name, j + i * TRECAPWIDTH, q);
	    if (j < TRECAPWIDTH) lputc(sep(j));
	  }
	  newline();
	  sput("%15s ", name);
	  sput("%5s|", "");
	  for (j = 1; j <= TRECAPWIDTH; j++) {
	    sprintf(q, "%5s", getscore(name, s, j + i * TRECAPWIDTH));
	    lput(q);
	    recap_entry1(s, name, j + i * TRECAPWIDTH, q);
	    if (j < TRECAPWIDTH) lputc(sep(j));
	  }
	  newline();
	}
      }
#undef sep
      endmono();
      
      hrule();
      begincolumns(70);
      for (j = TRECAPWIDTH * i + 1; j <= TRECAPWIDTH * i + TRECAPWIDTH; j++) {
	if (columnswidth() > 40) { endcolumns(); hrule(); begincolumns(70); }
	handrecord(s, j);
	newcol(3);
      }
      endcolumns();
      htmlend();
    }
  }  
    
}

#define RECAPWIDTH 8

static void recap_top(char *pagename, char *setname, int page, int numpages)
{
  int j;
  char r[100];

  large_centered(pagename);
  sput("Recap %s: [ ", setname);
  j = 0;
  while (TRUE) {
    sprintf(r, "%s-%s", get_leftname(setname, j), get_rightname(setname, j));
    if (page == j) lput(r);
    else addr(r, recapfilename(setname, j));
    j++;
    if (j == numpages) break;
    lput (" | ");
  }
  lput(" ]");
  newpar();
  addr("Return to main results page", "index.html");
  newpar();
}

static void recap_leftcol(char *setname)
{
  int i = 0, maxhandnum = get_maxhandnum(setname);

  newcol(6);
  colskip(2);
  colput0("total");
  colput0("-------");
  while (++i <= maxhandnum) {
    colput1("%5d", i);
    colput1("%5d", i);
  }
}

#define set_namesecondhalf(name, s) 					\
     ({									\
       char _u[1000];							\
       sprintf(_u, "set {namesecondhalf(%s)} {%s}", (name), (s));	\
       TclDo(_u);							\
     })
#define get_namesecondhalf(name)					\
     ({									\
       char _u[1000];							\
       sprintf(_u, "string trim [set {namesecondhalf(%s)}]", (name));	\
       TclDo(_u);							\
     })
#define namesecondhalf(name) get_namesecondhalf(name)

#define NAMEBREAK 7
static char *namefirsthalf(char *name)
{
  static char r[100];
  int i;
  
  if (strlen(name) <= NAMEBREAK) {
    set_namesecondhalf(name, name);
    return "";
  }

  /* find a good place to break the name, if any */
  i = 0;
  while (TRUE) {
    if (i == NAMEBREAK || name[i] == ' ') {
      if (i == NAMEBREAK || strlen(name + i + 1) <= NAMEBREAK) {
	set_namesecondhalf(name, name + i);
	nstrncpy(r, name, i);
	return r;
      }
    }
    i++;
  }    
}

static void separator_column(int n, char *s)
{
  newcol(strlen(s));
  while (n-- > 0) colput0(s);
}

static fl_bool blank(char *s)
{
  while (*s != '\0') if (!isspace(*s)) return FALSE; else ++s;
  return TRUE;
}

static void recap_namecol(char *setname, char *name, int *count)
{
  int i, maxhandnum = get_maxhandnum(setname);
  char *total = get_recap_total(setname, name);

  printf("recap_namecol(%s, %s)\n", setname, name);
  
  if (!strexists(total)) return;

  if (even(*count))
    separator_column(2 * maxhandnum + 4, "|");
  else
    separator_column(3, "|");
  ++*count;
  newcol(NAMEBREAK);
  colput0(namefirsthalf(name));
  colput0(namesecondhalf(name));
  colput1("%5s", total);
  for (i = -1; i < NAMEBREAK; i++) colputchar('-');
  colput0("");

  i = 0;
  while (++i <= maxhandnum) {
    char *e0 = get_recap_entry0(setname, name, i);
    if (strexists(e0) && !blank(e0)) {
      colput0(e0);
      colput0(get_recap_entry1(setname, name, i));
      bump_numhands(setname, name);
    } else colskip(2);
  }
}

static char *find_recap(char *setname, char *name)
{
  int i = 0;
  char *right;

  while (TRUE) {
    right = get_rightname(setname, i);
    if (!strexists(right))
      return recapfilename(setname, (i == 0) ? 0 : i - 1);
    if (strcmp(name, right) <= 0)
      return recapfilename(setname, i);
    i++;
  }
}

static void create_recap()
{
  char name[100], setname[200], lastname[200];
  int i, numpages, count;
  fl_bool needright;

  setloop(setname) {

    /* Compute division into pages */
    i = numpages = 0;
    needright = FALSE;
    nameloop(name) {
      if (strexists(get_recap_total(setname, name))) {
	strcpy(lastname, name); /* just in case this is the last one */
	if ((i % RECAPWIDTH) == 0) {
	  set_leftname(setname, numpages, name);
	  needright = TRUE;
	}
	if ((i % RECAPWIDTH) == RECAPWIDTH - 1) {
	  set_rightname(setname, numpages, name);
	  numpages++;
	  needright = FALSE;
	}
	i++;
      }
    }
    if (needright) {
      set_rightname(setname, numpages, lastname);
      numpages++;
    }

    /* generate the pages */
    for (i = 0; i < numpages; i++) {
      sprintf(name, "Recap Sheet for %s: %s to %s",
	      setname, get_leftname(setname, i), get_rightname(setname, i));
      htmlfile(recapfilename(setname, i), name, FALSE);
      recap_top(name, setname, i, numpages);

      begincolumns(70);
      recap_leftcol(setname);
      count = 0;
      namesubloop(name, get_leftname(setname, i), get_rightname(setname, i)) {
	recap_namecol(setname, name, &count);
      }
      endcolumns();
      
      htmlend();
    }
  }  
}

static void resultput(char *o, char *northname, char *southname,
		      char *eastname, char *westname, char *name)
{
  lput(o);
  newline();
  hspace(5);
  cnameput(northname, name);
  lput("(N) ");
  cnameput(southname, name);
  lput("(S) ");
  cnameput(eastname, name);
  lput("(E) ");
  cnameput(westname, name);
  lput("(W)");
  newline();
}

static void create_resultlisting(char *setname, char *name)
{
  int i, maxhandnum = get_maxhandnum(setname);

  newpar();
  lput(setname);
  locput(setname);
  lput(" (");
  addr("recap", find_recap(setname, name));
  lput(", ");
  addr("transposed recap", trecapfilename(setname, 0));
  lput(", ");
  addr("hand records", hrfilename(setname));
  lput(", ");
  addr("full results", fullfilename(setname));
  lput(")");
  newline();
  i = 0;
  while (++i <= maxhandnum) {
    char *t = get_recap_entry0(setname, name, i); /* Did he play this hand? */
    if (strexists(t) && !blank(t)) {
      char *o, *eastname, *westname, *northname, *southname;
      retrieve_result(setname, name, i, &o,
		      &northname, &southname, &eastname, &westname);
      bold();
      sput("%d", i);
      endbold();
      lput(": ");
      resultput(o, northname, southname, eastname, westname, name);
    }
  }
}  

static void create_named()
{
  char name[200], setname[200], fname[200], s[200], t[200];

  index_results();
  nameloop(name) {
    strcpy(fname, name_to_filename(name));
    sprintf(s, "%s's Floater results", name);
    htmlfile(fname, s, FALSE);
    large();
    sput("%s's Results", name);
    endlarge();
    newpar();
    setloop(setname) {
      int numhands = get_numhands(setname, name);
      sprintf(s, "%s: %d hands", setname, numhands);
      sprintf(t, "#%s", setname);
      addr(s, t);
      if (numhands > 0)
	sput(". %s average", get_recap_total(setname, name));
      lput(". ");
      addr("Recap sheet", find_recap(setname, name));
      newline();
    }

    setloop(setname) {
      int numhands = get_numhands(setname, name);
      if (numhands > 0) create_resultlisting(setname, name);
    }
    htmlend();
  }
}

#define handdumploop(setname, handnum, iter)		\
    for (iter = handdumpiter((setname), (handnum));	\
	 iter != NULL;					\
	 iter = handdumpnext(iter))

static void create_full()
{
  char setname[200], s[200];
  int i, numhands;
  void *iter;

  setloop(setname) {
      numhands = get_maxhandnum(setname);
      if (numhands > 0) {
	sprintf(s, "Full Results for %s", setname);
	htmlfile(fullfilename(setname), s, FALSE);
	bold();
	lput(setname);
	endbold();
	newline();
	i = 0;
	while (++i <= numhands) {
	  fl_bool first = TRUE;
	  handdumploop(setname, i, iter) {
	    char *o, *eastname, *westname, *northname, *southname;

	    if (first) {
	      begincolumns(60);
	      handrecord(setname, i);
	      endcolumns();
	      first = FALSE;
	    }
	    handdumpfetch(&o, &northname, &southname, &eastname, &westname,
			  iter);
	    resultput(o, northname, southname, eastname, westname, "");
	  }
	}
	htmlend();
      }
  }
}

static void create_hr()
{
  char setname[200], s[200];
  int i, numhands;

  setloop(setname) {
      numhands = get_maxhandnum(setname);
      if (numhands > 0) {
	sprintf(s, "Hand Records for %s", setname);
	htmlfile(hrfilename(setname), s, FALSE);
	bold();
	lput(setname);
	endbold();
	newline();
	i = 1;
	while (i <= numhands) {
	  begincolumns(70);
	  handrecord(setname, i++);
	  newcol(3);
	  handrecord(setname, i++);
	  endcolumns();
	  hrule();
	}
	htmlend();
      }
  }
}


void res2html(void)
{
  extern void tabmulti(char *); /* in comm.c */

  int i;
  char *s;

  rsdir = STRDUP(TclGet("rsdir"));
  webdir = "/var/lib/floater/www";    /* XXX this should be specifyable
				       * via the configuration */
  rswebdir = STRCAT(webdir, "/results");
  date = STRDUP(TclDo("exec date"));
  puts("Performing res2html");

  /* make sure the results directories exist */
  mkdir(rswebdir, 0755);
  mkdir(STRCAT(rswebdir, "/I"), 0755);

  
  /* checknewweek(); */
  for (i = 1; i <= WEEKS; i++) {
    printf("Loading %s\n", s = pastglobaldate(i));
    tabmulti(TclDo2("loadresults ", s));
  }

#if TESTING
  for (i = 1; i <= 10; i++) {
    char handname[200];

    sprintf(handname, "9Jul98IMP%d", i);
    result(handname, unifmod(100) * 10 * (unifmod(2) ? 1 : -1),
	   "1n p p p", "!8", "IMP", "AJoeN", "AJoeS", "ABob1", "ABob2", "");
    result(handname, unifmod(100) * 10 * (unifmod(2) ? 1 : -1),
	   "1n p p p", "!8", "IMP", "ABobN", "ABobS", "AJoe3", "AJoe4", "");
    result(handname, unifmod(100) * 10 * (unifmod(2) ? 1 : -1),
	   "1n p p p", "!8", "IMP", "AJedN", "AJedS", "ALester1", "ALester2", "");
    result(handname, unifmod(100) * 10 * (unifmod(2) ? 1 : -1),
	   "1n p p p", "!8", "IMP", "ALesterN", "ALesterS", "AJed3", "AJed4", "");

    sprintf(handname, "9Jul98MP%d", i);
    result(handname, unifmod(100) * 10 * (unifmod(2) ? 1 : -1),
	   "1n p p p", "!8", "MP", "AJoeN", "AJoeS", "ABob1", "ABob2", "");
    result(handname, unifmod(100) * 10 * (unifmod(2) ? 1 : -1),
	   "1n p p p", "!8", "MP", "ABobN", "ABobS", "AJoe3", "AJoe4", "");
    result(handname, unifmod(100) * 10 * (unifmod(2) ? 1 : -1),
	   "1n p p p", "!8", "MP", "AJedN", "AJedS", "ALester1", "ALester2", "");
    result(handname, unifmod(100) * 10 * (unifmod(2) ? 1 : -1),
	   "1n p p p", "!8", "MP", "ALesterN", "ALesterS", "AJed3", "AJed4", "");
  }
#endif

  scoreall();

  create_main();
  create_trecap();
  create_recap();
  create_named();
  create_full();
  create_hr();

  exit(0);
}
