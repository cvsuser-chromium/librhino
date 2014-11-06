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

#ifndef __DTCP_CHALLENGE_H__
#define __DTCP_CHALLENGE_H__

/// \file 
/// \brief Defines functions for creating and consuming data for the challenge command of the AKE
///
/// This header file is part of the \ref DtcpAkeCore library.

#include "DtcpConstants.h"
#include "DtcpAkeCoreInternal.h"

/// \brief Creates the command data for a challenge command when doing a full authentication.
///
/// \param aAkeSessionData - Input; Pointer to data for this session
/// \param aChallengeBuffer - Input/Output; Upon successful completion, buffer will be filled
///        with appropriate data
int DtcpChallenge_CreateFullAuthData(DtcpAkeCoreSessionData *aAkeSessionData,
                                     unsigned char aChallengeBuffer[DTCP_FULL_AUTH_CHALLENGE_SIZE]);

/// \brief Consumes the command data for a challenge command when doing a full authentication
///
/// \param aAkeSessionData - Input; Pointer to data for this session
/// \param aChallengeBuffer - Input; Buffer filled with data from the other device's challenge command
int DtcpChallenge_ConsumeFullAuthData(DtcpAkeCoreSessionData *aAkeSessionData,
                                      unsigned char aChallengeBuffer[DTCP_FULL_AUTH_CHALLENGE_SIZE]);

int DtcpChallenge_CreateRestrictedAuthData(DtcpAkeCoreSessionData *aAkeSessionData,
                                           unsigned char aChallengeData[DTCP_RESTRICTED_AUTH_CHALLENGE_SIZE]);

int DtcpChallenge_ConsumeRestrictedAuthData(DtcpAkeCoreSessionData *aAkeSessionData,
                                            unsigned char aChallengeData[DTCP_RESTRICTED_AUTH_CHALLENGE_SIZE]);

int DtcpChallenge_CreateEnhRestrictedAuthSinkData(DtcpAkeCoreSessionData *aAkeSessionData,
                                                  unsigned char aChallengeData[DTCP_ENH_RESTRICTED_AUTH_CHALLENGE_SINK_SIZE]);

int DtcpChallenge_ConsumeEnhRestrictedAuthSinkData(DtcpAkeCoreSessionData *aAkeSessionData,
                                                   unsigned char aChallengeData[DTCP_ENH_RESTRICTED_AUTH_CHALLENGE_SINK_SIZE]);

int DtcpChallenge_CreateEnhRestrictedAuthSourceData(DtcpAkeCoreSessionData *aAkeSessionData,
                                                    unsigned char aChallengeData[DTCP_RESTRICTED_AUTH_CHALLENGE_SIZE]);

int DtcpChallenge_ConsumeEnhRestrictedAuthSourceData(DtcpAkeCoreSessionData *aAkeSessionData,
                                                     unsigned char aChallengeData[DTCP_RESTRICTED_AUTH_CHALLENGE_SIZE]);


#endif // __DTCP_CHALLENGE_H__
