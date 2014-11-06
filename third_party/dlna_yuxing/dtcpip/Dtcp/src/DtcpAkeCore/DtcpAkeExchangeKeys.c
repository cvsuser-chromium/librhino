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
#include "DtcpAkeExchangeKeys.h"
#include "DtcpExchangeKeys.h"
#include "DtcpStatusCodes.h"

/// \file 
/// \brief Implements functions for creating and consuming data for the exchange key command of the AKE
///
/// This header file is part of the \ref DtcpAkeCore library.

int DtcpAkeExchKeys_CreateExchKeyData(DtcpAkeCoreSessionData *aAkeSessionData,
                                      EnExchKeyId             aExchKeyId,
                                      unsigned char           aExchKeyBuffer[DTCP_EXCHANGE_KEY_CMD_DATA_SIZE])
{
    int returnValue = SUCCESS;
    unsigned char localExchKey[DTCP_EXCHANGE_KEY_SIZE];
    int index = 0;
    int exchKeyLabel = 0;

    if(deviceModeSource == aAkeSessionData->DeviceMode)
    {
        if (aAkeSessionData->AuthenticatedFlag)
        {
            if (!DtcpExchKeys_IsInitialized(&aAkeSessionData->AkeCoreData->ExchangeKeyData))
            {
                OsWrap_SemaphoreWait(aAkeSessionData->AkeCoreData->DataSemaphore);

                returnValue = DtcpExchKeys_CreateExchKeys(&aAkeSessionData->AkeCoreData->ExchangeKeyData);

                OsWrap_SemaphorePost(aAkeSessionData->AkeCoreData->DataSemaphore);
            }
            returnValue = DtcpExchKeys_GetExchKey(&aAkeSessionData->AkeCoreData->ExchangeKeyData, aExchKeyId, localExchKey);
        }
        else
        {
            returnValue = DTCP_INVALID_COMMAND_SEQUENCE;
        }
    }

    if (SUCCESS == returnValue)
    {
        memset(aExchKeyBuffer, 0, DTCP_EXCHANGE_KEY_CMD_DATA_SIZE);
        aExchKeyBuffer[0] = aAkeSessionData->AkeCoreData->ExchangeKeyData.Label & 0xFF;

        if ((exchKeyIdCopyNever         == aExchKeyId) ||
            (exchKeyIdCopyOneGeneration == aExchKeyId) ||
            (exchKeyIdCopyNoMore        == aExchKeyId))
        {
            aExchKeyBuffer[1] = cipherM6 << 4;
        }
        else if (exchKeyIdAes128 == aExchKeyId)
        {
            aExchKeyBuffer[1] = cipherAes128 << 4;
        }
        else
        {
            aExchKeyBuffer[1] = cipherNoInfo << 4;
        }

        // Encrypt key with KAuth
        for (index = 0; index < DTCP_EXCHANGE_KEY_SIZE; ++index)
        {
            ///TODO define constant for offset into exchange key buffer
            aExchKeyBuffer[index + 2] = localExchKey[index] ^ aAkeSessionData->AuthKey[index];
        }
    }
    else
    {
        returnValue = DTCP_INVALID_DATA_FIELD;
    }
    
    return returnValue;
} // DtcpExchKeys_CreateExchKeyData

int DtcpAkeExchKeys_ConsumeExchKeyData(DtcpAkeCoreSessionData *aAkeSessionData,
                                       EnExchKeyId             aExchKeyId,
                                       unsigned char           aExchKeyBuffer[DTCP_EXCHANGE_KEY_CMD_DATA_SIZE])
{
    int returnValue = SUCCESS;
    unsigned char localExchKey[DTCP_EXCHANGE_KEY_SIZE];
    int index = 0;

    if(deviceModeSink == aAkeSessionData->DeviceMode)
    {
        if (aAkeSessionData->AuthenticatedFlag)
        {
            // Store exchange key label
            DtcpExchKeys_SetExchKeyLabel(&aAkeSessionData->ExchangeKeyData, aExchKeyBuffer[0]);
            //aAkeSessionData->ExchangeKeyData.Label = 0 & aExchKeyBuffer[0];

            // Decrypt exchange key with KAuth
            for (index = 0; index < DTCP_EXCHANGE_KEY_SIZE; ++index)
            {
                localExchKey[index] = aExchKeyBuffer[index + 2] ^ aAkeSessionData->AuthKey[index];
            }

            DtcpExchKeys_SetExchKey(&aAkeSessionData->ExchangeKeyData, aExchKeyId, localExchKey);
        }
        else
        {
            returnValue = DTCP_INVALID_COMMAND_SEQUENCE;
        }
    }
    else
    {
        returnValue = DTCP_INVALID_DATA_FIELD;
    }

    return returnValue;
} // DtcpExchKeys_ConsumeExchKeyData
