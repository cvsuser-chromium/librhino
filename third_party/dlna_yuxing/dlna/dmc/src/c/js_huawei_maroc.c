
#ifdef _JS_HUAWEI_MAROC_
/*
{"container_num":"2","containers":"[{"containerID": "0-1-1","parentID":"0","title":"music" },{"containerID": "0-1-2","parentID":"0","title":"movies"}]"}
*/
static void s_Huawei_ComposeContainerList_Maroc_sub(IXML_Node *node, json_object *new_object)
{
	IXML_Element* const elem = (IXML_Element*)node;
	const char *aim, *str;
	
	aim = "id";
	str = ixmlElement_getAttribute(elem, aim);
	json_object_object_add(new_object, "containerID",		Dlna_json_object_new_string( str ));
	
	aim = "parentID";
	str = ixmlElement_getAttribute(elem, aim);
	json_object_object_add(new_object, aim, 		Dlna_json_object_new_string( str ));
	
	aim = "dc:title";
	str = XMLUtil_FindFirstElementValue (node, aim,false, true);
	json_object_object_add(new_object, "title", 		Dlna_json_object_new_string( str ));
}

static void s_Huawei_ComposeContainerList_FillResult_Maroc(json_object *my_array, int num, char *value, int bufferLength)
{
    json_object *json = json_object_new_object();
    if( json )
    {
        if(my_array)
        {
            json_object_object_add(json, "container_num", json_object_new_int(num));
            json_object_object_add(json, "containers", my_array);
        }
        else
            json_object_object_add(json, "container_num", json_object_new_int(0));
        
        strcpy(value, json_object_to_json_string(json));
        json_object_put(json);
    }
}

static void s_Huawei_ComposeContainerList_Maroc(pClassContentDirectory me, char *value, int bufferLength)
{
	IXML_Document *subdoc = me->subdoc;
	json_object *my_array, *new_object;
	int jlen; 
	int jsonLength = 0;

	bufferLength -= 100;
	HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MANY, bufferLength, 0);
	
	IXML_NodeList* containers = ixmlDocument_getElementsByTagName(subdoc, "container"); 
	int const nb_containers = ixmlNodeList_length (containers);
	HT_DBG_FUNC(nb_containers, "nb_containers = ");
	
	if( (nb_containers > 0) && (me->index < nb_containers) )
	{
		int index = me->index;
		my_array = json_object_new_array();
		for (; me->index < nb_containers; me->index++) 
		{
			IXML_Node* const node = ixmlNodeList_item(containers, me->index);
			new_object = json_object_new_object();
			s_Huawei_ComposeContainerList_Maroc_sub(node, new_object);

			jlen = strlen(json_object_to_json_string(new_object));
			if( jsonLength + jlen > bufferLength )
			{
                json_object_put(new_object);
				break;
			}
			
			jsonLength += jlen;
			json_object_array_add(my_array, new_object);
		}
		
		s_Huawei_ComposeContainerList_FillResult_Maroc(my_array, me->index-index, value, bufferLength);
	}
	else
	{
		s_Huawei_ComposeContainerList_FillResult_Maroc(NULL, 0, value, bufferLength);
	}

	ixmlNodeList_free (containers);
	HT_DBG_FUNC_END(0, value);
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
#if 0
/*
[{"classID":"3","itemID": "3-2-1","filename":"rrtc.mpg"
*/
			str = classID;
			json_object_object_add(new_object, "classID", 		Dlna_json_object_new_string( str ));
			
			aim = "id";
			str = ixmlElement_getAttribute(elem, aim);
			json_object_object_add(new_object, "itemID", 		Dlna_json_object_new_string( str ));

			aim = "dc:title";
			str = XMLUtil_FindFirstElementValue (node, aim,false, true);
			json_object_object_add(new_object, "filename", 		Dlna_json_object_new_string( str ));

			aim = "dc:date";
			str = XMLUtil_FindFirstElementValue (node, aim,false, false);
			json_object_object_add(new_object, "date",		Dlna_json_object_new_string( str ));


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

			aim = "size";
			str = ixmlElement_getAttribute (res, aim);
			json_object_object_add(new_object, aim,		Dlna_json_object_new_string( str ));
			
			aim = "bitrate";
			str = ixmlElement_getAttribute (res, aim);
			json_object_object_add(new_object, aim,		Dlna_json_object_new_string( str ));
			
			aim = "resolution";
			str = ixmlElement_getAttribute (res, aim);
			json_object_object_add(new_object, aim,		Dlna_json_object_new_string( str ));
			
			aim = "duration";
			str = ixmlElement_getAttribute (res, aim);
