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
#include "DtcpExchangeKeys.h"
#include "DtcpStatusCodes.h"
#include "Rng.h"

/// \file
/// \brief Implements functions for storing and using the exchange keys
///
/// This file is part of the \ref DtcpAkeCore library.

EnExchKeyMask ExchKeyIdToMask(EnExchKeyId aExchKeyId)
{
    return (1 << aExchKeyId);
}

int DtcpExchKeys_Startup(DtcpExchKeyData *aExchKeyData)
{
    int returnValue = SUCCESS;

    if (!aExchKeyData)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (SUCCESS == returnValue)
    {
        aExchKeyData->ValidFlag = 0;
        aExchKeyData->Label = 0;
        memset(aExchKeyData->ExchKeys, 0, EXCHANGE_KEY_COUNT * DTCP_EXCHANGE_KEY_SIZE);
        aExchKeyData->Initialized = FALSE;
    }

    return returnValue;
} // DtcpExchKeys_Startup

int DtcpExchKeys_CreateExchKeys(DtcpExchKeyData *aExchKeyData)
{
    int returnValue = SUCCESS;
    int i;

    if (!aExchKeyData)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (SUCCESS == returnValue)
    {
        if (TRUE != aExchKeyData->Initialized)
        {
            aExchKeyData->ValidFlag = 0;
            for (i = 0; i < EXCHANGE_KEY_COUNT; ++i)
            {
                Rng_GetRandomNumber(aExchKeyData->ExchKeys[i], DTCP_EXCHANGE_KEY_SIZE);
                aExchKeyData->ValidFlag |= 1 << i;
            }
            aExchKeyData->Label++;
            aExchKeyData->Initialized = TRUE;
        }
    }

    return returnValue;
} // DtcpExchKeys_CreateExchKeys

int DtcpExchKeys_GetExchKey(DtcpExchKeyData *aExchKeyData,
                            EnExchKeyId aExchKeyId,
                            unsigned char aExchKey[DTCP_EXCHANGE_KEY_SIZE])
{
    int returnValue = SUCCESS;

    if(!aExchKeyData)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (SUCCESS == returnValue)
    {
        if (FALSE == aExchKeyData->Initialized)
        {
            returnValue = DtcpExchKeys_CreateExchKeys(aExchKeyData);
        }
    }

    if (SUCCESS == returnValue)
    {
        if (aExchKeyData->ValidFlag & ExchKeyIdToMask(aExchKeyId))
        {
            memcpy(aExchKey, &aExchKeyData->ExchKeys[aExchKeyId], DTCP_EXCHANGE_KEY_SIZE);
        }
        else
        {
            returnValue = DATA_NOT_AVAILABLE;
        }
    }

    return returnValue;
} // DtcpExchKeys_GetExchKey

int DtcpExchKeys_SetExchKey(DtcpExchKeyData *aExchKeyData,
                            EnExchKeyId aExchKeyId,
                            unsigned char aExchKey[DTCP_EXCHANGE_KEY_SIZE])
{
    int returnValue = SUCCESS;

    if(!aExchKeyData)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (SUCCESS == returnValue)
    {
        memcpy(&aExchKeyData->ExchKeys[aExchKeyId], aExchKey, DTCP_EXCHANGE_KEY_SIZE);
        aExchKeyData->Initialized = TRUE;
        aExchKeyData->ValidFlag |= ExchKeyIdToMask(aExchKeyId);
    }

    return returnValue;
} // DtcpExchKeys_SetExchKey

int DtcpExchKeys_IsExchKeyValid(DtcpExchKeyData *aExchKeyData,
                                EnExchKeyId aExchKeyId)
{
    int isValid = FALSE;
    int returnValue = SUCCESS;

    if (!aExchKeyData)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (SUCCESS == returnValue)
    {
        if (TRUE == aExchKeyData->Initialized)
        {
            if (aExchKeyData->ValidFlag & ExchKeyIdToMask(aExchKeyId))
            {
                isValid = TRUE;
            }
        }
    }

    return isValid;
} // DtcpExchKeys_IsExchKeyValid

int DtcpExchKeys_InvalidateExchKeys(DtcpExchKeyData *aExchKeyData)
{
    int returnValue = SUCCESS;

    if (!aExchKeyData)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (SUCCESS == returnValue)
    {
        if (TRUE == aExchKeyData->Initialized)
        {
            memset(&aExchKeyData->ExchKeys, 0, EXCHANGE_KEY_COUNT * DTCP_EXCHANGE_KEY_SIZE);
            aExchKeyData->ValidFlag = 0;
            aExchKeyData->Initialized = FALSE;
        }
    }

    return returnValue;
} // DtcpExchKeys_InvalidateExchKeys

int DtcpExchKeys_IsInitialized(DtcpExchKeyData *aExchKeyData)
{
    if (NULL != aExchKeyData)
    {
        return (aExchKeyData->Initialized);   
    }
    else
    {
        return 0;
    }
} // DtcpExchKeys_IsInitialized

int DtcpExchKeys_SetExchKeyLabel(DtcpExchKeyData *aExchKeyData,
                                 unsigned int     aExchKeyLabel)
{
    int returnValue = SUCCESS;
    
    if (NULL != aExchKeyData)
    {
        DtcpExchKeys_InvalidateExchKeys(aExchKeyData);
        aExchKeyData->Initialized = 1;
        aExchKeyData->Label = aExchKeyLabel & 0xFF;
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // DtcpExchKeys_SetExchKeyLabel

int DtcpExchKeys_GetExchKeyLabel(DtcpExchKeyData *aExchKeyData,
                                 unsigned int    *aExchKeyLabel)
{
    int returnValue = SUCCESS;
    
    if ((NULL != aExchKeyData) && (NULL != aExchKeyLabel))
    {
        if (TRUE == aExchKeyData->Initialized)
        {
            *aExchKeyLabel = aExchKeyData->Label & 0xFF;
        }
        else
        {
            returnValue = DATA_NOT_INITIALIZED;
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // DtcpExchKeys_GetExchKeyLabel
