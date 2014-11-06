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

/// \file
/// Implementation file for DTCP Ake Transport API for IP.
///
/// This file implements the interface to the DTCP Ake Transport API for IP
/// based networks.  The following pictures provide sequence diagrams and state
/// charts to help explain the functionality of this component.
/// \image html DtcpIpTrans_ListenerSequenceDiagram.gif
/// \image html DtcpIpTrans_InitiatorSequenceDiagram.gif
/// \image html DtcpIpTrans_MsgRecvSequenceDiagram.gif
/// \image html DtcpIpTrans_MsgSendSequenceDiagram.gif
/// \image html DtcpIpTrans_MsgRecvStateChart.gif
/// \image html DtcpIpTrans_MsgSendStateChart.gif


#include "DtcpAkeTransport.h"

#include "DtcpStatusCodes.h"

#include "BasicP2P.h"
#include "InetWrapper.h"
#include "OsWrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

///////////////////////////////////////////////////////////////
////// Macros and typedefs
///////////////////////////////////////////////////////////////

/// \brief defines a non-zero value for "true"
#define TRUE   1

/// \brief defines a zero value for "false"
#define FALSE  0

/// \brief ake message header size
#define AKE_MESSAGE_HEADER_SIZE               11

/// \brief message header control size
#define AKE_HEADER_CONTROL_SIZE                8

/// \brief index into header to where message type resides
#define MSG_TYPE_INDEX                         0

/// \brief index into header for lower byte of length field
#define MSG_LENGTH_LOWER_INDEX                 1

/// \brief index into header for upper byte of length field
#define MSG_LENGTH_UPPER_INDEX                 2

/// \brief index into header where the ctype field resides
#define MSG_CTYPE_INDEX                        3

/// \brief index into header where category and ake id fields reside
#define MSG_CATEGORY_AND_AKEID_INDEX           4

/// \brief index into header where sub function field resides
#define MSG_SUBFUNCTION_INDEX                  5

/// \brief index into header where ake procedure field resides
#define MSG_AKE_PROCEDURE_INDEX                6

/// \brief index into header where the exchange key field resides
#define MSG_EXCHANGE_KEY_INDEX                 7

/// \brief index into header where sub function dependant field resides
#define MSG_SUBFUNCTION_DEPENDENT_INDEX        8

/// \brief index into header where ake label field resides
#define MSG_AKE_LABEL_INDEX                    9

/// \brief index into header where the status field resides
#define MSG_STATUS_INDEX                      10


/// \brief worst case command timeout value in milliseconds
#define WORST_CASE_COMMAND_TIMEOUT         30000

/// \brief enum used to identify if a device is a source or a sink
typedef enum
{
    DeviceTypeSource,    ///< device is a source
    DeviceTypeSink       ///< device is a sink
} DeviceTypes;

/// \brief enum used to track the current state of a bus message in the internal read buffers
typedef enum
{
    Empty,          ///< internal buffers empty, no message received
    Partial,        ///< started receiving a message but don't have it all yet
    Complete,       ///< message has been completely received and can be processed
} BusMessageStates;

/// \brief enum used to track the state machine for commands sent and received
typedef enum
{
    NoActivity,                  ///< no outstanding command sent or received
    Canceled,                    ///< current command has been canceled
    CommandReceived,             ///< command has been received and needs to be processed
    WaitingForCommandResponse,   ///< sent a command and waiting for other device to respond
    CommandResponseReceived,     ///< other device has sent a response
    NeedToSendCommandResponse,   ///< received a command and need to respond
} CommandStates;

/// \brief maintain state information for each currently active ake session 
typedef struct __ake_session_data__
{
    DeviceTypes               Type;                  ///< indicates if this device is a sink or a source for this ake
    BasicP2PConnection      * Connection;            ///< state information for the basicp2p connection information
    void                    * CommandSemaphore;      ///< semaphore used to regulate multi-threaded access to command state info
    CommandStates             CommandState;          ///< current command state 
    int                       CancelAke;             ///< boolean indicating ake is in the process of being canceled
    int                       CommunicationStatus;   ///< doesn't appear to be used at this time, could probably be deleted.
    unsigned char           * MessageData;           ///< data buffer containing current message
    int                       MessageLength;         ///< length of current message
} AkeSessionData;

// IpDeviceId format is "xxx.xxx.xxx.xxx:yyyyy"
// where the first 4 xxx's are the dotted decimal notation of the IP address and
// yyyyy is the port number
// Need 12 bytes for the 4 xxx's, 
// Need 3 bytes for the 3 .'s
// Need 1 byte for the :
// Need 5 bytes for the port number
// Need 1 byte for the NULL terminator
// 12 + 3 + 1 + 5 + 1 = 22 bytes
// Allocate a nice even number of 32 for the actual memory anyway

/// \brief state information for a listen session
typedef struct __listen_session_data__
{
    BasicP2PConnection      * Connection;       ///< state information for the basicp2p connection information
    DtcpAke_SourceAke_Ptr     SourceAkeFunc;    ///< function provided by caller to process commands as a source device
    LinkedList              * AkeSessions;      ///< list of current active ake sessions
    char                      IpDeviceId[32];   ///< buffer to store our IP address as provided by caller
    void                    * UserData;         ///< location to store data that needs to be saved by caller
} ListenSessionData;

/// \brief structure for maintain access to both types of sessions for source devices
///
/// The UserData field of the BasicP2P connection structure
/// is used to store a copy of this structure so both type
/// of sessions can be accessed.  For client connections, only
/// the AkeSession field is used.  Both fields are used for
/// source connections.
typedef struct __session_handles_data__
{
    ListenSessionData          * ListenSession;     ///< session data for listening
    AkeSessionData             * AkeSession;        ///< client side session data
} SessionHandlesData;

