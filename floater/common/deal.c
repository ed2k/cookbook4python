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
#include "calendar.h"
#include "tickler.h"
#include "deal.h"
#include "br.h"

/* number of times to churn number generator in reseeding process */
#define RESEED_LINCONGS 20

char *seattostring[4] = {"North", "East", "South", "West"};
char seattochar[4] = {'N', 'E', 'S', 'W'};

int oneb[16] = {Nonevul, NS, EW, Both, NS, EW, Both, Nonevul,
			EW, Both, Nonevul, NS, Both, Nonevul, NS, EW};
static char *vs[4] = {"EW", "None", "Both", "NS"};
static char dlr[4] = {'N', 'E', 'S', 'W'};
char suits[] = "cdhs";

char card_to_char[] = "23456789TJQKA";

/* see Knuth for explanation of linear congruential method */
/* basic idea: x[n+1] = (a * x[n] + c) mod m */
/* I'm using a = 33343, c = 902761, m = 2 to 330th power */
/* This a is too small, so actual only use every REPEAT'th x[i] */
#define REPEAT 13

static int a = 33343;
static int c = 902761;

#define RANDXSIZE 11

/* 30 bits of x in each */
static unsigned int randx[RANDXSIZE] = {1276095297 & 0x3FFFFFFF,
					  1191902957 & 0x3FFFFFFF,
					  1599290432 & 0x3FFFFFFF,
					  693925063 & 0x3FFFFFFF,
					  569902669 & 0x3FFFFFFF,
					  677092687 & 0x3FFFFFFF,
					  1938724611 & 0x3FFFFFFF,
					  969352630 & 0x3FFFFFFF,
					  1659091758 & 0x3FFFFFFF,
					  671380899 & 0x3FFFFFFF,
					  1719302732 & 0x3FFFFFFF};

#define NUMGOODBITS 30
#define GOODBITS(x) ((x) & 0x3FFFFFFF)
#define STARTINDEX 10
#define STARTBITINDEX (NUMGOODBITS - 1)
#define LASTBITINDEX 0
#define BUMPINDEX { \
		      if (bitindex-- == LASTBITINDEX) \
		      { \
			  bitindex = STARTBITINDEX; \
			  ind--; \
		      } \
		  }
#define STOPINDEX 5
#define STOPBITINDEX (NUMGOODBITS >> 1)

#define BITS_PER_HAND 350

#define validassert() assert(ind >= STOPINDEX && bitindex >= LASTBITINDEX \
		     && ind <= STARTINDEX && bitindex <= STARTBITINDEX)

/* out of NUMGOODBITS * RANDXSIZE bits, how many do we actually use? */
/* (depends on stop and start indices) */
#define BITS_PER_LINCONG 164

static int ind = STARTINDEX;
static int bitindex = STARTBITINDEX;

static int mask[30] = {0x1,
			 0x2,
			 0x4,
			 0x8,
			 0x10,
			 0x20,
			 0x40,
			 0x80,
			 0x100,
			 0x200,
			 0x400,
			 0x800,
			 0x1000,
			 0x2000,
			 0x4000,
			 0x8000,
			 0x10000,
			 0x20000,
			 0x40000,
			 0x80000,
			 0x100000,
			 0x200000,
			 0x400000,
			 0x800000,
			 0x1000000,
			 0x2000000,
			 0x4000000,
			 0x8000000,
			 0x10000000,
			 0x20000000};

#define LOWHALF(x) ((x) & 0x7FFF)
#define HIGHHALF(x) ((x) >> 15)
#define LOWTOHIGH(x) ((LOWHALF(x)) << 15)
#define EXTRAS(x) ((x) >> 30)

static void printbits(void)
{
  int i;

  ind = STARTINDEX;      
  bitindex = STARTBITINDEX;
  
  for (i=0; i < RANDXSIZE * NUMGOODBITS; i++) {
    putchar((randx[ind] & mask[bitindex]) ? '1' : '0');
    BUMPINDEX;
  }
  puts("");

  ind = STARTINDEX;      
  bitindex = STARTBITINDEX;
}

static void printhex(void)
{
  int i, accum;

  ind = STARTINDEX;      
  bitindex = STARTBITINDEX;
  
  for (i = 0; i < (RANDXSIZE * NUMGOODBITS) / 4; i++) {
    accum = ((randx[ind] & mask[bitindex]) ? 8 : 0);
    BUMPINDEX;
    accum += ((randx[ind] & mask[bitindex]) ? 4 : 0);
    BUMPINDEX;
    accum += ((randx[ind] & mask[bitindex]) ? 2 : 0);
    BUMPINDEX;
    accum += ((randx[ind] & mask[bitindex]) ? 1 : 0);
    BUMPINDEX;
    printf("%1x", accum);
  }
  puts("");

  ind = STARTINDEX;      
  bitindex = STARTBITINDEX;
}

