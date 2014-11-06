/*-----------------------------------------------------------------------
 * File: MONPRO.C
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



/* Module name: monpro.c
   Montgomery Product (The m-ary add-shift method)
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"

#define LOWWORDMASK         ( ~(ICLWord)0 >> (WORDSIZE/2) )



void __stdcall MontProduct (ICLWord ModExpN0prime, RSAData ModExpModulus,
	RSAData *a, RSAInt *b, RSAData *t)
{
  register long	i, j;
  ICLWord	*tval, *aval, *bval, ai, m, C, S, sum, carry;
  ICLWord	a0b0, a0b1, a1b0, a1b1, mlow, mhigh;

  tval = t->value;
  aval = a->value;
  bval = b->value;
  t->length = ModExpModulus.length;

/* initialize t with zero, max(t.length) = s+1 words */
  for (i=MODULUS+1; i>=0; --i)
    tval[i]=0;

/* go into multiplication loop, by shifting at the same time */
  for (i=0; i<ModExpModulus.length; ++i) {
    ai=aval[i];
    C = 0;
    for (j=0; j<ModExpModulus.length; ++j) {
/* (C,S):=t[j] + b[j]*a[i] + C */
      a0b0 = (LOWWORDMASK & ai) * (LOWWORDMASK & bval[j]);
      a0b1 = (LOWWORDMASK & ai) * (bval[j] >> (WORDSIZE/2));
      a1b0 = (ai >> (WORDSIZE/2)) * (LOWWORDMASK & bval[j]);
      a1b1 = (ai >> (WORDSIZE/2)) * (bval[j] >> (WORDSIZE/2));
      sum=(a0b0 & LOWWORDMASK)+(C & LOWWORDMASK)+(tval[j] & LOWWORDMASK);
      carry=(a0b0 >> (WORDSIZE/2))+(a0b1 & LOWWORDMASK)+(a1b0 & LOWWORDMASK)+
        (C >> (WORDSIZE/2))+(tval[j] >> (WORDSIZE/2))+(sum >> (WORDSIZE/2));
      S=(sum & LOWWORDMASK)+(carry << (WORDSIZE/2));
      C=a1b1+(a0b1 >> (WORDSIZE/2))+(a1b0 >> (WORDSIZE/2))+(carry >> (WORDSIZE/2));
/* t[j]:=S */
      tval[j]=S;
    }

/* (C,S):=t[s] + C */
    sum=(C & LOWWORDMASK) + (tval[ModExpModulus.length] & LOWWORDMASK);
    carry=(C >> (WORDSIZE/2)) + (tval[ModExpModulus.length] >> (WORDSIZE/2)) +
        (sum >> (WORDSIZE/2));
/* t[s]:=S;   t[s+1]:=C */
    tval[ModExpModulus.length]=(sum & LOWWORDMASK) + (carry << (WORDSIZE/2));
    tval[ModExpModulus.length+1]=(carry >> (WORDSIZE/2));

/* m = t[0] * n0prime  */
    m = tval[0] * ModExpN0prime;
    mlow = m & LOWWORDMASK;
    mhigh = m >> (WORDSIZE/2);

/* (C,S):=m*n[0]+ t[0]     discard sum, it must be 0 */
    a0b0 = mlow * (LOWWORDMASK & ModExpModulus.value[0]);
    a0b1 = mlow * (ModExpModulus.value[0] >> (WORDSIZE/2));
    a1b0 = mhigh * (LOWWORDMASK & ModExpModulus.value[0]);
    a1b1 = mhigh * (ModExpModulus.value[0] >> (WORDSIZE/2));
    sum=(a0b0 & LOWWORDMASK)+(tval[0] & LOWWORDMASK);
    carry=(a0b0 >> (WORDSIZE/2))+(a0b1 & LOWWORDMASK)+(a1b0 & LOWWORDMASK)+
      (tval[0] >> (WORDSIZE/2))+(sum >> (WORDSIZE/2));
    C=a1b1 + (a1b0 >> (WORDSIZE/2))+(a0b1 >> (WORDSIZE/2))+(carry >> (WORDSIZE/2));

    for (j=1; j<ModExpModulus.length; ++j) {
/* (C,S):=t[j] + m*n[j] + C */
      a0b0 = mlow * (LOWWORDMASK & ModExpModulus.value[j]);
      a0b1 = mlow * (ModExpModulus.value[j] >> (WORDSIZE/2));
      a1b0 = mhigh * (LOWWORDMASK & ModExpModulus.value[j]);
      a1b1=  mhigh * (ModExpModulus.value[j] >> (WORDSIZE/2));

      sum=(a0b0 & LOWWORDMASK)+(C & LOWWORDMASK)+(tval[j] & LOWWORDMASK);
      carry = (a0b0 >> (WORDSIZE/2))+(a0b1 & LOWWORDMASK)+(a1b0 & LOWWORDMASK)+
        (C >> (WORDSIZE/2))+(tval[j] >> (WORDSIZE/2))+(sum>>(WORDSIZE/2));
      S = (sum & LOWWORDMASK)+(carry << (WORDSIZE/2));
      C = a1b1+(a0b1 >> (WORDSIZE/2))+(a1b0 >> (WORDSIZE/2))+(carry >> (WORDSIZE/2));
/* t[j-1]:=S */      
      tval[j-1]=S;
    }
/* (C,S):=t[s] + C */
    sum=(tval[ModExpModulus.length] & LOWWORDMASK) + (C & LOWWORDMASK);
    carry = (tval[ModExpModulus.length] >> (WORDSIZE/2)) + (C >> (WORDSIZE/2)) +
        (sum >> (WORDSIZE/2));
/* t[s-1]:=S */
    tval[ModExpModulus.length-1] = (sum & LOWWORDMASK) + (carry << (WORDSIZE/2));
/* t[s]:=t[s+1] + C */
    tval[ModExpModulus.length] = tval[ModExpModulus.length+1] + (carry >> (WORDSIZE/2));
  }
  tval[ModExpModulus.length+1] = 0;
/* do not discard the leading zero words! */
  i = ModExpModulus.length;
/* now, t.length==n.length */
  if (tval[i]==0)
    while (i-->0) {
      if (tval[i] < ModExpModulus.value[i])  return;
      if (tval[i] > ModExpModulus.value[i])  break;
    }
/* the t->value[ModExp.length] word must be zero */
  tval[ModExpModulus.length]=0;
/* Note that,if a subtraction is needed, t.length=n.length */
  C=1;     /* initial carry is 1 */
  for (i=0; i<ModExpModulus.length; ++i) {
    S = (tval[i] & (~(ICLWord)0 >> 1)) + ((m=~ModExpModulus.value[i]) & (~(ICLWord)0 >> 1)) + C;
    carry=(tval[i] >> (WORDSIZE-1)) + (m >> (WORDSIZE-1)) + (S >> (WORDSIZE-1));
    C = (carry & 2)!=0;
    tval[i] = (carry & 1) ? (S | ((ICLWord)1 << (WORDSIZE-1))) : (S & (~(ICLWord)0 >> 1));
  }
}