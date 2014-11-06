/* //////////////////////////////// "owndefs.h" ////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 1999 Intel Corporation. All Rights Reserved.
//
//
//
//  Author(s): Alexey Korchuganov
//
//  Created: 27-Jul-1999 20:27
//
*/
#ifndef __OWNDEFS_H__
#define __OWNDEFS_H__

#include "ippdefs.h"

#if defined(__ICC) || defined(__ECC) || defined( __ICL ) || defined ( __ECL )
  #define __INLINE static __inline
#elif defined( __GNUC__ )
  #define __INLINE static __inline__
#else
  #define __INLINE static
#endif

#if defined(__ICC) || defined(__ECC) || defined( __ICL ) || defined ( __ECL )
 #define __RESTRICT restrict
#else
 #define __RESTRICT
#endif


#if defined( IPP_W32DLL )
  #if defined( _MSC_VER ) || defined( __ICL ) || defined ( __ECL )
    #define IPPFUN(type,name,arg) __declspec(dllexport) type __STDCALL name arg
  #else
    #define IPPFUN(type,name,arg)                extern type __STDCALL name arg
  #endif
#else
  #define   IPPFUN(type,name,arg)                extern type __STDCALL name arg
#endif

#if !defined ( _IA64) && (defined ( _WIN64 ) || defined( linux64 ))
#define _IA64
#endif


#define _IPP_PX 0
#define _IPP_M6 1
#define _IPP_A6 2
#define _IPP_W7 4
#define _IPP_T7 8

#define _IPP_S1 11
#define _IPP_S2 22
#define _IPP_C2 23

#define _IPP64_PX  _IPP_PX
#define _IPP64_I7 64

#if (defined( __ICL ) || defined ( __ECL )) && (defined( _WIN32 ) && !defined( _WIN64 ))
__INLINE
Ipp32s IPP_INT_PTR( const void* ptr )  {
    union {
        void*   Ptr;
        Ipp32s  Int;
    } dd;
    dd.Ptr = (void*)ptr;
    return dd.Int;
}

__INLINE
Ipp32u IPP_UINT_PTR( const void* ptr )  {
    union {
        void*   Ptr;
        Ipp32u  Int;
    } dd;
    dd.Ptr = (void*)ptr;
    return dd.Int;
}

#elif (defined( __ICL ) || defined ( __ECL )) && defined( _WIN64 )
__INLINE
Ipp64s IPP_INT_PTR( const void* ptr )  {
    union {
        void*   Ptr;
        Ipp64s  Int;
    } dd;
    dd.Ptr = (void*)ptr;
    return dd.Int;
}

__INLINE
Ipp64u IPP_UINT_PTR( const void* ptr )  {
    union {
        void*    Ptr;
        Ipp64u   Int;
    } dd;
    dd.Ptr = (void*)ptr;
    return dd.Int;
}

#elif defined( __ICC ) && defined( linux32 )
__INLINE
Ipp32s IPP_INT_PTR( const void* ptr )  {
    union {
        void*   Ptr;
        Ipp32s  Int;
    } dd;
    dd.Ptr = (void*)ptr;
    return dd.Int;
}

__INLINE
Ipp32u IPP_UINT_PTR( const void* ptr )  {
    union {
        void*   Ptr;
        Ipp32u  Int;
    } dd;
    dd.Ptr = (void*)ptr;
    return dd.Int;
}

#elif defined ( __ECC ) && defined( linux64 )
__INLINE
Ipp64s IPP_INT_PTR( const void* ptr )  {
    union {
        void*   Ptr;
        Ipp64s  Int;
    } dd;
    dd.Ptr = (void*)ptr;
    return dd.Int;
}

__INLINE
Ipp64u IPP_UINT_PTR( const void* ptr )  {
    union {
        void*    Ptr;
        Ipp64u   Int;
    } dd;
    dd.Ptr = (void*)ptr;
    return dd.Int;
}

#else
  #define IPP_INT_PTR( ptr )  ( (long)(ptr) )
  #define IPP_UINT_PTR( ptr ) ( (unsigned long)(ptr) )
#endif


