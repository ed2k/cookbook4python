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

/* text.c - for both reading and writing to the screen when
 * we don't have Tk
 *
 * $Id: text.c,v 1.9 2004/09/26 17:33:57 lexspoon Exp $
 */


#include "floater.h"
#include "UI.h"

#if GUI_ONLY||SILENT

fl_bool needrefresh = FALSE;

#else

#include "comm.h"
#include "br.h"
#include "tickler.h"
#include "text.h"


extern fl_bool initializing; /* from main.c */

/* actually, this variable is only a rough guide, but if it is true, we make
   sure to refresh relatively soon */
fl_bool needrefresh = FALSE;

static fl_bool ccedit = FALSE;

static void Fmove(int y, int x);
static void Fmvaddstr(int y, int x, char *s);
static void Fmvaddch(int y, int x, char c);

static void inittextinput(void);
static void inittextoutput(void);
static void initeditor(void);
static void gotch_ccedit(int c);

/****************************************************************************
Code for multiplexing output to multiple "virtual screens"
****************************************************************************/ 

typedef struct scrnlist {
  int n; /* identifying number */
  int curx, cury; /* cursor position */
  int anchorx, anchory; /* our "anchor" */
  int drawx, drawy; /* where ch() or str() will draw */
  int statusbary, cmdliney, talkliney;
  int talktop, talkbottom; /* location of scrolling talk window */
  char **scrn; /* what's on the scrn */
  struct scrnlist *next;
} scrnlist;

static scrnlist *drawsc = NULL; /* the scrn we're drawing to */
static scrnlist *dispsc = NULL; /* the scrn we're displaying */

static scrnlist *scrns = NULL; /* the list of all scrns */

void makescrn(int n, int cmdliney, int talkliney, int statusbary,
		int talktop, int talkbottom)
{
  scrnlist *sc;
  int y, x;

  sc = alloc(scrnlist);
  sc->n = n;
  sc->cmdliney = cmdliney;
  sc->talkliney = talkliney;
  sc->statusbary = statusbary;
  sc->talktop = talktop;
  sc->talkbottom = talkbottom;
  sc->curx = sc->cury = sc->anchorx = sc->anchory = sc->drawx = sc->drawy = 0;
  sc->scrn = (char **) salloc(sizeof(char *) * LINES);
  for (y = 0; y < LINES; y++) {
    sc->scrn[y] = salloc(sizeof(char) * (COLS + 1));
    for (x = 0; x < COLS; x++) sc->scrn[y][x] = ' ';
    sc->scrn[y][x] = '\0';
  }

  sc->next = scrns;
  scrns = sc;
}

/* Change which scrn we're drawing on to scrn. */
void drawscrn(int scrn)
{
  scrnlist *sc;

  if (drawsc != NULL && drawsc->n == scrn) return;

  /* find screen we're switching to */
  for (sc = scrns; sc->n != scrn; ) {
    sc = sc->next;
    assert(sc != NULL);
  }

  drawsc = sc;
}

/* Pad s out with hyphens until it is the appropriate width, then
   display it at the given x position on the statusbar. */
static void draw_on_statusbar(int x, int width, char *s)
{
  char t[200];

  assert(width > 0);
  sprintf(t, "%s-----------------------------------------------------------------------------------", s);
  t[width] = '\0';
  Fmvaddstr(drawsc->statusbary, x, t);
}

void update_statusbar(void)
{
  char s[100], *t;
  fl_bool center;
  fl_bool want_auction = (dispsc->n == 1);
  int mid, start, end, width;
  int old = drawsc->n;
  static char *previous_scroll = NULL;
  static char *previous_myturn = NULL;
  static char *previous_bridge = NULL;
  static fl_bool show_init = FALSE;

  drawscrn(dispsc->n); /* draw on whatever screen we're displaying */

  if (!ccedit && atob(TclGet("scrolllock")))
    sprintf(s, "%d-%s/%s", 1 + atoi(TclGet("talklineattop")),
	    TclDo("uplevel #0 {expr $talklineattop + "
		  "$talkbottom - $talktop + 1}"),
	    TclGet("ntalklines"));
  else s[0] = '\0';
  if (previous_scroll == NULL || !streq(previous_scroll, s)) {
    draw_on_statusbar(65, COLS - 65, s);
    reset(previous_scroll);
    previous_scroll = STRDUP(s);
  }

  if (silentmyturntoplay()) t = "<PLAY>";
  else if (silentmyturntobid()) t = "<BID>";
  else t = "";
  if (previous_myturn == NULL || !streq(previous_myturn, t)) {
    draw_on_statusbar(1, 6, t);
    reset(previous_myturn);
    previous_myturn = STRDUP(t);
  }
  
  start = 8;
  end = 63;
  mid = (1 + end + start) / 2;
  width = end - start + 1;
  t = NULL;
  center = FALSE;
  if (want_auction && bridge_statusbar(s, start, end, &center) && strexists(s))
    t = s;
  else if ((t = display_of_previous_trick()) != NULL)
    center = TRUE;
  if (t == NULL) t = "";
  if (previous_bridge == NULL || !streq(previous_bridge, t)) {
    if (center) draw_on_statusbar(mid - strlen(t) / 2, strlen(t), t);
    else draw_on_statusbar(start, width, t);
    reset(previous_bridge);
    previous_bridge = STRDUP(t);
  }

  if (show_init != initializing) 
    if ((show_init = initializing))
      draw_on_statusbar(1, 30, "Initializing... Please wait");
    else
      draw_on_statusbar(1, 30, "");

  drawscrn(old); /* go back to drawing on whatever screen we were before */
}

