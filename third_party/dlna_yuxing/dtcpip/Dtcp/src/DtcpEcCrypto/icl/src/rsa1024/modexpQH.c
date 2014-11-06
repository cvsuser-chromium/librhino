/*-----------------------------------------------------------------------
 * File: MODEXPQH.C
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



/* Module name: modexpQH.c
   Modular Exponentiation using Montgomery product.
   This version can be compiled using either Binary, Quaternary or
   Hexadecimal method for exponentiation.
   The method selection is done by changind the EXP_BITS constant
   defined below. The correct values are:
   1		Binary Method
   2		Quaternary Method
   4		Hexadecimal Method

   The higher words from RSAData.length to MODULUS are assumed
   to be filled with 0's before this function is called.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"

#define	EXP_BITS	4
#define	EXP_POWERTABLESIZE	( (ICLWord)2 << (EXP_BITS-1) )


void __stdcall ICL_ModExpBQH (RSAData *a, RSAData *e, RSAData *n, RSAData *t)
{
	ICLWord	MIarray[2*MODULUS+2], MOarray[MODULUS+3];
	long		i, j, swap_length;
	ICLWord	mask, eval, *aval, *evalue, *swapptr;
	RSAData	Montgomery_Input, Montgomery_Output;
	int		shift, Index;
	RSAInt	PowerTable[EXP_POWERTABLESIZE];

	RSAData	pt;
	ICLWord ModExpN0prime;
	RSAData ModExpModulus;


	ModExpN0prime = N0Prime (n->value[0]);
	ModExpModulus.length = n->length;
	ModExpModulus.value = n->value;

	Montgomery_Input.value = MIarray;
	Montgomery_Output.value = MOarray;

/* the initial t_bar value:  MI = R mod n */
	Montgomery_Input.length=ModExpModulus.length + 1;
	for (j=ModExpModulus.length-1; j>=0; --j)
		MIarray[j] = 0;
	MIarray[ModExpModulus.length] = 1;
/* the very first member of the power table is R mod n */
	RSA_Rem (&Montgomery_Input, n, &PowerTable[0]);

/* compute a*R mod n */
	for (j=0; j<ModExpModulus.length; j++)        /* tmp = a*R */
		MIarray[j]=0;
	aval = a->value;
	for (i=0, j=ModExpModulus.length; i<a->length; i++, j++)
		MIarray[j]=aval[i];
	Montgomery_Input.length=a->length + ModExpModulus.length;
/* the second mandatory entry is R*a mod n */
	RSA_Rem (&Montgomery_Input, n, &PowerTable[1]);

/* fill the other entries of the power table by R*a^i mod n */
	for (i=2; i<EXP_POWERTABLESIZE; ++i) {
		pt.value = PowerTable[i-1].value;
		pt.length = PowerTable[i-1].length;
/* PT[i] = (A*R mod n) * PT[i-1] * R mod n */
		MontProduct (ModExpN0prime, ModExpModulus, &pt, &PowerTable[1], &Montgomery_Input);
		aval = PowerTable[i].value;
		for (j=0; j<MODULUS; ++j)
			aval[j] = MIarray[j];
		PowerTable[i].length = Montgomery_Input.length;
	}

/* * * * * *  The Exponentiation Loop  * * * * * */
  evalue = e->value;
  eval = evalue[e->length-1];
/* the initial value for t_bar to start the loop */
  Index = (eval & (~(ICLWord)0 << (WORDSIZE-EXP_BITS))) >> (WORDSIZE-EXP_BITS);
  aval = PowerTable[Index].value;
  for (i=0; i<Montgomery_Input.length; ++i)
  	MIarray[i] = aval[i];
  Montgomery_Input.length = PowerTable[Index].length;
  Montgomery_Output.length = ModExpModulus.length;

/* well done, start looping on each bit of exponent */
	shift = WORDSIZE - 2*EXP_BITS;
	mask = (~(ICLWord)0 << (WORDSIZE-EXP_BITS)) >> EXP_BITS;

	for (i=e->length-1; i>=0; --i) {
		eval = evalue[i];
		for (; mask!=0; mask>>=EXP_BITS, shift-=EXP_BITS) {
			for (j=0; j<EXP_BITS; ++j) {
				MontSquare (ModExpN0prime, ModExpModulus, 
					&Montgomery_Input, &Montgomery_Output);
/* swap (Montgomery_Input, Montgomery_Output) */
				swapptr = Montgomery_Input.value;
				Montgomery_Input.value = Montgomery_Output.value;
				Montgomery_Output.value = swapptr;
				swap_length = Montgomery_Input.length;
				Montgomery_Input.length = Montgomery_Output.length;
				Montgomery_Output.length = swap_length;
			}

			Index = (eval & mask) >> shift;
			if (Index!=0) {
				MontProduct (ModExpN0prime, ModExpModulus,
					&Montgomery_Input, &PowerTable[Index], &Montgomery_Output);
/* swap (Montgomery_Input, Montgomery_Output) */
				swapptr = Montgomery_Input.value;
				Montgomery_Input.value = Montgomery_Output.value;
				Montgomery_Output.value = swapptr;
				swap_length = Montgomery_Input.length;
				Montgomery_Input.length = Montgomery_Output.length;
				Montgomery_Output.length = swap_length;
			}
		}
		shift = WORDSIZE - EXP_BITS;
		mask = ~(ICLWord)0 << (WORDSIZE - EXP_BITS);
	}


/* Obtain natural representation of the Montgomery residue result */
	for (j=MODULUS-1; j>=1; --j)
		PowerTable[0].value[j]=0;
    PowerTable[0].value[0] = 1;
	PowerTable[0].length = 1;
	MontProduct (ModExpN0prime, ModExpModulus, 
		&Montgomery_Input, &PowerTable[0], &Montgomery_Output);

/* discard any leading zero words */

    j = i = Montgomery_Output.length;
    aval = Montgomery_Output.value;
    while (i>0 && aval[i-1]==0)
        --i;
    Montgomery_Output.length = i;

/* copy the result to t */
	for (i=j-1; i>=0; --i)
		t->value[i] = Montgomery_Output.value[i];
	t->length = Montgomery_Output.length;
}
