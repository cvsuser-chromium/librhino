/*-----------------------------------------------------------------------
 * File: DIV.C
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



/* Module name: div.c
   Multi-precision integer division.
   Only the quotient is returned.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"
#include "../../include/rsakg.h"

#define ShiftLeftOneBit(w) {register int cmp=w.length;\
			    w.value[cmp]=0;\
			    while (cmp-->0) {\
			      w.value[cmp+1]|=w.value[cmp] >> 31;\
			      w.value[cmp]   =w.value[cmp] << 1;\
			    }\
			    cmp=w.length;\
			    w.length=(w.value[cmp]!=0) ? cmp+1 : cmp;\
			   }

/* A simple restoring division */
void __stdcall	ICL_Div (RSAData *dividend, RSAData *divisor, RSAData *divide)
{
  register ICLWord	bitmask;
  register int		cnt, blength;
  RSAData		rem;
  ICLWord		*tval, *aval, remarray[MODULUS];

  blength=divisor->length;
/* Save time: if dividend<divisor then the result is obviously zero! */
  tval=divide->value;
  divide->length=1;
  for (cnt=0; cnt<MODULUS; ++cnt)
    tval[cnt] = 0;
  if (dividend->length<blength) return;

/* start with zero remainder. Note that, Subtract routine *MUST* be able to
deal with denormalized numbers. "rem" is a denormalized 'zero'! */
  rem.value=remarray;
  rem.length=blength;
  aval = dividend->value;
  for (cnt=0; cnt<MODULUS; ++cnt)
    remarray[cnt]=0;
/* start looping */
  for (cnt=dividend->length-1; cnt>=0; --cnt) {
    tval[cnt]=0;
    for (bitmask=0x80000000; bitmask!=0; bitmask>>=1) {
      remarray[0]|= (aval[cnt] & bitmask)!=0;     /* get the next a bit  */
      tval[cnt]<<=1;                   /* adjust the quotient             */
      if (ICL_Compare (&rem, divisor)>=0) {   /* subtract and shift left         */
	    ICL_Subtract (&rem, divisor);
	    tval[cnt] |= 1;                /* it divides! put 1 into quotient */
      }
      ShiftLeftOneBit (rem);           /* and shift partial remainder     */
    }
  }
/* This function discards the remainder. */
  cnt=dividend->length-1;
/* discard the leading 0's and find the normalized length of the result */
  while (cnt>0 && tval[cnt]==0)
    --cnt;
  divide->length=cnt+1;
}
