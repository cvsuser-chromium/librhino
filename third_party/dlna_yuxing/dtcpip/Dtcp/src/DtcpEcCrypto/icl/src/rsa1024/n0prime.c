/*-----------------------------------------------------------------------
 * File: N0PRIME.C
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



/* Module name: n0prime.c
   Computes n0' for Montgomery product.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"

ICLWord __stdcall N0Prime (ICLWord n0)
{
  register int		i;
  register ICLWord	A, B, C;
  long			n0pr;

  n0pr = 1;
  A = 0x00000001 << 1;
  B = 0x00000003;
  C = B & n0;
  for (i=2; i<1+8*sizeof(ICLWord); i++) {
    if (A < C) n0pr+=A;
    A = A << 1;
    B = (B << 1) | 0x00000001;
    C = n0*n0pr & B;
  }
  return (~n0pr + 1);
}
