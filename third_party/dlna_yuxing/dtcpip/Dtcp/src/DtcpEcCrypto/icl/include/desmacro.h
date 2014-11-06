/*-----------------------------------------------------------------------
 * File: DESMACRO.H
 *
 * Copyright (c) 1995-2000 Intel Corporation. All rights reserved.
 *-----------------------------------------------------------------------
 */

#ifdef ICL_DES
/***************************************************************************/

/* Module name: desmacro.h

   Contains #define macros to perform core DES algorithm
*/

#if !defined(INTEL_DESMACRO_INCLUDE)
#define INTEL_DESMACRO_INCLUDE	1

// Code for the Initial and Final Permutations takes advantage of the 
// non-random nature of the transformations.  They accomplish the
// permutations with fewer instructions than would be required for 
// a brute force approach.

#define ICL_DESinitialperm(left,right,tmp,state,key)    \
                                \
__asm bswap left \
__asm bswap right \
\
__asm rol right,4				\
__asm mov tmp,right				\
__asm xor tmp,left				\
__asm push esi					\
__asm and tmp,0f0f0f0f0h		\
__asm push eax                  \
__asm xor left,tmp				\
								\
__asm xor right,tmp				\
__asm push edi                  \
__asm ror right,20				\
__asm mov tmp,left				\
__asm xor tmp,right				\
__asm and tmp,0ffff0000h		\
__asm xor left,tmp				\
 								\
__asm xor right,tmp				\
__asm ror right,18				\
__asm mov tmp,left				\
__asm xor tmp,right				\
__asm and tmp,033333333h		\
__asm xor left,tmp				\
 								\
__asm xor right,tmp				\
__asm ror right,6				\
__asm mov tmp,left				\
__asm xor tmp,right				\
__asm mov eax,dword ptr [state+key*8]            /* load initial round key (lo word), the rest are pipelined */ \
__asm and tmp,00ff00ffh			\
__asm xor left,tmp				\
 								\
__asm xor right,tmp				\
__asm rol right,9				\
__asm mov tmp,left				\
__asm xor tmp,right				\
__asm and tmp,0aaaaaaaah		\
__asm xor left,tmp				\
__asm xor right,tmp  			\
__asm mov ebx,dword ptr [state+key*8+4]          /* load initial round key (hi word), the rest are pipelined */ \
__asm ror right,1				

/* ----------------------------------------------------------------------- */

/*
   ICL DES Final permutation code
*/

#define ICL_DESfinalperm(left,right,tmp)    \
\
__asm rol left,1				\
\
__asm mov tmp,left				\
__asm xor tmp,right				\
__asm pop edi                   \
__asm and tmp,0aaaaaaaah		\
__asm xor right,tmp				\
\
__asm xor left,tmp				\
__asm ror left,9				\
__asm mov tmp,left				\
__asm xor tmp,right				\
__asm pop eax                   \
__asm and tmp,00ff00ffh			\
__asm xor right,tmp				\
\
__asm xor left,tmp				\
__asm rol left,6				\
__asm mov tmp,left				\
__asm xor tmp,right				\
__asm pop esi                   \
__asm and tmp,33333333h			\
__asm xor right,tmp				\
\
__asm xor left,tmp				\
__asm rol left,18	    		\
__asm mov tmp,left				\
__asm xor tmp,right				\
__asm add edi,8                 \
__asm and tmp,0ffff0000h		\
__asm xor right,tmp				\
\
__asm xor left,tmp				\
__asm rol left,20	    		\
__asm mov tmp,left				\
__asm xor tmp,right				\
__asm add esi,8                 \
__asm and tmp,0f0f0f0f0h		\
__asm xor right,tmp				\
\
__asm xor left,tmp				\
__asm ror left,4	    		\
\
__asm bswap edx	\
__asm bswap ecx 


/*
DES assembler MACRO
this is the 32bit assembler version of the single round des function.

This function computes a round based on 'left' & 'right' registers and 
then preloads the next round key into eax and ebx
*/

