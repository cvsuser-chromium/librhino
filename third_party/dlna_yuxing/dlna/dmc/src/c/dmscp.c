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
*    filename:			ContentDirectory.js
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
#include "device.h"
#include "service.h"
#include "upnp.h"
#include "LinkedList.h"
#include "xml_util.h"
#include "json.h"


#include "dmscp.h"
#include "hitTime.h"
#include "coo.h"

static const char* const CRITERIA_BROWSE_METADATA = "BrowseMetadata";
static const char* const CRITERIA_BROWSE_CHILDREN = "BrowseDirectChildren";
static sem_t		s_dmscp_updated_lock;
static sem_t        s_dmscp_service_updated_lock;

static LinkedList	s_dmscp_updated_list;
static LinkedList	s_dmscp_service_updated_list;


//////////////////////////////////////////////////////////////////////////////////////////////////////////
static void s_Dmscp_Register_ServiceCalback(t_DMC_SERVICE_UPDATED_CALLBACK callback, int hnd)
{
	int *p = (int*)malloc( sizeof(int) *2 );
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,(int)callback, NULL);
	sem_wait(&(s_dmscp_service_updated_lock));

	p[0] = (int)callback;
	p[1] = hnd;
	ListAddTail( &s_dmscp_service_updated_list, p );

	sem_post(&(s_dmscp_service_updated_lock));
	HT_DBG_FUNC_END(hnd, NULL);
}

static void s_Dmscp_UnRegister_ServiceCalback(int hnd)
{
	int *p;
	ListNode *node;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,(int)hnd, NULL);
	sem_wait(&(s_dmscp_service_updated_lock));
	
	node = ListHead(&s_dmscp_service_updated_list);
	while( node )
	{
		p = (int *)(node->item);
		if( p[1] == hnd )
		{
			ListDelNode(&s_dmscp_service_updated_list, node, 0);
			free(p);
			break;
		}
		node = ListNext(&s_dmscp_service_updated_list, node);
	}
	
	sem_post(&(s_dmscp_service_updated_lock));
	HT_DBG_FUNC_END(hnd, NULL);
}

static void s_Dmscp_DmsUpdated_ServiceCallback(char *dmsUdn, char *name, char *value)
{
	ListNode * node;
	int *p;

	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, 0, dmsUdn);
	HT_DBG_FUNC(0, value);
	sem_wait(&(s_dmscp_service_updated_lock));
	
	node = ListHead(&(s_dmscp_service_updated_list));
	while(node)
	{
//		HT_DBG_FUNC(action, "node ok");
		p = (int*)(node->item);
		((t_DMC_SERVICE_UPDATED_CALLBACK)(p[0]))(dmsUdn, name, value, p[1]);
		
		node = ListNext(&(s_dmscp_service_updated_list), node);
	}

	sem_post(&(s_dmscp_service_updated_lock));
	
	HT_DBG_FUNC_END(0, name);
}

static void s_Dmscp_RegisterCalback(t_DMC_DMS_UPDATED_CALLBACK callback, int hnd)
{
	int *p = (int*)malloc( sizeof(int) *2 );
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,(int)callback, NULL);
	sem_wait(&(s_dmscp_updated_lock));

	p[0] = (int)callback;
	p[1] = hnd;
	ListAddTail( &s_dmscp_updated_list, p );

	sem_post(&(s_dmscp_updated_lock));
	HT_DBG_FUNC_END(hnd, NULL);
}

static void s_Dmscp_UnRegisterCalback(int hnd)
{
	int *p;
	ListNode *node;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,(int)hnd, NULL);
	sem_wait(&(s_dmscp_updated_lock));
	
	node = ListHead(&s_dmscp_updated_list);
	while( node )
	{
		p = (int *)(node->item);
		if( p[1] == hnd )
		{
			ListDelNode(&s_dmscp_updated_list, node, 0);
			free(p);
			break;
		}
		node = ListNext(&s_dmscp_updated_list, node);
	}
	
	sem_post(&(s_dmscp_updated_lock));
	HT_DBG_FUNC_END(hnd, NULL);
}

static void s_Dmscp_DmsUpdatedCallback(DeviceList_EventType type, Device *d, char *udn, const char* deviceName)
{
	ListNode * node;
	int *p;
	int action = (type == E_DEVICE_ADDED)? 1 : -1;

	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,type, udn);
	sem_wait(&(s_dmscp_updated_lock));
	
	node = ListHead(&(s_dmscp_updated_list));
	while(node)
	{
		HT_DBG_FUNC(action, "node ok");
		p = (int*)(node->item);
		((t_DMC_DMS_UPDATED_CALLBACK)(p[0]))(udn, d, (char*)deviceName, action, p[1]);
		
		node = ListNext(&(s_dmscp_updated_list), node);
	}

	sem_post(&(s_dmscp_updated_lock));
	
	HT_DBG_FUNC_END(action, deviceName);
}

static void s_Dmscp_Find_Int(struct json_object *json, char *key, int *p)
{
	const char *str;
	json_object *obj = json_object_object_get(json, key);
	if(obj && (str = json_object_get_string(obj)))
		*p = atoi(str);
}
static void s_Dmscp_Find_Bool(struct json_object *json, char *key, int *p)
{
	const char *str;
	json_object *obj = json_object_object_get(json, key);
	if(obj && (str = json_object_get_string(obj)))
	{
		if(!strcasecmp(str, "true"))
			*p = 1;
	}
}

int g_dmscp_show_inner_dms = 0;
int g_dmscp_dms_total_limit = 32;
int YX_Dmscp_Init(char *setting)
{
	struct json_object *json;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW,0, setting);

	sem_init(&(s_dmscp_updated_lock),0,1);
	ListInit(&(s_dmscp_updated_list), 0, free);
	
	sem_init(&(s_dmscp_service_updated_lock),0,1);
	ListInit(&(s_dmscp_service_updated_list), 0, free);
	
	DeviceList_init(s_Dmscp_DmsUpdatedCallback, s_Dmscp_DmsUpdated_ServiceCallback);

	if(setting &&(json = dlna_json_tokener_parse(setting)))
	{
		s_Dmscp_Find_Bool(json, "show_inner_dms",	&g_dmscp_show_inner_dms);
		s_Dmscp_Find_Int( json, "dms_total_limit",	&g_dmscp_dms_total_limit);
		json_object_put(json);
	}
	
	HT_DBG_FUNC_END(0, NULL);
	return 0;
}

