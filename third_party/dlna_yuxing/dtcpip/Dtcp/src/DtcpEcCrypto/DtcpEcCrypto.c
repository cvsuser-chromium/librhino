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

#include "DtcpEcCrypto.h"
#include "eccSupport.h"
#include <string.h>
#define LITTLE_ENDIAN
#if defined(BIG_ENDIAN)

#define htons(A)  (A)
#define htonl(A)  (A)
#define ntohs(A)  (A)
#define ntohl(A)  (A)

#elif defined(LITTLE_ENDIAN)

#define htons(A)  ((((A) & 0xff00) >> 8) | ((A) & 0x00ff) << 8))
#define htonl(A)  ((((A) & 0xff000000) >> 24) | (((A) & 0x00ff0000) >> 8) | \
(((A) & 0x0000ff00) << 8) | (((A) & 0x000000ff) << 24))
#define ntohs  htons
#define ntohl  htonl

#else

#error "One of BIG_ENDIAN or LITTLE_ENDIAN must be #define'd."

#endif





//----
void
 ExtractECCoords
  (
    W8 *aECpt,   /* IN: 320 bit EC point (big endian) */
	W8 *x,       /* OUT: 160 bit x coord(big e ) */
	W8 *y       /* OUT: 160 bit y coord(big e) */
   )
{
	int i;
	for (i=0;i<20;i++)
	{
		//x[i]=ntohl(aECpt[i]);
	    //y[i]=ntohl(aECpt[i+5]);
		x[i]=aECpt[i];
		y[i]=aECpt[i+20];	
	}
}

/* set first 160 bits of pSignature to big endian representation of c */
/* set second 160 bits of pSignature to big endian representation of d */
/* Assumption: aSig is big endian*/
void
 WriteSignature
  (
    W8 *aSig,    /* OUT: 320 bit Signature (big endian) */
	W8 *x,       /* IN: 160 bit x coord(big e) */
	W8 *y       /* IN: 160 bit y coord(big e) */
   )
{
	int i;
	for (i=0;i<20;i++)
	{
		//aSig[i]=htonl(x[i]);
	    //aSig[i+5]=htonl(y[i]);
		aSig[i]=x[i];
		aSig[i+20]=y[i];
		
	}
}

/*Output is lsb N bytes of X coord*/ 

void
 LsbN
  (
   W8 *pKauth, /* OUT: Kauth (N bits) */
   W8 *tempx,  /* IN: 160 bit x coord */
   unsigned int nbytes
   )
{
	int i;
	if (nbytes>20)
	{
		nbytes=20;
	}
    for (i = 0; i < nbytes; ++i)
    {
        pKauth[i] = tempx[20 - nbytes + i];
    }
}

/****************************************************************************
 * Definitions for ECC library functions:
 */

void
 InitializeEccParams
  (
   EccParams *eccParams,/*  OUT: ECC parameters  */
   W8 * workingMemory,      /*  IN:  ptr 25 32-bit-word (800 bits)  */
   W8 * eccP,                /*  IN:  EC prime field > 3 (160 bits)  */
   W8 * eccA,                /*  IN:  EC curve coefficient A (160 bits)  */
   W8 * eccB,                /*  IN:  EC curve coefficient B (160 bits)  */
   W8 * eccBaseX,            /*  IN:  ECC base point X-coord (160 bits)  */
   W8 * eccBaseY,            /*  IN:  ECC base point Y-coord (160 bits)  */
   W8 * eccR                 /*  IN:  ECC base point order (160 bits)  */
  )  /*  note:  eccP, eccA, eccBase_, and eccR params all big endian  */
{
	
	/* assumption: Internal representation = big endian, external, big endian*/
	/* Assumption: Base point is not infinity */
	int i,j;
	for (i=0;i<20;i+=4)
	{
		for (j=0;j<4;j++)
		{
		eccParams->eccP[i+j]=eccP[i+j];
		eccParams->eccA[i+j]=eccA[i+j];
		eccParams->eccB[i+j]=eccB[i+j];
		eccParams->eccG[i+j]=eccBaseX[i+j];
		eccParams->eccG[20+i+j]=eccBaseY[i+j];
		eccParams->eccR[i+j]=eccR[i+j];	
		}
	}
}

void
 SignData
  (
   W8 * pSignature,          /*  OUT: signature (320 bits)  */
   W8 * pBuffer,             /*  IN:  buffer of data to sign  */
   W32 bufferLengthBytes,    /*  IN:  length of buffer in bytes  */
   W8 * signersKey,         /*  IN:  signer's key (160 bits)  */
   EccParams *eccParams /*  IN:  ECC parameters (big endian!)  */
  )
{
	W8 f[20];   /* 160 bit f */
	W8 u[20];   /* 160 bit u */
	W8 V[40];  /* 320 bit V */
	W8 c[20];   /* 160 bit c */
	W8 d[20];   /* 160 bit d */

	/*Ecc params*/
    W8 *p = eccParams->eccP;   /* 160 bit prime > 3 */
	W8 *a = eccParams->eccA;   /* 160 bit curve param a */
	W8 *G =eccParams->eccG;  /* 320 bit basepoint G */
	W8 *r = eccParams->eccR;   /* 160 bit basepint order r */
 
	/*temp Vars */
	W8 temp1[20];   /* 160 temp */
	W8 One[20];
	W8 Zero[20];
	memset (Zero, 0, 20);
	memset (One, 0, 20);
	One[19]=0x01;


	/* Calculate f= SHA1(pBuffer).  Use msb same as size of eccR. */
	SHA1(f, pBuffer, bufferLengthBytes);
	f_msb_bits_in_r(f,r);
	/* reduce digest before its input to other routines */
	/* Question.. can input and output be the same?? */
	Mod(f,f,r);
	for(;;) {
		/* Generate random u value in range [1, r) */
		RNGf (u, r);
	    /* Calcuate curve point V= u*G */
		ecPtMultScalar(V, u, G, a, p); /* - continue if point at infinity..how?- */
	    /* Calculate c = Vx mod r.  If c = 0 recompute */
		Mod(c, V, r);
		if (!Compare(c, Zero))	continue;
		/* Calculate d= ((ModInv(u, r)*(f +c*signersKey)) mod r. If 0 restart*/
		
		ModMul(temp1, c, signersKey, r);
		ModAdd(temp1, temp1, f, r);		
		ModInv(d, u, r);
		ModMul(d, d,temp1, r);
		if (!Compare(d, Zero))	continue;
		/* set first 160 bits of pSignature to big endian representation of c */
		/* set second 160 bits of pSignature to big endian representation of d */
		WriteSignature(pSignature,c,d);
		break;
	}

}

