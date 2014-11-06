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
/// \brief Definition file for DTCP AKE interface.
///
/// This header file defines the interface between the DTCP component and the
/// transport component to use for marshalling an AKE between two remote
/// devices.  It currently only supports DTCP over IP.  To support 1394 or
/// USB, the "Listen" and "StartSink" functions will need to be duplicated
/// with bus specific parameters.
/// Usage model for a server is as follows: Open a listen handle and provide a
/// a function that will marshal AKE for new connections as they are received
/// from clients.  The caller must provide the IP address and the port number
/// will be allocated from the dynamic port range as defined by IANA.
/// Usage model for a client is as follows: Open a handle and then use the
/// send/recv functions to marshal an AKE with the remote device.  
/// Usage model for sending messages: A set of functions is provided that
/// determine how to package the bits into the bus specific format.  
/// To send a new message, allocate sufficient memory for the message header
/// and the actual message data that is required.  Then, call the "set" functions 
/// to set the various parameters of the message and then call "send" to 
/// actually transmit the message to the remote device.  To receive a message,
/// call "recv" and when the message comes in and the function returns, 
/// call the "get" functions to retrieve the various parameters of the message.
/// The memory for messages (both send and recv) must be allocated by the caller.

#ifndef __DTCP_AKE_TRANSPORT_INTERFACE_H__
#define __DTCP_AKE_TRANSPORT_INTERFACE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/// \defgroup AkeTransport AkeTransport
/// \brief Marshals an AKE between two remote devices.
///
/// Usage model for a server is as follows: Open a listen handle and provide a
/// a function that will marshal AKE for new connections as they are received
/// from clients.  The caller must provide the IP address and the port number
/// will be allocated from the dynamic port range as defined by IANA.
/// Usage model for a client is as follows: Open a handle and then use the
/// send/recv functions to marshal an AKE with the remote device.  
/// Usage model for sending messages: A set of functions is provided that
/// determine how to package the bits into the bus specific format.  
/// To send a new message, allocate sufficient memory for the message header
/// and the actual message data that is required.  Then, call the "set" functions 
/// to set the various parameters of the message and then call "send" to 
/// actually transmit the message to the remote device.  To receive a message,
/// call "recv" and when the message comes in and the function returns, 
/// call the "get" functions to retrieve the various parameters of the message.
/// The memory for messages (both send and recv) must be allocated by the caller.
/// @{

/// \brief Handle created by Transport component.
///
/// The API's defined in this interface use this handle to identify different
/// AKE sessions that have been established by the caller.  This handle type
/// is also used to identify Listening sessions that have been initiated by
/// a server device.  The CloseTransportHandle function will correctly close
/// a handle of either type.  A server does not need to explicitly close AKE
/// handles.  They are automatically closed when the SourceAKE function returns.
typedef void * DtcpTransportHandle;

/// \brief Identify the different types of commands that can be sent
typedef enum
{
    cmdStatus,                ///< Status Command
    cmdChallenge,             ///< Challenge Command
    cmdResponse,              ///< Response Command
    cmdExchangeKey,           ///< Exchange Key Command
    cmdSRM,                   ///< SRM Command
    cmdContentKey,            ///< Content Key Command
    cmdCancel,                ///< Cancel Command
    cmdInvalid                ///< Invalid Command.  Used to detect usage errors
} EnCommands;

/// The following functions provide a bus agnostic mechanism for setting and retrieving
/// message data.  These functions are implemented by the transport component and
/// the "Set" functions are used by the DTCP component to create a message and the "Get"
/// functions are used to parse a new message that has been retrieved via the "Recv" function.

