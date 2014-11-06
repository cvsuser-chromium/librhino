/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2002-2003 Intel Corporation. All Rights Reserved.
//
//              Intel(R) Integrated Performance Primitives
//                  Cryptographic Primitives (ippcp)
//
*/

#if !defined( __IPPCP_H__ ) || defined( _OWN_BLDPCS )
#define __IPPCP_H__

#ifndef __IPPDEFS_H__
  #include "ippdefs.h"
#endif

#ifdef  __cplusplus
extern "C" {
#endif


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippcpGetLibVersion
//  Purpose:    getting of the library version
//  Returns:    the structure of information about version of ippCP library
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/
IPPAPI( const IppLibraryVersion*, ippcpGetLibVersion, (void) )



/* Intel(R) Integrated Performance Primitives. Symmetric Cryptographic Object Module API */

#if !defined( _OWN_BLDPCS )

typedef enum {
    NONE  = 0, IppsCPPaddingNONE  = 0,
    PKCS7 = 1, IppsCPPaddingPKCS7 = 1, 
    ZEROS = 2, IppsCPPaddingZEROS = 2
} IppsCPPadding;

typedef struct _ippcpRijndael128 IppsRijndael128;
typedef struct _ippcpRijndael192 IppsRijndael192;
typedef struct _ippcpRijndael256 IppsRijndael256;

#endif /* _OWN_BLDPCS */


/*
// Pure Rijndael
*/
#if !defined( _OWN_BLDPCS )

typedef enum {
   IppsRijndaelKey128 = 128, /* 128-bit key (4 word) */
   IppsRijndaelKey192 = 192, /* 192-bit key (6 word) */
   IppsRijndaelKey256 = 256  /* 256-bit key (8 word) */
} IppsRijndaelKeyLength;

#endif /* _OWN_BLDPCS */

/*
// Rijndael of 128-bit message block size (AES)
*/
IPPAPI(IppStatus, ippsRijndael128BufferSize,(int *pSize))
IPPAPI(IppStatus, ippsRijndael128Init,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen, IppsRijndael128* pCtx))

IPPAPI(IppStatus, ippsRijndael128EncryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael128* pCtx,
                                             IppsCPPadding padding))
IPPAPI(IppStatus, ippsRijndael128DecryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael128* pCtx,
                                             IppsCPPadding padding))

IPPAPI(IppStatus, ippsRijndael128EncryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael128* pCtx,
                                             const Ipp8u* pIV,
                                             IppsCPPadding padding))
IPPAPI(IppStatus, ippsRijndael128DecryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael128* pCtx,
                                             const Ipp8u* pIV,
                                             IppsCPPadding padding))

IPPAPI(IppStatus, ippsRijndael128EncryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int cfbBlkSize,
                                             const IppsRijndael128* pCtx,
                                             const Ipp8u* pIV,
                                             IppsCPPadding padding))
IPPAPI(IppStatus, ippsRijndael128DecryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int cfbBlkSize,
                                             const IppsRijndael128* pCtx,
                                             const Ipp8u* pIV,
                                             IppsCPPadding padding))
/*
// Rijndael of 192-bit message block size
*/
IPPAPI(IppStatus, ippsRijndael192BufferSize,(int *pSize))
IPPAPI(IppStatus, ippsRijndael192Init,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen, IppsRijndael192* pCtx))

IPPAPI(IppStatus, ippsRijndael192EncryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael192* pCtx,
                                             IppsCPPadding padding))
IPPAPI(IppStatus, ippsRijndael192DecryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael192* pCtx,
                                             IppsCPPadding padding))

IPPAPI(IppStatus, ippsRijndael192EncryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael192* pCtx,
                                             const Ipp8u* pIV,
                                             IppsCPPadding padding))
IPPAPI(IppStatus, ippsRijndael192DecryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael192* pCtx,
                                             const Ipp8u* pIV,
                                             IppsCPPadding padding))

IPPAPI(IppStatus, ippsRijndael192EncryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int cfbBlkSize,
                                             const IppsRijndael192* pCtx,
                                             const Ipp8u* pIV,
                                             IppsCPPadding padding))
