/*-----------------------------------------------------------------------
 * File: MONSQU.C
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



/* Module name: monsqu.c
   Montgomery Square (The m-ary add-shift method)
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"

/* This function reads n0prime and the modulus n from global variables
   ModExpN0prime and ModExpModulus, thereby eliminating passing the
   same variable over and over again for function call overhead.
*/

#ifndef _MSC_VER
void __stdcall MontSquare (ICLWord ModExpN0prime, RSAData ModExpModulus,
	RSAData *a, RSAData *t)
{
  register int i, j;
  ICLWord	*tval, *aval, ai, aj, m, Cx, C, S, sum, carry;
  ICLWord	a0b0, a0b1, a1b0, a1b1, low, high;

  tval = t->value;
  aval = a->value;
  t->length = ModExpModulus.length;

/* initialize t with zero, max(t.length) = s+2 words */
  for (i=MODULUS+1; i>=0; --i)
    tval[i]=0;

/* go into multiplication loop, by shifting at the same time */
  for (i=0; i<ModExpModulus.length; ++i) {

/* compute the single multiplication, i.e. ai*ai and add it to t */
/* (C,S) = t[i] + a[i]*a[i];   t[i]=S  */
    ai=aval[i];
    a0b1 = (low = 0x0000FFFF & ai) * (high = ai >> 16);
    a0b0 = low * low;
    a1b1 = high * high;
    sum=(a0b0 & 0x0000FFFF) + (tval[i] & 0x0000FFFF);
    carry=(a0b0 >> 16)+((a0b1 & 0x0000FFFF) << 1)+(tval[i] >> 16)+(sum >> 16);
    S=(sum & 0x0000FFFF)+(carry << 16);
    C=a1b1 + ((a0b1 >> 16)<<1) + (carry >> 16);
    tval[i]=S;
    Cx=0;

    for (j=i+1; j<ModExpModulus.length; ++j) {
/* (Cx,C,S) = t[j] + 2*a[i]*a[j] + C + Cx*2^w;      t[j]=S   */
      aj=aval[j];
      a0b0 = low * (0x0000FFFF & aj);
      a0b1 = low * (aj >> 16);
      a1b0 = high * (0x0000FFFF & aj);
      a1b1 = high * (aj >> 16);
      sum=((a0b0 & 0x0000FFFF)<<1)+(C & 0x0000FFFF)+(tval[j] & 0x0000FFFF);
      carry=(((a0b0 >> 16)+(a0b1 & 0x0000FFFF)+(a1b0 & 0x0000FFFF))<<1)+
        (C >> 16)+(tval[j] >> 16)+(sum >> 16);
      S=(sum & 0x0000FFFF) + (carry << 16);
      tval[j]=S;
      sum=(((a1b1 & 0x0000FFFF)+(a1b0 >> 16)+(a0b1 >> 16))<<1)+
        (carry >> 16) + (Cx & 0x0000FFFF);
      carry=(sum >> 16) + ((a1b1 >> 16)<<1) + (Cx >> 16);
      C=(sum & 0x0000FFFF) + (carry << 16);
      Cx = carry >> 16;
    }
/* (C,S) = t[nlength]+C+Cx*2^w;    t[nlength]=S;  t[nlength+1]=C; */
    S = (tval[ModExpModulus.length] & 0x7FFFFFFF) + (C & 0x7FFFFFFF);
    C = (tval[ModExpModulus.length] >> 31) + (C >> 31) + (S >> 31);
    tval[ModExpModulus.length] = (C & 1)!=0 ? (S | 0x80000000) : (S & 0x7FFFFFFF);
    tval[ModExpModulus.length+1] = ((C & 2)!=0) + Cx;
/* the if (... clause is removed!  */
    m = tval[0] * ModExpN0prime;
    low = m & 0x0000FFFF;
    high = m >> 16;
    a0b0 = low * (0x0000FFFF & ModExpModulus.value[0]);
    a0b1 = low * (ModExpModulus.value[0] >> 16);
    a1b0 = high * (0x0000FFFF & ModExpModulus.value[0]);
    a1b1 = high * (ModExpModulus.value[0] >> 16);
    sum=(a0b0 & 0x0000FFFF)+(tval[0] & 0x0000FFFF);
    carry=(a0b0 >> 16)+(a0b1 & 0x0000FFFF)+(a1b0 & 0x0000FFFF)+
      (tval[0] >> 16)+(sum >> 16);
    C=a1b1 + (a1b0 >> 16)+(a0b1 >> 16)+(carry >> 16);
/* t[j] = t[j] + m*n[j] + C  */
    for (j=1; j<ModExpModulus.length; ++j) {
      a0b0 = low * (0x0000FFFF & ModExpModulus.value[j]);
      a0b1 = low * (ModExpModulus.value[j] >> 16);
      a1b0 = high * (0x0000FFFF & ModExpModulus.value[j]);
      a1b1=  high * (ModExpModulus.value[j] >> 16);

      sum=(a0b0 & 0x0000FFFF)+(C & 0x0000FFFF)+(tval[j] & 0x0000FFFF);
      carry = (a0b0 >> 16)+(a0b1 & 0x0000FFFF)+(a1b0 & 0x0000FFFF)+
        (C >> 16)+(tval[j] >> 16)+(sum>>16);
      S = (sum & 0x0000FFFF)+(carry << 16);
      C = a1b1+(a0b1 >> 16)+(a1b0 >> 16)+(carry >> 16);
      tval[j-1]=S;
    }
/* (C,S) := t[nlength] + C
   t[nlength-1] := S
   
*/
    S = (tval[ModExpModulus.length] & 0x7FFFFFFF) + (C & 0x7FFFFFFF);
    C = (tval[ModExpModulus.length] >> 31) + (C >> 31) + (S >> 31);
    tval[ModExpModulus.length-1] = (C & 1)!=0 ? (S | 0x80000000) : (S & 0x7FFFFFFF);
    C = (C & 2)!=0;

    sum=(tval[ModExpModulus.length+1] & 0x0000FFFF) + (C & 0x0000FFFF);
    carry = (tval[ModExpModulus.length+1] >> 16) + (C >> 16) + (sum>>16);
    tval[ModExpModulus.length] = (sum & 0x0000FFFF) + (carry << 16);
    tval[ModExpModulus.length+1] = carry >> 16;
  }
/* do not discard the leading zero words */
  j = i = (tval[ModExpModulus.length]==0) ? ModExpModulus.length : ModExpModulus.length+1;
  t->length = ModExpModulus.length;
/* now, either t==n or t>n   */
  if (i==ModExpModulus.length) {
    while (i-->0) {
      if (tval[i] < ModExpModulus.value[i])  return;
      if (tval[i] > ModExpModulus.value[i])  break;
    }
  }
/* Note that,if a subtraction is needed, t>n by at most one word */
  C=1;     /* initial carry is 1 */
  for (i=0; i<j; ++i) {
    S = (tval[i] & 0x7FFFFFFF) + ((m=~ModExpModulus.value[i]) & 0x7FFFFFFF) + C;
    carry=(tval[i] >> 31) + (m >> 31) + (S >> 31);
    C = (carry & 2)!=0;
    tval[i] = (carry & 1) ? (S | 0x80000000) : (S & 0x7FFFFFFF);
  }
/* do not discard the leading zero words */
}

