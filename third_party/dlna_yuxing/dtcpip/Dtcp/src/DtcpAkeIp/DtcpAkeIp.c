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

#include <stdio.h> // for debug/error messages.
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "DtcpAkeIp.h"
#include "DtcpAkeTransport.h"
#include "DtcpStatusCodes.h"
#include "DtcpConstants.h"
#include "DtcpAkeIpInternal.h"
#include "OsWrapper.h"
#include "DtcpAkeCore.h"

/// \file
/// \brief Implementation file for \ref DtcpAkeIp library.

#if 0
static int GlobalDisplayLevel             = -1;
static int GlobalDisplayLevelMatchExactly = 0;
static int GlobalLogToFile                = 0;
#endif

/// \addtogroup DtcpAkeIp
/// @{

static DtcpAkeIpData gAkeIpData; ///< Global, static data for library.

int DtcpAkeIp_Startup(ExchangeKeyUpdateRequest_Ptr  aExchangeKeyUpdateRequestFunc)
{
    int returnValue = SUCCESS;

    if (!aExchangeKeyUpdateRequestFunc)
    {
        returnValue = INVALID_ARGUMENT;
        return returnValue;
    }

    returnValue = OsWrap_SemaphoreInit(&gAkeIpData.DataSemaphore, 1);

    if (IS_SUCCESS(returnValue))
    {
        OsWrap_SemaphoreWait(gAkeIpData.DataSemaphore);
        gAkeIpData.SinkSessions = NULL;
        gAkeIpData.AkeLabelCounter = 0;

        // Load the AkeIpInterface
        gAkeIpData.AkeIpInterfaceLoaded = FALSE;
        gAkeIpData.AkeIpInterface.StructureSize = sizeof(DtcpAkeIpInterface);
        gAkeIpData.AkeIpInterface.InterfaceVersion = DTCP_AKE_IP_TRANSPORT_INTERFACE_VERSION;
        returnValue = DtcpAkeIp_GetInterface(&gAkeIpData.AkeIpInterface);

        if (IS_SUCCESS(returnValue))
        {
            returnValue = gAkeIpData.AkeIpInterface.Startup(&gAkeIpData.AkeIpMinMessageSize);
            if (IS_SUCCESS(returnValue))
            {
                gAkeIpData.AkeIpInterfaceLoaded = TRUE;
            }
        }

        if (IS_SUCCESS(returnValue))
        {
            returnValue = DtcpAkeCore_Startup(aExchangeKeyUpdateRequestFunc);
        }
        OsWrap_SemaphorePost(gAkeIpData.DataSemaphore);
    }
    return returnValue;
} // DtcpAkeIp_Startup

void DtcpAkeIp_Shutdown()
{
    OsWrap_SemaphoreWait(gAkeIpData.DataSemaphore);

    if (gAkeIpData.AkeIpInterfaceLoaded)
    {
        gAkeIpData.AkeIpInterface.Shutdown();
        gAkeIpData.AkeIpInterfaceLoaded = 0;
    }
    OsWrap_DeleteList(&gAkeIpData.SinkSessions);

    OsWrap_SemaphorePost(gAkeIpData.DataSemaphore);
    OsWrap_SemaphoreClose(gAkeIpData.DataSemaphore);

    DtcpAkeCore_Shutdown();

    return;
} // DtcpAkeIp_Shutdown

int DtcpAkeIp_Listen(char                  *aIpAddress,
                     unsigned int          *aSourcePortNumber,
                     DtcpAkeIpListenHandle *aListenHandle)
{
    int returnValue = SUCCESS;

    if (gAkeIpData.AkeIpInterfaceLoaded)
    {
        returnValue = gAkeIpData.AkeIpInterface.Listen(ListenCallback,
                                                       aIpAddress,
                                                       &gAkeIpData,
                                                       aSourcePortNumber,
                                                       aListenHandle);
    }
    else
    {
        returnValue = NOT_INITIALIZED;
        ERROR_MSG(1, returnValue, ("AKE IP transport interface not loaded\n"));
    }

    return returnValue;
} // DtcpAkeIp_Listen

int DtcpAkeIp_CancelListen(DtcpAkeIpListenHandle aListenHandle)
{
    int returnValue = SUCCESS;

    if (gAkeIpData.AkeIpInterfaceLoaded)
    {
        returnValue = gAkeIpData.AkeIpInterface.CloseTransportHandle(aListenHandle);
    }
    else
    {
        returnValue = NOT_INITIALIZED;
        ERROR_MSG(1, returnValue, ("AKE IP transport interface not loaded\n"));
    }

    return returnValue;
} // DtcpAkeIp_CancelListen

