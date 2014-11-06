//#############################################################################
//## Copyright (c) 2004 Intel Corporation All Rights Reserved. 
//## 
//## The source code contained or described herein and all documents related to
//## the source code ("Material") are owned by Intel Corporation or its 
//## suppliers or licensors. Title to the Material remains with Intel 
//## Corporation or its suppliers and licensors. The Material contains trade 
//## secrets and proprietary and confidential information of Intel or its
//## suppliers and licensors. The Material is protected by worldwide copyright
//## and trade secret laws and treaty provisions. No part of the Material may 
//## be used, copied, reproduced, modified, published, uploaded, posted, 
//## transmitted, distributed, or disclosed in any way without Intel's prior 
//## express written permission.
//## 
//## No license under any patent, copyright, trade secret or other 
//## intellectual property right is granted to or conferred upon you by 
//## disclosure or delivery of the Materials, either expressly, by 
//## implication, inducement, estoppel or otherwise. Any license under such 
//## intellectual property rights must be express and approved by Intel in 
//## writing.
//#############################################################################

#include <stdlib.h>
#include <memory.h>
#include "DtcpContentManagement.h"
#include "DtcpAkeCore.h"
#include "DtcpCore.h"
#include "DtcpStatusCodes.h"
#include "DtcpUtils.h"
#include "Rng.h"
#include "ippcp.h"

/// \file 
/// \brief Implementation file for the \ref DtcpContent library.
///
/// This file implements the interface for the \ref DtcpContent library.

#ifdef _MSC_VER
typedef unsigned __int64 int64;
#else /* assuming GCC */
typedef unsigned long long int64;
#endif

#if 0
static int GlobalDisplayLevel             = 0;
static int GlobalDisplayLevelMatchExactly = 0;
static int GlobalLogToFile                = 0;
#endif

/// \brief Data associated with a stream
typedef struct DtcpStreamData
{
    EnStreamTransport          StreamTransport;       ///< Stream transport for this stream
    unsigned char              HttpNonce[DTCP_CONTENT_KEY_NONCE_SIZE]; ///< Nonce for HTTP stream
    DtcpAkeCoreStreamHandle    AkeCoreStreamHandle; ///< Ake Core handle to stream
} DtcpStreamData;

/// \brief Data associated with a protected content packet
typedef struct DtcpPacketData
{
    IppsRijndael128 *CipherContext;             ///< Cipher context
    unsigned char    NextIv[DTCP_AES_IV_SIZE];  ///< IV to use for next call to encrypt/decrypt
    unsigned int     PayloadSize;               ///< Size of payload including padding bytes
    unsigned int     BytesProcessed;            ///< Number of bytes processed
} DtcpPacketData;

static void IncrementNonce(unsigned char aNonce[DTCP_CONTENT_KEY_NONCE_SIZE])
{
    int64 nonce;

    DEBUG_MSG(MSG_DEBUG+1, ("DtcpContentManagement::IncrementNonce\r\n"));
    // This implementation assumes a little endian processor and a 64 bit nonce.
    memcpy(&nonce, aNonce, DTCP_CONTENT_KEY_NONCE_SIZE);
    SwapInPlace(&nonce, 8);
    nonce = nonce + 1;
    SwapInPlace(&nonce, 8);
    memcpy(aNonce, &nonce, DTCP_CONTENT_KEY_NONCE_SIZE);
}

