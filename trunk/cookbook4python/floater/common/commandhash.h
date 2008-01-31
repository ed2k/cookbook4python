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
#ifndef __COMMANDHASH_H__
#define __COMMANDHASH_H__

#define AMBIGUOUS ((char *) 1)

#define NEW_ISSTRINGCOMMAND(s) \
	atob(TclDo3("uplevel #0 {info exists {isstringcommand(", \
		    itoa((long) s), ")}}"))
#define OLD_ISSTRINGCOMMAND(s) (((long) s) & 1)
#define ISSTRINGCOMMAND(s) \
  (using_old_isstringcommand ? OLD_ISSTRINGCOMMAND(s) : NEW_ISSTRINGCOMMAND(s))
extern fl_bool using_old_isstringcommand;

typedef int (*commandptr)(char *);

commandptr commandhash(char *s);
void cmdbind(char *s, commandptr binding);
void stringbind(char *s, char *binding);

void hashdump(void);

#endif