int DtcpAkeIp_OpenAke(char *aSourceIpAddress,
                      unsigned int aSourcePortNumber,
                      DtcpAkeIpSessionHandle *aAkeIpSessionHandle)
{
    int returnValue = SUCCESS;
    DtcpAkeIpSessionData *akeIpSessionData = NULL;

    if (!aSourceIpAddress || !aAkeIpSessionHandle)
    {
        returnValue = INVALID_ARGUMENT;
        return returnValue;
    }

    if (IS_SUCCESS(returnValue))
    {
        OsWrap_SemaphoreWait(gAkeIpData.DataSemaphore);
        returnValue = OsWrap_AddToList(&gAkeIpData.SinkSessions, &akeIpSessionData, sizeof(DtcpAkeIpSessionData));
        gAkeIpData.AkeLabelCounter++;
        OsWrap_SemaphorePost(gAkeIpData.DataSemaphore);
    }

    if (IS_SUCCESS(returnValue))
    {
        memset(akeIpSessionData->SourceIpAddress, 0, MAX_IP_ADDR_SIZE);
        memcpy(akeIpSessionData->SourceIpAddress, aSourceIpAddress, strlen(aSourceIpAddress));
        *aAkeIpSessionHandle = akeIpSessionData;
        akeIpSessionData->AkeIpData = &gAkeIpData;
        akeIpSessionData->SourcePortNumber = aSourcePortNumber;
        akeIpSessionData->AkeLabel = gAkeIpData.AkeLabelCounter;
    }
    else
    {
        returnValue = FAILURE;
        ERROR_MSG(1, returnValue, ("Unable to allocate %d bytes for sink session data\n", 
                                    sizeof(DtcpAkeIpSessionData)));
    }

    return returnValue;
} // DtcpAkeIp_OpenAke

int DtcpAkeIp_DoAke(DtcpAkeIpSessionHandle aAkeIpSessionHandle)
{
    int returnValue = SUCCESS;
    DtcpAkeIpSessionData *akeIpSessionData = NULL;

    // Get the akeIpSessionData pointer
    akeIpSessionData = (DtcpAkeIpSessionData *)aAkeIpSessionHandle;

    if (NULL == akeIpSessionData)
    {
        returnValue = INVALID_HANDLE;
        ERROR_MSG(1, returnValue, ("Unable to find sink session pointer using handle\n"));
        return returnValue;
    }

    if (!akeIpSessionData->AkeIpData->AkeIpInterfaceLoaded)
    {
        returnValue = NOT_INITIALIZED;
        ERROR_MSG(1, returnValue, ("AKE IP transport interface not loaded\n"));
        return returnValue;
    }

    returnValue = akeIpSessionData->AkeIpData->AkeIpInterface.StartSink(akeIpSessionData->SourceIpAddress,
                                                                        akeIpSessionData->SourcePortNumber,
                                                                        &akeIpSessionData->TransportHandle,
                                                                        &akeIpSessionData->MessagingInterface);

    if (IS_SUCCESS(returnValue))
    {        
        returnValue = StartSink(akeIpSessionData);
        akeIpSessionData->AkeIpData->AkeIpInterface.CloseTransportHandle(akeIpSessionData->TransportHandle);
    }

    return returnValue;
} // DtcpAkeIp_DoAke

int DtcpAkeIp_CloseAke(DtcpAkeIpSessionHandle aAkeIpSessionHandle)
{
    int returnValue = SUCCESS;

    DtcpAkeIpSessionData *akeIpSessionData = NULL;

    // Get the akeIpSessionData pointer
    akeIpSessionData = (DtcpAkeIpSessionData *)aAkeIpSessionHandle;

    if (NULL == akeIpSessionData)
    {
        returnValue = INVALID_HANDLE;
        ERROR_MSG(1, returnValue, ("Unable to find sink session pointer using handle\n"));
        return returnValue;
    }

    DtcpAkeCore_CloseAkeSession(akeIpSessionData->AkeCoreSessionHandle);
    akeIpSessionData->AkeIpData->AkeIpInterface.CloseTransportHandle(akeIpSessionData->TransportHandle);

    OsWrap_SemaphoreWait(gAkeIpData.DataSemaphore);
    OsWrap_RemoveFromList(&akeIpSessionData->AkeIpData->SinkSessions, akeIpSessionData);
    OsWrap_SemaphorePost(gAkeIpData.DataSemaphore);

    return returnValue;
} // DtcpAkeIp_CloseAke

//int DtcpAkeIp_GetAkeCoreHandle(DtcpAkeIpHandle aAkeIpHandle, DtcpAkeCoreHandle *aAkeCoreHandle)
//{
//    int returnValue = SUCCESS;
//    DtcpAkeIpData *akeIpData = (DtcpAkeIpData*)aAkeIpHandle;
//
//    if ((NULL != akeIpData) && (NULL != aAkeCoreHandle))
//    {
//        *aAkeCoreHandle = gAkeIpData.AkeCoreHandle;
//    }
//    else
//    {
//        returnValue = INVALID_ARGUMENT;
//    }
//
//    return returnValue;
//} // DtcpAkeIp_GetAkeCoreHandle

