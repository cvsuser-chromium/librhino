/*-----------------------------------------------------------------------
 * File: SUBTRACT.C
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



/* Module name: subtract.c
   Computes t = t - a
   assuming t>=a
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"

/* return t = t-a */
void __stdcall ICL_Subtract (RSAData *t, RSAData *a)
{
  int			pos;
  register ICLWord	*aval, *tval, msb, sum;
  ICLWord		aneg, carry;
  long			length;

  aval=a->value;
  tval=t->value;
  length = a->length;

  carry=1;
  for(pos=0; pos<length; ++pos) {
    sum=(tval[pos] & 0x7FFFFFFF) + ((aneg=~aval[pos]) & 0x7FFFFFFF) + carry;
    msb=(tval[pos] >> 31) + (aneg >> 31) + (sum >> 31);
    carry = (msb & 2)!=0;
    if (msb & 1)
      tval[pos]=sum | 0x80000000;
    else
      tval[pos]=sum & 0x7FFFFFFF;
  }
/* ... just add the carry-out (0/1) for the residue, if any */
  length = t->length;
  for (; pos<length; ++pos) {
    sum=(tval[pos] & 0x7FFFFFFF) + carry + 0x7FFFFFFF;
    msb=(tval[pos] >> 31) + (sum >> 31) + 1;
    carry = (msb & 2)!=0;
    if (msb & 1)
      tval[pos]=sum | 0x80000000;
    else
      tval[pos]=sum & 0x7FFFFFFF;
  }
/* compute the length of t */
  --pos;
  while (pos>0 && tval[pos]==0)
    --pos;
  t->length = pos + 1;
}
