/*-----------------------------------------------------------------------
 * File: PUBEXP.C
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



/* Module name: pubexp.c
   Public exponent generator for RSA Key generation program.
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"
#include "../../include/rsakg.h"
#include "../../include/iclproc.h"

/* Find a public exponent for the given Phi(n).
   The initial public exponent is given in "PublicExpCandidate".
   It's forced to be an odd number and relativcely prime to Phin
   The generated exponent will be located in "PublicExpCandidate".
*/
void __stdcall ICL_FindPublicExponent (RSAData *PublicExpCandidate, RSAData *PhiN)
{
	RSAData		gcd;
	ICLWord		garray[MODULUS];
	int			isOne;

	gcd.value = garray;
/* force the initial candidate to be an odd number */
	PublicExpCandidate->value[0] |= 1;

	for (isOne=0; !isOne; ) {
/* find their GCD, it must be 1 */
		ICL_GCD (PublicExpCandidate, PhiN, &gcd);
		isOne = ICL_IsOne (gcd);
/* if they are not relatively prime, try the next odd number */
		if (!isOne)
			ICL_NextOdd (PublicExpCandidate);
	}
/* the generated exponent is in PublicExpCandidate */
}
