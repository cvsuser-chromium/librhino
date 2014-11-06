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

#ifndef __DTCP_EXCHANGE_KEYS_H__
#define __DTCP_EXCHANGE_KEYS_H__

/// \file
/// \brief Defines functions and data structures for storing and using the exchange keys
///
/// This file is part of the \ref DtcpAkeCore library.

#include "DtcpConstants.h"
#include "DtcpCoreTypes.h"

// Constants
#define EXCHANGE_KEY_COUNT             4 ///< There are four types of exchange keys.

// Structures
/// \brief Data structure for storing a set of exchange keys
typedef struct DtcpExchKeyData
{
    unsigned int  Initialized;   ///< Flag indicating whether this structure has been initialized
    unsigned int  Label;         ///< Contains current exchange key label
    unsigned char ExchKeys[EXCHANGE_KEY_COUNT][DTCP_EXCHANGE_KEY_SIZE]; ///< Buffer for storing exchange keys
    unsigned int  ValidFlag;     ///< Bit mask indicating validity of each exchange key
} DtcpExchKeyData;

// Functions
/// \brief Initializes a DtcpExchKeyData structure
/// \param aExchKeyData - Input; Pointer to a DtcpExchKeyData structure
int DtcpExchKeys_Startup(DtcpExchKeyData *aExchKeyData);

/// \brief Creates the exchange keys
/// \param aExchKeyData - Input; Pointer to a DtcpExchKeyData structure
int DtcpExchKeys_CreateExchKeys(DtcpExchKeyData *aExchKeyData);

/// \brief Gets an exchange key
/// \param aExchKeyData - Input; Pointer to a DtcpExchKeyData structure
/// \param aExchKeyId - Input; Type of exchange key to get
/// \param aExchKey - Output; Upon successful completion buffer will be filled with exchange key
int DtcpExchKeys_GetExchKey(DtcpExchKeyData *aExchKeyData,
                            EnExchKeyId aExchKeyId,
                            unsigned char aExchKey[DTCP_EXCHANGE_KEY_SIZE]);

/// \brief Sets an exchange key
/// \param aExchKeyData - Input; Pointer to a DtcpExchKeyData structure
/// \param aExchKeyId - Input; Type of exchange key to set
/// \param aExchKey - Input; Pointer to a buffer containing the exchange key
int DtcpExchKeys_SetExchKey(DtcpExchKeyData *aExchKeyData,
                            EnExchKeyId aExchKeyId,
                            unsigned char aExchKey[DTCP_EXCHANGE_KEY_SIZE]);

/// \brief Checks if the specified exchange key is valid
/// \param aExchKeyData - Input; Pointer to a DtcpExchKeyData structure
/// \param aExchKeyId - Input; Type of exchange key to set
int DtcpExchKeys_IsExchKeyValid(DtcpExchKeyData *aExchKeyData,
                                EnExchKeyId aExchKeyId);

/// \brief Invalidates the exchange keys
/// \param aExchKeyData - Input; Pointer to a DtcpExchKeyData structure
int DtcpExchKeys_InvalidateExchKeys(DtcpExchKeyData *aExchKeyData);

/// \brief Checks if data structure has been initialized
/// \param aExchKeyData - Input; Pointer to a DtcpExchKeyData structure
int DtcpExchKeys_IsInitialized(DtcpExchKeyData *aExchKeyData);

/// \brief Sets the exchange key label
/// \param aExchKeyData - Input; Pointer to a DtcpExchKeyData structure
/// \param aExchKeyLabel - Input; Value of the exchange key label
int DtcpExchKeys_SetExchKeyLabel(DtcpExchKeyData *aExchKeyData,
                                 unsigned int     aExchKeyLabel);

/// \brief Gets the exchange key label
/// \param aExchKeyData - Input; Pointer to a DtcpExchKeyData structure
/// \param aExchKeyLabel - Output; Upon successful completion contains the value of the exchange key label
int DtcpExchKeys_GetExchKeyLabel(DtcpExchKeyData *aExchKeyData,
                                 unsigned int    *aExchKeyLabel);

// Return values
#define DATA_NOT_INITIALIZED           -2
#define DATA_NOT_AVAILABLE             -3

#endif // __DTCP_EXCHANGE_KEYS_H__
