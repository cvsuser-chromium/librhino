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
#include "Rng.h"
#include "DtcpStatusCodes.h"
#include "ippcp.h"

/// \file 
/// \brief Implementation file for the \ref Rng library.
///
/// This file implements the interface for the \ref Rng library.

/// \brief Internal data structure for the \ref Rng library.
typedef struct {
    unsigned char     RngSeed[DTCP_AES_BLOCK_SIZE]; ///< Random number generator seed
    UpdateRngSeed_Ptr UpdateRngSeedFunc;            ///< Pointer to a function to update the seed
} RngData;

static RngData gRngData;    ///< Global, static data for the library

int Rng_Initialize(unsigned char aRngSeed[DTCP_AES_BLOCK_SIZE], UpdateRngSeed_Ptr aUpdateRngSeedFunc)
{
    int returnValue = SUCCESS;

    memset(&gRngData.RngSeed, 0, DTCP_AES_BLOCK_SIZE);
    gRngData.UpdateRngSeedFunc = 0;

    if (aRngSeed && aUpdateRngSeedFunc)
    {
        memcpy(&gRngData.RngSeed, aRngSeed, DTCP_AES_BLOCK_SIZE);
        gRngData.UpdateRngSeedFunc = aUpdateRngSeedFunc;
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // Rng_Initialize

int Rng_GetRandomNumber(unsigned char *aRngBuffer, unsigned int aRngSize)
{
    int returnValue = SUCCESS;
    IppsRijndael128 *cipherContext = 0;
    int cipherContextSize = 0;
    unsigned int i;
    unsigned char plainText[DTCP_AES_BLOCK_SIZE];
    unsigned char cipherText[DTCP_AES_BLOCK_SIZE];

    if (gRngData.UpdateRngSeedFunc != 0)
    {
        if (aRngBuffer)
        {
            // Initialize cipher using seed value as the key
            if (ippStsNoErr == ippsRijndael128BufferSize(&cipherContextSize))
            {
                cipherContext = (IppsRijndael128 *)malloc(cipherContextSize);
                if (cipherContext)
                {
                    if (ippStsNoErr != ippsRijndael128Init(gRngData.RngSeed, IppsRijndaelKey128, cipherContext))
                    {
                        returnValue = FAILURE;
                    }
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

            if (IS_SUCCESS(returnValue))
            {
                // encrypt buffer to create random data
                for (i = 0; i < aRngSize; ++i)
                {
                    ippsRijndael128EncryptECB(plainText,
                                              cipherText,
                                              1,
                                              cipherContext,
                                              IppsCPPaddingZEROS);

                    plainText[0] ^= cipherText[0];
                    plainText[1] ^= cipherText[1];

                    cipherText[0] = cipherText[0] ^ cipherText[1] ^ cipherText[2] ^ cipherText[3] ^
                                    cipherText[4] ^ cipherText[5] ^ cipherText[6] ^ cipherText[7];
                    aRngBuffer[i] = cipherText[0];
                }

                // Create next seed value
                ippsRijndael128EncryptECB(plainText,
                                            cipherText,
                                            1,
                                            cipherContext,
                                            IppsCPPaddingZEROS);
                cipherText[0] = cipherText[0] ^ cipherText[1] ^ cipherText[2] ^ cipherText[3] ^
                                cipherText[4] ^ cipherText[5] ^ cipherText[6] ^ cipherText[7];
                memcpy(gRngData.RngSeed, cipherText, DTCP_AES_BLOCK_SIZE);

                // Update seed
                gRngData.UpdateRngSeedFunc((unsigned char *)gRngData.RngSeed, DTCP_AES_BLOCK_SIZE);
            }
        }
        else
        {
            returnValue = INVALID_ARGUMENT;
        }
    }
    else
    {
        returnValue = NOT_INITIALIZED;
    }

    if (cipherContext)
    {
        free(cipherContext);
    }
    return returnValue;
} // Rng_GetRandomNumber
