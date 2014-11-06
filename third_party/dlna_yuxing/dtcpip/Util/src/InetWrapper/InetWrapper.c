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
/// Implementation file for InetWrapper.



#include "InetWrapper.h"

#include "StatusCodes.h"

#ifdef WINDOWS_BUILD

#include <winsock2.h>
#include <ws2tcpip.h> // needed for IP_TTL
#include <windows.h>

#else

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <signal.h>

#include <semaphore.h>

typedef unsigned long SOCKET;

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

#endif

#include <stdio.h>
#include <time.h>
#include <memory.h>
#include <string.h>

///////////////////////////////////////////////////////////////
//// Macros and typedefs
///////////////////////////////////////////////////////////////

/// \brief defines a non-zero value for "true"
#define TRUE   1

/// \brief defines a zero value for "false"
#define FALSE  0

#define BUF_SIZE 4096
#define QUEUE_SIZE 10


#define IANA_START 49152
#define IANA_END   65535

///////////////////////////////////////////////////////////////
//// Global Variables
///////////////////////////////////////////////////////////////

#if 0
static int GlobalDisplayLevel             = -1;
static int GlobalDisplayLevelMatchExactly = 0;
static int GlobalLogToFile                = 0;
#endif

static int
WindowsSocketsInitialized = FALSE;


/// \brief Must be called before using socket API's on windows.  
///
/// Does nothing on Linux
int
InetWrap_OpenWindowsSockets()
{
    int returnValue = FAILURE;

#ifdef WINDOWS_BUILD
    if (WindowsSocketsInitialized)
    {
        returnValue = SUCCESS;
    }
    else
    {
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;
     
        wVersionRequested = MAKEWORD( 2, 2 );
     
        err = WSAStartup( wVersionRequested, &wsaData );
        if (0 == err) 
        {
            // Confirm that the WinSock DLL supports 2.2.
            // Note that if the DLL supports versions greater
            // than 2.2 in addition to 2.2, it will still return
            // 2.2 in wVersion since that is the version we
            // requested.
     
            if ( LOBYTE( wsaData.wVersion ) != 2 ||
                HIBYTE( wsaData.wVersion ) != 2 ) 
            {
                /* Tell the user that we could not find a usable */
                /* WinSock DLL.                                  */
                WSACleanup();
            }
            else
            {
                WindowsSocketsInitialized = TRUE;
                returnValue = SUCCESS;
            }
        } 
    }
#else
    // Don't need to do anything on Linux
    returnValue = SUCCESS;
#endif

    return (returnValue);
}

/// \brief Cleans up internal resources used by Windows socket API's. 
///
/// Does nothing on Linux
int
InetWrap_CloseWindowsSockets()
{
    int returnValue = SUCCESS;

#ifdef WINDOWS_BUILD
    WSACleanup();
    WindowsSocketsInitialized = FALSE;
#endif

    return (returnValue);
}

/// \brief OS agnostic API for closing a socket handle
int
InetWrap_CloseSocket(SOCKET_HANDLE aSocket)
{
#ifdef WINDOWS_BUILD
    return (closesocket((SOCKET) aSocket));
#else
    return (close((SOCKET) aSocket));
#endif
}