IPPAPI(IppStatus, ippsRijndael192DecryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int cfbBlkSize,
                                             const IppsRijndael192* pCtx,
                                             const Ipp8u* pIV,
                                             IppsCPPadding padding))
/*
// Rijndael of 256-bit message block size
*/
IPPAPI(IppStatus, ippsRijndael256BufferSize,(int *pSize))
IPPAPI(IppStatus, ippsRijndael256Init,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen, IppsRijndael256* pCtx))

IPPAPI(IppStatus, ippsRijndael256EncryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael256* pCtx,
                                             IppsCPPadding padding))
IPPAPI(IppStatus, ippsRijndael256DecryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael256* pCtx,
                                             IppsCPPadding padding))

IPPAPI(IppStatus, ippsRijndael256EncryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael256* pCtx,
                                             const Ipp8u* pIV,
                                             IppsCPPadding padding))
IPPAPI(IppStatus, ippsRijndael256DecryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael256* pCtx,
                                             const Ipp8u* pIV,
                                             IppsCPPadding padding))

IPPAPI(IppStatus, ippsRijndael256EncryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int cfbBlkSize,
                                             const IppsRijndael256* pCtx,
                                             const Ipp8u* pIV,
                                             IppsCPPadding padding))
IPPAPI(IppStatus, ippsRijndael256DecryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int cfbBlkSize,
                                             const IppsRijndael256* pCtx,
                                             const Ipp8u* pIV,
                                             IppsCPPadding padding))



/* Intel(R) Integrated Performance Primitives. One-Way Hash Function Object Module API */

#if !defined( _OWN_BLDPCS )

typedef struct _ippcpSHA1     IppsSHA1;

#endif /* _OWN_BLDPCS */

/*
// SHA1 Hash Primitives
*/
IPPAPI(IppStatus, ippsSHA1BufferSize,(int* pSize))
IPPAPI(IppStatus, ippsSHA1Init,(IppsSHA1* pState))

IPPAPI(IppStatus, ippsSHA1Update,(const Ipp8u* pSrc, int len, IppsSHA1* pState))
IPPAPI(IppStatus, ippsSHA1Final,(Ipp8u* pMD, IppsSHA1* pState))
IPPAPI(IppStatus, ippsSHA1MessageDigest,(const Ipp8u* pMsg, int len, Ipp8u* pMD))


/* Intel(R) Integrated Performance Primitives. Big Number Object Module API */

#if !defined( _OWN_BLDPCS )

typedef enum {
   IppsBigNumNEG=0,
   IppsBigNumPOS=1
} IppsBigNumSGN;

typedef enum {
   IppsBinaryMethod=0,
   IppsSlidingWindows=1
} IppsExpMethod;

typedef struct _ippcpBigNum      IppsBigNum; // structuralized buffer for BigNum arithmetics
typedef struct _ippcpMontgomery  IppsMont;   // structuralized buffer for Montgomery modulo scheme
typedef struct _ippcpPRNG        IppsPRNG;   // structuralized buffer for PRNG
typedef struct _ippcpPrime       IppsPrime;  // structuralized buffer for probable prime gen

#endif /* _OWN_BLDPCS */

#define IS_ZERO 0
#define GREATER_THAN_ZERO 1
#define LESS_THAN_ZERO 2
#define IS_PRIME 3
#define IS_COMPOSITE 4
#define IS_VALID_KEY 5
#define IS_INVALID_KEY 6
#define IS_INCOMPLETED_KEY 7


/* BigNum Arithmatic Operation */
// unsigned functions 
IPPAPI(IppStatus, ippsAdd_BNU, (const Ipp32u *a, const Ipp32u *b, Ipp32u *r, 
                                int n, Ipp32u *carry))
IPPAPI(IppStatus, ippsSub_BNU, (const Ipp32u *a, const Ipp32u *b, Ipp32u *r,
                                int n, Ipp32u *carry))

IPPAPI(IppStatus, ippsMulOne_BNU,  (const Ipp32u *a, Ipp32u *r, int n, 
                                    Ipp32u w, Ipp32u *carry))
