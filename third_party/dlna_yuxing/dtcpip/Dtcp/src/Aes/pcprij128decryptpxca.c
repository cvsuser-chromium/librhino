/*
//               INTEL CORPORATION PROPRIETARY INFORMATION
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Intel Corporation and may not be copied
// or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2002 Intel Corporation. All Rights Reserved.
//
//
// Purpose:
//    Cryptography Primitive.
//    Decrypt 128-bit data block according to Rijndael
//
// Contents:
//    Decrypt_RIJ128()
//
//
//    Created: Fri 24-May-2002 15:56
//  Author(s): Sergey Kirillov
//       mail: SergeyX_Kirillov@vniief.ims.intel.com
*/
#include "precomp.h"
#include "owncp.h"
#include "pcprij.h"
#include "pcprijtables.h"


/*
// Pseudo Code for EAS InvCipher
// was shown in Sec 5.3.5 of FIPS-197
//
// The reason why exactly Equivalent Inverse Cipher is used
// is the folowing: this case we have Inverse Cipher
// of the same structure as Forward Cipher.
// This means all consideration was already done for Forward Cipher
// are correct for Equivalent Inverse Cipher too.
//
//
// EqInvCipher(byte in[4*Nb], byte out[4*Nb], word dw[Nb*(Nr+1)])
// begin
//    byte state[4,Nb]
//
//    state = in
//
//    AddRoundKey(state, dw[Nr*Nb, (Nr+1)*Nb-1])
//
//    for round = Nr-1 step -1 downto 1
//       InvSubBytes(state)
//       InvShiftRows(state)
//       InvMixColumns(state)
//       AddRoundKey(state, dw[round*Nb, (round+1)*Nb-1])
//    end for
//
//    InvSubBytes(state)
//    InvShiftRows(state)
//    AddRoundKey(state, dw[0, Nb-1])
//
//    out = state
// end
//
*/

#if (_IPP==_IPP_PX) || (_IPP==_IPP_M6) || (_IPP==_IPP_A6) || (_IPP==_IPP_W7) || (_IPP==_IPP_T7)
/*
// RIJ128 regular round macro
//
// u   - input state
// v   - output state
// tbl - 4 tables
// rkey- round key (used in AddRoundKey() operation)
*/
#define RIJ128_DEC_STEP(u,v,tbl,rkey) \
   (v)[0] = ( (tbl)[0][ EBYTE((u)[0],0) ] \
             ^(tbl)[1][ EBYTE((u)[3],1) ] \
             ^(tbl)[2][ EBYTE((u)[2],2) ] \
             ^(tbl)[3][ EBYTE((u)[1],3) ] ) ^(rkey)[0]; \
   (v)[1] = ( (tbl)[0][ EBYTE((u)[1],0) ] \
             ^(tbl)[1][ EBYTE((u)[0],1) ] \
             ^(tbl)[2][ EBYTE((u)[3],2) ] \
             ^(tbl)[3][ EBYTE((u)[2],3) ] ) ^(rkey)[1]; \
   (v)[2] = ( (tbl)[0][ EBYTE((u)[2],0) ] \
             ^(tbl)[1][ EBYTE((u)[1],1) ] \
             ^(tbl)[2][ EBYTE((u)[0],2) ] \
             ^(tbl)[3][ EBYTE((u)[3],3) ] ) ^(rkey)[2]; \
   (v)[3] = ( (tbl)[0][ EBYTE((u)[3],0) ] \
             ^(tbl)[1][ EBYTE((u)[2],1) ] \
             ^(tbl)[2][ EBYTE((u)[1],2) ] \
             ^(tbl)[3][ EBYTE((u)[0],3) ] ) ^(rkey)[3]

