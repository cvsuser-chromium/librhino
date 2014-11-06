/*-----------------------------------------------------------------------
 * File: BYTES.C
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



/*	Module name: bytes.c
	Byte/Word related functions.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"
#include "../../include/iclproc.h"

/* pads the given structures unused high words with 0's to upto MODULUS words */
void __stdcall ICL_Padding0 (RSAData *a)
{
	long	cnt;
	ICLWord	*aval;

	cnt = a->length;
	aval = &a->value[cnt];
	while (cnt++<MODULUS)
	  *(aval++) = 0;
}


/* pads the excess bytes of RSAData whose size is "ByteLength" bytes.
   RSAData.length is not modified.
*/
void __stdcall	ICL_PadBytes0 (RSAData *a, long ByteLength)
{
	ICLWord	*aval;

	aval = &a->value[ICL_WordCount (ByteLength)];
/* Word.length */
	ByteLength = (sizeof (ICLWord) - (ByteLength & 0x03)) * 8;	/* 32, 24, 26, 8 */
/* if the last word is not fully used, strip off the excess bytes */
	if (ByteLength<32)
	  *aval = *aval & ( (~(ICLWord)0) >> ByteLength );
}


/* the exact number of bytes to store this word-aligned data */
int __stdcall ICL_ByteCount (RSAData *a)
{
	register long	length;
	ICLWord			sval;

/* the base length. Note that, a->length is word.length */
	length = (a->length-1) * sizeof (ICLWord);
	sval = a->value[a->length-1];
/* if there are excess bytes, then add them to the base length */
	if (sval & 0xFF000000)		length += 4;
	else if (sval & 0x00FF0000)	length += 3;
	else if (sval & 0x0000FF00)	length += 2;
	else if (sval & 0x000000FF)	length += 1;
	return length;
}
