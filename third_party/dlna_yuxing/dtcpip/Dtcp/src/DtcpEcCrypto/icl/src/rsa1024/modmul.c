/*-----------------------------------------------------------------------
 * File: MODMUL.C
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



/* Module name: modmul.c
   Computes t = a * b mod n
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"


void __stdcall ICL_ModMul (RSAData *a, RSAData *b, RSAData *n, RSAData *t)
{
  register int	cnt, k;
  ICLWord	*bval, bi, bitmask;
  ICLWord	uarray[MODULUS+1];
  RSAData	u;

/* u = 0 */
    u.length = 1;
    u.value = uarray;
    for (cnt=MODULUS; cnt>=0; --cnt)
        uarray[cnt] = 0;
    bval = b->value;

/* start the main loop */
    for (cnt=b->length-1; cnt>=0; --cnt) {
        bi = bval[cnt];
        for (bitmask=(ICLWord)1<<(WORDSIZE-1); bitmask!=0; bitmask>>=1) {
/* shl u,1 */
            uarray[u.length] = 0;
            for (k=u.length-1; k>=0; --k) {
                uarray[k+1] |= uarray[k] >> (WORDSIZE-1);
                uarray[k] <<= 1;
            }
            if (uarray[u.length])   ++u.length;

/* if u>=n then u = u - n */
            if (ICL_Compare (&u, n)>=0)
                ICL_Subtract (&u, n);

/* u = u+bi*a */
            if (bitmask & bi) {
                ICL_Add (&u, a, &u);
                while (u.length>1 && uarray[u.length-1]==0)
                    --u.length;
            }
/* if u>=n then u = u - n */
            if (ICL_Compare (&u, n)>=0)
                ICL_Subtract (&u, n);
        }
    }

/* copy u to t and adjust length */
    bval = t->value;
    for (cnt=u.length; cnt>=0; --cnt)
        bval[cnt] = uarray[cnt];
    for (cnt=u.length-1; bval[cnt]==0; --cnt) ;
    t->length = cnt+1;
}
