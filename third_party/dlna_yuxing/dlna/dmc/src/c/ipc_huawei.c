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
//#include "dmscp.h"
//#include "js_yuxing.h"
#include "js_huawei.h"

#include "dlna_api_raw.h"
#include "dlna_api_ipc.h"

#include "js_api.h"
#include "ipc.h"
#include "ipc_common.h"
#include "hitTime.h"


typedef enum _function_name_list_
{
	FUNC_Dmc_HuaweiJse = 0,
//	FUNC_Dmc_Huawei_BrowseMetadata,
	FUNC_Dmc_Huawei_GetDmsIp,
	FUNC_Dmc_Huawei_GetBestUri,
	FUNC_Dmc_Huawei_GetBestMediaInfo,
	FUNC_Dmc_HuaweiJse_V1R5,
	
	FUNC_SENDER_MAX,
}t_FUNC_NAME;


static t_IPC_RCV_FUNC	s_ipc_huawei_server_func[FUNC_SENDER_MAX];

/*-------------------------------------------------*/
static void s_ipc_call_send(int func_index, t_IPC_DATA *send)
{
	Ipc_CallAndReturn(g_ipc_client, enum_IPC_MOD_HUAWEI, func_index, send);
}
static void s_ipc_read(int func_index, t_IPC_DATA *send, void *buf)
{
	Ipc_CallAndReturnEx(g_ipc_client, enum_IPC_MOD_HUAWEI, func_index, send, buf, NULL);
}
static void s_ipc_read_malloc(int func_index, t_IPC_DATA *send, void **buf)
{
	Ipc_CallAndReturnEx(g_ipc_client, enum_IPC_MOD_HUAWEI, func_index, send, NULL, buf);
}


int IpcMode_Dmc_HuaweiJse(const char *func, const char *para, char *value, int len, int readonly)
{
	t_IPC_DATA data = {0};
	int temp[2];

	temp[0] = len;
	temp[1] = readonly;

	Ipc_Data_SetStruct(&data,  temp, 8);
	Ipc_Data_SetString(&data,  (char*)func);
	Ipc_Data_SetString(&data,  (char*)para);
	Ipc_Data_SetString(&data,  value);
#ifdef USE_OLD_IPC
	s_ipc_call_send( FUNC_Dmc_HuaweiJse, &data);

	if( readonly )
	{
		char *s = Ipc_Data_GetFirstString(&data);
		int slen = data.xxbuf_len[0];
		if( value && s && (slen <= len) )
			memcpy(value, s, slen);
	}
#else
	if(readonly)
		s_ipc_read( FUNC_Dmc_HuaweiJse, &data, value);
	else
		s_ipc_call_send( FUNC_Dmc_HuaweiJse, &data);
#endif
	return 0;
}	
static int IpcRcv_Dmc_HuaweiJse(int hnd, int func_index, t_IPC_DATA *data)
{
	int *temp = (int*)Ipc_Data_GetFirstString(data);
	int len = temp[0];
	char vbuf[len+10];
	
	strcpy(vbuf, Ipc_Data_GetString(data, 4));
	Raw_Dmc_HuaweiJse(Ipc_Data_GetString(data, 2), Ipc_Data_GetString(data, 3), vbuf, len);

	if( temp[1] )
	{
		t_IPC_DATA ack = {0};
		Ipc_Data_SetString(&ack,  vbuf);
		Ipc_Ack_Value(hnd, &ack);
	}
	return 1;
}	

int IpcMode_Dmc_HuaweiJse_V1R5(const char *func, const char *para, char *value, int len, int readonly)
{
	t_IPC_DATA data = {0};
	int temp[2];

	temp[0] = len;
	temp[1] = readonly;

	Ipc_Data_SetStruct(&data,  temp, 8);
	Ipc_Data_SetString(&data,  (char*)func);
	Ipc_Data_SetString(&data,  (char*)para);
	Ipc_Data_SetString(&data,  value);
#ifdef USE_OLD_IPC
	s_ipc_call_send( FUNC_Dmc_HuaweiJse_V1R5, &data);

	if( readonly )
	{
		char *s = Ipc_Data_GetFirstString(&data);
		int slen = data.xxbuf_len[0];
		if( value && s && (slen <= len) )
			memcpy(value, s, slen);
	}
#else
	if(readonly)
		s_ipc_read( FUNC_Dmc_HuaweiJse_V1R5, &data, value);
	else
		s_ipc_call_send( FUNC_Dmc_HuaweiJse_V1R5, &data);
#endif
	
	return 0;
}	