/* Change which scrn we're actually displaying. */
void displayscrn(int scrn)
{
  scrnlist *sc;
  int y;

  if (dispsc != NULL && dispsc->n == scrn) return;

  /* find scrn we're switching to */
  for (sc = scrns; sc->n != scrn; ) {
    sc = sc->next;
    assert(sc != NULL);
  }
  
  /* copy the new screen to curses */
  for (y = 0; y < LINES; y++) mvaddstr(y, 0, sc->scrn[y]);

  /* specially handle status bar, talk line, and command line */
  if (dispsc != NULL) {
    if (sc->statusbary >= 0 && dispsc->statusbary >= 0) {
      mvaddstr(sc->statusbary, 0, dispsc->scrn[dispsc->statusbary]);
      strcpy(sc->scrn[sc->statusbary], dispsc->scrn[dispsc->statusbary]);
    }
    if (sc->talkliney >= 0 && dispsc->talkliney >= 0) {
      mvaddstr(sc->talkliney, 0, dispsc->scrn[dispsc->talkliney]);
      strcpy(sc->scrn[sc->talkliney], dispsc->scrn[dispsc->talkliney]);
    }
    if (sc->cmdliney >= 0 && dispsc->cmdliney >= 0) {
      mvaddstr(sc->cmdliney, 0, dispsc->scrn[dispsc->cmdliney]);
      strcpy(sc->scrn[sc->cmdliney], dispsc->scrn[dispsc->cmdliney]);
    }
  }

  /* update cursor position */
  if (dispsc != NULL) {
    if (dispsc->cury == dispsc->cmdliney) sc->cury = sc->cmdliney;
    else if (dispsc->cury == dispsc->talkliney) sc->cury = sc->talkliney;
    else if (dispsc->cury == dispsc->statusbary) sc->cury = sc->statusbary;
  }
  move(sc->cury, sc->curx);

  dispsc = sc;

  /* reconfigure and draw talk region */
  TclDo5("uplevel #0 {set talktop ", itoa(sc->talktop), "; set talkbottom ",
	 itoa(sc->talkbottom), "}");
  if (sc->talktop >= 0) {
    TclDo5("catch {talkregion ", itoa(sc->talktop), " ",
	   itoa(sc->talkbottom), "}");
  }
}

static void inittextoutput(void)
{
  makescrn(MATRIX_SCREEN, 15, 16, 17, 18, LINES - 1);
  makescrn(NO_MATRIX_SCREEN, LINES - 2, LINES - 1, LINES - 3, 0, LINES - 4);
  makescrn(EDIT_SCREEN, -1, -1, LINES - 3, -1, -1);
  displayscrn(0);
  drawscrn(0);
  Fmvaddstr(17, 0, "------------------------------ status bar ------------------------------");
}

static void Fmove(int y, int x)
{
  /* assert(y >= 0 && y < LINES && x >= 0 && x < COLS); */
  if (!(y >= 0 && y < LINES && x >= 0 && x < COLS)) {
    char s[1000];
    sprintf(s, "%%s:%%s Fmove y=%d x=%d COLS=%d LINES=%d}",
	    y, x, COLS, LINES);
    mail_bug(s, __FILE__, __LINE__);
    x = y = 0;
  }
  drawsc->curx = x;
  drawsc->cury = y;
  if (dispsc == drawsc) move(y, x);
}

static void Fmvaddstr(int y, int x, char *s)
{
  int len = strlen(s);

  if ((x + len == COLS + 1) && (s[len - 1] == '\n')) s[--len] = '\0';
  assert(y >= 0 && y < LINES && x >= 0 && (x + len <= COLS));
  strncpy(&(drawsc->scrn[y][x]), s, len);
  if (dispsc == drawsc) mvaddstr(y, x, s);
}

static void Fmvaddch(int y, int x, char c)
{
  static char s[2] = {'\0', '\0'};

  s[0] = c;
  Fmvaddstr(y, x, s);
}

/****************************************************************************/

/* argv[1] should be two integers separated by a space (x then y) */
int anchor(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[])
{
  if (argc != 2) {
    SET_INTERP_RESULT("wrong # of arguments to `anchor'");
    return TCL_ERROR;
  }

  sscanf(argv[1], "%d %d", &drawsc->anchorx, &drawsc->anchory);
  drawsc->drawx = drawsc->anchorx;
  drawsc->drawy = drawsc->anchory;
  return TCL_OK;
}