#define IPP_ALIGN_TYPE(type, align) ((align)/sizeof(type)-1)
#define IPP_BYTES_TO_ALIGN(ptr, align) ((-(IPP_INT_PTR(ptr)&((align)-1)))&((align)-1))
#define IPP_ALIGNED_PTR(ptr, align) (void*)( (unsigned char*)(ptr) + (IPP_BYTES_TO_ALIGN( ptr, align )) )

#define IPP_MALLOC_ALIGNED_BYTES  32


#define IPP_MALLOC_ALIGNED_8BYTES   8
#define IPP_MALLOC_ALIGNED_16BYTES 16
#define IPP_MALLOC_ALIGNED_32BYTES 32

#define IPP_ALIGNED_ARRAY(align,arrtype,arrname,arrlength)\
 arrtype arrname##AlignedArrBuff[arrlength+IPP_ALIGN_TYPE(arrtype, align)];\
 arrtype *arrname = (arrtype*)IPP_ALIGNED_PTR(arrname##AlignedArrBuff,align)


#if defined(__ICC) || defined(__ECC) || defined( __ICL ) || defined ( __ECL )
    #define __ALIGN16 __declspec (align(16))
    #define __ALIGN32 __declspec (align(32))
#else
    #define __ALIGN16 
    #define __ALIGN32 
#endif


#if defined ( _M6 )
  #define _IPP   _IPP_M6
  #define _IPP64 _IPP64_PX

#elif defined( _A6 )
  #define _IPP   _IPP_A6
  #define _IPP64 _IPP64_PX

#elif defined( _W7 )
  #define _IPP   _IPP_W7
  #define _IPP64 _IPP64_PX

#elif defined( _T7 )
  #define _IPP   _IPP_T7
  #define _IPP64 _IPP64_PX
#elif defined( _I7 )
  #define _IPP   _IPP_PX
  #define _IPP64 _IPP64_I7

#elif defined( _StrongARM )
  #define _IPP   _IPP_S1
  #define _IPP64 _IPP64_PX

#elif defined( _XScale )
  #define _IPP   _IPP_S2
  #define _IPP64 _IPP64_PX

#elif defined( _XScale_Concan )
  #define _IPP   _IPP_C2
  #define _IPP64 _IPP64_PX

#else
  #define _IPP   _IPP_PX
  #define _IPP64 _IPP64_PX
#endif

#if defined( __cplusplus )
extern "C" {
#endif

/* /////////////////////////////////////////////////////////////////////////////
           IPP Context Identification
  /////////////////////////////////////////////////////////////////////////// */
typedef enum {
    idCtxUnknown = 0,
    idCtxFFT_C_16sc,
    idCtxFFT_C_16s,
    idCtxFFT_R_16s,
    idCtxFFT_C_32fc,
    idCtxFFT_C_32f,
    idCtxFFT_R_32f,
    idCtxFFT_C_64fc,
    idCtxFFT_C_64f,
    idCtxFFT_R_64f,
    idCtxDFT_C_16sc,
    idCtxDFT_C_16s,
    idCtxDFT_R_16s,
    idCtxDFT_C_32fc,
    idCtxDFT_C_32f,
    idCtxDFT_R_32f,
    idCtxDFT_C_64fc,
    idCtxDFT_C_64f,
    idCtxDFT_R_64f,
    idCtxDCTFwd_16s,
    idCtxDCTInv_16s,
    idCtxDCTFwd_32f,
    idCtxDCTInv_32f,
    idCtxDCTFwd_64f,
    idCtxDCTInv_64f,
    idCtxFFT2D_C_32fc,
    idCtxFFT2D_R_32f,
    idCtxDFT2D_C_32fc,
    idCtxDFT2D_R_32f,
    idCtxFFT2D_R_32s,
    idCtxDFT2D_R_32s,
    idCtxDCT2DFwd_32f,
    idCtxDCT2DInv_32f,
    idCtxMoment64f,
    idCtxMoment64s,
    idCtxRandUni_8u,
    idCtxRandUni_16s,
    idCtxRandUni_32f,
    idCtxRandGauss_8u,
    idCtxRandGauss_16s,
    idCtxRandGauss_32f,
    idCtxWTFwd_32f,
    idCtxWTFwd_8u32f,
    idCtxWTFwd_8s32f,
    idCtxWTFwd_16u32f,
    idCtxWTFwd_16s32f,
    idCtxWTFwd2D_32f_C1R,
    idCtxWTInv2D_32f_C1R,
    idCtxWTFwd2D_32f_C3R,
    idCtxWTInv2D_32f_C3R,
    idCtxWTInv_32f,
    idCtxWTInv_32f8u,
    idCtxWTInv_32f8s,
    idCtxWTInv_32f16u,
    idCtxWTInv_32f16s,
    idCtxMDCTFwd_32f,
    idCtxMDCTInv_32f,
    idCtxFIRBlock_32f,
    idCtxFDP_32f,
#if 0    
    idCtxRLMS_32f       = 'LMS1',
    idCtxRLMS32f_16s    = 'LMS' ,
    idCtxIIRAR_32f      = 'II01',
    idCtxIIRBQ_32f      = 'II02',
    idCtxIIRAR_32fc     = 'II03',
    idCtxIIRBQ_32fc     = 'II04',
    idCtxIIRAR32f_16s   = 'II05',
    idCtxIIRBQ32f_16s   = 'II06',
    idCtxIIRAR32fc_16sc = 'II07',
    idCtxIIRBQ32fc_16sc = 'II08',
    idCtxIIRAR32s_16s   = 'II09',
    idCtxIIRBQ32s_16s   = 'II10',
    idCtxIIRAR32sc_16sc = 'II11',
    idCtxIIRBQ32sc_16sc = 'II12',
    idCtxIIRAR_64f      = 'II13',
    idCtxIIRBQ_64f      = 'II14',
    idCtxIIRAR_64fc     = 'II15',
    idCtxIIRBQ_64fc     = 'II16',
    idCtxIIRAR64f_32f   = 'II17',
    idCtxIIRBQ64f_32f   = 'II18',
    idCtxIIRAR64fc_32fc = 'II19',
    idCtxIIRBQ64fc_32fc = 'II20',
    idCtxIIRAR64f_32s   = 'II21',
    idCtxIIRBQ64f_32s   = 'II22',
    idCtxIIRAR64fc_32sc = 'II23',
    idCtxIIRBQ64fc_32sc = 'II24',
    idCtxIIRAR64f_16s   = 'II25',
    idCtxIIRBQ64f_16s   = 'II26',
    idCtxIIRAR64fc_16sc = 'II27',
    idCtxIIRBQ64fc_16sc = 'II28',
    idCtxFIRSR_32f      = 'FI01',
    idCtxFIRSR_32fc     = 'FI02',
    idCtxFIRMR_32f      = 'FI03',
    idCtxFIRMR_32fc     = 'FI04',
    idCtxFIRSR32f_16s   = 'FI05',
    idCtxFIRSR32fc_16sc = 'FI06',
    idCtxFIRMR32f_16s   = 'FI07',
    idCtxFIRMR32fc_16sc = 'FI08',
    idCtxFIRSR32s_16s   = 'FI09',
    idCtxFIRSR32sc_16sc = 'FI10',
    idCtxFIRMR32s_16s   = 'FI11',
    idCtxFIRMR32sc_16sc = 'FI12',
    idCtxFIRSR_64f      = 'FI13',
    idCtxFIRSR_64fc     = 'FI14',
    idCtxFIRMR_64f      = 'FI15',
    idCtxFIRMR_64fc     = 'FI16',
    idCtxFIRSR64f_32f   = 'FI17',
    idCtxFIRSR64fc_32fc = 'FI18',
    idCtxFIRMR64f_32f   = 'FI19',
    idCtxFIRMR64fc_32fc = 'FI20',
    idCtxFIRSR64f_32s   = 'FI21',
    idCtxFIRSR64fc_32sc = 'FI22',
    idCtxFIRMR64f_32s   = 'FI23',
    idCtxFIRMR64fc_32sc = 'FI24',
    idCtxFIRSR64f_16s   = 'FI25',
    idCtxFIRSR64fc_16sc = 'FI26',
    idCtxFIRMR64f_16s   = 'FI27',
    idCtxFIRMR64fc_16sc = 'FI28',
    idCtxRLMS32s_16s    = 'LMSR',
    idCtxCLMS32s_16s    = 'LMSC',
#endif    
    idCtxEncode_JPEG2K,
    idCtxDES,
    idCtxBlowfish,
    idCtxRijndael,
    idCtxTwofish,
    idCtxSHA1,
    idCtxSHA256,
    idCtxSHA384,
    idCtxSHA512,
    idCtxMD5,
    idCtxHMAC,
    idCtxDAA,
    idCtxBigNum,
    idCtxMontgomery,
    idCtxPrimeNumber,
    idCtxPRNG,
    idCtxRSA,
    idCtxDSA,
    idCtxRFFT2_8u,
    idCtxHilbert_32f32fc,
    idCtxHilbert_16s32fc,
    idCtxHilbert_16s16sc,
    idCtxTone_16s,
    idCtxTriangle_16s,
    idCtxDFTOutOrd_C_32fc,
    idCtxDFTOutOrd_C_64fc
} IppCtxId;


/* /////////////////////////////////////////////////////////////////////////////
           Helpers
  /////////////////////////////////////////////////////////////////////////// */

#define IPP_NOERROR_RET()  return ippStsNoErr
#define IPP_ERROR_RET( ErrCode )  return (ErrCode)

#ifdef _IPP_DEBUG

    #define IPP_BADARG_RET( expr, ErrCode )\
                {if (expr) { IPP_ERROR_RET( ErrCode ); }}

#else

    #define IPP_BADARG_RET( expr, ErrCode )

#endif


    #define IPP_BAD_SIZE_RET( n )\
                IPP_BADARG_RET( (n)<=0, ippStsSizeErr )

    #define IPP_BAD_STEP_RET( n )\
                IPP_BADARG_RET( (n)<=0, ippStsStepErr )

    #define IPP_BAD_PTR1_RET( ptr )\
                IPP_BADARG_RET( NULL==(ptr), ippStsNullPtrErr )

    #define IPP_BAD_PTR2_RET( ptr1, ptr2 )\
                IPP_BADARG_RET(((NULL==(ptr1))||(NULL==(ptr2))), ippStsNullPtrErr)

    #define IPP_BAD_PTR3_RET( ptr1, ptr2, ptr3 )\
                IPP_BADARG_RET(((NULL==(ptr1))||(NULL==(ptr2))||(NULL==(ptr3))),\
                                                         ippStsNullPtrErr)

    #define IPP_BAD_PTR4_RET( ptr1, ptr2, ptr3, ptr4 )\
                {IPP_BAD_PTR2_RET( ptr1, ptr2 ); IPP_BAD_PTR2_RET( ptr3, ptr4 )}


/* ////////////////////////////////////////////////////////////////////////// */

typedef union { /* double precision */
    Ipp64s  hex;
    Ipp64f   fp;
} IppFP_64f;

typedef union { /* single precision */
    Ipp32s  hex;
    Ipp32f   fp;
} IppFP_32f;


extern const IppFP_32f ippConstantOfNAN_32f;
extern const IppFP_64f ippConstantOfNAN_64f;

extern const IppFP_32f ippConstantOfINF_32f;
extern const IppFP_64f ippConstantOfINF_64f;
extern const IppFP_32f ippConstantOfINF_NEG_32f;
extern const IppFP_64f ippConstantOfINF_NEG_64f;

#define NAN_32F      (ippConstantOfNAN_32f.fp)
#define NAN_64F      (ippConstantOfNAN_64f.fp)
#define INF_32F      (ippConstantOfINF_32f.fp)
#define INF_64F      (ippConstantOfINF_64f.fp)
#define INF_NEG_32F  (ippConstantOfINF_NEG_32f.fp)
#define INF_NEG_64F  (ippConstantOfINF_NEG_64f.fp)

/* Define NULL pointer value */
#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#define UNREFERENCED_PARAMETER(p) (p)=(p)

#if defined( _IPP_MARK_LIBRARY )
static char G[] = "IPPGenuineóÁÒÏ×";
#endif


#define STR(x)           #x
#define STR2(x)       STR(x)
#define MESSAGE( desc )\
     message(__FILE__ "(" STR2(__LINE__) "):" #desc)



#if defined( __cplusplus )
}
#endif

#endif /* __OWNDEFS_H__ */
/* //////////////////////// End of file "owndefs.h" ///////////////////////// */
