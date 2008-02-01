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

static char codebook[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static int invcodebook(char c)
{
  char *p;

  return ((p = strchr(codebook, c)) == NULL ? 0 : (p - codebook));
}

/*
   Take the lower n bits of integer source, and put them, one bit per char,
   into dest.  The biggest goes in dest[0].
*/
void putbits(int source, int n, char *dest)
{
  while (n--) *dest++ = (source & (1 << n)) ? 1 : 0;
}

/* Take n bits from source, and return the integer they represent.
source is in one-bit-per-char format.  Assumes n < sizeof(int) * 8. */
int getbits(char *source, int n)
{
  int result = 0;

  while (n--) if (*source++) result |= (1 << n);
  return result;
}  

/* Take the bits, 6 at a time, and put them into printable form.
bits is in one-bit-per-char format. */
void put6atatime(char *bits, int n, char *dest)
{
  if ((n % 6) != 0) {
    *dest++ = codebook[getbits(bits, n % 6)];
    n -= (n % 6);
    bits += (n % 6);
  }
    
  while (n > 0) {
    *dest++ = codebook[getbits(bits, 6)];
    n -= 6;
    bits += 6;
  }
}
    
/* the inverse of put6atatime */
void get6atatime(char *src, int n, char *bits)
{
  if ((n % 6) != 0) {
    putbits(invcodebook(*src++), n % 6, bits);
    n -= (n % 6);
    bits += (n % 6);
  }

  while (n > 0) {
    putbits(invcodebook(*src++), 6, bits);
    n -= 6;
    bits += 6;
  }
}

/* Try to generate a unique (worldwide) name based on addr and port. */
/* If the addr is not a numerical xx.xx.xx.xx IP address, then pick
   a random number. */
void setbogusnewname(char *name, char *addr, char *port)
{
  int b[5], i;
  char bits[48];

  b[0] = b[1] = b[2] = b[3] = b[4] = 0;
  /* parse IP addr as four 8 bit integers, b[0] through b[3] */
  if (isin('!', addr)) while (*addr++ != '!');
  while (isspace(*addr)) addr++;
  while (isdigit(*addr)) b[0] = b[0] * 10 + *addr++ - '0';
  if (!(*addr++ == '.')) goto randomaddr;
  while (isdigit(*addr)) b[1] = b[1] * 10 + *addr++ - '0';
  if (!(*addr++ == '.')) goto randomaddr;
  while (isdigit(*addr)) b[2] = b[2] * 10 + *addr++ - '0';
  if (!(*addr++ == '.')) goto randomaddr;
  while (isdigit(*addr)) b[3] = b[3] * 10 + *addr++ - '0';
  while (isspace(*addr)) addr++;
#if 0
  printf("Parsed IP addr as %d.%d.%d.%d\n", b[0], b[1], b[2], b[3]);
#endif
  if (!(*addr == '\0')) goto randomaddr;
  goto parseport;

 randomaddr:
  for (i = 0; i < 4; i++) b[i] = unifmod(256);

 parseport:
  /* parse port */
  while (isspace(*port)) port++;
  while (isdigit(*port)) b[4] = b[4] * 10 + *port++ - '0';
  while (isspace(*port)) port++;
  assert(*port == '\0');
  assert(b[4] < 65536);

  for (i=0; i<4; i++) putbits(b[i], 8, bits + 8 * i);
  putbits(b[i], 16, bits + 32);

  strcpy(name, "!");
  put6atatime(bits, 48, name + 1); /* set name[1] ... name[8] */
  name[9] = '\0';
}
