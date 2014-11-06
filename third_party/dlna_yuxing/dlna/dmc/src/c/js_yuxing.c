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
#include "coo.h"

#define GET_HND(hnd)	((pClassContentDirectory)hnd->pHandle)
#define x_IF_STR(s)	        if(s)

enum _e_dmscp_event_type_ {
	enum_DMSCP_MSG_DMS_ADDED = 0,
	enum_DMSCP_MSG_DMS_REMOVED,
	enum_DMSCP_MSG_ON_BROWSE_SEARCH,
} enum_DMSCP_EVENT_TYPE;

static char* s_dmscp_event_type[] =
{
	"DLNA_MSG_DMS_ADDED",
	"DLNA_MSG_DMS_REMOVED",
	"DLNA_MSG_ON_BROWSE_SEARCH",
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
static json_object* s_YXDL_AddDmsInfo(ListNode* node)	//512*50 Bytes
{	
    DeviceNode* devnode = node->item;
    Device *d = devnode->d;
    const char *str;
    json_object *new_dms = json_object_new_object();
    if( new_dms )
    {
        str = d->udn;
        json_object_object_add(new_dms, "UDN",          Dlna_json_object_new_string( str ));
        
        str = talloc_get_name (d);
        json_object_object_add(new_dms, "friendlyName", Dlna_json_object_new_string( str ));
        
        str = Device_GetDescDocItem(d,  "manufacturer", false);
        json_object_object_add(new_dms, "manufacturer", Dlna_json_object_new_string( str ));
        
        str = Device_GetDescDocItem(d,  "modelName", false);
        json_object_object_add(new_dms, "modelName",    Dlna_json_object_new_string( str ));
        
        str = Device_GetDescDocItem(d,  "serialNumber", false);
        json_object_object_add(new_dms, "sn",           Dlna_json_object_new_string( str ));
    }
    return new_dms;
}

static void s_YXDL_GetDmsListsEx( pYXCDL me, int start, int count, char *value, int bufferLength)	//512*50 Bytes
{	
	ListNode* node;
	int total, i;
	
	json_object *json;
	json_object *my_array, *new_dms;
	int jlen; 
	int jsonLength = 0;
	
	bufferLength -= 100;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,start, NULL);
	
    /* start=0: 0 is first; count=0: 0 means all */
	if( start < 0 || count < 0 )
		return;
	
	json = Dlna_CreateJson();

	DeviceList_Lock();

	total = ListSize(&GlobalDeviceList);
	if( start >= total )
	{
		json_object_object_add(json, "device_count", json_object_new_int(0));
	}
	else
	{
		node = ListHead (&GlobalDeviceList);
		i = 0;
		while(node && (i<start))
		{
			node = ListNext (&GlobalDeviceList, node);
			i++;
		}
		
        if( count == 0 )
            count = total;
        
        i = 0;
		my_array = json_object_new_array();
		while(my_array && node && (i<count))
		{
            new_dms = s_YXDL_AddDmsInfo(node);
            
			jlen = strlen(json_object_to_json_string(new_dms));
			if( jsonLength + jlen > bufferLength )
			{
                json_object_put(new_dms);
				break;
			}
			
			jsonLength += jlen;
			json_object_array_add(my_array, new_dms);
            i++;
            
			node = ListNext (&GlobalDeviceList, node);
		}

		json_object_object_add(json, "device_count", json_object_new_int(i));
		json_object_object_add(json, "devices", my_array);
	}

	DeviceList_Unlock();
	
	strcpy(value, json_object_to_json_string(json));
	json_object_put(json);
	HT_DBG_FUNC_END(0, 0);
}
static t_DLNA_JSON s_YXDL_GetDmsLists( pYXCDL me, int start, int count)	//512*50 Bytes
{
	int len = me->bufLen;
	char *ret = Dlna_GetStringBuf(&len);
	s_YXDL_GetDmsListsEx(me, start, count, ret, len);
	return ret;
}

