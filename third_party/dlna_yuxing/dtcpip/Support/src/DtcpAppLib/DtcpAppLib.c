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

#include "DtcpAppLib.h"
#include "DtcpAppLib_i.h"

#include "DtcpApi.h"
#include "StatusCodes.h"

#include <stdio.h>
#include <string.h>

/*****************************************************************************
 * DTCP-IP Common Functions                                                  *
 *****************************************************************************/

/*
 * int DtcpAppLib_Startup();
 * Implemented in DtcpAppLib_i.c
 */

int DtcpAppLib_Shutdown()
{
    DtcpApi_Shutdown();

    return SUCCESS;
}

/*****************************************************************************
 * DTCP-IP Source                                                            *
 *****************************************************************************/

/******************
 * Source globals *
 ******************/

static char gSourceIp[] = "127.0.0.1";  /* Source Ip Address to listen on */
static unsigned int gSourcePort = 0;  /* Source Port to bind to */

static DtcpAkeHandle gListenHandle;     /* DTCP Listener handle */

/********************
 * Source functions *
 ********************/

const char *DtcpAppLib_GetSourceIp()
{
    return &gSourceIp[0];
}

unsigned short DtcpAppLib_GetSourcePort()
{
    return gSourcePort;
}

int DtcpAppLib_Listen(const char *aSourceIp, unsigned short aSourcePort)
{
    int returnValue = LOGIC_ERROR;

    if (NULL != aSourceIp)
    {
        strcpy(gSourceIp, aSourceIp);
    }

    gSourcePort = aSourcePort;

    returnValue = DtcpApi_ListenIp(gSourceIp, &gSourcePort, &gListenHandle);

    if (IS_SUCCESS(returnValue))
    {
        DEBUG_MSG(MSG_INFO, ("DTCP Listening on %s:%d\n\n", gSourceIp, gSourcePort));
    } else {
        DEBUG_MSG(MSG_ERR, ("DTCP Listen FAILED: %d\r\n", returnValue) );
    }

    return returnValue;
}

int DtcpAppLib_CancelListen()
{
    int returnValue = LOGIC_ERROR;

    returnValue = DtcpApi_CancelListen(gListenHandle);

    return returnValue;
}

/*****************************************************************************
 * DTCP-IP Sink                                                              *
 *****************************************************************************/

int DtcpAppLib_DoAke(const char *aRemoteIp, unsigned short aRemotePort, void **aAkeHandle)
{
    int returnValue = LOGIC_ERROR;

    if (NULL == aRemoteIp || 0 == aRemotePort || NULL == aAkeHandle ) //|| NULL == *aAkeHandle)
    {
        returnValue = INVALID_ARGUMENT;
        return returnValue;
    }

    /* Add by Gaocn 2005_6_21 监视一下状态 */
    DEBUG_MSG_TIME( MSG_INFO, ( " DtcpAppLib_DoAke   :: *************************   AKE Start  *************************\r\n" ) );
    returnValue = DtcpApi_OpenAkeIp((char *) aRemoteIp, aRemotePort, aAkeHandle);

    if (IS_SUCCESS(returnValue))
    {
        returnValue = DtcpApi_DoAke(*aAkeHandle);
        if (IS_FAILURE(returnValue))
        {
		  /* Add by Gaocn 2005_6_21 监视一下状态 */
		  DEBUG_MSG_TIME( MSG_INFO, ( " DtcpAppLib_DoAke   :: ************************* AKE failed!! *************************\r\n" ) );
            //DEBUG_MSG(MSG_ERR, ("Do AKE failed: %d\n", returnValue));
        } else {
		  /* Add by Gaocn 2005_6_21 监视一下状态 */
		  DEBUG_MSG_TIME( MSG_INFO, ( " DtcpAppLib_DoAke   :: *************************    AKE OK    *************************\r\n" ) );
            //DEBUG_MSG(MSG_INFO, ("Do AKE succeeded.\n"));
        }
    }
    else
    {
        DEBUG_MSG(MSG_ERR, ("Open Ake failed: %d\n", returnValue));
    }

    return returnValue;
}

int DtcpAppLib_CloseAke(void *aAkeHandle)
{
    return DtcpApi_CloseAke(aAkeHandle);
}