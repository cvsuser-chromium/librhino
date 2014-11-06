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
/// \brief Definition file for \ref DtcpApi library.
///
/// This header file contains the interface to the \ref DtcpApi library.  This API
/// is the interface used by an application to implement DTCP using the DTCP
/// libraries.

#ifndef __DTCP_API_H__
#define __DTCP_API_H__

#include "DtcpConstants.h"
#include "DtcpContentManagementTypes.h"
#include "DtcpCoreTypes.h"
#include "DtcpAkeCoreTypes.h"
#include "RngTypes.h"


/// \defgroup DtcpApi DtcpApi
/// \brief Library that provides a single interface to be used by an application for
/// implementing DTCP-IP
/// 
/// \image html DtcpApi_SequenceDiagram.gif "Source and Sink Sequence Diagram"
/// @{

/// \example SampleApp.c
/// This is an example of how to initialize the DtcpApi library.

/// \example SampleAppSource.c
/// This is an example of how to use the DtcpApi library as a source device.

/// \example SampleAppSink.c 
/// This is an example of how to use the DtcpApi library as a sink device.


/// \brief Handle to an AKE session
typedef void *DtcpAkeHandle;

/// \brief Initializes the DTCP with the appropriate parameters.
///
/// This function must be called before any of the other DtcpApi functions can be called
/// and it should only be called one time.
///
/// This function is called by both source devices and sink devices.
/// \param aDeviceParams Input - Pointer to a structure containing all of the parameters
///                        describing this device
/// \param aRngSeed Input - Pointer to a buffer containing a random number generator seed
/// \param aExchangeKeyUpdateRequestFunc Input - Pointer to a callback function this is implemented
///                                      by the application to process a request by the DTCP-IP SDK to
///                                      to update the exchange keys by calling DtcpAkeCore_UpdateExchangeKey.
/// \param aUpdateSrmFunc Input - Pointer to a callback function that is implemented by the application to store an
///                         updated SRM
/// \param aUpdateRngSeedFunc Input - Pointer to a callback function that is implemented by the application 
///                         to get store the seed for the random number generator
int DtcpApi_Startup(DeviceParams                 *aDeviceParams,
                    unsigned char                 aRngSeed[DTCP_AES_BLOCK_SIZE],
                    ExchangeKeyUpdateRequest_Ptr  aExchangeKeyUpdateRequestFunc,
                    UpdateSrm_Ptr                 aUpdateSrmFunc,
                    UpdateRngSeed_Ptr             aUpdateRngSeedFunc);

/// \brief Shuts down the DTCP libraries and releases any resources that are in use
///        by the DTCP libraries.
///
/// Do not call any of the other DtcpApi functions after calling DtcpApi_Shutdown 
/// without first calling DtcpApi_Startup again.
///
/// This function is called by both source devices and sink devices.
void DtcpApi_Shutdown(void);

/// \brief Opens a stream for encryption.
///
/// This function creates a DtcpStreamHandle that is required for DtcpApi_CreatePacketHeader.
/// For HTTP, this function should be called once in response to each HTTP GET received by
/// HTTP server.
///
/// This function is called by source devices.
/// \param aStreamTransport Input - Specifies the type of transport will be used for this stream.
/// \param aStreamHandle Output - Handle identifying a stream.
int DtcpApi_OpenStream(EnStreamTransport  aStreamTransport,
                       DtcpStreamHandle  *aStreamHandle);

/// \brief Closes a stream
///
/// This function releases any resources associated with a stream.  It should be called
/// once for each successful DtcpApi_OpenStream call.
///
/// This function is called by source devices.
/// \param aStreamHandle Input - Handle identifying a stream.
int DtcpApi_CloseStream(DtcpStreamHandle  aStreamHandle);



/// \brief Initializes a protected content packet header.
///
/// This function needs to be called each time a protected content packet is
/// created by a source device.
///
/// The overall length of the packet will be the length of the header,
/// specified by DTCP_CONTENT_PACKET_HEADER_SIZE, plus the length of the content,
/// specified by aContentSize, plus the padding bytes required.  Padding bytes
/// are required at the end of the protected content packet if the length of
/// the content is not a multiple of 16 bytes.  The number of padding bytes 
/// required = 16 - (aContentSize % 16).
///
/// This function is called by source devices.
/// \param  aStreamHandle Input - Handle to the current stream
/// \param  aExtendedEmi Input - EMI type for this protected content packet
/// \param  aContentSize Input - Size in bytes of the content for this protected
///                              content packet (does not include padding bytes)
/// \param  aPacketHeader Input - Pointer to a buffer at least the size of a
///                               protected content packet header.  Upon successful
///                               completion, this buffer will contain a properly
///                               populated protected content packet header ready for
///                               delivery to the sink device.
/// \param  aPacketHandle Output - Handle identifying packet
int DtcpApi_CreatePacketHeader(DtcpStreamHandle  aStreamHandle,
                               EnExtendedEmi     aExtendedEmi,
                               unsigned int      aContentSize,
                               unsigned char     aPacketHeader[DTCP_CONTENT_PACKET_HEADER_SIZE],
                               DtcpPacketHandle *aPacketHandle);

