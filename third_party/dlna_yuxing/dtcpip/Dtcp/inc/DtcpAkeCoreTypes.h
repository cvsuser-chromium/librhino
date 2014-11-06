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

#ifndef __DTCP_AKE_CORE_TYPES_H__
#define __DTCP_AKE_CORE_TYPES_H__

/// \file 
/// \brief Defines the public types for the \ref DtcpAkeCore library.
/// 
/// See \ref DtcpAkeCore for more information.

/// \addtogroup DtcpAkeCore
/// @{

/// \brief Callback function implemented by the application.  Used to request updating
///        the exchange keys.  
///
/// This function will be called when the sink count limit
/// has been reached and another sink device tries to authenticate.  The application
/// should updated the exchange keys if it is not currently streaming content.
/// If the application is streaming content, it should not update the exchange keys.
/// The sink device that initiated the AKE will not successfully authenticate.
/// \retval 0 - Successfully updated exchange keys
/// \retval -1 - Did not update exchange keys
typedef int (*ExchangeKeyUpdateRequest_Ptr)();

/// \brief Bit mask specifying a command buffer
///
/// This bit mask is used to specify which buffer to free
typedef enum
{
    akeBufferChallenge         =  1, ///< Challenge command buffer
    akeBufferResponse          =  2, ///< Response command buffer
    akeBufferExchKeyCopyOneGen =  4, ///< Copy one generation exchange key command buffer
    akeBufferExchKeyCopyNoMore =  8, ///< Copy no more exchange key command buffer
    akeBufferExchKeyCopyNever  = 16, ///< Copy never exchange key command buffer
    akeBufferExchKeyAes128     = 32, ///< AES 128 exchange key command buffer
    akeBufferSrm               = 64, ///< SRM command buffer
} EnAkeBufferMask;

/// \brief Enumeration defining the valid values for the cipher_algorithm
///        field in the AKE_info of the ExchangeKey subfunction
typedef enum
{
    cipherM6     = 0x0,  ///< M6 (56 bits)
    cipherAes128 = 0x1,  ///< AES-128
    cipherNoInfo = 0xF   ///< Not used or no information
} EnCipherAlgorithm;

/// \brief Handle to a AKE session
typedef void *DtcpAkeCoreSessionHandle;

/// \brief Handle to a stream
typedef void *DtcpAkeCoreStreamHandle;
/// @}

#endif // __DTCP_AKE_CORE_TYPES_H__
