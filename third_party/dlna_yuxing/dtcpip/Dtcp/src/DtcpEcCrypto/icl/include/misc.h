/*-----------------------------------------------------------------------
 * File: MISC.H
 *
 * Copyright (c) 1995-2000 Intel Corporation. All rights reserved.
 *-----------------------------------------------------------------------
 */

// returns 2^p
#define ICLPower2(p) ((ICLWord)1<<(p))

// bitwise rotation to the left
#define rotl(x,n)   (((x)>>(32 - (n))) | ((x) << (n)))

// bitwise rotation to the right
#define rotr(x,n)   (((x)<<(32 - (n))) | ((x) >> (n)))

// rotation to the right, size specific, with masking
#define Srotr(x,n,bits)   (((x) << ((bits) - ((n)&((bits)-1))) ) | ((x) >> ((n)&((bits)-1))))

// rotation to the left, size specific, with masking
#define Srotl(x,n,bits)   (((x) >> ((bits) - ((n)&((bits)-1))) ) | ((x) << ((n)&((bits)-1))))

// translates little endian <----> big endian
#define bswap(y)   ((rotr(y, 8) & 0xff00ff00) |  \
                   (rotl(y, 8) & 0x00ff00ff))
   

// prototypes for misc.c in ICLProc directory

void __stdcall ICL_memoryset(char *addr, char value, long length);
void __stdcall ICL_memorycopy(char *dest, char *src, long length);
