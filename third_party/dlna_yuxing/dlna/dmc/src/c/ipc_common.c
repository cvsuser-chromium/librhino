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
*    filename:			ipc.c
*    author(s):			yanyongmeng
*    version:			0.1
*    date:				2011/2/12
* History
*/
/*-----------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


#include "dlna_api_raw.h"

#include "ipc.h"
#include "ipc_common.h"
#include "hitTime.h"
#include "dms.h"


typedef enum _function_name_list_
{
	FUNC_Dlna_ModeInit = 0,
		
	FUNC_UpnpStackInit,
	FUNC_UpnpStackDestroy,
	
	FUNC_Dmc_Init,
	FUNC_Dmc_Start,
	FUNC_Dmc_Stop,
	FUNC_Dmc_SearchDms,
	FUNC_Dmc_Destroy,
	
	FUNC_Dms_Init,
	FUNC_Dms_Start,
	FUNC_Dms_Stop,
	FUNC_Dms_SetNewName,
	FUNC_Dms_AddPvrItem,
	FUNC_Dms_RemovePvrItem,
	FUNC_Dms_FindFolderShared,
	FUNC_Dms_AddFolderShared,
	FUNC_Dms_RemoveFolderShared,
	FUNC_Dms_SetPause,
	FUNC_Dms_SetStoppingHttp,

	FUNC_Dms_VirtualFileTree_AddChild,
	FUNC_Dms_VirtualFileTree_DeleteChild,
	FUNC_Dms_VirtualFileTree_DeleteContainer,
	FUNC_Dms_VirtualFileTree_GetInfo,
	FUNC_Dms_VirtualFileTree_GetChildID,
	FUNC_Dms_VirtualFileTree_GetParentID,
	FUNC_Dms_ShareUsb_AddFolder,
	FUNC_Dms_ShareUsb_RemoveFolder,
	
	FUNC_SENDER_MAX,
}t_FUNC_NAME;

typedef enum _callback_name_list_
{
	CALLBACK_EventCallback = 0,
	
	CALLBACK_RECIEVER_MAX,
}t_CB_NAME;

//#define NEW_AND_ZERO(data)	t_IPC_DATA data = {0}
#define EXTERN	
#define STATIC static 

t_DLNA_EVENT_EX g_IPC_client_side_EventHandler = NULL;
static t_DLNA_CALLBACK	s_IPC_client_side_callback = NULL;
static t_IPC_RCV_FUNC	s_common_func[FUNC_SENDER_MAX];

static void s_ipc_call_send(int func_index, t_IPC_DATA *send)
{
	Ipc_CallAndReturn(g_ipc_client, enum_IPC_MOD_COMMON, func_index, send);
}

static void s_ipc_read(int func_index, t_IPC_DATA *send, void *buf)
{
	Ipc_CallAndReturnEx(g_ipc_client, enum_IPC_MOD_COMMON, func_index, send, buf, NULL);
}

/* firstly */
EXTERN void IpcMode_Dlna_ModeInit(enum_DlnaAppMode app_mode, t_DLNA_CALLBACK callback, t_DLNA_EVENT_EX EventHandler, enum_DlnaEvent EventHandlerType)
{
	t_IPC_DATA data = {0};
	int x1 = (int)app_mode;
	int	x2 = (int)callback;
	int	x3 = (int)EventHandler;
	int	x4 = (int)EventHandlerType;

	s_IPC_client_side_callback		= callback;
	g_IPC_client_side_EventHandler	= EventHandler;
	
	Ipc_Data_SetInt(&data,  &x1);
	Ipc_Data_SetInt(&data,  &x2);
	Ipc_Data_SetInt(&data,  &x3);
	Ipc_Data_SetInt(&data,  &x4);
	
	s_ipc_call_send( FUNC_Dlna_ModeInit, &data);
}
STATIC int s_IpcRcv_EventHandler(enum_IPC_MODULE module, enum_DlnaEvent eType, int handle, char *str)
{
	t_IPC_DATA data = {0};
	int x = (int)eType;
	
	Ipc_Data_SetInt(&data,  &x);
	Ipc_Data_SetInt(&data,  &handle);
	Ipc_Data_SetString(&data,  str);

	Ipc_CallAndReturn(g_ipc_server, module, CALLBACK_EventCallback, &data);

	return Ipc_Data_GetInt(&data);
}
STATIC int s_IpcRcv_Callback(enum_DlnaCallback eType, int value, char *str, void *user)
{
	t_IPC_DATA data = {0};
	int x = (int)eType;
	
	Ipc_Data_SetInt(&data,  &x);
	Ipc_Data_SetInt(&data,  &value);
	Ipc_Data_SetStruct(&data,  &user, sizeof(int));
	Ipc_Data_SetString(&data,  str);

	Ipc_CallAndReturn(g_ipc_server, enum_IPC_MOD_COMMON, CALLBACK_EventCallback, &data);

	return Ipc_Data_GetInt(&data);
}