int YX_Dmscp_Destroy(void)
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW,0, NULL);

	ListDestroy(&(s_dmscp_updated_list), 1);//??????????????????
	sem_destroy(&(s_dmscp_updated_lock) );
	
	ListDestroy(&(s_dmscp_service_updated_list), 1);//??????????????????
	sem_destroy(&(s_dmscp_service_updated_lock) );
	
	HT_DBG_FUNC_END(0, NULL);
	return 0;
}

int YX_Dmscp_Start(void)
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW,0, NULL);

	DeviceList_register_ctrlpt();
	
	HT_DBG_FUNC_END(0, NULL);
	return 0;
}

void YX_Dmscp_Stop(void)
{
	DeviceList_unregister_ctrlpt();
}

void YX_Dmscp_SearchDms(int remove_all)
{
	DeviceList_RefreshAll (remove_all);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////
static int s_DmsList_GetDmsNumber(pClassDmsList me)
{
	int ret = DeviceList_GetDeviceCount();
	return ret;
}

static int s_DmsList_GetDmsDescriptionXml (pClassDmsList me, char *DmsUdn, char *buf, int bufLen)
{
	Device* d;
	int ret = -2;
	
	DeviceList_Lock();
	if( (d = GetDeviceByUdn(DmsUdn)) != NULL )
	{
		if( strlen(d->descDocText) < bufLen )
		{
			strcpy(buf, d->descDocText);
			ret = 0;
		}
		else
		{
			strncpy(buf, d->descDocText, bufLen-1);
			ret = -1;
		}
	}
	DeviceList_Unlock();
	
	return ret;	
}

static void s_DmsList_SearchDms(pClassDmsList me, int remove_all)
{
	DeviceList_RefreshAll (remove_all);
}

static void s_DmsList_OnDmsUpdated(char *dmsUdn, Device*d, char *alias, int action, int cookie)
{
	pClassDmsList me = (pClassDmsList)cookie;
	if( me->OnDmsUpdated && me->parent )
		me->OnDmsUpdated(dmsUdn, d, alias, action, me->parent);
}

static void s_DmsList_OnServiceUpdated(char *dmsUdn, char *name, char *value, int cookie)
{
    pClassDmsList me = (pClassDmsList)cookie;
    if( me->OnServiceUpdated && me->parent )
        me->OnServiceUpdated(dmsUdn, name, value, me->parent);
}

static void s_DmsList_Release(pClassDmsList me)
{
	if( me)
	{
		s_Dmscp_UnRegisterCalback((int)me);
		s_Dmscp_UnRegister_ServiceCalback((int)me);
		free(me);
	}
}

int YX_DmsList_Create( t_DMC_DMS_UPDATED_CALLBACK OnDmsUpdated, int cookie, t_DMC_SERVICE_UPDATED_CALLBACK OnServiceUpdated)
{
	ClassDmsList *hnd=NULL;
	int size = sizeof(ClassDmsList);
	hnd = (ClassDmsList*)malloc(size);
	
	if( hnd )
	{
		memset(hnd,0,size);
		hnd->GetDmsNumber			= s_DmsList_GetDmsNumber;
		hnd->GetDmsDescriptionXml 	= s_DmsList_GetDmsDescriptionXml;
		hnd->SearchDms				= s_DmsList_SearchDms;
		hnd->Release				= s_DmsList_Release;

		hnd->OnDmsUpdated			= OnDmsUpdated;
		hnd->parent					= cookie;
		//调用在s_Dmscp_DmsUpdatedCallback和s_Dmscp_DmsUpdated_ServiceCallback
		if( OnDmsUpdated && cookie)
			s_Dmscp_RegisterCalback(s_DmsList_OnDmsUpdated, (int)hnd);
			
		hnd->OnServiceUpdated		= OnServiceUpdated;
		if( OnDmsUpdated && cookie)
			s_Dmscp_Register_ServiceCalback(s_DmsList_OnServiceUpdated, (int)hnd);
	}

	return (int)hnd;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SEM_WAIT()	sem_wait(&(me->lock))
#define SEM_POST()	sem_post(&(me->lock))

static int s_CD_SetObjectID(pClassContentDirectory me, t_DLNA_OBJECTID id) 
{
	if( me->objectid )
		free(me->objectid);
	me->objectid = Dlna_strdup(id);
	return 0;
}

static int s_CD_SetFilter(pClassContentDirectory me, char *filter) 
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW,(int)me, me->filter);
	if( me->filter )
		free(me->filter);
	me->filter = Dlna_strdup(filter);
	HT_DBG_FUNC_END(0, me->filter);
	return 0;
}
static int s_CD_SetSearchCriteria(pClassContentDirectory me, char *searchCriteria) 
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW,(int)me, searchCriteria);
	if( me->searchCriteria )
		free(me->searchCriteria);
	me->searchCriteria = Dlna_strdup(searchCriteria);
	HT_DBG_FUNC_END(0, me->searchCriteria);
	return 0;
}
static int s_CD_SetSortCriteria(pClassContentDirectory me, int sort,int order,char *sort_Criteria) 
{
	char *sortCriteria = NULL;
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW,(int)me, NULL);
	if (!sort_Criteria)
	{
		switch(sort){
		case 0:
			if(order)
			{
				sortCriteria = "-dc:title";
			}
			else
			{
				sortCriteria = "+dc:title";
			}
			break;
		case 1:	
			if(order)
			{
				sortCriteria = "-res@size";
			}
			else
			{
				sortCriteria = "+res@size";
			}
			//sortCriteria = "";
			break;
		case 2:
			/*if(order)
			{
				sortCriteria = "-dc:title";
			}
			else
			{
				sortCriteria = "+dc:title";
			}
			sortCriteria = "+dc:title";*/
			break;
		case 3:
			if(order)
			{
				sortCriteria = "-dc:date";
			}
			else
			{
				sortCriteria = "+dc:date";
			}
			//sortCriteria = "+dc:date";
			break;
		default:
			sortCriteria = "+dc:title";
			break;
		}
		
		HT_DBG_PRINTF(HT_MOD_DMC, HT_BIT_KEY, "sortCriteria = %s\n", sortCriteria);
	}
	else
	{
		sortCriteria = sort_Criteria;
	}
	if( me->sortCriteria )
		free(me->sortCriteria);
	me->sortCriteria = Dlna_strdup(sortCriteria);
	HT_DBG_FUNC_END(0, me->sortCriteria);
	return 0;
}
//static char *s_huawei_udn = NULL;
int s_CD_SetDmsUdn(pClassContentDirectory me, char *dmsUdn) 
{
	Device* d;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW,(int)me, dmsUdn);
	SEM_WAIT();
