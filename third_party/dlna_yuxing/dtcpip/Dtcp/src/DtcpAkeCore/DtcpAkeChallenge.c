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
#include "DtcpAkeChallenge.h"
#include "DtcpAkeCore.h"
#include "DtcpAkeCoreInternal.h"
#include "DtcpCert.h"
#include "DtcpCore.h"
#include "DtcpStatusCodes.h"
#include "Rng.h"

/// \file 
/// \brief Implements functions for creating and consuming data for the challenge command of the AKE
///
/// This header file is part of the \ref DtcpAkeCore library.

#if 0
static int GlobalDisplayLevel             = 0;
static int GlobalDisplayLevelMatchExactly = 0;
static int GlobalLogToFile                = 0;
#endif

int DtcpChallenge_CreateFullAuthData(DtcpAkeCoreSessionData *aAkeSessionData,
                                     unsigned char aChallengeCmdBuffer[DTCP_FULL_AUTH_CHALLENGE_SIZE])
{
    int returnValue = SUCCESS;
    unsigned char nonce[DTCP_FULL_AUTH_NONCE_SIZE];
    unsigned char *cert;
    unsigned int  certSize;

    returnValue = Rng_GetRandomNumber(nonce, DTCP_FULL_AUTH_NONCE_SIZE);

    if (IS_SUCCESS(returnValue))
    {
        aAkeSessionData->OurNonceSize = DTCP_FULL_AUTH_NONCE_SIZE;

        memcpy(aAkeSessionData->OurNonce, nonce, DTCP_FULL_AUTH_NONCE_SIZE);    
        memcpy(aChallengeCmdBuffer, nonce, DTCP_FULL_AUTH_NONCE_SIZE);

        returnValue = DtcpCore_GetCert(&cert, &certSize);
    }

    if (SUCCESS == returnValue)
    {
        memcpy(aChallengeCmdBuffer + DTCP_FULL_AUTH_NONCE_SIZE,
               cert,
               certSize);
    }

    return returnValue;
} // DtcpChallenge_CreateFullAuthData

int DtcpChallenge_ConsumeFullAuthData(DtcpAkeCoreSessionData *aAkeSessionData,
                                      unsigned char aChallengeCmdBuffer[DTCP_FULL_AUTH_CHALLENGE_SIZE])
{
    int returnValue = SUCCESS;
    int isCertValid = FALSE;
    unsigned char *theirCert;
    unsigned char  theirPublicKey[PUBLIC_KEY_SIZE];

    theirCert = aChallengeCmdBuffer + DTCP_FULL_AUTH_NONCE_SIZE;
    isCertValid = DtcpCert_VerifyCert(theirCert);
    if (IS_FAILURE(isCertValid))
    {
        returnValue = DTCP_OTHER_DEVICE_CERTIFICATE_INVALID;
        DEBUG_MSG(MSG_INFO, ("Device Certificate Invalid.  AKE Aborted.\r\n") );
    }

    if (IS_SUCCESS(returnValue))
    {
        // Save their AP flag value
        returnValue = DtcpCert_GetApFlag(theirCert, &aAkeSessionData->TheirApFlag);
    }

    if (SUCCESS == returnValue)
    {
        returnValue = DtcpCert_GetDeviceId(theirCert, aAkeSessionData->DeviceId);
        if (deviceModeSource == aAkeSessionData->DeviceMode)
        {
            // Check that sink count limit has not yet been reached
            if (IS_SUCCESS(returnValue))
            {
                returnValue = CheckSinkCountLimit(aAkeSessionData->AkeCoreData, 
                                                  aAkeSessionData->DeviceId,
                                                  aAkeSessionData->TheirApFlag);
            }

            if (DTCP_SINK_COUNT_LIMIT_REACHED == returnValue)
            {
                returnValue = aAkeSessionData->AkeCoreData->ExchangeKeyUpdateRequestFunc();
                if (IS_FAILURE(returnValue))
                {
                    returnValue = DTCP_SINK_COUNT_LIMIT_REACHED;
                    DEBUG_MSG(MSG_INFO, ("Device sink count reached.  AKE Aborted.\r\n") );
                }
            }
        }
    }

    if (IS_SUCCESS(returnValue))
    {
        // Save their SrmG value
        returnValue = DtcpCert_GetDeviceGen(theirCert, &aAkeSessionData->TheirSrmG);
    }

    if (IS_SUCCESS(returnValue))
    {
        memcpy(aAkeSessionData->TheirNonce, aChallengeCmdBuffer, DTCP_FULL_AUTH_NONCE_SIZE);    
        aAkeSessionData->TheirNonceSize = DTCP_FULL_AUTH_NONCE_SIZE;

        DtcpCert_GetPublicKey(theirCert, theirPublicKey);
        memcpy(aAkeSessionData->TheirPublicKey, theirPublicKey, PUBLIC_KEY_SIZE);
    }    

    if (deviceModeSink == aAkeSessionData->DeviceMode)
    {
        DtcpCore_GetFirstPhaseValue(aAkeSessionData->FirstPhaseValue,
                                    aAkeSessionData->FirstPhaseSecret);
    }

    return returnValue;
} // DtcpChallenge_ConsumeFullAuthData

