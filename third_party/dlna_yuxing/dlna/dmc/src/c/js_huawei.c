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
#include "dms.h"

//#define USE_SEARCH_METHOD	1
//#define USE_HYBIRD_GET                1
#define HUAWEI_OBJECT_HND	(void*)1
#define x_IF_STR(s)	        if(s)
#define EVENT_DLNA_DMS_CHANGED "EVENT_DLNA_DMSLIST_CHANGED"
#define x_STR_LEN(str) 		(sizeof(str)-1)

typedef enum
{
    x_DTB_UNKOWN=1,
    x_DTB_FOLDER=2,
    x_DTB_VIDEO=4,
    x_DTB_AUDIO=8,
    x_DTB_IMAGE=16,
    
	x_DTB_PVR=32,
	x_DTB_VOD=64,
}x_DTB;
#define x_DLNA_TYPE_ALL (x_DTB_UNKOWN|x_DTB_FOLDER|x_DTB_VIDEO|x_DTB_AUDIO|x_DTB_IMAGE|x_DTB_PVR|x_DTB_VOD)

typedef struct _hw_browse_cache_array_
{
	x_DTB				*array;
    int                 num;
    
    char                *udn;
    char                *item_id;
	int					seconds;
}x_HBCA;

static x_HBCA s_hbca = {0};

static enum_DlnaAppMode s_js_mode;

static pClassDmsList			s_huawei_dms_list = NULL;
static pClassContentDirectory	s_huawei_browser = NULL;
static pClassEventQueue 		s_huawei_event_queue= NULL;

static char *EVENT_TYPE = "EVENT_DLNA_DIAGNOSTIC_RESULT";
static int s_huawei_trigger_time = 0;
static int s_hw_get_device_list_mode = 0;
static char *s_hw_languageCode = NULL;

static void g_Huawei_FillRes_ByLanguageCode(json_object *new_object, const char *aim, char *m_val, char *languageCode);
static void s_Huawei_ComposeItemList_sub(pClassContentDirectory me, IXML_Node *node, char *classID, json_object *new_object, x_DTB dlna_type, char *languageCode);
static x_DTB s_HW_Browse_GetDlnaType(IXML_Node* const node, int stb_dms);

static void s_Huawei_EventQueue_Put(const char *string)
{
	if( s_huawei_event_queue )
		s_huawei_event_queue->Put(s_huawei_event_queue, HUAWEI_OBJECT_HND, (char*)string);
}

static int s_Is_Trigger_Mode(void)
{
	struct timeval time_x = {0,0};
	gettimeofday( &time_x, NULL );

    if( time_x.tv_sec>=s_huawei_trigger_time && ((time_x.tv_sec-s_huawei_trigger_time)<10) )
        return 1;
    else
        return 0;
}
static void s_Huawei_Trigger_TImer(void)
{
	struct timeval time_x = {0,0};
	gettimeofday( &time_x, NULL );
    s_huawei_trigger_time = time_x.tv_sec;
}

static void s_Dmc_Huawei_RecodTime()
{
	struct timeval time_x = {0,0};
	gettimeofday( &time_x, NULL );
    s_hbca.seconds = time_x.tv_sec;	
}
static int s_Dmc_Huawei_IsInSec(int sec)
{
	struct timeval time_x = {0,0};
	gettimeofday( &time_x, NULL );

    if( time_x.tv_sec>=s_hbca.seconds && ((time_x.tv_sec-s_hbca.seconds)<sec) )
        return 1;
    else
        return 0;
}

static int s_Is_HuaweiStbDms(const char *udn)
{
	char demo[]   = "uuid:898f9738-d930-4db4-a3cf-0007679bed36";
	char hw_udn[] = "uuid:898f9738-d930-4db4-a3cf-000767";

	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, 0, udn);
	if(strlen(udn) != x_STR_LEN(demo))
		return 0;
	if(strncmp(udn, hw_udn, x_STR_LEN(hw_udn)))
		return 0;
	
//	if(/*other watermark */)
	HT_DBG_FUNC_END(1, 0);
	return 1;
}

static void s_Huawei_OnDmsUpdated_Old(char *dmsUdn, Device *d, char *alias, int action, int cookie)
{
//	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,action, alias);
	if( action != 1 )
		return;
	
	json_object *json = json_object_new_object();
	json_object_object_add(json, "type", Dlna_json_object_new_string(EVENT_TYPE));
	json_object_object_add(json, "status_code", Dlna_json_object_new_string("0"));
	json_object_object_add(json, "stage_code", Dlna_json_object_new_string("DLNA_SEARCH_RESPONSE"));
	
	json_object *my_array = json_object_new_array();
	{
		const char *str;
		
		json_object *new_dms = json_object_new_object();

		str = d->friendlyName;
		json_object_object_add(new_dms, "friendlyName",Dlna_json_object_new_string( str ));

		str = Device_GetDescDocItem(d,  "deviceType", false);
		json_object_object_add(new_dms, "deviceType", Dlna_json_object_new_string( "urn:schemas-upnp-org:device:MediaServer" ));

		str = Device_GetDescDocItem(d,  "manufacturer", false);
		json_object_object_add(new_dms, "manufacturer", Dlna_json_object_new_string( str ));

		str = Device_GetDescDocItem(d,  "modelName", false);
		json_object_object_add(new_dms, "modelName",	Dlna_json_object_new_string( str ));
		
		json_object_array_add(my_array, new_dms);
	}
	json_object_object_add(json, "event_description", my_array);
    
	s_Huawei_EventQueue_Put(json_object_to_json_string(json) );
	json_object_put(json);
//	HT_DBG_FUNC_END(0, 0);
}
static void s_Huawei_OnDmsChanged(char *dmsUdn, Device *d, char *alias, int action, int cookie)
{
    const char *str;
	json_object *new_dms = json_object_new_object();
    if(!new_dms)
        return;
    
	json_object_object_add(new_dms, "type", Dlna_json_object_new_string(EVENT_DLNA_DMS_CHANGED));
    json_object_object_add(new_dms, "deviceID", Dlna_json_object_new_string(d->udn));
	json_object_object_add(new_dms, "status_code", json_object_new_int(action));

	str = d->friendlyName;
	json_object_object_add(new_dms, "friendlyName",Dlna_json_object_new_string( str ));

	str = Device_GetDescDocItem(d,  "deviceType", false);
	json_object_object_add(new_dms, "deviceType", Dlna_json_object_new_string( "urn:schemas-upnp-org:device:MediaServer" ));

	str = Device_GetDescDocItem(d,  "manufacturer", false);
	json_object_object_add(new_dms, "manufacturer", Dlna_json_object_new_string( str ));

	str = Device_GetDescDocItem(d,  "modelName", false);
	json_object_object_add(new_dms, "modelName",	Dlna_json_object_new_string( str ));
		
	s_Huawei_EventQueue_Put(json_object_to_json_string(new_dms) );
	json_object_put(new_dms);        
}
static void s_Huawei_OnDmsUpdated(char *dmsUdn, Device *d, char *alias, int action, int cookie)
{
    if( action == -1 )
    {
        s_Huawei_OnDmsChanged(dmsUdn, d, alias, action, cookie);
        return;
    }
    
    if(s_Is_Trigger_Mode())
        s_Huawei_OnDmsUpdated_Old(dmsUdn, d, alias, action, cookie);
    else
        s_Huawei_OnDmsChanged(dmsUdn, d, alias, action, cookie);
}
void Dlna_Huawei_OnIpChanged(char *netcardName, char *newIp)
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW,0, newIp);
	json_object *json = json_object_new_object();
	
	json_object_object_add(json, "type", Dlna_json_object_new_string(EVENT_TYPE));
	json_object_object_add(json, "status_code", Dlna_json_object_new_string("0"));
	json_object_object_add(json, "stage_code", Dlna_json_object_new_string("DLNA_IP_CHECKED"));

	json_object_object_add(json, "event_description", Dlna_json_object_new_string(newIp));

	s_Huawei_EventQueue_Put(json_object_to_json_string(json) );
	json_object_put(json);
	HT_DBG_FUNC_END(0, 0);
}

void Dlna_Huawei_OnSearchInitiated(void)
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW,0, NULL);
	json_object *json = json_object_new_object();
	
	json_object_object_add(json, "type", Dlna_json_object_new_string(EVENT_TYPE));
	json_object_object_add(json, "status_code", Dlna_json_object_new_string("0"));
	json_object_object_add(json, "stage_code", Dlna_json_object_new_string("DLNA_SEARCH_INITIATED"));

	s_Huawei_EventQueue_Put(json_object_to_json_string(json) );
	json_object_put(json);

    s_Huawei_Trigger_TImer();
    
	HT_DBG_FUNC_END(s_huawei_trigger_time, 0);
}

static json_object* s_Huawei_AddDmsInfo(ListNode* node)	//512*50 Bytes
{	
    DeviceNode* devnode = node->item;
    Device *d = devnode->d;
    const char *str;
    json_object *new_dms = json_object_new_object();
    if( new_dms )
    {
        str = d->udn;
        json_object_object_add(new_dms, "deviceID",     Dlna_json_object_new_string( str ));
        
        str = talloc_get_name (d);
        json_object_object_add(new_dms, "alias",        Dlna_json_object_new_string( str ));
        
        str = Device_GetDescDocItem(d,  "manufacturer", false);
        json_object_object_add(new_dms, "manufacturer", Dlna_json_object_new_string( str ));
        
        str = Device_GetDescDocItem(d,  "modelName", false);
        json_object_object_add(new_dms, "deviceModel",  Dlna_json_object_new_string( str ));
        
        str = Device_GetDescDocItem(d,  "serialNumber", false);
        json_object_object_add(new_dms, "sn",           Dlna_json_object_new_string( str ));
    }
    return new_dms;
}
static void s_Huawei_GetDmsLists( int start, int count, char *value, int bufferLength)	//512*50 Bytes
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
            new_dms = s_Huawei_AddDmsInfo(node);
            
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
	HT_DBG_FUNC_END(0, value);
}

