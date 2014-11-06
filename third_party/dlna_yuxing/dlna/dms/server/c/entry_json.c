/*
 * entry_json.c 
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "mime.h"
#include "hitTime.h"
#include "dms.h"


#define IN_OUT(x)	&(x)
typedef struct json_object	PACKET;
/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/
static void s_add_str_to_json(int io, PACKET *json, char *key, char **val)
{
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, (int)(*val), key);
	if(io)
	{
		if(*val)
			json_object_object_add(json, key, json_object_new_string(*val));
	}
	else
	{
		struct json_object *obj = json_object_object_get(json, key);
		if(obj)
		{
			const char *str = json_object_get_string(obj);
			if(str)
				*val = strdup(str);
		}
	}
	HT_DBG_FUNC_END(io, *val);
}

static void s_add_int_to_json(int io, PACKET *json, char *key, int *val)
{
	if(io)
		json_object_object_add(json, key, json_object_new_int(*val));
	else
	{
		struct json_object *obj = json_object_object_get(json, key);
		if(obj)
			*val = json_object_get_int(obj);
	}
}
static void s_add_uin_to_json(int io, PACKET *json, char *key, uint *val)
{
	s_add_int_to_json(io, json, key, (int*)val);
}
static void s_add_lon_to_json(int io, PACKET *json, char *key, long *val)
{
	s_add_int_to_json(io, json, key, (int*)val);
}

static void s_add_uch_to_json(int io, PACKET *json, char *key, uchar *val)
{
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, *val, key);
	if(io)
		json_object_object_add(json, key, json_object_new_int(*val));
	else
	{
		struct json_object *obj = json_object_object_get(json, key);
		if(obj)
			*val = (uchar)json_object_get_int(obj);
	}
	HT_DBG_FUNC_END(io, 0);
}

static void s_add_ush_to_json(int io, PACKET *json, char *key, ushort *val)
{
	if(io)
		json_object_object_add(json, key, json_object_new_int(*val));
	else
	{
		struct json_object *obj = json_object_object_get(json, key);
		if(obj)
			*val = (ushort)json_object_get_int(obj);
	}
}

static void s_add_llg_to_json(int io, PACKET *json, char *key, long long *val)
{
	if(io)
	{
		char temp[256];
		sprintf(temp, "%lld", *val);
		json_object_object_add(json, key, json_object_new_string(temp));
	}
	else
	{
		struct json_object *obj = json_object_object_get(json, key);
		if(obj)
		{
			const char *str = json_object_get_string(obj);
			if(str)
				sscanf(str, "%lld", val);
		}
	}
}

static void s_add_jso_to_json(int io, PACKET *json, char *key, struct json_object **val)
{
	const char *str;
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, (int)(*val), key);
	if(io)
	{
		if(*val)
		{
			str = json_object_to_json_string(*val);
			json_object_object_add(json, key, dlna_json_tokener_parse(str));
		}
	}
	else
	{
		struct json_object *obj = json_object_object_get(json, key);
		if(obj && (str = json_object_to_json_string(obj)))
			*val = dlna_json_tokener_parse(str);
	}
	HT_DBG_FUNC_END(io, 0);
}

static int s_is_both_ok(int io, PACKET *j_parent, void **pp, PACKET **json, const char *str, int len)
{
	if((io && *pp && (*json = json_object_new_object())) 
		|| (!io && (*json = json_object_object_get(j_parent, str)) && (*pp = coo_malloc_bzero(len))) )
		return 1;
	else
		return 0;
}


/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/
static void s_json_to_mime_type(int io, PACKET *j_parent, x_DMS_CI **pp)
{
	PACKET *json;
	int len = sizeof(x_DMS_CI);
	const char *str = "mime_type";
	
	if(s_is_both_ok(io, j_parent, (void **)pp, &json, str, len))
	{
		s_add_str_to_json(io, json, "upnp_class",		IN_OUT((*pp)->upnp_class));
		
		if(io)
			json_object_object_add(j_parent, str, json);
	}
}

static void s_json_to_container(int io, PACKET *j_parent, C_DMS_CCI **pp)
{
	PACKET *json;
	int len = sizeof(C_DMS_CCI);
	const char *str = "object";

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, (int)(*pp), "*pp = ");
	if(s_is_both_ok(io, j_parent, (void **)pp, &json, str, len))
	{
		s_json_to_mime_type(io, json,					IN_OUT((*pp)->mime_type));
		if(!io)	(*pp)->malloced = 1;
		s_add_ush_to_json(io, json, "type", 			IN_OUT((*pp)->type));
		s_add_uch_to_json(io, json, "minor_type", 		IN_OUT((*pp)->minor_type));
		
		s_add_str_to_json(io, json, "title",			IN_OUT((*pp)->title));
		s_add_str_to_json(io, json, "keyword",			IN_OUT((*pp)->keyword));
		s_add_lon_to_json(io, json, "date",				IN_OUT((*pp)->date));

		if(io)
			json_object_object_add(j_parent, str, json);
	}
	HT_DBG_FUNC_END(io, 0);
}

static void s_json_to_protocol_info(int io, PACKET *j_parent, C_DMS_IPI **xx)
{
	x_DMS_IPI **pp = (x_DMS_IPI **)xx;
	PACKET *json;
	int len = sizeof(C_DMS_IPI);
	const char *str = "protocol_info";

	if(s_is_both_ok(io, j_parent, (void **)pp, &json, str, len))
	{
		s_add_str_to_json(io, json, "id",				IN_OUT((*pp)->id));
		s_add_str_to_json(io, json, "mime", 			IN_OUT((*pp)->mime));
		s_add_str_to_json(io, json, "label",			IN_OUT((*pp)->label));
		s_add_uin_to_json(io, json, "class",			IN_OUT((*pp)->class));

		if(io)
			json_object_object_add(j_parent, str, json);
	}
}	

static void s_json_to_audio_info(int io, PACKET *j_parent, WAVEFORMATEX **pp)
{
	PACKET *json;
	int len = sizeof(WAVEFORMATEX);
	const char *str = "audio_info";

	if(s_is_both_ok(io, j_parent, (void **)pp, &json, str, len))
	{
		s_add_ush_to_json(io, json, "wFormatTag",		IN_OUT((*pp)->wFormatTag));
		s_add_ush_to_json(io, json, "nChannels", 		IN_OUT((*pp)->nChannels));
		s_add_lon_to_json(io, json, "nSamplesPerSec",	IN_OUT((*pp)->nSamplesPerSec));
		s_add_lon_to_json(io, json, "nAvgBytesPerSec",	IN_OUT((*pp)->nAvgBytesPerSec));
		s_add_ush_to_json(io, json, "nBlockAlign",		IN_OUT((*pp)->nBlockAlign));
		s_add_ush_to_json(io, json, "wBitsPerSample",	IN_OUT((*pp)->wBitsPerSample));
		s_add_ush_to_json(io, json, "cbSize",			IN_OUT((*pp)->cbSize));
		
		if(io)
			json_object_object_add(j_parent, str, json);
	}
}

static void s_json_to_upnp_info(int io, PACKET *j_parent, C_DMS_UPNI **pp)
{
	PACKET *json;
	int len = sizeof(C_DMS_UPNI);
	const char *str = "upnp_info";

	if(s_is_both_ok(io, j_parent, (void **)pp, &json, str, len))
	{
		s_add_str_to_json(io, json, "creator",			IN_OUT((*pp)->creator));
		s_add_str_to_json(io, json, "artist",			IN_OUT((*pp)->artist));
		s_add_str_to_json(io, json, "actor", 			IN_OUT((*pp)->actor));
		s_add_str_to_json(io, json, "producer",			IN_OUT((*pp)->producer));
		s_add_str_to_json(io, json, "genre", 			IN_OUT((*pp)->genre));
		s_add_str_to_json(io, json, "description",		IN_OUT((*pp)->description));
		
		if(io)
			json_object_object_add(j_parent, str, json);
	}
}

static void s_json_to_pvr(int io, PACKET *j_parent, C_DMS_PVRI **pp)
{
	PACKET *json;
	int len = sizeof(C_DMS_PVRI);
	const char *str = "pvr";

	if(s_is_both_ok(io, j_parent, (void **)pp, &json, str, len))
	{
		s_add_str_to_json(io, json, "recordedStartDateTime",IN_OUT((*pp)->recordedStartDateTime));
		s_add_str_to_json(io, json, "recordedEndDateTime",	IN_OUT((*pp)->recordedEndDateTime));
		
		if(io)
			json_object_object_add(j_parent, str, json);
	}
}

static void s_json_to_tuner(int io, PACKET *j_parent, C_DMS_TUNI **pp)
{
	PACKET *json;
	int len = sizeof(C_DMS_TUNI);
	const char *str = "tuner";

	if(s_is_both_ok(io, j_parent, (void **)pp, &json, str, len))
	{
		s_add_str_to_json(io, json, "frequency",		IN_OUT((*pp)->frequency));
		s_add_str_to_json(io, json, "channel",			IN_OUT((*pp)->channel));
		
		if(io)
			json_object_object_add(j_parent, str, json);
	}
}

static void s_json_to_optional(int io, PACKET *j_parent, C_DMS_OPTI **pp)
{
	PACKET *json;
	int len = sizeof(C_DMS_OPTI);
	const char *str = "optional";

	if(s_is_both_ok(io, j_parent, (void **)pp, &json, str, len))
	{
		s_json_to_audio_info(io, json,			 		IN_OUT((*pp)->audio_info));
		s_json_to_upnp_info(io, json,			 		IN_OUT((*pp)->upnp_info));
		s_json_to_pvr(io, json,			 				IN_OUT((*pp)->pvr));
		s_json_to_tuner(io, json,			 			IN_OUT((*pp)->tuner));

		if(io)
			json_object_object_add(j_parent, str, json);
	}
}	

static void s_json_to_thumbnail(int io, PACKET *j_parent, C_DMS_THUI **pp)
{
	PACKET *json;
	int len = sizeof(C_DMS_THUI);
	const char *str = "thumbnail";

	if(s_is_both_ok(io, j_parent, (void **)pp, &json, str, len))
	{
		s_add_int_to_json(io, json, "type", 			IN_OUT((*pp)->type));
		s_add_uin_to_json(io, json, "offset", 			IN_OUT((*pp)->offset));
		s_add_uin_to_json(io, json, "length", 			IN_OUT((*pp)->length));
		
		s_add_ush_to_json(io, json, "color_depth", 		IN_OUT((*pp)->color_depth));
		s_add_ush_to_json(io, json, "reserved", 		IN_OUT((*pp)->reserved));
		s_add_ush_to_json(io, json, "res_width", 		IN_OUT((*pp)->res_width));
		s_add_ush_to_json(io, json, "res_height", 		IN_OUT((*pp)->res_height));
		
		s_json_to_protocol_info(io, json,				IN_OUT((*pp)->protocol_info));
		if(!io)	(*pp)->malloced = 1;

		if(io)
			json_object_object_add(j_parent, str, json);
	}
}	

static int s_json_to_item(int io, PACKET *j_parent, C_DMS_CMI **pp)
{
	PACKET *json;
	int len = sizeof(C_DMS_CMI);
	const char *str = "object";

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, (int)(*pp), "*pp = ");
	if(s_is_both_ok(io, j_parent, (void **)pp, &json, str, len))
	{
		/* basic */
		s_json_to_protocol_info(io, json, 				IN_OUT((*pp)->protocol_info));
		if(!io)	(*pp)->malloced = 1;

		s_add_uch_to_json(io, json, "reserved", 		IN_OUT((*pp)->reserved));
		s_add_ush_to_json(io, json, "color_depth", 		IN_OUT((*pp)->color_depth));
		
		s_add_str_to_json(io, json, "title", 			IN_OUT((*pp)->title));
		s_add_str_to_json(io, json, "keyword", 			IN_OUT((*pp)->keyword));
		s_add_llg_to_json(io, json, "size", 			IN_OUT((*pp)->size));
		s_add_lon_to_json(io, json, "date", 			IN_OUT((*pp)->date));

		/* common */
		s_add_uin_to_json(io, json, "duration", 		IN_OUT((*pp)->duration));
		s_add_uin_to_json(io, json, "bitrate", 			IN_OUT((*pp)->bitrate));
		s_add_ush_to_json(io, json, "res_width", 		IN_OUT((*pp)->res_width));
		s_add_ush_to_json(io, json, "res_height", 		IN_OUT((*pp)->res_height));
		
		/*  more */
		s_json_to_optional(io, json,					IN_OUT((*pp)->optional));
		s_json_to_thumbnail(io, json, 					IN_OUT((*pp)->thumbnail));

		/* ext */
		s_add_jso_to_json(io, json, "extension",		IN_OUT((*pp)->extension));
		s_add_str_to_json(io, json, "user_info", 		IN_OUT((*pp)->user_info));
		
		if(io)
			json_object_object_add(j_parent, str, json);
	}

	HT_DBG_FUNC_END(io, 0);
	return 0;
}

