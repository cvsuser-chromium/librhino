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
#include "config.h"
#endif
#include "device_list.h"
#include "xml_util.h"

#include "dmscp.h"
#include "js_common.h"
#include "js_eventqueue.h"
#include "js_huawei.h"
#include "hitTime.h"
#include <sys/time.h>
#include "coo.h"

#define HUAWEI_OBJECT_HND	(void*)1
#define x_IF_STR(s)	        if(s)
#if 0
typedef enum
{
    x_DTB_UNKOWN=1,
    x_DTB_FOLDER=2,
    x_DTB_VIDEO=4,
    x_DTB_AUDIO=8,
    x_DTB_IMAGE=16,
}x_DTB;
#define x_DLNA_TYPE_ALL (x_DTB_UNKOWN|x_DTB_FOLDER|x_DTB_VIDEO|x_DTB_AUDIO|x_DTB_IMAGE)

typedef struct _hw_browse_cache_array_
{
	x_DTB				*array;
    int                 num;

    char                id[512];
    char                dms_udn[128];
}x_HBCA;

static x_HBCA s_hbca = {0};
#endif
static pClassDmsList			s_huawei_dms_list = NULL;
static pClassContentDirectory	s_huawei_browse = NULL;
static pClassContentDirectory	s_huawei_search = NULL;
static pClassEventQueue 		s_huawei_event_queue= NULL;
static int s_sort_flag = 0;
//static int

static void s_HuaweiV1R5_EventQueue_Put(const char *string)
{
  if( s_huawei_event_queue )
    s_huawei_event_queue->Put(s_huawei_event_queue, HUAWEI_OBJECT_HND, (char*)string);
}


#if 0

#endif
/*
   {"type":"EVENT_DLNA_DMSLIST_CHANGE"}
   */
static void s_HuaweiV1R5_OnDmsChanged(char *dmsUdn, Device *d, char *alias, int action, int cookie)
{
  json_object *new_dms = json_object_new_object();
  if(!new_dms)
    return;

  json_object_object_add(new_dms, "type", Dlna_json_object_new_string("EVENT_DLNA_DMSLIST_CHANGED"));

  s_HuaweiV1R5_EventQueue_Put(json_object_to_json_string(new_dms) );
  json_object_put(new_dms);
}

/*
   {"type":"EVENT_DLNA_FILELIST_CHANGE","deviceID":"uuid:a9c8074e-1dd1-11b2-92dc-fabd98cd1111","containerID":"10"}
   */
static void s_HuaweiV1R5_PostContainerChanged(char *dmsUdn, char *containerID)
{
  json_object *json = json_object_new_object();
  if(!json)
    return;

  json_object_object_add(json, "type", Dlna_json_object_new_string("EVENT_DLNA_FILELIST_CHANGE"));
  json_object_object_add(json, "deviceID", Dlna_json_object_new_string(dmsUdn));
  json_object_object_add(json, "containerID", Dlna_json_object_new_string(containerID));

  s_HuaweiV1R5_EventQueue_Put(json_object_to_json_string(json) );
  json_object_put(json);
}
static void s_HuaweiV1R5_OnServiceChanged(char *dmsUdn, char *name, char *value, int hnd)
{
  if( strcmp(name, "ContainerUpdateIDs") )
    return;
  if( !name || !value )
    return;

  char *p, *p1, *p2, separator = ',';

  p  = value;
  p1 = strchr(p, separator);
  p2 = p1? strchr(p1, separator):NULL;

  while(p1&&p2)
  {
    *p1 = 0;
    s_HuaweiV1R5_PostContainerChanged(dmsUdn, p);
    *p1 = separator;

    p  = p2+1;
    p1 = strchr(p, separator);
    p2 = p1? strchr(p1, separator):NULL;
  }

  if(p1)
  {
    *p1 = 0;
    s_HuaweiV1R5_PostContainerChanged(dmsUdn, p);
    *p1 = separator;
  }
}