#if 0
	if( s_huawei_udn )
		free(s_huawei_udn);
	s_huawei_udn = Dlna_strdup(dmsUdn);
#endif
	strcpy(me->udn, dmsUdn);
	me->dmsIp[0] = 0;
	
	DeviceList_Lock();
	if( (d = GetDeviceByUdn(me->udn)) != NULL )
		sscanf(d->baseURL, "http://%31[^:]", me->dmsIp);
	DeviceList_Unlock();
	
	SEM_POST();
	HT_DBG_FUNC_END(0, me->dmsIp);
	return 0;
}
static char* s_CD_GetDmsUdn(pClassContentDirectory me) 
{
	return me->udn;	
}

static int s_CD_GetSearchCapabilities(pClassContentDirectory me, char *buf, int bufLen) 
{
	int rc = 0;
	return rc;
}
static int s_CD_GetSortCapabilities(pClassContentDirectory me, char *buf, int bufLen) 	
{
	int rc = 0;
	return rc;
}


static void s_CD_Reset(pClassContentDirectory me) 
{
	SEM_WAIT();
	me->udn[0] = 0;
	me->dmsIp[0] = 0;
	SEM_POST();
}
	
static void s_CD_fresh(pClassContentDirectory hnd)
{
	hnd->nb_matched = 0;
	hnd->nb_returned = 0;
	hnd->index = 0;
}
static int s_Browse(const Service* serv, Upnp_FunPtr callback, pClassContentDirectory hnd,
	t_DLNA_OBJECTID objectId, const char *browseFlag, char *filter, int startingIndex, int requestedCount, char *sortCriteria)
{
	char index[128], count[128];
	sprintf(index, "%d", startingIndex);
	sprintf(count, "%d", requestedCount);
	
	if( filter == NULL )		filter = "*";
	if( sortCriteria == NULL )	sortCriteria = "";
	s_CD_fresh(hnd);
	
	int rc = Service_SendActionAsyncVaEx(serv, callback, (void*)hnd, "Browse",
									 "ObjectID",			objectId,
									 "BrowseFlag",			browseFlag,
									 "Filter", 	      		filter,
									 "StartingIndex",     	index,
									 "RequestedCount",    	count,
									 "SortCriteria",      	sortCriteria,
									 NULL, 		      		NULL);
	return rc;
}
static int s_Search(const Service* serv, Upnp_FunPtr callback, pClassContentDirectory hnd,
	t_DLNA_OBJECTID containerID, char *searchCriteria, char *filter, int startingIndex, int requestedCount, char *sortCriteria)
{
	char index[128], count[128];
	sprintf(index, "%d", startingIndex);
	sprintf(count, "%d", requestedCount);
	
	if( searchCriteria == NULL )searchCriteria = "*";
	if( filter == NULL )		filter = "*";
	if( sortCriteria == NULL )	sortCriteria = "";
	s_CD_fresh(hnd);
	
	int rc = Service_SendActionAsyncVaEx(serv, callback, (void*)hnd, "Search", 
									 "ContainerID",			containerID,
									 "SearchCriteria",		searchCriteria,
									 "Filter", 	      		filter,
									 "StartingIndex",		index,
								     "RequestedCount",		count,
									 "SortCriteria",      	sortCriteria,
									 NULL, 		      		NULL);
	
	return rc;
}

static int s_ParseBrowseOrSearchResult (Upnp_EventType EventType, void *Event, void *cookie)
{
	int kill = 0;
	pClassContentDirectory me = (pClassContentDirectory)cookie;
	struct Upnp_Action_Complete *pEvt = (struct Upnp_Action_Complete *)Event;
	IXML_Document *doc;

	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,EventType, "EventType = ");

	SEM_WAIT();
	
	me->sdk_err  = 0;
	me->http_err = 0;
	me->upnp_err = 0;
	
	if(me->ActionResult)
		ixmlDocument_free(me->ActionResult);
	me->ActionResult = NULL;
	if(me->subdoc)
		ixmlDocument_free(me->subdoc);
	me->subdoc = NULL;
	me->nb_matched = 0;
	me->nb_returned= 0;
	me->updateID = 0;

	if( Event==NULL || cookie==NULL )
		goto cleanup;

	HT_DBG_FUNC(pEvt->ErrCode, "ErrCode = ");
	me->sdk_err = pEvt->ErrCode;

	me->ActionResult = pEvt->ActionResult;
	pEvt->ActionResult = NULL;
	doc = me->ActionResult;
	if(!doc)
		goto cleanup;
	
	const char* s = XMLUtil_FindFirstElementValue(XML_D2N (doc), "UpdateID", true, false);
	STRING_TO_INT (s, me->updateID, 0);
	HT_DBG_FUNC(me->updateID, "updateID = ");
	s = XMLUtil_FindFirstElementValue(XML_D2N (doc), "TotalMatches", true, false);
	STRING_TO_INT (s, me->nb_matched, 0);
	HT_DBG_FUNC(me->nb_matched, "TotalMatches = ");
	s = XMLUtil_FindFirstElementValue(XML_D2N (doc), "NumberReturned", true, false);
	STRING_TO_INT (s, me->nb_returned, 0);
	HT_DBG_FUNC(me->nb_returned, "NumberReturned = ");
	if(s)
	{
		const char* const resstr = XMLUtil_FindFirstElementValue(XML_D2N (doc), "Result", true, false);
		if(me->nb_returned == 0)
		{
			HT_DBG_FUNC(0, resstr);
		}
		if (resstr) 
		{
			me->subdoc = ixmlParseBuffer (discard_const_p (char, resstr));
			HT_DBG_FUNC((int)(me->subdoc), "me->subdoc = ");		
		}
		goto cleanup;
	}
	
	s = XMLUtil_FindFirstElementValue(XML_D2N (doc), "faultstring", true, false);
	if(s && !strcmp(s, "UPnPError"))
	{
		s = XMLUtil_FindFirstElementValue(XML_D2N (doc), "errorDescription", true, false);
		HT_DBG_FUNC(0, s);
		s = XMLUtil_FindFirstElementValue(XML_D2N (doc), "errorCode", true, false);
		if(s)
			me->upnp_err = atoi(s);
	}
	