static t_DLNA_JSON s_YXDL_GetDmsDescriptionXml (pYXCDL me, char *DmsUdn)
{
	int len = me->bufLen;
	char *ret = Dlna_GetStringBuf(&len);
	pClassDmsList hnd = (pClassDmsList)(me->pHandle);
	hnd->GetDmsDescriptionXml(hnd, DmsUdn, ret, len);
	return ret;
}

static int s_YXDL_GetDmsNumber (pYXCDL me)
{
	pClassDmsList hnd = (pClassDmsList)(me->pHandle);
	return hnd->GetDmsNumber(hnd);
}

static void s_YXDL_SearchDms (pYXCDL me, int remove_all)
{
	pClassDmsList hnd = (pClassDmsList)(me->pHandle);
	hnd->SearchDms(hnd, remove_all);
}

static void s_YXDL_OnDmsUpdated(char *dmsUdn, Device*d, char *alias, int action, int cookie)
{
	pYXCDL me = (pYXCDL)cookie;
	json_object *json = json_object_new_object();
	const char *str = NULL;
		
	if( action == 1 )
		json_object_object_add(json, "eventType", 	Dlna_json_object_new_string(s_dmscp_event_type[enum_DMSCP_MSG_DMS_ADDED]));
	else
		json_object_object_add(json, "eventType", 	Dlna_json_object_new_string(s_dmscp_event_type[enum_DMSCP_MSG_DMS_REMOVED]));

	str = dmsUdn;
	json_object_object_add(json, "deviceID", 		Dlna_json_object_new_string( str ));
	str = alias;
	json_object_object_add(json, "alias", 			Dlna_json_object_new_string( str ));
	json_object_object_add(json, "action", 			json_object_new_int(action));
	
	me->list->Put(me->list, me, (char*)json_object_to_json_string(json));
	json_object_put(json);
}

static int s_YXDL_DispatchFunc(int hndx, char *func, char *para, char *value, int len)
{
	char *str = NULL;
	
	pYXCDL hnd=(pYXCDL)hndx;
	hnd->bufLen = len;
	if( strcasecmp(func, "GetDmsNumber") == 0 )
	{
		sprintf(value, "%d", hnd->GetDmsNumber(hnd));
		return 0;
	}
	else if( strcasecmp(func, "GetDmsLists") == 0 )
	{
		char *p1 = Dlna_FindCharAndInc(para, ',');
		str = hnd->GetDmsLists(hnd, atoi(para), atoi(p1));
	}
	else if( strcasecmp(func, "GetDmsDescriptionXml") == 0 )
	{
		str = hnd->GetDmsDescriptionXml(hnd, para);
	}
	else if( strcasecmp(func, "SearchDms") == 0 )
	{
		hnd->SearchDms(hnd, atoi(para));
		return 0;
	}
	else if( strcasecmp(func, "Release") == 0 )
	{
		hnd->Release(hnd);
		return 0;
	}
	else
	{
		sprintf(value, "%d", -1);
		return -1;
	}

	return Dlna_CheckStringAndReturn(str, value, len);
}

void s_YXDL_Release(pYXCDL p)
{
	if( p)
	{
		EventQueue_Release(p->list);
		free(p);
	}
}