typedef EnCommands (*DtcpAke_GetCommandType_Ptr)  (void * aMessageData                                      );
typedef int        (*DtcpAke_SetCommandType_Ptr)  (void * aMessageData, EnCommands aCommandType             );
typedef int        (*DtcpAke_GetCTypeResponse_Ptr)(void * aMessageData                                      );
typedef int        (*DtcpAke_SetCTypeResponse_Ptr)(void * aMessageData, int        aCTypeResponse           );
typedef int        (*DtcpAke_GetAkeProcedures_Ptr)(void * aMessageData                                      );
typedef int        (*DtcpAke_SetAkeProcedures_Ptr)(void * aMessageData, int        aAkeProcedures           );
typedef int        (*DtcpAke_GetExchangeKeys_Ptr) (void * aMessageData                                      );
typedef int        (*DtcpAke_SetExchangeKeys_Ptr) (void * aMessageData, int        aExchangeKeys            );
typedef int        (*DtcpAke_GetSubFuncDep_Ptr)   (void * aMessageData                                      );
typedef int        (*DtcpAke_SetSubFuncDep_Ptr)   (void * aMessageData, int        aSubFuncDep              );
typedef int        (*DtcpAke_GetAkeLabel_Ptr)     (void * aMessageData                                      );
typedef int        (*DtcpAke_SetAkeLabel_Ptr)     (void * aMessageData, int        aAkeLabel                );
typedef int        (*DtcpAke_GetStatus_Ptr)       (void * aMessageData                                      );
typedef int        (*DtcpAke_SetStatus_Ptr)       (void * aMessageData, int        aStatus                  );
typedef void *     (*DtcpAke_GetCommandData_Ptr)  (void * aMessageData, int      * aDataLength              );
typedef int        (*DtcpAke_SetCommandData_Ptr)  (void * aMessageData, void     * aData, int aDataLength   );

/// \brief Used to send a dtcp ake command to remote device.
///
/// Implemented by Transport component and called by the DTCP component
/// The message response from the remove device will be placed into the
/// aMessageData buffer and aMessageLength will be set to indicate how
/// many bytes have been received.  Make sure sufficient memory has
/// been allocated for the response message.
typedef int
(*DtcpAke_Send_Ptr)(
    DtcpTransportHandle    aTransportHandle,
    void                 * aMessageData,
    int                  * aMessageLength,
    int                    aMessageResponseTimeout);

/// \brief Used to wait for a dtcp ake command from remote device.
///
/// Implemented by Transport component and called by the DTCP component
/// aMessageTimeOut is measured in milliseconds.  This function will block
/// until a message is received or the timeout period elapses.  If the message
/// timesout, an error code will be returned.  If aMessageTimeOut is zero,
/// the function will block until a message is received.
typedef int 
(*DtcpAke_Recv_Ptr)(
    DtcpTransportHandle    aTransportHandle,
    void                 * aMessageData,
    int                  * aMessageLength,
    int                    aMessageTimeout);

/// \brief Structure to contain the messaging interface to the IP transport component.
///
/// The message buffer's passed to the Send/Recv functions are already in bus specific
/// format.  Use the various Get/Set functions in the DTCP component to read/write these
/// message buffers in a bus agnostic manner.
typedef struct _dtcp_ake_ip_messaging_interface_
{
    DtcpAke_Send_Ptr                    Send;                     ///< Send Function
    DtcpAke_Recv_Ptr                    Recv;                     ///< Receive Function
    DtcpAke_GetCommandType_Ptr          GetCommandType;           ///< Get command type Function
    DtcpAke_SetCommandType_Ptr          SetCommandType;           ///< Set command type Function
    DtcpAke_GetCTypeResponse_Ptr        GetCTypeResponse;         ///< Get ctype response Function
    DtcpAke_SetCTypeResponse_Ptr        SetCTypeResponse;         ///< Set ctype response Function
    DtcpAke_GetAkeProcedures_Ptr        GetAkeProcedures;         ///< Get ake procedures Function
    DtcpAke_SetAkeProcedures_Ptr        SetAkeProcedures;         ///< Set ake procedures Function
    DtcpAke_GetExchangeKeys_Ptr         GetExchangeKeys;          ///< Get exchange keys Function
    DtcpAke_SetExchangeKeys_Ptr         SetExchangeKeys;          ///< Set exchange keys Function
    DtcpAke_GetSubFuncDep_Ptr           GetSubFuncDep;            ///< Get sub function dependant Function
    DtcpAke_SetSubFuncDep_Ptr           SetSubFuncDep;            ///< Set sub function dependant Function
    DtcpAke_GetAkeLabel_Ptr             GetAkeLabel;              ///< Get ake label Function
    DtcpAke_SetAkeLabel_Ptr             SetAkeLabel;              ///< Set ake label Function
    DtcpAke_GetStatus_Ptr               GetStatus;                ///< Get status Function
    DtcpAke_SetStatus_Ptr               SetStatus;                ///< Set status Function
    DtcpAke_GetCommandData_Ptr          GetCommandData;           ///< Get command data Function
    DtcpAke_SetCommandData_Ptr          SetCommandData;           ///< Set command data Function
} DtcpAkeIpMessagingInterface;

