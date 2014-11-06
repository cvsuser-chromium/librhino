/*-----------------------------------------------------------------------
 * File: ADD.C
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


     
/* Module name: add.c
   Computes t = a + b
     
   The length of the result, t, is set to MODULUS.
*/
     
#include "../../include/icl.h"
#include "../../include/rsa.h"
     
void __stdcall ICL_Add (RSAData *a, RSAData *b, RSAData *t) 
{
  register ICLWord     *aval, *bval, *tval; 
  register long          pos, length;
  unsigned long          sum, carry, msb;
     
  aval=a->value;
  bval=b->value;
  tval=t->value;
/* perform addition by using the smaller's length, then .... */
  length = (a->length > b->length) ? b->length : a->length; 
  carry = 0;
  for (pos=0; pos<length; ++pos) {
    sum=(aval[pos] & (WORDMASK >> 1)) +
          (bval[pos] & (WORDMASK >> 1)) + carry;
    msb=(aval[pos] >> (WORDSIZE-1)) + (bval[pos] >> (WORDSIZE-1)) +
      (sum >> (WORDSIZE-1));
    carry = (msb & 2)!=0;
    if (msb & 1)
      tval[pos]=sum | ((ICLWord)1 << (WORDSIZE-1));
    else
      tval[pos]=sum & (WORDMASK >> 1);
  }
/* just add the carry-out (0/1) for the residue, if any */
  if (a->length > b->length)
    length = a->length;
  else {
    length = b->length;
    aval = bval;
  }
  for (; pos<length; ++pos) {
    if (carry==0)
      tval[pos]=aval[pos];
    else {
      sum=(aval[pos] & (WORDMASK >> 1)) + carry; 
      msb=(aval[pos] >> (WORDSIZE-1)) + (sum >> (WORDSIZE-1)); 
      carry = (msb & 2)!=0;
      if (msb & 1)
     tval[pos]=sum | ((ICLWord)1 << (WORDSIZE-1));
      else
     tval[pos]=sum & (WORDMASK >> 1);
    }
  }
  tval[pos]=carry;
/* leading 0s are not discarded */
  while (pos>0 && tval[pos]==0)
      --pos;
  t->length = pos+1;
}
