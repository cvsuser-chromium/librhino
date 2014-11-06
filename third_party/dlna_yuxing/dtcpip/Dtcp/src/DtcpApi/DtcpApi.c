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

#include "DtcpApi.h"
#include "DtcpCore.h"
#include "DtcpAkeIp.h"
#include "DtcpStatusCodes.h"
#include "DtcpContentManagement.h"
#include "Rng.h"

/// \file 
/// \brief Implementation file for the \ref DtcpApi library
///
/// This file implements the interface for the \ref DtcpApi library.

/// \addtogroup DtcpApi
/// @{

int DtcpApi_Startup(DeviceParams                 *aDeviceParams,
                    unsigned char                 aRngSeed[DTCP_AES_BLOCK_SIZE],
                    ExchangeKeyUpdateRequest_Ptr  aExchangeKeyUpdateRequestFunc,
                    UpdateSrm_Ptr                 aUpdateSrmFunc,
                    UpdateRngSeed_Ptr             aUpdateRngSeedFunc)
{
    int returnValue = SUCCESS;
    
    returnValue = DtcpCore_Startup(aDeviceParams,
                                   aUpdateSrmFunc);

    if (IS_SUCCESS(returnValue))
    {
        returnValue = Rng_Initialize(aRngSeed, aUpdateRngSeedFunc);
    }

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpAkeIp_Startup(aExchangeKeyUpdateRequestFunc);
    }

    if (IS_FAILURE(returnValue))
    {
        DtcpAkeIp_Shutdown();
        DtcpCore_Shutdown();
    }

    return returnValue;
} // DtcpApi_Startup

void DtcpApi_Shutdown(void)
{
    DtcpAkeIp_Shutdown();
    DtcpCore_Shutdown();
} // DtcpApi_Shutdown

int DtcpApi_ClosePacket(DtcpPacketHandle aPacketHandle)
{
    return DtcpContent_ClosePacket(aPacketHandle);
} // DtcpApi_ClosePacket

int DtcpApi_OpenStream(EnStreamTransport  aStreamTransport,
                       DtcpStreamHandle  *aStreamHandle)
{
    int returnValue = SUCCESS;

    if (!aStreamHandle)
    {
        returnValue = INVALID_ARGUMENT;
    }

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpContent_OpenStream(aStreamTransport,
                                             aStreamHandle);
    }

    return returnValue;
} // DtcpApi_OpenStream

int DtcpApi_CloseStream(DtcpStreamHandle  aStreamHandle)
{
    return DtcpContent_CloseStream(aStreamHandle);
} // DtcpApi_CloseStream


int DtcpApi_CreatePacketHeader(DtcpStreamHandle  aStreamHandle,
                               EnExtendedEmi     aExtendedEmi,
                               unsigned int      aContentSize,
                               unsigned char     aPacketHeader[DTCP_CONTENT_PACKET_HEADER_SIZE],
                               DtcpPacketHandle *aPacketHandle)
{
    int returnValue = SUCCESS;

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpContent_CreatePacketHeader(aStreamHandle,
                                                     aExtendedEmi,
                                                     aContentSize,
                                                     aPacketHeader,
                                                     aPacketHandle);
    }

    return returnValue;
} // DtcpApi_CreatePacketHeader

int DtcpApi_ConsumePacketHeader(DtcpAkeHandle     aAkeHandle,
                                unsigned char     aPacketHeader[DTCP_CONTENT_PACKET_HEADER_SIZE],
                                EnExtendedEmi    *aExtendedEmi,
                                unsigned int     *aContentSize,
                                DtcpPacketHandle *aPacketHandle)
{
    int returnValue = SUCCESS;
    DtcpAkeCoreSessionHandle akeCoreSessionHandle;

    returnValue = DtcpAkeIp_GetAkeCoreSessionHandle(aAkeHandle, &akeCoreSessionHandle);

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpContent_ConsumePacketHeader(akeCoreSessionHandle,
                                                      aPacketHeader,
                                                      aExtendedEmi,
                                                      aContentSize,
                                                      aPacketHandle);
    }
    return returnValue;
} // DtcpApi_ConsumePacketHeader

