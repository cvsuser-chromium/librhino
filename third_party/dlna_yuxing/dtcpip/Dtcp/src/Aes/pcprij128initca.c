/*
//               INTEL CORPORATION PROPRIETARY INFORMATION
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Intel Corporation and may not be copied
// or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2002 Intel Corporation. All Rights Reserved.
//
//
// Purpose:
//    Cryptography Primitive.
//    Initialization of Rijndael
//
// Contents:
//    ippsRijndael128GetSpecSize()
//    ippsRijndael128Init()
//
//
//    Created: Mon 20-May-2002 07:27
//  Author(s): Sergey Kirillov
//       mail: SergeyX_Kirillov@vniief.ims.intel.com
*/
#include "precomp.h"
#include "owncp.h"
#include "pcprij.h"


/*
// number of rounds (use [NK] for access)
*/
static int rij128nRounds[3] = {NR128_128, NR128_192, NR128_256};

/*
// number of keys (estimation only!)  (use [NK] for access)
//
// accurate number of keys necassary for encrypt/decrypt are:
//    nKeys = NB * (NR+1)
//       where NB - data block size (32-bit words)
//             NR - number of rounds (depend on NB and keyLen)
//
// but the estimation
//    estnKeys = (NK*n) >= nKeys
// or
//    estnKeys = ( (NB*(NR+1) + (NK-1)) / NK) * NK
//       where NK - key length (words)
//             NB - data block size (word)
//             NR - number of rounds (depend on NB and keyLen)
//             nKeys - accurate numner of keys
// is more convinient when calculates key extension
*/
static int rij128nKeys[3] = {44,  54,  64 };

/*
// helper for nRounds[] and estnKeys[] access
// note: x is length in 32-bits words
*/
__INLINE int rij_index(int x)
{ return (x-NB(128))>>1; }


/*F*
//    Name: ippsRijndael128BufferSize
//
// Purpose: Returns size of RIJ spec (bytes).
//
// Returns:                Reason:
//    ippStsNullPtrErr        pSzie == NULL
//    ippStsNoErr             no errors
//
// Parameters:
//    pSize       pointer RIJ spec size
//
*F*/
IPPFUN(IppStatus, ippsRijndael128BufferSize,(int* pSize))
{
   /* test size's pointer */
   IPP_BAD_PTR1_RET(pSize);

   *pSize = sizeof(IppsRijndael128)
           +sizeof(Ipp32u);

   return ippStsNoErr;
}


/*F*
//    Name: ippsRijndael128Init
//
// Purpose: Init RIJ spec for future usage.
//
// Returns:                Reason:
//    ippStsNullPtrErr        pKey == NULL
//                            pCtx == NULL
//    ippStsLengthErr         keyLen != IppsRijndaelKey128
//                            keyLen != IppsRijndaelKey192
//                            keyLen != IppsRijndaelKey256
//
// Parameters: 
//    pKey        security key
//    pCtx        pointer RIJ spec
//
*F*/
IPPFUN(IppStatus, ippsRijndael128Init,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen, IppsRijndael128* pCtx))
{
   int keyWords;
   int nExpKeys;
   int nRounds;

   /* test RIJ keyLen */
   IPP_BADARG_RET(( (keyLen!=IppsRijndaelKey128) &&
                    (keyLen!=IppsRijndaelKey192) &&
                    (keyLen!=IppsRijndaelKey256)), ippStsLengthErr);
   /* test key's & spec's pointers */
   IPP_BAD_PTR2_RET(pKey, pCtx);
   /* use 4-byte aligned Rijndael context */
   pCtx = (IppsRijndael128*)( IPP_ALIGNED_PTR(pCtx, 4) );

   keyWords = NK(keyLen);
   nExpKeys = rij128nKeys  [ rij_index(keyWords) ];
   nRounds  = rij128nRounds[ rij_index(keyWords) ];

   /* init spec */
   RIJ_ID(pCtx) = idCtxRijndael;
   RIJ_NB(pCtx) = NB(128);
   RIJ_NK(pCtx) = keyWords;
   RIJ_NR(pCtx) = nRounds;

   /* set key expansion */
   ExpandRijndaelKey(pKey, keyWords, NB(128), nRounds, nExpKeys,
                     RIJ_EKEYS(pCtx),
                     RIJ_DKEYS(pCtx));

   return ippStsNoErr;
}
