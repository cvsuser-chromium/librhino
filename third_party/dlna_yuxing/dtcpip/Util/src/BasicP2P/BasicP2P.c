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
/// \brief Implementation file for BasicP2P.

#include "BasicP2P.h"

#include "StatusCodes.h"
#include "OsWrapper.h"
#include "InetWrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


///////////////////////////////////////////////////////////////
//// Macros and typedefs
///////////////////////////////////////////////////////////////


/// \brief defines a non-zero value for "true"
#define TRUE   1

/// \brief defines a zero value for "false"
#define FALSE  0


///////////////////////////////////////////////////////////////
//// Global Variables
///////////////////////////////////////////////////////////////


#if 0
static int GlobalDisplayLevel             = -1;
static int GlobalDisplayLevelMatchExactly = 0;
static int GlobalLogToFile                = 0;
#endif

///////////////////////////////////////////////////////////////
/// Internal Function prototypes
///////////////////////////////////////////////////////////////

static int
ReaderThread(void * aParameter);

static int
ListenerThread(void * aParameter);



/// \brief Opens an Initiator connection to a remote device
///
/// Opens a TCP socket, connects to a remote device and launches
/// a reader thread for processing incoming messages.
static int
OpenInitiaterConnection(
    BasicP2PConnection   * aConnection)
{
    int returnValue = FAILURE;

    BasicP2PConnection   * connection = aConnection;

    connection->ConnectionSocket = InetWrap_OpenTCPSocket();
    if (INVALID_SOCKET_HANDLE != connection->ConnectionSocket)
    {
        DEBUG_MSG(MSG_DEBUG+3, ("Successfully called socket API\n"));

        returnValue = InetWrap_ConnectTCPSocket(connection->ConnectionSocket,
                                                connection->PeerAddress,
                                                connection->PortNumber);

        if (IS_SUCCESS(returnValue))
        {
            DEBUG_MSG(MSG_DEBUG+3, ("Successfully called connect API on port %d\n", connection->PortNumber));

            if (connection->ReaderThread)
            {
                connection->ReaderThreadTerminated = OsWrap_CreateEvent(TRUE, FALSE);

                if (connection->ReaderThreadTerminated)
                {
                    // create reader thread for processing incoming messages
                    returnValue = OsWrap_NewThread(ReaderThread,
                                                   connection,
                                                   0,
                                                   &connection->ReaderThreadID,
                                                   &connection->ReaderThreadHandle);
                }
            }
            else
            {
                returnValue = FAILURE;
            }
        }
        else
        {
            ERROR_MSG(1, returnValue, ("Error %d calling connect API\n", InetWrap_GetLastSocketError()));
        }
    }
    else
    {
        ERROR_MSG(1, returnValue, ("Error calling socket API, error=%d\n",
                                      InetWrap_GetLastSocketError()));
    }

    return (returnValue);
}

/// \brief Opens and binds a TCP socket to an IP address and port number
///
static int
OpenMasterSocket(
    BasicP2PConnection   * aConnection)
{
    int returnValue = FAILURE;

    BasicP2PConnection   * connection = aConnection;

    connection->MasterSocket = InetWrap_OpenTCPSocket();
    if (INVALID_SOCKET_HANDLE != connection->MasterSocket)
    {
        DEBUG_MSG(MSG_DEBUG+3, ("Successfully called socket API\n"));

        returnValue = InetWrap_BindTCPSocket(connection->MasterSocket,
                                             connection->LocalAddress,
                                             &connection->PortNumber);
        /* ignore return */ InetWrap_SetSocketTTL(connection->MasterSocket, 3);

        if (IS_SUCCESS(returnValue))
        {
            DEBUG_MSG(MSG_DEBUG+3, ("Successfully bound socket on port %d\n", connection->PortNumber));

            if (BlockingListener == connection->ConnectionMode)
            {
                DEBUG_MSG(MSG_DEBUG+3, ("Only 1 valid connection request will be accepted\n"));
            }

            if (connection->PeerAddress)
            {
                DEBUG_MSG(MSG_DEBUG+3, ("Only connections from %s will be accepted\n",
                                 connection->PeerAddress));
            }
        }
        else
        {
            DEBUG_MSG(MSG_ERR, ("Error binding socket\n"));
        }
    }
    else
    {
        DEBUG_MSG(MSG_ERR, ("Error opening socket, error=%d\n",
                                      InetWrap_GetLastSocketError()));
    }

    return (returnValue);
}

