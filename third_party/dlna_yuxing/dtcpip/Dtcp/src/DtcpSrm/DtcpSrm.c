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
#include <stdlib.h>
#include "DtcpSrm.h"
#include "DtcpStatusCodes.h"
#include "DtcpUtils.h"
#include "DtcpCore.h"

/// \file
/// \brief Implementation file for the \ref DtcpSrm library.
///
/// This file implements the interface for the \ref DtcpSrm library.

#ifdef _MSC_VER
typedef unsigned __int64 int64;
#else /* assuming GCC */
typedef unsigned long long int64;
#endif

static void GetCrlInfo(unsigned char aCrlEntryTypeBlock, unsigned int *aCrlEntryType, unsigned int *aCrlEntryCount)
{
    *aCrlEntryType = (aCrlEntryTypeBlock & 0xE0) >> 5;
    *aCrlEntryCount = (aCrlEntryTypeBlock & 0x1F);
    return;
}

static int GetCrlSize(unsigned char *aCrl, unsigned int *aCrlSize)
{
    int returnValue = SUCCESS;

    if ((NULL != aCrl) && (NULL != aCrlSize))
    {
        *aCrlSize = aCrl[0] << 8 | aCrl[1];
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // GetCrlSize

static int GetCrlEntriesPtr(unsigned char *aCrl, unsigned char **aCrlEntriesPtr)
{
    int returnValue = SUCCESS;

    if ((NULL != aCrl) && (NULL != aCrlEntriesPtr))
    {        
        *aCrlEntriesPtr = aCrl + DTCP_SRM_CRL_LENGTH_SIZE;
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    return returnValue;
} // GetCrlEntriesPtr

static int GetSrmGenSize(unsigned char *aSrm, 
                         unsigned int aSrmSize, 
                         unsigned int aSrmGen,
                         unsigned int *aSize)
{
    int returnValue = SUCCESS;
    unsigned int firstGenCrlSize  = 0;
    unsigned int secondGenCrlSize = 0;
    if ((NULL == aSrm)                  ||
        (aSrmSize > DTCP_SRM_MAX_SIZE)  ||
        (aSrmGen > DTCP_SRM_SECOND_GEN) ||
        (NULL == aSize))
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        GetCrlSize(aSrm + DTCP_SRM_HEADER_SIZE, &firstGenCrlSize);

        if (DTCP_SRM_FIRST_GEN == aSrmGen)
        {
            *aSize = DTCP_SRM_HEADER_SIZE + firstGenCrlSize;
        }
        else if (DTCP_SRM_SECOND_GEN == aSrmGen)
        {
            GetCrlSize(aSrm + DTCP_SRM_HEADER_SIZE + firstGenCrlSize, &secondGenCrlSize);
            *aSize = DTCP_SRM_HEADER_SIZE + firstGenCrlSize + secondGenCrlSize;
        }
    }

    return returnValue;
} // GetSrmGenSize

int DtcpSrm_VerifySrm(unsigned char    *aSrm,
                      unsigned int      aSrmSize)
{
    int returnValue = SUCCESS;
    unsigned int srmG = 0;
    unsigned int genSize = 0;
    unsigned int srmC = 0;
    unsigned char *publicKey;
    unsigned int publicKeySize;
    
    // Check that the generation is valid
    returnValue = DtcpSrm_GetSrmGeneration(aSrm, aSrmSize, &srmG);
    if ((DTCP_SRM_FIRST_GEN != srmG) && (DTCP_SRM_SECOND_GEN != srmG))
    {
        // Invalid generation
        returnValue = INVALID_SRM;
    }

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpCore_GetDtlaPublicKey(&publicKey, &publicKeySize);
    }

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpSrm_GetSrmPartCount(aSrm, aSrmSize, &srmC);
    }

    // Check that the signatures are valid
    if (IS_SUCCESS(returnValue))
    {
        // Check part 1
        returnValue = GetSrmGenSize(aSrm, aSrmSize, DTCP_SRM_FIRST_GEN, &genSize);
        if (IS_SUCCESS(returnValue))
        {
            returnValue = DtcpCore_VerifyData(aSrm,
                                              genSize - SIGNATURE_SIZE,
                                              aSrm + genSize - SIGNATURE_SIZE, 
                                              publicKey);
        }

        // Check part 2
        if ((IS_SUCCESS(returnValue)) && (srmC >= DTCP_SRM_SECOND_GEN) && (srmG >= DTCP_SRM_SECOND_GEN))
        {
            returnValue = GetSrmGenSize(aSrm, aSrmSize, DTCP_SRM_SECOND_GEN, &genSize);
            if (IS_SUCCESS(returnValue))
            {
                returnValue = DtcpCore_VerifyData(aSrm,
                                                  genSize - SIGNATURE_SIZE,
                                                  aSrm + genSize - SIGNATURE_SIZE, 
                                                  publicKey);
            }
        }
    }

    if (IS_FAILURE(returnValue))
    {
        returnValue = FAILURE;
    }

    return returnValue;
} // VerifySrm