/*
// RIJ128 last round macro
//
// u   - input state
// v   - output state
// tbl - just encryption S-box
// rkey- round key (used in AddRoundKey() operation)
*/
#define LAST_RIJ128_DEC_STEP(u,v,tbl,rkey) \
   (v)[0] = BYTES_TO_WORD( (tbl)[ EBYTE((u)[0],0) ], \
                           (tbl)[ EBYTE((u)[3],1) ], \
                           (tbl)[ EBYTE((u)[2],2) ], \
                           (tbl)[ EBYTE((u)[1],3) ] ) ^(rkey)[0]; \
   (v)[1] = BYTES_TO_WORD( (tbl)[ EBYTE((u)[1],0) ], \
                           (tbl)[ EBYTE((u)[0],1) ], \
                           (tbl)[ EBYTE((u)[3],2) ], \
                           (tbl)[ EBYTE((u)[2],3) ] ) ^(rkey)[1]; \
   (v)[2] = BYTES_TO_WORD( (tbl)[ EBYTE((u)[2],0) ], \
                           (tbl)[ EBYTE((u)[1],1) ], \
                           (tbl)[ EBYTE((u)[0],2) ], \
                           (tbl)[ EBYTE((u)[3],3) ] ) ^(rkey)[2]; \
   (v)[3] = BYTES_TO_WORD( (tbl)[ EBYTE((u)[3],0) ], \
                           (tbl)[ EBYTE((u)[2],1) ], \
                           (tbl)[ EBYTE((u)[1],2) ], \
                           (tbl)[ EBYTE((u)[0],3) ] ) ^(rkey)[3]


/* define number of column in the state */
#define SC NB(128)

void Decrypt_RIJ128(const Ipp32u* pInpBlk, Ipp32u* pOutBlk, int nr, const Ipp32u* pKeys)
{
   Ipp32u state[SC]; /* input/output state (for even/odd RIJ_DEC_STEP) */

   state[0] = pInpBlk[ 0] ^ (pKeys+nr*SC)[0];
   state[1] = pInpBlk[ 1] ^ (pKeys+nr*SC)[1];
   state[2] = pInpBlk[ 2] ^ (pKeys+nr*SC)[2];
   state[3] = pInpBlk[ 3] ^ (pKeys+nr*SC)[3];

   /* advance round key pointer */
   pKeys += 9 * SC;

   /* do rounds */
   switch(nr) {
   case NR128_256:
           RIJ128_DEC_STEP(state,pOutBlk, RijDecTbl,  pKeys+4*SC);
           RIJ128_DEC_STEP(pOutBlk,state, RijDecTbl,  pKeys+3*SC);
   case NR128_192:
           RIJ128_DEC_STEP(state,pOutBlk, RijDecTbl,  pKeys+2*SC);
           RIJ128_DEC_STEP(pOutBlk,state, RijDecTbl,  pKeys+1*SC);
   default:
           RIJ128_DEC_STEP(state,pOutBlk, RijDecTbl,  pKeys     );
           RIJ128_DEC_STEP(pOutBlk,state, RijDecTbl,  pKeys-1*SC);
           RIJ128_DEC_STEP(state,pOutBlk, RijDecTbl,  pKeys-2*SC);
           RIJ128_DEC_STEP(pOutBlk,state, RijDecTbl,  pKeys-3*SC);
           RIJ128_DEC_STEP(state,pOutBlk, RijDecTbl,  pKeys-4*SC);
           RIJ128_DEC_STEP(pOutBlk,state, RijDecTbl,  pKeys-5*SC);
           RIJ128_DEC_STEP(state,pOutBlk, RijDecTbl,  pKeys-6*SC);
           RIJ128_DEC_STEP(pOutBlk,state, RijDecTbl,  pKeys-7*SC);
           RIJ128_DEC_STEP(state,pOutBlk, RijDecTbl,  pKeys-8*SC);
      LAST_RIJ128_DEC_STEP(pOutBlk,state, RijDecSbox, pKeys-9*SC);
   }

   /* copy state to the output block */
   pOutBlk[0] = state[0];
   pOutBlk[1] = state[1];
   pOutBlk[2] = state[2];
   pOutBlk[3] = state[3];
}
#endif /* _IPP_PX */