/// \brief Verify that the connection from the remote device is acceptable.
///
static int
ValidatePeer(
    BasicP2PConnection   * aConnection,
    char                 * aAcceptedName,
    char                 * aAcceptedAddress)
{
    int returnValue = SUCCESS;

    BasicP2PConnection   * connection = aConnection;

    if (0 != strlen(connection->PeerAddress))
    {
        if (strcmp(connection->PeerAddress, aAcceptedAddress))
        {
            DEBUG_MSG(MSG_INFO, ("Ignoring connection from unexpected peer, name=%s, addr=%s\n",
                             aAcceptedName,
                             aAcceptedAddress));

            returnValue = FAILURE;
        }
    }

    return (returnValue);
}

/// \brief Blocks waiting from a remote connection on a bound socket
///
/// Once a valid connection is received, launches a reader thread 
/// for processing incoming messages.
static int
AcceptConnectionSocket(
    BasicP2PConnection   * aConnection)
{
    int returnValue = FAILURE;

    BasicP2PConnection   * connection = aConnection;

    DEBUG_MSG(MSG_DEBUG, ("Waiting for connection request...\n"));

    while (1)
    {
        char acceptedAddress[32];

        // This will block until a peer initiates a connection
        connection->ConnectionSocket = InetWrap_AcceptTCPConnection(
            connection->MasterSocket, 
            NULL, 
            acceptedAddress,
            0);

        if (INVALID_SOCKET_HANDLE != connection->ConnectionSocket)
        {
            DEBUG_MSG(MSG_DEBUG+3, ("Successfully called accept API\n"));

            /* ignore return */ InetWrap_SetSocketTTL(connection->ConnectionSocket, 3);
            // Verify that this is the client we wanted
            returnValue = ValidatePeer(connection,
                                       "",
                                       acceptedAddress);

            if (IS_SUCCESS(returnValue))
            {
                if (connection->ReaderThread)
                {
                    connection->ReaderThreadTerminated = OsWrap_CreateEvent(TRUE, FALSE);

                    if (connection->ReaderThreadTerminated)
                    {
                        // create reader thread for processing incoming messages
                        returnValue = OsWrap_NewThread(ReaderThread,
                                                       connection,
                                                       0,
                                                       &connection->ReaderThreadID,
                                                       &connection->ReaderThreadHandle);
                    }
                }

                break;
            }
            else
            {
                InetWrap_CloseSocket((void *) connection->ConnectionSocket);
            }
        }
        else
        {
            break;
        }
    }

    return (returnValue);
}

/// \brief Opens an Listening connection to wait for remote connections
///
/// Opens a TCP socket and binds to a port.  If mode is BlockingListener,
/// this function will block until a connection is received or socket is
/// closed or an error occurs.
static int
OpenListenerConnection(
    BasicP2PConnection   * aConnection)
{
    int returnValue = FAILURE;

    BasicP2PConnection   * connection = aConnection;

    returnValue = OpenMasterSocket(connection);

    if (IS_SUCCESS(returnValue))
    {
        connection->ListenerThreadTerminated = OsWrap_CreateEvent(TRUE, FALSE);

        if (connection->ListenerThreadTerminated)
        {
            // create listener thread for processing incoming connection
            returnValue = OsWrap_NewThread(ListenerThread,
                                           connection,
                                           0,
                                           &connection->ListenerThreadID,
                                           &connection->ListenerThreadHandle);

            if (IS_SUCCESS(returnValue) &&
                BlockingListener == connection->ConnectionMode)
            {
                int sysRet;
                int timeOut = OSWRAP_WAIT_INFINITE;

                if(connection->ListenerTimeout)
                {
                    timeOut = connection->ListenerTimeout;
                }

                sysRet = OsWrap_WaitForEvent(
                    connection->ListenerThreadTerminated,
                    timeOut);

                if (OSWRAP_WAIT_SIGNALED == sysRet)
                {
                    returnValue = connection->ListenerThreadReturnValue;
                }
                else
                {
                    returnValue = FAILURE;
                }
            }
        }
        else
        {
            returnValue = FAILURE;
        }
    }
    else
    {
        DEBUG_MSG(MSG_ERR, ("Error creating master socket for listening\n"));
    }

    return (returnValue);
}