/// \brief structure for maintaining information about the current bus message
typedef struct __bus_message_data__
{
    BusMessageStates      State;           ///< current state of message, empty partial or complete
    unsigned char       * MessageData;     ///< pointer to message data
    int                   MessageLength;   ///< length of message
} BusMessageData;



///////////////////////////////////////////////////////////////
////// Debug Message Variables
///////////////////////////////////////////////////////////////

#if 0
static int GlobalDisplayLevel             = -1;
static int GlobalDisplayLevelMatchExactly = 0;
static int GlobalLogToFile                = 0;
#endif

///////////////////////////////////////////////////////////////
////// Global Variables
///////////////////////////////////////////////////////////////

static int                  DtcpAkeTransportInitialized = FALSE;
static LinkedList         * ListenSessions              = NULL;
static LinkedList         * InitiateSessions            = NULL;



///////////////////////////////////////////////////////////////
////// Internal function prototypes
///////////////////////////////////////////////////////////////
static int
DtcpAke_GetSubFuncDep(
    void * aMessageData);

static int
DtcpAke_SetSubFuncDep(
    void * aMessageData,
    int    aSubFuncDep);
    
static int
DtcpAke_SetAkeLabel(
    void * aMessageData,
    int    aAkeLabel);


///////////////////////////////////////////////////////////////
////// Helper function
///////////////////////////////////////////////////////////////
/// \brief Display the contents of a message to STDOUT
static void
DisplaySentMessage(
    unsigned char * aMessage,
    int             aAkeInfoLength)
{
    int index;

    DEBUG_MSG(MSG_DEBUG, ("\t0x%02x\t0x%02x%02x\t0x",
                     aMessage[0], aMessage[1], aMessage[2]));

    for (index = 0; index < 8; index++)
    {
        DEBUG_MSG(MSG_DEBUG, ("%02x ",
                     aMessage[index + 3]));
    }

    DEBUG_MSG(MSG_DEBUG, ("\n"));

/*      if (aAkeInfoLength) */
/*      { */
/*          DEBUG_MSG(MSG_DEBUG, ("\t0x")); */

/*          for (index = 0; index < aAkeInfoLength; index++) */
/*          { */
/*              DEBUG_MSG(MSG_DEBUG, ("0x%02x ", */
/*                               aMessage[index + 11])); */
/*          } */

/*          DEBUG_MSG(MSG_DEBUG, ("\n")); */
/*      } */

}