STATIC int IpcRcv_Dlna_ModeInit(int hnd, int func_index, t_IPC_DATA *data)
{
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_KEY, func_index, NULL);
	
	enum_DlnaAppMode *app_mode		= (enum_DlnaAppMode*)Ipc_Data_GetFirstString(data);
	HT_DBG_FUNC((int)app_mode, "app_mode = ");
	t_DLNA_CALLBACK *callback		= (t_DLNA_CALLBACK*)Ipc_Data_GetString(data, 2);
	HT_DBG_FUNC((int)callback, "callback = ");
	t_DLNA_EVENT_EX *EventHandler	= (t_DLNA_EVENT_EX*)Ipc_Data_GetString(data, 3);
	HT_DBG_FUNC((int)EventHandler, "EventHandler = ");
	enum_DlnaEvent *EventHandlerType = (enum_DlnaEvent*)Ipc_Data_GetString(data, 4);
	HT_DBG_FUNC((int)EventHandlerType, "EventHandlerType = ");
	Raw_Dlna_ModeInit(*app_mode, *callback?s_IpcRcv_Callback:NULL, *EventHandler?s_IpcRcv_EventHandler:NULL, *EventHandlerType);
	HT_DBG_FUNC_END(0, 0);
	return 0;
}



/* upnp */
EXTERN int IpcMode_UpnpStackInit(char *ifname, char *ip_address, char *web_dir_path)
{
//	NEW_AND_ZERO(data);
	t_IPC_DATA data = {0};

	Ipc_Data_SetString(&data,  ifname);
	Ipc_Data_SetString(&data,  ip_address);
	Ipc_Data_SetString(&data,  web_dir_path);
	
	s_ipc_call_send( FUNC_UpnpStackInit, &data);
	return 0;
}
STATIC int IpcRcv_UpnpStackInit(int hnd, int func_index, t_IPC_DATA *data)
{
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_KEY, 0, NULL);
	UpnpStackInitRaw(Ipc_Data_GetFirstString(data), Ipc_Data_GetString(data, 2), Ipc_Data_GetString(data, 3));
	HT_DBG_FUNC_END(0,0);
	return 0;
}

EXTERN void IpcMode_UpnpStackDestroy(void)
{
	s_ipc_call_send(FUNC_UpnpStackDestroy,NULL);
}
STATIC int IpcRcv_UpnpStackDestroy(int hnd, int func_index, t_IPC_DATA *data)
{
	UpnpStackDestroyRaw();
	return 0;
}

/* dmc */
EXTERN int IpcMode_Dmc_Init(char *setting)
{
	t_IPC_DATA data = {0};
	
	Ipc_Data_SetString(&data,  setting);
	s_ipc_call_send( FUNC_Dmc_Init, &data);
	return 0;
}
STATIC int IpcRcv_Dmc_Init(int hnd, int func_index, t_IPC_DATA *data)
{
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
	Raw_Dmc_Init(Ipc_Data_GetFirstString(data));
	HT_DBG_FUNC_END(0,0);
	return 0;
}

EXTERN int IpcMode_Dmc_Start() 
{
	s_ipc_call_send( FUNC_Dmc_Start, NULL);
	return 0;
}
STATIC int IpcRcv_Dmc_Start(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dmc_Start( );
	return 0;
}

EXTERN void IpcMode_Dmc_Stop(void)
{
	s_ipc_call_send(FUNC_Dmc_Stop,NULL);
}
STATIC int IpcRcv_Dmc_Stop(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dmc_Stop();
	return 0;
}

