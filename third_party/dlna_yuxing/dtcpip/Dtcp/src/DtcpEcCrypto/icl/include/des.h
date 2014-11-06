/*-----------------------------------------------------------------------
 * File: DES.H
 *
 * Copyright (c) 1995-2000 Intel Corporation. All rights reserved.
 *-----------------------------------------------------------------------
 */

#ifdef ICL_DES
/***************************************************************************/

/*
   DES.H
   Type, constants, and macro declarations for DES in ICL.

   Includes ECB and CBC modes.

   Bren Sessions, Feb. 1996
*/

#if !defined(INTEL_DES_INCLUDE)
#define INTEL_DES_INCLUDE  1

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include "desmacro.h"

#define DES_ROUNDS 16

typedef struct {     /* Two 32bit words together as a 64-bit quanity (Left,Right) */
  ICLWord Left, Right;
} ICLDoubleWord;     /* 4+4 = 8 bytes */

extern ICLByte KeyBits[DES_ROUNDS*64];
extern ICLWord SBoxPerm[64*8];

void /*__stdcall*/ ICL_DESInitialPerm(ICLWord *left, ICLWord *right);
void /*__stdcall*/ ICL_DESFinalPerm(ICLWord *left, ICLWord *right);


#endif

/***************************************************************************/
#endif