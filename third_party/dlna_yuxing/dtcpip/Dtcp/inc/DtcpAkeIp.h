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

#ifndef __DTCP_AKE_IP_H__
#define __DTCP_AKE_IP_H__

/// \file
/// \brief Defines the interface to the \ref DtcpAkeIp library.
///
/// This header file contains the interface to the \ref DtcpAkeIp library.  


/// \defgroup DtcpAkeIp DtcpAkeIp
/// \brief This library provides the IP-specific functionality for 
/// establishing an AKE using DTCP.
///
/// While the \ref DtcpAkeCore library maintains the core data for 
/// establishing an AKE, this library maintains the IP-specific data
/// for establishing an AKE.  This includes maintaining the state machine
/// and checking and setting the appropriate values of the command 
/// headers.  Below are state machine diagrams for a source device and
/// a sink device.
/// \image html DtcpAkeIp_SourceStateMachine.gif "Source State Machine"
/// \image html DtcpAkeIp_SinkStateMachine.gif "Sink State Machine"
/// @{

#include "DtcpCore.h"
#include "DtcpAkeIpTypes.h"
#include "DtcpAkeCoreTypes.h"

/// \brief Initializes the DtcpAkeIp library with the appropriate parameters. 
///        This function must be called before any of the other DtcpAkeIp functions 
///        can be called.
/// \param aExchangeKeyUpdateRequestFunc - Input; Pointer to a function to request
///        an exchange key update.
int DtcpAkeIp_Startup(ExchangeKeyUpdateRequest_Ptr  aExchangeKeyUpdateRequestFunc);

/// \brief Uninitializes the DtcpAkeIp library.
void DtcpAkeIp_Shutdown();

/// \brief Causes this device to begin listening for sink devices to establish
///        an AKE.
///
/// This function is called by source devices.
/// \param aIpAddress - Input; Pointer to an IP address in dotted decimal notation
/// \param aSourcePortNumber - Input/Output; Pointer to a value containing the port 
///        number to listen to or zero to select an unused port.  Upon successful
///        completion will contain the port number that this device is listening on.
/// \param aListenHandle - Output; Upon successful completion will contain a valid
///        DtcpAkeIpListenHandle
int DtcpAkeIp_Listen(char                  *aIpAddress, 
                     unsigned int          *aSourcePortNumber,
                     DtcpAkeIpListenHandle *aListenHandle);

/// \brief Causes this device to stop listening for sink devices trying to establish
///        an AKE.
///
/// This function is called by source devices.
/// \param aListenHandle - Input; Handle to stop listening on.
int DtcpAkeIp_CancelListen(DtcpAkeIpListenHandle aListenHandle);

/// \brief Prepares this device to initiate an AKE with a source device.
///
/// This function is called by sink devices.
/// \param aSourceIpAddress - Input; Pointer to the IP address (in dotted decimal notation)
///        of the source device with which an AKE shall be initiated.
/// \param aSourcePortNumber- Input; The port number of the source device with which an
///        AKE shall be initiated.
/// \param aAkeIpSessionHandle - Output; Upon successful completion will contain a valid
///        DtcpAkeIpSessionHandle.
int DtcpAkeIp_OpenAke(char *aSourceIpAddress,
                      unsigned int aSourcePortNumber,
                      DtcpAkeIpSessionHandle *aAkeIpSessionHandle);

/// \brief Establishes an AKE with a source device.
///
/// This function is called by sink devices.
/// \param aAkeIpSessionHandle - Input; Session handle identifying the source device.
int DtcpAkeIp_DoAke(DtcpAkeIpSessionHandle aAkeIpSessionHandle);

/// \brief Releases any data structures associated with an AKE.
///
/// This function is called by sink devices.
/// \param aAkeIpSessionHandle - Input; Session handle identifying the session to close.
int DtcpAkeIp_CloseAke(DtcpAkeIpSessionHandle aAkeIpSessionHandle);

//int DtcpAkeIp_GetAkeCoreHandle(DtcpAkeIpHandle   aAkeIpHandle, 
//                               DtcpAkeCoreHandle *aAkeCoreHandle);

/// \brief Gets the session handle for the \ref DtcpAkeCore library.
///
/// \param aAkeIpSessionHandle - Input; A valid \ref DtcpAkeIp session handle.
/// \param aAkeCoreSessionHandle - Output; Upon successful completion will contain
///        the associated \ref DtcpAkeCore session handle.
int DtcpAkeIp_GetAkeCoreSessionHandle(DtcpAkeIpSessionHandle    aAkeIpSessionHandle, 
                                      DtcpAkeCoreSessionHandle *aAkeCoreSessionHandle);

/// \brief Creates a new set of exchange keys.
///
/// This function is called by source devices.
/// \param aExchangeKeyLabel - Output; Upon successful completion will contain the updated
///        exchange key label
int DtcpAkeIp_UpdateExchangeKey(unsigned int    *aExchangeKeyLabel);

/// \brief Determines whether or not the current exchange key label is valid
///
/// This function is called by sink devices.
/// \param aAkeIpSessionHandle - Input; Handle of the AKE session of which the 
///        exchange key label is being checked
/// \param aExchangeKeyLabel - Input; Value of the source device's exchange key label. A value
///        of -1 will cause the sink device to send a ContentKeyRequest to the source
///        device to retrieve the value of the current exchange key label.
/// \param aExchangeKeyValidFlag - Output; Upon successful completion will contain 0 if the 
///        exchange key is not valid and a non-zero value if the exchange key is valid.
int DtcpAkeIp_ValidateExchangeKey(DtcpAkeIpSessionHandle  aAkeIpSessionHandle,
                                  int                     aExchangeKeyLabel,
                                  int                    *aExchangeKeyValidFlag);


/// \brief Gets the exchange key label of the exchange keys that this device is using
/// when acting as a source device.
///
/// This function is called by source devices.
/// \param aExchangeKeyLabel - Output; Upon successful completion will contain the value
///        of the exchange key label.
int DtcpAkeIp_GetSourceExchangeKeyLabel(unsigned int    *aExchangeKeyLabel);

/// \brief Checks a source device's sink count limit.
///
/// This function is called by sink devices.
/// \param aSourceAkeIpAddr - Input; Pointer to the IP address (in dotted decimal notation)
///        of the source device to check.
/// \param aSourceAkePortNumber- Input; The port number of the source device to check.
int DtcpAkeIp_CheckSourceSinkLimit(char *aSourceAkeIpAddr, unsigned int aSourceAkePortNumber);


/// @}

#endif // __DTCP_AKE_IP_H__