IPPAPI(IppStatus, ippsMACOne_BNU_I,(const Ipp32u *a, Ipp32u *r, int n, 
                                    Ipp32u w, Ipp32u *carry))

IPPAPI(IppStatus, ippsMul_BNU4, (const Ipp32u *a, const Ipp32u *b, Ipp32u *r))
IPPAPI(IppStatus, ippsMul_BNU8, (const Ipp32u *a, const Ipp32u *b, Ipp32u *r))
IPPAPI(IppStatus, ippsSqr_BNU4, (const Ipp32u *a, Ipp32u *r))
IPPAPI(IppStatus, ippsSqr_BNU8, (const Ipp32u *a, Ipp32u *r))

IPPAPI(IppStatus, ippsDiv_64u32u, (Ipp64u a, Ipp32u b, Ipp32u *q, Ipp32u *r))
IPPAPI(IppStatus, ippsSqr_32u64u, (const Ipp32u *src, int n, Ipp64u *dst))

// signed functions
IPPAPI(IppStatus, ippsBigNumBufferSize, (int length, int *size))
IPPAPI(IppStatus, ippsBigNumInit, (int length, IppsBigNum *b))

IPPAPI(IppStatus, ippsCmpZero_BN, (const IppsBigNum *b, Ipp32u *result))
IPPAPI(IppStatus, ippsGetSize_BN, (const IppsBigNum *b, int *size))
IPPAPI(IppStatus, ippsSet_BN, (IppsBigNumSGN sgn, int length, const Ipp32u *data, 
                               IppsBigNum *x))
IPPAPI(IppStatus, ippsGet_BN, (IppsBigNumSGN *sgn, int *length, Ipp32u *data,
                               const IppsBigNum *x))

IPPAPI(IppStatus, ippsAdd_BN,   (IppsBigNum *a, IppsBigNum *b, IppsBigNum *r))
IPPAPI(IppStatus, ippsSub_BN,   (IppsBigNum *a, IppsBigNum *b, IppsBigNum *r))
IPPAPI(IppStatus, ippsMul_BN,   (IppsBigNum *a, IppsBigNum *b, IppsBigNum *r))
IPPAPI(IppStatus, ippsMAC_BN_I, (IppsBigNum *a, IppsBigNum *b, IppsBigNum *r))
IPPAPI(IppStatus, ippsDiv_BN,   (IppsBigNum *a, IppsBigNum *b, IppsBigNum *q, IppsBigNum *r))
IPPAPI(IppStatus, ippsMod_BN,   (IppsBigNum *a, IppsBigNum *m, IppsBigNum *r))
IPPAPI(IppStatus, ippsGcd_BN,   (IppsBigNum *a, IppsBigNum *b, IppsBigNum *g))
IPPAPI(IppStatus, ippsModInv_BN,(IppsBigNum *e, IppsBigNum *m, IppsBigNum *d))


/* Montgomery Module Operation */
IPPAPI(IppStatus, ippsMontBufferSize, (IppsExpMethod method, int length, int *size))
IPPAPI(IppStatus, ippsMontInit, (IppsExpMethod method, int length, IppsMont *m))

IPPAPI(IppStatus, ippsMontSet, (const Ipp32u *n, int length, IppsMont *m))
IPPAPI(IppStatus, ippsMontGet, (Ipp32u *n, int *length, const IppsMont *m))
IPPAPI(IppStatus, ippsMontForm,(IppsBigNum *a, IppsMont *m, IppsBigNum *r))
IPPAPI(IppStatus, ippsMontMul, (IppsBigNum *a, IppsBigNum *b, IppsMont *m, IppsBigNum *r))
IPPAPI(IppStatus, ippsMontExp, (IppsBigNum *a, IppsBigNum *e, IppsMont *m, IppsBigNum *r))

/* Pseudo-Random Number Generation */
IPPAPI(IppStatus, ippsPRNGBufferSize, (int bitlength, int *size))
IPPAPI(IppStatus, ippsPRNGInit, (int bitlength, IppsPRNG *r))

