#ifndef _DLNA_UPNP_H_
#define _DLNA_UPNP_H_

#include "dlna_type.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* firstly called function */
extern void Dlna_IpcModeInit(enum_DlnaIPC ipc_mode, int uid);	// must firstly call this function only if you want to run dlna module in another process.
void Dlna_ModeInit(enum_DlnaAppMode app_mode, t_DLNA_CALLBACK callback, t_DLNA_EVENT EventHandler, enum_DlnaEvent EventHandlerType);


/* upnp statck */
extern int UpnpStackInit(char *ifname, char *ip_address, char *web_dir_path);
extern void UpnpStackDestroy(void);


/* basic dmc interface*/
extern int Dmc_Init(char *setting);
extern int Dmc_Start(void);
extern void Dmc_Stop(void);
extern void Dmc_SearchDms(int remove_all);
extern void Dmc_Destroy(void);


/* basic dms interface */
extern int Dms_Init(char *dms_name);
extern int Dms_Start(void);
extern int Dms_Stop(void);
extern int Dms_SetNewName( char *new_name);
extern int Dms_AddPvrItem(char *path, char *itemName);
extern int Dms_RemovePvrItem(char *path, char *itemName);
extern int Dms_FindFolderShared(char *fullpath);
extern int Dms_AddFolderShared(char *fullpath, char *title, int isPvr);
extern int Dms_RemoveFolderShared(char *fullpath);
extern int Dms_SetPause(int mode);
extern int Dms_SetStoppingHttp(int mode);

int Dms_VirtualFileTree_AddChild(int parent_id, char *info);
void Dms_VirtualFileTree_DeleteChild(int parent_id, int child_id);
void Dms_VirtualFileTree_DeleteContainer(int parent_id, int container_id, int keep_if_child);
void Dms_VirtualFileTree_GetInfo(int id, int onlybasic, char *buf, int len);
int Dms_VirtualFileTree_GetChildID(int parent_id, char type, char *keyword, int is_recursive);
int Dms_VirtualFileTree_GetParentID(int my_id);

int Dms_ShareUsb_AddFolder(int parent_id, char *fullpath);
int Dms_ShareUsb_RemoveFolder(int parent_id, char *fullpath);

/* js parser for yuxing fake object */
extern int Dlna_JseRead( const char *name, char *buf, int len);	//this function just parse the js interface prefixed with "DLNA."
extern int Dlna_JseWrite( const char *name, char *buf, int len); 	//this function just parse the js interface prefixed with "DLNA."
/* special js parser for huawei */
extern int Dmc_HuaweiJse(const char *func, const char *para, char *value, int len, int readonly); //specially for Huawei's requirement
extern int Dmp_HuaweiJse( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult, int readonly );
/* huawei v1r5 */
extern int Dmc_HuaweiJse_V1R5(const char *func, const char *para, char *value, int len, int readonly); //specially for Huawei's requirement

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DLNA_UPNP_H_ */

