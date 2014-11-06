/*-----------------------------------------------------------------------
 * File: SHAEND.C
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




#include "../../include/icl.h"

// bitwise rotation to the left
#define rotl(x,n)   (((x)>>(32 - (n))) | ((x) << (n)))

// bitwise rotation to the right
#define rotr(x,n)   (((x)<<(32 - (n))) | ((x) >> (n)))

// translates little endian <----> big endian
#define bswap(y)   ((rotr(y, 8) & 0xff00ff00) |  \
                   (rotl(y, 8) & 0x00ff00ff))
   


int ICL_SHAEnd(ICLSHAState *SHAState,ICLSHADigest digest)
{
 ICLWord APPEND_STR[16]={0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 ICLWord Length[2];
 ICLData dat;
 long int index,i;

/* Save length  */
/* In reverse order */ 
 Length[1]= bswap(SHAState->count[0]);
 Length[0]= bswap(SHAState->count[1]);

 dat.value = (ICLByte *) APPEND_STR;
 index = (SHAState->count[0] >> 3) & 0x3F;
 dat.length = (index<56) ? (56-index) : (120-index);
 
 ICL_SHAProcess(&dat,SHAState);
 dat.value  = (ICLByte * ) Length;
 dat.length = 8;
 ICL_SHAProcess(&dat,SHAState);

/* Print State vector to digest */

 ((ICLWord *) digest)[0] = bswap(SHAState->state[0]);
 ((ICLWord *) digest)[1] = bswap(SHAState->state[1]);
 ((ICLWord *) digest)[2] = bswap(SHAState->state[2]);
 ((ICLWord *) digest)[3] = bswap(SHAState->state[3]);
 ((ICLWord *) digest)[4] = bswap(SHAState->state[4]);


/* Clear state information */
#if 0
 SHAState->state[0] =0;
 SHAState->state[1] =0;
 SHAState->state[2] =0;
 SHAState->state[3] =0;
 SHAState->state[4] =0;

 SHAState->count[0] =0;
 SHAState->count[1] =0;

 for (i=0;i<16;i++)
   ((ICLWord *) SHAState->buffer)[i] = 0UL;
#endif 
 return(0);
}