/*
   {"count":3,"dmsList":[
   {"deviceID":"uuid:a9c8074e-1dd1-11b2-92dc-fabd98cd1111","friendlyName":"HBox01","deviceMode":"EC2118","deviceSN":"111111"},
   {"deviceID":"uuid:a9c8074e-1dd1-11b2-92dc-fabd98cd2222","friendlyName":"HBox02","deviceMode":"EC2118","deviceSN":"222222"},
   {"deviceID":"uuid:a9c8074e-1dd1-11b2-92dc-fabd98cd3333","friendlyName":"HBox03","deviceMode":"EC2118","deviceSN":"333333"}]}
   */
static json_object* s_HuaweiV1R5_AddDmsInfo(ListNode* node)	//512*50 Bytes
{
  DeviceNode* devnode = node->item;
  Device *d = devnode->d;
  const char *str;
  json_object *new_dms = json_object_new_object();
  if( new_dms )
  {
    str = d->udn;
    Dlna_Json_Object_Add_String(new_dms, "deviceID", str);

    str = talloc_get_name (d);
    Dlna_Json_Object_Add_String(new_dms, "friendlyName", str);

    str = Device_GetDescDocItem(d,  "modelName", false);
    Dlna_Json_Object_Add_String(new_dms, "deviceMode", str);

    str = Device_GetDescDocItem(d,  "serialNumber", false);
    Dlna_Json_Object_Add_String(new_dms, "deviceSN", str);
  }
  return new_dms;
}
static void s_HuaweiV1R5_GetDmsLists( int start, int count, char *value, int bufferLength)	//512*50 Bytes
{
  ListNode* node;
  int total, i;

  json_object *json;
  json_object *my_array, *new_dms;
  int jlen;
  int jsonLength = 0;

  bufferLength -= 100;

  HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW,start, NULL);

  /* start=0: 0 is first; count=0: 0 means all */
  if( start < 0 || count < 0 )
    return;

  json = Dlna_CreateJson();

  DeviceList_Lock();

  total = ListSize(&GlobalDeviceList);
  if( start >= total )
  {
    Dlna_Json_Object_Add_Int(json, "count", 0);
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
      new_dms = s_HuaweiV1R5_AddDmsInfo(node);

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

    Dlna_Json_Object_Add_Int(json, "count", i);
    json_object_object_add(json, "dmsList", my_array);
  }

  DeviceList_Unlock();

  strcpy(value, json_object_to_json_string(json));
  json_object_put(json);
  HT_DBG_FUNC_END(0, value);
}




/*
   classID???????£?
   1??ͼƬ??
   11??ͼƬר????
   12????ר?????Ե?ͼƬ??
   2?????֡?
   21????ר?????Ե????֡?
   22??????ר????
   3????Ƶ??
   4?????Ŵ???????ǩ??
   */
static char* s_HuaweiV1R5_ParseClassID(char *para)
{
  char *criteria = "*";

  HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, 0, para);
  //	char *p1 = strchr(para, ',');
  if( para)
  {
    //p1++;
    if( strncmp(para, "1", 1) == 0 )
    {
      criteria = "upnp:class derivedfrom \"object.item.imageItem\"";
    }
    else if(strncmp(para, "12", 2) == 0)
    {
      criteria = "upnp:class derivedfrom \"object.item.imageItem\"";
    }
    else if(strncmp(para, "11", 2) == 0)
    {
      criteria = "upnp:class derivedfrom \"object.container.album.photoAlbum\"";
    }
    else if(strncmp(para, "2", 1) == 0)
    {
      criteria = "upnp:class derivedfrom \"object.item.audioItem\"";
    }
    else if(strncmp(para, "22", 2) == 0)
    {
      criteria = "upnp:class derivedfrom \"object.container.album.musicAlbum\"";
    }
    else if(strncmp(para, "3", 1) == 0)
    {
      criteria = "upnp:class derivedfrom \"object.item.videoItem\"";
    }
    else if(strncmp(para, "4", 1) == 0)
    {
    }
    else
    {
    }
  }
  HT_DBG_FUNC(0, criteria);
  return criteria;
}
static const char* s_HuaweiV1R5_GuessItemType(const char*upnp_class)
{
  const char* str= upnp_class;
  if(!str)
    return "unknown";

  if( strstr(str, "object.item.imageItem") )
    return "1";
  else if( strstr(str, "object.item.audioItem") )
    return "2";
  else if( strstr(str, "object.item.videoItem") )
    return "3";
  else
    return "unknown";
}
static const char* s_HuaweiV1R5_GuessItemType2(const char*upnp_class)
{
    const char* str= upnp_class;
    if(!str)
        return "a0";

    if( strstr(str, "object.item.imageItem") )
        return "a11";
    else if( strstr(str, "object.item.audioItem") )
        return "a3";
    else if( strstr(str, "object.item.videoItem") )
        return "a5";
    else if( strstr(str, "object.container.playlistContainer") )
        return "a2";
    else if( strstr(str, "object.container") )
        return "a2";
    else
        return "a0";
}


