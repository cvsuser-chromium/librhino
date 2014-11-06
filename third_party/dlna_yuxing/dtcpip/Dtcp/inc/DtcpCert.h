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
/// \brief Defines the interface file for \ref DtcpCert library.
///
/// This header file contains the interface to the \ref DtcpCert library.  This library
/// contains several functions intended to assist in working with DTCP certificates.

#ifndef __DTCP_CERT_H__
#define __DTCP_CERT_H__

#include "DtcpCertTypes.h"
#include "DtcpCoreTypes.h"
#include "DtcpConstants.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// \defgroup DtcpCert DtcpCert
/// \brief Helper functions for extracting information from certificates.
///
/// A DTCP certificate is a binary piece of data.  This library is 
/// intended to provide a user with helper functions to extract 
/// the desired information from the certificate without needing to
/// know how to parse the certificate.  It also provides a function
/// to verify the authenticity of the certificate.
/// @{

/// \brief Verifies the authenticity of a certificate.
///
/// \param aCert - Input; pointer to a certificate
/// \retval SUCCESS certificate is valid
/// \retval INVALID_CERTIFICATE certificate is invalid
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpCert_VerifyCert(unsigned char *aCert);

/// \brief Gets the type of a certificate.
///
/// \param aCert - Input; pointer to a certificate
/// \param aType - Output; pointer to a variable to get the type value
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpCert_GetType(unsigned char *aCert, unsigned int *aType);

/// \brief Gets the format of a certificate.
///
/// \param aCert - Input; pointer to a certificate
/// \param aFormat - Output; pointer to a variable to get the format value
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpCert_GetFormat(unsigned char *aCert, unsigned int *aFormat);

/// \brief Gets the device generation (SRMG) of a certificate.
///
/// \param aCert - Input; pointer to a certificate
/// \param aDeviceGen - Output; pointer to a variable to get the device generation value
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
/// \retval DTCP_CERT_INVALID_FORMAT This call is not appropriate for this certificate type
int DtcpCert_GetDeviceGen(unsigned char *aCert, unsigned int *aDeviceGen);

/// \brief Gets the authentication proxy flag of a certificate.
///
/// \param aCert - Input; pointer to a certificate
/// \param aApFlag - Output; pointer to a variable to get the authentication proxy flag value
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
/// \retval DTCP_CERT_INVALID_FORMAT This call is not appropriate for this certificate type
int DtcpCert_GetApFlag(unsigned char *aCert, unsigned int *aApFlag);

/// \brief Gets the device Id of a certificate.
///
/// \param aCert - Input; pointer to a certificate
/// \param aDeviceId - Output; pointer to a buffer to receive the device Id
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpCert_GetDeviceId(unsigned char *aCert, unsigned char aDeviceId[DTCP_DEVICE_ID_SIZE]);

/// \brief Gets the public key of a certificate.
///
/// \param aCert - Input; pointer to a certificate
/// \param aPublicKey - Output; pointer to a buffer to receive the public key
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
/// \retval DTCP_CERT_INVALID_FORMAT This call is not appropriate for this certificate type
int DtcpCert_GetPublicKey(unsigned char *aCert, unsigned char aPublicKey[PUBLIC_KEY_SIZE]);

/// \brief Gets the signature of a certificate.
///
/// \param aCert - Input; pointer to a certificate
/// \param aSignature - Output; pointer to a buffer to receive the signature
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
/// \retval DTCP_CERT_INVALID_FORMAT This call is not appropriate for this certificate type
int DtcpCert_GetSignature(unsigned char *aCert, unsigned char aSignature[SIGNATURE_SIZE]);

/// \brief Gets the device capability mask of a certificate.
///
/// \param aCert - Input; pointer to a certificate
/// \param aDeviceCapMask - Output; pointer to a buffer to receive the device capability mask
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
/// \retval DTCP_CERT_INVALID_FORMAT This call is not appropriate for this certificate type
int DtcpCert_GetDeviceCapMask(unsigned char *aCert, unsigned int *aDeviceCapMask);

/// \brief Gets the key selection vector of a certificate.
///
/// \param aCert - Input; pointer to a certificate
/// \param aKeySelectionVector - Output; pointer to a buffer to receive the key selection vector
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
/// \retval DTCP_CERT_INVALID_FORMAT This call is not appropriate for this certificate type
int DtcpCert_GetKeySelectionVector(unsigned char *aCert, unsigned int *aKeySelectionVector);

/// @}

#ifdef __cplusplus
}
#endif

#endif // __DTCP_CERT_H__
