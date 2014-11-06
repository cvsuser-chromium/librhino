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
/// \brief Definition file for Internet Abstraction interface.
///
/// This header file defines a set of API's that provide an OS agnostic interface
/// that provides a subset of networking capabilities that are used in the DTCP SDK.

#ifndef __INET_WRAPPER_H
#define __INET_WRAPPER_H

#ifdef __cplusplus
extern "C"
{
#endif

/// \defgroup InetWrapper InetWrapper
/// \brief Internet abstraction library
///
/// Provide an OS agnostic interface
/// for a subset of networking capabilities that are used in the DTCP SDK.
/// @{

/// \brief invalid socket handle
#define INVALID_SOCKET_HANDLE  ((void *) -1)

/// \brief socket handle typedef
///
/// Used on socket API's and internally maps to the appropriate
/// handle for each supported OS
typedef void * SOCKET_HANDLE;

extern int
InetWrap_OpenWindowsSockets();

extern int
InetWrap_CloseWindowsSockets();

extern SOCKET_HANDLE
InetWrap_OpenTCPSocket();

extern SOCKET_HANDLE
InetWrap_AcceptTCPConnection(
    SOCKET_HANDLE   aSocket,
    char          * aAcceptedName,
    char          * aAcceptedAddress,
    int             aAcceptedNameLength);

extern int
InetWrap_BindTCPSocket(
    SOCKET_HANDLE        aSocket,
    char               * aLocalName,
    int                * aPortNumber);

extern int
InetWrap_ConnectTCPSocket(
    SOCKET_HANDLE   aSocket,
    char          * aPeerName,
    int             aPortNumber);

extern int
InetWrap_TCPReadBufferSize(
    SOCKET_HANDLE     aSocket,
    int             * aBufferSize);

extern int
InetWrap_TCPRecvData(
    SOCKET_HANDLE     aSocket,
    char            * aBuffer,
    int             * aBufferSize);

extern int
InetWrap_TCPSendData(
    SOCKET_HANDLE     aSocket,
    char            * aBuffer,
    int               aBufferSize);

extern int
InetWrap_CloseSocket(
    SOCKET_HANDLE aSocket);

extern int
InetWrap_GetLastSocketError();

extern int
InetWrap_GetINetInfo(
    char * aName, 
    char * aAddress,
    int    aNameLength);

extern unsigned long
InetWrap_GetPeerAddress(
    char * aPeerName);

extern int
InetWrap_IsDottedDecimal(char * aAddress);

extern int 
InetWrap_SetSocketTTL(
    SOCKET_HANDLE        aSocket,
    int                  aTtl);

/// @}

#ifdef __cplusplus
}
#endif

#endif
