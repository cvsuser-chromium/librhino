/*-----------------------------------------------------------------------
 * File: MOVE.C
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



/*	Module name: move.c
	Move related functions.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"
#include "../../include/iclproc.h"


void __stdcall ICL_Padding0 (RSAData *a);


/* copies src to dest and fills the higher order words of dest with 0's */
void __stdcall	ICL_MoveWord (RSAData *src, RSAData *dest)
{
  register long	cnt, length=src->length;
  ICLWord	*dval, *sval;

  dval = dest->value;
  sval = src->value;
  for (cnt=0; cnt<length; ++cnt)
    dval[cnt] = sval[cnt];
/* pad the higher leading words to 0 */
  while (cnt<MODULUS)
    dval[cnt++]=0;
  dest->length = length;
}


/* copies src to dest, both are byte-length data, donot fill the upper words with 0's */
void __stdcall ICL_MoveByte (ICLData *src, ICLData *dest)
{
  register long	cnt=src->length;
  ICLByte	*dval, *sval;

  dval = dest->value;
  sval = src->value;
  dest->length = cnt;
  while (cnt-->0)
    dval[cnt] = sval[cnt];
}


/* copies src to dest and fills the higher order words of dest with 0's */
void __stdcall	ICL_MoveByte2Word (ICLData *src, RSAData *dest)
{
  register long	cnt, length;
  ICLWord	*dval;
  ICLByte	*sval;

  dval = dest->value;		/* word aligned */
  sval = src->value;		/* byte aligned */
  length = cnt = src->length/sizeof (ICLWord);	/* Word.length */

/* copy the words except the highest one */
  while (cnt-->0)
    dval[cnt] = ((ICLWord *)sval)[cnt];
  dest->length = length;

/* copy the last word. cnt is the number of bits to pad */
  cnt = (sizeof (ICLWord) - (src->length & 0x03)) * 8;	/* 32, 24, 26, 8 */
  if (cnt<32) {			/* if the last word is not fully used */
    dval[length] = ((ICLWord *)sval)[length] & ((~(ICLWord)0) >> cnt);
    dest->length++;
  }
  ICL_Padding0 (dest);
}

/* copies src to dest */
void __stdcall	ICL_MoveWord2Byte (RSAData *src, ICLData *dest)
{
  register long length;
  ICLWord	*sval;
  ICLByte	*dval;

  dval = dest->value;		/* byte aligned */
  sval = src->value;		/* word aligned */
/* number of bytes except the 0's in the higher bytes */
  dest->length = length = ICL_ByteCount (src);
  while (length-->0)
    dval[length] = ((ICLByte *)sval)[length];
}

