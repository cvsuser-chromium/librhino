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
/// \brief Defines the interface for the \ref DtcpSrm library.
///
/// This header file contains the interface to the \ref DtcpSrm library.  This library
/// contains several functions intended to assist in working with SRMs.

#ifndef __DTCP_SRM_H__
#define __DTCP_SRM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "DtcpConstants.h"
#include "DtcpCore.h"

/// \defgroup DtcpSrm DtcpSrm
/// \brief Provides functions to assist in the processing of SRMs.
///
/// This library is intended to provide functions that assist in the
/// processing of System Renewability Messages (SRM)s.  It provides
/// functions that extract information out of a SRM, check if a
/// particular device has been revoked, and determines whether a
/// SRM needs to be updated.  
/// @{

/// \brief Verifies the integrity of a SRM
///
/// \param aSrm - Input; pointer to a SRM
/// \param aSrmSize - Input; size (in bytes) of the SRM
/// \retval SUCCESS SRM is valid
/// \retval FAILURE SRM is invalid
int DtcpSrm_VerifySrm(unsigned char    *aSrm,
                      unsigned int      aSrmSize);

/// \brief Determines if a particular device ID has been revoked.
///
/// \param aSrm - Input; pointer to a SRM
/// \param aSrmSize - Input; size (in bytes) of the SRM
/// \param aDeviceId - Input; pointer to a buffer containing the Device ID to check
/// \param aIsRevokedFlag - Output; flag indicating whether or not the Device ID has 
///                         been revoked (0 = not revoked, 1 = revoked)
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
/// \retval INVALID_SRM The SRM is not valid
int DtcpSrm_IsDeviceRevoked(unsigned char   *aSrm, 
                            unsigned int     aSrmSize,
                            unsigned char    aDeviceId[DTCP_DEVICE_ID_SIZE],
                            unsigned int     *aIsRevokedFlag);

/// \brief Updates our SRM with one given to us by another DTCP device
///
/// \param aOurSrm - Input; pointer to our current SRM
/// \param aOurSrmSize - Input; size (in bytes) of our current SRM
/// \param aTheirSrm - Input; pointer to SRM provided by other DTCP product
/// \param aTheirSrmSize - Input; size (in bytes) of SRM provided by other DTCP product
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
/// \retval SRM_UPDATE_FAILED Could not update SRM
/// \retval INVALID_SRM The SRM is not valid
int DtcpSrm_UpdateSrm(unsigned char   *aOurSrm,
                      unsigned int     aOurSrmSize,
                      unsigned char   *aTheirSrm,
                      unsigned int     aTheirSrmSize);

/// \brief Determines whether any portion of our SRM should be sent to update the other
///        device's SRM
///
/// This function determines how much, if any, of the SRM for our device should be sent
/// to the other device to update its SRM based on the SRM Update Decision Tree.
/// \image html SRMUpdateDiagram.gif SRM Update Decision Tree
/// \image rtf SRMUpdateDiagram.gif SRM Update Decision Tree
/// \param aOurSrm - Input; pointer to our SRM
/// \param aOurSrmSize - Input; size (in bytes) of our SRM
/// \param aTheirSrmG - Input; The maximum SRM generation that the other DTCP product can store
/// \param aTheirSrmV - Input; The other DTCP product's current SRM version number
/// \param aTheirSrmC - Input; The other DTCP product's current number of parts stored in their SRM
/// \param aSrmUpdateSize - Output; size (in bytes) of our updated SRM
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpSrm_IsTheirSrmCurrent(unsigned char *aOurSrm,
                              unsigned int   aOurSrmSize,
                              unsigned int   aTheirSrmG,
                              unsigned int   aTheirSrmV,
                              unsigned int   aTheirSrmC,
                              unsigned int  *aSrmUpdateSize);

/// \brief Determines whether our SRM is current.
///
/// This function determines if our SRM is current based on the characteristics of their SRM.
/// \image html SRMUpdateDiagram.gif SRM Update Decision Tree
/// \image rtf SRMUpdateDiagram.gif SRM Update Decision Tree
/// \param aOurSrm - Input; pointer to our SRM
/// \param aOurSrmSize - Input; size (in bytes) of our SRM
/// \param aTheirSrmG - Input; The maximum SRM generation that the other DTCP product can store
/// \param aTheirSrmV - Input; The other DTCP product's current SRM version number
/// \param aTheirSrmC - Input; The other DTCP product's current number of parts stored in their SRM
/// \param aOurSrmUpdateRequiredFlag - Output; flag indicating whether our SRM is up to date.  (0 = NOT 
///                             Current, 1 = Current)
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
/// \todo Update image to reflect different logic.
int DtcpSrm_IsOurSrmCurrent(unsigned char *aOurSrm,
                            unsigned int   aOurSrmSize,
                            unsigned int   aTheirSrmG,
                            unsigned int   aTheirSrmV,
                            unsigned int   aTheirSrmC,
                            unsigned int  *aOurSrmUpdateRequiredFlag);

/// \brief Gets the generation number of a SRM
///
/// \param aSrm - Input; pointer to a SRM
/// \param aSrmSize - Input; size (in bytes) of the SRM
/// \param aSrmG - Output; The generation number of the SRM
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpSrm_GetSrmGeneration(unsigned char *aSrm, unsigned int aSrmSize, unsigned int *aSrmG);

/// \brief Gets the version number of a SRM
///
/// \param aSrm - Input; pointer to a SRM
/// \param aSrmSize - Input; size (in bytes) of the SRM
/// \param aSrmV - Output; The version number of the SRM
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
int DtcpSrm_GetSrmVersion(unsigned char *aSrm, unsigned int aSrmSize, unsigned int *aSrmV);

/// \brief Gets the number of parts contained in a SRM
///
/// \param aSrm - Input; pointer to a SRM
/// \param aSrmSize - Input; size (in bytes) of the SRM
/// \param aSrmC - Output; The number of parts contained in the SRM
/// \retval SUCCESS No errors occurred
/// \retval INVALID_ARGUMENT One of the input parameters was invalid
/// \retval INVALID_SRM The SRM is not valid
int DtcpSrm_GetSrmPartCount(unsigned char *aSrm, unsigned int aSrmSize, unsigned int *aSrmC);

/// @}

#ifdef __cplusplus
extern "C"
}
#endif

#endif // __DTCP_SRM_H__