int DtcpChallenge_CreateRestrictedAuthData(DtcpAkeCoreSessionData *aAkeSessionData,
                                           unsigned char aChallengeData[DTCP_RESTRICTED_AUTH_CHALLENGE_SIZE])
{
    int returnValue = SUCCESS;
    unsigned char *ksvPtr;
    unsigned int ksv = 0;

//    returnValue = DtcpCore_GetKsv(aAkeSessionData->AkeCoreData->DeviceHandle, &ksv);
    if (SUCCESS == returnValue) 
    {
        ///todo Get random nonce
        //SYSWrap_GetRandomNumber((unsigned char *)&aAkeSessionData->OurNonce,
        //                        DTCP_RESTRICTED_AUTH_NONCE_SIZE_BITS);

        memcpy(aChallengeData,
               &aAkeSessionData->OurNonce,
               DTCP_RESTRICTED_AUTH_NONCE_SIZE);

        ksvPtr = aChallengeData + DTCP_RESTRICTED_AUTH_NONCE_SIZE;
        ksvPtr[0] = 0x0F & (ksv >> 8);
        ksvPtr[1] = 0xFF & ksv;
    }
    return returnValue;
} // DtcpChallenge_CreateRestrictedAuthData

int DtcpChallenge_ConsumeRestrictedAuthData(DtcpAkeCoreSessionData *aAkeSessionData,
                                            unsigned char aChallengeData[DTCP_RESTRICTED_AUTH_CHALLENGE_SIZE])
{
    int returnValue = SUCCESS;
    unsigned char *ksvPtr;
    unsigned int ksv = 0;
    unsigned char *theirCert;
    int isCertValid;

    theirCert = aChallengeData + DTCP_RESTRICTED_AUTH_NONCE_SIZE;
    isCertValid = DtcpCert_VerifyCert(theirCert);
    if (FALSE == isCertValid)
    {
        returnValue = DTCP_OTHER_DEVICE_CERTIFICATE_INVALID;
    }

    if (SUCCESS == returnValue)
    {
        ksvPtr = aChallengeData + DTCP_FULL_AUTH_NONCE_SIZE;

        ksv = (ksvPtr[0] & 0x0F) | (ksvPtr[1]);
        aAkeSessionData->TheirKsv = ksv;

        SwapInPlace(&aAkeSessionData->TheirKsv, sizeof(aAkeSessionData->TheirKsv));

        // Verify KeySelectionVector
        //if (0 == DtcpCore_VerifyKsv(aAkeSessionData->TheirKsv))
        //{
        //    returnValue = DTCP_INVALID_DATA_FIELD;
        //}
    }

    return returnValue;
} // DtcpChallenge_ConsumeRestrictedAuthData

