/*-----------------------------------------------------------------------
 * File: RSAKG.H
 *
 * Copyright (c) 1995-2000 Intel Corporation. All rights reserved.
 *-----------------------------------------------------------------------
 */

/* Module name: rsakg.h
   Header file for RSA Key Generation routines and variables
*/

#if !defined(ICL_RSAKG_INCLUDE)
#define ICL_RSAKG_INCLUDE	1


/* Maximum number of bases to use in weak primality test */
#define	MAXNUMBEROFBASES                64

/* The small prime list to be used in trial division step */
extern ICLWord	PrimeList[];

#ifdef __cplusplus
extern "C" {
#endif

int __stdcall	ICL_TrialDivision
					(RSAData	*PseudoPrime);

int __stdcall	ICL_WeakPseudoPrimality
					(RSAData	*PseudoPrime,
 					 int		Count);

void __stdcall	ICL_NextOdd
					(RSAData	*Odd);

void __stdcall	ICL_ModularInverse
					(ICLData	*Number,
					ICLData		*Modulus,
					ICLData		*Inverse);

void __stdcall	ICL_EEA
					(RSAData	*a,
 					RSAData		*b,
				 	RSAData		*vpp,
 					RSAData		*upp,
 					RSAData		*rpp,
 					int			*k);

void __stdcall	ICL_Div
					(RSAData	*Dividend,
					RSAData		*Divisor,
					RSAData		*Divide);

void __stdcall	ICL_GCD
					(RSAData	*a,
					RSAData		*b,
					RSAData		*gcd);

int __stdcall	ICL_FindPrime
					(RSAData	*PseudoPrime);

void __stdcall	ICL_FindPublicExponent
					(RSAData	*PublicExpCandidate,
					RSAData		*PhiN);

int __stdcall	RSA_Remainder
					(RSAData	*a,
					ICLWord     b);

#ifdef __cplusplus
}
#endif

#endif