int DtcpSrm_GetSrmGeneration(unsigned char *aSrm, unsigned int aSrmSize, unsigned int *aSrmG)
{
    int returnValue = SUCCESS;

    if ((NULL != aSrm) && (NULL != aSrmG) && (aSrmSize > DTCP_SRM_HEADER_SIZE))
    {
        *aSrmG = 0;
        *aSrmG = *aSrm & 0x0F;
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    
    return returnValue;
} // DtcpSrm_GetGeneration

int DtcpSrm_GetSrmVersion(unsigned char *aSrm, unsigned int aSrmSize, unsigned int *aSrmV)
{
    int returnValue = SUCCESS;

    if ((NULL != aSrm) && (NULL != aSrmV) && (aSrmSize > DTCP_SRM_HEADER_SIZE))
    {
        *aSrmV = 0;
        *aSrmV = aSrm[2] << 8 | aSrm[3] ;
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    
    return returnValue;
} // DtcpSrm_GetVersion

int DtcpSrm_GetSrmPartCount(unsigned char *aSrm, unsigned int aSrmSize, unsigned int *aSrmC)
{
    int returnValue   = SUCCESS;
    unsigned int srmC = 0;
    unsigned char *current = 0;
    unsigned int crlLength = 0;

    if ((NULL != aSrm) && (NULL != aSrmC) && (aSrmSize > DTCP_SRM_HEADER_SIZE))
    {
        // Check for First-Generation SRM
        current = aSrm + DTCP_SRM_HEADER_SIZE;
        GetCrlSize(current, &crlLength);
        
        if ((crlLength > DTCP_SRM_FIRST_GEN_MAX_SIZE - DTCP_SRM_HEADER_SIZE) ||
            (crlLength < DTCP_SRM_CRL_MIN_SIZE))
        {
            returnValue = INVALID_SRM;
        }

        // Check for Second-Generation SRM
        if (IS_SUCCESS(returnValue))
        {
            current += crlLength;   
            if (current < aSrm + aSrmSize)
            {
                GetCrlSize(current, &crlLength);
                
                if ((crlLength > DTCP_SRM_SECOND_GEN_MAX_SIZE - DTCP_SRM_HEADER_SIZE - DTCP_SRM_CRL_MIN_SIZE) ||
                    (crlLength < DTCP_SRM_CRL_MIN_SIZE))
                {
                    returnValue = INVALID_SRM;
                }
                else
                {
                    ++srmC;
                }
            }
        }
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }
    
    if (IS_SUCCESS(returnValue))
    {
        *aSrmC = srmC;
    }

    return returnValue;
} // DtcpSrm_GetSrmPartCount

int DtcpSrm_IsDeviceRevoked(unsigned char   *aSrm, 
                            unsigned int     aSrmSize,
                            unsigned char    aDeviceId[DTCP_DEVICE_ID_SIZE],
                            unsigned int    *aIsRevokedFlag)
{
    int returnValue = SUCCESS;
    unsigned int isRevoked = FALSE;
    unsigned int crlLength = 0;
    unsigned char *currentCrlPtr = 0;
    unsigned char *endCrlPtr = 0;
    unsigned int crlEntryType = 0;
    unsigned int crlEntryCount = 0;
    unsigned int prevGenSize = 0;
    unsigned int i, j;
    int64 baseDeviceId = 0;
    int64 targetDeviceId = 0;
    unsigned int rangeCount = 0;
    unsigned int srmG = 0;

    if ((NULL != aSrm)                  &&
        (DTCP_SRM_MAX_SIZE >= aSrmSize) && 
        (NULL != aDeviceId)             && 
        (NULL != aIsRevokedFlag))
    {
        returnValue = DtcpSrm_GetSrmGeneration(aSrm, aSrmSize, &srmG);
    }
    else
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        currentCrlPtr = aSrm + DTCP_SRM_HEADER_SIZE;

        for (i = 0; i <= srmG; ++i, currentCrlPtr = endCrlPtr + SIGNATURE_SIZE - DTCP_SRM_CRL_LENGTH_SIZE)
        {
            GetCrlSize(currentCrlPtr, &crlLength);
            currentCrlPtr += DTCP_SRM_CRL_LENGTH_SIZE;
            endCrlPtr = currentCrlPtr + crlLength - SIGNATURE_SIZE;

            while (currentCrlPtr < endCrlPtr)
            {
                GetCrlInfo(*currentCrlPtr, &crlEntryType, &crlEntryCount);
                ++currentCrlPtr;
                if (0 == crlEntryType)
                {
                    for(j = 0; j < crlEntryCount; ++j)
                    {
                        if (0 == memcmp(currentCrlPtr, aDeviceId, DTCP_DEVICE_ID_SIZE))
                        {
                            isRevoked = TRUE;
                            currentCrlPtr += DTCP_DEVICE_ID_SIZE;
                            break;
                        }
                        currentCrlPtr += DTCP_DEVICE_ID_SIZE;
                    }
                }
                else if (1 == crlEntryType)
                {
                    baseDeviceId = 0;
                    targetDeviceId= 0;

                    for (j = 0; j < crlEntryCount; ++j)
                    {
                        memcpy(&baseDeviceId, currentCrlPtr, DTCP_DEVICE_ID_SIZE);
                        SwapInPlace(&baseDeviceId, 8);
                        baseDeviceId = baseDeviceId >> (64 - DTCP_DEVICE_ID_SIZE_BITS);

                        memcpy(&targetDeviceId, aDeviceId, DTCP_DEVICE_ID_SIZE);
                        SwapInPlace(&targetDeviceId, 8);
                        targetDeviceId = targetDeviceId >> (64 - DTCP_DEVICE_ID_SIZE_BITS);

                        currentCrlPtr += DTCP_DEVICE_ID_SIZE;
                        rangeCount = currentCrlPtr[0] << 8 | currentCrlPtr[1];
                        currentCrlPtr += 2;

                        if (targetDeviceId - baseDeviceId <= rangeCount)
                        {
                            isRevoked = TRUE;
                            break;
                        }
                    }
                }
                else
                {
                    // error!  Invalid crl entry type
                    returnValue = INVALID_ARGUMENT;
                }
                if ((SUCCESS != returnValue) || (TRUE == isRevoked))
                {
                    break;
                }
            }
        }
    }

    if (IS_SUCCESS(returnValue))
    {
        *aIsRevokedFlag = isRevoked;
    }

    return returnValue;
} // DtcpSrm_IsDeviceRevoked

int DtcpSrm_UpdateSrm(unsigned char    *aOurSrm,
                      unsigned int      aOurSrmSize,
                      unsigned char    *aTheirSrm,
                      unsigned int      aTheirSrmSize)
{
    int returnValue         = SUCCESS;
    int ourVersionNumber    = 0;
    int theirVersionNumber  = 0;
    unsigned int updateSize = 0;

    if ((NULL == aOurSrm)     ||
        (NULL == aTheirSrm))
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        // Verify the integrity of their Srm
        returnValue = DtcpSrm_VerifySrm(aTheirSrm, aTheirSrmSize);
    }

    if (IS_SUCCESS(returnValue))
    {
        // Verify version number is more recent than current version
        DtcpSrm_GetSrmVersion(aOurSrm, aOurSrmSize, &ourVersionNumber);
        DtcpSrm_GetSrmVersion(aTheirSrm, aTheirSrmSize, &theirVersionNumber);

        if ((theirVersionNumber > ourVersionNumber) ||
            ((theirVersionNumber == ourVersionNumber) && (aTheirSrmSize > aOurSrmSize)))
        {
            returnValue = DtcpCore_SetSrm(aTheirSrm, aTheirSrmSize);
        }
        else
        {
            returnValue = SRM_UPDATE_FAILED;
        }
    }

    return returnValue;
} // DtcpSrm_UpdateSrm