/// \brief Extracts information from a protected content packet header.
///
/// This function needs to be called for each protected content packet received
/// by a sink device.
///
/// The overall length of the packet will be the length of the header,
/// specified by DTCP_CONTENT_PACKET_HEADER_SIZE, plus the length of the content,
/// specified by aContentSize, plus the padding bytes required.  Padding bytes
/// are required at the end of the protected content packet if the length of
/// the content is not a multiple of 16 bytes.  The number of padding bytes 
/// required = 16 - (aContentSize % 16).
///
/// This function is called by sink devices.
/// \param  aAkeHandle Input - Handle to the Ake to which this protected content packet
///                            is associated.
/// \param  aPacketHeader Input - Pointer to a buffer containing a complete protected
///                               content packet header
/// \param  aExtendedEmi Output - EMI type for this protected content packet
/// \param  aContentSize Output - Size in bytes of the content for this protected content
///                               packet (does not include padding bytes)
/// \param  aPacketHandle Output - Handle identifying packet
int DtcpApi_ConsumePacketHeader(DtcpAkeHandle     aAkeHandle,
                                unsigned char     aPacketHeader[DTCP_CONTENT_PACKET_HEADER_SIZE],
                                EnExtendedEmi    *aExtendedEmi,
                                unsigned int     *aContentSize,
                                DtcpPacketHandle *aPacketHandle);

/// \brief Encrypts data for a packet. 
///
/// The size of the buffer being encrypted must be a multiple of 16 bytes.  If
/// encrypting the last buffer of the protected content packet and the number of
/// bytes remaining is not a multiple of 16 bytes, then padding bytes must be
/// added to the buffer.    The number of padding bytes 
/// required = 16 - (number_of_bytes_remaining % 16).
///
/// This function is called by source devices.
/// \param aPacketHandle Input - Handle identifying packet
/// \param aInBuffer Input - Pointer to buffer containing payload to encrypt
/// \param aOutBuffer Input - Pointer to buffer for encrypted payload
/// \param  aSize Input - Number of bytes to encrypt (must be multiple of 16 bytes - padded at 
///                       the end of the packet if necessary).
int DtcpApi_EncryptData(DtcpPacketHandle  aPacketHandle, 
                        unsigned char    *aInBuffer,
                        unsigned char    *aOutBuffer,
                        unsigned int      aSize);

/// \brief Decrypts data for a packet. This function is only called by a sink device.
///
/// The size of the buffer being decrypted must be a multiple of 16 bytes.  If
/// decrypting the last buffer of the protected content packet and the number of
/// bytes remaining is not a multiple of 16 bytes, then padding bytes must be
/// added to the buffer.    The number of padding bytes 
/// required = 16 - (number_of_bytes_remaining % 16).
///
/// This function is called by sink devices.
/// \param aPacketHandle Input - Handle identifying packet
/// \param aInBuffer Input - Pointer to buffer containing payload to encrypt
/// \param aOutBuffer Input - Pointer to buffer for decrypted payload
/// \param aSize Input - Number of bytes of encrypted content to decrypt 
///                (must be multiple of 16 bytes - padded at the end of the packet if 
///                 necessary).
int DtcpApi_DecryptData(DtcpPacketHandle  aPacketHandle, 
                        unsigned char    *aInBuffer,
                        unsigned char    *aOutBuffer,
                        unsigned int      aSize);

/// \brief Closes a protected content packet and releases any resources allocated
/// for the packet.
///
/// This function needs to be called once for each DtcpPacketHandle that is 
/// created in DtcpApi_CreatePacketHeader and DtcpApi_ConsumePacketHeader.
///
/// This function is called by both source devices and sink devices.
/// \param aPacketHandle Input - Handle to the protected content packet to close
int DtcpApi_ClosePacket(DtcpPacketHandle aPacketHandle);

