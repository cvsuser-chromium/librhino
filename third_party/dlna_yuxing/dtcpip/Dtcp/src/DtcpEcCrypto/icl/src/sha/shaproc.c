/*-----------------------------------------------------------------------
 * File: SHAPROC.C
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

extern void __stdcall ICL_SHATransform(ICLWord *state,ICLWord *buffer);

int ICL_SHAProcess(ICLData *Message,ICLSHAState *SHAState)
{
 long int index,i,partlen,j;
 ICLWord k;
 
 index = (SHAState->count[0]>>3)& 0x3F;
 
/* Add length of message to the counter*/
 k = Message->length <<3;
 SHAState->count[0] +=k;
 if (SHAState->count[0] < k )  
      SHAState->count[1]++; 

 SHAState->count[1] += Message->length >>29;
 
 partlen = 64 - index;
 
 if (Message->length >= partlen)
 {
   for (i=0;i<partlen;i++)
        SHAState->buffer[i+index]=Message->value[i];

   ICL_SHATransform(SHAState->state, (ICLWord *)SHAState->buffer);

   for (i=partlen;i<Message->length-63;i+=64)
    ICL_SHATransform(SHAState->state, (ICLWord *)&Message->value[i]);
   index=0;    
 }
 else i=0;

  for (j=0;j<Message->length-i;j++)
     SHAState->buffer[index+j] = Message->value[i+j];
     
         
 return(0);
}
