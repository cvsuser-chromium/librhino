/*-----------------------------------------------------------------------
 * File: COMPARE.C
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



/* Module name: compare.c
   Compares two RSAData integers. The return value is
   -1     a < b
   0      a == b
   1      a > b

   RSAData numbers are assumed to be normalized, i.e. the most significand
   word is not zero.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"

int __stdcall ICL_Compare (RSAData *a, RSAData *b)
{
  register long	alen, blen;

  alen=a->length;
  blen=b->length;
  if (alen > blen) return 1;
  if (alen < blen) return -1;
  while (alen-->0) {
    if (a->value[alen] > b->value[alen]) return 1;
    if (a->value[alen] < b->value[alen]) return -1;
  }
  return 0;
}