cleanup:

	kill = me->killMeInCallback;
	
	if( me->syncBrowsing == 1 )
	{
		me->syncBrowsing = 0;
		sem_post( &(me->semResultOK) );
	}
	else
	{
		if( me->OnBrowseResult && me->parent ) 
			me->OnBrowseResult(me->nb_returned, me->parent);
	}

	me->busy = 0;
	SEM_POST();

	if( kill )
		me->Release(me);
	
	HT_DBG_FUNC_END(kill, NULL);
	return kill;
}


static int s_CD_BrowseMetadataSink (Upnp_EventType EventType, void *Event, void *cookie)
{
	return s_ParseBrowseOrSearchResult(EventType, Event, cookie);
}	
static enum_DLNA_RET s_CD_BrowseMetedata(pClassContentDirectory me, t_DLNA_OBJECTID objectId)
{
	enum_DLNA_RET	rc=enum_DLNA_NO_THIS_DMS;
	Device* d;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,me->busy, me->udn);
	HT_DBG_FUNC(0, objectId);
	if(me->busy)
		return enum_DLNA_DMS_BUSY;
	me->busy=1;
	
	DeviceList_Lock();
	if( (d = GetDeviceByUdn(me->udn)) != NULL )
	{
		Service* const serv = Device_GetServiceFrom(d, CONTENT_DIR_SERVICE_TYPE, FROM_SERVICE_TYPE, false);
		rc = s_Browse(serv, s_CD_BrowseMetadataSink, me, objectId, CRITERIA_BROWSE_METADATA, me->filter, 0, 0, me->sortCriteria);
	}
	DeviceList_Unlock();
	
	if( rc )
		me->busy = 0;
	
	HT_DBG_FUNC_END(rc, NULL);
	return rc;
}


static int s_CD_BrowseChildrenSink (Upnp_EventType EventType, void *Event, void *cookie)
{
	return s_ParseBrowseOrSearchResult(EventType, Event, cookie);
}	
static enum_DLNA_RET s_CD_BrowseChildren(pClassContentDirectory me, t_DLNA_OBJECTID objectId, int startingIndex, int requestedCount)
{
	enum_DLNA_RET	rc=enum_DLNA_NO_THIS_DMS;
	Device* d;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,me->busy, me->udn);
	HT_DBG_FUNC(startingIndex, objectId);
	
	if(me->busy)
		return enum_DLNA_DMS_BUSY;
	me->busy=1;
	
	DeviceList_Lock();
	if( (d = GetDeviceByUdn(me->udn)) != NULL )
	{
		Service* const serv = Device_GetServiceFrom(d, CONTENT_DIR_SERVICE_TYPE, FROM_SERVICE_TYPE, false);
		rc = s_Browse(serv, s_CD_BrowseChildrenSink, me, objectId, CRITERIA_BROWSE_CHILDREN, me->filter, startingIndex, requestedCount, me->sortCriteria);
	}
	DeviceList_Unlock();
	
	if( rc )
		me->busy = 0;
	
	HT_DBG_FUNC_END(rc, NULL);
	return rc;
}

static int s_CD_SearchSink (Upnp_EventType EventType, void *Event, void *cookie)
{
	return s_ParseBrowseOrSearchResult(EventType, Event, cookie);
}	
static enum_DLNA_RET s_CD_Search(pClassContentDirectory me, t_DLNA_OBJECTID containerID, int startingIndex, int requestedCount)
{
	enum_DLNA_RET	rc=enum_DLNA_NO_THIS_DMS;
	Device* d;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,me->busy, me->udn);
	HT_DBG_FUNC(startingIndex, containerID);
	
	if(me->busy)
		return enum_DLNA_DMS_BUSY;
	me->busy=1;
	
	DeviceList_Lock();
	if( (d = GetDeviceByUdn(me->udn)) != NULL )
	{
		Service* const serv = Device_GetServiceFrom(d, CONTENT_DIR_SERVICE_TYPE, FROM_SERVICE_TYPE, false);
		rc = s_Search(serv, s_CD_SearchSink, me, containerID, me->searchCriteria, me->filter, startingIndex, requestedCount, me->sortCriteria);
	}
	DeviceList_Unlock();
	
	if( rc )
		me->busy = 0;
	
	HT_DBG_FUNC_END(rc, NULL);
	return rc;
}

static enum_DLNA_RET s_CD_SyncSearch(pClassContentDirectory me, t_DLNA_OBJECTID containerID, int startingIndex, int requestedCount)
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,me->busy, containerID);
	if(me->busy)
		return enum_DLNA_DMS_BUSY;	

	if( atoi(containerID) == -1 )
		containerID = "0";
	
	me->syncBrowsing = 1;
	int rc = me->Search(me, containerID, startingIndex, requestedCount);
	if( rc != 0 )
	{
		me->syncBrowsing = 0;
		return enum_DLNA_PARA_ERROR;	
	}

	sem_wait( &(me->semResultOK) );//等待信号量释放。释放在得到结果之后,zai s_ParseBrowseOrSearchResult
	me->busy = 0;
	HT_DBG_FUNC_END(0,0);
	return enum_DLNA_ACTION_OK;
}

static enum_DLNA_RET s_CD_SyncBrowseChildren(pClassContentDirectory me, t_DLNA_OBJECTID containerID, int startingIndex, int requestedCount)
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,me->busy, containerID);
	if(me->busy)
		return enum_DLNA_DMS_BUSY;	

	if( atoi(containerID) == -1 )
		containerID = "0";
	
	me->syncBrowsing = 1;
	int rc = me->BrowseChildren(me, containerID, startingIndex, requestedCount);
	if( rc != 0 )
	{
		me->syncBrowsing = 0;
		return enum_DLNA_PARA_ERROR;	
	}

	sem_wait( &(me->semResultOK) );
	me->busy = 0;
	HT_DBG_FUNC_END(0,0);
	return enum_DLNA_ACTION_OK;
}

