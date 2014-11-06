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
//    Decrypt byte data stream according to Rijndael (CBC mode)
//
// Contents:
//    ippsRijndael128DecryptCBC()
//
//
//    Created: Sat 25-May-2002 19:01
//  Author(s): Sergey Kirillov
//       mail: SergeyX_Kirillov@vniief.ims.intel.com
*/
#include "precomp.h"
#include "owncp.h"
#include "pcprij.h"
#include "pcpciphertool.h"


/*F*
//    Name: ippsRijndael128DecryptCBC
//
// Purpose: Decrypt byter data stream according to Rijndael in CBC mode.
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
//    ippStsPaddingErr        last decrypted block filler mismatch padding
//    ippStsNoErr             no errors
//
// Parameters:
//    pSrc        pointer to the source data stream
//    pDst        pointer to the target data stream
//    len         plaintest stream length (bytes)
//    pCtx        RIJ context
//    pIV         pointer to the initialization vector
//    padding     the padding scheme indicator
//
*F*/
IPPFUN(IppStatus, ippsRijndael128DecryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael128* pCtx,
                                             const Ipp8u* pIV,
                                             IppsCPPadding padding))
{
   Ipp32u tmpInp[NB(128)];
   Ipp32u tmpOut[NB(128)];
   Ipp32u     iv[NB(128)];
   Ipp32u  tmpIv[NB(128)];

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
   // decrypt block-by-block aligned streams
   */
   if( !(IPP_UINT_PTR(pSrc) & 0x3) && !(IPP_UINT_PTR(pDst) & 0x3)) {
      while(len >= MBS_RIJ128) {
         tmpIv[0] = ((Ipp32u*)pSrc)[0];
         tmpIv[1] = ((Ipp32u*)pSrc)[1];
         tmpIv[2] = ((Ipp32u*)pSrc)[2];
         tmpIv[3] = ((Ipp32u*)pSrc)[3];

         Decrypt_RIJ128((const Ipp32u*)pSrc, (Ipp32u*)pDst, RIJ_NR(pCtx), RIJ_DKEYS(pCtx));

         ((Ipp32u*)pDst)[0] ^= iv[0];
         ((Ipp32u*)pDst)[1] ^= iv[1];
         ((Ipp32u*)pDst)[2] ^= iv[2];
         ((Ipp32u*)pDst)[3] ^= iv[3];

         iv[0] = tmpIv[0];
         iv[1] = tmpIv[1];
         iv[2] = tmpIv[2];
         iv[3] = tmpIv[3];

         pSrc += MBS_RIJ128;
         pDst += MBS_RIJ128;
         len  -= MBS_RIJ128;
      }
   }

   /*
   // decrypt block-by-block misaligned streams
   */
   else {
      while(len >= MBS_RIJ128) {
         CopyBlock16(pSrc, tmpInp);

         Decrypt_RIJ128(tmpInp, tmpOut, RIJ_NR(pCtx), RIJ_DKEYS(pCtx));

         tmpOut[0] ^= iv[0];
         tmpOut[1] ^= iv[1];
         tmpOut[2] ^= iv[2];
         tmpOut[3] ^= iv[3];

         CopyBlock16(tmpOut, pDst);

         iv[0] = tmpInp[0];
         iv[1] = tmpInp[1];
         iv[2] = tmpInp[2];
         iv[3] = tmpInp[3];

         pSrc += MBS_RIJ128;
         pDst += MBS_RIJ128;
         len  -= MBS_RIJ128;
      }
   }

   /*
   // decrypt last data block
   */
   if(len) {
      Ipp8u filler = (Ipp8u)( (padding==IppsCPPaddingPKCS7)? (MBS_RIJ128-len) : 0);
      CopyBlock16(pSrc, tmpInp);
      Decrypt_RIJ128(tmpInp, tmpOut, RIJ_NR(pCtx), RIJ_DKEYS(pCtx));
      tmpOut[0] ^= iv[0];
      tmpOut[1] ^= iv[1];
      tmpOut[2] ^= iv[2];
      tmpOut[3] ^= iv[3];
      CopyBlock(tmpOut, pDst, len);
      if( !TestPadding(filler, ((Ipp8u*)&tmpOut)+len, MBS_RIJ128-len) )
         IPP_ERROR_RET(ippStsPaddingErr);
   }

   return ippStsNoErr;
}
