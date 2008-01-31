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
#include <stdlib.h>
#include <sys/types.h>
#ifdef TIME_H
#include <time.h>
#else
#include <sys/time.h>
#endif
#include "floater.h"

#ifdef USE_RAND
#define random rand
#define srandom srand
#endif

/* generate a random number modulo mod */
long unifmod(long mod)
{
  static char inited = FALSE;
  long mask, t;

  assert(mod > 0);

  if (!inited) {
    srandom((int) time(NULL));
    inited = TRUE;
  }

  /* create bitmask with leftmost one at (mod - 1)'s leftmost one */
  /* e.g. if mod is 237, mask should be 255; mod = 4 => mask = 3 */
  mask = 0;
  t = mod - 1;
  while (t > 0) {
    t >>= 1;
    mask = mask * 2 + 1;
  }
  
  while ((t = (random() & mask)) >= mod);
  return t;
}

/* reseed, using s to help us churn */
void churnsimpleseed(char *s)
{
  int i = 0, j;

  while (*s++ != '\0') i = 3 * i + *s + unifmod(1000);
  j = unifmod(20);
  srandom(i);
  while (j-- > 0) unifmod(111);
}

#if 0
  {
    int i;

    printf("testing unifmod(4): ");
    for (i=0; i<20; i++) printf("%d ", unifmod(4));
    printf("\ntesting unifmod(10): ");
    for (i=0; i<20; i++) printf("%d ", unifmod(10));
    puts("");
  }
#endif
