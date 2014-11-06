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
#ifndef __DLNA_API_RAW_H__
#define __DLNA_API_RAW_H__

#include "dlna_type.h"
#include "ipc.h"
#include "ipc_common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void Raw_Dlna_ModeInit(enum_DlnaAppMode app_mode, t_DLNA_CALLBACK callback, t_DLNA_EVENT_EX EventHandler, enum_DlnaEvent EventHandlerType);
void Raw_Playback_ModeInit(enum_DlnaAppMode app_mode, t_DLNA_CALLBACK callback, t_DLNA_EVENT_EX EventHandler, enum_DlnaEvent EventHandlerType);

/* upnp */
extern int UpnpStackInitRaw(char *ifname, char *ip_address, char *web_dir_path);
extern void UpnpStackDestroyRaw(void);

/* dmc */
extern int Raw_Dmc_Init(char *setting);
extern int Raw_Dmc_Start(void);
extern void Raw_Dmc_Stop(void);
extern void Raw_Dmc_SearchDms(int remove_all);
extern void Raw_Dmc_Destroy(void);
extern int Raw_Dmc_Huawei_BrowseMetadata(char *udn, char *id, char **metadata);
extern int Raw_Dmc_Huawei_GetDmsIp(char *udn, char **dms_ip);
extern int Raw_Dmc_Huawei_GetBestUri( char *udn, char *id, char **uri);
extern int Raw_Dmc_Huawei_GetBestMediaInfo( char *udn, char *id, t_MEDIA_INFO *minfo);

//int Raw_Dmr_Init(void);
//extern int Raw_Dmr_Start(DMREVRNT_CALLBACK Callback);
//extern int Raw_Dmr_Stop(void);
//extern int Raw_Dmr_DealRemoteRequest( int AcceptOrNot);

/* dms */
extern int Raw_Dms_Init(char *dms_name);
extern int Raw_Dms_Start(void);
extern int Raw_Dms_Stop(void);
extern int Raw_Dms_SetNewName( char *new_name);
extern int Raw_Dms_AddPvrItem(char *path, char *itemName);
extern int Raw_Dms_RemovePvrItem(char *path, char *itemName);
extern int Raw_Dms_FindFolderShared(char *fullpath);
extern int Raw_Dms_AddFolderShared(char *fullpath, char *title, int isPvr);
extern int Raw_Dms_RemoveFolderShared(char *fullpath);
extern int Raw_Dms_SetPause(int mode);
extern int Raw_Dms_SetStoppingHttp(int mode);
/* yuxing jse */
extern int Raw_Dlna_JsRead( const char *name, char *buf, int len);	//this function just parse the js interface prefixed with "DLNA."
extern int Raw_Dlna_JsWrite( const char *name, char *buf, int len); 	//this function just parse the js interface prefixed with "DLNA."
/* huawei jse */
extern int Raw_Dmc_HuaweiJse(const char *func, const char *para, char *value, int len); //specially for Huawei's requirement
extern int Raw_Dmp_HuaweiJse( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult );
/* huawei vir5 jse */
extern int Raw_Dmc_HuaweiJse_V1R5(const char *func, const char *para, char *value, int len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__DLNA_API_RAW_H__ */