IPPAPI(IppStatus, ippsPRNGSetPrimeQ, (Ipp32u *q, IppsPRNG *r))
IPPAPI(IppStatus, ippsPRNGSetSeed, (Ipp32u *seed, IppsPRNG *r))
IPPAPI(IppStatus, ippsPRNGAdd, (IppsBigNum *c, int bitlength, IppsPRNG *r))
IPPAPI(IppStatus, ippsPRNGGen, (Ipp32u *seed, IppsBigNum *c, int bitlength,
                                Ipp32u top, Ipp32u bottom, IppsPRNG *r))
IPPAPI(IppStatus, ippsPRNGGetRand, (int *curbitlength, Ipp32u *rseq, const IppsPRNG *r))

/* Probable Prime Number Generation */
IPPAPI(IppStatus, ippsPrimeBufferSize, (IppsExpMethod method, int bitlength, int *size))
IPPAPI(IppStatus, ippsPrimeInit, (IppsExpMethod method, int bitlength, IppsPrime *p))

IPPAPI(IppStatus, ippsPrimeGen, (IppsBigNum *c, Ipp32u *seed, int bitlength, int t, 
                                 IppsPrime *p))
IPPAPI(IppStatus, ippsPrimeTest,(IppsPrime *p, IppsBigNum *c, Ipp32u *seed, int t,
                                 Ipp32u *result))
IPPAPI(IppStatus, ippsPrimeSet, (const Ipp32u *prime, int bitlength, IppsPrime *p))
IPPAPI(IppStatus, ippsPrimeGet, (Ipp32u *prime, int *bitlength, const IppsPrime *p))

/* Intel(R) Integrated Performance Primitives. ECC (prime) Cryptographic Object Module API */

#if !defined( _OWN_BLDPCS )

/* operation result */
typedef enum {
   IppECIsValid,           /* valid domain parameters          */
   IppECCompositeBase,     /* field based on composite         */
   IppECComplicatedBase,   /* number of non-zero terms in the polynomial (> PRIME_ARR_MAX) */
   IppECIsNotAG,           /* equation solutions aren't group  */
   IppECCompositeOrder,    /* composite order of base point    */
   IppECInvalidOrder,      /* invalid base point order         */
   IppECIsWeakMOV,         /* weak Meneze-Okamoto-Vanstone  reduction attack */
   IppECIsWeakSSSA,        /* weak Semaev-Smart,Satoh-Araki reduction attack */
   IppECIsSupersingular,   /* supersingular curve */

   IppECValidKey,
   IppECInvalidPrivateKey, /* !(0 < Private < order) */
   IppECInvalidPublicKey,  /* (order*PublicKey != Infinity)    */
   IppECInvalidKeyPair,    /* (Private*BasePoint != PublicKey) */

   IppECPointIsValid,      /* point (P=(Px,Py)) on EC        */
   IppECPointIsAtInfinite, /* point (P=(Px,Py)) at Infinity  */
   IppECPointIsNotValid,   /* point (P=(Px,Py)) out-of EC    */

   IppECPointIsEqual,      /* compared points are equial     */
   IppECPointIsNotEqual,   /* compared points are different  */

   IppECSignIsValid,       /*   valid signature */
   IppECSignIsInvalid      /* invalid signature */
} IppECResult;

