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

#ifndef _DTCP_APP_LIB_I_H_
#define _DTCP_APP_LIB_I_H_

#include "DtcpCoreTypes.h"

/********************
 * Common functions *
 ********************/

int DtcpAppLib_InitializeDeviceParams(DeviceParams *aDeviceParams);

int DtcpAppLib_LoadSrm(char *aSrm, int *aSrmSize);
int DtcpAppLib_UpdateSrm(unsigned char *aSrm, unsigned int aSrmSize);

int DtcpAppLib_LoadRngSeed(char *aRngSeed, int *aRngSeedSize);
int DtcpAppLib_UpdateRngSeed(unsigned char *aRngSeed, unsigned int aRngSeedSize);
int DtcpAppLib_ExchangeKeyUpdateRequest();

int DtcpAppLib_LoadCert(char *aCert, int *aCertSize);

int DtcpAppLib_LoadPrivateKey(char *aPrivateKey, int *aPrivateKeySize);

int DtcpAppLib_LoadBytesFromFile(const char *aFilename, char *aDest, int *aDestSize);
int DtcpAppLib_SaveBytesToFile(const char *aFilename, unsigned char *aSrc, unsigned int aSrcSize);

/********************
 * Source functions *
 ********************/

/******************
 * Sink functions *
 ******************/

#endif /* defined _DTCP_APP_LIB_I_H_ */
