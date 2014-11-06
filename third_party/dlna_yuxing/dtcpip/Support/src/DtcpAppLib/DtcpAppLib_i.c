/*
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
*/

#include "DtcpAppLib_i.h"

#include "DtcpConstants.h"
#include "StatusCodes.h"

#include "DtcpApi.h"
#include "DtcpFullAuthDevice.h" /* gFullCert gFullAuthDevicePrivateKey */
#include "DtcpCryptoValues.h"   /* gDtlaPublicKey gEccParams */

#include <stdio.h>
#include <memory.h>
#include <errno.h>

/*****************************************************************************
 * DTCP-IP Common Functions                                                  *
 *****************************************************************************/

#define DTCP_RNG_SEED_FILENAME "dtcp.rng"
#define DTCP_SRM_FILENAME "dtcp.srm"
#define DTCP_CERT_FILENAME "dtcp.crt"
#define DTCP_PVT_KEY_FILENAME "dtcp.pvk"

static unsigned char gRngSeed[DTCP_AES_BLOCK_SIZE];
static unsigned int  gRngSeedSize = DTCP_AES_BLOCK_SIZE;
static unsigned char gSrm[DTCP_SRM_SECOND_GEN_MAX_SIZE];
static unsigned int  gSrmSize = DTCP_SRM_SECOND_GEN_MAX_SIZE;

unsigned char gCert[DTCP_BASELINE_FULL_CERT_SIZE];
unsigned int  gCertSize = DTCP_BASELINE_FULL_CERT_SIZE;
unsigned char gPrivKey[PRIVATE_KEY_SIZE];
unsigned int  gPrivKeySize = PRIVATE_KEY_SIZE;

static DeviceParams gDeviceParams;

int DtcpAppLib_Startup()
{
    int returnValue = LOGIC_ERROR;

    /* Add by Gaocn 2005_6_21 ¼àÊÓÒ»ÏÂ×´Ì¬ */
    DEBUG_MSG_TIME( MSG_INFO, ( " DtcpAppLib_Startup :: ************************* Init DTCP-IP *************************\r\n" ) );
    
    if ( IS_SUCCESS(returnValue = DtcpAppLib_InitializeDeviceParams(&gDeviceParams)) )
    {

        returnValue = DtcpApi_Startup(&gDeviceParams,
                                    gRngSeed,
                                    DtcpAppLib_ExchangeKeyUpdateRequest,
                                    DtcpAppLib_UpdateSrm,
                                    DtcpAppLib_UpdateRngSeed);
    }
    return returnValue;
}

int DtcpAppLib_InitializeDeviceParams(DeviceParams *aDeviceParams)
{
    int returnValue = LOGIC_ERROR;

    do {
        if ( IS_FAILURE(returnValue = DtcpAppLib_LoadRngSeed(gRngSeed, &gRngSeedSize)) )
            break;

        if ( IS_FAILURE(returnValue = DtcpAppLib_LoadSrm(gSrm, &gSrmSize)) )
            break;

        if ( IS_FAILURE(returnValue = DtcpAppLib_LoadCert(gCert, &gCertSize)) )
            break;

        if ( IS_FAILURE(returnValue = DtcpAppLib_LoadPrivateKey(gPrivKey, &gPrivKeySize)) )
            break;

        aDeviceParams->Cert = gCert;
        aDeviceParams->CertSize = gCertSize;
        aDeviceParams->Srm = gSrm;
        aDeviceParams->SrmSize = gSrmSize;
        memcpy(&aDeviceParams->DtlaPublicKey, gDtlaPublicKey, sizeof(gDtlaPublicKey));
        memcpy(&aDeviceParams->PrivateKey, gPrivKey, gPrivKeySize);
        memcpy(&aDeviceParams->EccParams, &gEccParams, sizeof(EccParams));
    } while (0);

    return returnValue;
}

int DtcpAppLib_UpdateRngSeed(unsigned char *aRngSeed, unsigned int aRngSeedSize)
{
    int returnValue = LOGIC_ERROR;    

    if ( DTCP_AES_BLOCK_SIZE != aRngSeedSize )
    {
        returnValue = FAILURE;
    }
    else
    {
        //aRngSeedSize = DTCP_AES_BLOCK_SIZE;
        returnValue = DtcpAppLib_SaveBytesToFile( DTCP_RNG_SEED_FILENAME, 
                                                   aRngSeed, aRngSeedSize);
    }

    return returnValue;
}

int DtcpAppLib_UpdateSrm(unsigned char *aSrm, unsigned int aSrmSize)
{
    int returnValue = LOGIC_ERROR;

    if ( DTCP_SRM_SECOND_GEN_MAX_SIZE < aSrmSize )
    {
        returnValue = FAILURE;
    }
    else
    {
        returnValue = DtcpAppLib_SaveBytesToFile( DTCP_SRM_FILENAME,
                                                   aSrm, aSrmSize);
    }

    return returnValue;
}