static int s_Is_HuaweiStbDmsBy(DeviceNode* devnode)	//512*50 Bytes
{	
	Device *d = devnode->d;
	int ret = s_Is_HuaweiStbDms(d->udn);
	return ret;
}
static int s_Huawei_GetDmsCount_STB(void)	//512*50 Bytes
{	
	ListNode* node;
	int i = 0;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, 0, 0);

	DeviceList_Lock();
	node = ListHead (&GlobalDeviceList);
	while(node)
	{
		DeviceNode* devnode = node->item;
		if(s_Is_HuaweiStbDmsBy(devnode))
			i++;
		node = ListNext (&GlobalDeviceList, node);
	}
	DeviceList_Unlock();
	
	HT_DBG_FUNC_END(i, 0);
	return i;
}
static void s_Huawei_GetDmsLists_STB(int start, int count, char *value, int bufferLength)	//512*50 Bytes
{	
	ListNode* node;
	int real = 0, i = -1;
	
	json_object *json;
	json_object *my_array = NULL, *new_dms;
	int jlen; 
	int jsonLength = 0;
	
	bufferLength -= 100;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY,start, 0);

    /* start=0: 0 is first; count=0: 0 means all */
	if( start < 0 || count < 0 )
		return;
	
	DeviceList_Lock();

	node = ListHead (&GlobalDeviceList);
	while(node)
	{
		DeviceNode* devnode = node->item;
		if(s_Is_HuaweiStbDmsBy(devnode))
			i++;
		
		if(i >= start)
			break;
		
		node = ListNext (&GlobalDeviceList, node);
	}

	my_array = json_object_new_array();
	while(my_array && node && (!count || real<count))
	{
		DeviceNode* devnode = node->item;
		if(s_Is_HuaweiStbDmsBy(devnode))
		{
	        new_dms = s_Huawei_AddDmsInfo(node);
	        
			jlen = strlen(json_object_to_json_string(new_dms));
			if( jsonLength + jlen > bufferLength )
			{
	            json_object_put(new_dms);
				break;
			}
			
			jsonLength += jlen;
			json_object_array_add(my_array, new_dms);
			real++;
		}
		node = ListNext (&GlobalDeviceList, node);
	}

	DeviceList_Unlock();
	
	if(!real && my_array)
		json_object_put(my_array);
	
	json = Dlna_CreateJson();
	if(real)
	{
		json_object_object_add(json, "device_count", json_object_new_int(real));
		json_object_object_add(json, "devices", my_array);
	}
	else
		json_object_object_add(json, "device_count", json_object_new_int(0));
	
	strcpy(value, json_object_to_json_string(json));
	json_object_put(json);
	HT_DBG_FUNC_END(0, value);
}



/*
ItemNumber_Get::deviceID,classID

以JSON封装返回描述信息：查找classID为11的返回值。
{"classID":11,"items_num":"56","deviceID":" deviceUDN"}
deviceID：设备的维一标识，
classID：内容的内型或专辑类型。这两个参数不带时，表示返回所有。
classID定义如下：
classID=1：image，单个的图片类型
classID=12：有专辑性性的单个图片类型
classID=11：photoAlbum，图片专辑
classID=2：audio，单个的音乐类型
classID=22：musicAlbum，音乐专辑
classID=3：video，单个视频类型
classID=4：bookmark，播放创建的书签
classID=3301：pvr内容
classID=3302：VOD下载内容

*/

static char* s_Huawei_ParseClassID(char *para)
{
	char *criteria = "*";

	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, 0, para);
	char *p1 = strchr(para, ',');	
	if( p1 )	
	{
		p1++;
		if( strncmp(p1, "1", 1) == 0 )
		{
			criteria = "upnp:class derivedfrom \"object.item.imageItem\"";
		}
		else if(strncmp(p1, "12", 2) == 0)
		{
			criteria = "upnp:class derivedfrom \"object.item.imageItem\"";
		}
		else if(strncmp(p1, "11", 2) == 0)
		{
			criteria = "upnp:class derivedfrom \"object.container.album.photoAlbum\"";
		}
		else if(strncmp(p1, "2", 1) == 0)
		{
			criteria = "upnp:class derivedfrom \"object.item.audioItem\"";
		}
		else if(strncmp(p1, "22", 2) == 0)
		{
			criteria = "upnp:class derivedfrom \"object.container.album.musicAlbum\"";
		}
		else if(!strncmp(p1, "3", 1) || !strncmp(p1, "3301", 4) || !strncmp(p1, "3302", 4))
		{
			criteria = "upnp:class derivedfrom \"object.item.videoItem\"";
		}
		else if(strncmp(p1, "4", 1) == 0)
		{
		}
		else if(strncmp(p1, "100", 3) == 0) // not huawei's definition, temporarily added for self test
		{
			criteria = "upnp:class derivedfrom \"object.container\"";
		}
		else
		{
		}
	}
	HT_DBG_FUNC(0, criteria);
	return criteria;
}

static void s_HW_Browse_FillResult(json_object *my_array, int num, char *value, int bufferLength)
{
    json_object *json = json_object_new_object();
    if( json )
    {
        if(my_array)
        {
            json_object_object_add(json, "return_num", json_object_new_int(num));
            json_object_object_add(json, "items", my_array);
        }
        else
            json_object_object_add(json, "return_num", json_object_new_int(0));
        
        strcpy(value, json_object_to_json_string(json));
        json_object_put(json);
    }
}

/*
{"container_num":"2","containers":"[{"containerID": "0-1-1","parentID":"0","title":"music" },{"containerID": "0-1-2","parentID":"0","title":"movies"}]"}
*/
static void s_Huawei_ComposeContainerList_sub(IXML_Node *node, json_object *new_object, int is_hybird, char *languageCode)
{
	IXML_Element* const elem = (IXML_Element*)node;
	const char *aim, *str;
	
	aim = "id";
	str = ixmlElement_getAttribute(elem, aim);
	json_object_object_add(new_object, is_hybird?"itemID":"containerID", Dlna_json_object_new_string( str ));
	
	aim = "parentID";
	str = ixmlElement_getAttribute(elem, aim);
	json_object_object_add(new_object, aim, 		Dlna_json_object_new_string( str ));
#if 0
	aim = "dc:title";
	str = XMLUtil_FindFirstElementValue (node, aim,false, true);
	json_object_object_add(new_object, "title", 	Dlna_json_object_new_string( str ));
#else
	aim = "dc:title";
	str = XMLUtil_FindFirstElementValue (node, aim,false, true);
	char *buf = malloc(strlen(str) + 2);
	buf[0] = 0;
	coo_str_from_xml((char*)str, buf);

	aim = "title";
	if(languageCode)
		g_Huawei_FillRes_ByLanguageCode(new_object, aim, buf, languageCode);
	else
		json_object_object_add(new_object, aim, Dlna_json_object_new_string( buf ));

	free(buf);
#endif	
}