/* argv[1] is width of rect; argv[2] is height */
int clearrect(ClientData clientData, Tcl_Interp *interp,
	      int argc, const char *argv[])
{
  int width, height, j;
  char s[100], format[10];

  if (argc != 3) {
    SET_INTERP_RESULT("wrong # of arguments to `clearrect'");
    return TCL_ERROR;
  }

  width = atoi(argv[1]);
  height = atoi(argv[2]);
  assert(width < 100);
  sprintf(format, "%%%ds", width); /* e.g. width=56 yields "%56s" */
  sprintf(s, format, "");
  assert(strlen(s) == width);
  for (j=0; j<height; j++) {
    Fmvaddstr(drawsc->drawy + j, drawsc->drawx, s);
  }
  return TCL_OK;
}

/* move down a line and to the anchor's x; doesn't change the anchor */
int down_and_anchor(ClientData clientData, Tcl_Interp *interp,
		     int argc, const char *argv[])
{
  if (argc != 2 && argc != 1) {
    SET_INTERP_RESULT("wrong # of arguments to `down_and_anchor'");
    return TCL_ERROR;
  }

  drawsc->drawy += (argc == 1) ? 1 : atoi(argv[1]);
  drawsc->drawx = drawsc->anchorx;
  return TCL_OK;
}

int right(ClientData clientData, Tcl_Interp *interp,
	  int argc, const char *argv[])
{
  if (argc != 2 && argc != 1) {
    SET_INTERP_RESULT("wrong # of arguments to `right'");
    return TCL_ERROR;
  }

  drawsc->drawx += (argc == 1) ? 1 : atoi(argv[1]);
  return TCL_OK;
}

int str(ClientData clientData, Tcl_Interp *interp,
	int argc, const char *argv[])
{
  if (argc != 2) {
    SET_INTERP_RESULT("wrong # of arguments to `str'");
    return TCL_ERROR;
  }

  Fmvaddstr(drawsc->drawy, drawsc->drawx, argv[1]);
  drawsc->drawx += strlen(argv[1]);
  return TCL_OK;
}

int ch(ClientData clientData, Tcl_Interp *interp,
       int argc, const char *argv[])
{
  if (argc != 2) {
    SET_INTERP_RESULT("wrong # of arguments to `ch'");
    return TCL_ERROR;
  }

  Fmvaddch(drawsc->drawy, drawsc->drawx, argv[1][0]);
  drawsc->drawx++;
  return TCL_OK;
}

/* Before refreshing the screen, this function is called to make sure
   the cursor is shown in the right place.  NB: Different from
   reset_cursor_position().  */
void move_to_curyx(void)
{
  move(dispsc->cury, dispsc->curx);
}

/* no args; just move to (drawsc->curx, drawsc->cury) */
int reset_cursor_position(ClientData clientData, Tcl_Interp *interp,
			  int argc, const char *argv[])
{
  if (argc != 1) {
    SET_INTERP_RESULT("wrong # of arguments to `reset_cursor_position'");
    return TCL_ERROR;
  }

  Fmove(drawsc->cury, drawsc->curx);
  return TCL_OK;
}

/* no args; just prepare to draw on screen currently being displayed */
int draw_on_current_display(ClientData clientData, Tcl_Interp *interp,
			    int argc, const char *argv[])
{
  static int old;

  if (argc != 2) {
    SET_INTERP_RESULT("wrong # of arguments to `draw_on_current_display'");
    return TCL_ERROR;
  }

  if (argv[1][0] == '+') {
    old = drawsc->n;
    drawscrn(dispsc->n);
  } else if (argv[1][0] == '-') {
    drawscrn(old);
  }
  return TCL_OK;
}

/****************************************************************************/

/* Scroll the talk window by n lines (negative means back). */ 
void floaterscroll(int n)
{
  TclDo2("talkscroll ", itoa(n));
}


#if MAKING_TUTORIAL
static void screen_shot(void)
{
  static int n = 0;
  char *s;

  n++;
  s = TEMPCAT("screen", itoa(n));
  scr_dump(s);
  log(TEMPCAT("include ", s));
  beep();
}
#endif

/* Input */

static void drawinputlines(void);
static void drawinputline(char *s, int p, int y, int *x);

#define MAX_INPUT_STRING 1000

#define ctrl(x) ((x) - 'a' + 1)

