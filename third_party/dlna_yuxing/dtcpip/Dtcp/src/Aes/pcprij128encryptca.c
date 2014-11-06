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
//    Encrypt 128-bit data block according to Rijndael
//
// Contents:
//    ippsRijndael128Encrypt()
//
//
//    Created: Tue 23-May-2002 10:12
//  Author(s): Sergey Kirillov
//       mail: SergeyX_Kirillov@vniief.ims.intel.com
*/
#if 0

#include "precomp.h"
#include "owncp.h"
#include "pcprij.h"
#include "pcpciphertool.h"


/*F*
//    Name: ippsRijndael128Encrypt   
//
// Purpose: Encrypts single 128-bit block accorging to Rijndael scheme.
//
// Returns:                Reason:
//    ippStsNullPtrErr        pCtx == NULL
//                            pInpBlk == NULL
//                            pOutBlk == NULL
//    ippStsContextMatchErr   pCtx->idCtx != idCtxRijndael
//    ippStsLengthErr         pCtx->nb mismatch function suffix 128
//    ippStsNoErr             no errors
//
// Parameters:
//    pInpBlk     pointer to the input  block 
//    pOutBlk     pointer to the output block
//    pCtx        pointer to the RIJ context
//
*F*/
IPPFUN(IppStatus, ippsRijndael128Encrypt,(const Ipp8u* pInpBlk,
                                                Ipp8u* pOutBlk,
                                          const IppsRijndael128* pCtx))
{
   /* test context */
   IPP_BAD_PTR1_RET(pCtx);
   /* use 4-byte aligned Rijndael context */
   pCtx = (IppsRijndael128*)( IPP_ALIGNED_PTR(pCtx, 4) );

   IPP_BADARG_RET(!RIJ_ID_TEST(pCtx), ippStsContextMatchErr);
   /* test input/output block pointers */
   IPP_BAD_PTR2_RET(pInpBlk, pOutBlk);
   /* test data block size */
   IPP_BADARG_RET(NB(128)!=RIJ_NB(pCtx), ippStsLengthErr);

   /*
   // encrypt data block
   */
   if( !(IPP_UINT_PTR(pInpBlk) & 0x3) && !(IPP_UINT_PTR(pOutBlk) & 0x3))
      Encrypt_RIJ128((const Ipp32u*)pInpBlk, (Ipp32u*)pOutBlk, RIJ_NR(pCtx), RIJ_EKEYS(pCtx));
   else {
      Ipp32u tmpInp[NB(128)];
      Ipp32u tmpOut[NB(128)];

      CopyBlock16(pInpBlk, tmpInp);
      Encrypt_RIJ128(tmpInp, tmpOut, RIJ_NR(pCtx), RIJ_EKEYS(pCtx));
      CopyBlock16(pOutBlk, tmpOut);
   }

   return ippStsNoErr;
}
#endif
