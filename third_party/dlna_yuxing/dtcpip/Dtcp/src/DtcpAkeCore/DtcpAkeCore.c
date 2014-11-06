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
#include "DtcpAkeCore.h"
#include "DtcpAkeCoreInternal.h"
#include "DtcpCore.h"
#include "DtcpSrm.h"
#include "DtcpStatusCodes.h"
#include "DtcpAkeChallenge.h"
#include "DtcpAkeResponse.h"
#include "DtcpAkeExchangeKeys.h"
#include "DtcpAkeRealTimeNonce.h"

/// \file 
/// \brief Implementation file for the \ref DtcpAkeCore library.
///
/// This file implements the interface for the \ref DtcpAkeCore library.

#if 0
static int GlobalDisplayLevel             = 0;
static int GlobalDisplayLevelMatchExactly = 0;
static int GlobalLogToFile                = 0;
#endif

static DtcpAkeCoreData gAkeCoreData; ///< Global, static data for DtcpAkeCore library

int DtcpAkeCore_Startup(ExchangeKeyUpdateRequest_Ptr  aExchangeKeyUpdateRequestFunc)
{
    int returnValue = SUCCESS;

    if (!aExchangeKeyUpdateRequestFunc)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        gAkeCoreData.DataSemaphore = NULL;
        returnValue = OsWrap_SemaphoreInit(&gAkeCoreData.DataSemaphore, 1);
    }

    if (IS_SUCCESS(returnValue))
    {
        OsWrap_SemaphoreWait(gAkeCoreData.DataSemaphore);

        gAkeCoreData.AkeSessions = NULL;
        DtcpExchKeys_Startup(&gAkeCoreData.ExchangeKeyData);

        gAkeCoreData.ExchangeKeyUpdateRequestFunc = aExchangeKeyUpdateRequestFunc;

        gAkeCoreData.AuthenticatedSinkDeviceCount = 0;
        gAkeCoreData.StreamCount = 0;
        gAkeCoreData.UpdateRealTimeNonceStreamCount = 0;
        memset(gAkeCoreData.RealTimeNonce, 0, DTCP_CONTENT_KEY_NONCE_SIZE);

        memset(gAkeCoreData.AuthenticatedSinkDevices, 0, DTCP_SINK_COUNT_LIMIT * DTCP_DEVICE_ID_SIZE);            

        // Start Real Time Nc timer
        returnValue = OsWrap_NewThread(RealTimeNonceTimer, 
                                       &gAkeCoreData,
                                        0,
                                        &gAkeCoreData.RealTimeNonceThreadId,
                                        &gAkeCoreData.RealTimeNonceThreadHandle);

        OsWrap_SemaphorePost(gAkeCoreData.DataSemaphore);
    }

    return returnValue;
} // DtcpAkeCore_Startup

void DtcpAkeCore_Shutdown()
{
    OsWrap_KillThread(gAkeCoreData.RealTimeNonceThreadId, gAkeCoreData.RealTimeNonceThreadHandle);
    OsWrap_DeleteList(&gAkeCoreData.AkeSessions);

    OsWrap_SemaphoreClose(gAkeCoreData.DataSemaphore);
    return;
} // DtcpAkeCore_Shutdown

