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

#ifndef __DTCP_AKE_CORE_INTERNAL_H__
#define __DTCP_AKE_CORE_INTERNAL_H__

/// \file
/// \brief Defines private types used within the \ref DtcpAkeCore library.
///
/// This header file contains the data structures used in the \ref DtcpAkeCore library.

#include "DtcpAkeCoreTypes.h"
#include "DtcpCoreTypes.h"
#include "DtcpExchangeKeys.h"
#include "DtcpUtils.h"
#include "OsWrapper.h"

/// \brief Data structure for maintaining the data for a source device as well
/// as the ake sessions for a sink device.
typedef struct DtcpAkeCoreData
{
    DtcpExchKeyData   ExchangeKeyData;              ///< Source device exchange key data
    LinkedList       *AkeSessions;                  ///< Linked list of active AKE sessions
    unsigned int      AuthenticatedSinkDeviceCount; ///< Count of authenticated sink devices
    unsigned char     AuthenticatedSinkDevices[DTCP_SINK_COUNT_LIMIT][DTCP_DEVICE_ID_SIZE]; ///< Device Id's of authenticated sink devices
    unsigned char     RealTimeNonce[DTCP_CONTENT_KEY_NONCE_SIZE]; ///< Nonce for RTP streams
    unsigned int      StreamCount;                  ///< Count of open streams
    unsigned int      UpdateRealTimeNonceStreamCount; ///< Count of open streams that update the RTP nonce
    void             *RealTimeNonceThreadId;        ///< Id of the thread used for updating the RTP nonce
    void             *RealTimeNonceThreadHandle;    ///< Handle of the thread used for updating the RTP nonce
    ExchangeKeyUpdateRequest_Ptr ExchangeKeyUpdateRequestFunc; ///< Pointer to a function that requests to update the exchange keys
    void             *DataSemaphore;                ///< Semaphore to protect shared data
} DtcpAkeCoreData;

/// \brief Data structure for maintaining the data for an AKE session.
typedef struct DtcpAkeCoreSessionData
{
    DtcpAkeCoreData  *AkeCoreData;                                        ///< Pointer to /ref DtcpAkeCoreData
    EnDeviceMode      DeviceMode;                                         ///< Mode that this device is operating in.
    unsigned char     DeviceId[DTCP_DEVICE_ID_SIZE];                      ///< Device Id of other device
    EnAkeTypeId       AkeTypeId;                                          ///< Type of AKE being performed
    unsigned char     OurNonce[DTCP_FULL_AUTH_NONCE_SIZE];                ///< This device's random nonce for the challenge
    unsigned int      OurNonceSize;                                       ///< Size of this device's random nonce
    unsigned char     TheirNonce[DTCP_FULL_AUTH_NONCE_SIZE];              ///< Other device's random nonce for the challenge
    unsigned int      TheirNonceSize;                                     ///< Size of their device's random nonce
    unsigned char     TheirPublicKey[PUBLIC_KEY_SIZE];                    ///< Other device's public key
    unsigned int      TheirKsv;                                           ///< Other device's key selection vector
    unsigned int      TheirSrmV;                                          ///< Other device's SRM version
    unsigned int      TheirSrmG;                                          ///< Other device's SRM generation
    unsigned int      TheirSrmC;                                          ///< Other device's SRM part count
    unsigned int      TheirApFlag;                                        ///< Other device's AP flag
    unsigned char     FirstPhaseValue[DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE]; ///< This device's Diffie-Hellman first phase value
    unsigned char     FirstPhaseSecret[DTCP_DH_FIRST_PHASE_SECRET_SIZE];  ///< This device's Diffie-Hellman first phase secret
    unsigned char     AuthKey[DTCP_AUTH_KEY_SIZE];                        ///< Authentication key (result of AKE)
    unsigned int      AuthenticatedFlag;                                  ///< Flag indicating whether the AKE succeeded
    DtcpExchKeyData   ExchangeKeyData;                                    ///< Exchange key data for this AKE
    unsigned char    *ChallengeCmdBuffer;                                 ///< Buffer for storing the challenge command data
    unsigned int      ChallengeCmdBufferSize;                             ///< Size of the challenge command buffer
    unsigned char    *ResponseCmdBuffer;                                  ///< Buffer for storing the response command data
    unsigned int      ResponseCmdBufferSize;                              ///< Size of the response command buffer
    unsigned char    *ExchKeyCopyOneGenCmdBuffer;                         ///< Buffer for storing the copy one generation exchange key command data
    unsigned int      ExchKeyCopyOneGenCmdBufferSize;                     ///< Size of the copy one generation exchange key command buffer
    unsigned char    *ExchKeyCopyNoMoreCmdBuffer;                         ///< Buffer for storing the copy no more exchange key command data  
    unsigned int      ExchKeyCopyNoMoreCmdBufferSize;                     ///< Size of the copy no more exchange key command buffer
    unsigned char    *ExchKeyCopyNeverCmdBuffer;                          ///< Buffer for storing the copy never exchange key command data
    unsigned int      ExchKeyCopyNeverCmdBufferSize;                      ///< Size of the copy never exchange key command buffer
    unsigned char    *ExchKeyAes128CmdBuffer;                             ///< Buffer for storing the AES-128 exchange key command data
    unsigned int      ExchKeyAes128CmdBufferSize;                         ///< Size of the AES-128 exchange key command buffer
    unsigned char    *SrmCmdBuffer;                                       ///< Buffer for storing the SRM command buffer
    unsigned int      SrmCmdBufferSize;                                   ///< Size of the SRM command buffer 
} DtcpAkeCoreSessionData;

/// \brief Data structure for storing data associated with a stream.
typedef struct
{
    DtcpAkeCoreData           *AkeCoreData;                    ///< Point to Content Management library data
    int                        UpdateRealTimeNonceStreamFlag;  ///< Flag indicating whether this stream should periodically increment the real time nonce
} DtcpAkeCoreStreamData;

/// \brief Checks to determine if the sink count limit has been reached
///
/// \param aAkeCoreData - Input; Pointer to a DtcpAkeCoreData structure
/// \param aDeviceId - Input; Buffer containing the device ID of the device trying to authenticate
/// \param aApFlag - Input; Value of AP flag for device trying to authenticate
/// \retval SUCCESS - No error
/// \retval DTCP_SINK_COUNT_LIMIT_REACHED - Sink count limit has been reached
int CheckSinkCountLimit(DtcpAkeCoreData *aAkeCoreData, 
                        unsigned char    aDeviceId[DTCP_DEVICE_ID_SIZE],
                        unsigned int     aApFlag);

#endif // __DTCP_AKE_CORE_INTERNAL_H__