int DmsList_Create( void)
{
	pYXCDL me=NULL;
	int size = sizeof(YXClassDmsList);

	me = (pYXCDL)malloc(size);
	memset(me,0,size);
	if( me )
	{
		me->DispatchFunc			= s_YXDL_DispatchFunc;
		me->pHandle 				= YX_DmsList_Create(s_YXDL_OnDmsUpdated, (int)me, NULL);
		
		me->GetDmsNumber			= s_YXDL_GetDmsNumber;
		me->GetDmsLists				= s_YXDL_GetDmsLists;
		me->GetDmsDescriptionXml 	= s_YXDL_GetDmsDescriptionXml;
		me->SearchDms				= s_YXDL_SearchDms;
		me->Release					= s_YXDL_Release;
		
		me->list 					= EventQueue_Create();
	}

	return (int)me;
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////
static int s_YXCD_SetFilter(pYXCCD me, char *filter) 
{
	pClassContentDirectory hnd = (pClassContentDirectory)(me->pHandle);
	return hnd->SetFilter(hnd, filter);
}
static int s_YXCD_SetSearchCriteria(pYXCCD me, char *searchCriteria) 
{
	pClassContentDirectory hnd = (pClassContentDirectory)(me->pHandle);
	return hnd->SetSearchCriteria(hnd, searchCriteria);
}
static int s_YXCD_SetSortCriteria(pYXCCD me, char *sortCriteria) 
{
	pClassContentDirectory hnd = (pClassContentDirectory)(me->pHandle);
	return hnd->SetSortCriteria(hnd, 0 , 0 , sortCriteria);
}
static int s_YXCD_SetDmsUdn(pYXCCD me, char *dmsUdn) 
{
	pClassContentDirectory hnd = (pClassContentDirectory)(me->pHandle);
	return hnd->SetDmsUdn(hnd, dmsUdn);
}
static char* s_YXCD_GetDmsUdn(pYXCCD me) 
{
	pClassContentDirectory hnd = (pClassContentDirectory)(me->pHandle);
	return hnd->GetDmsUdn(hnd);
}
static int s_YXCD_SetResultFormat(pYXCCD me, int format) 
{
	me->format = format;
	return format;
}
static char* s_YXCD_GetSearchCapabilities(pYXCCD me) 
{
	pClassContentDirectory hnd = (pClassContentDirectory)(me->pHandle);
	hnd->GetSearchCapabilities(hnd, NULL, 0);
	return NULL;
}
static char* s_YXCD_GetSortCapabilities(pYXCCD me) 	
{
	pClassContentDirectory hnd = (pClassContentDirectory)(me->pHandle);
	hnd->GetSortCapabilities(hnd, NULL, 0);
	return NULL;
}
static void s_YXCD_Reset(pYXCCD me) 
{
	pClassContentDirectory hnd = (pClassContentDirectory)(me->pHandle);
	hnd->Reset(hnd);
}

static void s_YXCD_FillResult(json_object *my_array, int nb_returned, int nb_matched, char *value, int bufferLength)
{
    json_object *json = json_object_new_object();
    if( json )
    {
        if(my_array)
        {
            json_object_object_add(json, "NumberReturned", json_object_new_int(nb_returned));
            json_object_object_add(json, "TotalMatches", json_object_new_int(nb_matched));
            json_object_object_add(json, "Result", my_array);
        }
        else
        {
            json_object_object_add(json, "NumberReturned", json_object_new_int(0));
            json_object_object_add(json, "TotalMatches", json_object_new_int(0));
        }
        
        strcpy(value, json_object_to_json_string(json));
        json_object_put(json);
    }
}
static void s_YXCD_ComposeContainerList_sub(IXML_Node *node, json_object *new_object)
{
	IXML_Element* const elem = (IXML_Element*)node;
	const char *aim, *str;
	
    aim = "id";
    str = ixmlElement_getAttribute(elem, aim);
    json_object_object_add(new_object, aim,         Dlna_json_object_new_string( str ));
    
    aim = "parentID";
    str = ixmlElement_getAttribute(elem, aim);
    json_object_object_add(new_object, aim,         Dlna_json_object_new_string( str ));
    
    aim = "childCount";
    str = ixmlElement_getAttribute(elem, aim);
    json_object_object_add(new_object, aim,         Dlna_json_object_new_string( str ));
    
    aim = "upnp:class";
    str = XMLUtil_FindFirstElementValue (node, aim,false, true);
    json_object_object_add(new_object, "upnpClass", Dlna_json_object_new_string( str ));
    
    aim = "dc:title";
    str = XMLUtil_FindFirstElementValue (node, aim,false, true);
    json_object_object_add(new_object, "title",     Dlna_json_object_new_string( str ));
    
    aim = "dc:date";
    str = XMLUtil_FindFirstElementValue (node, aim,false, false);
    json_object_object_add(new_object, "date",      Dlna_json_object_new_string( str ));
}
static void s_YXCD_ComposeItemList_sub(pYXCCD hnd, IXML_Node *node, json_object *new_object)
{
	IXML_Element* const elem = (IXML_Element*)node;
	const char *aim, *str;
	const char *upnpClass;

    aim = "id";
    str = ixmlElement_getAttribute(elem, aim);
    json_object_object_add(new_object, aim,         Dlna_json_object_new_string( str ));
    
    aim = "parentID";
    str = ixmlElement_getAttribute(elem, aim);
    json_object_object_add(new_object, aim,         Dlna_json_object_new_string( str ));

    aim = "upnp:class";
    str = XMLUtil_FindFirstElementValue (node, aim,false, true);    upnpClass = str;
    json_object_object_add(new_object, "upnpClass", Dlna_json_object_new_string( str ));

    aim = "dc:title";
    str = XMLUtil_FindFirstElementValue (node, aim,false, true);
	coo_str_rm_ext(str);
    json_object_object_add(new_object, "title",     Dlna_json_object_new_string( str ));

    aim = "dc:date";
    str = XMLUtil_FindFirstElementValue (node, aim,false, false);
    json_object_object_add(new_object, "date",      Dlna_json_object_new_string( str ));


	aim = "dc:description";
	str = XMLUtil_FindFirstElementValue (node, aim,false, false);
	x_IF_STR(str)
		json_object_object_add(new_object, "description",      Dlna_json_object_new_string( str ));


	aim = "upnp:recordedStartDateTime";
	str = XMLUtil_FindFirstElementValue (node, aim,false, false);
	x_IF_STR(str)
	json_object_object_add(new_object, "start_time",      Dlna_json_object_new_string( str ));
	
	IXML_NodeList* const reslist = ixmlElement_getElementsByTagName ((IXML_Element*)elem, "res");
    int const nb_reslist = ixmlNodeList_length (reslist);
    if( nb_reslist > 0 )
    {
        char *protocolInfo = NULL, *url=NULL;
        t_MEDIA_INFO mInfo; 
		IXML_Element *res = NULL;
		int idx = -1;
        memset(&mInfo, 0, sizeof(t_MEDIA_INFO));
        
        idx = YX_CD_GetPreferred(GET_HND(hnd), reslist, nb_reslist, &protocolInfo, &url, &mInfo);
		if (idx >= 0)
			res = (IXML_Element*)ixmlNodeList_item (reslist, idx);
        str = protocolInfo;
        json_object_object_add(new_object, "protocolInfo",  Dlna_json_object_new_string( str ));
        str = url;
        json_object_object_add(new_object, "url",           Dlna_json_object_new_string( str ));

		aim = "size";
		str = ixmlElement_getAttribute(res,aim);
		x_IF_STR(str)
			json_object_object_add(new_object, "size",Dlna_json_object_new_string(str));

		aim = "duration";
		str = ixmlElement_getAttribute (res, aim);
		x_IF_STR(str)
			json_object_object_add(new_object, aim, json_object_new_string( str ));

		aim = "prog_title";
		str = ixmlElement_getAttribute (res, aim);
		x_IF_STR(str)
			json_object_object_add(new_object, "programTitle", 	Dlna_json_object_new_string( str ));

		aim = "end_time";
		str = ixmlElement_getAttribute(res, aim);
		x_IF_STR(str)
			json_object_object_add(new_object, aim, json_object_new_string(str));

		aim = "PvrID";
		str = ixmlElement_getAttribute(res, aim);
		x_IF_STR(str)
			json_object_object_add(new_object, aim, json_object_new_string(str));

        if( protocolInfo ) free(protocolInfo);
        if( url ) free(url);
        
        if( strstr(upnpClass, "image") )
            str = YX_CD_GetThumbnailForImage (GET_HND(hnd), reslist, nb_reslist, 10000, 10000);
        else
            str = YX_CD_GetThumbnailForVideo (GET_HND(hnd), reslist, nb_reslist);
        json_object_object_add(new_object, "thumbnail",     Dlna_json_object_new_string( str ));
        if(str) free((char*)str);
    }
    else
    {
        str = NULL;
        json_object_object_add(new_object, "protocoalInfo", Dlna_json_object_new_string( str ));
        str = NULL;
        json_object_object_add(new_object, "url",           Dlna_json_object_new_string( str ));
        
        str = NULL;
        json_object_object_add(new_object, "thumbnail",     Dlna_json_object_new_string( str ));
    }

    if (reslist)
        ixmlNodeList_free (reslist);
}

static void s_ParseResultToYuxingJson (pYXCCD hnd, char *value, int bufferLength)
{
	pClassContentDirectory me = (pClassContentDirectory)(hnd->pHandle);
	IXML_Document *subdoc = me->subdoc;
	json_object *my_array, *new_object;
	int jlen; 
	int jsonLength = 0;

	bufferLength -= 100;
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, bufferLength, 0);
	
	IXML_NodeList* containers = ixmlDocument_getElementsByTagName(subdoc, "container"); 
	int const nb_containers = ixmlNodeList_length (containers);
	HT_DBG_FUNC(nb_containers, "nb_containers = ");
	
	IXML_NodeList* items =	ixmlDocument_getElementsByTagName (subdoc, "item"); 
	//int const nb_items = ixmlNodeList_length (items);
	HT_DBG_FUNC(ixmlNodeList_length (items), "nb_items = ");

	if( (me->nb_returned > 0) && (me->index < me->nb_returned) )
	{
		int index = me->index;
		my_array = json_object_new_array();
		for (; me->index < me->nb_returned; me->index++) 
		{
			int i = me->index; 
			bool const is_container = (i < nb_containers);
			IXML_Node* const node = ixmlNodeList_item(is_container ? containers : items,  is_container ? i : i - nb_containers);
			
			HT_DBG_FUNC(is_container, "is_container = ");
			new_object = json_object_new_object();
			if(is_container)
                s_YXCD_ComposeContainerList_sub(node, new_object);
			else
                s_YXCD_ComposeItemList_sub(hnd, node, new_object);
			
			jlen = strlen(json_object_to_json_string(new_object));
			if( jsonLength + jlen > bufferLength )
			{
                json_object_put(new_object);
				break;
			}
			
			jsonLength += jlen;
			json_object_array_add(my_array, new_object);
		}
		
        s_YXCD_FillResult(my_array, me->index-index, me->nb_matched, value, bufferLength);
	}
	else
	{
        s_YXCD_FillResult(0, 0, me->nb_matched, value, bufferLength);
	}
	
	if (containers)
		ixmlNodeList_free (containers);
	if (items)
		ixmlNodeList_free (items);
}

