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
/// \brief Defines the interface to the \ref DtcpCore library.
///
/// This header file contains the interface to the \ref DtcpCore library.  This library
/// is used to store core data for a DTCP product including certificate, SRM, keys,
/// and ECC parameters.

#ifndef __DTCP_CORE_H__
#define __DTCP_CORE_H__

#include "DtcpConstants.h"
#include "DtcpCoreTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// \defgroup DtcpCore DtcpCore
/// \brief Maintains core data and algorithms for a DTCP application.
///
/// Each DTCP device must maintain core data such as a certificate, SRM, 
/// public/private keys, and elliptic curve cryptography parameters. This library 
/// maintains that data as well as provides functions to get/set that data.
/// It also provides access to the elliptic curve cryptographic algorithms 
/// which require sensitive data such as the elliptic curve parameters as 
/// input.
/// @{

// Functions
/// \brief Initializes the DtcpCore library with the appropriate parameters.
///
/// \param aDeviceParams - Input; Pointer to a structure containing all of the parameters
///                        describing this device
/// \param aUpdateSrmFunc - Input; Pointer to a function that is called when the device
///                         needs to update its SRM
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
/// \retval INVALID_CERTIFICATE DTCP certificate is not valid
int DtcpCore_Startup(DeviceParams      *aDeviceParams,
                     UpdateSrm_Ptr      aUpdateSrmFunc);

/// \brief Releases any resources associated with the handle
///
void DtcpCore_Shutdown();

/// \brief Gets a pointer to a buffer containing the device's certificate
///
/// \param aCert - Output; Upon successful completion, will contain a pointer to a buffer containing the certificate
/// \param aCertSize - Output; Upon successful completion, will contain the length (in bytes) of the certificate
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpCore_GetCert(unsigned char **aCert, unsigned int *aCertSize);

/// \brief Gets a pointer to a buffer containing the device's SRM
///
/// \param aSrm - Output; Upon successful completion, will contain a pointer to a buffer containing the SRM
/// \param aSrmSize - Output; Upon successful completion, will contain the length (in bytes) of the SRM
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpCore_GetSrm(unsigned char **aSrm, unsigned int *aSrmSize);

/// \brief Stores a SRM
///
/// \param aSrm - Input; Pointer to a buffer containing an Srm
/// \param aSrmSize - Input; Upon successful completion, will contain the size (in bytes) of the certificate
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpCore_SetSrm(unsigned char *aSrm, unsigned int aSrmSize);

/// \brief Gets the supported AKE procedures
///
/// Currently this function only supports DTCP-IP (full authentication
/// certificates).
/// \param aAkeTypeMask - Output; Upon success completion, will contain a
///                       bitmask specifying the types of AKE this device
///                       is capable of performing
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpCore_GetSupportedAkeProcedures(EnAkeTypeMask *aAkeTypeMask);

/// \brief Gets the supported exchange keys
///
/// Currently this function only supports DTCP-IP.  The exchange key
/// returned will be for AES-128.
/// \param aExchKeyMask - Output; Upon success completion, will contain a
///                       bitmask specifying the types of exchange keys 
///                       this device is capable of using
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpCore_GetSupportedExchKeys(EnExchKeyMask *aExchKeyMask);

/// \brief Gets the DTLA public key
///
/// \param aDtlaPublicKey - Output; Upon successful completion, will contain a pointer to a buffer containing the DTLA public key
/// \param aDtlaPublicKeySize - Output; Upon successful completion, will contain the length (in bytes) of the DTLA public key
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpCore_GetDtlaPublicKey(unsigned char **aDtlaPublicKey, 
                              unsigned int *aDtlaPublicKeySize);

/// \brief Signs a buffer of data
///
/// \param aBuffer - Input; Pointer to a buffer containing the data to be signed
/// \param aBufferSize - Input; Length (in bytes) of the data to sign
/// \param aSignature - Output; Upon success completion, will contain the signature of the data
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpCore_SignData(unsigned char *aBuffer, 
                      unsigned int aBufferSize, 
                      unsigned char aSignature[SIGNATURE_SIZE]);

/// \brief Verifies a signature of data
///
/// \param aBuffer - Input; Pointer to a buffer containing the data to be signed
/// \param aBufferSize - Input; Length (in bytes) of the data to sign
/// \param aSignature - Input; Signature of the data to be verified
/// \param aKey - Input; Key to use to verify the signature
/// \retval SUCCESS if data is valid
/// \retval FAILURE if data is invalid
int DtcpCore_VerifyData(unsigned char *aBuffer,
                        unsigned int aBufferSize,
                        unsigned char aSignature[SIGNATURE_SIZE],
                        unsigned char aKey[ECC_PUBLIC_KEY_SIZE]);

/// \brief Computes the first phase value for a Diffie Helman key exchange
///
/// \param aFirstPhaseValue - Output; Upon completion, will contain the first phase value for the Diffie Helman key exchange
/// \param aFirstPhaseSecret - Optput; Upon completeion, will contain the first phase secret for the Diffie Helman key exchange 
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpCore_GetFirstPhaseValue(unsigned char aFirstPhaseValue[DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE],
                                unsigned char aFirstPhaseSecret[DTCP_DH_FIRST_PHASE_SECRET_SIZE]);

/// \brief Computes the first phase value for a Diffie Helman key exchange
///
/// \param aFirstPhaseValue - Input; Buffer containing the first phase value for the Diffie Helman key exchange
/// \param aFirstPhaseSecret - Input; Buffer containing the first phase secret for the Diffie Helman key exchange
/// \param aAuthKey - Output; Upon completion, will contain the Diffie Helman shared secret
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpCore_GetSharedSecret(unsigned char aFirstPhaseValue[DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE],
                             unsigned char aFirstPhaseSecret[DTCP_DH_FIRST_PHASE_SECRET_SIZE],
                             unsigned char aAuthKey[DTCP_AUTH_KEY_SIZE]);

/// @}

#ifdef __cplusplus
}
#endif

#endif // __DTCP_CORE_H__