int DtcpChallenge_CreateEnhRestrictedAuthSinkData(DtcpAkeCoreSessionData *aAkeSessionData,
                                                  unsigned char aChallengeData[DTCP_ENH_RESTRICTED_AUTH_CHALLENGE_SINK_SIZE])
{
    int returnValue = SUCCESS;
    unsigned char *cert;
    unsigned int  certSize;

    ///todo Get random nonce
    //SYSWrap_GetRandomNumber((unsigned char *)&aAkeSessionData->OurNonce,
    //                        DTCP_RESTRICTED_AUTH_NONCE_SIZE_BITS);

    memcpy(aChallengeData,
           &aAkeSessionData->OurNonce,
           DTCP_RESTRICTED_AUTH_NONCE_SIZE);

    // Get our device certificate
    returnValue = DtcpCore_GetCert(&cert, &certSize);
    if (SUCCESS == returnValue)
    {
        memcpy(aChallengeData + DTCP_RESTRICTED_AUTH_NONCE_SIZE,
               cert,
               certSize);
    }

    return returnValue;
} // DtcpChallenge_CreateEnhRestrictedAuthSinkData

int DtcpChallenge_ConsumeEnhRestrictedAuthSinkData(DtcpAkeCoreSessionData *aAkeSessionData,
                                                   unsigned char aChallengeData[DTCP_ENH_RESTRICTED_AUTH_CHALLENGE_SINK_SIZE])
{
    int returnValue = SUCCESS;
    int certValid = FALSE;
    unsigned char *ksvPtr;
    unsigned int ksv = 0;

    // Verify certificate
    certValid = DtcpCert_VerifyCert(aChallengeData + DTCP_RESTRICTED_AUTH_NONCE_SIZE);
    if (FALSE == certValid)
    {
        returnValue = DTCP_OTHER_DEVICE_CERTIFICATE_INVALID;
    }

    if (SUCCESS == returnValue)
    {
        ///todo Make sure source device's sink list is being updated for all auth types

        ///todo Examine SRM

        // Store their nonce and ksv
        memcpy(&aAkeSessionData->TheirNonce,
               aChallengeData,
               DTCP_RESTRICTED_AUTH_NONCE_SIZE);

        ksvPtr = aChallengeData + DTCP_FULL_AUTH_NONCE_SIZE;

        ksv = (ksvPtr[0] & 0x0F) | (ksvPtr[1]);
        aAkeSessionData->TheirKsv = ksv;

        SwapInPlace(&aAkeSessionData->TheirKsv, sizeof(aAkeSessionData->TheirKsv));

        // Verify KeySelectionVector
        //if (0 == DtcpCore_VerifyKsv(aAkeSessionData->TheirKsv))
        //{
        //    returnValue = DTCP_INVALID_DATA_FIELD;
        //}
    }    

    return returnValue;
} // DtcpChallenge_ConsumeEnhRestrictedAuthSinkData

int DtcpChallenge_CreateEnhRestrictedAuthSourceData(DtcpAkeCoreSessionData *aAkeSessionData,
                                                    unsigned char aChallengeData[DTCP_RESTRICTED_AUTH_CHALLENGE_SIZE])
{
    return DtcpChallenge_CreateRestrictedAuthData(aAkeSessionData, aChallengeData);
} // DtcpChallenge_CreateEnhRestrictedAuthSourceData

int DtcpChallenge_ConsumeEnhRestrictedAuthSourceData(DtcpAkeCoreSessionData *aAkeSessionData,
                                                     unsigned char aChallengeData[DTCP_RESTRICTED_AUTH_CHALLENGE_SIZE])
{
    return DtcpChallenge_ConsumeRestrictedAuthData(aAkeSessionData, aChallengeData);
} // DtcpChallenge_ConsumeEnhRestrictedAuthSourceData
