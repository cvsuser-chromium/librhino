/*-----------------------------------------------------------------------
 * File: MODSUB.C
 *
 * Copyright (c) 1995-2000 Intel Corporation. All rights reserved.
 *-----------------------------------------------------------------------
 */
/*
 * INTEL CONFIDENTIAL 
 * This file, software, or program is supplied under the terms of a 
 * license agreement or nondisclosure agreement with Intel Corporation 
 * and may not be copied or disclosed except in accordance with the 
 * terms of that agreement. This file, software, or program contains 
 * copyrighted material and/or trade secret information of Intel 
 * Corporation, and must be treated as such. Intel reserves all rights 
 * in this material, except as the license agreement or nondisclosure 
 * agreement specifically indicate. 
 */
/* 
 * WARNING: EXPORT RESTRICTED. 
 * This software listing contains cryptographic methods and technology. 
 * It is export restricted by the Office of Defense Trade Controls, United 
 * States Department of State and cannot be downloaded or otherwise 
 * exported or re-exported (i) into (or to a national or resident of) Cuba, 
 * Iraq, Libya, Yugoslavia, North Korea, Iran, Syria or any other country 
 * to which the US has embargoed goods; or (ii) to anyone on the US 
 * Treasury Department's list of Specially Designated Nationals or the US 
 * Commerce Department's Table of Denial Orders. By downloading or using 
 * this product, you are agreeing to the foregoing and you are representing 
 * and warranting that you are not located in, under the control of, or a 
 * national or resident of any such country or on any such list. 
 */



/* Module name:  modsub.c
   Computes t = a - b mod n
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"

void __stdcall ICL_ModSub (RSAData *a, RSAData *b, RSAData *n, RSAData *t)
{
  register ICLWord sum, msb;
  ICLWord	*bval, *aval, bneg, carry;
  long		length, pos;
  ICLWord	tmparray[MODULUS];

/* just to prevent pointer evaluation to access value[] array */
  aval=a->value;
  bval=b->value;
/* perform the subtraction and write the result onto tmp */
  length = n->length;
  carry=1;
  for(pos=0; pos<length; ++pos) {
    sum=(aval[pos] & 0x7fffffff) + ((bneg=~bval[pos]) & 0x7fffffff) + carry;
    msb=(aval[pos] >> 31) + (bneg >> 31) + (sum >> 31);
    carry = (msb & 2)!=0;
    if (msb&1)
      tmparray[pos]=sum | 0x80000000;
    else
      tmparray[pos]=sum & 0x7fffffff;
  }
/* check carry-out.
if (carry==0) and (a!=b) then add n to the current result.
Note that, if a==b, then carry=1, which obsoletes carry checking.
*/
  if (!carry && (ICL_Compare (a, b)!=0)) {           /* which means that a<b, add n to tmp */
    carry = 0;
    aval=n->value;
    for (pos=0; pos<length; ++pos) {
      sum = (tmparray[pos] & 0x7fffffff) + (aval[pos] & 0x7fffffff) + carry;
      msb = (tmparray[pos] >> 31) + (aval[pos] >> 31) + (sum >> 31);
      carry = (msb & 2)!=0;
      if (msb & 1)
        tmparray[pos] = sum | 0x80000000;
      else
        tmparray[pos] = sum & 0x7fffffff;
    }
    tmparray[pos]=0;
  }
/* copy tmparray to t */
  aval = t->value;
  for (pos=n->length-1; pos>=0; --pos)
    aval[pos] = tmparray[pos];
/* fill the upper words with 0's */
  for (pos=n->length-1; pos>1 && aval[pos]==0;)
    --pos;
  t->length = pos + 1;
}
