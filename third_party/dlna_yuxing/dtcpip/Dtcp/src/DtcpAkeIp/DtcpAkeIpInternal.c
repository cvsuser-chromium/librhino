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
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include "DtcpAkeIpInternal.h"
#include "DtcpAkeCore.h"
#include "DtcpStatusCodes.h"
#include "DtcpAkeTransport.h"
#include "DtcpConstants.h"
#include "OsWrapper.h"

/// \file
/// \brief Implementation file for internal functions of \ref DtcpAkeIp library

#define STATUS_MESSAGE_SIZE   0
#define MAX_MESSAGE_SIZE    2048

#define SEND_TIMEOUT 10000

///////////////////////////////////////////////////////////////
////// Debug Message Variables
///////////////////////////////////////////////////////////////

#if 0
static int GlobalDisplayLevel             = 0;
static int GlobalDisplayLevelMatchExactly = 0;
static int GlobalLogToFile                = 0;
#endif

char *ConvertToBinaryString(int aValue, int aBits)
{
    static char bin[65];
    int index;

    for(index = aBits - 1; index >= 0; --index)
    {
        if(aValue & 0x01)
            bin[index] = '1';
        else
            bin[index] = '0';

        aValue = aValue >> 1;
    }

    bin[aBits] = 0x00;

    return(bin);
}

void PrintBuffer(int DisplayLevel, unsigned char *aBuffer, unsigned int   aBufferSize)
{
    unsigned int bufferPos = 0;
    int i;

    while (bufferPos < aBufferSize)
    {
        DEBUG_MSG(DisplayLevel, ("\t"));                 
        for (i = 1; i <= 16; ++i)
        {
            DEBUG_MSG(DisplayLevel, ("%02X ", aBuffer[bufferPos]));
            ++bufferPos;
            if (bufferPos >= aBufferSize)
            {
                break;
            }
            if (0 == i % 4)
            {
                DEBUG_MSG(DisplayLevel, (" "));
            }
        }
        DEBUG_MSG(DisplayLevel, ("\n"));
    }
    DEBUG_MSG(DisplayLevel, ("\n"));
    
    return;
} // PrintBuffer

void PrintMessage(unsigned char               *aMessage,
                  unsigned int                 aMessageSize,
                  DtcpAkeIpMessagingInterface *aIMessaging)
{
    unsigned char *commandData;
    unsigned int messageLength;
    unsigned int cType;
    EnCommands   commandType;
#if 0	
    unsigned char commandString[256];
    unsigned char statusString[256];
    unsigned char cTypeString[256];
#else
	char commandString[256];
	char statusString[256];
	char cTypeString[256];
#endif
    unsigned int akeProcedures;
    unsigned int exchangeKeys;
    unsigned int subfunctionDep;
    unsigned int akeLabel;
    unsigned int status;

    commandData    = aIMessaging->GetCommandData(aMessage, &messageLength);
    cType          = aIMessaging->GetCTypeResponse(aMessage);
    commandType    = aIMessaging->GetCommandType(aMessage);
    akeProcedures  = aIMessaging->GetAkeProcedures(aMessage);
    exchangeKeys   = aIMessaging->GetExchangeKeys(aMessage);
    subfunctionDep = aIMessaging->GetSubFuncDep(aMessage);
    akeLabel       = aIMessaging->GetAkeLabel(aMessage);
    status         = aIMessaging->GetStatus(aMessage);

    memset(commandString, 0, 256);

    switch (commandType)
    {
    case cmdStatus:
        strcpy(commandString, "Status");
        break;

    case cmdChallenge:
        strcpy(commandString, "Challenge");
        break;

    case cmdResponse:
        strcpy(commandString, "Response");
        break;

    case cmdExchangeKey:
        strcpy(commandString, "Exchange Key");
        break;

    case cmdSRM:
        strcpy(commandString, "SRM");
        break;

    case cmdContentKey:
        strcpy(commandString, "Content Key Request");
        break;

    default:
        strcpy(commandString, "Undefined!");
        break;
    }

    switch (status)
    {
    case statusNoError:
        strcpy(statusString, "No Error");
        break;

    case statusNoMoreAuth:
        strcpy(statusString, "No More Authentication Procedures");
        break;
    case statusAnyOtherError:
        strcpy(statusString, "Any Other Error");
        break;
    case statusIncorrectCommandOrder:
        strcpy(statusString, "Incorrect Command Order");
        break;
    case statusAuthFailed:
        strcpy(statusString, "Authentication Failed");
        break;
    case statusDataSyntaxError:
        strcpy(statusString, "Data Syntax Error");
        break;
    case statusNoInformation:
        strcpy(statusString, "No Information");
        break;
    default:
        strcpy(statusString, "Undefined");
        break;
    }

    switch (cType)
    {
    case ctypeControl:
        strcpy(cTypeString, "Control");
        break;
    case ctypeStatus:
        strcpy(cTypeString, "Status");
        break;
    case ctypeSpecificInquiry:
        strcpy(cTypeString, "Specific Inquiry");
        break;
    case ctypeNotify:
        strcpy(cTypeString, "Notify");
        break;
    case ctypeGeneralInquiry:
        strcpy(cTypeString, "General Inquiry");
        break;
    case responseNotImplemented:
        strcpy(cTypeString, "Not Implemented");
        break;
    case responseAccepted:
        strcpy(cTypeString, "Accepted");
        break;
    case responseRejected:
        strcpy(cTypeString, "Rejected");
        break;
    case responseInTransition:
        strcpy(cTypeString, "In Transition");
        break;
    case responseImplementedStable:
        strcpy(cTypeString, "Implemented/Stable");
        break;
    case responseChanged:
        strcpy(cTypeString, "Changed");
        break;
    case responseInterim:
        strcpy(cTypeString, "Interim");
        break;
    default:
        strcpy(cTypeString, "Undefined");
        break;
    }

    DEBUG_MSG(MSG_DEBUG, ("AKE Message:\n"));
    DEBUG_MSG(MSG_DEBUG, ("  CommandType:    %s\n", commandString));
    DEBUG_MSG(MSG_DEBUG, ("  Size:           %d\n", messageLength));
    DEBUG_MSG(MSG_DEBUG, ("  CType:          %s\n", cTypeString));
    DEBUG_MSG(MSG_DEBUG, ("  AKE Procedure:  %s\n", ConvertToBinaryString(akeProcedures, 8)));
    DEBUG_MSG(MSG_DEBUG, ("  Exchange Keys:  %s\n", ConvertToBinaryString(exchangeKeys, 8)));
    DEBUG_MSG(MSG_DEBUG, ("  Subfunc Dep:    %X\n", subfunctionDep));
    DEBUG_MSG(MSG_DEBUG, ("  AKE Label:      %X\n", akeLabel));
    DEBUG_MSG(MSG_DEBUG, ("  Status:         %s\n", statusString));

    DEBUG_MSG(MSG_DEBUG, ("  Ake Header:\n"));    
    DEBUG_BUF(MSG_DEBUG, aMessage, 11);
    DEBUG_MSG(MSG_DEBUG, ("  Ake Info:\n"));
    DEBUG_BUF(MSG_DEBUG, aMessage + 11, aMessageSize - 11);

} // PrintMessage

