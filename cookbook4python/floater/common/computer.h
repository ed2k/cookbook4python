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
#if 0
/* computer.h */
/* useful stuff from br.c */

extern int myseat;
extern char *curastate;
extern char *curpstate;
extern char curcontract[10];
extern int curtrump;
extern char initialcards[4][35];
extern int currestricks, currespoints;

extern char declarer(char *auction, int dealer);

extern char myturntobid(void);
extern char myturntoplay(void);
extern char dummyturntoplay(void);
extern char executebid(char *b);
extern char cardmember(char *card, char *cards);
extern char suitmember(char suit, char *cards);

extern char *remainingcards(int seat);
extern char suitled(char *pstate);

extern char executeplay(char *p);

extern char playsmall_ifpossible(char suit);
extern void playsmall(char suit);
extern void followsmall(void);
extern void makerandomplay(void);
extern void makerandombid(void);
#endif