#define FLOATER_TAB ctrl('i')
#define FLOATER_TOGGLE_DISPLAY ctrl('v')
#define FLOATER_REFRESH ctrl('w')
#define FLOATER_REFRESH2 ctrl('l')
#define FLOATER_BEGINNING_OF_LINE ctrl('a')
#define FLOATER_KILL_REST_OF_LINE ctrl('k')
#define FLOATER_KILL FLOATER_KILL_REST_OF_LINE
#define FLOATER_YANK ctrl('y')
#define FLOATER_END_OF_LINE ctrl('e')
#define FLOATER_DELETE_UNDER ctrl('d')
#define FLOATER_DOWN ctrl('n')
#define FLOATER_UP ctrl('p')
#define FLOATER_LEFT ctrl('b')
#define FLOATER_RIGHT ctrl('f')
#define FLOATER_RETURN ctrl('m')
#define FLOATER_DONE_EDITING ctrl('u')
#define FLOATER_ABORT ctrl('x')
#define FLOATER_ERASE_ALL ctrl('r')
#if MAKING_TUTORIAL
#define FLOATER_SCREEN_SHOT ctrl('t')
#endif

/* these ifndefs should do nothing; just in case, the values used are bogus */
#ifndef KEY_HOME
#define KEY_HOME -1
#endif
#ifndef KEY_BACKSPACE
#define KEY_BACKSPACE -2
#endif
#ifndef KEY_DC
#define KEY_DC -3
#endif
#ifndef KEY_LEFT
#define KEY_LEFT -4
#endif
#ifndef KEY_RIGHT
#define KEY_RIGHT -5
#endif
#ifndef KEY_ENTER
#define KEY_ENTER -6
#endif
#ifndef KEY_UP
#define KEY_UP -7
#endif
#ifndef KEY_DOWN
#define KEY_DOWN -8
#endif

/* If it doesn't appear above, should we ignore this character? */
#define IGNORE(ch) ((ch) < ' ' || (ch) > 127 || isin((ch), "[]{}"))

#define CMDLINE_Y (drawsc->cmdliney)
#define TALKLINE_Y (drawsc->talkliney)
#define INPUT_X_OFFSET 8
#define CMDLINE_PROMPT  "command "
#define TALKLINE_PROMPT "   talk "
static char cmdline[MAX_INPUT_STRING], talkline[MAX_INPUT_STRING];
static int cmdlineoffset, talklineoffset;
static char *focus; /* points to either talkline or cmdline */
static fl_bool text2fieldinputmode = FALSE;

/* Input two items; return them in result1 and result2 */
/* length of label1 and label2 should be same as CMDLINE_PROMPT */
void text2fieldinput(char *label1, char *init1, 
		     char *label2, char *init2, char **result1, char **result2)
{
  int c;

  drawscrn(dispsc->n); /* draw on whatever screen we're displaying */
  assert(strlen(CMDLINE_PROMPT) == strlen(label1));
  assert(strlen(CMDLINE_PROMPT) == strlen(label2));
  strcpy(cmdline, init1);
  cmdlineoffset = strlen(cmdline);
  strcpy(talkline, init2);
  talklineoffset = strlen(talkline);
  Fmvaddstr(CMDLINE_Y, 0, label1);
  Fmvaddstr(TALKLINE_Y, 0, label2);
  focus = cmdline;
  drawinputlines();

  text2fieldinputmode = TRUE;
  textrefresh();
  while (text2fieldinputmode) if ((c = getch()) != ERR) gotch(c);
  *result1 = TEMPCOPY(cmdline);
  *result2 = TEMPCOPY(talkline);
  
  inittextinput();
  drawinputlines();
  textrefresh();
}

static void inittextinput(void)
{
  drawscrn(dispsc->n); /* draw on whatever scrn we're displaying */
  cmdline[0] = talkline[0] = '\0';
  cmdlineoffset = talklineoffset = 0;
  Fmvaddstr(CMDLINE_Y, 0, CMDLINE_PROMPT);
  Fmvaddstr(TALKLINE_Y, 0, TALKLINE_PROMPT);
  focus = cmdline;
  TclSet("talkwidth", itoa(COLS));
  drawsc->curx = INPUT_X_OFFSET;
  drawsc->cury = (focus == cmdline) ? CMDLINE_Y : TALKLINE_Y;
}

void inittextio(void)
{
  inittextoutput();
  inittextinput();
  initeditor();
}

/* s is the string to be displayed
   p is the cursor position within s
   y is the line to draw on
   *x is how many characters at the start of s we are currently not showing */
static void drawinputline(char *s, int p, int y, int *x)
{
  int width = COLS - INPUT_X_OFFSET;
  int len = strlen(s);
  char ch, lefttrunc, righttrunc; /* the last two are fl_booleans */

  /* compute what *x should be */
  if (len < width || (len == width && p < len)) *x = 0;
  else {
    int hypothetical = p - *x;

    if (hypothetical < 0) *x = p;
    else while (hypothetical-- >= width) ++*x;
  }    

  lefttrunc = (*x > 0);
  righttrunc = (len > width + *x);
  ch = lefttrunc ? (righttrunc ? '.' : '<') : (righttrunc ? '>' : ' ');
  Fmvaddch(y, INPUT_X_OFFSET - 1, ch);
  if (lefttrunc || righttrunc) {
    char temp;

    s += *x;
    temp = s[width];
    s[width] = '\0';
    addstr(s);
    if (strlen(s) < width) clrtoeol();
    s[width] = temp;
  } else {
    addstr(s);
    clrtoeol();
  }
}