static enum_DLNA_RET s_YXCD_BrowseMetedata(pYXCCD me, t_DLNA_OBJECTID objectId) 
{
	pClassContentDirectory hnd = (pClassContentDirectory)(me->pHandle);
	return hnd->BrowseMetedata(hnd, objectId);
}
static enum_DLNA_RET s_YXCD_BrowseChildren(pYXCCD me, t_DLNA_OBJECTID objectId, int startingIndex, int requestedCount) 	
{
	pClassContentDirectory hnd = (pClassContentDirectory)(me->pHandle);
	return hnd->BrowseChildren(hnd, objectId, startingIndex, requestedCount);
}
static enum_DLNA_RET s_YXCD_Search(pYXCCD me, t_DLNA_OBJECTID objectId, int startingIndex, int requestedCount) 
{
	pClassContentDirectory hnd = (pClassContentDirectory)(me->pHandle);
	return hnd->Search(hnd, objectId, startingIndex, requestedCount);
}

static void s_YXCD_OnBrowseResult(int nb_returned, int cookie)
{
	pYXCCD me = (pYXCCD)cookie;
	json_object *json = json_object_new_object();
	json_object_object_add(json, "eventType", Dlna_json_object_new_string(s_dmscp_event_type[enum_DMSCP_MSG_ON_BROWSE_SEARCH]));
	json_object_object_add(json, "NumberReturned", json_object_new_int(nb_returned));
	me->list->Put(me->list, me, (char*)json_object_to_json_string( json ));
	json_object_put(json);
}