int DtcpAppLib_LoadSrm(char *aSrm, int *aSrmSize)
{
    int returnValue = LOGIC_ERROR;

    if ( DTCP_SRM_SECOND_GEN_MAX_SIZE > *aSrmSize )
    {
        returnValue = FAILURE;
    }
    else
    {
        *aSrmSize = DTCP_SRM_SECOND_GEN_MAX_SIZE;
        returnValue = DtcpAppLib_LoadBytesFromFile(DTCP_SRM_FILENAME, aSrm, aSrmSize);
    }

    return returnValue;
}

int DtcpAppLib_LoadRngSeed(char *aRngSeed, int *aRngSeedSize)
{
    int returnValue = LOGIC_ERROR;

    if ( DTCP_AES_BLOCK_SIZE > *aRngSeedSize )
    {
        returnValue = FAILURE;
    }
    else
    {
        *aRngSeedSize = DTCP_AES_BLOCK_SIZE;
        returnValue = DtcpAppLib_LoadBytesFromFile(DTCP_RNG_SEED_FILENAME, aRngSeed, aRngSeedSize);
        if ( IS_SUCCESS(returnValue) && DTCP_AES_BLOCK_SIZE != *aRngSeedSize)
        {
            returnValue = FAILURE;
        }
    }

    return returnValue;
}

int DtcpAppLib_ExchangeKeyUpdateRequest()
{
    int returnValue = LOGIC_ERROR;
    int exchangeKeyLabel;

    returnValue = DtcpApi_UpdateExchangeKey(&exchangeKeyLabel);

    DEBUG_MSG(MSG_DEBUG+2, ("Updating exchange key, new label: %d\n", exchangeKeyLabel));
    
    return returnValue;
}

int DtcpAppLib_LoadCert(char *aCert, int *aCertSize)
{
    int returnValue = LOGIC_ERROR;

    if ( DTCP_BASELINE_FULL_CERT_SIZE > *aCertSize )
    {
        returnValue = FAILURE;
    }
    else
    {
        *aCertSize = DTCP_BASELINE_FULL_CERT_SIZE;
        returnValue = DtcpAppLib_LoadBytesFromFile(DTCP_CERT_FILENAME, aCert, aCertSize);
    }

    return returnValue;
}

int DtcpAppLib_LoadPrivateKey(char *aPrivateKey, int *aPrivateKeySize)
{
    int returnValue = LOGIC_ERROR;

    if ( PRIVATE_KEY_SIZE > *aPrivateKeySize )
    {
        returnValue = FAILURE;
    }
    else
    {
        *aPrivateKeySize = PRIVATE_KEY_SIZE;
        returnValue = DtcpAppLib_LoadBytesFromFile(DTCP_PVT_KEY_FILENAME, aPrivateKey, aPrivateKeySize);
    }

    return returnValue;
}

int DtcpAppLib_LoadBytesFromFile(const char *aFilename, char *aDest, int *aDestSize)
{
    int returnValue = LOGIC_ERROR;
    FILE *fin = NULL;
    int bytesRead = 0;

    if (NULL != aFilename && NULL != aDest && NULL != aDestSize && 0 < *aDestSize)
    {
        fin = fopen(aFilename, "rb");

        if (NULL != fin)
        {
            bytesRead = (int)fread(aDest, 1, (size_t) *aDestSize, fin);
            *aDestSize = bytesRead;
            fclose(fin);
            DEBUG_MSG(MSG_DEBUG+3, ("Read %d bytes from %s.\r\n", bytesRead, aFilename));
            returnValue = SUCCESS;
        }
        else
        {
            int fopen_errno = errno;
            
            // Check for file not found
            if (ENOENT == fopen_errno) {
                returnValue = FILE_NOT_FOUND;
            } else {
                returnValue = FAILURE;
            }
            
            DEBUG_MSG(MSG_DEBUG, ("Unable to open '%s' for reading!\n", aFilename));
            DEBUG_MSG(MSG_DEBUG+1, ("strerror(%d) = %s", fopen_errno, strerror(fopen_errno)));
        }
    }

    return returnValue;
}

int DtcpAppLib_SaveBytesToFile(const char *aFilename, unsigned char *aSrc, unsigned int aSrcSize)
{
    int returnValue = LOGIC_ERROR;
    FILE *fout = NULL;
    int bytesWritten = 0;

    if (NULL != aFilename && NULL != aSrc)
    {
        fout = fopen(aFilename, "wb");

        if (NULL != fout)
        {
            bytesWritten = (int)fwrite(aSrc, 1, (size_t) aSrcSize, fout);
            //ASSERT( bytesWritten == aSrcSize);
            fclose(fout);
            DEBUG_MSG(MSG_DEBUG+3, ("Wrote %d bytes to %s.\r\n", bytesWritten, aFilename));
        }
        else
        {
            returnValue = FAILURE;
            ERROR_MSG(MSG_ERR, returnValue, ("Unable to open %s for writing!\r\n", aFilename));
        }
    }
    return returnValue;
}