EXTERN void IpcMode_Dmc_SearchDms(int remove_all)
{
	t_IPC_DATA data = {0};
	
	Ipc_Data_SetInt(&data,  &remove_all);
	
	s_ipc_call_send( FUNC_Dmc_SearchDms, &data);
}
STATIC int IpcRcv_Dmc_SearchDms(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dmc_SearchDms( Ipc_Data_GetInt(data) );
	return 0;
}

EXTERN void IpcMode_Dmc_Destroy(void)
{
	s_ipc_call_send(FUNC_Dmc_Destroy,NULL);
}
STATIC int IpcRcv_Dmc_Destroy(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dmc_Destroy();
	return 0;
}


/* dms */
EXTERN int IpcMode_Dms_Init(char *cfg)
{
	t_IPC_DATA data = {0};
	Ipc_Data_SetString(&data,  cfg);
	s_ipc_call_send( FUNC_Dms_Init, &data);
	return 0;
}
STATIC int IpcRcv_Dms_Init(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dms_Init(Ipc_Data_GetFirstString(data));
	return 0;
}

EXTERN int IpcMode_Dms_Start(void)
{
	s_ipc_call_send(FUNC_Dms_Start, NULL );
	return 0;
}
STATIC int IpcRcv_Dms_Start(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dms_Start( );
	return 0;
}

EXTERN int IpcMode_Dms_Stop(void)
{
	s_ipc_call_send(FUNC_Dms_Stop, 0 );
	return 0;
}	
STATIC int IpcRcv_Dms_Stop(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dms_Stop( );
	return 0;
}

EXTERN int IpcMode_Dms_SetNewName( char *new_name)
{
	t_IPC_DATA data = {0};
	Ipc_Data_SetString(&data,  new_name);
	s_ipc_call_send( FUNC_Dms_SetNewName, &data);
	return 0;
}
STATIC int IpcRcv_Dms_SetNewName(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dms_SetNewName(Ipc_Data_GetFirstString(data));
	return 0;
}

EXTERN int IpcMode_Dms_AddPvrItem(char *path, char *itemName)
{
	t_IPC_DATA data = {0};
	Ipc_Data_SetString(&data,  path);
	Ipc_Data_SetString(&data,  itemName);
	s_ipc_call_send( FUNC_Dms_AddPvrItem, &data);
	return 0;
}
STATIC int IpcRcv_Dms_AddPvrItem(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dms_AddPvrItem(Ipc_Data_GetFirstString(data), Ipc_Data_GetString(data, 2) );
	return 0;
}

EXTERN int IpcMode_Dms_RemovePvrItem(char *path, char *itemName)
{
	t_IPC_DATA data = {0};
	Ipc_Data_SetString(&data,  path);
	Ipc_Data_SetString(&data,  itemName);
	s_ipc_call_send( FUNC_Dms_RemovePvrItem, &data);
	return 0;
}
STATIC int IpcRcv_Dms_RemovePvrItem(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dms_RemovePvrItem(Ipc_Data_GetFirstString(data), Ipc_Data_GetString(data, 2) );
	return 0;
}

EXTERN int IpcMode_Dms_FindFolderShared(char *fullpath)
{
	t_IPC_DATA data = {0};
	Ipc_Data_SetString(&data,  fullpath);
	s_ipc_call_send( FUNC_Dms_FindFolderShared, &data);
	return 0;
}	
STATIC int IpcRcv_Dms_FindFolderShared(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dms_FindFolderShared(Ipc_Data_GetFirstString(data));
	return 0;
}

EXTERN int IpcMode_Dms_AddFolderShared(char *fullpath, char *title, int isPvr)
{
	t_IPC_DATA data = {0};
	Ipc_Data_SetInt(&data,  &isPvr);
	Ipc_Data_SetString(&data,  fullpath);
	Ipc_Data_SetString(&data,  title);
	s_ipc_call_send( FUNC_Dms_AddFolderShared, &data);
	return 0;
}
STATIC int IpcRcv_Dms_AddFolderShared(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dms_AddFolderShared(Ipc_Data_GetString(data, 2), Ipc_Data_GetString(data, 3), Ipc_Data_GetInt(data));
	return 0;
}