int CreateContentKey(EnExtendedEmi aExtendedEmi,
                     unsigned char aExchangeKey[DTCP_EXCHANGE_KEY_SIZE],
                     unsigned char aNonce[DTCP_CONTENT_KEY_NONCE_SIZE],
                     unsigned char aCipherKey[DTCP_AES_KEY_SIZE],
                     unsigned char aCipherIv[DTCP_AES_IV_SIZE])
{
    int returnValue = SUCCESS;
    unsigned char contentKeyConstant[DTCP_CONTENT_KEY_CONSTANT_SIZE];
    unsigned char ivConstant[DTCP_IV_CONSTANT_SIZE];
    unsigned char keyGenBuffer[2 * DTCP_AES_BLOCK_SIZE];
    unsigned char ivGenBuffer[DTCP_CONTENT_KEY_NONCE_SIZE + DTCP_IV_CONSTANT_SIZE];
    IppsRijndael128 *cipherContext = NULL;            
    unsigned int cipherContextSize;
    int i;

    if (!aExchangeKey ||
        !aNonce       ||
        !aCipherKey)
    {
        returnValue = INVALID_ARGUMENT;
    }
    
    if (IS_SUCCESS(returnValue))
    {
        if (extEmiCopyNever == aExtendedEmi)
        {
            contentKeyConstant[0]  = 0xA3;
            contentKeyConstant[1]  = 0x80;
            contentKeyConstant[2]  = 0x6F;
            contentKeyConstant[3]  = 0x97;
            contentKeyConstant[4]  = 0x16;
            contentKeyConstant[5]  = 0x35;
            contentKeyConstant[6]  = 0xC1;
            contentKeyConstant[7]  = 0x79;
            contentKeyConstant[8]  = 0x6D;
            contentKeyConstant[9]  = 0xEA;
            contentKeyConstant[10] = 0xED;
            contentKeyConstant[11] = 0x24;
        }
        else if (extEmiCopyOneGenFormatCog == aExtendedEmi)
        {
            contentKeyConstant[0]  = 0x28;
            contentKeyConstant[1]  = 0xB5;
            contentKeyConstant[2]  = 0x22;
            contentKeyConstant[3]  = 0xC7;
            contentKeyConstant[4]  = 0xCD;
            contentKeyConstant[5]  = 0xB2;
            contentKeyConstant[6]  = 0x2D;
            contentKeyConstant[7]  = 0x49;
            contentKeyConstant[8]  = 0x0E;
            contentKeyConstant[9]  = 0xF8;
            contentKeyConstant[10] = 0xE3;
            contentKeyConstant[11] = 0xD4;
        }
        else if (extEmiCopyOneGenFormatNonCog == aExtendedEmi)
        {
            contentKeyConstant[0]  = 0x7C;
            contentKeyConstant[1]  = 0x0D;
            contentKeyConstant[2]  = 0x87;
            contentKeyConstant[3]  = 0xC7;
            contentKeyConstant[4]  = 0x6C;
            contentKeyConstant[5]  = 0x19;
            contentKeyConstant[6]  = 0x56;
            contentKeyConstant[7]  = 0x62;
            contentKeyConstant[8]  = 0xF9;
            contentKeyConstant[9]  = 0x8C;
            contentKeyConstant[10] = 0x14;
            contentKeyConstant[11] = 0xE6;
        }
        else if (extEmiMove == aExtendedEmi)
        {
            contentKeyConstant[0]  = 0xE6;
            contentKeyConstant[1]  = 0xED;
            contentKeyConstant[2]  = 0x3E;
            contentKeyConstant[3]  = 0x30;
            contentKeyConstant[4]  = 0x78;
            contentKeyConstant[5]  = 0xFF;
            contentKeyConstant[6]  = 0x0E;
            contentKeyConstant[7]  = 0x5B;
            contentKeyConstant[8]  = 0x3A;
            contentKeyConstant[9]  = 0x27;
            contentKeyConstant[10] = 0x3C;
            contentKeyConstant[11] = 0xCB;
        }
        else if (extEmiNoMoreCopies == aExtendedEmi)
        {
            contentKeyConstant[0]  = 0x45;
            contentKeyConstant[1]  = 0xB1;
            contentKeyConstant[2]  = 0x46;
            contentKeyConstant[3]  = 0xA0;
            contentKeyConstant[4]  = 0x61;
            contentKeyConstant[5]  = 0x1A;
            contentKeyConstant[6]  = 0x50;
            contentKeyConstant[7]  = 0x1A;
            contentKeyConstant[8]  = 0xC3;
            contentKeyConstant[9]  = 0x2B;
            contentKeyConstant[10] = 0xFA;
            contentKeyConstant[11] = 0xD4;
        }
        else if (extEmiCopyFreeEpnAsserted == aExtendedEmi)
        {
            contentKeyConstant[0]  = 0xDE;
            contentKeyConstant[1]  = 0x66;
            contentKeyConstant[2]  = 0x5A;
            contentKeyConstant[3]  = 0xDE;
            contentKeyConstant[4]  = 0xC0;
            contentKeyConstant[5]  = 0x15;
            contentKeyConstant[6]  = 0x61;
            contentKeyConstant[7]  = 0x12;
            contentKeyConstant[8]  = 0xCF;
            contentKeyConstant[9]  = 0x26;
            contentKeyConstant[10] = 0x28;
            contentKeyConstant[11] = 0xEB;
        }
        else if (extEmiCopyFree == aExtendedEmi)
        {
            // Error: No content key for copy free
            returnValue = INVALID_ARGUMENT;
        }
        else
        {
            returnValue = INVALID_ARGUMENT;
        }
        DEBUG_MSG(MSG_DEBUG, ("EMI: %d\n", aExtendedEmi));
    }

    // Create cipher context
    if (IS_SUCCESS(returnValue))
    {
        memcpy(keyGenBuffer,
               aExchangeKey,
               DTCP_EXCHANGE_KEY_SIZE);
        memcpy(keyGenBuffer + DTCP_EXCHANGE_KEY_SIZE,
               contentKeyConstant, 
               DTCP_CONTENT_KEY_CONSTANT_SIZE);
        memcpy(keyGenBuffer + DTCP_EXCHANGE_KEY_SIZE + DTCP_CONTENT_KEY_CONSTANT_SIZE,
               aNonce,
               DTCP_CONTENT_KEY_NONCE_SIZE);

        if (ippStsNoErr == ippsRijndael128BufferSize(&cipherContextSize))
        {
            cipherContext = (IppsRijndael128 *)malloc(cipherContextSize);
            if (!cipherContext)
            {
                returnValue = FAILURE;
            }
        }
        else
        {
            returnValue = FAILURE;
        }
    }

    // Initialize cipher
    if (IS_SUCCESS(returnValue))
    {
        if (ippStsNoErr != ippsRijndael128Init(keyGenBuffer, IppsRijndaelKey128, cipherContext))
        {
            returnValue = FAILURE;
        }
    }

    // Create the content key
    if (IS_SUCCESS(returnValue))
    {
        if (ippStsNoErr == ippsRijndael128EncryptECB(keyGenBuffer + DTCP_AES_BLOCK_SIZE,
                                                     aCipherKey,
                                                     DTCP_IP_CONTENT_KEY_SIZE,
                                                     cipherContext,
                                                     IppsCPPaddingNONE))
        {
            for (i = 0; i < DTCP_AES_KEY_SIZE; ++i)
            {
                unsigned char *y0 = keyGenBuffer + DTCP_AES_BLOCK_SIZE;
                aCipherKey[i] ^= y0[i];
            }
#ifdef DEMO_MODE
            DEBUG_MSG(MSG_DEBUG, ("\r\ncccccccccccccccccccccccccccccccccccccccccccccc\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    "cc           COMPUTED CONTENT KEY           cc\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    "cccccccccccccccccccccccccccccccccccccccccccccc\r\n"));
#endif
#if !defined(DEMO_MODE)
            DEBUG_MSG(MSG_DEBUG, ("Content Key: "));
            DEBUG_BUF(MSG_DEBUG, aCipherKey, DTCP_IP_CONTENT_KEY_SIZE);
#endif
        }
        else
        {
            returnValue = FAILURE;
        }
    }

    // Create the IV payload buffer
    if (IS_SUCCESS(returnValue))
    {
        ivConstant[0] = 0x95;
        ivConstant[1] = 0xDC;
        ivConstant[2] = 0x3A;
        ivConstant[3] = 0x44;
        ivConstant[4] = 0x90;
        ivConstant[5] = 0x28;
        ivConstant[6] = 0xEB;
        ivConstant[7] = 0x3C;
        memcpy(ivGenBuffer, ivConstant, DTCP_IV_CONSTANT_SIZE);
        memcpy(ivGenBuffer + DTCP_IV_CONSTANT_SIZE, aNonce, DTCP_CONTENT_KEY_NONCE_SIZE);
    }

    // Reinitialize the cipher for generating the IV
    if (IS_SUCCESS(returnValue))
    {
        if (ippStsNoErr != ippsRijndael128Init(aCipherKey, IppsRijndaelKey128, cipherContext))
        {
            returnValue = FAILURE;
        }
    }

    // Compute the IV
    if (IS_SUCCESS(returnValue))
    {
        if (ippStsNoErr == ippsRijndael128EncryptECB(ivGenBuffer,
                                                     aCipherIv,
                                                     DTCP_CONTENT_KEY_NONCE_SIZE + DTCP_IV_CONSTANT_SIZE,
                                                     cipherContext,
                                                     IppsCPPaddingNONE))
        {
#ifndef DEMO_MODE
            DEBUG_MSG(MSG_DEBUG, ("IV: "));
            DEBUG_BUF(MSG_DEBUG, aCipherIv, DTCP_AES_IV_SIZE);
#endif
        }
        else
        {
            returnValue = FAILURE;
        }
    }


    if (cipherContext)
    {
        free(cipherContext);
    }

    return returnValue;
} // CreateContentKey

