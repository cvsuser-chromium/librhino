/*-----------------------------------------------------------------------
 * File: RSA.H
 *
 * Copyright (c) 1995-2000 Intel Corporation. All rights reserved.
 *-----------------------------------------------------------------------
 */

/* Module name: rsa.h
   Data and private function declarations for RSA.
*/

#ifndef _ICL_RSA_INCLUDE_
#define _ICL_RSA_INCLUDE_


/***************************************************************/
/* RSA Encryption/Decryption declarations                      */
/***************************************************************/

#ifdef  _MSC_VER
#define DWORDSIZE    64
typedef unsigned __int64 ICLDWord;
#endif

typedef struct {
	long	length;				/* number of ICLWords */
	ICLWord value[MODULUS];
} RSAInt;

typedef struct {
	long	length;
	ICLWord	*value;				/* number of words */
} RSAData;



#if defined(_MSC_VER)
#define LowWord(x)		( (ICLWord)(x) )
#define HighWord(x)		( (ICLWord)( ((x) >> WORDSIZE) & 0xffffffff ) )
#endif

#define WORDMASK ( ~(ICLWord)0 )

#ifdef __cplusplus
extern "C" {
#endif


void     __stdcall ICL_ModExp     (RSAData *a, RSAData *e, RSAData *n, RSAData *t);
void     __stdcall ICL_ModExpCRT  (RSAData *C, RSAData *dp, RSAData *dq,
						 			RSAData *p, RSAData *q, RSAData *coeff, RSAData *t);
void	__stdcall  ICL_ModExpBQH  (RSAData *a, RSAData *e, RSAData *n, RSAData *t);

ICLWord __stdcall N0Prime        (ICLWord n0);
void __stdcall    ICL_Add        (RSAData *a, RSAData *b, RSAData *t);
void __stdcall    ICL_Mul        (RSAData *a, RSAData *b, RSAData *t);
void __stdcall    ICL_Square     (RSAData *a, RSAData *t);
void __stdcall    ICL_ModMul     (RSAData *a, RSAData *b, RSAData *n, RSAData *t);

int  __stdcall    ICL_Compare    (RSAData *a, RSAData *b);
void __stdcall    ICL_ModAdd     (RSAData *a, RSAData *b, RSAData *n, RSAData *t);
void __stdcall    ICL_ModSub     (RSAData *a, RSAData *b, RSAData *n, RSAData *t);
void __stdcall    MontProduct    (ICLWord ModExpN0prime, RSAData ModExpModulus, 
								  RSAData *a, RSAInt *b, RSAData *t);
void __stdcall    MontSquare     (ICLWord ModExpN0prime, RSAData ModExpModulus,
								  RSAData *a, RSAData *t);
void __stdcall    ICL_Rem        (RSAData *a, RSAData *b, RSAData *r);
void __stdcall    RSA_Rem        (RSAData *a, RSAData *b, RSAInt *r);
void __stdcall    ICL_Subtract   (RSAData *t, RSAData *a);
void __stdcall    ICL_Padding0   (RSAData *a);

#ifdef __cplusplus
}
#endif


#endif      /* _ICL_RSA_INCLUDE_ */

