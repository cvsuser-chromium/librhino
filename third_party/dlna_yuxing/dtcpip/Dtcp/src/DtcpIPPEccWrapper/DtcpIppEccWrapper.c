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


#define BITS2WORD_SIZE(x)  (((x)+31)>>5)
#include "ippcp.h"
#include <string.h>
#include "DtcpEcCrypto.h"

void *bswap160(W8 *a,W8 *dest)
{
	int i;
	if (a==dest)
	{
		W8 temp;
		for (i=0;i<10;i++) {
			temp= dest[19-i];
			dest[19-i]=a[i];
			a[i]= temp;
		}
	}
	else
	{
		for (i=0;i<20;i++) {
			dest[19-i]=a[i];
		}
	}
	return dest;
}

/*Compare 2 160 bit (unsigned) numbers, return 1 if a>b, -1 if a<b 0 if a==b*/
int 
Compare
  (
   W8 *a,         /* IN: 160 bit a */
   W8 *b          /* IN: 160 bit b */
   )
{
	int i;
	for (i=0;i<20;i++)
	{
		if (a[i] > b[i]) {
			return 1;
		}
		if (a[i] < b[i]) {
			return -1;
		}
	}
	return 0;
}

void f_msb_bits_in_r (W8 *f, const W8 *r)
{
	//calc bits in r
	//count unused bits
	int i,bits,offset_bytes;
	W8 mask =0x80;
	for(i=0,bits=160;i<20;i++)
	{
		if (r[i]) {
			//ok which bit in the byte
			while (0==(r[i] & mask))
			{
					mask>>=1;
					bits--;
			}
			break;
		}
		else {
			bits-=8;
		}
	}

	offset_bytes = (160-bits)/8;
	//rightshift 
	bits =8-(bits%8);
	i=20;
	while(i--)
	{
		f[i]= ((i>=offset_bytes)? (f[i-offset_bytes] >> bits) : 0)
			  | ((i-1>=offset_bytes)? (f[i-1-offset_bytes] << (8-bits)) : 0);
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


/* Generate random u value in range (0, max) */
void
RNGf(
	  W8 *r,            /* OUT: 160 bit result */ 
	  W8 *max           /* IN: 160 bit exclusive max */
	  )
{
	int length=0;
	W8 zero[20];

	/*  the number of bits used in max */
	int nbits=0;
	W8 n;
	int i;
    for (i=0;i<20;i++)
	{
		n = max[19-i];
		if (n){
			nbits = i*8;
			while(n != 0)
			{
				nbits++;
				n >>= 1;
			}
		}
	}
	length = (nbits + 7) / 8;
	memset(zero,0,20);
	do{
	memset(r, 0, 20);
	Rng_GetRandomNumber(r, length);
	r[length-1] &= 0xff >> (8 - nbits % 8);
	bswap160(r,r);
	}while( Compare(r,max)==1 || Compare(r,zero)==0) ;
	
}



// available input data formats
typedef enum {
   LittleEndian,
   BigEndian,
   HexString   
} DataFormat;

// convert bytes into dwords according to input data format
//
Ipp32u Bytes_Dword(Ipp8u b0, Ipp8u b1, Ipp8u b2, Ipp8u b3)
{
   return (b3<<24)|(b2<<16)|(b1<<8)|b0;
}

void Cvt8uLE_32u(const Ipp8u* pSrc, int srcLen, Ipp32u* pDst)
{
   while( srcLen>3 ) {
      *pDst++ = Bytes_Dword(pSrc[0], pSrc[1], pSrc[2], pSrc[3]);
      srcLen -=4;
      pSrc += 4;
   }
   switch(srcLen) {
      case 3:  *pDst = Bytes_Dword(pSrc[0], pSrc[1], pSrc[2], 0); break;
      case 2:  *pDst = Bytes_Dword(pSrc[0], pSrc[1], 0, 0); break;
      case 1:  *pDst = Bytes_Dword(pSrc[0], 0, 0, 0); break;
      default: break;
   }
}

void Cvt8uBE_32u(const Ipp8u* pSrc, int srcLen, Ipp32u* pDst)
{
   while( srcLen>3 ) {
      *pDst++ = Bytes_Dword(pSrc[3], pSrc[2], pSrc[1], pSrc[0]);
      srcLen -=4;
      pSrc += 4;
   }
   switch(srcLen) {
      case 3:  *pDst = Bytes_Dword(0, pSrc[1], pSrc[2], pSrc[0]); break;
      case 2:  *pDst = Bytes_Dword(0, 0, pSrc[2], pSrc[0]); break;
      case 1:  *pDst = Bytes_Dword(0, 0, 0, pSrc[0]); break;
      default: break;
   }
}

void Cvt8uHS_32u(const Ipp8u* pSrc, int srcLen, Ipp32u* pDst)
{
   pDst += (srcLen+3)/4 -1;

   switch( srcLen%4 ) {
      case 3:  *pDst-- = Bytes_Dword(0, pSrc[2], pSrc[1], pSrc[0]); break;
      case 2:  *pDst-- = Bytes_Dword(0, 0, pSrc[1], pSrc[0]); break;
      case 1:  *pDst-- = Bytes_Dword(0, 0, 0, pSrc[0]); break;
      default: break;
   }
   srcLen &= ~3;
   while( srcLen>3 ) {
      *pDst-- = Bytes_Dword(pSrc[3], pSrc[2], pSrc[1], pSrc[0]);
      srcLen -=4;
      pSrc += 4;
   }
}
void Cvt32uto8uHS(const Ipp8u* pSrc, int srcLen, Ipp8u* pDst)
{
   //Assume SRC Len is multiple of 4
   pDst += srcLen-4;

   while( srcLen>3 ) {
      pDst[0] = pSrc[3];
      pDst[1] = pSrc[2];
      pDst[2] = pSrc[1];
      pDst[3] = pSrc[0];
      pDst-=4;     
      srcLen -=4;
      pSrc += 4;
   }
}


// create (LittleEndian) BigNum
//
IppsBigNum* CreateBigNum(DataFormat fmt, const Ipp8u* pData, int length)
{
   int bnLength = (length+3)/4;
   int bnSize;
   IppsBigNum* pBN=NULL;
   ippsBigNumBufferSize(bnLength, &bnSize);
   pBN = (IppsBigNum*)( malloc(bnSize) );

   if( pBN ) {
      Ipp32u* pBuffer = (Ipp32u*)malloc(bnSize);
      if( pBuffer ) {
         switch( fmt ) {
            case BigEndian: Cvt8uBE_32u(pData, length, pBuffer); break;
            case HexString: Cvt8uHS_32u(pData, length, pBuffer); break;
            default:        Cvt8uLE_32u(pData, length, pBuffer); break;
         }
         ippsBigNumInit(bnLength, pBN);
         ippsSet_BN(IppsBigNumPOS, bnLength, pBuffer, pBN);
         free(pBuffer);
      }
      else {
         free(pBN);
         pBN = 0;
      }
   }
   return pBN;
}

IppsBigNum* DefineBigNum(int dwLength)
{
   int bnSize;
   IppsBigNum* pBN = NULL;
   ippsBigNumBufferSize(dwLength, &bnSize);
   pBN = (IppsBigNum*)( malloc(bnSize) );

   if( pBN )
      ippsBigNumInit(dwLength, pBN);
   return pBN;
}

void DestroyBigNum(const IppsBigNum* pBN)
{
   free((void *)pBN);
}

// print Big Number
//
void PrintBN(const IppsBigNum* pBN)
{			    
	int bnLen;
	int x;
	unsigned char *pData;
    IppsBigNumSGN sgn;
  ippsGetSize_BN(pBN, &bnLen);
  printf("\nBN Length = %d\n", bnLen);
   pData = malloc(bnLen*4);
   ippsGet_BN(&sgn, &bnLen, pData, pBN);
   for(x=0;x<bnLen*4;x++) printf(" %x ",pData[x]);

   free(pData);
}

static 
char HexDigitList[] = "0123456789ABCDEF";


static
void MessageRepresentation(int nBits, Ipp8u* pOctString, int length)
{
   int n;

   int stringBitSize = length*8;
   int nShiftBytes =0;
   int nShift=0;
   int nShiftBits=0;
   Ipp8u l; 
   if( nBits>=stringBitSize )
      return;

   // compute actual OctString size (bits)
   for(n=0; n<length; n++) {
      if( pOctString[n] ) {
         Ipp8u x = pOctString[n];
         Ipp8u testBit = 0x80;
         while( !(x&testBit) ) {
            stringBitSize--;
            testBit >>= 1;
         }
         break;
      }
      stringBitSize -= 8;
   }

   // number of shifts rigth
   nShift = stringBitSize-nBits;
   if( nShift<=0)
      return;

   // shift right (bytes)
   nShiftBytes = (nShift/8);
   if( nShiftBytes ) {
      for(n=length-nShiftBytes; n>0; n--)
         pOctString[n-1] = pOctString[n-1-nShiftBytes];
      for(n=0; n<nShiftBytes; n++)
         pOctString[n] = 0;
   }

   // shift right (bits)
   nShiftBits = nShift%8;
   if( !nShiftBits )
      return;
   l = pOctString[length-1];
   for(n=length-1; n>0; n--) {
      Ipp8u h = pOctString[n-1];
      pOctString[n] = (l>>nShiftBits) | (h<<(8-nShiftBits));
      l = h;
   }
   pOctString[0] = l>>nShiftBits;
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
   IppECResult result;

   const int feBitSize = 160;    // field element size (bits) is 160
   int ecPointSize;
   int eccStateSize;
   int orderBitSize;
   IppsECCP* pECC=NULL;
   IppsBigNum* pPrime=NULL;
   IppsBigNum* pA     = NULL;
   IppsBigNum* pB     = NULL;

   IppsBigNum* pGX    = NULL;
   IppsBigNum* pGY    = NULL;
   IppsBigNum* pOrder = NULL;
   IppsBigNum* pDtlaPubX   = NULL;
   IppsBigNum* pDtlaPubY = NULL;
   IppsECCPPoint* pDtlaPublic=NULL;
   Ipp8u Digest[20];
   IppsBigNum* pMsgRepresentation =NULL;

   DataFormat fmt = HexString;   // actual data format

   // size (bytes) of EC point
   //
   ippsECCPPointBufferSize(feBitSize, &ecPointSize);

   // allocate & init ECC state
   //
   ippsECCPBufferSize(feBitSize, &eccStateSize);
   pECC = (IppsECCP*)( malloc(eccStateSize));
   ippsECCPInit(feBitSize, pECC);

   // establish EC cryptosystem based on yours parameters
   //
   pPrime = CreateBigNum(fmt, eccParams->eccP,  20);

   pA     = CreateBigNum(fmt, eccParams->eccA, 20);
   pB     = CreateBigNum(fmt, eccParams->eccB, 20);

   pGX    = CreateBigNum(fmt, eccParams->eccG,    20);
   pGY    = CreateBigNum(fmt, eccParams->eccG+20,    20);
   pOrder = CreateBigNum(fmt, eccParams->eccR, 20);

     // set EC parameters
    ippsECCPSet(pPrime,
                  pA,pB,
                  pGX,pGY,pOrder,
                  1,    
                  pECC);

   // DTLA public key
   //
   pDtlaPubX   = CreateBigNum(
                                    fmt,
                                    verifyKey,
                                    20);
   pDtlaPubY   = CreateBigNum(
                                    fmt,
                                    verifyKey+20,
                                    20);
   pDtlaPublic = (IppsECCPPoint*)( malloc(ecPointSize) );
   ippsECCPPointInit(feBitSize, pDtlaPublic);
   ippsECCPSetPoint(pDtlaPubX,pDtlaPubY, pDtlaPublic, pECC);

   DestroyBigNum(pDtlaPubX);
   DestroyBigNum(pDtlaPubY);


   // compute digest by SHA-1
   ippsSHA1MessageDigest(pBuffer, bufferLengthBytes, Digest);
   // prepare message representation
   //    message representaion formally different
   //    than message digest
   //    f = most_signifinant_bits( bitsise(order), digest )
	f_msb_bits_in_r(Digest,eccParams->eccR);

	//	  
   //ippsECCPGetOrderBitSize(&orderBitSize, pECC);
   //MessageRepresentation(orderBitSize, Digest, 20);
   pMsgRepresentation = CreateBigNum(fmt, Digest, 20);

   // additionally Montgomery's technique, empoyed in ECC (prime case)
   // strictly request f < (base point) order
	ippsMod_BN(pMsgRepresentation, pOrder, pMsgRepresentation);

   // verify
   //
   {
      IppsBigNum* pSignC  = CreateBigNum(fmt, pSignature, 20);
      IppsBigNum* pSignD  = CreateBigNum(fmt, pSignature+20, 20);
      ippsECCPSetKeyPair(0, pDtlaPublic, ippTrue, pECC);

      ippsECCPVerifyDSA(pMsgRepresentation, pSignC,pSignD, &result, pECC);
      DestroyBigNum(pSignC);
      DestroyBigNum(pSignD);
   }

	DestroyBigNum(pPrime);
	DestroyBigNum(pA);
	DestroyBigNum(pB);
	DestroyBigNum(pGX);
	DestroyBigNum(pGY);
	DestroyBigNum(pOrder);

   DestroyBigNum(pMsgRepresentation);
   free(pDtlaPublic);
   free(pECC);

   return (IppECSignIsValid==result) ? 1:0;
   
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
   IppECResult result;

   const int feBitSize = 160;    // field element size (bits) is 160
   int ecPointSize;
   int eccStateSize;
   int orderBitSize;
   IppsECCP* pECC=NULL;
   IppsBigNum* pPrime=NULL;
   IppsBigNum* pA     = NULL;
   IppsBigNum* pB     = NULL;

   IppsBigNum* pGX    = NULL;
   IppsBigNum* pGY    = NULL;
   IppsBigNum* pOrder = NULL;
   IppsBigNum* pPrivKey   = NULL;
   Ipp8u Digest[20];
   IppsBigNum* pMsgRepresentation =NULL;
   int bnSize;
   int x;

   DataFormat fmt = HexString;   // actual data format

   // size (bytes) of EC point
   //
   ippsECCPPointBufferSize(feBitSize, &ecPointSize);

   // allocate & init ECC state
   //
   ippsECCPBufferSize(feBitSize, &eccStateSize);
   pECC = (IppsECCP*)( malloc(eccStateSize));
   ippsECCPInit(feBitSize, pECC);

   // establish EC cryptosystem based on yours parameters
   //
   pPrime = CreateBigNum(fmt, eccParams->eccP,  20);

   pA     = CreateBigNum(fmt, eccParams->eccA, 20);
   pB     = CreateBigNum(fmt, eccParams->eccB, 20);

   pGX    = CreateBigNum(fmt, eccParams->eccG,    20);
   pGY    = CreateBigNum(fmt, eccParams->eccG+20,    20);
   pOrder = CreateBigNum(fmt, eccParams->eccR, 20);

     // set EC parameters
    ippsECCPSet(pPrime,
                  pA,pB,
                  pGX,pGY,pOrder,
                  1,    
                  pECC);
   ippsECCPGetOrderBitSize(&orderBitSize, pECC);
   // size of BigNum (private keys & signature components) context
   ippsBigNumBufferSize(BITS2WORD_SIZE(feBitSize), &bnSize);

   // compute digest by SHA-1
   ippsSHA1MessageDigest(pBuffer, bufferLengthBytes, Digest);
   // prepare message representation
   //    message representaion formally different
   //    than message digest
   //    f = most_signifinant_bits( bitsise(order), digest )
   f_msb_bits_in_r(Digest,eccParams->eccR);
	//	  
   //MessageRepresentation(orderBitSize, Digest, 20);
   pMsgRepresentation = CreateBigNum(fmt, Digest, 20);

   // additionally Montgomery's technique, empoyed in ECC (prime case)
   // strictly request f < (base point) order
   ippsMod_BN(pMsgRepresentation, pOrder, pMsgRepresentation);

   // sign
   //
   {
	   int bnLen;
	   Ipp8u pDataX[20];
	   Ipp8u pDataY[20];
	   IppsBigNumSGN sgn;
	   IppsBigNum* pSignC = DefineBigNum(BITS2WORD_SIZE(orderBitSize));
	   IppsBigNum* pSignD = DefineBigNum(BITS2WORD_SIZE(orderBitSize));

	     // generate (ephemeral) key pair
		IppsBigNum* pEphPrivate = NULL; 
		IppsECCPPoint* pEphPublic = (IppsECCPPoint*)( malloc(ecPointSize) );
		ippsECCPPointInit(feBitSize, pEphPublic);

#ifndef USE_IPP_RNG
   		//Use our RNG to create random ephemeral key.
		RNGf(pDataX,eccParams->eccR);
		pEphPrivate = CreateBigNum(fmt,pDataX,20);
		// calculate ephemeral key (public part)
		ippsECCPPublicKey(pEphPrivate, pEphPublic,pECC);
#else
		//If we want to use the IPP RNG, it needs to be seeded at init time
		//Currently not done
		pEphPrivate = DefineBigNum(BITS2WORD_SIZE(orderBitSize));
		//The GenKeyPair uses the Ipp RNG, 
		ippsECCPGenKeyPair(pEphPrivate, pEphPublic, pECC);
#endif
		//Private Key
		//
		  // set ephemeral key pair into EC context
		ippsECCPSetKeyPair(pEphPrivate, pEphPublic, ippFalse, pECC);

      pPrivKey   = CreateBigNum(fmt,
                                    signersKey,
                                    20);

      ippsECCPSetKeyPair(pPrivKey, NULL, ippTrue, pECC);

      ippsECCPSignDSA(pMsgRepresentation, pSignC,pSignD, pECC);
      ippsGet_BN(&sgn, &bnLen, pDataX, pSignC);
      ippsGet_BN(&sgn, &bnLen, pDataY, pSignD);

	  Cvt32uto8uHS(pDataX, 20, pSignature);
	  Cvt32uto8uHS(pDataY, 20, pSignature+20);

	  free(pEphPublic);
      DestroyBigNum(pPrivKey);
	  DestroyBigNum(pEphPrivate);
	  DestroyBigNum(pSignC);
      DestroyBigNum(pSignD);
   }

	DestroyBigNum(pPrime);
	DestroyBigNum(pA);
	DestroyBigNum(pB);
	DestroyBigNum(pGX);
	DestroyBigNum(pGY);
	DestroyBigNum(pOrder);

   DestroyBigNum(pMsgRepresentation);
   free(pECC);

   return (IppECSignIsValid==result) ? 1:0;
   
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
  IppECResult result;

   const int feBitSize = 160;    // field element size (bits) is 160
   int ecPointSize;
   int eccStateSize;
   int orderBitSize;
   IppsECCP* pECC=NULL;
   IppsBigNum* pPrime=NULL;
   IppsBigNum* pA     = NULL;
   IppsBigNum* pB     = NULL;

   IppsBigNum* pGX    = NULL;
   IppsBigNum* pGY    = NULL;
   IppsBigNum* pOrder = NULL;
   IppsECCPPoint* pPoint = NULL;
   IppsECCPPoint* pPointG=NULL;

   IppsBigNum* pBigXk     = NULL;
   DataFormat fmt = HexString;   // actual data format

   // size (bytes) of EC point
   //

   ippsECCPPointBufferSize(feBitSize, &ecPointSize);

   // allocate & init ECC state
   //
   ippsECCPBufferSize(feBitSize, &eccStateSize);
   pECC = (IppsECCP*)( malloc(eccStateSize));
   ippsECCPInit(feBitSize, pECC);

   // establish EC cryptosystem based on yours parameters
   //
   pPrime = CreateBigNum(fmt, eccParams->eccP,  20);

   pA     = CreateBigNum(fmt, eccParams->eccA, 20);
   pB     = CreateBigNum(fmt, eccParams->eccB, 20);

   pGX    = CreateBigNum(fmt, eccParams->eccG,    20);
   pGY    = CreateBigNum(fmt, eccParams->eccG+20,    20);
   pOrder = CreateBigNum(fmt, eccParams->eccR, 20);

     // set EC parameters
   ippsECCPSet(pPrime,
                  pA,pB,
                  pGX,pGY,pOrder,
                  1,    
                  pECC);
     ippsECCPGetOrderBitSize(&orderBitSize, pECC);
	/* Generate random integer Xk in [1, r) */
    RNGf(pXk, eccParams->eccR);

	pBigXk=CreateBigNum(fmt, pXk, 20);
	pPoint=(IppsECCPPoint*)( malloc(ecPointSize) );
	ippsECCPPointInit(feBitSize, pPoint);

	pPointG= (IppsECCPPoint*)( malloc(ecPointSize) );
    ippsECCPPointInit(feBitSize, pPointG);
    ippsECCPSetPoint(pGX,pGY, pPointG, pECC);


	/* Calculate ec pt Xv = Xk*G */
	ippsECCPMulPointScalar(pPointG, pBigXk, pPoint, pECC);

	{	//Convert
	   int bnLen;
	   Ipp8u pDataX[20];
	   Ipp8u pDataY[20];
	   IppsBigNumSGN sgn;
	   IppsBigNum* pPointX = DefineBigNum(BITS2WORD_SIZE(orderBitSize));
	   IppsBigNum* pPointY=  DefineBigNum(BITS2WORD_SIZE(orderBitSize));

	   ippsECCPGetPoint(pPointX,pPointY, pPoint, pECC);
	   ippsGet_BN(&sgn, &bnLen, pDataX, pPointX);
       ippsGet_BN(&sgn, &bnLen, pDataY, pPointY);


	  Cvt32uto8uHS(pDataX, 20, pXv);
	  Cvt32uto8uHS(pDataY, 20, pXv+20);
		
		DestroyBigNum(pPointX);
		DestroyBigNum(pPointY);
	}





	free(pECC);
	free(pPoint);
	free(pPointG);

	DestroyBigNum(pPrime);
	DestroyBigNum(pA);
	DestroyBigNum(pB);
	DestroyBigNum(pGX);
	DestroyBigNum(pGY);
	DestroyBigNum(pOrder);
	DestroyBigNum(pBigXk);

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

   const int feBitSize = 160;    // field element size (bits) is 160
   int ecPointSize;
   int eccStateSize;
   int orderBitSize;
   IppsECCP* pECC=NULL;
   IppsBigNum* pPrime=NULL;
   IppsBigNum* pA     = NULL;
   IppsBigNum* pB     = NULL;

   IppsBigNum* pGX    = NULL;
   IppsBigNum* pGY    = NULL;
   IppsBigNum* pOrder = NULL;
   IppsECCPPoint* pPoint = NULL;
   IppsECCPPoint* pPointOut = NULL;

   IppsBigNum* pBigXk     = NULL;
   DataFormat fmt = HexString;   // actual data format
   ippsECCPPointBufferSize(feBitSize, &ecPointSize);

   // allocate & init ECC state
   //
   ippsECCPBufferSize(feBitSize, &eccStateSize);
   pECC = (IppsECCP*)( malloc(eccStateSize));
   ippsECCPInit(feBitSize, pECC);

   // establish EC cryptosystem based on yours parameters
   //
   pPrime = CreateBigNum(fmt, eccParams->eccP,  20);

   pA     = CreateBigNum(fmt, eccParams->eccA, 20);
   pB     = CreateBigNum(fmt, eccParams->eccB, 20);

   pGX    = CreateBigNum(fmt, eccParams->eccG,    20);
   pGY    = CreateBigNum(fmt, eccParams->eccG+20,    20);
   pOrder = CreateBigNum(fmt, eccParams->eccR, 20);

     // set EC parameters
   ippsECCPSet(pPrime,
                  pA,pB,
                  pGX,pGY,pOrder,
                  1,    
                  pECC);

   {
	   int bnLen;
	   Ipp8u pDataX[20];
	   Ipp8u pTempX[20];
	   IppsBigNumSGN sgn;

	  IppsBigNum* pPointX  = CreateBigNum(fmt, pYv, 20);
      IppsBigNum* pPointY  = CreateBigNum(fmt, pYv+20, 20);
      IppsBigNum* pSecret  = CreateBigNum(fmt, pXk, 20);

  	  pPoint= (IppsECCPPoint*)( malloc(ecPointSize) );
      ippsECCPPointInit(feBitSize, pPoint);
      ippsECCPSetPoint(pPointX,pPointY, pPoint, pECC);

   	  pPointOut= (IppsECCPPoint*)( malloc(ecPointSize) );
      ippsECCPPointInit(feBitSize, pPointOut);

  	  ippsECCPMulPointScalar(pPoint, pSecret, pPointOut, pECC);

	  ippsECCPGetPoint(pPointX,pPointY, pPointOut, pECC);
	  ippsGet_BN(&sgn, &bnLen, pDataX, pPointX);
	  /*Output is lsb 96 bits(12 bytes) of X coord*/ 
  	  Cvt32uto8uHS(pDataX, 20, pTempX);

	  LsbN(pKauth,pTempX,12);
	  DestroyBigNum(pPointX);
	  DestroyBigNum(pPointY);
	  DestroyBigNum(pSecret);
	  free(pPoint);
	  free(pPointOut);

   }
   	free(pECC);

	DestroyBigNum(pPrime);
	DestroyBigNum(pA);
	DestroyBigNum(pB);
	DestroyBigNum(pGX);
	DestroyBigNum(pGY);
	DestroyBigNum(pOrder);

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
   W8 * eccB,                /*  IN:  EC curve coefficient A (160 bits)  */
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
