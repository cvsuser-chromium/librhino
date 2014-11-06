/*-----------------------------------------------------------------------
 * File: MD5.H
 *
 * Copyright (c) 1995-2000 Intel Corporation. All rights reserved.
 *-----------------------------------------------------------------------
 */

/* Module name: md5.h
   Data and private function declarations for MD5 in ICL.
   Tiny version v0.1
*/

#if !defined(INTEL_TINY_MD5_INCLUDE)
#define INTEL_TINY_MD5_INCLUDE  1

#define c1   7
#define c2  12
#define c3  17
#define c4  22
#define c5   5
#define c6   9
#define c7  14
#define c8  20
#define c9   4
#define c10 11
#define c11 16
#define c12 23
#define c13  6
#define c14 10
#define c15 15
#define c16 21

/* f1 , f2 , f3 , and f4 transformations for rounds 1 , 2 , 3 , and 4.
Rotation is separate from addition to prevent recomputation.
 */

#define f1(a , b , c , d , mem , shift , offset) { \
 (a) += (((b)&(c))|((~b)&(d))) + (mem) + (offset); \
 (a) = (((a)>> (32-(shift))) | ((a)<<(shift))) + (b); \
}

#define f2(a , b , c , d , mem , shift , offset) { \
 (a) += (((b)&(d))|((c)&(~d))) + (mem) + (offset); \
 (a) = (((a)>> (32-(shift))) | ((a)<<(shift))) + (b); \
}

#define f3(a , b , c , d , mem , shift , offset) { \
 (a) +=  ((b)^(c)^(d))+ (mem) + (offset); \
 (a) = (((a)>> (32-(shift))) | ((a)<<(shift))) + (b); \
}

#define f4(a , b , c , d , mem , shift , offset) { \
 (a) += ((c)^((b)|(~d)))+ (mem) + (offset); \
 (a) = (((a)>> (32-(shift))) | ((a)<<(shift))) + (b); \
}


/***************************************************************/
/* MD5 Message Digest declarations                             */
/***************************************************************/


#if defined(__cplusplus)
extern "C" {
#endif

void __stdcall MD5Transform (ICLWord *digest ,ICLWord *buffer);

#if defined(__cplusplus)
}
#endif


#endif      /* INTEL_TINY_MD5_INCLUDE */
