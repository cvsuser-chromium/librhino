/*
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
*/

/// \file
/// \brief Definition file for \ref DtcpAppLib.
///
/// This header file contains the interface to the \ref DtcpAppLib library.  This API
/// is the high-level interface used by an application to integrate with the DTCP
/// AKE system.

#ifndef _DTCP_APP_LIB_H_
#define _DTCP_APP_LIB_H_

#   ifdef __cplusplus
extern  "C"
{
#   endif

/*******************************************************************************
*                              Common functions                                *
*******************************************************************************/

/// \defgroup DtcpAppLib DtcpAppLib
/// \brief Library that provides a single interface to be used by a source and sink
/// integration for initializing DTCP-IP.
/// @{

/// \brief Initializes the DTCP library
///
/// This function must be called before any of the other DTCP functions can be
/// called, and it should only be called once.
///
/// This function is called by both source and sink devices.
int DtcpAppLib_Startup();

/// \brief Shuts down the DTCP libraries and releases any resources that are
/// in use by the DTCP libraries.
///
/// Do not call any of the other DtcpApi functions after calling this function 
/// without first calling DtcpAppLib_Startup again.
///
/// This function is called by both source devices and sink devices.
int DtcpAppLib_Shutdown();

/*******************************************************************************
*                              Source functions                                *
*******************************************************************************/

// Provides a single interface to be used by an source
// integration for initializing DTCP-IP.

/// \brief Retreives the source's listen IP address in dotted decimal
/// notation (xxx.xxx.xxx.xxx)
///
/// This function should be called after the DTCP library has been
/// iniialized with DtcpAppLib_Startup.
/// This function is only used by source devices.
const char *DtcpAppLib_GetSourceIp();// const;

/// \brief Retreives the source's listen IP port.
///
/// This function should be called after the DTCP library has been
/// iniialized with DtcpAppLib_Startup.
/// This function is only called by source devices.
unsigned short DtcpAppLib_GetSourcePort();// const;

/// \brief Begins listening for a sink device to try to establish an AKE.
///
/// This function should only be called one time for each IP address being
/// listened to for AKE messages.
/// This function should be called after the DTCP library has been
/// iniialized with DtcpAppLib_Startup.
/// This function is only called by source devices.
///
/// \param aListenIp Input - A string containing the IP address of the source
///                          in dotted decimal notation (xxx.xxx.xxx.xxx)
///                          The IP accress "0.0.0.0" (IN_ADDR_ANY) can be used
///                          to bin to all currently enabled network adapters.
///                          On a successful listen, the IP address that this
///                          device is listening on can be obtained by calling
///                          DtcpAppLib_GetSourceIp().
///
/// \param aListenPort Input - The port number to listen to or zero to select an
///                            unused port. On a successful listen, the port number
///                            that this device is listening on can be obtained by
///                            calling DtcpAppLib_GetSourcePort().
int DtcpAppLib_Listen(const char *aListenIp, unsigned short aListenPort);

/// \brief Cancels listening for AKE requests.
///
/// This function is called by source devices.  This function should only be
/// called after calling DtcpAppLib_Listen().
int DtcpAppLib_CancelListen();

/*******************************************************************************
*                               Sink functions                                 *
*******************************************************************************/

// Provides a single interface to be used by a sink
// integration for initializing DTCP-IP.

/// \brief Initializes an IP AKE session.
/// Before an AKE is initiated by a sink device, this function must be called.
/// This function is called by sink devices.
///
/// \param aSourceAkeIpAddr Input - A string containing the IP address of the source in 
///                              dotted decimal notation (xxx.xxx.xxx.xxx)
/// \param aSourceAkePortNumber Input - The port number that the source device
///                               is listening on for AKE connections.
/// \param aAkeHandle Output - A handle to this AKE session.
int DtcpAppLib_DoAke(const char *remote_ip, unsigned short remote_port, void **aAkeHandle);

/// \brief Closes an AKE.
/// This function is called by sink devices.
///
/// \param aAkeHandle Input - A handle to the AKE session to close.
int DtcpAppLib_CloseAke(void *aAkeHandle);

/// @}

#   ifdef __cplusplus
}
#   endif /* extern "C" */

#endif /* defined _DTCP_APP_LIB_H_ */