static void drawinputlines(void)
{
  int old = drawsc->n;
  int cmdlinetrunc = 0, talklinetrunc = 0;

  assert(cmdlineoffset >= 0 && cmdlineoffset <= strlen(cmdline));
  assert(talklineoffset >= 0 && talklineoffset <= strlen(talkline));
  drawscrn(dispsc->n); /* draw on whatever screen we're displaying */
  drawinputline(cmdline, cmdlineoffset, CMDLINE_Y, &cmdlinetrunc);
  drawinputline(talkline, talklineoffset, TALKLINE_Y, &talklinetrunc);
  drawsc->curx = INPUT_X_OFFSET + ((focus == cmdline) ?
			   (cmdlineoffset - cmdlinetrunc) :
			   (talklineoffset - talklinetrunc));
  drawsc->cury = (focus == cmdline) ? CMDLINE_Y : TALKLINE_Y;
  if (drawsc->curx >= COLS) drawsc->curx = COLS - 1;
  drawscrn(old); /* go back to drawing on whatever screen we were before */
}

static void gotchs(char *s)
{
  while (*s != '\0') gotch(*s++);
}

#define DECR(x) (((x) > 0) ? --(x) : 0)
#define INCR(x) (((x) < strlen(focus)) ? ++(x) : 0)
void gotch(int c)
{
  extern void tasks(void);

  if (ccedit) {
    gotch_ccedit(c);
    return;
  }

  switch (c) {
#if MAKING_TUTORIAL
  case FLOATER_SCREEN_SHOT:
    screen_shot();
    break;
#endif
  case FLOATER_TAB:
    if (focus == cmdline) focus = talkline;
    else focus = cmdline;
    break;
  case FLOATER_REFRESH:
  case FLOATER_REFRESH2:
    wrefresh(curscr);
    return;
  case FLOATER_BEGINNING_OF_LINE:
  case KEY_HOME:
    if (focus == cmdline) cmdlineoffset = 0;
    else talklineoffset = 0;
    break;
  case FLOATER_KILL_REST_OF_LINE:
    focus[(focus == cmdline) ? cmdlineoffset : talklineoffset] = '\0';
    /* fall through */
  case FLOATER_END_OF_LINE:
    if (focus == cmdline) cmdlineoffset = strlen(cmdline);
    else talklineoffset = strlen(talkline);
    break;
  case FLOATER_DELETE_UNDER:
    {
      int offset = (focus == cmdline) ? cmdlineoffset : talklineoffset;

      if (focus[offset] == '\0') return;
      while ((focus[offset] = focus[offset + 1]) != '\0') offset++;
    }
    break;
  case KEY_BACKSPACE:
  case KEY_DC:
#ifdef KEY_SDC
  case KEY_SDC:
#endif
  delete:
    {
      int offset = (focus == cmdline) ? cmdlineoffset : talklineoffset;

      if (--offset < 0) return;
      while ((focus[offset] = focus[offset + 1]) != '\0') offset++;
    }
    /* fall through */
  case FLOATER_LEFT:
  case KEY_LEFT:
    if (focus == cmdline) DECR(cmdlineoffset);
    else DECR(talklineoffset);
    break;
  case FLOATER_RIGHT:
  case KEY_RIGHT:
    if (focus == cmdline) INCR(cmdlineoffset);
    else INCR(talklineoffset);
    break;
  case FLOATER_RETURN:
  case KEY_ENTER:
    if (text2fieldinputmode) {
      if (focus == cmdline) focus = talkline;
      else text2fieldinputmode = FALSE;
    } else {
      if (focus == cmdline) {
	if (cmdline[0] == '\0' && insert_default_action(FALSE, gotchs)) return;
#if MAKING_TUTORIAL
	log(TEMPCAT("# command ", cmdline));
#endif
	TclDo3("command {", tclclean(cmdline), "}");
	cmdlineoffset = 0;
      } else {
	if (talkline[0] == '\0' && insert_default_action(TRUE, gotchs)) return;
#if MAKING_TUTORIAL
	log(TEMPCAT("# talk ", talkline));
#endif
	if (talkline[0] != '/') TclDo3("talk {", tclclean(talkline), "}");
	else TclDo3("command {", tclclean(talkline + 1), "}");
	talklineoffset = 0;
      }
      focus[0] = '\0';
      tasks();
    }
    break;
  case FLOATER_TOGGLE_DISPLAY:
    displayscrn(1 - dispsc->n);
    break;
#ifdef KEY_PPAGE
  case KEY_PPAGE:
    floaterscroll(dispsc->talktop - dispsc->talkbottom);
    break;
#endif
#ifdef KEY_NPAGE
  case KEY_NPAGE:
    floaterscroll(dispsc->talkbottom - dispsc->talktop);
    break;
#endif
#ifdef KEY_LL
  case KEY_LL:
    TclDo("command bottom");
    break;
#endif
  case KEY_UP:
  case FLOATER_UP:
    floaterscroll(-1);
    break;
  case KEY_DOWN:
  case FLOATER_DOWN:
    floaterscroll(1);
    break;
  default:
    if (IGNORE(c)) return;
    if (strlen(unctrl(c)) != 1)
      if (streq(unctrl(c), "^?")) goto delete;
      else return;
    {
      int offset = (focus == cmdline) ? cmdlineoffset : talklineoffset;
      char temp[MAX_INPUT_STRING];

      if (strlen(focus) > MAX_INPUT_STRING - 2) { beep(); return; }
      strcpy(temp, focus + offset);
      strcpy(focus + offset + 1, temp);
      focus[offset] = c;
    }
    if (focus == cmdline) ++cmdlineoffset;
    else ++talklineoffset;
    break;
  }
  if (ccedit) return;
  drawinputlines();
  textrefresh();
}

