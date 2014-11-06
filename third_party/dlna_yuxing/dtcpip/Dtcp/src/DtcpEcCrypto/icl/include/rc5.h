/*-----------------------------------------------------------------------
 * File: RC5.H
 *
 * Copyright (c) 1995-2000 Intel Corporation. All rights reserved.
 *-----------------------------------------------------------------------
 */

#ifdef ICL_RC5
/***************************************************************************/

#if !defined(INTEL_RC5_INCLUDE)
#define INTEL_RC5_INCLUDE  1

typedef unsigned short int ICLShort;   // 16 bit data
typedef unsigned __int64 ICLDWord;     // 64 bit data

#define ICL_RC5_P16  0xb7e1
#define ICL_RC5_Q16  0x9e37
#define ICL_RC5_P32  0xb7e15163
#define ICL_RC5_Q32  0x9e3779b9
#define ICL_RC5_P64  0xb7e151628aed2a6b
#define ICL_RC5_Q64  0x9e3779b97f4a7c15

#define ICL_RC5_P64hi 0xb7e15162
#define ICL_RC5_P64lo 0x8aed2a6b
#define ICL_RC5_Q64hi 0x9e3779b9
#define ICL_RC5_Q64lo 0x7f4a7c15

#define ICL_RC5_ENCRYPT  1
#define ICL_RC5_DECRYPT  2

#define ICL_RC5_ECB  16
#define ICL_RC5_CBC  32

#define ICL_ZERO_PAD 256
#define ICL_PKCS_PAD 512

#endif     /* INTEL_RC5_INCLUDE */

/***************************************************************************/
#endif
