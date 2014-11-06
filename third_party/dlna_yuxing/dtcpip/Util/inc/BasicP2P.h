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
/// \brief Definition file for Basic Peer to Peer interface.
///
/// This header file defines a set of API's that provide a mechanism for simplifying
/// the implementation of a peer to peer network application.  Much of the work of
/// using the low level Berkely sockets API to establish the connection is provided.
/// Launching of additional threads to support asynchronous receipt of messages from
/// the remove device is provided.  Launching of a separate thread to listen for
/// new connections on the server side is provided.  The term "Listener" is used for
/// the device that waits for a connection from a remote device.  The term "Initiator"
/// is used for the device that performs the initial connect.  These terms are roughly
/// analogous to Server/Client but only with respect to how the connection is started.
/// Once the connection is established, the communication is true P2P and either device
/// can send/receive messages and close the connection.


#ifndef __BASIC_P2P_H
#define __BASIC_P2P_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "InetWrapper.h"
#include "OsWrapper.h"

/// \defgroup BasicP2P BasicP2P
/// \brief Basic peer to peer networking interface.
/// 
/// This library consists of a set API's that provide a mechanism for simplifying
/// the implementation of a peer to peer network application.  Much of the work of
/// using the low level Berkely sockets API to establish the connection is provided.
/// Launching of additional threads to support asynchronous receipt of messages from
/// the remove device is provided.  Launching of a separate thread to listen for
/// new connections on the server side is provided.  The term "Listener" is used for
/// the device that waits for a connection from a remote device.  The term "Initiator"
/// is used for the device that performs the initial connect.  These terms are roughly
/// analogous to Server/Client but only with respect to how the connection is started.
/// Once the connection is established, the communication is true P2P and either device
/// can send/receive messages and close the connection.
/// @{

/// \brief enum for identifying the connection type
///
/// Typedef used to identify what type of connection is being requested.
/// A "BlockingListener" will wait for a connection from one remote
/// device and then return from BasicP2P_OpenConnection when it is received.
/// A "NonBlockingListener" will return from BasicP2P_OpenConnection after
/// establishing the bound socket and launching a "listen thread" that will
/// accept connections from remote devices until the connection is closed.
typedef enum
{
    Initiater,            ///< client connection
    BlockingListener,     ///< listener connection, just one connection allowed
    NonBlockingListener   ///< listener connection, multiple connections allowed
} ConnectionModes;




/// \brief state information for a basicp2p connection
///
/// Structure used to maintain all relevant state information for 
/// a particular connection.  Structure data is populated during the
/// OpenConnection call.  The structure is allowed to be visible and
/// modifiable by using applications to allow maximum flexibility in
/// implementing P2P applications.
/// The using application must provide the implementation of the reader
/// and connection threads.  API's defined in OSWrapper.h are utilized
/// extensively in this library.  Implementations of reader and connection
/// threads can also use API's in OSWrapper.h.
typedef struct __basicp2pconnection__
{
    // Client supplied data
    char                         * LocalName;                      ///< pointer to malloc'ed buffer containing friendly name of local device
    char                         * PeerName;                       ///< pointer to malloc'ed buffer containing friendly name of peer device
    char                           LocalAddress[32];               ///< dotted decimal ip address of local device
    char                           PeerAddress[32];                ///< dotted decimal ip address of peer device
    int                            PortNumber;                     ///< port number used for connection
    ConnectionModes                ConnectionMode;                 ///< connection mode from this devices perspective
    void                         * ReaderThread;                   ///< address of reader thread function
    void                         * ConnectionThread;               ///< address of connection thread function
    int                            ListenerTimeout;                ///< timeout to use when doing a blocking listen
    void                         * UserData;                       ///< space allocated for user information, not used internally

    // Internal data
    SOCKET_HANDLE                  MasterSocket;                   ///< socket handle of master socket for listening connections
    SOCKET_HANDLE                  ConnectionSocket;               ///< socket handle for connection socket used for actually sending/receiving msgs
    void                         * ReaderThreadID;                 ///< id of launched reader thread
    void                         * ReaderThreadHandle;             ///< handle of launched reader thread
    int                            TerminateReaderThread;          ///< boolean used to tell reader thread to terminate
    int                            ReaderThreadReturnValue;        ///< return value from the reader thread
    EVENT_HANDLE                   ReaderThreadTerminated;         ///< event used to indicate that reader thread has successfully terminated
    void                         * ListenerThreadID;               ///< id of launched listener thread
    void                         * ListenerThreadHandle;           ///< handle of launched listener thread
    int                            TerminateListenerThread;        ///< boolean used to tell listener thread to terminate
    int                            ListenerThreadReturnValue;      ///< return value from the listener thread
    EVENT_HANDLE                   ListenerThreadTerminated;       ///< event used to indicate that listener thread has successfully terminated
    void                         * ConnectionThreadID;             ///< id of launched connection thread
    void                         * ConnectionThreadHandle;         ///< handle of launched connection thread
} BasicP2PConnection;

typedef int
(*ThreadFunc_Ptr)(
    BasicP2PConnection * aConnection);


extern int
BasicP2P_OpenConnection(
    ConnectionModes                 aConnectionMode,
    char                          * aLocalName,
    char                          * aPeerName,
    int                             aPortNumber,
    ThreadFunc_Ptr                  aReaderThread,
    ThreadFunc_Ptr                  aConnectionThread,
    int                             aListenerTimeout,
    void                          * aUserData,
    BasicP2PConnection           ** aConnection);

extern int
BasicP2P_CloseConnection(
    BasicP2PConnection           ** aConnection);

/// @}

#ifdef __cplusplus
}
#endif

#endif
