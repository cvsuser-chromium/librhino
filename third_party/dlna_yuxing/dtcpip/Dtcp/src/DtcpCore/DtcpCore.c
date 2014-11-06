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

#include <stdlib.h>
#include <memory.h>
#include "DtcpCore.h"
#include "DtcpCert.h"
#include "DtcpSrm.h"
#include "DtcpStatusCodes.h"
#include "DtcpCoreInternal.h"
#include "DtcpEcCrypto.h"
#include "OsWrapper.h"

/// \file 
/// \brief Implementation file for the \ref DtcpCore library.
///
/// This file implements the interface for the \ref DtcpCore library.

#if 0
static int GlobalDisplayLevel             = 0;
static int GlobalDisplayLevelMatchExactly = 0;
static int GlobalLogToFile                = 0;
#endif

static DtcpDeviceData gCoreData; ///< Global, static data for library.

int DtcpCore_Startup(DeviceParams      *aDeviceParams,
                     UpdateSrm_Ptr      aUpdateSrmFunc)

{
    int returnValue = SUCCESS;

    if (!aDeviceParams || !aUpdateSrmFunc)
    {
        returnValue = INVALID_ARGUMENT;
    }

    // Baseline sanity checks
    if (IS_SUCCESS(returnValue))
    {
        if ((aDeviceParams->CertSize > DTCP_EXTENDED_FULL_CERT_SIZE) ||
            (aDeviceParams->SrmSize  > DTCP_SRM_MAX_SIZE)            ||
            (0 == aDeviceParams->Cert)                               ||
            (0 == aDeviceParams->Srm))
        {
            returnValue = INVALID_ARGUMENT;
        }
    }

    // Initialize library semaphore
    if (IS_SUCCESS(returnValue))
    {
        gCoreData.DataSemaphore = NULL;
        returnValue = OsWrap_SemaphoreInit(&gCoreData.DataSemaphore, 1);
    }

    // Store device data (certificate, SRM, ECC parameters)
    if (IS_SUCCESS(returnValue))
    {
        OsWrap_SemaphoreWait(gCoreData.DataSemaphore);

        memcpy(&gCoreData.Cert, aDeviceParams->Cert, aDeviceParams->CertSize);
        gCoreData.CertSize = aDeviceParams->CertSize;
        memcpy(&gCoreData.Srm, aDeviceParams->Srm, aDeviceParams->SrmSize);
        gCoreData.SrmSize = aDeviceParams->SrmSize;
        memcpy(&gCoreData.DtlaPublicKey, aDeviceParams->DtlaPublicKey, ECC_PUBLIC_KEY_SIZE);
        memcpy(&gCoreData.PrivateKey, aDeviceParams->PrivateKey, ECC_PRIVATE_KEY_SIZE);
        
        gCoreData.UpdateSrmFunc = aUpdateSrmFunc;

        memcpy(&gCoreData.EccParams, &aDeviceParams->EccParams, sizeof(EccParams));

        OsWrap_SemaphorePost(gCoreData.DataSemaphore);
    }

    // Verify the certificate
    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpCert_VerifyCert(gCoreData.Cert);
        if (DTCP_CERT_INVALID_FORMAT == returnValue)
        {
            returnValue = INVALID_CERTIFICATE;
        }
    }

    // Verify the SRM
    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpSrm_VerifySrm(gCoreData.Srm, gCoreData.SrmSize);
        if (IS_FAILURE(returnValue))
        {
            returnValue = INVALID_SRM;
        }
    }

    if (IS_FAILURE(returnValue))
    {
        DtcpCore_Shutdown();
    }
    return returnValue;
} // DtcpCore_Startup

void DtcpCore_Shutdown()
{
    OsWrap_SemaphoreClose(gCoreData.DataSemaphore);
    gCoreData.DataSemaphore = NULL;

    memset(&gCoreData, 0, sizeof(gCoreData));

    return;
}

int DtcpCore_GetCert(unsigned char **aCert, unsigned int *aCertSize)
{
    int returnValue = SUCCESS;
    
    if (!aCert || !aCertSize)
    {
        returnValue = INVALID_ARGUMENT;
    }
    
    if (IS_SUCCESS(returnValue))
    {
        *aCert     = gCoreData.Cert;
        *aCertSize = gCoreData.CertSize;
    }

    return returnValue;
} // DtcpCore_GetCert

int DtcpCore_GetSrm(unsigned char **aSrm, unsigned int *aSrmSize)
{
    int returnValue = SUCCESS;
    
    if (!aSrm || !aSrmSize)
    {
        returnValue = INVALID_ARGUMENT;
    }
    
    if (IS_SUCCESS(returnValue))
    {
        *aSrm     = gCoreData.Srm;
        *aSrmSize = gCoreData.SrmSize;
    }

    return returnValue;
} // DtcpCore_GetSrm