static enum_DLNA_RET s_CD_SyncBrowseMetedata(pClassContentDirectory me, t_DLNA_OBJECTID itemID)
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,me->busy, itemID);
	if(me->busy)
		return enum_DLNA_DMS_BUSY;	

	if( atoi(itemID) == -1 )
		itemID = "0";
	
	me->syncBrowsing = 1;
	int rc = me->BrowseMetedata(me, itemID);
	if( rc != 0 )
	{
		me->syncBrowsing = 0;
		return enum_DLNA_PARA_ERROR;	
	}

	sem_wait( &(me->semResultOK) );
	me->busy = 0;
	HT_DBG_FUNC_END(0,0);
	return enum_DLNA_ACTION_OK;
}

static void s_CD_OnDmsUpdated(char *dmsUdn, Device*d, char *alias, int action, int cookie)
{
	pClassContentDirectory me = (pClassContentDirectory)cookie;
	if( action != 1 && (strcmp(dmsUdn, me->udn)==0) )
		if( me->OnDmsUpdated && me->parent )
			me->OnDmsUpdated(dmsUdn, d, alias, action, me->parent);
}
static void s_CD_OnServiceUpdated(char *dmsUdn, char *name, char *value, int cookie)
{
	pClassContentDirectory me = (pClassContentDirectory)cookie;
	if( (strcmp(dmsUdn, me->udn)==0) )
		if( me->OnServiceUpdated && me->parent )
			me->OnServiceUpdated(dmsUdn, name, value, me->parent);
}



static void s_CD_Release(pClassContentDirectory me)
{
	int killMeNow = 1;

	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, (int)me, 0);
	
	if( me == NULL )
		return;
	
	SEM_WAIT();
	if(me->busy)
	{
		me->killMeInCallback = 1;
		killMeNow = 0;
	}
	SEM_POST();
	
	HT_DBG_FUNC(killMeNow*100, 0);
	
	if( killMeNow )
	{
		s_Dmscp_UnRegisterCalback((int)me);
		s_Dmscp_UnRegister_ServiceCalback((int)me);
		sem_destroy(&(me->lock));
		sem_destroy(&(me->semResultOK));
		
		if( me->filter )			free(me->filter);
		if( me->searchCriteria )	free(me->searchCriteria);
		if( me->sortCriteria )		free(me->sortCriteria);
		if( me->ActionXml ) 		free(me->ActionXml);
		
		if(me->subdoc)
			ixmlDocument_free(me->subdoc);
		if(me->ActionResult)
			ixmlDocument_free(me->ActionResult);
		
        if( me->objectid )          free(me->objectid);
		free(me);
	}
	HT_DBG_FUNC_END(killMeNow, 0);
}


//浏览创建
int YX_CD_Create(t_DMC_DMS_UPDATED_CALLBACK OnDmsUpdated, t_DMC_BROWSE_CALLBACK OnBrowseResult, int cookie, t_DMC_SERVICE_UPDATED_CALLBACK OnServiceUpdated)
{
	ClassContentDirectory *hnd=NULL;
	int size = sizeof(ClassContentDirectory);
	hnd = (ClassContentDirectory*)malloc(size);
	
	if( hnd )
	{
		memset(hnd,0,size);
		hnd->SetDmsUdn				= s_CD_SetDmsUdn;
		hnd->GetDmsUdn				= s_CD_GetDmsUdn;
		hnd->GetSearchCapabilities	= s_CD_GetSearchCapabilities;
		hnd->GetSortCapabilities	= s_CD_GetSortCapabilities;
		
		hnd->SetFilter				= s_CD_SetFilter;
		hnd->SetSearchCriteria 		= s_CD_SetSearchCriteria;
		hnd->SetSortCriteria 		= s_CD_SetSortCriteria;
		
		hnd->BrowseMetedata 		= s_CD_BrowseMetedata;
		hnd->BrowseChildren 		= s_CD_BrowseChildren;
		hnd->Search 				= s_CD_Search;
		
		hnd->Reset 					= s_CD_Reset;
		hnd->Release				= s_CD_Release;
		
		hnd->SyncBrowseChildren		= s_CD_SyncBrowseChildren;
		hnd->SyncBrowseMetedata		= s_CD_SyncBrowseMetedata;
		hnd->SyncSearch 			= s_CD_SyncSearch;
		
		hnd->OnDmsUpdated			= OnDmsUpdated;
		hnd->OnBrowseResult			= OnBrowseResult;
		hnd->parent					= cookie;
		hnd->SetObjectID			= s_CD_SetObjectID;
		
		sem_init(&(hnd->semResultOK),0,0);
		sem_init(&(hnd->lock),0,1);
		
		if( OnDmsUpdated && cookie)
			s_Dmscp_RegisterCalback(s_CD_OnDmsUpdated, (int)hnd);
			
		hnd->OnServiceUpdated		= OnServiceUpdated;
		if( OnServiceUpdated && cookie)
			s_Dmscp_Register_ServiceCalback(s_CD_OnServiceUpdated, (int)hnd);

	}
	
	return  (int)hnd;	
}



/*-------------------------------------------------------------------------------------------*/
/* non-object functions */
/*-------------------------------------------------------------------------------------------*/
static int s_CD_Atoi(char *src, char *flag, int *val)
{
	if(!src || !flag ||!val)
		return -2;

	char *p = strstr(src, flag);
	if( p && (sscanf(p+strlen(flag), "%*[^0-9]%d", val) == 1) )
		return 0;

	return -1;
}


static void s_CD_ParseMime(char *mimetype, t_MEDIA_INFO* minfo )
{
	if( !(mimetype && minfo) )
		return;
	
	memset(minfo, 0, sizeof(t_MEDIA_INFO));

	/* main type */
	if( strncmp(mimetype,"video", 5) == 0 )
		minfo->majorType = 1;
	else if( strncmp(mimetype,"audio", 5) == 0 )
		minfo->majorType = 2;
	else if( strncmp(mimetype,"image", 5) == 0 )
		minfo->majorType = 3;
	else
		minfo->majorType = 0;
	
	/* LPCM specially */
	const char *str = "audio/L16";/* http-get:*:audio/L16;rate=44100;channels=1: */
	if( strncmp(mimetype, str, strlen(str)) == 0 )
	{
		int x;
		
		SET_WAVE_FORMAT(minfo->otherType.audioType);
		SET_WAVE_FORMAT(minfo->audioInfo.wFormatTag);
	
		minfo->audioInfo.nBlockAlign	= 2;
		minfo->audioInfo.wBitsPerSample = 16;
		if( s_CD_Atoi(mimetype, "rate", &x) == 0 )
			minfo->audioInfo.nSamplesPerSec = x;
		if( s_CD_Atoi(mimetype, "channels", &x) == 0 )
			minfo->audioInfo.nChannels = x;
	}
}