EXTERN int IpcMode_Dms_RemoveFolderShared(char *fullpath)
{
	t_IPC_DATA data = {0};
	Ipc_Data_SetString(&data,  fullpath);
	s_ipc_call_send( FUNC_Dms_RemoveFolderShared, &data);
	return 0;
}	
STATIC int IpcRcv_Dms_RemoveFolderShared(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dms_RemoveFolderShared(Ipc_Data_GetFirstString(data));
	return 0;
}


EXTERN int IpcMode_Dms_SetPause(int mode)
{
	t_IPC_DATA data = {0};
	Ipc_Data_SetInt(&data,  &mode);
	s_ipc_call_send( FUNC_Dms_SetPause, &data);
	return 0;
}	
STATIC int IpcRcv_Dms_SetPause(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dms_SetPause(Ipc_Data_GetInt(data));
	return 0;
}

EXTERN int IpcMode_Dms_SetStoppingHttp(int mode)
{
	t_IPC_DATA data = {0};
	Ipc_Data_SetInt(&data,  &mode);
	s_ipc_call_send( FUNC_Dms_SetStoppingHttp, &data);
	return 0;
}	
STATIC int IpcRcv_Dms_SetStoppingHttp(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dms_SetStoppingHttp(Ipc_Data_GetInt(data));
	return 0;
}

/*-------------------------------*/
EXTERN int IpcMode_Dms_VirtualFileTree_AddChild(int parent_id, char *info)
{
	int ret = 0;
	t_IPC_DATA data = {0};
	
	HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_KEY, parent_id, 0);
	Ipc_Data_SetInt(&data,  &parent_id);
	Ipc_Data_SetString(&data, info);
	s_ipc_read( FUNC_Dms_VirtualFileTree_AddChild, &data, &ret);

	HT_DBG_FUNC_END(ret, NULL);
	return ret;
}
STATIC int IpcRcv_Dms_VirtualFileTree_AddChild(int hnd, int func_index, t_IPC_DATA *data)
{
	int ret = Raw_Dms_VirtualFileTree_AddChild(Ipc_Data_GetInt(data), Ipc_Data_GetString(data, 2));

	t_IPC_DATA ack = {0};
	Ipc_Data_SetInt(&ack, &ret);
	Ipc_Ack_Value(hnd, &ack);
	return 1;
}

EXTERN void IpcMode_Dms_VirtualFileTree_DeleteChild(int parent_id, int child_id)
{
	t_IPC_DATA data = {0};
	
	Ipc_Data_SetInt(&data,  &parent_id);
	Ipc_Data_SetInt(&data,  &child_id);
	s_ipc_call_send( FUNC_Dms_VirtualFileTree_DeleteChild, &data);
}	
STATIC int IpcRcv_Dms_VirtualFileTree_DeleteChild(int hnd, int func_index, t_IPC_DATA *data)
{
    int *child_id = (int*)Ipc_Data_GetString(data, 2);
	Raw_Dms_VirtualFileTree_DeleteChild(Ipc_Data_GetInt(data), *child_id);
	return 0;
}	
EXTERN void IpcMode_Dms_VirtualFileTree_DeleteContainer(int parent_id, int container_id, int keep_if_child)
{
	t_IPC_DATA data = {0};
	
	Ipc_Data_SetInt(&data,  &parent_id);
	Ipc_Data_SetInt(&data,  &container_id);
	Ipc_Data_SetInt(&data,  &keep_if_child);
	s_ipc_call_send( FUNC_Dms_VirtualFileTree_DeleteContainer, &data);
}	
STATIC int IpcRcv_Dms_VirtualFileTree_DeleteContainer(int hnd, int func_index, t_IPC_DATA *data)
{
    int *container_id = (int*)Ipc_Data_GetString(data, 2);
    int *keep_if_child = (int*)Ipc_Data_GetString(data, 3);
	Raw_Dms_VirtualFileTree_DeleteContainer(Ipc_Data_GetInt(data), *container_id, *keep_if_child);
	return 0;
}	

