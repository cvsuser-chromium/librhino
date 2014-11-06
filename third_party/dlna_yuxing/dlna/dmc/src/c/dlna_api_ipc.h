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
#ifndef __DLNA_API_IPC_H__
#define __DLNA_API_IPC_H__

#include "dlna_type.h"

 void IpcMode_Dlna_ModeInit(enum_DlnaAppMode app_mode, t_DLNA_CALLBACK callback, t_DLNA_EVENT_EX EventHandler, enum_DlnaEvent EventHandlerType);

/* upnp */
extern int IpcMode_UpnpStackInit(char *ifname, char *ip_address, char *web_dir_path);
extern void IpcMode_UpnpStackDestroy(void);

/* dmc */
extern int IpcMode_Dmc_Init(char *setting);
extern int IpcMode_Dmc_Start(void); 
extern void IpcMode_Dmc_Stop(void); 
extern void IpcMode_Dmc_SearchDms(int remove_all);
extern void IpcMode_Dmc_Destroy(void);
//extern int IpcMode_Dmc_Huawei_BrowseMetadata( char *udn, char *id, char **metadata);
extern int IpcMode_Dmc_Huawei_GetDmsIp( char *udn, char **dms_ip);
extern int IpcMode_Dmc_Huawei_GetBestUri( char *udn, char *id, char **uri);
extern int IpcMode_Dmc_Huawei_GetBestMediaInfo( char *udn, char *id, t_MEDIA_INFO *minfo);

/* dmr */ 
extern int IpcMode_Dmr_Start(void);
extern void IpcMode_Dmr_Stop(void);
extern int IpcMode_Dmr_DealRemoteRequest( int AcceptOrNot);

/* dms */
extern int IpcMode_Dms_Init(char *dms_name);
extern int IpcMode_Dms_Start(void);
extern int IpcMode_Dms_Stop(void);
extern int IpcMode_Dms_SetNewName( char *new_name);
extern int IpcMode_Dms_AddPvrItem(char *path, char *itemName);
extern int IpcMode_Dms_RemovePvrItem(char *path, char *itemName);
extern int IpcMode_Dms_FindFolderShared(char *fullpath);
extern int IpcMode_Dms_AddFolderShared(char *fullpath, char *title, int isPvr);
extern int IpcMode_Dms_RemoveFolderShared(char *fullpath);
extern int IpcMode_Dms_SetPause(int mode);
extern int IpcMode_Dms_SetStoppingHttp(int mode);

int IpcMode_Dms_VirtualFileTree_AddChild(int parent_id, char *info);
void IpcMode_Dms_VirtualFileTree_DeleteChild(int parent_id, int child_id);
void IpcMode_Dms_VirtualFileTree_DeleteContainer(int parent_id, int container_id, int keep_if_child);
void IpcMode_Dms_VirtualFileTree_GetInfo(int id, int onlybasic, char *buf, int len);
int IpcMode_Dms_VirtualFileTree_GetChildID(int parent_id, char type, char *keyword, int is_recursive);
int IpcMode_Dms_VirtualFileTree_GetParentID(int my_id);

int IpcMode_Dms_ShareUsb_AddFolder(int parent_id, char *fullpath);
int IpcMode_Dms_ShareUsb_RemoveFolder(int parent_id, char *fullpath);

/* yuxing jse */
extern int IpcMode_Dlna_JsRead( const char *name, char *buf, int len);	//this function just parse the js interface prefixed with "DLNA."
extern int IpcMode_Dlna_JsWrite( const char *name, char *buf, int len); 	//this function just parse the js interface prefixed with "DLNA."
/* huawei jse */
extern int IpcMode_Dmc_HuaweiJse(const char *func, const char *para, char *value, int len, int readonly); //specially for Huawei's requirement
extern int IpcMode_Dmp_HuaweiJse( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult, int readonly );
/* huawei jse v1r5*/
extern int IpcMode_Dmc_HuaweiJse_V1R5(const char *func, const char *para, char *value, int len, int readonly);

#endif /*__DLNA_API_IPC_H__ */

