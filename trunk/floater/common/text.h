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
#ifndef __TEXT_H__
#define __TEXT_H__

#if GUI_ONLY||SILENT

extern fl_bool needrefresh;

/* #define ch(a,b,c,d) */
#define down_and_anchor 
#define move_to_curyx()
#define update_statusbar()
#define COLS 
#define noecho 
#define draw_on_current_display 
#define reset_cursor_position 
#define LINES 
#define nonl 
#define raw 
#define wrefresh 
#define halfdelay 
#define stdscr 0
#define str 
#define gotch 
#define clearrect 
#define floaterscroll 
#define initscr 
#define beginedit 
#define endwin() 0
#define drawscrn 
#define right(a,b,c,d) 0
#define inittextio 
#define anchor 
#define keypad 
#define intrflush 
#define wgetch 
#define draw_on_matrix_screen() 0
#define turn_off_scrolllock() 0
#define textrefresh() 0
#define text2fieldinput(a,b,c,d,e,f) 0

#else /* not GUI_ONLY */

#ifdef NCURSES_VERSION

#define USE_NODELAY 0 /* because ncurses has halfdelay(), which we prefer */

#else /* not ncurses */

#ifdef NO_HALFDELAY
#define USE_NODELAY 1
#else
#define USE_NODELAY 0
#endif /* NO_HALFDELAY */

#endif /* NCURSES_VERSION */

extern fl_bool needrefresh;

#define textrefresh() { \
			UIupdatestatus(); \
			move_to_curyx(); \
			refresh(); \
			needrefresh = FALSE; \
		      }

int draw_on_current_display(ClientData clientData, Tcl_Interp *interp,
			    int argc, const char *argv[]);
int reset_cursor_position(ClientData clientData, Tcl_Interp *interp,
			  int argc, const char *argv[]);

int clearrect(ClientData clientData, Tcl_Interp *interp,
	      int argc, const char *argv[]);
int anchor(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]);
int down_and_anchor(ClientData clientData, Tcl_Interp *interp,
		     int argc, const char *argv[]);
int right(ClientData clientData, Tcl_Interp *interp,
	  int argc, const char *argv[]);
int str(ClientData clientData, Tcl_Interp *interp,
	int argc, const char *argv[]);
int ch(ClientData clientData, Tcl_Interp *interp,
       int argc, const char *argv[]);

void text2fieldinput(char *label1, char *init1, 
		     char *label2, char *init2,
		     char **result1, char **result2);
void gotch(int c);
void inittextio(void);

void move_to_curyx(void);

#define MATRIX_SCREEN 0
#define NO_MATRIX_SCREEN 1
#define EDIT_SCREEN 2
void drawscrn(int scrn);
void displayscrn(int scrn);
#define draw_on_matrix_screen() \
	(hasWindows ? 0 : (drawscrn(MATRIX_SCREEN), 1))

#define turn_off_scrolllock() TclDo("turn_off_scrolllock")

void floaterscroll(int n);

void update_statusbar(void);

void beginedit(char *, char);

#endif /* GUI_ONLY */
#endif /* ifndef __TEXT_H__ */