/// \brief Begins listening for a sink device to try to establish an AKE.
///
/// This function should only be called one time for each IP address being
/// listened to for AKE messages.
/// 
/// This function is called by source devices.
/// \param aSourceAkeIpAddress Input - A string containing the IP address of the source in 
///                              dotted decimal notation (xxx.xxx.xxx.xxx)
/// \param aSourceAkePortNumber - Input/Output; Pointer to a value containing the port 
///        number to listen to or zero to select an unused port.  Upon successful
///        completion will contain the port number that this device is listening on.
/// \param aListenHandle Output - A handle associated with this IP address and port number.
int DtcpApi_ListenIp(char          *aSourceAkeIpAddress, 
                     unsigned int  *aSourceAkePortNumber,
                     DtcpAkeHandle *aListenHandle);

/// \brief Cancels listening for AKE requests.
///
/// This function is called by source devices.
/// \param aListenHandle Input - A handle to the IP address and port number to stop
///        listening on.
int DtcpApi_CancelListen(DtcpAkeHandle aListenHandle);

/// \brief Initializes an IP AKE session.
///
/// Before an AKE is initiated by a sink device, this function must be called.
///
/// This function is called by sink devices.
/// \param aSourceAkeIpAddr Input - A string containing the IP address of the source in 
///                              dotted decimal notation (xxx.xxx.xxx.xxx)
/// \param aSourceAkePortNumber Input - The port number that the source device
///                               is listening on for AKE connections.
/// \param aAkeHandle Output - A handle to this AKE session.
int DtcpApi_OpenAkeIp(char *aSourceAkeIpAddr, unsigned int aSourceAkePortNumber, DtcpAkeHandle *aAkeHandle);

/// \brief Establishes an AKE with the source device.
///
/// This function is called by sink devices.
/// \param aAkeHandle Input - A handle to the AKE session to establish.
int DtcpApi_DoAke(DtcpAkeHandle aAkeHandle);

/// \brief Closes an AKE.
///
/// This function is called by sink devices.
/// \param aAkeHandle Input - A handle to the AKE session to close.
int DtcpApi_CloseAke(DtcpAkeHandle aAkeHandle);

/// \brief Updates the exchange keys.
///
/// A source application is responsible for updating the exchange keys when appropriate.
/// The exchange keys should be updated after two hours of inactivity (two hours of not
/// streaming any content).  The exchange keys can also be updated when the SDK calls
/// the callback function UpdateExchangeKeyRequest if the application is not actively 
/// streaming content.  That callback function will be called when the sink count limit
/// has been reached and an additional sink device tries to authenticate.  
///
/// This function is called by source devices.
/// \param aExchangeKeyLabel Output - The updated exchange key label.
int DtcpApi_UpdateExchangeKey(unsigned int *aExchangeKeyLabel);

/// \brief Checks whether an exchange key label is still valid
///
/// If a source application invalidates its exchange keys, then the sink devices that
/// have previously completed an AKE will no longer be authenticated.  This function
/// is a way for a sink device to determine whether or not it needs to reauthenticate
/// with the source device.  If the exchange key label is not valid then it must 
/// reauthenticated to be able to decrypt content received from the source.
/// \param aAkeHandle Input - A handle to the AKE session with the source device in question
/// \param aExchangeKeyLabel Input - The value of the source device's exchange key label or
///                                  (-1) if the sink device should issue a CONTENT_KEY_REQUEST
///                                  subfunction to determine that exchange key label
/// \param aExchangeKeyValidFlag Output - An output flag indicating whether or not the sink
///                                       still has the valid set of exchange keys.  (1 == 
///                                       Valid, 0 == Not Valid)
int DtcpApi_ValidateExchangeKey(DtcpAkeHandle  aAkeHandle,
                                int            aExchangeKeyLabel,
                                int           *aExchangeKeyValidFlag);

/// \brief Gets the exchange key label of the application's source exchange keys.
///
/// This function is called by source devices to get its current exchange key label.
/// The exchange key label could then be sent to a sink device so that the sink device
/// could determine if it is still authenticated with the source device.
/// \param aExchangeKeyLabel Output - The current value of the source exchange key label.
int DtcpApi_GetSourceExchangeKeyLabel(unsigned int *aExchangeKeyLabel);

// Not sure if this function is really a valid usage, need to check specification.
int DtcpApi_CheckSourceSinkLimit(char *aSourceAkeIpAddr, unsigned int aSourceAkePortNumber);

/// @}

#endif // __DTCP_API_H__
