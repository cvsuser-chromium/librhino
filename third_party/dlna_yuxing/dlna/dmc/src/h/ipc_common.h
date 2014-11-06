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
*    filename:			ipc.h
*    author(s):			yanyongmeng
*    version:			0.1
*    date:				2011/2/12
* History
*/
/*-----------------------------------------------------------------------------------------------*/
#ifndef __IPC_COMMON_H__
#define __IPC_COMMON_H__

#include "dlna_type.h"
#include "ipc.h"

typedef enum _enum_ipc_module_
{
	enum_IPC_MOD_COMMON = 0, 
	enum_IPC_MOD_HUAWEI,
	enum_IPC_MOD_YUXING_FAKE,	
	enum_IPC_MOD_YUXING_OBJECT,	
	
	enum_IPC_MOD_MAXIMUM,
}enum_IPC_MODULE;

typedef int (*t_DLNA_EVENT_EX)(enum_IPC_MODULE module, enum_DlnaEvent type, int handle, char *str);

extern int g_ipc_client;
extern int g_ipc_server;
extern t_DLNA_EVENT_EX g_IPC_client_side_EventHandler;

void Ipc_ModeInit(enum_DlnaIPC ipc_mode, int uid);

extern void xxx_register_func(t_IPC_RCV_FUNC *list, int index, t_IPC_RCV_FUNC func);
int IpcCommon_Server_Func(int hnd, int func_index, t_IPC_DATA *data);
int IpcCommon_Client_Func(int hnd, int func_index, t_IPC_DATA *data);


int IpcHuawei_Client_Func(int hnd, int func_index, t_IPC_DATA *data);
int IpcHuawei_Server_Func(int hnd, int func_index, t_IPC_DATA *data);
void IpcHuawei_Client_Init(void);
void IpcHuawei_Server_Init(void);

int IpcYxFake_Client_Func(int hnd, int func_index, t_IPC_DATA *data);
int IpcYxFake_Server_Func(int hnd, int func_index, t_IPC_DATA *data);
void IpcYxFake_Client_Init(void);
void IpcYxFake_Server_Init(void);


#endif /*__IPC_COMMON_H__ */