static void s_json_to_virtual_object(int io, PACKET *json, C_DMS_VO **pp)
{
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MYRIAD, (int)(*pp), "*pp = ");
	if((io && *pp ) || (!io && (*pp = COO_OBJECT_NEW(C_DMS_VO))))
	{
		s_add_uch_to_json(io, json, "restricted", 		IN_OUT((*pp)->restricted));
		s_add_uch_to_json(io, json, "searchable", 		IN_OUT((*pp)->searchable));
		s_add_uch_to_json(io, json, "is_item", 			IN_OUT((*pp)->is_item));
		s_add_uch_to_json(io, json, "source_type", 		IN_OUT((*pp)->source_type));

		s_add_int_to_json(io, json, "id", 				IN_OUT((*pp)->id));
		s_add_uin_to_json(io, json, "updateID",			IN_OUT((*pp)->updateID));
	}
	HT_DBG_FUNC_END(io, 0);
}


/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/
int Dms_VirtualObject_To_String(C_DMS_VO *entry, int onlybasic, char **info)
{
	int ret = -1, io = 1;
	
	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MANY, (int)entry, "entry = ");
	if(!entry || !info)
		goto s_EXIT;

	PACKET *json = json_object_new_object();
	s_json_to_virtual_object(io, json, &entry);
	
	if(!onlybasic)
	{
		if(Dms_VirtualObject_IsItem(entry))
		{
			C_DMS_CMI *item = entry->object;
			s_json_to_item(io, json, &item);
		}
		else if(Dms_VirtualObject_IsContainer(entry))
		{
			C_DMS_CCI *container = entry->object;
			s_json_to_container(io, json, &container);
		}
		else
		{
		}
	}
	
	*info = strdup(json_object_to_json_string(json));
	HT_DBG_FUNC(0, *info);
	
	json_object_put(json);
	ret = 0;
	
