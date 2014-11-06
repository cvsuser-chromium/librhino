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
*    filename:			js_yuxing.c
*    author(s):			yanyongmeng
*    version:			0.1
*    date:				2011/2/12
* History
*/
/*-----------------------------------------------------------------------------------------------*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "device_list.h"
#include "xml_util.h"


#include "js_common.h"
#include "dmscp.h"
#include "js_yuxing.h"
#include "hitTime.h"
#include "ipc.h"
#include "ipc_common.h"

typedef enum _function_name_list_
{
	FUNC_YXDL_GetDmsLists = 0,
	FUNC_YXDL_GetDmsDescriptionXml,
	FUNC_YXDL_GetDmsNumber,
	FUNC_YXDL_SearchDms,
	FUNC_YXDL_Release,
	
	FUNC_YXCD_SetDmsUdn,
	FUNC_YXCD_GetDmsUdn,
	FUNC_YXCD_GetSearchCapabilities,
	FUNC_YXCD_GetSortCapabilities,
	FUNC_YXCD_SetFilter,
	FUNC_YXCD_SetSearchCriteria,
	FUNC_YXCD_SetSortCriteria,
	FUNC_YXCD_SetResultFormat,
	FUNC_YXCD_BrowseMetedata,
	FUNC_YXCD_BrowseChildren,
	FUNC_YXCD_Search,
	FUNC_YXCD_OnResultPartial,
	FUNC_YXCD_Reset,
	FUNC_YXCD_Release,
	FUNC_YXCD_SyncBrowseChildren,
	FUNC_YXCD_SyncBrowseMetedata,
	FUNC_YXCD_SyncSearch,
	
	FUNC_SENDER_MAX,
}t_FUNC_NAME;

static void s_ipc_call_send(int type, t_IPC_DATA *send)
{
	Ipc_CallAndReturn(g_ipc_client, 1, type, send);
}


//////////////////////////////////////////////////////////////////////////
static t_DLNA_JSON s_Ipc_YXDL_GetDmsLists( pYXCDL me, int start, int count)	//512*50 Bytes
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	Ipc_Data_SetInt(&data,  &start);
	Ipc_Data_SetInt(&data,  &count);
	
	s_ipc_call_send( FUNC_YXDL_GetDmsLists, &data);

	int len = me->bufLen;
	char *ret = Dlna_GetStringBuf(&len);
	strcpy(ret, Ipc_Data_GetFirstString(&data));
	
	return ret;
}

static t_DLNA_JSON s_Ipc_YXDL_GetDmsDescriptionXml (pYXCDL me, char *DmsUdn)
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	Ipc_Data_SetString(&data,  DmsUdn);
	
	s_ipc_call_send( FUNC_YXDL_GetDmsDescriptionXml, &data);

	int len = me->bufLen;
	char *ret = Dlna_GetStringBuf(&len);
	strcpy(ret, Ipc_Data_GetFirstString(&data));
	
	return ret;
}

static int s_Ipc_YXDL_GetDmsNumber (pYXCDL me)
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	
	s_ipc_call_send( FUNC_YXDL_GetDmsNumber, &data);

	int ret = Ipc_Data_GetInt(&data);

	return ret;
}

static void s_Ipc_YXDL_SearchDms (pYXCDL me, int remove_all)
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	Ipc_Data_SetInt(&data,  &remove_all);
	
	s_ipc_call_send( FUNC_YXDL_SearchDms, &data);
}

void s_Ipc_YXDL_Release(pYXCDL p)
{
	if( p)
	{
		t_IPC_DATA data = {0};
		
		Ipc_Data_SetInt(&data,	&(p->pHandle));
		
		s_ipc_call_send( FUNC_YXDL_Release, &data);
		
		free(p);
	}
}

int DmsList_Ipc_Create( void)
{
	int size = sizeof(YXClassDmsList);
	pYXCDL me= (pYXCDL)malloc(size);

	if( me )
	{
		memset(me,0,size);
		me->GetDmsNumber			= s_Ipc_YXDL_GetDmsNumber;
		me->GetDmsLists				= s_Ipc_YXDL_GetDmsLists;
		me->GetDmsDescriptionXml 	= s_Ipc_YXDL_GetDmsDescriptionXml;
		me->SearchDms				= s_Ipc_YXDL_SearchDms;
		me->Release					= s_Ipc_YXDL_Release;
	}

	return (int)me;
}


static int s_Ipc_YXCD_SetFilter(pYXCCD me, char *filter) 
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	Ipc_Data_SetString(&data,  filter);
	
	s_ipc_call_send( FUNC_YXCD_SetFilter, &data);

	int ret = Ipc_Data_GetInt(&data);

	return ret;
}
static int s_Ipc_YXCD_SetSearchCriteria(pYXCCD me, char *searchCriteria) 
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	Ipc_Data_SetString(&data,  searchCriteria);
	
	s_ipc_call_send( FUNC_YXCD_SetSearchCriteria, &data);

	int ret = Ipc_Data_GetInt(&data);

	return ret;
}
static int s_Ipc_YXCD_SetSortCriteria(pYXCCD me, char *sortCriteria) 
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	Ipc_Data_SetString(&data,  sortCriteria);
	
	s_ipc_call_send( FUNC_YXCD_SetSortCriteria, &data);

	int ret = Ipc_Data_GetInt(&data);

	return ret;
}
static int s_Ipc_YXCD_SetDmsUdn(pYXCCD me, char *dmsUdn) 
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	Ipc_Data_SetString(&data,  dmsUdn);
	
	s_ipc_call_send( FUNC_YXCD_SetDmsUdn, &data);

	int ret = Ipc_Data_GetInt(&data);

	return ret;
}
static char* s_Ipc_YXCD_GetDmsUdn(pYXCCD me) 
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	
	s_ipc_call_send( FUNC_YXCD_GetDmsUdn, &data);

	int len = me->bufLen;
	char *ret = Dlna_GetStringBuf(&len);
	strcpy(ret, Ipc_Data_GetFirstString(&data));
	
	return ret;
}
static int s_Ipc_YXCD_SetResultFormat(pYXCCD me, int format) 
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	Ipc_Data_SetInt(&data,  &format);
	
	s_ipc_call_send( FUNC_YXCD_SetResultFormat, &data);

	int ret = Ipc_Data_GetInt(&data);

	return ret;
}
static char* s_Ipc_YXCD_GetSearchCapabilities(pYXCCD me) 
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	
	s_ipc_call_send( FUNC_YXCD_GetSearchCapabilities, &data);

	int len = me->bufLen;
	char *ret = Dlna_GetStringBuf(&len);
	strcpy(ret, Ipc_Data_GetFirstString(&data));
	
	return ret;
}
static char* s_Ipc_YXCD_GetSortCapabilities(pYXCCD me) 	
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	
	s_ipc_call_send( FUNC_YXCD_GetSortCapabilities, &data);

	int len = me->bufLen;
	char *ret = Dlna_GetStringBuf(&len);
	strcpy(ret, Ipc_Data_GetFirstString(&data));
	
	return ret;
}
static void s_Ipc_YXCD_Reset(pYXCCD me) 
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	
	s_ipc_call_send( FUNC_YXCD_Reset, &data);
}

static enum_DLNA_RET s_Ipc_YXCD_BrowseMetedata(pYXCCD me, t_DLNA_OBJECTID objectId) 
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	Ipc_Data_SetString(&data,  objectId);
	
	s_ipc_call_send( FUNC_YXCD_BrowseMetedata, &data);

	int ret = Ipc_Data_GetInt(&data);

	return ret;
}
static enum_DLNA_RET s_Ipc_YXCD_BrowseChildren(pYXCCD me, t_DLNA_OBJECTID objectId, int startingIndex, int requestedCount) 	
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	Ipc_Data_SetString(&data,  objectId);
	Ipc_Data_SetInt(&data,  &startingIndex);
	Ipc_Data_SetInt(&data,  &requestedCount);
	
	s_ipc_call_send( FUNC_YXCD_BrowseChildren, &data);

	int ret = Ipc_Data_GetInt(&data);

	return ret;
}
static enum_DLNA_RET s_Ipc_YXCD_Search(pYXCCD me, t_DLNA_OBJECTID objectId, int startingIndex, int requestedCount) 
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	Ipc_Data_SetString(&data,  objectId);
	Ipc_Data_SetInt(&data,  &startingIndex);
	Ipc_Data_SetInt(&data,  &requestedCount);
	
	s_ipc_call_send( FUNC_YXCD_Search, &data);

	int ret = Ipc_Data_GetInt(&data);

	return ret;
}


static t_DLNA_JSON s_Ipc_YXCD_OnResultPartial( pYXCCD me, int format)
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	Ipc_Data_SetInt(&data,  &(format));
	
	s_ipc_call_send( FUNC_YXCD_OnResultPartial, &data);

	int len = me->bufLen;
	char *ret = Dlna_GetStringBuf(&len);
	strcpy(ret, Ipc_Data_GetFirstString(&data));
	
	return ret;
}


static t_DLNA_JSON s_Ipc_YXCD_SyncSearch(pYXCCD me, t_DLNA_OBJECTID containerID, int startingIndex, int requestedCount)
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	Ipc_Data_SetString(&data,  containerID);
	Ipc_Data_SetInt(&data,  &startingIndex);
	Ipc_Data_SetInt(&data,  &requestedCount);
	
	s_ipc_call_send( FUNC_YXCD_SyncSearch, &data);

	int len = me->bufLen;
	char *ret = Dlna_GetStringBuf(&len);
	strcpy(ret, Ipc_Data_GetFirstString(&data));
	
	return ret;
}

static t_DLNA_JSON s_Ipc_YXCD_SyncBrowseChildren(pYXCCD me, t_DLNA_OBJECTID containerID, int startingIndex, int requestedCount)
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	Ipc_Data_SetString(&data,  containerID);
	Ipc_Data_SetInt(&data,  &startingIndex);
	Ipc_Data_SetInt(&data,  &requestedCount);
	
	s_ipc_call_send( FUNC_YXCD_SyncBrowseChildren, &data);

	int len = me->bufLen;
	char *ret = Dlna_GetStringBuf(&len);
	strcpy(ret, Ipc_Data_GetFirstString(&data));
	
	return ret;
}

static t_DLNA_JSON s_Ipc_YXCD_SyncBrowseMetedata(pYXCCD me, t_DLNA_OBJECTID itemID)
{
	t_IPC_DATA data = {0};

	Ipc_Data_SetInt(&data,  &(me->pHandle));
	Ipc_Data_SetString(&data,  itemID);
	
	s_ipc_call_send( FUNC_YXCD_SyncBrowseMetedata, &data);

	int len = me->bufLen;
	char *ret = Dlna_GetStringBuf(&len);
	strcpy(ret, Ipc_Data_GetFirstString(&data));
	
	return ret;
}

static void s_Ipc_YXCD_Release(pYXCCD me)
{
	if(me)
	{
		t_IPC_DATA data = {0};
		
		Ipc_Data_SetInt(&data,	&(me->pHandle));
		
		s_ipc_call_send( FUNC_YXCD_Release, &data);
		
		free(me);
	}
}
int CD_Ipc_create(void)
{
	int size = sizeof(YXClassContentDirectory);
	YXClassContentDirectory *hnd= (YXClassContentDirectory*)malloc(size);

	if( hnd )
	{
		memset(hnd,0,size);
	
		hnd->pHandle				= YX_CD_Create(NULL, NULL, (int)hnd, NULL);
		
		hnd->SetDmsUdn				= s_Ipc_YXCD_SetDmsUdn;
		hnd->GetDmsUdn				= s_Ipc_YXCD_GetDmsUdn;
		hnd->GetSearchCapabilities	= s_Ipc_YXCD_GetSearchCapabilities;
		hnd->GetSortCapabilities	= s_Ipc_YXCD_GetSortCapabilities;
		
		hnd->SetFilter				= s_Ipc_YXCD_SetFilter;
		hnd->SetSearchCriteria 		= s_Ipc_YXCD_SetSearchCriteria;
		hnd->SetSortCriteria 		= s_Ipc_YXCD_SetSortCriteria;
		hnd->SetResultFormat		= s_Ipc_YXCD_SetResultFormat;
		
		hnd->BrowseMetedata 		= s_Ipc_YXCD_BrowseMetedata;
		hnd->BrowseChildren 		= s_Ipc_YXCD_BrowseChildren;
		hnd->Search 				= s_Ipc_YXCD_Search;
		hnd->OnResultPartial		= s_Ipc_YXCD_OnResultPartial;
		
		hnd->Reset 					= s_Ipc_YXCD_Reset;
		hnd->Release				= s_Ipc_YXCD_Release;

		hnd->SyncBrowseChildren		= s_Ipc_YXCD_SyncBrowseChildren;
		hnd->SyncBrowseMetedata		= s_Ipc_YXCD_SyncBrowseMetedata;
		hnd->SyncSearch 			= s_Ipc_YXCD_SyncSearch;
	}
	
	return  (int)hnd;	
}


