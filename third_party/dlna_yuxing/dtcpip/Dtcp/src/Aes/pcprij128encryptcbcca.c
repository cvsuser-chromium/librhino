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
//    Encrypt byte data stream according to Rijndael (CBC mode)
//
// Contents:
//    ippsRijndael128EncryptCBC()
//
//
//    Created: Sat 25-May-2002 18:36
//  Author(s): Sergey Kirillov
//       mail: SergeyX_Kirillov@vniief.ims.intel.com
*/
#include "precomp.h"
#include "owncp.h"
#include "pcprij.h"
#include "pcpciphertool.h"


/*F*
//    Name: ippsRijndael128EncryptCBC
//
// Purpose: Encrypt byter data stream according to Rijndael in CBC mode.
//
// Returns:                Reason:
//    ippStsNullPtrErr        pCtx == NULL
//                            pSrc == NULL
//                            pDst == NULL
//                            pIV  == NULL
//    ippStsContextMatchErr   pCtx->idCtx != idCtxRijndael
//    ippStsLengthErr         len <1
//                            pCtx->nb mismatch function suffix 128
//    ippStsPaddingSchemeErr  padding != IppsCPPaddingPKCS7
//                            padding != IppsCPPaddingZEROS
//                            padding != IppsCPPaddingNONE
//    ippStsUnderRunErr       (padding==IppsCPPaddingNONE) && (len%MBS_RIJ128)
//    ippStsNoErr             no errors
//
// Parameters:
//    pSrc        pointer to the source data stream
//    pDst        pointer to the target data stream
//    len         plaintext stream length (bytes)
//    pCtx        RIJ context
//    pIV         pointer to the initialization vector
//    padding     the padding scheme indicator
//
*F*/
IPPFUN(IppStatus, ippsRijndael128EncryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael128* pCtx,
                                             const Ipp8u* pIV,
                                             IppsCPPadding padding))
{
   Ipp32u tmpOut[NB(128)];
   Ipp32u     iv[NB(128)];

   /* test context */
   IPP_BAD_PTR1_RET(pCtx);
   /* use 4-byte aligned Rijndael context */
   pCtx = (IppsRijndael128*)( IPP_ALIGNED_PTR(pCtx, 4) );

   IPP_BADARG_RET(!RIJ_ID_TEST(pCtx), ippStsContextMatchErr);
   /* test source, initialization and destination pointers */
   IPP_BAD_PTR3_RET(pSrc, pIV, pDst);
   /* test stream length */
   IPP_BADARG_RET((len<1), ippStsLengthErr);
   /* test data block size */
   IPP_BADARG_RET(NB(128)!=RIJ_NB(pCtx), ippStsLengthErr);
   /* test padding */
   IPP_BADARG_RET(((IppsCPPaddingPKCS7!=padding)&&
                   (IppsCPPaddingZEROS!=padding)&&
                   (IppsCPPaddingNONE !=padding)), ippStsPaddingSchemeErr);
   /* test stream integrity */
   IPP_BADARG_RET(((len&(MBS_RIJ128-1)) && (IppsCPPaddingNONE==padding)), ippStsUnderRunErr);

   /* read IV */
   CopyBlock16(pIV, iv);

   /*
   // encrypt block-by-block aligned streams
   */
   if( !(IPP_UINT_PTR(pSrc) & 0x3) && !(IPP_UINT_PTR(pDst) & 0x3)) {
      while(len >= MBS_RIJ128) {
         iv[0] ^= ((Ipp32u*)pSrc)[0];
         iv[1] ^= ((Ipp32u*)pSrc)[1];
         iv[2] ^= ((Ipp32u*)pSrc)[2];
         iv[3] ^= ((Ipp32u*)pSrc)[3];

         Encrypt_RIJ128(iv, (Ipp32u*)pDst, RIJ_NR(pCtx), RIJ_EKEYS(pCtx));

         iv[0] = ((Ipp32u*)pDst)[0];
         iv[1] = ((Ipp32u*)pDst)[1];
         iv[2] = ((Ipp32u*)pDst)[2];
         iv[3] = ((Ipp32u*)pDst)[3];

         pSrc += MBS_RIJ128;
         pDst += MBS_RIJ128;
         len  -= MBS_RIJ128;
      }
   }

   /*
   // encrypt block-by-block misaligned streams
   */
   else {
      while(len >= MBS_RIJ128) {
         XorBlock16(pSrc, iv, iv);

         Encrypt_RIJ128(iv, tmpOut, RIJ_NR(pCtx), RIJ_EKEYS(pCtx));

         CopyBlock16(tmpOut, pDst);

         iv[0] = tmpOut[0];
         iv[1] = tmpOut[1];
         iv[2] = tmpOut[2];
         iv[3] = tmpOut[3];

         pSrc += MBS_RIJ128;
         pDst += MBS_RIJ128;
         len  -= MBS_RIJ128;

      }
   }

   /*
   // encrypt last data block
   */
   if(len) {
      Ipp32u tmp[NB(128)];
      Ipp8u filler = (Ipp8u)( (padding==IppsCPPaddingPKCS7)? (MBS_RIJ128-len) : 0);

      FillBlock16(filler, pSrc, tmp, len);
      iv[0] ^= tmp[0];
      iv[1] ^= tmp[1];
      iv[2] ^= tmp[2];
      iv[3] ^= tmp[3];

      Encrypt_RIJ128(iv, tmp, RIJ_NR(pCtx), RIJ_EKEYS(pCtx));
      CopyBlock16(&tmp, pDst);
   }

   return ippStsNoErr;
}
