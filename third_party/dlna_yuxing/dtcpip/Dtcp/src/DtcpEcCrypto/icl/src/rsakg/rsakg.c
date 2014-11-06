/*-----------------------------------------------------------------------
 * File: RSAKG.C
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



/* Module name: rsakg.c
   RSA Key Generation Routine
*/

#include "../../include/icl.h"
#include "../../include/rsa.h"
#include "../../include/rsakg.h"
#include "../../include/iclproc.h"

/* Generates RSA Public and Private Keys
   Note that input length is in bytes, but the internal operations are performed
   using the ICLWord length, i.e., using the RSAData structure as in RSA schemes.
*/
int ICL_RSAKeyGenerate
        (ICLData           *RandomP,				/* random number for prime[0]	*/
        ICLData            *RandomQ,				/* random number for prime[1]	*/
        ICLData            *PublicExpCandidate,		/* random number for public key	*/
        ICLRSAPublicKey    *RSAPublicKey,			/* Generated RSA public key		*/
        ICLRSAPrivateKey   *RSAPrivateKey)			/* generated RSA private key	*/
{
  long		cnt;
  ICLWord	carry, bitmask;
  RSAData	p, q, pm1, qm1, ExpCandidate;
/* length of the pm1array is one more than MODULUS, since it's also used in
   modulus computation as n=p*q and the modulus may be longer than MODULUS
*/
  ICLWord	parray[MODULUS], qarray[MODULUS],
  			pm1array[MODULUS+1], qm1array[MODULUS],
  			exparray[MODULUS];

/* set the local variable spaces -- memory reservation and initialization */
  p.value = parray;
  q.value = qarray;
  pm1.value = pm1array;
  qm1.value = qm1array;
  ExpCandidate.value = exparray;

/* initial bitmask is used to strip off the MSB, and set bit next to MSB */
  bitmask = (ICLWord)1 << (WORDSIZE-1);

/* find prime couples p and q until n=p*q < 2^MODULUSBITS */
  cnt = 1;
  do {

/* Generate Prime[0]=p and Prime[1]=q */
	ICL_MoveByte2Word (RandomP, &p);	/* p= RandomP */
	ICL_MoveByte2Word (RandomQ, &q);	/* q= RandomQ */

/* jam the LSB to 1, bitmask'th bit to 0, (bitmask-1)st bit to 1. */
	parray[0] |= (ICLWord)1;
	parray[p.length-1] &= ~bitmask;
	parray[p.length-1] |= bitmask >> 1;
	qarray[0] |= (ICLWord)1;
	qarray[p.length-1] &= ~bitmask;
	qarray[p.length-1] |= bitmask >> 1;

/* Find a prime for Prime[0] and Prime[1] */
	if (ICL_FindPrime (&p)!=0)	return -1;		/* if cannot find a prime */
	if (ICL_FindPrime (&q)!=0)	return -1;

/* Modulus -- n is computed here and held in pm1 */
	ICL_Mul (&p, &q, &pm1);

/* check the length of the modulus. if it's longer than MODULUS
   then compute another p,q pair
*/
   cnt = (pm1.length<=MODULUS);

/* adjust the bitmask to pint to the next lower bit */
	bitmask >>=1;

  } while (!cnt && bitmask!=0);

  if (!cnt)			/* still cannot find primes? -- this must be a bug!! */
  	return -1;		/* if this happens, please report */

/* two primes generated and the modulus is computed.
   copy the generated modulus to key structures
*/
  ICL_MoveWord2Byte (&pm1, &RSAPrivateKey->Modulus);
  ICL_MoveByte (&RSAPrivateKey->Modulus, &RSAPublicKey->Modulus);

/* copy p to Prime[0] and q to Prime[1] */
  ICL_MoveWord2Byte (&p, &RSAPrivateKey->Prime[0]);
  ICL_MoveWord2Byte (&q, &RSAPrivateKey->Prime[1]);

/* 1 is subtracted from both of these primes and they are located
   in pm1 and qm1. The multiplication of pm1 and qm1 is done and
   located in p. The multiplication of p and q will give n, the modulus.
   This will be used to generate public and private exponents.
*/
  ICL_MoveWord (&p, &pm1);
  ICL_MoveWord (&q, &qm1);

/* compute p-1 */
	carry = (pm1array[0] - 1UL) > pm1array[0];
	pm1array[0] -= 1UL;
	for (cnt=1; carry!=0 && cnt<MODULUS; ++cnt) {
		carry = (pm1array[cnt] - 1UL) > pm1array[cnt];
		pm1array[cnt] -= 1UL;
	}
	pm1.length -= pm1array[pm1.length-1]==0;
/* compute q-1 */
	carry = (qm1array[0] - 1UL) > qm1array[0];
	qm1array[0] -= 1UL;
	for (cnt=1; carry!=0 && cnt<MODULUS; ++cnt) {
		carry = (qm1array[cnt] - 1UL) > qm1array[cnt];
		qm1array[cnt] -= 1UL;
	}
	qm1.length -= qm1array[qm1.length-1]==0;

/* compute p = (p-1)*(q-1) which is Phi(Modulus) */
  ICL_Mul (&pm1, &qm1, &p);

/* Public Exponent -- e */
  ICL_MoveByte2Word (PublicExpCandidate, &ExpCandidate);
  ICL_FindPublicExponent (&ExpCandidate, &p);

/* the public exponent is in "ExpCandidate" now, copy it to its places */
  ICL_MoveWord2Byte (&ExpCandidate, &RSAPrivateKey->PublicExponent);
  ICL_MoveByte (&RSAPrivateKey->PublicExponent, &RSAPublicKey->PublicExponent);

/* Private Exponent -- d = e^(-1) mod (p-1)(q-1)*/
/* Note that, p contains Phi(n) = (p-1)*(q-1) */
  while (p.length>1 && p.value[p.length-1]==0)
      --p.length;
  p.length = ICL_ByteCount (&p);		/* since inverse operates on byte-counts */
  ICL_ModularInverse (&RSAPublicKey->PublicExponent, (ICLData *)&p,
  	&RSAPrivateKey->PrivateExponent);

  ICL_MoveByte2Word (&RSAPrivateKey->PrivateExponent, &q); /* q = d */
/* Prime Exponent[0] = d mod (p-1) */
  ICL_Rem (&q, &pm1, &ExpCandidate);
  ICL_MoveWord2Byte (&ExpCandidate, &RSAPrivateKey->PrimeExponent[0]);

/* Prime Exponent[1] = d mod (q-1) */
  ICL_Rem (&q, &qm1, &ExpCandidate);
  ICL_MoveWord2Byte (&ExpCandidate, &RSAPrivateKey->PrimeExponent[1]);

/* Coefficient -- coeff = q^{-1} mod p */
  ICL_ModularInverse (&RSAPrivateKey->Prime[1],
  		&RSAPrivateKey->Prime[0], &RSAPrivateKey->Coefficient);

  return 0;
}