s_EXIT:	
	HT_DBG_FUNC_END(ret, 0);
	return ret;
}

int Dms_VirtualObject_From_String(char *info, C_DMS_VO **p_entry)
{
	int io = 0, x, ret = -1;
	PACKET *json = NULL, *obj = NULL;

	HT_DBG_FUNC_START(HT_MOD_DMS, HT_BIT_MANY, 0, info);
	if(!p_entry || !info)
		goto s_EXIT;

	ret--;
	json = dlna_json_tokener_parse(info);
	if(!json )
		goto s_EXIT;

	ret--;
	obj = json_object_object_get(json, "is_item");
	if(!obj)
		goto s_EXIT;
	
	x = json_object_get_int(obj);
	*p_entry = NULL;
	s_json_to_virtual_object(io, json, p_entry);

	HT_DBG_FUNC(x, NULL);
	ret = 0;
	if(e_OBJTYPE_ITEM == x)
	{
		C_DMS_CMI *p = (*p_entry)->object;
		s_json_to_item(io, json, &p);
		(*p_entry)->object = p;
		(*p_entry)->FreeObject = Dms_VirtualObject_FreeItem;
	}
	else if(e_OBJTYPE_CONTAINER == x)
	{
		C_DMS_CCI *p = (*p_entry)->object;
		s_json_to_container(io, json, &p);
		(*p_entry)->object = p;
		(*p_entry)->FreeObject = Dms_VirtualObject_FreeContainerSelf;
	}
	else
	{
		ret = -10;
	}

	
s_EXIT:	
	if(json)
		json_object_put(json);
	HT_DBG_FUNC_END(ret, NULL);
	return ret;
}

/*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------*/


