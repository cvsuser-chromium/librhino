/*-----------------------------------------------------------------------
 * File: ICLPROC.H
 *
 * Copyright (c) 1995-2000 Intel Corporation. All rights reserved.
 *-----------------------------------------------------------------------
 */

/* Module name: iclproc.h
   Header file for miscellenaous functions.
*/

#if !defined(ICL_PROC_INCLUDE)
#define ICL_PROC_INCLUDE	1

/* Number of words required to store, "ByteLength" bytes */
#define	ICL_WordCount(ByteLength)	( ((ByteLength)+sizeof(ICLWord)-1) / sizeof(ICLWord) )
#define ICL_IsOne(a)	( (a).length==1 && (a).value[0]==1 )
#define ICL_IsZero(m)	( (m).length==1 && (m).value[0]==0 )
#define ICL_IsEven(m)	( ((m).value[0] & 1)!=1 )

#ifdef __cplusplus
extern "C" {
#endif

void __stdcall	ICL_MoveWord
										(RSAData	*src,
					 					RSAData		*dest);

void __stdcall	ICL_MoveByte			(ICLData	*src,
										ICLData		*dest);

void __stdcall	ICL_MoveByte2Word		(ICLData	*src,
										RSAData		*dest);

void __stdcall	ICL_MoveWord2Byte		(RSAData	*src,
										ICLData		*dest);

void __stdcall	ICL_PadBytes0			(RSAData	*a,
										long		ByteLength);

int __stdcall	ICL_ByteCount			(RSAData	*a);

#ifdef __cplusplus
}
#endif


#endif	/* ICL_PROC_INCLUDE */