/****************************************************************************
  text (convention card) editor
****************************************************************************/

/* allow at most 40 lines of text */
#define MAX_EDIT_Y 39

static int edityoffset = 0;
#define editywindow(y) ((y) + edityoffset)
#define inverse_editywindow(y) ((y) - edityoffset)

/* physical screen positions */
#define TOP_EDIT_Y 0
#define BOTTOM_EDIT_Y (LINES - 4)

#define EDIT_HELP_Y (LINES - 2)
#define CONFIRM_Y (LINES - 1)

static char *lines[MAX_EDIT_Y + 1];
static int edity, editx;

static char edit_readonly; /* whether this is a read-only editing session */

static void initeditor(void)
{
  int i;

  for (i = 0; i <= MAX_EDIT_Y; i++) lines[i] = NULL;
}


#define makenewline(n) allocnewline((n), TRUE)
#define makenewline_nofree(n) allocnewline((n), FALSE)
static void allocnewline(int n, fl_bool shouldfree)
{
  if (shouldfree) reset(lines[n]);
  lines[n] = (char *) salloc(sizeof(char) * (5 + MAX_INPUT_STRING));
  lines[n][0] = '\0';
}

static char *showeditline(char *s)
{
  if (strlen(s) >= COLS) {
    char *temp = STRNDUP(s, COLS - 1);
    temp[COLS - 1] = '$';
    temp[COLS] = '\0';
    return temp;
  } else return s;
}

static void showalleditlines(void)
{
  int j, k;

  for (j = TOP_EDIT_Y; j <= BOTTOM_EDIT_Y; j++) {
    k = editywindow(j);
    mvaddstr(j, 0, ((k <= MAX_EDIT_Y) ? showeditline(lines[k]) : ""));
    clrtoeol();
  }
}

static void seteditcursor(void)
{
  static int old_y = -1;
  char s[10];

  dispsc->cury = inverse_editywindow(edity);
  dispsc->curx = min(editx, COLS - 1);
  if (old_y != edity) {
    old_y = edity;
    sprintf(s, "L%d", old_y + 1);
    draw_on_statusbar(65, 4, s);
  }
}

/* returns whether it deems scrolling to be necessary */
static fl_bool adjustvisiblelines(void)
{
  if (inverse_editywindow(edity) < 0) {
    edityoffset = edity - 5;
    return TRUE;
  }
  if (inverse_editywindow(edity) > BOTTOM_EDIT_Y) {
    edityoffset = edity - (BOTTOM_EDIT_Y - 5);
    return TRUE;
  }
  return FALSE;
}

static int olddispsc;
static char *ccedit_direction = NULL;

void beginedit(char *direction, char readonly)
{
  int j;
  char s[100];
  char *help = readonly ?
    "^U (or ^X) = exit; ^N, ^P, ^A, ^E, ^F, ^B to move" :
    "^U = done; ^X = abort; ^R = erase all; ^N, ^P, ^A, ^E, ^F, ^B to move";

  draw_on_statusbar(65, COLS - 65, "L1");
  if (readonly) {
    sprintf(s, "Viewing %s", direction);
    draw_on_statusbar(69, strlen(s), s);
  }
  olddispsc = dispsc->n;
  reset(ccedit_direction);
  ccedit_direction = STRDUP(direction);
  displayscrn(EDIT_SCREEN);
  drawscrn(EDIT_SCREEN);
  mvaddstr(EDIT_HELP_Y, 40 - strlen(help) / 2, help);
  ccedit = TRUE;
  edit_readonly = readonly;
  for (j = 0; j <= MAX_EDIT_Y; j++) {
    makenewline(j);
    sprintf(s, "getccline %s %d", direction, j + 1);
    strcpy(lines[j], TclDo(s));
  }

  edityoffset = editx = edity = 0;
  adjustvisiblelines();
  showalleditlines();
  seteditcursor();
}

static fl_bool empty_line(char *s)
{
  while (*s != '\0') if (isspace(*s)) s++; else return FALSE;
  return TRUE;
}