/// \brief Closes a connection to a remote device.
///
/// This function waits for the separate reader thread
/// to gracefull terminate before closing the TCP socket.
/// If it fails to do so, the socket is closed which should
/// trigger the end of the reader thread incase it was block
/// waiting for data from the socket.  The reader thread is
/// supposed to avoid being blocked on "recv" so this doesn't
/// happen but if it is blocked, it will need to gracefully
/// handle the "socket closed" error return from "recv".
static void
CloseConnection(
    BasicP2PConnection   * aConnection)
{
    BasicP2PConnection   * connection = aConnection;

    connection->TerminateReaderThread = TRUE;

    // Give reader thread 1 second to terminate on its own
    // if it doesn't, closing the tcp connection should
    // make it go away so no need to forcibly terminate the thread
    OsWrap_WaitForEvent(connection->ReaderThreadTerminated, 1000);

    if (INVALID_SOCKET_HANDLE != connection->ConnectionSocket)
    {
        InetWrap_CloseSocket((void *) connection->ConnectionSocket);
        connection->ConnectionSocket = INVALID_SOCKET_HANDLE;
    }
}

/// \brief Closes an Initiator connection
///
static void
CloseInitiaterConnection(
    BasicP2PConnection   * aConnection)
{
    BasicP2PConnection   * connection = aConnection;

    CloseConnection(connection);
}

/// \brief Closes an BlockingListener connection
///
static void
CloseBlockingListenerConnection(
    BasicP2PConnection   * aConnection)
{
    BasicP2PConnection   * connection = aConnection;

    CloseConnection(connection);

    if (INVALID_SOCKET_HANDLE != connection->MasterSocket)
    {
        InetWrap_CloseSocket((void *) connection->MasterSocket);
        connection->MasterSocket = INVALID_SOCKET_HANDLE;
    }
}

/// \brief Closes an NonBlockingListener connection
///
static void
CloseNonBlockingListenerConnection(
    BasicP2PConnection   * aConnection)
{
    BasicP2PConnection   * connection = aConnection;

    if (INVALID_SOCKET_HANDLE != connection->ConnectionSocket)
    {
        // We are really closing one of the p2p connections
        // so don't close the master socket.
        CloseConnection(connection);
    }
    else if (INVALID_SOCKET_HANDLE != connection->MasterSocket)
    {
        // Closing the master socket will cause the Accept routine
        // to return with an error.  This gives the listener thread
        // a chance to terminate so there is no need for any other
        // synchronization with that thread.
        InetWrap_CloseSocket((void *) connection->MasterSocket);
        connection->MasterSocket = INVALID_SOCKET_HANDLE;
    }
}

/// \brief Release allocated resources for this connection
///
static int
DestroyConnection(
    BasicP2PConnection             ** aConnection)
{
    int returnValue = SUCCESS;

    BasicP2PConnection   * connection = *aConnection;

    if (connection)
    {
        if (connection->LocalName)
        {
            free(connection->LocalName);
        }

        if (connection->PeerName)
        {
            free(connection->PeerName);
        }

        if (connection->ReaderThreadTerminated)
        {
            OsWrap_CloseEvent(connection->ReaderThreadTerminated);
        }
        if (connection->ListenerThreadTerminated)
        {
            OsWrap_CloseEvent(connection->ListenerThreadTerminated);
        }

        free(connection);
    }

    *aConnection = NULL;

    return (returnValue);
}