#if 0
			json_object_object_add(new_object, aim,		Dlna_json_object_new_string( str ));
#else
			if(str)
			{
				unsigned int hh=0, mm = 0, ss = 0, duration =0;
				if (sscanf (str, "%u:%u:%u", &hh, &mm, &ss) == 3 )
				{
					duration = ss + 60*(mm + 60*hh);
					json_object_object_add(new_object, aim, json_object_new_int( duration ));
				}
			}
#endif


			char *transport, *mimetype, *dlna; 
            transport = NULL;
            mimetype = NULL;
            dlna = NULL;
			YX_CD_SplitInfo(protocolInfo, &transport, &mimetype, &dlna);
			str = mimetype;
			json_object_object_add(new_object, "protocolInfo",		Dlna_json_object_new_string( str ));

			aim = "fileFormat";
            char *dlna_pn = dlna? strstr(dlna, "DLNA.ORG_PN") : NULL;
            if (dlna_pn)
            {
                dlna_pn = strchr(dlna, ';');
                if (dlna_pn)
                    *dlna_pn = 0;
    			json_object_object_add(new_object, aim, Dlna_json_object_new_string( dlna ));
                if (dlna_pn)
                    *dlna_pn = ';';
            }
            else
    			json_object_object_add(new_object, aim, Dlna_json_object_new_string( "*" ));
                

			aim = "colorDepth";
			str = ixmlElement_getAttribute (res, aim);
			json_object_object_add(new_object, aim,		Dlna_json_object_new_string( str ));

/*
"filepath":"http://10.10.10.8/video/rrtc.mpg",
" iptvcontentID":"20100803001", " contentSource":"1","iptvuserID":"iptvuser20100898","contentType":"10"," programTitle":"樱桃小丸子48"},
*/
			str = url;
			json_object_object_add(new_object, "filepath", 	Dlna_json_object_new_string( str ));

			aim = "iptvcontentID";
			str = ixmlElement_getAttribute (res, aim);
			json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

			aim = "contentSource";
			str = ixmlElement_getAttribute (res, aim);
			json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

			aim = "iptvuserID";
			str = ixmlElement_getAttribute (res, aim);
			json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

			aim = "contentType";
			str = ixmlElement_getAttribute (res, aim);
			json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

			aim = "programTitle";
			str = ixmlElement_getAttribute (res, aim);
			json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));

			aim = "programRate";
			str = ixmlElement_getAttribute (res, aim);
			json_object_object_add(new_object, aim, 	Dlna_json_object_new_string( str ));
			
			if( protocolInfo ) free(protocolInfo);
			if( url ) free(url);
			if (reslist)
				ixmlNodeList_free (reslist);

#endif
static void s_Huawei_ComposeItemList_FillResult_Maroc(json_object *my_array, int num, char *value, int bufferLength)
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

#undef x_IF_STR
#define x_IF_STR(x)	{}
static void s_Huawei_ComposeItemList_Maroc_sub(pClassContentDirectory me, IXML_Node *node, char *classID, json_object *new_object)
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

	IXML_NodeList* const reslist = ixmlElement_getElementsByTagName ((IXML_Element*)elem, "res");
	int nb_reslist = ixmlNodeList_length (reslist);
	if( nb_reslist > 0 )
		idx = YX_CD_GetPreferred(me, reslist, nb_reslist, &protocolInfo, &url, &mInfo);

	if( idx >= 0 )
		res = (IXML_Element*)ixmlNodeList_item (reslist, idx);
	//			IXML_Element*element = (IXML_Element*)res;

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
    char *dlna_pn = dlna? strstr(dlna, "DLNA.ORG_PN") : NULL;
    if (dlna_pn)
    {
        dlna_pn = strchr(dlna, ';');
        if (dlna_pn)
            *dlna_pn = 0;
		json_object_object_add(new_object, aim, Dlna_json_object_new_string( dlna ));
        if (dlna_pn)
            *dlna_pn = ';';
    }
    else
		json_object_object_add(new_object, aim, Dlna_json_object_new_string( "*" ));