/*
{"conflict返回JSON封装的数组列表，样例如下
{"return_num":"2","items":"
[{"classID":"3","itemID": "3-2-1","filename":"rrtc.mpg","size":"5688754","bitrate":"512"," resolution":"720x576"," duration":"3600"," audiochannel":"2","protocolInfo":"video/mpeg","colorDepth":24,"filepath":"http://10.10.10.8/video/rrtc.mpg"," iptvcontentID":"20100803001", " contentSource":"1","iptvuserID":"iptvuser20100898","contentType":"10"," programTitle":"樱桃小丸子48"},

{"classID":"3","itemID": "3-2-2","filename":"rrtc1.mpg","size":"5688754","bitrate":"512"," resolution":"720x576"," duration":"3600"," audiochannel":"2","protocolInfo":" video/mpeg", "colorDepth":24,"filepath":"http://10.10.10.8/video/rrtc1.mpg" "iptvcontentID":"20100803001", " contentSource":"1", iptvuserID":"iptvuser20100898","contentType":"10", " programTitle":"樱桃小丸子48"}]"}
参数定义：
itemID：内容ID
filename：文件名
size：文件大小，以KB为单位
bitrate：比特率，仅对音乐和视频类型有效
resolution：视频分辨率
duration：视频长度，单位为秒。
audiochannel：声道数
protocolInfo：文件的MIME类型
colorDepth：色彩位数，单为bit。
filepath：文件路径，只有图片是必选。音乐或视频可以不返回。
iptvcontentID：IPTV系统中的内容ID，此信息由PVR或download时传入。
contentSource：该媒体项目的来源
iptvuserID：获取该内容的IPTV账号，此信息在创建PVR或download内容时创建
contentType：IPTV内容类型
*/
static const char* s_Huawei_GuessItemType(const char*upnp_class)
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
/*
image/bmp
image/jpeg
image/png

audio/L16
audio/mpeg
audio/x-wav
audio/x-ms-wma

video/mpeg
video/mpeg
video/vnd.dlna.mpeg-tts
video/mp4
video/mpeg
video/x-ms-asf
video/vnd.dlna.mpeg-tts
video/mp4
video/mpeg
video/vnd.dlna.mpeg-tts
video/x-ms-wmv
video/x-ms-asf
video/mpeg
video/vnd.dlna.mpeg-tts
video/x-msvideo


JPEG_*
PNG_*

LPCM,LPCM_*
MP3,MP3X
WMABASE,WMAFULL,WMAPRO

MPEG1
MPEG_*
MPEG4_*
AVC_*
WMV*
VC1_*
*/
static int s_Huawei_IsMediaSupported(t_MEDIA_INFO *minfo, char *mimetype, char *dlna_pn)
{
	char *fileformat = NULL, *p = mimetype;
	int ret = 0, type = minfo->majorType;

	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MYRIAD, type, dlna_pn);
	if(dlna_pn)
		fileformat = dlna_pn + CONST_STR_LEN("DLNA.ORG_PN=");
	HT_DBG_FUNC(0, fileformat);
	
	if( 1 == type )/* video */
	{
		p = p + CONST_STR_LEN("video/");
		if(fileformat)
		{
			if( !strncmp(fileformat, "MPEG1", CONST_STR_LEN("MPEG1")) )/* suppoted non-conditional */
			{
				ret = 1;
			}
			else if( !strncmp(fileformat, "MPEG_", CONST_STR_LEN("MPEG_")) )/* suppoted non-conditional */
			{
				ret = 1;
			}
			else if( !strncmp(fileformat, "MPEG4_", CONST_STR_LEN("MPEG4_")) )
			{
				if( !strcmp(p, "mp4") || !strcmp(p, "mpeg") || !strcmp(p, "x-ms-asf") || !strcmp(p, "vnd.dlna.mpeg-tts"))
					ret = 1;
			}
			else if( !strncmp(fileformat, "AVC_", CONST_STR_LEN("AVC_")) )
			{
				if( !strcmp(p, "mp4") || !strcmp(p, "mpeg") || !strcmp(p, "vnd.dlna.mpeg-tts"))
					ret = 1;
			}
			else if( !strncmp(fileformat, "WMV", CONST_STR_LEN("WMV")) )
			{
				if( !strcmp(p, "x-ms-wmv") )
					ret = 1;
			}
			else if( !strncmp(fileformat, "VC1_", CONST_STR_LEN("VC1_")) )
			{
				if( !strcmp(p, "x-ms-asf") || !strcmp(p, "mpeg") || !strcmp(p, "vnd.dlna.mpeg-tts"))
					ret = 1;
			}
			else
			{
			}
		}
		else
		{
			if( !strcmp(p, "x-msvideo") )
				ret = 1;
			else if(!strcmp(p, "mpeg") || !strcmp(p, "x-ms-wmv") || !strcmp(p, "vnd.dlna.mpeg-tts") )
				ret = -1;
			else if( !strcmp(p, "quicktime") )
				ret = 0;
			else
				ret = -1;
		}
	}
	else if( 2 == type )/* audio */
	{
		p = p + CONST_STR_LEN("audio/");
		
		if(fileformat)
		{
			if( !strcmp(p, "L16") )
			{
				ret = 1;
			}
			else if( !strcmp(p, "mpeg") )
			{
				ret = 1;
			}
			else if( !strcmp(p, "x-ms-wma") )
			{
				if( !strcmp(fileformat, "WMABASE") || !strcmp(fileformat, "WMAFULL") || !strcmp(fileformat, "WMAPRO"))
					ret = 1;
			}
			else
			{
			}
		}
		else
		{
			if( !strcmp(p, "x-wav") || !strcmp(p, "mpeg" ) )
				ret = 1;
			else if( !strcmp(p, "x-ms-wma") )
				ret = -1;
			else
				ret = 0;
		}
	}
	else if( 3 == type )/* image */
	{
		p = p + CONST_STR_LEN("image/");
		if(!strcmp(p, "bmp") || !strcmp(p, "jpeg") || !strcmp(p, "png"))
			ret = 1;
	}
	else
	{
	}
	HT_DBG_FUNC_END(ret, mimetype);
	return ret;
}
#if 0
static void s_Huawei_ComposeItemList_sub(pClassContentDirectory me, IXML_Node *node, char *classID, json_object *new_object)
{
	IXML_Element* const elem = (IXML_Element*)node;
	const char *aim, *str;

	/*
	[{"classID":"3","itemID": "3-2-1","filename":"rrtc.mpg"
	*/
	if(classID)
        str = classID;
    else
    {
        aim = "upnp:class";
        str = XMLUtil_FindFirstElementValue (node, aim,false, true); 
        str = s_Huawei_GuessItemType(str);
    }
	json_object_object_add(new_object, "classID",		Dlna_json_object_new_string( str ));

	aim = "id";
	str = ixmlElement_getAttribute(elem, aim);
	json_object_object_add(new_object, "itemID",		Dlna_json_object_new_string( str ));

	aim = "dc:title";
	str = XMLUtil_FindFirstElementValue (node, aim,false, true);
	coo_str_rm_ext(str);
	json_object_object_add(new_object, "filename",		Dlna_json_object_new_string( str ));

	aim = "dc:date";
	str = XMLUtil_FindFirstElementValue (node, aim,false, false);
	x_IF_STR(str)
	json_object_object_add(new_object, "date",		Dlna_json_object_new_string( str ));


	/*
	["size":"5688754","bitrate":"512"," resolution":"720x576"," duration":"3600"," audiochannel":"2","protocolInfo":"video/mpeg","colorDepth":24,
	*/
	char *protocolInfo = NULL, *url=NULL;
	t_MEDIA_INFO mInfo; 
	int idx = -1;
	IXML_Element *res = NULL;

	mInfo.majorType = 0;
	IXML_NodeList* const reslist = ixmlElement_getElementsByTagName ((IXML_Element*)elem, "res");
	int nb_reslist = ixmlNodeList_length (reslist);
	if( nb_reslist > 0 )
		idx = YX_CD_GetPreferred(me, reslist, nb_reslist, &protocolInfo, &url, &mInfo);

	if( idx >= 0 )
		res = (IXML_Element*)ixmlNodeList_item (reslist, idx);
	//			IXML_Element*element = (IXML_Element*)res;

	aim = "size";
	str = ixmlElement_getAttribute (res, aim);
	long long tmp_size = 0;
	if(str)
		tmp_size = atoll(str);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	aim = "bitrate";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	aim = "resolution";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	aim = "colorDepth";
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

	char *transport = NULL, *mimetype = NULL, *dlna = NULL; 
	YX_CD_SplitInfo(protocolInfo, &transport, &mimetype, &dlna);
	str = mimetype;
	json_object_object_add(new_object, "protocolInfo",		Dlna_json_object_new_string( str ));
#if 0
	aim = "fileFormat";
    char *semicolon = NULL, *dlna_pn = dlna? strstr(dlna, "DLNA.ORG_PN") : NULL;
    if (dlna_pn)
    {
        semicolon = strchr(dlna_pn, ';');
        if (semicolon)
            *semicolon = 0;
		json_object_object_add(new_object, aim, Dlna_json_object_new_string( dlna_pn ));
        if (semicolon)
            *semicolon = ';';
    }
    else
		json_object_object_add(new_object, aim, Dlna_json_object_new_string( "*" ));
#else

	char *semicolon = NULL, *dlna_pn = dlna? strstr(dlna, "DLNA.ORG_PN=") : NULL;
	if(dlna_pn && (semicolon = strchr(dlna_pn, ';')))
		*semicolon = 0;
	
	aim = "fileFormat";
	if(dlna_pn)
		json_object_object_add(new_object, aim, Dlna_json_object_new_string( dlna_pn ));
	else
		json_object_object_add(new_object, aim, Dlna_json_object_new_string( "*" ));
	
	int supported = s_Huawei_IsMediaSupported(&mInfo, mimetype, dlna_pn);
	aim = "compatibleFormat";
	json_object_object_add(new_object, aim, 	json_object_new_int( supported ));
	
	if(semicolon)
		*semicolon = ';';
#endif
	
	/*
	"filepath":"http://10.10.10.8/video/rrtc.mpg",
	" iptvcontentID":"20100803001", " contentSource":"1","iptvuserID":"iptvuser20100898","contentType":"10"," programTitle":"樱桃小丸子48"},
	*/
	str = url;
	if((mInfo.majorType == 3) && (tmp_size > 10*1024*1024))
	{
	//	str = "http://127.0.0.1/default.jpg";
	}
	json_object_object_add(new_object, "filepath",	Dlna_json_object_new_string( str ));

	aim = "channelNO";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	aim = "iptvcontentID";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	aim = "contentSource";
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

	aim = "programTitle";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	aim = "programRate";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	aim = "programDescription";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	aim = "parentTaskName";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	aim = "serialTaskName";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	aim = "periodTaskName";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	if( protocolInfo ) free(protocolInfo);
	if( url ) free(url);
	if (reslist)
		ixmlNodeList_free (reslist);
}
#endif

#if 0
static void s_Huawei_ComposeItemList(pClassContentDirectory me, char *classID, char *value, int bufferLength, int stb_dms)
{
	IXML_Document *subdoc = me->subdoc;
	json_object *my_array, *new_object;
	int jlen; 
	int jsonLength = 0;

	bufferLength -= 100;
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, bufferLength, 0);
	
	IXML_NodeList* items =	ixmlDocument_getElementsByTagName (subdoc, "item"); 
	int const nb_items = ixmlNodeList_length (items);
	HT_DBG_FUNC(nb_items, "nb_items = ");
	
	if( (nb_items > 0) && (me->index < nb_items) )
	{
		int index = me->index;
		my_array = json_object_new_array();
		for (; me->index < nb_items; me->index++) 
		{
			IXML_Node* const node = ixmlNodeList_item(items,  me->index);
			new_object = json_object_new_object();
			
            x_DTB dlna_type = s_HW_Browse_GetDlnaType(node, stb_dms);
			s_Huawei_ComposeItemList_sub(me, node, classID, new_object, dlna_type);

			jlen = strlen(json_object_to_json_string(new_object));
			if( jsonLength + jlen > bufferLength )
			{
                json_object_put(new_object);
				break;
			}
			
			jsonLength += jlen;
			json_object_array_add(my_array, new_object);
		}
		s_HW_Browse_FillResult(my_array, me->index-index, value, bufferLength);
	}
	else
	{
		s_HW_Browse_FillResult(NULL, 0, value, bufferLength);
	}

	HT_DBG_FUNC_END(0, value);
	ixmlNodeList_free (items);
}
#else
static void s_HW_Browse_ExactResult(pClassContentDirectory me, int wanted_type, char *value, int bufferLength, int stb_dms, char *languageCode);
static void s_Huawei_ComposeItemList(pClassContentDirectory me, char *classID, char *value, int bufferLength, int stb_dms)
{
	s_HW_Browse_ExactResult(me, x_DLNA_TYPE_ALL, value, bufferLength, stb_dms, NULL);
}
#endif

/*
{“item_num”:”2”,”items”:”
[{“itemID”: “3-2-1”,
”itemType”:2,
”filename”:”rrtc.mpg”,
”size”:”5688754”,
”bitrate”:”512”,
”resolution”:”720x576”,
”duration”:”3600”,
”audiochannel”:”2”,
”protocolInfo”:”video/mpeg”,
”colorDepth”:24,
”filepath”:”http://10.10.10.8/video/rrtc.mpg”,
”channelNO”:”12”,
”iptvcontentID”:”20100803001”,
”contentSource”:”1”,
”iptvuserID”:”iptvuser20100898”,
”contentType”:”300”,
”programTitle”:”樱桃小丸子48”,”classID”:”22”},
{“itemID”: “0-1-1”,
”itemType”:1,
”parentID”:”0”,
”title”:”music”}
]”
}
*/
static int s_StringIsBlank(char *str)
{
    int i,len = strlen(str);

    for(i=0; i<len; i++)
    {
        if(str[i]!=' ')
            return 0;
    }
    return 1;
}
static int s_StringIsBlank_OrNull(char *str)
{
    if(!str)
        return 1;
    
    if(s_StringIsBlank(str))
        return 1;

    return 0;
}

static x_DTB s_HW_Browse_GetDlnaType(IXML_Node* const node, int stb_dms)
{
    const char *aim, *str, *dot;
	char image[]= "object.item.imageItem";
	char audio[]= "object.item.audioItem";
	char video[]= "object.item.videoItem";
	char container[] = "object.container";
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MYRIAD, stb_dms, 0);
    aim = "upnp:class";
    str = XMLUtil_FindFirstElementValue (node, aim, false, false); //   upnpClass = str;
    if(!str)
        return x_DTB_UNKOWN;
	
    HT_DBG_FUNC(1, str);
    if( !strncmp(str, image, x_STR_LEN(image)) )
        return x_DTB_IMAGE;
    else if( !strncmp(str, audio, x_STR_LEN(audio)) )
        return x_DTB_AUDIO;
    else if( !strncmp(str, video, x_STR_LEN(video)) )
    {
#if 0		
		IXML_Element* element = XMLUtil_FindFirstElement(node, "res", false, false);
		if(!element)
			return x_DTB_VIDEO;
			
		aim = "contentType";
		str = ixmlElement_getAttribute(element, aim);
		if(!str)
		{
			element = (IXML_Element*)ixmlNode_getNextSibling((IXML_Node*)element);
			str = ixmlElement_getAttribute (element, aim);
			if(!str)
	        	return x_DTB_VIDEO;
		}
		
		if(!strcmp(str, "300"))
			return x_DTB_PVR;
		return x_DTB_VOD;
#else
		if(stb_dms)
		{
			aim = "id";
			str = ixmlElement_getAttribute((IXML_Element*)node, aim);
#if 0			
			if(str &&(dot = strchr(str, '.')))
			{
				if(!strcmp(dot+1, "300"))
					return x_DTB_PVR;
				return x_DTB_VOD;
			}
#else
			/* id.hw.300*/
			if(str && (dot = strchr(str, '.')))
			{
				if((dot = strchr(dot + 1, '.')))
				{
					if(!strcmp(dot+1, "300"))
						return x_DTB_PVR;
					return x_DTB_VOD;
				}
			}
#endif
		}
		return x_DTB_VIDEO;
#endif
    }
    else if( !strncmp(str, container, x_STR_LEN(container)) )
        return x_DTB_FOLDER;
    else
        return x_DTB_UNKOWN;
}