static void s_HW_V1R5_Browse_FillResult(json_object *my_array, int num, char *value, int bufferLength)
{
  json_object *json = json_object_new_object();
  if( json )
  {
    if(my_array)
    {
      json_object_object_add(json, "count", json_object_new_int(num));
      json_object_object_add(json, "fileList", my_array);
    }
    else
      json_object_object_add(json, "count", json_object_new_int(0));

    strcpy(value, json_object_to_json_string(json));
    json_object_put(json);
  }
}

static void s_HuaweiV1R5_ComposeContainerList_sub(IXML_Node *node, json_object *new_object)
{
  IXML_Element* const elem = (IXML_Element*)node;
  const char *aim, *str;

  aim = "id";
  str = ixmlElement_getAttribute(elem, aim);
  json_object_object_add(new_object, "objectID", Dlna_json_object_new_string( str ));

  aim = "parentID";
  str = ixmlElement_getAttribute(elem, aim);
  json_object_object_add(new_object, aim, 		Dlna_json_object_new_string( str ));

	aim = "dc:title";
	str = XMLUtil_FindFirstElementValue (node, aim,false, true);
	json_object_object_add(new_object, "filename", 		Dlna_json_object_new_string( str ));


	aim = "upnp:class";
    str = XMLUtil_FindFirstElementValue (node, aim,false, true);
	if (str)
	{
		str = s_HuaweiV1R5_GuessItemType2(str);
	}
   	else
   	{
		aim = "upnp:searchClass";//»ªÎªÌØÊâÐèÇó
       	str = XMLUtil_FindFirstElementValue (node, aim,false, true);
		if (str)
		{
			str = s_HuaweiV1R5_GuessItemType2(str);
		}
   	}
	x_IF_STR(str)
	json_object_object_add(new_object, "classID",		Dlna_json_object_new_string( str ));
}

/*
   {"count":3,"fileList":[{"objectID":"11","filename":"file1","classID":3,"filepath":"http://192.168.0.1/folder/file1.mpg","size":1024,"bitrate":2666,"resolution":"480*320","duration":60,"audiochannel":2,"protocolInfo":"video/mpeg","colorDepth":8,"contentSource":0,"channelNO":"123","iptvUserID":"user","contentType":0},{"objectID":"22","filename":"file2","classID":1,"filepath":"http://192.168.0.1/folder/file2.jpg","size":256,"protocolInfo":"image/jpeg","colorDepth":8,"contentSource":2},{"objectID":"33","filename":"folder1","classID":5,"filepath":"http://192.168.0.1/folder/folder1","contentSource":2}]}

   objectID	M	String(64)	?ļ?ID??
   filename	M	String(256)	?ļ???Ŀ¼??ר???????ơ?
   classID	M	Int(2)	?ļ????͡?
   filepath	M	String(256)	?ļ?·???????磬ĳ??DMS?Ϲ???????Ƶ?ļ???·??http://192.168.1.2/share/video1.mpg
   size	O	Int	?ļ???С????λ?ֽڡ???????ͨ?ļ???Ч??????????ΪĿ¼??ר??????ǩʱ???ֶ???Ч??
   bitrate	O	Int	?????ʣ???λkbps?????????ֺ???Ƶ?ļ???Ч??
   resolution	O	String(12)	?ֱ??ʡ????ͼƬ????Ƶ?ļ???Ч??
   duration	O	Int	???ȣ???λ?롣??????Ƶ????Ƶ?ļ???Ч??
   audiochannel	O	Int	??????????????Ƶ????Ƶ?ļ???Ч??
   protocolInfo	O	String(64)	?ļ???MIME???͡????磺jpgͼƬ??MIMEΪ image/jpeg??ϸ???????ο? RFC 2046????????ͨ?ļ???Ч??????????ΪĿ¼??ר??????ǩʱ???ֶ???Ч??
   colorDepth	O	Int	ɫ??λ??????λbit??????ͼƬ????Ƶ?ļ???Ч??
   contentSource	M	Int(1)	??Ŀ4Դ??
   0??IPTV??Ŀ??
   1??DVB??Ŀ??
   2??DMS??????Դ??
   channelNO	O	String(3)	Ƶ?:š?????????ΪIPTV??Ŀʱ??Ч??
   iptvContentID	O	String(128)	IPTVϵͳ?е?????ID??????IPTV downloadҵ?????ص??ļ???Ч??
   iptvUserID	O	String(32)	??ȡ?????ݵ?IPTV?ʺš?????????ΪIPTV??Ŀʱ??Ч??
   contentType	O	Int(1)	IPTV???????ͣ?????????ΪIPTV??Ŀʱ??Ч??
   0??PVR??Ŀ??
   1?????ؽ?Ŀ??
   */