static void lincong(void)
{
  unsigned int scratch[RANDXSIZE];
  register unsigned int carry, temp;
/*  static int calls = 0;*/
  register int i;
  int repeats;

/*  printbits();*/
/*  printf("used %d random bits so far\n", ++calls * BITS_PER_LINCONG);*/

  ind = STARTINDEX;      
  bitindex = STARTBITINDEX;

  for (repeats = 0; repeats < REPEAT; repeats++) {
    /* multiply x by a (mod m) */
    carry = 0;
    for (i = 0; i < RANDXSIZE; i++) {
      temp = a * HIGHHALF(randx[i]);
      scratch[i] = a * LOWHALF(randx[i]) + carry + LOWTOHIGH(temp);
      carry = EXTRAS(scratch[i]) + HIGHHALF(temp);
    }
    
    /* add c (mod m) */
    carry = c;
    for (i = 0; carry != 0 && i < RANDXSIZE; i++) {
      scratch[i] += carry;
      carry = EXTRAS(scratch[i]);
    }

    for (i = 0; i < RANDXSIZE; i++) randx[i] = GOODBITS(scratch[i]);
  }
}  

/* discard n bits */
static void skipbits(int n)
{
  validassert();
  while (ind != STARTINDEX || bitindex != STARTBITINDEX) {
    if (n-- == 0) return;
    BUMPINDEX;
  }

  while ((n -= BITS_PER_LINCONG) > 0) lincong();
  n += BITS_PER_LINCONG;

  while (n-- > 0) BUMPINDEX;
  validassert();
}

static int randbit(void)
{
  unsigned int r;

  if (ind == STOPINDEX && bitindex == STOPBITINDEX) lincong();
  
  r = randx[ind] & mask[bitindex];

  BUMPINDEX;

  return (r == 0) ? 0 : 1;
}

/* return a pseudo-random number in the range 0 to i-1 */
/* i is assumed to be at most 64, though this limitation is easily removed */
int dealrand(int i)
{
  int bits;
  int r, v, startv;

  if (i > 8) {
    if (i > 16)
      if (i > 32) bits = 6;
      else bits = 5;
    else bits = 4;
  }
  else if (i > 4) bits = 3;
  else if (i > 2) bits = 2;
  else bits = 1;
    
/*  printf("dealrand(%d): bits=%d ", i, bits);*/

  startv = 1 << (bits - 1);
  do {
    v = startv;
    r = 0;
    while (v > 0 && r < i) {
      if (randbit()) {r += v; /*printf("o");*/}
/*      else printf("z");*/
      v >>= 1;
    }
  } while (r >= i);

/*  printf(" (%d)\n", r);*/
  return r;
}

int print_suit(int *hand, int suit)
{
  int i,f;

  i = 0;
  while (i < 13 && whatsuit(hand[i]) > suit) i++;
  f = i;
  while (i < 13 && whatsuit(hand[i]) == suit) {
    putchar(card_to_char[whatcard(hand[i])]);
    i++;
  }
  if ((i - f) == 0) {
    putchar('-');
    return 1;
  }
  else return (i - f);
}

#define newline() putchar('\n')
void print_hand(int *deck, int num, int dealer, int vul)
{
  int i,j,k;

  printf("Deal %d, Dealer: %c, %s vul\n", num+1, dlr[dealer], vs[vul]);
  SUITLOOP(i) {
      printf("%*s", NStab, "");
    print_suit(deck + 13 * North, i);
    newline();
  }
  SUITLOOP(i) {
    printf("%*s", Wtab, "");
    printf("%*s", Etab - print_suit(deck + 13 * West, i), "");
    print_suit(deck + 13 * East, i);
    newline();
  }
  SUITLOOP(i) {
    printf("%*s", NStab, "");
    print_suit(deck + 13 * South, i);
    newline();
  }
}


int char2int_card(char c){
    if((c - '2') < 10) return (int)(c-'2');
    if( c == 'A')return 12;
    if (c=='K') return 11;
    if (c=='Q') return 10;
    if (c=='J') return 9;
    if (c=='T') return 8;
}
/*
0 AT32.6.872.98765 
 */
void setup_bridge_deck(char *seed, int *deck){
    
    //    if (*(seed+1) != ' ')
  //-yisu hajack the program
  // don't understand the implementation that generate the deck from seed
    int seat = *seed - '0';
    int suit = 3;
    int pos = 0;
    seed += 2;
    while (*seed != 0) {
        if (*seed == '|'){
            seat = *(seed+1) - '0';
            suit = 3;
            pos = 0;
            seed += 3;
        } else if (*seed == '.') {
            suit -= 1;
            seed ++;
        }        
        deck[seat*13+pos] = suit*13 + char2int_card(*seed) ;
        seed++;
        pos ++;
    }
    int i;
    for(i=0;i<52;i++)printf("%d ",(deck[i] %13)+2);
    printf("new deal\n");
}
void deal(int *deck, int verbose, int num, int dealer, int vul, int decksize)
{
    return;
  register int i, k, t;
  
  validassert();

  for (i=0; i<decksize; i++) deck[i] = i;
  if (decksize == 51) {
    /* remove the deuce of clubs */
    deck[51] = 0; 
    deck[0] = 51;
  }
  for (i=0; i<decksize-1; i++) {
    k = i + dealrand(decksize - i);
    t = deck[k];
    deck[k] = deck[i];
    deck[i] = t;
  }
  
/*  if (verbose > 0) { sortdeal(deck); print_hand(deck, num, dealer, vul); }*/
  validassert();
}