int DtcpAkeCore_OpenAkeSession(EnAkeTypeId               aAkeTypeId,
                               EnDeviceMode              aDeviceMode,
                               DtcpAkeCoreSessionHandle *aAkeCoreSessionHandle)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreSessionData *sessionData;

    if (aAkeCoreSessionHandle)
    {
        OsWrap_SemaphoreWait(gAkeCoreData.DataSemaphore);

        returnValue = OsWrap_AddToList(&gAkeCoreData.AkeSessions, 
                                       &sessionData,
                                       sizeof(DtcpAkeCoreSessionData));

        OsWrap_SemaphorePost(gAkeCoreData.DataSemaphore);

        if (IS_SUCCESS(returnValue) && NULL != sessionData)
        {
            memset(sessionData, 0, sizeof(DtcpAkeCoreSessionData));
            sessionData->AkeCoreData = &gAkeCoreData;
            sessionData->AkeTypeId = aAkeTypeId;
            sessionData->DeviceMode = aDeviceMode;
            sessionData->TheirSrmV = 0;
            sessionData->TheirSrmG = 0;
            sessionData->TheirSrmC = 0;
            *aAkeCoreSessionHandle = sessionData;
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // DtcpAkeCore_OpenAkeSession

int DtcpAkeCore_CloseAkeSession(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreSessionData *sessionData;

    sessionData = (DtcpAkeCoreSessionData *)aAkeCoreSessionHandle;

    if (0 != sessionData)
    {
        OsWrap_SemaphoreWait(gAkeCoreData.DataSemaphore);

        OsWrap_RemoveFromList(&gAkeCoreData.AkeSessions,
                              sessionData);

        OsWrap_SemaphorePost(gAkeCoreData.DataSemaphore);
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
       
    return returnValue;
} // DtcpAkeCore_CloseAkeSession

int DtcpAkeCore_ReleaseBuffers(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                               EnAkeBufferMask aBufferMask)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreSessionData *sessionData;

    sessionData = (DtcpAkeCoreSessionData *)aAkeCoreSessionHandle;

    if (0 != sessionData)
    {
        if (akeBufferChallenge & aBufferMask)
        {
            if (0 != sessionData->ChallengeCmdBuffer)
            {
                free(sessionData->ChallengeCmdBuffer);
                sessionData->ChallengeCmdBuffer = 0;
            }
            sessionData->ChallengeCmdBufferSize = 0;
        }

        if (akeBufferResponse & aBufferMask)
        {
            if (0 != sessionData->ResponseCmdBuffer)
            {
                free(sessionData->ResponseCmdBuffer);
                sessionData->ResponseCmdBuffer = 0;
            }
            sessionData->ResponseCmdBufferSize = 0;
        }

        if (akeBufferExchKeyCopyOneGen & aBufferMask)
        {
            if (0 != sessionData->ExchKeyCopyOneGenCmdBuffer)
            {
                free(sessionData->ExchKeyCopyOneGenCmdBuffer);
                sessionData->ExchKeyCopyOneGenCmdBuffer = 0;
            }
            sessionData->ExchKeyCopyOneGenCmdBufferSize = 0;
        }

        if (akeBufferExchKeyCopyNoMore & aBufferMask)
        {
            if (0 != sessionData->ExchKeyCopyNoMoreCmdBuffer)
            {
                free(sessionData->ExchKeyCopyNoMoreCmdBuffer);
                sessionData->ExchKeyCopyNoMoreCmdBuffer = 0;
            }
            sessionData->ExchKeyCopyNoMoreCmdBufferSize = 0;
        }

        if (akeBufferExchKeyCopyNever & aBufferMask)
        {
            if (0 != sessionData->ExchKeyCopyNeverCmdBuffer)
            {
                free(sessionData->ExchKeyCopyNeverCmdBuffer);
                sessionData->ExchKeyCopyNeverCmdBuffer = 0;
            }
            sessionData->ExchKeyCopyNeverCmdBufferSize = 0;
        }

        if (akeBufferExchKeyAes128 & aBufferMask)
        {
            if (0 != sessionData->ExchKeyAes128CmdBuffer)
            {
                free(sessionData->ExchKeyAes128CmdBuffer);
                sessionData->ExchKeyAes128CmdBuffer = 0;
            }
            sessionData->ExchKeyAes128CmdBufferSize = 0;
        }

        if (akeBufferSrm & aBufferMask)
        {
            if (0 != sessionData->SrmCmdBuffer)
            {
                free(sessionData->SrmCmdBuffer);
                sessionData->SrmCmdBuffer = 0;
            }
            sessionData->SrmCmdBufferSize = 0;
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    return returnValue;    
} // DtcpAkeCore_ReleaseBuffers

int DtcpAkeCore_CancelAke(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle)
{
    int returnValue = SUCCESS;

    return returnValue;
} // DtcpAkeCore_CancelAke

int DtcpAkeCore_CreateChallengeData(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                                    unsigned char **aChallengeCmdBufferPtr,
                                    unsigned int *aChallengeCmdBufferSize)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreSessionData *sessionData;

    sessionData = (DtcpAkeCoreSessionData *)aAkeCoreSessionHandle;

    if ((NULL != sessionData) && (OsWrap_EntryIsInList(&sessionData->AkeCoreData->AkeSessions, sessionData)))
    {
        if (akeTypeIdFullAuth == sessionData->AkeTypeId)
        {
            sessionData->ChallengeCmdBuffer     = (unsigned char *)malloc(DTCP_FULL_AUTH_CHALLENGE_SIZE);
            sessionData->ChallengeCmdBufferSize = DTCP_FULL_AUTH_CHALLENGE_SIZE;
        }
        //todo Figure out whether other ake types should be supported.

        if (0 != sessionData->ChallengeCmdBuffer)
        {
            returnValue = DtcpChallenge_CreateFullAuthData(aAkeCoreSessionHandle, sessionData->ChallengeCmdBuffer);
            if (IS_SUCCESS(returnValue))
            {
                *aChallengeCmdBufferPtr  = sessionData->ChallengeCmdBuffer;
                *aChallengeCmdBufferSize = sessionData->ChallengeCmdBufferSize;
            }
            else
            {
                *aChallengeCmdBufferPtr  = 0;
                *aChallengeCmdBufferSize = 0;
            }
        }
        else
        {
            sessionData->ChallengeCmdBufferSize = 0;
            returnValue = FAILURE;
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // DtcpAkeCore_CreateChallengeData

int DtcpAkeCore_ConsumeChallengeData(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                                     unsigned char *aChallengeCmdBuffer,
                                     unsigned int aChallengeCmdBufferSize)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreSessionData *sessionData;

    sessionData = (DtcpAkeCoreSessionData *)aAkeCoreSessionHandle;

    if ((NULL != sessionData) && 
        (NULL != aChallengeCmdBuffer) && 
        (OsWrap_EntryIsInList(&sessionData->AkeCoreData->AkeSessions, sessionData)))
    {
        if (akeTypeIdFullAuth == sessionData->AkeTypeId)
        {
            if (DTCP_FULL_AUTH_CHALLENGE_SIZE == aChallengeCmdBufferSize)
            {
                returnValue = DtcpChallenge_ConsumeFullAuthData(sessionData, aChallengeCmdBuffer);
            }
            else
            {
                returnValue = INVALID_ARGUMENT;
            }
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // DtcpAkeCore_ConsumeChallengeData

int DtcpAkeCore_CreateResponseData(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                                   unsigned char **aResponseCmdBufferPtr,
                                   unsigned int *aResponseCmdBufferSize)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreSessionData *sessionData;

    sessionData = (DtcpAkeCoreSessionData *)aAkeCoreSessionHandle;

    if ((NULL != sessionData) && 
        (NULL != aResponseCmdBufferPtr) && 
        (NULL != aResponseCmdBufferSize) &&
        (OsWrap_EntryIsInList(&sessionData->AkeCoreData->AkeSessions, sessionData)))
    {
        if (akeTypeIdFullAuth == sessionData->AkeTypeId)
        {
            sessionData->ResponseCmdBuffer     = (unsigned char *)malloc(DTCP_FULL_AUTH_RESPONSE_SIZE);
            sessionData->ResponseCmdBufferSize = DTCP_FULL_AUTH_RESPONSE_SIZE;
        }
        //todo Figure out whether other ake types should be supported.

        if (0 != sessionData->ResponseCmdBuffer)
        {
            returnValue = DtcpResponse_CreateFullAuthData(aAkeCoreSessionHandle, sessionData->ResponseCmdBuffer);
            if (IS_SUCCESS(returnValue))
            {
                *aResponseCmdBufferPtr  = sessionData->ResponseCmdBuffer;
                *aResponseCmdBufferSize = sessionData->ResponseCmdBufferSize;
            }
            else
            {
                *aResponseCmdBufferPtr  = 0;
                *aResponseCmdBufferSize = 0;
            }
        }
        else
        {
            sessionData->ResponseCmdBufferSize = 0;
            returnValue = FAILURE;
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // DtcpAkeCore_CreateResponseData

int DtcpAkeCore_ConsumeResponseData(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                                    unsigned char *aResponseCmdBuffer,
                                    unsigned int aResponseCmdBufferSize,
                                    unsigned int *aOurSrmUpdateRequiredFlag)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreSessionData *sessionData;

    sessionData = (DtcpAkeCoreSessionData *)aAkeCoreSessionHandle;

    if ((NULL != sessionData) && 
        (NULL != aResponseCmdBuffer) && 
        (OsWrap_EntryIsInList(&sessionData->AkeCoreData->AkeSessions, sessionData)))
    {
        if (akeTypeIdFullAuth == sessionData->AkeTypeId)
        {
            if (DTCP_FULL_AUTH_RESPONSE_SIZE == aResponseCmdBufferSize)
            {
                returnValue = DtcpResponse_ConsumeFullAuthData(sessionData, 
                                                               aResponseCmdBuffer, 
                                                               aOurSrmUpdateRequiredFlag);
            }
            else
            {
                returnValue = INVALID_ARGUMENT;
            }
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // DtcpAkeCore_ConsumeResponseData

int DtcpAkeCore_CreateExchangeKeyData(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                                      EnExchKeyId aExchKeyId,
                                      unsigned char **aExchangeKeyCmdBufferPtr,
                                      unsigned int *aExchangeKeyCmdBufferSize)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreSessionData *sessionData;
    unsigned char *exchKeyCmdBuffer;
    unsigned int exchKeyCmdBufferSize;

    sessionData = (DtcpAkeCoreSessionData *)aAkeCoreSessionHandle;

    if ((NULL != sessionData) && 
        (NULL != aExchangeKeyCmdBufferPtr) && 
        (NULL != aExchangeKeyCmdBufferSize) &&
        (OsWrap_EntryIsInList(&sessionData->AkeCoreData->AkeSessions, sessionData)))
    {
        exchKeyCmdBuffer = (unsigned char *)malloc(DTCP_EXCHANGE_KEY_CMD_DATA_SIZE);
        exchKeyCmdBufferSize = DTCP_EXCHANGE_KEY_CMD_DATA_SIZE;

        if (0 != exchKeyCmdBuffer)
        {
            returnValue = DtcpAkeExchKeys_CreateExchKeyData(aAkeCoreSessionHandle,
                                                            aExchKeyId,
                                                            exchKeyCmdBuffer);

            if (IS_SUCCESS(returnValue))
            {
                if (exchKeyIdCopyNever == aExchKeyId)
                {
                    sessionData->ExchKeyCopyNeverCmdBuffer = exchKeyCmdBuffer;
                    sessionData->ExchKeyCopyNeverCmdBufferSize = exchKeyCmdBufferSize;
                }

                if (exchKeyIdCopyOneGeneration == aExchKeyId)
                {
                    sessionData->ExchKeyCopyOneGenCmdBuffer = exchKeyCmdBuffer;
                    sessionData->ExchKeyCopyOneGenCmdBufferSize = exchKeyCmdBufferSize;
                }

                if (exchKeyIdCopyNoMore == aExchKeyId)
                {
                    sessionData->ExchKeyCopyNoMoreCmdBuffer = exchKeyCmdBuffer;
                    sessionData->ExchKeyCopyNoMoreCmdBufferSize = exchKeyCmdBufferSize;
                }

                if (exchKeyIdAes128 == aExchKeyId)
                {
                    sessionData->ExchKeyAes128CmdBuffer = exchKeyCmdBuffer;
                    sessionData->ExchKeyAes128CmdBufferSize = exchKeyCmdBufferSize;
                }

                *aExchangeKeyCmdBufferPtr = exchKeyCmdBuffer;
                *aExchangeKeyCmdBufferSize = exchKeyCmdBufferSize;
            }
        }
        else
        {
            returnValue = FAILURE;
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // DtcpAkeCore_CreateExchangeKeyData

int DtcpAkeCore_ConsumeExchangeKeyData(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                                       EnExchKeyId aExchKeyId,
                                       unsigned char *aExchangeKeyCmdBuffer,
                                       unsigned int aExchangeKeyCmdBufferSize)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreSessionData *sessionData;

    sessionData = (DtcpAkeCoreSessionData *)aAkeCoreSessionHandle;

    if ((NULL != sessionData) && 
        (NULL != aExchangeKeyCmdBuffer) && 
        (OsWrap_EntryIsInList(&sessionData->AkeCoreData->AkeSessions, sessionData)))
    {
        if (akeTypeIdFullAuth == sessionData->AkeTypeId)
        {
            if (DTCP_EXCHANGE_KEY_CMD_DATA_SIZE == aExchangeKeyCmdBufferSize)
            {
                returnValue = DtcpAkeExchKeys_ConsumeExchKeyData(sessionData, aExchKeyId, aExchangeKeyCmdBuffer);
            }
            else
            {
                returnValue = INVALID_ARGUMENT;
            }
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // DtcpAkeCore_ConsumeExchangeKeyData

int DtcpAkeCore_CreateSrmData(DtcpAkeCoreSessionHandle  aAkeCoreSessionHandle,
                              unsigned char           **aSrmCmdBufferPtr,
                              unsigned int             *aSrmCmdBufferSize)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreSessionData *sessionData;
    unsigned char *ourCert;
    unsigned int   ourCertSize;
    unsigned char *ourSrm;
    unsigned int   ourSrmSize;
    unsigned int   srmUpdateSize;

    sessionData = (DtcpAkeCoreSessionData *)aAkeCoreSessionHandle;

    if ((NULL != sessionData) && 
        (NULL != aSrmCmdBufferPtr) && 
        (NULL != aSrmCmdBufferSize) && 
        (OsWrap_EntryIsInList(&sessionData->AkeCoreData->AkeSessions, sessionData)))
    {
        *aSrmCmdBufferPtr = NULL;
        *aSrmCmdBufferSize = 0;

        // Determine if their SRM needs to be updated.
        returnValue = DtcpCore_GetCert(&ourCert, &ourCertSize);
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpCore_GetSrm(&ourSrm, &ourSrmSize);
    }

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpSrm_IsTheirSrmCurrent(ourSrm,
                                                ourSrmSize,
                                                sessionData->TheirSrmG,
                                                sessionData->TheirSrmV,
                                                sessionData->TheirSrmC,
                                                &srmUpdateSize);
    }

    if (IS_SUCCESS(returnValue) && (0 < srmUpdateSize))
    {
        sessionData->SrmCmdBuffer = NULL;
        sessionData->SrmCmdBuffer = (unsigned char *)malloc(srmUpdateSize);
        if (sessionData->SrmCmdBuffer)
        {
            sessionData->SrmCmdBufferSize = srmUpdateSize;
            memcpy(sessionData->SrmCmdBuffer, ourSrm, srmUpdateSize);
            *aSrmCmdBufferPtr  = sessionData->SrmCmdBuffer;
            *aSrmCmdBufferSize = srmUpdateSize;
        }
        else
        {
            returnValue = FAILURE;
        }
    }
    return returnValue;
} // DtcpAkeCore_CreateSrmData

int DtcpAkeCore_ConsumeSrmData(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                               unsigned char           *aSrmCmdBuffer,
                               unsigned int             aSrmCmdBufferSize)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreSessionData *sessionData;
    unsigned char *ourSrm;
    unsigned int   ourSrmSize;

    sessionData = (DtcpAkeCoreSessionData *)aAkeCoreSessionHandle;

    if ((NULL != sessionData) && 
        (NULL != aSrmCmdBuffer) && 
        (OsWrap_EntryIsInList(&sessionData->AkeCoreData->AkeSessions, sessionData)))
    {
        returnValue = DtcpCore_GetSrm(&ourSrm, &ourSrmSize);

        if (0 < aSrmCmdBufferSize)
        {
            returnValue = DtcpSrm_UpdateSrm(ourSrm,
                                            ourSrmSize,
                                            aSrmCmdBuffer,
                                            aSrmCmdBufferSize);
        }

    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // DtcpAkeCore_ConsumeSrmData

int DtcpAkeCore_GetSourceExchangeKeyLabel(unsigned int      *aExchangeKeyLabel)
{
    int returnValue = SUCCESS;

    if (aExchangeKeyLabel)
    {
        if (!DtcpExchKeys_IsInitialized(&gAkeCoreData.ExchangeKeyData))
        {
            OsWrap_SemaphoreWait(gAkeCoreData.DataSemaphore);
            returnValue = DtcpExchKeys_CreateExchKeys(&gAkeCoreData.ExchangeKeyData);
            OsWrap_SemaphorePost(gAkeCoreData.DataSemaphore);
        }

        returnValue = DtcpExchKeys_GetExchKeyLabel(&gAkeCoreData.ExchangeKeyData, aExchangeKeyLabel);
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    return returnValue;
} // DtcpAkeCore_GetSourceExchangeKeyLabel

int DtcpAkeCore_GetSourceExchangeKey(EnExchKeyId        aExchangeKeyId,
                                     unsigned char      aExchangeKeyBuffer[DTCP_EXCHANGE_KEY_SIZE],
                                     unsigned int      *aExchangeKeyLabel)
{
    int returnValue = SUCCESS;

    if (aExchangeKeyLabel)
    {
        if (!DtcpExchKeys_IsInitialized(&gAkeCoreData.ExchangeKeyData))
        {
            OsWrap_SemaphoreWait(gAkeCoreData.DataSemaphore);
            returnValue = DtcpExchKeys_CreateExchKeys(&gAkeCoreData.ExchangeKeyData);
            OsWrap_SemaphorePost(gAkeCoreData.DataSemaphore);
        }

        returnValue = DtcpExchKeys_GetExchKey(&gAkeCoreData.ExchangeKeyData, aExchangeKeyId, aExchangeKeyBuffer);
        returnValue = DtcpExchKeys_GetExchKeyLabel(&gAkeCoreData.ExchangeKeyData, aExchangeKeyLabel);
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    return returnValue;
} // DtcpAkeCore_GetSourceExchangeKeyLabel

int DtcpAkeCore_GetSinkExchangeKey(DtcpAkeCoreSessionHandle  aAkeCoreSessionHandle,
                                   EnExchKeyId               aExchangeKeyId,
                                   unsigned char             aExchangeKeyBuffer[DTCP_EXCHANGE_KEY_SIZE],
                                   unsigned int             *aExchangeKeyLabel)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreSessionData *sessionData;

    sessionData = (DtcpAkeCoreSessionData *)aAkeCoreSessionHandle;

    if ((NULL != sessionData) && (NULL != aExchangeKeyLabel))
    {
        returnValue = DtcpExchKeys_GetExchKey(&sessionData->ExchangeKeyData, aExchangeKeyId, aExchangeKeyBuffer);
        returnValue = DtcpExchKeys_GetExchKeyLabel(&sessionData->ExchangeKeyData, aExchangeKeyLabel);
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    return returnValue;
} // DtcpAkeCore_GetSinkExchangeKeyLabel

int DtcpAkeCore_GetDeviceMode(DtcpAkeCoreSessionHandle  aAkeCoreSessionHandle,
                              EnDeviceMode             *aDeviceMode)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreSessionData *sessionData;

    sessionData = (DtcpAkeCoreSessionData *)aAkeCoreSessionHandle;

    if ((NULL != sessionData) && (NULL != aDeviceMode))
    {
        *aDeviceMode = sessionData->DeviceMode;
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // DtcpAkeCore_GetDeviceMode

int DtcpAkeCore_OpenStream(int                      aUpdateRealTimeNonceFlag,
                           DtcpAkeCoreStreamHandle *aAkeCoreStreamHandle)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreStreamData *streamData = NULL;

    if (!aAkeCoreStreamHandle)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        streamData = (DtcpAkeCoreStreamData *)malloc(sizeof(DtcpAkeCoreStreamData));
        if (streamData)
        {
            streamData->UpdateRealTimeNonceStreamFlag = aUpdateRealTimeNonceFlag;
        }
        else
        {
            returnValue = FAILURE;
        }
    }

    if (IS_SUCCESS(returnValue))
    {   
        OsWrap_SemaphoreWait(gAkeCoreData.DataSemaphore);

        gAkeCoreData.StreamCount++;

        if (aUpdateRealTimeNonceFlag)
        {
            // Increment open RTP stream count
            gAkeCoreData.UpdateRealTimeNonceStreamCount++;
        }

        OsWrap_SemaphorePost(gAkeCoreData.DataSemaphore);
    }

    if (IS_SUCCESS(returnValue))
    {
        *aAkeCoreStreamHandle = (DtcpAkeCoreStreamHandle)streamData;                
    }

    return returnValue;
}

int DtcpAkeCore_CloseStream(DtcpAkeCoreStreamHandle aAkeCoreStreamHandle)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreStreamData *streamData = NULL;

    if (!aAkeCoreStreamHandle)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        streamData = (DtcpAkeCoreStreamData *)aAkeCoreStreamHandle;

        OsWrap_SemaphoreWait(gAkeCoreData.DataSemaphore);

        if ((0 < gAkeCoreData.UpdateRealTimeNonceStreamCount) && 
            (streamData->UpdateRealTimeNonceStreamFlag))
        {
            gAkeCoreData.UpdateRealTimeNonceStreamCount--;
        }

        if (0 < gAkeCoreData.StreamCount)
        {
            gAkeCoreData.StreamCount--;
        }

        OsWrap_SemaphorePost(gAkeCoreData.DataSemaphore);

        free(streamData);
    }

    return returnValue;
}

int DtcpAkeCore_GetRealTimeNonce(unsigned char      aNonce[DTCP_CONTENT_KEY_NONCE_SIZE])
{
    int returnValue = SUCCESS;

    if (!aNonce)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        memcpy(aNonce, gAkeCoreData.RealTimeNonce, DTCP_CONTENT_KEY_NONCE_SIZE);
    }

    return returnValue;
}

//int DtcpAkeCore_GetAkeCoreHandle(DtcpAkeCoreSessionHandle  aAkeCoreSessionHandle,
//                                 DtcpAkeCoreHandle        *aAkeCoreHandle)
//{
//    int returnValue = SUCCESS;
//    DtcpAkeCoreSessionData *sessionData = NULL;
//
//    if (!aAkeCoreSessionHandle || !aAkeCoreHandle)
//    {
//        returnValue = INVALID_ARGUMENT;
//    }
//
//    if (IS_SUCCESS(returnValue))
//    {
//        sessionData = (DtcpAkeCoreSessionData *)aAkeCoreSessionHandle;
//        *aAkeCoreHandle = sessionData->AkeCoreData;
//    }
//
//    return returnValue;
//} // DtcpAkeCore_GetAkeCoreHandle

int DtcpAkeCore_UpdateExchangeKey(unsigned int      *aExchangeKeyLabel)
{
    int returnValue = SUCCESS;

    if (!aExchangeKeyLabel)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        OsWrap_SemaphoreWait(gAkeCoreData.DataSemaphore);

        DtcpExchKeys_InvalidateExchKeys(&gAkeCoreData.ExchangeKeyData);
        DtcpExchKeys_CreateExchKeys(&gAkeCoreData.ExchangeKeyData);
        gAkeCoreData.AuthenticatedSinkDeviceCount = 0;
        memset(gAkeCoreData.AuthenticatedSinkDevices, 0, DTCP_SINK_COUNT_LIMIT * DTCP_DEVICE_ID_SIZE);

        OsWrap_SemaphorePost(gAkeCoreData.DataSemaphore);

        *aExchangeKeyLabel = gAkeCoreData.ExchangeKeyData.Label;
    }

    return returnValue;
} // DtcpAkeCore_UpdateExchangeKey

int DtcpAkeCore_ValidateExchangeKey(DtcpAkeCoreSessionHandle  aAkeCoreSessionHandle,
                                    int                       aExchangeKeyLabel,
                                    int                      *aExchangeKeyValidFlag)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreSessionData *sessionData = NULL;

    if (!aAkeCoreSessionHandle || !aExchangeKeyValidFlag)
    {
        if (aExchangeKeyValidFlag)
        {
            *aExchangeKeyValidFlag = 0;
        }
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        sessionData = (DtcpAkeCoreSessionData *)aAkeCoreSessionHandle;

        if (aExchangeKeyLabel == sessionData->ExchangeKeyData.Label)
        {
            *aExchangeKeyValidFlag = 1;
        }
        else
        {
            *aExchangeKeyValidFlag = 0;
        }
    }

    return returnValue;
} // DtcpAkeCore_ValidateExchangeKey

int DtcpAkeCore_CheckSinkCountLimit()
{
    int returnValue = SUCCESS;

    if (DTCP_SINK_COUNT_LIMIT <= gAkeCoreData.AuthenticatedSinkDeviceCount)
    {
        returnValue = DTCP_SINK_COUNT_LIMIT_REACHED;
    }
    return returnValue;
}