#else
	aim = "fileFormat";
	char *semicolon, *dlna_pn = dlna? strstr(dlna, "DLNA.ORG_PN") : NULL;
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
#endif
	/*
	"filepath":"http://10.10.10.8/video/rrtc.mpg",
	" iptvcontentID":"20100803001", " contentSource":"1","iptvuserID":"iptvuser20100898","contentType":"10"," programTitle":"樱桃小丸子48"},
	*/
	str = url;
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

	if( protocolInfo ) free(protocolInfo);
	if( url ) free(url);
	if (reslist)
		ixmlNodeList_free (reslist);
}

static void s_Huawei_ComposeItemList_Maroc(pClassContentDirectory me, char *classID, char *value, int bufferLength)
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
			s_Huawei_ComposeItemList_Maroc_sub(me, node, classID, new_object);

			jlen = strlen(json_object_to_json_string(new_object));
			if( jsonLength + jlen > bufferLength )
			{
                json_object_put(new_object);
				break;
			}
			
			jsonLength += jlen;
			json_object_array_add(my_array, new_object);
		}

		s_Huawei_ComposeItemList_FillResult_Maroc(my_array, me->index-index, value, bufferLength);
	}
	else
	{
		s_Huawei_ComposeItemList_FillResult_Maroc(NULL, 0, value, bufferLength);
	}

	HT_DBG_FUNC_END(0, value);
	ixmlNodeList_free (items);
}