int SendStatusCommand(DtcpAkeIpData *aAkeIpData, DtcpTransportHandle aTransportHandle, DtcpAkeIpMessagingInterface aMessagingInterface)
{
    int returnValue = SUCCESS;
    unsigned char *messageBuffer;
    int messageBufferLength;
    int status;
    int response;
    
    messageBufferLength = STATUS_MESSAGE_SIZE + aAkeIpData->AkeIpMinMessageSize;

    messageBuffer = (unsigned char *)malloc(messageBufferLength);
    if (messageBuffer)
    {
        memset(messageBuffer, 0, messageBufferLength);

        aMessagingInterface.SetAkeLabel(messageBuffer, 0xFF);
        aMessagingInterface.SetAkeProcedures(messageBuffer, 0xFF);
        aMessagingInterface.SetCommandType(messageBuffer, cmdStatus);
        aMessagingInterface.SetCTypeResponse(messageBuffer, ctypeStatus);
        aMessagingInterface.SetExchangeKeys(messageBuffer, 0xFF);
        aMessagingInterface.SetStatus(messageBuffer, 0xF);
        aMessagingInterface.SetSubFuncDep(messageBuffer, 0xFF);
        aMessagingInterface.SetCommandData(messageBuffer, NULL, 0);

#ifdef DEMO_MODE
        DEBUG_MSG(MSG_DEBUG, ("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    ">>     Sending status command      >>\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
        DEBUG_MSG(MSG_DEBUG, ("Sending status command\n"));
        PrintMessage(messageBuffer, messageBufferLength, &aMessagingInterface); // Uses MSG_DEBUG
#endif

        returnValue = aMessagingInterface.Send(aTransportHandle,
                                            messageBuffer,
                                            &messageBufferLength,
                                            SEND_TIMEOUT);

#ifdef DEMO_MODE
        DEBUG_MSG(MSG_DEBUG, ("\r\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    "<<    Received status response     <<\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
        DEBUG_MSG(MSG_DEBUG, ("Received status response\n"));
        PrintMessage(messageBuffer, messageBufferLength, &aMessagingInterface); // Uses MSG_DEBUG
#endif

        if (IS_SUCCESS(returnValue))
        {
            status = aMessagingInterface.GetStatus(messageBuffer);
            response = aMessagingInterface.GetCTypeResponse(messageBuffer);

            if ((statusNoMoreAuth == status) && (responseImplementedStable == response))
            {
                returnValue = DTCP_SINK_COUNT_LIMIT_REACHED;
            }
            else if ((statusNoError == status) && (responseImplementedStable == response))
            {
                returnValue = SUCCESS;
            }
            else
            {
                returnValue = FAILURE;
            }
        }
        free (messageBuffer);
    }
    else
    {
        returnValue = FAILURE;
    }

    return returnValue;
} // SendStatusCommand

int ProcessStatusCommand(DtcpAkeIpData               *aAkeIpData,
                         DtcpAkeIpMessagingInterface *aMessagingInterface,
                         DtcpTransportHandle          aTransportHandle,
                         unsigned char               *aMessageBuffer,
                         unsigned int                 aMessageBufferSize)
{
    int returnValue = SUCCESS;
    unsigned int bufferSize;
    //EnAkeTypeMask akeTypeMask;
    //EnExchKeyMask exchKeyMask;

    if (!aAkeIpData || !aMessagingInterface || !aTransportHandle || !aMessageBuffer)
    {
        returnValue = INVALID_ARGUMENT;
        return returnValue;
    }

    bufferSize = STATUS_MESSAGE_SIZE + aAkeIpData->AkeIpMinMessageSize;

#ifdef DEMO_MODE
    DEBUG_MSG(MSG_DEBUG, ("\r\n.....................................\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    "..   Processing status command     ..\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ".....................................\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
    DEBUG_MSG(MSG_DEBUG, ("Processing status command\n"));
    PrintMessage(aMessageBuffer, aMessageBufferSize, aMessagingInterface);  // Uses MSG_DEBUG
#endif

    // Fill in status information
    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpAkeCore_CheckSinkCountLimit();
        if (DTCP_SINK_COUNT_LIMIT_REACHED == returnValue)
        {
            aMessagingInterface->SetStatus(aMessageBuffer, statusNoMoreAuth);
        }
        else if (IS_SUCCESS(returnValue))
        {
            aMessagingInterface->SetStatus(aMessageBuffer, statusNoError);
        }
        else
        {
            aMessagingInterface->SetStatus(aMessageBuffer, statusAnyOtherError);
        }
        returnValue = SUCCESS;
    }

    if (IS_SUCCESS(returnValue))
    {
        aMessagingInterface->SetAkeProcedures(aMessageBuffer, akeTypeMaskFullAuth);
        aMessagingInterface->SetExchangeKeys(aMessageBuffer, exchKeyMaskAes128);
        aMessagingInterface->SetCTypeResponse(aMessageBuffer, responseImplementedStable);

        // Send response
#ifdef DEMO_MODE
        DEBUG_MSG(MSG_DEBUG, ("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    ">>     Sending status response     >>\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
        DEBUG_MSG(MSG_DEBUG, ("Sending status response\n"));
        PrintMessage(aMessageBuffer, aMessageBufferSize, aMessagingInterface); // Uses MSG_DEBUG
#endif        

        returnValue = aMessagingInterface->Send(aTransportHandle,
                                                aMessageBuffer,
                                                &bufferSize,
                                                SEND_TIMEOUT);
    }                                             
    return returnValue;
} // ProcessStatusCommand

int SendChallengeCommand(DtcpAkeIpSessionData *aAkeSessionData)
{
    int returnValue = SUCCESS;
    unsigned char *messageBuffer = NULL;
    int messageBufferLength = 0;
    unsigned char *challengeBuffer = NULL;
    unsigned int challengeBufferSize = 0;
    int status = 0;
    int response = 0;
    EnDeviceMode deviceMode;

    returnValue = DtcpAkeCore_GetDeviceMode(aAkeSessionData->AkeCoreSessionHandle, &deviceMode);

    returnValue = DtcpAkeCore_CreateChallengeData(aAkeSessionData->AkeCoreSessionHandle,
                                                  &challengeBuffer,
                                                  &challengeBufferSize);

    if (IS_SUCCESS(returnValue))
    {
        messageBufferLength = challengeBufferSize + aAkeSessionData->AkeIpData->AkeIpMinMessageSize;
        messageBuffer = (unsigned char *)malloc(messageBufferLength);
    }

    if (messageBuffer)
    {
        memset(messageBuffer, 0, messageBufferLength);

        if ((SUCCESS == returnValue) && (NULL != challengeBuffer))
        {
            if (deviceModeSource == deviceMode)
            {
                aAkeSessionData->MessagingInterface.SetSubFuncDep(messageBuffer, 0);
            }
            else
            {
                aAkeSessionData->MessagingInterface.SetSubFuncDep(messageBuffer, 1);
            }

            aAkeSessionData->MessagingInterface.SetAkeLabel(messageBuffer, aAkeSessionData->AkeLabel);
            aAkeSessionData->MessagingInterface.SetAkeProcedures(messageBuffer, akeTypeMaskFullAuth);
            aAkeSessionData->MessagingInterface.SetCommandType(messageBuffer, cmdChallenge);
            aAkeSessionData->MessagingInterface.SetCTypeResponse(messageBuffer, ctypeControl);
            aAkeSessionData->MessagingInterface.SetExchangeKeys(messageBuffer, exchKeyMaskAes128);
            aAkeSessionData->MessagingInterface.SetStatus(messageBuffer, 0xF);
            aAkeSessionData->MessagingInterface.SetCommandData(messageBuffer, 
                                                               challengeBuffer,
                                                               challengeBufferSize);

#ifdef DEMO_MODE
            DEBUG_MSG(MSG_DEBUG, ("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    ">>    Sending challenge command    >>\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
            DEBUG_MSG(MSG_DEBUG, ("Sending challenge command\n"));
            PrintMessage(messageBuffer, messageBufferLength, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif

            aAkeSessionData->ChallengeSentTime = OsWrap_GetCurrentTime();
            returnValue = aAkeSessionData->MessagingInterface.Send(aAkeSessionData->TransportHandle,
                                                                   messageBuffer,
                                                                   &messageBufferLength,
                                                                   SEND_TIMEOUT);

            if (IS_SUCCESS(returnValue))
            {
#ifdef DEMO_MODE
                DEBUG_MSG(MSG_DEBUG, ("\r\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
                DEBUG_MSG(MSG_DEBUG, (    "<<  Received challenge response    <<\r\n"));
                DEBUG_MSG(MSG_DEBUG, (    "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
                DEBUG_MSG(MSG_DEBUG, ("Received challenge response\n"));
                PrintMessage(messageBuffer, messageBufferLength, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif

                returnValue = DtcpAkeCore_ReleaseBuffers(aAkeSessionData->AkeCoreSessionHandle,
                                                        akeBufferChallenge);

                status = aAkeSessionData->MessagingInterface.GetStatus(messageBuffer);
                response = aAkeSessionData->MessagingInterface.GetCTypeResponse(messageBuffer);

                if ((responseAccepted == response) && (statusNoError == status))
                {
                    returnValue = SUCCESS;
                }
                else if ((responseRejected == response) && (statusNoMoreAuth == status))
                {
                    returnValue = DTCP_SINK_COUNT_LIMIT_REACHED;
                }
                else
                {
                    returnValue = FAILURE;
                }
            }
        }
        free(messageBuffer);
    }
    else
    {
        returnValue = FAILURE;
    }

    return returnValue;
} // SendChallengeCommand

int ProcessChallengeCommand(DtcpAkeIpSessionData *aAkeSessionData,
                            unsigned char *aMessageBuffer,
                            unsigned int aMessageBufferSize)
{
    int returnValue = SUCCESS;
    int sendReturnValue = SUCCESS;
    unsigned int bufferSize;
    unsigned char *challengeBuffer;
    unsigned int challengeBufferSize;

#ifdef DEMO_MODE
    DEBUG_MSG(MSG_DEBUG, ("\r\n.....................................\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    "..  Processing challenge command   ..\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ".....................................\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
    DEBUG_MSG(MSG_DEBUG, ("Processing challenge command\n"));
    PrintMessage(aMessageBuffer, aMessageBufferSize, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif

    if (stateIdle == aAkeSessionData->CurrentAuthState)
    {
        challengeBuffer = aAkeSessionData->MessagingInterface.GetCommandData(aMessageBuffer, 
                                                                            &challengeBufferSize);

        returnValue = DtcpAkeCore_ConsumeChallengeData(aAkeSessionData->AkeCoreSessionHandle,
                                                       challengeBuffer,
                                                       challengeBufferSize);
    }
    else
    {
        returnValue = DTCP_INVALID_COMMAND_SEQUENCE;
    }

    if (IS_SUCCESS(returnValue))
    {
        aAkeSessionData->MessagingInterface.SetStatus(aMessageBuffer, 0);
        aAkeSessionData->CurrentAuthState = stateChallenge;
        aAkeSessionData->MessagingInterface.SetCTypeResponse(aMessageBuffer, responseAccepted);
    }
    else
    {
        aAkeSessionData->CurrentAuthState = stateError;
        aAkeSessionData->MessagingInterface.SetCTypeResponse(aMessageBuffer, responseRejected);
        switch (returnValue)
        {
        case (DTCP_SINK_COUNT_LIMIT_REACHED):
            DEBUG_MSG(MSG_INFO, ("Sink count limit reached!\n"));
            aAkeSessionData->MessagingInterface.SetStatus(aMessageBuffer, statusNoMoreAuth);          
            break;

        case (DTCP_INVALID_COMMAND_SEQUENCE):
            aAkeSessionData->MessagingInterface.SetStatus(aMessageBuffer, statusIncorrectCommandOrder);
            break;

        default:
            aAkeSessionData->MessagingInterface.SetStatus(aMessageBuffer, statusAnyOtherError);
            break;
        }
    }

    aAkeSessionData->AkeLabel = aAkeSessionData->MessagingInterface.GetAkeLabel(aMessageBuffer);
    aAkeSessionData->MessagingInterface.SetCommandData(aMessageBuffer, NULL, 0);
    
    // Send response
#ifdef DEMO_MODE
    DEBUG_MSG(MSG_DEBUG, ("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ">>   Sending challenge response    >>\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
    DEBUG_MSG(MSG_DEBUG, ("Sending challenge response\n"));
    PrintMessage(aMessageBuffer, aMessageBufferSize, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif    

    bufferSize = aAkeSessionData->AkeIpData->AkeIpMinMessageSize;
    sendReturnValue = aAkeSessionData->MessagingInterface.Send(aAkeSessionData->TransportHandle,
                                                               aMessageBuffer,
                                                               &bufferSize,
                                                               SEND_TIMEOUT);

    if (IS_SUCCESS(returnValue))
    {
        returnValue = sendReturnValue;
    }

    return returnValue;
} // ProcessChallengeCommand

int SendResponseCommand(DtcpAkeIpSessionData *aAkeSessionData)
{
    int returnValue = SUCCESS;
    unsigned char *messageBuffer = NULL;
    int messageBufferLength = 0;
    unsigned char *responseBuffer = NULL;
    unsigned int responseBufferSize = 0;
    EnDeviceMode deviceMode;
    int status;
    int response;

    returnValue = DtcpAkeCore_GetDeviceMode(aAkeSessionData->AkeCoreSessionHandle, &deviceMode);

    returnValue = DtcpAkeCore_CreateResponseData(aAkeSessionData->AkeCoreSessionHandle,
                                                 &responseBuffer,
                                                 &responseBufferSize);

    if (IS_SUCCESS(returnValue))
    {
        messageBufferLength = responseBufferSize + aAkeSessionData->AkeIpData->AkeIpMinMessageSize;
        messageBuffer = (unsigned char *)malloc(messageBufferLength);
    }

    if (messageBuffer)
    {
        memset(messageBuffer, 0, messageBufferLength);

        if ((SUCCESS == returnValue) && (NULL != responseBuffer))
        {
            if (deviceModeSource == deviceMode)
            {
                aAkeSessionData->MessagingInterface.SetSubFuncDep(messageBuffer, 0);
            }
            else
            {
                aAkeSessionData->MessagingInterface.SetSubFuncDep(messageBuffer, 1);
            }

            aAkeSessionData->MessagingInterface.SetAkeLabel(messageBuffer, aAkeSessionData->AkeLabel);
            aAkeSessionData->MessagingInterface.SetAkeProcedures(messageBuffer, akeTypeMaskFullAuth);
            aAkeSessionData->MessagingInterface.SetCommandType(messageBuffer, cmdResponse);
            aAkeSessionData->MessagingInterface.SetCTypeResponse(messageBuffer, ctypeControl);
            aAkeSessionData->MessagingInterface.SetExchangeKeys(messageBuffer, exchKeyMaskAes128);
            aAkeSessionData->MessagingInterface.SetStatus(messageBuffer, 0xF);
            aAkeSessionData->MessagingInterface.SetCommandData(messageBuffer, 
                                                               responseBuffer,
                                                               responseBufferSize);

#ifdef DEMO_MODE
            DEBUG_MSG(MSG_DEBUG, ("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    ">>    Sending response command     >>\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
            DEBUG_MSG(MSG_DEBUG, ("Sending response command\n"));
            PrintMessage(messageBuffer, messageBufferLength, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif

            returnValue = aAkeSessionData->MessagingInterface.Send(aAkeSessionData->TransportHandle,
                                                                   messageBuffer,
                                                                   &messageBufferLength,
                                                                   SEND_TIMEOUT);
            if (IS_SUCCESS(returnValue))
            {
#ifdef DEMO_MODE
                DEBUG_MSG(MSG_DEBUG, ("\r\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
                DEBUG_MSG(MSG_DEBUG, (    "<<   Received response response    <<\r\n"));
                DEBUG_MSG(MSG_DEBUG, (    "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
                DEBUG_MSG(MSG_DEBUG, ("Received response response\n"));
                PrintMessage(messageBuffer, messageBufferLength, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif

                status = aAkeSessionData->MessagingInterface.GetStatus(messageBuffer);
                response = aAkeSessionData->MessagingInterface.GetCTypeResponse(messageBuffer);

                if ((statusNoError == status) && (responseAccepted == response))
                {
                    returnValue = SUCCESS;
                }
                else
                {
                    returnValue = FAILURE;
                }
            }

            DtcpAkeCore_ReleaseBuffers(aAkeSessionData->AkeCoreSessionHandle,
                                       akeBufferResponse);

        }
        free(messageBuffer);
        aAkeSessionData->ResponseSentTime = OsWrap_GetCurrentTime();
    }
    else
    {
        returnValue = FAILURE;
    }

    return returnValue;
} // SendResponseCommand

int ProcessResponseCommand(DtcpAkeIpSessionData *aAkeSessionData,
                           unsigned char *aMessageBuffer,
                           unsigned int aMessageBufferSize)
{
    int returnValue = SUCCESS;
    unsigned int bufferSize;
    unsigned char *responseBuffer;
    unsigned int responseBufferSize;

#ifdef DEMO_MODE
    DEBUG_MSG(MSG_DEBUG, ("\r\n.....................................\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    "..   Processing response command   ..\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ".....................................\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
    DEBUG_MSG(MSG_DEBUG, ("Processing response command\n"));
    PrintMessage(aMessageBuffer, aMessageBufferSize, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif

    if (stateChallenge == aAkeSessionData->CurrentAuthState)
    {
        responseBuffer = aAkeSessionData->MessagingInterface.GetCommandData(aMessageBuffer, 
                                                                            &responseBufferSize);

        returnValue = DtcpAkeCore_ConsumeResponseData(aAkeSessionData->AkeCoreSessionHandle,
                                                      responseBuffer,
                                                      responseBufferSize,
                                                      &aAkeSessionData->OurSrmUpdateRequiredFlag);
    }
    else
    {
        returnValue = DTCP_INVALID_COMMAND_SEQUENCE;
    }

    if (IS_SUCCESS(returnValue))
    {
        aAkeSessionData->MessagingInterface.SetStatus(aMessageBuffer, 0);
        aAkeSessionData->CurrentAuthState = stateResponse;
        aAkeSessionData->ResponseReceivedTime = OsWrap_GetCurrentTime();
        aAkeSessionData->MessagingInterface.SetCTypeResponse(aMessageBuffer, responseAccepted);
    }
    else
    {
        aAkeSessionData->MessagingInterface.SetStatus(aMessageBuffer, statusAnyOtherError);
        aAkeSessionData->MessagingInterface.SetCTypeResponse(aMessageBuffer, responseRejected);
    }

    // Send response
    aAkeSessionData->MessagingInterface.SetCommandData(aMessageBuffer, NULL, 0);

#ifdef DEMO_MODE
    DEBUG_MSG(MSG_DEBUG, ("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ">>   Sending response response     >>\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
    DEBUG_MSG(MSG_DEBUG, ("Sending response response\n"));
    PrintMessage(aMessageBuffer, aMessageBufferSize, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif
    
    bufferSize = aAkeSessionData->AkeIpData->AkeIpMinMessageSize;
    aAkeSessionData->MessagingInterface.Send(aAkeSessionData->TransportHandle,
                                             aMessageBuffer,
                                             &bufferSize,
                                             SEND_TIMEOUT);

    return returnValue;
} // ProcessResponseCommand

int SendExchangeKeyCommand(DtcpAkeIpSessionData *aAkeSessionData,
                           EnExchKeyId           aExchKeyId)
{
    int returnValue = SUCCESS;
    unsigned char *messageBuffer = NULL;
    int messageBufferLength = 0;
    unsigned char *exchangeKeyBuffer = NULL;
    unsigned int exchangeKeyBufferSize = 0;
    int status;
    int response;

    returnValue = DtcpAkeCore_CreateExchangeKeyData(aAkeSessionData->AkeCoreSessionHandle,
                                                    aExchKeyId,
                                                    &exchangeKeyBuffer,
                                                    &exchangeKeyBufferSize);

    if (IS_SUCCESS(returnValue))
    {
        messageBufferLength = exchangeKeyBufferSize + aAkeSessionData->AkeIpData->AkeIpMinMessageSize;
        messageBuffer = (unsigned char *)malloc(messageBufferLength);
    }

    if (messageBuffer)
    {
        memset(messageBuffer, 0, messageBufferLength);

        if ((SUCCESS == returnValue) && (NULL != exchangeKeyBuffer))
        {
            aAkeSessionData->MessagingInterface.SetAkeLabel(messageBuffer, aAkeSessionData->AkeLabel);
            aAkeSessionData->MessagingInterface.SetAkeProcedures(messageBuffer, akeTypeMaskFullAuth);
            aAkeSessionData->MessagingInterface.SetCommandType(messageBuffer, cmdExchangeKey);
            aAkeSessionData->MessagingInterface.SetCTypeResponse(messageBuffer, ctypeControl);
            aAkeSessionData->MessagingInterface.SetExchangeKeys(messageBuffer, exchKeyMaskAes128);
            aAkeSessionData->MessagingInterface.SetStatus(messageBuffer, 0xF);
            aAkeSessionData->MessagingInterface.SetSubFuncDep(messageBuffer, 0);
            aAkeSessionData->MessagingInterface.SetCommandData(messageBuffer, 
                                                               exchangeKeyBuffer,
                                                               exchangeKeyBufferSize);

#ifdef DEMO_MODE
            DEBUG_MSG(MSG_DEBUG, ("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    ">>  Sending exchange key command   >>\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
            DEBUG_MSG(MSG_DEBUG, ("Sending exchange key command\n"));
            PrintMessage(messageBuffer, messageBufferLength, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif

            returnValue = aAkeSessionData->MessagingInterface.Send(aAkeSessionData->TransportHandle,
                                                                   messageBuffer,
                                                                   &messageBufferLength,
                                                                   SEND_TIMEOUT);

            if (IS_SUCCESS(returnValue))
            {
#ifdef DEMO_MODE
                DEBUG_MSG(MSG_DEBUG, ("\r\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
                DEBUG_MSG(MSG_DEBUG, (    "<< Received exchange key response  <<\r\n"));
                DEBUG_MSG(MSG_DEBUG, (    "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
                DEBUG_MSG(MSG_DEBUG, ("Received exchange key response\n"));
                PrintMessage(messageBuffer, messageBufferLength, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif

                status = aAkeSessionData->MessagingInterface.GetStatus(messageBuffer);
                response = aAkeSessionData->MessagingInterface.GetCTypeResponse(messageBuffer);

                if ((statusNoError == status) && (responseAccepted == response))
                {
                    returnValue = SUCCESS;
                }
                else
                {
                    returnValue = FAILURE;
                }
            }
            DtcpAkeCore_ReleaseBuffers(aAkeSessionData->AkeCoreSessionHandle,
                                       akeBufferExchKeyAes128);
        }
        free(messageBuffer);
        aAkeSessionData->ExchangeKeySentTime = OsWrap_GetCurrentTime();
        aAkeSessionData->CurrentAuthState = stateAuthenticated;
    }
    else
    {
        returnValue = FAILURE;
    }

    return returnValue;
} // SendExchangeKeyCommand

int ProcessExchangeKeyCommand(DtcpAkeIpSessionData *aAkeSessionData,
                              unsigned char *aMessageBuffer,
                              unsigned int aMessageBufferSize)
{
    int returnValue = SUCCESS;
    unsigned int bufferSize;
    unsigned char *exchangeKeyBuffer;
    unsigned int exchangeKeyBufferSize;
    EnExchKeyMask exchangeKeyMask;
    EnExchKeyId   exchangeKeyId;

#ifdef DEMO_MODE
    DEBUG_MSG(MSG_DEBUG, ("\r\n.....................................\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ".. Processing exchange key command ..\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ".....................................\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
    DEBUG_MSG(MSG_DEBUG, ("Processing exchange key command\n"));
    PrintMessage(aMessageBuffer, aMessageBufferSize, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif
    
    if (stateResponse == aAkeSessionData->CurrentAuthState)
    {
        exchangeKeyBuffer = aAkeSessionData->MessagingInterface.GetCommandData(aMessageBuffer, 
                                                                            &exchangeKeyBufferSize);
        
        exchangeKeyMask = (EnExchKeyMask)aAkeSessionData->MessagingInterface.GetExchangeKeys(aMessageBuffer);

        if (exchangeKeyMask && exchKeyMaskAes128)
        {
            exchangeKeyId = exchKeyIdAes128;
        }
        else
        {
            returnValue = FAILURE;
        }
    }
    else
    {
        returnValue = DTCP_INVALID_COMMAND_SEQUENCE;
    }

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpAkeCore_ConsumeExchangeKeyData(aAkeSessionData->AkeCoreSessionHandle,
                                                         exchangeKeyId,
                                                         exchangeKeyBuffer,
                                                         exchangeKeyBufferSize);
    }   

    if (IS_SUCCESS(returnValue))
    {
        aAkeSessionData->MessagingInterface.SetStatus(aMessageBuffer, statusNoError);
        aAkeSessionData->CurrentAuthState = stateAuthenticated;
        aAkeSessionData->ExchangeKeyReceivedTime = OsWrap_GetCurrentTime();
        aAkeSessionData->MessagingInterface.SetCTypeResponse(aMessageBuffer, responseAccepted);
    }
    else
    {
        aAkeSessionData->MessagingInterface.SetStatus(aMessageBuffer, statusAnyOtherError);
        aAkeSessionData->MessagingInterface.SetCTypeResponse(aMessageBuffer, responseRejected);
    }

    // Send response
    aAkeSessionData->MessagingInterface.SetCommandData(aMessageBuffer, NULL, 0);

#ifdef DEMO_MODE
    DEBUG_MSG(MSG_DEBUG, ("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ">> Sending exchange key response  >>\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
    DEBUG_MSG(MSG_DEBUG, ("Sending exchange key response\n"));
    PrintMessage(aMessageBuffer, aMessageBufferSize, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif
    
    bufferSize = aAkeSessionData->AkeIpData->AkeIpMinMessageSize;
    returnValue = aAkeSessionData->MessagingInterface.Send(aAkeSessionData->TransportHandle,
                                                           aMessageBuffer,
                                                           &bufferSize,
                                                           SEND_TIMEOUT);

    return returnValue;
} // ProcessExchangeKeyCommand

int SendSrmCommand(DtcpAkeIpSessionData *aAkeSessionData)
{
    int returnValue = SUCCESS;
    unsigned char *messageBuffer = NULL;
    int messageBufferLength = 0;
    unsigned char *srmBuffer = NULL;
    unsigned int srmBufferSize = 0;
    EnDeviceMode deviceMode;
    int status;
    int response;

    returnValue = DtcpAkeCore_GetDeviceMode(aAkeSessionData->AkeCoreSessionHandle, &deviceMode);

    returnValue = DtcpAkeCore_CreateSrmData(aAkeSessionData->AkeCoreSessionHandle,
                                            &srmBuffer,
                                            &srmBufferSize);

    if (IS_SUCCESS(returnValue) && (0 < srmBufferSize))
    {
        messageBufferLength = srmBufferSize + aAkeSessionData->AkeIpData->AkeIpMinMessageSize;
        messageBuffer = (unsigned char *)malloc(messageBufferLength);

        if (messageBuffer)
        {
            memset(messageBuffer, 0, messageBufferLength);

            if ((SUCCESS == returnValue) && (NULL != srmBuffer))
            {
                if (deviceModeSource == deviceMode)
                {
                    aAkeSessionData->MessagingInterface.SetSubFuncDep(messageBuffer, 0);
                }
                else
                {
                    aAkeSessionData->MessagingInterface.SetSubFuncDep(messageBuffer, 1);
                }

                aAkeSessionData->MessagingInterface.SetAkeLabel(messageBuffer, aAkeSessionData->AkeLabel);
                aAkeSessionData->MessagingInterface.SetAkeProcedures(messageBuffer, akeTypeMaskFullAuth);
                aAkeSessionData->MessagingInterface.SetCommandType(messageBuffer, cmdSRM);
                aAkeSessionData->MessagingInterface.SetCTypeResponse(messageBuffer, ctypeControl);
                aAkeSessionData->MessagingInterface.SetExchangeKeys(messageBuffer, 0x00);
                aAkeSessionData->MessagingInterface.SetStatus(messageBuffer, 0xF);
                aAkeSessionData->MessagingInterface.SetCommandData(messageBuffer, 
                                                                srmBuffer,
                                                                srmBufferSize);

#ifdef DEMO_MODE
                DEBUG_MSG(MSG_DEBUG, ("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
                DEBUG_MSG(MSG_DEBUG, (    ">>      Sending SRM command        >>\r\n"));
                DEBUG_MSG(MSG_DEBUG, (    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
                DEBUG_MSG(MSG_DEBUG, ("Sending SRM command\n"));
                PrintMessage(messageBuffer, messageBufferLength, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif

                returnValue = aAkeSessionData->MessagingInterface.Send(aAkeSessionData->TransportHandle,
                                                                    messageBuffer,
                                                                    &messageBufferLength,
                                                                    SEND_TIMEOUT);
                if (IS_SUCCESS(returnValue))
                {
#ifdef DEMO_MODE
                    DEBUG_MSG(MSG_DEBUG, ("\r\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
                    DEBUG_MSG(MSG_DEBUG, (    "<<      Received SRM response      <<\r\n"));
                    DEBUG_MSG(MSG_DEBUG, (    "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
                    DEBUG_MSG(MSG_DEBUG, ("Received SRM response\n"));
                    PrintMessage(messageBuffer, messageBufferLength, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif

                    status = aAkeSessionData->MessagingInterface.GetStatus(messageBuffer);
                    response = aAkeSessionData->MessagingInterface.GetCTypeResponse(messageBuffer);

                    if ((statusNoError == status) && (responseAccepted == response))
                    {
                        returnValue = SUCCESS;
                    }
                    else
                    {
                        returnValue = FAILURE;
                    }
                }
                
                DtcpAkeCore_ReleaseBuffers(aAkeSessionData->AkeCoreSessionHandle,
                                           akeBufferSrm);
            }
            free(messageBuffer);

        }
        else
        {
            returnValue = FAILURE;
        }
    }

    if (IS_SUCCESS(returnValue))
    {
        if (FALSE == aAkeSessionData->OurSrmUpdateRequiredFlag)
        {
            aAkeSessionData->CurrentAuthState = stateCompleted;
        }
    }

    return returnValue;
} // SendSrmCommand

int ProcessSrmCommand(DtcpAkeIpSessionData *aAkeSessionData,
                      unsigned char *aMessageBuffer,
                      unsigned int aMessageBufferSize)
{
    int returnValue = SUCCESS;
    unsigned int bufferSize;
    unsigned char *srmBuffer;
    unsigned int srmBufferSize;

#ifdef DEMO_MODE
    DEBUG_MSG(MSG_DEBUG, ("\r\n.....................................\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    "..     Processing SRM command      ..\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ".....................................\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
    DEBUG_MSG(MSG_DEBUG, ("Processing SRM command\n"));
    PrintMessage(aMessageBuffer, aMessageBufferSize, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif
    
    srmBuffer = aAkeSessionData->MessagingInterface.GetCommandData(aMessageBuffer, &srmBufferSize);

    returnValue = DtcpAkeCore_ConsumeSrmData(aAkeSessionData->AkeCoreSessionHandle, 
                                             srmBuffer,
                                             srmBufferSize);

    if (IS_SUCCESS(returnValue))
    {
        aAkeSessionData->MessagingInterface.SetStatus(aMessageBuffer, statusNoError);
        aAkeSessionData->MessagingInterface.SetCTypeResponse(aMessageBuffer, responseAccepted);
    }
    else
    {
        aAkeSessionData->MessagingInterface.SetStatus(aMessageBuffer, statusAnyOtherError);
        aAkeSessionData->MessagingInterface.SetCTypeResponse(aMessageBuffer, responseRejected);
    }

    // Send response
    aAkeSessionData->MessagingInterface.SetCommandData(aMessageBuffer, NULL, 0);

#ifdef DEMO_MODE
    DEBUG_MSG(MSG_DEBUG, ("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ">>     Sending SRM response        >>\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
    DEBUG_MSG(MSG_DEBUG, ("Sending SRM response\n"));
    PrintMessage(aMessageBuffer, aMessageBufferSize, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif
    
    bufferSize = aAkeSessionData->AkeIpData->AkeIpMinMessageSize;
    returnValue = aAkeSessionData->MessagingInterface.Send(aAkeSessionData->TransportHandle,
                                                           aMessageBuffer,
                                                           &bufferSize,
                                                           SEND_TIMEOUT);
    aAkeSessionData->SrmReceivedTime = OsWrap_GetCurrentTime();

    if (IS_SUCCESS(returnValue))
    {
        aAkeSessionData->CurrentAuthState = stateCompleted;
    }

    return returnValue;
} // ProcessSrmCommand

int SendContentKeyRequestCommand(DtcpAkeIpSessionData *aAkeSessionData,
                                 int                  *aExchangeKeyLabel)
{
    int returnValue = SUCCESS;
    unsigned char *messageBuffer = NULL;
    int messageBufferLength = 0;
    unsigned char *contentKeyRequestBuffer = NULL;
    unsigned int contentKeyRequestBufferSize = DTCP_CONTENT_KEY_REQUEST_SIZE;
    unsigned char *messageData = NULL;
    int messageLength;

    contentKeyRequestBuffer = (unsigned char *)malloc(contentKeyRequestBufferSize);

    if (!contentKeyRequestBuffer)
    {
        returnValue = FAILURE;
    }
    
    if (IS_SUCCESS(returnValue))
    {
        messageBufferLength = contentKeyRequestBufferSize + aAkeSessionData->AkeIpData->AkeIpMinMessageSize;
        messageBuffer = (unsigned char *)malloc(messageBufferLength);
    }

    if (messageBuffer)
    {
        memset(messageBuffer, 0, messageBufferLength);

        if (SUCCESS == returnValue)
        {
            aAkeSessionData->MessagingInterface.SetAkeLabel(messageBuffer, 0);
            aAkeSessionData->MessagingInterface.SetAkeProcedures(messageBuffer, 0);
            aAkeSessionData->MessagingInterface.SetCommandType(messageBuffer, cmdContentKey);
            aAkeSessionData->MessagingInterface.SetCTypeResponse(messageBuffer, ctypeControl);
            aAkeSessionData->MessagingInterface.SetExchangeKeys(messageBuffer, 0);
            aAkeSessionData->MessagingInterface.SetStatus(messageBuffer, 0xF);
            aAkeSessionData->MessagingInterface.SetSubFuncDep(messageBuffer, 0);
            aAkeSessionData->MessagingInterface.SetCommandData(messageBuffer, 
                                                               contentKeyRequestBuffer,
                                                               contentKeyRequestBufferSize);

#ifdef DEMO_MODE
            DEBUG_MSG(MSG_DEBUG, ("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    ">> Sending content key request command >>\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
            DEBUG_MSG(MSG_DEBUG, ("Sending content key request command\n"));
            PrintMessage(messageBuffer, messageBufferLength, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif

            returnValue = aAkeSessionData->MessagingInterface.Send(aAkeSessionData->TransportHandle,
                                                                   messageBuffer,
                                                                   &messageBufferLength,
                                                                   SEND_TIMEOUT);

            if (IS_SUCCESS(returnValue))
            {
#ifdef DEMO_MODE
                DEBUG_MSG(MSG_DEBUG, ("\r\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
                DEBUG_MSG(MSG_DEBUG, (    "<< Received content key request response <<\r\n"));
                DEBUG_MSG(MSG_DEBUG, (    "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
                DEBUG_MSG(MSG_DEBUG, ("Received content key request response\n"));
                PrintMessage(messageBuffer, messageBufferLength, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif

                messageData = aAkeSessionData->MessagingInterface.GetCommandData(messageBuffer, &messageLength);
                *aExchangeKeyLabel = messageData[0];
            }
        }
        free(messageBuffer);
    }
    else
    {
        returnValue = FAILURE;
    }

    if (contentKeyRequestBuffer)
    {
        free(contentKeyRequestBuffer);
    }

    return returnValue;
} // SendContentKeyRequestCommand

int ProcessContentKeyRequestCommand(DtcpAkeIpData               *aAkeIpData,
                                    DtcpAkeIpMessagingInterface *aMessagingInterface,
                                    DtcpTransportHandle          aTransportHandle,
                                    unsigned char               *aMessageBuffer,
                                    unsigned int                 aMessageBufferSize)
{
    int returnValue = SUCCESS;
    unsigned int bufferSize;
    unsigned char *contentKeyRequestBuffer = NULL;
    unsigned int contentKeyRequestBufferSize = DTCP_CONTENT_KEY_REQUEST_SIZE;
    unsigned int exchangeKeyLabel;
    unsigned char nonce[DTCP_CONTENT_KEY_NONCE_SIZE];

#ifdef DEMO_MODE
    DEBUG_MSG(MSG_DEBUG, ("\r\n............................................\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ".. Processing content key request command ..\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    "............................................\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
    DEBUG_MSG(MSG_DEBUG, ("Processing content key request command\n"));
    PrintMessage(aMessageBuffer, aMessageBufferSize, aMessagingInterface); // Uses MSG_DEBUG
#endif

    contentKeyRequestBuffer = (unsigned char *)malloc(contentKeyRequestBufferSize);
    if (!contentKeyRequestBuffer)
    {
        returnValue = FAILURE;
    }

    if (IS_SUCCESS(returnValue))
    {
        // Get content key request info
        returnValue = DtcpAkeCore_GetSourceExchangeKeyLabel(&exchangeKeyLabel);
        if (IS_SUCCESS(returnValue))
        {
            returnValue = DtcpAkeCore_GetRealTimeNonce(nonce);
        }
    }

    // Fill the content key request buffer
    if (IS_SUCCESS(returnValue))
    {
        memset(contentKeyRequestBuffer, 0, contentKeyRequestBufferSize);

        contentKeyRequestBuffer[0] = (char)exchangeKeyLabel;
        contentKeyRequestBuffer[1] = 1 << 4;
        contentKeyRequestBuffer[3] = 1;
        memcpy(&contentKeyRequestBuffer[4], nonce, DTCP_CONTENT_KEY_NONCE_SIZE);
    }

    if (IS_SUCCESS(returnValue))
    {
        aMessagingInterface->SetStatus(aMessageBuffer, statusNoError);
        aMessagingInterface->SetCTypeResponse(aMessageBuffer, responseAccepted);
        aMessagingInterface->SetCommandData(aMessageBuffer, contentKeyRequestBuffer, DTCP_CONTENT_KEY_REQUEST_SIZE);
    }
    else
    {
        aMessagingInterface->SetStatus(aMessageBuffer, statusAnyOtherError);
        aMessagingInterface->SetCTypeResponse(aMessageBuffer, responseRejected);
    }

    // Send content key request response

#ifdef DEMO_MODE
    DEBUG_MSG(MSG_DEBUG, ("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ">> Sending content key request response >>\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
    DEBUG_MSG(MSG_DEBUG, ("Sending content key request response\n"));
    PrintMessage(aMessageBuffer, aMessageBufferSize, aMessagingInterface); // Uses MSG_DEBUG
#endif
    
    bufferSize = aAkeIpData->AkeIpMinMessageSize + DTCP_CONTENT_KEY_REQUEST_SIZE;
    returnValue = aMessagingInterface->Send(aTransportHandle,
                                            aMessageBuffer,
                                            &bufferSize,
                                            SEND_TIMEOUT);

    if (contentKeyRequestBuffer)
    {
        free(contentKeyRequestBuffer);
    }

    return returnValue;
} // ProcessContentKeyRequestCommand

int SendCancelCommand(DtcpAkeIpSessionData *aAkeSessionData)
{
    int returnValue = SUCCESS;
    unsigned char *messageBuffer;
    unsigned int messageBufferLength;
    unsigned int status;
    EnDeviceMode deviceMode;

    returnValue = DtcpAkeCore_GetDeviceMode(aAkeSessionData->AkeCoreSessionHandle, &deviceMode);

    if (!aAkeSessionData)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        messageBufferLength = aAkeSessionData->AkeIpData->AkeIpMinMessageSize;
        messageBuffer = (unsigned char *)malloc(messageBufferLength);

        if (messageBuffer)
        {
            memset(messageBuffer, 0, messageBufferLength);

            if (deviceModeSource == deviceMode)
            {
                aAkeSessionData->MessagingInterface.SetSubFuncDep(messageBuffer, 0);
            }
            else
            {
                aAkeSessionData->MessagingInterface.SetSubFuncDep(messageBuffer, 1);
            }
            aAkeSessionData->MessagingInterface.SetAkeLabel(messageBuffer, aAkeSessionData->AkeLabel);
            aAkeSessionData->MessagingInterface.SetAkeProcedures(messageBuffer, 0xFF);
            aAkeSessionData->MessagingInterface.SetCommandType(messageBuffer, cmdCancel);
            aAkeSessionData->MessagingInterface.SetCTypeResponse(messageBuffer, ctypeControl);
            aAkeSessionData->MessagingInterface.SetExchangeKeys(messageBuffer, 0xFF);
            aAkeSessionData->MessagingInterface.SetStatus(messageBuffer, 0xF);
            aAkeSessionData->MessagingInterface.SetCommandData(messageBuffer, NULL, 0);

#ifdef DEMO_MODE
            DEBUG_MSG(MSG_DEBUG, ("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    ">>        Sending cancel command        >>\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
            DEBUG_MSG(MSG_DEBUG, ("Sending cancel command\n"));
            PrintMessage(messageBuffer, messageBufferLength, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif

            returnValue = aAkeSessionData->MessagingInterface.Send(aAkeSessionData->TransportHandle,
                                                                   messageBuffer,
                                                                   &messageBufferLength,
                                                                   SEND_TIMEOUT);

            if (IS_SUCCESS(returnValue))
            {
#ifdef DEMO_MODE
                DEBUG_MSG(MSG_DEBUG, ("\r\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
                DEBUG_MSG(MSG_DEBUG, (    "<<    Received cancel response     <<\r\n"));
                DEBUG_MSG(MSG_DEBUG, (    "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
                DEBUG_MSG(MSG_DEBUG, ("Received cancel response\n"));
                PrintMessage(messageBuffer, messageBufferLength, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif
                status = aAkeSessionData->MessagingInterface.GetStatus(messageBuffer);
                if (0 == status)
                {
                    aAkeSessionData->CurrentAuthState = stateCancelled;
                }
                else
                {
                    aAkeSessionData->CurrentAuthState = stateError;
                }
            }
            free (messageBuffer);
        }
        else
        {
            returnValue = FAILURE;
        }
    }

    return returnValue;
} // SendCancelCommand

int ProcessCancelCommand(DtcpAkeIpSessionData *aAkeSessionData,
                         unsigned char        *aMessageBuffer,
                         unsigned int          aMessageBufferSize)
{
    int returnValue = SUCCESS;
    unsigned int bufferSize;

    if (!aAkeSessionData || !aMessageBuffer)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        bufferSize = aAkeSessionData->AkeIpData->AkeIpMinMessageSize;
#ifdef DEMO_MODE
        DEBUG_MSG(MSG_DEBUG, ("\r\n.....................................\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    "..   Processing cancel command     ..\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    ".....................................\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
        DEBUG_MSG(MSG_DEBUG, ("Processing cancel command\n"));
        PrintMessage(aMessageBuffer, aMessageBufferSize, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif

        if ((stateChallenge == aAkeSessionData->CurrentAuthState) ||
            (stateResponse  == aAkeSessionData->CurrentAuthState))
        {
            aAkeSessionData->CurrentAuthState = stateCancelled;
            aAkeSessionData->MessagingInterface.SetStatus(aMessageBuffer, statusNoError);
            aAkeSessionData->MessagingInterface.SetCTypeResponse(aMessageBuffer, responseAccepted);
        }
        else
        {
            aAkeSessionData->CurrentAuthState = stateError;
            aAkeSessionData->MessagingInterface.SetStatus(aMessageBuffer, statusAnyOtherError);
            aAkeSessionData->MessagingInterface.SetCTypeResponse(aMessageBuffer, responseRejected);
            returnValue = DTCP_INVALID_COMMAND_SEQUENCE;
        }
    }

#ifdef DEMO_MODE
    DEBUG_MSG(MSG_DEBUG, ("\r\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ">>       Sending cancel response        >>\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n"));
#endif
#if !defined(DEMO_MODE) || defined(DEMO_PACKET)
    DEBUG_MSG(MSG_DEBUG, ("Sending cancel response\n"));
    PrintMessage(aMessageBuffer, aMessageBufferSize, &aAkeSessionData->MessagingInterface); // Uses MSG_DEBUG
#endif
    
    returnValue = aAkeSessionData->MessagingInterface.Send(aAkeSessionData->TransportHandle,
                                                            aMessageBuffer,
                                                            &bufferSize,
                                                            SEND_TIMEOUT);

    return returnValue;
} // ProcessCancelCommand

int ProcessCommand(DtcpAkeIpSessionData *aAkeSessionData,
                   unsigned char        *aMessageBuffer,
                   unsigned int          aMessageBufferSize)
{
    int returnValue = SUCCESS;
    EnCommands commandType;
    EnDeviceMode deviceMode;

    returnValue = DtcpAkeCore_GetDeviceMode(aAkeSessionData->AkeCoreSessionHandle, &deviceMode);

    if (IS_SUCCESS(returnValue))
    {
        commandType = aAkeSessionData->MessagingInterface.GetCommandType(aMessageBuffer);
        switch (commandType)
        {
        case cmdStatus:
            returnValue = ProcessStatusCommand(aAkeSessionData, 
                                               &aAkeSessionData->MessagingInterface,
                                               aAkeSessionData->TransportHandle,
                                               aMessageBuffer,
                                               aMessageBufferSize);
            break;

        case cmdChallenge:
            returnValue = ProcessChallengeCommand(aAkeSessionData, aMessageBuffer, aMessageBufferSize);
            if (deviceModeSource == deviceMode)
            {
                if (IS_SUCCESS(returnValue) && 
                   (stateError         != aAkeSessionData->CurrentAuthState) &&
                   (stateCancelled     != aAkeSessionData->CurrentAuthState))
                {
                    returnValue = SendChallengeCommand(aAkeSessionData);
                }
                if (IS_SUCCESS(returnValue) && 
                   (stateError         != aAkeSessionData->CurrentAuthState) &&
                   (stateCancelled     != aAkeSessionData->CurrentAuthState))
                {                    
                    //returnValue = SendCancelCommand(aAkeSessionData);
                    returnValue = SendResponseCommand(aAkeSessionData);
                }
            }
            break;

        case cmdResponse:
            returnValue = ProcessResponseCommand(aAkeSessionData, aMessageBuffer, aMessageBufferSize);
            if (IS_SUCCESS(returnValue))
            {
                if (deviceModeSource == deviceMode)
                {
                    returnValue = SendExchangeKeyCommand(aAkeSessionData, exchKeyIdAes128);
                    if (IS_SUCCESS(returnValue) && 
                       (stateError         != aAkeSessionData->CurrentAuthState) &&
                       (stateCancelled     != aAkeSessionData->CurrentAuthState))
                    {
                        returnValue = SendSrmCommand(aAkeSessionData);
                    }
                }
                else
                {
                    returnValue = SendResponseCommand(aAkeSessionData);
                }
            }
            break;

        case cmdExchangeKey:
            returnValue = ProcessExchangeKeyCommand(aAkeSessionData, aMessageBuffer, aMessageBufferSize);
            if (IS_SUCCESS(returnValue))
            {
                if (deviceModeSink == deviceMode)
                {
                    returnValue = SendSrmCommand(aAkeSessionData);
                }
            }
            break;

        case cmdSRM:
            returnValue = ProcessSrmCommand(aAkeSessionData, aMessageBuffer, aMessageBufferSize);
            break;

        case cmdContentKey:
            returnValue = ProcessContentKeyRequestCommand(aAkeSessionData,
                                                          &aAkeSessionData->MessagingInterface,
                                                          aAkeSessionData->TransportHandle,
                                                          aMessageBuffer, aMessageBufferSize);
            break;

        case cmdCancel:
            returnValue = ProcessCancelCommand(aAkeSessionData, aMessageBuffer, aMessageBufferSize);
            break;

        case cmdInvalid:
            break;

        default:
            break;
        }
    }
    return returnValue;
} // ProcessCommand

int ListenCallback(DtcpTransportHandle          aTransportHandle,
                   void                        *aUserData,
                   DtcpAkeIpMessagingInterface *aMessagingInterface)
{
    int returnValue = SUCCESS;
    unsigned char messageBuffer[MAX_MESSAGE_SIZE];
    unsigned int messageSize = MAX_MESSAGE_SIZE;
    DtcpAkeIpData *akeIpData;
    DtcpAkeIpSessionData akeSession;
    EnCommands commandType;

    akeIpData = aUserData;

    returnValue = aMessagingInterface->Recv(aTransportHandle,
                                            messageBuffer,
                                            &messageSize,
                                            10000);

    // Determine type of message
    if (IS_SUCCESS(returnValue))
    {
        commandType = aMessagingInterface->GetCommandType(messageBuffer);

        // Process content key request command
        if (cmdContentKey == commandType)
        {
            returnValue = ProcessContentKeyRequestCommand(akeIpData, 
                                                          aMessagingInterface,
                                                          aTransportHandle,
                                                          messageBuffer, 
                                                          messageSize);
        }
        // Process status command
        else if (cmdStatus == commandType)
        {
            returnValue = ProcessStatusCommand(akeIpData,
                                               aMessagingInterface,
                                               aTransportHandle,
                                               messageBuffer,
                                               messageSize);
        }
        // Start AKE
        else if (cmdChallenge == commandType)
        {
#ifdef DEMO_MODE
            DEBUG_MSG(MSG_DEBUG, ("\r\n*************************************************************************\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    "**     Incoming Authentication & Key Exchange Request (DTCP-IP AKE)    **\r\n"));
            DEBUG_MSG(MSG_DEBUG, (    "*************************************************************************\r\n"));    
#endif
#if !defined(DEMO_MODE)
            DEBUG_MSG(MSG_DEBUG, ("Starting AKE!\n"));
#endif
            memcpy(&akeSession.MessagingInterface, aMessagingInterface, sizeof(DtcpAkeIpMessagingInterface));
            akeSession.TransportHandle = aTransportHandle;
            akeSession.AkeIpData = akeIpData;
            akeSession.CurrentAuthState = stateIdle;

            returnValue = DtcpAkeCore_OpenAkeSession(akeTypeIdFullAuth,
                                                     deviceModeSource,
                                                     &akeSession.AkeCoreSessionHandle);

            if (IS_SUCCESS(returnValue))
            {
                returnValue = ProcessCommand(&akeSession, messageBuffer, messageSize);
            }

            while (IS_SUCCESS(returnValue) && 
                (stateError         != akeSession.CurrentAuthState) &&
                (stateCancelled     != akeSession.CurrentAuthState) && 
                (stateCompleted     != akeSession.CurrentAuthState))
            {
                messageSize = 0;
                returnValue = aMessagingInterface->Recv(aTransportHandle,
                                                        messageBuffer,
                                                        &messageSize,
                                                        1000);
                if (IS_SUCCESS(returnValue))
                {
                    returnValue = ProcessCommand(&akeSession, messageBuffer, messageSize);
                }
                else
                {
                    if (RESPONSE_TIMEOUT == returnValue)
                    {
                        if (stateChallenge == akeSession.CurrentAuthState)
                        {
                            // Ake Time-out
                            if ((10.0 < OsWrap_GetElapsedTime(akeSession.ChallengeSentTime)) &&
                                (1.0  < OsWrap_GetElapsedTime(akeSession.ResponseSentTime)))
                            {
                                DEBUG_MSG(MSG_DEBUG, ("AKE Timeout!"));
                                returnValue = DTCP_AKE_TIMEOUT;
                            }
                        }
                        else if (stateAuthenticated == akeSession.ChallengeSentTime)
                        {
                            if (TRUE == akeSession.OurSrmUpdateRequiredFlag)
                            {
                                if ((9.0 < OsWrap_GetElapsedTime(akeSession.ResponseSentTime)) &&
                                    (1.0 < OsWrap_GetElapsedTime(akeSession.ExchangeKeySentTime)))
                                {
                                    // Timed out but there is no corrective action to take...
                                    // so return success and be done.
                                    DEBUG_MSG(MSG_DEBUG, ("Should have received an SRM message!  Timeout!"));
                                    akeSession.CurrentAuthState = stateCompleted;
                                    returnValue = SUCCESS;
                                }
                            }
                        }

                        if (DTCP_AKE_TIMEOUT != returnValue)
                        {
                            // Haven't really timed out
                            returnValue = SUCCESS;
                        }
                    }
                }
            }

            returnValue = DtcpAkeCore_CloseAkeSession(akeSession.AkeCoreSessionHandle);
        }
        // Invalid command sequence
        else
        {
            DEBUG_MSG(MSG_DEBUG, ("Invalid command sequence\n"));
            returnValue = DTCP_INVALID_COMMAND_SEQUENCE;
        }
    }
    if (IS_FAILURE(returnValue))
    {
#ifdef DEMO_MODE
        DEBUG_MSG(MSG_DEBUG, ("\r\n*****************************************************************\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    "**      Authentication & Key Exchange (DTCP-IP AKE) Failed     **\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    "*****************************************************************\r\n"));   
#endif
#if !defined(DEMO_MODE)
        DEBUG_MSG(MSG_DEBUG, ("Leaving AKE ListenCallback (AKE failed with error %d)\n", returnValue));
#endif
    }
    else
    {
        unsigned char exchangeKey[DTCP_EXCHANGE_KEY_SIZE];
        int exchangeKeyLabel;

        returnValue = DtcpAkeCore_GetSourceExchangeKey(exchKeyIdAes128,
            exchangeKey,
            &exchangeKeyLabel);
#ifdef DEMO_MODE
        DEBUG_MSG(MSG_DEBUG, ("\r\neeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    "ee    COMPUTED EXCHANGE KEY    ee\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\r\n"));
#else
        DEBUG_MSG(MSG_DEBUG, ("Exchange Key: "));
        DEBUG_BUF(MSG_DEBUG, exchangeKey, DTCP_EXCHANGE_KEY_SIZE);
#endif       
#ifdef DEMO_MODE
        DEBUG_MSG(MSG_DEBUG, ("\r\n*****************************************************************\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    "**     Authentication & Key Exchange (DTCP-IP AKE) Complete    **\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    "*****************************************************************\r\n"));   
#else
        DEBUG_MSG(MSG_DEBUG, ("AKE Completed!\r\n"));
#endif
    }

    return returnValue;
} // ListenCallback

int StartSink(DtcpAkeIpSessionData *aSinkSessionData)
{
    int returnValue = SUCCESS;
    unsigned char messageBuffer[MAX_MESSAGE_SIZE];
    unsigned int messageSize = MAX_MESSAGE_SIZE;

#ifdef DEMO_MODE
    DEBUG_MSG(MSG_DEBUG, ("\r\n************************************************************************\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    "**     Sending Authentication & Key Exchange Request (DTCP-IP AKE)    **\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    "************************************************************************\r\n"));
#endif
#if !defined(DEMO_MODE)
    DEBUG_MSG(MSG_DEBUG, ("Starting AKE!\n"));
#endif

    aSinkSessionData->CurrentAuthState = stateIdle;

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpAkeCore_OpenAkeSession(akeTypeIdFullAuth,
                                                 deviceModeSink,
                                                 &aSinkSessionData->AkeCoreSessionHandle);
    }

    if (IS_SUCCESS(returnValue))
    {
        returnValue = SendChallengeCommand(aSinkSessionData);
    }

    while (IS_SUCCESS(returnValue) && 
           (stateError         != aSinkSessionData->CurrentAuthState) &&
           (stateCancelled     != aSinkSessionData->CurrentAuthState) &&
           (stateCompleted     != aSinkSessionData->CurrentAuthState))
    {
        returnValue = aSinkSessionData->MessagingInterface.Recv(aSinkSessionData->TransportHandle,
                                                                messageBuffer,
                                                                &messageSize,
                                                                1000);

        if (IS_SUCCESS(returnValue))
        {
            returnValue = ProcessCommand(aSinkSessionData, messageBuffer, messageSize);
        }
        else
        {
            if (RESPONSE_TIMEOUT == returnValue)
            {
                if (stateIdle == aSinkSessionData->CurrentAuthState)
                {
#if !defined(DEMO_MODE)      
                    if (1.0 < OsWrap_GetElapsedTime(aSinkSessionData->ChallengeSentTime))
                    {
                        DEBUG_MSG(MSG_DEBUG, ("Timeout! - Did not receive Challenge command in time\n"));
                        returnValue = DTCP_AKE_TIMEOUT;
                    }
#endif
                }
                else if (stateChallenge == aSinkSessionData->CurrentAuthState)
                {
#if !defined(DEMO_MODE)
                    if (10.0 < OsWrap_GetElapsedTime(aSinkSessionData->ChallengeSentTime))
                    {
                        DEBUG_MSG(MSG_DEBUG, ("Timeout! - Did not receive Response command in time\n"));
                        DEBUG_MSG(MSG_DEBUG, ("Timeout! - Window: 10.0 seconds, Actual: %f\n", OsWrap_GetElapsedTime(aSinkSessionData->ChallengeSentTime)));
                        returnValue = DTCP_AKE_TIMEOUT;
                    }
#endif
                }
                else if (stateResponse == aSinkSessionData->CurrentAuthState)
                {
#if !defined(DEMO_MODE)
                    if (9.0 < OsWrap_GetElapsedTime(aSinkSessionData->ResponseSentTime))
                    {
                        DEBUG_MSG(MSG_DEBUG, ("Timeout! - Did not receive Exchange Key command in time\n"));
                        returnValue = DTCP_AKE_TIMEOUT;
                    }
#endif
                }
                else if (stateAuthenticated == aSinkSessionData->ChallengeSentTime)
                {
#if !defined(DEMO_MODE)                    
                    if (1.0 < OsWrap_GetElapsedTime(aSinkSessionData->ExchangeKeyReceivedTime))
                    {
                        DEBUG_MSG(MSG_DEBUG, ("Timeout! - Did not receive Srm command in time\n"));
                        returnValue = DTCP_AKE_TIMEOUT;
                    }
#endif
                }
                
                if (DTCP_AKE_TIMEOUT != returnValue)
                {
                    // Didn't time out
                    returnValue = SUCCESS;
                }
            }
        }
    }

    //returnValue = SendContentKeyRequestCommand(aSinkSessionData);

    if ((stateError         == aSinkSessionData->CurrentAuthState) ||
        (stateCancelled     == aSinkSessionData->CurrentAuthState) ||
        IS_FAILURE(returnValue))
    {
        // Close AkeCore session
        DtcpAkeCore_CloseAkeSession(aSinkSessionData->AkeCoreSessionHandle);
    
        if (stateError == aSinkSessionData->CurrentAuthState)
        {
            returnValue = FAILURE;               
        }
        else if (stateCancelled == aSinkSessionData->CurrentAuthState)
        {
            returnValue = AKE_CANCELED;
        }
    } else {

        unsigned char exchangeKey[DTCP_EXCHANGE_KEY_SIZE];
        unsigned int exchKeyLabel = 0;

        returnValue = DtcpAkeCore_GetSinkExchangeKey(aSinkSessionData->AkeCoreSessionHandle, 
                                                     exchKeyIdAes128,
                                                     exchangeKey,
                                                     &exchKeyLabel);

#ifdef DEMO_MODE
        DEBUG_MSG(MSG_DEBUG, ("\r\neeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    "ee    COMPUTED EXCHANGE KEY    ee\r\n"));
        DEBUG_MSG(MSG_DEBUG, (    "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\r\n"));
#endif
#if !defined(DEMO_MODE)
        DEBUG_MSG(MSG_DEBUG, ("Exchange Key: "));
        DEBUG_BUF(MSG_DEBUG, exchangeKey, DTCP_EXCHANGE_KEY_SIZE);
#endif
    }
#ifdef DEMO_MODE
    DEBUG_MSG(MSG_DEBUG, ("\r\n*****************************************************************\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    "**     Authentication & Key Exchange (DTCP-IP AKE) Complete    **\r\n"));
    DEBUG_MSG(MSG_DEBUG, (    "*****************************************************************\r\n"));   
#endif
#if !defined(DEMO_MODE)
    DEBUG_MSG(MSG_DEBUG, ("AKE Completed!\r\n"));
#endif

    return returnValue;
} // StartSink

int GetSourceExchangeKeyLabel(DtcpAkeIpSessionData *aAkeSessionData, int *aExchangeKeyLabel)
{
    int returnValue = SUCCESS;

    returnValue = SendContentKeyRequestCommand(aAkeSessionData, aExchangeKeyLabel);

    return returnValue;
} // GetSourceExchangeKeyLabel

int CheckSourceSinkLimit(DtcpAkeIpData *aAkeIpData, DtcpTransportHandle aTransportHandle, DtcpAkeIpMessagingInterface aMessagingInterface)
{
    int returnValue = SUCCESS;

    returnValue = SendStatusCommand(aAkeIpData, aTransportHandle, aMessagingInterface);

    return returnValue;
}