static void s_YXCD_OnResultPartialEx(pYXCCD me, int format, char *buf, int bufferLength)
{
	pClassContentDirectory hnd = (pClassContentDirectory)(me->pHandle);
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,format, "me->format = ");

	if( format == 1 )
		s_ParseResultToYuxingJson(me, buf, bufferLength);
	else if( format == 2 )
	{
        json_object *xml_result = NULL;
		if( hnd->ActionResult )
		{
			const char *result = XMLUtil_FindFirstElementValue(XML_D2N (hnd->ActionResult), "Result", true, true);
			xml_result = Dlna_json_object_new_string(result);
            bufferLength -= 100;
            if( strlen(json_object_to_json_string(xml_result)) > bufferLength )
            {
                json_object_put(xml_result);
                xml_result = NULL;
            }
		}
        s_YXCD_FillResult(xml_result, hnd->nb_returned, hnd->nb_matched, buf, bufferLength);
	}
	else
	{
	}
	
	HT_DBG_FUNC_END(format, NULL);
}
static t_DLNA_JSON s_YXCD_OnResultPartial( pYXCCD me, int format)
{
	int len = me->bufLen;
	char *buf = Dlna_GetStringBuf(&len);
	s_YXCD_OnResultPartialEx(me, format, buf, len);
	return buf;
}