/// \brief OS agnostic API for opening a TCP socket handle
///
/// Assumes AF_INET, SOCK_STREAM, and IPPROTO_TCP.  If other
/// values are needed, will need to add parameters to this function.
/// \todo Need a OpenUDPSocket API if we ever need to do UDP.
SOCKET_HANDLE
InetWrap_OpenTCPSocket()
{
    SOCKET_HANDLE sh;
    if (IS_FAILURE(InetWrap_OpenWindowsSockets()))
    {
        return (0);
    }

    sh = ((SOCKET_HANDLE) socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    {
        /* ignore return */ InetWrap_SetSocketTTL(sh, 3);        
    }

    return sh;
}

/// \brief Blocks waiting for a remote connection on a bound socket
///
/// Used by servers to wait for a connection from a remote computer.
/// This function will block until a connection is received or the
/// bound socket is closed or an error occurs.
SOCKET_HANDLE
InetWrap_AcceptTCPConnection(
    SOCKET_HANDLE   aSocket,
    char          * aAcceptedName,
    char          * aAcceptedAddress,
    int             aAcceptedNameLength)
{
    SOCKET        socket           = (SOCKET) aSocket;
    SOCKET        internalSocket;
    SOCKET_HANDLE newSocket        = INVALID_SOCKET_HANDLE;

    struct sockaddr_in peerID;
    int                peerIDSize = sizeof(peerID);

    // This will block until a peer initiates a connection
    internalSocket = accept(socket, 
                            (struct sockaddr *) &peerID, 
                            &peerIDSize);

    if (INVALID_SOCKET != internalSocket)
    {
        struct in_addr   addr;
        struct hostent * peer;

#ifdef WINDOWS_BUILD
        unsigned long * iAddr;
        iAddr = &addr.S_un.S_addr;
#else
        unsigned int * iAddr;
        iAddr = &addr.s_addr;
#endif

        *iAddr = peerID.sin_addr.s_addr;

        strcpy(aAcceptedAddress, inet_ntoa(addr));

        if (aAcceptedName)
        {
            peer = gethostbyaddr((char *) &peerID.sin_addr.s_addr,
                                 sizeof(peerID.sin_addr.s_addr),
                                 AF_INET);
            if (peer)
            {
                strncpy(aAcceptedName, 
                        peer->h_name, 
                        aAcceptedNameLength);

            }
            else
            {
                strcpy(aAcceptedName, "");
            }
        }
        
        newSocket = (SOCKET_HANDLE) internalSocket;
    }

    return (newSocket);
}

/// \brief Initiate a connection to a remote computer
///
/// Used by clients to initiate a connection to a remote computer.
int
InetWrap_ConnectTCPSocket(
    SOCKET_HANDLE   aSocket,
    char          * aPeerName,
    int             aPortNumber)
{
    int returnValue = SUCCESS;

    SOCKET socket     = (SOCKET) aSocket;

    struct sockaddr_in channel;

    memset(&channel, 0, sizeof(channel));

    channel.sin_addr.s_addr = InetWrap_GetPeerAddress(aPeerName);
    channel.sin_family      = AF_INET;
    channel.sin_port        = htons((short) aPortNumber);

    if (SOCKET_ERROR == connect(socket,
                                (struct sockaddr *) &channel,
                                sizeof(channel)))
    {
        returnValue = FAILURE;
    }

    return (returnValue);
}

/// \brief Bind a socket to a specified port and ip address
///
/// If ip address is NULL, will use INADDR_ANY.  
/// If port is zero, will search for an available port in the
/// dynamic port range of 49152 to 65535.
/// \todo Verify that we really need to set SO_REUSEADDR for bound sockets.
/// \todo determine why the current implementation lets the same port
/// be bound more than once.
int
InetWrap_BindTCPSocket(
    SOCKET_HANDLE        aSocket,
    char               * aLocalName,
    int                * aPortNumber)
{
    int returnValue = FAILURE;

    SOCKET socket     = (SOCKET) aSocket;
    int    portNumber = *aPortNumber;

    struct sockaddr_in channel;

    // Build address structure to bind to socket.
    memset(&channel, 0, sizeof(channel));
    channel.sin_family      = AF_INET;
    channel.sin_addr.s_addr = InetWrap_GetPeerAddress(aLocalName);

    if (portNumber)
    {
        channel.sin_port = htons((short) portNumber);

        DEBUG_MSG(MSG_DEBUG+3, ("Attempting to bind requested Port %d\n", portNumber));

        if (SOCKET_ERROR != bind(socket, 
                                 (struct sockaddr *) &channel, 
                                 sizeof(channel))
            )
        {
            returnValue = SUCCESS;
        }
        else
        {
            ERROR_MSG(1, returnValue, ("Can't bind to specified port %d, error %d!!\n", 
                                       portNumber,
                                       InetWrap_GetLastSocketError()));
        }
    }
    else
    {
        int index;

        // IANA dynamic/private port numbers range from 49152 to 65535
        // http://www.iana.org/numbers.html#P
        // Don't start at the beginning or end since that is probably
        // where others will be looking for ports.

        index = ((IANA_END - IANA_START) / 2) + IANA_START;

        for (; index <= IANA_END; index++)
        {
            channel.sin_port = htons((short) index);

            DEBUG_MSG(MSG_DEBUG+3, ("Attempting to bind Port %d\n", index));

            if (SOCKET_ERROR != bind(socket, 
                                        (struct sockaddr *) &channel, 
                                        sizeof(channel))
                )
            {
                returnValue = SUCCESS;
                portNumber = index;
                break;
            }
        }

        if (IS_FAILURE(returnValue))
        {
            // nothing available in top half, so try bottom half of dynamic range.

            index = ((IANA_END - IANA_START) / 2) + IANA_START - 1;

            for (; index >= IANA_START; index--)
            {
                channel.sin_port = htons((short) index);

                DEBUG_MSG(MSG_DEBUG+3, ("Attempting to bind Port %d\n", index));

                if (SOCKET_ERROR != bind(socket, 
                                            (struct sockaddr *) &channel, 
                                            sizeof(channel))
                    )
                {
                    returnValue = SUCCESS;
                    portNumber = index;
                    break;
                }
            }
        }

        if (IS_FAILURE(returnValue))
        {
            ERROR_MSG(1, returnValue, ("Can't bind to any dynamic ports between 49152 and 65535!!\n"));
        }
    }

    if (IS_SUCCESS(returnValue))
    {
        // specify queue size.
        if (SOCKET_ERROR == listen(socket, QUEUE_SIZE))
        {
            returnValue = FAILURE;
            DEBUG_MSG(MSG_ERR, ("Error calling listen API\n"));
        }
    }

    *aPortNumber = portNumber;

    return (returnValue);
}

/// \brief Query TCP socket to see how many bytes are in read buffer
///
/// For applications that need to determine message boundaries, this
/// API can be used to ensure that a complete message is available
/// before calling InetWrap_TCPRecvData which may block.
int
InetWrap_TCPReadBufferSize(
    SOCKET_HANDLE     aSocket,
    int             * aBufferSize)
{
    int returnValue = SUCCESS;

    SOCKET socket   = (SOCKET) aSocket;

    if (aBufferSize)
    {
#ifdef WINDOWS_BUILD
        if (ioctlsocket(
#else
        if (ioctl(
#endif
                socket, 
                FIONREAD, 
                aBufferSize))
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

/// \brief Retrieve a specified number of bytes for a TCP socket
///
/// This function will block until the requested number of bytes are
/// available or until the socket is closed or an error occurs.  If
/// more bytes are available, only the requested number of bytes are
/// returned and the remaining bytes stay in the read buffer.
int
InetWrap_TCPRecvData(
    SOCKET_HANDLE     aSocket,
    char            * aBuffer,
    int             * aBufferSize)
{
    int returnValue = SUCCESS;

    SOCKET socket   = (SOCKET) aSocket;

    if (aBuffer && aBufferSize)
    {
        int    bytesRead;

        bytesRead = recv(socket, 
                         aBuffer, 
                         *aBufferSize, 
                         0);

        *aBufferSize = bytesRead;
    }
    else
    {
        returnValue = FAILURE;
    }

    return (returnValue);
}

/// \brief Writes a specified number of bytes into the send buffer of
/// a TCP socket
int
InetWrap_TCPSendData(
    SOCKET_HANDLE     aSocket,
    char            * aBuffer,
    int               aBufferSize)
{
    int returnValue = SUCCESS;

    SOCKET socket   = (SOCKET) aSocket;

    if (aBuffer)
    {
        int sockRet;

        sockRet = send(socket, 
                       aBuffer, 
                       aBufferSize, 
                       0);

        if (SOCKET_ERROR == sockRet)
        {
            int error = InetWrap_GetLastSocketError();

            returnValue = FAILURE;

            ERROR_MSG(1, returnValue, ("%d error sending message!!\n", 
                                          error));
        }
    }
    else
    {
        returnValue = FAILURE;
    }

    return (returnValue);
}

/// \brief Query the error code for the last socket API that failed.
///
/// Will return whatever error code the underlaying OS provides.  No
/// mapping to a generic set of error codes is performed.
int
InetWrap_GetLastSocketError()
{
#ifdef WINDOWS_BUILD
    return (WSAGetLastError());
#else
    return (errno);
#endif
}

/// \brief Query for friendly name and dotted decimal IP address
///
/// aName is both an input and output parameter.  It can be set to
/// either a friendly name or a dotted decimal IP address.
/// Upon return, aName will contain the friendly name and aAddress
/// will contain the dotted decimal address.  If an error occurs,
/// aName is left unchanged and aAddress is a NULL string.
/// \todo Fix this function to not rely on eth0 for Linux.
int
InetWrap_GetINetInfo(char * aName,
                   char * aAddress,
                   int    aNameLength)
{
    int returnValue = SUCCESS;

    struct in_addr   addr;
    struct hostent * peer;
    unsigned long    hostBinaryAddress;

#ifdef WINDOWS_BUILD
    unsigned long * iAddr;
    iAddr = &addr.S_un.S_addr;
#else
    unsigned int * iAddr;
    iAddr = &addr.s_addr;
#endif

    if (NULL == aName || NULL == aAddress)
    {
        return (FAILURE);
    }

    if (IS_FAILURE(InetWrap_OpenWindowsSockets()))
    {
        return (FAILURE);
    }

    if (0 == strlen(aName))
    {
        gethostname(aName, aNameLength);
    }

    // If this call fails, then address must be a name instead of dotted decimal
    hostBinaryAddress = inet_addr(aName);

    if (-1 == hostBinaryAddress)
    {
        // Call failed, assume we have a DNS name
        peer = gethostbyname(aName);
    }
    else
    {
        // Call succeeded, assume we have a dotted decimal address
        peer = gethostbyaddr((const char *) &hostBinaryAddress, 
                             sizeof(hostBinaryAddress), 
                             AF_INET);
    }

    *iAddr = 0;
    if (peer)
    {
        strncpy(aName, peer->h_name, aNameLength);
        *iAddr = *((int *) peer->h_addr);
        strcpy(aAddress, inet_ntoa(addr));
    }
    else
    {
#ifdef WINDOWS_BUILD
        *aAddress = '\0';
        returnValue = FAILURE;
#else
        struct ifreq         ifr;
	    struct sockaddr_in * sin;
        int                  skt;

        skt = socket(AF_INET, SOCK_DGRAM, 0);

        /* ignore return */ InetWrap_SetSocketTTL(skt, 3);        

        if (skt)
        {
            strncpy(ifr.ifr_name, "eth0", sizeof (ifr.ifr_name));
            if (ioctl(skt, SIOCGIFADDR, (caddr_t)&ifr) < 0) 
            {
                returnValue = FAILURE;
                ERROR_MSG(1, returnValue, ("SIOCGIFADDR failed, s=0x%08x\n", skt));
            }
            else
            {
                sin = (struct sockaddr_in *)&ifr.ifr_addr;
                strcpy(aAddress, inet_ntoa(sin->sin_addr));
            }
            close(skt);
        }
        else
        {
            returnValue = FAILURE;
        }
#endif
    }

    return (returnValue);
}

/// \brief Convert a friendly name or dotted decimal into a binary value
///
/// If aPeerName is null, the return is INADDR_ANY.  Otherwise it converts
/// the name to a binary value that is in network byte order.
unsigned long
InetWrap_GetPeerAddress(
    char * aPeerName)
{
    unsigned long retVal = -1;

    if (IS_FAILURE(InetWrap_OpenWindowsSockets()))
    {
        return (0);
    }

    if (aPeerName)
    {
        // If this call fails, then address must be a name instead of
        // dotted decimal
        retVal = inet_addr(aPeerName);

        if (-1 == retVal)
        {
            struct hostent * peer;

            // Call failed, assume we have a DNS name
            peer = gethostbyname(aPeerName);

            if (peer)
            {
                if (sizeof(retVal) >= peer->h_length)
                {
                    memcpy(&retVal, 
                                   peer->h_addr, 
                                   peer->h_length);
                }
            }
        }
    }
    else
    {
        retVal = htonl(INADDR_ANY);
    }

    return (retVal);
}

/// \brief Return boolean indicator if incoming address is dotted decimal or not
///
/// If aAddress is a dotted decimal string, return value is TRUE, otherwise returns FALSE.
int
InetWrap_IsDottedDecimal(char * aAddress)
{
    int returnValue = TRUE;
    
    unsigned long binaryAddress = inet_addr(aAddress);

    if (-1 == binaryAddress)
    {
        // Call failed, assume we have a DNS name
        returnValue = FALSE;
    }
    
    return (returnValue);
}

int
InetWrap_SetSocketTTL(
    SOCKET_HANDLE        aSocket,
    int                  aTtl)
{
    int returnValue = TRUE;
    
    if (setsockopt((SOCKET) aSocket, IPPROTO_IP, IP_TTL, (char*)&aTtl, sizeof(aTtl)) < 0)
    {
        returnValue = FALSE;
    }

    return returnValue;
}