EXTERN void IpcMode_Dms_VirtualFileTree_GetInfo(int id, int onlybasic, char *buf, int len)
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,	&id);
	Ipc_Data_SetInt(&data,	&onlybasic);
	Ipc_Data_SetInt(&data,	&len);
	Ipc_Data_SetString(&data, buf);
	s_ipc_read( FUNC_Dms_VirtualFileTree_GetInfo, &data, buf);
}	
STATIC int IpcRcv_Dms_VirtualFileTree_GetInfo(int hnd, int func_index, t_IPC_DATA *data)
{
	int *onlybasic = (int*)Ipc_Data_GetString(data, 2);
	int *len = (int*)Ipc_Data_GetString(data, 3);
	char *buf = Ipc_Data_GetString(data, 4);
	Raw_Dms_VirtualFileTree_GetInfo(Ipc_Data_GetInt(data), *onlybasic, buf, *len);
	
	t_IPC_DATA ack = {0};
	Ipc_Data_SetString(&ack,buf);
	Ipc_Ack_Value(hnd, &ack);
	return 1;
}

EXTERN int IpcMode_Dms_VirtualFileTree_GetChildID(int parent_id, char type, char *keyword, int is_recursive)
{
	int ret = 0;
	int t = type;
	t_IPC_DATA data = {0};
	
	Ipc_Data_SetInt(&data,  &parent_id);
	Ipc_Data_SetInt(&data,  &t);
	Ipc_Data_SetInt(&data,  &is_recursive);
	Ipc_Data_SetString(&data, keyword);
	s_ipc_read( FUNC_Dms_VirtualFileTree_GetChildID, &data, &ret);

	return ret;
}
STATIC int IpcRcv_Dms_VirtualFileTree_GetChildID(int hnd, int func_index, t_IPC_DATA *data)
{
    int *type = (int*)Ipc_Data_GetString(data, 2);
	int *is_recursive = (int*)Ipc_Data_GetString(data, 3);
	int ret = Raw_Dms_VirtualFileTree_GetChildID(Ipc_Data_GetInt(data), (char)(*type), Ipc_Data_GetString(data, 4), *is_recursive);

	t_IPC_DATA ack = {0};
	Ipc_Data_SetInt(&ack, &ret);
	Ipc_Ack_Value(hnd, &ack);
	return 1;
}	


EXTERN int IpcMode_Dms_VirtualFileTree_GetParentID(int my_id)
{
	int ret = 0;
	t_IPC_DATA data = {0};
	
	Ipc_Data_SetInt(&data,  &my_id);
	s_ipc_read( FUNC_Dms_VirtualFileTree_GetParentID, &data, &ret);

	return ret;
}	
STATIC int IpcRcv_Dms_VirtualFileTree_GetParentID(int hnd, int func_index, t_IPC_DATA *data)
{
	int ret = Raw_Dms_VirtualFileTree_GetParentID(Ipc_Data_GetInt(data));

	t_IPC_DATA ack = {0};
	Ipc_Data_SetInt(&ack, &ret);
	Ipc_Ack_Value(hnd, &ack);
	return 1;
}	

EXTERN int IpcMode_Dms_ShareUsb_AddFolder(int parent_id, char *fullpath)
{
	int ret = 0;
	t_IPC_DATA data = {0};
	
	Ipc_Data_SetInt(&data,  &parent_id);
	Ipc_Data_SetString(&data, fullpath);
	s_ipc_read( FUNC_Dms_ShareUsb_AddFolder, &data, &ret);

	return ret;
}	
STATIC int IpcRcv_Dms_ShareUsb_AddFolder(int hnd, int func_index, t_IPC_DATA *data)
{
	int ret = Raw_Dms_ShareUsb_AddFolder(Ipc_Data_GetInt(data), Ipc_Data_GetString(data, 2));

	t_IPC_DATA ack = {0};
	Ipc_Data_SetInt(&ack, &ret);
	Ipc_Ack_Value(hnd, &ack);
	return 1;
}

