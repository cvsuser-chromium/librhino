/*-----------------------------------------------------------------------
 * File: GCD.C
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



/* Module name: gcd.c
   Greatest Common Divisor for ICL using Stein's algorithm.
   Part of RSA Key Generation.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"
#include "../../include/rsakg.h"
#include "../../include/iclproc.h"

#define ShiftRight(m) { register int len=m.length-1;\
			for (i=0; i<len; ++i)\
			  m.value[i]=(m.value[i]>>1) | (m.value[i+1]<<31);\
			m.value[len]>>=1;\
			if (m.value[len]==0 && len>=1)\
			  m.length=len;}


/* g = gcd (a,b) */
void __stdcall ICL_GCD (RSAData *a, RSAData *b, RSAData *g)
{
  register int	i, shift;
  int			power2, len;
  ICLWord		*val;
  RSAData		n1, n2, t;
  ICLWord		n1array[MODULUS], n2array[MODULUS], tarray[MODULUS];

/* allocate space for local variables */
  t.value=tarray;
  n1.value=n1array;
  n2.value=n2array;

/* space allocation is OK :-| Initialize the variables */
  ICL_MoveWord (a, &n1);		/* n1 = a */
  ICL_MoveWord (b, &n2);		/* n2 = b */

/* make any one of n1 or n2 odd */
  power2=0;
  while (ICL_IsEven (n1) && ICL_IsEven (n2)) {
    ShiftRight (n1);
    ShiftRight (n2);
    ++power2;
  }

/* if n2 is even, exchange n1 with n2 to make n2 odd */
  if (ICL_IsEven (n2)) {
    val=n1.value;      n1.value=n2.value;      n2.value=val;
    len=n1.length;     n1.length=n2.length;    n2.length=len;
  }

/* start the main loop, delta=|n1-n2|, poll on zero n1 */
  while (!ICL_IsZero (n1)) {
    while (ICL_IsEven (n1))		/* make n1 odd, n2 was odd at entry */
      ShiftRight (n1);
    if (ICL_Compare (&n1, &n2)<0) {
	  ICL_MoveWord (&n2, &t);			/* t = n1     */
/* compute   t := n2 - n1 */
	  ICL_Subtract (&t, &n1);
      ICL_MoveWord (&n1, &n2);			/* n2 = n1     */
    }
    else {
/* compute t := n1 - n2 */
		ICL_MoveWord (&n1, &t);			/* t = n1 */
		ICL_Subtract (&t, &n2);		/* t = n1 - n2 */
	}
    ShiftRight (t);
    ICL_MoveWord (&t, &n1);
  }

/* shift and copy 2^power2 * n2 to g  */
  len = power2 >> 5;
  shift = power2 & 0x1F;
  val = g->value;
  if (shift!=0) {
    for (i=1; i<n2.length; ++i)
      val[i+len]=(n2.value[i] << shift) | (n2.value[i-1] >> (32-shift));
    val[len] = n2.value[0] << shift;
    val[len+n2.length]=n2.value[n2.length-1] >> (32-shift);
  }
/* shift amount is a multiple of word size -- copy by displacement */
  else
    for (i=0; i<n2.length; ++i)
      val[i+len]=n2.value[i];

/* Note that, val = g->value */
  for (i=len-1; i>0; --i)		/* fill any lower word(s) with 0's */
    val[i]=0;

/* discard any leading 0's and adjust the length */
  len = len + n2.length - 1;
  while (len>0 && val[len]==0)
    --len;
  g->length = len+1;
}
