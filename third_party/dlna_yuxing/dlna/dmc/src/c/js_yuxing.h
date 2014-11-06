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
*    filename:			js_yuxing.h
*    author(s):			yanyongmeng
*    version:			0.1
*    date:				2011/2/12
* History
*/
/*-----------------------------------------------------------------------------------------------*/
#ifndef __JS_YUXING_H__
#define __JS_YUXING_H__

#include "dlna_type.h"
#include "js_eventqueue.h"

typedef struct _yx_dms_list_s_ *pYXCDL;
typedef struct _yx_dms_list_s_ {
	DEFAULT_DISPATCH_FUNC;
	int				(*GetDmsNumber)(pYXCDL me);
	t_DLNA_JSON		(*GetDmsLists)(pYXCDL me, int postion, int count);
	char*			(*GetDmsDescriptionXml)(pYXCDL me, char *dmsUdn);
	void			(*SearchDms)(pYXCDL me, int remove_all);
	void			(*Release)(pYXCDL me);

	/* private data */
//	t_DLNA_JSON 	(*GetEvent)(pYXCDL me);
	int 			pHandle;
	pClassEventQueue list;
	int 			bufLen;
}YXClassDmsList;


typedef struct _yx_content_directory_s_ *pYXCCD;
typedef struct _yx_content_directory_s_ {
	DEFAULT_DISPATCH_FUNC;
	int				(*SetDmsUdn)(pYXCCD me, char *dmsUdn); 
	char*			(*GetDmsUdn)(pYXCCD me); 
	char*			(*GetSearchCapabilities)(pYXCCD me); 
	char*			(*GetSortCapabilities)(pYXCCD me); 	
	
	int				(*SetFilter)(pYXCCD me, char *filter); 
	int				(*SetSearchCriteria)(pYXCCD me, char *searchCriteria); 
	int 			(*SetSortCriteria)(pYXCCD me, char *sortCriteria); 
	int 			(*SetResultFormat)(pYXCCD me, int format); 
	
	enum_DLNA_RET	(*BrowseMetedata)(pYXCCD me, t_DLNA_OBJECTID objectId);
	enum_DLNA_RET	(*BrowseChildren)(pYXCCD me, t_DLNA_OBJECTID objectId, int startingIndex, int requestedCount);
	enum_DLNA_RET	(*Search)(pYXCCD me, t_DLNA_OBJECTID objectId, int startingIndex, int requestedCount);
	t_DLNA_JSON		(*OnResultPartial)(pYXCCD me, int format);

	t_DLNA_JSON		(*SyncBrowseMetedata)(pYXCCD me, t_DLNA_OBJECTID objectId);
	t_DLNA_JSON		(*SyncBrowseChildren)(pYXCCD me, t_DLNA_OBJECTID objectId, int startingIndex, int requestedCount);
	t_DLNA_JSON		(*SyncSearch)(pYXCCD me, t_DLNA_OBJECTID objectId, int startingIndex, int requestedCount);

	void			(*Reset)(pYXCCD me); 
	void			(*Release)(pYXCCD me); 
	
	/* private data */
//	t_DLNA_JSON 	(*GetEvent)(pYXCCD me);
	int 			pHandle;
	pClassEventQueue list;
	int 			format;
	int 			bufLen;
} YXClassContentDirectory;

extern int DmsList_Create( void);
extern int CD_create(void);


#endif /*__JS_YUXING_H__ */

