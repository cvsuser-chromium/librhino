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
//    Cipher Tools
//
// Contents:
//    FillBlock8()   XorBlock8()
//    FillBlock16()  XorBlock16()
//    FillBlock24()  XorBlock24()
//    FillBlock32()  XorBlock32()
//                   XorBlock()
//
//    CopyBlock8()
//    CopyBlock16()
//    CopyBlock24()
//    CopyBlock32()
//    CopyBlock()
//
//    PaddBlock()
//    TestPadding()
//
//
//    Created: Mon 22-Jul-2002 17:08
//  Author(s): Sergey Kirillov
//       mail: SergeyX_Kirillov@vniief.ims.intel.com
*/
#include "precomp.h"
#include "owncp.h"
#include "pcpciphertool.h"


void FillBlock8(Ipp8u filler, const void* pSrc, void* pDst, int len)
{
   int n;
   for(n=0; n<len; n++) ((Ipp8u*)pDst)[n] = ((Ipp8u*)pSrc)[n];
   for(; n<8; n++) ((Ipp8u*)pDst)[n] = filler;
}

void FillBlock16(Ipp8u filler, const void* pSrc, void* pDst, int len)
{
   int n;
   for(n=0; n<len; n++) ((Ipp8u*)pDst)[n] = ((Ipp8u*)pSrc)[n];
   for(; n<16; n++) ((Ipp8u*)pDst)[n] = filler;
}

void FillBlock24(Ipp8u filler, const void* pSrc, void* pDst, int len)
{
   int n;
   for(n=0; n<len; n++) ((Ipp8u*)pDst)[n] = ((Ipp8u*)pSrc)[n];
   for(; n<24; n++) ((Ipp8u*)pDst)[n] = filler;
}

void FillBlock32(Ipp8u filler, const void* pSrc, void* pDst, int len)
{
   int n;
   for(n=0; n<len; n++) ((Ipp8u*)pDst)[n] = ((Ipp8u*)pSrc)[n];
   for(; n<32; n++) ((Ipp8u*)pDst)[n] = filler;
}


void XorBlock8(const void* pSrc1, const void* pSrc2, void* pDst)
{
   int n;
   for(n=0; n<8; n++)
      ((Ipp8u*)pDst)[n] = (Ipp8u)( ((Ipp8u*)pSrc1)[n]^((Ipp8u*)pSrc2)[n] );
}

void XorBlock16(const void* pSrc1, const void* pSrc2, void* pDst)
{
   int n;
   for(n=0; n<16; n++)
      ((Ipp8u*)pDst)[n] = (Ipp8u)( ((Ipp8u*)pSrc1)[n]^((Ipp8u*)pSrc2)[n] );
}
void XorBlock24(const void* pSrc1, const void* pSrc2, void* pDst)
{
   int n;
   for(n=0; n<24; n++)
      ((Ipp8u*)pDst)[n] = (Ipp8u)( ((Ipp8u*)pSrc1)[n]^((Ipp8u*)pSrc2)[n] );
}

void XorBlock32(const void* pSrc1, const void* pSrc2, void* pDst)
{
   int n;
   for(n=0; n<32; n++)
      ((Ipp8u*)pDst)[n] = (Ipp8u)( ((Ipp8u*)pSrc1)[n]^((Ipp8u*)pSrc2)[n] );
}

void XorBlock(const void* pSrc1, const void* pSrc2, void* pDst, int len)
{
   int n;
   for(n=0; n<len; n++)
      ((Ipp8u*)pDst)[n] = (Ipp8u)( ((Ipp8u*)pSrc1)[n]^((Ipp8u*)pSrc2)[n] );
}


void CopyBlock8(const void* pSrc, void* pDst)
{
   int n;
   for(n=0; n<8; n++) ((Ipp8u*)pDst)[n] = ((Ipp8u*)pSrc)[n];
}

void CopyBlock16(const void* pSrc, void* pDst)
{
   int n;
   for(n=0; n<16; n++) ((Ipp8u*)pDst)[n] = ((Ipp8u*)pSrc)[n];
}

void CopyBlock24(const void* pSrc, void* pDst)
{
   int n;
   for(n=0; n<24; n++) ((Ipp8u*)pDst)[n] = ((Ipp8u*)pSrc)[n];
}

void CopyBlock32(const void* pSrc, void* pDst)
{
   int n;
   for(n=0; n<32; n++) ((Ipp8u*)pDst)[n] = ((Ipp8u*)pSrc)[n];
}

void CopyBlock(const void* pSrc, void* pDst, int len)
{
   int n;
   for(n=0; n<len; n++) ((Ipp8u*)pDst)[n] = ((Ipp8u*)pSrc)[n];
}

void PaddBlock(Ipp8u filler, void* pDst, int len)
{
   int n;
   for(n=0; n<len; n++) ((Ipp8u*)pDst)[n] = filler;
}

int TestPadding(Ipp8u filler, void* pSrc, int len)
{
   int n;
   int x = 0;
   for(n=0; n<len; n++) x |= ((Ipp8u*)pSrc)[n];
   return x==filler;
}
