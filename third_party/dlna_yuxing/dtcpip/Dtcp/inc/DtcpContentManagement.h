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

#ifndef __DTCP_CONTENT_MANAGEMENT_H__
#define __DTCP_CONTENT_MANAGEMENT_H__

/// \file
/// \brief Defines the interface to the \ref DtcpContent library.
///
/// This header file contains the interface to the \ref DtcpContent library.  This library
/// is used to encrypt and decrypt content.

#include "DtcpConstants.h"
#include "DtcpContentManagementTypes.h"
#include "DtcpAkeCoreTypes.h"
#include "DtcpCoreTypes.h"

/// \defgroup DtcpContent DtcpContentManagement
/// \brief Provides the functions to encrypt/decrypt content
///
/// This library is for creating/consuming the protected content packets
/// used for DTCP-IP.  It provides functions for creating packet headers
/// as well as extracting useful information from the packet headers.  It
/// also provides functions for encrypting/decrypting the content of the
/// protected content packets.
/// Below is a diagram showing that a content stream after it has been
/// protected using DTCP.  The original stream is broken up into a stream
/// of protected content packets.
/// \image html DtcpIpSdk_ProtectedContentPacket.gif "Protected Content Packets"
/// @{

/// \brief Opens a stream for encrypting
///
/// This function is called by source devices
/// \param aStreamTransport - Input; Identifies the type of stream being encrypted
/// \param aStreamHandle - Output; Upon successful completion will contain a handle to the stream
int DtcpContent_OpenStream(EnStreamTransport            aStreamTransport,
                           DtcpStreamHandle            *aStreamHandle);

/// \brief Closes a stream
///
/// \param aStreamHandle - Input; Handle to the stream to close
int DtcpContent_CloseStream(DtcpStreamHandle aStreamHandle);

/// \brief Creates a protected content packet header
///
/// This function is called by source devices
/// \param aStreamHandle - Input; Handle to the stream that this packet is associated with
/// \param aExtendedEmi - Input; Extended EMI value of this packet
/// \param aContentSize - Input; Size (in bytes) of the payload of the packet
/// \param aPacketHeader - Input/Output; Pointer to a buffer that will be filled with the packet header
/// \param aPacketHandle - Output; Upon successful completion will contain a handle to the packet
int DtcpContent_CreatePacketHeader(DtcpStreamHandle  aStreamHandle,
                                   EnExtendedEmi     aExtendedEmi,
                                   unsigned int      aContentSize,
                                   unsigned char     aPacketHeader[DTCP_CONTENT_PACKET_HEADER_SIZE],
                                   DtcpPacketHandle *aPacketHandle);

/// \brief Extracts the useful information from a protected content packet header
///
/// This function is called by sink devices
/// \param aAkeCoreSessionHandle - Input; Handle to an AKE session
/// \param aPacketHeader - Input; Pointer to a buffer containing the protected content packet header
/// \param aExtendedEmi - Output; Upon successful completion will contain the extended EMI value of the packet
/// \param aContentSize - Output; Upon successful completion will contain the size (in bytes) of the content of this packet
/// \param aPacketHandle - Output; Upon successful completion will contain a handle to the packet
int DtcpContent_ConsumePacketHeader(DtcpAkeCoreSessionHandle aAkeCoreSessionHandle,
                                    unsigned char     aPacketHeader[DTCP_CONTENT_PACKET_HEADER_SIZE],
                                    EnExtendedEmi    *aExtendedEmi,
                                    unsigned int     *aContentSize,
                                    DtcpPacketHandle *aPacketHandle);

/// \brief Closes a packet
///
/// This function is called by both source and sink devices
/// \param aPacketHandle - Input; Handle of the packet to close
int DtcpContent_ClosePacket(DtcpPacketHandle aPacketHandle);

/// \brief Encrypts data in a packet
/// 
/// This function is called by source devices.  This function may be called
/// multiple times for each packet until the entire payload for the packet
/// has been encrypted.  For example, if the packet content size is 4096 bytes,
/// this function may be called four times, each time encrypting 1024 bytes.  Or
/// this function may be called once encrypting the entire 4096 bytes.
/// \param aPacketHandle - Input; Handle to the packet for which this data is a part
/// \param aInBuffer - Input; Pointer to a buffer containing the unencrypted content
/// \param aOutBuffer - Input/Output; Pointer to a buffer to place the enctypted content.  
/// This can point to the same buffer as aInBuffer and the content will be encrypted in place.
/// \param aBufferLength - Input; Length of both the aInBuffer and aOutBuffer
int DtcpContent_EncryptData(DtcpPacketHandle  aPacketHandle, 
                            unsigned char    *aInBuffer,
                            unsigned char    *aOutBuffer,
                            unsigned int      aBufferLength);


/// \brief Decrypts data in a packet
/// 
/// This function is called by sink devices.  This function may be called
/// multiple times for each packet until the entire payload for the packet
/// has been decrypted.  For example, if the packet content size is 4096 bytes,
/// this function may be called four times, each time decrypting 1024 bytes.  Or
/// this function may be called once decrypting the entire 4096 bytes.
/// \param aPacketHandle - Input; Handle to the packet for which this data is a part
/// \param aInBuffer - Input; Pointer to a buffer containing the encrypted content
/// \param aOutBuffer - Input/Output; Pointer to a buffer to place the decrypted content.  
/// This can point to the same buffer as aInBuffer and the content will be decrypted in place.
/// \param aBufferLength - Input; Length of both the aInBuffer and aOutBuffer
int DtcpContent_DecryptData(DtcpPacketHandle  aPacketHandle, 
                            unsigned char    *aInBuffer,
                            unsigned char    *aOutBuffer,
                            unsigned int      aBufferLength);

/// @}

#endif // __DTCP_CONTENT_MANAGEMENT_H__
