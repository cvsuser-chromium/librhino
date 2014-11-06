//#############################################################################
//## Copyright (c) 2004 Intel Corporation All Rights Reserved. 
//## 
//## The source code contained or described herein and all documents related to
//## the source code ("Material") are owned by Intel Corporation or its 
//## suppliers or licensors. Title to the Material remains with Intel 
//## Corporation or its suppliers and licensors. The Material contains trade 
//## secrets and proprietary and confidential information of Intel or its
//## suppliers and licensors. The Material is protected by worldwide copyright
//## and trade secret laws and treaty provisions. No part of the Material may 
//## be used, copied, reproduced, modified, published, uploaded, posted, 
//## transmitted, distributed, or disclosed in any way without Intel's prior 
//## express written permission.
//## 
//## No license under any patent, copyright, trade secret or other 
//## intellectual property right is granted to or conferred upon you by 
//## disclosure or delivery of the Materials, either expressly, by 
//## implication, inducement, estoppel or otherwise. Any license under such 
//## intellectual property rights must be express and approved by Intel in 
//## writing.
//#############################################################################

#ifndef DtcpEcCrypto___included

#include "DtcpEcCryptoTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup DtcpEcCrypto DtcpEcCrypto
/// \brief Elliptic curve library
/// @{

#ifndef WWbasics
#define WWbasics
typedef unsigned char W8;
typedef unsigned short W16;
typedef unsigned int W32;
#endif


/****************************************************************************
 * Definitions for ECC library functions:
 */

extern void
 InitializeEccParams
  (
   EccParams *eccParams,/*  OUT: ECC parameters  */
   W8 * workingMemory,      /*  IN:  ptr 25 32-bit-word (800 bits)  */
   W8 * eccP,                /*  IN:  EC prime field > 3 (160 bits)  */
   W8 * eccA,                /*  IN:  EC curve coefficient A (160 bits)  */
   W8 * eccB,                /*  IN:  EC curve coefficient B (160 bits)  */
   W8 * eccBaseX,            /*  IN:  ECC base point X-coord (160 bits)  */
   W8 * eccBaseY,            /*  IN:  ECC base point Y-coord (160 bits)  */
   W8 * eccR                 /*  IN:  ECC base point order (160 bits)  */
  );  /*  note:  eccP, eccA, eccBase*, and eccR params all big endian  */

extern void
 SignData
  (
   W8 * pSignature,          /*  OUT: signature (320 bits)  */
   W8 * pBuffer,             /*  IN:  buffer of data to sign  */
   W32 bufferLengthBytes,    /*  IN:  length of buffer in bytes  */
   W8 * signersKey,         /*  IN:  signer's key (160 bits)  */
   EccParams *eccParams /*  IN:  ECC parameters  */
  );

extern int
 VerifyData
  (
   W8 * pSignature,          /*  IN:  signature (320 bits)  */
   W8 * pBuffer,             /*  IN:  buffer of data to verify  */
   W32 bufferLengthBytes,    /*  IN:  length of buffer in bytes  */
   W8 * verifyKey,           /*  IN:  verify key (320 bits)  */
   EccParams *eccParams /*  IN:  ECC parameters  */
  );  /*  RETURNS:  1 if "valid"; 0 if "invalid" signature  */

//------
extern void
 GetFirstPhaseValue
  (
   W8 * pXv,                 /*  OUT: Diffie first phase value (320 bits)  */
   W8 * pXk,                 /*  OUT: secret information (160 bits)  */
   EccParams *eccParams /*  IN:  ECC parameters  */
  );

extern void
 GetSharedSecret
  (
   W8 * pKauth,              /*  OUT: Kauth (96 bits, little endian)  */
   W8 * pYv,                 /*  IN:  Diffie first phase value (320 bits)  */
   W8 * pXk,                 /*  IN:  secret information (160 bits)  */
   EccParams *eccParams /*  IN:  ECC parameters  */
  );
//----
/***************
 * SHA1 and RNG support functions
 */
/* Calculate f= SHA1(pBuffer).  Use msb same as size of eccR. */
extern void
 SHA1
  (
   W8 *f,                     /*  OUT: SHA digest (160bit) */
   W8 *pBuffer,               /*  IN:  Buffer to hash */
   W32 bufferLengthBytes      /*  IN:  Bytes in buffer  */
  );
/* Generate random r value in range (0, b).  Warning, parameter meaning may change */
extern void
 RNGf 
 (
  W8 *r,                      /*  OUT: Random value (160bit) */
  W8 *b                       /*  In: top of rnadom range + 1 (160bit) */
  );

/// @}

# ifdef __cplusplus
}
# endif

#define DtcpEcCrypto___included

#endif  /*  NDEF ECC___included  */

