/*
//               INTeL CORPORATION PROPRIETARY INFORMATION
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Intel Corporation and may not be copied
// or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2002 Intel Corporation. All Rights Reserved.
//
//
// Purpose:
//    Cryptography Primitive.
//    Internal Definitions and
//    Internal Rijndael based Encrypt/Decrypt Function Prototypes
//
//    Created: Mon 20-May-2002 11:06
//  Author(s): Sergey Kirillov
//       mail: SergeyX_Kirillov@vniief.ims.intel.com
//
*/
#if !defined(_PCP_RIJ_H)
#define _PCP_RIJ_H


/*
// The GF(256) modular polynomial and elements
*/
#define WPOLY  0x011B
#define BPOLY    0x1B

/*
// Make WORD using 4 arbitrary bytes
*/
#define BYTES_TO_WORD(b0,b1,b2,b3) ( ( ((Ipp32u)((Ipp8u)(b3))) <<24 ) \
                                    |( ((Ipp32u)((Ipp8u)(b2))) <<16 ) \
                                    |( ((Ipp32u)((Ipp8u)(b1))) << 8 ) \
                                    |( ((Ipp32u)((Ipp8u)(b0))) ) )
/*
// Make WORD setting byte in specified position
*/
#define BYTE0_TO_WORD(b)   BYTES_TO_WORD((b), 0,  0,  0)
#define BYTE1_TO_WORD(b)   BYTES_TO_WORD( 0, (b), 0,  0)
#define BYTE2_TO_WORD(b)   BYTES_TO_WORD( 0,  0, (b), 0)
#define BYTE3_TO_WORD(b)   BYTES_TO_WORD( 0,  0,  0, (b))

/*
// Extract byte from specified position n.
// Sure, n=0,1,2 or 3 only
*/
#define EBYTE(w,n) ((Ipp8u)((w) >> (8 * (n))))


/*
// Rijndael spec structure
*/
#if 0
struct _ippcpRijndael {
   IppCtxId    idCtx;      /* Rijndael spec identifier      */
   int         nk;         /* security key length (words)   */
   int         nb;         /* data block size (words)       */
   int         nr;         /* number of rounds              */
   Ipp32u*     enc_keys;   /* array of keys for encryprion  */
   Ipp32u*     dec_keys;   /* array of keys for decryprion  */
};                         /* actual keys are just below    */
#endif

/*
// Rijndael128, Rijndael192 and Rijndael256
// reserve space for maximum number of expanded keys
*/
struct _ippcpRijndael128 {
   IppCtxId    idCtx;         /* Rijndael spec identifier      */
   int         nk;            /* security key length (words)   */
   int         nb;            /* data block size (words)       */
   int         nr;            /* number of rounds              */
   Ipp32u      enc_keys[64];  /* array of keys for encryprion  */
   Ipp32u      dec_keys[64];  /* array of keys for decryprion  */
};

struct _ippcpRijndael192 {
   IppCtxId    idCtx;         /* Rijndael spec identifier      */
   int         nk;            /* security key length (words)   */
   int         nb;            /* data block size (words)       */
   int         nr;            /* number of rounds              */
   Ipp32u      enc_keys[96];  /* array of keys for encryprion  */
   Ipp32u      dec_keys[96];  /* array of keys for decryprion  */
};

struct _ippcpRijndael256 {
   IppCtxId    idCtx;         /* Rijndael spec identifier      */
   int         nk;            /* security key length (words)   */
   int         nb;            /* data block size (words)       */
   int         nr;            /* number of rounds              */
   Ipp32u      enc_keys[120]; /* array of keys for encryprion  */
   Ipp32u      dec_keys[120]; /* array of keys for decryprion  */
};

#define MBS_RIJ128   (128/8)  /* message block size (bytes) */
#define MBS_RIJ192   (192/8)
#define MBS_RIJ256   (256/8)

#define SR          (4)            /* number of rows in STATE data */

#define NB(msgBlks) ((msgBlks)/32) /* message block size (words)     */
                                   /* 4-word for 128-bits data block */
                                   /* 6-word for 192-bits data block */
                                   /* 8-word for 256-bits data block */

#define NK(keybits) ((keybits)/32)  /* key length (words): */
#define NK128 NK(IppsRijndaelKey128)/* 4-word for 128-bits security key */
#define NK192 NK(IppsRijndaelKey192)/* 6-word for 192-bits security key */
#define NK256 NK(IppsRijndaelKey256)/* 8-word for 256-bits security key */

#define NR128_128 (10)  /* number of rounds data: 128 bits key: 128 bits are used */
#define NR128_192 (12)  /* number of rounds data: 128 bits key: 192 bits are used */
#define NR128_256 (14)  /* number of rounds data: 128 bits key: 256 bits are used */
#define NR192_128 (12)  /* number of rounds data: 192 bits key: 128 bits are used */
#define NR192_192 (12)  /* number of rounds data: 192 bits key: 192 bits are used */
#define NR192_256 (14)  /* number of rounds data: 192 bits key: 256 bits are used */
#define NR256_128 (14)  /* number of rounds data: 256 bits key: 128 bits are used */
#define NR256_192 (14)  /* number of rounds data: 256 bits key: 192 bits are used */
#define NR256_256 (14)  /* number of rounds data: 256 bits key: 256 bits are used */

/*
// Useful macros
*/
#define RIJ_ID(ctx)        ((ctx)->idCtx)
#define RIJ_NB(ctx)        ((ctx)->nb)
#define RIJ_NK(ctx)        ((ctx)->nk)
#define RIJ_NR(ctx)        ((ctx)->nr)
#define RIJ_EKEYS(ctx)     ((ctx)->enc_keys)
#define RIJ_DKEYS(ctx)     ((ctx)->dec_keys)
#define RIJ_ID_TEST(ctx)   (RIJ_ID((ctx))==idCtxRijndael)

/*
// Internal functions
*/
void Encrypt_RIJ128(const Ipp32u* pInpBlk, Ipp32u* pOutBlk, int nr, const Ipp32u* pKeys);
void Encrypt_RIJ192(const Ipp32u* pInpBlk, Ipp32u* pOutBlk, int nr, const Ipp32u* pKeys);
void Encrypt_RIJ256(const Ipp32u* pInpBlk, Ipp32u* pOutBlk, int nr, const Ipp32u* pKeys);

void Decrypt_RIJ128(const Ipp32u* pInpBlk, Ipp32u* pOutBlk, int nr, const Ipp32u* pKeys);
void Decrypt_RIJ192(const Ipp32u* pInpBlk, Ipp32u* pOutBlk, int nr, const Ipp32u* pKeys);
void Decrypt_RIJ256(const Ipp32u* pInpBlk, Ipp32u* pOutBlk, int nr, const Ipp32u* pKeys);

void ExpandRijndaelKey(const Ipp8u* pKey, int NK, int NB, int NR, int nKeys,
                       Ipp32u* enc_keys, Ipp32u* dec_keys);

#endif /* _PCP_RIJ_H */
