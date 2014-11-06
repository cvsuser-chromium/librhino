/*-----------------------------------------------------------------------
 * File: ICL.H
 *
 * Copyright (c) 1995-2000 Intel Corporation. All rights reserved.
 *-----------------------------------------------------------------------
 */

/*
   Module name: icl.h
   Intel Cryptographic Library, Version v1.0

   This Intel Cryptographic Library contains:
   * 1024 bit RSA Encryption and Decryption
   * 1024 bit RSA Key Generation
   * MD5 Message Digest Algorithm
   * SHA Message Digest Algorithm
   * Data Encryption Standard in ECB/CBC mode
   * Password Based Encryption
   * Random Number Generator
   * RC4 with variable key length
   * RC5 16/32/64 with variable #rounds, variable key length in ECB/CBC modes
*/

#ifndef _ICL_INCLUDE_
#define _ICL_INCLUDE_

#define WORDSIZE				32
#define	MODULUSBITS				1024
#define MODULUS					(MODULUSBITS / WORDSIZE)
#define MODULUSBYTES			(MODULUSBITS / 8)

/* Maximum number of characters processed in the passphrase of PBE */
#define MAXPASSPHRASELENGTH		32

/* Method identifier for PBE */
enum {PBE_MD5DES=1, PBE_MD5IDEA, PBE_SHADES, PBE_SHAIDEA};
enum {ICL_PAD_NONE		=0,
	  ICL_PAD_CUSTOM	=ICL_PAD_NONE+1,
	  ICL_PAD_ZERO		=ICL_PAD_NONE+2,
	  ICL_PAD_ONE		=ICL_PAD_NONE+3,
	  ICL_PAD_ALTERNATE	=ICL_PAD_NONE+4,
	  ICL_PAD_FF		=ICL_PAD_NONE+5,
	  ICL_PAD_PKCS7		=ICL_PAD_NONE+6,
	  ICL_PAD_STEALING	=ICL_PAD_NONE+7};

/***************************************************************/
/*        Intel Cryptographic Library Supported Algorithms     */
/***************************************************************/

#define ICL_RSA

#ifdef _MD2
  #define ICL_MD2
#endif

#define ICL_DSA
#define ICL_MD5
#define ICL_SHA1 
#define ICL_DES
#define ICL_DESRandom
//#define ICL_RC4
#define ICL_RC5

/*-------- The following are the algorithm dependence of ICL library --------*/
/* There are two random number generators implemented (SHA1 & DES) for ICL_DSSSign.*/
#ifdef ICL_DSA
#ifndef ICL_SHA1
#define ICL_SHA1
#endif
#endif

/* ICl_RandGen is composed of a state machine and a DES core.*/
#ifdef ICL_DESRandom
#ifndef ICL_DES
#define ICL_DES
#endif
#endif

/***************************************************************/
/*         Intel Cryptographic Library Data Types              */
/***************************************************************/
/*                                                             */
/* Multi-precision integers are stored the least significant   */
/* byte first. This ensures that Intel 80x86 processors access */
/* the same data as bytes, words, dwords, or qwords without    */
/* reordering. The least significant byte of each integer is   */
/* stored in "value[0]" of  "ICLData" type.                    */
/***************************************************************/

/* One byte (8 bit) for ICL data types                         */
typedef unsigned char ICLByte;

/* One word for ICL data types                                 */
typedef unsigned long ICLWord;

/* DES Key , 64 bit with low order byte first                  */
typedef ICLByte ICLDESKey[8];

/* DES Initial Value, 64 bit with low order byte first         */
typedef ICLByte ICLDESIV[8];

/* RC5 Initial Value, 32,64,128 bits with low order byte first */
typedef ICLByte ICLRC5IV[16];

/* MD5 digested data, 128 bit with low order byte first        */
typedef ICLByte ICLMD5Digest[16];

/* MD2 digested data, 128 bit with low order byte first        */
#define ICLMD2_DIGEST_LENGTH	16
#define ICLMD2_ENC_BLOCK_LENGTH	(3 * ICLMD2_DIGEST_LENGTH)
typedef ICLByte ICLMD2Digest[ICLMD2_DIGEST_LENGTH];

/* SHA digested data, 160 bit with low order byte first        */
typedef ICLByte ICLSHADigest[20];

/* The salt type used in Password Based Encryption             */
typedef ICLWord ICLSalt[2];

/* The null terminated password string in PBE                  */
typedef ICLByte *ICLPassPhrase;