int
 VerifyData
  (
   W8 * pSignature,          /*  IN:  signature (320 bits)  */
   W8 * pBuffer,             /*  IN:  buffer of data to verify  */
   W32 bufferLengthBytes,    /*  IN:  length of buffer in bytes  */
   W8 *verifyKey,           /*  IN:  verify key (320 bits)  */
   EccParams *eccParams /*  IN:  ECC parameters  */
  )  /*  RETURNS:  1 if "valid"; 0 if "invalid" signature  */
{
	W8 c[20];   /* 160 bit c */
	W8 d[20];   /* 160 bit d */
	W8 f[20];   /* 160 bit f */	
	
	W8 h[20];   /* 160 bit h  */
	W8 h1[20];  /* 160 bit h1 */
	W8 h2[20];  /* 160 bit h2 */
	
    W8 P[40];  /* 320 bit P */

	W8 cc[20];  /* 160 bit cc */

	/*Ecc params*/
    W8 *p = eccParams->eccP;   /* 160 bit prime > 3 */
	W8 *a = eccParams->eccA;   /* 160 bit curve param a */
	W8 *G =eccParams->eccG;  /* 320 bit basepoint G */
	W8 *r = eccParams->eccR;   /* 160 bit basepint order r */

	/*temp Vars */
	W8 tempP[40];   /* 320 temp */
	W8 Zero[20];
	memset (Zero, 0, 20);

	/* set c to first 160 bits of pSignature interpreted as big endian */
	/* set d to second 160 bits of pSignature interpreted as big endian */
	ExtractECCoords(pSignature,c,d);
	/* if c is not [1, r-1] or d is not [1, r-1] then return invalid */
	if (!Compare(c, Zero) 
		|| !Compare(d, Zero)
		|| !(Compare (c, r) < 0)
		|| !(Compare (d, r) < 0)
		) {
		return 0;
	}		
	/* Calculate f= SHA1(pBuffer).  Use msb same as size of r. */
	SHA1(f, pBuffer, bufferLengthBytes);
	f_msb_bits_in_r(f,r);

	/* reduce digest before its input to other routines */
	/* Question.. can input and output be the same?? */
	Mod(f,f,r);
	/* Calculate h = ModInv(d, r), h1= f*h mod r, h2 =c*h mod r */
	ModInv(h, d, r); //<- this is trashing D (andc!)
	ModMul(h1, f, h, r);
	ModMul(h2, c, h, r);
	/* Calculate  EC pt P (xP, yP) = h1*G + h2*verifyKey. */ 

	ecPtMultScalar(P, h1, G, a, p);
	ecPtMultScalar(tempP, h2, verifyKey, a, p);
	ecPtAdd(P, P, tempP, a, p);
    /* if P isInfinity, then return invalid */
	if (isInfinity(P)) {
		return 0;
	}
	/* calculate cc = xP mod r  If cc=c then return valid else invalid */
	Mod(cc,P,r);
	return (0==Compare (c,cc))? 1:0;

}

//------
void
 GetFirstPhaseValue
  (
   W8 * pXv,                 /*  OUT: Diffie first phase value (320 bits)  */
   W8 * pXk,                 /*  OUT: secret information (160 bits)  */
   EccParams *eccParams /*  IN:  ECC parameters  */
  )
{
	/*Ecc params*/
    W8 *p = eccParams->eccP;   /* 160 bit prime > 3 */
	W8 *a = eccParams->eccA;   /* 160 bit curve param a */
	W8 *G =eccParams->eccG;  /* 320 bit basepoint G */
	W8 *r = eccParams->eccR;   /* 160 bit basepint order r */

	W8 One[20];
	memset (One, 0, 20);
	One[19]=0x01;

	/* Generate random integer Xk in [1, r) */
	RNGf (pXk, r);
	/* Calculate ec pt Xv = Xk*G */
	ecPtMultScalar(pXv, pXk, G, a, p);
}

void
 GetSharedSecret
  (
   W8 * pKauth,              /*  OUT: Kauth (96 bits, big endian)  */
   W8 * pYv,                 /*  IN:  Diffie first phase value (320 bits)  */
   W8 * pXk,                 /*  IN:  secret information (160 bits)  */
   EccParams *eccParams /*  IN:  ECC parameters  */
  )
{
	W8 *a = eccParams->eccA;
	W8 *p = eccParams->eccP;
	/*temp Vars */
	W8 temp[40];   /* 320 temp */
	W8 tempx[20];   /* 160 temp */
	W8 tempy[20];   /* 160 temp */
	/* Calculate point Xk*Yv.  X coord is shared secret */
	ecPtMultScalar(temp, pXk, pYv, a, p);
	ExtractECCoords(temp,tempx,tempy);
	/*Output is lsb 96 bits(12 bytes) of X coord*/ 
	LsbN(pKauth,tempx,12);
}

