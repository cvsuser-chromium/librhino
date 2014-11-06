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

#ifndef __DTCP_AKE_IP_INTERNAL_H__
#define __DTCP_AKE_IP_INTERNAL_H__

/// \file
/// \brief Defines private types used within the \ref DtcpAkeIp library.
///
/// This header file contains the data structures used in the \ref DtcpAkeIp library.

#include "DtcpAkeTransport.h"
#include "DtcpCoreTypes.h"
#include "DtcpAkeCoreTypes.h"
#include "OsWrapper.h"

#define MAX_IP_ADDR_SIZE  128

/// \brief States for establishing the AKE
typedef enum 
{
    stateIdle,                  ///< Idle state
    stateChallenge,             ///< Challenge state
    stateResponse,              ///< Response state
    stateExchangeKey,           ///< Exchange Key state
    stateSrm,                   ///< Srm state
    stateAuthenticated,         ///< Authenticated state
    stateCancelled,             ///< Cancelled state
    stateError,                 ///< Error state
    stateCompleted              ///< Completed state
} EnAuthState;

/// \brief CType values
typedef enum
{
    ctypeControl = 0,           ///< Control command
    ctypeStatus = 1,            ///< Status command
    ctypeSpecificInquiry = 2,   ///< Specific Inquiry command
    ctypeNotify = 3,            ///< Notify command
    ctypeGeneralInquiry = 4,    ///< General Inquiry command
} EnCtype;

/// \brief Response values
typedef enum
{
    responseNotImplemented = 0x8,       ///< Not Implemented response
    responseAccepted = 0x9,             ///< Accepted response
    responseRejected = 0xA,             ///< Rejected response
    responseInTransition = 0xB,         ///< In Transition response
    responseImplementedStable = 0xC,    ///< Implemented/Stable response
    responseChanged = 0xD,              ///< Changed response
    responseInterim = 0xF,              ///< Interim response
} EnResponse;

/// \brief Status values
typedef enum
{
    statusNoError = 0,                  ///< No error
    statusNoMoreAuth = 0x1,             ///< No more authentication available
    statusAnyOtherError = 0x7,          ///< Any other error
    statusIncorrectCommandOrder = 0x8,  ///< Incorrect command order
    statusAuthFailed = 0x9,             ///< Authentication failed
    statusDataSyntaxError = 0xA,        ///< Data syntax error
    statusNoInformation = 0xF,          ///< No information
} EnStatus;

/// \brief Data structure for storing the data associated with this library
typedef struct DtcpAkeIpData
{
    LinkedList          *SinkSessions;          ///< List of sink sessions
    //DtcpTransportHandle  ListenHandle;        
    DtcpAkeIpInterface   AkeIpInterface;        ///< Interface to \ref AkeTransport library
    int                  AkeIpInterfaceLoaded;  ///< Flag indicating if the Ake Ip Interface has been loaded
    int                  AkeIpMinMessageSize;   ///< Minimum message size
    unsigned char        AkeLabelCounter;       ///< AKE label counter
    void                *DataSemaphore;         ///< Semaphore protecting integrity of this data structure
} DtcpAkeIpData;

/// \brief Data structure for storing the data required for each AKE session
typedef struct DtcpAkeIpSessionData
{
    char                         SourceIpAddress[MAX_IP_ADDR_SIZE]; ///< IP address of source device
    unsigned int                 SourcePortNumber;                  ///< Port number of source device
    DtcpTransportHandle          TransportHandle;                   ///< Transport handle for this session
    DtcpAkeIpMessagingInterface  MessagingInterface;                ///< Messaging interface
    DtcpAkeIpData               *AkeIpData;                         ///< Pointer to DtcpAkeIpData
    DtcpAkeCoreSessionHandle     AkeCoreSessionHandle;              ///< \ref DtcpAkeCore session handle
    EnAuthState                  CurrentAuthState;                  ///< Current state of AKE
    unsigned int                 OurSrmUpdateRequiredFlag;          ///< Flag indicating whether our SRM should be updated
    int                          ChallengeSentTime;                 ///< Time at which challenge command was sent (for timeouts)
    int                          ResponseSentTime;                  ///< Time at which response command was sent (for timeouts)
    int                          ResponseReceivedTime;              ///< Time at which response was received (for timeouts)
    int                          ExchangeKeySentTime;               ///< Time at which exchange key command was sent (for timeouts)
    int                          ExchangeKeyReceivedTime;           ///< Time at which exchange key command was received (for timeouts)
    int                          SrmReceivedTime;                   ///< Time at which SRM command was received (for timeouts)
    unsigned char                AkeLabel;                          ///< Label for this AKE
} DtcpAkeIpSessionData;

/// \brief This is a callback function that is called by the \ref AkeTransport library
/// when a sink device has connected with this device.
///
/// \param aTransportHandle - Input; A transport handle for the \ref AkeTransport library
/// \param aUserData - Input; A pointer to a DtcpAkeIpData structure
/// \param aMessagingInterface - Input; A pointer to the messaging interface provided by
/// the \ref AkeTransport library
int ListenCallback(DtcpTransportHandle          aTransportHandle,
                   void                        *aUserData,
                   DtcpAkeIpMessagingInterface *aMessagingInterface);

/// \brief This function initiates an AKE with a source device.
/// 
/// \param aAkeSessionData - Input; Pointer to the session data for the AKE
int StartSink(DtcpAkeIpSessionData *aAkeSessionData);

/// \brief Gets the exchange key label from a source device.
///
/// \param aAkeSessionData - Input; Pointer to the session data corresponding to the 
/// source device from which the exchange key label will be retrieved
/// \param aExchangeKeyLabel - Output; Upon successful completion will contain the
/// exchange key label for this session.
int GetSourceExchangeKeyLabel(DtcpAkeIpSessionData *aAkeSessionData, int *aExchangeKeyLabel);

/// \brief Checks whether the source device's sink count limit has been reached.
///
/// \param aAkeIpData - Input; Pointer to the DtcpAkeIpData structure
/// \param aTransportHandle - Input; A transport handle for the \ref AkeTransport library
/// \param aMessagingInterface - Input; A pointer to the messaging interface provided by the \ref AkeTransport library
int CheckSourceSinkLimit(DtcpAkeIpData *aAkeIpData, DtcpTransportHandle aTransportHandle, DtcpAkeIpMessagingInterface aMessagingInterface);

#endif // __DTCP_AKE_IP_INTERNAL_H__