/***************************************************************/
/* ICLData has any number of ICLWords, not pre-allocated.      */
/* Space must be reserved by the user.                         */
/***************************************************************/

typedef struct {
  long         length;    /* number of bytes in 'value'        */
  ICLByte      *value;    /* memory address of the array       */
} ICLData;

/* RSA Public Key structure                                    */
typedef struct {
  ICLData      PublicExponent;      /* e                       */
  ICLData      Modulus;             /* n                       */
} ICLRSAPublicKey;

/* RSA Private Key Structure                                   */
typedef struct {
  ICLData      PublicExponent;     /* e                        */
  ICLData      PrivateExponent;    /* d                        */
  ICLData      Modulus;            /* n                        */
  ICLData      Prime[2];           /* p, q                     */
  ICLData      PrimeExponent[2];   /* d mod (p-1), d mod (q-1) */
  ICLData      Coefficient;        /* coeff = q^{-1} mod p     */
} ICLRSAPrivateKey;

/* RC4 Key structure                                           */
typedef struct {
  long         length;
  ICLByte      *value;
} ICLRC4Key;

/* RC4 State structure                                         */
typedef struct {
  ICLByte      state[256];             /* RC4 state            */
  ICLByte      i, j;
} ICLRC4State;

/* RC5 Key structure                                           */
typedef struct {
  long         length;
  ICLByte      *value;
} ICLRC5Key;

/* RC5 State Structure                                         */
typedef struct {
  ICLWord      regsize;         /* register size (16,32,64)    */
  ICLWord      rounds;          /* # of rounds (0..255)        */
  ICLWord      keysize;         /* # of bytes in key (0..255)  */
  ICLWord      iv[4];           /* holds CBC vector            */
  ICLWord      keytable[1024];  /* holds expanded key table    */
  ICLWord      mode;            /* tracks ECB, CBC, PAD modes  */
  ICLByte      buffer[32];      /* holds partial data chunks   */
  ICLWord      buflen;          /* length of buffer[]          */
  ICLWord      BLKS;            /* block length in bytes       */
  ICLWord      LOG2;            /* log2(BLKS)                  */
} ICLRC5State;

/* MD2 State structure                                           */
typedef struct _icl_md2state {
	ICLByte	EncBlock[ICLMD2_ENC_BLOCK_LENGTH];
	ICLByte InBuffer[ICLMD2_DIGEST_LENGTH];
	ICLByte CheckSum[ICLMD2_DIGEST_LENGTH];
	ICLWord Index;
} ICLMD2State;

/* MD5 State Structure                                         */
typedef struct {
  ICLWord      state[4];        /* MD5 State                   */
  ICLWord      count[2];        /* length of Message in bits   */
  ICLByte      buffer[64];      /* temprorary buffer           */
} ICLMD5State;

/* SHA State Structure                                         */
typedef struct {
  ICLWord      state[5];        /* SHA State                   */
  ICLWord      count[2];        /* length of Message in bits   */
  ICLByte      buffer[64];      /* temprorary buffer           */
} ICLSHAState;

/* DES State Structure                                         */
typedef struct {
  ICLWord      roundkey[32];    /* holds expanded key table    */
  ICLByte      vector[8];       /* holds CBC vector            */
  ICLByte      buffer[16];      /* holds partial 8-byte chunks */
  ICLWord      buflen;          /* length of buffer[]          */
  ICLWord      mode;            /* internal use (e=1,d=2)      */
} ICLDESState;

/* DSS Public Key Structure */
typedef struct {
  ICLData      PrimeModulus;    /* p */
  ICLData      PrimeDivisor;    /* q */
  ICLData      OrderQ;          /* g */
  ICLData      PublicKey;       /* y */
} ICLDSSPublicKey;

/* DSS Private Key Structure */
typedef struct {
  ICLData      PrimeModulus;    /* p */
  ICLData      PrimeDivisor;    /* q */
  ICLData      OrderQ;          /* g */
  ICLData      PrivateKey;      /* x */
} ICLDSSPrivateKey;


