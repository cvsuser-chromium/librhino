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
/// \brief Defines private types used within the \ref DtcpCore library.
///
/// This header file contains the data structures used in the \ref DtcpCore library.

#ifndef __DTCP_CORE_INTERNAL_H__
#define __DTCP_CORE_INTERNAL_H__

#include "DtcpConstants.h"
#include "DtcpEcCrypto.h"
#include "OsWrapper.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// \addtogroup DtcpCore
/// @{

/// \brief Data structure containing parameters for a DTCP device.
typedef struct DtcpDeviceData
{
    unsigned char Cert[DTCP_EXTENDED_FULL_CERT_SIZE]; ///< Buffer containing our device certificate.
    unsigned int  CertSize;                           ///< Size of our device certificate.
    unsigned char Srm[DTCP_SRM_MAX_SIZE];             ///< Buffer containing our SRM.
    unsigned int  SrmSize;                            ///< Size of our SRM.
    EccParams     EccParams;                          ///< Eliptic curve cryptography parameters.
    unsigned char DtlaPublicKey[ECC_PUBLIC_KEY_SIZE]; ///< Buffer containing the DTLA public key.
    unsigned char PrivateKey[ECC_PRIVATE_KEY_SIZE];   ///< Buffer containing our public key.
    UpdateSrm_Ptr UpdateSrmFunc;                      ///< Function pointer to call back to update our SRM.
    void         *DataSemaphore;                      ///< Semaphore protecting integrity of data.
} DtcpDeviceData;

/// @}

#ifdef __cplusplus
}
#endif

#endif // __DTCP_CORE_INTERNAL_H__