/* ----------------------------------------------------------------------- */
#define ICL_DESround1(left,right,key) \
\
__asm  xor eax,right                            /* ready upper bits for xor 2 */ \
__asm  xor ebx,right                            /* */ \
\
__asm  mov edi,eax                              /* make copy for shifting */ \
__asm  mov esi,ebx                              /* make copy for shifting */ \
__asm  ror edi,25                               /* shift index into LSBits */ \
__asm  shr esi,21                               /* shift index into LSBits */ \
__asm  and edi,0FCH                             /* mask off index */ \
__asm  and esi,0FCH                             /* mask off index */ \
__asm  xor left,dword ptr [SBoxPerm+edi+0]      /* lookup sbox 1 */ \
\
__asm  nop \
__asm  mov edi,eax                              /* make copy for shifting */ \
__asm  xor left,dword ptr [SBoxPerm+esi+256]    /* lookup sbox 2 */ \
__asm  mov esi,ebx                              /* make copy for shifting */ \
__asm  shr edi,17                               /* shift index into LSBits */ \
__asm  shr esi,13                               /* shift index into LSBits */ \
__asm  and edi,0fcH                             /* mask off index */ \
__asm  and esi,0fcH                             /* mask off index */ \
__asm  xor left,dword ptr [SBoxPerm+edi+512]    /* lookup sbox 3 */ \
\
__asm  nop \
__asm  mov edi,eax                              /* make copy for shifting */ \
__asm  xor left,dword ptr [SBoxPerm+esi+768]    /* lookup sbox 4 */ \
__asm  mov esi,ebx                              /* make copy for shifting */ \
__asm  shr edi,9                                /* shift index into LSBits */ \
__asm  shr esi,5                                /* shift index into LSBits */ \
__asm  and edi,0fcH                             /* mask off index */ \
__asm  and esi,0fcH                             /* mask off index */ \
__asm  shr eax,1                                /* shift index into LSBits */ \
__asm  xor left,dword ptr [SBoxPerm+edi+1024]   /* lookup sbox 5 */ \
\
__asm  mov edi,dword ptr [ebp+key*8] \
__asm  and ebx,08000001FH                       /* mask off index XXX */ \
__asm  xor left,dword ptr [SBoxPerm+esi+1280]   /* lookup sbox 6 */ \
__asm  and eax,0fcH                             /* mask off index */ \
__asm  rol ebx,3                                /* shift index into LSBits */ \
__asm  xor left,dword ptr [SBoxPerm+eax+1536]   /* lookup sbox 7 YYY */ \
__asm  mov eax,edi                              /* get next roundkey (left) */ \
__asm  \
__asm  xor left,dword ptr [SBoxPerm+ebx+1792]   /* lookup sbox 8 */ \
__asm  mov ebx,dword ptr [ebp+key*8+4]          /* get next roundkey (right) */ 

/* ----------------------------------------------------------------------- */

#define ICL_DESround2(a,b)  \
ICL_DESround1(ecx,edx,a)    \
ICL_DESround1(edx,ecx,b)	\

/* ----------------------------------------------------------------------- */
/* ICL_DESencryption(): takes [ecx,edx] and produces output [ecx,edx] */

#define ICL_DESencryption()  \
ICL_DESinitialperm(ecx,edx,ebx,ebp,0)  /* initial perm & byte swapping + preload initial round key */	 \
ICL_DESround2( 1, 2)     				/* rounds  1&2,   */ \
ICL_DESround2( 3, 4)	 				/* rounds  3&4,   */ \
ICL_DESround2( 5, 6)	 				/* rounds  5&6,   */ \
ICL_DESround2( 7, 8)	 				/* rounds  7&8,   */ \
ICL_DESround2( 9,10)	 				/* rounds  9&10,  */ \
ICL_DESround2(11,12)	 				/* rounds 11&12,  */ \
ICL_DESround2(13,14)	 				/* rounds 13&14   */ \
ICL_DESround2(15, 0)	 				/* rounds 15&16   */ \
ICL_DESfinalperm(ecx,edx,ebx)			/* final perm & byte swapping */	 

/* ----------------------------------------------------------------------- */
/* ICL_DESdecryption(): takes [ecx,edx] and produces output [edx,ecx] */

#define ICL_DESdecryption()  \
ICL_DESinitialperm(ecx,edx,ebx,ebp,15) /* initial perm & byte swapping + preload initial round key */	 \
ICL_DESround2(14,13)     				/* rounds  1&2,   */ \
ICL_DESround2(12,11)	 				/* rounds  3&4,   */ \
ICL_DESround2(10, 9)	 				/* rounds  5&6,   */ \
ICL_DESround2( 8, 7)	 				/* rounds  7&8,   */ \
ICL_DESround2( 6, 5)	 				/* rounds  9&10,  */ \
ICL_DESround2( 4, 3)	 				/* rounds 11&12,  */ \
ICL_DESround2( 2, 1)	 				/* rounds 13&14   */ \
ICL_DESround2( 0, 0)	 				/* rounds 15&16   */ \
ICL_DESfinalperm(ecx,edx,ebx)			/* final perm & byte swapping */	 

#endif

/***************************************************************************/
#endif