/// \brief Initialize connection structure and allocate resources
///
static int
CreateConnection(
    ConnectionModes                 aConnectionMode,
    char                          * aLocalName,
    char                          * aPeerName,
    int                             aPortNumber,
    ThreadFunc_Ptr                  aReaderThread,
    ThreadFunc_Ptr                  aConnectionThread,
    int                             aListenerTimeout,
    void                          * aUserData,
    BasicP2PConnection           ** aConnection)
{
    int returnValue = FAILURE;

    BasicP2PConnection   * connection;

    connection = malloc(sizeof(*connection));

    if (connection)
    {
        memset(connection, 0, sizeof(*connection));

        returnValue = SUCCESS;

        if (aLocalName)
        {
            if (InetWrap_IsDottedDecimal(aLocalName))
            {
                strcpy(connection->LocalAddress, aLocalName);
            }
            else
            {
                // Allocate extra memory for these strings incase the name passed in
                // is a short hand name and the GetINetInfo routine returns a more
                // complete name that is longer.
                connection->LocalName = malloc(strlen(aLocalName) + 512);
                
                returnValue = FAILURE;
                if (connection->LocalName)
                {
                    strcpy(connection->LocalName, aLocalName);
                    returnValue = InetWrap_GetINetInfo(connection->LocalName, 
                                                    connection->LocalAddress,
                                                    strlen(connection->LocalName) + 512);
                }
            }
        }

        if (aPeerName && IS_SUCCESS(returnValue))
        {
            if (InetWrap_IsDottedDecimal(aPeerName))
            {
                strcpy(connection->PeerAddress, aPeerName);
            }
            else
            {
                // Allocate extra memory for these strings incase the name passed in
                // is a short hand name and the GetINetInfo routine returns a more
                // complete name that is longer.
                connection->PeerName = malloc(strlen(aPeerName) + 512);
                
                returnValue = FAILURE;
                if (connection->PeerName)
                {
                    strcpy(connection->PeerName, aPeerName);
                    returnValue = InetWrap_GetINetInfo(connection->PeerName, 
                                                    connection->PeerAddress,
                                                    strlen(connection->PeerName) + 512);
                }
            }
        }

        if (IS_SUCCESS(returnValue))
        {
            connection->PortNumber                = aPortNumber;
            connection->ConnectionMode            = aConnectionMode;
            connection->ReaderThread              = (void *) aReaderThread;
            connection->ConnectionThread          = (void *) aConnectionThread;
            connection->ListenerTimeout           = aListenerTimeout;
            connection->UserData                  = aUserData;

            connection->MasterSocket              = INVALID_SOCKET_HANDLE;
            connection->ConnectionSocket          = INVALID_SOCKET_HANDLE;
            connection->ReaderThreadID            = NULL;
            connection->ReaderThreadHandle        = NULL;
            connection->TerminateReaderThread     = 0;
            connection->ReaderThreadReturnValue   = FAILURE;
            connection->ReaderThreadTerminated    = NULL;
            connection->ListenerThreadID          = NULL;
            connection->ListenerThreadHandle      = NULL;
            connection->TerminateListenerThread   = 0;
            connection->ListenerThreadReturnValue = FAILURE;
            connection->ListenerThreadTerminated  = NULL;
            connection->ConnectionThreadID        = NULL;
            connection->ConnectionThreadHandle    = NULL;
        }
    }

    if (IS_FAILURE(returnValue))
    {
        DestroyConnection(&connection);
    }
    
    *aConnection = connection;

    return (returnValue);
}

/// \brief Exported interface for closing a connection
///
int
BasicP2P_CloseConnection(
    BasicP2PConnection   ** aConnection)
{
    int returnValue = SUCCESS;

    BasicP2PConnection   * connection = *aConnection;

    if (connection)
    {
        switch (connection->ConnectionMode)
        {
        case Initiater:
            DEBUG_MSG(MSG_DEBUG+3, ("Closing I Connection: 0x%08x\n", connection));
            CloseInitiaterConnection(connection);
            break;
        case BlockingListener:
            DEBUG_MSG(MSG_DEBUG+3, ("Closing B Connection: 0x%08x\n", connection));
            CloseBlockingListenerConnection(connection);
            break;
        case NonBlockingListener:
            DEBUG_MSG(MSG_DEBUG+3, ("Closing N Connection: 0x%08x\n", connection));
            CloseNonBlockingListenerConnection(connection);
            break;
        default:
            returnValue = FAILURE;
            break;
        }

        DestroyConnection(aConnection);
    }
    else
    {
        returnValue = FAILURE;
    }

    return (returnValue);
}

