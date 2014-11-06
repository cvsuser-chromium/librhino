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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>

#include "dlna_api.h"
#include "dlna_api_raw.h"
#include "dlna_api_ipc.h"

#include "ipc.h"
#include "ipc_common.h"
#include "hitTime.h"
#include "dms.h"
#include "build_info.h"

#define IS_IpcMode_MODE(x)  (x)

static enum_DlnaIPC s_ipc_mode = 0;
static t_DLNA_EVENT s_event_handler = NULL;

static int s_EventHandler(enum_IPC_MODULE module, enum_DlnaEvent type, int handle, char *str)
{
  return s_event_handler( type, handle, str);
}

void Dlna_IpcModeInit(enum_DlnaIPC ipc_mode, int uid)
{
	printf("\n\nDLNA Lib build date: %s build version:%s platform:%s\n",build_date,build_version,build_platform);
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_KEY, ipc_mode, NULL);

	s_ipc_mode = ipc_mode;
	if( IS_IpcMode_MODE(s_ipc_mode) )
		Ipc_ModeInit(s_ipc_mode, uid);

	HT_DBG_FUNC_END(uid, 0);
}

void Dlna_ModeInit(enum_DlnaAppMode app_mode, t_DLNA_CALLBACK callback, t_DLNA_EVENT EventHandler, enum_DlnaEvent EventHandlerType)
{
  t_DLNA_EVENT_EX handler;

  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_KEY, app_mode, NULL);

  s_event_handler = EventHandler;
  handler = EventHandler? s_EventHandler:NULL;
  if( IS_IpcMode_MODE(s_ipc_mode) )
    IpcMode_Dlna_ModeInit(app_mode, callback, handler, EventHandlerType);
  else
    Raw_Dlna_ModeInit(app_mode, callback, handler, EventHandlerType);

  Raw_Playback_ModeInit(app_mode, callback, handler, EventHandlerType);

  HT_DBG_FUNC_END(s_ipc_mode, 0);
}

int UpnpStackInit(char *ifname, char *ip_address, char *web_dir_path)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_KEY, s_ipc_mode, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_UpnpStackInit( ifname, ip_address, web_dir_path );
  else
    ret = UpnpStackInitRaw( ifname, ip_address, web_dir_path );
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}

void UpnpStackDestroy(void)
{
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_KEY, s_ipc_mode, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    IpcMode_UpnpStackDestroy();
  else
    UpnpStackDestroyRaw();
  HT_DBG_FUNC_END(0, 0);
}



/* dmc */
int Dmc_Init(char *setting)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) ) {
    ret = IpcMode_Dmc_Init(setting);
  } else {
    ret = Raw_Dmc_Init(setting);
  }
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}

int Dmc_Start(void)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dmc_Start();
  else
    ret = Raw_Dmc_Start();
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}
void Dmc_Stop(void)
{
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    IpcMode_Dmc_Stop();
  else
    Raw_Dmc_Stop();
  HT_DBG_FUNC_END(0, 0);
}
void Dmc_SearchDms(int remove_all)
{
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    IpcMode_Dmc_SearchDms(remove_all);
  else
    Raw_Dmc_SearchDms(remove_all);
  HT_DBG_FUNC_END(0, 0);
}
void Dmc_Destroy(void)
{
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    IpcMode_Dmc_Destroy();
  else
    Raw_Dmc_Destroy();
  HT_DBG_FUNC_END(0, 0);
}
#if 0
/* for metadata may be too long to pass through ipc buffer, so Dmc_Huawei_BrowseMetadata is deprecated */
int Dmc_Huawei_BrowseMetadata( char *udn, char *id, char **metadata)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dmc_Huawei_BrowseMetadata( udn, id, metadata);
  else
    ret = Raw_Dmc_Huawei_BrowseMetadata( udn, id, metadata);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}
#endif

int Dmc_Huawei_GetDmsIp( char *udn, char **dms_ip)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dmc_Huawei_GetDmsIp( udn, dms_ip);
  else
    ret = Raw_Dmc_Huawei_GetDmsIp( udn, dms_ip);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}
int Dmc_Huawei_GetBestUri( char *udn, char *id, char **uri)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dmc_Huawei_GetBestUri( udn, id, uri);
  else
    ret = Raw_Dmc_Huawei_GetBestUri( udn, id, uri);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}
int Dmc_Huawei_GetBestMediaInfo( char *udn, char *id, t_MEDIA_INFO *minfo)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dmc_Huawei_GetBestMediaInfo( udn, id, minfo);
  else
    ret = Raw_Dmc_Huawei_GetBestMediaInfo( udn, id, minfo);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}


/* dms */
int Dms_Init(char *cfg)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_Init(cfg );
  else
    ret = Raw_Dms_Init(cfg );
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}

int Dms_Start(void)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_Start(  );
  else
    ret = Raw_Dms_Start(  );
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}
int Dms_Stop(void)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_Stop( );
  else
    ret = Raw_Dms_Stop( );
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}
int Dms_SetNewName( char *new_name)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_SetNewName (new_name);
  else
    ret = Raw_Dms_SetNewName (new_name);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}

int Dms_AddPvrItem(char *path, char *itemName)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_AddPvrItem (path, itemName);
  else
    ret = Raw_Dms_AddPvrItem (path, itemName);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}
int Dms_RemovePvrItem(char *path, char *itemName)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_RemovePvrItem(path, itemName);
  else
    ret = Raw_Dms_RemovePvrItem(path, itemName);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}


int Dms_FindFolderShared(char *fullpath)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_FindFolderShared(fullpath);
  else
    ret = Raw_Dms_FindFolderShared(fullpath);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}