int DtcpContent_OpenStream(EnStreamTransport            aStreamTransport,
                           DtcpStreamHandle            *aStreamHandle)
{
    int returnValue = SUCCESS;
    DtcpStreamData *streamData = NULL;

    if (!aStreamHandle)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        streamData = (DtcpStreamData *)malloc(sizeof(DtcpStreamData));
        if (streamData)
        {
            streamData->StreamTransport       = aStreamTransport;
        }
        else
        {
            returnValue = FAILURE;
        }
    }

    if (IS_SUCCESS(returnValue))
    {
        if (streamTransportRtp == aStreamTransport)
        {
            returnValue = DtcpAkeCore_OpenStream(TRUE,
                                                 &streamData->AkeCoreStreamHandle);
        }
        else
        {
            returnValue = DtcpAkeCore_OpenStream(FALSE,
                                                 &streamData->AkeCoreStreamHandle);
            Rng_GetRandomNumber(streamData->HttpNonce, DTCP_CONTENT_KEY_NONCE_SIZE);
        }
    }

    if (IS_SUCCESS(returnValue))
    {
        *aStreamHandle = (DtcpStreamHandle)streamData;                
    }
    else
    {
        if (streamData)
        {
            free(streamData);
        }
    }

    return returnValue;
} // DtcpContent_OpenStream

