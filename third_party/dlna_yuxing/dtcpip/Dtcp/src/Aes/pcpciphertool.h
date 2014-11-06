/*
//               INTeL CORPORATION PROPRIETARY INFORMATION
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Intel Corporation and may not be copied
// or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2002 Intel Corporation. All Rights Reserved.
//
//
// Purpose:
//    Cryptography Primitive.
//    Internal Definitions of Block Cipher Tools
//
//
//    Created: Sat 16-Feb-2002 14:48
//  Author(s): Sergey Kirillov
//       mail: SergeyX_Kirillov@vniief.ims.intel.com
//
*/
#if !defined(_PCP_CIPHERTOOL_H)
#define _PCP_CIPHERTOOL_H

void FillBlock8 (Ipp8u filler, const void* pSrc, void* pDst, int len);
void FillBlock16(Ipp8u filler, const void* pSrc, void* pDst, int len);
void FillBlock24(Ipp8u filler, const void* pSrc, void* pDst, int len);
void FillBlock32(Ipp8u filler, const void* pSrc, void* pDst, int len);

void XorBlock8 (const void* pSrc1, const void* pSrc2, void* pDst);
void XorBlock16(const void* pSrc1, const void* pSrc2, void* pDst);
void XorBlock24(const void* pSrc1, const void* pSrc2, void* pDst);
void XorBlock32(const void* pSrc1, const void* pSrc2, void* pDst);
void XorBlock  (const void* pSrc1, const void* pSrc2, void* pDst, int len);

void CopyBlock8 (const void* pSrc, void* pDst);
void CopyBlock16(const void* pSrc, void* pDst);
void CopyBlock24(const void* pSrc, void* pDst);
void CopyBlock32(const void* pSrc, void* pDst);

void CopyBlock(const void* pSrc, void* pDst, int len);

void PaddBlock(Ipp8u filler, void* pDst, int len);

int  TestPadding(Ipp8u filler, void* pSrc, int len);

#endif /* _PCP_CIPHERTOOL_H */