static t_DLNA_JSON s_YXCD_SyncResult(pYXCCD me, int rc)
{
	if( rc == enum_DLNA_ACTION_OK ) 
		return s_YXCD_OnResultPartial(me, me->format);
	else
	{
		int len = me->bufLen;
		char *buf = Dlna_GetStringBuf(&len);
		json_object *json = Dlna_CreateJson();
		json_object_object_add(json, "NumberReturned", json_object_new_int(rc));
		strcpy(buf, json_object_to_json_string(json));
		json_object_put(json);
		return buf;
	}
}
static t_DLNA_JSON s_YXCD_SyncSearch(pYXCCD me, t_DLNA_OBJECTID containerID, int startingIndex, int requestedCount)
{
	pClassContentDirectory p = (pClassContentDirectory)(me->pHandle);
	enum_DLNA_RET rc = p->SyncSearch(p, containerID, startingIndex, requestedCount);
	return s_YXCD_SyncResult(me, rc);
}

static t_DLNA_JSON s_YXCD_SyncBrowseChildren(pYXCCD me, t_DLNA_OBJECTID containerID, int startingIndex, int requestedCount)
{
	pClassContentDirectory p = (pClassContentDirectory)(me->pHandle);
	enum_DLNA_RET rc = p->SyncBrowseChildren(p, containerID, startingIndex, requestedCount);
	return s_YXCD_SyncResult(me, rc);
}

static t_DLNA_JSON s_YXCD_SyncBrowseMetedata(pYXCCD me, t_DLNA_OBJECTID itemID)
{
	pClassContentDirectory p = (pClassContentDirectory)(me->pHandle);
	enum_DLNA_RET rc = p->SyncBrowseMetedata(p, itemID);
	return s_YXCD_SyncResult(me, rc);
}

static void s_YXCD_OnDmsUpdated(char *dmsUdn, Device*d, char *alias, int action, int cookie)
{
	pYXCCD me = (pYXCCD)cookie;
	const char *str = NULL;
	json_object *json = json_object_new_object();
		
	json_object_object_add(json, "eventType", Dlna_json_object_new_string(s_dmscp_event_type[enum_DMSCP_MSG_DMS_REMOVED]));
	
	str = dmsUdn;
	json_object_object_add(json, "deviceID", Dlna_json_object_new_string( str ));
	str = alias;
	json_object_object_add(json, "alias", Dlna_json_object_new_string( str ));
	json_object_object_add(json, "action", json_object_new_int(action));
	
	me->list->Put(me->list, me, (char*)json_object_to_json_string(json));
	me->Reset(me);
	json_object_put(json);
}

