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


int IMPtable[] =
    {0, 0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4,
       5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9,
       9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11,
       11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
       13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14,
       14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
       14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
       15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
       16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
       17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18,
       18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
       18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
       19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
       20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
       20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
       21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
       21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
       21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
       22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
       22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
       22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
       23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
       23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
       23, 23};

static int IMP(int pointdiff)
{
  if (pointdiff >= 4000) return 24;
  else if (pointdiff <= -4000) return -24;
  else if (pointdiff < 0) return -IMPtable[-pointdiff / 10];
  else return IMPtable[pointdiff / 10];
}


/* size is the number of results; scores[] contain the results */
/* output is stored in fscores[] */
void IMPscore(int *scores, int size, float *fscores)
{
  int i, j, t;
  

  for (i=0; i<size; i++) fscores[i] = 0.0;

  for (i=0; i<size-1; i++)
    for (j=i+1; j<size; j++) {
      t = IMP(scores[i] - scores[j]);
      fscores[i] += t;
      fscores[j] -= t;
    }
      
  /* normalize by dividing by number of comparisons */
  if (size > 1) for (i=0; i<size; i++) fscores[i] /= (float)(size - 1);
}

/* size is the number of results, scores[] contain the results (sorted) */
/* output is stored in fscores[] */
void MPscore(int *scores, int size, float *fscores)
{
  int i, j, t;
  
  /* to first approximation, first guy gets size-1 matchpoints, etc. */
  for (i=0; i<size; i++) fscores[i] = size - i - 1;

  /* fixup for ties */
  for (i=0; i<size-1; ) {
    for (j=i+1; j < size && scores[i] == scores[j]; j++);
    /* i through j-1 are tied---give them all the average score of the group */
    t = (fscores[i] + fscores[j-1]) / 2.0;
    while (i < j) fscores[i++] = t;
  }
      
  /* convert to percentage */
  if (size > 1) for (i=0; i<size; i++) fscores[i] *= (100.0 / (size - 1));
  else fscores[0] = 50.0;
}

/* size is the number of results, scores[] contain the results */
/* output is stored in fscores[] */
void YUK3score(int *scores, int size, float *fscores)
{
  int i, j, t;
  return;

  for (i=0; i<size; i++) fscores[i] = 0.0;

  for (i=0; i<size-1; i++)
    for (j=i+1; j<size; j++) {
      t = IMP(scores[i] - scores[j]);
      fscores[i] += t;
      fscores[j] -= t;
    }
      
  /* normalize by dividing by number of comparisons */
  if (size > 1) for (i=0; i<size; i++) fscores[i] /= (float)(size - 1);
}




