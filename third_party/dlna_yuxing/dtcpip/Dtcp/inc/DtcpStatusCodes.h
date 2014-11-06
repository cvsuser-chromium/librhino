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

#ifndef __DTCP_STATUS_CODES_H__
#define __DTCP_STATUS_CODES_H__

#include "StatusCodes.h"

/// \file
/// \brief Defines the status codes for the DTCP libraries

#ifdef __cplusplus
extern  "C"
{
#endif

// Not errors, success with status
#define AKE_CANCELED                        100     ///< AKE canceled

#define BAD_STRUCTURE_SIZE                 -100     ///< Bad structure size
#define BAD_STRUCTURE_VERSION              -101     ///< Bad structure version
#define APP_CALLBACKS_NOT_VALID            -102     ///< Callback functions not valid
#define RESPONSE_TIMEOUT                   -103     ///< Command response timeout expired

#define NOT_INITIALIZED                    -200     ///< Data not initialized
#define INVALID_HANDLE                     -201     ///< Invalid handle

#define INVALID_CERTIFICATE                -302     ///< Invalid certificate
#define DTCP_CERT_INVALID_FORMAT           -303     ///< Function call is not appropriate for this certificate type

#define INVALID_SRM                        -401     ///< Invalid SRM
#define SRM_UPDATE_FAILED                  -402     ///< Unable to update SRM
#define DEVICE_REVOKED                     -403     ///< Device has been revoked

#define DTCP_INVALID_COMMAND_SEQUENCE           -505    ///< Invalid command sequence
#define DTCP_INVALID_DATA_FIELD                 -506    ///< Invalid data field
#define DTCP_OTHER_DEVICE_RESPONSE_DATA_INVALID -507    ///< Other device's response data invalid
#define DTCP_OTHER_DEVICE_CERTIFICATE_INVALID   -508    ///< Other device's certificate invalid
#define DTCP_SINK_COUNT_LIMIT_REACHED           -509    ///< Sink count limit has been reached

#define CONTENT_SIZE_TOO_LARGE                  -600    ///< Packet content size too large
#define INVALID_PACKET_HEADER                   -601    ///< Invalid packet header
#define INVALID_BUFFER_LENGTH                   -602    ///< Invalid buffer length
#define CONTENT_EXCEEDS_PACKET_SIZE             -603    ///< Content exceeds packet size
#define INVALID_EXCHANGE_KEY_LABEL              -604    ///< Invalid exchange key label

#define DTCP_AKE_TIMEOUT                        -700    ///< AKE timed out

#ifdef __cplusplus
}
#endif
#endif