int DtcpCore_SetSrm(unsigned char *aSrm, unsigned int aSrmSize)
{
    int returnValue = SUCCESS;
    
    if (!aSrm)
    {
        returnValue = INVALID_ARGUMENT;
    }
    
    if (IS_SUCCESS(returnValue))
    {
        if (DTCP_SRM_MAX_SIZE >= aSrmSize)
        {
            OsWrap_SemaphoreWait(gCoreData.DataSemaphore);

            memcpy(gCoreData.Srm, aSrm, aSrmSize);
            gCoreData.SrmSize = aSrmSize;

            OsWrap_SemaphorePost(gCoreData.DataSemaphore);
            gCoreData.UpdateSrmFunc(aSrm, aSrmSize);
        }
        else
        {
            returnValue = INVALID_ARGUMENT;
        }
    }

    return returnValue;
} // DtcpCore_SetSrm

int DtcpCore_GetSupportedAkeProcedures(EnAkeTypeMask *aAkeTypeMask)
{
    int returnValue = SUCCESS;
    unsigned int certFormat;

    if (!aAkeTypeMask)
    {
        returnValue = INVALID_ARGUMENT;
    }
    
    if (IS_SUCCESS(returnValue))
    {
        *aAkeTypeMask = 0;

        returnValue = DtcpCert_GetFormat(gCoreData.Cert, &certFormat);

        // Currently only supports full authentication devices since
        // that is all that DTCP/IP supports.
        if (certFormatBaselineFull == certFormat)
        {
            *aAkeTypeMask = akeTypeMaskFullAuth;
        }
    }
    
    return returnValue;
} // DtcpCore_GetSupportedAkeProcedures

int DtcpCore_GetSupportedExchKeys(EnExchKeyMask *aExchKeyMask)
{
    int returnValue = SUCCESS;
    unsigned int certFormat;

    if (!aExchKeyMask)
    {
        returnValue = INVALID_ARGUMENT;
    }
    
    if (IS_SUCCESS(returnValue))
    {
        *aExchKeyMask = 0;

        returnValue = DtcpCert_GetFormat(gCoreData.Cert, &certFormat);

        // Currently only works for DTCP/IP (AES-128)
        if (certFormatBaselineFull == certFormat)
        {
            *aExchKeyMask = exchKeyMaskAes128;
        }
    }
    
    return returnValue;
} // DtcpCore_GetSupportedExchKeys

int DtcpCore_GetDtlaPublicKey(unsigned char **aDtlaPublicKey, 
                              unsigned int *aDtlaPublicKeySize)
{
    int returnValue = SUCCESS;
    
    if (!aDtlaPublicKey || !aDtlaPublicKeySize)
    {
        returnValue = INVALID_ARGUMENT;
    }
    
    if (IS_SUCCESS(returnValue))
    {
        *aDtlaPublicKey     = gCoreData.DtlaPublicKey;
        *aDtlaPublicKeySize = ECC_PUBLIC_KEY_SIZE;
    }

    return returnValue;
} // DtcpCore_GetDtlaPublicKey


int DtcpCore_SignData(unsigned char *aBuffer, 
                      unsigned int aBufferSize, 
                      unsigned char aSignature[SIGNATURE_SIZE])
{
    int returnValue = SUCCESS;
    int eccReturn = 0;
    
    if (!aSignature || !aBuffer)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        SignData(aSignature, aBuffer, aBufferSize, gCoreData.PrivateKey, &gCoreData.EccParams);
    }
    return returnValue;
} // DtcpCore_SignData

int DtcpCore_VerifyData(unsigned char *aBuffer,
                        unsigned int aBufferSize,
                        unsigned char aSignature[SIGNATURE_SIZE],
                        unsigned char aKey[ECC_PUBLIC_KEY_SIZE])
{
    int returnValue = SUCCESS;
    int eccReturn = 0;
    
    if (!aSignature || !aKey)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        eccReturn = VerifyData(aSignature, aBuffer, aBufferSize, aKey, &gCoreData.EccParams);

    }

    returnValue = ((1 == eccReturn) && (IS_SUCCESS(returnValue))) ? SUCCESS : FAILURE;

    return returnValue;
} // DtcpCore_VerifyData

int DtcpCore_GetFirstPhaseValue(unsigned char aFirstPhaseValue[DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE],
                                unsigned char aFirstPhaseSecret[DTCP_DH_FIRST_PHASE_SECRET_SIZE])
{
    int returnValue = SUCCESS;
    
    if (!aFirstPhaseValue || !aFirstPhaseSecret)
    {
        returnValue = INVALID_ARGUMENT;
    }
    
    if (IS_SUCCESS(returnValue))
    {
        GetFirstPhaseValue(aFirstPhaseValue, aFirstPhaseSecret, &gCoreData.EccParams);
    }

    return returnValue;
} // DtcpCore_GetFirstPhaseValue

int DtcpCore_GetSharedSecret(unsigned char aFirstPhaseValue[DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE],
                             unsigned char aFirstPhaseSecret[DTCP_DH_FIRST_PHASE_SECRET_SIZE],
                             unsigned char aAuthKey[DTCP_AUTH_KEY_SIZE])
{
    int returnValue = SUCCESS;
    
    if (!aFirstPhaseValue || !aFirstPhaseSecret || !aAuthKey)
    {
        returnValue = INVALID_ARGUMENT;
    }
    
    if (IS_SUCCESS(returnValue))
    {
        GetSharedSecret(aAuthKey, aFirstPhaseValue, aFirstPhaseSecret, &gCoreData.EccParams);
    }

    return returnValue;
} // DtcpCore_GetSharedSecret
