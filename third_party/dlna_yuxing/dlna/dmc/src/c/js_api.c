/*-----------------------------------------------------------------------------------------------*/
/*
 * Yuxing Software CONFIDENTIAL
 * Copyright (c) 2003, 2011 Yuxing Corporation.  All rights reserved.
 *
 * The computer program contained herein contains proprietary information
 * which is the property of Yuxing Software Co., Ltd.  The program may be used
 * and/or copied only with the written permission of Yuxing Software Co., Ltd.
 * or in accordance with the terms and conditions stipulated in the
 * agreement/contract under which the programs have been supplied.
 *
 *    filename:			js_api.c
 *    author(s):			yanyongmeng
 *    version:			0.1
 *    date:				2011/2/12
 * History
 */
/*-----------------------------------------------------------------------------------------------*/

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include "upnp.h"
#include "LinkedList.h"


#include "js_common.h"
#include "dmscp.h"
#include "js_yuxing.h"
#include "js_huawei.h"

#include "dlna_api_raw.h"
#include "js_api.h"
#include "hitTime.h"


static enum_DlnaAppMode s_app_mode;

void Raw_Dlna_ModeInit(enum_DlnaAppMode app_mode, t_DLNA_CALLBACK callback, t_DLNA_EVENT_EX EventHandler, enum_DlnaEvent EventHandlerType)
{
  HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_KEY,(int)EventHandlerType, NULL);

  s_app_mode = app_mode;
  EventQueue_Module_Init(app_mode, callback, EventHandler, EventHandlerType);

  HT_DBG_FUNC_END((int)EventHandler, 0);
}

void Raw_Playback_ModeInit(enum_DlnaAppMode app_mode, t_DLNA_CALLBACK callback, t_DLNA_EVENT_EX EventHandler, enum_DlnaEvent EventHandlerType)
{
  HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_KEY,(int)EventHandlerType, NULL);

  EventQueue_Module_Init(app_mode, callback, EventHandler, EventHandlerType);

  HT_DBG_FUNC_END((int)EventHandler, 0);
}


int Raw_Dmc_Init(char *setting)
{
  HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_KEY, 0, setting);
  YX_Dmscp_Init(setting);
  HT_DBG_FUNC_END(0, 0);
  return 0;
}

void Raw_Dmc_Destroy(void)
{
  HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_KEY, 0, NULL);

  YX_Dmscp_Destroy();

  HT_DBG_FUNC_END(0, 0);
}


int Raw_Dmc_Start(void)
{
  HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_KEY, 0, NULL);

  if (enum_DlnaAppMode_HUAWEI == s_app_mode
    ||enum_DlnaAppMode_HUAWEI_QTEL == s_app_mode
    ||enum_DlnaAppMode_HUAWEI_MAROC == s_app_mode )
    HW_Dmscp_Init(s_app_mode);
  else if(enum_DlnaAppMode_HUAWEI_V1R5 == s_app_mode)
    HW_V1R5_Dmscp_Init(s_app_mode);

  YX_Dmscp_Start();

  HT_DBG_FUNC_END(0, 0);
  return 0;
}


void Raw_Dmc_Stop(void)
{
  HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_KEY, 0, NULL);

  YX_Dmscp_Stop();

  HT_DBG_FUNC_END(0, 0);
}

void Raw_Dmc_SearchDms(int remove_all)
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_KEY, 0, NULL);

	YX_Dmscp_SearchDms(remove_all);

	HT_DBG_FUNC_END(0, 0);
}