/* 附加信息之DLNA信息 */
/* DLNA信息4th_field = [pn-param];[op-param];[psparam];[ *(other-param)] */
/* (http-get:*:video/mpeg:) DLNA.ORG_PN=MPEG_PS_NTSC;DLNA.ORG_OP=00;DLNA.ORG_PS=-32,-16,-8,-2,-1,2,8,16,32;DLNA.ORG_CI=1;MEDIABOLIC.COM_videotype=MPEG2PS */
//http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_FULL;DLNA.ORG_OP=01;INTEL.COM_videotype=wmvVideoType

/* http-get:*:    application/x-dtcp1;CONTENTFORMAT=video/MP2T:           * */
/* http-get:*:application/x-dtcp1;CONTENTFORMAT=video/mpeg:*      video/MP2T */
/* http://192.168.2.1:80/web/2.mpg?CONTENTPROTECTIONTYPE=DTCP1&DTCP1HOST=192.168.2.1&DTCP1PORT=8000 */

void s_CD_CreateMediaInfo(char *mimetype, IXML_Element*res, char *dlna, t_MEDIA_INFO *minfo)
{
	const char *str = NULL;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MYRIAD,0,mimetype);
	if(!mimetype || !minfo)
		return;
	memset(minfo, 0, sizeof(t_MEDIA_INFO));

	s_CD_ParseMime(mimetype, minfo);

	/* attributes */
	str = ixmlElement_getAttribute (res, "size");
	if(str)
		minfo->fileSize = atoll(str);
	
	str = ixmlElement_getAttribute (res, "resolution");
	if(str)
		sscanf ( str, "%dx%d", &(minfo->resWidth), &(minfo->resHeight) );
	
	str = ixmlElement_getAttribute (res, "colorDepth");
	if(str)
		minfo->colorDepth = atoi(str);
	
	str = ixmlElement_getAttribute (res, "bitrate");
	if(str)
		minfo->bitrate = atoi(str);

	/*
	H+:MM:SS[.F+]
	or
	H+:MM:SS[.F0/F1]
	where:
	H+: one or more digits to indicate elapsed hours,
	MM: exactly 2 digits to indicate minutes (00 to 59),
	SS: exactly 2 digits to indicate seconds (00 to 59),
	F+: any number of digits (including no digits) to indicate fractions of seconds,
	F0/F1: a fraction, with F0 and F1 at least one digit long, and F0 < F1.
	The string MAY be preceded by a “+” or “-” sign, and the decimal point itself MUST be omitted if there
	are no fractional second digits.
	*/
	str = ixmlElement_getAttribute (res, "duration");
	if(str)
	{
	//	int hh = 0;
		unsigned int hh=0, mm = 0, ss = 0;
		if (sscanf (str, "%u:%u:%u", &hh, &mm, &ss) == 3 )
		{
			minfo->xduration = (ss + 60*(mm + 60*hh)) * 1000;
		}
	}

	/* PVR sharing */
	str = ixmlElement_getAttribute (res, "contentSource");
	if(str)
	{
		minfo->pvr_flag = 1;
		minfo->pvr_contentSource = atoi(str);
		str = ixmlElement_getAttribute (res, "localEncrypted");
		if(str)
		{
			minfo->pvr_encrypted = atoi(str);
			if( minfo->pvr_encrypted )
			{
				str = ixmlElement_getAttribute (res, "localEncryptionKey");
				strncpy(minfo->pvr_keys, str, sizeof(minfo->pvr_keys)-1);
			}
		}
	}

	/* DLNA  specially */
	if( dlna )
	{
		if( strstr(dlna, "DLNA.ORG_CI=1") )
			minfo->dlnaCI = 1;
		if( strstr(dlna, "DLNA.ORG_PS=") )
			minfo->dlnaPS = 1;
	}	
	HT_DBG_FUNC_END(minfo->pvr_encrypted, minfo->pvr_keys);
}

static int s_YX_CD_IpMatched (pClassContentDirectory me, const char *uri)
{
	char ip[32]="";
	bool rc = false;
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MYRIAD,0,uri);
	if( me->dmsIp[0] && (sscanf(uri, "http://%31[^:]", ip ) == 1) && (strcmp(me->dmsIp,ip)==0) )
		rc = true;
	HT_DBG_FUNC_END(rc, me->dmsIp);
	return rc;
}
int YX_CD_SplitInfo(const char *protocol, char **transport, char **mimetype, char **dlna)
{
	char *p1, *p2, *p3;

    if (!protocol || !transport || !mimetype || !dlna)
        return false;

    p2=p3=0;
    p1 = strchr(protocol, ':');
	if( p1 )
	{
        *p1 = 0;
        p1++;
	}
    
	if( p1 && (p2= strchr(p1, ':')) )	
	{
        *p2 = 0;
        p2++;
	}
    
	if( p2 && (p3= strchr(p2, ':')) )	
	{
        *p3 = 0;
        p3++;
	}
	
	if( p1 && p2 && p3 )
	{
		*transport = (char*)protocol;
		*mimetype = p2;
		*dlna = p3;
		return true;
	}
    
	return false;
}
void YX_CD_ParseProtocolInfo(char *protocolInfo, t_MEDIA_INFO* minfo )
{
	char *transport, *mime, *dlna;
	if ( !protocolInfo || !minfo )
		return;
	
	if (YX_CD_SplitInfo(protocolInfo, &transport, &mime, &dlna))
    	s_CD_ParseMime(mime, minfo);
}

int s_YX_CD_CheckInfo(const char *protocol, char **dup_protocol, char **transport, char **mimetype, char **dlna)
{
    int ret = false;
    
    if (!protocol || !dup_protocol || !transport || !mimetype || !dlna)
        return ret;

    *dup_protocol = Dlna_strdup(protocol);
    if(*dup_protocol)
    {
        ret = YX_CD_SplitInfo(*dup_protocol, transport, mimetype, dlna);
        if(ret==false)
            free(*dup_protocol);
    }

    return ret;
}


