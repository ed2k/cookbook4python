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
#ifndef __DEAL_H__
#define __DEAL_H__

/* from the famous saying, "0neb neb0 eb0n b0ne" */
#define ONEB(x) oneb[(x) & 15]
extern int oneb[];
extern char *seattostring[];
extern char seattochar[];
extern char card_to_char[];

char suit_to_char(int s);
int inverse_card_to_char(char c);

void deal(int *deck, int verbose, int num, int dealer, int vul, int decksize);
void sortdeal(int *deck, int decksize, int cardsperhand);

#define NStab 17
#define Etab 30
#define Wtab 0
#define EW 0
#define Nonevul 1
#define Both 2
#define NS 3
#define True 1
#define False 0
#define White 0
#define Red 3
#define Fav 1
#define Unfav 4
#define Noseat -1
#define North 0
#define East 1
#define South 2
#define West 3
#define rightyseat(s) (((s) + 3) % 4)
#define leftyseat(s) (((s) + 1) % 4)
#define Clubs 0
#define Diamonds 1
#define Hearts 2
#define Spades 3
#define Notrump 4
#define SUITLOOP(i) for (i=Spades; i>=Clubs; i--)

#define whatsuit(card) ((card)/13)
#define whatcard(card) ((card)%13)

#define DEALNO_TO_DEALER(x) (((x) + disnumplayers - 1) % disnumplayers)
#define DEALNO_TO_DEALERx(x,y) (((x) + (y) - 1) % (y))
#define DEALNO_TO_DEALER4(x) (((x) + 3) % 4)

char *makeseed(char *seed, int dealindex);
void setseed(char *seed);
char *getseed(void);
void reseed(char *t, int i);
char *makerndseed(void);

int char_to_strain(char);

#endif