static x_DTB s_HW_ClassID_To_DlnaType(char *classID)
{
    x_DTB ret;
    int i = atoi(classID);
    
    if(i==1)
        ret = x_DTB_IMAGE;
    else if( i==2 )
        ret = x_DTB_AUDIO;
    else if( i==3 )
        ret = x_DTB_VIDEO;
    else if( i==3301 )
        ret = x_DTB_PVR;
    else if( i==3302 )
        ret = x_DTB_VOD;
		//ret = x_DTB_VIDEO | x_DTB_PVR | x_DTB_VOD;
    else
        ret = 0;
    return ret;
}

static int s_HW_GetEpgHybirdType(char *classID, char *itemType)
{
    int x, ret;

	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MYRIAD, 0, itemType);
    
    if( s_StringIsBlank_OrNull(itemType) )
    {
        if( s_StringIsBlank_OrNull(classID) )
            ret = x_DLNA_TYPE_ALL;
        else
            ret = s_HW_ClassID_To_DlnaType(classID) | x_DTB_FOLDER;
    }
    else
    {
        x = atoi(itemType);
        if(x==1)//folder
            ret = x_DTB_FOLDER;
        else if( x==2 )//item
        {   
            if( s_StringIsBlank_OrNull(classID) )
                ret = x_DTB_VIDEO | x_DTB_AUDIO | x_DTB_IMAGE | x_DTB_PVR | x_DTB_VOD;
            else
                ret = s_HW_ClassID_To_DlnaType(classID);
        }
        else
            ret = 0;
    }

    HT_DBG_FUNC_END(ret, classID);
    return ret;
}

static int s_HW_Browse_IsMatchedType(x_DTB dlna_type, int wanted_type)
{
#if 0    
    if( wanted_type==x_DLNA_TYPE_ANY_ITEM )
    {
        if(itemType==x_DLNA_TYPE_IMAGE || itemType==x_DLNA_TYPE_AUDIO || itemType==x_DLNA_TYPE_VIDEO )
            return 1;
    }
    else
    {
        if( itemType==wanted_type )
            return 1;
    }
#endif    
    return (wanted_type&dlna_type);
}
#if 0
static void s_HW_Browse_ConvertToArray(pClassContentDirectory me, x_HBCA *hbca)
{
    int i, type, total;

	IXML_Document *subdoc = me->subdoc;
	IXML_NodeList* containers = ixmlDocument_getElementsByTagName(subdoc, "container"); 
	int const nb_containers = ixmlNodeList_length (containers);
	IXML_NodeList* items =	ixmlDocument_getElementsByTagName (subdoc, "item"); 
	int const nb_items = ixmlNodeList_length (items);

    total = nb_containers + nb_items;
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, total, 0);
	if( total<1 )
        goto EXIT;

    hbca->array = realloc(hbca->array, (hbca->num+total+2)*sizeof(int *) );
    if( !hbca->array )
    {
        hbca->num = 0;
        goto EXIT;
    }

	for(i=0; i<total; i++) 
	{
		bool const is_container = (i < nb_containers);
		IXML_Node* const node = ixmlNodeList_item(is_container ? containers : items,  is_container ? i : i - nb_containers);
		
        type = is_container? x_DTB_FOLDER : s_HW_Browse_GetDlnaType(node);
        hbca->array[i+hbca->num] = type;
	}
	hbca->num += total;
    HT_DBG_FUNC(hbca->num, 0);
    
EXIT:    
    HT_DBG_FUNC_END((int)(hbca->array), 0);
    
	if (containers)
		ixmlNodeList_free (containers);
	if (items)
		ixmlNodeList_free (items);
}
static void s_HW_Browse_ConvertToArray(pClassContentDirectory me, x_HBCA *hbca)
{
    int i, type, total;

	IXML_Document *subdoc = me->subdoc;
	IXML_NodeList* list = ixmlDocument_getElementsByTagName(subdoc, "*"); 
	int const nb_list = ixmlNodeList_length (list);
//	IXML_NodeList* items =	ixmlDocument_getElementsByTagName (subdoc, "item"); 
//	int const nb_items = ixmlNodeList_length (items);

	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MYRIAD, nb_list, "nb_list =");
	if( nb_list<1 )
        goto EXIT;

    hbca->array = realloc(hbca->array, (hbca->num + nb_list + 2) * sizeof(int));
    if( !hbca->array )
    {
        hbca->num = 0;
        goto EXIT;
    }
    HT_DBG_FUNC(hbca->num, "hbca->num =");

	total = 0;
	for(i=0; i<nb_list; i++) 
	{
		IXML_Node* const node = ixmlNodeList_item(list, i);
		const char *name = ixmlNode_getNodeName(node);

		HT_DBG_FUNC(i, name);
		if(!strcmp(name, "item") || !strcmp(name, "container"))
		{
	        type = s_HW_Browse_GetDlnaType(node);
	        hbca->array[total + hbca->num] = type;
			total++;
		}
	}
    HT_DBG_FUNC(total, "total = ");
	hbca->num += total;
    
EXIT:    
    HT_DBG_FUNC_END((int)(hbca->array), 0);
    
	if (list)
		ixmlNodeList_free (list);
}

#else
#define x_CTA_CACHE_LEN		1024*2
static void s_HW_Browse_ConvertToArray(pClassContentDirectory me, x_HBCA *hbca, int stb_dms)
{
    int i, total, type;
	int cache[x_CTA_CACHE_LEN];

	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MYRIAD, hbca->num, "hbca->num =");
	
	IXML_Element*element = XMLUtil_FindFirstElement((IXML_Node *)(me->subdoc), "DIDL-Lite", true, false);
	IXML_Node *node = ixmlNode_getFirstChild((IXML_Node *)(element));
	
	i = 0;
	total = 0;
	while(node && (total < x_CTA_CACHE_LEN))
	{
		const char *name = ixmlNode_getNodeName(node);
		type = 0;
		if(!strcmp(name, "item"))
		{
	        type = s_HW_Browse_GetDlnaType(node, stb_dms);
	        cache[total] = type;
			total++;
		}
		else if(!strcmp(name, "container"))
		{
	        cache[total] = x_DTB_FOLDER;
			total++;
		}
		else
		{
		}

		HT_DBG_FUNC(type, name);
		i++;
		node = ixmlNode_getNextSibling(node);
	}

	HT_DBG_FUNC(total, "total = ");
	if(total)
	{
	    hbca->array = realloc(hbca->array, (hbca->num + total + 2) * sizeof(x_DTB));
	    if( !hbca->array )
	    {
	        hbca->num = 0;
	        goto EXIT;
	    }

		memcpy((char*)(hbca->array) + hbca->num * sizeof(x_DTB), cache, total * sizeof(x_DTB));
		hbca->num += total;
	}
    
EXIT:    
    HT_DBG_FUNC_END((int)(hbca->array), "hbca->array =");
}

#endif


static int s_HW_Browse_MatchedTotal(x_HBCA *hbca, char *classID, char *itemType)
{
    int j, wanted_type, ret=0;

	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MYRIAD, hbca->num, classID);
    wanted_type = s_HW_GetEpgHybirdType(classID, itemType);
    if( wanted_type==x_DLNA_TYPE_ALL )
        return hbca->num;
    if( wanted_type== 0 )
        return 0;

    for( j=0; j<hbca->num; j++ )
    {
		HT_DBG_FUNC(hbca->array[j], "hbca->array[j] = ");
        if( s_HW_Browse_IsMatchedType(hbca->array[j], wanted_type) )
            ret++;
    }

    HT_DBG_FUNC_END(wanted_type, itemType);
    return ret;
}
static int s_HW_Browse_MatchedTotal_Ex(x_HBCA *hbca, char *classID, char *itemType, int *containers)
{
	int ret = 0;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, hbca->num, classID);

    if( s_StringIsBlank_OrNull(itemType) )
    {
		*containers = s_HW_Browse_MatchedTotal(hbca, classID, "1");
		ret	= s_HW_Browse_MatchedTotal(hbca, classID, "2");
    }
    else
    {
        if(!strcmp(itemType, "1"))//folder
			ret = s_HW_Browse_MatchedTotal(hbca, classID, itemType);
        else if(!strcmp(itemType, "2"))//item
			ret	= s_HW_Browse_MatchedTotal(hbca, classID, itemType);
        else
        {
        }
    }

    HT_DBG_FUNC_END(ret, itemType);
	return ret;
}


