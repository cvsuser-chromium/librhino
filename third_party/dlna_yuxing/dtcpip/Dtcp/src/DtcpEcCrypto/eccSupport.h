//#############################################################################
//## Copyright (c) 2004 Intel Corporation All Rights Reserved. 
//## 
//## The source code contained or described herein and all documents related to
//## the source code ("Material") are owned by Intel Corporation or its 
//## suppliers or licensors. Title to the Material remains with Intel 
//## Corporation or its suppliers and licensors. The Material contains trade 
//## secrets and proprietary and confidential information of Intel or its
//## suppliers and licensors. The Material is protected by worldwide copyright
//## and trade secret laws and treaty provisions. No part of the Material may 
//## be used, copied, reproduced, modified, published, uploaded, posted, 
//## transmitted, distributed, or disclosed in any way without Intel's prior 
//## express written permission.
//## 
//## No license under any patent, copyright, trade secret or other 
//## intellectual property right is granted to or conferred upon you by 
//## disclosure or delivery of the Materials, either expressly, by 
//## implication, inducement, estoppel or otherwise. Any license under such 
//## intellectual property rights must be express and approved by Intel in 
//## writing.
//#############################################################################

#ifndef WWbasics
#define WWbasics
typedef unsigned char W8;
typedef unsigned short W16;
typedef unsigned int W32;
#endif



/* Calculate digest= SHA1(pBuffer) */
void
SHA1(
	 W8 *digest,           /* OUT: 160 bit digest */ 
	 W8 *pBuffer,           /* IN: Buffer pointer */
	 W32 bufferLengthBytes  /* IN: Buffer length */
	 );

/* Generate random u value in range (0, max) */
void
RNGf(
	  W8 *r,            /* OUT: 160 bit result */ 
	  W8 *max           /* IN: 160 bit exclusive max */
	  );

void
ecPtAdd(
		W8* R,          /* OUT: 320 bit ec point */
		W8* P,          /* IN: 320 bit ec point */
		W8* Q,          /* IN: 320 bit ec point */
		W8* a,          /* IN: 160 bit ec parameter */
		W8* p           /* IN: 160 bit ec field prime */ 
		);


void
ecPtMultScalar(
			   W8* R,          /* OUT: 320 bit ec point */
			   W8* k,          /* IN: 160 bit constant */
		       W8* P,          /* IN: 320 bit ec point */	
		       W8* a,          /* IN: 160 bit ec parameter */
		       W8* p           /* IN: 160 bit ec field prime */ 
			   ); 

void
Mod(
	W8* r,            /* OUT: 160 bit result */
	W8* x,            /* IN: 160 bit x */
	W8* m             /* IN: 160 bit modulus */
	);

void
ModAdd(
	   W8* r,            /* OUT: 160 bit result */ 
	   W8* x,            /* IN: 160 bit x */ 
	   W8* y,            /* IN: 160 bit y */
	   W8* m             /* IN: 160 bit moduus */
	   );	
void
ModSub(
	   W8* r,            /* OUT: 160 bit result */ 
	   W8* x,            /* IN: 160 bit x */ 
	   W8* y,            /* IN: 160 bit y */
	   W8* m             /* IN: 160 bit moduus */
	   );

void
ModMul(
	   W8* r,            /* OUT: 160 bit result */ 
	   W8* x,            /* IN: 160 bit x */ 
	   W8* y,            /* IN: 160 bit y */
	   W8* m             /* IN: 160 bit modulus */
	   );
void
ModInv(
	   W8* r,            /* OUT: 160 bit result */ 
	   W8* x,            /* IN: 160 bit x */ 
	   W8* m             /* IN: 160 bit modulus */
	   );

int
Compare(
		W8* x,           /* IN: 160 bit x */ 
		W8* y            /* IN: 160 bit y */
		);	
int
isInfinity(
           W8 *aECpt     /* IN: 320 bit ECpt */
          );	

void f_msb_bits_in_r (W8 *f, const W8 *r);