EXTERN void IpcMode_Dms_ShareUsb_RemoveFolder(int parent_id, char *fullpath)
{
	t_IPC_DATA data = {0};
	
	Ipc_Data_SetInt(&data,  &parent_id);
	Ipc_Data_SetString(&data, fullpath);
	s_ipc_call_send( FUNC_Dms_ShareUsb_RemoveFolder, &data);
}	
STATIC int IpcRcv_Dms_ShareUsb_RemoveFolder(int hnd, int func_index, t_IPC_DATA *data)
{
	Raw_Dms_ShareUsb_RemoveFolder(Ipc_Data_GetInt(data), Ipc_Data_GetString(data, 2));
	return 0;
}	

/*-------------------------------*/
int IpcCommon_Server_Func(int hnd, int func_index, t_IPC_DATA *data)
{
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, func_index, "func_index = ");
	int ret = s_common_func[func_index](hnd, func_index, data);
	HT_DBG_FUNC_END(ret, "ret = ");
	return ret;
}	

//STATIC int s_IpcRcv_Callback(enum_DlnaCallback eType, int value, char *str, void *user)
int IpcCommon_Client_Func(int hnd, int func_index, t_IPC_DATA *data)
{
	enum_DlnaCallback eType = (enum_DlnaCallback)Ipc_Data_GetInt(data);
	int *value = (int*)Ipc_Data_GetString(data, 2);
	int *user = (int*)Ipc_Data_GetString(data, 3);
	char *str = Ipc_Data_GetString(data, 4);
	int ret = -1;
	
	if( s_IPC_client_side_callback )
		ret = s_IPC_client_side_callback(eType, *value, str, (void*)(*user));
	
	t_IPC_DATA ack = {0};
	Ipc_Data_SetInt(&ack, &ret);
	Ipc_Ack_Value(hnd, &ack);
	return 1;
}

static void IpcCommon_Server_Init(void)
{
	t_IPC_RCV_FUNC *list = s_common_func;
	
	xxx_register_func(list, FUNC_Dlna_ModeInit, 	IpcRcv_Dlna_ModeInit);
	
	xxx_register_func(list, FUNC_UpnpStackInit, 	IpcRcv_UpnpStackInit);
	xxx_register_func(list, FUNC_UpnpStackDestroy, 	IpcRcv_UpnpStackDestroy);
	
	xxx_register_func(list, FUNC_Dmc_Init,			IpcRcv_Dmc_Init);
	xxx_register_func(list, FUNC_Dmc_Start,			IpcRcv_Dmc_Start);
	xxx_register_func(list, FUNC_Dmc_Stop,			IpcRcv_Dmc_Stop);
	xxx_register_func(list, FUNC_Dmc_SearchDms,		IpcRcv_Dmc_SearchDms);
	xxx_register_func(list, FUNC_Dmc_Destroy,		IpcRcv_Dmc_Destroy);

	xxx_register_func(list, FUNC_Dms_Init,			IpcRcv_Dms_Init);
	xxx_register_func(list, FUNC_Dms_Start,			IpcRcv_Dms_Start);
	xxx_register_func(list, FUNC_Dms_Stop,			IpcRcv_Dms_Stop);
	xxx_register_func(list, FUNC_Dms_SetNewName,	IpcRcv_Dms_SetNewName);
	xxx_register_func(list, FUNC_Dms_AddPvrItem,	IpcRcv_Dms_AddPvrItem);
	xxx_register_func(list, FUNC_Dms_RemovePvrItem,	IpcRcv_Dms_RemovePvrItem);
	xxx_register_func(list, FUNC_Dms_FindFolderShared,IpcRcv_Dms_FindFolderShared);
	xxx_register_func(list, FUNC_Dms_AddFolderShared,IpcRcv_Dms_AddFolderShared);
	xxx_register_func(list, FUNC_Dms_RemoveFolderShared,IpcRcv_Dms_RemoveFolderShared);

	xxx_register_func(list, FUNC_Dms_SetPause,		IpcRcv_Dms_SetPause);
	xxx_register_func(list, FUNC_Dms_SetStoppingHttp,IpcRcv_Dms_SetStoppingHttp);

	xxx_register_func(list, FUNC_Dms_VirtualFileTree_AddChild,			IpcRcv_Dms_VirtualFileTree_AddChild);
	xxx_register_func(list, FUNC_Dms_VirtualFileTree_DeleteChild,		IpcRcv_Dms_VirtualFileTree_DeleteChild);
	xxx_register_func(list, FUNC_Dms_VirtualFileTree_DeleteContainer,	IpcRcv_Dms_VirtualFileTree_DeleteContainer);
	xxx_register_func(list, FUNC_Dms_VirtualFileTree_GetInfo,			IpcRcv_Dms_VirtualFileTree_GetInfo);
	xxx_register_func(list, FUNC_Dms_VirtualFileTree_GetChildID,		IpcRcv_Dms_VirtualFileTree_GetChildID);
	xxx_register_func(list, FUNC_Dms_VirtualFileTree_GetParentID,		IpcRcv_Dms_VirtualFileTree_GetParentID);
	xxx_register_func(list, FUNC_Dms_ShareUsb_AddFolder,				IpcRcv_Dms_ShareUsb_AddFolder);
	xxx_register_func(list, FUNC_Dms_ShareUsb_RemoveFolder,				IpcRcv_Dms_ShareUsb_RemoveFolder);
}