static int s_HW_Browse_NewPositionNum(x_HBCA *hbca, int wanted_type, int *position, int *num)
{
    int i, j;
    int new_position = 0, real_num = 0, ret=1;
    int len = hbca->num;
    x_DTB *array = hbca->array;
    
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, len, "len =");
    // find the new position
    for( i=0; i<len; i++ )
    {
        if( s_HW_Browse_IsMatchedType(array[i], wanted_type) )
        {
            if( new_position == *position)
                break;
            new_position++;
        }
    }

    if(i==len)
        ret = 0;
    *position = i;
    HT_DBG_FUNC(i, "i = ");

    if(ret && *num)
    {
        //count the new num
        for( j=i; j<len; j++ )
        {
            if( s_HW_Browse_IsMatchedType(array[j], wanted_type) )
            {
                real_num++;
                if( real_num == *num)
                    break;
            }
        }
        if(j==len)
            j--;
        *num = j-i+1;
        HT_DBG_FUNC(j, "j = ");
		
		if(!real_num)
			ret = 0;
    }

    
    HT_DBG_FUNC_END(ret, 0);
    return ret;
}
#if 0
static void s_HW_Browse_ExactResult(pClassContentDirectory me, int wanted_type, char *value, int bufferLength, int stb_dms, char *languageCode)
{
	IXML_Document *subdoc = me->subdoc;
	json_object *my_array, *new_object;
	int jlen; 
	int jsonLength = 0;
    int i, real_num=0;

	bufferLength -= 100;
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MYRIAD, wanted_type, "wanted_type= ");
	
	IXML_NodeList* containers = ixmlDocument_getElementsByTagName(subdoc, "container"); 
	int const nb_containers = ixmlNodeList_length (containers);
	IXML_NodeList* items =	ixmlDocument_getElementsByTagName (subdoc, "item"); 
	int const nb_items = ixmlNodeList_length (items);

	if( nb_containers || nb_items )
	{
		my_array = json_object_new_array();
		for(i=0; i<nb_containers+nb_items; i++) 
		{
			bool const is_container = (i < nb_containers);
			IXML_Node* const node = ixmlNodeList_item(is_container ? containers : items,  is_container ? i : i - nb_containers);

            x_DTB dlna_type = s_HW_Browse_GetDlnaType(node, stb_dms);

            HT_DBG_FUNC(dlna_type, "dlna_type = ");
            if( s_HW_Browse_IsMatchedType(dlna_type, wanted_type) )
            {
    			new_object = json_object_new_object();
    			if(is_container)
    			{
    				json_object_object_add(new_object, "itemType", json_object_new_int(1));
    				s_Huawei_ComposeContainerList_sub(node, new_object, 1, languageCode);
    			}
    			else
    			{
    				json_object_object_add(new_object, "itemType", json_object_new_int(2));
    				s_Huawei_ComposeItemList_sub(me, node, NULL, new_object, dlna_type, languageCode);
    			}
    			
    			jlen = strlen(json_object_to_json_string(new_object));
    			if( jsonLength + jlen > bufferLength )
    			{
                    json_object_put(new_object);
    				break;
    			}
    			
    			jsonLength += jlen;
    			json_object_array_add(my_array, new_object);
                real_num++;
            }
		}
		
        s_HW_Browse_FillResult(my_array, real_num, value, bufferLength);
	}
	else
	{
        s_HW_Browse_FillResult(NULL, 0, value, bufferLength);
	}


	if (containers)
		ixmlNodeList_free (containers);
	if (items)
		ixmlNodeList_free (items);
    
	HT_DBG_FUNC(0, me->udn);
	HT_DBG_FUNC_END(0, 0);
}
#else
static void s_HW_Browse_ExactResult(pClassContentDirectory me, int wanted_type, char *value, int bufferLength, int stb_dms, char *languageCode)
{
//	IXML_Document *subdoc = me->subdoc;
	json_object *my_array, *new_object;
	int jlen, jsonLength = 0;
    int is_container = -1, real_num = 0;

	bufferLength -= 100;
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MYRIAD, wanted_type, "wanted_type= ");

	my_array = json_object_new_array();
	IXML_Element*element = XMLUtil_FindFirstElement((IXML_Node *)(me->subdoc), "DIDL-Lite", true, false);
	IXML_Node *node = ixmlNode_getFirstChild((IXML_Node *)(element));
	while(node)
	{
		const char *name = ixmlNode_getNodeName(node);
		
		if(!strcmp(name, "item"))
			is_container = 0;
		else if(!strcmp(name, "container"))
			is_container = 1;
		else
			is_container = -1;
		
		if(is_container >= 0)
		{
            x_DTB dlna_type = s_HW_Browse_GetDlnaType(node, stb_dms);

            HT_DBG_FUNC(dlna_type, "dlna_type = ");
            if( s_HW_Browse_IsMatchedType(dlna_type, wanted_type) )
            {
    			new_object = json_object_new_object();
    			if(is_container)
    			{
    				json_object_object_add(new_object, "itemType", json_object_new_int(1));
    				s_Huawei_ComposeContainerList_sub(node, new_object, 1, languageCode);
    			}
    			else
    			{
    				json_object_object_add(new_object, "itemType", json_object_new_int(2));
    				s_Huawei_ComposeItemList_sub(me, node, NULL, new_object, dlna_type, languageCode);
    			}
    			
    			jlen = strlen(json_object_to_json_string(new_object));
    			if( jsonLength + jlen > bufferLength )
    			{
                    json_object_put(new_object);
    				break;
    			}
    			
    			jsonLength += jlen;
    			json_object_array_add(my_array, new_object);
                real_num++;
            }
		}

		node = ixmlNode_getNextSibling(node);
	}


	if(real_num)
        s_HW_Browse_FillResult(my_array, real_num, value, bufferLength);
	else
	{
		if(my_array)
        	json_object_put(my_array);
        s_HW_Browse_FillResult(NULL, 0, value, bufferLength);
	}

	HT_DBG_FUNC(0, me->udn);
	HT_DBG_FUNC_END(0, 0);
}

#endif
static int s_HW_Browse_GetOneByOne(pClassContentDirectory me, char *id, int wanted_type, int position, int num, x_HBCA *hbca, char *value, int bufferLength, int stb_dms, char *languageCode)
{
	json_object *my_array, *new_object;
	int jlen; 
	int jsonLength = 0;
    int j;
    x_DTB *array = hbca->array;
    int counter = 0;

	bufferLength -= 100;
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, bufferLength, 0);

    my_array = json_object_new_array();
    for(j=position ; j<position+num; j++ )
    {
        if( s_HW_Browse_IsMatchedType(array[j], wanted_type) )
        {
            HT_DBG_FUNC(j, "j = ");
            me->SyncBrowseChildren(me, id, j, 1);
			if(me->sdk_err == UPNP_E_SOCKET_CONNECT)
			{
				json_object_put(my_array);
				return -1;
			} 
			
            if(me->nb_returned!=1)
                continue;

            IXML_NodeList *containers=NULL, *items=NULL;
            IXML_Node*node=NULL;

            containers = ixmlDocument_getElementsByTagName(me->subdoc, "container"); 
            if(containers)
                node = ixmlNodeList_item(containers, 0);
            else
            {
                items = ixmlDocument_getElementsByTagName(me->subdoc, "item"); 
                node = ixmlNodeList_item(items, 0);
            }
            if( !node )
                continue;

            x_DTB dlna_type = s_HW_Browse_GetDlnaType(node, stb_dms);
            
            if( s_HW_Browse_IsMatchedType(dlna_type, wanted_type) )
            {
                new_object = json_object_new_object();
                if(containers)
                {
                    json_object_object_add(new_object, "itemType", json_object_new_int(1));
                    s_Huawei_ComposeContainerList_sub(node, new_object, 1, languageCode);
                }
                else
                {
                    json_object_object_add(new_object, "itemType", json_object_new_int(2));
                    s_Huawei_ComposeItemList_sub(me, node, NULL, new_object, dlna_type, languageCode);
                }
                
                jlen = strlen(json_object_to_json_string(new_object));
                if( jsonLength + jlen > bufferLength )
                {
                    json_object_put(new_object);
                    break;
                }
                
                jsonLength += jlen;
                json_object_array_add(my_array, new_object);
                counter++;
            }
            
            if (containers)
                ixmlNodeList_free (containers);
            if (items)
                ixmlNodeList_free (items);
        }
    } 

    s_HW_Browse_FillResult(my_array, counter, value, bufferLength);

	HT_DBG_FUNC_END(0, value);
	return 0;
}


static char *s_Dmc_Huawei_RootIdString(char *id)
{
	if( id && (atoi(id)==-1) )
		return "0";
	else
		return id;
}
static char* s_Huawei_FindCommaAndInc(char *para)
{
	return Dlna_FindCharAndInc(para, ',');
}


