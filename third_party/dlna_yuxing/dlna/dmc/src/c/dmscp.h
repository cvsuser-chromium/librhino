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
*    filename:			dmscp.h
*    author(s):			yanyongmeng
*    version:			0.1
*    date:				2011/2/12
* History
*/
/*-----------------------------------------------------------------------------------------------*/
#ifndef __DMSCP_H__
#define __DMSCP_H__

#include <semaphore.h>
#include "ixml.h"
#include "device.h"

#include "dlna_type.h"

typedef void	(*t_DMC_DMS_UPDATED_CALLBACK)(char *dmsUdn, Device *d, char *alias, int action, int hnd);
typedef void	(*t_DMC_BROWSE_CALLBACK)(int ret, int hnd);
typedef void	(*t_DMC_SERVICE_UPDATED_CALLBACK)(char *dmsUdn, char *name, char *value, int hnd);


typedef enum _e_dlna_ret_ {
	enum_DLNA_ACTION_OK 	= 0,
	enum_DLNA_PARA_ERROR,
		
	enum_DLNA_NO_THIS_DMS,
	enum_DLNA_DMS_BUSY,
	
	enum_DLNA_NO_THIS_DMR,
	enum_DLNA_DMR_BUSY,
} enum_DLNA_RET;

typedef struct _dms_list_s_ *pClassDmsList;
typedef struct _dms_list_s_ {
//	DEFAULT_DISPATCH_FUNC;
//	t_DLNA_JSON		(*GetEvent)(pClassDmsList me);
	
	int				(*GetDmsNumber)(pClassDmsList me);
	int				(*GetDmsDescriptionXml )(pClassDmsList me, char *dmsUdn, char *buf, int bufLen);
	void			(*SearchDms)(pClassDmsList me, int remove_all);
	void			(*Release)(pClassDmsList me);
	
	t_DMC_DMS_UPDATED_CALLBACK	OnDmsUpdated;
	t_DMC_SERVICE_UPDATED_CALLBACK  OnServiceUpdated;
	int							parent;
}ClassDmsList;

typedef struct _content_directory_s_ *pClassContentDirectory;
typedef struct _content_directory_s_ {
//	DEFAULT_DISPATCH_FUNC;
//	t_DLNA_JSON 	(*GetEvent)(pClassContentDirectory me);
	
	int				(*SetDmsUdn)(pClassContentDirectory me, char *dmsUdn); 
	char*			(*GetDmsUdn)(pClassContentDirectory me); 
	int				(*GetSearchCapabilities)(pClassContentDirectory me, char *buf, int bufLen); 
	int				(*GetSortCapabilities)(pClassContentDirectory me, char *buf, int bufLen); 	
	
	int				(*SetFilter)(pClassContentDirectory me, char *filter); 
	int				(*SetSearchCriteria)(pClassContentDirectory me, char *searchCriteria); 
	int 			(*SetSortCriteria)(pClassContentDirectory me, int sort,int order,char *sort_Criteria); 
//	int				(*SetResultFormat)(pClassContentDirectory me, int format); 
	
	enum_DLNA_RET	(*BrowseMetedata)(pClassContentDirectory me, t_DLNA_OBJECTID objectId);
	enum_DLNA_RET	(*BrowseChildren)(pClassContentDirectory me, t_DLNA_OBJECTID objectId, int startingIndex, int requestedCount);
	enum_DLNA_RET	(*Search)(pClassContentDirectory me, t_DLNA_OBJECTID objectId, int startingIndex, int requestedCount);
//	t_DLNA_JSON		(*OnResultPartial)(pClassContentDirectory me, char *buf, int bufLen);
	
	void			(*Reset)(pClassContentDirectory me); 
	void			(*Release)(pClassContentDirectory me); 
	
		
	enum_DLNA_RET	(*SyncBrowseMetedata)(pClassContentDirectory me, t_DLNA_OBJECTID objectId);
	enum_DLNA_RET	(*SyncBrowseChildren)(pClassContentDirectory me, t_DLNA_OBJECTID objectId, int startingIndex, int requestedCount);
	enum_DLNA_RET	(*SyncSearch)(pClassContentDirectory me, t_DLNA_OBJECTID objectId, int startingIndex, int requestedCount);

	sem_t			semResultOK;
	int				syncBrowsing;
	
	char			udn[256];
	char			dmsIp[32];
	int				busy;
	
	sem_t			lock;
	int				killMeInCallback;
	
	char*			filter;
	char*			searchCriteria;
	char*			sortCriteria;
	
//	char			objectId[256];
	char*			ActionXml;
	IXML_Document*	ActionResult;
	int				nb_matched;
	int				nb_returned;
	int				updateID;
	IXML_Document*	subdoc;
	int				index;
	int				sdk_err;
	int				http_err;
	int				upnp_err;
	
	t_DMC_DMS_UPDATED_CALLBACK	OnDmsUpdated;
	t_DMC_BROWSE_CALLBACK		OnBrowseResult;
	t_DMC_SERVICE_UPDATED_CALLBACK  OnServiceUpdated;
	int							parent;
	int				(*SetObjectID)(pClassContentDirectory me, t_DLNA_OBJECTID id); 
    char*           objectid;
} ClassContentDirectory;

extern char *Dlna_strdup(const char *str);

int YX_Dmscp_Init(char *setting);
int YX_Dmscp_Destroy(void);
int YX_Dmscp_Start(void);
void YX_Dmscp_Stop(void);
void YX_Dmscp_SearchDms(int remove_all);

extern int YX_DmsList_Create( t_DMC_DMS_UPDATED_CALLBACK OnDmsUpdated, int cookie, t_DMC_SERVICE_UPDATED_CALLBACK OnServiceUpdated);
extern int YX_CD_Create(t_DMC_DMS_UPDATED_CALLBACK OnDmsUpdated, t_DMC_BROWSE_CALLBACK OnBrowseResult, int cookie, t_DMC_SERVICE_UPDATED_CALLBACK OnServiceUpdated);

//extern int YX_CD_IpMatched (pClassContentDirectory me, const char *uri);
extern int YX_CD_GetPreferred (pClassContentDirectory me, IXML_NodeList *reslist, int len, char **protocolInfo, char **url, t_MEDIA_INFO *mInfo);
extern char *YX_CD_GetThumbnailForVideo (pClassContentDirectory me, IXML_NodeList *reslist, int len);
extern char *YX_CD_GetThumbnailForImage (pClassContentDirectory me, IXML_NodeList *reslist, int len, int max_w, int max_h);


extern int YX_CD_SplitInfo(const char *protocol, char **transport, char **mimetype, char **dlna);

extern int YX_CD_GetMetadata(pClassContentDirectory me, char *id, char **metadata);
extern int YX_CD_GetBestRes (char *dms_ip, char *metadata, char **res);
extern int YX_CD_GetBestUri (char *dms_ip, char *metadata, char **uri);
extern int YX_CD_GetBestMediaInfo(char *dms_ip, char *metadata, t_MEDIA_INFO *minfo);


#endif /*__DMSCP_H__ */