int DtcpAkeIp_GetAkeCoreSessionHandle(DtcpAkeIpSessionHandle    aAkeIpSessionHandle, 
                                      DtcpAkeCoreSessionHandle *aAkeCoreSessionHandle)
{
    int returnValue = SUCCESS;
    DtcpAkeIpSessionData *akeIpSessionData = (DtcpAkeIpSessionData*)aAkeIpSessionHandle;

    if ((NULL != akeIpSessionData) && (NULL != aAkeCoreSessionHandle))
    {
        *aAkeCoreSessionHandle = akeIpSessionData->AkeCoreSessionHandle;
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // DtcpAkeIp_GetAkeCoreSessionHandle

int DtcpAkeIp_UpdateExchangeKey(unsigned int    *aExchangeKeyLabel)
{
    int returnValue = SUCCESS;

    if (!aExchangeKeyLabel)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpAkeCore_UpdateExchangeKey(aExchangeKeyLabel);
    }

    return returnValue;
} // DtcpAkeIp_UpdateExchangeKey

int DtcpAkeIp_ValidateExchangeKey(DtcpAkeIpSessionHandle  aAkeIpSessionHandle,
                                  int                     aExchangeKeyLabel,
                                  int                    *aExchangeKeyValidFlag)
{
    int returnValue = SUCCESS;
    DtcpAkeIpSessionData *akeIpSessionData = NULL;

    if (!aAkeIpSessionHandle || !aExchangeKeyValidFlag)
    {
        if (aExchangeKeyValidFlag)
        {
            *aExchangeKeyValidFlag = 0;
        }
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        akeIpSessionData = (DtcpAkeIpSessionData *)aAkeIpSessionHandle;
        if (-1 == aExchangeKeyLabel)
        {
            if (!akeIpSessionData->AkeIpData->AkeIpInterfaceLoaded)
            {
                returnValue = NOT_INITIALIZED;
            }

            if (IS_SUCCESS(returnValue))
            {
                returnValue = akeIpSessionData->AkeIpData->AkeIpInterface.StartSink(akeIpSessionData->SourceIpAddress,
                                                                                    akeIpSessionData->SourcePortNumber,
                                                                                    &akeIpSessionData->TransportHandle,
                                                                                    &akeIpSessionData->MessagingInterface);
            }

            if (IS_SUCCESS(returnValue))
            {        
                returnValue = GetSourceExchangeKeyLabel(akeIpSessionData,
                                                        &aExchangeKeyLabel);
                akeIpSessionData->AkeIpData->AkeIpInterface.CloseTransportHandle(akeIpSessionData->TransportHandle);
            }
        }

        returnValue = DtcpAkeCore_ValidateExchangeKey(akeIpSessionData->AkeCoreSessionHandle,
                                                      aExchangeKeyLabel,
                                                      aExchangeKeyValidFlag);
    }

    return returnValue;
} // DtcpAkeIp_ValidateExchangeKey

int DtcpAkeIp_GetSourceExchangeKeyLabel(unsigned int    *aExchangeKeyLabel)
{
    int returnValue = SUCCESS;

    if (!aExchangeKeyLabel)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpAkeCore_GetSourceExchangeKeyLabel(aExchangeKeyLabel);
    }

    return returnValue;
} // DtcpAkeIp_GetSourceExchangeKeyLabel

int DtcpAkeIp_CheckSourceSinkLimit(char *aSourceAkeIpAddr, unsigned int aSourceAkePortNumber)
{
    int returnValue = SUCCESS;
    DtcpTransportHandle          transportHandle;
    DtcpAkeIpMessagingInterface  messagingInterface;

    if (!aSourceAkeIpAddr)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        if (!gAkeIpData.AkeIpInterfaceLoaded)
        {
            returnValue = NOT_INITIALIZED;
        }

        if (IS_SUCCESS(returnValue))
        {
            returnValue = gAkeIpData.AkeIpInterface.StartSink(aSourceAkeIpAddr,
                                                              aSourceAkePortNumber,
                                                              &transportHandle,
                                                              &messagingInterface);
        }

        if (IS_SUCCESS(returnValue))
        {        
            returnValue = CheckSourceSinkLimit(&gAkeIpData, transportHandle, messagingInterface);
            gAkeIpData.AkeIpInterface.CloseTransportHandle(transportHandle);
        }
    }
    return returnValue;
} // DtcpAkeIp_CheckSourceSinkLimit
/// @}