/***************************************************************/
/*                    Function Prototypes                      */
/***************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ICL_DES
int ICL_DESBeginECB
        (ICLDESKey          DESkey,
        ICLDESState         *DESstate);

int ICL_DESEncryptECB
        (ICLData            *PlainText,
        ICLDESState         *DESstate,
        ICLData             *CipherText);

int ICL_DESDecryptECB
        (ICLData            *CipherText,
        ICLDESState         *DESstate,
        ICLData             *PlainText);

int ICL_DESEndECB
        (ICLDESState        *DESstate);

int ICL_DESBeginCBC
        (ICLDESKey          DESkey,
        ICLDESIV            DESIV,
        ICLDESState         *DESstate);

int ICL_DESEncryptCBC
        (ICLData            *PlainText,
        ICLDESState         *DESstate,
        ICLData             *CipherText);

int ICL_DESDecryptCBC
        (ICLData            *CipherText,
        ICLDESState         *DESstate,
        ICLData             *PlainText);

int ICL_DESEndCBC
        (ICLDESState        *DESstate,
        ICLData             *RemainingOutput);

int ICL_RandGen
        (ICLSalt			SeedValue,
		long				Length,
        ICLData				*RandomNumber);

int ICL_DESBeginCBC_NoPad
        (ICLDESKey          DESkey,
        ICLDESIV            DESIV,
        ICLDESState         *DESstate);

int ICL_DESEncryptCBC_NoPad
        (ICLData            *PlainText,
        ICLDESState         *DESstate,
        ICLData             *CipherText);

int ICL_DESDecryptCBC_NoPad
        (ICLData            *CipherText,
        ICLDESState         *DESstate,
        ICLData             *PlainText);

int ICL_DESEndCBC_NoPad
        (ICLDESState        *DESstate,
        ICLData             *RemainingOutput);

/* DES ECB Pad Zero */
int	ICL_DESBeginECB_PadZERO
		(ICLDESKey			key,
        ICLDESIV			DESIV,
		ICLDESState			*DESstate);

int ICL_DESEncryptECB_PadZERO
		(ICLData 			*PlainText,
		 ICLDESState 		*DESstate,
		 ICLData 			*CipherText);

int ICL_DESDecryptECB_PadZERO
		(ICLData 			*CipherText,
		 ICLDESState 		*DESstate,
		 ICLData 			*PlainText);

/* DES ECB Pad One */
int ICL_DESEncryptECB_PadONE
		(ICLData 			*PlainText,
		 ICLDESState 		*DESstate,
		 ICLData 			*CipherText);

int ICL_DESDecryptECB_PadONE
		(ICLData 			*CipherText,
		 ICLDESState 		*DESstate,
		 ICLData 			*PlainText);

/* DES ECB Pad PKCS7 */
int	ICL_DESBeginECB_PadPKCS7
		(ICLDESKey			key,
		ICLDESState			*DESstate);

int ICL_DESEncryptECB_PadPKCS7
		(ICLData 			*PlainText,
		 ICLDESState 		*DESstate,
		 ICLData 			*CipherText);

int ICL_DESDecryptECB_PadPKCS7
		(ICLData 			*CipherText,
		 ICLDESState 		*DESstate,
		 ICLData 			*PlainText);

int ICL_DESEndECB_PadPKCS7
        (ICLDESState        *DESstate,
        ICLData             *RemainingOutput);

#endif

#ifdef ICL_RSA
int ICL_RSAPublicKeyOperation
	    (ICLData            *PlainText,
        ICLRSAPublicKey     *RSAPublicKey,
        ICLData             *CipherText);

int ICL_RSAPrivateKeyOperation
        (ICLData            *CipherText,
        ICLRSAPrivateKey    *RSAPrivateKey,
        ICLData             *PlainText);

int ICL_RSAKeyGenerate
        (ICLData            *RandomP,
        ICLData             *RandomQ,
        ICLData             *PublicExpCandidate,
        ICLRSAPublicKey     *RSAPublicKey,
        ICLRSAPrivateKey    *RSAPrivateKey);
#endif

int ICL_PBEncrypt
        (ICLData            *PlainText,
        ICLPassPhrase       PassPhrase,
        int                 IterationCount,
        ICLSalt             *Salt,
        ICLData             *CipherText,
        int                 Method);

int ICL_PBDecrypt
        (ICLData            *CipherText,
        ICLPassPhrase       PassPhrase,
        int                 IterationCount,
        ICLSalt             Salt,
        ICLData             *PlainText,
        int                 Method);

#ifdef ICL_RC4
int ICL_RC4Begin
        (ICLRC4Key          RC4Key,
        ICLRC4State         *RC4State);

int ICL_RC4Process
        (ICLData            *input,
        ICLRC4State         *RC4State,
        ICLData             *output);

int ICL_RC4End
        (ICLRC4State        *RC4State);
#endif

#ifdef ICL_RC5
int ICL_RC5Begin
        (ICLWord            regsize,
        ICLWord             rounds,
        ICLRC5Key           *RC5key,
        ICLWord             mode, 
        ICLRC5State         *RC5state);

