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

#include <memory.h>
#include "DtcpAkeRealTimeNonce.h"
#include "DtcpUtils.h"
#include "DtcpStatusCodes.h"
#include "DtcpAkeCoreInternal.h"
#include "Rng.h"

/// \file 
/// \brief Implements functions for running the nonce timer for real time streams
///
/// This header file is part of the \ref DtcpAkeCore library.

#ifdef _MSC_VER
typedef unsigned __int64 int64;
#else /* assuming GCC */
typedef unsigned long long int64;
#endif


static void IncrementNonce(unsigned char aNonce[DTCP_CONTENT_KEY_NONCE_SIZE])
{
    int64 nonce;

    DEBUG_MSG(MSG_INFO, ("DtcpAkeRealTimeNonce::IncrementNonce\r\n"));

    // This implementation assumes a little endian processor and a 64 bit nonce.
    memcpy(&nonce, aNonce, DTCP_CONTENT_KEY_NONCE_SIZE);
    SwapInPlace(&nonce, 8);
    nonce = nonce + 1;
    SwapInPlace(&nonce, 8);
    memcpy(aNonce, &nonce, DTCP_CONTENT_KEY_NONCE_SIZE);
}

int RealTimeNonceTimer(void *aParameter)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreData *coreData;

    if (!aParameter)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        coreData = (DtcpAkeCoreData *)aParameter;

        // Create initial nonce value
        OsWrap_SemaphoreWait(coreData->DataSemaphore);

        Rng_GetRandomNumber(coreData->RealTimeNonce, DTCP_CONTENT_KEY_NONCE_SIZE);

        OsWrap_SemaphorePost(coreData->DataSemaphore);

        while (1)
        {
            // Sleep for timer period
            OsWrap_Sleep(DTCP_RTP_NONCE_TIMER_PERIOD * 1000);

            if (0 < coreData->UpdateRealTimeNonceStreamCount)
            {
                // Increment nonce
                OsWrap_SemaphoreWait(coreData->DataSemaphore);
                IncrementNonce(coreData->RealTimeNonce);
                OsWrap_SemaphorePost(coreData->DataSemaphore);

            }
        }
    }


    return returnValue;
}