/* set/get flags */
typedef enum {
   IppECCPStd      = 0x10000,       /* random (recommended) EC over FG(p): */
   IppECCPStd112r1 = IppECCPStd,    /* secp112r1 request */
   IppECCPStd112r2 = IppECCPStd+1,  /* secp112r2 request */
   IppECCPStd128r1 = IppECCPStd+2,  /* secp128r1 request */
   IppECCPStd128r2 = IppECCPStd+3,  /* secp128r2 request */
   IppECCPStd160r1 = IppECCPStd+4,  /* secp160r1 request */
   IppECCPStd160r2 = IppECCPStd+5,  /* secp160r2 request */
   IppECCPStd192r1 = IppECCPStd+6,  /* secp192r1 request */
   IppECCPStd224r1 = IppECCPStd+7,  /* secp224r1 request */
   IppECCPStd256r1 = IppECCPStd+8,  /* secp256r1 request */
   IppECCPStd384r1 = IppECCPStd+9,  /* secp384r1 request */
   IppECCPStd521r1 = IppECCPStd+10, /* secp521r1 request */

   IppECCBStd      = 0x20000,       /* random (recommended) EC over FG(2^m): */
   IppECCBStd113r1 = IppECCBStd,    /* sect113r1 request */
   IppECCBStd113r2 = IppECCBStd+1,  /* sect113r2 request */
   IppECCBStd131r1 = IppECCBStd+2,  /* sect131r1 request */
   IppECCBStd131r2 = IppECCBStd+3,  /* sect131r2 request */
   IppECCBStd163r1 = IppECCBStd+4,  /* sect163r1 request */
   IppECCBStd163r2 = IppECCBStd+5,  /* sect163r2 request */
   IppECCBStd193r1 = IppECCBStd+6,  /* sect193r1 request */
   IppECCBStd193r2 = IppECCBStd+7,  /* sect193r2 request */
   IppECCBStd233r1 = IppECCBStd+8,  /* sect233r1 request */
   IppECCBStd283r1 = IppECCBStd+9,  /* sect283r1 request */
   IppECCBStd409r1 = IppECCBStd+10, /* sect409r1 request */
   IppECCBStd571r1 = IppECCBStd+11, /* sect571r1 request */

   IppECCKStd      = 0x40000,       /* Koblitz (recommended) EC over FG(2^m): */
   IppECCBStd163k1 = IppECCKStd,    /* Koblitz 163 request */
   IppECCBStd233k1 = IppECCKStd+1,  /* Koblitz 233 request */
   IppECCBStd239k1 = IppECCKStd+2,  /* Koblitz 239 request */
   IppECCBStd283k1 = IppECCKStd+3,  /* Koblitz 283 request */
   IppECCBStd409k1 = IppECCKStd+4,  /* Koblitz 409 request */
   IppECCBStd571k1 = IppECCKStd+5   /* Koblitz 571 request */
} IppECCType;

#endif /* _OWN_BLDPCS */

IPPAPI( const char*, ippECCGetResultString, (IppECResult code))


/*
// ECCP (prime case),  EC: y^2 = x^3 +A*x + B
*/
#if !defined( _OWN_BLDPCS )
   typedef struct _ippcpECCP      IppsECCP;
   typedef struct _ippcpECCB      IppsECCB;
   typedef struct _ippcpECCPPoint IppsECCPPoint;
   typedef struct _ippcpECCBPoint IppsECCBPoint;
#endif /* _OWN_BLDPCS */

/*
// ECC Initialization
*/
IPPAPI(IppStatus, ippsECCPBufferSize, (int feBitSize, int* pSize))
IPPAPI(IppStatus, ippsECCBBufferSize, (int feBitSize, int* pSize))