static void s_HuaweiV1R5_ComposeItemList_sub(pClassContentDirectory me, IXML_Node *node, const char *classID, json_object *new_object)
{
	IXML_Element* const elem = (IXML_Element*)node;
	const char *aim = NULL, *str = NULL;

	aim = "id";
	str = ixmlElement_getAttribute(elem, aim);
	x_IF_STR(str)
		json_object_object_add(new_object, "objectID",		Dlna_json_object_new_string( str ));
    //printf("%s %d %s\n",__FILE__,__LINE__,str);
	aim = "dc:title";
	str = XMLUtil_FindFirstElementValue (node, aim,false, true);
	if (str)
	{
		coo_str_rm_ext(str);
		json_object_object_add(new_object, "filename",		Dlna_json_object_new_string( str ));
	}
	    //printf("%s %d %s\n",__FILE__,__LINE__,str);

	/*
	[{"classID":"3","itemID": "3-2-1","filename":"rrtc.mpg"
	*/
//	if(classID)
//        str = classID;
//    else
    {
        aim = "upnp:class";
        str = XMLUtil_FindFirstElementValue (node, aim,false, true);
	 x_IF_STR(str)
        str = s_HuaweiV1R5_GuessItemType(str);
    }
	x_IF_STR(str)
	json_object_object_add(new_object, "classID",		Dlna_json_object_new_string( str ));
    //printf("%s %d %s\n",__FILE__,__LINE__,str);

	/*
	["size":"5688754","bitrate":"512"," resolution":"720x576"," duration":"3600"," audiochannel":"2","protocolInfo":"video/mpeg","colorDepth":24,
	*/
	char *protocolInfo = NULL, *url=NULL;
	t_MEDIA_INFO mInfo;
	int idx = -1;
	IXML_Element *res = NULL;

	IXML_NodeList* const reslist = ixmlElement_getElementsByTagName ((IXML_Element*)elem, "res");
	int nb_reslist = ixmlNodeList_length (reslist);
	if( nb_reslist > 0 )
		idx = YX_CD_GetPreferred(me, reslist, nb_reslist, &protocolInfo, &url, &mInfo);

	if( idx >= 0 )
		res = (IXML_Element*)ixmlNodeList_item (reslist, idx);
	//			IXML_Element*element = (IXML_Element*)res;
	//if (res){
			//printf("111111111\n");
	//}
	if (res){
		aim = "size";
		str = ixmlElement_getAttribute (res, aim);
		x_IF_STR(str)
		json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));
	    //printf("%s %d %s\n",__FILE__,__LINE__,str);

  aim = "bitrate";
  str = ixmlElement_getAttribute (res, aim);
  x_IF_STR(str)
    json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

  aim = "resolution";
  str = ixmlElement_getAttribute (res, aim);
  x_IF_STR(str)
    json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

  aim = "duration";
  str = ixmlElement_getAttribute (res, aim);
  if(str)
  {
    unsigned int hh=0, mm = 0, ss = 0, duration =0;
    if (sscanf (str, "%u:%u:%u", &hh, &mm, &ss) == 3 )
    {
      duration = ss + 60*(mm + 60*hh);
      json_object_object_add(new_object, aim, json_object_new_int( duration ));
    }
  }

  aim = "colorDepth";
  str = ixmlElement_getAttribute (res, aim);
  x_IF_STR(str)
    json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

  char *transport, *mimetype, *dlna;
  YX_CD_SplitInfo(protocolInfo, &transport, &mimetype, &dlna);
  str = mimetype;
  json_object_object_add(new_object, "protocolInfo",		Dlna_json_object_new_string( str ));

  /*
     "filepath":"http://10.10.10.8/video/rrtc.mpg",
     " iptvcontentID":"20100803001", " contentSource":"1","iptvuserID":"iptvuser20100898","contentType":"10"," programTitle":"ӣ??С????48"},
     */
  str = url;
  json_object_object_add(new_object, "filepath",	Dlna_json_object_new_string( str ));

  aim = "contentSource";
  str = ixmlElement_getAttribute (res, aim);
  x_IF_STR(str)
    json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

  aim = "channelNO";
  str = ixmlElement_getAttribute (res, aim);
  x_IF_STR(str)
    json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

  aim = "iptvcontentID";
  str = ixmlElement_getAttribute (res, aim);
  x_IF_STR(str)
    json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

  aim = "iptvuserID";
  str = ixmlElement_getAttribute (res, aim);
  x_IF_STR(str)
    json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

		aim = "contentType";
		str = ixmlElement_getAttribute (res, aim);
		x_IF_STR(str)
		json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));
		    //printf("%s %d %s\n",__FILE__,__LINE__,str);
	}
