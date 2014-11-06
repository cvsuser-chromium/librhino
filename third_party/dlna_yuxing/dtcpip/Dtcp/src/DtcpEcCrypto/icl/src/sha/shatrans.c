/*-----------------------------------------------------------------------
 * File: SHATRANS.C
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
#include "../../include/misc.h"

#include <stdio.h>

// The basic functions

#define f1(x,y,z)	(z^ ( x & (y^z) ))
#define f2(x,y,z)	(x ^ y ^ z)
#define f3(x,y,z)	( ( x & y ) | ( z & ( x | y ) ))

#define P(A,B,C,D,E) // printf("A %8.8x B %8.8x C %8.8x D %8.8x E %8.8x\n", A, B, C, D, E)
  
#define F1(A,B,C,D,E,data) \
	E += f1(B,C,D) + rotl(A,5) + data + 0x5a827999UL; \
	B = rotl(B,30); P(A,B,C,D,E)


#define F2(A,B,C,D,E,data)  \
	E += f2(B,C,D) + rotl(A,5) + data + 0x6ed9eba1UL; \
	B = rotl(B,30); P(A,B,C,D,E)


#define F3(A,B,C,D,E,data)  \
	E += f3(B,C,D) + rotl(A,5) + data + 0x8f1bbcdcUL; \
	B = rotl(B,30); P(A,B,C,D,E)


#define F4(A,B,C,D,E,data) \
	E += f2(B,C,D) + rotl(A,5) + data + 0xca62c1d6UL; \
	B = rotl(B,30); P(A,B,C,D,E)


#define  FX(W,i) (( W[i&15] = rotl(W[i&15]^W[(i-14)&15]^W[(i-8)&15]^W[(i-3)&15],1)),W[i&15]) // printf("data %8.8x ", W[i&15]),W[i&15])
#define PX(x) x // (printf("data %8.8x ", x), x)

void __stdcall ICL_SHATransform(ICLWord state[5],ICLWord buffer[16])
{
    int i;
    register ICLWord A,B,C,D,E;
    ICLWord block[16];
    
    for (i = 0; i < 16; ++i)
    {
      block[i] = bswap(buffer[i]);
    }
   
    A = state[0];
    B = state[1];
    C = state[2];
    D = state[3];
    E = state[4];

// Begin Transform

	F1( A,B,C,D,E,PX(block[0]));
	F1( E,A,B,C,D,PX(block[1]));
  	F1( D,E,A,B,C,PX(block[2]));
	F1( C,D,E,A,B,PX(block[3]));
	F1( B,C,D,E,A,PX(block[4]));
	
	F1( A,B,C,D,E,PX(block[5]));
	F1( E,A,B,C,D,PX(block[6]));
	F1( D,E,A,B,C,PX(block[7]));
	F1( C,D,E,A,B,PX(block[8]));
	F1( B,C,D,E,A,PX(block[9]));
	
	F1( A,B,C,D,E,PX(block[10]));
	F1( E,A,B,C,D,PX(block[11]));
	F1( D,E,A,B,C,PX(block[12]));
	F1( C,D,E,A,B,PX(block[13]));
	F1( B,C,D,E,A,PX(block[14]));

           
	F1( A,B,C,D,E,PX(block[15]));
   	F1( E,A,B,C,D,FX(block,16));
	F1( D,E,A,B,C,FX(block,17));
	F1( C,D,E,A,B,FX(block,18));
	F1( B,C,D,E,A,FX(block,19));
    
	F2( A,B,C,D,E,FX(block,20));
	F2( E,A,B,C,D,FX(block,21));
	F2( D,E,A,B,C,FX(block,22));
	F2( C,D,E,A,B,FX(block,23));
	F2( B,C,D,E,A,FX(block,24));

	F2( A,B,C,D,E,FX(block,25));
	F2( E,A,B,C,D,FX(block,26));
	F2( D,E,A,B,C,FX(block,27));
	F2( C,D,E,A,B,FX(block,28));
	F2( B,C,D,E,A,FX(block,29));
	
	F2( A,B,C,D,E,FX(block,30));
	F2( E,A,B,C,D,FX(block,31));
	F2( D,E,A,B,C,FX(block,32));
	F2( C,D,E,A,B,FX(block,33));
	F2( B,C,D,E,A,FX(block,34));
	          
	F2( A,B,C,D,E,FX(block,35));
	F2( E,A,B,C,D,FX(block,36));
	F2( D,E,A,B,C,FX(block,37));
	F2( C,D,E,A,B,FX(block,38));
	F2( B,C,D,E,A,FX(block,39));

	
	F3( A,B,C,D,E,FX(block,40));
	F3( E,A,B,C,D,FX(block,41));
	F3( D,E,A,B,C,FX(block,42));
	F3( C,D,E,A,B,FX(block,43));
	F3( B,C,D,E,A,FX(block,44));

	F3( A,B,C,D,E,FX(block,45));
	F3( E,A,B,C,D,FX(block,46));
	F3( D,E,A,B,C,FX(block,47));
	F3( C,D,E,A,B,FX(block,48));
	F3( B,C,D,E,A,FX(block,49));
	
	F3( A,B,C,D,E,FX(block,50));
	F3( E,A,B,C,D,FX(block,51));
	F3( D,E,A,B,C,FX(block,52));
	F3( C,D,E,A,B,FX(block,53));
	F3( B,C,D,E,A,FX(block,54));
	          
	F3( A,B,C,D,E,FX(block,55));
	F3( E,A,B,C,D,FX(block,56));
	F3( D,E,A,B,C,FX(block,57));
	F3( C,D,E,A,B,FX(block,58));
	F3( B,C,D,E,A,FX(block,59));


	F4( A,B,C,D,E,FX(block,60));
	F4( E,A,B,C,D,FX(block,61));
	F4( D,E,A,B,C,FX(block,62));
	F4( C,D,E,A,B,FX(block,63));
	F4( B,C,D,E,A,FX(block,64));

	F4( A,B,C,D,E,FX(block,65));
	F4( E,A,B,C,D,FX(block,66));
	F4( D,E,A,B,C,FX(block,67));
	F4( C,D,E,A,B,FX(block,68));
	F4( B,C,D,E,A,FX(block,69));
	
	F4( A,B,C,D,E,FX(block,70));
	F4( E,A,B,C,D,FX(block,71));
	F4( D,E,A,B,C,FX(block,72));
	F4( C,D,E,A,B,FX(block,73));
	F4( B,C,D,E,A,FX(block,74));

	F4( A,B,C,D,E,FX(block,75));
	F4( E,A,B,C,D,FX(block,76));
	F4( D,E,A,B,C,FX(block,77));
	F4( C,D,E,A,B,FX(block,78));
	F4( B,C,D,E,A,FX(block,79));

// End of Transform

// Clear Memory

// for (i=0;i<16;i++) block[i]=0UL;

    state[0] += A;
    state[1] += B;
    state[2] += C;
    state[3] += D;
    state[4] += E;
}

	
