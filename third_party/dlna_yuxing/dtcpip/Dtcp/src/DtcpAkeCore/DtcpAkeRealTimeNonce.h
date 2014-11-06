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

#ifndef __DTCP_REAL_TIME_NONCE_H__
#define __DTCP_REAL_TIME_NONCE_H__

/// \file 
/// \brief Defines functions for running the nonce timer for real time streams
///
/// This header file is part of the \ref DtcpAkeCore library.

#include "DtcpConstants.h"

/// \brief This function runs in its own thread and increments the real time nonce.
/// \param aParameter - Input; Pointer to DtcpAkeCoreData structure
int RealTimeNonceTimer(void *aParameter);

#endif  // __DTCP_REAL_TIME_NONCE_H__