static int s_YXCD_DispatchFunc(int hndx, char *func, char *para, char *value, int len)
{
	char *str, *p1, *p2, *p3, *p4;
	int ret;
	
	str=p1=p2=p3=p4=NULL;
	
	pYXCCD hnd=(pYXCCD)hndx;
	hnd->bufLen = len;
	if( strcasecmp(func, "SetDmsUdn") == 0 )
	{
		hnd->SetDmsUdn(hnd, value);
		return 0;
	}
	else if( strcasecmp(func, "GetDmsUdn") == 0 )
	{
		str = hnd->GetDmsUdn(hnd);
	}
	else if( strcasecmp(func, "SetFilter") == 0 )
	{
		hnd->SetFilter(hnd, value);
		return 0;
	}
	else if( strcasecmp(func, "SetSearchCriteria") == 0 )
	{
		hnd->SetSearchCriteria(hnd, value);
		return 0;
	}
	else if( strcasecmp(func, "SetSortCriteria") == 0 )
	{
		hnd->SetSortCriteria(hnd, value);
		return 0;
	}
	else if( strcasecmp(func, "GetSearchCapabilities") == 0 )
	{
		str = hnd->GetSearchCapabilities(hnd);
	}
	else if( strcasecmp(func, "GetSortCapabilities") == 0 )
	{
		str = hnd->GetSortCapabilities(hnd);
	}
	else if( strcasecmp(func, "BrowseMetedata") == 0 )
	{
		sprintf(value, "%d", hnd->BrowseMetedata(hnd, para));
		return 0;
	}
	else if( strcasecmp(func, "BrowseChildren") == 0 )
	{
		p1 = Dlna_FindCharAndInc(para, ',');
		p2 = Dlna_FindCharAndInc(p1, ',');

		if( p2 )
			ret = hnd->BrowseChildren(hnd, para, atoi(p1), atoi(p2));
		else
			ret = hnd->BrowseChildren(hnd, para, -1, -1);
		sprintf(value, "%d",ret);
		return 0;
	}
	
	else if( strcasecmp(func, "Search") == 0 )
	{
		p1 = Dlna_FindCharAndInc(para, ',');
		p2 = Dlna_FindCharAndInc(p1, ',');

		if( p2 )
			ret = hnd->Search(hnd, para, atoi(p1), atoi(p2));
		else
			ret = hnd->Search(hnd, para, -1, -1);
		sprintf(value, "%d",ret);
		return 0;
	}
	else if( strcasecmp(func, "OnResultPartial") == 0 )
	{
		str = hnd->OnResultPartial(hnd, atoi(para));
	}
	else if( strcasecmp(func, "Reset") == 0 )
	{
		hnd->Reset(hnd);
		return 0;
	}
	else if( strcasecmp(func, "Release") == 0 )
	{
		hnd->Release(hnd);
		return 0;
	}	
	else
	{
		sprintf(value, "%d", -1);
		return -1;
	}

	return Dlna_CheckStringAndReturn(str, value, len);
}

static void s_YXCD_Release(pYXCCD me)
{
	if(me)
	{
		pClassContentDirectory hnd = (pClassContentDirectory)(me->pHandle);
		hnd->Release(hnd);
		free(me);
	}
}
int CD_create(void)
{
	YXClassContentDirectory *hnd=NULL;
	int size = sizeof(YXClassContentDirectory);

	hnd = (YXClassContentDirectory*)malloc(size);
	memset(hnd,0,size);
	
	if( hnd )
	{
		hnd->DispatchFunc			= s_YXCD_DispatchFunc;
		hnd->pHandle				= YX_CD_Create(s_YXCD_OnDmsUpdated, s_YXCD_OnBrowseResult, (int)hnd, NULL);
		
		hnd->SetDmsUdn				= s_YXCD_SetDmsUdn;
		hnd->GetDmsUdn				= s_YXCD_GetDmsUdn;
		hnd->GetSearchCapabilities	= s_YXCD_GetSearchCapabilities;
		hnd->GetSortCapabilities	= s_YXCD_GetSortCapabilities;
		
		hnd->SetFilter				= s_YXCD_SetFilter;
		hnd->SetSearchCriteria 		= s_YXCD_SetSearchCriteria;
		hnd->SetSortCriteria 		= s_YXCD_SetSortCriteria;
		hnd->SetResultFormat		= s_YXCD_SetResultFormat;
		
		hnd->BrowseMetedata 		= s_YXCD_BrowseMetedata;
		hnd->BrowseChildren 		= s_YXCD_BrowseChildren;
		hnd->Search 				= s_YXCD_Search;
		hnd->OnResultPartial		= s_YXCD_OnResultPartial;
		
		hnd->Reset 					= s_YXCD_Reset;
		hnd->Release				= s_YXCD_Release;

		hnd->SyncBrowseChildren		= s_YXCD_SyncBrowseChildren;
		hnd->SyncBrowseMetedata		= s_YXCD_SyncBrowseMetedata;
		hnd->SyncSearch 			= s_YXCD_SyncSearch;

		hnd->list 					= EventQueue_Create();
	}
	
	return  (int)hnd;	
}



