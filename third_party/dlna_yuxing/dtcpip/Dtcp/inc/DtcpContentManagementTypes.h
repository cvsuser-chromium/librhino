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

#ifndef __DTCP_CONTENT_MANAGEMENT_TYPES_H__
#define __DTCP_CONTENT_MANAGEMENT_TYPES_H__

/// \file 
/// \brief Defines the public types for the \ref DtcpContent library.
/// 
/// See \ref DtcpContent for more information.

/// \addtogroup DtcpContent
/// @{

/// \brief Handle to a stream
typedef void *DtcpStreamHandle;

/// \brief Handle to a single Protected Content Packet
typedef void *DtcpPacketHandle;

/// \brief Types of streams
typedef enum
{
    streamTransportRtp,           ///< Stream transport is RTP
    streamTransportHttp           ///< Stream transport is HTTP
} EnStreamTransport;

/// \brief Extended EMI states
typedef enum
{
    extEmiCopyNever = 12,           ///< Copy Never
    extEmiCopyOneGenFormatCog=10,   ///< Copy One Generation - Format cognizant recording only
    extEmiCopyOneGenFormatNonCog=8, ///< Copy One Generation - Format non cognizant recording permitted
    extEmiMove=6,                   ///< Move
    extEmiNoMoreCopies=4,           ///< No More Copies
    extEmiCopyFreeEpnAsserted=2,    ///< Copy Free with EPN asserted
    extEmiCopyFree=0                ///< Copy Free
} EnExtendedEmi;

/// @}

#endif // __DTCP_CONTENT_MANAGEMENT_TYPES_H__
