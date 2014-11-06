/*-----------------------------------------------------------------------
 * File: MODEXP.C
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



/* Module name: modexp.c
   Modular Exponentiation using Montgomery product.

   The higher words from RSAData.length to MODULUS are assumed
   to be filled with 0's before this function is called.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"

void __stdcall ICL_ModExp (RSAData *a, RSAData *e, RSAData *n, RSAData *t)
{
  RSAInt	a_bar;
  RSAData	tmp, t_bar;
  ICLWord	tmparray[2*MODULUS+2], tbararray[MODULUS+3];
  long		i, j;
  ICLWord	mask, eval, *aval, *evalue;
  RSAData	*Montgomery_Input, *Montgomery_Output, *swapptr;
  /* reserve space for former global variables */
  ICLWord	ModExpN0prime;
  RSAData	ModExpModulus;


  ModExpN0prime = N0Prime (n->value[0]);
  ModExpModulus.length = n->length;
  ModExpModulus.value = n->value;

  tmp.value = tmparray;
  t_bar.value = tbararray;

/* the initial t_bar value:  t_bar = R mod n */
  tmp.length=ModExpModulus.length + 1;
  for (j=ModExpModulus.length-1; j>=0; --j)
    tmparray[j] = 0;
  tmparray[ModExpModulus.length] = 1;
  ICL_Rem (&tmp, n, &t_bar);

/* compute a*R mod n */
  for (j=0; j<ModExpModulus.length; j++)        /* tmp = a*R */
    tmparray[j]=0;
  aval = a->value;
  for (i=0, j=ModExpModulus.length; i<a->length; i++, j++)
    tmparray[j]=aval[i];
  tmp.length=a->length + ModExpModulus.length;
  RSA_Rem (&tmp, n, &a_bar);           /* a_bar = a*R mod n */

/* * * * * *  The Exponentiation Loop  * * * * * */
  mask = 0x80000000UL;
  evalue = e->value;
/* by-pass any leading zeros in the exponent, start from MSB of exponent */
  eval = evalue[e->length-1];
  while ((mask!=0) && (eval & mask)==0)
    mask>>=1;
/* well done, start looping on each bit of exponent */
  Montgomery_Input = &t_bar;
  Montgomery_Output = &tmp;
  evalue  = e->value;

  for (i=e->length-1; i>=0; --i) {
    eval = evalue[i];
      for (; mask!=0; mask>>=1) {
		MontSquare (ModExpN0prime, ModExpModulus, Montgomery_Input, Montgomery_Output);
      if ((eval & mask)!=0) {
		MontProduct (ModExpN0prime, ModExpModulus, 
			Montgomery_Output, &a_bar, Montgomery_Input);
		continue;
      }
	  swapptr = Montgomery_Input;
	  Montgomery_Input = Montgomery_Output;
	  Montgomery_Output = swapptr;
    }
    mask=0x80000000;
  }

/* Obtain natural representation of the Montgomery residue result */
	for (j=MODULUS-1; j>=1; --j)
		a_bar.value[j]=0;
    a_bar.value[0] = 1;
	a_bar.length = 1;
	MontProduct (ModExpN0prime, ModExpModulus, Montgomery_Input, &a_bar, Montgomery_Output);

/* discard any leading zero words */
    i = MODULUS;
    aval = Montgomery_Output->value;
    while (i>0 && aval[i-1]==0)
        --i;
    Montgomery_Output->length = i;

/* copy the result to t */
	for (i=Montgomery_Output->length-1; i>=0; --i)
		t->value[i] = Montgomery_Output->value[i];
	t->length = Montgomery_Output->length;
}