static int s_Huawei_ParseJs_Maroc(const char *func, char *para, char *value, int len)
{
	int ret = 0;
//	char *str = NULL;
	char *p1, *p2, *p3;

	p1 = p2 = p3 =NULL;
//	len = 4096;
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
*/
	if( strcmp(func, "ItemNumber_Get") == 0 )
	{
		char *criteria = s_Huawei_ParseClassID(para);
		p1 = s_Huawei_FindCommaAndInc(para);
		
		pClassContentDirectory me = (pClassContentDirectory)YX_CD_Create(NULL, NULL, 0, NULL);
		me->SetDmsUdn(me, para);
		me->SetSearchCriteria(me, criteria);
		//me->SetResultFormat(me, 0);
		me->SyncSearch(me, "0", 0, 1);
		if( me->nb_matched > 0 )
			ret = me->nb_matched;
		else
		{
			if(me->nb_returned>0)
			{
				me->SyncSearch(me, "0", 0, 0);
				ret = me->nb_returned;
			}
			else
				ret = 0;
		}
		me->Release(me);
		
		
		json_object *new_obj = json_object_new_object();
		json_object_object_add(new_obj, "classID", Dlna_json_object_new_string(p1));
		json_object_object_add(new_obj, "items_num", json_object_new_int(ret));
		json_object_object_add(new_obj, "deviceID", Dlna_json_object_new_string(para));
		strcpy(value, json_object_to_json_string(new_obj) );
		json_object_put(new_obj);
		return 0;
	}
/*
ItemList_Get:: deviceID,classID,postion,count
*/
	else if( strcmp(func, "ItemList_Get") == 0 )
	{
		char *criteria = s_Huawei_ParseClassID(para);
		p1 = s_Huawei_FindCommaAndInc(para);
		p2 = s_Huawei_FindCommaAndInc(p1);
		p3 = s_Huawei_FindCommaAndInc(p2);
		
//		pClassContentDirectory me = (pClassContentDirectory)CD_create();
		pClassContentDirectory me = s_huawei_browser;
		me->SetDmsUdn(me, para);
		me->SetSearchCriteria(me, criteria);
		//me->SetResultFormat(me, 0);
		me->SyncSearch(me, "0", atoi(p2), atoi(p3));
		s_Huawei_ComposeItemList_Maroc(me, p1, value, len);
//		me->Release(me);
		return 0;
	}
/*
ContainerNumber_Get:: deviceID ,containerID

输入设备ID，目录ID，以JSON格式返回文件夹总数，样例如下：
{"containers_num":"56","deviceID":"<deviceUDN>"}

参数定义：
containerID：目录ID，跟目录为-1，第一次获取时从根目录开始获取。
*/
	else if( strcmp(func, "ContainerNumber_Get") == 0 )
	{
		p1 = s_Huawei_FindCommaAndInc(para);

		char *parent = "upnp:class derivedfrom \"object.container\" and parentID = \"%s\"";
		char *criteria = (char*)malloc(strlen(p1) + strlen(parent) + 10);
		sprintf(criteria, parent, p1);
		
		pClassContentDirectory me = (pClassContentDirectory)YX_CD_Create(NULL, NULL, 0, NULL);
		me->SetDmsUdn(me, para);
//		me->SetSearchCriteria(me, criteria);
		//me->SetResultFormat(me, 0);
		ret = me->SyncBrowseChildren(me, p1, 0, 1);
		if( me->nb_matched > 0 )
			ret = me->nb_matched;
		else
		{
			if(me->nb_returned>0)
			{
				ret = me->SyncBrowseChildren(me, p1, 0, 0);
				ret = me->nb_returned;
			}
			else
				ret = 0;
		}		
		me->Release(me);
		
		json_object *new_obj = json_object_new_object();
		json_object_object_add(new_obj, "containers_num", json_object_new_int(ret));
		json_object_object_add(new_obj, "deviceID", Dlna_json_object_new_string(para));
		strcpy(value, json_object_to_json_string(new_obj) );
		json_object_put(new_obj);
		free(criteria);
		
		return 0;
	}
/*
ContItemNumber_Get:: deviceID ,containerID

输入设备ID，目录ID，以JSON格式返回总数，样例如下：
{"items_num":"56","deviceID":"<deviceUDN>"}
*/
	else if( strcmp(func, "ContItemNumber_Get") == 0 )
	{
		p1 = s_Huawei_FindCommaAndInc(para);

		char *parent = "upnp:class derivedfrom \"object.item\" and parentID = \"%s\"";
		char *criteria = (char*)malloc(strlen(p1) + strlen(parent) + 10);
		sprintf(criteria, parent, p1);
		
		pClassContentDirectory me = (pClassContentDirectory)YX_CD_Create(NULL, NULL, 0, NULL);
		me->SetDmsUdn(me, para);
//		me->SetSearchCriteria(me, criteria);
		//me->SetResultFormat(me, 0);
		me->SyncBrowseChildren(me, p1, 0, 1);
		if( me->nb_matched > 0 )
			ret = me->nb_matched;
		else
		{
			if(me->nb_returned>0)
			{
				me->SyncBrowseChildren(me, p1, 0, 0);
				ret = me->nb_returned;
			}
			else
				ret = 0;
		}		
		me->Release(me);

		json_object *new_obj = json_object_new_object();
		json_object_object_add(new_obj, "items_num", json_object_new_int(ret));
		json_object_object_add(new_obj, "deviceID", Dlna_json_object_new_string(para));
		strcpy(value, json_object_to_json_string(new_obj) );
		json_object_put(new_obj);
		free(criteria);
		return 0;
	}
/*
ContainerList_Get: deviceID ,containerID,postion,count

以JSON封装的格式返回目录数量和列表，样例如下：
{"container_num":"2","containers":"[{"containerID": "0-1-1","parentID":"0","title":"music" },{"containerID": "0-1-2","parentID":"0","title":"movies"}]"}

参数定义：
parentID：父目录ID，当它为"-1"时表示为根目录。
containerID：目录ID
title：目录名称
*/
	else if( strcmp(func, "ContainerList_Get") == 0 )
	{
		p1 = s_Huawei_FindCommaAndInc(para);
		p2 = s_Huawei_FindCommaAndInc(p1);
		p3 = s_Huawei_FindCommaAndInc(p2);

		char *parent = "upnp:class derivedfrom \"object.container\" and parentID = \"%s\"";
		char *criteria = (char*)malloc(strlen(p1) + strlen(parent) + 10);
		sprintf(criteria, parent, p1);
		
//		pClassContentDirectory me = (pClassContentDirectory)CD_create();
		pClassContentDirectory me = s_huawei_browser;
		me->SetDmsUdn(me, para);
//		me->SetSearchCriteria(me, criteria);
		//me->SetResultFormat(me, 0);
		me->SyncBrowseChildren(me, p1, atoi(p2), atoi(p3));
		s_Huawei_ComposeContainerList_Maroc(me, value, len);
//		me->Release(me);
		free(criteria);
		return 0;
	}
/*
ContItemList_Get: deviceID ,containerID,postion,count
*/
	else if( strcmp(func, "ContItemList_Get") == 0 )
	{
		p1 = s_Huawei_FindCommaAndInc(para);
		p2 = s_Huawei_FindCommaAndInc(p1);
		p3 = s_Huawei_FindCommaAndInc(p2);
		
		char *parent = "upnp:class derivedfrom \"object.item\" and parentID = \"%s\"";
		char *criteria = (char*)malloc(strlen(p1) + strlen(parent) + 10);
		sprintf(criteria, parent, p1);
		
//		pClassContentDirectory me = (pClassContentDirectory)CD_create();
		pClassContentDirectory me = s_huawei_browser;
		me->SetDmsUdn(me, para);
//		me->SetSearchCriteria(me, criteria);
		//me->SetResultFormat(me, 0);
		me->SyncBrowseChildren(me, p1, atoi(p2), atoi(p3));
		s_Huawei_ComposeItemList_Maroc(me, p1, value, len);
//		me->Release(me);
		free(criteria);
		return 0;
	}
	else
	{
	}
	
	return -1;
}

