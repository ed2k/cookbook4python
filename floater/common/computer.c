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

/* computer.c - a computer player */


#include "floater.h"
#include "floatcmd.h"
#include "tickler.h"
#include "comm.h"
#include "br.h"
#include "deal.h"
#include "calendar.h"
#include "computer.h"

#if 0
/* stubs for the linker if more interesting functions not defined */
void makecomputerbid(void) 
{ 
  makerandombid(); 
}
void makecomputerplay(void) 
{ 
  makerandomplay(); 
}
#else

void makecomputerbid(void)
{
  char *bids[] = {"pass", "x", "xx", "1c", "1d", "1h", "1s", "1n",
		  "2c", "2d", "2h", "2s", "2n",
		  "3c", "3d", "3h", "3s", "3n",
		  "pass", "x", "xx", "1c", "1d", "1h", "1s", "1n",
		  "pass", "x", "xx", "1c", "1d", "1h", "1s", "1n",
		  "pass", "x", "xx", "1c", "1d", "1h", "1s", "1n",
		  "pass", "x", "xx", "1c", "1d", "1h", "1s", "1n",
		  "2c", "2d", "2h", "2s", "2n",
		  "pass", "x", "xx", "1c", "1d", "1h", "1s", "1n",
		  "2c", "2d", "2h", "2s", "2n",
		  "pass", "x", "xx", "1c", "1d", "1h", "1s", "1n",
		  "2c", "2d", "2h", "2s", "2n",
		  "3c", "3d", "3h", "3s", "3n",
		  "pass", "pass", "pass", "pass", 
		  "pass", "pass", "pass", "pass", 
		  "pass", "pass", "pass", "pass", 
		  "pass", "pass", "pass", "pass", 
		  "pass", "pass", "pass", "pass", 
		  "pass", "pass", "pass", "pass", 
		  "pass", "pass", "pass", "pass", 
		  "pass", "pass", "pass", "pass", 
		  "pass", "pass", "pass", "pass", 
		  "pass", "pass", "pass", "pass", 
		  "x", "xx", "1c", "1d", "1h", "1s", "1n",
		  "2c", "2d", "2h", "2s", "2n",
		  "3c", "3d", "3h", "3s", "3n",
		  "4c", "4d", "4h", "4s", "4n",
		  "4c", "4d", "4h", "4s", "4n",
		  "4c", "4d", "4h", "4s"};
  /*
  , "4n",
      "5c", "5d", "5h", "5s", "5n",
      "6c", "6d", "6h", "6s", "6n",
      "7c", "7d", "7h", "7s", "7n", "x", "x", "x", "x", "x"};
      */

  char *hand = remainingcards(myseat);   /* use initialcards? */
  int c = 0, d = 0, h = 0, s = 0;
  int hcp = 0;
  int i = 0;
  int len = strlen(curastate);
  char longsuit, shortsuit;
  int lsl, ssl;
  char bid[3];

  while(hand[i]) {
    switch(tolower(hand[i])) {
    case 'c': c++; break;
    case 'd': d++; break;
    case 'h': h++; break;
    case 's': s++; break;
    }
    switch(tolower(hand[i+1])) {
    case 'a': hcp += 4; break;
    case 'k': hcp += 3; break;
    case 'q': hcp += 2; break;
    case 'j': hcp += 1; break;
    }
    i += 2;
  }

  /* note that this picks higher of equals */
  if (c > d && c > h && c > s) {longsuit = 'c'; lsl = c;}
  else if (d > h && d > s) {longsuit = 'd'; lsl = d;}
  else if (h > s) {longsuit = 'h'; lsl = h;}
  else {longsuit = 's'; lsl = s;}
	
  if (c < d && c < h && c < s) {shortsuit = 'c'; ssl = c;}
  else if (d < h && d < s) {shortsuit = 'd'; ssl = d;}
  else if (h < s) {shortsuit = 'h'; ssl = h;}
  else {shortsuit = 's'; ssl = s;}
	
  if (len <= 2) { /* first or second seat */ 
    if (hcp >= 21) {
      strcpy(bid, "2c");   
    } else if (hcp >= 15 && hcp <= 17 && ssl > 1 && lsl < 5) {
      strcpy(bid, "1n");
    } else if (hcp >= 12) {
      sprintf(bid, "1%c", longsuit);
    } else if (lsl > 6 || (lsl > 5 && longsuit != 'c')) {
      sprintf(bid, "%d%c", lsl - 4, longsuit);
    } else {
      strcpy(bid, "p");
    }
  } else {
    hand = remainingcards(myseat ^ 2);  /* cheat; look at partners hand */
    i = 0;
    while(hand[i]) {
      switch(tolower(hand[i])) {
      case 'c': c++; break;
      case 'd': d++; break;
      case 'h': h++; break;
      case 's': s++; break;
      }
      switch(tolower(hand[i+1])) {
      case 'a': hcp += 4; break;
      case 'k': hcp += 3; break;
      case 'q': hcp += 2; break;
      case 'j': hcp += 1; break;
      }
      i += 2;
    }
		
    /* note that this picks higher of equals */
    if (c > d && c > h && c > s) {longsuit = 'c'; lsl = c;}
    else if (d > h && d > s) {longsuit = 'd'; lsl = d;}
    else if (h > s) {longsuit = 'h'; lsl = h;}
    else {longsuit = 's'; lsl = s;}
		
    if (c < d && c < h && c < s) {shortsuit = 'c'; ssl = c;}
    else if (d < h && d < s) {shortsuit = 'd'; ssl = d;}
    else if (h < s) {shortsuit = 'h'; ssl = h;}
    else {shortsuit = 's'; ssl = s;}
		
    if (hcp >= 37) {
      sprintf(bid, "7%c", (lsl > 7 ? longsuit : 'n'));
    } else if (hcp >= 33) {
      sprintf(bid, "6%c", (lsl > 7 ? longsuit : 'n'));
    } else if (hcp >= 29 && (longsuit == 'c' || longsuit == 'd')) {
      if (ssl < 5 || lsl > 8)
	sprintf(bid, "5%c", longsuit);
      else
	sprintf(bid, "3n");
    } else if (hcp >= 26) {
      if ((longsuit == 'h' || longsuit == 's') && lsl >= 8) {
	sprintf(bid, "4%c", longsuit);
      } else {
	sprintf(bid, "3n");
      }
    } else if (lsl > 7) {
      sprintf(bid, "%d%c", lsl - 6, longsuit);
    } else {
      sprintf(bid, "p");
    }
  }	   
	
  if (executebid(bid)) return;
  if (strlen(curastate) > 8 && unifmod(10) > 2) { assert(executebid("p")); }
  else while (!executebid(bids[unifmod(sizeof(bids)/sizeof(bids[0]))]));
}