#else   /* if thsi is a Microsoft Visual C/C++ compiler */

void __stdcall MontSquare (ICLWord ModExpN0prime, RSAData ModExpModulus,
	RSAData *a, RSAData *t)
{
  register int i, j;
  ICLWord	*tval, *aval, ai, aj, m, Cx, C, S, carry;
  ICLDWord	CS, CS2;

  tval = t->value;
  aval = a->value;
  t->length = ModExpModulus.length;

/* initialize t with zero, max(t.length) = s+2 words */
  for (i=MODULUS+1; i>=0; --i)
    tval[i]=0;

/* go into multiplication loop, by shifting at the same time */
  for (i=0; i<ModExpModulus.length; ++i) {

/* compute the single multiplication, i.e. ai*ai and add it to t */
/* (C,S) = t[i] + a[i]*a[i];   t[i]=S  */
    ai=aval[i];

	CS = (ICLDWord)ai * (ICLDWord)ai + (ICLDWord)tval[i];
    tval[i]=LowWord (CS);
	C = HighWord (CS);
    Cx=0;

    for (j=i+1; j<ModExpModulus.length; ++j) {
/* (Cx,C,S) = t[j] + 2*a[i]*a[j] + C + Cx*2^w;      t[j]=S   */
      aj=aval[j];

	  CS2 = (ICLDWord)ai*(ICLDWord)aj;
	  CS = (ICLDWord)tval[j] + (ICLDWord)C;

	  tval[j] = LowWord (CS) + LowWord (CS2 << 1);
	  CS = (ICLDWord)Cx + (ICLDWord)HighWord (CS + (CS2 << 1));
	  C = LowWord (CS);
	  Cx = HighWord (CS) + LowWord (CS2 >> (2*WORDSIZE-1));
    }

/* (C,S) = t[nlength]+C+Cx*2^w;    t[nlength]=S;  t[nlength+1]=C; */
    CS = (ICLDWord)tval[ModExpModulus.length] + C + ((ICLDWord)Cx << WORDSIZE);
	tval[ModExpModulus.length] = LowWord (CS);
	tval[ModExpModulus.length+1] = HighWord (CS);

/* the if (... clause is removed!  */
    m = tval[0] * ModExpN0prime;
/* (C,S) = m*n[0] + t[0] */
	CS = (ICLDWord)m * ModExpModulus.value[0] + tval[0];
	C = HighWord (CS);		/* discard the low word it's 0 */

/* t[j] = t[j] + m*n[j] + C  */
    for (j=1; j<ModExpModulus.length; ++j) {
	  CS = (ICLDWord)m * ModExpModulus.value[j] + tval[j] + C;
	  tval[j-1] = LowWord (CS);
	  C = HighWord (CS);
    }
/* (C,S) := t[nlength] + C;
   t[nlength-1] := S;
   (C,S) := C + t[nlength+1]
   t[nlength] := S
   t[nlength+1] := C
*/
	CS = (ICLDWord)tval[ModExpModulus.length] + (ICLDWord)C;
	tval[ModExpModulus.length-1] = LowWord (CS);
	C = HighWord (CS);

	CS = (ICLDWord)tval[ModExpModulus.length+1] + C;
	tval[ModExpModulus.length] = LowWord (CS);
	tval[ModExpModulus.length+1] = HighWord (CS);
  }
/* do not discard the leading zero words */
  j = i = (tval[ModExpModulus.length]==0) ? ModExpModulus.length : ModExpModulus.length+1;
/* now, either t==n or t>n   */
  if (i==ModExpModulus.length) {
    while (i-->0) {
      if (tval[i] < ModExpModulus.value[i])  return;
      if (tval[i] > ModExpModulus.value[i])  break;
    }
  }
/* Note that,if a subtraction is needed, t>n by at most one word */
  C=1;     /* initial carry is 1 */
  for (i=0; i<j; ++i) {
    S = (tval[i] & 0x7FFFFFFF) + ((m=~ModExpModulus.value[i]) & 0x7FFFFFFF) + C;
    carry=(tval[i] >> 31) + (m >> 31) + (S >> 31);
    C = (carry & 2)!=0;
    tval[i] = (carry & 1) ? (S | 0x80000000) : (S & 0x7FFFFFFF);
  }
/* do not discard the leading zero words */
}


#endif  /* _MSC_VER */