/*-------------------------------*/
void xxx_register_func(t_IPC_RCV_FUNC *list, int index, t_IPC_RCV_FUNC func)
{
	list[index] = func;
}	


int g_ipc_client = 0;
int g_ipc_server = 0;
#define FIFO_NAME			"dlna"

static void s_api_ipc_create_server(void)
{
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
	
	int handle = Ipc_Server_Create(FIFO_NAME, enum_IPC_MOD_MAXIMUM);
	Ipc_Register_Module(handle, enum_IPC_MOD_COMMON, IpcCommon_Server_Func);
	Ipc_Register_Module(handle, enum_IPC_MOD_HUAWEI, IpcHuawei_Server_Func);
	Ipc_Register_Module(handle, enum_IPC_MOD_YUXING_FAKE, IpcYxFake_Server_Func);

	IpcCommon_Server_Init();
	IpcHuawei_Server_Init();
	IpcYxFake_Server_Init();

	g_ipc_server = handle;
	HT_DBG_FUNC_END(handle, "g_ipc_server = ");
}
static void s_api_ipc_create_client(void)
{
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
	
	int handle = Ipc_Client_Create(FIFO_NAME, enum_IPC_MOD_MAXIMUM);
	Ipc_Register_Module(handle, enum_IPC_MOD_COMMON, IpcCommon_Client_Func);
	Ipc_Register_Module(handle, enum_IPC_MOD_HUAWEI, IpcHuawei_Client_Func);
	Ipc_Register_Module(handle, enum_IPC_MOD_YUXING_FAKE, IpcYxFake_Client_Func);

	IpcHuawei_Client_Init();
	IpcYxFake_Client_Init();
	
	g_ipc_client = handle;
	HT_DBG_FUNC_END(handle, "g_ipc_client = ");
}
static void s_IpcRcv_Init_1(int uid)
{
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, uid, "uid = ");
	pid_t pid=fork();
	if (pid < 0)
	{
		HT_DBG_FUNC( pid, "error in fork! pid = ");
	}
	else if (pid == 0)
	{
		HT_DBG_FUNC( getpid(), "i am the child process, my process id = ");
		HT_DBG_FUNC( uid, "my uid = ");

        if(uid)
		    setuid(uid); 
//        HySDK_PvrInit(1);
        
		s_api_ipc_create_server();
		int runing = 1;
		while(runing)
			sleep(1);
	}
	else
	{
		HT_DBG_FUNC( getpid(), "i am the parent process, my process id = ");
		s_api_ipc_create_client();
	}
}

void Ipc_ModeInit(enum_DlnaIPC ipc_mode, int uid)
{
	switch(ipc_mode)
	{
		case enum_DlnaIPC_InSameProcess:
            Ipc_MakeFifo(FIFO_NAME);
			s_api_ipc_create_server();
			s_api_ipc_create_client();
			break;
			
		case enum_DlnaIPC_StartServerByFork:
            Ipc_MakeFifo(FIFO_NAME);
			s_IpcRcv_Init_1(uid);
			break;
			
		case enum_DlnaIPC_StartServerByShell:
			s_api_ipc_create_client();
			break;
			
		default:
			break;
	}		
}

