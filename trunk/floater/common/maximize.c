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


/* $Id: maximize.c,v 1.5 2003/12/03 03:50:25 lexspoon Exp $
 *
 * Only compiled into the MS Windows version of Floater.  This file
 * contains a routine for maximizing a window
 */



#include <tkWinInt.h>

/*
 * Event structure for synthetic activation events.  These events are
 * placed on the event queue whenever a toplevel gets a WM_MOUSEACTIVATE
 * message.
 */

typedef struct ActivateEvent {
    Tcl_Event ev;
    TkWindow *winPtr;
} ActivateEvent;

/*
 * A data structure of the following type holds information for
 * each window manager protocol (such as WM_DELETE_WINDOW) for
 * which a handler (i.e. a Tcl command) has been defined for a
 * particular top-level window.
 */

typedef struct ProtocolHandler {
    Atom protocol;		/* Identifies the protocol. */
    struct ProtocolHandler *nextPtr;
				/* Next in list of protocol handlers for
				 * the same top-level window, or NULL for
				 * end of list. */
    Tcl_Interp *interp;		/* Interpreter in which to invoke command. */
    char command[4];		/* Tcl command to invoke when a client
				 * message for this protocol arrives. 
				 * The actual size of the structure varies
				 * to accommodate the needs of the actual
				 * command. THIS MUST BE THE LAST FIELD OF
				 * THE STRUCTURE. */
} ProtocolHandler;

#define HANDLER_SIZE(cmdLength) \
    ((unsigned) (sizeof(ProtocolHandler) - 3 + cmdLength))

/*
 * A data structure of the following type holds window-manager-related
 * information for each top-level window in an application.
 */

typedef struct TkWmInfo {
    TkWindow *winPtr;		/* Pointer to main Tk information for
				 * this window. */
    HWND wrapper;		/* This is the decorative frame window
				 * created by the window manager to wrap
				 * a toplevel window.  This window is
				 * a direct child of the root window. */
    Tk_Uid titleUid;		/* Title to display in window caption.  If
				 * NULL, use name of widget. */
    Tk_Uid iconName;		/* Name to display in icon. */
    TkWindow *masterPtr;	/* Master window for TRANSIENT_FOR property,
				 * or NULL. */
    XWMHints hints;		/* Various pieces of information for
				 * window manager. */
    char *leaderName;		/* Path name of leader of window group
				 * (corresponds to hints.window_group).
				 * Malloc-ed. Note:  this field doesn't
				 * get updated if leader is destroyed. */
    Tk_Window icon;		/* Window to use as icon for this window,
				 * or NULL. */
    Tk_Window iconFor;		/* Window for which this window is icon, or
				 * NULL if this isn't an icon for anyone. */

    /*
     * Information used to construct an XSizeHints structure for
     * the window manager:
     */

    int defMinWidth, defMinHeight, defMaxWidth, defMaxHeight;
				/* Default resize limits given by system. */
    int sizeHintsFlags;		/* Flags word for XSizeHints structure.
				 * If the PBaseSize flag is set then the
				 * window is gridded;  otherwise it isn't
				 * gridded. */
    int minWidth, minHeight;	/* Minimum dimensions of window, in
				 * grid units, not pixels. */
    int maxWidth, maxHeight;	/* Maximum dimensions of window, in
				 * grid units, not pixels, or 0 to default. */
    Tk_Window gridWin;		/* Identifies the window that controls
				 * gridding for this top-level, or NULL if
				 * the top-level isn't currently gridded. */
    int widthInc, heightInc;	/* Increments for size changes (# pixels
				 * per step). */
    struct {
	int x;	/* numerator */
	int y;  /* denominator */
    } minAspect, maxAspect;	/* Min/max aspect ratios for window. */
    int reqGridWidth, reqGridHeight;
				/* The dimensions of the window (in
				 * grid units) requested through
				 * the geometry manager. */
    int gravity;		/* Desired window gravity. */

    /*
     * Information used to manage the size and location of a window.
     */

    int width, height;		/* Desired dimensions of window, specified
				 * in grid units.  These values are
				 * set by the "wm geometry" command and by
				 * ConfigureNotify events (for when wm
				 * resizes window).  -1 means user hasn't
				 * requested dimensions. */
    int x, y;			/* Desired X and Y coordinates for window.
				 * These values are set by "wm geometry",
				 * plus by ConfigureNotify events (when wm
				 * moves window).  These numbers are
				 * different than the numbers stored in
				 * winPtr->changes because (a) they could be
				 * measured from the right or bottom edge
				 * of the screen (see WM_NEGATIVE_X and
				 * WM_NEGATIVE_Y flags) and (b) if the window
				 * has been reparented then they refer to the
				 * parent rather than the window itself. */
    int borderWidth, borderHeight;
				/* Width and height of window dressing, in
				 * pixels for the current style/exStyle.  This
				 * includes the border on both sides of the
				 * window. */
    int configWidth, configHeight;
				/* Dimensions passed to last request that we
				 * issued to change geometry of window.  Used
				 * to eliminate redundant resize operations. */
    HMENU hMenu;		/* the hMenu associated with this menu */
    DWORD style, exStyle;	/* Style flags for the wrapper window. */

    /*
     * List of children of the toplevel which have private colormaps.
     */

    TkWindow **cmapList;	/* Array of window with private colormaps. */
    int cmapCount;		/* Number of windows in array. */

    /*
     * Miscellaneous information.
     */

    ProtocolHandler *protPtr;	/* First in list of protocol handlers for
				 * this window (NULL means none). */
    int cmdArgc;		/* Number of elements in cmdArgv below. */
    char **cmdArgv;		/* Array of strings to store in the
				 * WM_COMMAND property.  NULL means nothing
				 * available. */
    char *clientMachine;	/* String to store in WM_CLIENT_MACHINE
				 * property, or NULL. */
    int flags;			/* Miscellaneous flags, defined below. */
    struct TkWmInfo *nextPtr;	/* Next in list of all top-level windows. */
} WmInfo;

/*
 *----------------------------------------------------------------------
 *
 * maximize_window (based on Tk_WmCmd)
 *
 *	This procedure is invoked to maximize a toplevel window.
 *	Works under MS WINDOWS only.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	Maximizes a window.
 *
 *----------------------------------------------------------------------
 */

int
maximize_window(
    ClientData clientData,	/* Unused. */
    Tcl_Interp *interp,		/* Current interpreter. */
    int argc,			/* Number of arguments. */
    const char **argv           /* Argument strings. */
)
{
  Tk_Window tkwin = Tk_MainWindow(interp);
  TkWindow *winPtr;
  WmInfo *wmPtr;

  if (argc != 2) {
    Tcl_AppendResult(interp, "wrong # args: should be \"",
		     argv[0], " window\"", (char *) NULL);
    return TCL_ERROR;
  }
  winPtr = (TkWindow *) Tk_NameToWindow(interp, argv[1], tkwin);
  if (winPtr == NULL) {
    return TCL_ERROR;
  }
  if (!(winPtr->flags & TK_TOP_LEVEL)) {
    Tcl_AppendResult(interp, "window \"", winPtr->pathName,
		     "\" isn't a top-level window", (char *) NULL);
    return TCL_ERROR;
  }
  wmPtr = winPtr->wmInfoPtr;
  TkpWmSetState(winPtr, ZoomState);
  return TCL_OK;
}
