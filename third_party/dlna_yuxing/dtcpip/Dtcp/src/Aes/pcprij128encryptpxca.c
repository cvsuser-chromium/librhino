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
//    Encrypt 128-bit data block according to Rijndael
//
// Contents:
//    Encrypt_RIJ128()
//
//
//    Created: Tue 23-May-2002 10:35
//  Author(s): Sergey Kirillov
//       mail: SergeyX_Kirillov@vniief.ims.intel.com
*/
#include "precomp.h"
#include "owncp.h"
#include "pcprij.h"
#include "pcprijtables.h"


/*
// Pseudo Code for EAS Cipher
// was shown in Sec 5.1 of FIPS-197
//
// Cipher(byte in[4*Nb], byte out[4*Nb], word w[Nb*(Nr+1)])
// begin
//    byte state[4,Nb]
//
//    state = in
//
//    AddRoundKey(state, w[0, Nb-1])   // See Sec. 5.1.4
//
//    for round = 1 step 1 to Nr–1
//       SubBytes(state)               // See Sec. 5.1.1
//       ShiftRows(state)              // See Sec. 5.1.2
//       MixColumns(state)             // See Sec. 5.1.3
//       AddRoundKey(state, w[round*Nb, (round+1)*Nb-1])
//    end for
//
//    SubBytes(state)
//    ShiftRows(state)
//    AddRoundKey(state, w[Nr*Nb, (Nr+1)*Nb-1])
//
//    out = state
// end
//
// Note:
//    The notation w[i,j] looks strange because w[] is easy linear array
//    I'll try to use w[round*Nb] instead
//
//
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    Brief Consideration of AES Encryption
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// There are 4 separate steps:
//      I. copy input block into the state
//         (and AddRoundKey() operation with round_key[0] may by involve)
//     II. regular rounding ((Nr-1) times) means applying of the following operation sequence:
//             SubBytes(state)
//             ShiftRows(state)
//             MixColumns(state)
//             AddRoundKey(state, w[round*Nb, (round+1)*Nb-1])
//    III. last (irregular) rounding means applying of the following operation sequence:
//             SubBytes(state)
//             ShiftRows(state)
//             AddRoundKey(state, w[Nr*Nb, (Nr+1)*Nb-1])
//     IV. copy state into the output block
//
// All 4 steps are considered below with suitable degree of detales.
//
*/