void sortdeal(int *deck, int decksize, int cardsperhand)
{
  register int i, j, k, t;
  return;
  for (i=0; i<decksize; i+=cardsperhand)
    for (j=0; j<cardsperhand - 1; j++)
      for (k=j; k<cardsperhand; k++)
	if (deck[k+i] > deck[j+i]) {
	  t = deck[k+i];
	  deck[k+i] = deck[j+i];
	  deck[j+i] = t;
	}
}


char suit_to_char(int s)
{
  switch(s) {
  case Spades: return 's';
  case Hearts: return 'h';
  case Diamonds: return 'd';
  case Clubs: return 'c';
  case Notrump: return 'n';
  default: assert(0);
  }
}

int char_to_strain(char c)
{
  switch (toupper(c)) {
  case 'S': return Spades;
  case 'H': return Hearts;
  case 'D': return Diamonds;
  case 'C': return Clubs;
  case 'N': return Notrump;
  default:  assert(0);
  }
}
 
int inverse_card_to_char(char c)
{
  static fl_bool inited = FALSE;
  static int a[256];

  if (!inited) {
    int i;

    for (i = 0; i < 256; i++) a[i] = -1;
    i = 0;
    while (card_to_char[i] != '\0') {
      a[tolower(card_to_char[i])] = i;
      a[toupper(card_to_char[i])] = i;
      i++;
    }
    inited = TRUE;
  }
  assert(a[c] >= 0);
  return a[c];
}
      
/******************************************************************************
getseed() and setseed() convert seeds to and from ascii.
Format is ind \  bitindex \ the seed coded 6atatime.
******************************************************************************/
char *getseed(void)
{
  int i;
  char bits[RANDXSIZE * NUMGOODBITS];
  char *ret;
  int sizeofret;

  for (i=0; i<RANDXSIZE; i++)
    putbits(randx[i], NUMGOODBITS, bits + i * NUMGOODBITS);
  
  ret = markgarbage(salloc(sizeofret=(sizeof(char) * (20 + sizeof(bits) / 6))));
  for (i=0; i<sizeofret; i++) ret[i] = '\0';
  validassert();
  sprintf(ret, "%d\\%d\\", ind, bitindex);
  put6atatime(bits, sizeof(bits), ret + strlen(ret));
  return ret;
}


/*
seed is somethin like 9\15\aiJfmB01x7CKykEtLawMswKYg4lqEHv1jm8Id5vLWxzFOoNZbvDQQG_
 */
void setseed(char *seed)
{
    return;
  char bits[RANDXSIZE * NUMGOODBITS];
  int i;
  int newind, newbitindex;

  newind = parseint(&seed);
  newbitindex = parseint(&seed);
  validassert();
  get6atatime(seed, sizeof(bits), bits);
  for (i=0; i<RANDXSIZE; i++)
    randx[i] = getbits(bits + i * NUMGOODBITS, NUMGOODBITS);
#if 0
  { printf("%s: %d,%d,", myname, newind, newbitindex); printhex(); }
#endif
  ind = newind;
  bitindex = newbitindex;
}

/* start with seed, then skip dealindex deals */
char *makeseed(char *seed, int dealindex)
{
  setseed(seed);
#if DEBUGBR
  status2("Making seed for dealindex ", itoa(dealindex));
#endif
  skipbits(BITS_PER_HAND * dealindex);
  return getseed();
}

/* create a seed using t and i; the same t and i always yield the same result,
   and different ones tend to yield different results */
void reseed(char *t, int i)
{
  unsigned int k;
  unsigned char *s;

  /* printf("reseed(%s, %d) => ", t, i); */
  setseed("10\\29\\MD6tBHCv7tfUzxApXHTHh-AZNoW51PzjpcD5xyW2i47suoBHWjmenpM");
  for (k=0; k<RANDXSIZE; k++) randx[k] ^= GOODBITS(i);
  if (i != GOODBITS(i)) randx[0] ^= GOODBITS(i >> (NUMGOODBITS / 2));

  assert(RANDXSIZE > 2);
  k = 1;
  s = (unsigned char *) t;
  while (*s != '\0') {
    randx[2 + (k % (RANDXSIZE - 2))] ^= GOODBITS(((unsigned int)*s) * k);
    k++;
    s++;
  }
  for (k=0; k<RESEED_LINCONGS; k++) lincong();
  /* puts(getseed()); */
}

/* create and return a random seed based on the date and time */
char *makerndseed(void)
{
  reseed(date_and_time(), unifmod(100000));
  return getseed();
}