#endif

#ifdef _JS_HUAWEI_QTEL_
static int s_Huawei_ParseJs_Qtel(const char *func, char *para, char *value, int len)
{
	int ret = 0;
//	char *str = NULL;
	char *p1, *p2, *p3;

	p1 = p2 = p3 =NULL;
//	len = 4096;

/*
ItemNumber_Get::deviceID,classID
以JSON封装返回描述信息：查找classID为11的返回值。
{"classID":11,"items_num":"56","deviceID":" deviceUDN"}
*/
	if( strcmp(func, "ItemNumber_Get") == 0 )
	{
		char *criteria = s_Huawei_ParseClassID(para);
		p1 = s_Huawei_FindCommaAndInc(para);
		if( !p1 )
			return -1;
		
		pClassContentDirectory me = (pClassContentDirectory)YX_CD_Create(NULL, NULL, 0, NULL);
		me->SetDmsUdn(me, para);
		me->SetSearchCriteria(me, criteria);
		me->SyncSearch(me, "0", 0, 1);
		ret = me->nb_matched;
		me->Release(me);
		
		json_object *new_obj = json_object_new_object();
		json_object_object_add(new_obj, "classID", Dlna_json_object_new_string(p1));
		json_object_object_add(new_obj, "items_num", json_object_new_int(ret));
		json_object_object_add(new_obj, "deviceID", Dlna_json_object_new_string(para));
		strcpy(value, json_object_to_json_string(new_obj) );
		json_object_put(new_obj);
		return 0;
	}
/*
ItemList_Get:: deviceID,classID,postion,count
*/
	else if( strcmp(func, "ItemList_Get") == 0 )
	{
		char *criteria = s_Huawei_ParseClassID(para);
		p1 = s_Huawei_FindCommaAndInc(para);
		p2 = s_Huawei_FindCommaAndInc(p1);
		p3 = s_Huawei_FindCommaAndInc(p2);//解析参数
		if( !p1 || !p2 || !p3 )
			return -1;
		
		pClassContentDirectory me = s_huawei_browser;
		me->SetDmsUdn(me, para);
		me->SetSearchCriteria(me, criteria);
		me->SetFilter(me, "*");
		me->SyncSearch(me, "0", atoi(p2), atoi(p3));
		/*从static int s_ParseBrowseOrSearchResult (Upnp_EventType EventType, void *Event, void *cookie)
返回*/
		int stb_dms = s_Is_HuaweiStbDms(para);
		s_Huawei_ComposeItemList(me, p1, value, len, stb_dms);//组成返回buf
		return 0;
	}
	else
	{
	}
	
	return -1;
}
#endif

