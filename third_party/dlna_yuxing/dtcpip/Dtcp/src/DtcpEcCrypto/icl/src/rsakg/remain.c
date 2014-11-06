/*-----------------------------------------------------------------------
 * File: REMAIN.C
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



/* Module name: remain.c
   Checks the remainder of division a by b.
   If remainder is 0 then returns 0, otherwise returns -1
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"
#include "../../include/rsakg.h"


/*  If remainder is 0 then returns 0, otherwise returns -1
    The base relies on:
      r = a mod b, where   W=2^w   (w=32)
      a = \sum_{i=0}^{n-1} (A_iW^i)      and     b = B_0
    and
      r = \sum_{i=0}^{n-1} (A_iW^i)  mod  b = B_0
      r = \sum_{i=0}^{n-1} (A_i mod B_0)(W^i mod B_0)
      r = \sum_{i=0}^{n-1} (A_i mod B_0)(\prod_{j=1}^{i}(W mod B_0))

    Note: b is only 1/2 word long (usually a prime)
    Returns:
    0   divisible (composite, remainder=0)
    -1  not divisible (remainder!=0)
*/
int __stdcall RSA_Remainder (RSAData *a, ICLWord b)
{
    ICLWord     rem, w, *aval;
    int         cnt;

    aval = a->value;
/* 2^32 mod b */
#if defined(_MSC_VER)
#pragma warning(disable:4146)    /* unary minus below */
#endif
    w = ((ICLWord)-b) % b;
#if defined(_MSC_VER)
#pragma warning(default:4146)    /* unary minus below */
#endif

    rem = 0;

/* start the \sum operation starting from the MSW */
    for (cnt=a->length-1; cnt>=0; --cnt)
        rem = (rem*w + aval[cnt]) % b;

/* the remainder is stored in "rem" */
    return (rem==0) ? 0 : -1;
}