///////////////////////////////////////////////////////////////
////// Helper function
///////////////////////////////////////////////////////////////
/// \brief Helper function to wait for a particular state
///
/// The semaphore must be owned when calling this function.  If
/// not, results are unpredictable.  The semaphore will still be
/// owned by the calling thread when this function returns,
/// even if it returns an error.
static int
WaitForState(
    AkeSessionData     * aSession,
    CommandStates        aDesiredState,
    int                  aTimeOut)
{
    int returnValue = FAILURE;

    int startTime            = OsWrap_GetCurrentTime();
    AkeSessionData * session = aSession;

    while (1)
    {
        if (session->CancelAke)
        {
            returnValue = AKE_CANCELED;
            break;
        }

        if (aSession->CommandState == aDesiredState)
        {
            returnValue = SUCCESS;
            break;
        }

        OsWrap_SemaphorePost(session->CommandSemaphore);

        OsWrap_Sleep(1);

        OsWrap_SemaphoreWait(session->CommandSemaphore);

        if (aTimeOut)
        {
            if (((float) aTimeOut) / 1000 < OsWrap_GetElapsedTime(startTime))
            {
                returnValue = RESPONSE_TIMEOUT;
                break;
            }
        }
    }

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Helper function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, return the message length
static int
GetMessageLength(unsigned char * aMessageData)
{
    int returnValue = 0;

    returnValue = (aMessageData[MSG_LENGTH_LOWER_INDEX] << 8) | aMessageData[MSG_LENGTH_UPPER_INDEX];

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Helper function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, set the message length
static int
SetMessageLength(unsigned char * aMessageData, int aMessageLength)
{
    int returnValue = SUCCESS;

    aMessageData[MSG_LENGTH_LOWER_INDEX] = (aMessageLength >> 8) & 0xff;
    aMessageData[MSG_LENGTH_UPPER_INDEX] = aMessageLength & 0xff;

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Helper function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, return boolean indicating if this is a command or a response
static int
IsNewCommand(unsigned char * aMessageData)
{
    int returnValue = ((aMessageData[MSG_CTYPE_INDEX] & 0x0f) < 8);

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, return the command type
static EnCommands
DtcpAke_GetCommandType(
    void * aMessageData)
{
    EnCommands returnValue;

    unsigned char * messageData = (unsigned char *) aMessageData;

    //KLUDGE.  It turns out that the only way to be certain that a
    //         response frame is a status command or a control command
    //         is to look at the subfunction_dependant field.  The spec
    //         clearly says that this field must be 0xff for both command
    //         and response frames for status commands.  See section 8.3.3
    if (0xff == DtcpAke_GetSubFuncDep(messageData))
    {
        returnValue = cmdStatus;
    }
    else
    {
        switch (messageData[MSG_SUBFUNCTION_INDEX])
        {
        case 1:
            returnValue = cmdChallenge;
            break;
        case 2:
            returnValue = cmdResponse;
            break;
        case 3:
            returnValue = cmdExchangeKey;
            break;
        case 4:
            returnValue = cmdSRM;
            break;
        case 0x80:
            returnValue = cmdContentKey;
            break;
        case 0xc0:
            returnValue = cmdCancel;
            break;
        default:
            returnValue = cmdInvalid;
            break;
        }
    }
    
    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, set the command type
static int
DtcpAke_SetCommandType(
    void *     aMessageData,
    EnCommands aCommandType)
{
    int returnValue = SUCCESS;
    
    unsigned char * messageData = (unsigned char *) aMessageData;

    // Make sure the type, category and AKE_ID fields are set correctly.
    // If they ever evolve to be something other than default values
    // we will have to expose a set of functions to allow the caller to 
    // set them but for now, this is simpler.
    messageData[MSG_TYPE_INDEX]               = 0x01;
    messageData[MSG_CATEGORY_AND_AKEID_INDEX] = 0;

    switch (aCommandType)
    {
    case cmdStatus:
        messageData[MSG_SUBFUNCTION_INDEX] = 0xff;
        DtcpAke_SetSubFuncDep(messageData,   0xff);
        DtcpAke_SetAkeLabel  (messageData,   0xff);
        break;
    case cmdChallenge:
        messageData[MSG_SUBFUNCTION_INDEX] = 1;
        break;
    case cmdResponse:
        messageData[MSG_SUBFUNCTION_INDEX] = 2;
        break;
    case cmdExchangeKey:
        messageData[MSG_SUBFUNCTION_INDEX] = 3;
        break;
    case cmdSRM:
        messageData[MSG_SUBFUNCTION_INDEX] = 4;
        break;
    case cmdContentKey:
        messageData[MSG_SUBFUNCTION_INDEX] = 0x80;
        break;
    case cmdCancel:
        messageData[MSG_SUBFUNCTION_INDEX] = 0xc0;
        break;
    default:
        returnValue = FAILURE;
        break;
    }
    
    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, return the ctype response
static int
DtcpAke_GetCTypeResponse(
    void * aMessageData)
{
    int returnValue;
    
    unsigned char * messageData = (unsigned char *) aMessageData;

    returnValue = messageData[MSG_CTYPE_INDEX] & 0x0f;
        
    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, set the ctype response
static int
DtcpAke_SetCTypeResponse(
    void * aMessageData,
    int    aCTypeResponse)
{
    int returnValue = SUCCESS;
    
    unsigned char * messageData = (unsigned char *) aMessageData;
    
    // Make sure the reserved bits are all zero
    messageData[MSG_CTYPE_INDEX] &= 0x8f;

    messageData[MSG_CTYPE_INDEX] &= 0xf0;
    messageData[MSG_CTYPE_INDEX] |= (((unsigned char) aCTypeResponse) & 0x0f);

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, return the ake procedures
static int
DtcpAke_GetAkeProcedures(
    void * aMessageData)
{
    int returnValue;
    
    unsigned char * messageData = (unsigned char *) aMessageData;

    returnValue = messageData[MSG_AKE_PROCEDURE_INDEX];
        
    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, set the ake procedures
static int
DtcpAke_SetAkeProcedures(
    void * aMessageData,
    int    aAkeProcedures)
{
    int returnValue = SUCCESS;
    
    unsigned char * messageData = (unsigned char *) aMessageData;
    
    messageData[MSG_AKE_PROCEDURE_INDEX] = (unsigned char) aAkeProcedures;

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, return the exchange keys field
static int
DtcpAke_GetExchangeKeys(
    void * aMessageData)
{
    int returnValue;
    
    unsigned char * messageData = (unsigned char *) aMessageData;

    returnValue = messageData[MSG_EXCHANGE_KEY_INDEX];
        
    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, set the exchange keys field
static int
DtcpAke_SetExchangeKeys(
    void * aMessageData,
    int    aExchangeKeys)
{
    int returnValue = SUCCESS;
    
    unsigned char * messageData = (unsigned char *) aMessageData;
    
    messageData[MSG_EXCHANGE_KEY_INDEX] = (unsigned char) aExchangeKeys;

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, return the subfunction dependant field
static int
DtcpAke_GetSubFuncDep(
    void * aMessageData)
{
    int returnValue;

    unsigned char * messageData = (unsigned char *) aMessageData;

    returnValue = messageData[MSG_SUBFUNCTION_DEPENDENT_INDEX];
        
    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, set the subfunction dependant field
static int
DtcpAke_SetSubFuncDep(
    void * aMessageData,
    int    aSubFuncDep)
{
    int returnValue = SUCCESS;

    unsigned char * messageData = (unsigned char *) aMessageData;
    
    messageData[MSG_SUBFUNCTION_DEPENDENT_INDEX] = (unsigned char) aSubFuncDep;

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, return the ake label
static int
DtcpAke_GetAkeLabel(
    void * aMessageData)
{
    int returnValue;

    unsigned char * messageData = (unsigned char *) aMessageData;

    returnValue = messageData[MSG_AKE_LABEL_INDEX];
        
    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, set the ake label
static int
DtcpAke_SetAkeLabel(
    void * aMessageData,
    int    aAkeLabel)
{
    int returnValue = SUCCESS;

    unsigned char * messageData = (unsigned char *) aMessageData;
    
    messageData[MSG_AKE_LABEL_INDEX] = (unsigned char) aAkeLabel;

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, return the message status
static int
DtcpAke_GetStatus(
    void * aMessageData)
{
    int returnValue;
    
    unsigned char * messageData = (unsigned char *) aMessageData;

    returnValue = messageData[MSG_STATUS_INDEX] & 0x0f;
        
    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, set the message status
static int
DtcpAke_SetStatus(
    void * aMessageData,
    int    aStatus)
{
    int returnValue = SUCCESS;
    
    unsigned char * messageData = (unsigned char *) aMessageData;
    
    // Make sure the reserved bits are all zero
    messageData[MSG_STATUS_INDEX] &= 0x0f;

    messageData[MSG_STATUS_INDEX] &= 0xf0;
    messageData[MSG_STATUS_INDEX] |= (unsigned char) (aStatus & 0x0f);

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, return the command data
static void *
DtcpAke_GetCommandData(
    void * aMessageData,
    int  * aDataLength)
{
    void * returnValue;
    
    unsigned char * messageData = (unsigned char *) aMessageData;

    *aDataLength = GetMessageLength(aMessageData) - AKE_HEADER_CONTROL_SIZE;
    
    returnValue = (void *) &messageData[AKE_MESSAGE_HEADER_SIZE];

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Given a complete message buffer, set the command data
static int
DtcpAke_SetCommandData(
    void * aMessageData,
    void * aData,
    int    aDataLength)
{
    int returnValue = SUCCESS;
    
    unsigned char * messageData = (unsigned char *) aMessageData;
    unsigned char * commandData = (unsigned char *) aData;
    
    SetMessageLength(aMessageData, aDataLength + AKE_HEADER_CONTROL_SIZE);
    
    if (NULL != messageData)
    {
        memcpy (&messageData[AKE_MESSAGE_HEADER_SIZE], commandData, aDataLength);
    }

    return (returnValue);
}


///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Send a command or a response to the other device
static int 
DtcpAke_Send(
    DtcpTransportHandle    aTransportHandle,
    void                 * aMessageData,
    int                  * aMessageLength,
    int                    aMessageResponseTimeout)
{
    int returnValue = FAILURE;

    AkeSessionData * session = (AkeSessionData *) aTransportHandle;

    BasicP2PConnection   * connection = session->Connection;

    int newCommandMessage = IsNewCommand(aMessageData);
    
    DEBUG_MSG(MSG_DEBUG, ("DtcpAke_Send CommandState=%d\n", session->CommandState));

    // make sure we own the semaphore before accessing multi-threaded data fields, mainly the CommandState
    OsWrap_SemaphoreWait(session->CommandSemaphore);

    if (newCommandMessage                    &&
        NoActivity == session->CommandState)
    {
        DisplaySentMessage((unsigned char *) aMessageData, 
                           *aMessageLength - AKE_MESSAGE_HEADER_SIZE);

        // send new command to other device
        returnValue = InetWrap_TCPSendData(connection->ConnectionSocket,
                                           (unsigned char *) aMessageData,
                                           *aMessageLength);

        if (IS_SUCCESS(returnValue))
        {
            session->CommandState = WaitingForCommandResponse;

            // OK, now wait for the other device to send the response
            returnValue = WaitForState(session, 
                                       CommandResponseReceived,
                                       aMessageResponseTimeout);

            if (IS_SUCCESS(returnValue))
            {
                if (session->CancelAke)
                {
                    returnValue = AKE_CANCELED;

                    session->CommandState = Canceled;
                }
                else
                {
                    memcpy((unsigned char *) aMessageData, 
                           session->MessageData,
                           session->MessageLength);

                    *aMessageLength = session->MessageLength;

                    // The memory for the new message was allocated
                    // in the GetBusMessage routine and can be
                    // released here.  GetBusMessage will allocate
                    // a new buffer for each new message.
                    free(session->MessageData);
                    session->MessageData = NULL;
                    
                    session->CommandState = NoActivity;
                }
            }
        }
    }
    else if (!newCommandMessage                                 &&
             NeedToSendCommandResponse == session->CommandState)
    {
        // send command response to other device
        returnValue = InetWrap_TCPSendData(connection->ConnectionSocket,
                                           (unsigned char *) aMessageData,
                                           *aMessageLength);
                                         
        session->CommandState = NoActivity;
    }
    else
    {
        returnValue = FAILURE;
        ERROR_MSG(1, returnValue, ("Sending message of type %d with invalid CommandState %d\n", 
                                      newCommandMessage,
                                      session->CommandState));
    }

    // relinquish the semaphore
    OsWrap_SemaphorePost(session->CommandSemaphore);

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Receive a command or a response to the other device
static int 
DtcpAke_Recv(
    DtcpTransportHandle    aTransportHandle,
    void                 * aMessageData,
    int                  * aMessageLength,
    int                    aMessageTimeout)
{
    int returnValue = FAILURE;

    AkeSessionData * session = (AkeSessionData *) aTransportHandle;

    // make sure we own the semaphore before accessing multi-threaded data fields, mainly the CommandState
    OsWrap_SemaphoreWait(session->CommandSemaphore);

    returnValue = WaitForState(session, 
                               CommandReceived,
                               aMessageTimeout);

    if (IS_SUCCESS(returnValue))
    {
        if (session->CancelAke)
        {
            returnValue = AKE_CANCELED;

            session->CommandState = Canceled;
        }
        else
        {
            memcpy((unsigned char *) aMessageData, 
                    session->MessageData,
                    session->MessageLength);

            *aMessageLength = session->MessageLength;

            // The memory for the new message was allocated
            // in the GetBusMessage routine and can be
            // released here.  GetBusMessage will allocate
            // a new buffer for each new message.
            free(session->MessageData);
            session->MessageData = NULL;

            session->CommandState = NeedToSendCommandResponse;

            returnValue = SUCCESS;
        }
    }

    // relinquish the semaphore
    OsWrap_SemaphorePost(session->CommandSemaphore);

    return (returnValue);
}

/// \brief fill in interface structure for calling library
static void
GetMessagingInterface(
    DtcpAkeIpMessagingInterface * aMessagingInterface)
{
    aMessagingInterface->Send             = &DtcpAke_Send;
    aMessagingInterface->Recv             = &DtcpAke_Recv;
    aMessagingInterface->GetCommandType   = &DtcpAke_GetCommandType;
    aMessagingInterface->SetCommandType   = &DtcpAke_SetCommandType;
    aMessagingInterface->GetCTypeResponse = &DtcpAke_GetCTypeResponse;
    aMessagingInterface->SetCTypeResponse = &DtcpAke_SetCTypeResponse;
    aMessagingInterface->GetAkeProcedures = &DtcpAke_GetAkeProcedures;
    aMessagingInterface->SetAkeProcedures = &DtcpAke_SetAkeProcedures;
    aMessagingInterface->GetExchangeKeys  = &DtcpAke_GetExchangeKeys;
    aMessagingInterface->SetExchangeKeys  = &DtcpAke_SetExchangeKeys;
    aMessagingInterface->GetSubFuncDep    = &DtcpAke_GetSubFuncDep;
    aMessagingInterface->SetSubFuncDep    = &DtcpAke_SetSubFuncDep;
    aMessagingInterface->GetAkeLabel      = &DtcpAke_GetAkeLabel;
    aMessagingInterface->SetAkeLabel      = &DtcpAke_SetAkeLabel;
    aMessagingInterface->GetStatus        = &DtcpAke_GetStatus;
    aMessagingInterface->SetStatus        = &DtcpAke_SetStatus;
    aMessagingInterface->GetCommandData   = &DtcpAke_GetCommandData;
    aMessagingInterface->SetCommandData   = &DtcpAke_SetCommandData;
}


///////////////////////////////////////////////////////////////
////// Helper function
///////////////////////////////////////////////////////////////
/// \brief process a received message and set command state appropriately
static int 
ProcessBusMessage(
    BasicP2PConnection   * aConnection,
    BusMessageData       * aBusMessage)
{
    int returnValue = SUCCESS;

    BasicP2PConnection * connection     = aConnection;

    SessionHandlesData * sessionHandles = (SessionHandlesData *) connection->UserData;
    AkeSessionData     * session        = sessionHandles->AkeSession;

    // make sure we own the semaphore before accessing multi-threaded data fields, mainly the CommandState
    OsWrap_SemaphoreWait(session->CommandSemaphore);

    if (IsNewCommand(aBusMessage->MessageData))
    {
        // make sure we in an idle mode before signaling other thread that a new message has arrived
        returnValue = WaitForState(session, 
                                   NoActivity,
                                   WORST_CASE_COMMAND_TIMEOUT);

        if (IS_SUCCESS(returnValue))
        {
            session->MessageData   = aBusMessage->MessageData;
            session->MessageLength = aBusMessage->MessageLength;
            session->CommandState  = CommandReceived;
        }
        else
        {
            ERROR_MSG(1, returnValue, ("Failed to get bus state back to 'no activity' to process new message\n"));

            session->CancelAke = TRUE;
        }
    }
    else
    {
        if (WaitingForCommandResponse == session->CommandState)
        {
            session->MessageData   = aBusMessage->MessageData;
            session->MessageLength = aBusMessage->MessageLength;
            session->CommandState  = CommandResponseReceived;
        }
        else
        {
            ERROR_MSG(1, returnValue, ("Bad Command State %d.  Received msg ack when not expecting one\n",
                                          session->CommandState));

            session->CancelAke = TRUE;
        }
    }

    // relinquish the semaphore
    OsWrap_SemaphorePost(session->CommandSemaphore);

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Helper function
///////////////////////////////////////////////////////////////
/// \brief Retrieve a new message from the TCP read buffer
static int 
GetBusMessage(
    BasicP2PConnection    * aConnection,
    BusMessageData        * aBusMessage)
{
    int returnValue = SUCCESS;

    BasicP2PConnection   * connection = aConnection;

    int bytesAvailable;

    // with tcp, there is no certainty that a complete message will arrive in the internal receive
    // buffers.  This block of code will make sure there is enough data in the receive buffer to
    // get the dtcp message length field and then make sure the entire message is received before
    // processing the message.

    returnValue = InetWrap_TCPReadBufferSize(connection->ConnectionSocket, 
                                             &bytesAvailable);

    if (IS_SUCCESS(returnValue) && bytesAvailable)
    {
        if (AKE_MESSAGE_HEADER_SIZE <= bytesAvailable     && 
            Empty                   == aBusMessage->State)
        {
            char header[AKE_MESSAGE_HEADER_SIZE];

            int readSize = AKE_MESSAGE_HEADER_SIZE;

            // every dtcp message has a fixed length header that contains a length
            // field so read the header and then we can know how long the total message is.

            returnValue = InetWrap_TCPRecvData(connection->ConnectionSocket, 
                                               header,
                                               &readSize);

            if (IS_SUCCESS(returnValue))
            {
                bytesAvailable -= readSize;

                aBusMessage->MessageLength = GetMessageLength((unsigned char *) header) -
                                             AKE_HEADER_CONTROL_SIZE + 
                                             AKE_MESSAGE_HEADER_SIZE;

                aBusMessage->MessageData = malloc(aBusMessage->MessageLength);

                if (aBusMessage->MessageData)
                {
                    memcpy(aBusMessage->MessageData, header, AKE_MESSAGE_HEADER_SIZE);
                
                    aBusMessage->State = (aBusMessage->MessageLength == AKE_MESSAGE_HEADER_SIZE)
                                         ? Complete 
                                         : Partial;
                }
                else
                {
                    returnValue = FAILURE;
                }
            }
        }

        if (IS_SUCCESS(returnValue)                      &&
            Partial        == aBusMessage->State         &&
            bytesAvailable >= aBusMessage->MessageLength - AKE_MESSAGE_HEADER_SIZE)
        {
            int readSize;

            // we now have the rest of the message in the internal receive buffer so go get it.

            readSize = aBusMessage->MessageLength - AKE_MESSAGE_HEADER_SIZE;

            returnValue = InetWrap_TCPRecvData(connection->ConnectionSocket, 
                                               &aBusMessage->MessageData[AKE_MESSAGE_HEADER_SIZE],
                                               &readSize);

            if (IS_SUCCESS(returnValue))
            {
                aBusMessage->State = Complete;
            }
        }
    }

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Helper function
///////////////////////////////////////////////////////////////
/// \brief Thread function for asynchronously receiving messages from remote device
///
/// Loops waiting for new messages and posts them to DTCP_AKE_Recv.
/// Does not call Berkley sockets recv function until an entire message is in the buffer.
/// Will terminate if an error occurs or signaled to do so by another thread.
/// Upon termination, sets an event to signal other threads that this thread is finished.
static int 
ReaderThread(
    BasicP2PConnection    * aConnection)
{
    int returnValue = SUCCESS;

    BasicP2PConnection   * connection = aConnection;
    BusMessageData         busMessage;

    SessionHandlesData * sessionHandles = NULL;
    AkeSessionData     * session        = NULL;

    // If we are a server, we need to give the ConnectionThread
    // a chance to create a session for us before processing any messages.

    // Don't make a copy of UserData until the AkeSession value is filled in
    // by ConnectionThread.  That function malloc's new memory and replaces
    // UserData with a pointer to the new memory and fills in the AkeSession value.
    while (1)
    {
        sessionHandles = (SessionHandlesData *) connection->UserData;

        if (connection->TerminateReaderThread)
        {
            break;
        }

        if (sessionHandles)
        {
            if (sessionHandles->AkeSession)
            {
                session = sessionHandles->AkeSession;
                break;
            }
        }

        OsWrap_Sleep(1);
    }

    busMessage.State = Empty;

    while (IS_SUCCESS(returnValue) && !connection->TerminateReaderThread)
    {
        // This function may get called several times before a complete message is received
        returnValue = GetBusMessage(connection, &busMessage);

        if (IS_SUCCESS(returnValue))
        {
            if (Complete == busMessage.State)
            {
                // we got a complete message, validate it and notify other thread that we have received a new message
                returnValue = ProcessBusMessage(connection, &busMessage);

                busMessage.State = Empty;

                if (IS_FAILURE(returnValue))
                {
                    ERROR_MSG(1, returnValue, ("Error while processing DTCP Command!!\n"));
                }
            }

            OsWrap_Sleep(1);
        }
        else
        {
            ERROR_MSG(1, returnValue, ("Error while reading new bus message!!\n"));
        }
    }

    DEBUG_MSG(MSG_DEBUG+2, ("Done with reader thread while loop\n"));

    if (IS_FAILURE(returnValue))
    {
        if (session)
        {
            OsWrap_SemaphoreWait(session->CommandSemaphore);
            session->CancelAke = TRUE;
            OsWrap_SemaphorePost(session->CommandSemaphore);
        }
    }

    connection->ReaderThreadReturnValue = returnValue;

    OsWrap_SetEvent(connection->ReaderThreadTerminated);

    return 0;
}

///////////////////////////////////////////////////////////////
////// Helper function
///////////////////////////////////////////////////////////////
/// \brief Signals to other threads that an Ake is to be canceled.
/// 
/// Will block until other threads have successfully processed
/// the cancel notification.  When this function returns, resources
/// can be released for this Ake session.
static int
CancelAkeSession(
    AkeSessionData * aSession)
{
    int returnValue = SUCCESS;

    AkeSessionData * session  = aSession;

    OsWrap_SemaphoreWait(session->CommandSemaphore);

    session->CancelAke = TRUE;

    if (!(NoActivity == session->CommandState ||
          Canceled   == session->CommandState))
    {
        // Wait for any current communication to be processed

        returnValue = WaitForState(session, 
                                   Canceled,
                                   WORST_CASE_COMMAND_TIMEOUT);
    }

    OsWrap_SemaphorePost(session->CommandSemaphore);

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Helper function
///////////////////////////////////////////////////////////////
/// \brief Releases resources for an Ake session
static int
CloseAkeSession(
    LinkedList     ** aList,
    AkeSessionData  * aSession)
{
    int returnValue = SUCCESS;

    AkeSessionData * session  = aSession;

    if (session->Connection)
    {
        // Save a copy of this malloc'ed pointer before
        // it is released in the CloseConnection call.
        void * userData = session->Connection->UserData;

        BasicP2P_CloseConnection(&session->Connection);

        if (userData)
        {
            free(userData);
        }
    }

    if (session->CommandSemaphore)
    {
        OsWrap_SemaphoreClose(session->CommandSemaphore);
    }

    OsWrap_RemoveFromList(aList, session);

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Helper function
///////////////////////////////////////////////////////////////
/// \brief Allocates resources needed for an Ake session
static int
OpenAkeSession(
    LinkedList     ** aList,
    AkeSessionData ** aSession)
{
    int returnValue = FAILURE;

    AkeSessionData * session;

    *aSession = NULL;

    returnValue = OsWrap_AddToList(aList,
                                   &session,
                                   sizeof(*session));

    if (IS_SUCCESS(returnValue))
    {
        returnValue = OsWrap_SemaphoreInit(&session->CommandSemaphore, 1);

        if (IS_SUCCESS(returnValue))
        {
            session->Connection             = NULL;
            session->CommandState           = NoActivity;
            session->CommunicationStatus    = SUCCESS;
            session->CancelAke              = FALSE;

            *aSession = session;
        }
        else
        {
            OsWrap_RemoveFromList(aList, session);

            returnValue = FAILURE;
        }
    }

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Helper function
///////////////////////////////////////////////////////////////
/// \brief Releases resources for an Listen session
static int
CloseListenSession(
    ListenSessionData * aSession)
{
    int returnValue = SUCCESS;

    ListenSessionData * session  = aSession;

    OsWrap_RemoveFromList(&ListenSessions, session);

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Helper function
///////////////////////////////////////////////////////////////
/// \brief Allocates resources needed for an Listen session
static int
OpenListenSession(
    ListenSessionData ** aSession)
{
    int returnValue = FAILURE;

    ListenSessionData * session;

    *aSession = NULL;

    returnValue = OsWrap_AddToList(&ListenSessions,
                                   &session,
                                   sizeof(*session));

    *aSession = session;

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Helper function
///////////////////////////////////////////////////////////////
/// \brief Thread function used to marshal Ake for a source device
///
/// Allocates internal resources for a new connection.  Calls
/// DTCP component's source Ake function which marshalls the AKE.
/// Releases resources when connection is finished.
static int 
ConnectionThread(
    BasicP2PConnection    * aConnection)
{
    int returnValue = SUCCESS;

    BasicP2PConnection    * connection = aConnection;

    AkeSessionData        * session;
    SessionHandlesData    * sessionHandles;
    ListenSessionData     * listenSession;

    DEBUG_MSG(MSG_DEBUG, ("New Source Connection Handle=0x%08x\n", connection));

    sessionHandles = (SessionHandlesData *) malloc(sizeof(*sessionHandles));
    if (sessionHandles)
    {
        // Need to make an extra copy of the session handles structure so each 
        // connection has its own copy with its own AkeSession pointer in it.

        memcpy(sessionHandles, connection->UserData, sizeof(*sessionHandles));

        listenSession = sessionHandles->ListenSession;

        returnValue = OpenAkeSession(&listenSession->AkeSessions, &session);

        if (IS_SUCCESS(returnValue))
        {
            DtcpAkeIpMessagingInterface messagingInterface;
    
            session->Type       = DeviceTypeSource;
            session->Connection = connection;

            sessionHandles->AkeSession = session;

            connection->UserData = (void *) sessionHandles;

            GetMessagingInterface(&messagingInterface);
            
            // Ignore return code from this function.  If it fails,
            // we still need to let the thread terminate normally
            // and the DTCP component must use another mechanism
            // for notifying the app of an error.
            listenSession->SourceAkeFunc(
                (DtcpTransportHandle) session,
                listenSession->UserData,
                &messagingInterface);

            //NOTE: When this function returns, we are done listening for
            //      commands from this particular remote device so we can
            //      close the session.

            CloseAkeSession(&listenSession->AkeSessions, session);

            DEBUG_MSG(MSG_DEBUG, ("Source Connection Handle=0x%08x has been Closed\n", connection));
        }
        else
        {
            free(sessionHandles);
        }
    }

    return (0);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Allocates internal resources needed before we begin processing
static int 
DtcpAke_Startup(
    int * aMessageHeaderSize)
{
    int returnValue = FAILURE;

    if (DtcpAkeTransportInitialized)
    {
        return (FAILURE);
    }

    if (NULL == aMessageHeaderSize)
    {
        return (INVALID_ARGUMENT);
    }

    *aMessageHeaderSize = AKE_MESSAGE_HEADER_SIZE;
    
    returnValue = SUCCESS;

    if (IS_SUCCESS(returnValue))
    {
        DtcpAkeTransportInitialized = TRUE;
    }

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief Releases internal resources before component is unloaded
///
/// \todo If either session list is not empty, should we go through
/// and cancel all the AKE's and Listen connections before deleting
/// the lists???
static int 
DtcpAke_Shutdown()
{
    int returnValue = FAILURE;

    if (!DtcpAkeTransportInitialized)
    {
        return (FAILURE);
    }

    OsWrap_DeleteList(&ListenSessions);
    OsWrap_DeleteList(&InitiateSessions);

    DtcpAkeTransportInitialized = FALSE;

    return (returnValue);
}


///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief starts up a dtcp source device
static int 
DtcpAkeIp_Listen(
    DtcpAke_SourceAke_Ptr     aSourceAkeFunc,
    char                    * aIpAddress,
    void                    * aUserData,
    int                     * aIpPortNumber,
    DtcpTransportHandle     * aTransportHandle)
{
    int returnValue = FAILURE;

    ListenSessionData * listenSession = NULL;

    if (!DtcpAkeTransportInitialized)
    {
        return (FAILURE);
    }

    if (NULL == aIpAddress      ||
        NULL == aSourceAkeFunc  ||
        NULL == aIpPortNumber   ||
        NULL == aTransportHandle)
    {
        returnValue = INVALID_ARGUMENT;
        return (returnValue);
    }

    if (!InetWrap_IsDottedDecimal(aIpAddress))
    {
        returnValue = INVALID_ARGUMENT;
        return (returnValue);
    }

    returnValue = OpenListenSession(&listenSession);

    if (IS_SUCCESS(returnValue))
    {
        int portNumber = *aIpPortNumber;

        SessionHandlesData * sessionHandles = (SessionHandlesData *)
            malloc(sizeof(*sessionHandles));

        listenSession->UserData = aUserData;
        
        if (sessionHandles)
        {
            sessionHandles->ListenSession = listenSession;
            sessionHandles->AkeSession    = NULL;

            returnValue = BasicP2P_OpenConnection(NonBlockingListener,
                                                    aIpAddress,
                                                    NULL,
                                                    portNumber,
                                                    ReaderThread,
                                                    ConnectionThread,
                                                    0,
                                                    sessionHandles,
                                                    &listenSession->Connection);

            if (IS_SUCCESS(returnValue))
            {
                listenSession->SourceAkeFunc = aSourceAkeFunc;

                *aIpPortNumber    = listenSession->Connection->PortNumber;
                *aTransportHandle = (DtcpTransportHandle) listenSession;
            }
        }
        else
        {
            returnValue = FAILURE;
        }
    }

    if (IS_FAILURE(returnValue))
    {
        CloseListenSession(listenSession);
    }

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief initiates a client connection to a source device
static int 
DtcpAkeIp_StartSink(
    char                        * aIpAddress,
    int                           aIpPortNumber,
    DtcpTransportHandle         * aTransportHandle,
    DtcpAkeIpMessagingInterface * aMessagingInterface)
{
    int returnValue = FAILURE;

    AkeSessionData * session;

    if (NULL == aIpAddress         ||
        0    == aIpPortNumber      ||
        NULL == aTransportHandle   ||
        NULL == aMessagingInterface)
    {
        returnValue = INVALID_ARGUMENT;
        return (returnValue);
    }

    if (!InetWrap_IsDottedDecimal(aIpAddress))
    {
        returnValue = INVALID_ARGUMENT;
        return (returnValue);
    }

    returnValue = OpenAkeSession(&InitiateSessions, &session);

    if (IS_SUCCESS(returnValue))
    {
        SessionHandlesData * sessionHandles = (SessionHandlesData *)
            malloc(sizeof(*sessionHandles));

        if (sessionHandles)
        {
            sessionHandles->ListenSession = NULL;
            sessionHandles->AkeSession    = session;

            returnValue = BasicP2P_OpenConnection(
                Initiater,
                NULL,
                aIpAddress,
                aIpPortNumber,
                ReaderThread,
                NULL,
                0,
                sessionHandles,
                &session->Connection);
        }
        else
        {
            returnValue = FAILURE;
        }

        if (IS_SUCCESS(returnValue))
        {
            session->Type     = DeviceTypeSink;
            *aTransportHandle = (DtcpTransportHandle) session;
            GetMessagingInterface(aMessagingInterface);
        }
        else
        {
            CloseAkeSession(&InitiateSessions, session);
        }
    }

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief close out resources for handles allocated by DtcpAkeIp_Listen and DtcpAkeIp_StartSink
static int 
DtcpAke_CloseTransportHandle(
    DtcpTransportHandle aTransportHandle)
{
    int returnValue = SUCCESS;

    if (aTransportHandle)
    {
        if (OsWrap_EntryIsInList(&ListenSessions, (void *) aTransportHandle))
        {
            ListenSessionData * listenSession = (ListenSessionData *) aTransportHandle;

            LinkedList        * currentEntry  = listenSession->AkeSessions;
            AkeSessionData    * session       = NULL;

            int startTime;
            int forcibleTermination = FALSE;

            // For all currently open Ake sessions for this listener, go through
            // and cancel them all.  The connection thread func will actually
            // close the sessions so all we have to do is cancel them.
            while (currentEntry)
            {
                session      = currentEntry->Memory;
                currentEntry = (LinkedList *) currentEntry->Next;
                CancelAkeSession(session);
            }

            startTime = OsWrap_GetCurrentTime();

            while (listenSession->AkeSessions)
            {
                OsWrap_Sleep(1);

                // If things haven't cleaned up after 2 seconds, we need to bail out of here.
                if (2.0 < OsWrap_GetElapsedTime(startTime))
                {
                    forcibleTermination = TRUE;
                    break;
                }
            }

            // If we didn't have a good cleanup, and we close these resources, we risk having
            // a separate thread fault.  Instead, lets let a few resources leak out the door.

            if (!forcibleTermination)
            {
                if (listenSession->Connection->UserData)
                {
                    free(listenSession->Connection->UserData);
                }

                BasicP2P_CloseConnection(&listenSession->Connection);
                OsWrap_RemoveFromList(&ListenSessions, listenSession);
            }
        }
        else if (OsWrap_EntryIsInList(&InitiateSessions, (void *) aTransportHandle))
        {
            AkeSessionData * session = (AkeSessionData *) aTransportHandle;

            CancelAkeSession(session);
            CloseAkeSession(&InitiateSessions, session);
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

    return (returnValue);
}

///////////////////////////////////////////////////////////////
////// Exported function
///////////////////////////////////////////////////////////////
/// \brief fill in interface structure
DLLEXPORT int 
DtcpAkeIp_GetInterface(
    DtcpAkeIpInterface * aInterface)
{
    int returnValue = SUCCESS;

    if (aInterface)
    {
        if (sizeof(DtcpAkeIpInterface) == aInterface->StructureSize)
        {
            if (DTCP_AKE_IP_TRANSPORT_INTERFACE_VERSION == aInterface->InterfaceVersion)
            {
                aInterface->Startup                  = &DtcpAke_Startup;
                aInterface->Shutdown                 = &DtcpAke_Shutdown;
                aInterface->CloseTransportHandle     = &DtcpAke_CloseTransportHandle;
                aInterface->Listen                   = &DtcpAkeIp_Listen;
                aInterface->StartSink                = &DtcpAkeIp_StartSink;
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
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return (returnValue);
}

