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
#include "DtcpAkeCoreInternal.h"
#include "DtcpStatusCodes.h"
#include "DtcpConstants.h"

/// \file 
/// \brief Implements some of the internal functions used \ref DtcpAkeCore library
///
/// This header file is part of the \ref DtcpAkeCore library.

int CheckSinkCountLimit(DtcpAkeCoreData *aAkeCoreData, 
                        unsigned char    aDeviceId[DTCP_DEVICE_ID_SIZE],
                        unsigned int     aApFlag)
{
    int returnValue = SUCCESS;
    int i;
    int deviceIdPresent = FALSE;

    if (DTCP_SINK_COUNT_LIMIT <= aAkeCoreData->AuthenticatedSinkDeviceCount)
    {
        // If AP flag is not set, device ID may be present and already counted
        // towards sink count limit.  If the AP flag is set, then it doesn't
        // matter if the device ID is already present or not because every time
        // a device with the AP flag set tries to authenticate, it counts towards
        // the sink count
        if (!aApFlag)
        {
            // Sink count limit reached, check for this device Id
            for (i = 0; i < DTCP_SINK_COUNT_LIMIT; ++i)
            {
                if (0 == memcmp(aDeviceId, 
                                aAkeCoreData->AuthenticatedSinkDevices[i], 
                                DTCP_DEVICE_ID_SIZE))
                {
                    deviceIdPresent = TRUE;
                    break;
                }
            }
        }
        if (!deviceIdPresent)
        {
            returnValue = DTCP_SINK_COUNT_LIMIT_REACHED;
        }
    }
    return returnValue;
} // CheckSinkCountLimit