/*
//////////////////////////////
//                          //
// 128 bits data block size //
//                          //
//////////////////////////////

//
// I. Copy input block (and adding round_key[0])
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// It is easy step. Following sec 3.4 FIPS-197 just provide copying
//    state[r,c] = in[r+4*c]
// Note STATE in reality is declared as
//    Ipp32u state[4]
// but input are bytes. To avoid any alignment access problem
// macro BYTES_TO_WORD() is using for constucts single word based on 4 bytes input.
//
//
// II. Regular rounding
// ~~~~~~~~~~~~~~~~~~~~
// Line below are perform main single step of AES encryption - the sequence of
//    SubBytes(statte)
//    ShiftRows(state)
//    MixColumns(state)
// operation. (Last operation AddRoundKey() was skipped from following consideration
// because of trivial - just XOR)
//
// Let
//    u[r,c] - is input state
//    v[r,c] - is output state, 0<= r <4, 0<= c <4
// I'll use u00, v00 and so on instead of u[0,0], v[0,0] and so on accordingly.
//
// Then
//
//             SubByte()	ShiftRows()	                           MixColumns()						
// r,c         ~~~~~~~~~   ~~~~~~~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~						
// 0,0   v00   Sbox[u00]   Sbox[u00]   {2}Sbox[u00] xor {3}Sbox[u11] xor {1}Sbox[u22] xor {1}Sbox[u33]
// 0,1   v01   Sbox[u01]   Sbox[u01]   {2}Sbox[u01] xor {3}Sbox[u12] xor {1}Sbox[u23] xor {1}Sbox[u30]
// 0,2   v02   Sbox[u02]   Sbox[u02]   {2}Sbox[u02] xor {3}Sbox[u13] xor {1}Sbox[u20] xor {1}Sbox[u31]
// 0,3   v03   Sbox[u03]   Sbox[u03]   {2}Sbox[u03] xor {3}Sbox[u10] xor {1}Sbox[u21] xor {1}Sbox[u32]
//
// 1,0   v10   Sbox[u10]   Sbox[u11]   {1}Sbox[u00] xor {2}Sbox[u11] xor {3}Sbox[u22] xor {1}Sbox[u33]
// 1,1   v11   Sbox[u11]   Sbox[u12]   {1}Sbox[u01] xor {2}Sbox[u12] xor {3}Sbox[u23] xor {1}Sbox[u30]
// 1,2   v12   Sbox[u12]   Sbox[u13]   {1}Sbox[u02] xor {2}Sbox[u13] xor {3}Sbox[u20] xor {1}Sbox[u31]
// 1,3   v13   Sbox[u13]   Sbox[u10]   {1}Sbox[u03] xor {2}Sbox[u10] xor {3}Sbox[u21] xor {1}Sbox[u32]
//
// 2,0   v20   Sbox[u20]   Sbox[u22]   {1}Sbox[u00] xor {1}Sbox[u11] xor {2}Sbox[u22] xor {3}Sbox[u33]
// 2,1   v21   Sbox[u21]   Sbox[u23]   {1}Sbox[u01] xor {1}Sbox[u12] xor {2}Sbox[u23] xor {3}Sbox[u30]
// 2,2   v22   Sbox[u22]   Sbox[u20]   {1}Sbox[u02] xor {1}Sbox[u13] xor {2}Sbox[u20] xor {3}Sbox[u31]
// 2,3   v23   Sbox[u23]   Sbox[u21]   {1}Sbox[u03] xor {1}Sbox[u10] xor {2}Sbox[u21] xor {3}Sbox[u32]
//
// 3,0   v30   Sbox[u30]   Sbox[u33]   {3}Sbox[u00] xor {1}Sbox[u11] xor {1}Sbox[u22] xor {2}Sbox[u33]
// 3,1   v31   Sbox[u31]   Sbox[u30]   {3}Sbox[u01] xor {1}Sbox[u12] xor {1}Sbox[u23] xor {2}Sbox[u30]
// 3,2   v32   Sbox[u32]   Sbox[u31]   {3}Sbox[u02] xor {1}Sbox[u13] xor {1}Sbox[u20] xor {2}Sbox[u31]
// 3,3   v33   Sbox[u33]   Sbox[u32]   {3}Sbox[u03] xor {1}Sbox[u10] xor {1}Sbox[u21] xor {2}Sbox[u32]
//
// Let
//    V0 = word( v00,v10,v20,v30 )
//    V1 = word( v01,v11,v21,v31 )
//    V2 = word( v02,v12,v22,v32 )
//    V3 = word( v03,v13,v23,v33 )
// are columns of the outpur state v[r,c]
//
// Then
// V0 = word( {2}Sbox[u00], {1}Sbox[u00], {1}Sbox[u00], {3}Sbox[u00] ) xor
//      word( {3}Sbox[u11], {2}Sbox[u11], {1}Sbox[u11], {1}Sbox[u11] ) xor
//      word( {1}Sbox[u22], {3}Sbox[u22], {2}Sbox[u22], {1}Sbox[u22] ) xor
//      word( {1}Sbox[u33], {1}Sbox[u33], {3}Sbox[u33], {2}Sbox[u33] )
// V1 = word( {2}Sbox[u01], {1}Sbox[u01], {1}Sbox[u01], {3}Sbox[u01] ) xor
//      word( {3}Sbox[u12], {2}Sbox[u12], {1}Sbox[u12], {1}Sbox[u12] ) xor
//      word( {1}Sbox[u23], {3}Sbox[u23], {2}Sbox[u23], {1}Sbox[u23] ) xor
//      word( {1}Sbox[u30], {1}Sbox[u30], {3}Sbox[u30], {2}Sbox[u30] )
// V2 = word( {2}Sbox[u02], {1}Sbox[u02], {1}Sbox[u02], {3}Sbox[u02] ) xor
//      word( {3}Sbox[u13], {2}Sbox[u13], {1}Sbox[u13], {1}Sbox[u13] ) xor
//      word( {1}Sbox[u20], {3}Sbox[u20], {2}Sbox[u20], {1}Sbox[u20] ) xor
//      word( {1}Sbox[u31], {1}Sbox[u31], {3}Sbox[u31], {2}Sbox[u31] )
// V3 = word( {2}Sbox[u03], {1}Sbox[u03], {1}Sbox[u03], {3}Sbox[u03] ) xor
//      word( {3}Sbox[u10], {2}Sbox[u10], {1}Sbox[u10], {1}Sbox[u10] ) xor
//      word( {1}Sbox[u21], {3}Sbox[u21], {2}Sbox[u21], {1}Sbox[u21] ) xor
//      word( {1}Sbox[u32], {1}Sbox[u32], {3}Sbox[u32], {2}Sbox[u32] )
//
// Assigning by the similar way
//    U0 = word( u00,u10,u20,u30 )
//    U1 = word( u01,u11,u21,u31 )
//    U2 = word( u02,u12,u22,u32 )
//    U3 = word( u03,u13,u23,u33 )
// are columns of the input state u[r,c]
//
// and X(n) n-th byte of word X
// rewite V0, V1, V2 and V3 as follows
// V0 = word( {2}Sbox[U0(0)], {1}Sbox[U0(0)], {1}Sbox[U0(0)], {3}Sbox[U0(0)] ) xor
//      word( {3}Sbox[U1(1)], {2}Sbox[U1(1)], {1}Sbox[U1(1)], {1}Sbox[U1(1)] ) xor
//      word( {1}Sbox[U2(2)], {3}Sbox[U2(2)], {2}Sbox[U2(2)], {1}Sbox[U2(2)] ) xor
//      word( {1}Sbox[U3(3)], {1}Sbox[U3(3)], {3}Sbox[U3(3)], {2}Sbox[U3(3)] )
// V1 = word( {2}Sbox[U1(0)], {1}Sbox[U1(0)], {1}Sbox[U1(0)], {3}Sbox[U1(0)] ) xor
//      word( {3}Sbox[U2(1)], {2}Sbox[U2(1)], {1}Sbox[U2(1)], {1}Sbox[U2(1)] ) xor
//      word( {1}Sbox[U3(2)], {3}Sbox[U3(2)], {2}Sbox[U3(2)], {1}Sbox[U3(2)] ) xor
//      word( {1}Sbox[U0(3)], {1}Sbox[U0(3)], {3}Sbox[U0(3)], {2}Sbox[U0(3)] )
// V2 = word( {2}Sbox[U2(0)], {1}Sbox[U2(0)], {1}Sbox[U2(0)], {3}Sbox[U2(0)] ) xor
//      word( {3}Sbox[U3(1)], {2}Sbox[U3(1)], {1}Sbox[U3(1)], {1}Sbox[U3(1)] ) xor
//      word( {1}Sbox[U0(2)], {3}Sbox[U0(2)], {2}Sbox[U0(2)], {1}Sbox[U0(2)] ) xor
//      word( {1}Sbox[U1(3)], {1}Sbox[U1(3)], {3}Sbox[U1(3)], {2}Sbox[U1(3)] )
// V3 = word( {2}Sbox[U3(0)], {1}Sbox[U3(0)], {1}Sbox[U3(0)], {3}Sbox[U3(0)] ) xor
//      word( {3}Sbox[U0(1)], {2}Sbox[U0(1)], {1}Sbox[U0(1)], {1}Sbox[U0(1)] ) xor
//      word( {1}Sbox[U1(2)], {3}Sbox[U1(2)], {2}Sbox[U1(2)], {1}Sbox[U1(2)] ) xor
//      word( {1}Sbox[U2(3)], {1}Sbox[U2(3)], {3}Sbox[U2(3)], {2}Sbox[U2(3)] )
//
// Easy to see expressions
//    T0 = word( {2}Sbox[y], {1}Sbox[y], {1}Sbox[y], {3}Sbox[y] )
//    T1 = word( {3}Sbox[y], {2}Sbox[y], {1}Sbox[y], {1}Sbox[y] )
//    T2 = word( {1}Sbox[y], {3}Sbox[y], {2}Sbox[y], {1}Sbox[y] )
//    T3 = word( {1}Sbox[y], {1}Sbox[y], {3}Sbox[y], {2}Sbox[y] )
// can be precaculated before encryption. Really, T0, T1, T2 and T3 are
// WORD tables and each entry of these tables was generated using the S-box
// defined in sec 5.1.1 FIPS-197.
//
// Source contains definition of T0, T1, T2 and T3 like the follows:
// const Ipp32u EncTbl[4][256] = {
//    { ENC_S_BOX(fwd_t0) },
//    { ENC_S_BOX(fwd_t1) },
//    { ENC_S_BOX(fwd_t2) },
//    { ENC_S_BOX(fwd_t3) }
// };
//
// and
//    T0 = EncTbl[0]
//    T1 = EncTbl[1]
//    T2 = EncTbl[2]
//    T3 = EncTbl[3]
//
// Based on brief consideration above we have an ability construct
// not bad code of AES encryption step. AES_ENC_STEP() is the example.
//
//
// III. Last round
// ~~~~~~~~~~~~~~~
// This step in general close to Regular Rounging
// (but significantly easy, because of no MixColumn() operation).
// Line below are perform last round of AES encryption - the sequence of
//    SubBytes(statte)
//    ShiftRows(state)
// operation. (Last operation AddRoundKey() was skipped again
// because of trivial - just XOR)
//
// Using the notation of input (u) and output (v) state we have
//
//             SubByte()	ShiftRows()
// r,c         ~~~~~~~~~   ~~~~~~~~~~~
// 0,0   v00   Sbox[u00]   Sbox[u00]  
// 0,1   v01   Sbox[u01]   Sbox[u01]  
// 0,2   v02   Sbox[u02]   Sbox[u02]  
// 0,3   v03   Sbox[u03]   Sbox[u03]  
//
// 1,0   v10   Sbox[u10]   Sbox[u11]  
// 1,1   v11   Sbox[u11]   Sbox[u12]  
// 1,2   v12   Sbox[u12]   Sbox[u13]  
// 1,3   v13   Sbox[u13]   Sbox[u10]  
//
// 2,0   v20   Sbox[u20]   Sbox[u22]  
// 2,1   v21   Sbox[u21]   Sbox[u23]  
// 2,2   v22   Sbox[u22]   Sbox[u20]  
// 2,3   v23   Sbox[u23]   Sbox[u21]  
//
// 3,0   v30   Sbox[u30]   Sbox[u33]  
// 3,1   v31   Sbox[u31]   Sbox[u30]  
// 3,2   v32   Sbox[u32]   Sbox[u31]  
// 3,3   v33   Sbox[u33]   Sbox[u32]  
//
// Using V0, V1, V2 and V3 as columns of the output state v[r,c]
// Then
// V0 = word( Sbox[u00], Sbox[u11], Sbox[u22], Sbox[u33] )
// V1 = word( Sbox[u01], Sbox[u12], Sbox[u23], Sbox[u30] )
// V2 = word( Sbox[u02], Sbox[u13], Sbox[u20], Sbox[u31] )
// V3 = word( Sbox[u03], Sbox[u10], Sbox[u21], Sbox[u32] )
//
// Using U0, U1, U2 and U3 as columns of the input state u[r,c]
// Then (1-st form)
// V0 = word( Sbox[U0(0)], Sbox[U1(1)], Sbox[U2(2)], Sbox[U3(3)] )
// V1 = word( Sbox[U1(0)], Sbox[U2(1)], Sbox[U3(2)], Sbox[U0(3)] )
// V2 = word( Sbox[U2(0)], Sbox[U3(1)], Sbox[U0(2)], Sbox[U1(3)] )
// V3 = word( Sbox[U3(0)], Sbox[U0(1)], Sbox[U1(2)], Sbox[U2(3)] )
//
// There is an ability performs V0, V1, V2 and V3 in other (2-nd form)
// Look
// V0 = word( Sbox[u00],         0,         0,         0 ) xor
//      word(         0, Sbox[u11],         0,         0 ) xor
//      word(         0,         0, Sbox[u22],         0 ) xor
//      word(         0,         0,         0, Sbox[u33] )
// V1 = word( Sbox[u01],         0,         0,         0 ) xor
//      word(         0, Sbox[u12],         0,         0 ) xor
//      word(         0,         0, Sbox[u23],         0 ) xor
//      word(         0,         0,         0, Sbox[u30] )
// V2 = word( Sbox[u02],         0,         0,         0 ) xor
//      word(         0, Sbox[u13],         0,         0 ) xor
//      word(         0,         0, Sbox[u20],         0 ) xor
//      word(         0,         0,         0, Sbox[u31] )
// V3 = word( Sbox[u03],         0,         0,         0 ) xor
//      word(         0, Sbox[u10],         0,         0 ) xor
//      word(         0,         0, Sbox[u21],         0 ) xor
//      word(         0,         0,         0, Sbox[u32] )
//
// Or acordingly
// V0 = word( Sbox[U0(0)],           0,           0,           0 ) xor
//      word(           0, Sbox[U1(1)],           0,           0 ) xor
//      word(           0,           0, Sbox[U2(2)],           0 ) xor
//      word(           0,           0,           0, Sbox[U3(3)] )
// V1 = word( Sbox[U1{0}],           0,           0,           0 ) xor
//      word(           0, Sbox[U2{1}],           0,           0 ) xor
//      word(           0,           0, Sbox[U3(2)],           0 ) xor
//      word(           0,           0,           0, Sbox[U0(3)] )
// V2 = word( Sbox[U2(0)],           0,           0,           0 ) xor
//      word(           0, Sbox[U3(1)],           0,           0 ) xor
//      word(           0,           0, Sbox[U0(2)],           0 ) xor
//      word(           0,           0,           0, Sbox[U1(3)] )
// V3 = word( Sbox[U3(0)],           0,           0,           0 ) xor
//      word(           0, Sbox[U0(1)],           0,           0 ) xor
//      word(           0,           0, Sbox[U1(2)],           0 ) xor
//      word(           0,           0,           0, Sbox[U2(3)] )
//
// Both (1-st and 2-nd form) are quite close together.
// And it is seems 1-st form is more convinient on SA/XSC architecture.
// Future will show ....
//
//
// IV. Copy into the output
// ~~~~~~~~~~~~~~~~~~~~~~~~
// It is easy step too. Following sec 3.4 FIPS-197 just provide copying
//    out[r+4*c] = state[r,c]
// Note STATE in reality is declared as
//    Ipp32u state[4]
// but output are bytes. To avoid any alignment access problem
// macro EBYTESD() is using for extract byte from the word.
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
#define RIJ128_ENC_STEP(u,v,tbl,rkey) \
   (v)[0] = ( (tbl)[0][ EBYTE((u)[0],0) ] \
             ^(tbl)[1][ EBYTE((u)[1],1) ] \
             ^(tbl)[2][ EBYTE((u)[2],2) ] \
             ^(tbl)[3][ EBYTE((u)[3],3) ] ) ^(rkey)[0]; \
   (v)[1] = ( (tbl)[0][ EBYTE((u)[1],0) ] \
             ^(tbl)[1][ EBYTE((u)[2],1) ] \
             ^(tbl)[2][ EBYTE((u)[3],2) ] \
             ^(tbl)[3][ EBYTE((u)[0],3) ] ) ^(rkey)[1]; \
   (v)[2] = ( (tbl)[0][ EBYTE((u)[2],0) ] \
             ^(tbl)[1][ EBYTE((u)[3],1) ] \
             ^(tbl)[2][ EBYTE((u)[0],2) ] \
             ^(tbl)[3][ EBYTE((u)[1],3) ] ) ^(rkey)[2]; \
   (v)[3] = ( (tbl)[0][ EBYTE((u)[3],0) ] \
             ^(tbl)[1][ EBYTE((u)[0],1) ] \
             ^(tbl)[2][ EBYTE((u)[1],2) ] \
             ^(tbl)[3][ EBYTE((u)[2],3) ] ) ^(rkey)[3]

/*
// RIJ128 last round macro
//
// u   - input state
// v   - output state
// tbl - just encryption S-box
// rkey- round key (used in AddRoundKey() operation)
*/
#define LAST_RIJ128_ENC_STEP(u,v,tbl,rkey) \
   (v)[0] = BYTES_TO_WORD( (tbl)[ EBYTE((u)[0],0) ], \
                           (tbl)[ EBYTE((u)[1],1) ], \
                           (tbl)[ EBYTE((u)[2],2) ], \
                           (tbl)[ EBYTE((u)[3],3) ] ) ^(rkey)[0]; \
   (v)[1] = BYTES_TO_WORD( (tbl)[ EBYTE((u)[1],0) ], \
                           (tbl)[ EBYTE((u)[2],1) ], \
                           (tbl)[ EBYTE((u)[3],2) ], \
                           (tbl)[ EBYTE((u)[0],3) ] ) ^(rkey)[1]; \
   (v)[2] = BYTES_TO_WORD( (tbl)[ EBYTE((u)[2],0) ], \
                           (tbl)[ EBYTE((u)[3],1) ], \
                           (tbl)[ EBYTE((u)[0],2) ], \
                           (tbl)[ EBYTE((u)[1],3) ] ) ^(rkey)[2]; \
   (v)[3] = BYTES_TO_WORD( (tbl)[ EBYTE((u)[3],0) ], \
                           (tbl)[ EBYTE((u)[0],1) ], \
                           (tbl)[ EBYTE((u)[1],2) ], \
                           (tbl)[ EBYTE((u)[2],3) ] ) ^(rkey)[3]


