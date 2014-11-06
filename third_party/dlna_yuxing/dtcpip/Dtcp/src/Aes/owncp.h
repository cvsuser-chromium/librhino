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
//  Purpose: 
//
//  Author(s): Alexey Korchuganov
//
//  Created: 2-Aug-1999 15:40
//
//  Some addition for ippCP especial by Sergey Kirillov
//  mail: SergeyX_Kirillov@vniief.ims.intel.com
*/
#ifndef __OWNCP_H__
#define __OWNCP_H__

#ifndef __OWNDEFS_H__
  #include "owndefs.h"
#endif

#ifndef __IPPCP_H__
  #include "ippcp.h"
#endif


#ifdef _WIN32_WCE
  #pragma warning( disable : 4206 4710 4115)  
#endif

/*
// Common ippCP Macros
*/

/* WORD and DWORD manipulators */
#define LODWORD(x)    ((Ipp32u)(x))
#define HIDWORD(x)    ((Ipp32u)(((Ipp64u)(x) >>32) & 0xFFFFFFFF))

#define MAKEHWORD(bLo,bHi) ((Ipp16u)(((Ipp8u)(bLo))  | ((Ipp16u)((Ipp8u)(bHi))) << 8))
#define MAKEWORD(hLo,hHi)  ((Ipp32u)(((Ipp16u)(hLo)) | ((Ipp32u)((Ipp16u)(hHi))) << 16))
#define MAKEDWORD(wLo,wHi) ((Ipp64u)(((Ipp32u)(wLo)) | ((Ipp64u)((Ipp32u)(wHi))) << 32))

/* Logical Shifts (right and left) of WORD */
#define LSR32(x,nBits)  ((x)>>(nBits))
#define LSL32(x,nBits)  ((x)<<(nBits))

/* Rorate (right and left) of WORD */
#if defined(_MSC_VER)
#  define ROR32(x, nBits)  _lrotr((x),(nBits))
#  define ROL32(x, nBits)  _lrotl((x),(nBits))
#else
#  define ROR32(x, nBits) (LSR32((x),(nBits)) | LSL32((x),32-(nBits)))
#  define ROL32(x, nBits) ROR32((x),(32-(nBits)))
#endif

/* Logical Shifts (right and left) of DWORD */
#define LSR64(x,nBits)  ((x)>>(nBits))
#define LSL64(x,nBits)  ((x)<<(nBits))

/* Rorate (right and left) of DWORD */
#define ROR64(x, nBits) (LSR64((x),(nBits)) | LSL64((x),64-(nBits)))
#define ROL64(x, nBits) ROR64((x),(64-(nBits)))

#define ENDIANNESS(x) ((ROR32((x), 24) & 0x00ff00ff) | (ROR32((x), 8) & 0xff00ff00))

#if defined(__ICL) || defined(__ECL)
#define IPP_DECLARE_PTR(TYPE,NAME)\
    __declspec(align(8))Ipp8u __buffer##NAME[sizeof(TYPE)];\
    TYPE * NAME = (TYPE *)(__buffer##NAME)
#else
#define IPP_DECLARE_PTR(TYPE,NAME)\
    Ipp8u __buffer##NAME[sizeof(TYPE)+8];\
    TYPE * NAME = (TYPE *)IPP_ALIGNED_PTR(__buffer##NAME,8)
#endif /*__ICL __ECL*/

#define IPP_DECLARE_PTR2(TYPE,NAME1,NAME2)\
    IPP_DECLARE_PTR(TYPE,NAME1);\
    IPP_DECLARE_PTR(TYPE,NAME2)

#define IPP_DECLARE_PTR3(TYPE,NAME1,NAME2,NAME3)\
    IPP_DECLARE_PTR2(TYPE,NAME1,NAME2);\
    IPP_DECLARE_PTR(TYPE,NAME3)

#define IPP_MAKE_MULTIPLE_OF_8(x) ((x) = ((x)+7)&(~7))
#define IPP_MAKE_MULTIPLE_OF_16(x) ((x) = ((x)+15)&(~15))

#endif /* __OWNCP_H__ */
/* ////////////////////////// End of file "owncp.h" ////////////////////////// */