extern int new_AVTransport(t_PLAYER_EVENT callback, void *user);
extern int new_ImagePlayer(t_PLAYER_EVENT callback, void *user);
extern int new_RenderingCS(void);

static int s_Dlna_ParseJs( const char *name, char *buf, int len)
{
	char *identifier = "DLNA.";
	
	if( strncasecmp(name, identifier, strlen(identifier)) == 0 )
	{
		char *dot1 = Dlna_FindCharAndInc(name, '.'); 
		char *dot2 = Dlna_FindCharAndInc(dot1, '.'); 
		char *dot3 = Dlna_FindCharAndInc(dot2, '.'); 
		
		int hnd = atoi(dot1);
#if 0
		if(hnd == 1 )
			return Raw_Dmc_HuaweiJse(dot2, dot3, buf, len);
#endif			
		if(hnd)
			return ((pClassDmcBase)hnd)->DispatchFunc(hnd, dot2, dot3, buf, len);

		if( strcmp(dot2, "ClassDmsList") == 0 )
		{
			hnd = DmsList_Create();
		}
		else if( strcmp(dot2, "ClassContentDirectory") == 0 )
		{
			hnd = CD_create();
		}
#if 0		
		else if( strcmp(dot2, "CreateDmrList") == 0 )
		{
		//	hnd = DmrList_Create();
		}
#endif		
		else if( strcmp(dot2, "ClassImagePlayer") == 0 )
		{
			hnd = new_ImagePlayer(NULL, NULL);
		}
		else if( strcmp(dot2, "ClassAVPlayer") == 0 )
		{
			hnd = new_AVTransport(NULL, NULL);
		}
		else if( strcmp(dot2, "ClassRenderingControl") == 0 )
		{
			hnd = new_RenderingCS();
		}
#if 0		
		else if( strcmp(dot2, "ClassConnectionManager") == 0 )
		{
			//hnd = new_ConnectionManager();
		}
		else if( strcmp(dot2, "GetHnd") == 0 )
		{
			//hnd = s_Dlna_EventQueue_GetHnd(buf,len);
		}
#endif		
		else if( strcmp(dot2, "GetEvent") == 0 )
		{
			return Js_EventQueue_GetEvent(buf,len);
		}
		else if( strcmp(dot2, "print") == 0 )
		{
			return 0;
		}
		else
		{
		}
		
		sprintf(buf, "%d", hnd);
		return 0;
	}

	return -1;
}
int Raw_Dlna_JsRead( const char *name, char *buf, int len)
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_KEY,len, name);
	HT_DBG_FUNC(len, buf);
	
	int ret = s_Dlna_ParseJs(name,buf,len);
	
	HT_DBG_FUNC_END(ret, buf);
	return ret;
}
int Raw_Dlna_JsWrite( const char *name, char *buf, int len)
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_KEY,len, name);
	HT_DBG_FUNC(len, buf);

	int ret = s_Dlna_ParseJs(name,buf,len);
	
	HT_DBG_FUNC_END(ret, buf);
	return ret;
}