/*
	aim = "programTitle";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

     aim = "programRate";
     str = ixmlElement_getAttribute (res, aim);
     x_IF_STR(str)
     json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));
     */
  if( protocolInfo ) free(protocolInfo);
  if( url ) free(url);
  if (reslist)
    ixmlNodeList_free (reslist);
}

static void s_HuaweiV1R5_ComposeList(pClassContentDirectory me, const char *classID, int sort, char *value, int bufferLength)
{
  json_object *my_array, *new_object;
  int jlen, jsonLength = 0;

  bufferLength -= 100;
  HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, bufferLength, 0);

  IXML_NodeList* containers = ixmlDocument_getElementsByTagName(me->subdoc, "container");
  int nb_containers = ixmlNodeList_length (containers);
  HT_DBG_FUNC(nb_containers, "nb_containers = ");

  IXML_NodeList* items =	ixmlDocument_getElementsByTagName (me->subdoc, "item");
  int nb_items = ixmlNodeList_length (items);
  HT_DBG_FUNC(ixmlNodeList_length (items), "nb_items = ");
  nb_items = 0;

  if(me->nb_returned > 0)
  {
    int index = me->index;
    my_array = json_object_new_array();
    for(; me->index < me->nb_returned; me->index++)
    {
      int i = me->index;
      bool const is_container = (i < nb_containers);
      IXML_Node* const node = ixmlNodeList_item(is_container ? containers : items,  is_container ? i : i - nb_containers);

      HT_DBG_FUNC(is_container, "is_container = ");
      new_object = json_object_new_object();
      if(is_container)
        s_HuaweiV1R5_ComposeContainerList_sub(node, new_object);
      else
        s_HuaweiV1R5_ComposeItemList_sub(me, node, classID, new_object);

      jlen = strlen(json_object_to_json_string(new_object));
      if( jsonLength + jlen > bufferLength )
      {
        json_object_put(new_object);
        break;
      }

      jsonLength += jlen;
      json_object_array_add(my_array, new_object);
    }

    s_HW_V1R5_Browse_FillResult(my_array, me->index-index, value, bufferLength);
  }
  else
  {
    s_HW_V1R5_Browse_FillResult(NULL, 0, value, bufferLength);
  }

  if (containers)
    ixmlNodeList_free (containers);
  if (items)
    ixmlNodeList_free (items);

  HT_DBG_FUNC_END(0, value);
}

#if 0

#endif