int DtcpContent_CloseStream(DtcpStreamHandle aStreamHandle)
{
    int returnValue = SUCCESS;
    DtcpStreamData *streamData = NULL;

    if (!aStreamHandle)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        streamData = (DtcpStreamData *)aStreamHandle;

        returnValue = DtcpAkeCore_CloseStream(streamData->AkeCoreStreamHandle);

        free(streamData);
    }

    return returnValue;
} // DtcpContent_CloseStream

int DtcpContent_ClosePacket(DtcpPacketHandle aPacketHandle)
{
    int returnValue = SUCCESS;
    DtcpPacketData *data = (DtcpPacketData *)aPacketHandle;

    if (data)
    {
        if (data->CipherContext)
        {
            free(data->CipherContext);
        }
        free(data);
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // DtcpContent_ClosePacket

int DtcpContent_CreatePacketHeader(DtcpStreamHandle  aStreamHandle,
                                   EnExtendedEmi     aExtendedEmi,
                                   unsigned int      aContentSize,
                                   unsigned char     aPacketHeader[DTCP_CONTENT_PACKET_HEADER_SIZE],
                                   DtcpPacketHandle *aPacketHandle)
{
    int returnValue = SUCCESS;
    DtcpStreamData *streamData = NULL;
    DtcpPacketData *data = NULL;
    int exchangeKeyLabel;
    int cipherContextSize = 0;
    unsigned char cipherKey[DTCP_AES_KEY_SIZE];
    unsigned char exchangeKey[DTCP_EXCHANGE_KEY_SIZE];
    unsigned char nonce[DTCP_CONTENT_KEY_NONCE_SIZE];
    unsigned char cipherIv[DTCP_AES_IV_SIZE];

    // Check arguments
    if (!aStreamHandle || !aPacketHeader || !aPacketHandle || !aContentSize)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        streamData = (DtcpStreamData *)aStreamHandle;

        // Create memory for packet data
        data = (DtcpPacketData *)malloc(sizeof(DtcpPacketData));
        if (!data)
        {
            returnValue = FAILURE;
        }
    }

    // Create cipher context
    if (IS_SUCCESS(returnValue))
    {
        if (ippStsNoErr == ippsRijndael128BufferSize(&cipherContextSize))
        {
            data->CipherContext = (IppsRijndael128 *)malloc(cipherContextSize);
            if (!data->CipherContext)
            {
                returnValue = FAILURE;
            }
        }
        else
        {
            returnValue = FAILURE;
        }
    }

    // Save the content size
    if (IS_SUCCESS(returnValue))
    {
        data->BytesProcessed = 0;
        if (DTCP_MAXIMUM_PROTECTED_PACKET_SIZE >= aContentSize)
        {
            data->PayloadSize = aContentSize;
            if (0 != data->PayloadSize % DTCP_AES_BLOCK_SIZE)
            {
                data->PayloadSize += DTCP_AES_BLOCK_SIZE - (data->PayloadSize % DTCP_AES_BLOCK_SIZE);
            }
        }
        else
        {
            returnValue = CONTENT_SIZE_TOO_LARGE;
        }
    }

    // Get the exchange key
    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpAkeCore_GetSourceExchangeKey(exchKeyIdAes128,
                                                       exchangeKey,
                                                       &exchangeKeyLabel);

#ifdef DEMO_MODE && 0
        DEBUG_MSG(MSG_DEBUG, ("\r\neeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    "ee    COMPUTED EXCHANGE KEY    ee\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\r\n"));
#endif
#if !defined(DEMO_MODE)
        DEBUG_MSG(MSG_DEBUG, ("Exchange Key: "));
        DEBUG_BUF(MSG_DEBUG, exchangeKey, DTCP_EXCHANGE_KEY_SIZE);
#endif
    }

    // Retrieve the appropriate nonce
    if (IS_SUCCESS(returnValue))
    {
        if (streamTransportRtp == streamData->StreamTransport)
        {
            DtcpAkeCore_GetRealTimeNonce(nonce);
        }
        else if (streamTransportHttp == streamData->StreamTransport)
        {
            memcpy(nonce, streamData->HttpNonce, DTCP_CONTENT_KEY_NONCE_SIZE);
            IncrementNonce(streamData->HttpNonce);
        }

        // Create cipher key
        returnValue = CreateContentKey(aExtendedEmi, exchangeKey, nonce, cipherKey, cipherIv);
    }

    if (IS_SUCCESS(returnValue))
    {
        memcpy(data->NextIv, cipherIv, DTCP_AES_IV_SIZE);
    }

    // Initialize cipher
    if (IS_SUCCESS(returnValue))
    {
        if (ippStsNoErr != ippsRijndael128Init(cipherKey, IppsRijndaelKey128, data->CipherContext))
        {
            returnValue = FAILURE;
        }
    }

    if (IS_SUCCESS(returnValue))
    {
        // Populate packet header
        memset(aPacketHeader, 0, DTCP_CONTENT_PACKET_HEADER_SIZE);

        aPacketHeader[0] = aPacketHeader[0] | aExtendedEmi;
        aPacketHeader[1] = exchangeKeyLabel & 0xFF;

        memcpy(&aPacketHeader[2], nonce, DTCP_CONTENT_KEY_NONCE_SIZE);

        aPacketHeader[10] = (aContentSize & 0xFF000000) >> 24;
        aPacketHeader[11] = (aContentSize & 0x00FF0000) >> 16;
        aPacketHeader[12] = (aContentSize & 0x0000FF00) >> 8;
        aPacketHeader[13] = (aContentSize & 0x000000FF);
    }

    if (IS_SUCCESS(returnValue))
    {
        *aPacketHandle = data;
    }
    else
    {
        if (data)
        {
            if (data->CipherContext)
            {
                free (data->CipherContext);
            }
            free(data);
        }

        if (0 != aPacketHandle)
        {
            *aPacketHandle = 0;
        }
    }

    return returnValue;
} // DtcpContent_CreatePacketHeader

int DtcpContent_ConsumePacketHeader(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                                    unsigned char            aPacketHeader[DTCP_CONTENT_PACKET_HEADER_SIZE],
                                    EnExtendedEmi           *aExtendedEmi,
                                    unsigned int            *aContentSize,
                                    DtcpPacketHandle        *aPacketHandle)
{
    int returnValue = SUCCESS;
    DtcpPacketData *data = NULL;
    unsigned int exchKeyLabel = 0;
    unsigned int packetExchKeyLabel = 0;
    int cipherContextSize = 0;
    unsigned char cipherKey[DTCP_AES_KEY_SIZE];
    unsigned char exchangeKey[DTCP_EXCHANGE_KEY_SIZE];
    unsigned char nonce[DTCP_CONTENT_KEY_NONCE_SIZE];
    unsigned char cipherIv[DTCP_AES_IV_SIZE];

    // Check arguments
    if ((0 == aAkeCoreSessionHandle) ||
        (!aPacketHeader)             ||
        (!aExtendedEmi)              ||
        (!aContentSize)              ||
        (!aPacketHandle))
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        // Allocate memory for packet data
        data = (DtcpPacketData *)malloc(sizeof(DtcpPacketData));
        if (data)
        {
            memset(data, 0, sizeof(DtcpPacketData));
        }
        else
        {
            returnValue = FAILURE;
        }
    }

    if (IS_SUCCESS(returnValue))
    {
        // Check exchange key label
        packetExchKeyLabel = aPacketHeader[1] & 0xFF;
    }

    // Extract info from packet header
    if (IS_SUCCESS(returnValue))
    {
        // Initialize bytes processed
        data->BytesProcessed = 0;

        // Get extended EMI
        *aExtendedEmi = aPacketHeader[0] & 0x0F;

        // Get content size
        *aContentSize = ((aPacketHeader[10] << 24) |
                        (aPacketHeader[11] << 16) |
                        (aPacketHeader[12] << 8) |
                        (aPacketHeader[13]));

        // Store packet size
        data->PayloadSize = *aContentSize;
        if (0 != data->PayloadSize % DTCP_AES_BLOCK_SIZE)
        {
            data->PayloadSize += DTCP_AES_BLOCK_SIZE - (data->PayloadSize % DTCP_AES_BLOCK_SIZE);
        }

        if (DTCP_MAXIMUM_PROTECTED_PACKET_SIZE < data->PayloadSize)
        {
            returnValue = INVALID_PACKET_HEADER;
        }
    }

    // Get the exchange key and make sure exchange keys match
    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpAkeCore_GetSinkExchangeKey(aAkeCoreSessionHandle, 
                                                     exchKeyIdAes128,
                                                     exchangeKey,
                                                     &exchKeyLabel);

#ifdef DEMO_MODE && 0
            DEBUG_MSG(MSG_DEBUG, ("\r\neeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    "ee    COMPUTED EXCHANGE KEY    ee\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\r\n"));
#endif
#if !defined(DEMO_MODE)
        DEBUG_MSG(MSG_DEBUG, ("Exchange Key: "));
        DEBUG_BUF(MSG_DEBUG, exchangeKey, DTCP_EXCHANGE_KEY_SIZE);
#endif
        if (packetExchKeyLabel != exchKeyLabel)
        {
            DEBUG_MSG(MSG_WARN, ("Packet exchange key label: %d, My exchange key label: %d\n", packetExchKeyLabel, exchKeyLabel));
            returnValue = INVALID_EXCHANGE_KEY_LABEL;
        }
    }

    // Create cipher context
    if (IS_SUCCESS(returnValue))
    {
        // Get cipher context size
        if (ippStsNoErr == ippsRijndael128BufferSize(&cipherContextSize))
        {
            // Create cipher context
            data->CipherContext = (IppsRijndael128 *)malloc(cipherContextSize);
            if (data->CipherContext)
            {
                memset(data->CipherContext, 0, cipherContextSize);
            }
            else
            {
                returnValue = FAILURE;
            }
        }
        else
        {
            returnValue = FAILURE;
        }
    }

    // Initialze IV
    if (IS_SUCCESS(returnValue))
    {
        memcpy(nonce, &aPacketHeader[2], DTCP_CONTENT_KEY_NONCE_SIZE);
    }

    // Create cipher key
    if (IS_SUCCESS(returnValue))
    {
        returnValue = CreateContentKey(*aExtendedEmi, exchangeKey, nonce, cipherKey, cipherIv);
    }

    if (IS_SUCCESS(returnValue))
    {
        memcpy(data->NextIv, cipherIv, DTCP_AES_IV_SIZE);
    }
    
    // Initialize cipher
    if (IS_SUCCESS(returnValue))
    {
        if (ippStsNoErr != ippsRijndael128Init(cipherKey, IppsRijndaelKey128, data->CipherContext))
        {
            returnValue = FAILURE;
        }
    }
    
    // Assign packet handle
    if (IS_SUCCESS(returnValue))
    {
        *aPacketHandle = data;
    }
    else
    {
        // Clean up due to error
        if (data)
        {
            if (data->CipherContext)
            {
                free(data->CipherContext);
            }
            free(data);
        }

        *aPacketHandle = 0;
    }

    return returnValue;
} // DtcpContent_ConsumePacketHeader

int DtcpContent_EncryptData(DtcpPacketHandle  aPacketHandle, 
                            unsigned char    *aInBuffer,
                            unsigned char    *aOutBuffer,
                            unsigned int      aBufferLength)
{
    int returnValue = SUCCESS;
    DtcpPacketData *data = NULL;

    // Check arguments
    if ((0 == aPacketHandle) || 
        (NULL == aInBuffer)  ||
        (NULL == aOutBuffer) ||
        (0 == aBufferLength))
    {
        returnValue = INVALID_ARGUMENT;
    }

    // Check buffer size (must be multiple of DTCP_AES_BLOCK_SIZE)
    if (SUCCESS == returnValue)
    {
        if (0 != aBufferLength % DTCP_AES_BLOCK_SIZE)
        {
            returnValue = INVALID_BUFFER_LENGTH;
        }
    }

    // Check that buffer does not exceed length of payload for this packet
    if (SUCCESS == returnValue)
    {
        data = (DtcpPacketData *)aPacketHandle;
        if (data->PayloadSize < data->BytesProcessed + aBufferLength)
        {
            returnValue = CONTENT_EXCEEDS_PACKET_SIZE;
        }
    }

    // Encrypt
    if (SUCCESS == returnValue)
    {
        // Encrypt data
        if (ippStsNoErr == ippsRijndael128EncryptCBC(aInBuffer, 
                                                    aOutBuffer,
                                                    aBufferLength,
                                                    data->CipherContext,
                                                    data->NextIv,
                                                    IppsCPPaddingNONE))
        {
            // Save next IV
            memcpy(data->NextIv, &aOutBuffer[aBufferLength - DTCP_AES_IV_SIZE], DTCP_AES_IV_SIZE);

            // Update number of bytes processed
            data->BytesProcessed += aBufferLength;
        }
        else
        {
            returnValue = FAILURE;
        }
    }

    return returnValue;
} // DtcpContent_EncryptData


int DtcpContent_DecryptData(DtcpPacketHandle  aPacketHandle, 
                            unsigned char    *aInBuffer,
                            unsigned char    *aOutBuffer,
                            unsigned int      aBufferLength)
{
    int returnValue = SUCCESS;
    DtcpPacketData *data = NULL;
    unsigned char tempIv[DTCP_AES_IV_SIZE];

    // Check arguments
    if ((0 == aPacketHandle) || 
        (NULL == aInBuffer)  ||
        (NULL == aOutBuffer))
    {
        returnValue = INVALID_ARGUMENT;
    }

    // Check buffer size (must be multiple of DTCP_AES_BLOCK_SIZE)
    if (SUCCESS == returnValue)
    {
        if (0 != aBufferLength % DTCP_AES_BLOCK_SIZE)
        {
            returnValue = INVALID_BUFFER_LENGTH;
        }
    }

    // Check that buffer does not exceed length of payload for this packet
    if (SUCCESS == returnValue)
    {
        data = (DtcpPacketData *)aPacketHandle;
        if (data->PayloadSize < data->BytesProcessed + aBufferLength)
        {
            returnValue = CONTENT_EXCEEDS_PACKET_SIZE;
        }
    }

    // Decrypt
    if (SUCCESS == returnValue)
    {
        // Decrypt data
        memcpy(tempIv, &aInBuffer[aBufferLength - DTCP_AES_IV_SIZE], DTCP_AES_IV_SIZE);
        if (ippStsNoErr == ippsRijndael128DecryptCBC(aInBuffer, 
                                                     aOutBuffer,
                                                     aBufferLength,
                                                     data->CipherContext,
                                                     data->NextIv,
                                                     IppsCPPaddingNONE))
        {
            // Save next IV
            memcpy(data->NextIv, tempIv, DTCP_AES_IV_SIZE);

            // Update number of processed bytes
            data->BytesProcessed += aBufferLength;
        }
        else
        {
            returnValue = FAILURE;
        }
    }

    return returnValue;
} // DtcpContent_DecryptData