/* define number of column in the state */
#define SC NB(128)

void Encrypt_RIJ128(const Ipp32u* pInpBlk, Ipp32u* pOutBlk, int nr, const Ipp32u* pKeys)
{
   Ipp32u state[SC]; /* input/output state (for even/odd RIJ_ENC_STEP) */

   state[0] = pInpBlk[ 0] ^ pKeys[0];
   state[1] = pInpBlk[ 1] ^ pKeys[1];
   state[2] = pInpBlk[ 2] ^ pKeys[2];
   state[3] = pInpBlk[ 3] ^ pKeys[3];

   /* advance round key pointer */
   pKeys += (nr-9) * SC;

   /* do rounds */
   switch(nr) {
   case NR128_256:
           RIJ128_ENC_STEP(state,pOutBlk, RijEncTbl,  pKeys-4*SC);
           RIJ128_ENC_STEP(pOutBlk,state, RijEncTbl,  pKeys-3*SC);
   case NR128_192:
           RIJ128_ENC_STEP(state,pOutBlk, RijEncTbl,  pKeys-2*SC);
           RIJ128_ENC_STEP(pOutBlk,state, RijEncTbl,  pKeys-1*SC);
   default:
           RIJ128_ENC_STEP(state,pOutBlk, RijEncTbl,  pKeys     );
           RIJ128_ENC_STEP(pOutBlk,state, RijEncTbl,  pKeys+1*SC);
           RIJ128_ENC_STEP(state,pOutBlk, RijEncTbl,  pKeys+2*SC);
           RIJ128_ENC_STEP(pOutBlk,state, RijEncTbl,  pKeys+3*SC);
           RIJ128_ENC_STEP(state,pOutBlk, RijEncTbl,  pKeys+4*SC);
           RIJ128_ENC_STEP(pOutBlk,state, RijEncTbl,  pKeys+5*SC);
           RIJ128_ENC_STEP(state,pOutBlk, RijEncTbl,  pKeys+6*SC);
           RIJ128_ENC_STEP(pOutBlk,state, RijEncTbl,  pKeys+7*SC);
           RIJ128_ENC_STEP(state,pOutBlk, RijEncTbl,  pKeys+8*SC);
      LAST_RIJ128_ENC_STEP(pOutBlk,state, RijEncSbox, pKeys+9*SC);
   }

   /* copy state to the output block */
   pOutBlk[0] = state[0];
   pOutBlk[1] = state[1];
   pOutBlk[2] = state[2];
   pOutBlk[3] = state[3];
}
#endif /* _IPP_PX */