int DtcpSrm_IsTheirSrmCurrent(unsigned char *aOurSrm,
                              unsigned int   aOurSrmSize,
                              unsigned int   aTheirSrmG,
                              unsigned int   aTheirSrmV,
                              unsigned int   aTheirSrmC,
                              unsigned int  *aSrmUpdateSize)
{
    int returnValue = SUCCESS;
    int isUpdateRequired = TRUE;
    unsigned int ourSrmV = 0;
    unsigned int ourSrmC = 0;

    if ((NULL == aOurSrm)  || 
        (NULL == aSrmUpdateSize) ||
        (DTCP_SRM_MAX_SIZE < aOurSrmSize) ||
        (DTCP_SRM_SECOND_GEN < aTheirSrmG) ||
        (DTCP_SRM_SECOND_GEN < aTheirSrmC))
    {
        returnValue = INVALID_ARGUMENT;
    }

    *aSrmUpdateSize = 0;
    returnValue = DtcpSrm_GetSrmVersion(aOurSrm, aOurSrmSize, &ourSrmV);
    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpSrm_GetSrmPartCount(aOurSrm, aOurSrmSize, &ourSrmC);
    }

    if (IS_SUCCESS(returnValue))
    {
        // Compare SRM version numbers
        if (ourSrmV == aTheirSrmV)
        {
            // Compare SRM part counts
            if (ourSrmC > aTheirSrmC)
            {
                // Compare their SRM part capacity
                if (aTheirSrmC == aTheirSrmG)
                {
                    isUpdateRequired = FALSE;
                }
            }
            else
            {
                isUpdateRequired = FALSE;
            }
        }
        else if (ourSrmV < aTheirSrmV)
        {   
            isUpdateRequired = FALSE;
        }   
    }

    if ((IS_SUCCESS(returnValue)) && (TRUE == isUpdateRequired))
    {
        // Compare our SRM part count with their part capacity
        if (ourSrmC > aTheirSrmG)
        {
            // Update their SRM up to aTheirSrmG from our SRM
            returnValue = GetSrmGenSize(aOurSrm, aOurSrmSize, aTheirSrmG, aSrmUpdateSize);
        }
        else
        {
            // Update their SRM with our SRM
            *aSrmUpdateSize = aOurSrmSize;
        }
    }

    return returnValue;
} // DtcpSrm_IsTheirSrmCurrent

