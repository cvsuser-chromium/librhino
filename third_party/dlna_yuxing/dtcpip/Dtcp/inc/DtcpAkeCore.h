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

#ifndef __DTCP_AKE_CORE_H__
#define __DTCP_AKE_CORE_H__

#include "DtcpCoreTypes.h"
#include "DtcpAkeCoreTypes.h"

/// \file
/// \brief Defines the interface to the \ref DtcpAkeCore library.
///
/// This header file contains the interface to the \ref DtcpAkeCore library.  This library
/// implementes the functions that create and consume the data transferred
/// between two devices when performing an authentication and key exchange.

#ifdef __cplusplus
extern "C"
{
#endif

/// \defgroup DtcpAkeCore DtcpAkeCore
/// \brief Core algorithms for authentication and key exchange
/// 
/// This library implements functions that create and consume the data
/// transferred back and forth when performing an authentication and 
/// key exchange.  It contains functions for each step in the AKE (i.e. 
/// challenge commands, response commands, exchange key commands, etc.).
/// @{

/// \brief Initializes the DtcpAkeCore library.
/// 
/// This function initializes the library and starts the real time nonce timer.
/// It must be called before any of the other functions are called.  It should
/// only be called once until DtcpAkeCore_Shutdown is called and then the 
/// library may be re-initialized.
/// \param aExchangeKeyUpdateRequestFunc Input - Pointer to a callback function this is implemented
///                                      by the application to process a request by the DTCP-IP SDK to
///                                      to update the exchange keys by calling DtcpApi_UpdateExchangeKeys.
int DtcpAkeCore_Startup(ExchangeKeyUpdateRequest_Ptr  aExchangeKeyUpdateRequestFunc);

/// \brief Releases any resource associated with the handle.
///
void DtcpAkeCore_Shutdown();

/// \brief Opens a new AKE session
///
/// This function initializes resources for a new AKE session.  It must
/// be called before any function call requiring a valid 
/// DtcpAkeCoreSessionHandle is called.
/// \param aAkeTypeId - Input; Specifies the type of AKE to perform
/// \param aDeviceMode - Input; Specifies the mode (source or sink) that
///                      this device is operating in for the AKE
/// \param aAkeCoreSessionHandle - Output; Upon successful completion will
///                                contain a valid handle that can be used
///                                with any function requiring a 
///                                DtcpAkeCoreSessionHandle
int DtcpAkeCore_OpenAkeSession(EnAkeTypeId               aAkeTypeId,
                               EnDeviceMode              aDeviceMode,
                               DtcpAkeCoreSessionHandle *aAkeCoreSessionHandle);

/// \brief Releases any resources associated with the handle.
///
/// \param aAkeCoreSessionHandle - Input; Handle to an AKE session
int DtcpAkeCore_CloseAkeSession(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle);

/// \brief Releases the buffers specified by the buffer mask
///
/// The DtcpAkeCore_Create* calls allocate internal buffers that must be
/// released when they are no longer needed in order to prevent memory
/// leaks.
/// \param aAkeCoreSessionHandle - Input; A valid handle to an AKE session
/// \param aBufferMask - Input; a bit mask specifying which buffer is to be
///                      released
int DtcpAkeCore_ReleaseBuffers(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                               EnAkeBufferMask aBufferMask);

/// \brief Cancels an AKE session
///
/// \param aAkeCoreSessionHandle - Input; A valid handle to an AKE session
int DtcpAkeCore_CancelAke(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle);

/// \brief Creates the challenge data for an AKE
/// 
/// Creates the appropriate challenge data for the AKE_Info part of the
/// AKE messages.  This call allocates memory and DtcpAkeCore_ReleaseBuffers()
/// must be called to prevent a memory leak.
/// \param aAkeCoreSessionHandle - Input; A valid handle to an AKE session
/// \param aChallengeCmdBufferPtr - Output; Upon successful completion, will
///                                 point to a buffer containing the challenge
///                                 data
/// \param aChallengeCmdBufferSize - Output; Upon successful completion, will
///        contain the size of the buffer pointed to by aChallengeCmdBufferPtr
int DtcpAkeCore_CreateChallengeData(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                                    unsigned char **aChallengeCmdBufferPtr,
                                    unsigned int *aChallengeCmdBufferSize);

/// \brief Consumes the challenge data for an AKE
///
/// This function consumes the challenge data received from another device
/// while performing an AKE.
/// \param aAkeCoreSessionHandle - Input; A valid handle to an AKE session
/// \param aChallengeCmdBuffer - Input; Points to a buffer containing the 
///        challenge data
/// \param aChallengeCmdBufferSize - Input; Size in bytes of the data 
///        pointed to by the aChallengeCmdBuffer
int DtcpAkeCore_ConsumeChallengeData(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                                     unsigned char *aChallengeCmdBuffer,
                                     unsigned int aChallengeCmdBufferSize);

/// \brief Creates the response data for an AKE
/// 
/// Creates the appropriate response data for the AKE_Info part of the
/// AKE messages.  This call allocates memory and DtcpAkeCore_ReleaseBuffers()
/// must be called to prevent a memory leak.
/// \param aAkeCoreSessionHandle - Input; A valid handle to an AKE session
/// \param aResponseCmdBufferPtr - Output; Upon successful completion, will
///                                 point to a buffer containing the response
///                                 data
/// \param aResponseCmdBufferSize - Output; Upon successful completion, will
///        contain the size of the buffer pointed to by aResponseCmdBufferPtr
int DtcpAkeCore_CreateResponseData(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                                   unsigned char **aResponseCmdBufferPtr,
                                   unsigned int *aResponseCmdBufferSize);

/// \brief Consumes the response data for an AKE
///
/// This function consumes the response data received from another device
/// while performing an AKE.
/// \param aAkeCoreSessionHandle - Input; A valid handle to an AKE session
/// \param aResponseCmdBuffer - Input; Points to a buffer containing the 
///        response data
/// \param aResponseCmdBufferSize - Input; Size in bytes of the data 
///        pointed to by the aResponseCmdBuffer
/// \param aOurSrmUpdateRequiredFlag - Output; Flag indicating whether our SRM should be updated or not.
///                                    (1 == Our SRM should be updated, 0 == Our SRM should not be udpated)
int DtcpAkeCore_ConsumeResponseData(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                                    unsigned char *aResponseCmdBuffer,
                                    unsigned int  aResponseCmdBufferSize,
                                    unsigned int *aOurSrmUpdateRequiredFlag);

/// \brief Creates the exchange key data for an AKE
/// 
/// Creates the appropriate exchange key data for the AKE_Info part of the
/// AKE messages.  This call allocates memory and DtcpAkeCore_ReleaseBuffers()
/// must be called to prevent a memory leak.
/// \param aAkeCoreSessionHandle - Input; A valid handle to an AKE session
/// \param aExchKeyId - Input; Specifies which exchange key data to create
/// \param aExchangeKeyCmdBufferPtr - Output; Upon successful completion, will
///        point to a buffer containing the exchange key data
/// \param aExchangeKeyCmdBufferSize - Output; Upon successful completion, will
///        contain the size of the buffer pointed to by aExchangeKeyeCmdBufferPtr
int DtcpAkeCore_CreateExchangeKeyData(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                                      EnExchKeyId aExchKeyId,
                                      unsigned char **aExchangeKeyCmdBufferPtr,
                                      unsigned int *aExchangeKeyCmdBufferSize);

/// \brief Consumes the exchange key data for an AKE
///
/// This function consumes the exchange key data received from another device
/// while performing an AKE.
/// \param aAkeCoreSessionHandle - Input; A valid handle to an AKE session
/// \param aExchKeyId - Input; Specifies which type of exchange key this data
///        is for
/// \param aExchangeKeyCmdBuffer - Input; Points to a buffer containing the 
///        exchange key data
/// \param aExchangeKeyCmdBufferSize - Input; Size in bytes of the data 
///        pointed to by the aExchangeKeyCmdBuffer
int DtcpAkeCore_ConsumeExchangeKeyData(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                                       EnExchKeyId aExchKeyId,
                                       unsigned char *aExchangeKeyCmdBuffer,
                                       unsigned int aExchangeKeyCmdBufferSize);

/// \brief Creates the SRM data for an AKE
/// 
/// Creates the appropriate SRM data for the AKE_Info part of the
/// AKE messages.  This call allocates memory and DtcpAkeCore_ReleaseBuffers()
/// must be called to prevent a memory leak.
/// \param aAkeCoreSessionHandle - Input; A valid handle to an AKE session
/// \param aSrmCmdBufferPtr - Output; Upon successful completion, will
///        point to a buffer containing the SRM data
/// \param aSrmCmdBufferSize - Output; Upon successful completion, will
///        contain the size of the buffer pointed to by aSrmCmdBufferPtr
int DtcpAkeCore_CreateSrmData(DtcpAkeCoreSessionHandle  aAkeCoreSessionHandle,
                              unsigned char           **aSrmCmdBufferPtr,
                              unsigned int             *aSrmCmdBufferSize);

/// \brief Consumes the SRM data for an AKE
///
/// This function consumes the SRM data received from another device
/// while performing an AKE.
/// \param aAkeCoreSessionHandle - Input; A valid handle to an AKE session
/// \param aSrmCmdBuffer - Input; Points to a buffer containing the 
///        SRM data
/// \param aSrmCmdBufferSize - Input; Size in bytes of the data 
///        pointed to by the aSrmCmdBuffer
int DtcpAkeCore_ConsumeSrmData(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                               unsigned char           *aSrmCmdBuffer,
                               unsigned int             aSrmCmdBufferSize);

/// \brief Gets the exchange key label for a device operating as a source
///
/// \param aExchangeKeyLabel - Output; Upon successful completion, will
///        contain the current exchange key label
int DtcpAkeCore_GetSourceExchangeKeyLabel(unsigned int      *aExchangeKeyLabel);

/// \brief Gets the exchange key and label for a device operating as a source
///
/// \param aExchangeKeyId - Input; Id of the exchange key to get
/// \param aExchangeKeyBuffer - Output; Upon successful completion, will contain the exchange key.
/// \param aExchangeKeyLabel - Output; Upon successful completion, will
///        contain the current exchange key label
int DtcpAkeCore_GetSourceExchangeKey(EnExchKeyId        aExchangeKeyId,
                                     unsigned char      aExchangeKeyBuffer[DTCP_EXCHANGE_KEY_SIZE],
                                     unsigned int      *aExchangeKeyLabel);

/// \brief Gets the exchange key and label for a device operating as a sink
///
/// \param aAkeCoreSessionHandle - Input; A valid handle to an AKE session
/// \param aExchangeKeyId - Input; Id of the exchange key to get
/// \param aExchangeKeyBuffer - Output; Upon successful completion, will contain the exchange key.
/// \param aExchangeKeyLabel - Output; Upon successful completion, will
///        contain the current exchange key label
int DtcpAkeCore_GetSinkExchangeKey(DtcpAkeCoreSessionHandle  aAkeCoreSessionHandle,
                                   EnExchKeyId               aExchangeKeyId,
                                   unsigned char             aExchangeKeyBuffer[DTCP_EXCHANGE_KEY_SIZE],
                                   unsigned int             *aExchangeKeyLabel);

/// \brief Gets the mode that this device is operating in for the AKE
///        session referred to by the aAkeCoreSessionHandle.
///
/// \param aAkeCoreSessionHandle - Input; A valid handle to an AKE session
/// \param aDeviceMode - Output; Upon successful completion, will
///        contain the mode this device is operating in for this AKE
///        session
int DtcpAkeCore_GetDeviceMode(DtcpAkeCoreSessionHandle  aAkeCoreSessionHandle,
                              EnDeviceMode             *aDeviceMode);

/// \brief Opens a stream for encryption.
///
/// \param aUpdateRealTimeNonceFlag Input - Flag indicating whether or not this stream
///        should update the real time nonce.  (0 = No, 1 = Yes)
/// \param aAkeCoreStreamHandle Output - Handle identifying this stream.
int DtcpAkeCore_OpenStream(int                      aUpdateRealTimeNonceFlag,
                           DtcpAkeCoreStreamHandle *aAkeCoreStreamHandle);

/// \brief Closes a stream
///
/// \param aAkeCoreStreamHandle Input - Handle identifying the stream.
int DtcpAkeCore_CloseStream(DtcpAkeCoreStreamHandle aAkeCoreStreamHandle);

/// \brief Gets the current value of the real time nonce
///
/// \param aNonce Output - Pointer to a buffer to receive the nonce
int DtcpAkeCore_GetRealTimeNonce(unsigned char      aNonce[DTCP_CONTENT_KEY_NONCE_SIZE]);

///// \brief Gets a handle to the DtcpAkeCore library
/////
///// \param aAkeCoreSessionHandle Input - Handle to an AKE session
///// \param aAkeCoreHandle Output - Handle to the DtcpAkeCore library
//int DtcpAkeCore_GetAkeCoreHandle(DtcpAkeCoreSessionHandle  aAkeCoreSessionHandle,
//                                 DtcpAkeCoreHandle        *aAkeCoreHandle);

/// \brief Updates the source exchange keys
///
/// \param aExchangeKeyLabel Output - The updated exchange key label
int DtcpAkeCore_UpdateExchangeKey(unsigned int      *aExchangeKeyLabel);

int DtcpAkeCore_ValidateExchangeKey(DtcpAkeCoreSessionHandle  aAkeCoreSessionHandle,
                                    int                       aExchangeKeyLabel,
                                    int                      *aExchangeKeyValidFlag);

int DtcpAkeCore_CheckSinkCountLimit();

/// @}

#ifdef __cplusplus
extern "C"
{
#endif

#endif // __DTCP_AKE_CORE_H__
