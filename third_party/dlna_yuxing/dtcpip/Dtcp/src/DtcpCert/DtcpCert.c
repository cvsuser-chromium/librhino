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

#include <memory.h>
#include "DtcpCert.h"
#include "DtcpCore.h"
#include "DtcpStatusCodes.h"

/// \file
/// \brief Implementation file for the \ref DtcpCert library.
///
/// This file implements the interface for the DtcpCert library.  See \ref DtcpCert for more information.

int DtcpCert_VerifyCert(unsigned char *aCert)
{
    int returnValue = SUCCESS;
    int bufferSize = 0;
    unsigned int certFormat;
    unsigned char certSignature[SIGNATURE_SIZE];
    unsigned char *publicKey;
    unsigned int publicKeySize;

    if (!aCert)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpCert_GetFormat(aCert, &certFormat);
    }

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpCert_GetSignature(aCert, certSignature);
    }

    if (IS_SUCCESS(returnValue))
    {
        if (certFormatBaselineFull == certFormat)
        {
            bufferSize = DTCP_BASELINE_FULL_CERT_SIZE - SIGNATURE_SIZE;
        }

        returnValue = DtcpCore_GetDtlaPublicKey(&publicKey, &publicKeySize);

        if (IS_SUCCESS(returnValue))
        {
            returnValue = DtcpCore_VerifyData(aCert, bufferSize, certSignature, publicKey);
        }

        if (IS_FAILURE(returnValue))
        {
            returnValue = INVALID_CERTIFICATE;
        }
    }

    return returnValue;
} // DtcpCert_VerifyCert

int DtcpCert_GetType(unsigned char *aCert, unsigned int *aType)
{
    int returnValue = SUCCESS;

    if ((0 != aCert) && (0 != aType))
    {
        *aType = 0;
        *aType = (aCert[0] & 0xF0) >> 4;
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    return returnValue;
} // DtcpCert_GetFormat

int DtcpCert_GetFormat(unsigned char *aCert, unsigned int *aFormat)
{
    int returnValue = SUCCESS;

    if ((0 != aCert) && (0 != aFormat))
    {
        *aFormat = 0;
        *aFormat = aCert[0] & 0x0F;
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    return returnValue;
} // DtcpCert_GetFormat

int DtcpCert_GetDeviceGen(unsigned char *aCert, unsigned int *aDeviceGen)
{
    int returnValue = SUCCESS;
    unsigned int format;

    if ((0 != aCert) && (0 != aDeviceGen))
    {
        DtcpCert_GetFormat(aCert, &format);

        if ((certFormatBaselineFull == format) ||
            (certFormatExtendedFull == format))
        {
            *aDeviceGen = 0;
            *aDeviceGen = (aCert[1] & 0xF0) >> 4;
        }
        else
        {
            returnValue = DTCP_CERT_INVALID_FORMAT;
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    return returnValue;
} // DtcpCert_GetDeviceGen

int DtcpCert_GetApFlag(unsigned char *aCert, unsigned int *aAPFlag)
{
    int returnValue = SUCCESS;
    unsigned int format;

    if ((0 != aCert) && (0 != aAPFlag))
    {
        DtcpCert_GetFormat(aCert, &format);

        if ((certFormatBaselineFull == format) ||
            (certFormatExtendedFull == format))
        {
            *aAPFlag = 0;
            *aAPFlag = (aCert[2] & 0x01);
        }
        else
        {
            returnValue = DTCP_CERT_INVALID_FORMAT;
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    return returnValue;
} // DtcpCert_GetAPFlag

int DtcpCert_GetDeviceId(unsigned char *aCert, unsigned char aDeviceId[DTCP_DEVICE_ID_SIZE])
{
    int returnValue = SUCCESS;

    if ((0 != aCert) && (0 != aDeviceId))
    {
        memcpy(aDeviceId, &aCert[3], DTCP_DEVICE_ID_SIZE);
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    return returnValue;
} // DtcpCert_GetDeviceID

int DtcpCert_GetPublicKey(unsigned char *aCert, unsigned char aPublicKey[PUBLIC_KEY_SIZE])
{
    int returnValue = SUCCESS;
    unsigned int format;

    if ((0 != aCert) && (0 != aPublicKey))
    {
        DtcpCert_GetFormat(aCert, &format);

        if ((certFormatBaselineFull == format) ||
            (certFormatExtendedFull == format))
        {
            memcpy(aPublicKey, &aCert[8], PUBLIC_KEY_SIZE);
        }
        else
        {
            returnValue = DTCP_CERT_INVALID_FORMAT;
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    return returnValue;
} // DtcpCert_GetPublicKey

int DtcpCert_GetSignature(unsigned char *aCert, unsigned char aSignature[SIGNATURE_SIZE])
{
    int returnValue = SUCCESS;
    unsigned int format;

    if ((0 != aCert) && (0 != aSignature))
    {
        DtcpCert_GetFormat(aCert, &format);

        if (certFormatBaselineFull == format)
        {
            memcpy(aSignature, &aCert[48], SIGNATURE_SIZE);
        }
        else if (certFormatExtendedFull == format)
        {
            memcpy(aSignature, &aCert[92], SIGNATURE_SIZE);
        }
        else if (certFormatRestricted == format)
        {
            memcpy(aSignature, &aCert[8], SIGNATURE_SIZE);
        }
        else
        {
            returnValue = DTCP_CERT_INVALID_FORMAT;
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    return returnValue;
} // DtcpCert_GetSignature

int DtcpCert_GetDeviceCapMask(unsigned char *aCert, unsigned int *aDeviceCapMask)
{
    int returnValue = SUCCESS;
    unsigned int format;        

    if ((0 != aCert) && (0 != aDeviceCapMask))
    {
        DtcpCert_GetFormat(aCert, &format);

        if (certFormatExtendedFull == format)
        {
            *aDeviceCapMask = 0;
            *aDeviceCapMask = aCert[91] & 0x0F;
        }
        else
        {
            returnValue = DTCP_CERT_INVALID_FORMAT;
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    return returnValue;
} // DtcpCert_GetDeviceCapMask

int DtcpCert_GetKeySelectionVector(unsigned char *aCert, unsigned int *aKeySelectionVector)
{
    int returnValue = SUCCESS;
    unsigned int format;        

    if ((0 != aCert) && (0 != aKeySelectionVector))
    {
        DtcpCert_GetFormat(aCert, &format);

        if (certFormatRestricted == format)
        {
            *aKeySelectionVector = 0;
            *aKeySelectionVector = ((aCert[1] & 0x0F) << 8) && (aCert[2]);
        }
        else
        {
            returnValue = DTCP_CERT_INVALID_FORMAT;
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    return returnValue;    
} // DtcpCert_GetKeySelectionVector
