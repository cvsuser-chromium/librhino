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

#ifndef __RNG_TYPES_H__
#define __RNG_TYPES_H__

/// \file
/// \brief Defines the public types used by the \ref Rng library.
///
/// See \ref Rng for more information.

/// \addtogroup Rng
/// @{

/// \brief Callback function called by the \ref Rng library when the seed for
/// the random number generator should be stored.
///
/// \param aRngSeed - Input; Pointer to a buffer containing the seed
/// \param aRngSeedSize - Input; Size (in bytes) of the seed
typedef int (*UpdateRngSeed_Ptr)(unsigned char *aRngSeed, unsigned int aRngSeedSize);

/// @}

#endif // __RNG_TYPES_H__
