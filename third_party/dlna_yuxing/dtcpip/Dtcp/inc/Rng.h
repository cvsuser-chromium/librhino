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

#ifndef __RNG_H__
#define __RNG_H__

#include "RngTypes.h"
#include "DtcpConstants.h"

/// \file
/// \brief Definition file for \ref Rng library.
///
/// This header file contains the interface to the \ref Rng library.

/// \defgroup Rng Rng
/// \brief Provides an implementation of a random number generator
/// @{

/// \brief Initializes the random number generator.
///
/// \param aRngSeed - Input; Initial seed of the random number generator
/// \param aUpdateRngSeedFunc - Input; Pointer to a function to update the seed
int Rng_Initialize(unsigned char aRngSeed[DTCP_AES_BLOCK_SIZE], UpdateRngSeed_Ptr aUpdateRngSeedFunc);

/// \brief Gets a random number
///
/// \param aRngBuffer - Input; Pointer to a buffer to hold the random number.  It needs to be 
///        of sufficient size to hold the random number requested.
/// \param aRngSize - Input; Size (in bytes) of the random number to get.
int Rng_GetRandomNumber(unsigned char *aRngBuffer, unsigned int aRngSize);

/// @}

#endif // __RNG_H__