#if 1
int YX_CD_GetPreferred (pClassContentDirectory me, IXML_NodeList *reslist, int len, char **protocolInfo, char **url, t_MEDIA_INFO *mInfo)
{
	char *dup_protocol, *transport, *mimetype, *dlna; 
	int i;
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MYRIAD,len,0);
	for (i = 0; i < len; i++)
	{
		IXML_Node *res = ixmlNodeList_item (reslist, i);
		IXML_Element*element = (IXML_Element*)res;

		const char* protocol = ixmlElement_getAttribute (element, "protocolInfo");
		const char* uri = XMLUtil_GetElementValue (element);
	//	if (uri == NULL || protocol == NULL ||sscanf (protocol, "http-get:*:%63[^:]", mimetype) != 1) 
	
		HT_DBG_FUNC(i, protocol);
		if (uri == NULL || protocol == NULL || !s_YX_CD_CheckInfo(protocol, &dup_protocol, &transport, &mimetype, &dlna)) 
			continue;
		
		if( strstr(transport, "http-get") )
		{
			if( s_YX_CD_IpMatched(me,uri ))
			{	
				t_MEDIA_INFO info;
				s_CD_CreateMediaInfo(mimetype, element, dlna, &info);
				
				if( info.dlnaCI == 0 )	//raw media file
				{
					if(protocolInfo)
					{
						*protocolInfo = Dlna_strdup(protocol);
					}
					if(url)
					{
						*url = Dlna_strdup(uri);
					}
					if(mInfo)
						memcpy(mInfo, &info, sizeof(t_MEDIA_INFO));
					HT_DBG_FUNC_END(i, 0);
                    free(dup_protocol);
					return i;
				}
			}
		}
		else
		{
		}
        free(dup_protocol);
	}
	
	return -1;
}
#endif
char *YX_CD_GetThumbnailForVideo (pClassContentDirectory me, IXML_NodeList *reslist, int len)
{
	char *dup_protocol, *transport, *mimetype, *dlna; 
	int i;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, len, 0);
	
	for (i = 0; i < len; i++)
	{
		IXML_Node *res = ixmlNodeList_item (reslist, i);
		IXML_Element*element = (IXML_Element*)res;

		const char* protocol = ixmlElement_getAttribute (element, "protocolInfo");
		const char* uri = XMLUtil_GetElementValue (element);
	//	if (uri == NULL || protocol == NULL ||sscanf (protocol, "http-get:*:%63[^:]", mimetype) != 1) 
	
		if (uri == NULL || protocol == NULL || !s_YX_CD_CheckInfo(protocol, &dup_protocol, &transport, &mimetype, &dlna)) 
			continue;
		if( strstr(transport, "http-get") )
		{
			if( s_YX_CD_IpMatched(me,uri ))
			{	
				t_MEDIA_INFO info;
				s_CD_CreateMediaInfo(mimetype, element, dlna, &info);

				//if( strcmp(info.majorType, "image") ==0 )	
				if( info.majorType == 3 )
				{
					HT_DBG_FUNC(0, uri);
                    free(dup_protocol);
					return Dlna_strdup(uri);
				}
			}
		}
        free(dup_protocol);
	}
	
	return NULL;
}

#define THUMBNAIL_MAX_PX		100000
char *YX_CD_GetThumbnailForImage (pClassContentDirectory me, IXML_NodeList *reslist, int len, int max_w, int max_h)
{
	char *dup_protocol, *transport, *mimetype, *dlna; 
	int i, index=-1;

	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, len, 0);
	
	for (i = 0; i < len; i++)
	{
		IXML_Node *res = ixmlNodeList_item (reslist, i);
		IXML_Element*element = (IXML_Element*)res;

		const char* protocol = ixmlElement_getAttribute (element, "protocolInfo");
		const char* uri = XMLUtil_GetElementValue (element);
	//	if (uri == NULL || protocol == NULL ||sscanf (protocol, "http-get:*:%63[^:]", mimetype) != 1) 
	
		if (uri == NULL || protocol == NULL || !s_YX_CD_CheckInfo(protocol, &dup_protocol, &transport, &mimetype, &dlna)) 
			continue;
		if( strstr(transport, "http-get") )
		{
			if( s_YX_CD_IpMatched(me,uri ))
			{	
				t_MEDIA_INFO info;
				s_CD_CreateMediaInfo(mimetype, element, dlna, &info);
				int w = info.resWidth;
				int h = info.resHeight;
				if( w>0 && w<max_w && h>0 && h<max_h )
				{
					max_w = w;
					max_h = h;
					index = i;
				}
			}
		}
        free(dup_protocol);
	}

	if( index != -1 )
	{
		HT_DBG_FUNC(index, NULL);
		return Dlna_strdup( XMLUtil_GetElementValue( (IXML_Element*)(ixmlNodeList_item (reslist, index)) ) );
	}
	return NULL;
}

#if 0
int s_YX_CD_GetMinfoByUri (char *uri, IXML_NodeList *reslist, int len, char **protocolInfo, t_MEDIA_INFO *mInfo)
{
	char *transport, *mimetype, *dlna; 
	int i;
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,len,0);
	for (i = 0; i < len; i++)
	{
		IXML_Node *res = ixmlNodeList_item (reslist, i);
		IXML_Element*element = (IXML_Element*)res;

		const char* protocol = ixmlElement_getAttribute (element, "protocolInfo");
		const char* url = XMLUtil_GetElementValue (element);

		if (uri == NULL || protocol == NULL || !s_YX_CD_CheckInfo(protocol, &dup_protocol, &transport, &mimetype, &dlna)) 
			continue;
		
		if( strcmp(uri, url) == 0 )
		{
			s_CD_CreateMediaInfo(mimetype, element, dlna, mInfo);
			if(protocolInfo)
				*protocolInfo = Dlna_strdup(protocol);
			HT_DBG_FUNC_END(i, 0);
			return i;
		}
	}
	
	HT_DBG_FUNC_END(-1, 0);
	return -1;
}