/// \brief Thread function used by source devices to marshall an AKE
///
/// DTCP component implements this function.  When Transport component
/// receives a new connection on its listen thread, it spawns
/// a new thread and then calls this function.
/// aUserData will contain the same value that was passed into aUserData
/// on the Listen API.
typedef int 
(*DtcpAke_SourceAke_Ptr)(
    DtcpTransportHandle           aTransportHandle,
    void                        * aUserData,
    DtcpAkeIpMessagingInterface * aMessagingInterface);

/// \brief Used by source devices to start a listening thread
///
/// Implemented by Transport component.  
/// Specific to IP.  A 1394 Transport component must
/// implement this function with different parameters 
/// that pass a 1394 device identifier instead of
/// ipaddress and port number.  
/// Spawns a listening thread and then returns.
/// aIpAddress must be in dotted decimal notation.
/// aIpPortNumber will return the actual port number
/// that was bound to the socket that was opened
/// for listening.
typedef int 
(*DtcpAkeIp_Listen_Ptr)(
    DtcpAke_SourceAke_Ptr     aSourceAkeFunc,
    char                    * aIpAddress,
    void                    * aUserData,
    int                     * aIpPortNumber,
    DtcpTransportHandle     * aTransportHandle);

/// \brief Used by sink devices to start an AKE with a source device
///
/// Implemented by Transport component.  
/// Specific to IP.  A 1394 Transport component must
/// implement this function with different parameters 
/// that pass a 1394 device identifier instead of
/// ipaddress and port number.  
/// Opens a handle to a device.  The DTCP component should call this for
/// each sink AKE it needs to perform.
/// This function returns after successfully establishing a connection to
/// the specified remote device.  The DTCP component then marshalls the
/// AKE by calling aRecvFunc and aSendFunc.
/// aIpDeviceID supports both friendly name and dotted decimal notation.
typedef int 
(*DtcpAkeIp_StartSink_Ptr)(
    char                        * aIpAddress,
    int                           aIpPortNumber,
    DtcpTransportHandle         * aTransportHandle,
    DtcpAkeIpMessagingInterface * aMessagingInterface);

/// \brief Close handles returned by Listen and StartSink functions.
///
/// Implemented by Transport component.  
/// For handles opened by Listen function, all currently active
/// connections to remote computers are canceled before this function
/// returns.
typedef int 
(*DtcpAke_CloseTransportHandle_Ptr)(
    DtcpTransportHandle aTransportHandle);

/// \brief Start the transport component
///
/// Implemented by Transport component.  
/// Must be called before any other calls.  Will allocate any
/// internal resources that are required for usage.
/// aMessageHeaderSize will be set and any pointers being used in the
/// Send/Recv or any of the Get/Set functions must point to a memory
/// buffer of at least this size plus the command data field.  This mechanism 
/// should ease compatibility with transport components for other buses.
typedef int 
(*DtcpAke_Startup_Ptr)(
    int * aMessageHeaderSize);

/// \brief Shutdown the transport component
///
/// Implemented by Transport component.
/// Called to close any internally allocated resources
/// before unloading component.
typedef int 
(*DtcpAke_Shutdown_Ptr)();

/// Current interfaces versions
#define DTCP_AKE_IP_TRANSPORT_INTERFACE_VERSION          2

/// Structure to contain the interface to the IP transport component.
typedef struct _dtcp_ake_ip_transport_interface_
{
    int                                 StructureSize;                  ///< set to size of structure by entity needing structure to be completed
    int                                 InterfaceVersion;               ///< set by entity needing structure to be completed 
    DtcpAke_Startup_Ptr                 Startup;                        ///< Startup Function
    DtcpAke_Shutdown_Ptr                Shutdown;                       ///< Shutdown Function
    DtcpAke_CloseTransportHandle_Ptr    CloseTransportHandle;           ///< Close handle Function

    //NOTE: These functions will need to be changed to support other Buses
    DtcpAkeIp_Listen_Ptr                Listen;                        ///< Listen Function
    DtcpAkeIp_StartSink_Ptr             StartSink;                     ///< Start sink Function
} DtcpAkeIpInterface;

#ifdef WINDOWS_BUILD
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/// The IP Transport component must implement this interface.
/// The DTCP component will use this function to get the
/// main transport interface
DLLEXPORT int 
DtcpAkeIp_GetInterface(
    DtcpAkeIpInterface * aInterface);

/// typedef used for indirect function calls
typedef int 
(*DtcpAkeIp_GetInterface_Ptr)(
    DtcpAkeIpInterface * aInterface);

/// @}

#ifdef __cplusplus
}
#endif

#endif