/* Find the last line of the cc (the editing buffer) that is not blank. */
static int lastccline(void)
{
  int i = MAX_EDIT_Y;

  while (i >= 0 && empty_line(lines[i])) i--;
  return i; 
}

static void endedit(fl_bool save_changes)
{
  int j, n;

  ccedit = FALSE;
  draw_on_statusbar(65, COLS - 65, "");
  if (save_changes) {
    TclDo2("beginnewcc ", ccedit_direction);
    for (j = 0, n = lastccline(); n >= 0; j++, n--) 
      TclDo3("addnewcc {", braceclean(lines[j]), "}");
    TclDo("set s [endnewcc]; if [string compare $s \"\"] {talkmsg $s}");
    selfmessage(makemsg2(CC_TO_HOST, ccedit_direction,
			 TclDo2("ccstr ", ccedit_direction)));
  }
  displayscrn(olddispsc);
  dispsc->curx = INPUT_X_OFFSET;
  dispsc->cury = (focus == cmdline) ? CMDLINE_Y : TALKLINE_Y;
}

static void done_editing(void)
{
  endedit(!edit_readonly);
}

static void discard_changes(void)
{
  endedit(FALSE);
}

static void erase_everything(void)
{
  int i;

  editx = edity = edityoffset = 0;
  for (i = 0; i <= MAX_EDIT_Y; i++) makenewline(i);
  showalleditlines();
  seteditcursor();
  textrefresh();
}

typedef void (*vvfn)(void);

static fl_bool confirming = FALSE;
static vvfn confirmfn;
static int oldx, oldy;

static void confirm(char *s, vvfn f)
{
  char t[100];

  oldx = dispsc->curx;
  oldy = dispsc->cury;
  sprintf(t, "%s (y/n)? ", s);
  mvaddstr(CONFIRM_Y, 0, t);
  dispsc->cury = CONFIRM_Y;
  dispsc->curx = strlen(t);
  clrtoeol();
  textrefresh();
  confirming = TRUE;
  confirmfn = f;
}

static void gotch_confirming(int c)
{
  if (c == 'y' || c == 'Y') (*confirmfn)();
  else if (c == 'n' || c == 'N') {
    dispsc->curx = oldx;
    dispsc->cury = oldy;
  } else { beep(); return; }
  if (ccedit) {
    mvaddstr(CONFIRM_Y, 0, " ");
    clrtoeol();
  }
  confirming = FALSE;
}

static void insertstr(char *s)
{
  char *out;
  int len = strlen(lines[edity]) + strlen(s);
      
  out = salloc(sizeof(char) * (5 + min(len, MAX_INPUT_STRING)));
      
  strcpy(out, lines[edity]);
  strcpy(out + editx, s);
  if (strlen(lines[edity]) != editx)
    strcpy(out + strlen(out), lines[edity] + editx);

  reset(lines[edity]);
  lines[edity] = out;
}

static char *strkilled(fl_bool *deletedcr)
{
  if (empty_line(lines[edity] + editx)) {
    *deletedcr = TRUE;
    return TEMPCAT(lines[edity] + editx, "\n");
  }
  return (lines[edity] + editx);
}

/* Try to move the cursor right (or to start of next line, if at end of line).
   Returns whether it actually moved the cursor (i.e. true unless eof) */ 
static fl_bool moveright(void)
{
  if (editx < strlen(lines[edity])) editx++;
  else if (edity < MAX_EDIT_Y) {
    edity++;
    editx = 0;
  } else return FALSE;
  return TRUE;
}

