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
*    filename:			js_huawei.h
*    author(s):			yanyongmeng
*    version:			0.1
*    date:				2011/2/12
* History
*/
/*-----------------------------------------------------------------------------------------------*/
#ifndef __JS_HUAWEI_H__
#define __JS_HUAWEI_H__

#include "dlna_type.h"

extern void Dlna_Huawei_OnIpChanged(char *netcardName, char *newIp);
extern void Dlna_Huawei_OnSearchInitiated(void);
extern void HW_Dmscp_Init(enum_DlnaAppMode mode);
extern void HW_V1R5_Dmscp_Init(enum_DlnaAppMode mode);


#endif /*__JS_HUAWEI_H__ */