int ICL_RC5SetIV
        (ICLRC5IV           IV, 
        ICLRC5State         *RC5state);

int ICL_RC5Encrypt
        (ICLData            *PlainText,
        ICLRC5State         *RC5state,
        ICLData             *CipherText);

int ICL_RC5Decrypt
        (ICLData            *CipherText,
        ICLRC5State         *RC5state,
        ICLData             *PlainText);

int ICL_RC5End
        (ICLRC5State        *RC5state,
        ICLData             *RemainingOutput);
#endif

#ifdef ICL_MD2
int ICL_MD2Begin(ICLMD2State *pState);

int ICL_MD2Process(ICLData *pMessage, ICLMD2State *pState);

int ICL_MD2End(ICLMD2State *pState, ICLMD2Digest Digest);
#endif

#ifdef ICL_MD5
int ICL_MD5Begin
        (ICLMD5State        *MD5State);

int ICL_MD5Process
        (ICLData            *Message,
        ICLMD5State         *MD5State);

int ICL_MD5End
        (ICLMD5State        *MD5State,
        ICLMD5Digest        digest);

int ICL_PBEDeriveWithMD5
		(ICLPassPhrase		PassPhrase,
		int					IterationCount,
		ICLDESKey			*DESKey,
		ICLDESIV			*DESIV,
		ICLSalt				*Salt);

/* Compatability macros. New code should call ICL_PBEDeriveWithMD5 */
#define ICL_PBEwithMD5(_PassPhrase_,_IterationCount_,_DESKey_,_DESIV_,_Salt_ ) \
	ICL_PBEDeriveWithMD5( _PassPhrase_, _IterationCount_, _DESKey_, _DESIV_, _Salt_ )

#define ICL_PBDwithMD5(_PassPhrase_,_IterationCount_,_DESKey_,_DESIV_,_Salt_ ) \
	ICL_PBEDeriveWithMD5( _PassPhrase_, _IterationCount_, _DESKey_, _DESIV_, _Salt_ )

#endif

#ifdef ICL_SHA1
int ICL_SHABegin
        (ICLSHAState        *SHAState);

int ICL_SHAProcess
        (ICLData            *Message,
        ICLSHAState         *SHAState);

int ICL_SHAEnd
        (ICLSHAState        *SHAState,
        ICLSHADigest        digest);

int ICL_PBEDeriveWithSHA
		(ICLPassPhrase		PassPhrase,
		int					IterationCount,
		ICLDESKey			*DESKey,
		ICLDESIV			*DESIV,
		ICLSalt				*Salt);

/* Compatability macros. New code should call ICL_PBEDeriveWithSHA */
#define ICL_PBEwithSHA(_PassPhrase_,_IterationCount_,_DESKey_,_DESIV_,_Salt_ ) \
	ICL_PBEDeriveWithSHA( _PassPhrase_, _IterationCount_, _DESKey_, _DESIV_, _Salt_ )

#define ICL_PBDwithSHA(_PassPhrase_,_IterationCount_,_DESKey_,_DESIV_,_Salt_ ) \
	ICL_PBEDeriveWithSHA( _PassPhrase_, _IterationCount_, _DESKey_, _DESIV_, _Salt_ )

#endif

#ifdef ICL_DSA
int ICL_DSSSign
        (ICLData            *Message,
		ICLData				*KSeed,
        ICLDSSPrivateKey    *PrivateKey,
        ICLData             *Signature_r,
        ICLData             *Signature_s);

int ICL_DSSVerify
        (ICLData            *Message,
        ICLData             *Signature_r,
        ICLData             *Signature_s,
        ICLDSSPublicKey     *PublicKey);

int ICL_DSSSignDigest
        (ICLData            *Digest,
		ICLData				*KSeed,
        ICLDSSPrivateKey    *PrivateKey,
        ICLData             *Signature_r,
        ICLData             *Signature_s);

int ICL_DSSVerifyDigest
        (ICLData            *Digest,
        ICLData             *Signature_r,
        ICLData             *Signature_s,
        ICLDSSPublicKey     *PublicKey);

int ICL_DSSKeyGenerate
        (ICLData            *Seed,
        ICLData             *XSeed,
        int                 L,
        ICLDSSPublicKey     *PublicKey,
        ICLDSSPrivateKey    *PrivateKey);
#endif

#ifdef __cplusplus
}
#endif

#endif     /* _ICL_INCLUDE_ */