static void s_HW_ClearHBCA(x_HBCA *phbca)
{
	if(!phbca)
		return;
	
    if(phbca->udn)
        free(phbca->udn);
	phbca->udn = NULL;
	
    if(phbca->item_id)
        free(phbca->item_id);
	phbca->item_id = NULL;
	
    if(phbca->array)
        free(phbca->array);
    phbca->array = NULL;
	
    phbca->num = 0;	
}
static int s_Huawei_ParseJs_Qtel(const char *func, char *para, char *value, int len);
static int s_Huawei_ParseJs_Maroc(const char *func, char *para, char *value, int len);
static int s_Huawei_ParseJs(const char *func, const char *const_para, char *value, int len)
{
	int ret = 0;
    char *p1, *p2, *p3, *p4, *p5;
	char *para = (char *)const_para;

    p1 = p2 = p3 = p4 = p5 = NULL;
//	len = 4096;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, 0, 0);

    if( enum_DlnaAppMode_HUAWEI_MAROC == s_js_mode )
    {
        if( strcmp(func, "ItemNumber_Get") == 0 ||
            strcmp(func, "ItemList_Get") == 0 ||
            strcmp(func, "ContainerNumber_Get") == 0 ||
            strcmp(func, "ContItemNumber_Get") == 0 ||
            strcmp(func, "ContainerList_Get") == 0 ||
            strcmp(func, "ContItemList_Get") == 0 )
        {
            return s_Huawei_ParseJs_Maroc(func, para, value, len);
        }
    }
	
    if( enum_DlnaAppMode_HUAWEI_QTEL == s_js_mode )
    {
        if( strcmp(func, "ItemNumber_Get") == 0 ||
            strcmp(func, "ItemList_Get") == 0)
        {
            return s_Huawei_ParseJs_Qtel(func, para, value, len);
        }
    }	
    
/*
DeviceNumber_Get
sValue=iPanel.ioctlRead('DeviceNumber_Get,{"deviceType":"deviceType"}')
*/
	if( strcmp(func, "DeviceNumber_Get") == 0 )
	{
#if 0		
		pClassDmsList me = s_huawei_dms_list;
		ret = me->GetDmsNumber(me);
		sprintf(value, "%d",ret);
		return 0;
#else
		
		json_object *json;
		json_object *j1;
        json = dlna_json_tokener_parse(para);
		pClassDmsList me = s_huawei_dms_list;
		ret = me->GetDmsNumber(me);

		HT_DBG_FUNC((int)json, "json = ");
        if(!json)
        {
			s_hw_get_device_list_mode = 0;
			sprintf(value, "%d",ret);
			return 0;
        }

        j1 = json_object_object_get(json, "deviceType");
        if(j1 && (p1 = (char*)json_object_get_string(j1)))
        {
			if(!strcmp(p1, "ALL"))
			{
				s_hw_get_device_list_mode = 0;
				sprintf(value, "%d",ret);
			}
			else if(!strcmp(p1, "STB"))
			{
				s_hw_get_device_list_mode = 1;
				sprintf(value, "%d", s_Huawei_GetDmsCount_STB());
			}
			else
			{
			}
        }
		HT_DBG_FUNC(s_hw_get_device_list_mode, "s_hw_get_device_list_mode = ");
        json_object_put(json);
		return 0;
#endif
	}
/*
DeviceList_Get::postion,count
DeviceList_Get::postion,count,deviceType
*/
	else if( strcmp(func, "DeviceList_Get") == 0 )
	{
		if(!s_hw_get_device_list_mode)
		{
			p1 = s_Huawei_FindCommaAndInc(para);
	        if( !p1 )
	            return -1;
			s_Huawei_GetDmsLists(atoi(para), atoi(p1), value, len);
			return 0;
		}
		else
		{
			p1 = s_Huawei_FindCommaAndInc(para);
	        if( !p1 )
	            return -1;
			s_Huawei_GetDmsLists_STB(atoi(para), atoi(p1), value, len);
			return 0;
		}
	}

/*
sValue=iPanel.ioctlRead("ItemNumber_Get::deviceID,classID")
sValue	M	Int	 	查询正常时，返回的内容数量。
 	DMP或取超时，返回-1。
EPG获得-1时，知道正在查询的DMS可能存在异常，给用户相关性的提示信息。

sValue=iPanel.ioctlRead("ItemNumber_Get::deviceID,classID,languageCode")

*/
	else if( strcmp(func, "ItemNumber_Get") == 0 )
	{
		char *criteria = s_Huawei_ParseClassID(para);
		p1 = s_Huawei_FindCommaAndInc(para);
		p2 = s_Huawei_FindCommaAndInc(p1);
        if( !p1 )
            return -1;
		
		if( s_hbca.udn && !strcmp(s_hbca.udn, s_huawei_browser->udn) && s_Dmc_Huawei_IsInSec(1) )
		{
			ret = -1;
			sprintf(value, "%d", ret);
			return 0;
		}

		if(s_hw_languageCode)
		{
			free(s_hw_languageCode);
			s_hw_languageCode = NULL;
		}
		if(p2)
			s_hw_languageCode = strdup(p2);
		
		pClassContentDirectory me = (pClassContentDirectory)YX_CD_Create(NULL, NULL, 0, NULL);
		me->SetDmsUdn(me, para);
		me->SetSearchCriteria(me, criteria);
		me->SyncSearch(me, "0", 0, 1);
		ret = me->nb_matched;
		if(me->sdk_err == UPNP_E_SOCKET_CONNECT)
			ret = -1;
		me->Release(me);
		
		sprintf(value, "%d", ret);
		return 0;
	}
/*
sJson=iPanel.ioctlRead("ItemList_Get::deviceID,classID,postion,count")
有一些场景中，访问这是一个同步接口，正常情况以JOSN方式返回列表。
当获取列表超时，机顶盒返回-1，EPG得到-1后，即认为DMS离线。
*/
	else if( strcmp(func, "ItemList_Get") == 0 )
	{
		char *criteria = s_Huawei_ParseClassID(para);
		p1 = s_Huawei_FindCommaAndInc(para);
		p2 = s_Huawei_FindCommaAndInc(p1);
		p3 = s_Huawei_FindCommaAndInc(p2);
        if( !p1 || !p2 || !p3 )
            return -1;
		
		if( s_hbca.udn && !strcmp(s_hbca.udn, s_huawei_browser->udn) && s_Dmc_Huawei_IsInSec(1) )
		{
			ret = -1;
			sprintf(value, "%d", ret);
			return 0;
		}
		pClassContentDirectory me = s_huawei_browser;
		me->SetDmsUdn(me, para);
		me->SetSearchCriteria(me, criteria);
		me->SetFilter(me, "*");
		me->SyncSearch(me, "0", atoi(p2), atoi(p3));
		if(me->sdk_err == UPNP_E_SOCKET_CONNECT)
		{
			ret = -1;
			sprintf(value, "%d", ret);
			return 0;
		}
		int stb_dms = s_Is_HuaweiStbDms(const_para);
		s_Huawei_ComposeItemList(me, p1, value, len, stb_dms);
		return 0;
	}
/*
Item_Delte
*/
	else if( strcmp(func, "Item_Delte") == 0 )
	{
        return -1;//cant support
	}
/*
    接口原型
    sJson=iPanel.ioctlRead("ContItemNumber_Get::deviceID ,containerID,classID,itemType")
    deviceID            M   String  设备ID。
    containerID     M   String  目录ID，根目录为-1，第一次获取时从根目录开始获取。
    classID         O   String  classID.     如果不携，表示不用区分内容类型。
    itemType        O                       1：表示文件夹  2：媒体内容    如果不携带这个参数，并表同时返回内容和文件夹总数
                                                
    返回参数
    {"items_num":"56","deviceID":"<deviceUDN>"}
    items_num   M   String  目录ID，根目录为-1，第一次获取时从根目录开始获取。
    deviceID    M   String  设备ID。

sJson=iPanel.ioctlRead("ContItemNumber_Get::deviceID ,containerID,classID,itemType, languageCode")    
*/
	else if( strcmp(func, "ContItemNumber_Get") == 0 )
	{
        int nb_matched, nb_returned, itemType, nb_container;
		x_HBCA *phbca = &s_hbca;
		json_object *new_obj;
                                                //para: deviceID
        p1 = s_Huawei_FindCommaAndInc(para);    // p1:    containerID
        p2 = s_Huawei_FindCommaAndInc(p1);      // p2:    classID
        p3 = s_Huawei_FindCommaAndInc(p2);      //p3:     itemType
        p4 = s_Huawei_FindCommaAndInc(p3);      //p4:     languageCode
        p1 = s_Dmc_Huawei_RootIdString(p1);
        if( !p1 )
            return -1;

		itemType = s_StringIsBlank_OrNull(p3);
		nb_container = 0;
		
		if( phbca->udn && !strcmp(phbca->udn, s_huawei_browser->udn) && s_Dmc_Huawei_IsInSec(1) )
		{
			nb_matched = -1;
			goto s_EXIT;
		}
#if 0		
		if(phbca->udn && phbca->item_id && phbca->array)
		{
			if(!strcmp(phbca->udn, para) && !strcmp(phbca->item_id, p1))
			{
				//same as previous 
				nb_matched = s_HW_Browse_MatchedTotal(phbca, p2, p3); 
				goto s_EXIT;
			}
		}
#endif

		if(s_hw_languageCode)
		{
			free(s_hw_languageCode);
			s_hw_languageCode = NULL;
		}
		if(p4)
			s_hw_languageCode = strdup(p4);

        s_HW_ClearHBCA(phbca);
		phbca->udn = strdup(para);
		phbca->item_id = strdup(p1);

		int stb_dms = s_Is_HuaweiStbDms(const_para);
		pClassContentDirectory me = (pClassContentDirectory)YX_CD_Create(NULL, NULL, 0, NULL);
    	me->SetDmsUdn(me, para);

    	//me->SetFilter(me, "res@contentType");
    	me->SetFilter(me, "@id");
		
    	me->SyncBrowseChildren(me, p1, 0, 0);
    	nb_matched = me->nb_matched;
        nb_returned= me->nb_returned;
        if( nb_matched>=0 && nb_returned>0 )
        {
            if( nb_returned == nb_matched )// ok, we have get all
                s_HW_Browse_ConvertToArray(me, phbca, stb_dms);
            else// we have to get the remained
            {
                while(me->nb_returned>0)
                {
					s_HW_Browse_ConvertToArray(me, phbca, stb_dms);
                    me->SyncBrowseChildren(me, p1, nb_returned, 0);
                    nb_returned += me->nb_returned;
                }
            }
			nb_matched = s_HW_Browse_MatchedTotal_Ex(phbca, p2, p3, &nb_container);
        }
        else if(me->sdk_err == UPNP_E_SUCCESS)
        {
			me->SyncBrowseChildren(me, p1, 0, 100);
			nb_matched = me->nb_matched;
			nb_returned= me->nb_returned;
			if( nb_matched>=0 && nb_returned>0 )
			{
				if( nb_returned == nb_matched )// ok, we have get all
					s_HW_Browse_ConvertToArray(me, phbca, stb_dms);
				else// we have to get the remained
				{
				   while(me->nb_returned>0)
				   {
					   s_HW_Browse_ConvertToArray(me, phbca, stb_dms);
					   me->SyncBrowseChildren(me, p1, nb_returned, 100);
					   nb_returned += me->nb_returned;
				   }
				}
				nb_matched = s_HW_Browse_MatchedTotal_Ex(phbca, p2, p3, &nb_container);
			}
			else
				nb_matched = 0;
        }
		else
		{
			nb_matched = 0;
		}
		
		if(me->sdk_err == UPNP_E_SOCKET_CONNECT)
		{
			s_HW_ClearHBCA(phbca);
			nb_matched = -1;
		}
		me->Release(me);
		
s_EXIT:
    	new_obj = json_object_new_object();
		json_object_object_add(new_obj, "items_num", json_object_new_int(nb_matched));
		if(itemType)
			json_object_object_add(new_obj, "containers_num", json_object_new_int(nb_container));
    	json_object_object_add(new_obj, "deviceID", Dlna_json_object_new_string(para));
    	strcpy(value, json_object_to_json_string(new_obj) );
    	json_object_put(new_obj);
    	return 0;
	}
/*
    接口原型
    sJson=iPanel.ioctlRead("ContItemList_Get: deviceID ,containerID,postion,count,classID,itemType")
    postion	M	Int	获取目录的起始位置。
    count	M	Int	获取的数目。
    其余参数同上
*/
	else if( strcmp(func, "ContItemList_Get") == 0 )
	{
        int position, num, real_num;
		x_HBCA *phbca = &s_hbca;
                                                //para: deviceID
        p1 = s_Huawei_FindCommaAndInc(para);    // p1:    containerID
        p2 = s_Huawei_FindCommaAndInc(p1);      // p2:    postion
        p3 = s_Huawei_FindCommaAndInc(p2);      //p3:     count
        p4 = s_Huawei_FindCommaAndInc(p3);      //p4:     classID
        p5 = s_Huawei_FindCommaAndInc(p4);      //p5:     itemType
        p1 = s_Dmc_Huawei_RootIdString(p1);

        if( !p1 || !p2 || !p3 )
            return -1;
        
        if( !s_hbca.array)
            return -2;
        
        position = atoi(p2);
        if(position<0)
            position = 0;
        num = atoi(p3);
        if(num<0)
            return -3;

		if( phbca->udn && !strcmp(phbca->udn, s_huawei_browser->udn) && s_Dmc_Huawei_IsInSec(1) )
		{
			real_num = -1;
			sprintf(value, "%d", real_num);
			return 0;
		}
		
        int s_num = num;
        int wanted_type = s_HW_GetEpgHybirdType(p4, p5);
        real_num = 1;
        if( wanted_type!=x_DLNA_TYPE_ALL )
            real_num=s_HW_Browse_NewPositionNum(&s_hbca, wanted_type, &position, &num); 
        
        if( real_num )
        {
        	//pClassContentDirectory me = (pClassContentDirectory)YX_CD_Create(NULL, NULL, 0);
			int stb_dms = s_Is_HuaweiStbDms(const_para);
            pClassContentDirectory me = s_huawei_browser;
        	me->SetDmsUdn(me, para);
			me->SetFilter(me, "*");

#if 0			
            me->SyncBrowseChildren(me, p1, position, num);

			if(me->sdk_err == UPNP_E_SOCKET_CONNECT)
			{
				real_num = -1;
				sprintf(value, "%d", real_num);
			}    
			else
			{
	            if( (num==0) || (me->nb_returned == num) )
	                s_HW_Browse_ExactResult(me, wanted_type, value, len, stb_dms);
	            else
	            {
	                real_num = s_HW_Browse_GetOneByOne(me, p1, wanted_type, position, num, &s_hbca, value, len, stb_dms); 
					if(real_num == -1)
					{
						//s_HW_ClearHBCA(&s_hbca);
						real_num = -1;
						sprintf(value, "%d", real_num);
					}
	            }
			}
#else
			if((s_num == 0) || (num - s_num)<20)
			{
				me->SyncBrowseChildren(me, p1, position, num);
				if(me->sdk_err == UPNP_E_SOCKET_CONNECT)
				{
					real_num = -1;
					sprintf(value, "%d", real_num);
				}	 
				else
					s_HW_Browse_ExactResult(me, wanted_type, value, len, stb_dms, s_hw_languageCode);
			}
			else
			{
				real_num = s_HW_Browse_GetOneByOne(me, p1, wanted_type, position, num, &s_hbca, value, len, stb_dms, s_hw_languageCode); 
				//解析媒体信息
				if(real_num == -1)
					sprintf(value, "%d", real_num);
			}

#endif
        	//me->Release(me);
        }
        else
            s_HW_Browse_FillResult(0, 0, value, len);
    	return 0;
	}
/*
dlna.statutsTest.triggered
*/
	else if( strcmp(func, "dlna.statutsTest.triggered") == 0 )
	{
		Dlna_Huawei_OnIpChanged(UpnpStackGetIfname(), UpnpGetServerIpAddress());
		Dlna_Huawei_OnSearchInitiated();
		YX_Dmscp_SearchDms(1);
		return 0;
	}
	else
	{
	}
	
	return -1;
}