static int IpcRcv_Dmc_HuaweiJse_V1R5(int hnd, int func_index, t_IPC_DATA *data)
{
    int *temp = (int*)Ipc_Data_GetFirstString(data);
    int len = temp[0];
    char vbuf[len+10];
    
    strcpy(vbuf, Ipc_Data_GetString(data, 4));
    Raw_Dmc_HuaweiJse_V1R5(Ipc_Data_GetString(data, 2), Ipc_Data_GetString(data, 3), vbuf, len);

    if( temp[1] )
    {
        t_IPC_DATA ack = {0};
        Ipc_Data_SetString(&ack,  vbuf);
        Ipc_Ack_Value(hnd, &ack);
    }
    return 1;
}

#if 0
int IpcMode_Dmc_Huawei_BrowseMetadata( char *udn, char *id, char **metadata)
{
	t_IPC_DATA data = {0};
	int ret = -1;

	if(!id || !metadata)
		return -2;
	
	Ipc_Data_SetString(&data,  udn);
	Ipc_Data_SetString(&data,  id);
	
	s_ipc_call_send( FUNC_Dmc_Huawei_BrowseMetadata, &data);

    if( Ipc_Data_GetFirstString(&data) )
    {
		*metadata = strdup(Ipc_Data_GetFirstString(&data));
		ret =0;
     }
	return ret;
}	
static int IpcRcv_Dmc_Huawei_BrowseMetadata(int hnd, int func_index, t_IPC_DATA *data)
{
	char *metedata=NULL;

	int ret = Raw_Dmc_Huawei_BrowseMetadata( Ipc_Data_GetFirstString(data), Ipc_Data_GetString(data, 2), &metedata);
	if(metedata)
	{
		t_IPC_DATA ack = {0};
		Ipc_Data_SetString(&ack,  metedata);
		Ipc_Ack_Value(hnd, &ack);
	    free(metedata);
	}
	return ret;
}	
#endif

int IpcMode_Dmc_Huawei_GetDmsIp( char *udn, char **dms_ip)
{
	t_IPC_DATA data = {0};
	int ret = -1;

	if(!dms_ip)
		return -2;
	
	Ipc_Data_SetString(&data,  udn);
	
#ifdef USE_OLD_IPC
	s_ipc_call_send( FUNC_Dmc_Huawei_GetDmsIp, &data);

    if( Ipc_Data_GetFirstString(&data) )
    {
		*dms_ip = strdup(Ipc_Data_GetFirstString(&data));
		ret =0;
     }
#else
	s_ipc_read_malloc( FUNC_Dmc_Huawei_GetDmsIp, &data, (void**)dms_ip);
	if(*dms_ip)
		ret = 0;
#endif	
	return ret;
}	
static int IpcRcv_Dmc_Huawei_GetDmsIp(int hnd, int func_index, t_IPC_DATA *data)
{
	char *dms_ip=NULL;

	int ret = Raw_Dmc_Huawei_GetDmsIp( Ipc_Data_GetFirstString(data), &dms_ip);
	if(dms_ip)
	{
		t_IPC_DATA ack = {0};
		Ipc_Data_SetString(&ack,  dms_ip);
		Ipc_Ack_Value(hnd, &ack);
	    free(dms_ip);
	}
	return ret;
}	


int IpcMode_Dmc_Huawei_GetBestUri( char *udn, char *id, char **uri)
{
	t_IPC_DATA data = {0};
	int ret = -1;

	if(!id || !uri)
		return -2;
	
	Ipc_Data_SetString(&data,  udn);
	Ipc_Data_SetString(&data,  id);
	
#ifdef USE_OLD_IPC
	s_ipc_call_send( FUNC_Dmc_Huawei_GetBestUri, &data);

    if( Ipc_Data_GetFirstString(&data) )
    {
		*uri = strdup(Ipc_Data_GetFirstString(&data));
		ret =0;
     }
#else
	s_ipc_read_malloc( FUNC_Dmc_Huawei_GetBestUri, &data, (void**)uri);
	if(*uri)
		ret = 0;
#endif	
	return ret;
}	
static int IpcRcv_Dmc_Huawei_GetBestUri(int hnd, int func_index, t_IPC_DATA *data)
{
	char *uri=NULL;

	int ret = Raw_Dmc_Huawei_GetBestUri( Ipc_Data_GetFirstString(data), Ipc_Data_GetString(data, 2), &uri);
	if(uri)
	{
		t_IPC_DATA ack = {0};
		Ipc_Data_SetString(&ack,  uri);
		Ipc_Ack_Value(hnd, &ack);
	    free(uri);
	}
	return ret;
}	

