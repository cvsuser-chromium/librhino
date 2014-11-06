/*-----------------------------------------------------------------------
 * File: MODINV.C
 *
 * Copyright (c) 1995-2000 Intel Corporation. All rights reserved.
 *-----------------------------------------------------------------------
 */
/*
 * INTEL CONFIDENTIAL 
 * This file, software, or program is supplied under the terms of a 
 * license agreement or nondisclosure agreement with Intel Corporation 
 * and may not be copied or disclosed except in accordance with the 
 * terms of that agreement. This file, software, or program contains 
 * copyrighted material and/or trade secret information of Intel 
 * Corporation, and must be treated as such. Intel reserves all rights 
 * in this material, except as the license agreement or nondisclosure 
 * agreement specifically indicate. 
 */
/* 
 * WARNING: EXPORT RESTRICTED. 
 * This software listing contains cryptographic methods and technology. 
 * It is export restricted by the Office of Defense Trade Controls, United 
 * States Department of State and cannot be downloaded or otherwise 
 * exported or re-exported (i) into (or to a national or resident of) Cuba, 
 * Iraq, Libya, Yugoslavia, North Korea, Iran, Syria or any other country 
 * to which the US has embargoed goods; or (ii) to anyone on the US 
 * Treasury Department's list of Specially Designated Nationals or the US 
 * Commerce Department's Table of Denial Orders. By downloading or using 
 * this product, you are agreeing to the foregoing and you are representing 
 * and warranting that you are not located in, under the control of, or a 
 * national or resident of any such country or on any such list. 
 */



/* Module name: modinv.c
   Modular Inverse Computation.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"
#include "../../include/rsakg.h"
#include "../../include/iclproc.h"

/* the modular inverse of "Number" is computed where the modulus is "Modulus"
   The inverse is located in "inverse".
   This function returns 0 on success, -1 on failure to find the inverse.
   length of input and output is byte based.
*/
void __stdcall	ICL_ModularInverse (ICLData *Number,
			ICLData *Modulus, ICLData *Inverse)
{
  int		k;
  long		NumberLength, ModulusLength;
  RSAData	s, t, g, *inv;
  ICLWord	sarray[MODULUS], tarray[MODULUS], garray[MODULUS];

/* value initialization */
  s.value=sarray;
  t.value=tarray;
  g.value=garray;

  inv = (RSAData *)Inverse;     /* an alias pointer for Inverse */

/* adjust the lengths of inputs to reflect their word numbers */
  NumberLength = Number->length;		/* first, save their lengths */
  ModulusLength = Modulus->length;

  Number->length = ICL_WordCount (NumberLength);
  Modulus->length = ICL_WordCount (ModulusLength);
/* pad any leading byte(s) */
  ICL_PadBytes0 ((RSAData *)Number, NumberLength);
  ICL_PadBytes0 ((RSAData *)Modulus, ModulusLength);


/* run the Extended Euclidean Algorithm */
  ICL_EEA ((RSAData *)Modulus, (RSAData *)Number, &s, &t, &g, &k);

  if ((g.length==1) && (g.value[0]==1)) {	/* if GCD(.)==1 */
    if ((k & 1)==0) {   /* if number of iterations is even, make a subtract */
      ICL_MoveWord ((RSAData *)Modulus, inv);
      ICL_Subtract (inv, &t);	/* Inverse = Modulus - t */
    }
    else
      ICL_MoveWord (&t, inv);
  }
  else {	/* cannot find an inverse for this modulus, since GCD(.)!=1 */
    Inverse->length=1;	/* return 0 to indicate this */
    Inverse->value[0]=0;
  }
/* fill the higher words with 0's */
  for (k=inv->length; k<MODULUS; ++k)
    inv->value[k] = 0;
/* and find the actual length */
  k=inv->length;
  while (k>1 && inv->value[k-1]==0)
    --k;
  inv->length = k;

/* restore the lengths of the input variables */
  Number->length = NumberLength;
  Modulus->length = ModulusLength;
/* find the byte-length of the Inverse */
  Inverse->length = ICL_ByteCount (inv);
}