void makecomputerplay(void)
{
  char *plays[] = {"sa", "ha", "da", "ca",
		   "s2", "h2", "d2", "c2",
		   "s3", "h3", "d3", "c3",
		   "s4", "h4", "d4", "c4",
		   "s5", "h5", "d5", "c5",
		   "s6", "h6", "d6", "c6",
		   "s7", "h7", "d7", "c7",
		   "s8", "h8", "d8", "c8",
		   "s9", "h9", "d9", "c9",
		   "st", "ht", "dt", "ct",
		   "sj", "hj", "dj", "cj",
		   "sq", "hq", "dq", "cq",
		   "sk", "hk", "dk", "ck"};
	
  char play[3];
  char *tmp;
  int seat = dummyturntoplay ? myseat ^ 2 : myseat;
	
  if (strlen(curpstate) % 8 == 2)		/* second hand low */
    if(playsmall_ifpossible(suitled(curpstate))) return;
    else if (strlen(curpstate) % 8 == 3) {
      /* third hand high */
      if ((tmp = strchr(remainingcards(seat), suitled(curpstate))) != NULL) {
	play[0] = tmp[0]; play[1] = tmp[1]; play[2] = '\0';
	if (executeplay(play)) return;
      }
    }
  if (!executeplay(plays[0]))
    while (!executeplay(plays[unifmod(sizeof(plays)/sizeof(plays[0]))]));
}




#endif