void YX_CD_ParseMetadataByUri(char *metadata, char *uri, char **protocolInfo, t_MEDIA_INFO* minfo )
{
	if( !(metadata && uri && minfo) )
		return;
	
	memset(minfo, 0, sizeof(t_MEDIA_INFO));
	
	IXML_Document *subdoc = ixmlParseBuffer( metadata );
	IXML_NodeList* items =	ixmlDocument_getElementsByTagName (subdoc, "item"); 
	IXML_Node* const node = ixmlNodeList_item(items,  0);
	IXML_Element* const elem = (IXML_Element*)node;
	IXML_NodeList* const reslist = ixmlElement_getElementsByTagName ((IXML_Element*)elem, "res");
	int nb_reslist = ixmlNodeList_length (reslist);
	
	if( nb_reslist > 0 )
		nb_reslist = s_YX_CD_GetMinfoByUri(uri, reslist, nb_reslist, protocolInfo, minfo);
	
	if (reslist)
		ixmlNodeList_free (reslist);
	if (items)
		ixmlNodeList_free (items);
}
#endif



char *Dlna_strdup(const char *str)
{
	return str?strdup(str) : NULL;
}


int YX_CD_GetMetadata(pClassContentDirectory me, char *id, char **metadata)
{
	int ret = -1;

	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW,(int)me, id);

	me->SyncBrowseMetedata(me, id);
	if( me->nb_returned > 0)
	{
		const char *result = XMLUtil_FindFirstElementValue(XML_D2N (me->ActionResult), "Result", true, true);
		*metadata = Dlna_strdup(result);
		ret = 0;
	}
	
	HT_DBG_FUNC_END(ret, *metadata);
	return ret;
}


static int s_CD_IpMatched_Ex (char *dms_ip, const char *uri)
{
	char ip[32]="";
	bool rc = false;

	if(!dms_ip || !uri)
		return rc;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,0,uri);
	if( (sscanf(uri, "http://%31[^:]", ip ) == 1) && (strcmp(dms_ip,ip)==0) )
		rc = true;
	HT_DBG_FUNC_END(rc, dms_ip);
	
	return rc;
}

static IXML_Node* s_CD_GetWantedRes(char *dms_ip, IXML_NodeList *reslist, char *method, char *option)
{
	char *dup_protocol, *transport, *mimetype, *dlna; 
	int i, len;
	IXML_Node *ret = NULL;

	if(!dms_ip || !reslist || !method || !option)
		return ret;
	
	len = ixmlNodeList_length(reslist);
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,len,0);
	
	for (i = 0; i < len; i++)
	{
		IXML_Node *res = ixmlNodeList_item (reslist, i);
		IXML_Element*element = (IXML_Element*)res;
		const char* protocol = ixmlElement_getAttribute (element, "protocolInfo");
		const char* uri = XMLUtil_GetElementValue (element);
	
		HT_DBG_FUNC(i, protocol);
        if (uri == NULL || protocol == NULL || !s_YX_CD_CheckInfo(protocol, &dup_protocol, &transport, &mimetype, &dlna)) 
			continue;
		
		if(strstr(transport, method) && s_CD_IpMatched_Ex(dms_ip, uri) )
		{
			if( !strstr(dlna, "DLNA.ORG_CI=1") )//raw media file
			{
				ret = res;
                free(dup_protocol);
				break;
			}
		}
        free(dup_protocol);
	}
	
	HT_DBG_FUNC_END((int)ret, 0);
	return ret;
}

static int s_CD_GetBestRes (char *dms_ip, char *metadata, IXML_Node **best_res)
{
	int ret = -1;
	
	if(!dms_ip || !metadata || !best_res)
		return -2;
	
	IXML_Document *subdoc = ixmlParseBuffer (discard_const_p (char, metadata));
	if(subdoc)
	{
		IXML_NodeList* items =	ixmlDocument_getElementsByTagName (subdoc, "item"); 
		if(items)
		{
			IXML_Node* node = ixmlNodeList_item(items,  0);
			if(node)
			{
				IXML_NodeList* reslist = ixmlElement_getElementsByTagName ((IXML_Element*)node, "res");
				if(reslist)
				{
					IXML_Node* res = s_CD_GetWantedRes(dms_ip, reslist, "http-get","null");
					if(res)
					{
						*best_res = ixmlNode_cloneNode(res, true);
						ret = 0;
					}
					ixmlNodeList_free (reslist);
				}
			}
			ixmlNodeList_free (items);
		}
		ixmlDocument_free(subdoc);
	}
	return ret;
}

int YX_CD_GetBestRes (char *dms_ip, char *metadata, char **res)
{
	IXML_Node *node=NULL;
	int ret = -1;
	
	if(!dms_ip || !metadata || !res)
		return -2;

	ret = s_CD_GetBestRes(dms_ip, metadata, &node);
	if(node)
	{
		*res=Dlna_strdup(node->nodeValue);
		ixmlNode_free(node);
		ret = 0;
	}
	return ret;
}
int YX_CD_GetBestUri (char *dms_ip, char *metadata, char **uri)
{
	IXML_Node *node=NULL;
	int ret = -1;
	
	if(!dms_ip || !metadata || !uri)
		return -2;
	
	ret = s_CD_GetBestRes(dms_ip, metadata, &node);
	if(node)
	{
		IXML_Element*element = (IXML_Element*)node;
		const char* str = XMLUtil_GetElementValue (element);
		*uri=Dlna_strdup(str);
		ixmlNode_free(node);
		ret = 0;
	}
	return ret;
}
int YX_CD_GetBestMediaInfo(char *dms_ip, char *metadata, t_MEDIA_INFO *minfo)
{
	char *dup_protocol, *transport, *mimetype, *dlna; 
	IXML_Node *node=NULL;
	int ret = -1;
	
	if(!dms_ip || !metadata || !minfo)
		return -2;
	
	ret = s_CD_GetBestRes(dms_ip, metadata, &node);
	if(node)
	{
		IXML_Element*element = (IXML_Element*)node;
		const char* protocol = ixmlElement_getAttribute (element, "protocolInfo");
		const char* uri = XMLUtil_GetElementValue (element);
		if (uri && protocol && s_YX_CD_CheckInfo(protocol, &dup_protocol, &transport, &mimetype, &dlna))
		{
			s_CD_CreateMediaInfo(mimetype, element, dlna, minfo);
            free(dup_protocol);
			ret = 0;
		}
		ixmlNode_free(node);
	}
	return ret;
}

