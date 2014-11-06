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

#ifndef __DTCP_CORE_TYPES_H__
#define __DTCP_CORE_TYPES_H__

#include "DtcpEcCryptoTypes.h"

/// \file 
/// \brief Defines the public types for the \ref DtcpCore library.
/// 
/// See \ref DtcpCore for more information.

/// \addtogroup DtcpCore
/// @{

/// \brief Callback function implemented by the application.  Used to store
///        an updated SRM, received from another device, in non-volatile memory.
///
/// \param aSrm Input - Pointer to a buffer containing the new SRM
/// \param aSrmSize Input - Size (in bytes) of the new SRM
/// \retval 0 No errors occurred.
/// \retval -1 Error occurred, unable to save new SRM.
typedef int (*UpdateSrm_Ptr)(unsigned char *aSrm, unsigned int aSrmSize);

// Types
/// \brief Bitmask for the types of AKE.
typedef enum
{
    akeTypeMaskRestrictedAuth = 1,           ///< Restricted authentication
    akeTypeMaskEnhancedRestrictedAuth = 2,   ///< Enhanced restricted authentication
    akeTypeMaskFullAuth = 4,                 ///< Full authentication
    akeTypeMaskExtendedFullAuth = 8,         ///< Extended full authentication
} EnAkeTypeMask;

/// \brief Unique ID for each type of AKE.
typedef enum
{
    akeTypeIdRestrictedAuth = 0,             ///< Restricted authentication
    akeTypeIdEnhancedRestrictedAuth = 1,     ///< Enhanced restricted authenticatio
    akeTypeIdFullAuth = 2,                   ///< Full authentication
    akeTypeIdExtendedFullAuth = 3,           ///< Extended full authentication
} EnAkeTypeId;

/// \brief Modes that a device could be operating in during an AKE.
typedef enum
{
    deviceModeSource,                        ///< Device is in source mode
    deviceModeSink                           ///< Device is in sink mode
} EnDeviceMode;

/// \brief Unique Id for each exchange key.
typedef enum
{
    exchKeyIdCopyNever = 0,                  ///< Copy Never exchange key
    exchKeyIdCopyOneGeneration = 1,          ///< Copy One Generation exchange key
    exchKeyIdCopyNoMore = 2,                 ///< Copy No More exchange key
    exchKeyIdAes128 = 3,                     ///< AES - 128 exchange key
} EnExchKeyId;

/// \brief Bitmask for the exchange keys.
typedef enum
{
    exchKeyMaskCopyNever = 1,                ///< Copy Never exchange key
    exchKeyMaskCopyOneGeneration = 2,        ///< Copy One Generation exchange key
    exchKeyMaskCopyNoMore = 4,               ///< Copy No More exchange key
    exchKeyMaskAes128 = 8,                   ///< AES - 128 exchange key
} EnExchKeyMask;

/// \brief Parameters describing a DTCP device
typedef struct
{
    unsigned char *Cert;                               ///< Pointer to a certificate
    unsigned int   CertSize;                           ///< Size (in bytes) of the certificate
    unsigned char *Srm;                                ///< Pointer to a SRM
    unsigned int   SrmSize;                            ///< Size (in bytes) of the SRM
    unsigned char  DtlaPublicKey[ECC_PUBLIC_KEY_SIZE]; ///< DTLA public key
    unsigned char  PrivateKey[ECC_PRIVATE_KEY_SIZE];   ///< This device's private key
    EccParams      EccParams;                          ///< ECC parameters
} DeviceParams;
/// @}

#endif // __DTCP_CORE_TYPES_H__