IPPAPI(IppStatus, ippsECCPInit, (int feBitSize, IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBInit, (int feBitSize, IppsECCB* pECC))

/*
// ECC Domain Parameters Set Up and Retrieve
*/
IPPAPI(IppStatus, ippsECCPSet, (const IppsBigNum* pPrime,
                                const IppsBigNum* pA, const IppsBigNum* pB,
                                const IppsBigNum* pGX,const IppsBigNum* pGY,const IppsBigNum* pOrder,
                                int cofactor,
                                IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBSet, (const IppsBigNum* pPrime,
                                const IppsBigNum* pA, const IppsBigNum* pB,
                                const IppsBigNum* pGX,const IppsBigNum* pGY,const IppsBigNum* pOrder,
                                int cofactor,
                                IppsECCB* pECC))

IPPAPI(IppStatus, ippsECCPSetStd,(IppECCType flag, IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBSetStd,(IppECCType flag, IppsECCB* pECC))

IPPAPI(IppStatus, ippsECCPSetRand,(const IppsBigNum* pSeed, const IppsBigNum* pContent,
                                   IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBSetRand,(const IppsBigNum* pSeed, const IppsBigNum* pContent,
                                   IppsECCB* pECC))

IPPAPI(IppStatus, ippsECCPGet, (IppsBigNum* pPrime,
                                IppsBigNum* pA, IppsBigNum* pB,
                                IppsBigNum* pGX,IppsBigNum* pGY,IppsBigNum* pOrder,
                                int* cofactor,
                                IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBGet, (IppsBigNum* pPrime,
                                IppsBigNum* pA, IppsBigNum* pB,
                                IppsBigNum* pGX,IppsBigNum* pGY,IppsBigNum* pOrder,
                                int* cofactor,
                                IppsECCB* pECC))

IPPAPI(IppStatus, ippsECCPGetOrderBitSize,(int* pBitSize, IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBGetOrderBitSize,(int* pBitSize, IppsECCB* pECC))

/*
// ECC Validation
*/
IPPAPI(IppStatus, ippsECCPValidate, (IppsBigNum* c, const Ipp32u* pSeed, int nTrials, IppECResult* pResult, IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBValidate, (IppsBigNum* c, const Ipp32u* pSeed, int nTrials, IppECResult* pResult, IppsECCB* pECC))

/*
// EC Point Initialization
*/
IPPAPI(IppStatus, ippsECCPPointBufferSize, (int feBitSize, int* pSize))
IPPAPI(IppStatus, ippsECCBPointBufferSize, (int feBitSize, int* pSize))

IPPAPI(IppStatus, ippsECCPPointInit, (int feBitSize, IppsECCPPoint* pPoint))
IPPAPI(IppStatus, ippsECCBPointInit, (int feBitSize, IppsECCBPoint* pPoint))

/*
// EC Point Set Up and Retrieve
*/
IPPAPI(IppStatus, ippsECCPSetPoint,(const IppsBigNum* pX, const IppsBigNum* pY,
                                    IppsECCPPoint* pPoint, IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBSetPoint,(const IppsBigNum* pX, const IppsBigNum* pY,
                                    IppsECCBPoint* pPoint, IppsECCB* pECC))

IPPAPI(IppStatus, ippsECCPSetPointAtInfinity,(IppsECCPPoint* pPoint, IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBSetPointAtInfinity,(IppsECCBPoint* pPoint, IppsECCB* pECC))

IPPAPI(IppStatus, ippsECCPGetPoint,(IppsBigNum* pX, IppsBigNum* pY,
                                    const IppsECCPPoint* pPoint, IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBGetPoint,(IppsBigNum* pX, IppsBigNum* pY,
                                    const IppsECCBPoint* pPoint, IppsECCB* pECC))

/*
// Point Tests (return result)
*/
IPPAPI(IppStatus, ippsECCPCheckPoint,  (const IppsECCPPoint* pP,
                                        IppECResult* pResult, IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBCheckPoint,  (const IppsECCBPoint* pP,
                                        IppECResult* pResult, IppsECCB* pECC))

/*
// Point Operations
*/
IPPAPI(IppStatus, ippsECCPComparePoint,(const IppsECCPPoint* pP, const IppsECCPPoint* pQ,
                                        IppECResult* pResult, IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBComparePoint,(const IppsECCBPoint* pP, const IppsECCBPoint* pQ,
                                        IppECResult* pResult, const IppsECCB* pECC))

IPPAPI(IppStatus, ippsECCPNegativePoint, (const IppsECCPPoint* pP,
                                          IppsECCPPoint* pR, IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBNegativePoint, (const IppsECCBPoint* pP,
                                          IppsECCBPoint* pR, IppsECCB* pECC))

IPPAPI(IppStatus, ippsECCPAddPoint,      (const IppsECCPPoint* pP, const IppsECCPPoint* pQ,
                                          IppsECCPPoint* pR, IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBAddPoint,      (const IppsECCBPoint* pP, const IppsECCBPoint* pQ,
                                          IppsECCBPoint* pR, IppsECCB* pECC))

IPPAPI(IppStatus, ippsECCPMulPointScalar,(const IppsECCPPoint* pP, const IppsBigNum* pK,
                                          IppsECCPPoint* pR, IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBMulPointScalar,(const IppsECCBPoint* pP, const IppsBigNum* pK,
                                          IppsECCBPoint* pR, IppsECCB* pECC))

/*
// Key Generation, Validation and Set Up
*/
IPPAPI(IppStatus, ippsECCPGenKeyPair,     (IppsBigNum* pPrivate, IppsECCPPoint* pPublic,
                                           IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBGenKeyPair,     (IppsBigNum* pPrivate, IppsECCBPoint* pPublic,
                                           IppsECCB* pECC))

IPPAPI(IppStatus, ippsECCPPublicKey,      (const IppsBigNum* pPrivate,
                                           IppsECCPPoint* pPublic,
                                           IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBPublicKey,      (const IppsBigNum* pPrivate,
                                           IppsECCBPoint* pPublic,
                                           IppsECCB* pECC))

IPPAPI(IppStatus, ippsECCPValidateKeyPair,(const IppsBigNum* pPrivate, const IppsECCPPoint* pPublic,
                                           IppECResult* pResult,
                                           IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBValidateKeyPair,(const IppsBigNum* pPrivate, const IppsECCBPoint* pPublic,
                                           IppECResult* pResult,
                                           IppsECCB* pECC))

IPPAPI(IppStatus, ippsECCPSetKeyPair,     (const IppsBigNum* pPrivate, const IppsECCPPoint* pPublic,
                                           IppBool regular,
                                           IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBSetKeyPair,     (const IppsBigNum* pPrivate, const IppsECCBPoint* pPublic,
                                           IppBool regular,
                                           IppsECCB* pECC))

/*
// Shared Value Derivation Functions (DH version)
*/
IPPAPI(IppStatus, ippsECCPDH, (const IppsECCPPoint* pPublic,
                               IppsBigNum* pShare,
                               IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBDH, (const IppsECCBPoint* pPublic,
                               IppsBigNum* pShare,
                               IppsECCB* pECC))

IPPAPI(IppStatus, ippsECCPDHC,(const IppsECCPPoint* pPublic,
                               IppsBigNum* pShare,
                               IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBDHC,(const IppsECCBPoint* pPublic,
                               IppsBigNum* pShare,
                               IppsECCB* pECC))

/*
// Singing/Verifying (DSA version)
*/
IPPAPI(IppStatus, ippsECCPSignDSA,  (const IppsBigNum* pMsgDigest,
                                     IppsBigNum* pSignX, IppsBigNum* pSignY,
                                     IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBSignDSA,  (const IppsBigNum* pMsgDigest,
                                     IppsBigNum* pSignX, IppsBigNum* pSignY,
                                     IppsECCB* pECC))

IPPAPI(IppStatus, ippsECCPVerifyDSA,(const IppsBigNum* pMsgDigest,
                                     const IppsBigNum* pSignX, const IppsBigNum* pSignY,
                                     IppECResult* pResult,
                                     IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBVerifyDSA,(const IppsBigNum* pMsgDigest,
                                     const IppsBigNum* pSignX, const IppsBigNum* pSignY,
                                     IppECResult* pResult,
                                     IppsECCB* pECC))

/*
// Singing/Verifying (NR version)
*/
IPPAPI(IppStatus, ippsECCPSignNR,   (const IppsBigNum* pMsgDigest,
                                     IppsBigNum* pSignX, IppsBigNum* pSignY,
                                     IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBSignNR,   (const IppsBigNum* pMsgDigest,
                                     IppsBigNum* pSignX, IppsBigNum* pSignY,
                                     IppsECCB* pECC))

IPPAPI(IppStatus, ippsECCPVerifyNR, (const IppsBigNum* pMsgDigest,
                                     const IppsBigNum* pSignX, const IppsBigNum* pSignY,
                                     IppECResult* pResult,
                                     IppsECCP* pECC))
IPPAPI(IppStatus, ippsECCBVerifyNR, (const IppsBigNum* pMsgDigest,
                                     const IppsBigNum* pSignX, const IppsBigNum* pSignY,
                                     IppECResult* pResult,
                                     IppsECCB* pECC))

#ifdef  __cplusplus
}
#endif

#endif /* __IPPCP_H__ */