int IpcMode_Dmc_Huawei_GetBestMediaInfo( char *udn, char *id, t_MEDIA_INFO *minfo)
{
	t_IPC_DATA data = {0};
	int ret = -1;

	if(!id || !minfo)
		return -2;
	
	Ipc_Data_SetString(&data,  udn);
	Ipc_Data_SetString(&data,  id);
	
#ifdef USE_OLD_IPC
	s_ipc_call_send( FUNC_Dmc_Huawei_GetBestMediaInfo, &data);

    if( Ipc_Data_GetFirstString(&data) )
    {
		memcpy(minfo, Ipc_Data_GetFirstString(&data), sizeof(t_MEDIA_INFO));
		ret =0;
     }
#else
	s_ipc_read( FUNC_Dmc_Huawei_GetBestMediaInfo, &data, minfo);
	if(minfo->majorType)
		ret = 0;
#endif	
	return ret;
}	
static int IpcRcv_Dmc_Huawei_GetBestMediaInfo(int hnd, int func_index, t_IPC_DATA *data)
{
	t_MEDIA_INFO minfo={0};

	int ret = Raw_Dmc_Huawei_GetBestMediaInfo( Ipc_Data_GetFirstString(data), Ipc_Data_GetString(data, 2), &minfo);
	if(!ret)
	{
		t_IPC_DATA ack = {0};
		Ipc_Data_SetStruct(&ack,  &minfo, sizeof(t_MEDIA_INFO));
		Ipc_Ack_Value(hnd, &ack);
	}
	return ret;
}	

/*-------------------------------------------------*/
int IpcHuawei_Client_Func(int hnd, int func_index, t_IPC_DATA *data)
{
	enum_DlnaEvent event_type = (enum_DlnaEvent)Ipc_Data_GetInt(data);
	int *handle = (int*)Ipc_Data_GetString(data, 2);
	int ret = -1;
	
	if( g_IPC_client_side_EventHandler )
		ret = g_IPC_client_side_EventHandler(enum_IPC_MOD_HUAWEI, event_type, *handle, Ipc_Data_GetString(data, 3));
	
	t_IPC_DATA ack = {0};
	Ipc_Data_SetInt(&ack, &ret);
	Ipc_Ack_Value(hnd, &ack);
	return 1;
}

int IpcHuawei_Server_Func(int hnd, int func_index, t_IPC_DATA *data)
{
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, func_index, "func_index = ");
	int ret = s_ipc_huawei_server_func[func_index](hnd, func_index, data);
	HT_DBG_FUNC_END(ret, "ret = ");
	return ret;
}	

void IpcHuawei_Client_Init(void)
{
}

void IpcHuawei_Server_Init(void)
{
	t_IPC_RCV_FUNC *list;

	list = s_ipc_huawei_server_func;
	xxx_register_func(list, FUNC_Dmc_HuaweiJse,	IpcRcv_Dmc_HuaweiJse);
	//xxx_register_func(list, FUNC_Dmc_Huawei_BrowseMetadata, IpcRcv_Dmc_Huawei_BrowseMetadata);
	xxx_register_func(list, FUNC_Dmc_Huawei_GetDmsIp, IpcRcv_Dmc_Huawei_GetDmsIp);
	xxx_register_func(list, FUNC_Dmc_Huawei_GetBestUri, IpcRcv_Dmc_Huawei_GetBestUri);
	xxx_register_func(list, FUNC_Dmc_Huawei_GetBestMediaInfo, IpcRcv_Dmc_Huawei_GetBestMediaInfo);

    xxx_register_func(list, FUNC_Dmc_HuaweiJse_V1R5, IpcRcv_Dmc_HuaweiJse_V1R5);
}