static void gotch_ccedit(int c)
{
  extern void tasks(void);
  static fl_bool killing = FALSE;
  static char *killbuffer = NULL;
  fl_bool kill = FALSE, deletedcr = FALSE, all_dirty = FALSE, dirty = FALSE;
  fl_bool yank = FALSE;
  int i, j;

  drawscrn(EDIT_SCREEN);

  if (confirming) {
    gotch_confirming(c);
    return;
  }

  switch (c) {
#if MAKING_TUTORIAL
  case FLOATER_SCREEN_SHOT:
    screen_shot();
    break;
#endif
  case FLOATER_REFRESH:
  case FLOATER_REFRESH2:
    wrefresh(curscr);
    return;
  case FLOATER_BEGINNING_OF_LINE:
    editx = 0;
    break;
  case KEY_HOME:
    editx = edity = 0;
    break;
  case FLOATER_YANK:
    if (edit_readonly) goto complain;
    if (!strexists(killbuffer)) return;
    insertstr(TEMPCAT(killbuffer, "\t")); /* tab marks end of yanked text */
    yank = dirty = TRUE;
    break;
  case FLOATER_KILL:
    if (edit_readonly) goto complain;
    if (killing) {
      char *s;

      s = STRCAT(killbuffer, strkilled(&deletedcr));
      reset(killbuffer);
      killbuffer = s;
    } else {
      reset(killbuffer);
      killbuffer = STRDUP(strkilled(&deletedcr));
    }
    lines[edity][editx] = '\0';
    dirty = kill = TRUE;
    break;
  case FLOATER_END_OF_LINE:
    editx = strlen(lines[edity]);
    break;
  case FLOATER_DELETE_UNDER:
  delete_under:
    if (edit_readonly) goto complain;
    if (editx == strlen(lines[edity])) deletedcr = TRUE;
    else {
      i = editx;
      while ((lines[edity][i] = lines[edity][i+1]) != '\0') i++;
    }
    dirty = TRUE;
    break;
  case KEY_BACKSPACE:
  case KEY_DC:
#ifdef KEY_SDC
  case KEY_SDC:
#endif
  delete:
    if (edit_readonly) goto complain;
    /* fall through */
  case FLOATER_LEFT:
  case KEY_LEFT:
    if (editx > 0) editx--;
    else if (edity > 0) {
      edity--;
      editx = strlen(lines[edity]);
    } else break;
    if (!(c == FLOATER_LEFT || c == KEY_LEFT)) goto delete_under;
    break;
  case FLOATER_RIGHT:
  case KEY_RIGHT:
    moveright();
    break;
  case KEY_UP:
  case FLOATER_UP:
    if (edity > 0) edity--;
    if (editx > strlen(lines[edity])) editx = strlen(lines[edity]);
    break;
  case KEY_DOWN:
  case FLOATER_DOWN:
    if (edity < MAX_EDIT_Y) edity++;
    if (editx > strlen(lines[edity])) editx = strlen(lines[edity]);
    break;
  case FLOATER_DONE_EDITING:
    if (edit_readonly) confirm("Done", discard_changes);
    else confirm("Done editing", done_editing);
    break;
  case FLOATER_ABORT:
    if (edit_readonly) confirm("Done", discard_changes);
    else confirm("Discard changes", discard_changes);
    break;
  case FLOATER_ERASE_ALL:
    if (edit_readonly) goto complain;
    confirm("Erase everything", erase_everything);
    break;
  case FLOATER_RETURN:
  case KEY_ENTER:
    if (edit_readonly) goto complain;
    c = '\n';
    /* fall through */
  default:
    if (c != '\n' && IGNORE(c)) return;
    if (strlen(unctrl(c)) != 1 && c != '\n')
      if (streq(unctrl(c), "^?")) goto delete;
      else return;
    if (edit_readonly) goto complain;
    {
      int offset = editx;
      char *focus = lines[edity];
      char temp[MAX_INPUT_STRING];

      if (strlen(focus) > MAX_INPUT_STRING - 2) { beep(); return; }
      strcpy(temp, focus + offset);
      strcpy(focus + offset + 1, temp);
      focus[offset] = c;
    }
    editx++;
    dirty = TRUE;
    break;
  complain:
    beep();
    return;
  }
  /* end of switch (c) */

  if (confirming || !ccedit) goto done;

  if (deletedcr) {
    if (edity < MAX_EDIT_Y) {
      insertstr(lines[edity + 1]);
      reset(lines[edity + 1]);
      for (i = edity; i <= MAX_EDIT_Y - 2; i++) lines[i + 1] = lines[i + 2];
      makenewline_nofree(i + 1);
    } else beep();
    all_dirty = TRUE;
  }

  if (all_dirty || dirty) {
    fl_bool needbump = FALSE;

    /* clean up occurences of \n in current line */
    if (edity == MAX_EDIT_Y && isin('\n', lines[edity])) {
      i = 0;
      while (lines[edity][i] != '\n') i++;
      lines[edity][i] = '\0';
      if (editx > strlen(lines[edity])) editx = strlen(lines[edity]);
    } else while (isin('\n', lines[edity])) {
      i = strlen(lines[edity]);
      while (lines[edity][--i] != '\n'); /* find last '\n' */
      lines[edity][i] = '\0';
      reset(lines[MAX_EDIT_Y]);
      for (j = MAX_EDIT_Y; j >= edity + 2; j--) lines[j] = lines[j - 1];
      makenewline_nofree(edity + 1);
      strcpy(lines[edity + 1], lines[edity] + i + 1);
      if (editx == i + 1) needbump = TRUE;
      all_dirty = TRUE;
    }    

    if (needbump) {
      editx = 0;
      edity++;
    }
    
    if (yank) {
      /* find and remove bogus tab that marks the end of the yanked text */
      while (lines[edity][editx] != '\t' && moveright());
      if (lines[edity][editx] == '\t') {
	yank = needbump = FALSE;
	/* less efficient but equally valid would be gotch(ctrl('d')) (?) */
	goto delete_under;
      }
    }
  }

  if (adjustvisiblelines() || all_dirty) showalleditlines();
  else if (dirty) {
    mvaddstr(inverse_editywindow(edity), 0, showeditline(lines[edity]));
    clrtoeol();
  }
	     
  seteditcursor();
  textrefresh();

done:
  killing = kill;
}

#endif /* !GUI_ONLY */
