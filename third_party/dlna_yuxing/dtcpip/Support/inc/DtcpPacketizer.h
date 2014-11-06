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
/// \brief Definition file for \ref DtcpPacketizer.
///
/// This header file contains the interface to the \ref DtcpPacketizer library.
/// This API is the high-level interface used by an application to integrate with
/// the DTCP-IP PCP system.

#ifndef _DTCP_PACKETIZER_H_
#define _DTCP_PACKETIZER_H_

#include "DtcpContentManagementTypes.h"

#include <stddef.h> /* size_t */

#   ifdef __cplusplus
extern  "C"
{
#   endif

/*******************************************************************************
*                              Common Functions                                *
*******************************************************************************/

/// \defgroup DtcpPacketizer DtcpPacketizer
/// \brief Library that provides a single interface to be used by a source and sink
/// integration for DTCP-IP PCPs.
/// @{

// DTCP-IP MAX packet size 128*(1024*1024)
#define DTCP_MAX_PACKET_SIZE (134217728)

/// \brief Parameters describing a DTCP-IP source output or sink input PCP stream
struct dtcp_stream_t
{
    int source; ///< Source / Sink - 0 = Sink, 1 = Source

	DtcpStreamHandle dtcp_stream_handle;    ///< Source - Handle to DTCP-IP source stream
	size_t dtcp_stream_content_length;      ///< Source - Byte lenght of content stream, Sink - Cumulative content stream length
	size_t dtcp_stream_content_remaining;   ///< Source - Number of bytes remaining in content stream
	EnExtendedEmi dtcp_emi;                 ///< Source - EMI value of content stream, Sink - EMI value from PCPH (Sink's EMI may change per PCPH)

	DtcpPacketHandle dtcp_packet_handle;    ///< Source / Sink - Handle to PCPH
	size_t dtcp_packet_content_length;      ///< Source / Sink - Byte length of PCP
	size_t dtcp_packet_content_remaining;   ///< Source / Sink - Number of content bytes processed
    size_t dtcp_packet_bytes_remaining;     ///< Source / Sink - Number of content byts to process
    size_t dtcp_packet_padding_bytes;       ///< Sink - Number of padding bytes for this PCP

	size_t max_packet_size;                 ///< Source - Max PCP size for content stream

    void *ake_handle;                       ///< Sink - Handle to AKE to content source
};

/// \brief Destroys a content stream created by DtcpPacketizer_CreateContentSink or 
/// DtcpPacketizer_CreateContentSource.
/// This function is used by both source and sink devices.
///
/// \param dtcp_stream Input - Pointer to a completed dtcp_stream_t
int DtcpPacketizer_DestroyContentStream(struct dtcp_stream_t *dtcp_stream);

/*******************************************************************************
*                              Source functions                                *
*******************************************************************************/

/// \brief Creates a content source output stream.
/// This function currently only supports the HTTP transport.
///
/// \param dtcp_stream Input - Pointer to an allocated dtcp_stream_t
/// \param content_emi Input - EMI value for the PCP.  See DtcpContentManagement.h:EnExtendedEmi
/// \param content_length Input - Total length of content source stream.
/// \param max_packet_size Input - Maximum size for PCP, < DTCP_MAX_PACKET_SIZE
int DtcpPacketizer_CreateContentSource(struct dtcp_stream_t *dtcp_stream, int content_emi,
                             size_t content_length, size_t max_packet_size);

/// \brief Packetizes the input clear content into PCPs, and adds PCPHs as necessary.
/// Since the encrypted data will be expanded by PCPHs and padding bytes, multiple
/// calls to this function may be necessary to encrypt the desired data.  Always check
/// the value of total_content_processed, if all data was processed it will be equal to
/// clear_content_size.
///
/// \param dtcp_stream Input - Pointer to a dtcp_stream_t
/// \param clear_content Input - Clear text content
/// \param clear_content_size Input - Byte length of the clear_content
/// \param encrypted_data Input  - Allocated buffer
///                       Output - Encrypted content data
/// \param encrypted_data_size Input  - Size of encrypted_data buffer
///                            Output - Byte Length of encrypted data (includes PCPHs)
/// \param total_content_processed Output - Number of content bytes processed (excludes PCPHs)
int DtcpPacketizer_PacketizeData(struct dtcp_stream_t *dtcp_stream, 
				  /* IN */ char *clear_content, /* IN */ size_t clear_content_size, 
				  /* IN/OUT */ char **encrypted_data, /* IN/OUT */ size_t *encrypted_data_size,
                  /* OUT */ size_t *total_content_processed);


/*******************************************************************************
*                               Sink functions                                 *
*******************************************************************************/

/// \brief Creates a content sink input stream.
/// This function currently only supports the HTTP transport.
///
/// \param dtcp_stream Input - Pointer to an allocated dtcp_stream_t
/// \param ake_handle Input - AKE handle of the content source
int DtcpPacketizer_CreateContentSink(struct dtcp_stream_t *dtcp_stream, void *ake_handle);

/// \brief Depacketizes the input PCPs, removing PCPHs as necessary.
/// Since the clear content will be smaller, less PCPHs and padding bytes, multiple
/// calls to this function will not be necessary to decrypt the desired data, so long
/// as.  encrypted_data_size <= clear_content_size.  Always check the value of
/// total_data_processed, if all data was processed it will be equal to
/// encrypted_data_size.
///
/// \param dtcp_stream Input - Pointer to a dtcp_stream_t
/// \param encrypted_data Input - Encrypted PCP data
/// \param encrypted_data_size Input - Length of the encrypted data
/// \param clear_content Input  - Allocated buffer
///                      Output - Decrypted clear content
/// \param clear_content_size Input  - Size of clear text content
///                           Output - Byte Length of clear content (excludes PCPHs)
/// \param total_data_processed Output - Number of data bytes processed (includes PCPHs)
int DtcpPacketizer_DepacketizeData(struct dtcp_stream_t *dtcp_stream, 
				  /* IN */ char *encrypted_data, /* IN */ size_t encrypted_data_size, 
				  /* IN/OUT */ char *clear_content, /* IN/OUT */ size_t *clear_content_size,
                  /* OUT */ size_t *total_data_processed);

/// @}

#   ifdef __cplusplus
}
#   endif /* extern "C" */

#endif /* _DTCP_PACKETIZER_H_ */

