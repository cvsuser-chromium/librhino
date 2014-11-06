/*-----------------------------------------------------------------------
 * File: REM.C
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



/* Module name: rem.c
   Computes the remainder of division a by b
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"

#define ShiftRight(m) { posb=m.length-1;\
                        for (posa=0; posa<posb; ++posa)\
                          m.value[posa]=\
                            (m.value[posa]>>1)|(m.value[posa+1]<<31);\
                        m.value[posb]>>=1;\
                        if (m.value[posb]==0 && posb>=1)\
                          m.length=posb;}

void __stdcall ICL_Rem (RSAData *a, RSAData *b, RSAData *r)
{
  register int	posa, posb;
  int		worddiff, bdiff;
  RSAData	sb, rem;
  ICLWord	sbarray[2*MODULUS+2], remarray[2*MODULUS+2];
  ICLWord	msw, *bval;

  if (ICL_Compare (a, b)<0) {    /* a = a mod b   already */
    bval = a->value;
    for (posa=a->length-1; posa>=0; --posa)
      r->value[posa] = bval[posa];
    r->length = a->length;
    for (posa=a->length; posa<MODULUS; ++posa)  /* fill upper words with 0 */
        r->value[posa] = 0;
    return;
  }
  rem.value = remarray;
  sb.value = sbarray;

/* sbval will contain the shifted b-value thru iterations
   rem contains the computed partial remainder
*/
  bval=a->value;
  for (posa=a->length-1; posa>=0; --posa)
    remarray[posa]=bval[posa];

/* find the difference btw a and b in bits */
  msw = remarray[(rem.length=a->length)-1];
  for (posa=0; msw!=0; ++posa)
    msw>>=1;
  msw = b->value[b->length-1];
  for (posb=0; msw!=0; ++posb)
    msw>>=1;
/* posa and posb indicate the actual bit indices of MSWs */
  worddiff = rem.length - b->length - (posa<posb);
  bdiff = (posa>=posb) ? posa-posb : (8*sizeof(ICLWord))+posa-posb;

/* OK, the difference is 32*worddiff + bdiff.
Now shift b and copy to sb */
  bval = b->value;
  if (bdiff==0) {
    for (posa=worddiff+(posb=b->length-1); posb>=0; --posb, --posa)
      sbarray[posa]=bval[posb];
    sb.length = rem.length;
  }
  else {
    for (posa=worddiff+(posb=b->length-1); posb>0; --posb, --posa)
      sbarray[posa]=(bval[posb] << bdiff) | (bval[posb-1] >> (32-bdiff));
    sbarray[worddiff] = bval[0] << bdiff;
    sbarray[rem.length]=0;
    sbarray[b->length+worddiff] = bval[b->length-1] >> (32-bdiff);
    sb.length = rem.length + (sbarray[rem.length]!=0);
  }

/* fill the lower words with 0 */
  for (posa=worddiff-1; posa>=0; --posa)
    sbarray[posa]=0;

/* OK, now start the reduction loop */
  worddiff=(worddiff << 5) + bdiff;     /* number of bits to scan */
  while (worddiff-->=0) {
    if (ICL_Compare (&rem, &sb)>=0)
      ICL_Subtract (&rem, &sb);
    ShiftRight (sb);
  }

/* copy the remainder in rem to r */
  bval = r->value;
  for (posa=0; posa<rem.length; ++posa)
    bval[posa] = remarray[posa];
  r->length = rem.length;

/* fill the upper words with 0's */
  while (posa<MODULUS)
    bval[posa++]=0;
}

void __stdcall RSA_Rem (RSAData *a, RSAData *b, RSAInt *r)
{
  register int	posa, posb;
  int		worddiff, bdiff;
  RSAData	sb, rem;
  ICLWord	sbarray[2*MODULUS+2], remarray[2*MODULUS+2];
  ICLWord	msw, *bval;

  if (ICL_Compare (a, b)<0) {    /* a = a mod b   already */
    bval = a->value;
    for (posa=a->length-1; posa>=0; --posa)
      r->value[posa] = bval[posa];
    r->length = a->length;
    for (posa=a->length; posa<MODULUS; ++posa)  /* fill upper words with 0 */
        r->value[posa] = 0;
    return;
  }
  rem.value = remarray;
  sb.value = sbarray;

/* sbval will contain the shifted b-value thru iterations */
  bval=a->value;
  for (posa=a->length-1; posa>=0; --posa)
    remarray[posa]=bval[posa];
  rem.length=a->length;

/* find the difference btw a and b in bits */
  msw = remarray[rem.length-1];
  for (posa=0; msw!=0; ++posa)
    msw>>=1;
  msw = b->value[b->length-1];
  for (posb=0; msw!=0; ++posb)
    msw>>=1;
  worddiff = rem.length - b->length - (posa<posb);
  bdiff = (posa>=posb) ? posa-posb : 32+posa-posb;

/* OK, the difference is 32*worddiff + bdiff. Now shift b and copy to sb */
  bval = b->value;
  if (bdiff==0) {
    for (posa=worddiff+(posb=b->length-1); posb>=0; --posb, --posa)
      sbarray[posa]=bval[posb];
    sb.length = rem.length;
  }
  else {
    for (posa=worddiff+(posb=b->length-1); posb>0; --posb, --posa)
      sbarray[posa]=(bval[posb] << bdiff) | (bval[posb-1] >> (32-bdiff));
    sbarray[worddiff] = bval[0] << bdiff;
    sbarray[rem.length]=0;
    sbarray[b->length+worddiff] = bval[b->length-1] >> (32-bdiff);
    sb.length = rem.length + (sbarray[rem.length]!=0);
  }

/* fill the lower words with 0 */
  for (posa=worddiff-1; posa>=0; --posa)
    sbarray[posa]=0;

/* OK, now start the reduction loop */
  worddiff=(worddiff << 5) + bdiff;     /* number of bits to scan */
  while (worddiff-->=0) {
    if (ICL_Compare (&rem, &sb)>=0)
      ICL_Subtract (&rem, &sb);
    ShiftRight (sb);
  }

/* copy the remainder in rem to r */
  bval = r->value;
  for (posa=0; posa<rem.length; ++posa)
    bval[posa] = remarray[posa];
  r->length = rem.length;

/* fill the upper words with 0's */
  while (posa<MODULUS)
    bval[posa++]=0;
}