int Dms_AddFolderShared(char *fullpath, char *title, int isPvr)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_AddFolderShared(fullpath, title, isPvr);
  else
    ret = Raw_Dms_AddFolderShared(fullpath, title, isPvr);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}
int Dms_RemoveFolderShared(char *fullpath)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_RemoveFolderShared(fullpath);
  else
    ret = Raw_Dms_RemoveFolderShared(fullpath);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}

int Dms_SetPause(int mode)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_SetPause(mode);
  else
    ret = Raw_Dms_SetPause(mode);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}

int Dms_SetStoppingHttp(int mode)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_SetStoppingHttp(mode);
  else
    ret = Raw_Dms_SetStoppingHttp(mode);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}

/*---------------------*/

int Dms_VirtualFileTree_AddChild(int parent_id, char *info)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_VirtualFileTree_AddChild(parent_id, info);
  else
    ret = Raw_Dms_VirtualFileTree_AddChild(parent_id, info);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}
void Dms_VirtualFileTree_DeleteChild(int parent_id, int child_id)
{
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    IpcMode_Dms_VirtualFileTree_DeleteChild(parent_id, child_id);
  else
    Raw_Dms_VirtualFileTree_DeleteChild(parent_id, child_id);
  HT_DBG_FUNC_END(0, 0);
}
void Dms_VirtualFileTree_DeleteContainer(int parent_id, int container_id, int keep_if_child)
{
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    IpcMode_Dms_VirtualFileTree_DeleteContainer(parent_id, container_id, keep_if_child);
  else
    Raw_Dms_VirtualFileTree_DeleteContainer(parent_id, container_id, keep_if_child);
  HT_DBG_FUNC_END(0, 0);
}

void Dms_VirtualFileTree_GetInfo(int id, int onlybasic, char *buf, int len)
{
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    IpcMode_Dms_VirtualFileTree_GetInfo(id, onlybasic, buf, len);
  else
    Raw_Dms_VirtualFileTree_GetInfo(id, onlybasic, buf, len);
  HT_DBG_FUNC_END(0, 0);
}

int Dms_VirtualFileTree_GetChildID(int parent_id, char type, char *keyword, int is_recursive)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_VirtualFileTree_GetChildID(parent_id, type, keyword, is_recursive);
  else
    ret = Raw_Dms_VirtualFileTree_GetChildID(parent_id, type, keyword, is_recursive);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}
int Dms_VirtualFileTree_GetParentID(int my_id)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_VirtualFileTree_GetParentID(my_id);
  else
    ret = Raw_Dms_VirtualFileTree_GetParentID(my_id);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}

int Dms_ShareUsb_AddFolder(int parent_id, char *fullpath)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_ShareUsb_AddFolder(parent_id, fullpath);
  else
    ret = Raw_Dms_ShareUsb_AddFolder(parent_id, fullpath);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}
int Dms_ShareUsb_RemoveFolder(int parent_id, char *fullpath)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dms_ShareUsb_RemoveFolder(parent_id, fullpath);
  else
    ret = Raw_Dms_ShareUsb_RemoveFolder(parent_id, fullpath);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}
/*
int Raw_Dmr_Init(void);
int Raw_Dmr_Start(DMREVRNT_CALLBACK Callback);
int Raw_Dmr_Stop(void);
*/

//removed by teddy. at 2013.08.26 Mon 14:58:58.
#if 0
/* dmr */
int Dmr_Init(char *cfg)
{
	int ret;
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
	/*if( IS_IpcMode_MODE(s_ipc_mode) )
		ret = IpcMode_Dms_Init(cfg );
	else*/
		ret = Raw_Dmr_Init( );
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

int Dmr_Start(DMREVRNT_CALLBACK Callback)
{
	int ret;
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
	ret = Raw_Dmr_Start(Callback);

	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
int Dmr_Stop(void)
{
	int ret;
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_FEW, 0, NULL);
	//if( IS_IpcMode_MODE(s_ipc_mode) )
	//	ret = IpcMode_Dms_Stop( );
	//else
	ret = Raw_Dmr_Stop( );
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}
#endif
/*---------------------*/
int Dlna_JseRead( const char *name, char *buf, int len)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_MANY, len, buf);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dlna_JsRead(name, buf, len);
  else
    ret = Raw_Dlna_JsRead(name, buf, len);
  HT_DBG_FUNC_END(ret, buf);
  return ret;
}

int Dlna_JseWrite( const char *name, char *buf, int len)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_MANY, len, buf);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dlna_JsWrite(name, buf, len);
  else
    ret = Raw_Dlna_JsWrite(name, buf, len);
  HT_DBG_FUNC_END(ret, buf);
  return ret;
}

int Dmc_HuaweiJse(const char *func, const char *para, char *value, int len, int readonly)
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_MANY, 0, func);
  if( IS_IpcMode_MODE(s_ipc_mode) )
    ret = IpcMode_Dmc_HuaweiJse(func, para, value, len, readonly);
  else
    ret = Raw_Dmc_HuaweiJse(func, para, value, len);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}
int Dmc_HuaweiJse_V1R5(const char *func, const char *para, char *value, int len, int readonly)
{
	int ret;
	HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_KEY, 0, func);
	if( IS_IpcMode_MODE(s_ipc_mode) )
		ret = IpcMode_Dmc_HuaweiJse_V1R5(func, para, value, len, readonly);
	else
		ret = Raw_Dmc_HuaweiJse_V1R5(func, para, value, len);
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

int Dmp_HuaweiJse( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult, int readonly )
{
  int ret;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_MANY, 0, aFieldName);
  ret = Raw_Dmp_HuaweiJse(aFieldName, aFieldParam, aFieldValue, aResult);
  HT_DBG_FUNC_END(ret, 0);
  return ret;
}




