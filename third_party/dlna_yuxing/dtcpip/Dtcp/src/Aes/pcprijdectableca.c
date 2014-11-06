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
//    Decrypt tables for Rijndael
//
// Contents:
//    RijDecTbl[4][256]
//
//
//    Created: Tue 23-May-2002 10:35
//  Author(s): Sergey Kirillov
//       mail: SergeyX_Kirillov@vniief.ims.intel.com
*/
#include "precomp.h"
#include "owncp.h"
#include "pcprij.h"
#include "pcprijtables.h"


/*
// Reference to pcrijencryptpxca.c
// for details
*/

/* Decryprion Tables */
const Ipp32u RijDecTbl[4][256] = {
   { DEC_SBOX(inv_t0) },
   { DEC_SBOX(inv_t1) },
   { DEC_SBOX(inv_t2) },
   { DEC_SBOX(inv_t3) }
};