int Raw_Dmc_HuaweiJse(const char *func, const char *para, char *value, int len)
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_KEY,len, func);
	HT_DBG_FUNC(len, para);
	HT_DBG_FUNC(len, value);

	int ret = s_Huawei_ParseJs(func, para, value, len);
	
	HT_DBG_FUNC_END(ret, value);
	return ret;
}

void HW_Dmscp_Init(enum_DlnaAppMode mode)
{
    s_js_mode = mode;
    
	if(s_huawei_event_queue == NULL)
		s_huawei_event_queue = EventQueue_Create();
	
	Dlna_Huawei_OnSearchInitiated();
	
	if(s_huawei_dms_list == NULL)
		s_huawei_dms_list = (pClassDmsList)YX_DmsList_Create(s_Huawei_OnDmsUpdated, (int)HUAWEI_OBJECT_HND, NULL);
	if(s_huawei_browser == NULL)
		s_huawei_browser = (pClassContentDirectory)YX_CD_Create(NULL, NULL, 0, NULL);
}

#if 0
static void s_Huawei_GetPreferred(pClassContentDirectory me, char **protocolInfo, char **url, t_MEDIA_INFO *mInfo)
{
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW,0, 0);
	IXML_NodeList* items =	ixmlDocument_getElementsByTagName (me->subdoc, "item"); 
	IXML_Node* const node = ixmlNodeList_item(items,  0);
	IXML_Element* const elem = (IXML_Element*)node;
	IXML_NodeList* const reslist = ixmlElement_getElementsByTagName ((IXML_Element*)elem, "res");
	int nb_reslist = ixmlNodeList_length (reslist);
	HT_DBG_FUNC(nb_reslist, "nb_reslist=");
	if( nb_reslist > 0 )
		nb_reslist = YX_CD_GetPreferred(me, reslist, nb_reslist, protocolInfo, url, mInfo);
	
	HT_DBG_FUNC(nb_reslist, "YX_CD_GetPreferred=");
	if (reslist)
		ixmlNodeList_free (reslist);
	
	ixmlNodeList_free (items);
	HT_DBG_FUNC_END(nb_reslist, 0);
}

char *Raw_Dmc_Huawei_BrowseMetadata(char *udn, char *id, char **best_uri, t_MEDIA_INFO*best_minfo)
{
	char *str = NULL;

	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW,(int)s_huawei_browser, udn);
	HT_DBG_FUNC(0, id);

	if( udn == NULL )
	{
		pClassContentDirectory me = s_huawei_browser;
        HT_DBG_FUNC(0, me->udn);
		me->SyncBrowseMetedata(me, id);
		if( me->nb_returned > 0)
		{
			const char *result = XMLUtil_FindFirstElementValue(XML_D2N (me->ActionResult), "Result", true, true);
			str = Dlna_strdup(result);
			s_Huawei_GetPreferred(me, NULL, best_uri, best_minfo);
		}
	}
	else
	{
		pClassContentDirectory me = (pClassContentDirectory)YX_CD_Create(NULL, NULL,0);
		me->SetDmsUdn(me, udn);
		me->SyncBrowseMetedata(me, id);
		if( me->nb_returned > 0)
		{
			const char *result = XMLUtil_FindFirstElementValue(XML_D2N (me->ActionResult), "Result", true, true);
			str = Dlna_strdup(result);
			s_Huawei_GetPreferred(me, NULL, best_uri, best_minfo);
		}
		me->Release(me);
	}

	HT_DBG_FUNC_END(0, str);
	return str;
}
#endif

int Raw_Dmc_Huawei_GetDmsIp(char *udn, char **dms_ip)
{
	int ret = -1;

	if(!dms_ip)
		return -2;
	*dms_ip = NULL;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW,(int)s_huawei_browser, udn);

	if( udn == NULL )
	{
		pClassContentDirectory me = s_huawei_browser;
		*dms_ip = Dlna_strdup(me->dmsIp);
		ret = 0;
	}
	else
	{
		pClassContentDirectory me = (pClassContentDirectory)YX_CD_Create(NULL, NULL,0, NULL);
		me->SetDmsUdn(me, udn);
		*dms_ip = Dlna_strdup(me->dmsIp);
		ret = 0;
		me->Release(me);
	}
	
	HT_DBG_FUNC_END(ret, *dms_ip);
	return ret;
}

int Raw_Dmc_Huawei_BrowseMetadata(char *udn, char *id, char **metadata)
{
	int ret = -1;
	int sdk_err;

	if(!id || !metadata)
		return -2;
	*metadata = NULL;
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_FEW,(int)s_huawei_browser, udn);
	HT_DBG_FUNC(0, id);

	if( udn == NULL )
	{
		pClassContentDirectory me = s_huawei_browser;
		ret = YX_CD_GetMetadata(me, id, metadata);
		sdk_err = me->sdk_err;
	}
	else
	{
		pClassContentDirectory me = (pClassContentDirectory)YX_CD_Create(NULL, NULL,0, NULL);
		me->SetDmsUdn(me, udn);
		ret = YX_CD_GetMetadata(me, id, metadata);
		sdk_err = me->sdk_err;
		me->Release(me);
	}

	if(sdk_err == UPNP_E_SOCKET_CONNECT)
	{
		s_Dmc_Huawei_RecodTime();
	}
	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

int Raw_Dmc_Huawei_GetBestUri( char *udn, char *id, char **uri)
{
	int ret = -1;
	char *metadata=NULL, *dms_ip=NULL;

	if(!id || !uri)
		return -2;
	*uri = NULL;

	ret = Raw_Dmc_Huawei_GetDmsIp(udn, &dms_ip);
	ret = Raw_Dmc_Huawei_BrowseMetadata(udn, id, &metadata);

	if(dms_ip&&metadata)
	{
		YX_CD_GetBestUri(dms_ip, metadata, uri);
		ret = 0;
	}
	
	if(dms_ip)
		free(dms_ip);
	if(metadata)
		free(metadata);

	return ret;
}	

int Raw_Dmc_Huawei_GetBestMediaInfo( char *udn, char *id, t_MEDIA_INFO *minfo)
{
	int ret = -1;
	char *metadata=NULL, *dms_ip=NULL;

	if(!id || !minfo)
		return -2;

	ret = Raw_Dmc_Huawei_GetDmsIp(udn, &dms_ip);
	ret = Raw_Dmc_Huawei_BrowseMetadata(udn, id, &metadata);

	if(dms_ip&&metadata)
	{
		YX_CD_GetBestMediaInfo(dms_ip, metadata, minfo);
		ret = 0;
	}
	
	if(dms_ip)
		free(dms_ip);
	if(metadata)
		free(metadata);

	return ret;
}	


#if 1
static void s_Huawei_ComposeItemList_StdProperty(pClassContentDirectory me, IXML_Node *node, char *classID, json_object *new_object, int skip_title)
{
	IXML_Element* const elem = (IXML_Element*)node;
	const char *aim, *str;

	/*
	[{"classID":"3","itemID": "3-2-1","filename":"rrtc.mpg"
	*/
	if(classID)
		str = classID;
	else
	{
		aim = "upnp:class";
		str = XMLUtil_FindFirstElementValue (node, aim,false, true); 
		str = s_Huawei_GuessItemType(str);
	}
	json_object_object_add(new_object, "classID",		Dlna_json_object_new_string( str ));

	aim = "id";
	str = ixmlElement_getAttribute(elem, aim);
	json_object_object_add(new_object, "itemID",		Dlna_json_object_new_string( str ));

	if(!skip_title)
	{
		aim = "dc:title";
		str = XMLUtil_FindFirstElementValue (node, aim,false, true);
		coo_str_rm_ext(str);
		json_object_object_add(new_object, "filename",		Dlna_json_object_new_string( str ));
	}

	aim = "dc:date";
	str = XMLUtil_FindFirstElementValue (node, aim,false, false);
	x_IF_STR(str)
	json_object_object_add(new_object, "date",		Dlna_json_object_new_string( str ));

/*
artist		O	string(30)	演唱者
album		O	string(30)	专辑
year			O	string(30)	音乐年份
genre		O	string(30)	音乐流派
copyright		O	string(30)	版权信息
composer	O	string		作曲
sampleRate:	O	string(30)	音频采样率。
url			O	string(30)	间乐信息URL
coverURL		O	string(30)	mp3文件的封面海报地址，
*/
	aim = "upnp:artist";
	str = XMLUtil_FindFirstElementValue (node, aim,false, false);
	x_IF_STR(str)
	json_object_object_add(new_object, "artist",		Dlna_json_object_new_string( str ));

	aim = "upnp:album";
	str = XMLUtil_FindFirstElementValue (node, aim,false, false);
	x_IF_STR(str)
	json_object_object_add(new_object, "album",			Dlna_json_object_new_string( str ));

	aim = "upnp:year";
	str = XMLUtil_FindFirstElementValue (node, aim,false, false);
	x_IF_STR(str)
	json_object_object_add(new_object, "year",			Dlna_json_object_new_string( str ));

	aim = "upnp:genre";
	str = XMLUtil_FindFirstElementValue (node, aim,false, false);
	x_IF_STR(str)
	json_object_object_add(new_object, "genre",			Dlna_json_object_new_string( str ));

	aim = "upnp:rights";
	str = XMLUtil_FindFirstElementValue (node, aim,false, false);
	x_IF_STR(str)
	json_object_object_add(new_object, "copyright",		Dlna_json_object_new_string( str ));

	aim = "upnp:author";
	str = XMLUtil_FindFirstElementValue (node, aim,false, false);
	x_IF_STR(str)
	json_object_object_add(new_object, "composer",		Dlna_json_object_new_string( str ));
	
	aim = "upnp:albumArtURI";
	str = XMLUtil_FindFirstElementValue (node, aim,false, false);
	x_IF_STR(str)
	json_object_object_add(new_object, "coverURL",		Dlna_json_object_new_string( str ));
}