/// \brief Exported interface for opening a P2P connection
///
/// aLocalName is only meaningful for a Listener and is optional.
/// If NULL, will use INADDR_ANY.
/// aPeerName is required for Initiator and optional for a Listener.
/// If Listener and NULL, will accept connections from any remote device.
/// If Listener and aPeerName is non-NULL, will only accept connections
/// from the identified device.
/// If aPortNumber is zero, will allocate the first available
/// port in the dynamic port range of 49152 to 65535.  If non-zero
/// will attempt to bind to that port and return an error if unavailable.
/// aReaderThread must be provided.
/// aConnectionThread is only meaningful for a NonBlockingListener.
/// aListenerTimeout is only meaningful for a BlockingListener and is optional.
/// It is measured in milliseconds.  If zero, will wait indefinitely for a connection
/// and your thread will block.  If non-zero, will wait that number of milliseconds
/// and return an error if no connection is received during that time period.
/// aUserData is provided to allow users to associate additional information with
/// the connection.  The value passed in is placed in the UserData field in the
/// connection structure and is otherwise not used internally.  It is the responsibility
/// of the user to free any resources associated with this data field.
int
BasicP2P_OpenConnection(
    ConnectionModes                 aConnectionMode,
    char                          * aLocalName,
    char                          * aPeerName,
    int                             aPortNumber,
    ThreadFunc_Ptr                  aReaderThread,
    ThreadFunc_Ptr                  aConnectionThread,
    int                             aListenerTimeout,
    void                          * aUserData,
    BasicP2PConnection           ** aConnection)
{
    int returnValue = FAILURE;

    BasicP2PConnection   * connection = NULL;

    if (NULL == aConnection)
    {
        return (returnValue);
    }

    returnValue = InetWrap_OpenWindowsSockets();
    if (IS_FAILURE(returnValue))
    {
        return (returnValue);
    }

    returnValue = CreateConnection(aConnectionMode,
                                   aLocalName,
                                   aPeerName,
                                   aPortNumber,
                                   aReaderThread,
                                   aConnectionThread,
                                   aListenerTimeout,
                                   aUserData,
                                   &connection);

    if (IS_SUCCESS(returnValue))
    {
        switch (connection->ConnectionMode)
        {
            case Initiater:
                DEBUG_MSG(MSG_DEBUG+3, ("Opening I Connection: 0x%08x\n", connection));
                returnValue = OpenInitiaterConnection(connection);
                break;
            case BlockingListener:
            case NonBlockingListener:
                DEBUG_MSG(MSG_DEBUG+3, ("Opening %c Connection: 0x%08x\n", 
                                 (BlockingListener == connection->ConnectionMode) ? 'B' : 'N',
                                 connection));
                returnValue = OpenListenerConnection(connection);
                break;
            default:
                returnValue = FAILURE;
                break;
        }
    
        if (IS_FAILURE(returnValue))
        {
            BasicP2P_CloseConnection(&connection);
        }
    }

    *aConnection = connection;

    return (returnValue);
}

/*  static int */
/*  ReaderThread(void * aParameter) */
/*  { */
/*      int returnValue = SUCCESS; */

/*      BasicP2PConnection   * connection = (BasicP2PConnection *) aParameter; */

/*      while (IS_SUCCESS(returnValue) && !connection->TerminateReaderThread) */
/*      { */
/*          int bytesAvailable; */

/*          if (0 == ioctlsocket(connection->ConnectionSocket,  */
/*                               FIONREAD,  */
/*                               &bytesAvailable)) */
/*          { */
/*              DEBUG_MSG(3, ("bytesAvailable = %d\n", bytesAvailable)); */

/*              if (bytesAvailable) */
/*              { */
/*                  int    bytesRead; */
/*                  char * dataBuffer = malloc(bytesAvailable); */