static char *s_HuaweiV1R5_RootIdString(char *id)
{
  if( id && (atoi(id)==-1) )
    return "0";
  else
    return id;
}
#if 0
static char* s_HuaweiV1R5_FindCommaAndInc(char *para)
{
  return Dlna_FindCharAndInc(para, ',');
}
#endif
static int s_HuaweiV1R5_ParseJs(const char *func, const char *para, char *value, int len)
{
	int ret = 0;
    int x1, x2, x3, x4;
    char *p1, *p2, *p3;
    json_object *json;
    json_object *j1, *j2, *j3, *j4, *j5;
/*
    p1 = Dlna_FindCharAndInc(func, '\'');
    if(!p1)
        return -1;

    p2 = strrchr(p1, '\'');
    if(!p2)
        return -1;

    *p2 = 0;
    func = (const char *)p1;
    */
//	len = 4096;
    x3 = 0;

/*
var sValue= Utility.getValueByName('dlna.getDmsCount')
*/
	if( strcmp(func, "dlna.getDmsCount") == 0 )
	{
		pClassDmsList me = s_huawei_dms_list;
		ret = me->GetDmsNumber(me);
		sprintf(value, "%d",ret);
		return 0;
	}
/*
var sJson = Utility.getValueByName('dlna.getDmsList,{"index":index,"count":count}')
*/
	else if( strcmp(func, "dlna.getDmsList") == 0 )
	{
		/*
		p1 = s_HuaweiV1R5_FindCommaAndInc(para);
        if( !p1 )
            return -1;

        json = dlna_json_tokener_parse(p1);
        */
        json = dlna_json_tokener_parse(para);

        if(!json)
            return -2;

    j1 = json_object_object_get(json, "index");
    j2 = json_object_object_get(json, "count");
    if(j1 && j2)
    {
      x1 = json_object_get_int(j1);
      x2 = json_object_get_int(j2);
      s_HuaweiV1R5_GetDmsLists(x1, x2, value, len);
    }

    json_object_put(json);
    return 0;
  }
  /*
     var sValue = Utility.getValueByName('dlna.openFileList,{"deviceID":"deviceID","containerID":"containerID","sort": sort}')
     var sValue = Utility.getValueByName('dlna.openFileList,{"deviceID":"deviceID","classID":classID,"sort":sort}')
     */
  else if( strcmp(func, "dlna.openFileList") == 0 )
  {/*
      p1 = s_HuaweiV1R5_FindCommaAndInc(para);
      if( !p1 )
      return -1;
      */
    json = dlna_json_tokener_parse(para);
    if(!json)
      return -2;

        j1 = json_object_object_get(json, "deviceID");
        j2 = json_object_object_get(json, "containerID");
        j3 = json_object_object_get(json, "classID");
        j4 = json_object_object_get(json, "sort");
        j5 = json_object_object_get(json, "order");
		/*hnd->SetDmsUdn				= s_CD_SetDmsUdn;
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
		hnd->SetObjectID			= s_CD_SetObjectID;*/
        if(j1 && j2 && !j3 && j4) // browse
        {

            p1 = (char*)json_object_get_string(j1);
            p2 = (char*)json_object_get_string(j2);
            x4 = json_object_get_int(j4);

            if(p1 && p2)
            {
                pClassContentDirectory me = s_huawei_browse;
                me->SetDmsUdn(me, p1);
                me->SetObjectID(me, s_HuaweiV1R5_RootIdString(p2));
                s_sort_flag = x4;

		 HT_DBG_PRINTF(HT_MOD_DMC, HT_BIT_KEY, "s_sort_flag = %d\n", s_sort_flag);
		  if(j5)
		 	me->SetSortCriteria(me,s_sort_flag,json_object_get_int(j5),NULL);
		  else
		  	me->SetSortCriteria(me,s_sort_flag,0,NULL);

                sprintf(value, "%d", (int)me);
            }
        }
        else if( j1 && !j2 && j3 && j4 ) //search
        {

            p1 = (char*)json_object_get_string(j1);
            p3 = (char*)json_object_get_string(j3);
		//HT_DBG_PRINTF(HT_MOD_DMC, HT_BIT_MANY, "classid = %s\n",j3);
    		//HT_DBG_PRINTF(HT_MOD_DMC, HT_BIT_MANY, "classid  p3= %s\n",p3);

            x4 = json_object_get_int(j4);
            if(p1 && p3)
            {
                char *criteria = s_HuaweiV1R5_ParseClassID(p3);
                pClassContentDirectory me = s_huawei_search;
                me->SetDmsUdn(me, p1);
                me->SetObjectID(me, "0");
                me->SetSearchCriteria(me, criteria);
		  s_sort_flag = x4;
		   if(j5)
		 	me->SetSortCriteria(me,s_sort_flag,json_object_get_int(j5),NULL);
		  else
		  	me->SetSortCriteria(me,s_sort_flag,0,NULL);

                sprintf(value, "%d", (int)me);
            }
        }
        else
        {
        }

        json_object_put(json);
		return 0;
	}
/*
var sValue = Utility.getValueByName('dlna.getCount,{"listID":listID}')
*/
	else if( strcmp(func, "dlna.getCount") == 0 )
	{/*
		p1 = s_HuaweiV1R5_FindCommaAndInc(para);
        if( !p1 )
            return -1;
        */
        json = dlna_json_tokener_parse(para);
        if(!json)
            return -2;

    j1 = json_object_object_get(json, "listID");
    if(j1 && (x1 = json_object_get_int(j1)))
    {
      pClassContentDirectory me = (pClassContentDirectory)x1;
      if(me==s_huawei_browse)
      {
        me->SyncBrowseChildren(me, me->objectid, 0, 1);
        ret = me->nb_matched;
        sprintf(value, "%d",ret);
      }
      else if(me==s_huawei_search)
      {
        me->SyncSearch(me, me->objectid, 0, 1);
        ret = me->nb_matched;
        sprintf(value, "%d",ret);
      }
      else
      {
      }
    }

    json_object_put(json);
    return 0;
  }
  /*
     var sJson = Utility.getValueByName('dlna.getList,{"listID":listID,"index":index,"count":count}')
     */
  else if( strcmp(func, "dlna.getList") == 0 )
  {/*
      p1 = s_HuaweiV1R5_FindCommaAndInc(para);
      if( !p1 )
      return -1;
      */
    json = dlna_json_tokener_parse(para);
    if(!json)
      return -2;

    j1 = json_object_object_get(json, "listID");
    j2 = json_object_object_get(json, "index");
    j3 = json_object_object_get(json, "count");
    if(j1 && j2 && j3 && (x1 = json_object_get_int(j1)))
    {
      pClassContentDirectory me = (pClassContentDirectory)x1;
      x2 = json_object_get_int(j2);
      x3 = json_object_get_int(j3);
      if(me==s_huawei_browse)
      {
        me->SyncBrowseChildren(me, me->objectid, x2, x3);
        s_HuaweiV1R5_ComposeList(me, para, s_sort_flag, value, len);
      }
      else if(me==s_huawei_search)
      {
        me->SyncSearch(me, me->objectid, x2, x3);
        s_HuaweiV1R5_ComposeList(me, para, s_sort_flag, value, len);
      }
      else
      {
      }
    }

    json_object_put(json);
    return 0;
  }
  else
  {
  }

  return -1;
}

int Raw_Dmc_HuaweiJse_V1R5(const char *func, const char *para, char *value, int len)
{
  HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_KEY,len, func);
  HT_DBG_FUNC(len, para);
  HT_DBG_FUNC(len, value);

  int ret = s_HuaweiV1R5_ParseJs(func, para, value, len);

  HT_DBG_FUNC_END(ret, value);
  return ret;
}

void HW_V1R5_Dmscp_Init(enum_DlnaAppMode mode)
{
  if(s_huawei_event_queue == NULL)
    s_huawei_event_queue = EventQueue_Create();

  //	Dlna_Huawei_OnSearchInitiated();

  if(s_huawei_dms_list == NULL)
    s_huawei_dms_list = (pClassDmsList)YX_DmsList_Create(s_HuaweiV1R5_OnDmsChanged, (int)HUAWEI_OBJECT_HND, NULL);
  if(s_huawei_browse == NULL)
    s_huawei_browse = (pClassContentDirectory)YX_CD_Create(NULL, NULL, 0, s_HuaweiV1R5_OnServiceChanged);
  if(s_huawei_search == NULL)
    s_huawei_search = (pClassContentDirectory)YX_CD_Create(NULL, NULL, 0, s_HuaweiV1R5_OnServiceChanged);
}