int DtcpSrm_IsOurSrmCurrent(unsigned char *aOurSrm,
                            unsigned int   aOurSrmSize,
                            unsigned int   aTheirSrmG,
                            unsigned int   aTheirSrmV,
                            unsigned int   aTheirSrmC,
                            unsigned int  *aOurSrmUpdateRequiredFlag)
{
    int returnValue = SUCCESS;
    int isUpdateRequired = FALSE;
    unsigned int ourSrmV = 0;
    unsigned int ourSrmC = 0;
    unsigned int ourSrmG = 0;

    if ((NULL == aOurSrm)  || 
        (NULL == aOurSrmUpdateRequiredFlag) ||
        (DTCP_SRM_MAX_SIZE < aOurSrmSize) ||
        (DTCP_SRM_SECOND_GEN < aTheirSrmG) ||
        (DTCP_SRM_SECOND_GEN < aTheirSrmC))
    {
        returnValue = INVALID_ARGUMENT;
    }

    *aOurSrmUpdateRequiredFlag = FALSE;
    returnValue = DtcpSrm_GetSrmVersion(aOurSrm, aOurSrmSize, &ourSrmV);
    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpSrm_GetSrmPartCount(aOurSrm, aOurSrmSize, &ourSrmC);
    }
    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpSrm_GetSrmGeneration(aOurSrm, aOurSrmSize, &ourSrmG);
    }

    if (IS_SUCCESS(returnValue))
    {
        // Compare SRM version numbers
        if (ourSrmV < aTheirSrmV)
        {
            isUpdateRequired = TRUE;
        }
        else if (ourSrmV == aTheirSrmV)
        {
            // Compare SRM part counts
            if (aTheirSrmC > ourSrmC)
            {
                // Compare our SRM part capacity
                if (ourSrmC < ourSrmG)
                {
                    isUpdateRequired = TRUE;
                }
            }
        }
        *aOurSrmUpdateRequiredFlag = isUpdateRequired;
    }

    return returnValue;
} // DtcpSrm_IsOurSrmCurrent