/*                  bytesRead = recv(connection->ConnectionSocket,  */
/*                                   dataBuffer,  */
/*                                   bytesAvailable,  */
/*                                   0); */

/*                  //TBD do we just blindly deliver data to a callback */
/*                  //    function regardless of any message breaks? */
/*              } */

/*              if (!connection->TerminateReaderThread) */
/*              { */
/*                  OsWrap_Sleep(1); */
/*              } */
/*          } */
/*          else */
/*          { */
/*              returnValue = FAILURE; */
/*              break; */
/*          } */
/*      } */

/*      DEBUG_MSG(2, ("Done with reader thread while loop\n")); */

/*      if (NonBlockingListener == connection->ConnectionMode) */
/*      { */
/*          TcpP2P_CloseConnection(connection); */
/*      } */
/*      else */
/*      { */
/*          connection->ReaderThreadReturnValue = returnValue; */

/*          OsWrap_SetEvent(connection->ReaderThreadTerminated); */
/*      } */

/*      return 0; */
/*  } */

/// \brief Internal function used to launch the reader thread provided by user.
///
static int
ReaderThread(void * aParameter)
{
    int returnValue;

    BasicP2PConnection   * connection = (BasicP2PConnection *) aParameter;

    ThreadFunc_Ptr readerThread = (ThreadFunc_Ptr) connection->ReaderThread;

    DEBUG_MSG(MSG_DEBUG+3, ("ReaderThread: Top\n"));

    returnValue = readerThread(connection);

    DEBUG_MSG(MSG_DEBUG+3, ("ReaderThread: Bottom\n"));

    return (returnValue);
}

/// \brief Internal function used to launch the connection thread provided by user.
///
static int
ConnectionThread(void * aParameter)
{
    int returnValue;

    BasicP2PConnection   * connection = (BasicP2PConnection *) aParameter;

    ThreadFunc_Ptr connectionThread = (ThreadFunc_Ptr) connection->ConnectionThread;

    returnValue = connectionThread(connection);

    return (returnValue);
}

/// \brief Internal function used to launch a thread that will listen for remote connections.
///
/// If mode is BlockingListener, will wait for one connection and then terminate.
/// If mode is NonBlockingListener will loop indefinitely until an error occurs.
/// An error return from Accept is the only viable way to terminate this thread
/// when closing the master connection.  When a connection is received, launches
/// a new thread using the function provided by the user to keep the connection active until
/// it is ready to be closed.  This thread will be terminated when the user function returns.
static int
ListenerThread(void * aParameter)
{
    BasicP2PConnection   * masterConnection = (BasicP2PConnection *) aParameter;

    DEBUG_MSG(MSG_DEBUG+3, ("ListenerThread: Top\n"));

    if (BlockingListener == masterConnection->ConnectionMode)
    {
        masterConnection->ListenerThreadReturnValue = 
            AcceptConnectionSocket(masterConnection);
    }
    else
    {
        int returnValue = SUCCESS;

        while (IS_SUCCESS(returnValue))
        {
            BasicP2PConnection   * newConnection;

            returnValue = CreateConnection(NonBlockingListener,
                                           masterConnection->LocalName,
                                           masterConnection->PeerName,
                                           masterConnection->PortNumber,
                                           masterConnection->ReaderThread,
                                           masterConnection->ConnectionThread,
                                           0,
                                           masterConnection->UserData,
                                           &newConnection);

            if (IS_SUCCESS(returnValue))
            {
                newConnection->MasterSocket = masterConnection->MasterSocket;

                returnValue = AcceptConnectionSocket(newConnection);

                if (IS_SUCCESS(returnValue))
                {
                    if (newConnection->ConnectionThread)
                    {
                        returnValue = OsWrap_NewThread(ConnectionThread,
                                                       newConnection,
                                                       0,
                                                       &newConnection->ConnectionThreadID,
                                                       &newConnection->ConnectionThreadHandle);
                    }
                }
            }
        }
    }

    OsWrap_SetEvent(masterConnection->ListenerThreadTerminated);

    DEBUG_MSG(MSG_DEBUG+3, ("ListenerThread: Bottom\n"));

    return 0;
}

