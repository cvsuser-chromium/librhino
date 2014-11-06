/*-----------------------------------------------------------------------
 * File: MODADD.C
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



/* Module name: modadd.c
   Computes t = a + b mod n
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"

void __stdcall ICL_ModAdd (RSAData *a, RSAData *b, RSAData *n, RSAData *t)
{
  register ICLWord	*aval, *bval, *tval;
  register long		pos;
  unsigned long		sum, carry, msb;
  int			length;

/* Computation loop */
  aval=a->value;
  bval=b->value;
  tval=t->value;
/* perform addition by using the smaller's length, then .... */
  length = (a->length > b->length) ? b->length : a->length;
  carry = 0;
  for (pos=0; pos<length; ++pos) {
    sum=(aval[pos] & 0x7fffffff) + (bval[pos] & 0x7fffffff) + carry;
    msb=(aval[pos] >> 31) + (bval[pos] >> 31) +
      (sum >> 31);
    carry = (msb & 2)!=0;
    if (msb & 1)
      tval[pos]=sum | 0x80000000;
    else
      tval[pos]=sum & 0x7fffffff;
  }
/* just add the carry-out (0/1) for the residue, if any */
  if (a->length > b->length)
    length = a->length;
  else {
    length = b->length;
    aval=bval;
  }
  for (; pos<length; ++pos) {
    if (carry==0)
      tval[pos]=aval[pos];
    else {
      sum=(aval[pos] & 0x7fffffff) + carry;
      msb=(aval[pos] >> 31) + (sum >> 31);
      carry = (msb & 2)!=0;
      if (msb&1)
	tval[pos]=sum | 0x80000000;
      else
	tval[pos]=sum & 0x7fffffff;
    }
  }
/* OK, get the length and do the mod operation */
  tval[pos]=carry;  
  while (pos>0 && tval[pos]==0)
    --pos;
  t->length=pos+1;
/* check is the result is larger than n, if so, subtract n from t */
  pos = ICL_Compare (t, n);
  if (pos==0) {               /* t == n */
    t->length=1;              /* the result is zero */
    t->value[0]=0;
  }
  else if (pos>0)             /*   t > n   */
    ICL_Subtract (t, n);       /* t = t - n */
}