int DtcpApi_EncryptData(DtcpPacketHandle  aPacketHandle, 
                        unsigned char    *aInBuffer,
                        unsigned char    *aOutBuffer,
                        unsigned int      aSize)
{
    int returnValue = SUCCESS;

    returnValue = DtcpContent_EncryptData(aPacketHandle,
                                          aInBuffer,
                                          aOutBuffer,
                                          aSize);

    return returnValue;
} // DtcpApi_EncryptData

int DtcpApi_DecryptData(DtcpPacketHandle  aPacketHandle, 
                        unsigned char    *aInBuffer,
                        unsigned char    *aOutBuffer,
                        unsigned int      aSize)
{
    int returnValue = SUCCESS;

    returnValue = DtcpContent_DecryptData(aPacketHandle,
                                          aInBuffer,
                                          aOutBuffer,
                                          aSize);

    return returnValue;
} // DtcpApi_DecryptData

int DtcpApi_ListenIp(char          *aSourceAkeIpAddress,
                     unsigned int  *aSourceAkePortNumber,
                     DtcpAkeHandle *aListenHandle)
{
    int returnValue = SUCCESS;

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpAkeIp_Listen(aSourceAkeIpAddress,
                                       aSourceAkePortNumber,
                                       aListenHandle);
    }

    return returnValue;
} // DtcpApi_ListenIp

int DtcpApi_CancelListen(DtcpAkeHandle aListenHandle)
{
    int returnValue = SUCCESS;

    returnValue = DtcpAkeIp_CancelListen(aListenHandle);

    return returnValue;
} // DtcpApi_CancelListen

int DtcpApi_OpenAkeIp(char *aSourceAkeIpAddr, unsigned int aSourceAkePortNumber, DtcpAkeHandle *aAkeHandle)
{
    int returnValue = SUCCESS;

    returnValue = DtcpAkeIp_OpenAke(aSourceAkeIpAddr, aSourceAkePortNumber, aAkeHandle);

    return returnValue;
} // DtcpApi_OpenAkeIp

int DtcpApi_DoAke(DtcpAkeHandle aAkeHandle)
{
    int returnValue = SUCCESS;

    returnValue = DtcpAkeIp_DoAke((DtcpAkeIpSessionHandle)aAkeHandle);

    return returnValue;
} // DtcpApi_DoAke

int DtcpApi_CloseAke(DtcpAkeHandle aAkeHandle)
{
    int returnValue = SUCCESS;

    returnValue = DtcpAkeIp_CloseAke((DtcpAkeIpSessionHandle)aAkeHandle);

    return returnValue;
} // DtcpApi_CloseAke

int DtcpApi_UpdateExchangeKey(unsigned int *aExchangeKeyLabel)
{
    int returnValue = SUCCESS;

    returnValue = DtcpAkeIp_UpdateExchangeKey(aExchangeKeyLabel);

    return returnValue;
} // DtcpApi_UpdateExchangeKey

int DtcpApi_ValidateExchangeKey(DtcpAkeHandle  aAkeHandle,
                                int            aExchangeKeyLabel,
                                int           *aExchangeKeyValidFlag)
{
    int returnValue = SUCCESS;
    DtcpAkeIpSessionHandle akeIpSessionHandle = (DtcpAkeIpSessionHandle)aAkeHandle;

    returnValue = DtcpAkeIp_ValidateExchangeKey(akeIpSessionHandle,
                                                aExchangeKeyLabel,
                                                aExchangeKeyValidFlag);

    return returnValue;
} // DtcpApi_ValidateExchangeKey

int DtcpApi_GetSourceExchangeKeyLabel(unsigned int *aExchangeKeyLabel)
{
    return DtcpAkeIp_GetSourceExchangeKeyLabel(aExchangeKeyLabel);
}

int DtcpApi_CheckSourceSinkLimit(char *aSourceIpAddr, unsigned int aSourcePortNumber)
{
    return DtcpAkeIp_CheckSourceSinkLimit(aSourceIpAddr, aSourcePortNumber);
}
/// @}