static void s_Huawei_ComposeItemList_Some_Res(pClassContentDirectory me, IXML_Element *res, char *protocolInfo, char *url, t_MEDIA_INFO *minfo, json_object *new_object)
{
	const char *aim, *str;
	
	aim = "sampleFrequency";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, "sampleRate",	Dlna_json_object_new_string( str ));

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

	char *transport = NULL, *mimetype = NULL, *dlna = NULL; 
	YX_CD_SplitInfo(protocolInfo, &transport, &mimetype, &dlna);
	str = mimetype;
	json_object_object_add(new_object, "protocolInfo",		Dlna_json_object_new_string( str ));

	char *semicolon = NULL, *dlna_pn = dlna? strstr(dlna, "DLNA.ORG_PN=") : NULL;
	if(dlna_pn && (semicolon = strchr(dlna_pn, ';')))
		*semicolon = 0;
	
	aim = "fileFormat";
	if(dlna_pn)
		json_object_object_add(new_object, aim, Dlna_json_object_new_string( dlna_pn ));
	else
		json_object_object_add(new_object, aim, Dlna_json_object_new_string( "*" ));
	
	int supported = s_Huawei_IsMediaSupported(minfo, mimetype, dlna_pn);
	aim = "compatibleFormat";
	json_object_object_add(new_object, aim, 	json_object_new_int( supported ));
	
	if(semicolon)
		*semicolon = ';';

	/*
	"filepath":"http://10.10.10.8/video/rrtc.mpg",
	" iptvcontentID":"20100803001", " contentSource":"1","iptvuserID":"iptvuser20100898","contentType":"10"," programTitle":"樱桃小丸子48"},
	*/
	str = url;
	//if((mInfo.majorType == 3) && (tmp_size > 10*1024*1024))
	{
	//	str = "http://127.0.0.1/default.jpg";
	}
	json_object_object_add(new_object, "filepath",	Dlna_json_object_new_string( str ));
}

/*
["size":"5688754","bitrate":"512"," resolution":"720x576"," duration":"3600"," audiochannel":"2","protocolInfo":"video/mpeg","colorDepth":24,
*/
static void s_Huawei_ComposeItemList_StdItem(pClassContentDirectory me, IXML_Node *node, char *classID, json_object *new_object)
{
	const char *aim, *str;
	IXML_Element* const elem = (IXML_Element*)node;
	IXML_Element *res = NULL;
	char *protocolInfo = NULL, *url=NULL;
	t_MEDIA_INFO mInfo; 
	
	s_Huawei_ComposeItemList_StdProperty(me, node, classID, new_object, 0);
	//解析文件描述信息。媒体信息
	mInfo.majorType = 0;
	IXML_NodeList* const reslist = ixmlElement_getElementsByTagName ((IXML_Element*)elem, "res");
	int nb_reslist = ixmlNodeList_length (reslist);
	if( nb_reslist < 1 )
		goto s_EXIT;
	int idx = YX_CD_GetPreferred(me, reslist, nb_reslist, &protocolInfo, &url, &mInfo);
	if(idx < 0)
		goto s_EXIT;
	res = (IXML_Element*)ixmlNodeList_item (reslist, idx);

	s_Huawei_ComposeItemList_Some_Res(me, res, protocolInfo, url, &mInfo, new_object);
	//解析是否支持播放的类型
	aim = "size";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	aim = "bitrate";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	aim = "resolution";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	aim = "colorDepth";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	aim = "bitsPerSample";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

	aim = "nrAudioChannels";
	str = ixmlElement_getAttribute (res, aim);
	x_IF_STR(str)
	json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

s_EXIT:
	if( protocolInfo )	free(protocolInfo);
	if( url )			free(url);
	if (reslist)		ixmlNodeList_free (reslist);
}

//"program_description_all_lang": "{ \"count\": 2, \"eng\": \123456789"\", \"hun\": \"abc\" }",
static void g_Huawei_FillRes_ByLanguageCode(json_object *new_object, const char *aim, char *m_val, char *languageCode)
{
	const char *s;
	struct json_object *j, *j_lang = dlna_json_tokener_parse(m_val);

//	languageCode = "hun";
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MYRIAD,(int)j_lang, m_val);
	HT_DBG_FUNC(0, languageCode);
	if(j_lang && (j = json_object_object_get(j_lang, "count")))
	{
		if((j = json_object_object_get(j_lang, languageCode)) && (s = json_object_get_string(j)))
		{
			HT_DBG_FUNC(1, languageCode);
			json_object_object_add(new_object, aim, Dlna_json_object_new_string( s ));
		}
		else if((j = json_object_object_get(j_lang, "eng")) && (s = json_object_get_string(j)))
		{
			HT_DBG_FUNC(2, "eng");
			json_object_object_add(new_object, aim, Dlna_json_object_new_string( s ));
		}
		else
		{
			HT_DBG_FUNC(3, 0);
			char *key; struct json_object *val; struct lh_entry *entry; 
			for(entry = json_object_get_object(j_lang)->head; (entry ? (key = (char*)entry->k, val = (struct json_object*)entry->v, entry) : 0); entry = entry->next)
			{
				if(strcmp(key, "count"))
				{
					HT_DBG_FUNC(33, key);
					s = (char*)json_object_get_string(val);
					json_object_object_add(new_object, aim, Dlna_json_object_new_string( s ));
					break;
				}
			}
		}
	}
	else
		json_object_object_add(new_object, aim, Dlna_json_object_new_string( m_val));

	if(j_lang)
		json_object_put(j_lang);	
	HT_DBG_FUNC_END(0, languageCode);
}
static void s_Huawei_ComposeItemList_HwPvr(pClassContentDirectory me, IXML_Node *node, char *classID, json_object *new_object, char *languageCode)
{
	const char *aim, *str;
	IXML_Element* const elem = (IXML_Element*)node;
	IXML_Element *res = NULL;
	char *protocolInfo = NULL, *url=NULL;
	t_MEDIA_INFO mInfo; 
	
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MYRIAD,(int)node, classID);
	s_Huawei_ComposeItemList_StdProperty(me, node, classID, new_object, 1);

	{
		aim = "dc:title";
		str = XMLUtil_FindFirstElementValue (node, aim,false, true);
		char *buf = malloc(strlen(str) + 2);
		buf[0] = 0;
		coo_str_from_xml((char*)str, buf);

		aim = "filename";
		if(languageCode)
			g_Huawei_FillRes_ByLanguageCode(new_object, aim, buf, languageCode);
		else
			json_object_object_add(new_object, aim, Dlna_json_object_new_string( buf ));

		free(buf);
	}
	
	mInfo.majorType = 0;
	IXML_NodeList* const reslist = ixmlElement_getElementsByTagName ((IXML_Element*)elem, "res");
	int nb_reslist = ixmlNodeList_length (reslist);
	if( nb_reslist < 1 )
		goto s_EXIT;
	int idx = YX_CD_GetPreferred(me, reslist, nb_reslist, &protocolInfo, &url, &mInfo);
	if(idx < 0)
		goto s_EXIT;
	res = (IXML_Element*)ixmlNodeList_item (reslist, idx);

	s_Huawei_ComposeItemList_Some_Res(me, res, protocolInfo, url, &mInfo, new_object);
	
	IXML_Node *attrNode = res->n.firstAttr;
	while( attrNode != NULL ) 
	{
		aim = attrNode->nodeName;
		str = attrNode->nodeValue;
		HT_DBG_FUNC(3, aim);
		HT_DBG_FUNC(4, str);
		if(aim && str 
			&& strcasecmp(aim, "duration")
			&& strcasecmp(aim, "sampleFrequency")) 
		{
			HT_DBG_FUNC(5, languageCode);
			char *buf = malloc(strlen(str) + 2);
			buf[0] = 0;
			coo_str_from_xml((char*)str, buf);
			HT_DBG_FUNC(6, buf);

			if(languageCode)
			{
#if 0				
				struct json_object *j, *j_lang = dlna_json_tokener_parse(buf);
				if(j_lang && (j = json_object_object_get(j_lang, "count")))
				{
					const char *s;
					if((j = json_object_object_get(j_lang, languageCode)) && (s = json_object_get_string(j)))
					{
						HT_DBG_FUNC(10, languageCode);
						json_object_object_add(new_object, aim, Dlna_json_object_new_string( s ));
					}
					else if((j = json_object_object_get(j_lang, "eng")) && (s = json_object_get_string(j)))
					{
						HT_DBG_FUNC(11, "eng");
						json_object_object_add(new_object, aim, Dlna_json_object_new_string( s ));
					}
					else
					{
						//"program_description_all_lang": "{ \"count\": 2, \"eng\": \123456789"\", \"hun\": \"\" }",
#if 0
						struct lh_entry *entry = json_object_get_object(j_lang)->head; 
						if(entry && entry->next) 
						{
							entry = entry->next;
							s = json_object_get_string((struct json_object*)entry->v);
							json_object_object_add(new_object, aim, Dlna_json_object_new_string( s ));
						}
#else
						HT_DBG_FUNC(12, 0);
						char *key; struct json_object *val; struct lh_entry *entry; 
						for(entry = json_object_get_object(j_lang)->head; (entry ? (key = (char*)entry->k, val = (struct json_object*)entry->v, entry) : 0); entry = entry->next)
						{
							if(strcmp(key, "count"))
							{
								HT_DBG_FUNC(13, key);
								s = (char*)json_object_get_string(val);
								json_object_object_add(new_object, aim, Dlna_json_object_new_string( s ));
								break;
							}
						}
#endif
					}
					
					json_object_put(j_lang);	
				}
				else
					json_object_object_add(new_object, aim, Dlna_json_object_new_string( buf ));
#else
				g_Huawei_FillRes_ByLanguageCode(new_object, aim, buf, languageCode);
#endif
			}
			else
				json_object_object_add(new_object, aim, Dlna_json_object_new_string( buf ));
			
			free(buf);
		}
		
		attrNode = attrNode->nextSibling;
	}

s_EXIT:
	if( protocolInfo )	free(protocolInfo);
	if( url )			free(url);
	if (reslist)		ixmlNodeList_free (reslist);
	HT_DBG_FUNC_END(0, 0);
}

static void s_Huawei_ComposeItemList_sub(pClassContentDirectory me, IXML_Node *node, char *classID, json_object *new_object, x_DTB dlna_type, char *languageCode)
{
	if(dlna_type == x_DTB_PVR || dlna_type == x_DTB_VOD)
		s_Huawei_ComposeItemList_HwPvr(me, node, classID, new_object, languageCode);
	else
		s_Huawei_ComposeItemList_StdItem(me, node, classID, new_object);
}

#endif

#define _JS_HUAWEI_MAROC_
#define _JS_HUAWEI_QTEL_
#include "js_huawei_maroc.c"

