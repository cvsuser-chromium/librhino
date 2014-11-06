/*-----------------------------------------------------------------------
 * File: EEA.C
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



/* Module name: eea.c
   Extended Euclidean Algorithm using Berlekamp's version.
   On return:
     b*upp - a*vpp = (-1)(k-1)*rpp
   where k is the iteration number when the partial remainder becomes 0.
   if k is odd
	      a*a^-1 = 1 mod b    ----->   a^-1 = b - vpp
	      b*b^-1 = 1 mod a    ----->   b^-1 = t
   if k is even
	      a*a^-1 = 1 mod b    ----->   a^-1 = s
	      b*b^-1 = 1 mod a    ----->   b^-1 = a - upp
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"
#include "../../include/rsakg.h"
#include "../../include/iclproc.h"


void __stdcall	ICL_EEA (RSAData *a, RSAData *b, RSAData *vpp, RSAData *upp, RSAData *rpp, int *k)
{
  RSAData	q, r, rp, u, up, v, vp;
  ICLWord	qarray[2*MODULUS],
  		rarray[2*MODULUS], rparray[2*MODULUS],
		uarray[2*MODULUS], uparray[2*MODULUS],
		varray[2*MODULUS], vparray[2*MODULUS];

/* fill the arrays with 0's */
  for (*k=0; *k<2*MODULUS; ++*k)
    qarray[*k]=rarray[*k]=rparray[*k]=
    	uarray[*k]=uparray[*k]=varray[*k]=vparray[*k] = 0;
/* value initialization */
  q.value  = qarray;
  r.value  = rarray;
  rp.value = rparray;
  u.value  = uarray;
  up.value = uparray;
  v.value  = varray;
  vp.value = vparray;

/* Initialize the variables */
  *k=0;
  upp->value[0]=0;    upp->length=1;
  up.value[0]=1;      up.length=1;
  vpp->value[0]=1;    vpp->length=1;
  vp.value[0]=0;      vp.length=1;

  ICL_MoveWord (a,  rpp);			/* rpp = a       */
  ICL_MoveWord (b, &rp);

  /* start looping on a zero rp (remainder) */
  while (!ICL_IsZero (rp)) {
    ICL_Div (rpp, &rp, &q);

    ICL_Mul (&q, &up, &u);
    ICL_Add (&u, upp, &u);
    while (u.length>1 && u.value[u.length-1]==0)
        --u.length;

    ICL_Mul (&q, &vp, &v);
    ICL_Add (&v, vpp, &v);
    while (v.length>1 && v.value[v.length-1]==0)
        --v.length;

    ICL_Rem (rpp, &rp, &r);
/* discard the leading 0's since ICL_Rem does not do this */
    while (r.length>1 && rarray[r.length-1]==0)
    	--r.length;

    ICL_MoveWord (&up, upp);
    ICL_MoveWord (&u, &up);

    ICL_MoveWord (&vp, vpp);
    ICL_MoveWord (&v, &vp);

    ICL_MoveWord (&rp, rpp);
    ICL_MoveWord (&r, &rp);
    
    ++*k;
  }
/* Now,   b*upp - a*vpp = (-1)^(k-1)*rpp   */
}
