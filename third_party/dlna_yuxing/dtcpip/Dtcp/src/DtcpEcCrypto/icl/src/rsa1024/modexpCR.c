/*-----------------------------------------------------------------------
 * File: MODEXPCR.C
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



/* Module name: modexpCR.c
   Modular exponentiation using CRT

   The higher words from RSAData.length to MODULUS are assumed
   to be filled with 0's before this function is called.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"

/* t = C^d mod n,   where
   							dp = d mod (p-1)
							dq = d mod (q-1)
*/
void __stdcall ICL_ModExpCRT (RSAData *C, RSAData *dp, RSAData *dq,
		   RSAData *p, RSAData *q, RSAData *coeff, RSAData *t)
{
  RSAData	M1, M2, temp1, temp2;
/* M1 and M2 require (MODULUS+2) words of space since
   MonSqu requires (MODULUS+2) words of space
*/
  ICLWord	M1array[MODULUS+2], M2array[MODULUS+2];
  ICLWord	temp1array[MODULUS], temp2array[MODULUS];

  M1.value = M1array;
  M2.value = M2array;
  temp1.value = temp1array;
  temp2.value = temp2array;

/* Compute M1=(C mod p)^dp mod p,  M2=(C mod q)^dq mod q */
  ICL_Rem (C, p, &temp1);

/* M1 = C^dp mod p */
  ICL_ModExpBQH (&temp1, dp, p, &M1);

/* temp1 = C mod q */
  ICL_Rem (C, q, &temp1);

/* M2 = C^dq mod q */
  ICL_ModExpBQH (&temp1, dq, q, &M2);

/* temp2 = M2 mod p */
  ICL_Rem (&M2, p, &temp2);

/* temp1 = (M1-M2) mod p */
  ICL_ModSub (&M1, &temp2, p, &temp1);

/* temp2 = (temp1 * coeff) mod p */
  ICL_ModMul (&temp1, coeff, p, &temp2);

/* temp1 = temp2 * q */
  ICL_Mul (&temp2, q, &temp1);

/* t2 = ((M1-M2)*coeff mod p)*q */
  ICL_Add (&M2, &temp1, t);

/* t  = ((M1-M2)*coeff mod p)*q + M2 */
/* discard the leading 0's */
  while (t->length>1 && t->value[t->length-1]==0)
    t->length--;
/* Note that coeff = q^{-1} mod p */
}
