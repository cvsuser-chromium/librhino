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
#include "DtcpAkeResponse.h"
#include "DtcpAkeCore.h"
#include "DtcpAkeCoreInternal.h"
#include "DtcpCert.h"
#include "DtcpCore.h"
#include "DtcpSrm.h"
#include "DtcpStatusCodes.h"

/// \file 
/// \brief Implements functions for creating and consuming data for the response command of the AKE
///
/// This header file is part of the \ref DtcpAkeCore library.

#if 0
static int GlobalDisplayLevel             = 0;
static int GlobalDisplayLevelMatchExactly = 0;
static int GlobalLogToFile                = 0;
#endif

int DtcpResponse_CreateFullAuthData(DtcpAkeCoreSessionData *aAkeSessionData,
                                    unsigned char           aResponseCmdBuffer[])
{
    int returnValue = SUCCESS;
    unsigned char responseBufferToSign[DTCP_FULL_AUTH_RESPONSE_SIGN_BUFFER_SIZE];
    unsigned char *currentPtr = responseBufferToSign;
    int ourSrmVersionNumber = 0;
    int ourSrmPartCount = 0;
    unsigned char *ourSrm;
    unsigned int   ourSrmSize;
    int deviceRevokedFlag = FALSE;

    if (deviceModeSource == aAkeSessionData->DeviceMode)
    {
        DtcpCore_GetFirstPhaseValue(aAkeSessionData->FirstPhaseValue,
                                    aAkeSessionData->FirstPhaseSecret);
    }

    DEBUG_MSG(MSG_DEBUG+5, ("First Phase Value: \n"));
    DEBUG_BUF(MSG_DEBUG+5, aAkeSessionData->FirstPhaseValue, DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE);
    
    DEBUG_MSG(MSG_DEBUG+5, ("First Phase Secret: \n"));
    DEBUG_BUF(MSG_DEBUG+5, aAkeSessionData->FirstPhaseSecret, DTCP_DH_FIRST_PHASE_SECRET_SIZE);

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpCore_GetSrm(&ourSrm, &ourSrmSize);
    }

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpSrm_IsDeviceRevoked(ourSrm,
                                              ourSrmSize,
                                              aAkeSessionData->DeviceId,
                                              &deviceRevokedFlag);
        if (IS_SUCCESS(returnValue) && (TRUE == deviceRevokedFlag))
        {
            returnValue = DEVICE_REVOKED;
            DEBUG_MSG(MSG_INFO, ("Device revoked in SRM.  AKE Aborted.\r\n") );
        }
    }

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpSrm_GetSrmVersion(ourSrm, ourSrmSize, &ourSrmVersionNumber);        
    }

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpSrm_GetSrmPartCount(ourSrm, ourSrmSize, &ourSrmPartCount);
    }

    // Compute signature
    memset(currentPtr, 0, DTCP_FULL_AUTH_RESPONSE_SIGN_BUFFER_SIZE);

    memcpy(currentPtr, aAkeSessionData->TheirNonce, DTCP_FULL_AUTH_NONCE_SIZE);
    currentPtr += DTCP_FULL_AUTH_NONCE_SIZE;
    memcpy(currentPtr,
           aAkeSessionData->FirstPhaseValue,
           DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE);
    currentPtr += DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE;

    currentPtr[0] = 0xFF & (ourSrmVersionNumber >> 8);
    currentPtr[1] = 0xFF & (ourSrmVersionNumber);

    currentPtr += DTCP_SRM_VERSION_NUMBER_SIZE;
    currentPtr[0] = 0x0F & (ourSrmPartCount);

    DtcpCore_SignData(responseBufferToSign,
                      DTCP_FULL_AUTH_RESPONSE_SIGN_BUFFER_SIZE,
                      aResponseCmdBuffer + DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE + DTCP_SRM_VERSION_NUMBER_SIZE + 1);

    // Copy first phase value
    memcpy(aResponseCmdBuffer,
           responseBufferToSign + DTCP_FULL_AUTH_NONCE_SIZE,
           DTCP_FULL_AUTH_RESPONSE_SIGN_BUFFER_SIZE - DTCP_FULL_AUTH_NONCE_SIZE);

    return returnValue;
} // DtcpResponse_CreateFullAuthData

int DtcpResponse_ConsumeFullAuthData(DtcpAkeCoreSessionData *aAkeSessionData,
                                     unsigned char           aResponseCmdBuffer[],                                     
                                     unsigned int           *aOurSrmUpdateRequiredFlag)
{
    int returnValue = SUCCESS;
    unsigned char responseBufferToVerify[DTCP_FULL_AUTH_RESPONSE_SIGN_BUFFER_SIZE];
    unsigned char *currentPtr = responseBufferToVerify;
    unsigned char *ourSrm = 0;
    unsigned int   ourSrmSize = 0;
    int i;
    int deviceIdFound = 0;

    // Verify response data
    memset(currentPtr, 0, DTCP_FULL_AUTH_RESPONSE_SIGN_BUFFER_SIZE);

    memcpy(currentPtr, aAkeSessionData->OurNonce, DTCP_FULL_AUTH_NONCE_SIZE);
    currentPtr += DTCP_FULL_AUTH_NONCE_SIZE;
    memcpy(currentPtr, 
           aResponseCmdBuffer, 
           DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE + DTCP_SRM_VERSION_NUMBER_SIZE + 1);
    
    if (SUCCESS != DtcpCore_VerifyData(responseBufferToVerify,
                                       DTCP_FULL_AUTH_RESPONSE_SIGN_BUFFER_SIZE,
                                       aResponseCmdBuffer + 
                                          DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE + 
                                          DTCP_SRM_VERSION_NUMBER_SIZE +
                                       1,
                                       aAkeSessionData->TheirPublicKey))
    {
        returnValue = DTCP_OTHER_DEVICE_RESPONSE_DATA_INVALID;
    }

    if (IS_SUCCESS(returnValue))
    {
        aAkeSessionData->TheirSrmV = aResponseCmdBuffer[DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE] << 8 |
                                     aResponseCmdBuffer[DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE + 1];

        aAkeSessionData->TheirSrmC = aResponseCmdBuffer[DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE + 2] & 0x0F;

        returnValue = CheckSinkCountLimit(aAkeSessionData->AkeCoreData, 
                                          aAkeSessionData->DeviceId,
                                          aAkeSessionData->TheirApFlag);

        if (DTCP_SINK_COUNT_LIMIT_REACHED == returnValue)
        {
            returnValue = aAkeSessionData->AkeCoreData->ExchangeKeyUpdateRequestFunc();
            if (IS_FAILURE(returnValue))
            {
                returnValue = DTCP_SINK_COUNT_LIMIT_REACHED;
            }
        }

    }

    if (IS_SUCCESS(returnValue))
    {
        // Insert device id into list if it is not already present or if the AP flag
        // is set
        if (aAkeSessionData->TheirApFlag)
        {
            deviceIdFound = 0;
        }
        else
        {
            for(i = 0; i < DTCP_SINK_COUNT_LIMIT; ++i)
            {
                if (0 == memcmp(aAkeSessionData->DeviceId, 
                                aAkeSessionData->AkeCoreData->AuthenticatedSinkDevices[i], 
                                DTCP_DEVICE_ID_SIZE))
                {
                    deviceIdFound = 1;
                    break;
                }
            }
        }

        if (!deviceIdFound)
        {
            OsWrap_SemaphoreWait(aAkeSessionData->AkeCoreData->DataSemaphore);

            memcpy(aAkeSessionData->AkeCoreData->AuthenticatedSinkDevices[aAkeSessionData->AkeCoreData->AuthenticatedSinkDeviceCount],
                   aAkeSessionData->DeviceId,
                   DTCP_DEVICE_ID_SIZE);
        
            aAkeSessionData->AkeCoreData->AuthenticatedSinkDeviceCount++;

            OsWrap_SemaphorePost(aAkeSessionData->AkeCoreData->DataSemaphore);
        }
    }

    if (SUCCESS == returnValue)
    {
        // Compute Kauth
        DtcpCore_GetSharedSecret(aResponseCmdBuffer,
                                 aAkeSessionData->FirstPhaseSecret,
                                 aAkeSessionData->AuthKey);
        aAkeSessionData->AuthenticatedFlag = 1;

#if defined(DEMO_MODE)
        DEBUG_MSG(MSG_DEBUG, ("\r\naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    "aa   COMPUTED AUTHENTICATION KEY   aa\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n"));
#else
        DEBUG_MSG(MSG_DEBUG, ("AuthKey: \n"));
        DEBUG_BUF(MSG_DEBUG, aAkeSessionData->AuthKey, DTCP_AUTH_KEY_SIZE);
#endif
    }

    *aOurSrmUpdateRequiredFlag = FALSE;
    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpCore_GetSrm(&ourSrm,
                                      &ourSrmSize);

        if (IS_SUCCESS(returnValue))
        {
            returnValue = DtcpSrm_IsOurSrmCurrent(ourSrm,
                                                ourSrmSize,
                                                aAkeSessionData->TheirSrmG,
                                                aAkeSessionData->TheirSrmV,
                                                aAkeSessionData->TheirSrmC,
                                                aOurSrmUpdateRequiredFlag);
        }
    }

    return returnValue;
} // DtcpResponse_ConsumeFullAuthData
